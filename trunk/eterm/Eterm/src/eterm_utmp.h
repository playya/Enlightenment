/*
 * Copyright (C) 1997-2001, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ETERM_UTMP_H_
#define ETERM_UTMP_H_

#include <X11/Xfuncproto.h>
#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */

#ifdef UTMP_SUPPORT
# ifdef HAVE_LIBUTEMPTER
#  include <utempter.h>
#  define add_utmp_entry(p, h, f)  addToUtmp(p, h, f)
#  define remove_utmp_entry()      removeFromUtmp()
# endif

/************ Macros and Definitions ************/
# ifndef UTMP_FILENAME
#   ifdef UTMP_FILE
#     define UTMP_FILENAME UTMP_FILE
#   elif defined(_PATH_UTMP)
#     define UTMP_FILENAME _PATH_UTMP
#   else
#     define UTMP_FILENAME "/etc/utmp"
#   endif
# endif

# ifndef LASTLOG_FILENAME
#  ifdef _PATH_LASTLOG
#   define LASTLOG_FILENAME _PATH_LASTLOG
#  else
#   define LASTLOG_FILENAME "/usr/adm/lastlog"	/* only on BSD systems */
#  endif
# endif

# ifndef WTMP_FILENAME
#   ifdef WTMP_FILE
#     define WTMP_FILENAME WTMP_FILE
#   elif defined(_PATH_WTMP)
#     define WTMP_FILENAME _PATH_WTMP
#   elif defined(SYSV)
#     define WTMP_FILENAME "/etc/wtmp"
#   else
#     define WTMP_FILENAME "/usr/adm/wtmp"
#   endif
# endif

# ifndef TTYTAB_FILENAME
#   ifdef TTYTAB
#     define TTYTAB_FILENAME TTYTAB_FILENAME
#   else
#     define TTYTAB_FILENAME "/etc/ttytab"
#   endif
# endif

# ifndef USER_PROCESS
#   define USER_PROCESS 7
# endif
# ifndef DEAD_PROCESS
#   define DEAD_PROCESS 8
# endif

/************ Function Prototypes ************/
_XFUNCPROTOBEGIN

# ifndef HAVE_LIBUTEMPTER
extern void add_utmp_entry(const char *, const char *, int);
extern void remove_utmp_entry(void);
# endif

_XFUNCPROTOEND

#else /* UTMP_SUPPORT */
# define add_utmp_entry(p, h, f)  NOP
# define remove_utmp_entry()      NOP
#endif

#endif	/* ETERM_UTMP_H_ */
