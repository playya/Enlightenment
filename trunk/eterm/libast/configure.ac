dnl# $Id$

test "x$LIBTOOLIZE" = "x" && LIBTOOLIZE=libtoolize
test "x$ACLOCAL" = "x" && ACLOCAL=aclocal
test "x$AUTOCONF" = "x" && AUTOCONF=autoconf
test "x$AUTOHEADER" = "x" && AUTOHEADER=autoheader
test "x$AUTOMAKE" = "x" && AUTOMAKE=automake

AC_SUBST(LIBTOOLIZE)
AC_SUBST(ACLOCAL)
AC_SUBST(AUTOCONF)
AC_SUBST(AUTOHEADER)
AC_SUBST(AUTOMAKE)

SAVE_LIBTOOLIZE="$LIBTOOLIZE"
SAVE_ACLOCAL="$ACLOCAL"
SAVE_AUTOCONF="$AUTOCONF"
SAVE_AUTOHEADER="$AUTOHEADER"
SAVE_AUTOMAKE="$AUTOMAKE"

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(libast, 0.7.1)
AC_DEFINE_UNQUOTED(LIBAST_VERSION, "$VERSION", [Version])
AM_CONFIG_HEADER(config.h include/libast/sysdefs.h)

LIBTOOLIZE="${SAVE_LIBTOOLIZE:-libtoolize}"
ACLOCAL="${SAVE_ACLOCAL:-aclocal}"
AUTOCONF="${SAVE_AUTOCONF:-autoconf}"
AUTOHEADER="${SAVE_AUTOHEADER:-autoheader}"
AUTOMAKE="${SAVE_AUTOMAKE:-automake}"

AC_MSG_CHECKING(for preferred libtoolize)
AC_MSG_RESULT($LIBTOOLIZE)
AC_MSG_CHECKING(for preferred aclocal)
AC_MSG_RESULT($ACLOCAL)
AC_MSG_CHECKING(for preferred autoconf)
AC_MSG_RESULT($AUTOCONF)
AC_MSG_CHECKING(for preferred autoheader)
AC_MSG_RESULT($AUTOHEADER)
AC_MSG_CHECKING(for preferred automake)
AC_MSG_RESULT($AUTOMAKE)

dnl# Set some basic variables
AUTHORS="Michael Jennings (mej@eterm.org)"
AC_SUBST(AUTHORS)
AC_DEFINE_UNQUOTED(AUTHORS, "$AUTHORS", [Authors])

AC_CANONICAL_BUILD()
AC_CANONICAL_HOST()
AC_CANONICAL_TARGET()

AC_PROG_CC
AC_PROG_CPP

AST_STD_CHECKS()
AST_PROG_CHECKS()
AST_VAR_CHECKS()
AST_HEADER_CHECKS()
AST_TYPE_CHECKS()
AST_FUNC_CHECKS()

AC_PROG_LIBTOOL()

AST_ARG_DEBUG(DEBUG)
if test $DEBUG -eq 0 ; then
    AC_WARN(*** Debugging support disabled.  Client programs will ***)
    AC_WARN(*** not have access to any libast debugging routines! ***)
fi

AC_ARG_WITH(pedantry,
    [  --with-pedantry         add -Werror -std=c99 -pedantic to CFLAGS],
    [
        if test "$withval" != "no"; then
            CFLAGS="$CFLAGS -Wall -Werror -std=c99"
            AC_DEFINE(STRICT_ISO_C99, 1, [Defined if strict ISO C99 (9899:1999) is requested or required.])
        fi
    ]
)

AC_MSG_CHECKING(if STRICT_ISO_C99 required)
AC_TRY_RUN([
               int main () {
                   int a = 0, b = 0;
                   (long) a = (long) b;
                   return 0;
               }
           ], AC_MSG_RESULT(no), [
               AC_MSG_RESULT(yes)
               AC_DEFINE(STRICT_ISO_C99, 1, [Defined if strict ISO C99 (9899:1999) is requested or required.])
           ], [
               AC_MSG_RESULT(no - assumed because cross-compiling)
])

AST_X11_SUPPORT()
AST_IMLIB2_SUPPORT()
AST_MMX_SUPPORT()
AST_ARG_REGEXP(REGEXP)
AST_ARG_BACKQUOTE_EXEC(ALLOW_BACKQUOTE_EXEC)
AST_PTHREADS()

LIBCURL_CHECK_CONFIG([yes], [7.9.0], [], [])

AST_FLAGS()
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)

basedir=.
AC_OUTPUT(include/libast/types.h Makefile include/Makefile \
          include/libast/Makefile src/Makefile test/Makefile \
          libast-config)
test -f libast-config && chmod 755 libast-config

AST_STATUS()
