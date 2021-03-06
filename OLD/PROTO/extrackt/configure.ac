dnl Process this file with autoconf to produce a configure script.

rm -f config.cache

AC_INIT(configure.ac)

AC_DEFINE_UNQUOTED([CONFIGURED_WITH], "$*", [what options were given to configure])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(extrackt, 0.0.2)
AM_CONFIG_HEADER(config.h)

AC_PREREQ(2.52)

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_CXX
AC_LANG(C)


AM_ENABLE_SHARED
dnl AM_DISABLE_STATIC
AM_PROG_LIBTOOL

if test X"$enable_shared" = Xyes; then
 DLLDFLAGS=-export-dynamic
 AC_SUBST(DLLDFLAGS)
fi

AC_CHECK_HEADERS(sys/vfs.h sys/statvfs.h)
AC_CHECK_HEADERS(linux/cdrom.h linux/ucdrom.h sys/cdio.h io/cam/cdrom.h sys/mntent.h)
AC_CHECK_HEADERS(linux/soundcard.h machine/soundcard.h sys/soundcard.h sys/audioio.h sys/audio.io.h sun/audioio.h)


dnl ###########################################################################
dnl # functions
dnl ###################

AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AC_C_BIGENDIAN

AC_STRUCT_TM
AC_STRUCT_ST_BLKSIZE
AC_STRUCT_ST_BLOCKS

AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_SIGNAL
AC_TYPE_OFF_T
AC_TYPE_MODE_T

AC_CHECK_TYPE(ulong, ,[AC_DEFINE(ulong, u_long,[FreeBSD compat])])

AC_CHECK_HEADERS([limits.h])

AC_HEADER_TIME

AC_CHECK_FUNCS([localtime_r])
AC_CHECK_FUNCS([nl_langinfo])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strncasecmp])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([select])

AC_CHECK_FUNCS([ftello])
AC_CHECK_FUNCS([fseeko])
AC_CHECK_FUNCS([strtoul])
AC_CHECK_FUNCS([strtoull])


AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_MALLOC
AC_FUNC_FORK
AC_FUNC_MKTIME
AC_FUNC_LSTAT
AC_FUNC_CLOSEDIR_VOID

PKG_CHECK_MODULES(ECORE, [ecore >= 0.9.9],
   [
     AC_DEFINE(HAVE_ECORE, 1, have ECORE)
     ecore="ECORE "
     ecore_libs=$ECORE_LIBS
     ecore_includes=$ECORE_CFLAGS
   ],
   [
     AC_MSG_ERROR([No ECORE, can't build.])
   ]
)

PKG_CHECK_MODULES(EET, [eet >= 0.9.10],
   [
     AC_DEFINE(HAVE_EET, 1, have EET)
     eet="EET "
     eet_libs=$EET_LIBS
     eet_includes=$EET_CFLAGS
   ],
   [
     AC_MSG_ERROR([No EET, can't build.])
   ]
)

PKG_CHECK_MODULES(EVAS, [evas >= 0.9.9],
   [
     AC_DEFINE(HAVE_EVAS, 1, have EVAS)
     evas="EVAS "
     evas_libs=$EVAS_LIBS
     evas_includes=$EVAS_CFLAGS
   ],
   [
     AC_MSG_ERROR([No EVAS, can't build.])
   ]
)

PKG_CHECK_MODULES(ETK, [etk >= 0.1.0],
   [
     AC_DEFINE(HAVE_ETK, 1, have ETK)
     etk="ETK "
     AM_CONDITIONAL(HAVE_ETK, true)
     AM_CONDITIONAL(BUILD_FRONTEND_ETK, true)
     have_etk="yes"
     etk_libs=$ETK_LIBS
     etk_includes=$ETK_CFLAGS
   ],
   [
     AC_DEFINE(HAVE_ETK, 0, no ETK)
     AM_CONDITIONAL(HAVE_ETK, false)
     AM_CONDITIONAL(BUILD_FRONTEND_ETK, false)
     AC_MSG_WARN([No ETK, can't build ETK plugin.])
     have_etk="no"
     etk_libs=""
     etk_includes=""
   ]
)

PKG_CHECK_MODULES(ENHANCE, [enhance >= 0.0.1],
   [
     AC_DEFINE(HAVE_ENHANCE, 1, have ENHANCE)
     enhance="ENHANCE "
     AM_CONDITIONAL(HAVE_ENHANCE, true)
     have_enhance="yes"
     enhance_libs=$ENHANCE_LIBS
     ehnahce_includes=$ENHANCE_CFLAGS
     if test "x${have_etk}" = "xyes"; then
       AM_CONDITIONAL(BUILD_FRONTEND_ETK, true)
     else
       AM_CONDITIONAL(BUILD_FRONTEND_ETK, false)
     fi
   ],
   [
     AC_DEFINE(HAVE_ENHANCE, 0, no ENHANCE)
     AM_CONDITIONAL(HAVE_ENHANCE, false)
     AM_CONDITIONAL(BUILD_FRONTEND_ETK, false)
     AC_MSG_WARN([No Enhance, can't build ETK plugin.])
     have_enhance="no"
     enhance_libs=""
     enhance_includes=""
   ]
)

dnl ###########################################################################
dnl ###########################################################################
dnl ###########################################################################

dnl prefix
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_PREFIX,
  "${ac_default_prefix}", Prefix)
else
  AC_DEFINE_UNQUOTED(PACKAGE_PREFIX, "${prefix}", Prefix)
fi

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}", [Shared Data Directory])
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}", [Shared Data Directory])
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "${packagesrcdir}", src)

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl


my_includes="$ecore_includes $etk_includes $evas_includes $eet_includes $enhance_includes"
my_libs="$ecore_libs $etk_libs $evas_libs $eet_libs $enhance_libs"

AC_SUBST(my_includes)
AC_SUBST(my_libs)

dnl
dnl Adjust the Makefile.am conditionals accordingly
dnl

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
src/bin/etk_fe/Makefile
data/Makefile
data/other/Makefile
data/other/extrackt.desktop
data/etk_fe/Makefile
data/images/Makefile
])

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  ETK plugin..............: $have_enhance"
echo
echo "Installation Path.........: $prefix"
echo
echo "Compilation...............: make"
echo "Installation..............: make install"
echo

