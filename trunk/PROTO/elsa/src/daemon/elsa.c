#include "elsa.h"
#include <sys/types.h>
#include <unistd.h>
#include "Ecore_Getopt.h"
#include <xcb/xcb.h>

#define ELSA_DISPLAY ":0.0"


static Eina_Bool _open_log();
//static Eina_Bool _close_log();
static void _remove_lock();
static void _signal_cb();
static Eina_Bool _elsa_client_del(void *data, int type, void *event);

static unsigned char _testing = 0;
static Ecore_Exe *_elsa_client = NULL;


static void
_signal_cb(int sig)
{
   fprintf(stderr, PACKAGE": signal %d received\n", sig);
   elsa_session_shutdown();
   elsa_xserver_shutdown();
   exit(1);
}

static Eina_Bool
_get_lock()
{
   FILE *f;
   char buf[128];
   int my_pid;

   my_pid = getpid();
   f = fopen(elsa_config->lockfile, "r");
   if (!f)
     {
        /* No lockfile, so create one */
        f = fopen(elsa_config->lockfile, "w");
        if (!f)
          {
             fprintf(stderr, PACKAGE": Couldn't create lockfile %s!\n",
                     elsa_config->lockfile);
             return (EINA_FALSE);
          }
        snprintf(buf, sizeof(buf), "%d\n", my_pid);
        if (!fwrite(buf, strlen(buf), 1, f))
          {
             fclose(f);
             fprintf(stderr, PACKAGE": Couldn't write the lockfile\n");
             return EINA_FALSE;
          }
        fclose(f);
     }
   else
     {
        int pid = 0;
        /* read the lockfile */
        if (fgets(buf, sizeof(buf), f))
          pid = atoi(buf);
        fclose(f);
        if (pid == my_pid)
          return EINA_TRUE;
        fprintf(stderr, "A lock file are present another instance are present ?\n");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_update_lock()
{
   FILE *f;
   char buf[128];
   f = fopen(elsa_config->lockfile, "w");
   snprintf(buf, sizeof(buf), "%d\n", getpid());
   if (!fwrite(buf, strlen(buf), 1, f))
     fprintf(stderr, PACKAGE": Coudn't update lockfile\n");
   fclose(f);
}

static void
_remove_lock()
{
   remove(elsa_config->lockfile);
}

static Eina_Bool
_open_log()
{
   FILE *elog;
   if (_testing) return EINA_TRUE;
   elog = fopen(elsa_config->logfile, "a");
   if (!elog)
     {
        fprintf(stderr, PACKAGE": could not open logfile %s!!!\n",
                elsa_config->logfile);
        return EINA_FALSE;
     }
   fclose(elog);
   if (!freopen(elsa_config->logfile, "a", stdout))
     fprintf(stderr, PACKAGE": Error on reopen stdout\n");
   setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
   if (!freopen(elsa_config->logfile, "a", stderr))
     fprintf(stderr, PACKAGE": Error on reopen stderr\n");
   setvbuf(stderr, NULL, _IONBF, BUFSIZ);
   return EINA_TRUE;
}

void
elsa_close_log()
{
   if (!_testing)
   {
      fclose(stderr);
      fclose(stdout);
   }
}

static void
_elsa_wait(int pid, const char *display, const char *session_end)
{
   char buf[16]; /* I think is sufisant ... */
   snprintf(buf, sizeof(buf), "%d", pid);
   execl("/usr/sbin/elsa_wait", "/usr/sbin/elsa",
         buf, elsa_session_login_get(), display, session_end, NULL);
   fprintf(stderr, PACKAGE": HUM HUM HUM ...\n\n\n");
}

int
elsa_main()
{
   fprintf(stderr, PACKAGE": Run client\n");
   if (elsa_config->autologin)
     ecore_main_loop_quit();
   else
     ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                             _elsa_client_del, NULL);
     _elsa_client = ecore_exe_run("elsa_client -d ':0.0'", NULL);
   return 0;
}

static Eina_Bool
_elsa_client_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *ev;
   ev = event;
   if (ev->exe != _elsa_client)
     return ECORE_CALLBACK_PASS_ON;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_DONE;
}


static const Ecore_Getopt options =
{
   "elsa",
   "%prog [options]",
   VERSION,
   "(C) 2011 Enlightenment, see AUTHORS",
   "GPL, see COPYING",
   "Elsa is a login manager, written using core efl libraries",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_TRUE('n', "nodaemon", "Don't daemonize."),
      ECORE_GETOPT_STORE_TRUE('t', "test", "run in test mode."),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};

int
main (int argc, char ** argv)
{
   int args;
   int pid;
   char *dname = strdup(ELSA_DISPLAY);
   unsigned char nodaemon = 0;
   unsigned char quit_option = 0;
   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(nodaemon),
        ECORE_GETOPT_VALUE_BOOL(_testing),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_NONE
     };

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, PACKAGE": ERROR: could not parse options.\n");
        return -1;
     }

   if (quit_option)
     return 0;
   if(getuid() != 0 && !_testing)
     {
        fprintf(stderr, "Only root can run this program\n");
        return 1;
     }
   if (_testing)
     nodaemon = EINA_TRUE;

   elsa_config_init();
   if (!_testing && !_get_lock())
     {
        exit(1);
     }

   if (!nodaemon && elsa_config->daemonize)
     {
        if (daemon(0, 1) == -1)
          {
             fprintf(stderr, PACKAGE": Error on daemonize !");
             exit(1);
          }
        _update_lock();
     }

   if (!_open_log())
      exit(1);
   ecore_init();
   /* Initialise event handler */

   elsa_pam_init(PACKAGE, dname, NULL);
   elsa_session_init(elsa_config->command.xauth_file);

   pid = elsa_xserver_init(elsa_main, dname);
   signal(SIGQUIT, _signal_cb);
   signal(SIGTERM, _signal_cb);
   signal(SIGKILL, _signal_cb);
   signal(SIGINT, _signal_cb);
   signal(SIGHUP, _signal_cb);
   signal(SIGPIPE, _signal_cb);
   signal(SIGALRM, _signal_cb);
   if (elsa_config->autologin)
     {
        xcb_connection_t *disp = NULL;
        disp = xcb_connect(dname, NULL);
        ecore_main_loop_begin();
        elsa_pam_item_set(ELSA_PAM_ITEM_USER, elsa_config->userlogin);
        elsa_session_login(elsa_config->command.session_login);
        sleep(30);
        xcb_disconnect(disp);
     }
   else
     {
        elsa_server_init();
        ecore_main_loop_begin();
        elsa_server_shutdown();
     }
   elsa_xserver_shutdown();
   elsa_pam_shutdown();
   ecore_shutdown();
   _elsa_wait(pid, dname, elsa_config_shutdown());
   _remove_lock();
   elsa_close_log();
   return 0;
}

