/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
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
#include "E.h"
#include <X11/keysym.h>
#include <sys/time.h>

void
MapUnmap(int start)
{
   /* this function will map and unmap all the windows based on the progress
    * through the startup sequence
    */

   static Window      *wlist = NULL;
   Window              par;
   Window              rt;
   XWindowAttributes   attr;
   static unsigned int num = 0;
   int                 i;

   EDBUG(6, "MapUnmap");
   switch (start)
     {
     case 0:
	XQueryTree(disp, VRoot.win, &rt, &par, &wlist, &num);
	if (wlist)
	  {
	     for (i = 0; i < (int)num; i++)
	       {
		  if ((init_win_ext) && (init_win_ext == wlist[i]))
		    {
		       wlist[i] = 0;
		    }
		  else
		    {
		       XGetWindowAttributes(disp, wlist[i], &attr);
		       if (attr.map_state == IsUnmapped)
			 {
			    wlist[i] = 0;
			 }
		       else
			 {
			    EUnmapWindow(disp, wlist[i]);
			 }
		    }
	       }
	  }
	break;
     case 1:
	if (wlist)
	  {
	     for (i = 0; i < (int)num; i++)
	       {
		  if (wlist[i])
		    {
		       if (XGetWindowAttributes(disp, wlist[i], &attr))
			 {
			    if (attr.override_redirect)
			      {
				 if (init_win1)
				   {
				      XRaiseWindow(disp, init_win1);
				      XRaiseWindow(disp, init_win2);
				   }
				 if (init_win_ext)
				    XRaiseWindow(disp, init_win_ext);
				 EdgeWindowsShow();
				 RaiseProgressbars();
				 EMapWindow(disp, wlist[i]);
			      }
			    else
			      {
				 if (Mode.wm.exiting)
				    EMapWindow(disp, wlist[i]);
				 else
				    AddToFamily(wlist[i]);
			      }
			 }
		    }
	       }
	     XFree(wlist);
	     wlist = NULL;
	  }
	break;
     default:
	break;
     }
   EDBUG_RETURN_;
}

/*
 * This function sets up all of our connections to X
 */
void
SetupX(void)
{
   char                buf[128];

   EDBUG(6, "SetupX");

   /* In case we are going to fork, set up the master pid */
   Mode.wm.master = 1;
   Mode.wm.master_pid = getpid();

   /* Open a connection to the diplay nominated by the DISPLAY variable */
   if (!dstr)
      dstr = getenv("DISPLAY");
   if (!dstr)
      dstr = ":0";
   disp = XOpenDisplay(dstr);
   /* if cannot connect to display */
   if (!disp)
     {
	Alert(_("Enlightenment cannot connect to the display nominated by\n"
		"your shell's DISPLAY environment variable. You may set this\n"
		"variable to indicate which display name Enlightenment is to\n"
		"connect to. It may be that you do not have an Xserver already\n"
		"running to serve that Display connection, or that you do not\n"
		"have permission to connect to that display. Please make sure\n"
		"all is correct before trying again. Run an Xserver by running\n"
		"xdm or startx first, or contact your local system\n"
		"administrator, or Xserver vendor, or read the X, xdm and\n"
		"startx manual pages before proceeding.\n"));
	EExit(1);
     }

   VRoot.scr = DefaultScreen(disp);
   Mode.display.screens = ScreenCount(disp);

   Mode.wm.master_screen = VRoot.scr;

   /* Start up on multiple heads, if appropriate */
   if ((Mode.display.screens > 1) && (!Mode.wm.single))
     {
	int                 i;
	char                subdisplay[255];
	char               *dispstr;

	dispstr = DisplayString(disp);

	strcpy(subdisplay, DisplayString(disp));

	for (i = 0; i < Mode.display.screens; i++)
	  {
	     pid_t               pid;

	     if (i == VRoot.scr)
		continue;

	     pid = fork();
	     if (pid)
	       {
		  /* We are the master */
		  child_count++;
		  e_children =
		     Erealloc(e_children, sizeof(pid_t) * child_count);
		  e_children[child_count - 1] = pid;
	       }
	     else
	       {
		  /* We are a slave */
		  Mode.wm.master = 0;
		  VRoot.scr = i;
#ifdef SIGSTOP
		  kill(getpid(), SIGSTOP);
#endif
		  /* Find the point to concatenate the screen onto */
		  dispstr = strchr(subdisplay, ':');
		  if (NULL != dispstr)
		    {
		       dispstr = strchr(dispstr, '.');
		       if (NULL != dispstr)
			  *dispstr = '\0';
		    }
		  Esnprintf(subdisplay + strlen(subdisplay), 10, ".%d", i);
		  dstr = Estrdup(subdisplay);
		  disp = XOpenDisplay(dstr);
		  /* Terminate the loop as I am the child process... */
		  break;
	       }
	  }
     }

   Esetenv("DISPLAY", DisplayString(disp), 1);

   /* set up an error handler for then E would normally have fatal X errors */
   XSetErrorHandler((XErrorHandler) EHandleXError);
   /* set up a handler for when the X Connection goes down */
   XSetIOErrorHandler((XIOErrorHandler) HandleXIOError);

   /* Root defaults */
   RRoot.scr = DefaultScreen(disp);
   RRoot.win = DefaultRootWindow(disp);
   RRoot.vis = DefaultVisual(disp, RRoot.scr);
   RRoot.depth = DefaultDepth(disp, RRoot.scr);
   RRoot.cmap = DefaultColormap(disp, RRoot.scr);
   RRoot.w = DisplayWidth(disp, RRoot.scr);
   RRoot.h = DisplayHeight(disp, RRoot.scr);

   VRoot.win = RRoot.win;
   VRoot.vis = RRoot.vis;
   VRoot.depth = RRoot.depth;
   VRoot.cmap = RRoot.cmap;

   if (Mode.wm.window)
     {
	XSetWindowAttributes attr;
	XClassHint         *xch;
	XTextProperty       xtp;

	/* Running E in its own virtual root window */
	attr.backing_store = NotUseful;
	attr.override_redirect = False;
	attr.colormap = VRoot.cmap;
	attr.border_pixel = 0;
	attr.background_pixel = 0;
	attr.save_under = True;
	VRoot.win = XCreateWindow(disp, RRoot.win, 0, 0, VRoot.w, VRoot.h, 0,
				  VRoot.depth, InputOutput, VRoot.vis,
				  CWOverrideRedirect | CWSaveUnder |
				  CWBackingStore | CWColormap | CWBackPixel |
				  CWBorderPixel, &attr);

	xtp.encoding = XA_STRING;
	xtp.format = 8;
	xtp.value = (unsigned char *)("Enlightenment");
	xtp.nitems = strlen((char *)(xtp.value));
	XSetWMName(disp, VRoot.win, &xtp);
	xch = XAllocClassHint();
	xch->res_name = (char *)"Virtual-Root";
	xch->res_class = (char *)"Enlightenment";
	XSetClassHint(disp, VRoot.win, xch);
	XFree(xch);
     }
   else
     {
	/* Running E normally on the root window */
	VRoot.w = RRoot.w;
	VRoot.h = RRoot.h;
     }

   Esnprintf(buf, sizeof(buf), "%#lx", VRoot.win);
   Esetenv("ENL_WM_ROOT", buf, 1);

   /* initialise imlib */
   imlib_set_cache_size(2048 * 1024);
   imlib_set_font_cache_size(512 * 1024);
   imlib_set_color_usage(128);

   imlib_context_set_dither(1);

   imlib_context_set_display(disp);
   imlib_context_set_visual(VRoot.vis);
   imlib_context_set_colormap(VRoot.cmap);
   imlib_context_set_dither_mask(0);

   /* Initialise event handling */
   EventsInit();

   /* just in case - set them up again */
   /* set up an error handler for then E would normally have fatal X errors */
   XSetErrorHandler((XErrorHandler) EHandleXError);
   /* set up a handler for when the X Connection goes down */
   XSetIOErrorHandler((XIOErrorHandler) HandleXIOError);

   /* select all the root window events to start managing */
   Mode.wm.xselect = 1;
   XSelectInput(disp, VRoot.win,
		ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
		LeaveWindowMask | ButtonMotionMask | PropertyChangeMask |
		SubstructureRedirectMask | KeyPressMask | KeyReleaseMask |
		PointerMotionMask | ResizeRedirectMask |
		SubstructureNotifyMask);
   XSync(disp, False);
   Mode.wm.xselect = 0;

   /* warn, if necessary about X version problems */
   if (ProtocolVersion(disp) != 11)
     {
	AlertX(_("X server version error"), _("Ignore this error"), "",
	       _("Quit Enlightenment"),
	       _("WARNING:\n"
		 "This is not an X11 Xserver. It in fact talks the X%i protocol.\n"
		 "This may mean Enlightenment will either not function, or\n"
		 "function incorrectly. If it is later than X11, then your\n"
		 "server is one the author of Enlightenment neither have\n"
		 "access to, nor have heard of.\n"), ProtocolVersion(disp));
     }

   ICCCM_SetIconSizes();
   ICCCM_Focus(NULL);
   MWM_SetInfo();

   /* damn that bloody numlock stuff - ok I'd rather XFree got fixed to not */
   /* have it as a modifier and everyone have to write specific code to mask */
   /* it out - but well.... */
   /* ok under Xfree Numlock and Scollock are lock modifiers and we need */
   /* to hunt them down to mask them out - EVIL EVIL EVIL hack but needed */
   {
      XModifierKeymap    *mod;
      KeyCode             nl, sl;
      int                 i;
      int                 masks[8] = {
	 ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask,
	 Mod4Mask, Mod5Mask
      };

      mod = XGetModifierMapping(disp);
      nl = XKeysymToKeycode(disp, XK_Num_Lock);
      sl = XKeysymToKeycode(disp, XK_Scroll_Lock);
      if ((mod) && (mod->max_keypermod > 0))
	{
	   for (i = 0; i < (8 * mod->max_keypermod); i++)
	     {
		if ((nl) && (mod->modifiermap[i] == nl))
		   numlock_mask = masks[i / mod->max_keypermod];
		else if ((sl) && (mod->modifiermap[i] == sl))
		   scrollock_mask = masks[i / mod->max_keypermod];
	     }
	}
      mask_mod_combos[0] = 0;
      mask_mod_combos[1] = LockMask;
      mask_mod_combos[2] = numlock_mask;
      mask_mod_combos[3] = scrollock_mask;
      mask_mod_combos[4] = numlock_mask | scrollock_mask;
      mask_mod_combos[5] = LockMask | numlock_mask;
      mask_mod_combos[6] = LockMask | scrollock_mask;
      mask_mod_combos[7] = LockMask | numlock_mask | scrollock_mask;

      if (mod)
	 XFreeModifiermap(mod);
   }

   /* Now we're going to set a bunch of default settings in E - in case we
    * don't ever get to load a config file for some odd reason. */
   memset(&Conf, 0, sizeof(EConf));
   Conf.areas.nx = 2;
   Conf.areas.ny = 1;
   Conf.areas.wraparound = 0;
   Conf.autoraise.enable = 0;
   Conf.autoraise.delay = 0.5;
   Conf.backgrounds.hiquality = 1;
   Conf.backgrounds.user = 1;
   Conf.backgrounds.timeout = 240;
   Conf.desks.num = 2;
   Conf.desks.wraparound = 0;
   Conf.desks.dragdir = 2;
   Conf.desks.dragbar_width = 16;
   Conf.desks.dragbar_ordering = 1;
   Conf.desks.dragbar_length = 0;
   Conf.desks.slidein = 1;
   Conf.desks.slidespeed = 6000;
   Conf.dialogs.headers = 0;
   Conf.dock.dirmode = DOCK_DOWN;
   Conf.dock.startx = 0;
   Conf.dock.starty = 0;
   Conf.focus.mode = MODE_FOCUS_SLOPPY;
   Conf.focus.clickraises = 0;
   Conf.focus.transientsfollowleader = 1;
   Conf.focus.switchfortransientmap = 1;
   Conf.focus.all_new_windows_get_focus = 0;
   Conf.focus.new_transients_get_focus = 0;
   Conf.focus.new_transients_get_focus_if_group_focused = 1;
   Conf.focus.raise_on_next = 1;
   Conf.focus.warp_on_next = 0;
   Conf.warplist.raise_on_select = 1;
   Conf.warplist.warp_on_select = 0;
   Conf.group_config.iconify = 1;
   Conf.group_config.kill = 0;
   Conf.group_config.move = 1;
   Conf.group_config.raise = 0;
   Conf.group_config.set_border = 1;
   Conf.group_config.stick = 1;
   Conf.group_config.shade = 1;
   Conf.group_config.mirror = 1;
   Conf.pagers.enable = 1;
   Conf.pagers.zoom = 1;
   Conf.pagers.title = 1;
   Conf.pagers.hiq = 1;
   Conf.pagers.snap = 1;
   Conf.pagers.scanspeed = 10;
   Conf.group_swapmove = 1;
   Conf.pagers.sel_button = 2;
   Conf.pagers.win_button = 1;
   Conf.pagers.menu_button = 3;
   Conf.snap.enable = 1;
   Conf.snap.edge_snap_dist = 8;
   Conf.snap.screen_snap_dist = 32;
   Conf.tooltips.enable = 1;
   Conf.tooltips.delay = 0.5;
   Conf.tooltips.showroottooltip = 1;
#ifdef WITH_TARTY_WARP
   Conf.warplist.enable = 1;
#else
   Conf.warplist.enable = 0;
#endif /* WITH_TARTY_WARP */
   Conf.warplist.warpsticky = 1;
   Conf.warplist.warpshaded = 1;
   Conf.warplist.warpiconified = 0;
   Conf.warplist.warpfocused = 1;

   Conf.deskmode = MODE_NONE;
   Conf.movemode = 0;
   Conf.dockapp_support = 1;
   Conf.primaryicondir = ICON_RIGHT;
   Conf.resizemode = 1;
   Conf.geominfomode = 1;
   Conf.slidemode = 0;
   Conf.cleanupslide = 1;
   Conf.mapslide = 1;
   Conf.slidespeedmap = 6000;
   Conf.slidespeedcleanup = 8000;
   Conf.shadespeed = 8000;
   Conf.animate_shading = 1;
   Conf.sound = 1;
   Conf.button_move_resistance = 5;
   Conf.autosave = 1;
   Conf.memory_paranoia = 1;
   Conf.save_under = 0;
   Conf.menuslide = 0;
   Conf.menusonscreen = 1;
   Conf.warpmenus = 1;
   Conf.place.manual = 0;
   Conf.edge_flip_resistance = 15;

#ifdef ENABLE_THEME_TRANSPARENCY
   Conf.st_trans.menu = ICLASS_ATTR_BG;
   Conf.st_trans.menu_item = ICLASS_ATTR_BG;
   Conf.st_trans.tooltip = ICLASS_ATTR_GLASS;
   Conf.st_trans.widget = ICLASS_ATTR_BG;
   Conf.st_trans.hilight = ICLASS_ATTR_OPAQUE;
   Conf.st_trans.border = ICLASS_ATTR_BG;
   Conf.st_trans.iconbox = ICLASS_ATTR_BG;
   Conf.st_trans.dialog = ICLASS_ATTR_BG;
   Conf.st_trans.pager = ICLASS_ATTR_BG;
   Conf.st_trans.warplist = ICLASS_ATTR_BG;
#endif

   Conf.menukeys.left = XK_Left;
   Conf.menukeys.right = XK_Right;
   Conf.menukeys.up = XK_Up;
   Conf.menukeys.down = XK_Down;
   Conf.menukeys.escape = XK_Escape;
   Conf.menukeys.ret = XK_Return;

   ScreenInit();

   MenusInit();

   EDBUG_RETURN_;
}

static void         ChkDir(char *d);

static void
ChkDir(char *d)
{
   if (!isdir(d))
     {
	Alert(_("The directory %s is apparently not a directory\n"
		"This is a fatal condition.\n" "Please remove this file\n"), d);
	EExit(1);
     }
   if (!canexec(d))
     {
	Alert(_("Do not have execute access to %s\n"
		"This is a fatal condition.\n"
		"Please check the ownership and permissions of this\n"
		"directory and take steps to rectify this.\n"), d);
	EExit(1);
     }
   if (!canread(d))
     {
	Alert(_("Do not have read access to %s\n" "This is a fatal condition.\n"
		"Please check the ownership and permissions of this\n"
		"directory and take steps to rectify this.\n"), d);
	EExit(1);
     }
   if (!canwrite(d))
     {
	Alert(_("Do not have write access to %s\n"
		"This is a fatal condition.\n"
		"Please check the ownership and permissions of this\n"
		"directory and take steps to rectify this.\n"), d);
	EExit(1);
     }
}

void
SetupDirs()
{
   char                s[1024], ss[1024], *home;

   EDBUG(6, "SetupDirs");
   Esnprintf(s, sizeof(s), "%s", EDirUser());
   home = homedir(getuid());
   if (home)
     {
	ChkDir(home);
	Efree(home);
     }
   if (exists(s))
     {
	if (!isdir(s))
	  {
	     Esnprintf(ss, sizeof(ss), "%s.old", EDirUser());
	     E_mv(s, ss);
	     E_md(s);
	  }
	else
	   ChkDir(EDirUser());
     }
   else
      E_md(s);
   Esnprintf(s, sizeof(s), "%s/themes", EDirUser());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/backgrounds", EDirUser());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/cached", EDirUserCache());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/cached/img", EDirUserCache());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/cached/cfg", EDirUserCache());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/cached/bgsel", EDirUserCache());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   Esnprintf(s, sizeof(s), "%s/cached/pager", EDirUserCache());
   if (!exists(s))
      E_md(s);
   else
      ChkDir(s);
   EDBUG_RETURN_;
}

void
SetupEnv()
{
   char                s[1024];

   Esetenv("EVERSION", ENLIGHTENMENT_VERSION, 1);
   Esetenv("EROOT", EDirRoot(), 1);
   Esetenv("EBIN", EDirBin(), 1);
   Esnprintf(s, sizeof(s), "%i", getpid());
   Esetenv("EPID", s, 1);
   Esetenv("ETHEME", themepath, 1);
   Esetenv("ECONFDIR", EDirUser(), 1);
   Esetenv("ECACHEDIR", EDirUserCache(), 1);
}

Window
MakeExtInitWin(void)
{
   Display            *d2;
   Window              win;
   XGCValues           gcv;
   GC                  gc;
   Pixmap              pmap, mask;
   Atom                a;
   CARD32              val;
   int                 i;
   XSetWindowAttributes attr;

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("MakeExtInitWin\n");

   a = XInternAtom(disp, "ENLIGHTENMENT_RESTART_SCREEN", False);
   XSync(disp, False);
   if (fork())
     {
	UngrabX();

	if (EventDebug(EDBUG_TYPE_SESSION))
	   Eprintf("MakeExtInitWin - parent\n");

	for (;;)
	  {
	     Atom                aa;
	     int                 format_ret;
	     unsigned long       bytes_after, num_ret;
	     unsigned char      *puc;

	     /* Hack to give the child some space. Not foolproof. */
	     sleep(1);

	     puc = NULL;
	     XGetWindowProperty(disp, VRoot.win, a, 0, 0x7fffffff, True,
				XA_CARDINAL, &aa, &format_ret, &num_ret,
				&bytes_after, &puc);
	     XSync(disp, False);
	     if (puc)
	       {
		  win = *((Window *) puc);
		  XFree(puc);
		  break;
	       }
	  }
	fflush(stdout);

	if (EventDebug(EDBUG_TYPE_SESSION))
	   Eprintf("MakeExtInitWin - parent - %#lx\n", win);

	return win;
     }

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("MakeExtInitWin - child\n");

   /* on solairs connection stays up - close */
   XSetErrorHandler((XErrorHandler) NULL);
   XSetIOErrorHandler((XIOErrorHandler) NULL);

   SignalsRestore();

   d2 = XOpenDisplay(DisplayString(disp));
   close(ConnectionNumber(disp));
   XGrabServer(d2);

   imlib_set_cache_size(2048 * 1024);
   imlib_set_font_cache_size(512 * 1024);
   imlib_set_color_usage(128);

   imlib_context_set_display(d2);
   imlib_context_set_visual(DefaultVisual(d2, DefaultScreen(d2)));
   imlib_context_set_colormap(DefaultColormap(d2, DefaultScreen(d2)));

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.colormap = VRoot.cmap;
   attr.border_pixel = 0;
   attr.background_pixel = 0;
   attr.save_under = True;
   win = XCreateWindow(d2, VRoot.win, 0, 0, VRoot.w, VRoot.h, 0, VRoot.depth,
		       InputOutput, VRoot.vis,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixel | CWBorderPixel, &attr);
   pmap = ECreatePixmap(d2, win, VRoot.w, VRoot.h, VRoot.depth);
   gcv.subwindow_mode = IncludeInferiors;
   gc = XCreateGC(d2, win, GCSubwindowMode, &gcv);
   XCopyArea(d2, VRoot.win, pmap, gc, 0, 0, VRoot.w, VRoot.h, 0, 0);
   ESetWindowBackgroundPixmap(d2, win, pmap);
   EMapRaised(d2, win);
   EFreePixmap(d2, pmap);
   XFreeGC(d2, gc);
   val = win;
   a = XInternAtom(d2, "ENLIGHTENMENT_RESTART_SCREEN", False);
   XChangeProperty(d2, VRoot.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)&val, 1);
   XSelectInput(d2, win, StructureNotifyMask);
   XUngrabServer(d2);
   XSync(d2, False);

   {
      Window              w2, ww;
      char               *f, s[1024];
      Imlib_Image        *im;
      struct timeval      tv;
      int                 dd, x, y;
      unsigned int        mm;
      Cursor              cs = 0;
      XColor              cl;

      w2 = XCreateWindow(d2, win, 0, 0, 32, 32, 0, VRoot.depth, InputOutput,
			 VRoot.vis,
			 CWOverrideRedirect | CWBackingStore | CWColormap |
			 CWBackPixel | CWBorderPixel, &attr);
      pmap = ECreatePixmap(d2, w2, 16, 16, 1);
      gc = XCreateGC(d2, pmap, 0, &gcv);
      XSetForeground(d2, gc, 0);
      XFillRectangle(d2, pmap, gc, 0, 0, 16, 16);
      XFreeGC(d2, gc);
      mask = ECreatePixmap(d2, w2, 16, 16, 1);
      gc = XCreateGC(d2, mask, 0, &gcv);
      XSetForeground(d2, gc, 0);
      XFillRectangle(d2, mask, gc, 0, 0, 16, 16);
      XFreeGC(d2, gc);
      cs = XCreatePixmapCursor(d2, pmap, mask, &cl, &cl, 0, 0);
      XDefineCursor(d2, win, cs);
      XDefineCursor(d2, w2, cs);

      for (i = 1;; i++)
	{
	   if (i > 12)
	      i = 1;

	   Esnprintf(s, sizeof(s), "pix/wait%i.png", i);
	   if (EventDebug(EDBUG_TYPE_SESSION))
	      Eprintf("MakeExtInitWin - child %s\n", s);

	   f = FindFile(s);
	   im = NULL;
	   if (f)
	     {
		im = imlib_load_image(f);
		Efree(f);
	     }

	   if (im)
	     {
		imlib_context_set_image(im);
		imlib_context_set_drawable(w2);
		imlib_render_pixmaps_for_whole_image(&pmap, &mask);
		EShapeCombineMask(d2, w2, ShapeBounding, 0, 0, mask, ShapeSet);
		ESetWindowBackgroundPixmap(d2, w2, pmap);
		imlib_free_pixmap_and_mask(pmap);
		XClearWindow(d2, w2);
		XQueryPointer(d2, win, &ww, &ww, &dd, &dd, &x, &y, &mm);
		EMoveResizeWindow(d2, w2,
				  x - imlib_image_get_width() / 2,
				  y - imlib_image_get_height() / 2,
				  imlib_image_get_width(),
				  imlib_image_get_height());
		EMapWindow(d2, w2);
		imlib_free_image();
	     }
	   tv.tv_sec = 0;
	   tv.tv_usec = 50000;
	   select(0, NULL, NULL, NULL, &tv);
	   XSync(d2, False);
	}
   }
/*    {
 * XEvent              ev;
 * 
 * XSync(d2, False);
 * for (;;)
 * {
 * XNextEvent(d2, &ev);
 * }
 * } */

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("MakeExtInitWin - child exit\n");

   exit(0);
}

void
SetupUserInitialization(void)
{

   FILE               *f;
   char                file[FILEPATH_LEN_MAX];

   EDBUG(3, "SetupUserInitialization");

   Esnprintf(file, sizeof(file), "%s/.initialized", EDirUser());
   if (isfile(file))
     {
	Mode.firsttime = 0;
     }
   else
     {
	Mode.firsttime = 1;
	f = fopen(file, "w");
	fprintf(f, "Congratulations, you have run enlightenment before.\n");
	fprintf(f, "Removing this file and the *.menu files in this\n");
	fprintf(f, "directory will cause enlightenment to regenerate them\n");
	fprintf(f, "and run the documentation browser.\n");
	fclose(f);
	if (fork())
	   EDBUG_RETURN_;
	Esnprintf(file, sizeof(file), "exec %s/scripts/e_gen_menu", EDirRoot());
	execl(usershell(getuid()), usershell(getuid()), "-c", (char *)file,
	      NULL);
	exit(0);

     }

   EDBUG_RETURN_;

}
