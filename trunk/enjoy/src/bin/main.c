#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

#include "private.h"

#include <Ecore_Getopt.h>
#include <Ecore_File.h>
#include <stdlib.h>
#include "gettext.h"

int _log_domain = -1;
static App app;

static const Ecore_Getopt options = {
  PACKAGE_NAME,
  "%prog [options] [url]",
  PACKAGE_VERSION "Revision:" stringify(VREV),
  "(C) 2010 ProFUSION embedded systems",
  "LGPL-3",
  "Music player for mobiles and desktops.",
  EINA_TRUE,
  {
    ECORE_GETOPT_APPEND_METAVAR
    ('a', "add", "Add (recursively) directory to music library.",
     "DIRECTORY", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND_METAVAR
    ('d', "del", "Delete (recursively) directory from music library.",
     "DIRECTORY", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

EAPI int
elm_main(int argc, char **argv)
{
   int r = 0, args;
   Eina_Bool quit_option = EINA_FALSE;
   const char *home;
   char *s;

   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_LIST(app.add_dirs),
      ECORE_GETOPT_VALUE_LIST(app.del_dirs),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };

#if ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
   textdomain(GETTEXT_PACKAGE);
#endif

   _log_domain = eina_log_domain_register("enjoy", NULL);
   if (_log_domain < 0)
     {
        EINA_LOG_CRIT("could not create log domain 'enjoy'.");
        return -1;
     }

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        ERR("Could not parse command line options.");
        return -1;
     }

   if (quit_option)
     {
        DBG("Command lines option requires quit.");
        return 0;
     }

   elm_theme_extension_add(NULL, PACKAGE_DATA_DIR "/default.edj");
   elm_theme_overlay_add(NULL, PACKAGE_DATA_DIR "/default.edj");

   home = getenv("HOME");
   if (!home || !home[0])
     {
        CRITICAL("Could not get $HOME");
        r = -1;
        goto end;
     }

   snprintf(app.configdir, sizeof(app.configdir), "%s/.config/enjoy", home);
   if (!ecore_file_mkpath(app.configdir))
     {
        ERR("Could not create %s", app.configdir);
        r = -1;
        goto end;
     }

   app.win = win_new(&app);
   if (!app.win) goto end;

   elm_run();

   evas_object_del(app.win);

 end:
   EINA_LIST_FREE(app.add_dirs, s) free(s);
   EINA_LIST_FREE(app.del_dirs, s) free(s);

   eina_log_domain_unregister(_log_domain);
   _log_domain = -1;
   elm_shutdown();
   efreet_mime_shutdown();

   return r;
}

#endif
ELM_MAIN()
