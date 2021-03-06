dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(retina, 0.0.1)
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_ENABLE_SHARED
AM_PROG_LIBTOOL

CFLAGS="$CFLAGS -Wall"

if test "x${exec_prefix}" = "xNONE"; then
  if test "x${prefix}" = "xNONE"; then
    bindir="${ac_default_prefix}/bin";
  else
    bindir="${prefix}/bin";
  fi
else
  if test "x${prefix}" = "xNONE"; then
    bindir="${ac_default_prefix}/bin";
  else
    bindir="${prefix}/bin";
  fi
fi

if test "x${exec_prefix}" = "xNONE"; then
  if test "x${prefix}" = "xNONE"; then
    libdir="${ac_default_prefix}/lib";
  else
    libdir="${prefix}/lib";
  fi
else
  if test "x${prefix}" = "xNONE"; then
    libdir="${ac_default_prefix}/lib";
  else
    libdir="${prefix}/lib";
  fi
fi

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [PACKAGE_DATA_DIR])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [PACKAGE_DATA_DIR])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datadir}/${PACKAGE}", [PACKAGE_DATA_DIR])
fi

dnl Set PACKAGE_BIN_DIR in config.h.
if test "x${bindir}" = "xNONE"; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${ac_default_prefix}/bin", [PACKAGE_BIN_DIR])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${prefix}/bin", [PACKAGE_BIN_DIR])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${bindir}", [PACKAGE_BIN_DIR])
fi

dnl Set PACKAGE_LIB_DIR in config.h.
if test "x${libdir}" = "xNONE"; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${ac_default_prefix}/lib", [PACKAGE_LIB_DIR])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${prefix}/lib", [PACKAGE_LIB_DIR])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${libdir}", [PACKAGE_LIB_DIR])
fi

PROG="imlib2-config";
AC_PATH_PROG(IMLIB2_CONFIG, $PROG, "", $PATH)
if test -z "$IMLIB2_CONFIG" ; then
  AC_MSG_ERROR($PROG is not in your \$PATH. Please ensure it is.)
fi
imlib2_libs=`$IMLIB2_CONFIG --libs`
imlib2_cflags=`$IMLIB2_CONFIG --cflags`
AC_SUBST(imlib2_libs)
AC_SUBST(imlib2_cflags)

PROG="evas-config";
AC_PATH_PROG(EVAS_CONFIG, $PROG, "", $PATH)
if test -z "$EVAS_CONFIG" ; then
  AC_MSG_ERROR($PROG is not in your \$PATH. Please ensure it is.)
fi
evas_libs=`$EVAS_CONFIG --libs`
evas_cflags=`$EVAS_CONFIG --cflags`
AC_SUBST(evas_libs)
AC_SUBST(evas_cflags)

PROG="ecore-config";
AC_PATH_PROG(ECORE_CONFIG, $PROG, "", $PATH)
if test -z "$ECORE_CONFIG" ; then
  AC_MSG_ERROR($PROG is not in your \$PATH. Please ensure it is.)
fi
ecore_libs=`$ECORE_CONFIG --libs`
ecore_cflags=`$ECORE_CONFIG --cflags`
AC_SUBST(ecore_libs)
AC_SUBST(ecore_cflags)

AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
data/images/Makefile
],[
])
