/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */
#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)log_register.c	10.22 (Sleepycat) 9/27/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "edb_int.h"
#include "shqueue.h"
#include "log.h"
#include "common_ext.h"

/*
 * log_register --
 *	Register a file name.
 */
int
log_register(edblp, edbp, name, type, idp)
	DB_LOG *edblp;
	DB *edbp;
	const char *name;
	DBTYPE type;
	u_int32_t *idp;
{
	DBT fid_edbt, r_name;
	DB_LSN r_unused;
	FNAME *fnp, *reuse_fnp;
	size_t len;
	u_int32_t maxid;
	int inserted, ret;
	char *fullname;
	void *namep;

	inserted = 0;
	fullname = NULL;
	fnp = namep = reuse_fnp = NULL;

	LOG_PANIC_CHECK(edblp);

	/* Check the arguments. */
	if (type != DB_BTREE && type != DB_HASH && type != DB_RECNO) {
		__edb_err(edblp->edbenv, "log_register: unknown DB file type");
		return (EINVAL);
	}

	/* Get the log file id. */
	if ((ret = __edb_appname(edblp->edbenv,
	    DB_APP_DATA, NULL, name, 0, NULL, &fullname)) != 0)
		return (ret);

	LOCK_LOGREGION(edblp);

	/*
	 * See if we've already got this file in the log, finding the
	 * (maximum+1) in-use file id and some available file id (if we
	 * find an available fid, we'll use it, else we'll have to allocate
	 * one after the maximum that we found).
	 */
	for (maxid = 0, fnp = SH_TAILQ_FIRST(&edblp->lp->fq, __fname);
	    fnp != NULL; fnp = SH_TAILQ_NEXT(fnp, q, __fname)) {
		if (fnp->ref == 0) {		/* Entry is not in use. */
			if (reuse_fnp == NULL)
				reuse_fnp = fnp;
			continue;
		}
		if (!memcmp(edbp->fileid, fnp->ufid, DB_FILE_ID_LEN)) {
			++fnp->ref;
			goto found;
		}
		if (maxid <= fnp->id)
			maxid = fnp->id + 1;
	}

	/* Fill in fnp structure. */

	if (reuse_fnp != NULL)		/* Reuse existing one. */
		fnp = reuse_fnp;
	else if ((ret = __edb_shalloc(edblp->addr, sizeof(FNAME), 0, &fnp)) != 0)
		goto err;
	else				/* Allocate a new one. */
		fnp->id = maxid;

	fnp->ref = 1;
	fnp->s_type = type;
	memcpy(fnp->ufid, edbp->fileid, DB_FILE_ID_LEN);

	len = strlen(name) + 1;
	if ((ret = __edb_shalloc(edblp->addr, len, 0, &namep)) != 0)
		goto err;
	fnp->name_off = R_OFFSET(edblp, namep);
	memcpy(namep, name, len);

	/* Only do the insert if we allocated a new fnp. */
	if (reuse_fnp == NULL)
		SH_TAILQ_INSERT_HEAD(&edblp->lp->fq, fnp, q, __fname);
	inserted = 1;

found:	/* Log the registry. */
	if (!F_ISSET(edblp, DBC_RECOVER)) {
		r_name.data = (void *)name;		/* XXX: Yuck! */
		r_name.size = strlen(name) + 1;
		memset(&fid_edbt, 0, sizeof(fid_edbt));
		fid_edbt.data = edbp->fileid;
		fid_edbt.size = DB_FILE_ID_LEN;
		if ((ret = __log_register_log(edblp, NULL, &r_unused,
		    0, LOG_OPEN, &r_name, &fid_edbt, fnp->id, type)) != 0)
			goto err;
		if ((ret = __log_add_logid(edblp, edbp, name, fnp->id)) != 0)
			goto err;
	}

	if (0) {
err:		/*
		 * XXX
		 * We should grow the region.
		 */
		if (inserted)
			SH_TAILQ_REMOVE(&edblp->lp->fq, fnp, q, __fname);
		if (namep != NULL)
			__edb_shalloc_free(edblp->addr, namep);
		if (fnp != NULL)
			__edb_shalloc_free(edblp->addr, fnp);
	}

	if (idp != NULL)
		*idp = fnp->id;
	UNLOCK_LOGREGION(edblp);

	if (fullname != NULL)
		__os_freestr(fullname);

	return (ret);
}

/*
 * log_unregister --
 *	Discard a registered file name.
 */
int
log_unregister(edblp, fid)
	DB_LOG *edblp;
	u_int32_t fid;
{
	DBT fid_edbt, r_name;
	DB_LSN r_unused;
	FNAME *fnp;
	int ret;

	LOG_PANIC_CHECK(edblp);

	ret = 0;
	LOCK_LOGREGION(edblp);

	/* Find the entry in the log. */
	for (fnp = SH_TAILQ_FIRST(&edblp->lp->fq, __fname);
	    fnp != NULL; fnp = SH_TAILQ_NEXT(fnp, q, __fname))
		if (fid == fnp->id)
			break;
	if (fnp == NULL) {
		__edb_err(edblp->edbenv, "log_unregister: non-existent file id");
		ret = EINVAL;
		goto ret1;
	}

	/* Unlog the registry. */
	if (!F_ISSET(edblp, DBC_RECOVER)) {
		memset(&r_name, 0, sizeof(r_name));
		r_name.data = R_ADDR(edblp, fnp->name_off);
		r_name.size = strlen(r_name.data) + 1;
		memset(&fid_edbt, 0, sizeof(fid_edbt));
		fid_edbt.data = fnp->ufid;
		fid_edbt.size = DB_FILE_ID_LEN;
		if ((ret = __log_register_log(edblp, NULL, &r_unused,
		    0, LOG_CLOSE, &r_name, &fid_edbt, fid, fnp->s_type)) != 0)
			goto ret1;
	}

	/*
	 * If more than 1 reference, just decrement the reference and return.
	 * Otherwise, free the name.
	 */
	--fnp->ref;
	if (fnp->ref == 0)
		__edb_shalloc_free(edblp->addr, R_ADDR(edblp, fnp->name_off));

	/*
	 * Remove from the process local table.  If this operation is taking
	 * place during recovery, then the logid was never added to the table,
	 * so do not remove it.
	 */
	if (!F_ISSET(edblp, DBC_RECOVER))
		__log_rem_logid(edblp, fid);

ret1:	UNLOCK_LOGREGION(edblp);
	return (ret);
}
