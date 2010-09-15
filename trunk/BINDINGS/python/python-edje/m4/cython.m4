dnl AM_CHECK_CYTHON([VERSION [,ACTION-IF-FOUND [,ACTION-IF-NOT-FOUND]]])
dnl Check if a Cython version is installed
dnl Defines CYTHON_VERSION and CYTHON_FOUND
AC_DEFUN([AM_CHECK_CYTHON],
[
AC_REQUIRE([AM_PATH_PYTHON])
ifelse([$1], [], [_msg=""], [_msg=" >= $1"])
AC_MSG_CHECKING(for Cython$_msg)
AC_CACHE_VAL(py_cv_cython, [

prog="import Cython.Compiler.Version; print Cython.Compiler.Version.version"
CYTHON_VERSION=`$PYTHON -c "$prog" 2>&AC_FD_CC`

py_cv_cython=no
if test "x$CYTHON_VERSION" != "x"; then
   py_cv_cython=yes
fi

if test "x$py_cv_cython" = "xyes"; then
   ifelse([$1], [], [:],
      AS_VERSION_COMPARE([$CYTHON_VERSION], [$1], [py_cv_cython=no]))
fi
])

AC_MSG_RESULT([$py_cv_cython])

if test "x$py_cv_cython" = "xyes"; then
   CYTHON_FOUND=yes
   ifelse([$2], [], [:], [$2])
else
   CYTHON_FOUND=no
   ifelse([$3], [], [AC_MSG_ERROR([Could not find usable Cython$_msg])], [$3])
fi
])

