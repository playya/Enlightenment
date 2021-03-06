/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_seek.c	10.11 (Sleepycat) 10/12/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "edb_int.h"
#include "os_jump.h"

/*
 * __edb_os_seek --
 *	Seek to a page/byte offset in the file.
 *
 * PUBLIC: int __edb_os_seek __P((int, size_t, edb_pgno_t, u_int32_t, int, int));
 */
int
__edb_os_seek(fd, pgsize, pageno, relative, isrewind, whence)
	int fd;
	size_t pgsize;
	edb_pgno_t pageno;
	u_int32_t relative;
	int isrewind, whence;
{
	off_t offset;
	int ret;

	if (__edb_jump.j_seek != NULL)
		ret = __edb_jump.j_seek(fd,
		    pgsize, pageno, relative, isrewind, whence);
	else {
		offset = (off_t)pgsize * pageno + relative;
		if (isrewind)
			offset = -offset;

		ret = lseek(fd, offset, whence);
	}
	return (ret == -1 ? errno : 0);
}
