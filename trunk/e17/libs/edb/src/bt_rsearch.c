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
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
static const char sccsid[] = "@(#)bt_rsearch.c	10.21 (Sleepycat) 12/2/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "edb_int.h"
#include "edb_page.h"
#include "btree.h"

/*
 * __bam_rsearch --
 *	Search a btree for a record number.
 *
 * PUBLIC: int __bam_rsearch __P((DBC *, edb_recno_t *, u_int32_t, int, int *));
 */
int
__bam_rsearch(edbc, recnop, flags, stop, exactp)
	DBC *edbc;
	edb_recno_t *recnop;
	u_int32_t flags;
	int stop, *exactp;
{
	BINTERNAL *bi;
	CURSOR *cp;
	DB *edbp;
	DB_LOCK lock;
	PAGE *h;
	RINTERNAL *ri;
	edb_indx_t indx, top;
	edb_pgno_t pg;
	edb_recno_t i, recno, total;
	int ret, stack;

	edbp = edbc->edbp;
	cp = edbc->internal;

	BT_STK_CLR(cp);

	/*
	 * There are several ways we search a btree tree.  The flags argument
	 * specifies if we're acquiring read or write locks and if we are
	 * locking pairs of pages.  In addition, if we're adding or deleting
	 * an item, we have to lock the entire tree, regardless.  See btree.h
	 * for more details.
	 *
	 * If write-locking pages, we need to know whether or not to acquire a
	 * write lock on a page before getting it.  This depends on how deep it
	 * is in tree, which we don't know until we acquire the root page.  So,
	 * if we need to lock the root page we may have to upgrade it later,
	 * because we won't get the correct lock initially.
	 *
	 * Retrieve the root page.
	 */
	pg = PGNO_ROOT;
	stack = LF_ISSET(S_STACK);
	if ((ret = __bam_lget(edbc,
	    0, pg, stack ? DB_LOCK_WRITE : DB_LOCK_READ, &lock)) != 0)
		return (ret);
	if ((ret = memp_fget(edbp->mpf, &pg, 0, &h)) != 0) {
		(void)__BT_LPUT(edbc, lock);
		return (ret);
	}

	/*
	 * Decide if we need to save this page; if we do, write lock it.
	 * We deliberately don't lock-couple on this call.  If the tree
	 * is tiny, i.e., one page, and two threads are busily updating
	 * the root page, we're almost guaranteed deadlocks galore, as
	 * each one gets a read lock and then blocks the other's attempt
	 * for a write lock.
	 */
	if (!stack &&
	    ((LF_ISSET(S_PARENT) && (u_int8_t)(stop + 1) >= h->level) ||
	    (LF_ISSET(S_WRITE) && h->level == LEAFLEVEL))) {
		(void)memp_fput(edbp->mpf, h, 0);
		(void)__BT_LPUT(edbc, lock);
		if ((ret = __bam_lget(edbc, 0, pg, DB_LOCK_WRITE, &lock)) != 0)
			return (ret);
		if ((ret = memp_fget(edbp->mpf, &pg, 0, &h)) != 0) {
			(void)__BT_LPUT(edbc, lock);
			return (ret);
		}
		stack = 1;
	}

	/*
	 * If appending to the tree, set the record number now -- we have the
	 * root page locked.
	 *
	 * Delete only deletes exact matches, read only returns exact matches.
	 * Note, this is different from __bam_search(), which returns non-exact
	 * matches for read.
	 *
	 * The record may not exist.  We can only return the correct location
	 * for the record immediately after the last record in the tree, so do
	 * a fast check now.
	 */
	total = RE_NREC(h);
	if (LF_ISSET(S_APPEND)) {
		*exactp = 0;
		*recnop = recno = total + 1;
	} else {
		recno = *recnop;
		if (recno <= total)
			*exactp = 1;
		else {
			*exactp = 0;
			if (!LF_ISSET(S_PAST_EOF) || recno > total + 1) {
				(void)memp_fput(edbp->mpf, h, 0);
				(void)__BT_LPUT(edbc, lock);
				return (DB_NOTFOUND);
			}
		}
	}

	/*
	 * !!!
	 * Record numbers in the tree are 0-based, but the recno is
	 * 1-based.  All of the calculations below have to take this
	 * into account.
	 */
	for (total = 0;;) {
		switch (TYPE(h)) {
		case P_LBTREE:
			recno -= total;

			/*
			 * There may be logically deleted records on the page,
			 * walk the page correcting for them.  The record may
			 * not exist if there are enough deleted records in the
			 * page.
			 */
			if (recno <= (edb_recno_t)NUM_ENT(h) / P_INDX)
				for (i = recno - 1;; --i) {
					if (B_DISSET(GET_BKEYDATA(h,
					    i * P_INDX + O_INDX)->type))
						++recno;
					if (i == 0)
						break;
				}
			if (recno > (edb_recno_t)NUM_ENT(h) / P_INDX) {
				*exactp = 0;
				if (!LF_ISSET(S_PAST_EOF) || recno >
				    (edb_recno_t)(NUM_ENT(h) / P_INDX + 1)) {
					ret = DB_NOTFOUND;
					goto err;
				}

			}

			/* Correct from 1-based to 0-based for a page offset. */
			--recno;
			BT_STK_ENTER(cp, h, recno * P_INDX, lock, ret);
			return (ret);
		case P_IBTREE:
			for (indx = 0, top = NUM_ENT(h);;) {
				bi = GET_BINTERNAL(h, indx);
				if (++indx == top || total + bi->nrecs >= recno)
					break;
				total += bi->nrecs;
			}
			pg = bi->pgno;
			break;
		case P_LRECNO:
			recno -= total;

			/* Correct from 1-based to 0-based for a page offset. */
			--recno;
			BT_STK_ENTER(cp, h, recno, lock, ret);
			return (ret);
		case P_IRECNO:
			for (indx = 0, top = NUM_ENT(h);;) {
				ri = GET_RINTERNAL(h, indx);
				if (++indx == top || total + ri->nrecs >= recno)
					break;
				total += ri->nrecs;
			}
			pg = ri->pgno;
			break;
		default:
			return (__edb_pgfmt(edbp, h->pgno));
		}
		--indx;

		if (stack) {
			/* Return if this is the lowest page wanted. */
			if (LF_ISSET(S_PARENT) && stop == h->level) {
				BT_STK_ENTER(cp, h, indx, lock, ret);
				return (ret);
			}
			BT_STK_PUSH(cp, h, indx, lock, ret);
			if (ret != 0)
				goto err;

			if ((ret =
			    __bam_lget(edbc, 0, pg, DB_LOCK_WRITE, &lock)) != 0)
				goto err;
		} else {
			/*
			 * Decide if we want to return a pointer to the next
			 * page in the stack.  If we do, write lock it and
			 * never unlock it.
			 */
			if ((LF_ISSET(S_PARENT) &&
			    (u_int8_t)(stop + 1) >= (u_int8_t)(h->level - 1)) ||
			    (h->level - 1) == LEAFLEVEL)
				stack = 1;

			(void)memp_fput(edbp->mpf, h, 0);

			if ((ret =
			    __bam_lget(edbc, 1, pg, stack && LF_ISSET(S_WRITE) ?
			    DB_LOCK_WRITE : DB_LOCK_READ, &lock)) != 0)
				goto err;
		}

		if ((ret = memp_fget(edbp->mpf, &pg, 0, &h)) != 0)
			goto err;
	}
	/* NOTREACHED */

err:	BT_STK_POP(cp);
	__bam_stkrel(edbc, 0);
	return (ret);
}

/*
 * __bam_adjust --
 *	Adjust the tree after adding or deleting a record.
 *
 * PUBLIC: int __bam_adjust __P((DBC *, int32_t));
 */
int
__bam_adjust(edbc, adjust)
	DBC *edbc;
	int32_t adjust;
{
	CURSOR *cp;
	DB *edbp;
	EPG *epg;
	PAGE *h;
	int ret;

	edbp = edbc->edbp;
	cp = edbc->internal;

	/* Update the record counts for the tree. */
	for (epg = cp->sp; epg <= cp->csp; ++epg) {
		h = epg->page;
		if (TYPE(h) == P_IBTREE || TYPE(h) == P_IRECNO) {
			if (DB_LOGGING(edbc) &&
			    (ret = __bam_cadjust_log(edbp->edbenv->lg_info,
			    edbc->txn, &LSN(h), 0, edbp->log_fileid,
			    PGNO(h), &LSN(h), (u_int32_t)epg->indx,
			    adjust, 1)) != 0)
				return (ret);

			if (TYPE(h) == P_IBTREE)
				GET_BINTERNAL(h, epg->indx)->nrecs += adjust;
			else
				GET_RINTERNAL(h, epg->indx)->nrecs += adjust;

			if (PGNO(h) == PGNO_ROOT)
				RE_NREC_ADJ(h, adjust);

			if ((ret = memp_fset(edbp->mpf, h, DB_MPOOL_DIRTY)) != 0)
				return (ret);
		}
	}
	return (0);
}

/*
 * __bam_nrecs --
 *	Return the number of records in the tree.
 *
 * PUBLIC: int __bam_nrecs __P((DBC *, edb_recno_t *));
 */
int
__bam_nrecs(edbc, rep)
	DBC *edbc;
	edb_recno_t *rep;
{
	DB *edbp;
	DB_LOCK lock;
	PAGE *h;
	edb_pgno_t pgno;
	int ret;

	edbp = edbc->edbp;

	pgno = PGNO_ROOT;
	if ((ret = __bam_lget(edbc, 0, pgno, DB_LOCK_READ, &lock)) != 0)
		return (ret);
	if ((ret = memp_fget(edbp->mpf, &pgno, 0, &h)) != 0)
		return (ret);

	*rep = RE_NREC(h);

	(void)memp_fput(edbp->mpf, h, 0);
	(void)__BT_TLPUT(edbc, lock);

	return (0);
}

/*
 * __bam_total --
 *	Return the number of records below a page.
 *
 * PUBLIC: edb_recno_t __bam_total __P((PAGE *));
 */
edb_recno_t
__bam_total(h)
	PAGE *h;
{
	edb_recno_t nrecs;
	edb_indx_t indx, top;

	nrecs = 0;
	top = NUM_ENT(h);

	switch (TYPE(h)) {
	case P_LBTREE:
		/* Check for logically deleted records. */
		for (indx = 0; indx < top; indx += P_INDX)
			if (!B_DISSET(GET_BKEYDATA(h, indx + O_INDX)->type))
				++nrecs;
		break;
	case P_IBTREE:
		for (indx = 0; indx < top; indx += O_INDX)
			nrecs += GET_BINTERNAL(h, indx)->nrecs;
		break;
	case P_LRECNO:
		nrecs = NUM_ENT(h);
		break;
	case P_IRECNO:
		for (indx = 0; indx < top; indx += O_INDX)
			nrecs += GET_RINTERNAL(h, indx)->nrecs;
		break;
	}

	return (nrecs);
}
