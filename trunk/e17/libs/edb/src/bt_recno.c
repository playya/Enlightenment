/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)bt_recno.c	10.53 (Sleepycat) 12/11/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#endif

#include "edb_int.h"
#include "edb_page.h"
#include "btree.h"
#include "edb_ext.h"
#include "shqueue.h"
#include "edb_shash.h"
#include "lock.h"
#include "lock_ext.h"

static int __ram_add __P((DBC *, edb_recno_t *, DBT *, u_int32_t, u_int32_t));
static int __ram_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
static int __ram_fmap __P((DBC *, edb_recno_t));
static int __ram_i_delete __P((DBC *));
static int __ram_put __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
static int __ram_source __P((DB *, RECNO *, const char *));
static int __ram_sync __P((DB *, u_int32_t));
static int __ram_update __P((DBC *, edb_recno_t, int));
static int __ram_vmap __P((DBC *, edb_recno_t));
static int __ram_writeback __P((DBC *));

/*
 * In recno, there are two meanings to the on-page "deleted" flag.  If we're
 * re-numbering records, it means the record was implicitly created.  We skip
 * over implicitly created records if doing a cursor "next" or "prev", and
 * return DB_KEYEMPTY if they're explicitly requested..  If not re-numbering
 * records, it means that the record was implicitly created, or was deleted.
 * We skip over implicitly created or deleted records if doing a cursor "next"
 * or "prev", and return DB_KEYEMPTY if they're explicitly requested.
 *
 * If we're re-numbering records, then we have to detect in the cursor that
 * a record was deleted, and adjust the cursor as necessary on the next get.
 * If we're not re-numbering records, then we can detect that a record has
 * been deleted by looking at the actual on-page record, so we completely
 * ignore the cursor's delete flag.  This is different from the B+tree code.
 * It also maintains whether the cursor references a deleted record in the
 * cursor, and it doesn't always check the on-page value.
 */
#define	CD_SET(edbp, cp) {						\
	if (F_ISSET(edbp, DB_RE_RENUMBER))				\
		F_SET(cp, C_DELETED);					\
}
#define	CD_CLR(edbp, cp) {						\
	if (F_ISSET(edbp, DB_RE_RENUMBER))				\
		F_CLR(cp, C_DELETED);					\
}
#define	CD_ISSET(edbp, cp)						\
	(F_ISSET(edbp, DB_RE_RENUMBER) && F_ISSET(cp, C_DELETED))

/*
 * __ram_open --
 *	Recno open function.
 *
 * PUBLIC: int __ram_open __P((DB *, DB_INFO *));
 */
int
__ram_open(edbp, edbinfo)
	DB *edbp;
	DB_INFO *edbinfo;
{
	BTREE *t;
	DBC *edbc;
	RECNO *rp;
	int ret, t_ret;

	/* Allocate and initialize the private btree structure. */
	if ((ret = __edb_os_calloc(1, sizeof(BTREE), &t)) != 0)
		return (ret);
	edbp->internal = t;
	__bam_setovflsize(edbp);

	/* Allocate and initialize the private recno structure. */
	if ((ret = __edb_os_calloc(1, sizeof(*rp), &rp)) != 0)
		return (ret);
	/* Link in the private recno structure. */
	t->recno = rp;

	/*
	 * Intention is to make sure all of the user's selections are okay
	 * here and then use them without checking.
	 */
	if (edbinfo == NULL) {
		rp->re_delim = '\n';
		rp->re_pad = ' ';
		rp->re_fd = -1;
		F_SET(rp, RECNO_EOF);
	} else {
		/*
		 * If the user specified a source tree, open it and map it in.
		 *
		 * !!!
		 * We don't complain if the user specified transactions or
		 * threads.  It's possible to make it work, but you'd better
		 * know what you're doing!
		 */
		if (edbinfo->re_source == NULL) {
			rp->re_fd = -1;
			F_SET(rp, RECNO_EOF);
		} else {
			if ((ret =
			    __ram_source(edbp, rp, edbinfo->re_source)) != 0)
			goto err;
		}

		/* Copy delimiter, length and padding values. */
		rp->re_delim =
		    F_ISSET(edbp, DB_RE_DELIMITER) ? edbinfo->re_delim : '\n';
		rp->re_pad = F_ISSET(edbp, DB_RE_PAD) ? edbinfo->re_pad : ' ';

		if (F_ISSET(edbp, DB_RE_FIXEDLEN)) {
			if ((rp->re_len = edbinfo->re_len) == 0) {
				__edb_err(edbp->edbenv,
				    "record length must be greater than 0");
				ret = EINVAL;
				goto err;
			}
		} else
			rp->re_len = 0;
	}

	/* Initialize the remaining fields/methods of the DB. */
	edbp->am_close = __ram_close;
	edbp->del = __ram_delete;
	edbp->put = __ram_put;
	edbp->stat = __bam_stat;
	edbp->sync = __ram_sync;

	/* Start up the tree. */
	if ((ret = __bam_read_root(edbp)) != 0)
		goto err;

	/* Set the overflow page size. */
	__bam_setovflsize(edbp);

	/* If we're snapshotting an underlying source file, do it now. */
	if (edbinfo != NULL && F_ISSET(edbinfo, DB_SNAPSHOT)) {
		/* Allocate a cursor. */
		if ((ret = edbp->cursor(edbp, NULL, &edbc, 0)) != 0)
			goto err;

		/* Do the snapshot. */
		if ((ret = __ram_update(edbc,
		    DB_MAX_RECORDS, 0)) != 0 && ret == DB_NOTFOUND)
			ret = 0;

		/* Discard the cursor. */
		if ((t_ret = edbc->c_close(edbc)) != 0 && ret == 0)
			ret = t_ret;

		if (ret != 0)
			goto err;
	}

	return (0);

err:	/* If we mmap'd a source file, discard it. */
	if (rp->re_smap != NULL)
		(void)__edb_unmapfile(rp->re_smap, rp->re_msize);

	/* If we opened a source file, discard it. */
	if (rp->re_fd != -1)
		(void)__edb_os_close(rp->re_fd);
	if (rp->re_source != NULL)
		__edb_os_freestr(rp->re_source);

	__edb_os_free(rp, sizeof(*rp));

	return (ret);
}

/*
 * __ram_delete --
 *	Recno edb->del function.
 */
static int
__ram_delete(edbp, txn, key, flags)
	DB *edbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	CURSOR *cp;
	DBC *edbc;
	edb_recno_t recno;
	int ret, t_ret;

	DB_PANIC_CHECK(edbp);

	/* Check for invalid flags. */
	if ((ret = __edb_delchk(edbp,
	    key, flags, F_ISSET(edbp, DB_AM_RDONLY))) != 0)
		return (ret);

	/* Acquire a cursor. */
	if ((ret = edbp->cursor(edbp, txn, &edbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, txn, "ram_delete", key, NULL, flags);

	/* Check the user's record number and fill in as necessary. */
	if ((ret = __ram_getno(edbc, key, &recno, 0)) != 0)
		goto err;

	/* Do the delete. */
	cp = edbc->internal;
	cp->recno = recno;
	ret = __ram_i_delete(edbc);

	/* Release the cursor. */
err:	if ((t_ret = edbc->c_close(edbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * __ram_i_delete --
 *	Internal version of recno delete, called by __ram_delete and
 *	__ram_c_del.
 */
static int
__ram_i_delete(edbc)
	DBC *edbc;
{
	BKEYDATA bk;
	BTREE *t;
	CURSOR *cp;
	DB *edbp;
	DBT hdr, data;
	PAGE *h;
	edb_indx_t indx;
	int exact, ret, stack;

	edbp = edbc->edbp;
	cp = edbc->internal;
	t = edbp->internal;
	stack = 0;

	/*
	 * If this is CDB and this isn't a write cursor, then it's an error.
	 * If it is a write cursor, but we don't yet hold the write lock, then
	 * we need to upgrade to the write lock.
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

	/* Search the tree for the key; delete only deletes exact matches. */
	if ((ret = __bam_rsearch(edbc, &cp->recno, S_DELETE, 1, &exact)) != 0)
		goto err;
	if (!exact) {
		ret = DB_NOTFOUND;
		goto err;
	}
	stack = 1;

	h = cp->csp->page;
	indx = cp->csp->indx;

	/*
	 * If re-numbering records, the on-page deleted flag can only mean
	 * that this record was implicitly created.  Applications aren't
	 * permitted to delete records they never created, return an error.
	 *
	 * If not re-numbering records, the on-page deleted flag means that
	 * this record was implicitly created, or, was deleted at some time.
	 * The former is an error because applications aren't permitted to
	 * delete records they never created, the latter is an error because
	 * if the record was "deleted", we could never have found it.
	 */
	if (B_DISSET(GET_BKEYDATA(h, indx)->type)) {
		ret = DB_KEYEMPTY;
		goto err;
	}

	if (F_ISSET(edbp, DB_RE_RENUMBER)) {
		/* Delete the item, adjust the counts, adjust the cursors. */
		if ((ret = __bam_ditem(edbc, h, indx)) != 0)
			goto err;
		__bam_adjust(edbc, -1);
		__ram_ca(edbp, cp->recno, CA_DELETE);

		/*
		 * If the page is empty, delete it.   The whole tree is locked
		 * so there are no preparations to make.
		 */
		if (NUM_ENT(h) == 0 && h->pgno != PGNO_ROOT) {
			stack = 0;
			ret = __bam_dpages(edbc);
		}
	} else {
		/* Use a delete/put pair to replace the record with a marker. */
		if ((ret = __bam_ditem(edbc, h, indx)) != 0)
			goto err;

		B_TSET(bk.type, B_KEYDATA, 1);
		bk.len = 0;
		memset(&hdr, 0, sizeof(hdr));
		hdr.data = &bk;
		hdr.size = SSZA(BKEYDATA, data);
		memset(&data, 0, sizeof(data));
		data.data = (char *)"";
		data.size = 0;
		if ((ret = __edb_pitem(edbc,
		    h, indx, BKEYDATA_SIZE(0), &hdr, &data)) != 0)
			goto err;
	}
	F_SET(t->recno, RECNO_MODIFIED);

err:	if (stack)
		__bam_stkrel(edbc, 0);

	/* If we upgraded the CDB lock upon entry; downgrade it now. */
	if (F_ISSET(edbp, DB_AM_CDB) && F_ISSET(edbc, DBC_RMW))
		(void)__lock_downgrade(edbp->edbenv->lk_info, edbc->mylock,
		    DB_LOCK_IWRITE, 0);
	return (ret);
}

/*
 * __ram_put --
 *	Recno edb->put function.
 */
static int
__ram_put(edbp, txn, key, data, flags)
	DB *edbp;
	DB_TXN *txn;
	DBT *key, *data;
	u_int32_t flags;
{
	DBC *edbc;
	edb_recno_t recno;
	int ret, t_ret;

	DB_PANIC_CHECK(edbp);

	/* Check for invalid flags. */
	if ((ret = __edb_putchk(edbp,
	    key, data, flags, F_ISSET(edbp, DB_AM_RDONLY), 0)) != 0)
		return (ret);

	/* Allocate a cursor. */
	if ((ret = edbp->cursor(edbp, txn, &edbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, txn, "ram_put", key, data, flags);

	/*
	 * If we're appending to the tree, make sure we've read in all of
	 * the backing source file.  Otherwise, check the user's record
	 * number and fill in as necessary.
	 */
	ret = flags == DB_APPEND ?
	    __ram_update(edbc, DB_MAX_RECORDS, 0) :
	    __ram_getno(edbc, key, &recno, 1);

	/* Add the record. */
	if (ret == 0)
		ret = __ram_add(edbc, &recno, data, flags, 0);

	/* Discard the cursor. */
	if ((t_ret = edbc->c_close(edbc)) != 0 && ret == 0)
		ret = t_ret;

	/* Return the record number if we're appending to the tree. */
	if (ret == 0 && flags == DB_APPEND)
		*(edb_recno_t *)key->data = recno;

	return (ret);
}

/*
 * __ram_sync --
 *	Recno edb->sync function.
 */
static int
__ram_sync(edbp, flags)
	DB *edbp;
	u_int32_t flags;
{
	DBC *edbc;
	int ret, t_ret;

	/*
	 * Sync the underlying btree.
	 *
	 * !!!
	 * We don't need to do a panic check or flags check, the "real"
	 * sync function does all that for us.
	 */
	if ((ret = __edb_sync(edbp, flags)) != 0)
		return (ret);

	/* Allocate a cursor. */
	if ((ret = edbp->cursor(edbp, NULL, &edbc, 0)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, NULL, "ram_sync", NULL, NULL, flags);

	/* Copy back the backing source file. */
	ret = __ram_writeback(edbc);

	/* Discard the cursor. */
	if ((t_ret = edbc->c_close(edbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * __ram_close --
 *	Recno edb->close function.
 *
 * PUBLIC: int __ram_close __P((DB *));
 */
int
__ram_close(edbp)
	DB *edbp;
{
	RECNO *rp;

	rp = ((BTREE *)edbp->internal)->recno;

	/* Close any underlying mmap region. */
	if (rp->re_smap != NULL)
		(void)__edb_unmapfile(rp->re_smap, rp->re_msize);

	/* Close any backing source file descriptor. */
	if (rp->re_fd != -1)
		(void)__edb_os_close(rp->re_fd);

	/* Free any backing source file name. */
	if (rp->re_source != NULL)
		__edb_os_freestr(rp->re_source);

	/* Free allocated memory. */
	__edb_os_free(rp, sizeof(RECNO));
	((BTREE *)edbp->internal)->recno = NULL;

	/* Close the underlying btree. */
	return (__bam_close(edbp));
}

/*
 * __ram_c_del --
 *	Recno cursor->c_del function.
 *
 * PUBLIC: int __ram_c_del __P((DBC *, u_int32_t));
 */
int
__ram_c_del(edbc, flags)
	DBC *edbc;
	u_int32_t flags;
{
	CURSOR *cp;
	DB *edbp;
	int ret;

	edbp = edbc->edbp;
	cp = edbc->internal;

	DB_PANIC_CHECK(edbp);

	/* Check for invalid flags. */
	if ((ret = __edb_cdelchk(edbp, flags,
	    F_ISSET(edbp, DB_AM_RDONLY), cp->recno != RECNO_OOB)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, edbc->txn, "ram_c_del", NULL, NULL, flags);

	/*
	 * If we are running CDB, this had better be either a write
	 * cursor or an immediate writer.
	 */
	if (F_ISSET(edbp, DB_AM_CDB))
		if (!F_ISSET(edbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

	/*
	 * The semantics of cursors during delete are as follows: if record
	 * numbers are mutable (DB_RE_RENUMBER is set), deleting a record
	 * causes the cursor to automatically point to the record immediately
	 * following.  In this case it is possible to use a single cursor for
	 * repeated delete operations, without intervening operations.
	 *
	 * If record numbers are not mutable, then records are replaced with
	 * a marker containing a delete flag.  If the record referenced by
	 * this cursor has already been deleted, we will detect that as part
	 * of the delete operation, and fail.
	 */
	return (__ram_i_delete(edbc));
}

/*
 * __ram_c_get --
 *	Recno cursor->c_get function.
 *
 * PUBLIC: int __ram_c_get __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__ram_c_get(edbc, key, data, flags)
	DBC *edbc;
	DBT *key, *data;
	u_int32_t flags;
{
	CURSOR *cp, copy;
	DB *edbp;
	PAGE *h;
	edb_indx_t indx;
	int exact, ret, stack, tmp_rmw;

	edbp = edbc->edbp;
	cp = edbc->internal;

	DB_PANIC_CHECK(edbp);

	/* Check for invalid flags. */
	if ((ret = __edb_cgetchk(edbc->edbp,
	    key, data, flags, cp->recno != RECNO_OOB)) != 0)
		return (ret);

	/* Clear OR'd in additional bits so we can check for flag equality. */
	tmp_rmw = 0;
	if (LF_ISSET(DB_RMW)) {
		if (!F_ISSET(edbp, DB_AM_CDB)) {
			tmp_rmw = 1;
			F_SET(edbc, DBC_RMW);
		}
		LF_CLR(DB_RMW);
	}

	DEBUG_LREAD(edbc, edbc->txn, "ram_c_get",
	    flags == DB_SET || flags == DB_SET_RANGE ? key : NULL, NULL, flags);

	/* Initialize the cursor for a new retrieval. */
	copy = *cp;

retry:	/* Update the record number. */
	stack = 0;
	switch (flags) {
	case DB_CURRENT:
		/*
		 * If record numbers are mutable: if we just deleted a record,
		 * there is no action necessary, we return the record following
		 * the deleted item by virtue of renumbering the tree.
		 */
		break;
	case DB_NEXT:
		/*
		 * If record numbers are mutable: if we just deleted a record,
		 * we have to avoid incrementing the record number so that we
		 * return the right record by virtue of renumbering the tree.
		 */
		if (CD_ISSET(edbp, cp))
			break;

		if (cp->recno != RECNO_OOB) {
			++cp->recno;
			break;
		}
		/* FALLTHROUGH */
	case DB_FIRST:
		flags = DB_NEXT;
		cp->recno = 1;
		break;
	case DB_PREV:
		if (cp->recno != RECNO_OOB) {
			if (cp->recno == 1) {
				ret = DB_NOTFOUND;
				goto err;
			}
			--cp->recno;
			break;
		}
		/* FALLTHROUGH */
	case DB_LAST:
		flags = DB_PREV;
		if (((ret = __ram_update(edbc,
		    DB_MAX_RECORDS, 0)) != 0) && ret != DB_NOTFOUND)
			goto err;
		if ((ret = __bam_nrecs(edbc, &cp->recno)) != 0)
			goto err;
		if (cp->recno == 0) {
			ret = DB_NOTFOUND;
			goto err;
		}
		break;
	case DB_SET:
	case DB_SET_RANGE:
		if ((ret = __ram_getno(edbc, key, &cp->recno, 0)) != 0)
			goto err;
		break;
	}

	/* Return the key if the user didn't give us one. */
	if (flags != DB_SET && flags != DB_SET_RANGE &&
	    (ret = __edb_retcopy(key, &cp->recno, sizeof(cp->recno),
	    &edbc->rkey.data, &edbc->rkey.ulen, edbp->edb_malloc)) != 0)
		goto err;

	/* Search the tree for the record. */
	if ((ret = __bam_rsearch(edbc, &cp->recno,
	    F_ISSET(edbc, DBC_RMW) ? S_FIND_WR : S_FIND, 1, &exact)) != 0)
		goto err;
	stack = 1;
	if (!exact) {
		ret = DB_NOTFOUND;
		goto err;
	}
	h = cp->csp->page;
	indx = cp->csp->indx;

	/*
	 * If re-numbering records, the on-page deleted flag means this record
	 * was implicitly created.  If not re-numbering records, the on-page
	 * deleted flag means this record was implicitly created, or, it was
	 * deleted at some time.  Regardless, we skip such records if doing
	 * cursor next/prev operations, and fail if the application requested
	 * them explicitly.
	 */
	if (B_DISSET(GET_BKEYDATA(h, indx)->type)) {
		if (flags == DB_NEXT || flags == DB_PREV) {
			(void)__bam_stkrel(edbc, 0);
			goto retry;
		}
		ret = DB_KEYEMPTY;
		goto err;
	}

	/* Return the data item. */
	if ((ret = __edb_ret(edbp,
	    h, indx, data, &edbc->rdata.data, &edbc->rdata.ulen)) != 0)
		goto err;

	/* The cursor was reset, no further delete adjustment is necessary. */
	CD_CLR(edbp, cp);

err:	if (stack)
		(void)__bam_stkrel(edbc, 0);

	/* Release temporary lock upgrade. */
	if (tmp_rmw)
		F_CLR(edbc, DBC_RMW);

	if (ret != 0)
		*cp = copy;

	return (ret);
}

/*
 * __ram_c_put --
 *	Recno cursor->c_put function.
 *
 * PUBLIC: int __ram_c_put __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__ram_c_put(edbc, key, data, flags)
	DBC *edbc;
	DBT *key, *data;
	u_int32_t flags;
{
	CURSOR *cp, copy;
	DB *edbp;
	int exact, ret;
	void *arg;

	edbp = edbc->edbp;
	cp = edbc->internal;

	DB_PANIC_CHECK(edbp);

	if ((ret = __edb_cputchk(edbc->edbp, key, data, flags,
	    F_ISSET(edbc->edbp, DB_AM_RDONLY), cp->recno != RECNO_OOB)) != 0)
		return (ret);

	DEBUG_LWRITE(edbc, edbc->txn, "ram_c_put", NULL, data, flags);

	/*
	 * If we are running CDB, this had better be either a write
	 * cursor or an immediate writer.  If it's a regular writer,
	 * that means we have an IWRITE lock and we need to upgrade
	 * it to a write lock.
	 */
	if (F_ISSET(edbp, DB_AM_CDB)) {
		if (!F_ISSET(edbc, DBC_RMW | DBC_WRITER))
			return (EINVAL);

		if (F_ISSET(edbc, DBC_RMW) &&
		    (ret = lock_get(edbp->edbenv->lk_info, edbc->locker,
		    DB_LOCK_UPGRADE, &edbc->lock_edbt, DB_LOCK_WRITE,
		    &edbc->mylock)) != 0)
			return (EAGAIN);
	}

	/* Initialize the cursor for a new retrieval. */
	copy = *cp;

	/*
	 * To split, we need a valid key for the page.  Since it's a cursor,
	 * we have to build one.
	 *
	 * The split code discards all short-term locks and stack pages.
	 */
	if (0) {
split:		arg = &cp->recno;
		if ((ret = __bam_split(edbc, arg)) != 0)
			goto err;
	}

	if ((ret = __bam_rsearch(edbc, &cp->recno, S_INSERT, 1, &exact)) != 0)
		goto err;
	if (!exact) {
		ret = DB_NOTFOUND;
		goto err;
	}
	if ((ret = __bam_iitem(edbc, &cp->csp->page,
	    &cp->csp->indx, key, data, flags, 0)) == DB_NEEDSPLIT) {
		if ((ret = __bam_stkrel(edbc, 0)) != 0)
			goto err;
		goto split;
	}
	if ((ret = __bam_stkrel(edbc, 0)) != 0)
		goto err;

	switch (flags) {
	case DB_AFTER:
		/* Adjust the cursors. */
		__ram_ca(edbp, cp->recno, CA_IAFTER);

		/* Set this cursor to reference the new record. */
		cp->recno = copy.recno + 1;
		break;
	case DB_BEFORE:
		/* Adjust the cursors. */
		__ram_ca(edbp, cp->recno, CA_IBEFORE);

		/* Set this cursor to reference the new record. */
		cp->recno = copy.recno;
		break;
	}

	/* The cursor was reset, no further delete adjustment is necessary. */
	CD_CLR(edbp, cp);

err:	if (F_ISSET(edbp, DB_AM_CDB) && F_ISSET(edbc, DBC_RMW))
		(void)__lock_downgrade(edbp->edbenv->lk_info, edbc->mylock,
		    DB_LOCK_IWRITE, 0);

	if (ret != 0)
		*cp = copy;

	return (ret);
}

/*
 * __ram_ca --
 *	Adjust cursors.
 *
 * PUBLIC: void __ram_ca __P((DB *, edb_recno_t, ca_recno_arg));
 */
void
__ram_ca(edbp, recno, op)
	DB *edbp;
	edb_recno_t recno;
	ca_recno_arg op;
{
	CURSOR *cp;
	DBC *edbc;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 */
	DB_THREAD_LOCK(edbp);
	for (edbc = TAILQ_FIRST(&edbp->active_queue);
	    edbc != NULL; edbc = TAILQ_NEXT(edbc, links)) {
		cp = edbc->internal;
		switch (op) {
		case CA_DELETE:
			if (recno > cp->recno)
				--cp->recno;
			if (recno == cp->recno)
				CD_SET(edbp, cp);
			break;
		case CA_IAFTER:
			if (recno > cp->recno)
				++cp->recno;
			break;
		case CA_IBEFORE:
			if (recno >= cp->recno)
				++cp->recno;
			break;
		}
	}
	DB_THREAD_UNLOCK(edbp);
}

/*
 * __ram_getno --
 *	Check the user's record number, and make sure we've seen it.
 *
 * PUBLIC: int __ram_getno __P((DBC *, const DBT *, edb_recno_t *, int));
 */
int
__ram_getno(edbc, key, rep, can_create)
	DBC *edbc;
	const DBT *key;
	edb_recno_t *rep;
	int can_create;
{
	DB *edbp;
	edb_recno_t recno;

	edbp = edbc->edbp;

	/* Check the user's record number. */
	if ((recno = *(edb_recno_t *)key->data) == 0) {
		__edb_err(edbp->edbenv, "illegal record number of 0");
		return (EINVAL);
	}
	if (rep != NULL)
		*rep = recno;

	/*
	 * Btree can neither create records nor read them in.  Recno can
	 * do both, see if we can find the record.
	 */
	return (edbp->type == DB_RECNO ?
	    __ram_update(edbc, recno, can_create) : 0);
}

/*
 * __ram_update --
 *	Ensure the tree has records up to and including the specified one.
 */
static int
__ram_update(edbc, recno, can_create)
	DBC *edbc;
	edb_recno_t recno;
	int can_create;
{
	BTREE *t;
	DB *edbp;
	RECNO *rp;
	edb_recno_t nrecs;
	int ret;

	edbp = edbc->edbp;
	t = edbp->internal;
	rp = t->recno;

	/*
	 * If we can't create records and we've read the entire backing input
	 * file, we're done.
	 */
	if (!can_create && F_ISSET(rp, RECNO_EOF))
		return (0);

	/*
	 * If we haven't seen this record yet, try to get it from the original
	 * file.
	 */
	if ((ret = __bam_nrecs(edbc, &nrecs)) != 0)
		return (ret);
	if (!F_ISSET(rp, RECNO_EOF) && recno > nrecs) {
		if ((ret = rp->re_irec(edbc, recno)) != 0)
			return (ret);
		if ((ret = __bam_nrecs(edbc, &nrecs)) != 0)
			return (ret);
	}

	/*
	 * If we can create records, create empty ones up to the requested
	 * record.
	 */
	if (!can_create || recno <= nrecs + 1)
		return (0);

	edbc->rdata.dlen = 0;
	edbc->rdata.doff = 0;
	edbc->rdata.flags = 0;
	if (F_ISSET(edbp, DB_RE_FIXEDLEN)) {
		if (edbc->rdata.ulen < rp->re_len) {
			if ((ret =
			    __edb_os_realloc(&edbc->rdata.data, rp->re_len)) != 0) {
				edbc->rdata.ulen = 0;
				edbc->rdata.data = NULL;
				return (ret);
			}
			edbc->rdata.ulen = rp->re_len;
		}
		edbc->rdata.size = rp->re_len;
		memset(edbc->rdata.data, rp->re_pad, rp->re_len);
	} else
		edbc->rdata.size = 0;

	while (recno > ++nrecs)
		if ((ret = __ram_add(edbc,
		    &nrecs, &edbc->rdata, 0, BI_DELETED)) != 0)
			return (ret);
	return (0);
}

/*
 * __ram_source --
 *	Load information about the backing file.
 */
static int
__ram_source(edbp, rp, fname)
	DB *edbp;
	RECNO *rp;
	const char *fname;
{
	size_t size;
	u_int32_t bytes, mbytes, oflags;
	int ret;

	/*
	 * !!!
	 * The caller has full responsibility for cleaning up on error --
	 * (it has to anyway, in case it fails after this routine succeeds).
	 */
	if ((ret = __edb_appname(edbp->edbenv,
	    DB_APP_DATA, NULL, fname, 0, NULL, &rp->re_source)) != 0)
		return (ret);

	oflags = F_ISSET(edbp, DB_AM_RDONLY) ? DB_RDONLY : 0;
	if ((ret =
	    __edb_open(rp->re_source, oflags, oflags, 0, &rp->re_fd)) != 0) {
		__edb_err(edbp->edbenv, "%s: %s", rp->re_source, strerror(ret));
		return (ret);
	}

	/*
	 * XXX
	 * We'd like to test to see if the file is too big to mmap.  Since we
	 * don't know what size or type off_t's or size_t's are, or the largest
	 * unsigned integral type is, or what random insanity the local C
	 * compiler will perpetrate, doing the comparison in a portable way is
	 * flatly impossible.  Hope that mmap fails if the file is too large.
	 */
	if ((ret = __edb_os_ioinfo(rp->re_source,
	    rp->re_fd, &mbytes, &bytes, NULL)) != 0) {
		__edb_err(edbp->edbenv, "%s: %s", rp->re_source, strerror(ret));
		return (ret);
	}
	if (mbytes == 0 && bytes == 0) {
		F_SET(rp, RECNO_EOF);
		return (0);
	}

	size = mbytes * MEGABYTE + bytes;
	if ((ret = __edb_mapfile(rp->re_source,
	    rp->re_fd, (size_t)size, 1, &rp->re_smap)) != 0)
		return (ret);
	rp->re_cmap = rp->re_smap;
	rp->re_emap = (u_int8_t *)rp->re_smap + (rp->re_msize = size);
	rp->re_irec = F_ISSET(edbp, DB_RE_FIXEDLEN) ?  __ram_fmap : __ram_vmap;
	return (0);
}

/*
 * __ram_writeback --
 *	Rewrite the backing file.
 */
static int
__ram_writeback(edbc)
	DBC *edbc;
{
	DB *edbp;
	DBT key, data;
	RECNO *rp;
	edb_recno_t keyno;
	ssize_t nw;
	int fd, ret, t_ret;
	u_int8_t delim, *pad;

	edbp = edbc->edbp;
	rp = ((BTREE *)edbp->internal)->recno;

	/* If the file wasn't modified, we're done. */
	if (!F_ISSET(rp, RECNO_MODIFIED))
		return (0);

	/* If there's no backing source file, we're done. */
	if (rp->re_source == NULL) {
		F_CLR(rp, RECNO_MODIFIED);
		return (0);
	}

	/*
	 * Read any remaining records into the tree.
	 *
	 * !!!
	 * This is why we can't support transactions when applications specify
	 * backing (re_source) files.  At this point we have to read in the
	 * rest of the records from the file so that we can write all of the
	 * records back out again, which could modify a page for which we'd
	 * have to log changes and which we don't have locked.  This could be
	 * partially fixed by taking a snapshot of the entire file during the
	 * edb_open(), or, since edb_open() isn't transaction protected, as part
	 * of the first DB operation.  But, if a checkpoint occurs then, the
	 * part of the log holding the copy of the file could be discarded, and
	 * that would make it impossible to recover in the face of disaster.
	 * This could all probably be fixed, but it would require transaction
	 * protecting the backing source file, i.e. mpool would have to know
	 * about it, and we don't want to go there.
	 */
	if ((ret =
	    __ram_update(edbc, DB_MAX_RECORDS, 0)) != 0 && ret != DB_NOTFOUND)
		return (ret);

	/*
	 * !!!
	 * Close any underlying mmap region.  This is required for Windows NT
	 * (4.0, Service Pack 2) -- if the file is still mapped, the following
	 * open will fail.
	 */
	if (rp->re_smap != NULL) {
		(void)__edb_unmapfile(rp->re_smap, rp->re_msize);
		rp->re_smap = NULL;
	}

	/* Get rid of any backing file descriptor, just on GP's. */
	if (rp->re_fd != -1) {
		(void)__edb_os_close(rp->re_fd);
		rp->re_fd = -1;
	}

	/* Open the file, truncating it. */
	if ((ret = __edb_open(rp->re_source,
	    DB_SEQUENTIAL | DB_TRUNCATE,
	    DB_SEQUENTIAL | DB_TRUNCATE, 0, &fd)) != 0) {
		__edb_err(edbp->edbenv, "%s: %s", rp->re_source, strerror(ret));
		return (ret);
	}

	/*
	 * We step through the records, writing each one out.  Use the record
	 * number and the edbp->get() function, instead of a cursor, so we find
	 * and write out "deleted" or non-existent records.
	 */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.size = sizeof(edb_recno_t);
	key.data = &keyno;

	/*
	 * We'll need the delimiter if we're doing variable-length records,
	 * and the pad character if we're doing fixed-length records.
	 */
	delim = rp->re_delim;
	if (F_ISSET(edbp, DB_RE_FIXEDLEN)) {
		if ((ret = __edb_os_malloc(rp->re_len, NULL, &pad)) != 0)
			goto err;
		memset(pad, rp->re_pad, rp->re_len);
	} else
		COMPQUIET(pad, NULL);
	for (keyno = 1;; ++keyno) {
		switch (ret = edbp->get(edbp, NULL, &key, &data, 0)) {
		case 0:
			if ((ret =
			    __edb_os_write(fd, data.data, data.size, &nw)) != 0)
				goto err;
			if (nw != (ssize_t)data.size) {
				ret = EIO;
				goto err;
			}
			break;
		case DB_KEYEMPTY:
			if (F_ISSET(edbp, DB_RE_FIXEDLEN)) {
				if ((ret =
				    __edb_os_write(fd, pad, rp->re_len, &nw)) != 0)
					goto err;
				if (nw != (ssize_t)rp->re_len) {
					ret = EIO;
					goto err;
				}
			}
			break;
		case DB_NOTFOUND:
			ret = 0;
			goto done;
		}
		if (!F_ISSET(edbp, DB_RE_FIXEDLEN)) {
			if ((ret = __edb_os_write(fd, &delim, 1, &nw)) != 0)
				goto err;
			if (nw != 1) {
				ret = EIO;
				goto err;
			}
		}
	}

err:
done:	/* Close the file descriptor. */
	if ((t_ret = __edb_os_close(fd)) != 0 || ret == 0)
		ret = t_ret;

	if (ret == 0)
		F_CLR(rp, RECNO_MODIFIED);
	return (ret);
}

/*
 * __ram_fmap --
 *	Get fixed length records from a file.
 */
static int
__ram_fmap(edbc, top)
	DBC *edbc;
	edb_recno_t top;
{
	DB *edbp;
	DBT data;
	RECNO *rp;
	edb_recno_t recno;
	u_int32_t len;
	u_int8_t *sp, *ep, *p;
	int ret;

	if ((ret = __bam_nrecs(edbc, &recno)) != 0)
		return (ret);

	edbp = edbc->edbp;
	rp = ((BTREE *)(edbp->internal))->recno;

	if (edbc->rdata.ulen < rp->re_len) {
		if ((ret = __edb_os_realloc(&edbc->rdata.data, rp->re_len)) != 0) {
			edbc->rdata.ulen = 0;
			edbc->rdata.data = NULL;
			return (ret);
		}
		edbc->rdata.ulen = rp->re_len;
	}

	memset(&data, 0, sizeof(data));
	data.data = edbc->rdata.data;
	data.size = rp->re_len;

	sp = (u_int8_t *)rp->re_cmap;
	ep = (u_int8_t *)rp->re_emap;
	while (recno < top) {
		if (sp >= ep) {
			F_SET(rp, RECNO_EOF);
			return (DB_NOTFOUND);
		}
		len = rp->re_len;
		for (p = edbc->rdata.data;
		    sp < ep && len > 0; *p++ = *sp++, --len)
			;

		/*
		 * Another process may have read this record from the input
		 * file and stored it into the database already, in which
		 * case we don't need to repeat that operation.  We detect
		 * this by checking if the last record we've read is greater
		 * or equal to the number of records in the database.
		 *
		 * XXX
		 * We should just do a seek, since the records are fixed
		 * length.
		 */
		if (rp->re_last >= recno) {
			if (len != 0)
				memset(p, rp->re_pad, len);

			++recno;
			if ((ret = __ram_add(edbc, &recno, &data, 0, 0)) != 0)
				return (ret);
		}
		++rp->re_last;
	}
	rp->re_cmap = sp;
	return (0);
}

/*
 * __ram_vmap --
 *	Get variable length records from a file.
 */
static int
__ram_vmap(edbc, top)
	DBC *edbc;
	edb_recno_t top;
{
	DBT data;
	RECNO *rp;
	edb_recno_t recno;
	u_int8_t *sp, *ep;
	int delim, ret;

	rp = ((BTREE *)(edbc->edbp->internal))->recno;

	if ((ret = __bam_nrecs(edbc, &recno)) != 0)
		return (ret);

	memset(&data, 0, sizeof(data));

	delim = rp->re_delim;

	sp = (u_int8_t *)rp->re_cmap;
	ep = (u_int8_t *)rp->re_emap;
	while (recno < top) {
		if (sp >= ep) {
			F_SET(rp, RECNO_EOF);
			return (DB_NOTFOUND);
		}
		for (data.data = sp; sp < ep && *sp != delim; ++sp)
			;

		/*
		 * Another process may have read this record from the input
		 * file and stored it into the database already, in which
		 * case we don't need to repeat that operation.  We detect
		 * this by checking if the last record we've read is greater
		 * or equal to the number of records in the database.
		 */
		if (rp->re_last >= recno) {
			data.size = sp - (u_int8_t *)data.data;
			++recno;
			if ((ret = __ram_add(edbc, &recno, &data, 0, 0)) != 0)
				return (ret);
		}
		++rp->re_last;
		++sp;
	}
	rp->re_cmap = sp;
	return (0);
}

/*
 * __ram_add --
 *	Add records into the tree.
 */
static int
__ram_add(edbc, recnop, data, flags, bi_flags)
	DBC *edbc;
	edb_recno_t *recnop;
	DBT *data;
	u_int32_t flags, bi_flags;
{
	BKEYDATA *bk;
	CURSOR *cp;
	DB *edbp;
	PAGE *h;
	edb_indx_t indx;
	int exact, isdeleted, ret, stack;

	edbp = edbc->edbp;
	cp = edbc->internal;

retry:	/* Find the slot for insertion. */
	if ((ret = __bam_rsearch(edbc, recnop,
	    S_INSERT | (flags == DB_APPEND ? S_APPEND : 0), 1, &exact)) != 0)
		return (ret);
	h = cp->csp->page;
	indx = cp->csp->indx;
	stack = 1;

	/*
	 * If re-numbering records, the on-page deleted flag means this record
	 * was implicitly created.  If not re-numbering records, the on-page
	 * deleted flag means this record was implicitly created, or, it was
	 * deleted at some time.
	 *
	 * If DB_NOOVERWRITE is set and the item already exists in the tree,
	 * return an error unless the item was either marked for deletion or
	 * only implicitly created.
	 */
	isdeleted = 0;
	if (exact) {
		bk = GET_BKEYDATA(h, indx);
		if (B_DISSET(bk->type))
			isdeleted = 1;
		else
			if (flags == DB_NOOVERWRITE) {
				ret = DB_KEYEXIST;
				goto err;
			}
	}

	/*
	 * Select the arguments for __bam_iitem() and do the insert.  If the
	 * key is an exact match, or we're replacing the data item with a
	 * new data item, replace the current item.  If the key isn't an exact
	 * match, we're inserting a new key/data pair, before the search
	 * location.
	 */
	switch (ret = __bam_iitem(edbc,
	    &h, &indx, NULL, data, exact ? DB_CURRENT : DB_BEFORE, bi_flags)) {
	case 0:
		/*
		 * Don't adjust anything.
		 *
		 * If we inserted a record, no cursors need adjusting because
		 * the only new record it's possible to insert is at the very
		 * end of the tree.  The necessary adjustments to the internal
		 * page counts were made by __bam_iitem().
		 *
		 * If we overwrote a record, no cursors need adjusting because
		 * future DBcursor->get calls will simply return the underlying
		 * record (there's no adjustment made for the DB_CURRENT flag
		 * when a cursor get operation immediately follows a cursor
		 * delete operation, and the normal adjustment for the DB_NEXT
		 * flag is still correct).
		 */
		break;
	case DB_NEEDSPLIT:
		/* Discard the stack of pages and split the page. */
		(void)__bam_stkrel(edbc, 0);
		stack = 0;

		if ((ret = __bam_split(edbc, recnop)) != 0)
			goto err;

		goto retry;
		/* NOTREACHED */
	default:
		goto err;
	}


err:	if (stack)
		__bam_stkrel(edbc, 0);

	return (ret);
}
