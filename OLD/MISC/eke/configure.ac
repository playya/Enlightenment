dnl Process this file with autoconf to produce a configure script

AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(eke, 0.1)
AM_CONFIG_HEADER(src/config.h)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL

AC_PATH_GENERIC(edje, , ,
    AC_MSG_ERROR(Cannot find edje: Is edje-config in your path?))

AC_PATH_GENERIC(ecore, , ,
    AC_MSG_ERROR(Cannot find ecore: Is ecore-config in your path?))

AC_PATH_GENERIC(esmart, , ,
    AC_MSG_ERROR(Cannot find esmart: Is esmart-config in your path?))

AC_PATH_GENERIC(xml2, , ,
    AC_MSG_ERROR(Cannot find libxml2: Is xml2-config in your path?))

AC_PATH_GENERIC(ewl, , ,
    AC_MSG_ERROR(Cannot find ewl: Is ewl-config in your path?))

AC_EXPAND_DIR(PACKAGE_DATA_DIR, "$datadir/${PACKAGE}")
AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "$PACKAGE_DATA_DIR", [Shared data directory])

AC_MSG_CHECKING(whether to build EWL gui)
want_ewl_gui="yes"
AC_ARG_ENABLE(ewl-gui, [ --disable-ewl-gui      disable build of the EWL gui ],
    [
        if [ test "$enableval" = "yes" ]; then
            AC_MSG_RESULT(yes)
            want_ewl_gui="yes"
        else
            AC_MSG_RESULT(no)
            want_ewl_gui="no"
        fi
    ], [
        AC_MSG_RESULT($want_ewl_gui)
    ]
)
if test "x$want_ewl_gui" = "xyes"; then
        AM_CONDITIONAL(BUILD_EWL_GUI, true)
        AC_DEFINE_UNQUOTED(BUILD_EWL_GUI, 1, [Build EWL GUI])
else
        AM_CONDITIONAL(BUILD_EWL_GUI, false)
        AC_DEFINE_UNQUOTED(BUILD_EWL_GUI, 0, [Disable EWL GUI])
fi

AC_MSG_CHECKING(whether to build Edje gui)
want_edje_gui="yes"
AC_ARG_ENABLE(edje-gui, [ --disable-edje-gui      disable build of the Edje gui ],
    [
        if [ test "$enableval" = "yes" ]; then
            AC_MSG_RESULT(yes)
            want_edje_gui="yes"
        else
            AC_MSG_RESULT(no)
            want_edje_gui="no"
        fi
    ], [
        AC_MSG_RESULT($want_edje_gui)
    ]
)
if test "x$want_edje_gui" = "xyes"; then
        AM_CONDITIONAL(BUILD_EDJE_GUI, true)
        AC_DEFINE_UNQUOTED(BUILD_EDJE_GUI, 1, [Build Edje GUI])
else
        AM_CONDITIONAL(BUILD_EDJE_GUI, false)
        AC_DEFINE_UNQUOTED(BUILD_EDJE_GUI, 0, [Disable Edje GUI])
fi

AC_OUTPUT([
Makefile
m4/Makefile
src/Makefile
data/Makefile
data/themes/Makefile
data/themes/default/Makefile
])

if test "x$want_edje_gui" = "xno"; then
    if test "x$want_ewl_gui" = "xno"; then
        echo
        echo "WARNING: No GUI being built!"
        echo
    fi
fi

echo
echo "$PACKAGE $VERSION"
echo
echo
echo "$PACKAGE $VERSION will be installed in $prefix"
echo
echo "Type 'make' ('gmake' on some systems) to compile $PACKAGE."
echo

