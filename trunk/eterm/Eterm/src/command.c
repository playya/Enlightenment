/*--------------------------------*-C-*---------------------------------*
 * File:	command.c
 */
/* notes: */
/*----------------------------------------------------------------------*
 * Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 * You can do what you like with this source code as long as
 * you don't try to make money out of it and you include an
 * unaltered copy of this message (including the copyright).
 *
 * This module has been very heavily modified by R. Nation
 * <nation@rocket.sanders.lockheed.com>
 * No additional restrictions are applied
 *
 * Additional modification by Garrett D'Amore <garrett@netcom.com> to
 * allow vt100 printing.  No additional restrictions are applied.
 *
 * Integrated modifications by Steven Hirsch <hirsch@emba.uvm.edu> to
 * properly support X11 mouse report mode and support for DEC
 * "private mode" save/restore functions.
 *
 * Integrated key-related changes by Jakub Jelinek <jj@gnu.ai.mit.edu>
 * to handle Shift+function keys properly.
 * Should be used with enclosed termcap / terminfo database.
 *
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions.
 *
 * Further modification and cleanups for Solaris 2.x and Linux 1.2.x
 * by Raul Garcia Garcia <rgg@tid.es>. No additional restrictions.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/

static const char cvs_ident[] = "$Id$";

/* includes: */
#include "config.h"
#include "feature.h"

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#include <sys/types.h>
#include <limits.h>

/* X11 Headers */
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/IntrinsicP.h>
#ifdef USE_GETGRNAME
# include <grp.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if defined (__svr4__)
# include <sys/resource.h>	/* for struct rlimit */
# include <sys/stropts.h>	/* for I_PUSH */
# ifdef HAVE_SYS_STRTIO_H
#  include <sys/strtio.h>
# endif
# ifdef HAVE_BSDTTY_H
#  include <bsdtty.h>
# endif
# define _NEW_TTY_CTRL		/* to get proper defines in <termios.h> */
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#else
# include <sgtty.h>
#endif
#if defined(__sun) && defined(__SVR4)
# include <sys/strredir.h>
#endif
#include <sys/wait.h>
#include <sys/stat.h>
#if defined(linux)
# include <linux/tty.h>		/* For N_TTY_BUF_SIZE. */
#endif
#if defined(linux)
# include <string.h>		/* For strsep(). -vendu */
#endif

/* Eterm-specific Headers */
#include "command.h"
#include "main.h"
#include "../libmej/debug.h"
#include "debug.h"
#include "../libmej/mem.h"
#include "../libmej/strings.h"
#include "events.h"
#include "graphics.h"
#include "grkelot.h"
#include "options.h"
#include "pixmap.h"
#ifdef PROFILE
# include "profile.h"
#endif
#include "screen.h"
#include "scrollbar.h"
#include "string.h"
#include "term.h"
#ifdef USE_POSIX_THREADS
# include "threads.h"
#endif
#ifdef UTMP_SUPPORT
# include "eterm_utmp.h"
#endif
#include "windows.h"

/* local variables */
int my_ruid, my_euid, my_rgid, my_egid;
char initial_dir[PATH_MAX + 1];
static char *ptydev = NULL, *ttydev = NULL;	/* pty/tty name */
int cmd_fd = -1;		/* file descriptor connected to the command */
pid_t cmd_pid = -1;		/* process id if child */
int Xfd = -1;			/* file descriptor of X server connection */
unsigned int num_fds = 0;	/* number of file descriptors being used */
struct stat ttyfd_stat;		/* original status of the tty we will use */
int refresh_count = 0, refresh_limit = 1, refresh_type = FAST_REFRESH;
Atom wmDeleteWindow;
unsigned char cmdbuf_base[CMD_BUF_SIZE], *cmdbuf_ptr, *cmdbuf_endp;
/* Addresses pasting large amounts of data
 * code pinched from xterm
 */
static char *v_buffer;		/* pointer to physical buffer */
static char *v_bufstr = NULL;	/* beginning of area to write */
static char *v_bufptr;		/* end of area to write */
static char *v_bufend;		/* end of physical buffer */
/* OffiX Dnd (drag 'n' drop) support */
#ifdef OFFIX_DND
static Atom DndProtocol, DndSelection;
#endif /* OFFIX_DND */
#ifdef USE_XIM
XIC Input_Context = NULL;	/* input context */
#endif

/* Substitutes for missing system functions */
#if !defined(_POSIX_VERSION) && defined(__svr4__)
int
getdtablesize(void)
{
  struct rlimit rlim;

  getrlimit(RLIMIT_NOFILE, &rlim);
  return rlim.rlim_cur;
}
# endif

/* Take care of suid/sgid super-user (root) privileges */
void
privileges(int mode)
{

#ifdef __CYGWIN32__
  return;
#endif

  switch (mode) {
    case IGNORE:
      /* Revoke suid/sgid privs and return to normal uid/gid -- mej */
      D_UTMP(("[%ld]: Before privileges(REVERT): [ %ld, %ld ]  [ %ld, %ld ]\n",
	      getpid(), getuid(), getgid(), geteuid(), getegid()));

#ifdef HAVE_SETRESGID
      setresgid(my_rgid, my_rgid, my_egid);
#elif defined(HAVE_SAVED_UIDS)
      setregid(my_rgid, my_rgid);
#else
      setregid(my_egid, -1);
      setregid(-1, my_rgid);
#endif

#ifdef HAVE_SETRESUID
      setresuid(my_ruid, my_ruid, my_euid);
#elif defined(HAVE_SAVED_UIDS)
      setreuid(my_ruid, my_ruid);
#else
      setreuid(my_euid, -1);
      setreuid(-1, my_ruid);
#endif

      D_UTMP(("[%ld]: After privileges(REVERT): [ %ld, %ld ]  [ %ld, %ld ]\n",
	      getpid(), getuid(), getgid(), geteuid(), getegid()));
      break;

    case SAVE:
      break;

    case RESTORE:
      D_UTMP(("[%ld]: Before privileges(INVOKE): [ %ld, %ld ]  [ %ld, %ld ]\n",
	      getpid(), getuid(), getgid(), geteuid(), getegid()));

#ifdef HAVE_SETRESUID
      setresuid(my_ruid, my_euid, my_euid);
#elif defined(HAVE_SAVED_UIDS)
      setreuid(my_ruid, my_euid);
#else
      setreuid(-1, my_euid);
      setreuid(my_ruid, -1);
#endif

#ifdef HAVE_SETRESGID
      setresgid(my_rgid, my_egid, my_egid);
#elif defined(HAVE_SAVED_UIDS)
      setregid(my_rgid, my_egid);
#else
      setregid(-1, my_egid);
      setregid(my_rgid, -1);
#endif

      D_UTMP(("[%ld]: After privileges(INVOKE): [ %ld, %ld ]  [ %ld, %ld ]\n",
	      getpid(), getuid(), getgid(), geteuid(), getegid()));
      break;
  }
}

char *
sig_to_str(int sig)
{

  /* NOTE: This can't be done with a switch because of possible conflicting
   * conflicting signal types. -vendu
   */

#ifdef SIGHUP
  if (sig == SIGHUP) {
    return ("SIGHUP");
  }
#endif

#ifdef SIGINT
  if (sig == SIGINT) {
    return ("SIGINT");
  }
#endif

#ifdef SIGQUIT
  if (sig == SIGQUIT) {
    return ("SIGQUIT");
  }
#endif

#ifdef SIGILL
  if (sig == SIGILL) {
    return ("SIGILL");
  }
#endif

#ifdef SIGTRAP
  if (sig == SIGTRAP) {
    return ("SIGTRAP");
  }
#endif

#ifdef SIGABRT
  if (sig == SIGABRT) {
    return ("SIGABRT");
  }
#endif

#ifdef SIGIOT
  if (sig == SIGIOT) {
    return ("SIGIOT");
  }
#endif

#ifdef SIGEMT
  if (sig == SIGEMT) {
    return ("SIGEMT");
  }
#endif

#ifdef SIGFPE
  if (sig == SIGFPE) {
    return ("SIGFPE");
  }
#endif

#ifdef SIGKILL
  if (sig == SIGKILL) {
    return ("SIGKILL");
  }
#endif

#ifdef SIGBUS
  if (sig == SIGBUS) {
    return ("SIGBUS");
  }
#endif

#ifdef SIGSEGV
  if (sig == SIGSEGV) {
    return ("SIGSEGV");
  }
#endif

#ifdef SIGSYS
  if (sig == SIGSYS) {
    return ("SIGSYS");
  }
#endif

#ifdef SIGPIPE
  if (sig == SIGPIPE) {
    return ("SIGPIPE");
  }
#endif

#ifdef SIGALRM
  if (sig == SIGALRM) {
    return ("SIGALRM");
  }
#endif

#ifdef SIGTERM
  if (sig == SIGTERM) {
    return ("SIGTERM");
  }
#endif

#ifdef SIGUSR1
  if (sig == SIGUSR1) {
    return ("SIGUSR1");
  }
#endif

#ifdef SIGUSR2
  if (sig == SIGUSR2) {
    return ("SIGUSR2");
  }
#endif

#ifdef SIGCHLD
  if (sig == SIGCHLD) {
    return ("SIGCHLD");
  }
#endif

#ifdef SIGCLD
  if (sig == SIGCLD) {
    return ("SIGCLD");
  }
#endif

#ifdef SIGPWR
  if (sig == SIGPWR) {
    return ("SIGPWR");
  }
#endif

#ifdef SIGVTALRM
  if (sig == SIGVTALRM) {
    return ("SIGVTALRM");
  }
#endif

#ifdef SIGPROF
  if (sig == SIGPROF) {
    return ("SIGPROF");
  }
#endif

#ifdef SIGIO
  if (sig == SIGIO) {
    return ("SIGIO");
  }
#endif

#ifdef SIGPOLL
  if (sig == SIGPOLL) {
    return ("SIGPOLL");
  }
#endif

#ifdef SIGWINCH
  if (sig == SIGWINCH) {
    return ("SIGWINCH");
  }
#endif

#ifdef SIGWINDOW
  if (sig == SIGWINDOW) {
    return ("SIGWINDOW");
  }
#endif

#ifdef SIGSTOP
  if (sig == SIGSTOP) {
    return ("SIGSTOP");
  }
#endif

#ifdef SIGTSTP
  if (sig == SIGTSTP) {
    return ("SIGTSTP");
  }
#endif

#ifdef SIGCONT
  if (sig == SIGCONT) {
    return ("SIGCONT");
  }
#endif

#ifdef SIGTTIN
  if (sig == SIGTTIN) {
    return ("SIGTTIN");
  }
#endif

#ifdef SIGTTOU
  if (sig == SIGTTOU) {
    return ("SIGTTOU");
  }
#endif

#ifdef SIGURG
  if (sig == SIGURG) {
    return ("SIGURG");
  }
#endif

#ifdef SIGLOST
  if (sig == SIGLOST) {
    return ("SIGLOST");
  }
#endif

#ifdef SIGRESERVE
  if (sig == SIGRESERVE) {
    return ("SIGRESERVE");
  }
#endif

#ifdef SIGDIL
  if (sig == SIGDIL) {
    return ("SIGDIL");
  }
#endif

#ifdef SIGXCPU
  if (sig == SIGXCPU) {
    return ("SIGXCPU");
  }
#endif

#ifdef SIGXFSZ
  if (sig == SIGXFSZ) {
    return ("SIGXFSZ");
  }
#endif
  return ("Unknown signal");
}

const char *
event_type_to_name(int type)
{

  if (type == KeyPress) {
    return "KeyPress";
  }
  if (type == KeyRelease) {
    return "KeyRelease";
  }
  if (type == ButtonPress) {
    return "ButtonPress";
  }
  if (type == ButtonRelease) {
    return "ButtonRelease";
  }
  if (type == MotionNotify) {
    return "MotionNotify";
  }
  if (type == EnterNotify) {
    return "EnterNotify";
  }
  if (type == LeaveNotify) {
    return "LeaveNotify";
  }
  if (type == FocusIn) {
    return "FocusIn";
  }
  if (type == FocusOut) {
    return "FocusOut";
  }
  if (type == KeymapNotify) {
    return "KeymapNotify";
  }
  if (type == Expose) {
    return "Expose";
  }
  if (type == GraphicsExpose) {
    return "GraphicsExpose";
  }
  if (type == NoExpose) {
    return "NoExpose";
  }
  if (type == VisibilityNotify) {
    return "VisibilityNotify";
  }
  if (type == CreateNotify) {
    return "CreateNotify";
  }
  if (type == DestroyNotify) {
    return "DestroyNotify";
  }
  if (type == UnmapNotify) {
    return "UnmapNotify";
  }
  if (type == MapNotify) {
    return "MapNotify";
  }
  if (type == MapRequest) {
    return "MapRequest";
  }
  if (type == ReparentNotify) {
    return "ReparentNotify";
  }
  if (type == ConfigureNotify) {
    return "ConfigureNotify";
  }
  if (type == ConfigureRequest) {
    return "ConfigureRequest";
  }
  if (type == GravityNotify) {
    return "GravityNotify";
  }
  if (type == ResizeRequest) {
    return "ResizeRequest";
  }
  if (type == CirculateNotify) {
    return "CirculateNotify";
  }
  if (type == CirculateRequest) {
    return "CirculateRequest";
  }
  if (type == PropertyNotify) {
    return "PropertyNotify";
  }
  if (type == SelectionClear) {
    return "SelectionClear";
  }
  if (type == SelectionRequest) {
    return "SelectionRequest";
  }
  if (type == SelectionNotify) {
    return "SelectionNotify";
  }
  if (type == ColormapNotify) {
    return "ColormapNotify";
  }
  if (type == ClientMessage) {
    return "ClientMessage";
  }
  if (type == MappingNotify) {
    return "MappingNotify";
  }
  return "Bad Event!";
}

const char *
request_code_to_name(int code)
{

  if (code == X_CreateWindow) {
    return "XCreateWindow";
  }
  if (code == X_ChangeWindowAttributes) {
    return "XChangeWindowAttributes";
  }
  if (code == X_GetWindowAttributes) {
    return "XGetWindowAttributes";
  }
  if (code == X_DestroyWindow) {
    return "XDestroyWindow";
  }
  if (code == X_DestroySubwindows) {
    return "XDestroySubwindows";
  }
  if (code == X_ChangeSaveSet) {
    return "XChangeSaveSet";
  }
  if (code == X_ReparentWindow) {
    return "XReparentWindow";
  }
  if (code == X_MapWindow) {
    return "XMapWindow";
  }
  if (code == X_MapSubwindows) {
    return "XMapSubwindows";
  }
  if (code == X_UnmapWindow) {
    return "XUnmapWindow";
  }
  if (code == X_UnmapSubwindows) {
    return "XUnmapSubwindows";
  }
  if (code == X_ConfigureWindow) {
    return "XConfigureWindow";
  }
  if (code == X_CirculateWindow) {
    return "XCirculateWindow";
  }
  if (code == X_GetGeometry) {
    return "XGetGeometry";
  }
  if (code == X_QueryTree) {
    return "XQueryTree";
  }
  if (code == X_InternAtom) {
    return "XInternAtom";
  }
  if (code == X_GetAtomName) {
    return "XGetAtomName";
  }
  if (code == X_ChangeProperty) {
    return "XChangeProperty";
  }
  if (code == X_DeleteProperty) {
    return "XDeleteProperty";
  }
  if (code == X_GetProperty) {
    return "XGetProperty";
  }
  if (code == X_ListProperties) {
    return "XListProperties";
  }
  if (code == X_SetSelectionOwner) {
    return "XSetSelectionOwner";
  }
  if (code == X_GetSelectionOwner) {
    return "XGetSelectionOwner";
  }
  if (code == X_ConvertSelection) {
    return "XConvertSelection";
  }
  if (code == X_SendEvent) {
    return "XSendEvent";
  }
  if (code == X_GrabPointer) {
    return "XGrabPointer";
  }
  if (code == X_UngrabPointer) {
    return "XUngrabPointer";
  }
  if (code == X_GrabButton) {
    return "XGrabButton";
  }
  if (code == X_UngrabButton) {
    return "XUngrabButton";
  }
  if (code == X_ChangeActivePointerGrab) {
    return "XChangeActivePointerGrab";
  }
  if (code == X_GrabKeyboard) {
    return "XGrabKeyboard";
  }
  if (code == X_UngrabKeyboard) {
    return "XUngrabKeyboard";
  }
  if (code == X_GrabKey) {
    return "XGrabKey";
  }
  if (code == X_UngrabKey) {
    return "XUngrabKey";
  }
  if (code == X_AllowEvents) {
    return "XAllowEvents";
  }
  if (code == X_GrabServer) {
    return "XGrabServer";
  }
  if (code == X_UngrabServer) {
    return "XUngrabServer";
  }
  if (code == X_QueryPointer) {
    return "XQueryPointer";
  }
  if (code == X_GetMotionEvents) {
    return "XGetMotionEvents";
  }
  if (code == X_TranslateCoords) {
    return "XTranslateCoords";
  }
  if (code == X_WarpPointer) {
    return "XWarpPointer";
  }
  if (code == X_SetInputFocus) {
    return "XSetInputFocus";
  }
  if (code == X_GetInputFocus) {
    return "XGetInputFocus";
  }
  if (code == X_QueryKeymap) {
    return "XQueryKeymap";
  }
  if (code == X_OpenFont) {
    return "XOpenFont";
  }
  if (code == X_CloseFont) {
    return "XCloseFont";
  }
  if (code == X_QueryFont) {
    return "XQueryFont";
  }
  if (code == X_QueryTextExtents) {
    return "XQueryTextExtents";
  }
  if (code == X_ListFonts) {
    return "XListFonts";
  }
  if (code == X_ListFontsWithInfo) {
    return "XListFontsWithInfo";
  }
  if (code == X_SetFontPath) {
    return "XSetFontPath";
  }
  if (code == X_GetFontPath) {
    return "XGetFontPath";
  }
  if (code == X_CreatePixmap) {
    return "XCreatePixmap";
  }
  if (code == X_FreePixmap) {
    return "XFreePixmap";
  }
  if (code == X_CreateGC) {
    return "XCreateGC";
  }
  if (code == X_ChangeGC) {
    return "XChangeGC";
  }
  if (code == X_CopyGC) {
    return "XCopyGC";
  }
  if (code == X_SetDashes) {
    return "XSetDashes";
  }
  if (code == X_SetClipRectangles) {
    return "XSetClipRectangles";
  }
  if (code == X_FreeGC) {
    return "XFreeGC";
  }
  if (code == X_ClearArea) {
    return "XClearArea";
  }
  if (code == X_CopyArea) {
    return "XCopyArea";
  }
  if (code == X_CopyPlane) {
    return "XCopyPlane";
  }
  if (code == X_PolyPoint) {
    return "XPolyPoint";
  }
  if (code == X_PolyLine) {
    return "XPolyLine";
  }
  if (code == X_PolySegment) {
    return "XPolySegment";
  }
  if (code == X_PolyRectangle) {
    return "XPolyRectangle";
  }
  if (code == X_PolyArc) {
    return "XPolyArc";
  }
  if (code == X_FillPoly) {
    return "XFillPoly";
  }
  if (code == X_PolyFillRectangle) {
    return "XPolyFillRectangle";
  }
  if (code == X_PolyFillArc) {
    return "XPolyFillArc";
  }
  if (code == X_PutImage) {
    return "XPutImage";
  }
  if (code == X_GetImage) {
    return "XGetImage";
  }
  if (code == X_PolyText8) {
    return "XPolyText8";
  }
  if (code == X_PolyText16) {
    return "XPolyText16";
  }
  if (code == X_ImageText8) {
    return "XImageText8";
  }
  if (code == X_ImageText16) {
    return "XImageText16";
  }
  if (code == X_CreateColormap) {
    return "XCreateColormap";
  }
  if (code == X_FreeColormap) {
    return "XFreeColormap";
  }
  if (code == X_CopyColormapAndFree) {
    return "XCopyColormapAndFree";
  }
  if (code == X_InstallColormap) {
    return "XInstallColormap";
  }
  if (code == X_UninstallColormap) {
    return "XUninstallColormap";
  }
  if (code == X_ListInstalledColormaps) {
    return "XListInstalledColormaps";
  }
  if (code == X_AllocColor) {
    return "XAllocColor";
  }
  if (code == X_AllocNamedColor) {
    return "XAllocNamedColor";
  }
  if (code == X_AllocColorCells) {
    return "XAllocColorCells";
  }
  if (code == X_AllocColorPlanes) {
    return "XAllocColorPlanes";
  }
  if (code == X_FreeColors) {
    return "XFreeColors";
  }
  if (code == X_StoreColors) {
    return "XStoreColors";
  }
  if (code == X_StoreNamedColor) {
    return "XStoreNamedColor";
  }
  if (code == X_QueryColors) {
    return "XQueryColors";
  }
  if (code == X_LookupColor) {
    return "XLookupColor";
  }
  if (code == X_CreateCursor) {
    return "XCreateCursor";
  }
  if (code == X_CreateGlyphCursor) {
    return "XCreateGlyphCursor";
  }
  if (code == X_FreeCursor) {
    return "XFreeCursor";
  }
  if (code == X_RecolorCursor) {
    return "XRecolorCursor";
  }
  if (code == X_QueryBestSize) {
    return "XQueryBestSize";
  }
  if (code == X_QueryExtension) {
    return "XQueryExtension";
  }
  if (code == X_ListExtensions) {
    return "XListExtensions";
  }
  if (code == X_ChangeKeyboardMapping) {
    return "XChangeKeyboardMapping";
  }
  if (code == X_GetKeyboardMapping) {
    return "XGetKeyboardMapping";
  }
  if (code == X_ChangeKeyboardControl) {
    return "XChangeKeyboardControl";
  }
  if (code == X_GetKeyboardControl) {
    return "XGetKeyboardControl";
  }
  if (code == X_Bell) {
    return "XBell";
  }
  if (code == X_ChangePointerControl) {
    return "XChangePointerControl";
  }
  if (code == X_GetPointerControl) {
    return "XGetPointerControl";
  }
  if (code == X_SetScreenSaver) {
    return "XSetScreenSaver";
  }
  if (code == X_GetScreenSaver) {
    return "XGetScreenSaver";
  }
  if (code == X_ChangeHosts) {
    return "XChangeHosts";
  }
  if (code == X_ListHosts) {
    return "XListHosts";
  }
  if (code == X_SetAccessControl) {
    return "XSetAccessControl";
  }
  if (code == X_SetCloseDownMode) {
    return "XSetCloseDownMode";
  }
  if (code == X_KillClient) {
    return "XKillClient";
  }
  if (code == X_RotateProperties) {
    return "XRotateProperties";
  }
  if (code == X_ForceScreenSaver) {
    return "XForceScreenSaver";
  }
  if (code == X_SetPointerMapping) {
    return "XSetPointerMapping";
  }
  if (code == X_GetPointerMapping) {
    return "XGetPointerMapping";
  }
  if (code == X_SetModifierMapping) {
    return "XSetModifierMapping";
  }
  if (code == X_GetModifierMapping) {
    return "XGetModifierMapping";
  }
  if (code == X_NoOperation) {
    return "XNoOperation";
  }
  return "Unknown";
}

/* Try to get a stack trace when we croak */
void
dump_stack_trace(void)
{

  char cmd[256];

#ifdef NO_STACK_TRACE
  return;
#endif

  print_error("Attempting to dump a stack trace....\n");
  signal(SIGTSTP, exit);	/* Don't block on tty output, just die */

#ifdef HAVE_U_STACK_TRACE
  U_STACK_TRACE();
  return;
#elif defined(GDB)
  snprintf(cmd, sizeof(cmd), "/bin/echo backtrace | " GDB " " APL_NAME " %d", getpid());
#elif defined(PSTACK)
  snprintf(cmd, sizeof(cmd), PSTACK " %d", getpid());
#elif defined(DBX)
#  ifdef _AIX
  snprintf(cmd, sizeof(cmd), "/bin/echo 'where\ndetach' | " DBX " -a %d", getpid());
#  elif defined(__sgi)
  snprintf(cmd, sizeof(cmd), "/bin/echo 'where\ndetach' | " DBX " -p %d", getpid());
#  else
  snprintf(cmd, sizeof(cmd), "/bin/echo 'where\ndetach' | " DBX " %s %d", orig_argv0, getpid());
#  endif
#else
  print_error("Your system does not support any of the methods Eterm uses.  Exiting.\n");
  return;
#endif
  system(cmd);
}

void
hard_exit(void) {

  dump_stack_trace();
#ifdef HAVE__EXIT
  _exit(-1);
#else
  abort();
#endif

}

/* signal handling, exit handler */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
RETSIGTYPE
Child_signal(int sig)
{

  int pid, save_errno = errno;

  D_CMD(("Received signal %s (%d)\n", sig_to_str(sig), sig));

  do {
    errno = 0;
  } while ((-1 == (pid = waitpid(-1, NULL, WNOHANG))) &&
	   (errno == EINTR));

  D_CMD(("pid == %d, cmd_pid == %d\n", pid, cmd_pid));
  /* If the child that exited is the command we spawned, or if the
     child exited before fork() returned in the parent, it must be
     our immediate child that exited.  We exit gracefully. */
  if (pid == cmd_pid || cmd_pid == -1) {
    if (Options & Opt_pause) {
      const char *message = "\r\nPress any key to exit " APL_NAME "....";

      scr_refresh(DEFAULT_REFRESH);
      scr_add_lines(message, 1, strlen(message));
      scr_refresh(DEFAULT_REFRESH);
      keypress_exit = 1;
      return;
    }
    exit(EXIT_SUCCESS);
  }
  errno = save_errno;

  D_CMD(("Child_signal: installing signal handler\n"));
  signal(SIGCHLD, Child_signal);

  return ((RETSIGTYPE) 0);
}

/* Handles signals usually sent by a user, like HUP, TERM, INT. */
RETSIGTYPE
Exit_signal(int sig)
{

  print_error("Received terminal signal %s (%d)", sig_to_str(sig), sig);
  signal(sig, SIG_DFL);

#ifdef UTMP_SUPPORT
  privileges(INVOKE);
  cleanutent();
  privileges(REVERT);
#endif

  D_CMD(("Exit_signal(): exit(%s)\n", sig_to_str(sig)));
  exit(sig);
}

/* Handles abnormal termination signals -- mej */
static RETSIGTYPE
SegvHandler(int sig)
{

  print_error("Received terminal signal %s (%d)", sig_to_str(sig), sig);
  signal(sig, SIG_DFL);		/* Let the OS handle recursive seg faults */

  /* Lock down security so we don't write any core files as root. */
  privileges(REVERT);
  umask(077);

  /* Make an attempt to dump a stack trace */
  dump_stack_trace();

  /* Exit */
  exit(sig);
}

/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO:  Also free up X resources, etc., if possible
 */
void
clean_exit(void)
{
  scr_release();
  privileges(INVOKE);

#ifndef __CYGWIN32__
  if (ttydev) {
    D_CMD(("Restoring \"%s\" to mode %03o, uid %d, gid %d\n", ttydev, ttyfd_stat.st_mode,
	   ttyfd_stat.st_uid, ttyfd_stat.st_gid));
    if (chmod(ttydev, ttyfd_stat.st_mode) != 0) {
      D_UTMP(("chmod(\"%s\", %03o) failed:  %s\n", ttydev, ttyfd_stat.st_mode, strerror(errno)));
    }
    if (chown(ttydev, ttyfd_stat.st_uid, ttyfd_stat.st_gid) != 0) {
      D_UTMP(("chown(\"%s\", %d, %d) failed:  %s\n", ttydev, ttyfd_stat.st_uid, ttyfd_stat.st_gid, strerror(errno)));
    }
  }
#endif /* __CYGWIN32__ */

#ifdef UTMP_SUPPORT
  cleanutent();
#endif
  privileges(REVERT);
#ifdef USE_POSIX_THREADS
  /* Get rid of threads if there are any running. Doesn't work yet. */
# if 0
  D_THREADS(("pthread_kill_other_threads_np();\n"));
  pthread_kill_other_threads_np();
  D_THREADS(("pthread_exit();\n"));
# endif
#endif
}

/* Acquire a pseudo-teletype from the system. */
/*
 * On failure, returns -1.
 * On success, returns the file descriptor.
 *
 * If successful, ttydev and ptydev point to the names of the
 * master and slave parts
 */

#ifdef __sgi
inline int
sgi_get_pty(void)
{

  int fd = -1;

  ptydev = ttydev = _getpty(&fd, O_RDWR | O_NDELAY, 0620, 0);
  return (ptydev == NULL ? -1 : fd);

}
#endif

#ifdef _AIX
inline int
aix_get_pty(void)
{

  int fd = -1;

  if ((fd = open("/dev/ptc", O_RDWR)) < 0)
    return (-1);
  else
    ptydev = ttydev = ttyname(fd);
  return (fd);
}
#endif

#ifdef ALL_NUMERIC_PTYS
inline int
sco_get_pty(void)
{

  static char pty_name[] = "/dev/ptyp??\0\0\0";
  static char tty_name[] = "/dev/ttyp??\0\0\0";
  int idx;
  int fd = -1;

  ptydev = pty_name;
  ttydev = tty_name;

  for (idx = 0; idx < 256; idx++) {

    sprintf(ptydev, "%s%d", "/dev/ptyp", idx);
    sprintf(ttydev, "%s%d", "/dev/ttyp", idx);

    if (access(ttydev, F_OK) < 0) {
      idx = 256;
      break;
    }
    if ((fd = open(ptydev, O_RDWR)) >= 0) {
      if (access(ttydev, R_OK | W_OK) == 0)
	return (fd);
      close(fd);
    }
  }
  return (-1);
}
#endif

#if defined (__svr4__) || defined(__CYGWIN32__) || ((__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1))
inline int
svr_get_pty(void)
{

  int fd = -1;

  /* open the STREAMS, clone device /dev/ptmx (master pty) */
  if ((fd = open("/dev/ptmx", O_RDWR)) < 0) {
    return (-1);
  } else {
    if (grantpt(fd) != 0) {
      print_error("grantpt(%d) failed:  %s\n", fd, strerror(errno));
      return (-1);
    } else if (unlockpt(fd) != 0) {
      print_error("unlockpt(%d) failed:  %s\n", fd, strerror(errno));
      return (-1);
    } else {
      ptydev = ttydev = ptsname(fd);
      if (ttydev == NULL) {
	print_error("ptsname(%d) failed:  %s\n", fd, strerror(errno));
	return (-1);
      }
    }
  }
  return (fd);
}
#endif

#define PTYCHAR1 "pqrstuvwxyz"
#define PTYCHAR2 "0123456789abcdefghijklmnopqrstuvwxyz"

inline int
gen_get_pty(void)
{

  static char pty_name[] = "/dev/pty??";
  static char tty_name[] = "/dev/tty??";
  int len = sizeof(tty_name);
  char *c1, *c2;
  int fd = -1;

  ptydev = pty_name;
  ttydev = tty_name;

  for (c1 = PTYCHAR1; *c1; c1++) {
    ptydev[len - 3] = ttydev[len - 3] = *c1;
    for (c2 = PTYCHAR2; *c2; c2++) {
      ptydev[len - 2] = ttydev[len - 2] = *c2;
      if ((fd = open(ptydev, O_RDWR)) >= 0) {
	if (access(ttydev, R_OK | W_OK) == 0)
	  return (fd);
	close(fd);
      }
    }
  }
  return (-1);
}

int
get_pty(void)
{

  int fd = -1;

#if defined(__sgi)
  fd = sgi_get_pty();
#elif defined(_AIX)
  fd = aix_get_pty();
#elif defined(__svr4__) || defined(__CYGWIN32__)
  fd = svr_get_pty();
#elif ((__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 1))
  fd = svr_get_pty();
#elif defined(ALL_NUMERIC_PTYS)	/* SCO OSr5 */
  fd = sco_get_pty();
#endif

  /* Fall back on this method */
  if (fd == -1) {
    fd = gen_get_pty();
  }
  if (fd != -1) {
    fcntl(fd, F_SETFL, O_NDELAY);
    return (fd);
  } else {
    print_error("Can't open pseudo-tty -- %s", strerror(errno));
    return (-1);
  }
}

/* establish a controlling teletype for new session */
/*
 * On some systems this can be done with ioctl() but on others we
 * need to re-open the slave tty.
 */
int
get_tty(void)
{
  int fd;
  pid_t pid;

  /*
   * setsid() [or setpgrp] must be before open of the terminal,
   * otherwise there is no controlling terminal (Solaris 2.4, HP-UX 9)
   */
#ifndef ultrix
# ifdef NO_SETSID
  pid = setpgrp(0, 0);
# else
  pid = setsid();
# endif
  if (pid < 0) {
    D_TTYMODE(("%s: setsid() failed: %s, PID == %d\n", rs_name, strerror(errno), pid));
  }
#endif /* ultrix */

  privileges(INVOKE);
  if (ttydev == NULL) {
    print_error("Slave tty device name is NULL.  Failed to open slave pty.\n");
    exit(EXIT_FAILURE);
  } else if ((fd = open(ttydev, O_RDWR)) < 0) {
    print_error("Can't open slave tty %s -- %s", ttydev, strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    D_TTY(("Opened slave tty %s\n", ttydev));
    privileges(REVERT);
  }

#if defined (__svr4__)
  /*
   * Push STREAMS modules:
   *  ptem: pseudo-terminal hardware emulation module.
   *  ldterm: standard terminal line discipline.
   *  ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
   */
  ioctl(fd, I_PUSH, "ptem");
  ioctl(fd, I_PUSH, "ldterm");
  ioctl(fd, I_PUSH, "ttcompat");
#else /* __svr4__ */
  {
    /* change ownership of tty to real uid and real group */
    unsigned int mode = 0620;
    gid_t gid = my_rgid;

# ifdef USE_GETGRNAME
    {
      struct group *gr = getgrnam(TTY_GRP_NAME);

      if (gr) {
	/* change ownership of tty to real uid, "tty" gid */
	gid = gr->gr_gid;
	mode = 0620;
      }
    }
# endif				/* USE_GETGRNAME */

    privileges(INVOKE);
# ifndef __CYGWIN32__
    fchown(fd, my_ruid, gid);	/* fail silently */
    fchmod(fd, mode);
# endif
    privileges(REVERT);
  }
#endif /* __svr4__ */

  /*
   * Close all file descriptors.  If only stdin/out/err are closed,
   * child processes remain alive upon deletion of the window.
   */
  {
    unsigned short i;

    for (i = 0; i < num_fds; i++) {
      if (i != fd)
	close(i);
    }
  }

  /* Reopen stdin, stdout and stderr over the tty file descriptor */
  dup(fd);			/* 0: stdin */
  dup(fd);			/* 1: stdout */
  dup(fd);			/* 2: stderr */

  if (fd > 2)
    close(fd);

  privileges(INVOKE);

#ifdef ultrix
  if ((fd = open("/dev/tty", O_RDONLY)) >= 0) {
    ioctl(fd, TIOCNOTTY, 0);
    close(fd);
  } else {
# ifdef NO_SETSID
    pid = setpgrp(0, 0);
# else
    pid = setsid();
# endif
    if (pid < 0) {
      D_TTYMODE(("%s: setsid() failed: %s, PID == %d\n", rs_name, strerror(errno), pid));
    }
  }

  /* no error, we could run with no tty to begin with */
#else /* ultrix */

# ifdef TIOCSCTTY
  ioctl(0, TIOCSCTTY, 0);
# endif

  /* set process group */
# if defined (_POSIX_VERSION) || defined (__svr4__)
  tcsetpgrp(0, pid);
# elif defined (TIOCSPGRP)
  ioctl(0, TIOCSPGRP, &pid);
# endif

  /* svr4 problems: reports no tty, no job control */
  /* # if !defined (__svr4__) && defined (TIOCSPGRP) */

  close(open(ttydev, O_RDWR, 0));
  /* # endif */
#endif /* ultrix */

  privileges(REVERT);

  return (fd);
}

/* debug_ttymode() */
#if DEBUG >= DEBUG_TTYMODE && defined(HAVE_TERMIOS_H)

  /* cpp token stringize doesn't work on all machines <sigh> */
#  define SHOW_TTY_FLAG(flag,name) do { \
                                     if ((ttymode->c_iflag) & (flag)) { \
                                       fprintf(stderr, "+%s ", name); \
                                     } else { \
                                       fprintf(stderr, "-%s ", name); \
                                     } \
                                   } while (0)
#  define SHOW_CONT_CHAR(entry, name) fprintf(stderr, "%s=%#3o ", name, ttymode->c_cc[entry])
static void
debug_ttymode(ttymode_t * ttymode)
{

  /* c_iflag bits */
  fprintf(stderr, "Input flags:  ");
  SHOW_TTY_FLAG(IGNBRK, "IGNBRK");
  SHOW_TTY_FLAG(BRKINT, "BRKINT");
  SHOW_TTY_FLAG(IGNPAR, "IGNPAR");
  SHOW_TTY_FLAG(PARMRK, "PARMRK");
  SHOW_TTY_FLAG(INPCK, "INPCK");
  SHOW_TTY_FLAG(ISTRIP, "ISTRIP");
  SHOW_TTY_FLAG(INLCR, "INLCR");
  SHOW_TTY_FLAG(IGNCR, "IGNCR");
  SHOW_TTY_FLAG(ICRNL, "ICRNL");
  SHOW_TTY_FLAG(IXON, "IXON");
  SHOW_TTY_FLAG(IXOFF, "IXOFF");
#  ifdef IUCLC
  SHOW_TTY_FLAG(IUCLC, "IUCLC");
#  endif
#  ifdef IXANY
  SHOW_TTY_FLAG(IXANY, "IXANY");
#  endif
#  ifdef IMAXBEL
  SHOW_TTY_FLAG(IMAXBEL, "IMAXBEL");
#  endif
  fprintf(stderr, "\n");

  fprintf(stderr, "Control character mappings:  ");
  SHOW_CONT_CHAR(VINTR, "VINTR");
  SHOW_CONT_CHAR(VQUIT, "VQUIT");
  SHOW_CONT_CHAR(VERASE, "VERASE");
  SHOW_CONT_CHAR(VKILL, "VKILL");
  SHOW_CONT_CHAR(VEOF, "VEOF");
  SHOW_CONT_CHAR(VEOL, "VEOL");
#  ifdef VEOL2
  SHOW_CONT_CHAR(VEOL2, "VEOL2");
#  endif
#  ifdef VSWTC
  SHOW_CONT_CHAR(VSWTC, "VSWTC");
#  endif
#  ifdef VSWTCH
  SHOW_CONT_CHAR(VSWTCH, "VSWTCH");
#  endif
  SHOW_CONT_CHAR(VSTART, "VSTART");
  SHOW_CONT_CHAR(VSTOP, "VSTOP");
  SHOW_CONT_CHAR(VSUSP, "VSUSP");
#  ifdef VDSUSP
  SHOW_CONT_CHAR(VDSUSP, "VDSUSP");
#  endif
#  ifdef VREPRINT
  SHOW_CONT_CHAR(VREPRINT, "VREPRINT");
#  endif
#  ifdef VDISCRD
  SHOW_CONT_CHAR(VDISCRD, "VDISCRD");
#  endif
#  ifdef VWERSE
  SHOW_CONT_CHAR(VWERSE, "VWERSE");
#  endif
#  ifdef VLNEXT
  SHOW_CONT_CHAR(VLNEXT, "VLNEXT");
#  endif
  fprintf(stderr, "\n\n");
}

#  undef SHOW_TTY_FLAG
#  undef SHOW_CONT_CHAR
#endif /* DEBUG_TTYMODE */

/* get_ttymode() */
static void
get_ttymode(ttymode_t * tio)
{
#ifdef HAVE_TERMIOS_H
  /*
   * standard System V termios interface
   */
  if (GET_TERMIOS(0, tio) < 0) {
    /* return error - use system defaults */
    tio->c_cc[VINTR] = CINTR;
    tio->c_cc[VQUIT] = CQUIT;
    tio->c_cc[VERASE] = CERASE;
    tio->c_cc[VKILL] = CKILL;
    tio->c_cc[VSTART] = CSTART;
    tio->c_cc[VSTOP] = CSTOP;
    tio->c_cc[VSUSP] = CSUSP;
# ifdef VDSUSP
    tio->c_cc[VDSUSP] = CDSUSP;
# endif
# ifdef VREPRINT
    tio->c_cc[VREPRINT] = CRPRNT;
# endif
# ifdef VDISCRD
    tio->c_cc[VDISCRD] = CFLUSH;
# endif
# ifdef VWERSE
    tio->c_cc[VWERSE] = CWERASE;
# endif
# ifdef VLNEXT
    tio->c_cc[VLNEXT] = CLNEXT;
# endif
  }
  tio->c_cc[VEOF] = CEOF;
  tio->c_cc[VEOL] = VDISABLE;
# ifdef VEOL2
  tio->c_cc[VEOL2] = VDISABLE;
# endif
# ifdef VSWTC
  tio->c_cc[VSWTC] = VDISABLE;
# endif
# ifdef VSWTCH
  tio->c_cc[VSWTCH] = VDISABLE;
# endif
# if VMIN != VEOF
  tio->c_cc[VMIN] = 1;
# endif
# if VTIME != VEOL
  tio->c_cc[VTIME] = 0;
# endif

  /* input modes */
  tio->c_iflag = (BRKINT | IGNPAR | ICRNL | IXON
# ifdef IMAXBEL
		  | IMAXBEL
# endif
      );

  /* output modes */
  tio->c_oflag = (OPOST | ONLCR);

  /* control modes */
  tio->c_cflag = (CS8 | CREAD);

  /* line discipline modes */
  tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK
# if defined (ECHOCTL) && defined (ECHOKE)
		  | ECHOCTL | ECHOKE
# endif
      );

  /*
   * guess an appropriate value for Backspace
   */

#if defined(FORCE_BACKSPACE) && 0
  PrivMode(1, PrivMode_BackSpace);
  tio->c_cc[VERASE] = '\b';	/* force ^H for stty setting...   */
  SET_TERMIOS(0, tio);		/*  ...and make it stick -- casey */
#elif defined(FORCE_DELETE) && 0
  PrivMode(0, PrivMode_BackSpace);
  tio->c_cc[VERASE] = 0x7f;	/* force ^? for stty setting...   */
  SET_TERMIOS(0, tio);		/*  ...and make it stick -- casey */
#else
  PrivMode((tio->c_cc[VERASE] == '\b'), PrivMode_BackSpace);
#endif

#else /* HAVE_TERMIOS_H */

  /*
   * sgtty interface
   */

  /* get parameters -- gtty */
  if (ioctl(0, TIOCGETP, &(tio->sg)) < 0) {
    tio->sg.sg_erase = CERASE;	/* ^H */
    tio->sg.sg_kill = CKILL;	/* ^U */
  }
  tio->sg.sg_flags = (CRMOD | ECHO | EVENP | ODDP);

  /* get special characters */
  if (ioctl(0, TIOCGETC, &(tio->tc)) < 0) {
    tio->tc.t_intrc = CINTR;	/* ^C */
    tio->tc.t_quitc = CQUIT;	/* ^\ */
    tio->tc.t_startc = CSTART;	/* ^Q */
    tio->tc.t_stopc = CSTOP;	/* ^S */
    tio->tc.t_eofc = CEOF;	/* ^D */
    tio->tc.t_brkc = -1;
  }
  /* get local special chars */
  if (ioctl(0, TIOCGLTC, &(tio->lc)) < 0) {
    tio->lc.t_suspc = CSUSP;	/* ^Z */
    tio->lc.t_dsuspc = CDSUSP;	/* ^Y */
    tio->lc.t_rprntc = CRPRNT;	/* ^R */
    tio->lc.t_flushc = CFLUSH;	/* ^O */
    tio->lc.t_werasc = CWERASE;	/* ^W */
    tio->lc.t_lnextc = CLNEXT;	/* ^V */
  }
  /* get line discipline */
  ioctl(0, TIOCGETD, &(tio->line));
# ifdef NTTYDISC
  tio->line = NTTYDISC;
# endif				/* NTTYDISC */
  tio->local = (LCRTBS | LCRTERA | LCTLECH | LPASS8 | LCRTKIL);

  /*
   * guess an appropriate value for Backspace
   */

# ifdef FORCE_BACKSPACE
  PrivMode(1, PrivMode_BackSpace);
  tio->sg.sg_erase = '\b';
  SET_TTYMODE(0, tio);
# elif defined (FORCE_DELETE)
  PrivMode(0, PrivMode_BackSpace);
  tio->sg.sg_erase = 0x7f;
  SET_TTYMODE(0, tio);
# else
  PrivMode((tio->sg.sg_erase == '\b'), PrivMode_BackSpace);
# endif

#endif /* HAVE_TERMIOS_H */
}

/* Xlocale */
XFontSet
create_fontset(const char *font1, const char *font2)
{
  XFontSet fontset = 0;
  char *fontname, **ml, *ds;
  int mc;
  const char fs_base[] = ",-misc-fixed-*-r-*-*-*-120-*-*-*-*-*-*";

  ASSERT(font1 != NULL);

#ifdef MULTI_CHARSET
  if (!font2) {
    font2 = "*";
  }
  fontname = MALLOC(strlen(font1) + strlen(font2) + sizeof(fs_base) + 2);
  if (fontname) {
    strcpy(fontname, font1);
    strcat(fontname, fs_base);
    strcat(fontname, ",");
    strcat(fontname, font2);
  }
#else
  fontname = MALLOC(strlen(font1) + sizeof(fs_base) + 1);
  if (fontname) {
    strcpy(fontname, font1);
    strcat(fontname, fs_base);
  }
#endif
  if (fontname) {
    setlocale(LC_ALL, "");
    fontset = XCreateFontSet(Xdisplay, fontname, &ml, &mc, &ds);
    FREE(fontname);
    if (mc) {
      XFreeStringList(ml);
      fontset = 0;
    }
  }
  return fontset;
}

#if defined(USE_XIM) || defined(MULTI_CHARSET)

#ifdef USE_XIM
static int xim_real_init(void);
# ifdef USE_X11R6_XIM
static void xim_destroy_cb(XIM xim, XPointer client_data,
			      XPointer call_data);
static void xim_instantiate_cb(Display *display, XPointer client_data,
				  XPointer call_data);
# endif
#endif

void
init_locale(void)
{
   char *locale = NULL;

   locale = setlocale(LC_CTYPE, "");
   TermWin.fontset = (XFontSet) -1;
   if (locale == NULL)
      print_error("Setting locale failed.");
   else {
#ifdef MULTI_CHARSET
      TermWin.fontset = create_fontset(rs_font[0], rs_mfont[0]);
#else
      TermWin.fontset = create_fontset(rs_font[0], (const char *) NULL);
#endif
#ifdef USE_XIM
# ifdef MULTI_CHARSET
      if (strcmp(locale, "C"))
# endif
      {
	if (xim_real_init() != -1)
	  return;

# ifdef USE_X11R6_XIM
	 XRegisterIMInstantiateCallback(Xdisplay, NULL, NULL, NULL,
					xim_instantiate_cb, NULL);
# endif
      }
#endif
   }
}
#endif	/* USE_XIM || MULTI_CHARSET */

#ifdef USE_XIM

static void
xim_set_size(XRectangle * size)
{
  size->x = TermWin.internalBorder;
  size->y = TermWin.internalBorder;
  size->width = Width2Pixel(TermWin.ncol);
  size->height = Height2Pixel(TermWin.nrow);
}

static void
xim_set_color(unsigned long *fg, unsigned long *bg)
{
  *fg = PixColors[fgColor];
  *bg = PixColors[bgColor];
}

static void
xim_send_spot(void)
{
  XPoint          spot;
  XVaNestedList   preedit_attr;
  XIMStyle        input_style;

  if (Input_Context == NULL)
    return;
  else {
    XGetICValues(Input_Context, XNInputStyle, &input_style, NULL);
    if (!(input_style & XIMPreeditPosition))
      return;
  }
  xim_get_position(&spot);

  preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);
  XSetICValues(Input_Context, XNPreeditAttributes, preedit_attr, NULL);
  XFree(preedit_attr);
}

static void
xim_get_area(XRectangle *preedit_rect, XRectangle *status_rect,
	     XRectangle *needed_rect)
{
  preedit_rect->x = needed_rect->width
		    + (scrollbar_visible() && !(Options & Opt_scrollBar_right)
		      ? (SB_WIDTH) : 0);
  preedit_rect->y = Height2Pixel(TermWin.nrow - 1);

  preedit_rect->width = Width2Pixel(TermWin.ncol + 1) - needed_rect->width
			+ (!(Options & Opt_scrollBar_right)
			  ? (SB_WIDTH) : 0);
  preedit_rect->height = Height2Pixel(1);

  status_rect->x = (scrollbar_visible() && !(Options & Opt_scrollBar_right))
		   ? (SB_WIDTH) : 0;
  status_rect->y = Height2Pixel(TermWin.nrow - 1);

  status_rect->width = needed_rect->width ? needed_rect->width
					  : Width2Pixel(TermWin.ncol + 1);
  status_rect->height = Height2Pixel(1);
}

#ifdef USE_X11R6_XIM
static void
xim_destroy_cb(XIM xim, XPointer client_data, XPointer call_data)
{
  Input_Context = NULL;
  XRegisterIMInstantiateCallback(Xdisplay, NULL, NULL, NULL,
				 xim_instantiate_cb, NULL);
}

static void
xim_instantiate_cb(Display *display, XPointer client_data,
		   XPointer call_data)
{
  xim_real_init();
  if (Input_Context)
    XUnregisterIMInstantiateCallback(Xdisplay, NULL, NULL, NULL,
				     xim_instantiate_cb, NULL);
}
#endif

static int
xim_real_init(void)
{
  char           *p, *s, buf[64], tmp[1024];
  char           *end, *next_s;
  XIM             xim = NULL;
  XIMStyle        input_style = 0;
  XIMStyles      *xim_styles = NULL;
  int             found;
  XPoint          spot;
  XRectangle      rect, status_rect, needed_rect;
  unsigned long   fg, bg;
  XVaNestedList   preedit_attr = NULL;
  XVaNestedList   status_attr = NULL;

  if (Input_Context)
    return 0;

  if (rs_inputMethod && *rs_inputMethod) {
    strncpy(tmp, rs_inputMethod, sizeof(tmp) - 1);
    for (s = tmp; *s; s = next_s + 1) {
      for (; *s && isspace(*s); s++) ;
      if (!*s)
        break;
      for (end = s; (*end && (*end != ',')); end++) ;
      for (next_s = end--; ((end >= s) && isspace(*end)); end--) ;
      *(end + 1) = '\0';

      if (*s) {
	snprintf(buf, sizeof(buf), "@im=%s", s);
	if ((p = XSetLocaleModifiers(buf)) != NULL && *p
	    && (xim = XOpenIM(Xdisplay, NULL, NULL, NULL)) != NULL)
          break;
      }
      if (!*next_s)
        break;
    }
  }

  /* try with XMODIFIERS env. var. */
  if (xim == NULL && (p = XSetLocaleModifiers("")) != NULL && *p)
    xim = XOpenIM(Xdisplay, NULL, NULL, NULL);

#ifndef USE_X11R6_XIM
  /* try with no modifiers base */
  if (xim == NULL && (p = XSetLocaleModifiers("@im=none")) != NULL && *p)
    xim = XOpenIM(Xdisplay, NULL, NULL, NULL);
#endif

  if (xim == NULL)
    xim = XOpenIM(Xdisplay, NULL, NULL, NULL);

  if (xim == NULL)
    return -1;

#ifdef USE_X11R6_XIM
  {
    XIMCallback destroy_cb;

    destroy_cb.callback = xim_destroy_cb;
    destroy_cb.client_data = NULL;
    if (XSetIMValues(xim, XNDestroyCallback, &destroy_cb, NULL))
      print_error("Could not set destroy callback to IM");
  }
#endif

  if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
      || !xim_styles) {
    print_error("input method doesn't support any style");
    XCloseIM(xim);
    return -1;
  }
  strncpy(tmp, (rs_preeditType ? rs_preeditType
		               : "OverTheSpot,OffTheSpot,Root"),
	  sizeof(tmp) - 1);
  for (found = 0, s = tmp; *s && !found; s = next_s + 1) {
    unsigned short  i;

    for (; *s && isspace(*s); s++) ;
    if (!*s)
      break;
    for (end = s; (*end && (*end != ',')); end++) ;
    for (next_s = end--; ((end >= s) && isspace(*end)); end--) ;
    *(end + 1) = '\0';

    if (!strcmp(s, "OverTheSpot"))
      input_style = (XIMPreeditPosition | XIMStatusNothing);
    else if (!strcmp(s, "OffTheSpot"))
      input_style = (XIMPreeditArea | XIMStatusArea);
    else if (!strcmp(s, "Root"))
      input_style = (XIMPreeditNothing | XIMStatusNothing);

    for (i = 0; i < xim_styles->count_styles; i++) {
      if (input_style == xim_styles->supported_styles[i]) {
	found = 1;
	break;
      }
    }
  }
  XFree(xim_styles);

  if (found == 0) {
    print_error("input method doesn't support my preedit type");
    XCloseIM(xim);
    return -1;
  }
  if ((input_style != (XIMPreeditNothing | XIMStatusNothing))
      && (input_style != (XIMPreeditArea | XIMStatusArea))
      && (input_style != (XIMPreeditPosition | XIMStatusNothing))) {
    print_error("This program does not support the preedit type");
    XCloseIM(xim);
    return -1;
  }
  if (input_style & XIMPreeditPosition) {
    xim_set_size(&rect);
    xim_get_position(&spot);
    xim_set_color(&fg, &bg);

    preedit_attr = XVaCreateNestedList(0, XNArea, &rect, 
				       XNSpotLocation, &spot,
				       XNForeground, fg,
				       XNBackground, bg,
				       XNFontSet, TermWin.fontset,
				       NULL);
  } else if (input_style & XIMPreeditArea) {
    xim_set_color(&fg, &bg);

    /* 
     * The necessary width of preedit area is unknown
     * until create input context.
     */
    needed_rect.width = 0;

    xim_get_area(&rect, &status_rect, &needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
				       XNForeground, fg,
				       XNBackground, bg,
				       XNFontSet, TermWin.fontset,
				       NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect,
				      XNForeground, fg,
				      XNBackground, bg,
				      XNFontSet, TermWin.fontset,
				      NULL);
  }

  Input_Context = XCreateIC(xim, XNInputStyle, input_style,
			    XNClientWindow, TermWin.parent,
			    XNFocusWindow, TermWin.parent,
			    preedit_attr ? XNPreeditAttributes : NULL,
			    preedit_attr,
			    status_attr ? XNStatusAttributes : NULL,
			    status_attr,
			    NULL);
  XFree(preedit_attr);
  XFree(status_attr);
  if (Input_Context == NULL) {
    print_error("Failed to create input context");
    XCloseIM(xim);
    return -1;
  }

  if (input_style & XIMPreeditArea)
    xim_set_status_position();
  return 0;
}

void
xim_set_status_position(void)
{
  XIMStyle        input_style;
  XRectangle      preedit_rect, status_rect, *needed_rect;
  XVaNestedList   preedit_attr, status_attr;

  if (Input_Context == NULL)
    return;

  XGetICValues(Input_Context, XNInputStyle, &input_style, NULL);

  if (input_style & XIMPreeditArea) {
    /* Getting the necessary width of preedit area */
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &needed_rect, NULL);
    XGetICValues(Input_Context, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);

    xim_get_area(&preedit_rect, &status_rect, needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &preedit_rect, NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect, NULL);

    XSetICValues(Input_Context,
		 XNPreeditAttributes, preedit_attr,
		 XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
  }
}

void xim_set_fontset(void)
{
  XIMStyle        input_style;
  XVaNestedList	  preedit_attr;
  XVaNestedList	  status_attr;

  if (Input_Context == NULL)
    return;

  XGetICValues(Input_Context, XNInputStyle, &input_style, NULL);

  if (input_style & (XIMPreeditArea | XIMPreeditPosition)) {
    preedit_attr = XVaCreateNestedList(0, XNFontSet, TermWin.fontset, NULL);
    status_attr = XVaCreateNestedList(0, XNFontSet, TermWin.fontset, NULL);

    XSetICValues(Input_Context,
		 XNPreeditAttributes, preedit_attr,
		 XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
  }
}
#endif /* USE_XIM */

/* run_command() */
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
int
run_command(char *argv[])
{

  ttymode_t tio;
  int ptyfd;

  /* Save and then give up any super-user privileges */
  privileges(IGNORE);

  ptyfd = get_pty();
  if (ptyfd < 0)
    return (-1);

  /* store original tty status for restoration clean_exit() -- rgg 04/12/95 */
  lstat(ttydev, &ttyfd_stat);
  D_CMD(("Original settings of %s are mode %o, uid %d, gid %d\n", ttydev, ttyfd_stat.st_mode,
	 ttyfd_stat.st_uid, ttyfd_stat.st_gid));

  /* install exit handler for cleanup */
#ifdef HAVE_ATEXIT
  atexit(clean_exit);
#else
# if defined (__sun__)
  on_exit(clean_exit, NULL);	/* non-ANSI exit handler */
# else
  print_error("no atexit(), UTMP entries can't be cleaned");
# endif
#endif

  /*
   * get tty settings before fork()
   * and make a reasonable guess at the value for BackSpace
   */
  get_ttymode(&tio);
  /* add Backspace value */
  SavedModes |= (PrivateModes & PrivMode_BackSpace);

  /* add value for scrollBar */
  if (scrollbar_visible()) {
    PrivateModes |= PrivMode_scrollBar;
    SavedModes |= PrivMode_scrollBar;
  }
#if DEBUG >= DEBUG_TTYMODE && defined(HAVE_TERMIOS_H)
  if (debug_level >= DEBUG_TTYMODE) {
    debug_ttymode(&tio);
  }
#endif

  /* spin off the command interpreter */
  signal(SIGHUP, Exit_signal);
#ifndef __svr4__
  signal(SIGINT, Exit_signal);
#endif
  signal(SIGQUIT, SegvHandler);
  signal(SIGTERM, Exit_signal);
  signal(SIGCHLD, Child_signal);
  signal(SIGSEGV, SegvHandler);
  signal(SIGBUS, SegvHandler);
  signal(SIGABRT, SegvHandler);
  signal(SIGFPE, SegvHandler);
  signal(SIGILL, SegvHandler);
  signal(SIGSYS, SegvHandler);

  /* need to trap SIGURG for SVR4 (Unixware) rlogin */
  /* signal (SIGURG, SIG_DFL); */

  D_CMD(("run_command(): forking\n"));
  cmd_pid = fork();
  D_CMD(("After fork(), cmd_pid == %d\n", cmd_pid));
  if (cmd_pid < 0) {
    print_error("fork(): %s", strerror(errno));
    return (-1);
  }
  if (cmd_pid == 0) {		/* child */

    /* signal (SIGHUP, Exit_signal); */
    /* signal (SIGINT, Exit_signal); */
#ifdef HAVE_UNSETENV
    /* avoid passing old settings and confusing term size */
    unsetenv("LINES");
    unsetenv("COLUMNS");
    /* avoid passing termcap since terminfo should be okay */
    unsetenv("TERMCAP");
#endif /* HAVE_UNSETENV */
    /* establish a controlling teletype for the new session */
    get_tty();

    /* initialize terminal attributes */
    SET_TTYMODE(0, &tio);

    /* become virtual console, fail silently */
    if (Options & Opt_console) {
      int fd = 1;

      privileges(INVOKE);
#ifdef SRIOCSREDIR
      fd = open(CONSOLE, O_WRONLY);
      if (fd < 0 || ioctl(fd, SRIOCSREDIR, 0) < 0) {
	if (fd >= 0)
	  close(fd);
      }
#elif defined(TIOCCONS)
      ioctl(0, TIOCCONS, &fd);
#endif /* SRIOCSREDIR */
      privileges(REVERT);
    }
    tt_winsize(0);		/* set window size */

    /* Permanently revoke all privileges for the child process.  
       Root shells for everyone are tres uncool.... ;^) -- mej */
#ifdef _HPUX_SOURCE
    setresuid(my_ruid, my_ruid, my_euid);
    setresgid(my_rgid, my_rgid, my_egid);
#else
    /* No special treatment is needed for systems with saved uids/gids,
       because the exec*() calls reset the saved uid/gid to the
       effective uid/gid                               -- mej */
# ifndef __CYGWIN32__
    setregid(my_rgid, my_rgid);
    setreuid(my_ruid, my_ruid);
# endif				/* __CYGWIN32__ */
#endif /* _HPUX_SOURCE */

    D_UTMP(("Child process reset\n"));
    my_euid = my_ruid;
    my_egid = my_rgid;

    /* reset signals and spin off the command interpreter */
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGSYS, SIG_DFL);
    signal(SIGALRM, SIG_DFL);

    /*
     * mimick login's behavior by disabling the job control signals
     * a shell that wants them can turn them back on
     */
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
#endif /* SIGTSTP */

    /* command interpreter path */
    D_CMD(("[%d] About to spawn shell\n", getpid()));
    if (chdir(initial_dir)) {
      print_warning("Unable to chdir to \"%s\" -- %s\n", initial_dir, strerror(errno));
    }
    if (argv != NULL) {
#if DEBUG >= DEBUG_CMD
      if (debug_level >= DEBUG_CMD) {
	int i;

	for (i = 0; argv[i]; i++) {
	  DPRINTF1(("argv[%d] = \"%s\"\n", i, argv[i]));
	}
      }
#endif
      execvp(argv[0], argv);
      print_error("execvp() failed, cannot execute \"%s\": %s", argv[0], strerror(errno));
    } else {

      const char *argv0, *shell;

      if ((shell = getenv("SHELL")) == NULL || *shell == '\0')
	shell = "/bin/sh";

      argv0 = my_basename(shell);
      if (Options & Opt_loginShell) {
	char *p = MALLOC((strlen(argv0) + 2) * sizeof(char));

	p[0] = '-';
	strcpy(&p[1], argv0);
	argv0 = p;
      }
      execlp(shell, argv0, NULL);
      print_error("execlp() failed, cannot execute \"%s\": %s", shell, strerror(errno));
    }
    sleep(3);			/* Sleep to make sure fork() returns in the parent, and so user can read error message */
    exit(EXIT_FAILURE);
  }
#ifdef UTMP_SUPPORT
  privileges(RESTORE);
  if (Options & Opt_utmpLogging)
    makeutent(ttydev, display_name);	/* stamp /etc/utmp */
  privileges(IGNORE);
#endif

#if 0
  D_THREADS(("run_command(): pthread_join(resize_sub_thr)\n"));
  pthread_join(resize_sub_thr, NULL);
#endif

  D_CMD(("run_command() returning\n"));
  return (ptyfd);
}

/* init_command() */
void
init_command(char *argv[])
{

  /* Initialize the command connection.        This should be called after
     the X server connection is established. */

  /* Enable delete window protocol */
  wmDeleteWindow = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(Xdisplay, TermWin.parent, &wmDeleteWindow, 1);

#ifdef OFFIX_DND
  /* Enable OffiX Dnd (drag 'n' drop) protocol */
  DndProtocol = XInternAtom(Xdisplay, "DndProtocol", False);
  DndSelection = XInternAtom(Xdisplay, "DndSelection", False);
#endif /* OFFIX_DND */

  init_locale();

  /* get number of available file descriptors */
#ifdef _POSIX_VERSION
  num_fds = sysconf(_SC_OPEN_MAX);
#else
  num_fds = getdtablesize();
#endif

#ifdef META8_OPTION
  meta_char = (Options & Opt_meta8 ? 0x80 : 033);
#endif

#ifdef GREEK_SUPPORT
  greek_init();
#endif

  Xfd = XConnectionNumber(Xdisplay);
  D_CMD(("Xfd = %d\n", Xfd));
  cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

  if ((cmd_fd = run_command(argv)) < 0) {
    print_error("aborting");
    exit(EXIT_FAILURE);
  }
}

/* window resizing */
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
void
tt_winsize(int fd)
{

  struct winsize ws;

  if (fd < 0)
    return;

  ws.ws_col = (unsigned short) TermWin.ncol;
  ws.ws_row = (unsigned short) TermWin.nrow;
#ifndef __CYGWIN32__
  ws.ws_xpixel = ws.ws_ypixel = 0;
#endif
  ioctl(fd, TIOCSWINSZ, &ws);
}

void
tt_resize(void)
{
  tt_winsize(cmd_fd);
}

/* attempt to `write' COUNT to the input buffer */
unsigned int
cmd_write(const unsigned char *str, unsigned int count)
{

  int n;

  n = (count - (cmdbuf_ptr - cmdbuf_base));
  /* need to insert more chars that space available in the front */
  if (n > 0) {
    /* try and get more space from the end */
    unsigned char *src, *dst;

    dst = (cmdbuf_base + sizeof(cmdbuf_base) - 1);	/* max pointer */

    if ((cmdbuf_ptr + n) > dst)
      n = (dst - cmdbuf_ptr);	/* max # chars to insert */

    if ((cmdbuf_endp + n) > dst)
      cmdbuf_endp = (dst - n);	/* truncate end if needed */

    /* equiv: memmove ((cmdbuf_ptr+n), cmdbuf_ptr, n); */
    src = cmdbuf_endp;
    dst = src + n;
    /* FIXME: anything special to avoid possible pointer wrap? */
    while (src >= cmdbuf_ptr)
      *dst-- = *src--;

    /* done */
    cmdbuf_ptr += n;
    cmdbuf_endp += n;
  }
  while (count-- && cmdbuf_ptr > cmdbuf_base) {
    /* sneak one in */
    cmdbuf_ptr--;
    *cmdbuf_ptr = str[count];
  }

  return (0);
}

#ifdef BACKGROUND_CYCLING_SUPPORT
RETSIGTYPE
check_pixmap_change(int sig)
{

  static time_t last_update = 0;
  time_t now;
  static unsigned long image_idx = 0;
  void (*old_handler) (int);
  static unsigned char in_cpc = 0;

  if (in_cpc)
    CPC_RETURN(0);
  in_cpc = 1;
  D_PIXMAP(("check_pixmap_change(%d):  rs_anim_delay == %lu seconds, last_update == %lu\n", sig, rs_anim_delay, last_update));
  if (!rs_anim_delay)
    CPC_RETURN(0);
  if (last_update == 0) {
    last_update = time(NULL);
    old_handler = signal(SIGALRM, check_pixmap_change);
    alarm(rs_anim_delay);
    in_cpc = 0;
    CPC_RETURN(0);
  }
  now = time(NULL);
  D_PIXMAP(("now %lu >= %lu (last_update %lu + rs_anim_delay %lu) ?\n", now, last_update + rs_anim_delay, last_update, rs_anim_delay));
  if (now >= last_update + rs_anim_delay || 1) {
    D_PIXMAP(("Time to update pixmap.  now == %lu\n", now));
    Imlib_destroy_image(imlib_id, images[image_bg].current->iml->im);
    images[image_bg].current->iml->im = NULL;
    xterm_seq(XTerm_Pixmap, rs_anim_pixmaps[image_idx++]);
    last_update = now;
    old_handler = signal(SIGALRM, check_pixmap_change);
    alarm(rs_anim_delay);
    if (rs_anim_pixmaps[image_idx] == NULL) {
      image_idx = 0;
    }
  }
  in_cpc = 0;
  if (old_handler) {
    CPC_RETURN((*old_handler) (sig));
  } else {
    CPC_RETURN(sig);
  }
}
#endif /* BACKGROUND_CYCLING_SUPPORT */

/* cmd_getc() - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
unsigned char
cmd_getc(void)
{

#define TIMEOUT_USEC 2500
  static short refreshed = 0;
  fd_set readfds;
  int retval;
  struct timeval value, *delay;

  /* If there has been a lot of new lines, then update the screen
   * What the heck I'll cheat and only refresh less than every page-full.
   * the number of pages between refreshes is refresh_limit, which
   * is incremented here because we must be doing flat-out scrolling.
   *
   * refreshing should be correct for small scrolls, because of the
   * time-out
   */
  if (refresh_count >= (refresh_limit * (TermWin.nrow - 1))) {
    if (refresh_limit < REFRESH_PERIOD)
      refresh_limit++;
    refresh_count = 0;
    refreshed = 1;
    D_CMD(("cmd_getc(): scr_refresh() #1\n"));
#ifdef PROFILE
    P_CALL(scr_refresh(refresh_type), "cmd_getc()->scr_refresh()");
#else
    scr_refresh(refresh_type);
#endif
  }
  /* characters already read in */
  if (CHARS_READ()) {
    RETURN_CHAR();
  }
  for (;;) {

    v_doPending();
    while (XPending(Xdisplay)) {	/* process pending X events */

      XEvent ev;

      refreshed = 0;
      XNextEvent(Xdisplay, &ev);

#ifdef USE_XIM
      if (!XFilterEvent(&ev, ev.xkey.window)) {
	event_dispatch(&ev);
      }
#else
      event_dispatch(&ev);
#endif

      /* in case button actions pushed chars to cmdbuf */
      if (CHARS_READ()) {
	RETURN_CHAR();
      }
    }

#ifdef SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
    if (scrollbar_isUp()) {
      if (!scroll_arrow_delay-- && scr_page(UP, 1)) {
	scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
	refreshed = 0;
      }
    } else if (scrollbar_isDn()) {
      if (!scroll_arrow_delay-- && scr_page(DN, 1)) {
	scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
	refreshed = 0;
      }
    }
#endif /* SCROLLBAR_BUTTON_CONTINUAL_SCROLLING */

    /* Nothing to do! */
    FD_ZERO(&readfds);
    FD_SET(cmd_fd, &readfds);
    FD_SET(Xfd, &readfds);
    value.tv_usec = TIMEOUT_USEC;
    value.tv_sec = 0;

    if (refreshed
#ifdef SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	&& !(scrollbar_isUpDn())
#endif
	) {
      delay = NULL;
    } else {
      delay = &value;
    }
    retval = select(num_fds, &readfds, NULL, NULL, delay);

    /* See if we can read from the application */
    if (FD_ISSET(cmd_fd, &readfds)) {

      /*      unsigned int count = BUFSIZ; */
      register unsigned int count = CMD_BUF_SIZE;

      cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

      /* while (count > sizeof(cmdbuf_base) / 2) */
      while (count) {

	/*      int n = read(cmd_fd, cmdbuf_endp, count); */
	register int n = read(cmd_fd, cmdbuf_endp, count);

	if (n <= 0)
	  break;
	cmdbuf_endp += n;
	count -= n;
      }
      /* some characters read in */
      if (CHARS_BUFFERED()) {
	RETURN_CHAR();
      }
    }
    /* select statement timed out - better update the screen */

    if (retval == 0) {
      refresh_count = 0;
      refresh_limit = 1;
      if (!refreshed) {
	refreshed = 1;
	D_CMD(("cmd_getc(): scr_refresh() #2\n"));
	scr_refresh(refresh_type);
	if (scrollbar_visible())
	  scrollbar_show(1);
#ifdef USE_XIM
	xim_send_spot();
#endif
      }
    }
  }

  D_CMD(("cmd_getc() returning\n"));
  return (0);
}

/* tt_write(), tt_printf() - output to command */
/*
 * Send count characters directly to the command
 */
void
tt_write(const unsigned char *buf, unsigned int count)
{

  v_writeBig(cmd_fd, (char *) buf, count);

#if 0				/* Fixes the bug that hung Eterm when pasting a lot of stuff */
  while (count > 0) {
    int n = write(cmd_fd, buf, count);

    if (n > 0) {
      count -= n;
      buf += n;
    }
  }
#endif
}

/*
 * Send printf() formatted output to the command.
 * Only use for small ammounts of data.
 */
void
tt_printf(const unsigned char *fmt,...)
{
  static unsigned char buf[256];
  va_list arg_ptr;

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);
  tt_write(buf, strlen(buf));
}

#ifndef USE_POSIX_THREADS
/* Read and process output from the application */

void
main_loop(void)
{
  /*   int ch; */
  register int ch;

  D_CMD(("[%d] main_loop() called\n", getpid()));

#ifdef BACKGROUND_CYCLING_SUPPORT
  if (rs_anim_delay) {
    check_pixmap_change(0);
  }
#endif
  do {
    while ((ch = cmd_getc()) == 0);	/* wait for something */
    if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      /* Read a text string from the input buffer */
      int nlines = 0;

      /*           unsigned char * str; */
      register unsigned char *str;

      /*
       * point to the start of the string,
       * decrement first since already did get_com_char ()
       */
      str = --cmdbuf_ptr;
      while (cmdbuf_ptr < cmdbuf_endp) {

	ch = *cmdbuf_ptr++;
	if (ch >= ' ' || ch == '\t' || ch == '\r') {
	  /* nothing */
	} else if (ch == '\n') {
	  nlines++;
	  if (++refresh_count >= (refresh_limit * (TermWin.nrow - 1)))
	    break;
	} else {		/* unprintable */
	  cmdbuf_ptr--;
	  break;
	}
      }
      D_SCREEN(("Adding lines, str == 0x%08x, cmdbuf_ptr == 0x%08x, cmdbuf_endp == 0x%08x\n", str, cmdbuf_ptr,
		cmdbuf_endp));
      D_SCREEN(("Command buffer base == 0x%08x, length %lu, end at 0x%08x\n", cmdbuf_base, CMD_BUF_SIZE,
		cmdbuf_base + CMD_BUF_SIZE - 1));
      scr_add_lines(str, nlines, (cmdbuf_ptr - str));
    } else {
      switch (ch) {
# ifdef NO_VT100_ANS
	case 005:
	  break;
# else
	case 005:
	  tt_printf(VT100_ANS);
	  break;		/* terminal Status */
# endif
	case 007:
	  scr_bell();
	  break;		/* bell */
	case '\b':
	  scr_backspace();
	  break;		/* backspace */
	case 013:
	case 014:
	  scr_index(UP);
	  break;		/* vertical tab, form feed */
	case 016:
	  scr_charset_choose(1);
	  break;		/* shift out - acs */
	case 017:
	  scr_charset_choose(0);
	  break;		/* shift in - acs */
	case 033:
	  process_escape_seq();
	  break;
      }
    }
  } while (ch != EOF);
}
#endif

/* output a burst of any pending data from a paste... */
int
v_doPending(void)
{

  if (v_bufstr >= v_bufptr)
    return (0);
  v_writeBig(cmd_fd, NULL, 0);
  return (1);
}

/* Write data to the pty as typed by the user, pasted with the mouse,
 * or generated by us in response to a query ESC sequence.
 * Code stolen from xterm 
 */
void
v_writeBig(int f, char *d, int len)
{

  int written;
  int c = len;

  if (v_bufstr == NULL && len > 0) {

    v_buffer = malloc(len);
    v_bufstr = v_buffer;
    v_bufptr = v_buffer;
    v_bufend = v_buffer + len;
  }
  /*
   * Append to the block we already have.
   * Always doing this simplifies the code, and
   * isn't too bad, either.  If this is a short
   * block, it isn't too expensive, and if this is
   * a long block, we won't be able to write it all
   * anyway.
   */

  if (len > 0) {
    if (v_bufend < v_bufptr + len) {	/* we've run out of room */
      if (v_bufstr != v_buffer) {
	/* there is unused space, move everything down */
	/* possibly overlapping bcopy here */

	/* bcopy(v_bufstr, v_buffer, v_bufptr - v_bufstr); */
	memcpy(v_buffer, v_bufstr, v_bufptr - v_bufstr);
	v_bufptr -= v_bufstr - v_buffer;
	v_bufstr = v_buffer;
      }
      if (v_bufend < v_bufptr + len) {
	/* still won't fit: get more space */
	/* Don't use XtRealloc because an error is not fatal. */
	int size = v_bufptr - v_buffer;		/* save across realloc */

	v_buffer = realloc(v_buffer, size + len);
	if (v_buffer) {
	  v_bufstr = v_buffer;
	  v_bufptr = v_buffer + size;
	  v_bufend = v_bufptr + len;
	} else {
	  /* no memory: ignore entire write request */
	  print_error("cannot allocate buffer space\n");
	  v_buffer = v_bufstr;	/* restore clobbered pointer */
	  c = 0;
	}
      }
    }
    if (v_bufend >= v_bufptr + len) {	/* new stuff will fit */
      memcpy(v_bufptr, d, len);	/* bcopy(d, v_bufptr, len); */
      v_bufptr += len;
    }
  }
  /*
   * Write out as much of the buffer as we can.
   * Be careful not to overflow the pty's input silo.
   * We are conservative here and only write
   * a small amount at a time.
   *
   * If we can't push all the data into the pty yet, we expect write
   * to return a non-negative number less than the length requested
   * (if some data written) or -1 and set errno to EAGAIN,
   * EWOULDBLOCK, or EINTR (if no data written).
   *
   * (Not all systems do this, sigh, so the code is actually
   * a little more forgiving.)
   */

  if (v_bufptr > v_bufstr) {
    written = write(f, v_bufstr, v_bufptr - v_bufstr <= MAX_PTY_WRITE ?
		    v_bufptr - v_bufstr : MAX_PTY_WRITE);
    if (written < 0) {
      written = 0;
    }
    D_TTY(("v_writeBig(): Wrote %d characters\n", written));
    v_bufstr += written;
    if (v_bufstr >= v_bufptr)	/* we wrote it all */
      v_bufstr = v_bufptr = v_buffer;
  }
  /*
   * If we have lots of unused memory allocated, return it
   */
  if (v_bufend - v_bufptr > 1024) {	/* arbitrary hysteresis */
    /* save pointers across realloc */
    int start = v_bufstr - v_buffer;
    int size = v_bufptr - v_buffer;
    int allocsize = size ? size : 1;

    v_buffer = realloc(v_buffer, allocsize);
    if (v_buffer) {
      v_bufstr = v_buffer + start;
      v_bufptr = v_buffer + size;
      v_bufend = v_buffer + allocsize;
    } else {
      /* should we print a warning if couldn't return memory? */
      v_buffer = v_bufstr - start;	/* restore clobbered pointer */
    }
  }
}
