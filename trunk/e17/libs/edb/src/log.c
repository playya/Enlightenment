/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log.c	10.63 (Sleepycat) 10/10/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <shqueue.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "edb_int.h"
#include "shqueue.h"
#include "log.h"
#include "edb_dispatch.h"
#include "txn.h"
#include "txn_auto.h"
#include "common_ext.h"

static int __log_recover __P((DB_LOG *));

/*
 * log_open --
 *	Initialize and/or join a log.
 */
int
log_open(path, flags, mode, edbenv, lpp)
	const char *path;
	u_int32_t flags;
	int mode;
	DB_ENV *edbenv;
	DB_LOG **lpp;
{
	DB_LOG *edblp;
	LOG *lp;
	int ret;

	/* Validate arguments. */
#ifdef HAVE_SPINLOCKS
#define	OKFLAGS	(DB_CREATE | DB_THREAD)
#else
#define	OKFLAGS	(DB_CREATE)
#endif
	if ((ret = __edb_fchk(edbenv, "log_open", flags, OKFLAGS)) != 0)
		return (ret);

	/* Create and initialize the DB_LOG structure. */
	if ((ret = __edb_os_calloc(1, sizeof(DB_LOG), &edblp)) != 0)
		return (ret);

	if (path != NULL && (ret = __edb_os_strdup(path, &edblp->dir)) != 0)
		goto err;

	edblp->edbenv = edbenv;
	edblp->lfd = -1;
	ZERO_LSN(edblp->c_lsn);
	edblp->c_fd = -1;

	/*
	 * The log region isn't fixed size because we store the registered
	 * file names there.  Make it fairly large so that we don't have to
	 * grow it.
	 */
#define	DEF_LOG_SIZE	(30 * 1024)

	/* Map in the region. */
	edblp->reginfo.edbenv = edbenv;
	edblp->reginfo.appname = DB_APP_LOG;
	if (path == NULL)
		edblp->reginfo.path = NULL;
	else
		if ((ret = __edb_os_strdup(path, &edblp->reginfo.path)) != 0)
			goto err;
	edblp->reginfo.file = DB_DEFAULT_LOG_FILE;
	edblp->reginfo.mode = mode;
	edblp->reginfo.size = DEF_LOG_SIZE;
	edblp->reginfo.edbflags = flags;
	edblp->reginfo.flags = REGION_SIZEDEF;
	if ((ret = __edb_rattach(&edblp->reginfo)) != 0)
		goto err;

	/*
	 * The LOG structure is first in the region, the rest of the region
	 * is free space.
	 */
	edblp->lp = edblp->reginfo.addr;
	edblp->addr = (u_int8_t *)edblp->lp + sizeof(LOG);

	/* Initialize a created region. */
	if (F_ISSET(&edblp->reginfo, REGION_CREATED)) {
		__edb_shalloc_init(edblp->addr, DEF_LOG_SIZE - sizeof(LOG));

		/* Initialize the LOG structure. */
		lp = edblp->lp;
		lp->persist.lg_max = edbenv == NULL ? 0 : edbenv->lg_max;
		if (lp->persist.lg_max == 0)
			lp->persist.lg_max = DEFAULT_MAX;
		lp->persist.magic = DB_LOGMAGIC;
		lp->persist.version = DB_LOGVERSION;
		lp->persist.mode = mode;
		SH_TAILQ_INIT(&lp->fq);

		/* Initialize LOG LSNs. */
		lp->lsn.file = 1;
		lp->lsn.offset = 0;
	}

	/* Initialize thread information, mutex. */
	if (LF_ISSET(DB_THREAD)) {
		F_SET(edblp, DB_AM_THREAD);
		if ((ret = __edb_shalloc(edblp->addr,
		    sizeof(edb_mutex_t), MUTEX_ALIGNMENT, &edblp->mutexp)) != 0)
			goto err;
		(void)__edb_mutex_init(edblp->mutexp, 0);
	}

	/*
	 * If doing recovery, try and recover any previous log files before
	 * releasing the lock.
	 */
	if (F_ISSET(&edblp->reginfo, REGION_CREATED) &&
	    (ret = __log_recover(edblp)) != 0)
		goto err;

	UNLOCK_LOGREGION(edblp);
	*lpp = edblp;
	return (0);

err:	if (edblp->reginfo.addr != NULL) {
		if (edblp->mutexp != NULL)
			__edb_shalloc_free(edblp->addr, edblp->mutexp);

		UNLOCK_LOGREGION(edblp);
		(void)__edb_rdetach(&edblp->reginfo);
		if (F_ISSET(&edblp->reginfo, REGION_CREATED))
			(void)log_unlink(path, 1, edbenv);
	}

	if (edblp->reginfo.path != NULL)
		__edb_os_freestr(edblp->reginfo.path);
	if (edblp->dir != NULL)
		__edb_os_freestr(edblp->dir);
	__edb_os_free(edblp, sizeof(*edblp));
	return (ret);
}

/*
 * __log_panic --
 *	Panic a log.
 *
 * PUBLIC: void __log_panic __P((DB_ENV *));
 */
void
__log_panic(edbenv)
	DB_ENV *edbenv;
{
	if (edbenv->lg_info != NULL)
		edbenv->lg_info->lp->rlayout.panic = 1;
}

/*
 * __log_recover --
 *	Recover a log.
 */
static int
__log_recover(edblp)
	DB_LOG *edblp;
{
	DBT edbt;
	DB_LSN lsn;
	LOG *lp;
	u_int32_t chk;
	int cnt, found_checkpoint, ret;

	lp = edblp->lp;

	/*
	 * Find a log file.  If none exist, we simply return, leaving
	 * everything initialized to a new log.
	 */
	if ((ret = __log_find(edblp, 0, &cnt)) != 0)
		return (ret);
	if (cnt == 0)
		return (0);

	/*
	 * We have the last useful log file and we've loaded any persistent
	 * information.  Pretend that the log is larger than it can possibly
	 * be, and read the last file, looking for the last checkpoint and
	 * the log's end.
	 */
	lp->lsn.file = cnt + 1;
	lp->lsn.offset = 0;
	lsn.file = cnt;
	lsn.offset = 0;

	/* Set the cursor.  Shouldn't fail, leave error messages on. */
	memset(&edbt, 0, sizeof(edbt));
	if ((ret = __log_get(edblp, &lsn, &edbt, DB_SET, 0)) != 0)
		return (ret);

	/*
	 * Read to the end of the file, saving checkpoints.  This will fail
	 * at some point, so turn off error messages.
	 */
	found_checkpoint = 0;
	while (__log_get(edblp, &lsn, &edbt, DB_NEXT, 1) == 0) {
		if (edbt.size < sizeof(u_int32_t))
			continue;
		memcpy(&chk, edbt.data, sizeof(u_int32_t));
		if (chk == DB_txn_ckp) {
			lp->chkpt_lsn = lsn;
			found_checkpoint = 1;
		}
	}

	/*
	 * We now know where the end of the log is.  Set the first LSN that
	 * we want to return to an application and the LSN of the last known
	 * record on disk.
	 */
	lp->lsn = lp->s_lsn = lsn;
	lp->lsn.offset += edblp->c_len;

	/* Set up the current buffer information, too. */
	lp->len = edblp->c_len;
	lp->b_off = 0;
	lp->w_off = lp->lsn.offset;

	/*
	 * It's possible that we didn't find a checkpoint because there wasn't
	 * one in the last log file.  Start searching.
	 */
	while (!found_checkpoint && cnt > 1) {
		lsn.file = --cnt;
		lsn.offset = 0;

		/* Set the cursor.  Shouldn't fail, leave error messages on. */
		if ((ret = __log_get(edblp, &lsn, &edbt, DB_SET, 0)) != 0)
			return (ret);

		/*
		 * Read to the end of the file, saving checkpoints.  Shouldn't
		 * fail, leave error messages on.
		 */
		while (__log_get(edblp, &lsn, &edbt, DB_NEXT, 0) == 0) {
			if (edbt.size < sizeof(u_int32_t))
				continue;
			memcpy(&chk, edbt.data, sizeof(u_int32_t));
			if (chk == DB_txn_ckp) {
				lp->chkpt_lsn = lsn;
				found_checkpoint = 1;
			}
		}
	}
	/*
	 * Reset the cursor lsn to the beginning of the log, so that an
	 * initial call to DB_NEXT does the right thing.
	 */
	ZERO_LSN(edblp->c_lsn);

	/* If we never find a checkpoint, that's okay, just 0 it out. */
	if (!found_checkpoint)
		ZERO_LSN(lp->chkpt_lsn);

	/*
	 * !!!
	 * The test suite explicitly looks for this string -- don't change
	 * it here unless you also change it there.
	 */
	__edb_err(edblp->edbenv,
	    "Finding last valid log LSN: file: %lu offset %lu",
	    (u_long)lp->lsn.file, (u_long)lp->lsn.offset);

	return (0);
}

/*
 * __log_find --
 *	Try to find a log file.  If find_first is set, valp will contain
 * the number of the first log file, else it will contain the number of
 * the last log file.
 *
 * PUBLIC: int __log_find __P((DB_LOG *, int, int *));
 */
int
__log_find(edblp, find_first, valp)
	DB_LOG *edblp;
	int find_first, *valp;
{
	u_int32_t clv, logval;
	int cnt, fcnt, ret;
	const char *dir;
	char **names, *p, *q;

	*valp = 0;

	/* Find the directory name. */
	if ((ret = __log_name(edblp, 1, &p, NULL, 0)) != 0)
		return (ret);
	if ((q = __edb_rpath(p)) == NULL)
		dir = PATH_DOT;
	else {
		*q = '\0';
		dir = p;
	}

	/* Get the list of file names. */
	ret = __edb_os_dirlist(dir, &names, &fcnt);
	__edb_os_freestr(p);
	if (ret != 0) {
		__edb_err(edblp->edbenv, "%s: %s", dir, strerror(ret));
		return (ret);
	}

	/*
	 * Search for a valid log file name, return a value of 0 on
	 * failure.
	 *
	 * XXX
	 * Assumes that atoi(3) returns a 32-bit number.
	 */
	for (cnt = fcnt, clv = logval = 0; --cnt >= 0;) {
		if (strncmp(names[cnt], LFPREFIX, sizeof(LFPREFIX) - 1) != 0)
			continue;

		clv = atoi(names[cnt] + (sizeof(LFPREFIX) - 1));
		if (find_first) {
			if (logval != 0 && clv > logval)
				continue;
		} else
			if (logval != 0 && clv < logval)
				continue;

		if (__log_valid(edblp, clv, 1) == 0)
			logval = clv;
	}

	*valp = logval;

	/* Discard the list. */
	__edb_os_dirfree(names, fcnt);

	return (0);
}

/*
 * log_valid --
 *	Validate a log file.
 *
 * PUBLIC: int __log_valid __P((DB_LOG *, u_int32_t, int));
 */
int
__log_valid(edblp, number, set_persist)
	DB_LOG *edblp;
	u_int32_t number;
	int set_persist;
{
	LOGP persist;
	ssize_t nw;
	char *fname;
	int fd, ret;

	/* Try to open the log file. */
	if ((ret = __log_name(edblp,
	    number, &fname, &fd, DB_RDONLY | DB_SEQUENTIAL)) != 0) {
		__edb_os_freestr(fname);
		return (ret);
	}

	/* Try to read the header. */
	if ((ret = __edb_os_seek(fd, 0, 0, sizeof(HDR), 0, SEEK_SET)) != 0 ||
	    (ret = __edb_os_read(fd, &persist, sizeof(LOGP), &nw)) != 0 ||
	    nw != sizeof(LOGP)) {
		if (ret == 0)
			ret = EIO;

		(void)__edb_os_close(fd);

		__edb_err(edblp->edbenv,
		    "Ignoring log file: %s: %s", fname, strerror(ret));
		goto err;
	}
	(void)__edb_os_close(fd);

	/* Validate the header. */
	if (persist.magic != DB_LOGMAGIC) {
		__edb_err(edblp->edbenv,
		    "Ignoring log file: %s: magic number %lx, not %lx",
		    fname, (u_long)persist.magic, (u_long)DB_LOGMAGIC);
		ret = EINVAL;
		goto err;
	}
	if (persist.version < DB_LOGOLDVER || persist.version > DB_LOGVERSION) {
		__edb_err(edblp->edbenv,
		    "Ignoring log file: %s: unsupported log version %lu",
		    fname, (u_long)persist.version);
		ret = EINVAL;
		goto err;
	}

	/*
	 * If we're going to use this log file, set the region's persistent
	 * information based on the headers.
	 */
	if (set_persist) {
		edblp->lp->persist.lg_max = persist.lg_max;
		edblp->lp->persist.mode = persist.mode;
	}
	ret = 0;

err:	__edb_os_freestr(fname);
	return (ret);
}

/*
 * log_close --
 *	Close a log.
 */
int
log_close(edblp)
	DB_LOG *edblp;
{
	u_int32_t i;
	int ret, t_ret;

	LOG_PANIC_CHECK(edblp);

	/* We may have opened files as part of XA; if so, close them. */
	__log_close_files(edblp);

	/* Discard the per-thread pointer. */
	if (edblp->mutexp != NULL) {
		LOCK_LOGREGION(edblp);
		__edb_shalloc_free(edblp->addr, edblp->mutexp);
		UNLOCK_LOGREGION(edblp);
	}

	/* Close the region. */
	ret = __edb_rdetach(&edblp->reginfo);

	/* Close open files, release allocated memory. */
	if (edblp->lfd != -1 && (t_ret = __edb_os_close(edblp->lfd)) != 0 && ret == 0)
		ret = t_ret;
	if (edblp->c_edbt.data != NULL)
		__edb_os_free(edblp->c_edbt.data, edblp->c_edbt.ulen);
	if (edblp->c_fd != -1 &&
	    (t_ret = __edb_os_close(edblp->c_fd)) != 0 && ret == 0)
		ret = t_ret;
	if (edblp->edbentry != NULL) {
		for (i = 0; i < edblp->edbentry_cnt; i++)
			if (edblp->edbentry[i].name != NULL)
				__edb_os_freestr(edblp->edbentry[i].name);
		__edb_os_free(edblp->edbentry,
		    (edblp->edbentry_cnt * sizeof(DB_ENTRY)));
	}

	if (edblp->dir != NULL)
		__edb_os_freestr(edblp->dir);

	if (edblp->reginfo.path != NULL)
		__edb_os_freestr(edblp->reginfo.path);
	__edb_os_free(edblp, sizeof(*edblp));

	return (ret);
}

/*
 * log_unlink --
 *	Exit a log.
 */
int
log_unlink(path, force, edbenv)
	const char *path;
	int force;
	DB_ENV *edbenv;
{
	REGINFO reginfo;
	int ret;

	memset(&reginfo, 0, sizeof(reginfo));
	reginfo.edbenv = edbenv;
	reginfo.appname = DB_APP_LOG;
	if (path != NULL && (ret = __edb_os_strdup(path, &reginfo.path)) != 0)
		return (ret);
	reginfo.file = DB_DEFAULT_LOG_FILE;
	ret = __edb_runlink(&reginfo, force);
	if (reginfo.path != NULL)
		__edb_os_freestr(reginfo.path);
	return (ret);
}

/*
 * log_stat --
 *	Return LOG statistics.
 */
int
log_stat(edblp, gspp, edb_malloc)
	DB_LOG *edblp;
	DB_LOG_STAT **gspp;
	void *(*edb_malloc) __P((size_t));
{
	LOG *lp;
	int ret;

	*gspp = NULL;
	lp = edblp->lp;

	LOG_PANIC_CHECK(edblp);

	if ((ret = __edb_os_malloc(sizeof(**gspp), edb_malloc, gspp)) != 0)
		return (ret);

	/* Copy out the global statistics. */
	LOCK_LOGREGION(edblp);
	**gspp = lp->stat;

	(*gspp)->st_magic = lp->persist.magic;
	(*gspp)->st_version = lp->persist.version;
	(*gspp)->st_mode = lp->persist.mode;
	(*gspp)->st_lg_max = lp->persist.lg_max;

	(*gspp)->st_region_nowait = lp->rlayout.lock.mutex_set_nowait;
	(*gspp)->st_region_wait = lp->rlayout.lock.mutex_set_wait;

	(*gspp)->st_cur_file = lp->lsn.file;
	(*gspp)->st_cur_offset = lp->lsn.offset;

	(*gspp)->st_refcnt = lp->rlayout.refcnt;
	(*gspp)->st_regsize = lp->rlayout.size;

	UNLOCK_LOGREGION(edblp);

	return (0);
}
