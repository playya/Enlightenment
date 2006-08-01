#include <Ecore.h>
#include <Ecore_Config.h>
#include "Entranced.h"
#include "Entranced_Display.h"
#include "auth.h"
#include "util.h"
#include "ipc.h"

/* Globals */
/* Entranced_Display *d; */
static Ecore_Event_Handler *_e_handler = NULL;
static Ecore_Event_Handler *_d_handler = NULL;
static Ecore_Event_Filter *_e_filter = NULL;

static struct sigaction _entrance_d_sa;

static unsigned char is_respawning = 0;
static unsigned char exev = 0;
static Ecore_Timer *respawn_timer = NULL;

/**
 * Write the entranced pid to the defined pidfile.
 * @param pid The spawner's process ID, which is the pid after the fork if there was one
 * @return 0 if the operation was successful, 1 otherwise.
 */
int
Entranced_Write_Pidfile(pid_t pid)
{
   FILE *fp;
   int size, result = 1;
   char buf[PATH_MAX];

   size = snprintf(buf, PATH_MAX, "%d", pid);
   if ((fp = fopen(PIDFILE, "w+")))
   {
      fwrite(buf, sizeof(char), size, fp);
      fclose(fp);
      result = 0;
   }

   return result;
}

/**
 * Make entranced a daemon by fork-and-exit. This is the default behavior.
 */
void
Entranced_Fork_And_Exit(void)
{
   pid_t entranced_pid;

   switch (entranced_pid = fork())
   {
     case 0:
        break;
     default:
        if (Entranced_Write_Pidfile(entranced_pid))
        {
           syslog(LOG_CRIT, "%d is the pid, but I couldn't write to %s.",
                  entranced_pid, PIDFILE);
           kill(entranced_pid, SIGKILL);
           exit(1);
        }
        exit(0);
   }
}



int
Entranced_Respawn_Reset(void *data)
{
   entranced_debug("Respawn timer reset.\n");
   is_respawning = 0;
   ecore_timer_del(respawn_timer);
   respawn_timer = NULL;
   return 0;
}


/* Event Filters */
void *
Entranced_Filter_Start(void *data)
{
   return &exev;
}

int
Entranced_Filter_Loop(void *data, void *loop_data, int type, void *event)
{

   /* Filter out redundant exit events */
   if (type == ECORE_EXE_EVENT_DEL)
   {
      if (exev)
         return 0;
      else
         exev = 1;
   }

   return 1;
}

void
Entranced_Filter_End(void *data, void *loop_data)
{
   exev = 0;
}

/* Event handlers */

/*int _Entranced_SIGUSR(void *data, int type, void *event) {*/
static void
_Entranced_SIGUSR(int sig)
{
/*    Ecore_Event_Signal_User *e = (Ecore_Event_Signal_User *) event; */

   entranced_debug("SIGUSR event triggered.\n");

   /* X sends SIGUSR1 to let us know it is ready */
/*    if (e->number == 1)*/
/*   x_ready = 1; this becomes below */
   Entranced_Display_XReady_Set(1);
/*    return 1; */
}

int
Entranced_Exe_Exited(void *data, int type, void *event)
{
   Ecore_Exe_Event_Del *e = (Ecore_Exe_Event_Del *) event;
   Entranced_Display *d = (Entranced_Display *) data;

   entranced_debug("Ecore_Exe_Event_Del triggered.\n");

   if (is_respawning)
   {
      entranced_debug("Event ignored.\n");
      return 1;
   }
   else
   {
      entranced_debug("Processing Event.\n");
   }

   is_respawning = 1;
   respawn_timer = ecore_timer_add(1.0, Entranced_Respawn_Reset, d);

   if (e->exe == d->e_exe || e->pid == ecore_exe_pid_get(d->e_exe))
   {
      /* Entrance GUI failed to initialize correctly */
      if (!d->client.connected)
      {
         syslog(LOG_CRIT, "Entrance GUI initialization failure. Aborting.");
         fprintf(stderr, "Entrance has detected that the GUI is failing to launch properly.\n");
         fprintf(stderr, "Please check your installation. Aborting.\n\n");
         ecore_main_loop_quit(); 
      }

      /* Session exited or crashed */
      if (e->exited)
      {
         syslog(LOG_INFO, "The session has ended normally.");
         if (e->exit_code == EXITCODE)
         {
            ecore_main_loop_quit();
            return 0;
         }

      }
      else if (e->signalled)
         syslog(LOG_INFO, "The session was terminated with signal %d.",
                e->exit_signal);

      kill(d->pid, SIGHUP);
      sleep(3);
      if (waitpid(d->pid, NULL, WNOHANG) > 0)
      {
         syslog(LOG_INFO, "The X Server apparently died as well.");
         if (!Entranced_Display_X_Restart(d))
            exit(1);
      }

   }
   else if (e->pid == d->pid)
   {
      /* X terminated for some reason */
      if (e->exited)
         syslog(LOG_INFO, "The X Server terminated for some reason.");
      else if (e->signalled)
         syslog(LOG_INFO, "The X server was terminated with signal %d.",
                e->exit_signal);

      sleep(2);
      kill(d->pid, SIGKILL);
      if (!Entranced_Display_X_Restart(d))
         exit(1);

   }
   else 
   {
      return 1;
   }

   d->client.connected = 0;
   entranced_auth_user_remove(d);
   Entranced_Display_Spawn_Entrance(d);

   return 1;
}

int
Entranced_Signal_Exit(void *data, int type, void *event)
{
   entranced_debug("Ecore_Signal_Exit_Triggered\n");
   syslog(LOG_INFO, "Caught exit signal.");
   syslog(LOG_INFO, "Display and display manager are shutting down.");
   ecore_main_loop_quit();
   return 0;
}

void
Entranced_AtExit(void)
{
   entranced_debug("Entranced exits.\n");
}



void usage(char* name)
{
   /* This should probably in a separate usage function, but bleh */
   printf("Entranced - Launcher for the Entrance Display Manager\n");
   printf("Usage: %s [OPTION] ...\n\n", name);
   printf
	  ("--------------------------------------------------------------------------\n");
   printf("  -c CONFIG          Specify config file for greeter\n");
   printf("  -d DISPLAY         Connect to an existing X server\n");
   printf("  -help              Display this help message\n");
   /*printf("  -verbose           Display extra debugging info\n");*/
   printf
	  ("  -nodaemon          Don't fork to background (useful for init scripts)\n");
   printf
	  ("==========================================================================\n\n");
   printf
	  ("Note: if you're launching Entrance from within an existing X session, don't\n");
   printf
	  ("try to use entranced or you may get unexpected results. Instead, launch\n");
   printf("entrance directly by typing \"entrance\".\n\n");
   exit(0);
}

/*
 * Main function
 */

int
main(int argc, char **argv)
{
   int c;
   int nodaemon = 0;            /* TODO: Config-ize this variable */
   Entranced_Display *d;
   char *str = NULL;
   struct option d_opt[] = {
      {"config", 1, 0, 'c'},
      {"display", 1, 0, 'd'},
      {"nodaemon", 0, 0, 'n'},
      {"help", 0, 0, 'h'},
      {"disable-xauth", 0, 0, 'a'},
      {"verbose", 0, 0, 'v'},
      {0, 0, 0, 0}
   };
   pid_t entranced_pid = getpid();

   /* Initialize Ecore */
   ecore_init();
   if (ecore_config_init("entrance") != ECORE_CONFIG_ERR_SUCC)
   {
      ecore_shutdown();
      return -1;
   }
   ecore_app_args_set(argc, (const char **) argv);

   openlog("entranced", LOG_NOWAIT, LOG_DAEMON);

   /* Set up a spawner context */
   d = Entranced_Display_New();
   entranced_ipc_display_set(d);

   /* Parse command-line options */
   while (1)
   {
      c = getopt_long_only(argc, argv, "c:d:nhv", d_opt, NULL);
      if (c == -1)
         break;
      switch (c)
      {
        case 'c':
           d->config = strdup(optarg);
           break;
        case 'd':
           d->name = strdup(optarg);
           break;
        case 'a':
           d->auth_en = 0;
           break;
        case 'n':
           nodaemon = 1;
           break;
        case 'h':
			usage(argv[0]);
        /*case 'v':
           config.debuglevel = 1;*/

      }
   }

   if (!d->name)
      d->name = strdup(X_DISP);
   
   str = strstr(d->name, ":");

   if(!str || str >= (d->name + strlen(d->name) - 1))
      d->dispnum = 0;
   else
      d->dispnum = atoi(str + 1);

   entranced_debug("entranced: main: display number is %d\n", d->dispnum);


   entranced_pid = getpid();
   if (nodaemon)
   {
      if (Entranced_Write_Pidfile(entranced_pid))
      {
         syslog(LOG_CRIT, "%d is the pid, but I couldn't write to %s.",
                entranced_pid, PIDFILE);
         exit(1);
      }
   }
   else
   {
      Entranced_Fork_And_Exit();
   }

   /* Check to make sure entrance binary is executable */
   if (access(ENTRANCE, X_OK))
   {
      syslog(LOG_CRIT,
             "Fatal Error: Unable to launch entrance binary. Aborting.");
      exit(1);
   }

   /* Init IPC */
   if (!entranced_ipc_init(getpid()))
      exit(1);

   /* Daemonize */
   if (!nodaemon)
   {
      /* This causes socket communication issues, yet unidentified */
      /*
      close(0);
      close(1);
      close(2);
      */
      freopen("/dev/null", "r", stdin);
      freopen("/dev/null", "w", stdout);
      freopen("/dev/null", "w", stderr);
   }

   /* Event filter */
   _e_filter =
      ecore_event_filter_add(Entranced_Filter_Start, Entranced_Filter_Loop,
                             Entranced_Filter_End, NULL);

   /* Set up event handlers */
   _e_handler =
      ecore_event_handler_add(ECORE_EXE_EVENT_DEL, Entranced_Exe_Exited, d);
   _d_handler =
      ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, Entranced_Signal_Exit,
                              NULL);
/*    _sigusr1_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER, _Entranced_SIGUSR, NULL); */

   /* Manually add signal handler for SIGUSR1 */
   _entrance_d_sa.sa_handler = _Entranced_SIGUSR;
   _entrance_d_sa.sa_flags = SA_RESTART;
   sigemptyset(&_entrance_d_sa.sa_mask);
   sigaction(SIGUSR1, &_entrance_d_sa, NULL);

   /* Launch X Server */
   syslog(LOG_INFO, "Starting X server.");
   Entranced_Display_Spawn_X(d);

   if (d->status == NOT_RUNNING)
   {
      free(d);
      syslog(LOG_CRIT, "Could not start X server.");
      fprintf(stderr, "Entrance could not start the X server. Please check your config.\n");
      exit(1);
   }

   /* Run Entrance */
   syslog(LOG_INFO, "Starting Entrance.");
   Entranced_Display_Spawn_Entrance(d);

   /* Main program loop */
   entranced_debug("Entering main loop.\n");
   ecore_main_loop_begin();

   /* Shut down */
   entranced_debug("Exited main loop! Shutting down...\n");
   if (d->e_exe)
      ecore_exe_terminate(d->e_exe);
   kill(d->pid, SIGTERM);
   sleep(5);
   /* Die harder */
   if (d->e_exe)
      ecore_exe_kill(d->e_exe);
   kill(d->pid, SIGKILL);

   entranced_auth_user_remove(d);

   if (d->authfile)
      unlink(d->authfile);

   closelog();
   entranced_ipc_shutdown();
   ecore_config_shutdown();
   ecore_shutdown();
   exit(0);
}
