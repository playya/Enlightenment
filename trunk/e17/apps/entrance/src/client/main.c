/**
@file main.c
@brief When entrance starts, and ui specific variables
*/
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <Edje.h>
#include <Esmart/Esmart_Text_Entry.h>
#include <Esmart/Esmart_Container.h>
#include "entrance.h"
#include "entrance_session.h"
#include "entrance_x_session.h"
#include "entrance_ipc.h"

#define WINW 800
#define WINH 600

static Entrance_Session *session = NULL;

static int
idler_before_cb(void *data)
{
   edje_thaw();
   return 1;
}

static int
idler_after_cb(void *data)
{
   edje_freeze();
   return 1;
}


/**
 * get the hostname of the machine, surrounded by the before and after
 * strings the config specifies
 * @return - a valid string for the hostname, Localhost on failure or
 * whatever the system provides
 */
static char *
get_my_hostname(void)
{
   char buf[255];               /* some standard somewhere limits hostname
                                   lengths to this */
   char *dot;
   char message[PATH_MAX];

   char *result = NULL;

   if (!(gethostname(buf, 255)))
   {
      /* Ensure that hostname is in short form */
      dot = strstr(buf, ".");
      if (dot)
         *dot = '\0';

      snprintf(message, PATH_MAX, "%s %s %s", session->config->before.string,
               buf, session->config->after.string);
   }
   else
      snprintf(message, PATH_MAX, "%s Localhost %s",
               session->config->before.string, session->config->after.string);
   result = strdup(message);
   return (result);
}

/**
 * what to do if we SIGINT(^c) it
 * @param data - no clue
 * @param ev_type - kill event ?
 * @param ev - event data
 * @return 1
 * Obviously I want to exit here.
 */
static int
exit_cb(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   return 1;
}

/**
 * what to do when we receive a window delete event
 * @param ee - the Ecore_Evas that received the event
 */
static void
window_del_cb(Ecore_Evas * ee)
{
   ecore_main_loop_quit();
}

/**
 * handle when the ecore_evas needs to be resized
 * @param ee - The Ecore_Evas we're resizing 
 */
static void
window_resize_cb(Ecore_Evas * ee)
{
   Evas_Object *o = NULL;

   if ((o = evas_object_name_find(ecore_evas_get(ee), "ui")))
   {
      int w, h;

      ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
      evas_object_resize(o, w, h);
   }
}

/**
 * swap key input focus between the password and user entries
 * @param o - the object we want to swap focus with
 * @param selecto - whether to focus on o, or the other entry
 */
static void
focus_swap(Evas_Object * o, int selecto)
{
   Evas_Object *oo = NULL;

   if (!strcmp(esmart_text_entry_edje_part_get(o), "EntrancePassEntry"))
   {
      if ((oo =
           evas_object_name_find(evas_object_evas_get(o),
                                 "EntranceUserEntry")))
      {
         esmart_text_entry_text_set(oo, "");
      }
      esmart_text_entry_text_set(o, "");
   }
   else if (!strcmp(esmart_text_entry_edje_part_get(o), "EntranceUserEntry"))
   {
      oo =
         evas_object_name_find(evas_object_evas_get(o), "EntrancePassEntry");
   }
   if (oo)
   {
      selecto ? evas_object_focus_set(oo, 0) : evas_object_focus_set(o, 0);
      selecto ? evas_object_focus_set(o, 1) : evas_object_focus_set(oo, 1);
   }
}

/**
 * when Enter is hit on the keyboard we end up here
 * @param data - The smart object that is this Entry
 * @param str - The string that was in the buffer when Enter was pressed
 */
static void
interp_return_key(void *data, const char *str)
{
   Evas_Object *o = NULL;
   Entrance_User *eu = NULL;

   o = (Evas_Object *) data;

   if (!strcmp(esmart_text_entry_edje_part_get(o), "EntranceUserEntry"))
   {
      if (!entrance_auth_set_user(session->auth, str))
      {
         edje_object_signal_emit(esmart_text_entry_edje_object_get(o),
                                 "EntranceUserAuth", "");
         if ((eu = evas_hash_find(session->config->users.hash, str)))
            entrance_session_user_set(session, eu);
         focus_swap(o, 0);
      }
      else
      {
         esmart_text_entry_text_set(o, "");
         entrance_session_user_reset(session);
         edje_object_signal_emit(esmart_text_entry_edje_object_get(o),
                                 "EntranceUserFail", "");
         focus_swap(o, 1);
      }
   }
   if (!strcmp(esmart_text_entry_edje_part_get(o), "EntrancePassEntry"))
   {
      if (session->auth->user && strlen(session->auth->user) > 0)
      {
         entrance_auth_set_pass(session->auth, str);
         if (!entrance_session_auth_user(session))
         {
            session->authed = 1;
            edje_object_signal_emit(esmart_text_entry_edje_object_get(o),
                                    "EntranceUserAuthSuccess", "");
         }
         else
         {
            entrance_session_user_reset(session);
            edje_object_signal_emit(esmart_text_entry_edje_object_get(o),
                                    "EntranceUserAuthFail", "");
            focus_swap(o, 0);
         }
      }
   }
}

/**
 * an edje signal emission 
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 */
static void
focus(void *data, Evas_Object * o, const char *emission, const char *source)
{
   Evas_Object *oo = NULL;      /* in nexus this looks in infinity ! */

   if ((oo = (Evas_Object *) data))
   {
      if (!strcmp(emission, "In"))
      {
         if (!evas_object_focus_get(oo))
         {
            evas_object_focus_set(oo, 1);
         }
      }
      else if (!strcmp(emission, "Out"))
      {
         if (evas_object_focus_get(oo))
         {
            evas_object_focus_set(oo, 1);
            evas_object_focus_set(oo, 0);
         }
      }
      else
         fprintf(stderr, "Unknown signal Emission(%s)", emission);
   }
}

/**
 * Set the "EntranceDate" part's text
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Attempt to set the Part named "EntranceDate" to the results of
 * localtime.  This way the interval is configurable via a program in
 * the theme and not statically bound to a value.  
 */
static void
set_date(void *data, Evas_Object * o, const char *emission,
         const char *source)
{
   if (edje_object_part_exists(o, "EntranceDate"))
   {
      struct tm *now;
      char buf[PATH_MAX];
      time_t _t = time(NULL);

      now = localtime(&_t);
      strftime(buf, PATH_MAX, session->config->date.string, now);
      edje_object_part_text_set(o, "EntranceDate", buf);
   }
}

/**
 * Set the "EntranceTime" part's text
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Attempt to set the Part named "EntranceTime" to the results of
 * localtime.  This way the interval is configurable via a program in
 * the theme and not statically bound to a value.  
 */
static void
set_time(void *data, Evas_Object * o, const char *emission,
         const char *source)
{
   if (edje_object_part_exists(o, "EntranceTime"))
   {
      struct tm *now;
      char buf[PATH_MAX];
      time_t _t = time(NULL);

      now = localtime(&_t);
      strftime(buf, PATH_MAX, session->config->time.string, now);
      edje_object_part_text_set(o, "EntranceTime", buf);
   }
}

/**
 * Executed when an EntranceAuthSuccessDone signal is emitted
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Ensure that the session is authed, and quit the main ecore_loop
 */
static void
done_cb(void *data, Evas_Object * o, const char *emission, const char *source)
{
   if (!session->authed)
      syslog(LOG_CRIT,
             "Theme attempted to launch session without finishing authentication. Please fix your theme.");
   else
   {
      /* 
       * Request cookie here and call ecore_main_loop_quit, after we
       * receive the cookie back from server
       */
   }
   entrance_session_setup_user_session(session);
}

/**
 * Executed when a Session is selected
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Attempt to set the Part named "EntranceTime" to the results of
 * localtime.  This way the interval is configurable via a program in
 * the theme and not statically bound to a value.  
 */
void
session_item_selected_cb(void *data, Evas_Object * o, const char *emission,
                         const char *source)
{
   if (session && data)
   {
      Entrance_X_Session *exs = (Entrance_X_Session *) data;

      entrance_session_x_session_set(session, exs);
   }
}

/**
 * Executed when a Session is selected
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Attempt to set the Part named "EntranceTime" to the results of
 * localtime.  This way the interval is configurable via a program in
 * the theme and not statically bound to a value.  
 */
void
user_selected_cb(void *data, Evas_Object * o, const char *emission,
                 const char *source)
{
   if (session && data)
   {
      entrance_session_user_set(session, (Entrance_User *) data);
   }
}

/**
 * Executed when a Session is unselected
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Set the current EntranceFace part back to nothing
 */
void
user_unselected_cb(void *data, Evas_Object * o, const char *emission,
                   const char *source)
{
   if (session && data)
   {
      entrance_session_user_reset(session);
/*      edje_object_signal_emit(o, "EntranceUserFail", "");*/
   }
}

/**
 * Executed when an EntranceSystemReboot signal is emitted
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 */
static void
reboot_cb(void *data, Evas_Object * o, const char *emission,
          const char *source)
{
   if ((session->config->reboot) && (!session->testing))
   {
      pid_t pid;

      entrance_session_free(session);
      session = NULL;
      switch (pid = fork())
      {
        case 0:
           if (execl
               ("/bin/sh", "/bin/sh", "-c", "/sbin/shutdown -r now", NULL))
           {
              syslog(LOG_CRIT,
                     "Reboot failed: Unable to execute /sbin/shutdown");
              exit(0);
           }
        case -1:
           syslog(LOG_CRIT,
                  "Reboot failed: could not fork to execute shutdown script");
           break;
        default:
           syslog(LOG_INFO, "The system is being rebooted");
           exit(EXITCODE);
      }
   }
   else if (session->testing)
   {
      syslog(LOG_INFO, "Reboot Unsupported in testing mode");
   }
}

/**
 * Executed when an EntranceSystemHalt signal is emitted
 * @param data - the data passed when the callback was added
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 */
static void
shutdown_cb(void *data, Evas_Object * o, const char *emission,
            const char *source)
{
   pid_t pid;

   if ((session->config->halt) && (!session->testing))
   {
      entrance_session_free(session);
      session = NULL;
      switch (pid = fork())
      {
        case 0:
           if (execl
               ("/bin/sh", "/bin/sh", "-c", "/sbin/shutdown -h now", NULL))
           {
              syslog(LOG_CRIT,
                     "Shutdown failed: Unable to execute /sbin/shutdown");
              exit(0);
           }
        case -1:
           syslog(LOG_CRIT,
                  "Shutdown failed: could not fork to execute shutdown script");
           break;
        default:
           syslog(LOG_INFO, "The system is being shut down");
           exit(EXITCODE);
      }
   }
   else if (session->testing)
   {
      syslog(LOG_INFO, "Shutdown Unsupported in testing mode");
   }
}

/**
 * Executed when an SessionDefaultSet signal is emitted
 * @param data - the Entrance_Session in context
 * @param o - the evas object(Edje) that created the signal
 * @param emission - the signal "type" that was emitted
 * @param source - the signal originated from this "part"
 * Save out the current Entrance_Session's EntranceSession as the user's new
 * default session to be executed when they log in.
 */
static void
_session_set(void *data, Evas_Object * o, const char *emission,
             const char *source)
{
   Entrance_Session *e = NULL;

   if ((e = (Entrance_Session *) data))
   {
      entrance_session_user_session_default_set(e);
   }
}

static void
_container_scroll(void *data, Evas_Object * o, const char *emission,
                  const char *source)
{
   double sx = 0.0, sy = 0.0;
   Evas_Object *container = NULL;

   if ((container = data))
   {
      double container_length = 0.0;

      container_length = esmart_container_elements_length_get(container);
      edje_object_part_drag_value_get(session->edje, source, &sx, &sy);
      switch (esmart_container_direction_get(container))
      {
        case CONTAINER_DIRECTION_HORIZONTAL:
           esmart_container_scroll_offset_set(container,
                                              (int) (sx * container_length));
           break;
        case CONTAINER_DIRECTION_VERTICAL:
           esmart_container_scroll_offset_set(container,
                                              (int) (sy * container_length));
           break;
        default:
           fprintf(stderr, "Unknown Container Orientation\n");
           break;
      }
   }
}

/**
 * print the "Help" associated with the app, shows cli args etc
 * @param argv the argv that was passed from the application
 */
static void
entrance_help(char **argv)
{
   printf("Entrance - The Enlightened Display Manager\n");
   printf("Usage: %s [OPTION]...\n\n", argv[0]);
   printf
      ("---------------------------------------------------------------------------\n");
   printf("  -c, --config=CONFIG          Specify a custom config file\n");
   printf
      ("  -d, --display=DISPLAY        Specify which display Entrance should use\n");
   printf("  -h, --help                   Display this help message\n");
   printf
      ("  -g, --geometry=WIDTHxHEIGHT  Specify the size of the Entrance window.\n");
   printf
      ("                               Use of this option disables fullscreen mode.\n");
   printf
      ("  -t, --theme=THEME            Specify the theme to load. You may specify\n");
   printf
      ("                               either the name of an installed theme, or an\n");
   printf
      ("                               arbitrary path to an eet file (use ./ for\n");
   printf("                               the current directory).\n");
   printf
      ("  -T, --test                   Enable testing mode. This will cause xterm\n");
   printf
      ("                               to be executed instead of the selected\n");
   printf
      ("                               session upon authentication, and uses a\n");
   printf
      ("                               geometry of 800x600 (-g overrides this)\n");
   printf
      ("===========================================================================\n\n");
   printf
      ("Note: To automatically launch an X server that will be managed, please use\n");
   printf
      ("      entranced instead of entrance. Entrance requires an existing X server\n");
   printf("      to run. Run entranced --help for more information.\n\n");
   exit(0);
}

/**
 * we handle this iteration outside of the theme, update date and time
 * @param data a pointer to the main edje in entrance
 * @return 1 so the ecore_timer keeps going and going and ...
 */
int
timer_cb(void *data)
{
   Evas_Object *o = NULL;

   if ((o = (Evas_Object *) data))
   {
      set_date(NULL, o, NULL, NULL);
      set_time(NULL, o, NULL, NULL);
   }
   return (1);
}

/**
 * main - where it all starts !
 * @param argc - the number of arguments entrance was called with
 * @param argv - the args entrance was called with 
 * <p>Entrance works like this:</p>
 * <ol>
 * <li> Init Ecore </li>
 * <li> Parse command line arguments </li>
 * <li> Create a New Entrance_Session(Parses config for you) </li>
 * <li> Init Ecore_X </li>
 * <li> Init Ecore_Evas </li>
 * <li> Init Edje </li>
 * <li> Detect Ecore_Evas type from config, software or gl</li>
 * <li> Set the cursor specified in the config </li>
 * <li> Add key modifiers, setup caches and paths </li>
 * <li> Load theme specified in config, or from cli(cli overrides
 * config)</li>
 * <li> Swallow the username and password entries into the edje </li>
 * <li> Detect theme part presence, swallow/setup as appropriate </li>
 * <li> Setup signal callbacks that our main edje might emit </li>
 * <li> Show the main edje </li>
 * <li> Emit an "In" signal on the main entry for lazy themers </li>
 * <li> Tell the Entrance_Sesssion that the Ecore_Evas belongs to it</li>
 * <li> Run.............. until ecore_main_(loop_quit is called</li>
 * <li> If the user is authenticated, try to run their session</li>
 * <li>Shut down edje, ecore_evas, ecore_x, ecore</li>
 * </ol>
 */
int
main(int argc, char *argv[])
{
   int i = 0;
   char buf[PATH_MAX];
   char *str = NULL;
   char *display = NULL;
   Ecore_X_Window ew;
   Evas *evas = NULL;
   Ecore_Evas *e = NULL;
   Ecore_Timer *timer = NULL;
   Evas_Object *o = NULL, *edje = NULL;
   Evas_Coord x, y, w, h;
   char *entries[] = { "EntranceUserEntry", "EntrancePassEntry" };
   int entries_count = 2;
   const char *container_orientation = NULL;
   int c;
   struct option d_opt[] = {
      {"help", 0, 0, 'h'},
      {"display", 1, 0, 'd'},
      {"geometry", 1, 0, 'g'},
      {"theme", 1, 0, 't'},
      {"test", 0, 0, 'T'},
      {"config", 1, 0, 'c'},
      {0, 0, 0, 0}
   };
   int g_x = WINW, g_y = WINH;
   char *theme = NULL;
   char *config = NULL;
   int fullscreen = 1;
   pid_t server_pid = 0;
   int testing = 0;

   /* Basic ecore initialization */
   if (!ecore_init())
      return (-1);
   ecore_app_args_set(argc, (const char **) argv);

   /* Parse command-line options */
   while (1)
   {
      char *t;

      c = getopt_long(argc, argv, "hd:g:t:Tc:z:", d_opt, NULL);
      if (c == -1)
         break;
      switch (c)
      {
        case 'h':
           entrance_help(argv);
        case 'd':
           display = strdup(optarg);
           break;
        case 'g':
           t = strchr((const char *) optarg, 'x');
           if (!t || t >= (optarg + strlen(optarg)))
           {
              syslog(LOG_CRIT,
                     "Invalid argument '%s' given for geometry. Exiting.",
                     optarg);
              return (-1);
           }
           else
           {
              g_x = atoi((const char *) optarg);
              g_y = atoi((const char *) (t + 1));
              if (!g_x || !g_y)
              {
                 syslog(LOG_CRIT,
                        "Invalid argument '%s' given for geometry. Exiting.",
                        optarg);
                 return (-1);
              }
              fullscreen = 0;
           }
           break;
        case 't':
           /* Allow arbitrary paths to theme files */
           t = strchr((const char *) optarg, '/');
           if (t)
              theme = strdup(optarg);
           else
           {
              theme = calloc(1, PATH_MAX);
              t = strrchr((const char *) optarg, '.');
              if (t && !strcmp(t, ".eet"))
                 snprintf(theme, PATH_MAX, "%s/themes/%s", PACKAGE_DATA_DIR,
                          optarg);
              else
                 snprintf(theme, PATH_MAX, "%s/themes/%s.eet",
                          PACKAGE_DATA_DIR, optarg);
           }
           break;
        case 'T':
           testing = 1;
           fullscreen = 0;
           break;
        case 'c':
           config = strdup(optarg);
           break;
        case 'z':
           printf("entrance: main: z optarg = %s\n", optarg);
           server_pid = (pid_t) atoi(optarg);
           break;
        default:
           entrance_help(argv);
      }
   }

   if (!testing)
      if (!entrance_ipc_init(server_pid))
         return -1;

   session = entrance_session_new(config, display, testing);

   if (config)
      free(config);

#if 0
   printf("entrance: main: XAUTHORITY = %s\n", getenv("XAUTHORITY"));
#endif

#if 1
   if (!ecore_x_init(display))
   {
      if (display)
         syslog(LOG_CRIT,
                "Cannot initialize requested display \"%s\". Exiting.",
                display);
      else if ((str = getenv("DISPLAY")))
         syslog(LOG_CRIT,
                "Cannot initialize default display \"%s\". Exiting.", str);
      else
         syslog(LOG_CRIT, "No DISPLAY variable set! Exiting.");
      return (-1);
   }
#endif
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_cb, NULL);
   ecore_idle_enterer_add(idler_before_cb, NULL);

   if (ecore_evas_init())
   {
      edje_init();
      edje_freeze();
      edje_frametime_set(1.0 / 30.0);

      /* setup our ecore_evas */
      /* testing mode decides entrance window size * * Use rendering engine
         specified in config. On systems with * hardware acceleration, GL
         should improve performance appreciably */
      if (!session->config->engine)
         e = ecore_evas_software_x11_new(NULL, 0, 0, 0, g_x, g_y);
#ifdef HAVE_ECORE_GL_X11
      else if (session->config->engine)
         e = ecore_evas_gl_x11_new(NULL, 0, 0, 0, g_x, g_y);
#endif
      else
      {
         fprintf(stderr,
                 "Warning: Invalid Evas engine specified in config. Defaulting to software engine.\n");
         e = ecore_evas_software_x11_new(NULL, 0, 0, 0, g_x, g_y);
      }

      ew = ecore_evas_software_x11_window_get(e);
      if (session->testing)
         ecore_evas_title_set(e, "Entrance - Testing Mode");
      else
         ecore_evas_title_set(e, "Entrance");
      ecore_evas_callback_delete_request_set(e, window_del_cb);
      ecore_evas_callback_resize_set(e, window_resize_cb);
      ecore_evas_cursor_set(e, session->config->pointer, 12, 0, 0);
      ecore_evas_move(e, 0, 0);

      /* Evas specific callbacks */
      evas = ecore_evas_get(e);
      evas_image_cache_set(evas, 8 * 1024 * 1024);
      evas_font_cache_set(evas, 1 * 1024 * 1024);
      evas_font_path_append(evas, PACKAGE_DATA_DIR "/fonts");
      evas_font_path_append(evas, PACKAGE_DATA_DIR "/data/fonts");
      evas_key_modifier_add(evas, "Control_L");
      evas_key_modifier_add(evas, "Control_R");
      evas_key_modifier_add(evas, "Shift_L");
      evas_key_modifier_add(evas, "Shift_R");
      evas_key_modifier_add(evas, "Alt_L");
      evas_key_modifier_add(evas, "Alt_R");

      /* Load our theme as an edje */
      edje = edje_object_add(evas);
      if (!theme)
         snprintf(buf, PATH_MAX, "%s/themes/%s", PACKAGE_DATA_DIR,
                  session->config->theme);
      else
      {
         snprintf(buf, PATH_MAX, "%s", theme);
         if (session->config->theme)
            free(session->config->theme);
         session->config->theme = strdup(buf);
      }
      if (!edje_object_file_set(edje, buf, "Main"))
      {
         syslog(LOG_CRIT, "Failed to load theme %s\n", buf);
         entrance_session_free(session);
         exit(1);
      }
      evas_object_move(edje, 0, 0);
      evas_object_resize(edje, g_x, g_y);
      evas_object_name_set(edje, "ui");
      evas_object_layer_set(edje, 0);
      entrance_session_edje_object_set(session, edje);

      /* Setup the entries */
      for (i = 0; i < entries_count; i++)
      {
         if (edje_object_part_exists(edje, entries[i]))
         {
            edje_object_part_geometry_get(edje, entries[i], &x, &y, &w, &h);
            o = esmart_text_entry_new(evas);
            evas_object_move(o, x, y);
            evas_object_resize(o, w, h);
            evas_object_layer_set(o, 1);
            esmart_text_entry_max_chars_set(o, 32);
            esmart_text_entry_is_password_set(o, i);
            evas_object_name_set(o, entries[i]);
            esmart_text_entry_edje_part_set(o, edje, entries[i]);

            esmart_text_entry_return_key_callback_set(o, interp_return_key,
                                                      o);

            edje_object_signal_callback_add(edje, "In", entries[i], focus, o);

            edje_object_signal_callback_add(edje, "Out", entries[i], focus,
                                            o);
            edje_object_part_swallow(edje, entries[i], o);
            evas_object_show(o);
         }
         o = NULL;
      }

      /* See if we have a EntranceHostname part, set it */
      if (edje_object_part_exists(edje, "EntranceHostname"))
      {
         if ((str = get_my_hostname()))
         {
            edje_object_part_text_set(edje, "EntranceHostname", str);
            free(str);
         }
      }
      /* See if we have an EntranceTime part, setup a timer to automatically
         update the Time */
      if (edje_object_part_exists(edje, "EntranceTime"))
      {
         edje_object_signal_callback_add(edje, "Go", "EntranceTime", set_time,
                                         o);
         edje_object_signal_emit(edje, "Go", "EntranceTime");
         timer = ecore_timer_add(0.5, timer_cb, edje);
      }
      /* See if we have an EntranceDate part, setup a timer if one isn't
         already running to automatically update the Date */
      if (edje_object_part_exists(edje, "EntranceDate"))
      {
         edje_object_signal_callback_add(edje, "Go", "EntranceDate", set_date,
                                         o);
         edje_object_signal_emit(edje, "Go", "EntranceDate");
         if (!timer)
            timer = ecore_timer_add(0.5, timer_cb, edje);
      }
      /* See if we have an EntranceSession part, set it to the first element
         in the config's session list */
      if (edje_object_part_exists(edje, "EntranceSession"))
      {
         entrance_session_x_session_set(session,
                                        entrance_session_x_session_default_get
                                        (session));
      }
      /* See if we have an EntranceSessionList part, tell the session to load 
         the session list if it exists. */
      if (edje_object_part_exists(edje, "EntranceSessionList"))
      {
         entrance_session_xsession_list_add(session);
         if ((container_orientation =
              edje_object_data_get(edje,
                                   "entrance.xsessions.list.orientation")))
         {
            entrance_session_list_direction_set(session,
                                                session->session_container,
                                                container_orientation);
         }
         edje_object_signal_callback_add(edje, "drag",
                                         "entrance.xsessions.list.scroller",
                                         _container_scroll,
                                         session->session_container);
      }
      /* See if we have an EntranceUserList part, tell the session to load
         the user list if it exists. */
      if (edje_object_part_exists(edje, "EntranceUserList"))
      {
         entrance_session_user_list_add(session);
         if ((container_orientation =
              edje_object_data_get(edje, "entrance.users.list.orientation")))
         {
            entrance_session_list_direction_set(session,
                                                session->user_container,
                                                container_orientation);
         }
         edje_object_signal_callback_add(edje, "drag",
                                         "entrance.users.list.scroller",
                                         _container_scroll,
                                         session->user_container);
      }

      /**
       * Setup Edje callbacks for signal emissions from our main edje
       * It's useful to delay showing of your edje till all your
       * callbacks have been added, otherwise show might not trigger all
       * the desired events 
       */
      edje_object_signal_callback_add(edje, "EntranceUserAuthSuccessDone", "",
                                      done_cb, e);
      edje_object_signal_callback_add(edje, "EntranceSystemReboot", "",
                                      reboot_cb, e);
      edje_object_signal_callback_add(edje, "EntranceSystemHalt", "",
                                      shutdown_cb, e);
      edje_object_signal_callback_add(edje, "SessionDefaultSet", "",
                                      _session_set, session);
      evas_object_show(edje);
      /* set focus to user input by default */
      edje_object_signal_emit(edje, "In", "EntranceUserEntry");

      if (fullscreen)
         ecore_evas_fullscreen_set(e, 1);
      else
         ecore_evas_resize(e, g_x, g_y);
      ecore_idle_enterer_add(idler_after_cb, NULL);

      entrance_session_ecore_evas_set(session, e);
      entrance_ipc_session_set(session);
      entrance_session_run(session);

      if (session->authed)
      {
         entrance_session_start_user_session(session);
         entrance_session_free(session);
      }
      else
      {
         entrance_session_free(session);
         ecore_evas_shutdown();
      }
      if (!testing)
         entrance_ipc_shutdown();
      edje_shutdown();
      ecore_x_shutdown();
      ecore_shutdown();
   }
   else
   {
      fprintf(stderr, "Fatal error: Could not initialize ecore_evas!\n");
      exit(1);
   }

   return (0);
}
