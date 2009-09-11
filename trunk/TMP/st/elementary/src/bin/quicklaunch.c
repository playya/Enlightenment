#include <Elementary.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

static double restart_time = 0.0;

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

static struct sigaction old_sigint;
static struct sigaction old_sigterm;
static struct sigaction old_sigquit;
static struct sigaction old_sigalrm;
static struct sigaction old_sigusr1;
static struct sigaction old_sigusr2;
static struct sigaction old_sighup;
static struct sigaction old_sigchld;
static struct sigaction old_sigsegv;
static struct sigaction old_sigill;
static struct sigaction old_sigfpe;
static struct sigaction old_sigbus;
static struct sigaction old_sigabrt;
static int _log_dom = -1;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)

static void
post_fork(void *data)
{
   sigaction(SIGINT, &old_sigint, NULL);
   sigaction(SIGTERM, &old_sigterm, NULL);
   sigaction(SIGQUIT, &old_sigquit, NULL);
   sigaction(SIGALRM, &old_sigalrm, NULL);
   sigaction(SIGUSR1, &old_sigusr1, NULL);
   sigaction(SIGUSR2, &old_sigusr2, NULL);
   sigaction(SIGHUP, &old_sighup, NULL);
   sigaction(SIGCHLD, &old_sigchld, NULL);
   sigaction(SIGSEGV, &old_sigsegv, NULL);
   sigaction(SIGILL, &old_sigill, NULL);
   sigaction(SIGFPE, &old_sigfpe, NULL);
   sigaction(SIGBUS, &old_sigbus, NULL);
   sigaction(SIGABRT, &old_sigabrt, NULL);
   if ((_log_dom > -1) && (_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
	eina_log_domain_unregister(_log_dom);
	_log_dom = -1;
     }
}

static void
child_handler(int x, siginfo_t *info, void *data)
{
   int status;
   pid_t pid;

   while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

static void
crash_handler(int x, siginfo_t *info, void *data)
{
   double t;

   ERR("crash detected. restarting.");
   t = ecore_time_get();
   if ((t - restart_time) <= 2.0)
     {
	CRITICAL("crash too fast - less than 2 seconds. abort restart");
	exit(-1);
     }
   ecore_app_restart();
}

static void
handle_run(int fd, unsigned long bytes)
{
   unsigned char *buf = NULL;
   int i, num;
   char **argv = NULL;
   char *cwd;
   int argc;

   buf = alloca(bytes);
   if ((num = read(fd, buf, bytes)) < 0)
     {
	close(fd);
	return;
     }
   close(fd);
   argc = ((unsigned long *)(buf))[0];
   argv = (char **)(&(((unsigned long *)(buf))[1]));
   for (i = 0; i < argc; i++) argv[i] = buf + (unsigned long)argv[i];
   cwd = argv[argc - 1] + strlen(argv[argc - 1]) + 1;
   elm_quicklaunch_prepare(argc, argv);
   elm_quicklaunch_fork(argc, argv, cwd, post_fork, NULL);
   elm_quicklaunch_cleanup();
}

int
main(int argc, char **argv)
{
   int sock, socket_unix_len;
   struct stat st;
   struct sockaddr_un socket_unix;
   struct linger lin;
   char buf[PATH_MAX];
   struct sigaction action;

   if (!eina_init())
     {
	fprintf(stderr, "ERROR: failed to init eina.");
	exit(-1);
     }
   _log_dom = eina_log_domain_register
     ("elementary_quicklaunch", EINA_COLOR_CYAN);
   if (_log_dom < 0)
     {
	EINA_LOG_ERR("could not register elementary_quicklaunch log domain.");
	_log_dom = EINA_LOG_DOMAIN_GLOBAL;
     }

   if (!getenv("DISPLAY"))
     {
	CRITICAL("DISPLAY env var not set");
	exit(-1);
     }
   snprintf(buf, sizeof(buf), "/tmp/elm-ql-%i", getuid());
   if (stat(buf, &st) < 0) mkdir(buf, S_IRUSR | S_IWUSR | S_IXUSR);
   snprintf(buf, sizeof(buf), "/tmp/elm-ql-%i/%s", getuid(), getenv("DISPLAY"));
   unlink(buf);
   sock = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sock < 0)
     {
	CRITICAL("cannot create socket for socket for '%s': %s",
		 buf, strerror(errno));
	exit(-1);
     }
   if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
     {
	CRITICAL("cannot set close on exec socket for '%s' (fd=%d): %s",
		 buf, sock, strerror(errno));
	exit(-1);
     }
   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
     {
	CRITICAL("cannot set linger for socket for '%s' (fd=%d): %s",
		 buf, sock, strerror(errno));
	exit(-1);
     }
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (bind(sock, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
	CRITICAL("cannot bind socket for '%s' (fd=%d): %s",
		 buf, sock, strerror(errno));
	exit(-1);
     }
   if (listen(sock, 4096) < 0)
     {
	CRITICAL("listen(sock=%d, 4096): %s", sock, strerror(errno));
	exit(-1);
     }
   elm_quicklaunch_init(argc, argv);
   restart_time = ecore_time_get();

   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, &old_sigint);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGTERM, &action, &old_sigterm);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGQUIT, &action, &old_sigquit);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGALRM, &action, &old_sigalrm);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGUSR1, &action, &old_sigusr1);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGUSR2, &action, &old_sigusr2);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGHUP, &action, &old_sighup);

   action.sa_handler = NULL;
   action.sa_sigaction = child_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGCHLD, &action, &old_sigchld);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGSEGV, &action, &old_sigsegv);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGILL, &action, &old_sigill);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGFPE, &action, &old_sigfpe);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGBUS, &action, &old_sigbus);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGABRT, &action, &old_sigabrt);

   for (;;)
     {
	int fd;
	struct sockaddr_un client;
	socklen_t len;

	elm_quicklaunch_sub_init(argc, argv);
	elm_quicklaunch_seed();
	len = sizeof(struct sockaddr_un);
	fd = accept(sock, (struct sockaddr *)&client, &len);
	if (fd >= 0)
	  {
	     int bytes;
	     char line[4096];
	     int num;

	     num = read(fd, &bytes, sizeof(unsigned long));
	     if (num == sizeof(unsigned long))
	       {
		  ecore_app_args_set(argc, (const char **)argv);
		  handle_run(fd, bytes);
	       }
	  }
	elm_quicklaunch_sub_shutdown();
     }
   elm_quicklaunch_shutdown();

   if ((_log_dom > -1) && (_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
	eina_log_domain_unregister(_log_dom);
	_log_dom = -1;
     }
   eina_shutdown();

   return 0;
}
