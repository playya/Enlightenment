dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(configure.ac)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(emphasis, 0.0.1)

AC_PREREQ(2.52)

AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_ENABLE_SHARED
AM_PROG_LIBTOOL
      
dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Shared Data Directory])
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [Shared Data Directory])
fi

dnl Set PACKAGE_BIN_DIR in config.h.
if test "x${bindir}" = 'xNONE'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${ac_default_prefix}/bin", [Installation Directory for User Executables])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${prefix}/bin", [Installation Directory for User Executables])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${bindir}", [Installation Directory for User Executables])
fi

dnl Set PACKAGE_LIB_DIR in config.h.
if test "x${libdir}" = 'xNONE'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${ac_default_prefix}/lib", [Installation Directory for Libraries])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${prefix}/lib", [Installation Directory for Libraries])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${libdir}", [Installation Directory for Libraries])
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}", [Source Code Directory])

AC_CHECK_FUNCS(strndup)
AC_CHECK_LIB(pthread, pthread_create)

PKG_CHECK_MODULES(EMPHASIS, [
  ecore >= 0.9.9.022
  ecore-config
  etk >= 0.1.0.002
  enhance >= 0.0.1
  libxml-2.0 >= 2.6.0
  libmpd >= 0.12.0
])

if test X"$enable_shared" = Xyes; then
 DLLDFLAGS=-export-dynamic
 AC_SUBST(DLLDFLAGS)
fi

# now check for the actual version of libmpd -- it seems stupid enough
# not to publish its version number via the headers or the lib itself.
PKG_CHECK_EXISTS(libmpd >= 0.12.4, AC_DEFINE(LIBMPD_0_12_4, 1, [ Have libmpd >= 0.12.4 ]), [])
PKG_CHECK_EXISTS(libmpd >= 0.13, AC_DEFINE(LIBMPD_0_13, 1, [ Have libmpd >= 0.13 ]), [])

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
data/Makefile
data/images/Makefile
],[
touch emphasis_docs.tar.gz
])
