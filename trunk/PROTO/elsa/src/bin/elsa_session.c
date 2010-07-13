
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "elsa.h"


char *mcookie;
char **env;
char *_user;
static pid_t _session_pid;
static int _elsa_session_userid_set(struct passwd *pwd);

long
elsa_session_seed_get() {
    struct timespec ts;
    long pid = getpid();
    long tm = time(NULL);
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
       ts.tv_sec = ts.tv_nsec = 0;
    return pid + tm + (ts.tv_sec ^ ts.tv_nsec);
}

static int
_elsa_session_cookie_add(const char *mcookie, const char *display,
                         const char *xauth_cmd, const char *auth_file) {
    char buf[4096];
    FILE *cmd;
    if (!xauth_cmd || !auth_file) return 1;
    snprintf(buf, sizeof(buf), "%s -f %s -q", xauth_cmd, auth_file);
    fprintf(stderr, "Elsa: write auth %s\n", buf);

    cmd = popen(strdup(buf), "w");
    if (!cmd) return 1;
    fprintf(cmd, "remove %s\n", display);
    fprintf(cmd, "add %s . %s\n", display, mcookie);
    fprintf(cmd, "exit\n");
    pclose(cmd);
    return 0;
}

static int
_elsa_session_userid_set(struct passwd *pwd) {
   if (!pwd) return 1;
   if (initgroups(pwd->pw_name, pwd->pw_gid) != 0) return 1;
   if (setgid(pwd->pw_gid) != 0) return 1;
   if (setuid(pwd->pw_uid) != 0) return 1;

   fprintf(stderr, "Elsa: name -> %s, gid -> %d, uid -> %d\n", pwd->pw_name, pwd->pw_gid, pwd->pw_uid);
   return 0;
}

int
elsa_session_init(struct passwd *pwd) {
   char buf[4096];
   char **tmp;
   fprintf(stderr, "%s: Session Init\n", PACKAGE);
   env = elsa_pam_env_list_get();
   elsa_pam_end();
   for (tmp = env; *tmp; ++tmp)
      fprintf(stderr, "%s: env %s\n", PACKAGE, *tmp);
   if(_elsa_session_userid_set(pwd)) return 1;
   if (pwd->pw_shell[0] == '\0') {
      setusershell();
      strcpy(pwd->pw_shell, getusershell());
      endusershell();
   }
   snprintf(buf, sizeof(buf), "%s/.Xauthority", pwd->pw_dir);
   _elsa_session_cookie_add(mcookie, ":0", elsa_config->command.xauth.path, buf);
   return 0;
}

void
elsa_session_run(struct passwd *pwd) {
#ifdef USE_PAM
   char buf[4096];
   pid_t pid;
   pid = fork();
   if (pid == 0) {
      if (elsa_session_init(pwd)) {
         fprintf(stderr, "Elsa: couldn't open session\n");
         exit(1);
      }
      fprintf(stderr, "%s: Session Run\n", PACKAGE);
      snprintf(buf, sizeof(buf),
               "%s %s ",
               elsa_config->command.session.start,
               pwd->pw_name);
      system(buf);
      chdir(pwd->pw_dir);
      fprintf(stderr, "je suis une session de %s\n", pwd->pw_name);

      snprintf(buf, sizeof(buf), "%s/.elsa_session.log", pwd->pw_dir);
      remove(buf);
      snprintf(buf, sizeof(buf),
               "%s > %s/.elsa_session.log",
               elsa_config->command.session.login,
               pwd->pw_dir);
      execle(pwd->pw_shell, pwd->pw_shell, "-c",
             buf, NULL, env);
      fprintf(stderr, "Enlightenment n'est pas lance\n");
   }
   else {
      elsa_session_pid_set(pid);
      _user = strdup(pwd->pw_name);
      elsa_gui_shutdown();
   }
#endif
}

void
elsa_session_pid_set(pid_t pid) {
   fprintf(stderr, "%s: session pid %d\n", PACKAGE, pid);
   _session_pid = pid;
}

pid_t
elsa_session_pid_get() {
   return _session_pid;
}

void
elsa_session_shutdown() {
   struct passwd *pwd = NULL;
   char buf[4096];
   fprintf(stderr, PACKAGE": Session Shutdown\n");
   if (_user) {
      snprintf(buf, sizeof(buf),
               "%s %s ",
               elsa_config->command.session.stop,
               _user);
      free(_user);
      system(buf);
   }
}

void
elsa_session_auth(const char *file) {
   /* this is the mit cookie */

   uint16_t word;
   uint8_t hi, lo;
   int i;
   char buf[4096];

   mcookie = calloc(32, sizeof(char));

   const char *dig = "0123456789abcdef";
   srand(elsa_session_seed_get());
   for (i=0; i<32; i+=4) {
      word = rand() & 0xffff;
      lo = word & 0xff;
      hi = word >> 8;
      mcookie[i] = dig[lo & 0x0f];
      mcookie[i+1] = dig[lo >> 4];
      mcookie[i+2] = dig[hi & 0x0f];
      mcookie[i+3] = dig[hi >> 4];
   }
   remove(file);
   snprintf(buf, sizeof(buf), "XAUTHORITY=%s", file);
   putenv(buf);
   fprintf(stderr, "Elsa: cookie %s \n", mcookie);
   _elsa_session_cookie_add(mcookie, ":0", "/usr/bin/xauth", file);
}

