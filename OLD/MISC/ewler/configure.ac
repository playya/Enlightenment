dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(ewler, 0.1.1)
AM_CONFIG_HEADER(src/config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_ENABLE_SHARED
AM_PROG_LIBTOOL

if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

dnl Add the languages which your application supports here.
ALL_LINGUAS=""

dnl Set PACKAGE_LOCALE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${ac_default_prefix}/${DATADIRNAME}/locale", [Locale data directory])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "${prefix}/${DATADIRNAME}/locale", [Locale data directory])
fi

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Shared data directory])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [Shared data directory])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datadir}/${PACKAGE}", [Shared data directory])
fi

dnl Set PACKAGE_BIN_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/bin'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${ac_default_prefix}/bin", [User executables directory])
  else
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${prefix}/bin", [User executables directory])
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${bindir}", [User executables directory])
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}", [Source code directory])

PROG="esmart-config";
AC_PATH_PROG(ESMART_CONFIG, $PROG, "", $PATH)
if [ test -z $ESMART_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
esmart_cflags=`$ESMART_CONFIG --cflags`
esmart_libs=`$ESMART_CONFIG --libs`
AC_SUBST(esmart_cflags)
AC_SUBST(esmart_libs)

PROG="evas-config";
AC_PATH_PROG(EVAS_CONFIG, $PROG, "", $PATH)
if [ test -z $EVAS_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
evas_cflags=`$EVAS_CONFIG --cflags`
evas_libs=`$EVAS_CONFIG --libs`
AC_SUBST(evas_cflags)
AC_SUBST(evas_libs)

PROG="edb-config";
AC_PATH_PROG(EDB_CONFIG, $PROG, "", $PATH)
if [ test -z $EDB_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
edb_cflags=`$EDB_CONFIG --cflags`
edb_libs=`$EDB_CONFIG --libs`
AC_SUBST(edb_cflags)
AC_SUBST(edb_libs)

PROG="ecore-config";
AC_PATH_PROG(ECORE_CONFIG, $PROG, "", $PATH)
if [ test -z $ECORE_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
ecore_cflags=`$ECORE_CONFIG --cflags`
ecore_libs=`$ECORE_CONFIG --libs`
AC_SUBST(ecore_cflags)
AC_SUBST(ecore_libs)

PROG="edje-config";
AC_PATH_PROG(EDJE_CONFIG, $PROG, "", $PATH)
if [ test -z $EDJE_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
edje_cflags=`$EDJE_CONFIG --cflags`
edje_libs=`$EDJE_CONFIG --libs`
AC_SUBST(edje_cflags)
AC_SUBST(edje_libs)

PROG="ewl-config";
AC_PATH_PROG(EWL_CONFIG, $PROG, "", $PATH)
if [ test -z $EWL_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
ewl_cflags=`$EWL_CONFIG --cflags`
ewl_libs=`$EWL_CONFIG --libs`
AC_SUBST(ewl_cflags)
AC_SUBST(ewl_libs)

PROG="exml-config";
AC_PATH_PROG(EXML_CONFIG, $PROG, "", $PATH)
if [ test -z $EXML_CONFIG ]; then
  echo $PROG " is not in your \$PATH. Please ensure it is.";
  echo "Read the manual page for you shell as to how to extend your path.";
  echo "FATAL ERROR. ABORT.";
  exit -1;
fi
exml_cflags=`$EXML_CONFIG --cflags`
exml_libs=`$EXML_CONFIG --libs`
AC_SUBST(exml_cflags)
AC_SUBST(exml_libs)

AC_OUTPUT([
Makefile 
src/Makefile 
data/Makefile
data/schemas/Makefile
data/images/Makefile
data/themes/Makefile
data/themes/ewler/Makefile
])

