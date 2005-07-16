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
 * Extended Window Manager Hints.
 */
#include "E.h"
#include "ecore-e16.h"
#include "ewins.h"

/*
 * _NET_WM_MOVERESIZE client message actions
 */
#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT     0
#define _NET_WM_MOVERESIZE_SIZE_TOP         1
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT    2
#define _NET_WM_MOVERESIZE_SIZE_RIGHT       3
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT 4
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM      5
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT  6
#define _NET_WM_MOVERESIZE_SIZE_LEFT        7
#define _NET_WM_MOVERESIZE_MOVE             8
#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD    9
#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD   10

/* Window state property change actions */
#define _NET_WM_STATE_REMOVE    0
#define _NET_WM_STATE_ADD       1
#define _NET_WM_STATE_TOGGLE    2

#ifndef ENABLE_HINTS_GNOME
Atom                _G_WIN_LAUER;
#endif

/*
 * Set/clear Atom in list
 */
static void
atom_list_set(Ecore_X_Atom * atoms, int size, int *count, Ecore_X_Atom atom,
	      int set)
{
   int                 i, n, in_list;

   n = *count;

   /* Check if atom is in list or not (+get index) */
   for (i = 0; i < n; i++)
      if (atoms[i] == atom)
	 break;
   in_list = i < n;

   if (set && !in_list)
     {
	/* Add it (if space left) */
	if (n < size)
	   atoms[n++] = atom;
	*count = n;
     }
   else if (!set && in_list)
     {
	/* Remove it */
	atoms[i] = atoms[--n];
	*count = n;
     }
}

/*
 * Initialize EWMH stuff
 */
void
EWMH_Init(Window win_wm_check)
{
   Ecore_X_Atom        atom_list[64];
   int                 atom_count;

#ifndef USE_ECORE_X
   ecore_x_netwm_init();
#endif

   atom_count = 0;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_SUPPORTED;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_DESKTOP_GEOMETRY;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_DESKTOP_NAMES;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_CURRENT_DESKTOP;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_DESKTOP_VIEWPORT;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WORKAREA;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_VIRTUAL_ROOTS;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_SHOWING_DESKTOP;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_ACTIVE_WINDOW;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_CLIENT_LIST;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_CLIENT_LIST_STACKING;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_CLOSE_WINDOW;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_MOVERESIZE_WINDOW;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_MOVERESIZE;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_NAME;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ICON_NAME;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_DESKTOP;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_MODAL;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_STICKY;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_SHADED;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_ABOVE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_BELOW;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_MOVE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_SHADE;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_STICK;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_ACTION_CLOSE;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STRUT_PARTIAL;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_STRUT;

   atom_list[atom_count++] = ECORE_X_ATOM_NET_FRAME_EXTENTS;
   atom_list[atom_count++] = ECORE_X_ATOM_NET_WM_WINDOW_OPACITY;

   ecore_x_window_prop_atom_set(VRoot.win, ECORE_X_ATOM_NET_SUPPORTED,
				atom_list, atom_count);

   /* Set WM info properties */
   ecore_x_netwm_wm_identify(VRoot.win, win_wm_check, e_wm_name);
}

/*
 * Desktops
 */

void
EWMH_SetDesktopCount(void)
{
   ecore_x_netwm_desk_count_set(VRoot.win, DesksGetNumber());
}

void
EWMH_SetDesktopRoots(void)
{
   int                 i, n_desks;
   Ecore_X_Window     *wl;

   n_desks = DesksGetNumber();
   wl = Emalloc(n_desks * sizeof(Ecore_X_Window));
   if (!wl)
      return;

   for (i = 0; i < n_desks; i++)
      wl[i] = DeskGetWin(i);

   ecore_x_netwm_desk_roots_set(VRoot.win, wl, n_desks);

   Efree(wl);
}

void
EWMH_SetDesktopNames(void)
{
   /* Fall back to defaults */
   ecore_x_netwm_desk_names_set(VRoot.win, NULL, DesksGetNumber());
}

void
EWMH_SetDesktopSize(void)
{
   int                 ax, ay;

   GetAreaSize(&ax, &ay);
   ecore_x_netwm_desk_size_set(VRoot.win, ax * VRoot.w, ay * VRoot.h);
}

void
EWMH_SetWorkArea(void)
{
   unsigned int       *p_coord;
   int                 n_coord, i, n_desks;

   n_desks = DesksGetNumber();
   n_coord = 4 * n_desks;
   p_coord = Emalloc(n_coord * sizeof(unsigned int));
   if (!p_coord)
      return;

   for (i = 0; i < n_desks; i++)
     {
	p_coord[4 * i] = 0;
	p_coord[4 * i + 1] = 0;
	p_coord[4 * i + 2] = VRoot.w;
	p_coord[4 * i + 3] = VRoot.h;
     }

   ecore_x_netwm_desk_workareas_set(VRoot.win, p_coord, n_desks);

   Efree(p_coord);
}

void
EWMH_SetCurrentDesktop(void)
{
   ecore_x_netwm_desk_current_set(VRoot.win, DesksGetCurrent());
}

void
EWMH_SetDesktopViewport(void)
{
   unsigned int       *p_coord;
   int                 n_coord, i, ax, ay, n_desks;

   n_desks = DesksGetNumber();
   n_coord = 2 * n_desks;
   p_coord = Emalloc(n_coord * sizeof(unsigned int));
   if (!p_coord)
      return;

   for (i = 0; i < n_desks; i++)
     {
	DeskGetArea(i, &ax, &ay);
	p_coord[2 * i] = ax * VRoot.w;
	p_coord[2 * i + 1] = ay * VRoot.h;
     }

   ecore_x_netwm_desk_viewports_set(VRoot.win, p_coord, n_desks);

   Efree(p_coord);
}

void
EWMH_SetShowingDesktop(int on)
{
   ecore_x_netwm_showing_desktop_set(VRoot.win, on);
}

/*
 * Window status
 */

void
EWMH_SetClientList(void)
{
   Ecore_X_Window     *wl;
   int                 i, num;
   EWin              **lst;

   /* Mapping order */
   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if (num > 0)
     {
	wl = Emalloc(num * sizeof(Ecore_X_Window));
	for (i = 0; i < num; i++)
	   wl[i] = lst[num - i - 1]->client.win;
	ecore_x_netwm_client_list_set(VRoot.win, wl, num);
	Efree(wl);
     }
   else
     {
	ecore_x_netwm_client_list_set(VRoot.win, NULL, 0);
     }
   if (lst)
      Efree(lst);
}

void
EWMH_SetClientStacking(void)
{
   Ecore_X_Window     *wl;
   int                 i, num;
   EWin               *const *lst;

   /* Stacking order */
   lst = EwinListStackGet(&num);
   if (num > 0)
     {
	wl = Emalloc(num * sizeof(Ecore_X_Window));
	for (i = 0; i < num; i++)
	   wl[i] = lst[num - i - 1]->client.win;
	ecore_x_netwm_client_list_stacking_set(VRoot.win, wl, num);
	Efree(wl);
     }
   else
     {
	ecore_x_netwm_client_list_stacking_set(VRoot.win, NULL, 0);
     }
}

void
EWMH_SetActiveWindow(Window win)
{
   static Window       win_last_set;

   if (win == win_last_set)
      return;

   ecore_x_netwm_client_active_set(VRoot.win, win);
   win_last_set = win;
}

/*
 * Functions that set X11-properties from E-window internals
 */

void
EWMH_SetWindowName(Window win, const char *name)
{
   const char         *str;

   str = EstrInt2Enc(name, 1);
   ecore_x_netwm_name_set(win, str);
   EstrInt2EncFree(str, 1);
}

void
EWMH_SetWindowDesktop(const EWin * ewin)
{
   unsigned int        val;

   if (EoIsSticky(ewin))
      val = 0xFFFFFFFF;
   else
      val = EoGetDesk(ewin);
   ecore_x_netwm_desktop_set(ewin->client.win, val);
}

void
EWMH_SetWindowState(const EWin * ewin)
{
   Ecore_X_Atom        atom_list[64];
   int                 len = sizeof(atom_list) / sizeof(Ecore_X_Atom);
   int                 atom_count;

   atom_count = 0;
   atom_list_set(atom_list, len, &atom_count, ECORE_X_ATOM_NET_WM_STATE_STICKY,
		 EoIsSticky(ewin));
   atom_list_set(atom_list, len, &atom_count, ECORE_X_ATOM_NET_WM_STATE_SHADED,
		 ewin->state.shaded);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR,
		 ewin->props.skip_ext_task);
   atom_list_set(atom_list, len, &atom_count, ECORE_X_ATOM_NET_WM_STATE_HIDDEN,
		 ewin->state.iconified || ewin->state.shaded);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT,
		 ewin->state.maximized_vert);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ,
		 ewin->state.maximized_horz);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN, ewin->state.fullscreen);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER,
		 ewin->props.skip_ext_pager);
   atom_list_set(atom_list, len, &atom_count, ECORE_X_ATOM_NET_WM_STATE_ABOVE,
		 EoGetLayer(ewin) >= 6);
   atom_list_set(atom_list, len, &atom_count, ECORE_X_ATOM_NET_WM_STATE_BELOW,
		 EoGetLayer(ewin) <= 2);
   atom_list_set(atom_list, len, &atom_count,
		 ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION,
		 ewin->state.attention);

   ecore_x_window_prop_atom_set(ewin->client.win, ECORE_X_ATOM_NET_WM_STATE,
				atom_list, atom_count);
}

void
EWMH_SetWindowBorder(const EWin * ewin)
{
   unsigned int        val[4];

   if (ewin->border)
     {
	val[0] = ewin->border->border.left;
	val[1] = ewin->border->border.right;
	val[2] = ewin->border->border.top;
	val[3] = ewin->border->border.bottom;
     }
   else
      val[0] = val[1] = val[2] = val[3] = 0;

   ecore_x_window_prop_card32_set(ewin->client.win,
				  ECORE_X_ATOM_NET_FRAME_EXTENTS, val, 4);
}

void
EWMH_SetWindowOpacity(const EWin * ewin)
{
   ecore_x_netwm_opacity_set(ewin->client.win, ewin->ewmh.opacity);
   ecore_x_netwm_opacity_set(EoGetWin(ewin), ewin->ewmh.opacity);
}

/*
 * Functions that set E-window internals from X11-properties
 */

static void
EWMH_GetWindowName(EWin * ewin)
{
   char               *val;

   _EFREE(ewin->ewmh.wm_name);

   ecore_x_netwm_name_get(ewin->client.win, &val);
   if (!val)
      return;
   ewin->ewmh.wm_name = EstrUtf82Int(val, 0);
   Efree(val);

   EwinChange(ewin, EWIN_CHANGE_NAME);
}

static void
EWMH_GetWindowIconName(EWin * ewin)
{
   char               *val;

   _EFREE(ewin->ewmh.wm_icon_name);

   ecore_x_netwm_icon_name_get(ewin->client.win, &val);
   if (!val)
      return;
   ewin->ewmh.wm_icon_name = EstrUtf82Int(val, 0);
   Efree(val);

   EwinChange(ewin, EWIN_CHANGE_ICON_NAME);
}

static void
EWMH_GetWindowDesktop(EWin * ewin)
{
   int                 num;
   unsigned int        desk;

   num = ecore_x_netwm_desktop_get(ewin->client.win, &desk);
   if (num <= 0)
      return;

   if (desk == 0xFFFFFFFF)
     {
	/* It is possible to distinguish between "sticky" and "on all desktops". */
	/* E doesn't */
	EoSetSticky(ewin, 1);
     }
   else
     {
	EoSetDesk(ewin, desk);
	EoSetSticky(ewin, 0);
     }
   EwinChange(ewin, EWIN_CHANGE_DESKTOP);
}

static void
EWMH_GetWindowState(EWin * ewin)
{
   Ecore_X_Atom       *p_atoms, atom;
   int                 i, n_atoms;

   n_atoms = ecore_x_window_prop_atom_list_get(ewin->client.win,
					       ECORE_X_ATOM_NET_WM_STATE,
					       &p_atoms);
   if (n_atoms <= 0)
      return;

   /* We must clear/set all according to not present/present */
/* EoSetSticky(ewin, 0); Do not override if set via _NET_WM_DESKTOP */
   ewin->state.shaded = 0;
   ewin->props.skip_ext_task = ewin->props.skip_ext_pager = 0;
   ewin->state.maximized_horz = ewin->state.maximized_vert = 0;
   ewin->state.fullscreen = ewin->state.attention = 0;
/* ewin->layer = No ... TBD */

   for (i = 0; i < n_atoms; i++)
     {
	atom = p_atoms[i];
	if (atom == ECORE_X_ATOM_NET_WM_STATE_STICKY)
	   EoSetSticky(ewin, 1);
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SHADED)
	   ewin->state.shaded = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR)
	   ewin->props.skip_ext_task = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER)
	   ewin->props.skip_ext_pager = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_HIDDEN)
	   ;			/* ewin->state.iconified = 1; No - WM_STATE does this */
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
	   ewin->state.maximized_vert = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
	   ewin->state.maximized_horz = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN)
	   ewin->state.fullscreen = 1;
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_ABOVE)
	   EoSetLayer(ewin, 6);
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_BELOW)
	   EoSetLayer(ewin, 2);
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION)
	   ewin->state.attention = 1;
     }
   Efree(p_atoms);
}

static void
EWMH_GetWindowType(EWin * ewin)
{
   Ecore_X_Atom       *p_atoms, atom;
   int                 n_atoms;

   n_atoms = ecore_x_window_prop_atom_list_get(ewin->client.win,
					       ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					       &p_atoms);
   if (n_atoms <= 0)
      return;

   atom = p_atoms[0];
   if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP)
     {
	EoSetLayer(ewin, 0);
	EoSetSticky(ewin, 1);
#if 0				/* Should be configurable */
	ewin->focusclick = 1;
#endif
	ewin->props.skip_focuslist = 1;
	ewin->props.fixedpos = 1;
	ewin->props.donthide = 1;
	EwinSetBorderByName(ewin, "BORDERLESS");
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK)
     {
	ewin->props.skip_ext_task = 1;
	ewin->props.skip_winlist = 1;
	ewin->props.skip_focuslist = 1;
	EoSetSticky(ewin, 1);
	ewin->props.never_use_area = 1;
	ewin->props.donthide = 1;
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY)
     {
	/* Epplets hit this */
	ewin->props.skip_ext_task = 1;
	ewin->props.skip_winlist = 1;
	ewin->props.skip_focuslist = 1;
	ewin->props.never_use_area = 1;
	ewin->props.donthide = 1;
     }
#if 0				/* Not used by E (yet?) */
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR)
     {
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU)
     {
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH)
     {
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG)
     {
     }
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL)
     {
     }
#endif
   Efree(p_atoms);
}

static void
EWMH_GetWindowIcons(EWin * ewin)
{
   unsigned int       *val;
   int                 i, num;

   if (ewin->ewmh.wm_icon)
     {
	Efree(ewin->ewmh.wm_icon);
	ewin->ewmh.wm_icon = NULL;
     }

   num = ecore_x_window_prop_card32_list_get(ewin->client.win,
					     ECORE_X_ATOM_NET_WM_ICON, &val);
   ewin->ewmh.wm_icon_len = num;
   if (num <= 0)
      return;

   if (num < (int)(2 + val[0] * val[1]))
     {
	Eprintf
	   ("*** EWMH_GetWindowIcons Icon data/size mismatch (ignoring): %s: N=%d WxH=%dx%d\n",
	    EwinGetName(ewin), num, val[0], val[1]);
	Efree(val);
	return;
     }

   for (i = 0; i < num - 1;)
     {
#if 0
	Eprintf("App %s: Icon %d-%4d - %dx%d\n",
		EwinGetName(ewin), num, i, val[i], val[i + 1]);
#endif
	i += 2 + val[i] * val[i + 1];
     }
   if (i != num)
      Eprintf
	 ("*** EWMH_GetWindowIcons Icon data/size mismatch: %s: %d!=%d\n",
	  EwinGetName(ewin), i, num);

   ewin->ewmh.wm_icon = val;

   EwinChange(ewin, EWIN_CHANGE_ICON_PMAP);
}

static void
EWMH_GetWindowMisc(EWin * ewin)
{
   int                 num;
   Ecore_X_Window      win;

   num = ecore_x_window_prop_window_get(ewin->client.win,
					ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
					&win, 1);
   if (num <= 0)
      return;

   ewin->props.vroot = 1;
   EoSetDesk(ewin, DesksGetCurrent());
}

static void
EWMH_GetWindowOpacity(EWin * ewin)
{
   int                 num;
   unsigned int        opacity;

   num = ecore_x_netwm_opacity_get(ewin->client.win, &opacity);
   if (num <= 0)
      return;

   if (ewin->ewmh.opacity == opacity)
      return;

   ewin->ewmh.opacity = opacity;

   /* Set frame window hint for xcompmgr */
   ecore_x_netwm_opacity_set(EoGetWin(ewin), opacity);

   EwinChange(ewin, EWIN_CHANGE_OPACITY);
}

static void
EWMH_GetWindowStrut(EWin * ewin)
{
   int                 num;
   unsigned int        val[12];

   num = ecore_x_window_prop_card32_get(ewin->client.win,
					ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, val,
					12);

   if (num < 4)
      num = ecore_x_window_prop_card32_get(ewin->client.win,
					   ECORE_X_ATOM_NET_WM_STRUT, val, 4);
   if (num < 4)
      return;

   ewin->strut.left = val[0];
   ewin->strut.right = val[1];
   ewin->strut.top = val[2];
   ewin->strut.bottom = val[3];
#if 0				/* FIXME - Handle in placement code */
   if (num < 12)
      return;
   ewin->strut.left_start_y = val[4];
   ewin->strut.left_end_y = val[5];
   ewin->strut.right_start_y = val[6];
   ewin->strut.right_end_y = val[7];
   ewin->strut.top_start_x = val[8];
   ewin->strut.top_end_x = val[9];
   ewin->strut.bottom_start_x = val[10];
   ewin->strut.bottom_end_x = val[11];
#endif
}

void
EWMH_SetWindowActions(const EWin * ewin)
{
   Ecore_X_Atom        aa[10];
   int                 num;

   num = 0;
   if (!ewin->state.inhibit_move)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_MOVE;
   if (!ewin->state.inhibit_resize)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
   if (!ewin->state.inhibit_iconify)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
   if (!ewin->state.inhibit_shade)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_SHADE;
   if (!ewin->state.inhibit_stick)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_STICK;
   if (!ewin->state.inhibit_max_hor)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
   if (!ewin->state.inhibit_max_ver)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
   if (!ewin->state.inhibit_fullscreeen)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
   if (!ewin->state.inhibit_change_desk)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
   if (!ewin->state.inhibit_close)
      aa[num++] = ECORE_X_ATOM_NET_WM_ACTION_CLOSE;

   ecore_x_window_prop_atom_set(ewin->client.win,
				ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, aa, num);
}

void
EWMH_GetWindowHints(EWin * ewin)
{
   EWMH_GetWindowMisc(ewin);
   EWMH_GetWindowOpacity(ewin);
   EWMH_GetWindowName(ewin);
   EWMH_GetWindowIconName(ewin);
   EWMH_GetWindowDesktop(ewin);
   EWMH_GetWindowState(ewin);
   EWMH_GetWindowType(ewin);
   EWMH_GetWindowIcons(ewin);
   EWMH_GetWindowStrut(ewin);
}

/*
 * Delete all (_NET_...) properties set on window
 */
void
EWMH_DelWindowHints(const EWin * ewin)
{
   XDeleteProperty(disp, ewin->client.win, ECORE_X_ATOM_NET_WM_DESKTOP);
   XDeleteProperty(disp, ewin->client.win, ECORE_X_ATOM_NET_WM_STATE);
}

/*
 * Process configuration requests from clients
 */
static int
do_set(int is_set, int action)
{
   switch (action)
     {
     case _NET_WM_STATE_REMOVE:
	return 0;
     case _NET_WM_STATE_ADD:
	return 1;
     case _NET_WM_STATE_TOGGLE:
	return !is_set;
     }
   return -1;
}

void
EWMH_ProcessClientMessage(XClientMessageEvent * ev)
{
   EWin               *ewin;

   /*
    * The ones that don't target an application window
    */
   if (ev->message_type == ECORE_X_ATOM_NET_CURRENT_DESKTOP)
     {
	DeskGoto(ev->data.l[0]);
	goto done;
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_DESKTOP_VIEWPORT)
     {
	SetCurrentArea(ev->data.l[0] / VRoot.w, ev->data.l[1] / VRoot.h);
	goto done;
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_SHOWING_DESKTOP)
     {
	EwinsShowDesktop(ev->data.l[0]);
	goto done;
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN)
     {
#if 0
	Eprintf
	   ("EWMH_ProcessClientMessage: ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN: %lx: %s\n",
	    ev->window, (char *)ev->data.l);
#endif
	goto done;
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_STARTUP_INFO)
     {
#if 0
	Eprintf
	   ("EWMH_ProcessClientMessage: ECORE_X_ATOM_NET_STARTUP_INFO      : %lx: %s\n",
	    ev->window, (char *)ev->data.l);
#endif
	goto done;
     }

   /*
    * The ones that do target an application window
    */
   ewin = EwinFindByClient(ev->window);
   if (ewin == NULL)
     {
	/* Some misbehaving clients go here */
	if (ev->message_type == ECORE_X_ATOM_NET_WM_DESKTOP)
	  {
	     ecore_x_netwm_desktop_set(ev->window, ev->data.l[0]);
	  }
	else if (ev->message_type == ECORE_X_ATOM_NET_WM_STATE)
	  {
	     ecore_x_window_prop_atom_list_change(ev->window,
						  ECORE_X_ATOM_NET_WM_STATE,
						  ev->data.l[1], ev->data.l[0]);
	     if (ev->data.l[2] ==
		 (long)ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ ||
		 ev->data.l[2] ==
		 (long)ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
		ecore_x_window_prop_atom_list_change(ev->window,
						     ECORE_X_ATOM_NET_WM_STATE,
						     ev->data.l[2],
						     ev->data.l[0]);
	  }
	goto done;
     }

   if (ev->message_type == ECORE_X_ATOM_NET_ACTIVE_WINDOW)
     {
	if (ewin->state.iconified)
	   EwinDeIconify(ewin);
	RaiseEwin(ewin);
	if (ewin->state.shaded)
	   EwinUnShade(ewin);
	FocusToEWin(ewin, FOCUS_SET);
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_CLOSE_WINDOW)
     {
	EwinOpClose(ewin);
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_WM_DESKTOP)
     {
	if ((unsigned)ev->data.l[0] == 0xFFFFFFFF)
	  {
	     if (!EoIsSticky(ewin))
		EwinStick(ewin);
	  }
	else
	  {
	     if (EoIsSticky(ewin))
		EwinUnStick(ewin);
	     else
		EwinMoveToDesktop(ewin, ev->data.l[0]);
	  }
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_WM_STATE)
     {
	/*
	 * It is assumed(!) that only the MAXIMIZE H/V ones can be set
	 * in one message.
	 */
	int                 action;
	Atom                atom, atom2;

	action = ev->data.l[0];
	atom = ev->data.l[1];
	atom2 = ev->data.l[2];
	if (atom == ECORE_X_ATOM_NET_WM_STATE_STICKY)
	  {
	     action = do_set(EoIsSticky(ewin), action);
	     if (action)
		EwinStick(ewin);
	     else
		EwinUnStick(ewin);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SHADED)
	  {
	     action = do_set(ewin->state.shaded, action);
	     if (action)
		EwinShade(ewin);
	     else
		EwinUnShade(ewin);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR)
	  {
	     action = do_set(ewin->props.skip_ext_task, action);
	     ewin->props.skip_ext_task = action;
	     EWMH_SetWindowState(ewin);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER)
	  {
	     action = do_set(ewin->props.skip_ext_pager, action);
	     ewin->props.skip_ext_pager = action;
	     EWMH_SetWindowState(ewin);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT ||
		 atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
	  {
	     void                (*func) (EWin *, const char *);
	     int                 maxh, maxv;

	     maxh = ewin->state.maximized_horz;
	     maxv = ewin->state.maximized_vert;
	     if (atom2 == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT ||
		 atom2 == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
	       {
		  /* (ok - ok) */
		  func = MaxSize;
		  maxh = do_set(maxh, action);
		  maxv = do_set(maxv, action);
	       }
	     else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
	       {
		  func = MaxHeight;
		  maxv = do_set(maxv, action);
	       }
	     else
	       {
		  func = MaxWidth;
		  maxh = do_set(maxh, action);
	       }

	     if ((ewin->state.maximized_horz == maxh) &&
		 (ewin->state.maximized_vert == maxv))
		goto done;

	     if ((ewin->state.maximized_horz && !maxh) ||
		 (ewin->state.maximized_vert && !maxv))
		ewin->toggle = 1;
	     else
		ewin->toggle = 0;

	     func(ewin, "available");
	     EWMH_SetWindowState(ewin);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN)
	  {
	     action = do_set(ewin->state.fullscreen, action);
	     if (ewin->state.fullscreen == action)
		goto done;

	     EwinSetFullscreen(ewin, action);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_ABOVE)
	  {
	     action = do_set(EoGetLayer(ewin) >= 6, action);
	     if (action)
		EwinOpSetLayer(ewin, 6);
	     else
		EwinOpSetLayer(ewin, 4);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_BELOW)
	  {
	     action = do_set(EoGetLayer(ewin) <= 2, action);
	     if (action)
		EwinOpSetLayer(ewin, 2);
	     else
		EwinOpSetLayer(ewin, 4);
	  }
	else if (atom == ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION)
	  {
	     action = do_set(ewin->state.attention, action);
	     ewin->state.attention = action;
	     EWMH_SetWindowState(ewin);
	  }
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_MOVERESIZE_WINDOW)
     {
	int                 flags, grav, x, y, w, h;

	flags = ev->data.l[0];
	grav = flags & 0xf;
	if (grav == 0)
	   grav = ewin->client.grav;
	x = (flags & 0x100) ? ev->data.l[1] : EoGetX(ewin);
	y = (flags & 0x200) ? ev->data.l[2] : EoGetY(ewin);
	w = (flags & 0x400) ? ev->data.l[3] : ewin->client.w;
	h = (flags & 0x800) ? ev->data.l[4] : ewin->client.h;
	EwinMoveResizeWithGravity(ewin, x, y, w, h, grav);
     }
   else if (ev->message_type == ECORE_X_ATOM_NET_WM_MOVERESIZE)
     {
	switch (ev->data.l[2])
	  {
	  case _NET_WM_MOVERESIZE_SIZE_TOPLEFT:
	  case _NET_WM_MOVERESIZE_SIZE_TOP:
	  case _NET_WM_MOVERESIZE_SIZE_TOPRIGHT:
	  case _NET_WM_MOVERESIZE_SIZE_RIGHT:
	  case _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:
	  case _NET_WM_MOVERESIZE_SIZE_BOTTOM:
	  case _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT:
	  case _NET_WM_MOVERESIZE_SIZE_LEFT:
	     ActionResizeStart(ewin, 1, MODE_RESIZE);
	     break;
	  case _NET_WM_MOVERESIZE_MOVE:
	     ActionMoveStart(ewin, 1, 0, 0);
	     break;

	  case _NET_WM_MOVERESIZE_SIZE_KEYBOARD:
	     /* doResize(NULL); */
	     break;
	  case _NET_WM_MOVERESIZE_MOVE_KEYBOARD:
	     /* doMove(NULL); */
	     break;
	  }
     }

 done:
   ;
}

/*
 * Process received window property change
 */
void
EWMH_ProcessPropertyChange(EWin * ewin, Atom atom_change)
{
   if (atom_change == ECORE_X_ATOM_NET_WM_NAME)
      EWMH_GetWindowName(ewin);
   else if (atom_change == ECORE_X_ATOM_NET_WM_ICON_NAME)
      EWMH_GetWindowIconName(ewin);
   else if (atom_change == ECORE_X_ATOM_NET_WM_STRUT_PARTIAL ||
	    atom_change == ECORE_X_ATOM_NET_WM_STRUT)
      EWMH_GetWindowStrut(ewin);
   else if (atom_change == ECORE_X_ATOM_NET_WM_WINDOW_OPACITY)
      EWMH_GetWindowOpacity(ewin);
}
