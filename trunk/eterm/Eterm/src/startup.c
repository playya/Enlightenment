/*
 * Copyright (C) 1997-2000, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static const char cvs_ident[] = "$Id$";

#include "config.h"
#include "feature.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>

#include "../libmej/debug.h"	/* from libmej */
#include "../libmej/mem.h"
#include "../libmej/strings.h"
#include "debug.h"
#include "startup.h"
#include "actions.h"
#include "buttons.h"
#include "command.h"
#include "eterm_utmp.h"
#include "events.h"
#include "options.h"
#include "pixmap.h"
#include "screen.h"
#include "scrollbar.h"
#include "term.h"
#include "windows.h"

char *orig_argv0;

#ifdef PIXMAP_SUPPORT
/* Set to one in case there is no WM, or a lousy one
   that doesn't send the right events (*cough*
   Window Maker *cough*) -- mej */
short bg_needs_update = 1;
#endif
TermWin_t TermWin;
Display *Xdisplay;		/* display */
Colormap cmap;
unsigned int debug_level = 0;	/* Level of debugging information to display */
const char *display_name = NULL;
unsigned int colorfgbg;

int
eterm_bootstrap(int argc, char *argv[])
{

  int i;
  char *val;
  static char windowid_string[20], *display_string, *term_string;	/* "WINDOWID=\0" = 10 chars, UINT_MAX = 10 chars */
  orig_argv0 = argv[0];

  /* Security enhancements -- mej */
  putenv("IFS= \t\n");
  my_ruid = getuid();
  my_euid = geteuid();
  my_rgid = getgid();
  my_egid = getegid();
  privileges(REVERT);
  install_handlers();

  PABLO_START_TRACING();
  getcwd(initial_dir, PATH_MAX);

  /* Open display, get options/resources and create the window */
  if ((display_name = getenv("DISPLAY")) == NULL)
    display_name = ":0";

  /* This MUST be called before any other Xlib functions */

  get_initial_options(argc, argv);
  init_defaults();

#ifdef NEED_LINUX_HACK
  privileges(INVOKE);		/* xdm in new Linux versions requires ruid != root to open the display -- mej */
#endif
  Xdisplay = XOpenDisplay(display_name);
#ifdef NEED_LINUX_HACK
  privileges(REVERT);
#endif
  if (!Xdisplay) {
    print_error("can't open display %s", display_name);
    exit(EXIT_FAILURE);
  }
#if DEBUG >= DEBUG_X
  if (debug_level >= DEBUG_X) {
    XSetErrorHandler((XErrorHandler) abort);
  } else {
    XSetErrorHandler((XErrorHandler) xerror_handler);
  }
#else
  XSetErrorHandler((XErrorHandler) xerror_handler);
#endif

  if (Options & Opt_install) {
    cmap = XCreateColormap(Xdisplay, Xroot, Xvisual, AllocNone);
    XInstallColormap(Xdisplay, cmap);
  } else {
    cmap = Xcmap;
  }
  imlib_context_set_display(Xdisplay);
  imlib_context_set_visual(Xvisual);
  imlib_context_set_colormap(cmap);

  get_modifiers();  /* Set up modifier masks before parsing config files. */

  /* Initialize the parser */
  conf_init_subsystem();

  if ((theme_dir = conf_parse_theme(rs_theme, THEME_CFG, 1)) != NULL) {
    char *tmp;

    D_OPTIONS(("conf_parse_theme() returned \"%s\"\n", theme_dir));
    tmp = (char *) MALLOC(strlen(theme_dir) + sizeof("ETERM_THEME_ROOT=\0"));
    sprintf(tmp, "ETERM_THEME_ROOT=%s", theme_dir);
    putenv(tmp);
  }
  if ((user_dir = conf_parse_theme(rs_theme, (rs_config_file ? rs_config_file : USER_CFG), 0)) != NULL) {
    char *tmp;

    D_OPTIONS(("conf_parse_theme() returned \"%s\"\n", user_dir));
    tmp = (char *) MALLOC(strlen(user_dir) + sizeof("ETERM_USER_ROOT=\0"));
    sprintf(tmp, "ETERM_USER_ROOT=%s", user_dir);
    putenv(tmp);
  }

#if defined(PIXMAP_SUPPORT)
  if (rs_path || theme_dir || user_dir) {
    register unsigned long len;
    register char *tmp;

    len = strlen(initial_dir);
    if (rs_path) {
      len += strlen(rs_path) + 1;	/* +1 for the colon */
    }
    if (theme_dir) {
      len += strlen(theme_dir) + 1;
    }
    if (user_dir) {
      len += strlen(user_dir) + 1;
    }
    tmp = MALLOC(len + 1);	/* +1 here for the NUL */
    snprintf(tmp, len + 1, "%s%s%s%s%s%s%s", (rs_path ? rs_path : ""), (rs_path ? ":" : ""), initial_dir,
	     (theme_dir ? ":" : ""), (theme_dir ? theme_dir : ""), (user_dir ? ":" : ""), (user_dir ? user_dir : ""));
    tmp[len] = '\0';
    FREE(rs_path);
    rs_path = tmp;
    D_OPTIONS(("New rs_path set to \"%s\"\n", rs_path));
  }
#endif
  get_options(argc, argv);
  D_UTMP(("Saved real uid/gid = [ %d, %d ]  effective uid/gid = [ %d, %d ]\n", my_ruid, my_rgid, my_euid, my_egid));
  D_UTMP(("Now running with real uid/gid = [ %d, %d ]  effective uid/gid = [ %d, %d ]\n", getuid(), getgid(), geteuid(),
	  getegid()));

  post_parse();

#ifdef PREFER_24BIT
  cmap = DefaultColormap(Xdisplay, Xscreen);

  /*
   * If depth is not 24, look for a 24bit visual.
   */
  if (Xdepth != 24) {
    XVisualInfo vinfo;

    if (XMatchVisualInfo(Xdisplay, Xscreen, 24, TrueColor, &vinfo)) {
      Xdepth = 24;
      Xvisual = vinfo.visual;
      cmap = XCreateColormap(Xdisplay, RootWindow(Xdisplay, Xscreen),
			     Xvisual, AllocNone);
    }
  }
#endif

  process_colors();

  Create_Windows(argc, argv);
  scr_reset();			/* initialize screen */

  /* Initialize the scrollbar */
  scrollbar_init(szHint.width, szHint.height - bbar_calc_docked_height(BBAR_DOCKED));
  scrollbar_mapping(Options & Opt_scrollbar);

  /* Initialize the menu subsystem. */
  menu_init();

  if (buttonbar) {
    bbar_init(buttonbar, szHint.width);
  }

#if DEBUG >= DEBUG_X
  if (debug_level >= DEBUG_X) {
    XSynchronize(Xdisplay, True);
  }
#endif

#ifdef DISPLAY_IS_IP
  /* Fixup display_name for export over pty to any interested terminal
   * clients via "ESC[7n" (e.g. shells).  Note we use the pure IP number
   * (for the first non-loopback interface) that we get from
   * network_display().  This is more "name-resolution-portable", if you
   * will, and probably allows for faster x-client startup if your name
   * server is beyond a slow link or overloaded at client startup.  Of
   * course that only helps the shell's child processes, not us.
   *
   * Giving out the display_name also affords a potential security hole
   */

  val = display_name = network_display(display_name);
  if (val == NULL)
#endif /* DISPLAY_IS_IP */
    val = XDisplayString(Xdisplay);
  if (display_name == NULL)
    display_name = val;		/* use broken `:0' value */

  i = strlen(val);
  display_string = MALLOC(i + 9);

  sprintf(display_string, "DISPLAY=%s", val);
  sprintf(windowid_string, "WINDOWID=%u", (unsigned int) TermWin.parent);

  /* add entries to the environment:
   * DISPLAY:       X display name
   * WINDOWID:      X windowid of the window
   * COLORTERM:     Terminal supports color
   * COLORTERM_BCE: Terminal supports BCE
   * TERM:          Terminal type for termcap/terminfo
   */
  putenv(display_string);
  putenv(windowid_string);
  if (Xdepth <= 2) {
    putenv("COLORTERM=" COLORTERMENV "-mono");
    putenv("COLORTERM_BCE=" COLORTERMENV "-mono");
    putenv("TERM=" TERMENV);
  } else {
    if (rs_term_name != NULL) {
      i = strlen(rs_term_name);
      term_string = MALLOC(i + 6);

      sprintf(term_string, "TERM=%s", rs_term_name);
      putenv(term_string);
    } else {
#ifdef DEFINE_XTERM_COLOR
      if (Xdepth <= 2)
	putenv("TERM=" TERMENV);
      else
	putenv("TERM=" TERMENV "-color");
#else
      putenv("TERM=" TERMENV);
#endif
    }
    putenv("COLORTERM=" COLORTERMENV);
    putenv("COLORTERM_BCE=" COLORTERMENV);
  }
  putenv("ETERM_VERSION=" VERSION);

  D_CMD(("init_command()\n"));
  init_command(rs_execArgs);

  main_loop();

  return (EXIT_SUCCESS);
}
