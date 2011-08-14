#include "elsa.h"
#include <sys/wait.h>
#include <unistd.h>

typedef struct Elsa_Xserver_
{
   const char *dname;
   Elsa_X_Cb start;
} Elsa_Xserver;

Elsa_Xserver *_xserver;
Ecore_Event_Handler *_handler_start;

/*
 * man Xserver
 * SIGUSR1 This  signal  is  used  quite  differently  from  either of the
 * above.  When the server starts, it checks to see if it has inherite
 * SIGUSR1 as SIG_IGN instead of the usual SIG_DFL.  In this case, the server
 * sends a SIGUSR1 to its parent process after it has set up the various
 * connection schemes.  Xdm uses this feature to recognize when connecting to
 * the server is possible.
 * */
static void
_env_set(const char *dname)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "DISPLAY=%s", dname);
   putenv(strdup(buf));
}

static int
_xserver_start()
{
   char *buf = NULL;
   char **args = NULL;
   pid_t pid;

   pid = fork();
   if (!pid)
     {
        char *token;
        int num_token = 0;
        elsa_close_log();
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);

        if (!(buf = strdup(elsa_config->command.xinit_args)))
          goto xserver_error;
        token = strtok(buf, " ");
        while(token)
          {
            ++num_token;
            token = strtok(NULL, " ");
          }
        if (buf) free(buf);
        if (num_token)
          {
             int i;
             if (!(buf = strdup(elsa_config->command.xinit_args)))
               goto xserver_error;
             if (!(args = calloc(num_token + 2, sizeof(char *))))
               {
                  if (buf) free(buf);
                  goto xserver_error;
               }
             args[0] = (char *)elsa_config->command.xinit_path;
             token = strtok(buf, " ");
             ++num_token;
             for(i = 1; i < num_token; ++i)
               {
                  if (token)
                    args[i] = token;
                  token = strtok(NULL, " ");
               }
             args[num_token] = NULL;
          }
        else
          {
             if (!(args = calloc(2, sizeof(char*))))
               goto xserver_error;
             args[0] = (char *)elsa_config->command.xinit_path;
             args[1] = NULL;
          }
        execv(args[0], args);
        if (buf) free(buf);
        if (args) free(args);
        fprintf(stderr, PACKAGE": Couldn't launch Xserver ...\n");
     }
   return pid;
xserver_error:
   _exit(EXIT_FAILURE);
}

static Eina_Bool
_xserver_started(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   _env_set(_xserver->dname);
   _xserver->start(_xserver->dname);
   return ECORE_CALLBACK_PASS_ON;
}

int
elsa_xserver_init(Elsa_X_Cb start, const char *dname)
{
   int pid;
   char buf[64];
   sigset_t newset;
   sigemptyset(&newset);

   _xserver = calloc(1, sizeof(Elsa_Xserver));
   _xserver->dname = eina_stringshare_add(dname);
   _xserver->start = start;
   pid = _xserver_start();
   snprintf(buf, sizeof(buf), "ELSA_XPID=%d", pid);
   putenv(strdup(buf));
   printf("need my pid here %s\n", getenv("ELSA_XPID"));
   _handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                            _xserver_started,
                                            NULL);
   return pid;
}

void
elsa_xserver_end()
{
   printf("ending xserver\n");
   unsetenv("ELSA_XPID");
}

void
elsa_xserver_shutdown()
{
   eina_stringshare_del(_xserver->dname);
   free(_xserver);
   ecore_event_handler_del(_handler_start);
}

