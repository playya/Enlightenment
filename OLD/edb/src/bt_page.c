/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
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
static const char sccsid[] = "@(#)bt_page.c	10.17 (Sleepycat) 1/3/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "edb_int.h"
#include "edb_page.h"
#include "btree.h"

/*
 * __bam_new --
 *	Get a new page, preferably from the freelist.
 *
 * PUBLIC: int __bam_new __P((DBC *, u_int32_t, PAGE **));
 */
int
__bam_new(edbc, type, pagepp)
	DBC *edbc;
	u_int32_t type;
	PAGE **pagepp;
{
	BTMETA *meta;
	DB *edbp;
	DB_LOCK metalock;
	PAGE *h;
	edb_pgno_t pgno;
	int ret;

	edbp = edbc->edbp;
	meta = NULL;
	h = NULL;
	metalock = LOCK_INVALID;

	pgno = PGNO_METADATA;
	if ((ret = __bam_lget(edbc, 0, pgno, DB_LOCK_WRITE, &metalock)) != 0)
		goto err;
	if ((ret = memp_fget(edbp->mpf, &pgno, 0, (PAGE **)&meta)) != 0)
		goto err;

	if (meta->free == PGNO_INVALID) {
		if ((ret = memp_fget(edbp->mpf, &pgno, DB_MPOOL_NEW, &h)) != 0)
			goto err;
		ZERO_LSN(h->lsn);
		h->pgno = pgno;
	} else {
		pgno = meta->free;
		if ((ret = memp_fget(edbp->mpf, &pgno, 0, &h)) != 0)
			goto err;
		meta->free = h->next_pgno;
	}

	/* Log the change. */
	if (DB_LOGGING(edbc)) {
		if ((ret = __bam_pg_alloc_log(edbp->edbenv->lg_info, edbc->txn,
		    &meta->lsn, 0, edbp->log_fileid, &meta->lsn, &h->lsn,
		    h->pgno, (u_int32_t)type, meta->free)) != 0)
			goto err;
		LSN(h) = LSN(meta);
	}

	(void)memp_fput(edbp->mpf, (PAGE *)meta, DB_MPOOL_DIRTY);
	(void)__BT_TLPUT(edbc, metalock);

	P_INIT(h, edbp->pgsize, h->pgno, PGNO_INVALID, PGNO_INVALID, 0, type);
	*pagepp = h;
	return (0);

err:	if (h != NULL)
		(void)memp_fput(edbp->mpf, h, 0);
	if (meta != NULL)
		(void)memp_fput(edbp->mpf, meta, 0);
	if (metalock != LOCK_INVALID)
		(void)__BT_TLPUT(edbc, metalock);
	return (ret);
}

/*
 * __bam_lput --
 *	The standard lock put call.
 *
 * PUBLIC: int __bam_lput __P((DBC *, DB_LOCK));
 */
int
__bam_lput(edbc, lock)
	DBC *edbc;
	DB_LOCK lock;
{
	return (__BT_LPUT(edbc, lock));
}

/*
 * __bam_free --
 *	Add a page to the head of the freelist.
 *
 * PUBLIC: int __bam_free __P((DBC *, PAGE *));
 */
int
__bam_free(edbc, h)
	DBC *edbc;
	PAGE *h;
{
	BTMETA *meta;
	DB *edbp;
	DBT ledbt;
	DB_LOCK metalock;
	edb_pgno_t pgno;
	u_int32_t dirty_flag;
	int ret, t_ret;

	edbp = edbc->edbp;

	/*
	 * Retrieve the metadata page and insert the page at the head of
	 * the free list.  If either the lock get or page get routines
	 * fail, then we need to put the page with which we were called
	 * back because our caller assumes we take care of it.
	 */
	dirty_flag = 0;
	pgno = PGNO_METADATA;
	if ((ret = __bam_lget(edbc, 0, pgno, DB_LOCK_WRITE, &metalock)) != 0)
		goto err;
	if ((ret = memp_fget(edbp->mpf, &pgno, 0, (PAGE **)&meta)) != 0) {
		(void)__BT_TLPUT(edbc, metalock);
		goto err;
	}

	/* Log the change. */
	if (DB_LOGGING(edbc)) {
		memset(&ledbt, 0, sizeof(ledbt));
		ledbt.data = h;
		ledbt.size = P_OVERHEAD;
		if ((ret = __bam_pg_free_log(edbp->edbenv->lg_info,
		    edbc->txn, &meta->lsn, 0, edbp->log_fileid, h->pgno,
		    &meta->lsn, &ledbt, meta->free)) != 0) {
			(void)memp_fput(edbp->mpf, (PAGE *)meta, 0);
			(void)__BT_TLPUT(edbc, metalock);
			return (ret);
		}
		LSN(h) = LSN(meta);
	}

	/*
	 * The page should have nothing interesting on it, re-initialize it,
	 * leaving only the page number and the LSN.
	 */
#ifdef DIAGNOSTIC
	{ edb_pgno_t __pgno; DB_LSN __lsn;
		__pgno = h->pgno;
		__lsn = h->lsn;
		memset(h, 0xedb, edbp->pgsize);
		h->pgno = __pgno;
		h->lsn = __lsn;
	}
#endif
	P_INIT(h, edbp->pgsize, h->pgno, PGNO_INVALID, meta->free, 0, P_INVALID);

	/* Link the page on the metadata free list. */
	meta->free = h->pgno;

	/* Discard the metadata page. */
	ret = memp_fput(edbp->mpf, (PAGE *)meta, DB_MPOOL_DIRTY);
	if ((t_ret = __BT_TLPUT(edbc, metalock)) != 0)
		ret = t_ret;

	/* Discard the caller's page reference. */
	dirty_flag = DB_MPOOL_DIRTY;
err:	if ((t_ret = memp_fput(edbp->mpf, h, dirty_flag)) != 0 && ret == 0)
		ret = t_ret;

	/*
	 * XXX
	 * We have to unlock the caller's page in the caller!
	 */
	return (ret);
}

#ifdef DEBUG
/*
 * __bam_lt --
 *	Print out the list of locks currently held by a cursor.
 *
 * PUBLIC: int __bam_lt __P((DBC *));
 */
int
__bam_lt(edbc)
	DBC *edbc;
{
	DB *edbp;
	DB_LOCKREQ req;

	edbp = edbc->edbp;
	if (F_ISSET(edbp, DB_AM_LOCKING)) {
		req.op = DB_LOCK_DUMP;
		lock_vec(edbp->edbenv->lk_info, edbc->locker, 0, &req, 1, NULL);
	}
	return (0);
}
#endif

/*
 * __bam_lget --
 *	The standard lock get call.
 *
 * PUBLIC: int __bam_lget
 * PUBLIC:    __P((DBC *, int, edb_pgno_t, edb_lockmode_t, DB_LOCK *));
 */
int
__bam_lget(edbc, do_couple, pgno, mode, lockp)
	DBC *edbc;
	int do_couple;
	edb_pgno_t pgno;
	edb_lockmode_t mode;
	DB_LOCK *lockp;
{
	DB *edbp;
	DB_LOCKREQ couple[2];
	int ret;

	edbp = edbc->edbp;

	if (!F_ISSET(edbp, DB_AM_LOCKING)) {
		*lockp = LOCK_INVALID;
		return (0);
	}

	edbc->lock.pgno = pgno;

	/*
	 * If the object not currently locked, acquire the lock and return,
	 * otherwise, lock couple.  If we fail and it's not a system error,
	 * convert to EAGAIN.
	 */
	if (do_couple) {
		couple[0].op = DB_LOCK_GET;
		couple[0].obj = &edbc->lock_edbt;
		couple[0].mode = mode;
		couple[1].op = DB_LOCK_PUT;
		couple[1].lock = *lockp;

		if (edbc->txn == NULL)
			ret = lock_vec(edbp->edbenv->lk_info,
			    edbc->locker, 0, couple, 2, NULL);
		else
			ret = lock_tvec(edbp->edbenv->lk_info,
			    edbc->txn, 0, couple, 2, NULL);
		if (ret != 0) {
			/* If we fail, discard the lock we held. */
			__BT_LPUT(edbc, *lockp);

			return (ret < 0 ? EAGAIN : ret);
		}
		*lockp = couple[0].lock;
	} else {
		if (edbc->txn == NULL)
			ret = lock_get(edbp->edbenv->lk_info,
			    edbc->locker, 0, &edbc->lock_edbt, mode, lockp);
		else
			ret = lock_tget(edbp->edbenv->lk_info,
			    edbc->txn, 0, &edbc->lock_edbt, mode, lockp);
		 return (ret < 0 ? EAGAIN : ret);
	}
	return (0);
}
