#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "../config.h"

#include "entrance_auth.h"

#ifdef HAVE_PAM
int
entrance_end_user_session(Entrance_Auth * e)
{
   int pamerr;

   if (!e->pam.handle)
      return ERROR_NO_PAM_INIT;

   syslog(LOG_INFO, "Ending PAM session for user \"%s\".", e->user);

   if ((pamerr = pam_close_session(e->pam.handle, PAM_SILENT)) != PAM_SUCCESS)
   {
      syslog(LOG_CRIT, "PAM: %s.", pam_strerror(e->pam.handle, pamerr));
      return ERROR_NO_PAM_INIT;
   }

   return E_SUCCESS;
}
#endif

static void
child(int sig, siginfo_t *si, void *foo)
{
   syslog(LOG_INFO, "Child exit signal.");
}

int
main(int argc, char **argv)
{
   pid_t pid = -1;
   int status;
   struct sigaction action;

#ifdef HAVE_PAM
   char *user = NULL;
   char *display = NULL;
   Entrance_Auth *e = NULL;
#endif

   openlog("entrance_login", LOG_PID, LOG_DAEMON);

#ifdef HAVE_PAM
   if (argc != 4)
#else
   if (argc != 2)
#endif
   {
      syslog(LOG_CRIT, "Wrong number of arguments: %d!", argc);
      return 0;
   }

   if (getuid() != 0)
   {
      syslog(LOG_CRIT, "Not running as root!");
      exit(1);
   }

   pid = atoi(argv[1]);
#ifdef HAVE_PAM
   user = argv[2];
   display = argv[3];

   syslog(LOG_CRIT, "Wait for %s on %s", user, display);
   if (user && display)
   {
      e = entrance_auth_new();
      if (entrance_auth_user_set(e, user))
      {
         syslog(LOG_CRIT, "Can't set user %s!", user);
         exit(1);
      }
      entrance_auth_pam_initialize(e, display);
   }
#endif

   action.sa_sigaction = child;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGCHLD, &action, NULL);
   syslog(LOG_CRIT, "Wait for pid %i", pid);
   if (waitpid(pid, &status, 0) == pid)
   {
#ifdef HAVE_PAM
      if (e)
      {
         if (entrance_end_user_session(e) != E_SUCCESS)
            syslog(LOG_INFO, "Error Shutting down PAM");
         entrance_auth_free(e);
      }
#endif
      syslog(LOG_CRIT, "Wait done - child exited normally");
      closelog();
      exit(0);
   }
   syslog(LOG_CRIT, "Wait error: %s", strerror(errno));
   return -1;
}
