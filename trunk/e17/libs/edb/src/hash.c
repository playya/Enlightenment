/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994
 *	Margo Seltzer.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)hash.c	10.63 (Sleepycat) 12/11/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "edb_int.h"
#include "shqueue.h"
#include "edb_page.h"
#include "edb_am.h"
#include "edb_ext.h"
#include "hash.h"
#include "btree.h"
#include "log.h"
#include "edb_shash.h"
#include "lock.h"
#include "lock_ext.h"

static int  __ham_c_close __P((DBC *));
static int  __ham_c_del __P((DBC *, u_int32_t));
static int  __ham_c_destroy __P((DBC *));
static int  __ham_c_get __P((DBC *, DBT *, DBT *, u_int32_t));
static int  __ham_c_put __P((DBC *, DBT *, DBT *, u_int32_t));
static int  __ham_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
static int  __ham_dup_return __P((DBC *, DBT *, u_int32_t));
static int  __ham_expand_table __P((DBC *));
static void __ham_init_htab __P((DBC *, u_int32_t, u_int32_t));
static int  __ham_lookup __P((DBC *, const DBT *, u_int32_t, edb_lockmode_t));
static int  __ham_overwrite __P((DBC *, DBT *));

/************************** INTERFACE ROUTINES ***************************/
/* OPEN/CLOSE */

/*
 * __ham_open --
 *
 * PUBLIC: int __ham_open __P((DB *, DB_INFO *));
 */
int
__ham_open(edbp, edbinfo)
	DB *edbp;
	DB_INFO *edbinfo;
{
	DB_ENV *edbenv;
	DBC *edbc;
	HASH_CURSOR *hcp;
	int file_existed, ret;

	edbc = NULL;
	edbenv = edbp->edbenv;

	/* Set the hash function if specified by the user. */
	if (edbinfo != NULL && edbinfo->h_hash != NULL)
		edbp->h_hash = edbinfo->h_hash;

	/*
	 * Initialize the remaining fields of the edbp.  The only function
	 * that differs from the default set is __ham_stat().
	 */
	edbp->internal = NULL;
	edbp->am_close = __ham_close;
	edbp->del = __ham_delete;
	edbp->stat = __ham_stat;

	/* Get a cursor we can use for the rest of this function. */
	if ((ret = edbp->cursor(edbp, NULL, &edbc, 0)) != 0)
		goto out;

	hcp = (HASH_CURSOR *)edbc->internal;
	GET_META(edbp, hcp, ret);
	if (ret != 0)
		goto out;

	/*
	 * If this is a new file, initialize it, and put it back dirty.
	 */

	/* Initialize the hdr structure */
	if (hcp->hdr->magic == DB_HASHMAGIC) {
		file_existed = 1;
		/* File exists, verify the data in the header. */
		if (edbp->h_hash == NULL)
			edbp->h_hash =
			    hcp->hdr->version < 5 ? __ham_func4 : __ham_func5;
		if (edbp->h_hash(CHARKEY, sizeof(CHARKEY)) !=
		    hcp->hdr->h_charkey) {
			__edb_err(edbp->edbenv, "hash: incompatible hash function");
			ret = EINVAL;
			goto out;
		}
		if (F_ISSET(hcp->hdr, DB_HASH_DUP))
			F_SET(edbp, DB_AM_DUP);
	} else {
		/*
		 * File does not exist, we must initialize the header.  If
		 * locking is enabled that means getting a write lock first.
		 */
		file_existed = 0;
		if (F_ISSET(edbp, DB_AM_LOCKING) &&
		    ((ret = lock_put(edbenv->lk_info, hcp->hlock)) != 0 ||
		    (ret = lock_get(edbenv->lk_info, edbc->locker, 0,
		        &edbc->lock_edbt, DB_LOCK_WRITE, &hcp->hlock)) != 0)) {
			if (ret < 0)
				ret = EAGAIN;
			goto out;
		}

		__ham_init_htab(edbc, edbinfo != NULL ? edbinfo->h_nelem : 0,
		    edbinfo != NULL ? edbinfo->h_ffactor : 0);
		if (F_ISSET(edbp, DB_AM_DUP))
			F_SET(hcp->hdr, DB_HASH_DUP);
		if ((ret = __ham_dirty_page(edbp, (PAGE *)hcp->hdr)) != 0)
			goto out;
	}

	/* Release the meta data page */
	RELEASE_META(edbp, hcp);
	if ((ret  = edbc->c_close(edbc)) != 0)
		goto out;

	/* Sync the file so that we know that the meta data goes to disk. */
	if (!file_existed && (ret = edbp->sync(edbp, 0)) != 0)
		goto out;
	return (0);

out:	(void)__ham_close(edbp);
	return (ret);
}

/*
 * PUBLIC: int __ham_close __P((DB *));
 */
int
__ham_close(edbp)
	DB *edbp;
{
	COMPQUIET(edbp, NULL);
	return (0);
}

/************************** LOCAL CREATION ROUTINES **********************/
/*
 * Returns 0 on No Error
 */
static void
__ham_init_htab(edbc, nelem, ffactor)
	DBC *edbc;
	u_int32_t nelem, ffactor;
{
	DB *edbp;
	HASH_CURSOR *hcp;
	int32_t l2, nbuckets;

	hcp = (HASH_CURSOR *)edbc->internal;
	edbp = edbc->edbp;
	memset(hcp->hdr, 0, sizeof(HASHHDR));
	hcp->hdr->ffactor = ffactor;
	hcp->hdr->pagesize = edbp->pgsize;
	ZERO_LSN(hcp->hdr->lsn);
	hcp->hdr->magic = DB_HASHMAGIC;
	hcp->hdr->version = DB_HASHVERSION;

	if (edbp->h_hash == NULL)
		edbp->h_hash = hcp->hdr->version < 5 ? __ham_func4 : __ham_func5;
	hcp->hdr->h_charkey = edbp->h_hash(CHARKEY, sizeof(CHARKEY));
	if (nelem != 0 && hcp->hdr->ffactor != 0) {
		nelem = (nelem - 1) / hcp->hdr->ffactor + 1;
		l2 = __edb_log2(nelem > 2 ? nelem : 2);
	} else
		l2 = 2;

	nbuckets = 1 << l2;

	hcp->hdr->ovfl_point = l2;
	hcp->hdr->last_freed = PGNO_INVALID;

	hcp->hdr->max_bucket = hcp->hdr->high_mask = nbuckets - 1;
	hcp->hdr->low_mask = (nbuckets >> 1) - 1;
	memcpy(hcp->hdr->uid, edbp->fileid, DB_FILE_ID_LEN);
}

static int
__ham_delete(edbp, txn, key, flags)
	DB *edbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DBC *edbc;
	HASH_CURSOR *hcp;
	int ret, tret;

	DB_PANIC_CHECK(edbp);

	if ((ret =
	    __edb_delchk(edbp, key, flags, F_ISSET(edbp, DB_AM_RDONLY))) != 0)
		return (ret);

	if ((ret = edbp->cursor(edbp, txn, &edbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, txn, "ham_delete", key, NULL, flags);

	hcp = (HASH_CURSOR *)edbc->internal;
	GET_META(edbp, hcp, ret);
	if (ret != 0)
		goto out;

	hcp->stats.hash_deleted++;
	if ((ret = __ham_lookup(edbc, key, 0, DB_LOCK_WRITE)) == 0)
		if (F_ISSET(hcp, H_OK))
			ret = __ham_del_pair(edbc, 1);
		else
			ret = DB_NOTFOUND;

	RELEASE_META(edbp, hcp);
out:	if ((tret = edbc->c_close(edbc)) != 0 && ret == 0)
		ret = tret;
	return (ret);
}

/* ****************** CURSORS ********************************** */
/*
 * __ham_c_init --
 *	Initialize the hash-specific portion of a cursor.
 *
 * PUBLIC: int __ham_c_init __P((DBC *));
 */
int
__ham_c_init(edbc)
	DBC *edbc;
  {
	HASH_CURSOR *new_curs;
	int ret;

	if ((ret = __os_calloc(1, sizeof(struct cursor_t), &new_curs)) != 0)
		return (ret);
	if ((ret =
	    __os_malloc(edbc->edbp->pgsize, NULL, &new_curs->split_buf)) != 0) {
		__os_free(new_curs, sizeof(*new_curs));
		return (ret);
	}

	new_curs->edbc = edbc;

	edbc->internal = new_curs;
	edbc->c_am_close = __ham_c_close;
	edbc->c_am_destroy = __ham_c_destroy;
	edbc->c_del = __ham_c_del;
	edbc->c_get = __ham_c_get;
	edbc->c_put = __ham_c_put;

	__ham_item_init(new_curs);

	return (0);
}

/*
 * __ham_c_close --
 *	Close down the cursor from a single use.
 */
static int
__ham_c_close(edbc)
	DBC *edbc;
{
	int ret;

	if ((ret = __ham_item_done(edbc, 0)) != 0)
		return (ret);

	__ham_item_init((HASH_CURSOR *)edbc->internal);
	return (0);
}

/*
 * __ham_c_destroy --
 *	Cleanup the access method private part of a cursor.
 */
static int
__ham_c_destroy(edbc)
	DBC *edbc;
{
	HASH_CURSOR *hcp;

	hcp = (HASH_CURSOR *)edbc->internal;
	if (hcp->split_buf != NULL)
		__os_free(hcp->split_buf, edbc->edbp->pgsize);
	__os_free(hcp, sizeof(HASH_CURSOR));

	return (0);
}

static int
__ham_c_del(edbc, flags)
	DBC *edbc;
	u_int32_t flags;
{
	DB *edbp;
	DBT repledbt;
	HASH_CURSOR *hcp;
	HASH_CURSOR save_curs;
	edb_pgno_t ppgno, chg_pgno;
	int ret, t_ret;

	DEBUG_LWRITE(edbc, edbc->txn, "ham_c_del", NULL, NULL, flags);
	edbp = edbc->edbp;
	DB_PANIC_CHECK(edbp);
	hcp = (HASH_CURSOR *)edbc->internal;

	if ((ret = __edb_cdelchk(edbc->edbp, flags,
	    F_ISSET(edbc->edbp, DB_AM_RDONLY), IS_VALID(hcp))) != 0)
		return (ret);

	if (F_ISSET(hcp, H_DELETED))
		return (DB_NOTFOUND);

	/*
	 * If we are in the concurrent DB product and this cursor
	 * is not a write cursor, then this request is invalid.
	 * If it is a simple write cursor, then we need to upgrade its
	 * lock.
	 */
	if (F_ISSET(edbp, DB_AM_CDB)) {
		/* Make sure it's a valid update cursor. */
		if (!F_ISSET(edbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

		if (F_ISSET(edbc, DBC_RMW) &&
		    (ret = lock_get(edbp->edbenv->lk_info, edbc->locker,
		    DB_LOCK_UPGRADE, &edbc->lock_edbt, DB_LOCK_WRITE,
		    &edbc->mylock)) != 0)
			return (EAGAIN);
	}

	GET_META(edbp, hcp, ret);
	if (ret != 0)
		return (ret);

	SAVE_CURSOR(hcp, &save_curs);
	hcp->stats.hash_deleted++;

	if ((ret = __ham_get_cpage(edbc, DB_LOCK_WRITE)) != 0)
		goto out;
	if (F_ISSET(hcp, H_ISDUP) && hcp->dpgno != PGNO_INVALID) {
		/*
		 * We are about to remove a duplicate from offpage.
		 *
		 * There are 4 cases.
		 * 1. We will remove an item on a page, but there are more
		 *    items on that page.
		 * 2. We will remove the last item on a page, but there is a
		 *    following page of duplicates.
		 * 3. We will remove the last item on a page, this page was the
		 *    last page in a duplicate set, but there were dups before
		 *    it.
		 * 4. We will remove the last item on a page, removing the last
		 *    duplicate.
		 * In case 1 hcp->dpagep is unchanged.
		 * In case 2 hcp->dpagep comes back pointing to the next dup
		 *     page.
		 * In case 3 hcp->dpagep comes back NULL.
		 * In case 4 hcp->dpagep comes back NULL.
		 *
		 * Case 4 results in deleting the pair off the master page.
		 * The normal code for doing this knows how to delete the
		 * duplicates, so we will handle this case in the normal code.
		 */
		ppgno = PREV_PGNO(hcp->dpagep);
		if (ppgno == PGNO_INVALID &&
		    NEXT_PGNO(hcp->dpagep) == PGNO_INVALID &&
		    NUM_ENT(hcp->dpagep) == 1)
			goto normal;

		/* Remove item from duplicate page. */
		chg_pgno = hcp->dpgno;
		if ((ret = __edb_drem(edbc,
		    &hcp->dpagep, hcp->dndx, __ham_del_page)) != 0)
			goto out;

		if (hcp->dpagep == NULL) {
			if (ppgno != PGNO_INVALID) {		/* Case 3 */
				hcp->dpgno = ppgno;
				if ((ret = __ham_get_cpage(edbc,
				    DB_LOCK_READ)) != 0)
					goto out;
				hcp->dndx = NUM_ENT(hcp->dpagep);
				F_SET(hcp, H_DELETED);
			} else {				/* Case 4 */
				ret = __ham_del_pair(edbc, 1);
				hcp->dpgno = PGNO_INVALID;
				/*
				 * Delpair updated the cursor queue, so we
				 * don't have to do that here.
				 */
				chg_pgno = PGNO_INVALID;
			}
		} else if (PGNO(hcp->dpagep) != hcp->dpgno) {
			hcp->dndx = 0;				/* Case 2 */
			hcp->dpgno = PGNO(hcp->dpagep);
			if (ppgno == PGNO_INVALID)
				memcpy(HOFFDUP_PGNO(P_ENTRY(hcp->pagep,
				    H_DATAINDEX(hcp->bndx))),
				    &hcp->dpgno, sizeof(edb_pgno_t));
			/*
			 * We need to put the master page here, because
			 * although we have a duplicate page, the master
			 * page is dirty, and ham_item_done assumes that
			 * if you have a duplicate page, it's the only one
			 * that can be dirty.
			 */
			ret = __ham_put_page(edbp, hcp->pagep, 1);
			hcp->pagep = NULL;
			F_SET(hcp, H_DELETED);
		} else						/* Case 1 */
			F_SET(hcp, H_DELETED);
		if (chg_pgno != PGNO_INVALID)
			__ham_c_update(hcp, chg_pgno, 0, 0, 1);
	} else if (F_ISSET(hcp, H_ISDUP)) {			/* on page */
		if (hcp->dup_off == 0 && DUP_SIZE(hcp->dup_len) ==
		    LEN_HDATA(hcp->pagep, hcp->hdr->pagesize, hcp->bndx))
			ret = __ham_del_pair(edbc, 1);
		else {
			repledbt.flags = 0;
			F_SET(&repledbt, DB_DBT_PARTIAL);
			repledbt.doff = hcp->dup_off;
			repledbt.dlen = DUP_SIZE(hcp->dup_len);
			repledbt.size = 0;
			repledbt.data =
			    HKEYDATA_DATA(H_PAIRDATA(hcp->pagep, hcp->bndx));
			ret = __ham_replpair(edbc, &repledbt, 0);
			hcp->dup_tlen -= DUP_SIZE(hcp->dup_len);
			F_SET(hcp, H_DELETED);
			__ham_c_update(hcp, hcp->pgno,
			    DUP_SIZE(hcp->dup_len), 0, 1);
		}

	} else
		/* Not a duplicate */
normal:		ret = __ham_del_pair(edbc, 1);

out:	if ((t_ret = __ham_item_done(edbc, ret == 0)) != 0 && ret == 0)
		ret = t_ret;
	RELEASE_META(edbp, hcp);
	RESTORE_CURSOR(edbp, hcp, &save_curs, ret);
	if (F_ISSET(edbp, DB_AM_CDB) && F_ISSET(edbc, DBC_RMW))
		(void)__lock_downgrade(edbp->edbenv->lk_info, edbc->mylock,
		    DB_LOCK_IWRITE, 0);
	return (ret);
}

static int
__ham_c_get(edbc, key, data, flags)
	DBC *edbc;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DB *edbp;
	HASH_CURSOR *hcp, save_curs;
	edb_lockmode_t lock_type;
	int get_key, ret, t_ret;

	DEBUG_LREAD(edbc, edbc->txn, "ham_c_get",
	    flags == DB_SET || flags == DB_SET_RANGE ? key : NULL,
	    NULL, flags);

	hcp = (HASH_CURSOR *)edbc->internal;
	edbp = edbc->edbp;
	DB_PANIC_CHECK(edbp);
	SAVE_CURSOR(hcp, &save_curs);
	if ((ret =
	    __edb_cgetchk(edbp, key, data, flags, IS_VALID(hcp))) != 0)
		return (ret);

	/* Clear OR'd in additional bits so we can check for flag equality. */
	if (LF_ISSET(DB_RMW)) {
		lock_type = DB_LOCK_WRITE;
		LF_CLR(DB_RMW);
	} else
		lock_type = DB_LOCK_READ;

	GET_META(edbp, hcp, ret);
	if (ret != 0)
		return (ret);
	hcp->stats.hash_get++;
	hcp->seek_size = 0;

	ret = 0;
	get_key = 1;
	switch (flags) {
	case DB_PREV:
		if (hcp->bucket != BUCKET_INVALID) {
			ret = __ham_item_prev(edbc, lock_type);
			break;
		}
		/* FALLTHROUGH */
	case DB_LAST:
		ret = __ham_item_last(edbc, lock_type);
		break;
	case DB_FIRST:
		ret = __ham_item_first(edbc, lock_type);
		break;
	case DB_NEXT_DUP:
		if (hcp->bucket == BUCKET_INVALID)
			ret = EINVAL;
		else {
			F_SET(hcp, H_DUPONLY);
			ret = __ham_item_next(edbc, lock_type);
		}
		break;
	case DB_NEXT:
		if (hcp->bucket == BUCKET_INVALID)
			hcp->bucket = 0;
		ret = __ham_item_next(edbc, lock_type);
		break;
	case DB_SET:
	case DB_SET_RANGE:
	case DB_GET_BOTH:
		if (F_ISSET(edbc, DBC_CONTINUE)) {
			F_SET(hcp, H_DUPONLY);
			ret = __ham_item_next(edbc, lock_type);
		} else if (F_ISSET(edbc, DBC_KEYSET))
			ret = __ham_item(edbc, lock_type);
		else
			ret = __ham_lookup(edbc, key, 0, lock_type);
		get_key = 0;
		break;
	case DB_CURRENT:
		if (F_ISSET(hcp, H_DELETED)) {
			ret = DB_KEYEMPTY;
			goto out;
		}

		ret = __ham_item(edbc, lock_type);
		break;
	}

	/*
	 * Must always enter this loop to do error handling and
	 * check for big key/data pair.
	 */
	while (1) {
		if (ret != 0 && ret != DB_NOTFOUND)
			goto out1;
		else if (F_ISSET(hcp, H_OK)) {
			/* Get the key. */
			if (get_key && (ret = __edb_ret(edbp, hcp->pagep,
			    H_KEYINDEX(hcp->bndx), key, &edbc->rkey.data,
			    &edbc->rkey.size)) != 0)
				goto out1;

			ret = __ham_dup_return(edbc, data, flags);
			break;
		} else if (!F_ISSET(hcp, H_NOMORE)) {
			abort();
			break;
		}

		/*
		 * Ran out of entries in a bucket; change buckets.
		 */
		switch (flags) {
			case DB_LAST:
			case DB_PREV:
				ret = __ham_item_done(edbc, 0);
				if (hcp->bucket == 0) {
					ret = DB_NOTFOUND;
					goto out1;
				}
				hcp->bucket--;
				hcp->bndx = NDX_INVALID;
				if (ret == 0)
					ret = __ham_item_prev(edbc, lock_type);
				break;
			case DB_FIRST:
			case DB_NEXT:
				ret = __ham_item_done(edbc, 0);
				hcp->bndx = NDX_INVALID;
				hcp->bucket++;
				hcp->pgno = PGNO_INVALID;
				hcp->pagep = NULL;
				if (hcp->bucket > hcp->hdr->max_bucket) {
					ret = DB_NOTFOUND;
					goto out1;
				}
				if (ret == 0)
					ret = __ham_item_next(edbc, lock_type);
				break;
			case DB_GET_BOTH:
			case DB_NEXT_DUP:
			case DB_SET:
			case DB_SET_RANGE:
				/* Key not found. */
				ret = DB_NOTFOUND;
				goto out1;
		}
	}
out1:	if ((t_ret = __ham_item_done(edbc, 0)) != 0 && ret == 0)
		ret = t_ret;
out:	RELEASE_META(edbp, hcp);
	RESTORE_CURSOR(edbp, hcp, &save_curs, ret);
	return (ret);
}

static int
__ham_c_put(edbc, key, data, flags)
	DBC *edbc;
	DBT *key;
	DBT *data;
	u_int32_t flags;
{
	DB *edbp;
	DBT tmp_val, *myval;
	HASH_CURSOR *hcp, save_curs;
	u_int32_t nbytes;
	int ret, t_ret;

	edbp = edbc->edbp;
	DB_PANIC_CHECK(edbp);
	DEBUG_LWRITE(edbc, edbc->txn, "ham_c_put",
	    flags == DB_KEYFIRST || flags == DB_KEYLAST ? key : NULL,
	    data, flags);
	hcp = (HASH_CURSOR *)edbc->internal;

	if ((ret = __edb_cputchk(edbp, key, data, flags,
	    F_ISSET(edbp, DB_AM_RDONLY), IS_VALID(hcp))) != 0)
		return (ret);

	if (F_ISSET(hcp, H_DELETED) &&
	    flags != DB_KEYFIRST && flags != DB_KEYLAST)
		return (DB_NOTFOUND);

	/*
	 * If we are in the concurrent DB product and this cursor
	 * is not a write cursor, then this request is invalid.
	 * If it is a simple write cursor, then we need to upgrade its
	 * lock.
	 */
	if (F_ISSET(edbp, DB_AM_CDB)) {
		/* Make sure it's a valid update cursor. */
		if (!F_ISSET(edbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

		if (F_ISSET(edbc, DBC_RMW) &&
		    (ret = lock_get(edbp->edbenv->lk_info, edbc->locker,
		    DB_LOCK_UPGRADE, &edbc->lock_edbt, DB_LOCK_WRITE,
		    &edbc->mylock)) != 0)
			return (EAGAIN);
	}

	GET_META(edbp, hcp, ret);
	if (ret != 0)
		return (ret);

	SAVE_CURSOR(hcp, &save_curs);
	hcp->stats.hash_put++;

	switch (flags) {
	case DB_KEYLAST:
	case DB_KEYFIRST:
		nbytes = (ISBIG(hcp, key->size) ? HOFFPAGE_PSIZE :
		    HKEYDATA_PSIZE(key->size)) +
		    (ISBIG(hcp, data->size) ? HOFFPAGE_PSIZE :
		    HKEYDATA_PSIZE(data->size));
		if ((ret = __ham_lookup(edbc,
		    key, nbytes, DB_LOCK_WRITE)) == DB_NOTFOUND) {
			ret = 0;
			if (hcp->seek_found_page != PGNO_INVALID &&
			    hcp->seek_found_page != hcp->pgno) {
				if ((ret = __ham_item_done(edbc, 0)) != 0)
					goto out;
				hcp->pgno = hcp->seek_found_page;
				hcp->bndx = NDX_INVALID;
			}

			if (F_ISSET(data, DB_DBT_PARTIAL) && data->doff != 0) {
				/*
				 * A partial put, but the key does not exist
				 * and we are not beginning the write at 0.
				 * We must create a data item padded up to doff
				 * and then write the new bytes represented by
				 * val.
				 */
				if ((ret = __ham_init_edbt(&tmp_val,
				    data->size + data->doff,
				    &edbc->rdata.data, &edbc->rdata.size)) == 0) {
					memset(tmp_val.data, 0, data->doff);
					memcpy((u_int8_t *)tmp_val.data +
					    data->doff, data->data, data->size);
					myval = &tmp_val;
				}
			} else
				myval = (DBT *)data;

			if (ret == 0)
				ret = __ham_add_el(edbc, key, myval, H_KEYDATA);
			goto done;
		}
		break;
	case DB_BEFORE:
	case DB_AFTER:
	case DB_CURRENT:
		ret = __ham_item(edbc, DB_LOCK_WRITE);
		break;
	}

	if (ret == 0) {
		if ((flags == DB_CURRENT && !F_ISSET(hcp, H_ISDUP)) ||
		    ((flags == DB_KEYFIRST || flags == DB_KEYLAST) &&
		    !F_ISSET(edbp, DB_AM_DUP)))
			ret = __ham_overwrite(edbc, data);
		else
			ret = __ham_add_dup(edbc, data, flags);
	}

done:	if (ret == 0 && F_ISSET(hcp, H_EXPAND)) {
		ret = __ham_expand_table(edbc);
		F_CLR(hcp, H_EXPAND);
	}

	if ((t_ret = __ham_item_done(edbc, ret == 0)) != 0 && ret == 0)
		ret = t_ret;

out:	RELEASE_META(edbp, hcp);
	RESTORE_CURSOR(edbp, hcp, &save_curs, ret);
	if (F_ISSET(edbp, DB_AM_CDB) && F_ISSET(edbc, DBC_RMW))
		(void)__lock_downgrade(edbp->edbenv->lk_info, edbc->mylock,
		    DB_LOCK_IWRITE, 0);
	return (ret);
}

/********************************* UTILITIES ************************/

/*
 * __ham_expand_table --
 */
static int
__ham_expand_table(edbc)
	DBC *edbc;
{
	DB *edbp;
	HASH_CURSOR *hcp;
	DB_LSN new_lsn;
	u_int32_t old_bucket, new_bucket, spare_ndx;
	int ret;

	edbp = edbc->edbp;
	hcp = (HASH_CURSOR *)edbc->internal;
	ret = 0;
	DIRTY_META(edbp, hcp, ret);
	if (ret)
		return (ret);

	/*
	 * If the split point is about to increase, make sure that we
	 * have enough extra pages.  The calculation here is weird.
	 * We'd like to do this after we've upped max_bucket, but it's
	 * too late then because we've logged the meta-data split.  What
	 * we'll do between then and now is increment max bucket and then
	 * see what the log of one greater than that is; here we have to
	 * look at the log of max + 2.  VERY NASTY STUFF.
	 */
	if (__edb_log2(hcp->hdr->max_bucket + 2) > hcp->hdr->ovfl_point) {
		/*
		 * We are about to shift the split point.  Make sure that
		 * if the next doubling is going to be big (more than 8
		 * pages), we have some extra pages around.
		 */
		if (hcp->hdr->max_bucket + 1 >= 8 &&
		    hcp->hdr->spares[hcp->hdr->ovfl_point] <
		    hcp->hdr->spares[hcp->hdr->ovfl_point - 1] +
		    hcp->hdr->ovfl_point + 1)
			__ham_init_ovflpages(edbc);
	}

	/* Now we can log the meta-data split. */
	if (DB_LOGGING(edbc)) {
		if ((ret = __ham_splitmeta_log(edbp->edbenv->lg_info,
		    edbc->txn, &new_lsn, 0, edbp->log_fileid,
		    hcp->hdr->max_bucket, hcp->hdr->ovfl_point,
		    hcp->hdr->spares[hcp->hdr->ovfl_point],
		    &hcp->hdr->lsn)) != 0)
			return (ret);

		hcp->hdr->lsn = new_lsn;
	}

	hcp->stats.hash_expansions++;
	new_bucket = ++hcp->hdr->max_bucket;
	old_bucket = (hcp->hdr->max_bucket & hcp->hdr->low_mask);

	/*
	 * If the split point is increasing, copy the current contents
	 * of the spare split bucket to the next bucket.
	 */
	spare_ndx = __edb_log2(hcp->hdr->max_bucket + 1);
	if (spare_ndx > hcp->hdr->ovfl_point) {
		hcp->hdr->spares[spare_ndx] =
		    hcp->hdr->spares[hcp->hdr->ovfl_point];
		hcp->hdr->ovfl_point = spare_ndx;
	}

	if (new_bucket > hcp->hdr->high_mask) {
		/* Starting a new doubling */
		hcp->hdr->low_mask = hcp->hdr->high_mask;
		hcp->hdr->high_mask = new_bucket | hcp->hdr->low_mask;
	}

	if (BUCKET_TO_PAGE(hcp, new_bucket) > MAX_PAGES(hcp)) {
		__edb_err(edbp->edbenv,
		    "hash: Cannot allocate new bucket.  Pages exhausted.");
		return (ENOSPC);
	}

	/* Relocate records to the new bucket */
	return (__ham_split_page(edbc, old_bucket, new_bucket));
}

/*
 * PUBLIC: u_int32_t __ham_call_hash __P((HASH_CURSOR *, u_int8_t *, int32_t));
 */
u_int32_t
__ham_call_hash(hcp, k, len)
	HASH_CURSOR *hcp;
	u_int8_t *k;
	int32_t len;
{
	u_int32_t n, bucket;

	n = (u_int32_t)(hcp->edbc->edbp->h_hash(k, len));

	bucket = n & hcp->hdr->high_mask;
	if (bucket > hcp->hdr->max_bucket)
		bucket = bucket & hcp->hdr->low_mask;
	return (bucket);
}

/*
 * Check for duplicates, and call __edb_ret appropriately.  Release
 * everything held by the cursor.
 */
static int
__ham_dup_return(edbc, val, flags)
	DBC *edbc;
	DBT *val;
	u_int32_t flags;
{
	DB *edbp;
	HASH_CURSOR *hcp;
	PAGE *pp;
	DBT *myval, tmp_val;
	edb_indx_t ndx;
	edb_pgno_t pgno;
	u_int32_t off, tlen;
	u_int8_t *hk, type;
	int cmp, ret;
	edb_indx_t len;

	/* Check for duplicate and return the first one. */
	edbp = edbc->edbp;
	hcp = (HASH_CURSOR *)edbc->internal;
	ndx = H_DATAINDEX(hcp->bndx);
	type = HPAGE_TYPE(hcp->pagep, ndx);
	pp = hcp->pagep;
	myval = val;

	/*
	 * There are 4 cases:
	 * 1. We are not in duplicate, simply call edb_ret.
	 * 2. We are looking at keys and stumbled onto a duplicate.
	 * 3. We are in the middle of a duplicate set. (ISDUP set)
	 * 4. This is a duplicate and we need to return a specific item.
	 */

	/*
	 * Here we check for the case where we just stumbled onto a
	 * duplicate.  In this case, we do initialization and then
	 * let the normal duplicate code handle it.
	 */
	if (!F_ISSET(hcp, H_ISDUP))
		if (type == H_DUPLICATE) {
			F_SET(hcp, H_ISDUP);
			hcp->dup_tlen = LEN_HDATA(hcp->pagep,
			    hcp->hdr->pagesize, hcp->bndx);
			hk = H_PAIRDATA(hcp->pagep, hcp->bndx);
			if (flags == DB_LAST || flags == DB_PREV) {
				hcp->dndx = 0;
				hcp->dup_off = 0;
				do {
					memcpy(&len,
					    HKEYDATA_DATA(hk) + hcp->dup_off,
					    sizeof(edb_indx_t));
					hcp->dup_off += DUP_SIZE(len);
					hcp->dndx++;
				} while (hcp->dup_off < hcp->dup_tlen);
				hcp->dup_off -= DUP_SIZE(len);
				hcp->dndx--;
			} else {
				memcpy(&len,
				    HKEYDATA_DATA(hk), sizeof(edb_indx_t));
				hcp->dup_off = 0;
				hcp->dndx = 0;
			}
			hcp->dup_len = len;
		} else if (type == H_OFFDUP) {
			F_SET(hcp, H_ISDUP);
			memcpy(&pgno, HOFFDUP_PGNO(P_ENTRY(hcp->pagep, ndx)),
			    sizeof(edb_pgno_t));
			if (flags == DB_LAST || flags == DB_PREV) {
				if ((ret = __edb_dend(edbc,
				    pgno, &hcp->dpagep)) != 0)
					return (ret);
				hcp->dpgno = PGNO(hcp->dpagep);
				hcp->dndx = NUM_ENT(hcp->dpagep) - 1;
			} else if ((ret = __ham_next_cpage(edbc,
			    pgno, 0, H_ISDUP)) != 0)
				return (ret);
		}


	/*
	 * If we are retrieving a specific key/data pair, then we
	 * may need to adjust the cursor before returning data.
	 */
	if (flags == DB_GET_BOTH) {
		if (F_ISSET(hcp, H_ISDUP)) {
			if (hcp->dpgno != PGNO_INVALID) {
				if ((ret = __edb_dsearch(edbc, 0, val,
				    hcp->dpgno, &hcp->dndx, &hcp->dpagep, &cmp))
				    != 0)
					return (ret);
				if (cmp == 0)
					hcp->dpgno = PGNO(hcp->dpagep);
			} else {
				__ham_dsearch(edbc, val, &off, &cmp);
				hcp->dup_off = off;
			}
		} else {
			hk = H_PAIRDATA(hcp->pagep, hcp->bndx);
			if (((HKEYDATA *)hk)->type == H_OFFPAGE) {
				memcpy(&tlen,
				    HOFFPAGE_TLEN(hk), sizeof(u_int32_t));
				memcpy(&pgno,
				    HOFFPAGE_PGNO(hk), sizeof(edb_pgno_t));
				if ((ret = __edb_moff(edbp, val,
				    pgno, tlen, edbp->dup_compare, &cmp)) != 0)
					return (ret);
			} else {
				/*
				 * We do not zero tmp_val since the comparison
				 * routines may only look at data and size.
				 */
				tmp_val.data = HKEYDATA_DATA(hk);
				tmp_val.size = LEN_HDATA(hcp->pagep,
				    edbp->pgsize, hcp->bndx);
				cmp = edbp->dup_compare == NULL ?
				    __bam_defcmp(&tmp_val, val) :
				    edbp->dup_compare(&tmp_val, val);
			}
		}

		if (cmp != 0)
			return (DB_NOTFOUND);
	}

	/*
	 * Now, everything is initialized, grab a duplicate if
	 * necessary.
	 */
	if (F_ISSET(hcp, H_ISDUP))
		if (hcp->dpgno != PGNO_INVALID) {
			pp = hcp->dpagep;
			ndx = hcp->dndx;
		} else {
			/*
			 * Copy the DBT in case we are retrieving into user
			 * memory and we need the parameters for it.  If the
			 * user requested a partial, then we need to adjust
			 * the user's parameters to get the partial of the
			 * duplicate which is itself a partial.
			 */
			memcpy(&tmp_val, val, sizeof(*val));
			if (F_ISSET(&tmp_val, DB_DBT_PARTIAL)) {
				/*
				 * Take the user's length unless it would go
				 * beyond the end of the duplicate.
				 */
				if (tmp_val.doff + hcp->dup_off > hcp->dup_len)
					tmp_val.dlen = 0;
				else if (tmp_val.dlen + tmp_val.doff >
				    hcp->dup_len)
					tmp_val.dlen =
					    hcp->dup_len - tmp_val.doff;
				
				/*
				 * Calculate the new offset.
				 */
				tmp_val.doff += hcp->dup_off;
			} else {
				F_SET(&tmp_val, DB_DBT_PARTIAL);
				tmp_val.dlen = hcp->dup_len;
				tmp_val.doff = hcp->dup_off + sizeof(edb_indx_t);
			}
			myval = &tmp_val;
		}


	/*
	 * Finally, if we had a duplicate, pp, ndx, and myval should be
	 * set appropriately.
	 */
	if ((ret = __edb_ret(edbp, pp, ndx, myval, &edbc->rdata.data,
	    &edbc->rdata.size)) != 0)
		return (ret);

	/*
	 * In case we sent a temporary off to edb_ret, set the real
	 * return values.
	 */
	val->data = myval->data;
	val->size = myval->size;

	return (0);
}

static int
__ham_overwrite(edbc, nval)
	DBC *edbc;
	DBT *nval;
{
	HASH_CURSOR *hcp;
	DBT *myval, tmp_val;
	u_int8_t *hk;

	hcp = (HASH_CURSOR *)edbc->internal;
	if (F_ISSET(edbc->edbp, DB_AM_DUP))
		return (__ham_add_dup(edbc, nval, DB_KEYLAST));
	else if (!F_ISSET(nval, DB_DBT_PARTIAL)) {
		/* Put/overwrite */
		memcpy(&tmp_val, nval, sizeof(*nval));
		F_SET(&tmp_val, DB_DBT_PARTIAL);
		tmp_val.doff = 0;
		hk = H_PAIRDATA(hcp->pagep, hcp->bndx);
		if (HPAGE_PTYPE(hk) == H_OFFPAGE)
			memcpy(&tmp_val.dlen,
			    HOFFPAGE_TLEN(hk), sizeof(u_int32_t));
		else
			tmp_val.dlen = LEN_HDATA(hcp->pagep,
			    hcp->hdr->pagesize,hcp->bndx);
		myval = &tmp_val;
	} else /* Regular partial put */
		myval = nval;

	return (__ham_replpair(edbc, myval, 0));
}

/*
 * Given a key and a cursor, sets the cursor to the page/ndx on which
 * the key resides.  If the key is found, the cursor H_OK flag is set
 * and the pagep, bndx, pgno (dpagep, dndx, dpgno) fields are set.
 * If the key is not found, the H_OK flag is not set.  If the sought
 * field is non-0, the pagep, bndx, pgno (dpagep, dndx, dpgno) fields
 * are set indicating where an add might take place.  If it is 0,
 * non of the cursor pointer field are valid.
 */
static int
__ham_lookup(edbc, key, sought, mode)
	DBC *edbc;
	const DBT *key;
	u_int32_t sought;
	edb_lockmode_t mode;
{
	DB *edbp;
	HASH_CURSOR *hcp;
	edb_pgno_t pgno;
	u_int32_t tlen;
	int match, ret, t_ret;
	u_int8_t *hk;

	edbp = edbc->edbp;
	hcp = (HASH_CURSOR *)edbc->internal;
	/*
	 * Set up cursor so that we're looking for space to add an item
	 * as we cycle through the pages looking for the key.
	 */
	if ((ret = __ham_item_reset(edbc)) != 0)
		return (ret);
	hcp->seek_size = sought;

	hcp->bucket = __ham_call_hash(hcp, (u_int8_t *)key->data, key->size);
	while (1) {
		if ((ret = __ham_item_next(edbc, mode)) != 0)
			return (ret);

		if (F_ISSET(hcp, H_NOMORE))
			break;

		hk = H_PAIRKEY(hcp->pagep, hcp->bndx);
		switch (HPAGE_PTYPE(hk)) {
		case H_OFFPAGE:
			memcpy(&tlen, HOFFPAGE_TLEN(hk), sizeof(u_int32_t));
			if (tlen == key->size) {
				memcpy(&pgno,
				    HOFFPAGE_PGNO(hk), sizeof(edb_pgno_t));
				if ((ret = __edb_moff(edbp,
				    key, pgno, tlen, NULL, &match)) != 0)
					return (ret);
				if (match == 0) {
					F_SET(hcp, H_OK);
					return (0);
				}
			}
			break;
		case H_KEYDATA:
			if (key->size == LEN_HKEY(hcp->pagep,
			    hcp->hdr->pagesize, hcp->bndx) &&
			    memcmp(key->data,
			    HKEYDATA_DATA(hk), key->size) == 0) {
				F_SET(hcp, H_OK);
				return (0);
			}
			break;
		case H_DUPLICATE:
		case H_OFFDUP:
			/*
			 * These are errors because keys are never
			 * duplicated, only data items are.
			 */
			return (__edb_pgfmt(edbp, PGNO(hcp->pagep)));
		}
		hcp->stats.hash_collisions++;
	}

	/*
	 * Item was not found, adjust cursor properly.
	 */

	if (sought != 0)
		return (ret);

	if ((t_ret = __ham_item_done(edbc, 0)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * Initialize a edbt using some possibly already allocated storage
 * for items.
 * PUBLIC: int __ham_init_edbt __P((DBT *, u_int32_t, void **, u_int32_t *));
 */
int
__ham_init_edbt(edbt, size, bufp, sizep)
	DBT *edbt;
	u_int32_t size;
	void **bufp;
	u_int32_t *sizep;
{
	int ret;

	memset(edbt, 0, sizeof(*edbt));
	if (*sizep < size) {
		if ((ret = __os_realloc(bufp, size)) != 0) {
			*sizep = 0;
			return (ret);
		}
		*sizep = size;
	}
	edbt->data = *bufp;
	edbt->size = size;
	return (0);
}

/*
 * Adjust the cursor after an insert or delete.  The cursor passed is
 * the one that was operated upon; we just need to check any of the
 * others.
 *
 * len indicates the length of the item added/deleted
 * add indicates if the item indicated by the cursor has just been
 * added (add == 1) or deleted (add == 0).
 * dup indicates if the addition occurred into a duplicate set.
 *
 * PUBLIC: void __ham_c_update
 * PUBLIC:    __P((HASH_CURSOR *, edb_pgno_t, u_int32_t, int, int));
 */
void
__ham_c_update(hcp, chg_pgno, len, add, is_dup)
	HASH_CURSOR *hcp;
	edb_pgno_t chg_pgno;
	u_int32_t len;
	int add, is_dup;
{
	DB *edbp;
	DBC *cp;
	HASH_CURSOR *lcp;
	int page_deleted;

	/*
	 * Regular adds are always at the end of a given page, so we never
	 * have to adjust anyone's cursor after a regular add.
	 */
	if (!is_dup && add)
		return;

	/*
	 * Determine if a page was deleted.    If this is a regular update
	 * (i.e., not is_dup) then the deleted page's number will be that in
	 * chg_pgno, and the pgno in the cursor will be different.  If this
	 * was an onpage-duplicate, then the same conditions apply.  If this
	 * was an off-page duplicate, then we need to verify if hcp->dpgno
	 * is the same (no delete) or different (delete) than chg_pgno.
	 */
	if (!is_dup || hcp->dpgno == PGNO_INVALID)
		page_deleted =
		    chg_pgno != PGNO_INVALID && chg_pgno != hcp->pgno;
	else
		page_deleted =
		    chg_pgno != PGNO_INVALID && chg_pgno != hcp->dpgno;

	edbp = hcp->edbc->edbp;
	DB_THREAD_LOCK(edbp);

	for (cp = TAILQ_FIRST(&edbp->active_queue); cp != NULL;
	    cp = TAILQ_NEXT(cp, links)) {
		if (cp->internal == hcp)
			continue;

		lcp = (HASH_CURSOR *)cp->internal;

		if (!is_dup && lcp->pgno != chg_pgno)
			continue;

		if (is_dup) {
			if (F_ISSET(hcp, H_DELETED) && lcp->pgno != chg_pgno)
				continue;
			if (!F_ISSET(hcp, H_DELETED) && lcp->dpgno != chg_pgno)
				continue;
		}

		if (page_deleted) {
			if (is_dup) {
				lcp->dpgno = hcp->dpgno;
				lcp->dndx = hcp->dndx;
			} else {
				lcp->pgno = hcp->pgno;
				lcp->bndx = hcp->bndx;
				lcp->bucket = hcp->bucket;
			}
			F_CLR(lcp, H_ISDUP);
			continue;
		}

		if (!is_dup && lcp->bndx > hcp->bndx)
			lcp->bndx--;
		else if (!is_dup && lcp->bndx == hcp->bndx)
			F_SET(lcp, H_DELETED);
		else if (is_dup && lcp->bndx == hcp->bndx) {
			/* Assign dpgno in case there was page conversion. */
			lcp->dpgno = hcp->dpgno;
			if (add && lcp->dndx >= hcp->dndx )
				lcp->dndx++;
			else if (!add && lcp->dndx > hcp->dndx)
				lcp->dndx--;
			else if (!add && lcp->dndx == hcp->dndx)
				F_SET(lcp, H_DELETED);

			/* Now adjust on-page information. */
			if (lcp->dpgno == PGNO_INVALID)
				if (add) {
					lcp->dup_tlen += len;
					if (lcp->dndx > hcp->dndx)
						lcp->dup_off += len;
				} else {
					lcp->dup_tlen -= len;
					if (lcp->dndx > hcp->dndx)
						lcp->dup_off -= len;
				}
		}
	}
	DB_THREAD_UNLOCK(edbp);
}

