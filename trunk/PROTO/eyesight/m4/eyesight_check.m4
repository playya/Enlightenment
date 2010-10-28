
dnl use: EYESIGHT_CHECK_DEP_IMG(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EYESIGHT_CHECK_DEP_IMG],
[

requirement=""

PKG_CHECK_MODULES([ECORE_FILE],
   [ecore-file],
   [
    have_dep="yes"
    requirement="ecore-file"
   ],
   [have_dep="no"])

if test "x$1" = "xstatic" ; then
   requirement_eyesight="${requirement} ${requirement_eyesight}"
fi


AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EYESIGHT_CHECK_DEP_POPPLER(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EYESIGHT_CHECK_DEP_POPPLER],
[

requirement=""

PKG_CHECK_MODULES([POPPLER],
   [poppler >= 0.12],
   [
    have_dep="yes"
    requirement="poppler"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   AC_LANG_PUSH(C++)
   CPPFLAGS_save=${CPPFLAGS}
   CPPFLAGS="${CPPFLAGS} ${POPPLER_CFLAGS}"
   AC_CHECK_HEADER([GlobalParams.h],
      [have_dep="yes"],
      [
       AC_MSG_WARN([Xpdf headers not found. Verify that poppler is configured with the option --enable-xpdf-headers])
       have_dep="no"
      ])
   CPPFLAGS=${CPPFLAGS_save}
   AC_LANG_POP(C++)
fi

if test "x${have_dep}" = "xyes" ; then
   PKG_CHECK_MODULES([POPPLER_0_14],
      [poppler >= 0.14],
      [AC_DEFINE([HAVE_POPPLER_0_14], [1], [Set to 1 if poppler 0.14 is installed])],
      [dummy=yes])
fi

if test "x$1" = "xstatic" ; then
   requirement_eyesight="${requirement} ${requirement_eyesight}"
fi

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EYESIGHT_CHECK_DEP_POSTSCRIPT(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EYESIGHT_CHECK_DEP_POSTSCRIPT],
[

requirement=""

PKG_CHECK_MODULES([POSTSCRIPT],
   [libspectre],
   [
    have_dep="yes"
    requirement="libspectre"
   ],
   [have_dep="no"])

if test "x$1" = "xstatic" ; then
   requirement_eyesight="${requirement} ${requirement_eyesight}"
fi

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EYESIGHT_CHECK_DEP_TXT(want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EYESIGHT_CHECK_DEP_TXT],
[
have_dep="yes"

AS_IF([test "x$have_dep" = "xyes"], [$2], [$3])

])

dnl use: EYESIGHT_CHECK_MODULE(description, want_module[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([EYESIGHT_CHECK_MODULE],
[
m4_pushdef([UP], m4_translit([$1], [-a-z], [_A-Z]))dnl
m4_pushdef([DOWN], m4_translit([$1], [-A-Z], [_a-z]))dnl

want_module="$2"

AC_ARG_ENABLE([DOWN],
   [AC_HELP_STRING([--enable-]DOWN, [enable build of $1 module @<:@default=yes@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_module="yes"
    else
       if test "x${enableval}" = "xstatic" ; then
          enable_module="static"
       else
          enable_module="no"
       fi
    fi
   ],
   [enable_module="yes"])

if test "x${enable_module}" = "xyes" || test "x${enable_module}" = "xstatic" ; then
   want_module="yes"
fi

have_module="no"
if test "x${want_module}" = "xyes" && (test "x${enable_module}" = "xyes" || test "x${enable_module}" = "xstatic") ; then
   m4_default([EYESIGHT_CHECK_DEP_]m4_defn([UP]))(${enable_module}, [have_module="yes"], [have_module="no"])
fi

AC_MSG_CHECKING([whether to enable $1 module built])
AC_MSG_RESULT([${have_module}])

static_module="no"
if test "x${have_module}" = "xyes" && test "x${enable_module}" = "xstatic" ; then
   static_module="yes"
fi

AM_CONDITIONAL(EYESIGHT_BUILD_[]UP, [test "x${have_module}" = "xyes"])
AM_CONDITIONAL(EYESIGHT_STATIC_BUILD_[]UP, [test "x${static_module}" = "xyes"])

if test "x${static_module}" = "xyes" ; then
   AC_DEFINE(EYESIGHT_STATIC_BUILD_[]UP, 1, [Set to 1 if $1 is statically built])
   have_static_module="yes"
fi

enable_[]DOWN="no"
if test "x${have_module}" = "xyes" ; then
   enable_[]DOWN=${enable_module}
   AC_DEFINE(EYESIGHT_BUILD_[]UP, 1, [Set to 1 if $1 is built])
fi

AS_IF([test "x$have_module" = "xyes"], [$3], [$4])

m4_popdef([UP])
m4_popdef([DOWN])
])
