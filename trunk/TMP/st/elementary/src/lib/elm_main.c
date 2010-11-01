#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <dlfcn.h>      /* dlopen,dlclose,etc */

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

static Eina_Bool _elm_signal_exit(void *data, int ev_type, void *ev);

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
_elm_signal_exit(void *data __UNUSED__, int ev_type __UNUSED__, void *ev __UNUSED__)
{
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}

void
_elm_rescale(void)
{
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale();
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
elm_init(int argc, char **argv)
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
elm_quicklaunch_init(int argc, char **argv)
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
elm_quicklaunch_sub_init(int argc, char **argv)
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
        _elm_module_init();
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
static int (*qr_main) (int argc, char **argv) = NULL;

EAPI Eina_Bool
elm_quicklaunch_prepare(int argc __UNUSED__, char **argv)
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
   free(exe);
   qr_main = dlsym(qr_handle, "elm_main");
   INF("dlsym(%p, 'elm_main') = %p", qr_handle, qr_main);
   if (!qr_main)
     {
	WRN("not quicklauncher capable: no elm_main in '%s'", exe);
	dlclose(qr_handle);
	qr_handle = NULL;
	return EINA_FALSE;
     }
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
elm_quicklaunch_fork(int argc, char **argv, char *cwd, void (postfork_func) (void *data), void *postfork_data)
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
elm_quicklaunch_fallback(int argc, char **argv)
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
	for (;;)
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
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on error (right
 *         now just invalid policy identifier, but in future policy
 *         value might be enforced).
 */
EAPI Eina_Bool
elm_policy_set(unsigned int policy, int value)
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
 * Flush all caches & dump all data that can be to lean down to use less memory
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
        evas_image_cache_flush(e);
        evas_font_cache_flush(e);
        evas_render_dump(e);
     }
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
elm_object_scale_set(Evas_Object *obj, double scale)
{
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
   return elm_widget_scale_get(obj);
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
elm_object_style_set(Evas_Object *obj, const char *style)
{
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
elm_object_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
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
   return elm_widget_disabled_get(obj);
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
elm_profile_dir_get(const char *profile, Eina_Bool is_user)
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
 * @return The profiles list.
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
 * @ingroup Scaling
 *
 */
EAPI void
elm_profile_set(const char *profile)
{
   if (!profile)
     return;
   _elm_config_profile_set(profile);
}

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to all Elementary
 * applications. All running Elementary windows will be affected.
 *
 * @param profile The profile's name
 * @ingroup Scaling
 *
 */
EAPI void
elm_profile_all_set(const char *profile)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;

   if (!atom)
     atom = ecore_x_atom_get("ENLIGHTENMENT_PROFILE");
   ecore_x_window_prop_string_set(ecore_x_window_root_first_get(),
                                  atom, profile);
#endif
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
 * Get the enable status of the focus highlight
 *
 * This gets whether the highlight on focused objects is enabled or not
 * @ingroup Config
 */
EAPI Eina_Bool
elm_focus_highlight_enable_get(void)
{
   return _elm_config->focus_highlight_enable;
}

/**
 * Set the enable status of the focus highlight
 *
 * Set whether to show or not the highlight on focused objects
 * @param enable Enable highlight if EINA_TRUE, disable otherwise
 * @ingroup Config
 */
EAPI void
elm_focus_highlight_enable_set(Eina_Bool enable)
{
   _elm_config->focus_highlight_enable = !!enable;
}

/**
 * Get the enable status of the highlight animation
 *
 * Get whether the focus highlight, if enabled, will animate its switch from
 * one object to the next
 * @ingroup Config
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
 * @ingroup Config
 */
EAPI void
elm_focus_highlight_animate_set(Eina_Bool animate)
{
   _elm_config->focus_highlight_animate = !!animate;
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
elm_coords_finger_size_adjust(int times_w, Evas_Coord *w, int times_h, Evas_Coord *h)
{
   if ((w) && (*w < (_elm_config->finger_size * times_w)))
     *w = _elm_config->finger_size * times_w;
   if ((h) && (*h < (_elm_config->finger_size * times_h)))
     *h = _elm_config->finger_size * times_h;
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
elm_object_focus_allow_set(Evas_Object *obj, Eina_Bool enable)
{
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
elm_object_focus_custom_chain_set(Evas_Object *obj, Eina_List *objs)
{
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
elm_object_focus_custom_chain_append(Evas_Object *obj, Evas_Object *child, Evas_Object *relative_child)
{
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
elm_object_focus_custom_chain_prepend(Evas_Object *obj, Evas_Object *child, Evas_Object *relative_child)
{
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
elm_object_focus_cycle(Evas_Object *obj, Elm_Focus_Direction dir)
{
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
 * @ingroup Widget
 */
EAPI void
elm_object_focus_direction_go(Evas_Object *obj, int x, int y)
{
   elm_widget_focus_direction_go(obj, x, y);
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
elm_object_scroll_lock_x_set(Evas_Object *obj, Eina_Bool lock)
{
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
elm_object_scroll_lock_y_set(Evas_Object *obj, Eina_Bool lock)
{
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
 */
EAPI Eina_Bool
elm_object_widget_check(const Evas_Object *obj)
{
   return elm_widget_is(obj);
}

/**
 * Get the first parent of the given object that is an Elementary widget.
 *
 * @param obj the object to query.
 * @return the parent object that is an Elementary widget, or @c NULL
 *         if no parent is, or no parents at all.
 */
EAPI Evas_Object *
elm_object_parent_widget_get(const Evas_Object *obj)
{
   return elm_widget_parent_widget_get(obj);
}

/**
 * Get the top level parent of an Elementary widget.
 *
 * @param obj The object to query.
 * @return The top level Elementary widget, or @c NULL if parent cannot be
 * found.
 */
EAPI Evas_Object *
elm_object_top_widget_get(const Evas_Object *obj)
{
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
 */
EAPI const char *
elm_object_widget_type_get(const Evas_Object *obj)
{
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
elm_object_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
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
    return elm_widget_signal_callback_del(obj, emission, source, func);
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
elm_object_tree_dot_dump(const Evas_Object *top, const char *file)
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
