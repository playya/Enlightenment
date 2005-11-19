/*
 * Copyright (C) 2003-2005 Kim Woelders
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
/*
 * Feeble attempt to collect hint stuff in one place
 */
#include "E.h"
#include "desktops.h"		/* Should not be here */
#include "ecore-e16.h"
#include "ewins.h"
#include "hints.h"
#include "xwin.h"

static Atom         ENL_WIN_DATA;
static Atom         ENL_WIN_BORDER;

/*
 * Functions that set X11-properties from E-internals
 */

void
HintsInit(void)
{
   Atom                atom;
   Window              win;

   win = ECreateWindow(VRoot.win, -200, -200, 5, 5, 0);

   ICCCM_Init();
   MWM_SetInfo();
#if ENABLE_GNOME
   GNOME_SetHints(win);
#endif
   EWMH_Init(win);
   atom = XInternAtom(disp, "ENLIGHTENMENT_VERSION", False);
   ecore_x_window_prop_string_set(VRoot.win, atom, e_wm_version);

   if (Mode.wm.window)
     {
	HintsSetWindowName(VRoot.win, "Enlightenment");
	HintsSetWindowClass(VRoot.win, "Virtual-Root", "Enlightenment");
     }

   Mode.hints.old_root_pmap = HintsGetRootPixmap(VRoot.win);

   ENL_WIN_DATA = XInternAtom(disp, "ENL_WIN_DATA", False);
   ENL_WIN_BORDER = XInternAtom(disp, "ENL_WIN_BORDER", False);
}

void
HintsSetRootHints(Window win __UNUSED__)
{
   /* Nothing done here for now */
}

void
HintsSetClientList(void)
{
#if ENABLE_GNOME
   GNOME_SetClientList();
#endif
   EWMH_SetClientList();
   EWMH_SetClientStacking();
}

void
HintsSetClientStacking(void)
{
   EWMH_SetClientStacking();
}

void
HintsSetDesktopConfig(void)
{
#if ENABLE_GNOME
   GNOME_SetDeskCount();
   GNOME_SetDeskNames();
#endif
   EWMH_SetDesktopCount();
   EWMH_SetDesktopRoots();
   EWMH_SetDesktopNames();
   EWMH_SetWorkArea();
}

void
HintsSetViewportConfig(void)
{
#if ENABLE_GNOME
   GNOME_SetAreaCount();
#endif
   EWMH_SetDesktopSize();
}

void
HintsSetCurrentDesktop(void)
{
#if ENABLE_GNOME
   GNOME_SetCurrentDesk();
#endif
   EWMH_SetCurrentDesktop();
   HintsSetDesktopViewport();
}

void
HintsSetDesktopViewport(void)
{
#if ENABLE_GNOME
   GNOME_SetCurrentArea();
#endif
   EWMH_SetDesktopViewport();
}

void
HintsSetActiveWindow(Window win)
{
   EWMH_SetActiveWindow(win);
}

void
HintsSetWindowName(Window win, const char *name)
{
   ecore_x_icccm_title_set(win, name);

   EWMH_SetWindowName(win, name);
}

void
HintsSetWindowClass(Window win, const char *name, const char *clss)
{
   XClassHint         *xch;

   xch = XAllocClassHint();
   xch->res_name = (char *)name;
   xch->res_class = (char *)clss;
   XSetClassHint(disp, win, xch);
   XFree(xch);
}

void
HintsSetWindowDesktop(const EWin * ewin)
{
#if ENABLE_GNOME
   GNOME_SetEwinDesk(ewin);
#endif
   EWMH_SetWindowDesktop(ewin);
}

void
HintsSetWindowArea(const EWin * ewin __UNUSED__)
{
#if ENABLE_GNOME
   GNOME_SetEwinArea(ewin);
#endif
}

void
HintsSetWindowState(const EWin * ewin)
{
#if ENABLE_GNOME
   GNOME_SetHint(ewin);
#endif
   EWMH_SetWindowState(ewin);
   EWMH_SetWindowActions(ewin);
}

void
HintsSetWindowOpacity(const EWin * ewin)
{
   EWMH_SetWindowOpacity(ewin);
}

void
HintsSetWindowBorder(const EWin * ewin)
{
   EWMH_SetWindowBorder(ewin);
}

/*
 * Functions that set E-internals from X11-properties
 */

void
HintsGetWindowHints(EWin * ewin)
{
#if ENABLE_GNOME
   GNOME_GetHints(ewin, 0);
#endif
   EWMH_GetWindowHints(ewin);
}

/*
 * Functions that delete X11-properties
 */

void
HintsDelWindowHints(const EWin * ewin)
{
#if ENABLE_GNOME
   GNOME_DelHints(ewin);
#endif
   EWMH_DelWindowHints(ewin);
}

/*
 * Functions processing received X11 messages
 */

void
HintsProcessPropertyChange(EWin * ewin, Atom atom_change)
{
   char               *name;

   name = XGetAtomName(disp, atom_change);
   if (name == NULL)
      return;

   if (!memcmp(name, "WM_", 3))
      ICCCM_ProcessPropertyChange(ewin, atom_change);
   else if (!memcmp(name, "_NET_", 5))
      EWMH_ProcessPropertyChange(ewin, atom_change);
#if 0				/* No! - ENABLE_GNOME */
   else if (!memcmp(name, "_WIN_", 5))
      GNOME_GetHints(ewin, atom_change);
#endif
   XFree(name);
}

void
HintsProcessClientMessage(XClientMessageEvent * event)
{
   char               *name;

   name = XGetAtomName(disp, event->message_type);
   if (name == NULL)
      return;

   if (!memcmp(name, "WM_", 3))
      ICCCM_ProcessClientMessage(event);
   else if (!memcmp(name, "_NET_", 5))
      EWMH_ProcessClientMessage(event);
#if ENABLE_GNOME
   else if (!memcmp(name, "_WIN_", 5))
      GNOME_ProcessClientMessage(event);
#endif
   XFree(name);
}

Pixmap
HintsGetRootPixmap(Window win)
{
   Atom                a = 0;
   Ecore_X_Pixmap      pm;
   int                 num;

   a = XInternAtom(disp, "_XROOTPMAP_ID", False);

   pm = None;
   num = ecore_x_window_prop_xid_get(win, a, XA_PIXMAP, &pm, 1);

   return pm;
}

void
HintsSetRootInfo(Window win, Pixmap pmap, unsigned int color)
{
   static Atom         a = 0, aa = 0;
   Ecore_X_Pixmap      pm;

   if (!a)
     {
	a = XInternAtom(disp, "_XROOTPMAP_ID", False);
	aa = XInternAtom(disp, "_XROOTCOLOR_PIXEL", False);
     }

   if (Conf.hints.set_xroot_info_on_root_window)
      win = VRoot.win;

   pm = pmap;
   ecore_x_window_prop_xid_set(win, a, XA_PIXMAP, &pm, 1);

   ecore_x_window_prop_card32_set(win, aa, &color, 1);
}

typedef union
{
   struct
   {
      unsigned            version:8;
      unsigned            rsvd:22;
      unsigned            docked:1;
      unsigned            iconified:1;
   } b;
   int                 all:32;
} EWinInfoFlags;

#define ENL_DATA_ITEMS      8
#define ENL_DATA_VERSION    0

void
EHintsSetInfo(const EWin * ewin)
{
   int                 c[ENL_DATA_ITEMS];
   EWinInfoFlags       f;

   if (EwinIsInternal(ewin))
      return;

   f.all = 0;
   f.b.version = ENL_DATA_VERSION;
   f.b.docked = ewin->state.docked;
   f.b.iconified = ewin->state.iconified;
   c[0] = f.all;

   c[1] = EwinFlagsEncode(ewin);

   c[2] = 0;

   c[3] = ewin->lx;
   c[4] = ewin->ly;
   c[5] = ewin->lw;
   c[6] = ewin->lh;
   c[7] = ewin->ll;

   ecore_x_window_prop_card32_set(_EwinGetClientXwin(ewin), ENL_WIN_DATA,
				  (unsigned int *)c, ENL_DATA_ITEMS);

   ecore_x_window_prop_string_set(_EwinGetClientXwin(ewin), ENL_WIN_BORDER,
				  ewin->normal_border->name);

   if (EventDebug(EDBUG_TYPE_SNAPS))
      Eprintf("Snap set einf  %#lx: %4d+%4d %4dx%4d: %s\n",
	      _EwinGetClientXwin(ewin), ewin->client.x, ewin->client.y,
	      ewin->client.w, ewin->client.h, EwinGetName(ewin));
}

void
EHintsGetInfo(EWin * ewin)
{
   char               *str;
   int                 num;
   int                 c[ENL_DATA_ITEMS + 1];
   EWinInfoFlags       f;

   if (EwinIsInternal(ewin))
      return;

   num = ecore_x_window_prop_card32_get(_EwinGetClientXwin(ewin), ENL_WIN_DATA,
					(unsigned int *)c, ENL_DATA_ITEMS + 1);
   if (num != ENL_DATA_ITEMS)
     {
#if 1				/* FIXME - Remove this after a while */
	num =
	   ecore_x_window_prop_card32_get(_EwinGetClientXwin(ewin),
					  XInternAtom(disp, "ENL_INTERNAL_DATA",
						      False),
					  (unsigned int *)c, 1);
	if (num > 0)
	  {
	     ewin->state.identified = 1;
	     ewin->client.grav = StaticGravity;
	     ewin->state.placed = 1;
	  }
#endif
	return;
     }

   ewin->state.identified = 1;
   ewin->client.grav = StaticGravity;
   ewin->state.placed = 1;

   f.all = c[0];
   if (f.b.version != ENL_DATA_VERSION)
      return;
   ewin->icccm.start_iconified = f.b.iconified;
   ewin->state.docked = f.b.docked;

   EwinFlagsDecode(ewin, c[1]);

   ewin->lx = c[3];
   ewin->ly = c[4];
   ewin->lw = c[5];
   ewin->lh = c[6];
   ewin->ll = c[7];

   str =
      ecore_x_window_prop_string_get(_EwinGetClientXwin(ewin), ENL_WIN_BORDER);
   if (str)
      EwinSetBorderByName(ewin, str);
   Efree(str);

   if (EventDebug(EDBUG_TYPE_SNAPS))
      Eprintf("Snap get einf  %#lx: %4d+%4d %4dx%4d: %s\n",
	      _EwinGetClientXwin(ewin), ewin->client.x, ewin->client.y,
	      ewin->client.w, ewin->client.h, EwinGetName(ewin));
}

void
EHintsSetDeskInfo(void)
{
   Atom                a;
   int                 i, ax, ay, n_desks;
   unsigned int       *c;

   n_desks = DesksGetNumber();
   if (n_desks <= 0)
      return;

   c = Emalloc(2 * n_desks * sizeof(unsigned int));
   if (!c)
      return;

   for (i = 0; i < n_desks; i++)
     {
	DeskGetArea(DeskGet(i), &ax, &ay);
	c[(i * 2)] = ax;
	c[(i * 2) + 1] = ay;
     }

   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   ecore_x_window_prop_card32_set(VRoot.win, a, c, 2 * n_desks);

   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   c[0] = DesksGetCurrentNum();
   ecore_x_window_prop_card32_set(VRoot.win, a, c, 1);

   Efree(c);
}

void
EHintsGetDeskInfo(void)
{
   Atom                a;
   unsigned int       *c;
   int                 num, i, n_desks;

   n_desks = DesksGetNumber();
   c = Emalloc(2 * n_desks * sizeof(unsigned int));
   if (!c)
      return;

   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   num = ecore_x_window_prop_card32_get(VRoot.win, a, c, 2 * n_desks);
   if (num > 0)
     {
	for (i = 0; i < (num / 2); i++)
	   DeskSetArea(DeskGet(i), c[(i * 2)], c[(i * 2) + 1]);
     }

   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   num = ecore_x_window_prop_card32_get(VRoot.win, a, c, 1);
   if (num > 0)
     {
	DesksSetCurrent(DeskGet(c[0]));
     }
   else
     {
	/* Used to test if we should run cmd_init */
	Mode.wm.session_start = 1;
     }

   Efree(c);
}

void
EHintsSetInfoOnAll(void)
{
   int                 i, num;
   EWin               *const *lst;

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("SetEInfoOnAll\n");

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
      if (!EwinIsInternal(lst[i]))
	 EHintsSetInfo(lst[i]);

   EHintsSetDeskInfo();
}
