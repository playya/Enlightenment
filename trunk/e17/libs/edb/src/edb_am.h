/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)edb_am.h	10.15 (Sleepycat) 11/22/98
 */
#ifndef _DB_AM_H
#define _DB_AM_H

#define DB_ISBIG	0x01
#define	DB_ADD_DUP	0x10
#define	DB_REM_DUP	0x20
#define	DB_ADD_BIG	0x30
#define	DB_REM_BIG	0x40
#define	DB_SPLITOLD	0x50
#define	DB_SPLITNEW	0x60
#define	DB_ADD_PAGE	0x70
#define	DB_REM_PAGE	0x80

/*
 * Standard initialization and shutdown macros for all recovery functions.
 *
 * Requires the following local variables:
 *
 *	DB *file_edbp, *medbp;
 *	DB_MPOOLFILE *mpf;
 *	int ret;
 */
#define	REC_INTRO(func) {						\
	file_edbp = NULL;						\
	edbc = NULL;							\
	if ((ret = func(edbtp->data, &argp)) != 0)			\
		goto out;						\
	if ((ret =							\
	    __edb_fileid_to_edb(logp, &file_edbp, argp->fileid)) != 0) {	\
		if (ret	== DB_DELETED) {				\
			ret = 0;					\
			goto done;					\
		}							\
		goto out;						\
	}								\
	if (file_edbp == NULL)						\
		goto out;						\
	if ((ret = file_edbp->cursor(file_edbp, NULL, &edbc, 0)) != 0)	\
		goto out;						\
	F_SET(edbc, DBC_RECOVER);					\
	mpf = file_edbp->mpf;						\
}

#define	REC_CLOSE {							\
	if (argp != NULL)						\
		__edb_os_free(argp, sizeof(*argp));				\
	if (edbc != NULL)						\
		edbc->c_close(edbc);					\
	return (ret);							\
}

/*
 * No-op versions of the same macros.
 */
#define	REC_NOOP_INTRO(func) {						\
	if ((ret = func(edbtp->data, &argp)) != 0)			\
		return (ret);						\
}
#define	REC_NOOP_CLOSE {						\
	if (argp != NULL)						\
		__edb_os_free(argp, sizeof(*argp));				\
	return (ret);							\
}

/*
 * Standard debugging macro for all recovery functions.
 */
#ifdef DEBUG_RECOVER
#define	REC_PRINT(func)							\
	(void)func(logp, edbtp, lsnp, redo, info);
#else
#define	REC_PRINT(func)							\
	COMPQUIET(info, NULL);
#endif

#include "edb_auto.h"
#include "edb_ext.h"
#endif
