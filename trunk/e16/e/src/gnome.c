/*
 * Copyright (C) 2000 Carsten Haitzler, Geoff Harrison and various contributors
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

/* WIN_WM_NAME STRING - contains a string identifier for the WM's name */
#define XA_WIN_WM_NAME                     "_WIN_WM_NAME"

/* WIN_WM_NAME VERSION - contains a string identifier for the WM's version */
#define XA_WIN_WM_VERSION                  "_WIN_WM_VERSION"

/* WIN_AREA CARD32[2] contains the current desktop area X,Y */
#define XA_WIN_AREA                        "_WIN_AREA"

/* WIN_AREA CARD32[2] contains the current desktop area size WxH */
#define XA_WIN_AREA_COUNT                  "_WIN_AREA_COUNT"

/* array of atoms - atom being one of the following atoms */
#define XA_WIN_PROTOCOLS                   "_WIN_PROTOCOLS"

/* array of iocn in various sizes */
/* Type: array of CARD32 */
/*       first item is icon count (n) */
/*       second item is icon record length (in CARD32s) */
/*       this is followed by (n) icon records as follows */
/*           pixmap (XID) */
/*           mask (XID) */
/*           width (CARD32) */
/*           height (CARD32) */
/*           depth (of pixmap, mask is assumed to be of depth 1) (CARD32) */
/*           drawable (screen root drawable of pixmap) (XID) */
/*           ... additional fields can be added at the end of this list */
#define XA_WIN_ICONS                    "_WIN_ICONS"

/* WIN_WORKSPACE CARD32 contains the current desktop number */
#define XA_WIN_WORKSPACE                   "_WIN_WORKSPACE"
/* WIN_WORKSPACE_COUNT CARD32 contains the number of desktops */
#define XA_WIN_WORKSPACE_COUNT             "_WIN_WORKSPACE_COUNT"

/* WIN_WORKSPACE_NAMES StringList (Text Property) of workspace names */
/* unused by enlightenment */
#define XA_WIN_WORKSPACE_NAMES             "_WIN_WORKSPACE_NAMES"

/* ********** Don't use this.. iffy at best. *********** */
/* The available work area for client windows. The WM can set this and the WM */
/* and/or clients may change it at any time. If it is changed the WM and/or  */
/* clients should honor the changes. If this property does not exist a client */
/* or WM can create it. */
/*
 * CARD32              min_x;
 * CARD32              min_y;
 * CARD32              max_x;
 * CARD32              max_y;
 */
#define XA_WIN_WORKAREA                    "_WIN_WORKAREA"
/* array of 4 CARD32's */

/* This is a list of window id's the WM is currently managing - primarily */
/* for being able to have external "tasklist" apps */
#define XA_WIN_CLIENT_LIST                 "_WIN_CLIENT_LIST"
/* array of N XID's */

/*********************************************************/
/* Properties on client windows                          */
/*********************************************************/

/* The layer the window exists in */
/*      0 = Desktop */
/*      1 = Below */
/*      2 = Normal (default app layer) */
/*      4 = OnTop */
/*      6 = Dock (always on top - for panel) */
/* The app sets this alone, not the WM. If this property changes the WM */
/* should comply and change the appearance/behavior of the Client window */
/* if this hint does nto exist the WM Will create it ont he Client window */
#define WIN_LAYER_DESKTOP                0
#define WIN_LAYER_BELOW                  2
#define WIN_LAYER_NORMAL                 4
#define WIN_LAYER_ONTOP                  6
#define WIN_LAYER_DOCK                   8
#define WIN_LAYER_ABOVE_DOCK             10
#define WIN_LAYER_MENU                   12
#define XA_WIN_LAYER                     "_WIN_LAYER"
/* WIN_LAYER = CARD32 */

/* flags for the window's state. The WM will change these as needed when */
/* state changes. If the property contains info on client map, E will modify */
/* the windows state accordingly. if the Hint does not exist the WM will */
/* create it on the client window. 0 for the bit means off, 1 means on. */
/* unused (default) values are 0 */

/* removed Minimized - no explanation of what it really means - ambiguity */
/* should not be here if not clear */
#define WIN_STATE_STICKY          (1<<0)    /* everyone knows sticky */
#define WIN_STATE_RESERVED_BIT1   (1<<1)    /* removed minimize here */
#define WIN_STATE_MAXIMIZED_VERT  (1<<2)    /* window in maximized V state */
#define WIN_STATE_MAXIMIZED_HORIZ (1<<3)    /* window in maximized H state */
#define WIN_STATE_HIDDEN          (1<<4)    /* not on taskbar but window visible */
#define WIN_STATE_SHADED          (1<<5)    /* shaded (NeXT style) */
#define WIN_STATE_HID_WORKSPACE   (1<<6)    /* not on current desktop */
#define WIN_STATE_HID_TRANSIENT   (1<<7)    /* owner of transient is hidden */
#define WIN_STATE_FIXED_POSITION  (1<<8)    /* window is fixed in position even */
#define WIN_STATE_ARRANGE_IGNORE  (1<<9)    /* ignore for auto arranging */
					 /* when scrolling about large */
					 /* virtual desktops ala fvwm */
#define XA_WIN_STATE              "_WIN_STATE"
/* WIN_STATE = CARD32 */

/* Preferences for behavior for app */
/* ONLY the client sets this */
#define WIN_HINTS_SKIP_FOCUS             (1<<0)	/* "alt-tab" skips this win */
#define WIN_HINTS_SKIP_WINLIST           (1<<1)	/* not in win list */
#define WIN_HINTS_SKIP_TASKBAR           (1<<2)	/* not on taskbar */
#define WIN_HINTS_GROUP_TRANSIENT        (1<<3)	/* ??????? */
#define WIN_HINTS_FOCUS_ON_CLICK         (1<<4)	/* app only accepts focus when clicked */
#define WIN_HINTS_DO_NOT_COVER           (1<<5)	/* attempt to not cover this window */
#define XA_WIN_HINTS                     "_WIN_HINTS"
/* WIN_HINTS = CARD32 */

/* Application state - also "color reactiveness" - the app can keep changing */
/* this property when it changes its state and the WM or monitoring program */
/* will pick this up and display somehting accordingly. ONLY the client sets */
/* this. */
#define WIN_APP_STATE_NONE                 0
#define WIN_APP_STATE_ACTIVE1              1
#define WIN_APP_STATE_ACTIVE2              2
#define WIN_APP_STATE_ERROR1               3
#define WIN_APP_STATE_ERROR2               4
#define WIN_APP_STATE_FATAL_ERROR1         5
#define WIN_APP_STATE_FATAL_ERROR2         6
#define WIN_APP_STATE_IDLE1                7
#define WIN_APP_STATE_IDLE2                8
#define WIN_APP_STATE_WAITING1             9
#define WIN_APP_STATE_WAITING2             10
#define WIN_APP_STATE_WORKING1             11
#define WIN_APP_STATE_WORKING2             12
#define WIN_APP_STATE_NEED_USER_INPUT1     13
#define WIN_APP_STATE_NEED_USER_INPUT2     14
#define WIN_APP_STATE_STRUGGLING1          15
#define WIN_APP_STATE_STRUGGLING2          16
#define WIN_APP_STATE_DISK_TRAFFIC1        17
#define WIN_APP_STATE_DISK_TRAFFIC2        18
#define WIN_APP_STATE_NETWORK_TRAFFIC1     19
#define WIN_APP_STATE_NETWORK_TRAFFIC2     20
#define WIN_APP_STATE_OVERLOADED1          21
#define WIN_APP_STATE_OVERLOADED2          22
#define WIN_APP_STATE_PERCENT000_1         23
#define WIN_APP_STATE_PERCENT000_2         24
#define WIN_APP_STATE_PERCENT010_1         25
#define WIN_APP_STATE_PERCENT010_2         26
#define WIN_APP_STATE_PERCENT020_1         27
#define WIN_APP_STATE_PERCENT020_2         28
#define WIN_APP_STATE_PERCENT030_1         29
#define WIN_APP_STATE_PERCENT030_2         30
#define WIN_APP_STATE_PERCENT040_1         31
#define WIN_APP_STATE_PERCENT040_2         32
#define WIN_APP_STATE_PERCENT050_1         33
#define WIN_APP_STATE_PERCENT050_2         34
#define WIN_APP_STATE_PERCENT060_1         35
#define WIN_APP_STATE_PERCENT060_2         36
#define WIN_APP_STATE_PERCENT070_1         37
#define WIN_APP_STATE_PERCENT070_2         38
#define WIN_APP_STATE_PERCENT080_1         39
#define WIN_APP_STATE_PERCENT080_2         40
#define WIN_APP_STATE_PERCENT090_1         41
#define WIN_APP_STATE_PERCENT090_2         42
#define WIN_APP_STATE_PERCENT100_1         43
#define WIN_APP_STATE_PERCENT100_2         44
#define XA_WIN_APP_STATE                   "_WIN_APP_STATE"
/* WIN_APP_STATE = CARD32 */

/* Expanded space occupied - this is the area on screen the app's window */
/* will occupy when "expanded" - ie if you have a button on an app that */
/* "hides" it by reducing its size, this is the geometry of the expanded */
/* window - so the window manager can allow for this when doign auto */
/* positioing of client windows assuming the app can at any point use this */
/* this area and thus try and keep it clear. ONLY the client sets this */
/*
 * CARD32              x;
 * CARD32              y;
 * CARD32              width;
 * CARD32              height;
 */
#define XA_WIN_EXPANDED_SIZE               "_WIN_EXPANDED_SIZE"
/* array of 4 CARD32's */

/* CARD32 that contians the desktop number the application is on If the */
/* application's state is "sticky" it is irrelevant. Only the WM should */
/* change this. */
#define XA_WIN_WORKSPACE                   "_WIN_WORKSPACE"

/* This atom is a 32-bit integer that is either 0 or 1 (currently). */
/* 0 denotes everything is as per usual but 1 denotes that ALL configure */
/* requests by the client on the client window with this property are */
/* not just a simple "moving" of the window, but the result of a user */
/* moving the window BUT the client handling that interaction by moving */
/* its own window. The window manager should respond accordingly by assuming */
/* any configure requests for this window whilst this atom is "active" in */
/* the "1" state are a client move and should handle flipping desktops if */
/* the window is being dragged "off screem" or across desktop boundaries */
/* etc. This atom is ONLY ever set by the client */
#define XA_WIN_CLIENT_MOVING               "_WIN_CLIENT_MOVING"
/* WIN_CLIENT_MOVING = CARD32 */

/* Designed for checking if the WIN_ supporting WM is still there  */
/* and kicking about - basically check this property - check the window */
/* ID it points to - then check that window Id has this property too */
/* if that is the case the WIN_ supporting WM is there and alive and the */
/* list of WIN_PROTOCOLS is valid */
#define XA_WIN_SUPPORTING_WM_CHECK         "_WIN_SUPPORTING_WM_CHECK"
/* CARD32 */

/*********************************************************/
/* How an app can modify things after mapping            */
/*********************************************************/

/* For a client to change layer or state it should send a client message */
/* to the root window as follows: */
/*
 * Display             *disp;
 * Window               root, client_window;
 * XClientMessageEvent  xev;
 * CARD32                new_layer;
 * 
 *     xev.type = ClientMessage;
 *     xev.window = client_window;
 *     xev.message_type = XInternAtom(disp, XA_WIN_LAYER, False);
 *     xev.format = 32;
 *     xev.data.l[0] = new_layer;
 *     xev.data.l[1] = CurrentTime;
 *     XSendEvent(disp, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 */
/*
 * Display             *disp;
 * Window               root, client_window;
 * XClientMessageEvent  xev;
 * CARD32               mask_of_members_to_change, new_members;
 * 
 *     xev.type = ClientMessage;
 *     xev.window = client_window;
 *     xev.message_type = XInternAtom(disp, XA_WIN_STATE, False);
 *     xev.format = 32;
 *     xev.data.l[0] = mask_of_members_to_change;
 *     xev.data.l[1] = new_members;
 *     xev.data.l[2] = CurrentTimep;
 *     XSendEvent(disp, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 */
/*
 * Display             *disp;
 * Window               root, client_window;
 * XClientMessageEvent  xev;
 * CARD32               new_desktop_number;
 * 
 *     xev.type = ClientMessage;
 *     xev.window = client_window;
 *     xev.message_type = XInternAtom(disp, XA_WIN_WORKSPACE, False);
 *     xev.format = 32;
 *     xev.data.l[0] = new_desktop_number;
 *     xev.data.l[2] = CurrentTimep;
 *     XSendEvent(disp, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 */

void
GNOME_GetHintIcons(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   CARD32             *retval;
   int                 size;
   unsigned int        i;
   Pixmap              pmap;
   Pixmap              mask;

   EDBUG(6, "GNOME_GetHintIcons");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_ICONS, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_PIXMAP, &size);
   if (retval)
   {
      for (i = 0; i < (size / (sizeof(CARD32))); i += 2)
      {
         pmap = retval[i];
         mask = retval[i + 1];
      }
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_GetHintLayer(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   CARD32             *retval;
   int                 size;

   EDBUG(6, "GNOME_GetHintLayer");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_LAYER, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      ewin->layer = *retval;
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_GetHintState(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   CARD32             *retval;
   int                 size;

   EDBUG(6, "GNOME_GetHintState");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_STATE, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      if (*retval & WIN_STATE_SHADED)
         ewin->shaded = 1;
      if (*retval & WIN_STATE_STICKY)
         ewin->sticky = 1;
      if (*retval & WIN_STATE_FIXED_POSITION)
         ewin->fixedpos = 1;
      if (*retval & WIN_STATE_ARRANGE_IGNORE)
         ewin->ignorearrange = 1;
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_GetHintAppState(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   unsigned char      *retval;
   int                 size;

   /* have nothing interesting to do with an app state (lamp) right now */
   EDBUG(6, "GNOME_GetHintAppState");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_APP_STATE, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_GetHintDesktop(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   unsigned char      *retval;
   int                 size;
   int                *desk;

   EDBUG(6, "GNOME_GetHintDesktop");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_WORKSPACE, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      desk = (int *)retval;
      ewin->desktop = *desk;
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_GetHint(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   int                *retval;
   int                 size;

   /* E doesn't really care about these hints right now */
   EDBUG(6, "GNOME_GetHint");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_HINTS, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      if (*retval & WIN_HINTS_SKIP_TASKBAR)
         ewin->skiptask = 1;
      if (*retval & WIN_HINTS_SKIP_FOCUS)
         ewin->skipfocus = 1;
      if (*retval & WIN_HINTS_SKIP_WINLIST)
         ewin->skipwinlist = 1;
      if (*retval & WIN_HINTS_FOCUS_ON_CLICK)
         ewin->focusclick = 1;
      if (*retval & WIN_HINTS_DO_NOT_COVER)
         ewin->never_use_area = 1;
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_SetHint(EWin * ewin)
{
   static Atom         atom_set = 0;
   int                 val;

   EDBUG(6, "GNOME_SetHint");
   if ((ewin->menu) || (ewin->pager))
      EDBUG_RETURN_;
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_STATE, False);
   val = 0;
   if (ewin->sticky)
      val |= WIN_STATE_STICKY;
   if (ewin->shaded)
      val |= WIN_STATE_SHADED;
   if (ewin->fixedpos)
      val |= WIN_STATE_FIXED_POSITION;
   XChangeProperty(disp, ewin->client.win, atom_set, XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)&val, 1);
   EDBUG_RETURN_;
}

void
GNOME_SetEwinArea(EWin * ewin)
{
   static Atom         atom_set = 0;
   CARD32              val[2];

   EDBUG(6, "GNOME_SetEwinArea");
   if ((ewin->menu) || (ewin->pager))
      EDBUG_RETURN_;
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_AREA, False);
   val[0] = (CARD32) ewin->area_x;
   val[1] = (CARD32) ewin->area_y;
   XChangeProperty(disp, ewin->client.win, atom_set, XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)val, 2);
   EDBUG_RETURN_;
}

void
GNOME_SetEwinDesk(EWin * ewin)
{
   static Atom         atom_set = 0;
   int                 val;

   EDBUG(6, "GNOME_SetEwinDesk");
   if ((ewin->menu) || (ewin->pager))
      EDBUG_RETURN_;
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_WORKSPACE, False);
   val = ewin->desktop;
   XChangeProperty(disp, ewin->client.win, atom_set, XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)&val, 1);
   EDBUG_RETURN_;
}

void
GNOME_GetExpandedSize(EWin * ewin, Atom atom_change)
{
   static Atom         atom_get = 0;
   CARD32             *retval;
   int                 size;

   EDBUG(6, "GNOME_GetExpandedSize");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!atom_get)
      atom_get = XInternAtom(disp, XA_WIN_EXPANDED_SIZE, False);
   if ((atom_change) && (atom_change != atom_get))
      EDBUG_RETURN_;
   retval = AtomGet(ewin->client.win, atom_get, XA_CARDINAL, &size);
   if (retval)
   {
      ewin->expanded_x = retval[0];
      ewin->expanded_y = retval[1];
      ewin->expanded_width = retval[2];
      ewin->expanded_height = retval[3];
      Efree(retval);
   }
   EDBUG_RETURN_;
}

void
GNOME_SetUsedHints(void)
{
   static Atom         atom_set = 0;
   Atom                list[10];

   EDBUG(6, "GNOME_SetUsedHints");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_PROTOCOLS, False);
   list[0] = XInternAtom(disp, XA_WIN_LAYER, False);
   list[1] = XInternAtom(disp, XA_WIN_STATE, False);
   list[2] = XInternAtom(disp, XA_WIN_HINTS, False);
   list[3] = XInternAtom(disp, XA_WIN_APP_STATE, False);
   list[4] = XInternAtom(disp, XA_WIN_EXPANDED_SIZE, False);
   list[5] = XInternAtom(disp, XA_WIN_ICONS, False);
   list[6] = XInternAtom(disp, XA_WIN_WORKSPACE, False);
   list[7] = XInternAtom(disp, XA_WIN_WORKSPACE_COUNT, False);
   list[8] = XInternAtom(disp, XA_WIN_WORKSPACE_NAMES, False);
   list[9] = XInternAtom(disp, XA_WIN_CLIENT_LIST, False);
   XChangeProperty(disp, root.win, atom_set, XA_ATOM, 32, PropModeReplace,
                   (unsigned char *)list, 10);
   EDBUG_RETURN_;
}

void
GNOME_SetCurrentArea(void)
{
   static Atom         atom_set = 0;
   CARD32              val[2];
   int                 ax, ay;

   EDBUG(6, "GNOME_SetCurrentArea");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_AREA, False);
   GetCurrentArea(&ax, &ay);
   val[0] = ax;
   val[1] = ay;
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)val, 2);
   EDBUG_RETURN_;
}

void
GNOME_SetCurrentDesk(void)
{
   static Atom         atom_set = 0;
   CARD32              val;

   EDBUG(6, "GNOME_SetCurrentDesk");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_WORKSPACE, False);
   val = (CARD32) desks.current;
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&val, 1);
   GNOME_SetCurrentArea();
   EDBUG_RETURN_;
}

void
GNOME_SetWMCheck(void)
{
   static Atom         atom_set = 0;
   CARD32              val;
   Window              win;

   EDBUG(6, "GNOME_SetWMCheck");

   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_SUPPORTING_WM_CHECK, False);
   win = ECreateWindow(root.win, -200, -200, 5, 5, 0);
   val = win;
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&val, 1);
   XChangeProperty(disp, win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&val, 1);
   EDBUG_RETURN_;
}

void
GNOME_SetDeskCount(void)
{
   static Atom         atom_set = 0;
   CARD32              val;

   EDBUG(6, "GNOME_SetDeskCount");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_WORKSPACE_COUNT, False);
   val = mode.numdesktops;
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)&val, 1);
   EDBUG_RETURN_;
}

void
GNOME_SetAreaCount(void)
{
   static Atom         atom_set = 0;
   int                 ax, ay;
   CARD32              val[2];

   EDBUG(6, "GNOME_SetAreaCount");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_AREA_COUNT, False);
   GetAreaSize(&ax, &ay);
   val[0] = ax;
   val[1] = ay;
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)val, 2);
   EDBUG_RETURN_;
}

void
GNOME_SetDeskNames(void)
{
   static Atom         atom_set = 0;
   XTextProperty       text;
   char                s[1024], *names[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
   int                 i;

   EDBUG(6, "GNOME_SetDeskNames");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_WORKSPACE_NAMES, False);
   for (i = 0; i < mode.numdesktops; i++)
   {
      Esnprintf(s, sizeof(s), "%i", i);
      names[i] = duplicate(s);
   }
   if (XStringListToTextProperty(names, mode.numdesktops, &text))
   {
      XSetTextProperty(disp, root.win, &text, atom_set);
      XFree(text.value);
   }
   for (i = 0; i < mode.numdesktops; i++)
      if (names[i])
         Efree(names[i]);
   EDBUG_RETURN_;
}

void
GNOME_SetClientList(void)
{
   static Atom         atom_set = 0;
   Window             *wl;
   int                 j, i, num;
   EWin              **lst;

   EDBUG(6, "GNOME_SetClientList");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_CLIENT_LIST, False);
   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   wl = NULL;
   j = 0;
   if (lst)
   {
      wl = Emalloc(sizeof(Window) * num);
      for (i = 0; i < num; i++)
      {
         if ((!lst[i]->menu) && (!lst[i]->pager) && (!lst[i]->skiptask)
             && lst[i]->iconified != 4)
            wl[j++] = lst[i]->client.win;
      }
   }
   XChangeProperty(disp, root.win, atom_set, XA_CARDINAL, 32, PropModeReplace,
                   (unsigned char *)wl, j);
   if (wl)
      Efree(wl);
   if (lst)
      Efree(lst);
   EDBUG_RETURN_;
}

void
GNOME_SetWMNameVer(void)
{
   static Atom         atom_set = 0, atom_set2 = 0;
   const char         *wm_name = "Enlightenment";
   const char         *wm_version = ENLIGHTENMENT_VERSION;

   EDBUG(6, "GNOME_SetWMNameVer");
   if (!atom_set)
      atom_set = XInternAtom(disp, XA_WIN_WM_NAME, False);
   XChangeProperty(disp, root.win, atom_set, XA_STRING, 8, PropModeReplace,
                   (unsigned char *)wm_name, strlen(wm_name));
   if (!atom_set2)
      atom_set2 = XInternAtom(disp, XA_WIN_WM_VERSION, False);
   XChangeProperty(disp, root.win, atom_set2, XA_STRING, 8, PropModeReplace,
                   (unsigned char *)wm_version, strlen(wm_version));
   EDBUG_RETURN_;
}

void
GNOME_DelHints(EWin * ewin)
{
   static Atom         atom_get[7] = { 0, 0, 0, 0, 0, 0, 0 };

   EDBUG(6, "GNOME_DelHints");
   if (!atom_get[0])
   {
      atom_get[0] = XInternAtom(disp, XA_WIN_WORKSPACE, False);
      atom_get[1] = XInternAtom(disp, XA_WIN_LAYER, False);
      atom_get[2] = XInternAtom(disp, XA_WIN_STATE, False);
      atom_get[3] = XInternAtom(disp, XA_WIN_HINTS, False);
      atom_get[4] = XInternAtom(disp, XA_WIN_APP_STATE, False);
      atom_get[5] = XInternAtom(disp, XA_WIN_WORKSPACE, False);
      atom_get[6] = XInternAtom(disp, XA_WIN_AREA, False);
   }
   XDeleteProperty(disp, ewin->client.win, atom_get[0]);
   XDeleteProperty(disp, ewin->client.win, atom_get[1]);
   XDeleteProperty(disp, ewin->client.win, atom_get[2]);
   XDeleteProperty(disp, ewin->client.win, atom_get[3]);
   XDeleteProperty(disp, ewin->client.win, atom_get[4]);
   XDeleteProperty(disp, ewin->client.win, atom_get[5]);
   XDeleteProperty(disp, ewin->client.win, atom_get[6]);
   EDBUG_RETURN_;
}

void
GNOME_GetHints(EWin * ewin, Atom atom_change)
{
   EDBUG(6, "GNOME_GetHints");
   GNOME_GetHintDesktop(ewin, atom_change);
   GNOME_GetHintIcons(ewin, atom_change);
   GNOME_GetHintLayer(ewin, atom_change);
   GNOME_GetHintState(ewin, atom_change);
   GNOME_GetHintAppState(ewin, atom_change);
   GNOME_GetHint(ewin, atom_change);
   GNOME_GetExpandedSize(ewin, atom_change);
   EDBUG_RETURN_;
}

void
GNOME_SetHints(void)
{
   EDBUG(6, "GNOME_SetHints");
   GNOME_SetWMNameVer();
   GNOME_SetUsedHints();
   GNOME_SetDeskCount();
   GNOME_SetDeskNames();
   GNOME_SetAreaCount();
   GNOME_SetWMCheck();
   EDBUG_RETURN_;
}
