dnl Process this file with autoconf to produce a configure script.

# get rid of that stupid cache mechanism
rm -f config.cache

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(enterminus, 0.1)
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AM_ENABLE_SHARED
AM_PROG_LIBTOOL
LIBS=""

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
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Shared Data Directory])
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [Shared Data Directory])
fi

dnl Set PACKAGE_BIN_DIR in config.h.
if test "x${bindir}" = 'xNONE'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${ac_default_prefix}/bin", "Binary dir")
  else
    AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${prefix}/bin", "Binary dir")
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_BIN_DIR, "${bindir}", "Binary dir")
fi

dnl Set PACKAGE_LIB_DIR in config.h.
if test "x${libdir}" = 'xNONE'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${ac_default_prefix}/lib", "Lib dir")
  else
    AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${prefix}/lib", "Lib dir")
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_LIB_DIR, "${libdir}", "Lib dir")
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}", "Package dir")


# add your required libs / include here
PKG_CHECK_MODULES(evas, [evas >= 0.9.9 ecore-evas >= 0.9.9])
PKG_CHECK_MODULES(ecore, [ecore >= 0.9.9])

CFLAGS="-Wall $CFLAGS"

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
data/Makefile
enterminus-config
],[
chmod +x enterminus-config
touch enterminus_docs.tar.gz
])
