#include "spawner.h"

/* funcs */
Spawner_Display *spawner_display_new(void);
static void spawn_entrance(void);
static void spawn_x(void);
static int start_server_once(Spawner_Display * d);
void entrance_exit(int signum);
void x_exit(int signum);
double get_time(void);

/* globals */
Spawner_Display *d;

/**
 * write_entranced_pidfile - write the entranced pid to the specified pidfile
 * @pid - the pid_t variable received from the first fork called
 */
int
write_entranced_pidfile(pid_t pid)
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
   return (result);
}

/**
 * fork_and_exit - initial call in entranced, fork and exit for ease of use
 * To make this useable from startup scripts it's currently setup like this.
 * If this is incorrect or whatever please feel free to let me know.
 */
void
fork_and_exit(void)
{
   pid_t entranced_pid;

   switch (entranced_pid = fork())
   {
     case 0:
        break;
     default:
        if (write_entranced_pidfile(entranced_pid))
        {
           fprintf(stderr, "%d is the pid, but I couldn't write to %s\n",
                   entranced_pid, PIDFILE);
           kill(entranced_pid, SIGKILL);
           exit(1);
        }

        exit(0);
   }
}

static int
x_error_handler_ignore(Display * d, XErrorEvent * e)
{
   /* do nothing */
   return 0;
}

/**
 * x_server_killall - Kill all X clients
 *
 * This function will attempt to reset the X server by killing
 * all client windows, prior to respawning the entrance client.
 */
static void
x_server_killall(void)
{
   int screens, i, j;

   if (!d || !(d->display))
      return;

   /* Don't want entranced barfing over a BadWindow error or sth */
   XSetErrorHandler(x_error_handler_ignore);

   XGrabServer(d->display);
   screens = ScreenCount(d->display);

   /* Traverse window tree starting from root, drag them * all before the
      firing squad */
   for (i = 0; i < screens; ++i)
   {
      Window root_r;
      Window parent_r;
      Window *children_r = NULL;
      int num_children = 0;
      Window root = RootWindow(d->display, i);

      while (XQueryTree
             (d->display, root, &root_r, &parent_r, &children_r,
              &num_children) && num_children > 0)
      {

         for (j = 0; j < num_children; ++j)
         {
            XKillClient(d->display, children_r[j]);
         }

         XFree(children_r);
      }
   }

   XUngrabServer(d->display);
   XSync(d->display, False);
}


/**
 * main - startup the entranced process
 * @argc - not used
 * @argv - not used
 * Entranced starts off by forking off a child process, writing the child's
 * pid to a pidfile, and returning.  The forked child begins a new X session
 * and then starts the entrance process.
 */
int
main(int argc, char **argv)
{
   int c;
   int nodaemon = 0;
   struct option d_opt[] = {
      {"nodaemon", 0, 0, 1},
      {"help", 0, 0, 2},
      {0, 0, 0, 0}
   };
   pid_t entranced_pid = getpid();

   putenv("DISPLAY");
   /* get command line arguments */
   while (1)
   {
      c = getopt_long_only(argc, argv, "d:", d_opt, NULL);
      if (c == -1)
         break;
      switch (c)
      {
        case 'd':              /* display */
           setenv("DISPLAY", optarg, 1);
           break;
        case 1:                /* nodaemon */
           nodaemon = 1;
           break;
        case 2:
           printf("Entranced - Launcher for the Entrance Display Manager\n");
           printf("Usage: %s [OPTION] ...\n\n", argv[0]);
           printf
              ("--------------------------------------------------------------------------\n");
           printf("  -d DISPLAY         Connect to an existing X server\n");
           printf("  -help              Display this help message\n");
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

        default:
           fprintf(stderr, "Warning: Unknown command line option\n");
      }
   }

   if (!getenv("DISPLAY"))
      setenv("DISPLAY", X_DISP, 1);

   if (nodaemon)
   {
      if (write_entranced_pidfile(entranced_pid))
      {
         fprintf(stderr, "%d is the pid, but I couldn't write to %s\n",
                 entranced_pid, PIDFILE);
         exit(1);
      }
   }
   else
   {
      fork_and_exit();
   }

   /* Check to make sure entrance binary is executable */
   if (access(ENTRANCE, X_OK))
   {
      fprintf(stderr,
              "Entrance: Fatal Error: Unable to launch entrance binary. Aborting.\n");
      exit(1);
   }

   if (!nodaemon)
   {
      close(0);
      close(1);
      close(2);
   }

   /* register child signal handler */
   signal(SIGCHLD, entrance_exit);
   signal(SIGHUP, entrance_exit);
   signal(SIGUSR1, entrance_exit);
   signal(SIGTERM, entrance_exit);

   /* setup a spawner context */
   d = spawner_display_new();

   /* run X */
   spawn_x();

   if (d->status == NOT_RUNNING)
   {
      free(d);
      fprintf(stderr, "Entrance: Could not start X server\n");
      exit(1);
   }

   /* run entrance */
   spawn_entrance();

   for (;;)
   {
      pause();
   }

   return 0;
}

/* display_new */
Spawner_Display *
spawner_display_new(void)
{
   Spawner_Display *d;

   d = malloc(sizeof(Spawner_Display));
   memset(d, 0, sizeof(Spawner_Display));
   d->xprog = strdup(X_SERVER);
   d->attempts = 5;
   d->status = NOT_RUNNING;
   return (d);
}

/* spawn_entrance */
static void
spawn_entrance(void)
{
   pid_t pid, ppid;

   ppid = getpid();

   /* First fork */
   switch (pid = fork())
   {
     case 0:
        /* Then fork and Exit */
        if ((pid = fork()) == -1)
        {
           fprintf(stderr,
                   "Entrance: FATAL: Could not fork() entrance process\n");
           exit(1);
        }
        else if (pid)
           exit(0);
        else
        {
           /* Declare independence from the colonial masters */
           setsid();

           /* Then fork again. woohoo */
           if ((pid = fork()) == -1)
           {
              fprintf(stderr,
                      "Entrance: FATAL: Could not fork() entrance process\n");
              exit(1);
           }
           if (pid)
           {
              /* Wait for client session process to die, then destroy this
                 process group */
              pid_t chld;

              while ((chld = wait(NULL)))
              {
                 if (chld == pid)
                 {
                    /* Tell daemon that this session is done. */
                    kill(ppid, SIGUSR1);

                    /* Die hard */
                    kill(0, SIGKILL);
                    /* For the hell of it */
                    exit(0);
                 }
              }
           }
           else
           {
              /* Launch entrance client */
              if (execl(ENTRANCE, ENTRANCE, d->name, NULL) < 0)
                 exit(1);
           }
        }
        break;
     case -1:
        fprintf(stderr,
                "Entrance: FATAL: Could not fork() entrance process\n");
        exit(1);
        break;
     default:
        d->pid.client = pid;
        break;
   }
}

/* entrance_exit */
void
entrance_exit(int signum)
{
   int status = 0;
   pid_t pid;

   /* Terminate X session */
   if (signum == SIGTERM || signum == SIGHUP)
   {
      kill(d->pid.x, SIGTERM);
      exit(0);
   }

   /* The session process has died */
   else if (signum == SIGUSR1)
   {
      /* Die Hard Like Bruce Willis */
      sleep(1);
      if (!waitpid(d->pid.x, &status, WNOHANG))
         x_server_killall();
      else
      {
         d->display = NULL;
         d->status = NOT_RUNNING;
         spawn_x();
      }
      spawn_entrance();
   }

   /* X Server died (likely) */
   else
   {
      pid = wait(&status);
      if (pid == d->pid.x)
      {
         printf("INFO: X Server died.\n");
         if (d->display)
         {
            /* Die Hard 2 */
            kill(d->pid.x, SIGTERM);
            sleep(1);
            d->display = NULL;
         }
         if (d->status == LAUNCHING)
         {
            d->status = NOT_RUNNING;
            fprintf(stderr,
                    "Entrance: X died mysteriously whilst launching.\n"
                    "        Waiting 10 seconds before trying again.\n");
            sleep(10);
         }
         d->status = NOT_RUNNING;

         spawn_x();
         spawn_entrance();
      }
   }
}

/* spawn_x */
static void
spawn_x(void)
{
   int i = 0;

   d->status = NOT_RUNNING;
   while ((i < d->attempts) && (d->status != RUNNING))
   {
      if (start_server_once(d) == RUNNING)
      {
         d->status = RUNNING;
         break;
      }
      i++;
   }
}


/* start_server_once */
static int
start_server_once(Spawner_Display * d)
{
   double start_time = 0;
   int pid;
   int dspnum = 0;

   d->status = LAUNCHING;
   switch (pid = fork())
   {
     case 0:
        execl("/bin/sh", "/bin/sh", "-c", d->xprog, d->xprog, NULL);
        start_time = get_time();
        break;
     case -1:
        fprintf(stderr, "Entrance: Could not fork() to spawn X process\n");
        perror("Entrance");
        exit(0);
        break;
     default:
        d->pid.x = pid;
        break;
   }

   d->name = strdup(getenv("DISPLAY"));
   while (!(d->display = XOpenDisplay(d->name)))
   {
      double current_time;

      current_time = get_time();
      usleep(100000);
      if (((start_time - current_time) > 5.0) || (dspnum > 2))
         break;
   }

   if (!d->display)
      return NOT_RUNNING;

   return RUNNING;
}

/* get_time */
double
get_time(void)
{
   struct timeval timev;

   gettimeofday(&timev, NULL);
   return (double) timev.tv_sec + (((double) timev.tv_usec) / 1000000);
}
