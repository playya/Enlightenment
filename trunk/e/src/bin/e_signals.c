/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 * NOTE TO FreeBSD users. Install libexecinfo from 
 * ports/devel/libexecinfo and add -lexecinfo to LDFLAGS
 * to add backtrace support.
 */
#include "e.h"
#include <X11/Xlib.h>

static volatile Eina_Bool _e_x_composite_shutdown_try = 0;
static void
_e_x_composite_shutdown(void)
{
   Ecore_X_Display *dpy;
   Ecore_X_Window root;

   if (_e_x_composite_shutdown_try) return; /* we failed :-( */
   _e_x_composite_shutdown_try = 1;

   dpy = ecore_x_display_get();
   root = DefaultRootWindow(dpy);

   /* ignore errors, we really don't care at this point */
   ecore_x_composite_unredirect_subwindows(root, ECORE_X_COMPOSITE_UPDATE_MANUAL);
   _e_x_composite_shutdown_try = 0;
}

#define _e_write_safe(fd, buf) _e_write_safe_int(fd, buf, sizeof(buf))
static void
_e_write_safe_int(int fd, const char *buf, size_t size)
{
   while (size > 0)
     {
	ssize_t done = write(fd, buf, size);
	if (done >= 0)
	  {
	     buf += done;
	     size -= done;
	  }
	else
	  {
	     if ((errno == EAGAIN) || (errno == EINTR))
	       continue;
	     else
	       {
		  perror("write");
		  return;
	       }
	  }
     }
}

static void
_e_gdb_print_backtrace(int fd)
{
   char cmd[1024];
   size_t size;

   // FIXME: we are in a segv'd state. do as few function calls and things
   // depending on a known working state as possible. this also prevents the
   // white box allowing recovery or deeper gdbing, thus until this works
   // properly, it's disabled (properly means always reliable, always
   // printf bt and allows e to continue and pop up box, perferably allowing
   // debugging in the gui etc. etc.
   return;
   
   if (getenv("E_NO_GDB_BACKTRACE"))
     return;

   size = snprintf(cmd, sizeof(cmd),
		   "gdb --pid=%d "
		   "-ex 'thread apply all bt' "
		   "-ex detach -ex quit", getpid());

   if (size >= sizeof(cmd))
     return;

   _e_write_safe(fd, "EXECUTING GDB AS: ");
   _e_write_safe_int(fd, cmd, size);
   _e_write_safe(fd, "\n");
   system(cmd); // TODO: use popen() or fork()+pipe()+exec() and save to 'fd'
}

#define _e_backtrace(msg) _e_backtrace_int(2, msg, sizeof(msg))
static void
_e_backtrace_int(int fd, const char *msg, size_t msg_len)
{
   char attachmsg[1024];
   void *array[255];
   size_t size;

   _e_write_safe_int(fd, msg, msg_len);
   _e_write_safe(fd, "\nBEGIN TRACEBACK\n");
   size = backtrace(array, 255);
   backtrace_symbols_fd(array, size, fd);
   _e_write_safe(fd, "END TRACEBACK\n");

   size = snprintf(attachmsg, sizeof(attachmsg),
		   "debug with: gdb --pid=%d\n", getpid());
   if (size < sizeof(attachmsg))
     _e_write_safe_int(fd, attachmsg, size);

   _e_gdb_print_backtrace(fd);
}

/* a tricky little devil, requires e and it's libs to be built
 * with the -rdynamic flag to GCC for any sort of decent output. 
 */
EAPI void
e_sigseg_act(int x, siginfo_t *info, void *data)
{
   _e_backtrace("**** SEGMENTATION FAULT ****");
   _e_x_composite_shutdown();
   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_ungrab();
   ecore_x_sync();
   e_alert_show("This is very bad. Enlightenment SEGV'd.\n"
		"\n"
		"This is not meant to happen and is likely a sign of\n"
		"a bug in Enlightenment or the libraries it relies\n"
		"on. You can gdb attach to this process now to try\n"
		"debug it or you could exit, or just hit restart to\n"
		"try and get your desktop back the way it was.\n"
		"\n"
		"Please compile everything with -g in your CFLAGS\n");
   exit(-11); 
}

EAPI void
e_sigill_act(int x, siginfo_t *info, void *data)
{
   _e_backtrace("**** ILLEGAL INSTRUCTION ****");
   _e_x_composite_shutdown();
   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_ungrab();
   ecore_x_sync();
   e_alert_show("This is very bad. Enlightenment SIGILL'd.\n"
		"\n"
		"This is not meant to happen and is likely a sign of\n"
		"a bug in Enlightenment or the libraries it relies\n"
		"on. You can gdb attach to this process now to try\n"
		"debug it or you could exit, or just hit restart to\n"
		"try and get your desktop back the way it was.\n"
		"\n"
		"Please compile everything with -g in your CFLAGS\n");
   exit(-11);
}

EAPI void
e_sigfpe_act(int x, siginfo_t *info, void *data)
{
   _e_backtrace("**** FLOATING POINT EXCEPTION ****");
   _e_x_composite_shutdown();
   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_ungrab();
   ecore_x_sync();
   e_alert_show("This is very bad. Enlightenment SIGFPE'd.\n"
		"\n"
		"This is not meant to happen and is likely a sign of\n"
		"a bug in Enlightenment or the libraries it relies\n"
		"on. You can gdb attach to this process now to try\n"
		"debug it or you could exit, or just hit restart to\n"
		"try and get your desktop back the way it was.\n"
		"\n"
		"Please compile everything with -g in your CFLAGS\n");
   exit(-11);
}

EAPI void
e_sigbus_act(int x, siginfo_t *info, void *data)
{
   _e_backtrace("**** BUS ERROR ****");
   _e_x_composite_shutdown();
   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_ungrab();
   ecore_x_sync();
   e_alert_show("This is very bad. Enlightenment SIGILL'd.\n"
		"\n"
		"This is not meant to happen and is likely a sign of\n"
		"a bug in Enlightenment or the libraries it relies\n"
		"on. You can gdb attach to this process now to try\n"
		"debug it or you could exit, or just hit restart to\n"
		"try and get your desktop back the way it was.\n"
		"\n"
		"Please compile everything with -g in your CFLAGS\n");
   exit(-11);
}

EAPI void
e_sigabrt_act(int x, siginfo_t *info, void *data)
{
   _e_backtrace("**** ABORT ****");
   _e_x_composite_shutdown();
   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_ungrab();
   ecore_x_sync();
   e_alert_show("This is very bad. Enlightenment SIGABRT'd.\n"
		"\n"
		"This is not meant to happen and is likely a sign of\n"
		"a bug in Enlightenment or the libraries it relies\n"
		"on. You can gdb attach to this process now to try\n"
		"debug it or you could exit, or just hit restart to\n"
		"try and get your desktop back the way it was.\n"
		"\n"
		"Please compile everything with -g in your CFLAGS\n");
   exit(-11);
}
