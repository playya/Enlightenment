#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <dlfcn.h> /* dlopen,dlclose,etc */

#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define SEMI_BROKEN_QUICKLAUNCH 1

static Elm_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Elm_Version *elm_version = &_version;

Eina_Bool
_elm_dangerous_call_check(const char *call)
{
   char buf[256];
   const char *eval;

   snprintf(buf, sizeof(buf), "%i.%i.%i.%i", VMAJ, VMIN, VMIC, VREV);
   eval = getenv("ELM_NO_FINGER_WAGGLING");
   if ((eval) && (!strcmp(eval, buf)))
     return 0;
   printf("ELEMENTARY FINGER WAGGLE!!!!!!!!!!\n"
          "\n"
          "  %s() used.\n"
          "PLEASE see the API documentation for this function. This call\n"
          "should almost never be used. Only in very special cases.\n"
          "\n"
          "To remove this warning please set the environment variable:\n"
          "  ELM_NO_FINGER_WAGGLING\n"
          "To the value of the Elementary version + revision number. e.g.:\n"
          "  1.2.5.40295\n"
          "\n"
          ,
          call);
   return 1;
}

/**
 * @defgroup Start Getting Started
 *
 * To write an Elementary app, you can get started with the following:
 *
 * @code
 * #include <Elementary.h>
 * #ifndef ELM_LIB_QUICKLAUNCH
 * EAPI int
 * elm_main(int argc, char **argv)
 * {
 *    // create window(s) here and do any application init
 *    elm_run(); // run main loop
 *    elm_shutdown(); // after mainloop finishes running, shutdown
 *    return 0; // exit 0 for exit code
 * }
 * #endif
 * ELM_MAIN()
 * @endcode
 *
 * To take full advantage of the quicklaunch architecture for launching
 * processes as quickly as possible (saving time at startup time like
 * connecting to X11, loading and linking shared libraries) you may want to
 * use the following configure.in/configure.ac and Makefile.am and autogen.sh
 * script to generate your files. It is assumed your application uses the
 * main.c file for its code.
 *
 * configure.in/configure.ac:
 *
@verbatim
AC_INIT(myapp, 0.0.0, myname@mydomain.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.in)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

AC_LIBTOOL_WIN32_DLL
define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([ELEMENTARY], elementary)

AC_OUTPUT(Makefile)
@endverbatim
 *
 * Makefile.am:
 *
@verbatim
AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) @ELEMENTARY_CFLAGS@

bin_PROGRAMS      = myapp
myapp_LTLIBRARIES = myapp.la

myappdir = $(libdir)

myapp_la_SOURCES = main.c
myapp_la_LIBADD = @ELEMENTARY_LIBS@
myapp_la_CFLAGS =
myapp_la_LDFLAGS = -module -avoid-version -no-undefined

myapp_SOURCES = main.c
myapp_LDADD = @ELEMENTARY_LIBS@
myapp_CFLAGS = -DELM_LIB_QUICKLAUNCH=1
@endverbatim
 *
 * autogen.sh:
 *
@verbatim
#!/bin/sh
rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh
rm -rf m4
mkdir m4

touch README
echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS -I m4 || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1

if [ -z "$NOCONFIGURE" ]; then
  ./configure "$@"
fi
@endverbatim
 *
 * To gnerate all the things needed to bootstrap just run:
 *
@verbatim
./autogen.sh
@endverbatim
 *
 * This will generate Makefile.in's, the confgure script and everything else.
 * After this it works like all normal autotools projects:
@verbatim
./configure
make
sudo make install
@endverbatim
 *
 * Note sudo was assumed to get root permissions, as this would install in
 * /usr/local which is system-owned. Ue any way you like to gain root, or
 * specify a different prefix with configure:
 *
@verbatim
./confiugre --prefix=$HOME/mysoftware
@endverbatim
 *
 * Also remember that autotools buys you some useful commands like:
@verbatim
make uninstall
@endverbatim
 *
 * This uninstalls the software after it was installed with "make install".
 * It is very useful to clear up what you built if you wish to clean the
 * system.
 *
@verbatim
make distcheck
@endverbatim
 *
 * This firstly checks if your build tree is "clean" and ready for
 * distribution. It also builds a tarball (myapp-0.0.0.tar.gz) that is
 * ready to upload and distribute to the world, that contains the generated
 * Makefile.in's and configure script. The users do not need to run
 * autogen.sh - just configure and on. They don't need autotools installed.
 * This tarball also builds cleanly, has all the sources it needs to build
 * included (that is sources for your application, not libraries it depends
 * on like Elementary). It builds cleanly in a buildroot and does not
 * contain any files that are temporarily generated like binaries and other
 * build-gnerated files, so the tarball is clean, and no need to worry
 * about cleaning up your tree before packaging.
 *
@verbatim
make clean
@endverbatim
 *
 * This cleans up all build files (binaries, objects etc.) from the tree.
 *
@verbatim
make distclean
@endverbatim
 *
 * This cleans out all files from the build and from configure's output too.
 *
@verbatim
make maintainer-clean
@endverbatim
 *
 * This deletes all the files autogen.sh will produce so the tree is clean
 * to be put into a revision-control system (like CVS, SVN or GIT for example).
 *
 * The above will build a library - libmyapp.so and install in the target
 * library directory (default is /usr/local/lib). You will also get a
 * myapp.a and myapp.la - these are useless and can be deleted. Libtool likes
 * to generate these all the time. You will also get a binary in the target
 * binary directory (default is /usr/local/bin). This is a "debug binary".
 * This will run and dlopen() the myapp.so and then jump to it's elm_main
 * function. This allows for easy debugging with GDB and Valgrind. When you
 * are ready to go to production do the following:
 *
 * 1. delete the myapp binary. i.e. rm /usr/local/bin/myapp
 *
 * 2. symlink the myapp binary to elementary_run (supplied by elementary).
 * i.e. ln -s elmentary_run /usr/local/bin/myapp
 *
 * 3. run elementary_quicklaunch as part of your graphical login session and
 * keep it running.
 *
 * This will man elementary_quicklaunch does pre-initialization before the
 * application needs to be run, saving the effort at the time the application
 * is needed, thus speeding up the time it takes to appear.
 *
 * If you don't want to use the quicklaunch infrastructure (which is
 * optional), you can execute the old fashioned way by just running the
 * myapp binary loader than will load the myapp.so for you, or you can
 * remove the split-file binary and put it into one binary as things always
 * have been with the following configure.in/configure.ac and Makfile.am
 * files:
 *
 * configure.in/configure.ac:
 *
@verbatim
AC_INIT(myapp, 0.0.0, myname@mydomain.com)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR(configure.in)

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

AC_C_BIGENDIAN
AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST

PKG_CHECK_MODULES([ELEMENTARY], elementary)

AC_OUTPUT(Makefile)
@endverbatim
 *
 * Makefile.am:
 *
@verbatim
AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) @ELEMENTARY_CFLAGS@

bin_PROGRAMS      = myapp

myapp_SOURCES = main.c
myapp_LDADD = @ELEMENTARY_LIBS@
myapp_CFLAGS =
@endverbatim
 *
 * Notice that they are the same as before, just with libtool and library
 * building sections removed. Both ways work for building elementary
 * applications. It is up to you to decide what is best for you. If you just
 * follow the template above, you can do it both ways and can decide at build
 * time. The more advanced of you may suggest making it a configure option.
 * That is perfectly valid, but has been left out here for simplicity, as our
 * aim to have an Elementary (and EFL) tutorial, not an autoconf & automake
 * document.
 *
 */

static Eina_Bool _elm_signal_exit(void *data,
                                  int   ev_type,
                                  void *ev);

char *_elm_appname = NULL;
const char *_elm_data_dir = NULL;
const char *_elm_lib_dir = NULL;
int _elm_log_dom = -1;

EAPI int ELM_EVENT_POLICY_CHANGED = 0;

static int _elm_init_count = 0;
static int _elm_sub_init_count = 0;
static int _elm_ql_init_count = 0;
static int _elm_policies[ELM_POLICY_LAST];
static Ecore_Event_Handler *_elm_exit_handler = NULL;
static Eina_Bool quicklaunch_on = 0;

static Eina_Bool
_elm_signal_exit(void *data  __UNUSED__,
                 int ev_type __UNUSED__,
                 void *ev    __UNUSED__)
{
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}

void
_elm_rescale(void)
{
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale(NULL, EINA_FALSE);
}

/**
 * @defgroup General General
 */

/**
 * Inititalise Elementary
 *
 * @return The init counter value.
 *
 * This call is exported only for use by the ELM_MAIN() macro. There is no
 * need to use this if you use this macro (which is highly advisable).
 * @ingroup General
 */
EAPI int
elm_init(int    argc,
         char **argv)
{
   _elm_init_count++;
   if (_elm_init_count > 1) return _elm_init_count;
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   return _elm_init_count;
}

/**
 * Shut down Elementary
 *
 * @return The init counter value.
 *
 * This should be called at the end of your application just before it ceases
 * to do any more processing. This will clean up any permanent resources your
 * application may have allocated via Elementary that would otherwise persist
 * on an exit without this call.
 * @ingroup General
 */
EAPI int
elm_shutdown(void)
{
   _elm_init_count--;
   if (_elm_init_count > 0) return _elm_init_count;
   elm_quicklaunch_sub_shutdown();
   elm_quicklaunch_shutdown();
   return _elm_init_count;
}

#ifdef ELM_EDBUS
static int _elm_need_e_dbus = 0;
#endif
EAPI Eina_Bool
elm_need_e_dbus(void)
{
#ifdef ELM_EDBUS
   if (_elm_need_e_dbus++) return EINA_TRUE;
   e_dbus_init();
   e_hal_init();
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_unneed_e_dbus(void)
{
#ifdef ELM_EDBUS
   if (--_elm_need_e_dbus) return;

   _elm_need_e_dbus = 0;
   e_hal_shutdown();
   e_dbus_shutdown();
#endif
}

#ifdef ELM_EFREET
static int _elm_need_efreet = 0;
#endif
EAPI Eina_Bool
elm_need_efreet(void)
{
#ifdef ELM_EFREET
   if (_elm_need_efreet++) return EINA_TRUE;
   efreet_init();
   efreet_mime_init();
   efreet_trash_init();
    /*
     {
        Eina_List **list;

        list = efreet_icon_extra_list_get();
        if (list)
          {
             e_user_dir_concat_static(buf, "icons");
             *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buf));
             e_prefix_data_concat_static(buf, "data/icons");
             *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buf));
          }
     }
   */
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_unneed_efreet(void)
{
#ifdef ELM_EFREET
   if (--_elm_need_efreet) return;

   _elm_need_efreet = 0;
   efreet_trash_shutdown();
   efreet_mime_shutdown();
   efreet_shutdown();
#endif
}

EAPI void
elm_quicklaunch_mode_set(Eina_Bool ql_on)
{
   quicklaunch_on = ql_on;
}

EAPI Eina_Bool
elm_quicklaunch_mode_get(void)
{
   return quicklaunch_on;
}

EAPI int
elm_quicklaunch_init(int    argc,
                     char **argv)
{
   char buf[PATH_MAX], *s;

   _elm_ql_init_count++;
   if (_elm_ql_init_count > 1) return _elm_ql_init_count;
   eina_init();
   _elm_log_dom = eina_log_domain_register("elementary", EINA_COLOR_LIGHTBLUE);
   if (!_elm_log_dom)
     {
        EINA_LOG_ERR("could not register elementary log domain.");
        _elm_log_dom = EINA_LOG_DOMAIN_GLOBAL;
     }

   eet_init();
   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);

   memset(_elm_policies, 0, sizeof(_elm_policies));
   if (!ELM_EVENT_POLICY_CHANGED)
     ELM_EVENT_POLICY_CHANGED = ecore_event_type_new();

   ecore_file_init();

   _elm_exit_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);

   if (argv) _elm_appname = strdup(ecore_file_file_get(argv[0]));

   if (!_elm_data_dir)
     {
        s = getenv("ELM_DATA_DIR");
        _elm_data_dir = eina_stringshare_add(s);
     }
   if (!_elm_data_dir)
     {
        s = getenv("ELM_PREFIX");
        if (s)
          {
             snprintf(buf, sizeof(buf), "%s/share/elementary", s);
             _elm_data_dir = eina_stringshare_add(buf);
          }
     }
   if (!_elm_lib_dir)
     {
        s = getenv("ELM_LIB_DIR");
        _elm_lib_dir = eina_stringshare_add(s);
     }
   if (!_elm_lib_dir)
     {
        s = getenv("ELM_PREFIX");
        if (s)
          {
             snprintf(buf, sizeof(buf), "%s/lib", s);
             _elm_lib_dir = eina_stringshare_add(buf);
          }
     }
#ifdef HAVE_DLADDR
   if ((!_elm_data_dir) || (!_elm_lib_dir))
     {
        Dl_info elementary_dl;
        // libelementary.so/../../share/elementary/
        if (dladdr(elm_init, &elementary_dl))
          {
             char *dir, *dir2;

             dir = ecore_file_dir_get(elementary_dl.dli_fname);
             if (dir)
               {
                  if (!_elm_lib_dir)
                    {
                       if (ecore_file_is_dir(dir))
                         _elm_lib_dir = eina_stringshare_add(dir);
                    }
                  if (!_elm_data_dir)
                    {
                       dir2 = ecore_file_dir_get(dir);
                       if (dir2)
                         {
                            snprintf(buf, sizeof(buf), "%s/share/elementary", dir2);
                            if (ecore_file_is_dir(buf))
                              _elm_data_dir = eina_stringshare_add(buf);
                            free(dir2);
                         }
                    }
                  free(dir);
               }
          }
     }
#endif
   if (!_elm_data_dir)
     _elm_data_dir = eina_stringshare_add(PACKAGE_DATA_DIR);
   if (!_elm_data_dir)
     _elm_data_dir = eina_stringshare_add("/");
   if (!_elm_lib_dir)
     _elm_lib_dir = eina_stringshare_add(PACKAGE_LIB_DIR);
   if (!_elm_lib_dir)
     _elm_lib_dir = eina_stringshare_add("/");

   _elm_config_init();
   return _elm_ql_init_count;
}

EAPI int
elm_quicklaunch_sub_init(int    argc,
                         char **argv)
{
   _elm_sub_init_count++;
   if (_elm_sub_init_count > 1) return _elm_sub_init_count;
   if (quicklaunch_on)
     {
#ifdef SEMI_BROKEN_QUICKLAUNCH
        return _elm_sub_init_count;
#endif
     }
   if (!quicklaunch_on)
     {
        ecore_app_args_set(argc, (const char **)argv);
        evas_init();
        edje_init();
        _elm_module_init();
        _elm_config_sub_init();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
#ifdef HAVE_ELEMENTARY_X
             ecore_x_init(NULL);
#endif
          }
        ecore_evas_init(); // FIXME: check errors
        ecore_imf_init();
     }
   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_sub_shutdown(void)
{
   _elm_sub_init_count--;
   if (_elm_sub_init_count > 0) return _elm_sub_init_count;
   if (quicklaunch_on)
     {
#ifdef SEMI_BROKEN_QUICKLAUNCH
        return _elm_sub_init_count;
#endif
     }
   if (!quicklaunch_on)
     {
        _elm_win_shutdown();
        _elm_module_shutdown();
        ecore_imf_shutdown();
        ecore_evas_shutdown();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
#ifdef HAVE_ELEMENTARY_X
             ecore_x_disconnect();
#endif
          }
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_SDL) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_SDL) ||
            ENGINE_COMPARE(ELM_OPENGL_SDL) ||
            ENGINE_COMPARE(ELM_SOFTWARE_WIN32) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
#undef ENGINE_COMPARE
          evas_cserve_disconnect();
        edje_shutdown();
        evas_shutdown();
     }
   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_shutdown(void)
{
   _elm_ql_init_count--;
   if (_elm_ql_init_count > 0) return _elm_ql_init_count;
   eina_stringshare_del(_elm_data_dir);
   _elm_data_dir = NULL;
   eina_stringshare_del(_elm_lib_dir);
   _elm_lib_dir = NULL;

   free(_elm_appname);
   _elm_appname = NULL;

   _elm_config_shutdown();

   ecore_event_handler_del(_elm_exit_handler);
   _elm_exit_handler = NULL;

   _elm_theme_shutdown();
   _elm_unneed_efreet();
   _elm_unneed_e_dbus();
   _elm_unneed_ethumb();
   ecore_file_shutdown();
   ecore_shutdown();
   eet_shutdown();

   if ((_elm_log_dom > -1) && (_elm_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
        eina_log_domain_unregister(_elm_log_dom);
        _elm_log_dom = -1;
     }

   _elm_widget_type_clear();

   eina_shutdown();
   return _elm_ql_init_count;
}

EAPI void
elm_quicklaunch_seed(void)
{
#ifndef SEMI_BROKEN_QUICKLAUNCH
   if (quicklaunch_on)
     {
        Evas_Object *win, *bg, *bt;

        win = elm_win_add(NULL, "seed", ELM_WIN_BASIC);
        bg = elm_bg_add(win);
        elm_win_resize_object_add(win, bg);
        evas_object_show(bg);
        bt = elm_button_add(win);
        elm_button_label_set(bt, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~-_=+\\|]}[{;:'\",<.>/?");
        elm_win_resize_object_add(win, bt);
        ecore_main_loop_iterate();
        evas_object_del(win);
        ecore_main_loop_iterate();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
# ifdef HAVE_ELEMENTARY_X
             ecore_x_sync();
# endif
          }
        ecore_main_loop_iterate();
     }
#endif
}

static void *qr_handle = NULL;
static int (*qr_main)(int    argc,
                      char **argv) = NULL;

EAPI Eina_Bool
elm_quicklaunch_prepare(int argc __UNUSED__,
                        char   **argv)
{
#ifdef HAVE_FORK
   char *exe = elm_quicklaunch_exe_path_get(argv[0]);
   if (!exe)
     {
        ERR("requested quicklaunch binary '%s' does not exist\n", argv[0]);
        return EINA_FALSE;
     }
   else
     {
        char *exe2, *p;
        char *exename;

        exe2 = malloc(strlen(exe) + 1 + 10);
        strcpy(exe2, exe);
        p = strrchr(exe2, '/');
        if (p) p++;
        else p = exe2;
        exename = alloca(strlen(p) + 1);
        strcpy(exename, p);
        *p = 0;
        strcat(p, "../lib/");
        strcat(p, exename);
        strcat(p, ".so");
        if (!access(exe2, R_OK | X_OK))
          {
             free(exe);
             exe = exe2;
          }
        else
          free(exe2);
     }
   qr_handle = dlopen(exe, RTLD_NOW | RTLD_GLOBAL);
   if (!qr_handle)
     {
        fprintf(stderr, "dlerr: %s\n", dlerror());
        WRN("dlopen('%s') failed: %s", exe, dlerror());
        free(exe);
        return EINA_FALSE;
     }
   INF("dlopen('%s') = %p", exe, qr_handle);
   qr_main = dlsym(qr_handle, "elm_main");
   INF("dlsym(%p, 'elm_main') = %p", qr_handle, qr_main);
   if (!qr_main)
     {
        WRN("not quicklauncher capable: no elm_main in '%s'", exe);
        dlclose(qr_handle);
        qr_handle = NULL;
        free(exe);
        return EINA_FALSE;
     }
   free(exe);
   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)argv;
#endif
}

#ifdef HAVE_FORK
static void
save_env(void)
{
   int i, size;
   extern char **environ;
   char **oldenv, **p;

   oldenv = environ;

   for (i = 0, size = 0; environ[i]; i++)
     size += strlen(environ[i]) + 1;

   p = malloc((i + 1) * sizeof(char *));
   if (!p) return;

   environ = p;

   for (i = 0; oldenv[i]; i++)
     environ[i] = strdup(oldenv[i]);
   environ[i] = NULL;
}

#endif

EAPI Eina_Bool
elm_quicklaunch_fork(int    argc,
                     char **argv,
                     char  *cwd,
                     void (postfork_func) (void *data),
                     void  *postfork_data)
{
#ifdef HAVE_FORK
   pid_t child;
   int ret;
   int real_argc;
   char **real_argv;

   // FIXME:
   // need to accept current environment from elementary_run
   if (!qr_main)
     {
        int i;
        char **args;

        child = fork();
        if (child > 0) return EINA_TRUE;
        else if (child < 0)
          {
             perror("could not fork");
             return EINA_FALSE;
          }
        setsid();
        if (chdir(cwd) != 0)
          perror("could not chdir");
        args = alloca((argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) args[i] = argv[i];
        args[argc] = NULL;
        WRN("%s not quicklaunch capable, fallback...", argv[0]);
        execvp(argv[0], args);
        ERR("failed to execute '%s': %s", argv[0], strerror(errno));
        exit(-1);
     }
   child = fork();
   if (child > 0) return EINA_TRUE;
   else if (child < 0)
     {
        perror("could not fork");
        return EINA_FALSE;
     }
   if (postfork_func) postfork_func(postfork_data);

   if (quicklaunch_on)
     {
#ifdef SEMI_BROKEN_QUICKLAUNCH
        ecore_app_args_set(argc, (const char **)argv);
        evas_init();
        edje_init();
        _elm_config_sub_init();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
# ifdef HAVE_ELEMENTARY_X
             ecore_x_init(NULL);
# endif
          }
        ecore_evas_init(); // FIXME: check errors
        ecore_imf_init();
        _elm_module_init();
#endif
     }

   setsid();
   if (chdir(cwd) != 0)
     perror("could not chdir");
   // FIXME: this is very linux specific. it changes argv[0] of the process
   // so ps etc. report what you'd expect. for other unixes and os's this
   // may just not work
   save_env();
   if (argv)
     {
        char *lastarg, *p;

        ecore_app_args_get(&real_argc, &real_argv);
        lastarg = real_argv[real_argc - 1] + strlen(real_argv[real_argc - 1]);
        for (p = real_argv[0]; p < lastarg; p++) *p = 0;
        strcpy(real_argv[0], argv[0]);
     }
   ecore_app_args_set(argc, (const char **)argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)argc;
   (void)argv;
   (void)cwd;
   (void)postfork_func;
   (void)postfork_data;
#endif
}

EAPI void
elm_quicklaunch_cleanup(void)
{
#ifdef HAVE_FORK
   if (qr_handle)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        qr_main = NULL;
     }
#endif
}

EAPI int
elm_quicklaunch_fallback(int    argc,
                         char **argv)
{
   int ret;
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   elm_quicklaunch_prepare(argc, argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return ret;
}

EAPI char *
elm_quicklaunch_exe_path_get(const char *exe)
{
   static char *path = NULL;
   static Eina_List *pathlist = NULL;
   const char *pathitr;
   const Eina_List *l;
   char buf[PATH_MAX];
   if (exe[0] == '/') return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '/')) return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/')) return strdup(exe);
   if (!path)
     {
        const char *p, *pp;
        char *buf2;
        path = getenv("PATH");
        buf2 = alloca(strlen(path) + 1);
        p = path;
        pp = p;
        for (;; )
          {
             if ((*p == ':') || (!*p))
               {
                  int len;

                  len = p - pp;
                  strncpy(buf2, pp, len);
                  buf2[len] = 0;
                  pathlist = eina_list_append(pathlist, eina_stringshare_add(buf2));
                  if (!*p) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (!*p) break;
                  p++;
               }
          }
     }
   EINA_LIST_FOREACH(pathlist, l, pathitr)
     {
        snprintf(buf, sizeof(buf), "%s/%s", pathitr, exe);
        if (!access(buf, R_OK | X_OK)) return strdup(buf);
     }
   return NULL;
}

/**
 * Run the main loop
 *
 * This call should be called just after all initialization is complete. This
 * function will not return until elm_exit() is called. It will keep looping
 * running the main event/processing loop for Elementary.
 * @ingroup General
 */
EAPI void
elm_run(void)
{
   ecore_main_loop_begin();
}

/**
 * Exit the main loop
 *
 * If this call is called, it will flag the main loop to cease processing and
 * return back to its parent function.
 * @ingroup General
 */
EAPI void
elm_exit(void)
{
   ecore_main_loop_quit();
}

/**
 * Set new policy value.
 *
 * This will emit the ecore event ELM_EVENT_POLICY_CHANGED in the main
 * loop giving the event information Elm_Event_Policy_Changed with
 * policy identifier, new and old values.
 *
 * @param policy policy identifier as in Elm_Policy.
 * @param value policy value, depends on identifiers, usually there is
 *        an enumeration with the same prefix as the policy name, for
 *        example: ELM_POLICY_QUIT and Elm_Policy_Quit
 *        (ELM_POLICY_QUIT_NONE, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED).
 * @ingroup General
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on error (right
 *         now just invalid policy identifier, but in future policy
 *         value might be enforced).
 */
EAPI Eina_Bool
elm_policy_set(unsigned int policy,
               int          value)
{
   Elm_Event_Policy_Changed *ev;

   if (policy >= ELM_POLICY_LAST)
     return EINA_FALSE;

   if (value == _elm_policies[policy])
     return EINA_TRUE;

   /* TODO: validade policy? */

   ev = malloc(sizeof(*ev));
   ev->policy = policy;
   ev->new_value = value;
   ev->old_value = _elm_policies[policy];

   _elm_policies[policy] = value;

   ecore_event_add(ELM_EVENT_POLICY_CHANGED, ev, NULL, NULL);

   return EINA_TRUE;
}

/**
 * Gets the policy value set for given identifier.
 *
 * @param policy policy identifier as in Elm_Policy.
 * @ingroup General
 *
 * @return policy value. Will be 0 if policy identifier is invalid.
 */
EAPI int
elm_policy_get(unsigned int policy)
{
   if (policy >= ELM_POLICY_LAST)
     return 0;
   return _elm_policies[policy];
}

/**
 * @defgroup Scaling Selective Widget Scaling
 *
 * Different widgets can be scaled independently. These functions allow you to
 * manipulate this scaling on a per-widget basis. The object and all its
 * children get their scaling factors multiplied by the scale factor set.
 * This is multiplicative, in that if a child also has a scale size set it is
 * in turn multiplied by its parent's scale size. 1.0 means “don't scale”,
 * 2.0 is double size, 0.5 is half etc.
 */

/**
 * Set the scaling factor
 *
 * @param obj The object
 * @param scale Scale factor (from 0.0 up, with 1.0 == no scaling)
 * @ingroup Scaling
 */
EAPI void
elm_object_scale_set(Evas_Object *obj,
                     double       scale)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scale_set(obj, scale);
}

/**
 * Get the scaling factor
 *
 * @param obj The object
 * @return The scaling factor set by elm_object_scale_set()
 * @ingroup Scaling
 */
EAPI double
elm_object_scale_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0.0);
   return elm_widget_scale_get(obj);
}

/**
 * Get the global scaling factor
 *
 * This gets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @return The scaling factor
 * @ingroup Scaling
 */
EAPI double
elm_scale_get(void)
{
   return _elm_config->scale;
}

/**
 * Set the global scaling factor
 *
 * This sets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @param scale The scaling factor to set
 * @ingroup Scaling
 */
EAPI void
elm_scale_set(double scale)
{
   if (_elm_config->scale == scale) return;
   _elm_config->scale = scale;
   _elm_rescale();
}

/**
 * Set the global scaling factor for all applications on the display
 *
 * This sets the globally configured scaling factor that is applied to all
 * objects for all applications.
 * @param scale The scaling factor to set
 * @ingroup Scaling
 */
EAPI void
elm_scale_all_set(double scale)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int scale_i = (unsigned int)(scale * 1000.0);

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_SCALE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &scale_i, 1);
#endif
}

/**
 * @defgroup Styles Styles
 *
 * Widgets can have different styles of look. These generic API's set
 * styles of widgets, if they support them (and if the theme(s) do).
 */

/**
 * Set the style
 *
 * This sets the name of the style
 * @param obj The object
 * @param style The style name to use
 * @ingroup Styles
 */
EAPI void
elm_object_style_set(Evas_Object *obj,
                     const char  *style)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_style_set(obj, style);
}

/**
 * Get the style
 *
 * This gets the style being used for that widget. Note that the string
 * pointer is only valid as longas the object is valid and the style doesn't
 * change.
 *
 * @param obj The object
 * @return The style name
 * @ingroup Styles
 */
EAPI const char *
elm_object_style_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_style_get(obj);
}

/**
 * Set the disable state
 *
 * This sets the disable state for the widget.
 *
 * @param obj The object
 * @param disabled The state
 * @ingroup Styles
 */
EAPI void
elm_object_disabled_set(Evas_Object *obj,
                        Eina_Bool    disabled)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_disabled_set(obj, disabled);
}

/**
 * Get the disable state
 *
 * This gets the disable state for the widget.
 *
 * @param obj The object
 * @return True, if the widget is disabled
 * @ingroup Styles
 */
EAPI Eina_Bool
elm_object_disabled_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_disabled_get(obj);
}

/**
 * @defgroup Config Elementary Config
 *
 * Elementary configuration is formed by a set options bounded to a
 * given @ref Profile profile, like @ref Theme theme, @ref Fingers
 * "finger size", etc. These are functions with which one syncronizes
 * changes made to those values to the configuration storing files, de
 * facto. You most probably don't want to use the functions in this
 * group unlees you're writing an elementary configuration manager.
 */

/**
 * Save back Elementary's configuration, so that it will persist on
 * future sessions.
 *
 * @return @c EINA_TRUE, when sucessful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * This function will take effect -- thus, do I/O -- immediately. Use
 * it when you want to apply all configuration changes at once. The
 * current configuration set will get saved onto the current profile
 * configuration file.
 *
 */
EAPI Eina_Bool
elm_config_save(void)
{
   return _elm_config_save();
}

/**
 * Reload Elementary's configuration, bounded to current selected
 * profile.
 *
 * @return @c EINA_TRUE, when sucessful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * Useful when you want to force reloading of configuration values for
 * a profile. If one removes user custom configuration directories,
 * for example, it will force a reload with system values insted.
 *
 */
EAPI void
elm_config_reload(void)
{
   _elm_config_reload();
}

/**
 * @defgroup Profile Elementary Profile
 *
 * Profiles are pre-set options that affect the whole look-and-feel of
 * Elementary-based applications. There are, for example, profiles
 * aimed at desktop computer applications and others aimed at mobile,
 * touchscreen-based ones. You most probably don't want to use the
 * functions in this group unlees you're writing an elementary
 * configuration manager.
 */

/**
 * Get Elementary's profile in use.
 *
 * This gets the global profile that is applied to all Elementary
 * applications.
 *
 * @return The profile's name
 * @ingroup Profile
 */
EAPI const char *
elm_profile_current_get(void)
{
   return _elm_config_current_profile_get();
}

/**
 * Get an Elementary's profile directory path in the filesystem. One
 * may want to fetch a system profile's dir or an user one (fetched
 * inside $HOME).
 *
 * @param profile The profile's name
 * @param is_user Whether to lookup for an user profile (@c EINA_TRUE)
 *                or a system one (@c EINA_FALSE)
 * @return The profile's directory path.
 * @ingroup Profile
 *
 * @note You must free it with elm_profile_dir_free().
 */
EAPI const char *
elm_profile_dir_get(const char *profile,
                    Eina_Bool   is_user)
{
   return _elm_config_profile_dir_get(profile, is_user);
}

/**
 * Free an Elementary's profile directory path, as returned by
 * elm_profile_dir_get().
 *
 * @param p_dir The profile's path
 * @ingroup Profile
 *
 */
EAPI void
elm_profile_dir_free(const char *p_dir)
{
   free((void *)p_dir);
}

/**
 * Get Elementary's list of available profiles.
 *
 * @return The profiles list. List node data are the profile name
 *         strings.
 * @ingroup Profile
 *
 * @note One must free this list, after usage, with the function
 *       elm_profile_list_free().
 */
EAPI Eina_List *
elm_profile_list_get(void)
{
   return _elm_config_profiles_list();
}

/**
 * Free Elementary's list of available profiles.
 *
 * @param The profiles list, as returned by elm_profile_list_get().
 * @ingroup Profile
 *
 */
EAPI void
elm_profile_list_free(Eina_List *l)
{
   const char *dir;

   EINA_LIST_FREE(l, dir)
     eina_stringshare_del(dir);
}

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to Elementary
 * applications. Just the process the call comes from will be
 * affected.
 *
 * @param profile The profile's name
 * @ingroup Profile
 *
 */
EAPI void
elm_profile_set(const char *profile)
{
   EINA_SAFETY_ON_NULL_RETURN(profile);
   _elm_config_profile_set(profile);
}

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to all Elementary
 * applications. All running Elementary windows will be affected.
 *
 * @param profile The profile's name
 * @ingroup Profile
 *
 */
EAPI void
elm_profile_all_set(const char *profile)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_PROFILE");
   ecore_x_window_prop_string_set(ecore_x_window_root_first_get(),
                                  atom, profile);
#endif
}

/**
 * @defgroup Engine Elementary Engine
 *
 * These are functions setting and querying which rendering engine
 * Elementary will use for drawing its windows' pixels.
 */

/**
 * Get Elementary's rendering engine in use.
 *
 * This gets the global rendering engine that is applied to all
 * Elementary applications.
 *
 * @return The rendering engine's name
 * @ingroup Engine
 *
 * @note there's no need to free the returned string, here.
 */
EAPI const char *
elm_engine_current_get(void)
{
   return _elm_config->engine;
}

/**
 * Set Elementary's rendering engine for use.
 *
 * This gets sets global rendering engine that is applied to all
 * Elementary applications. Note that it will take effect only to
 * subsequent Elementary window creations.
 *
 * @param The rendering engine's name
 * @ingroup Engine
 *
 * @note there's no need to free the returned string, here.
 */
EAPI void
elm_engine_set(const char *engine)
{
   EINA_SAFETY_ON_NULL_RETURN(engine);

   _elm_config_engine_set(engine);
}

/**
 * @defgroup Fonts Elementary Fonts
 *
 * These are functions dealing with font rendering, selection and the
 * like for Elementary applications. One might fetch which system
 * fonts are there to use and set custom fonts for individual classes
 * of UI items containing text (text classes).
 */

/**
 * Get Elementary's list of supported text classes.
 *
 * @return The text classes list, with @c Elm_Text_Class blobs as data.
 * @ingroup Fonts
 *
 * Release the list with elm_text_classes_list_free().
 */
EAPI const Eina_List *
elm_text_classes_list_get(void)
{
   return _elm_config_text_classes_get();
}

/**
 * Free Elementary's list of supported text classes.
 *
 * @ingroup Fonts
 *
 * @see elm_text_classes_list_get().
 */
EAPI void
elm_text_classes_list_free(const Eina_List *list)
{
   _elm_config_text_classes_free((Eina_List *)list);
}

/**
 * Get Elementary's list of font overlays, set with
 * elm_font_overlay_set().
 *
 * @return The font overlays list, with @c Elm_Font_Overlay blobs as
 * data.
 *
 * @ingroup Fonts
 *
 * For each text class, one can set a <b>font overlay</b> for it,
 * overriding the default font properties for that class coming from
 * the theme in use. There is no need to free this list.
 *
 * @see elm_font_overlay_set() and elm_font_overlay_unset().
 */
EAPI const Eina_List *
elm_font_overlay_list_get(void)
{
   return _elm_config_font_overlays_list();
}

/**
 * Set a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 * @param font Font name and style string
 * @param size Font size
 *
 * @ingroup Fonts
 *
 * @p font has to be in the format returned by
 * elm_font_fontconfig_name_get(). @see elm_font_overlay_list_get()
 * and @elm_font_overlay_unset().
 */
EAPI void
elm_font_overlay_set(const char    *text_class,
                     const char    *font,
                     Evas_Font_Size size)
{
   _elm_config_font_overlay_set(text_class, font, size);
}

/**
 * Unset a font overlay for a given Elementary text class.
 *
 * @param text_class Text class name
 *
 * @ingroup Fonts
 *
 * This will bring back text elements belonging to text class @p
 * text_class back to their default font settings.
 */
EAPI void
elm_font_overlay_unset(const char *text_class)
{
   _elm_config_font_overlay_remove(text_class);
}

/**
 * Apply the changes made with elm_font_overlay_set() and
 * elm_font_overlay_unset() on the current Elementary window.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 */
EAPI void
elm_font_overlay_apply(void)
{
   _elm_config_font_overlay_apply();
}

/**
 * Apply the changes made with elm_font_overlay_set() and
 * elm_font_overlay_unset() on all Elementary application windows.
 *
 * @ingroup Fonts
 *
 * This applies all font overlays set to all objects in the UI.
 */
EAPI void
elm_font_overlay_all_apply(void)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int dummy = (unsigned int)(1 * 1000.0);

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_FONT_OVERLAY");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(), atom, &dummy,
                                  1);
#endif
}

/**
 * Translate a font (family) name string in fontconfig's font names
 * syntax into an @c Elm_Font_Properties struct.
 *
 * @param font The font name and styles string
 * @return the font properties struct
 *
 * @ingroup Fonts
 *
 * @note The reverse translation can be achived with
 * elm_font_fontconfig_name_get(), for one style only (single font
 * instance, not family).
 */
EAPI Elm_Font_Properties *
elm_font_properties_get(const char *font)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(font, NULL);
   return _elm_font_properties_get(NULL, font);
}

/**
 * Free font properties return by elm_font_properties_get().
 *
 * @param efp the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void
elm_font_properties_free(Elm_Font_Properties *efp)
{
   const char *str;

   EINA_SAFETY_ON_NULL_RETURN(efp);
   EINA_LIST_FREE(efp->styles, str)
     if (str) eina_stringshare_del(str);
   if (efp->name) eina_stringshare_del(efp->name);
   free(efp);
}

/**
 * Translate a font name, bound to a style, into fontconfig's font names
 * syntax.
 *
 * @param name The font (family) name
 * @param style The given style (may be @c NULL)
 *
 * @return the font name and style string
 *
 * @ingroup Fonts
 *
 * @note The reverse translation can be achived with
 * elm_font_properties_get(), for one style only (single font
 * instance, not family).
 */
EAPI const char *
elm_font_fontconfig_name_get(const char *name,
                             const char *style)
{
   char buf[256];

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   if (!style || style[0] == 0) return eina_stringshare_add(name);
   snprintf(buf, 256, "%s" ELM_FONT_TOKEN_STYLE "%s", name, style);
   return eina_stringshare_add(buf);
}

/**
 * Free the font string return by elm_font_fontconfig_name_get().
 *
 * @param efp the font properties struct
 *
 * @ingroup Fonts
 */
EAPI void
elm_font_fontconfig_name_free(const char *name)
{
   eina_stringshare_del(name);
}

/**
 * Create a font hash table of available system fonts.
 *
 * One must call it with @p list being the return value of
 * evas_font_available_list(). The hash will be indexed by font
 * (family) names, being its values @c Elm_Font_Properties blobs.
 *
 * @param list The list of available system fonts, as returned by
 * evas_font_available_list().
 * @return the font hash.
 *
 * @ingroup Fonts
 *
 * @note The user is supposed to get it populated at least with 3
 * default font families (Sans, Serif, Monospace), which should be
 * present on most systems.
 */
EAPI Eina_Hash *
elm_font_available_hash_add(Eina_List *list)
{
   Eina_Hash *font_hash;
   Eina_List *l;
   void *key;

   font_hash = NULL;

   /* populate with default font families */
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Sans:style=Bold Oblique");

   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Serif:style=Bold Oblique");

   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Monospace:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Monospace:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Monospace:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash,
                                            "Monospace:style=Bold Oblique");

   EINA_LIST_FOREACH(list, l, key)
     font_hash = _elm_font_available_hash_add(font_hash, key);

   return font_hash;
}

/**
 * Free the hash return by elm_font_available_hash_add().
 *
 * @param hash the hash to be freed.
 *
 * @ingroup Fonts
 */
EAPI void
elm_font_available_hash_del(Eina_Hash *hash)
{
   _elm_font_available_hash_del(hash);
}

/**
 * @defgroup Fingers Fingers
 *
 * Elementary is designed to be finger-friendly for touchscreens, and so in
 * addition to scaling for display resolution, it can also scale based on
 * finger "resolution" (or size).
 */

/**
 * Get the configured finger size
 *
 * This gets the globally configured finger size in pixels
 *
 * @return The finger size
 * @ingroup Fingers
 */
EAPI Evas_Coord
elm_finger_size_get(void)
{
   return _elm_config->finger_size;
}

/**
 * Set the configured finger size
 *
 * This sets the globally configured finger size in pixels
 *
 * @param size The finger size
 * @ingroup Fingers
 */
EAPI void
elm_finger_size_set(Evas_Coord size)
{
   if (_elm_config->finger_size == size) return;
   _elm_config->finger_size = size;
   _elm_rescale();
}

/**
 * Set the configured finger size for all applications on the display
 *
 * This sets the globally configured finger size in pixels for all applications
 * on the display
 *
 * @param size The finger size
 * @ingroup Fingers
 */
EAPI void
elm_finger_size_all_set(Evas_Coord size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_FINGER_SIZE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * Adjust size of an element for finger usage
 *
 * This takes width and height sizes (in pixels) as input and a size multiple
 * (which is how many fingers you want to place within the area), and adjusts
 * the size tobe large enough to accommodate finger. On return the w and h
 * sizes poiner do by these parameters will be modified.
 *
 * @param times_w How many fingers should fit horizontally
 * @param w Pointer to the width size to adjust
 * @param times_h How many fingers should fit vertically
 * @param h Pointer to the height size to adjust
 * @ingroup Fingers
 */
EAPI void
elm_coords_finger_size_adjust(int         times_w,
                              Evas_Coord *w,
                              int         times_h,
                              Evas_Coord *h)
{
   if ((w) && (*w < (_elm_config->finger_size * times_w)))
     *w = _elm_config->finger_size * times_w;
   if ((h) && (*h < (_elm_config->finger_size * times_h)))
     *h = _elm_config->finger_size * times_h;
}

/**
 * @defgroup Caches Caches
 *
 * These are functions which let one fine-tune some cache values for
 * Elementary applications, thus allowing for performance adjustments.
 */

/**
 * Flush all caches & dump all data that can be to lean down to use
 * less memory
 *
 * @ingroup Caches
 */
EAPI void
elm_all_flush(void)
{
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        Evas *e = evas_object_evas_get(obj);
        edje_file_cache_flush();
        edje_collection_cache_flush();
        eet_clearcache();
        evas_image_cache_flush(e);
        evas_font_cache_flush(e);
        evas_render_dump(e);
     }
}

/**
 * Get the configured cache flush interval time
 *
 * This gets the globally configured cache flush interval time, in
 * ticks
 *
 * @return The cache flush interval time
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI int
elm_cache_flush_interval_get(void)
{
   return _elm_config->cache_flush_poll_interval;
}

/**
 * Set the configured cache flush interval time
 *
 * This sets the globally configured cache flush interval time, in ticks
 *
 * @param size The cache flush interval time
 * @ingroup Caches
 *
 * @see elm_all_flush()
 */
EAPI void
elm_cache_flush_interval_set(int size)
{
   if (_elm_config->cache_flush_poll_interval == size) return;
   _elm_config->cache_flush_poll_interval = size;

   _elm_recache();
}

/**
 * Set the configured cache flush interval time for all applications on the
 * display
 *
 * This sets the globally configured cache flush interval time -- in ticks
 * -- for all applications on the display.
 *
 * @param size The cache flush interval time
 * @ingroup Caches
 */
EAPI void
elm_cache_flush_interval_all_set(int size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_CACHE_FLUSH_INTERVAL");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * Get the configured font cache size
 *
 * This gets the globally configured font cache size, in bytes
 *
 * @return The font cache size
 * @ingroup Caches
 */
EAPI int
elm_font_cache_get(void)
{
   return _elm_config->font_cache;
}

/**
 * Set the configured font cache size
 *
 * This sets the globally configured font cache size, in bytes
 *
 * @param size The font cache size
 * @ingroup Caches
 */
EAPI void
elm_font_cache_set(int size)
{
   if (_elm_config->font_cache == size) return;
   _elm_config->font_cache = size;

   _elm_recache();
}

/**
 * Set the configured font cache size for all applications on the
 * display
 *
 * This sets the globally configured font cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The font cache size
 * @ingroup Caches
 */
EAPI void
elm_font_cache_all_set(int size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_FONT_CACHE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * Get the configured image cache size
 *
 * This gets the globally configured image cache size, in bytes
 *
 * @return The image cache size
 * @ingroup Caches
 */
EAPI int
elm_image_cache_get(void)
{
   return _elm_config->image_cache;
}

/**
 * Set the configured image cache size
 *
 * This sets the globally configured image cache size, in bytes
 *
 * @param size The image cache size
 * @ingroup Caches
 */
EAPI void
elm_image_cache_set(int size)
{
   if (_elm_config->image_cache == size) return;
   _elm_config->image_cache = size;

   _elm_recache();
}

/**
 * Set the configured image cache size for all applications on the
 * display
 *
 * This sets the globally configured image cache size -- in bytes
 * -- for all applications on the display.
 *
 * @param size The image cache size
 * @ingroup Caches
 */
EAPI void
elm_image_cache_all_set(int size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_IMAGE_CACHE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * Get the configured edje file cache size.
 *
 * This gets the globally configured edje file cache size, in number
 * of files.
 *
 * @return The edje file cache size
 * @ingroup Caches
 */
EAPI int
elm_edje_file_cache_get(void)
{
   return _elm_config->edje_cache;
}

/**
 * Set the configured edje file cache size
 *
 * This sets the globally configured edje file cache size, in number
 * of files.
 *
 * @param size The edje file cache size
 * @ingroup Caches
 */
EAPI void
elm_edje_file_cache_set(int size)
{
   if (_elm_config->edje_cache == size) return;
   _elm_config->edje_cache = size;

   _elm_recache();
}

/**
 * Set the configured edje file cache size for all applications on the
 * display
 *
 * This sets the globally configured edje file cache size -- in number
 * of files -- for all applications on the display.
 *
 * @param size The edje file cache size
 * @ingroup Caches
 */
EAPI void
elm_edje_file_cache_all_set(int size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_EDJE_FILE_CACHE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * Get the configured edje collections (groups) cache size.
 *
 * This gets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @return The edje collections cache size
 * @ingroup Caches
 */
EAPI int
elm_edje_collection_cache_get(void)
{
   return _elm_config->edje_collection_cache;
}

/**
 * Set the configured edje collections (groups) cache size
 *
 * This sets the globally configured edje collections cache size, in
 * number of collections.
 *
 * @param size The edje collections cache size
 * @ingroup Caches
 */
EAPI void
elm_edje_collection_cache_set(int size)
{
   if (_elm_config->edje_collection_cache == size) return;
   _elm_config->edje_collection_cache = size;

   _elm_recache();
}

/**
 * Set the configured edje collections (groups) cache size for all
 * applications on the display
 *
 * This sets the globally configured edje collections cache size -- in
 * number of collections -- for all applications on the display.
 *
 * @param size The edje collections cache size
 * @ingroup Caches
 */
EAPI void
elm_edje_collection_cache_all_set(int size)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int size_i = (unsigned int)size;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_EDJE_COLLECTION_CACHE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &size_i, 1);
#endif
}

/**
 * @defgroup Focus Focus
 *
 * Objects have focus. This is what determines where the keyboard input goes to
 * within the application window.
 */

/**
 * Get the focus of the object
 *
 * This gets the focused property of the object.
 *
 * @param obj The object
 * @return 1 if the object is focused, 0 if not.
 * @ingroup Focus
 */
EAPI Eina_Bool
elm_object_focus_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_focus_get(obj);
}

/**
 * Set the focus to the object
 *
 * This sets the focus target for keyboard input to be the object indicated.
 *
 * @param obj The object
 * @ingroup Focus
 */
EAPI void
elm_object_focus(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   if (elm_widget_focus_get(obj))
     return;

   elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
}

/**
 * Remove the focus from the object
 *
 * This removes the focus target for keyboard input from be the object
 * indicated.
 *
 * @param obj The object
 * @ingroup Focus
 */
EAPI void
elm_object_unfocus(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   if (!elm_widget_can_focus_get(obj)) return;
   elm_widget_focused_object_clear(obj);
}

/**
 * Set the ability for the object to focus
 *
 * This sets the ability for the object to be able to get keyboard focus or
 * not. By default all objects are able to be focused.
 *
 * @param obj The object
 * @param enable 1 if the object can be focused, 0 if not
 * @ingroup Focus
 */
EAPI void
elm_object_focus_allow_set(Evas_Object *obj,
                           Eina_Bool    enable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_can_focus_set(obj, enable);
}

/**
 * Get the ability for the object to focus
 *
 * This gets the ability for the object to be able to get keyboard focus or
 * not. By default all objects are able to be focused.
 *
 * @param obj The object
 * @return 1 if the object is allowed to be focused, 0 if not.
 * @ingroup Focus
 */
EAPI Eina_Bool
elm_object_focus_allow_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return (elm_widget_can_focus_get(obj)) || (elm_widget_child_can_focus_get(obj));
}

/**
 * Set custom focus chain.
 *
 * This function i set one new and overwrite any previous custom focus chain
 * with the list of objects. The previous list will be deleted and this list
 * will be managed. After setted, don't modity it.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container object
 * @param objs Chain of objects to pass focus
 * @ingroup Focus
 */
EAPI void
elm_object_focus_custom_chain_set(Evas_Object *obj,
                                  Eina_List   *objs)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_set(obj, objs);
}

/**
 * Unset custom focus chain
 *
 * @param obj The container object
 * @ingroup Focus
 */
EAPI void
elm_object_focus_custom_chain_unset(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_unset(obj);
}

/**
 * Get custom focus chain
 *
 * @param obj The container object
 * @ingroup Focus
 */
EAPI const Eina_List *
elm_object_focus_custom_chain_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focus_custom_chain_get(obj);
}

/**
 * Append object to custom focus chain.
 *
 * @note If relative_child equal to NULL or not in custom chain, the object
 * will be added in end.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container object
 * @param child The child to be added in custom chain
 * @param relative_child The relative object to position the child
 * @ingroup Focus
 */
EAPI void
elm_object_focus_custom_chain_append(Evas_Object *obj,
                                     Evas_Object *child,
                                     Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(child);
   elm_widget_focus_custom_chain_append(obj, child, relative_child);
}

/**
 * Prepend object to custom focus chain.
 *
 * @note If relative_child equal to NULL or not in custom chain, the object
 * will be added in begin.
 *
 * @note On focus cycle, only will be evaluated children of this container.
 *
 * @param obj The container object
 * @param child The child to be added in custom chain
 * @param relative_child The relative object to position the child
 * @ingroup Focus
 */
EAPI void
elm_object_focus_custom_chain_prepend(Evas_Object *obj,
                                      Evas_Object *child,
                                      Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(child);
   elm_widget_focus_custom_chain_prepend(obj, child, relative_child);
}

/**
 * Give focus to next object in object tree.
 *
 * Give focus to next object in focus chain of one object sub-tree.
 * If the last object of chain already have focus, the focus will go to the
 * first object of chain.
 *
 * @param obj The object root of sub-tree
 * @param dir Direction to cycle the focus
 *
 * @ingroup Focus
 */
EAPI void
elm_object_focus_cycle(Evas_Object        *obj,
                       Elm_Focus_Direction dir)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_cycle(obj, dir);
}

/**
 * Give focus to near object in one direction.
 *
 * Give focus to near object in direction of one object.
 * If none focusable object in given direction, the focus will not change.
 *
 * @param obj The reference object
 * @param x Horizontal component of direction to focus
 * @param y Vertical component of direction to focus
 *
 * @ingroup Focus
 */
EAPI void
elm_object_focus_direction_go(Evas_Object *obj,
                              int          x,
                              int          y)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_direction_go(obj, x, y);
}

/**
 * Get the enable status of the focus highlight
 *
 * This gets whether the highlight on focused objects is enabled or not
 * @ingroup Focus
 */
EAPI Eina_Bool
elm_focus_highlight_enabled_get(void)
{
   return _elm_config->focus_highlight_enable;
}

/**
 * Set the enable status of the focus highlight
 *
 * Set whether to show or not the highlight on focused objects
 * @param enable Enable highlight if EINA_TRUE, disable otherwise
 * @ingroup Focus
 */
EAPI void
elm_focus_highlight_enabled_set(Eina_Bool enable)
{
   _elm_config->focus_highlight_enable = !!enable;
}

/**
 * Get the enable status of the highlight animation
 *
 * Get whether the focus highlight, if enabled, will animate its switch from
 * one object to the next
 * @ingroup Focus
 */
EAPI Eina_Bool
elm_focus_highlight_animate_get(void)
{
   return _elm_config->focus_highlight_animate;
}

/**
 * Set the enable status of the highlight animation
 *
 * Set whether the focus highlight, if enabled, will animate its switch from
 * one object to the next
 * @param animate Enable animation if EINA_TRUE, disable otherwise
 * @ingroup Focus
 */
EAPI void
elm_focus_highlight_animate_set(Eina_Bool animate)
{
   _elm_config->focus_highlight_animate = !!animate;
}

/**
 * @defgroup Scrolling Scrolling
 *
 * These are functions setting how scrollable views in Elementary
 * widgets should behave on user interaction.
 */

/**
 * Get whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @return the thumb scroll bouncing state
 *
 * This is the default behavior for touch screens, in general.
 * @ingroup Scrolling
 */
EAPI Eina_Bool
elm_scroll_bounce_enabled_get(void)
{
   return _elm_config->thumbscroll_bounce_enable;
}

/**
 * Set whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @param enabled the thumb scroll bouncing state
 *
 * @see elm_thumbscroll_bounce_enabled_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bounce_enabled_set(Eina_Bool enabled)
{
   _elm_config->thumbscroll_bounce_enable = enabled;
}

/**
 * Set whether scrollers should bounce when they reach their
 * viewport's edge during a scroll, for all Elementary application
 * windows.
 *
 * @param enabled the thumb scroll bouncing state
 *
 * @see elm_thumbscroll_bounce_enabled_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bounce_enabled_all_set(Eina_Bool enabled)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int bounce_enable_i = (unsigned int)enabled;

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_BOUNCE_ENABLE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &bounce_enable_i, 1);
#endif
}

/**
 * Get the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @return the thumb scroll bounce friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_bounce_friction_get(void)
{
   return _elm_config->thumbscroll_bounce_friction;
}

/**
 * Set the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @param friction the thumb scroll bounce friction
 *
 * @see elm_thumbscroll_bounce_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bounce_friction_set(double friction)
{
   _elm_config->thumbscroll_bounce_friction = friction;
}

/**
 * Set the amount of inertia a scroller will impose at bounce
 * animations, for all Elementary application windows.
 *
 * @param friction the thumb scroll bounce friction
 *
 * @see elm_thumbscroll_bounce_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bounce_friction_all_set(double friction)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int bounce_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_BOUNCE_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &bounce_friction_i, 1);
#endif
}

/**
 * Get the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @return the page scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_page_scroll_friction_get(void)
{
   return _elm_config->page_scroll_friction;
}

/**
 * Set the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @param friction the page scroll friction
 *
 * @see elm_thumbscroll_page_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_page_scroll_friction_set(double friction)
{
   _elm_config->page_scroll_friction = friction;
}

/**
 * Set the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations, for all Elementary application windows.
 *
 * @param friction the page scroll friction
 *
 * @see elm_thumbscroll_page_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_page_scroll_friction_all_set(double friction)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int page_scroll_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_PAGE_SCROLL_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &page_scroll_friction_i, 1);
#endif
}

/**
 * Get the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @return the bring in scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_bring_in_scroll_friction_get(void)
{
   return _elm_config->bring_in_scroll_friction;
}

/**
 * Set the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @param friction the bring in scroll friction
 *
 * @see elm_thumbscroll_bring_in_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bring_in_scroll_friction_set(double friction)
{
   _elm_config->bring_in_scroll_friction = friction;
}

/**
 * Set the amount of inertia a scroller will impose at region bring
 * animations, for all Elementary application windows.
 *
 * @param friction the bring in scroll friction
 *
 * @see elm_thumbscroll_bring_in_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_bring_in_scroll_friction_all_set(double friction)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int bring_in_scroll_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom)
     atom =
       ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_BRING_IN_SCROLL_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &bring_in_scroll_friction_i, 1);
#endif
}

/**
 * Get the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @return the zoom friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_zoom_friction_get(void)
{
   return _elm_config->zoom_friction;
}

/**
 * Set the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @param friction the zoom friction
 *
 * @see elm_thumbscroll_zoom_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_zoom_friction_set(double friction)
{
   _elm_config->zoom_friction = friction;
}

/**
 * Set the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API, for all Elementary
 * application windows.
 *
 * @param friction the zoom friction
 *
 * @see elm_thumbscroll_zoom_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_zoom_friction_all_set(double friction)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int zoom_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_ZOOM_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &zoom_friction_i, 1);
#endif
}

/**
 * Get whether scrollers should be draggable from any point in their
 * views.
 *
 * @return the thumb scroll state
 *
 * @note This is the default behavior for touch screens, in general.
 * @note All other functions namespaced with "thumbscroll" will only
 *       have effect if this mode is enabled.
 *
 * @ingroup Scrolling
 */
EAPI Eina_Bool
elm_scroll_thumbscroll_enabled_get(void)
{
   return _elm_config->thumbscroll_enable;
}

/**
 * Set whether scrollers should be draggable from any point in their
 * views.
 *
 * @param enabled the thumb scroll state
 *
 * @see elm_thumbscroll_enabled_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_enabled_set(Eina_Bool enabled)
{
   _elm_config->thumbscroll_enable = enabled;
}

/**
 * Set whether scrollers should be draggable from any point in their
 * views, for all Elementary application windows.
 *
 * @param enabled the thumb scroll state
 *
 * @see elm_thumbscroll_enabled_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_enabled_all_set(Eina_Bool enabled)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int ts_enable_i = (unsigned int)enabled;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_ENABLE");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &ts_enable_i, 1);
#endif
}

/**
 * Get the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @return the thumb scroll threshould
 *
 * One would use higher values for touch screens, in general, because
 * of their inherent imprecision.
 * @ingroup Scrolling
 */
EAPI unsigned int
elm_scroll_thumbscroll_threshold_get(void)
{
   return _elm_config->thumbscroll_threshold;
}

/**
 * Set the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @param threshold the thumb scroll threshould
 *
 * @see elm_thumbscroll_threshould_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_threshold_set(unsigned int threshold)
{
   _elm_config->thumbscroll_threshold = threshold;
}

/**
 * Set the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling, for all Elementary
 * application windows.
 *
 * @param threshold the thumb scroll threshould
 *
 * @see elm_thumbscroll_threshould_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_threshold_all_set(unsigned int threshold)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int ts_threshold_i = (unsigned int)threshold;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_THRESHOLD");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &ts_threshold_i, 1);
#endif
}

/**
 * Get the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @return the thumb scroll momentum threshould
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_thumbscroll_momentum_threshold_get(void)
{
   return _elm_config->thumbscroll_momentum_threshold;
}

/**
 * Set the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @param threshold the thumb scroll momentum threshould
 *
 * @see elm_thumbscroll_momentum_threshould_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_momentum_threshold_set(double threshold)
{
   _elm_config->thumbscroll_momentum_threshold = threshold;
}

/**
 * Set the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second), for all Elementary application windows.
 *
 * @param threshold the thumb scroll momentum threshould
 *
 * @see elm_thumbscroll_momentum_threshould_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_momentum_threshold_all_set(double threshold)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int ts_momentum_threshold_i = (unsigned int)(threshold * 1000.0);

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_MOMENTUM_THRESHOLD");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &ts_momentum_threshold_i, 1);
#endif
}

/**
 * Get the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @return the thumb scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_thumbscroll_friction_get(void)
{
   return _elm_config->thumbscroll_friction;
}

/**
 * Set the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @param friction the thumb scroll friction
 *
 * @see elm_thumbscroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_friction_set(double friction)
{
   _elm_config->thumbscroll_friction = friction;
}

/**
 * Set the amount of inertia a scroller will impose at self scrolling
 * animations, for all Elementary application windows.
 *
 * @param friction the thumb scroll friction
 *
 * @see elm_thumbscroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_friction_all_set(double friction)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int ts_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &ts_friction_i, 1);
#endif
}

/**
 * Get the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @return the thumb scroll border friction
 *
 * @ingroup Scrolling
 */
EAPI double
elm_scroll_thumbscroll_border_friction_get(void)
{
   return _elm_config->thumbscroll_border_friction;
}

/**
 * Set the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @param friction the thumb scroll border friction. @c 0.0 for
 *        perfect synchrony between two movements, @c 1.0 for maximum
 *        lag.
 *
 * @see elm_thumbscroll_border_friction_get()
 * @note parameter value will get bound to 0.0 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_border_friction_set(double friction)
{
   if (friction < 0.0)
     friction = 0.0;

   if (friction > 1.0)
     friction = 1.0;

   _elm_config->thumbscroll_friction = friction;
}

/**
 * Set the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually, for all Elementary application windows.
 *
 * @param friction the thumb scroll border friction. @c 0.0 for
 *        perfect synchrony between two movements, @c 1.0 for maximum
 *        lag.
 *
 * @see elm_thumbscroll_border_friction_get()
 * @note parameter value will get bound to 0.0 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void
elm_scroll_thumbscroll_border_friction_all_set(double friction)
{
   if (friction < 0.0)
     friction = 0.0;

   if (friction > 1.0)
     friction = 1.0;

#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;
   unsigned int border_friction_i = (unsigned int)(friction * 1000.0);

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_THUMBSCROLL_BORDER_FRICTION");
   ecore_x_window_prop_card32_set(ecore_x_window_root_first_get(),
                                  atom, &border_friction_i, 1);
#endif
}

/**
 * @defgroup Scrollhints Scrollhints
 *
 * Objects when inside a scroller can scroll, but this may not always be
 * desirable in certain situations. This allows an object to hint to itself
 * and parents to "not scroll" in one of 2 ways.
 *
 * 1. To hold on scrolling. This means just flicking and dragging may no
 * longer scroll, but pressing/dragging near an edge of the scroller will
 * still scroll. This is automastically used by the entry object when
 * selecting text.
 * 2. To totally freeze scrolling. This means it stops. until popped/released.
 */

/**
 * Push the scroll hold by 1
 *
 * This increments the scroll hold count by one. If it is more than 0 it will
 * take effect on the parents of the indicated object.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_hold_push(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_hold_push(obj);
}

/**
 * Pop the scroll hold by 1
 *
 * This decrements the scroll hold count by one. If it is more than 0 it will
 * take effect on the parents of the indicated object.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_hold_pop(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_hold_pop(obj);
}

/**
 * Push the scroll freeze by 1
 *
 * This increments the scroll freeze count by one. If it is more than 0 it will
 * take effect on the parents of the indicated object.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_freeze_push(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_freeze_push(obj);
}

/**
 * Lock the scrolling of the given widget (and thus all parents)
 *
 * This locks the given object from scrolling in the X axis (and implicitly
 * also locks all parent scrollers too from doing the same).
 *
 * @param obj The object
 * @param lock The lock state (1 == locked, 0 == unlocked)
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_lock_x_set(Evas_Object *obj,
                             Eina_Bool    lock)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_drag_lock_x_set(obj, lock);
}

/**
 * Lock the scrolling of the given widget (and thus all parents)
 *
 * This locks the given object from scrolling in the Y axis (and implicitly
 * also locks all parent scrollers too from doing the same).
 *
 * @param obj The object
 * @param lock The lock state (1 == locked, 0 == unlocked)
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_lock_y_set(Evas_Object *obj,
                             Eina_Bool    lock)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_drag_lock_y_set(obj, lock);
}

/**
 * Get the scrolling lock of the given widget
 *
 * This gets the lock for X axis scrolling.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI Eina_Bool
elm_object_scroll_lock_x_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_drag_lock_x_get(obj);
}

/**
 * Get the scrolling lock of the given widget
 *
 * This gets the lock for X axis scrolling.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI Eina_Bool
elm_object_scroll_lock_y_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_drag_lock_y_get(obj);
}

/**
 * Pop the scroll freeze by 1
 *
 * This decrements the scroll freeze count by one. If it is more than 0 it will
 * take effect on the parents of the indicated object.
 *
 * @param obj The object
 * @ingroup Scrollhints
 */
EAPI void
elm_object_scroll_freeze_pop(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_freeze_pop(obj);
}

/**
 * @defgroup WidgetNavigation Widget Tree Navigation.
 *
 * How to check if an Evas Object is an Elementary widget? How to get
 * the first elementary widget that is parent of the given object?
 * These are all covered in widget tree navigation.
 */

/**
 * Check if the given Evas Object is an Elementary widget.
 *
 * @param obj the object to query.
 * @return @c EINA_TRUE if it is an elementary widget variant,
 *         @c EINA_FALSE otherwise
 * @ingroup WidgetNavigation
 */
EAPI Eina_Bool
elm_object_widget_check(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_is(obj);
}

/**
 * Get the first parent of the given object that is an Elementary widget.
 *
 * @param obj the object to query.
 * @return the parent object that is an Elementary widget, or @c NULL
 *         if no parent is, or no parents at all.
 * @ingroup WidgetNavigation
 */
EAPI Evas_Object *
elm_object_parent_widget_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_parent_widget_get(obj);
}

/**
 * Get the top level parent of an Elementary widget.
 *
 * @param obj The object to query.
 * @return The top level Elementary widget, or @c NULL if parent cannot be
 * found.
 * @ingroup WidgetNavigation
 */
EAPI Evas_Object *
elm_object_top_widget_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_top_get(obj);
}

/**
 * Get the string that represents this Elementary widget.
 *
 * @note Elementary is weird and exposes itself as a single
 *       Evas_Object_Smart_Class of type "elm_widget", so
 *       evas_object_type_get() always return that, making debug and
 *       language bindings hard. This function tries to mitigate this
 *       problem, but the solution is to change Elementary to use
 *       proper inheritance.
 *
 * @param obj the object to query.
 * @return Elementary widget name, or @c NULL if not a valid widget.
 * @ingroup WidgetNavigation
 */
EAPI const char *
elm_object_widget_type_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_type_get(obj);
}

/**
 * Send a signal to the widget edje object.
 *
 * This function sends a signal to the edje object of the obj. An edje program
 * can respond to a signal by specifying matching 'signal' and
 * 'source' fields.
 *
 * @param obj The object
 * @param emission The signal's name.
 * @param source The signal's source.
 * @ingroup General
 */
EAPI void
elm_object_signal_emit(Evas_Object *obj,
                       const char  *emission,
                       const char  *source)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_signal_emit(obj, emission, source);
}

/**
 * Add a callback for a signal emitted by widget edje object.
 *
 * This function connects a callback function to a signal emitted by the
 * edje object of the obj.
 * Globs can occur in either the emission or source name.
 *
 * @param obj The object
 * @param emission The signal's name.
 * @param source The signal's source.
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @param data A pointer to data to pass in to the callback function.
 * @ingroup General
 */
EAPI void 
elm_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
    EINA_SAFETY_ON_NULL_RETURN(obj);
    EINA_SAFETY_ON_NULL_RETURN(func);
    elm_widget_signal_callback_add(obj, emission, source, func, data);
}

/**
 * Remove a signal-triggered callback from an widget edje object.
 *
 * This function removes a callback, previoulsy attached to a signal emitted
 * by the edje object of the obj.
 * The parameters emission, source and func must match exactly those passed to
 * a previous call to elm_object_signal_callback_add(). The data pointer that
 * was passed to this call will be returned.
 *
 * @param obj The object
 * @param emission The signal's name.
 * @param source The signal's source.
 * @param func The callback function to be executed when the signal is
 * emitted.
 * @return The data pointer
 * @ingroup General
 */
EAPI void *
elm_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
    return elm_widget_signal_callback_del(obj, emission, source, func);
}

/**
 * Add a callback for a event emitted by widget or their children.
 *
 * This function connects a callback function to any key_down key_up event
 * emitted by the @p obj or their children.
 * This only will be called if no other callback has consumed the event.
 * If you want consume the event, and no other get it, func should return
 * EINA_TRUE and put EVAS_EVENT_FLAG_ON_HOLD in event_flags.
 *
 * @warning Accept duplicated callback addition.
 *
 * @param obj The object
 * @param func The callback function to be executed when the event is
 * emitted.
 * @param data Data to pass in to the callback function.
 * @ingroup General
 */
EAPI void
elm_object_event_callback_add(Evas_Object *obj, Elm_Event_Cb func, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   elm_widget_event_callback_add(obj, func, data);
}

/**
 * Remove a event callback from an widget.
 *
 * This function removes a callback, previoulsy attached to event emission
 * by the @p obj.
 * The parameters func and data must match exactly those passed to
 * a previous call to elm_object_event_callback_add(). The data pointer that
 * was passed to this call will be returned.
 *
 * @param obj The object
 * @param func The callback function to be executed when the event is
 * emitted.
 * @param data Data to pass in to the callback function.
 * @return The data pointer
 * @ingroup General
 */
EAPI void *
elm_object_event_callback_del(Evas_Object *obj, Elm_Event_Cb func, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
   return elm_widget_event_callback_del(obj, func, data);
}


/**
 * @defgroup Debug Debug
 */

/**
 * Print Tree object hierarchy in stdout
 *
 * @param obj The root object
 * @ingroup Debug
 */
EAPI void
elm_object_tree_dump(const Evas_Object *top)
{
#ifdef ELM_DEBUG
   elm_widget_tree_dump(top);
#else
   return;
   (void)top;
#endif
}

/**
 * Print Elm Objects tree hierarchy in file as dot(graphviz) syntax.
 *
 * @param obj The root object
 * @param file The path of output file
 * @ingroup Debug
 */
EAPI void
elm_object_tree_dot_dump(const Evas_Object *top,
                         const char        *file)
{
#ifdef ELM_DEBUG
   FILE *f = fopen(file, "w");
   elm_widget_tree_dot_dump(top, f);
   fclose(f);
#else
   return;
   (void)top;
   (void)file;
#endif
}

/**
 * Set the duration for occuring long press event.
 *
 * @param lonpress_timeout Timeout for long press event
 * @ingroup Longpress
 */
EAPI void
elm_longpress_timeout_set(double longpress_timeout)
{
   _elm_config->longpress_timeout = longpress_timeout;
}

/**
 * Get the duration for occuring long press event.
 *
 * @return Timeout for long press event
 * @ingroup Longpress
 */
EAPI double
elm_longpress_timeout_get(void)
{
   return _elm_config->longpress_timeout;
}
