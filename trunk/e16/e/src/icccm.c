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

#ifdef USE_ECORE_X		/* Shouldn't be here */
extern Atom         ECORE_X_ATOM_WM_PROTOCOLS;
extern Atom         ECORE_X_ATOM_WM_DELETE_WINDOW;
extern Atom         ECORE_X_ATOM_WM_TAKE_FOCUS;
#endif

#define FREE_AND_CLEAR(ptr) if (ptr) { Efree(ptr); ptr = NULL; }

static Atom         E_XA_WM_CHANGE_STATE = 0;
static Atom         E_XA_WM_NAME = 0;
static Atom         E_XA_WM_COLORMAP_WINDOWS = 0;
static Atom         E_XA_WM_NORMAL_HINTS = 0;
static Atom         E_XA_WM_CLASS = 0;
static Atom         E_XA_WM_COMMAND = 0;
static Atom         E_XA_WM_CLIENT_MACHINE = 0;
static Atom         E_XA_WM_ICON_NAME = 0;
static Atom         E_XA_WM_WINDOW_ROLE = 0;
static Atom         E_XA_WM_HINTS = 0;
static Atom         E_XA_WM_CLIENT_LEADER = 0;
static Atom         E_XA_WM_TRANSIENT_FOR = 0;

void
ICCCM_Init(void)
{
   E_XA_WM_CHANGE_STATE = XInternAtom(disp, "WM_CHANGE_STATE", False);
   E_XA_WM_NAME = XInternAtom(disp, "WM_NAME", False);
   E_XA_WM_COLORMAP_WINDOWS = XInternAtom(disp, "WM_COLORMAP_WINDOWS", False);
   E_XA_WM_NORMAL_HINTS = XInternAtom(disp, "WM_NORMAL_HINTS", False);
   E_XA_WM_CLASS = XInternAtom(disp, "WM_CLASS", False);
   E_XA_WM_COMMAND = XInternAtom(disp, "WM_COMMAND", False);
   E_XA_WM_CLIENT_MACHINE = XInternAtom(disp, "WM_CLIENT_MACHINE", False);
   E_XA_WM_ICON_NAME = XInternAtom(disp, "WM_ICON_NAME", False);
   E_XA_WM_WINDOW_ROLE = XInternAtom(disp, "WM_WINDOW_ROLE", False);
   E_XA_WM_HINTS = XInternAtom(disp, "WM_HINTS", False);
   E_XA_WM_CLIENT_LEADER = XInternAtom(disp, "WM_CLIENT_LEADER", False);
   E_XA_WM_TRANSIENT_FOR = XInternAtom(disp, "WM_TRANSIENT_FOR", False);

#ifndef USE_ECORE_X
   ecore_x_icccm_init();
#endif

   ICCCM_SetIconSizes();

   if (Mode.wm.window)
     {
	Atom                wm_props[1] = { ECORE_X_ATOM_WM_DELETE_WINDOW };
	XSetWMProtocols(disp, VRoot.win, wm_props, 1);
     }
}

void
ICCCM_ProcessClientMessage(XClientMessageEvent * event)
{
   EWin               *ewin;
   Atom                a;

   if (event->message_type == E_XA_WM_CHANGE_STATE)
     {
	ewin = FindItem(NULL, event->window, LIST_FINDBY_ID, LIST_TYPE_EWIN);
	if (ewin == NULL)
	   return;

	if (event->data.l[0] == IconicState)
	  {
	     EwinIconify(ewin);
	  }
     }
   else if (event->message_type == ECORE_X_ATOM_WM_PROTOCOLS)
     {
	a = event->data.l[0];
	if (a == ECORE_X_ATOM_WM_DELETE_WINDOW && event->window == VRoot.win)
	   SessionExit(NULL);
     }
}

char               *
e16_icccm_name_get(Window win)
{
   char               *name;
   XTextProperty       xtp;

   if (XGetWMName(disp, win, &xtp))
     {
	int                 items;
	char              **list;
	Status              s;

	if (xtp.format == 8)
	  {
	     s = XmbTextPropertyToTextList(disp, &xtp, &list, &items);
	     if ((s == Success) && (items > 0))
	       {
		  name = Estrdup(*list);
		  XFreeStringList(list);
	       }
	     else
	       {
		  name = Estrdup((char *)xtp.value);
	       }
	  }
	else
	  {
	     name = Estrdup((char *)xtp.value);
	  }
	XFree(xtp.value);
     }
   else
     {
	name = Estrdup("None");
     }

   return name;
}

void
ICCCM_GetTitle(EWin * ewin, Atom atom_change)
{
   EDBUG(6, "ICCCM_GetTitle");

   if (atom_change && atom_change != E_XA_WM_NAME)
      EDBUG_RETURN_;

   _EFREE(ewin->icccm.wm_name);

   ewin->icccm.wm_name = e16_icccm_name_get(ewin->client.win);

   EwinChange(ewin, EWIN_CHANGE_NAME);

   EDBUG_RETURN_;
}

void
ICCCM_GetColormap(EWin * ewin)
{
   XWindowAttributes   xwa;
   Window              win, *wlist;
   int                 num;

   EDBUG(6, "ICCCM_GetColormap");

   if (EwinIsInternal(ewin))
      EDBUG_RETURN_;

   win = ewin->client.win;
   wlist = AtomGet(win, E_XA_WM_COLORMAP_WINDOWS, XA_WINDOW, &num);
   if (wlist)
     {
	win = wlist[0];
	Efree(wlist);
     }

   ewin->client.cmap = 0;
   if (XGetWindowAttributes(disp, ewin->client.win, &xwa) && xwa.colormap)
      ewin->client.cmap = xwa.colormap;

   EDBUG_RETURN_;
}

void
ICCCM_Delete(EWin * ewin)
{
   if (EwinIsInternal(ewin))
     {
	EUnmapWindow(disp, ewin->client.win);
	return;
     }

   if (ewin->client.delete_window)
      ecore_x_icccm_delete_window_send(ewin->client.win, CurrentTime);
   else
      XKillClient(disp, ewin->client.win);
}

#if 0				/* Deprecated */
void
ICCCM_Save(EWin * ewin)
{
   if (EwinIsInternal(ewin))
      return;

   ecore_x_icccm_send_save_yourself(ewin->client.win);
}
#endif

void
ICCCM_Iconify(EWin * ewin)
{
   EUnmapWindow(disp, ewin->client.win);
   ecore_x_icccm_state_set_iconic(ewin->client.win);
   AddItem(ewin, "ICON", ewin->client.win, LIST_TYPE_ICONIFIEDS);
}

void
ICCCM_DeIconify(EWin * ewin)
{
   EMapWindow(disp, ewin->client.win);
   ecore_x_icccm_state_set_normal(ewin->client.win);
   RemoveItem("ICON", ewin->client.win, LIST_FINDBY_BOTH, LIST_TYPE_ICONIFIEDS);
}

void
ICCCM_Withdraw(EWin * ewin)
{
   /* We have a choice of deleting the WM_STATE property
    * or changing the value to Withdrawn. Since twm/fvwm does
    * it that way, we change it to Withdrawn.
    */
   ecore_x_icccm_state_set_withdrawn(ewin->client.win);

   XRemoveFromSaveSet(disp, ewin->client.win);
}

void
ICCCM_MatchSize(EWin * ewin)
{
   int                 w, h;
   int                 i, j;
   double              aspect;

   EDBUG(6, "ICCCM_MatchSize");

   w = ewin->client.w;
   h = ewin->client.h;

   if (w < ewin->client.width.min)
      w = ewin->client.width.min;
   if (w > ewin->client.width.max)
      w = ewin->client.width.max;
   if (h < ewin->client.height.min)
      h = ewin->client.height.min;
   if (h > ewin->client.height.max)
      h = ewin->client.height.max;
   if ((w > 0) && (h > 0))
     {
	w -= ewin->client.base_w;
	h -= ewin->client.base_h;
	if ((w > 0) && (h > 0))
	  {
	     aspect = ((double)w) / ((double)h);
	     if (Mode.mode == MODE_RESIZE_H)
	       {
		  if (aspect < ewin->client.aspect_min)
		     h = (int)((double)w / ewin->client.aspect_min);
		  if (aspect > ewin->client.aspect_max)
		     h = (int)((double)w / ewin->client.aspect_max);
	       }
	     else if (Mode.mode == MODE_RESIZE_V)
	       {
		  if (aspect < ewin->client.aspect_min)
		     w = (int)((double)h * ewin->client.aspect_min);
		  if (aspect > ewin->client.aspect_max)
		     w = (int)((double)h * ewin->client.aspect_max);
	       }
	     else
	       {
		  if (aspect < ewin->client.aspect_min)
		     w = (int)((double)h * ewin->client.aspect_min);
		  if (aspect > ewin->client.aspect_max)
		     h = (int)((double)w / ewin->client.aspect_max);
	       }
	     i = w / ewin->client.w_inc;
	     j = h / ewin->client.h_inc;
	     w = i * ewin->client.w_inc;
	     h = j * ewin->client.h_inc;
	  }
	w += ewin->client.base_w;
	h += ewin->client.base_h;
     }
   ewin->client.w = w;
   ewin->client.h = h;

   EDBUG_RETURN_;
}

void
ICCCM_Configure(EWin * ewin)
{
   XEvent              ev;
   int                 d;
   Window              child;

   EDBUG(6, "ICCCM_Configure");

   if (EwinIsInternal(ewin))
      EDBUG_RETURN_;

   XMoveResizeWindow(disp, ewin->client.win, 0, 0,
		     ewin->client.w, ewin->client.h);

   ev.type = ConfigureNotify;
   ev.xconfigure.display = disp;
   ev.xconfigure.event = ewin->client.win;
   ev.xconfigure.window = ewin->client.win;
   d = EoGetDesk(ewin);
   ev.xconfigure.x = DeskGetX(d) + ewin->client.x;
   ev.xconfigure.y = DeskGetY(d) + ewin->client.y;
   if (Mode.wm.window)
      XTranslateCoordinates(disp, VRoot.win, RRoot.win,
			    ev.xconfigure.x, ev.xconfigure.y,
			    &ev.xconfigure.x, &ev.xconfigure.y, &child);
   ev.xconfigure.width = ewin->client.w;
   ev.xconfigure.height = ewin->client.h;
   ev.xconfigure.border_width = 0;
   ev.xconfigure.above = EoGetWin(ewin);
   ev.xconfigure.override_redirect = False;
   XSendEvent(disp, ewin->client.win, False, StructureNotifyMask, &ev);

   EDBUG_RETURN_;
}

void
ICCCM_AdoptStart(EWin * ewin)
{
   Window              win = ewin->client.win;

   if (!EwinIsInternal(ewin))
      XAddToSaveSet(disp, win);
}

void
ICCCM_Adopt(EWin * ewin)
{
   Window              win = ewin->client.win;

   if (ewin->client.start_iconified)
      ecore_x_icccm_state_set_iconic(win);
   else
      ecore_x_icccm_state_set_normal(win);
}

void
ICCCM_Cmap(EWin * ewin)
{
   XWindowAttributes   xwa;
   Window             *wlist = NULL;
   int                 i, num;

   EDBUG(6, "ICCCM_Cmap");

   if (!ewin)
     {
	if (Mode.current_cmap)
	  {
	     XUninstallColormap(disp, Mode.current_cmap);
	     Mode.current_cmap = 0;
	  }
	EDBUG_RETURN_;
     }

   if (MenusActive())
      EDBUG_RETURN_;

   ICCCM_GetColormap(ewin);

   if (EwinIsInternal(ewin))
      EDBUG_RETURN_;

   if ((ewin->client.cmap) && (Mode.current_cmap != ewin->client.cmap))
     {
	wlist =
	   AtomGet(ewin->client.win, E_XA_WM_COLORMAP_WINDOWS, XA_WINDOW, &num);
	if (wlist)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (XGetWindowAttributes(disp, wlist[i], &xwa))
		    {
		       if (xwa.colormap != DefaultColormap(disp, VRoot.scr))
			 {
			    XInstallColormap(disp, xwa.colormap);
			    Mode.current_cmap = xwa.colormap;
			 }
		    }
	       }
	     Efree(wlist);
	     EDBUG_RETURN_;
	  }
	XInstallColormap(disp, ewin->client.cmap);
	Mode.current_cmap = ewin->client.cmap;
     }

   EDBUG_RETURN_;
}

void
ICCCM_Focus(EWin * ewin)
{
   EDBUG(6, "ICCCM_Focus");

   if (EventDebug(EDBUG_TYPE_FOCUS))
     {
	if (ewin)
	   Eprintf("ICCCM_Focus %#lx %s\n", ewin->client.win,
		   EwinGetName(ewin));
	else
	   Eprintf("ICCCM_Focus None\n");
     }

   if (!ewin)
     {
	XSetInputFocus(disp, VRoot.win, RevertToPointerRoot, CurrentTime);
	HintsSetActiveWindow(None);
	EDBUG_RETURN_;
     }

   if (ewin->client.take_focus)
     {
	ecore_x_icccm_take_focus_send(ewin->client.win, CurrentTime);
     }

   XSetInputFocus(disp, ewin->client.win, RevertToPointerRoot, CurrentTime);

   HintsSetActiveWindow(ewin->client.win);

   EDBUG_RETURN_;
}

void
ICCCM_GetGeoms(EWin * ewin, Atom atom_change)
{
   XSizeHints          hint;
   Window              ww;
   long                mask;
   unsigned int        dummy, w, h, bw;
   int                 x, y;

   EDBUG(6, "ICCCM_GetGeoms");

   if (atom_change && atom_change != E_XA_WM_NORMAL_HINTS)
      EDBUG_RETURN_;

   x = ewin->client.x;
   y = ewin->client.y;
   w = ewin->client.w;
   h = ewin->client.h;
   bw = ewin->client.bw;
   EGetGeometry(disp, ewin->client.win, &ww, &x, &y, &w, &h, &bw, &dummy);
   ewin->client.x = x;
   ewin->client.y = y;
   ewin->client.w = w;
   ewin->client.h = h;
   ewin->client.bw = bw;

   if (XGetWMNormalHints(disp, ewin->client.win, &hint, &mask))
     {
	if (!(ewin->client.already_placed))
	  {
	     if ((hint.flags & USPosition) || ((hint.flags & PPosition)))
	       {
		  if (hint.flags & PWinGravity)
		    {
		       ewin->client.grav = hint.win_gravity;
		    }
		  else
		    {
		       ewin->client.grav = NorthWestGravity;
		    }
		  ewin->client.x = x;
		  ewin->client.y = y;
		  if ((hint.flags & PPosition) && (!EoIsSticky(ewin)))
		    {
		       int                 dsk;

		       dsk = EoGetDesk(ewin);
		       if ((dsk < 0) || (dsk >= DesksGetNumber()))
			  dsk = DesksGetCurrent();
		       ewin->client.x -= DeskGetX(dsk);
		       ewin->client.y -= DeskGetY(dsk);
		       if (ewin->client.x + ewin->client.w >= VRoot.w)
			 {
			    ewin->client.x += DeskGetX(dsk);
			 }
		       else if (ewin->client.x < 0)
			 {
			    ewin->client.x += DeskGetX(dsk);
			 }
		       if (ewin->client.y + ewin->client.h >= VRoot.h)
			 {
			    ewin->client.y += DeskGetY(dsk);
			 }
		       else if (ewin->client.y < 0)
			 {
			    ewin->client.y += DeskGetY(dsk);
			 }
		    }
		  ewin->client.already_placed = 1;
	       }
	  }
	else
	  {
	     ewin->client.x = 0;
	     ewin->client.y = 0;
	     ewin->client.already_placed = 0;
	  }

	if (hint.flags & PMinSize)
	  {
	     ewin->client.width.min = hint.min_width;
	     ewin->client.height.min = hint.min_height;
	  }
	else
	  {
	     ewin->client.width.min = 0;
	     ewin->client.height.min = 0;
	  }

	if (hint.flags & PMaxSize)
	  {
	     ewin->client.width.max = hint.max_width;
	     ewin->client.height.max = hint.max_height;
	     if (hint.max_width < ewin->client.w)
		ewin->client.width.max = ewin->client.w;
	     if (hint.max_height < ewin->client.h)
		ewin->client.height.max = ewin->client.h;
	  }
	else
	  {
	     ewin->client.width.max = 65535;
	     ewin->client.height.max = 65535;
	  }

	if (hint.flags & PResizeInc)
	  {
	     ewin->client.w_inc = hint.width_inc;
	     ewin->client.h_inc = hint.height_inc;
	     if (ewin->client.w_inc < 1)
		ewin->client.w_inc = 1;
	     if (ewin->client.h_inc < 1)
		ewin->client.h_inc = 1;
	  }
	else
	  {
	     ewin->client.w_inc = 1;
	     ewin->client.h_inc = 1;
	  }

	if (hint.flags & PAspect)
	  {
	     if ((hint.min_aspect.y > 0.0) && (hint.min_aspect.x > 0.0))
	       {
		  ewin->client.aspect_min =
		     ((double)hint.min_aspect.x) / ((double)hint.min_aspect.y);
	       }
	     else
	       {
		  ewin->client.aspect_min = 0.0;
	       }
	     if ((hint.max_aspect.y > 0.0) && (hint.max_aspect.x > 0.0))
	       {
		  ewin->client.aspect_max =
		     ((double)hint.max_aspect.x) / ((double)hint.max_aspect.y);
	       }
	     else
	       {
		  ewin->client.aspect_max = 65535.0;
	       }
	  }
	else
	  {
	     ewin->client.aspect_min = 0.0;
	     ewin->client.aspect_max = 65535.0;
	  }

	if (hint.flags & PBaseSize)
	  {
	     ewin->client.base_w = hint.base_width;
	     ewin->client.base_h = hint.base_height;
	  }
	else
	  {
	     ewin->client.base_w = ewin->client.width.min;
	     ewin->client.base_h = ewin->client.height.min;
	  }

	if (ewin->client.width.min < ewin->client.base_w)
	   ewin->client.width.min = ewin->client.base_w;
	if (ewin->client.height.min < ewin->client.base_h)
	   ewin->client.height.min = ewin->client.base_h;
     }

   ewin->client.no_resize_h = 0;
   ewin->client.no_resize_v = 0;
   if (ewin->client.width.min == ewin->client.width.max)
      ewin->client.no_resize_h = 1;
   if (ewin->client.height.min == ewin->client.height.max)
      ewin->client.no_resize_v = 1;

   if (EventDebug(EDBUG_TYPE_SNAPS))
      Eprintf("Snap get icccm %#lx: %4d+%4d %4dx%4d: %s\n",
	      ewin->client.win, ewin->client.x, ewin->client.y,
	      ewin->client.w, ewin->client.h, EwinGetName(ewin));

   EDBUG_RETURN_;
}

static char        *
WinGetWMCommand(Window win)
{
   int                 cargc, i, size;
   char              **cargv, *s;

   s = NULL;

   if (!XGetCommand(disp, win, &cargv, &cargc))
      return NULL;
   if (cargc <= 0)
      return NULL;

   size = strlen(cargv[0]) + 1;
   s = Emalloc(size);
   strcpy(s, cargv[0]);
   for (i = 1; i < cargc; i++)
     {
	size += strlen(cargv[i]) + 1;
	s = Erealloc(s, size);
	strcat(s, " ");
	strcat(s, cargv[i]);
     }
   XFreeStringList(cargv);

   return s;
}

void
ICCCM_GetInfo(EWin * ewin, Atom atom_change)
{
   XClassHint          hint;
   XTextProperty       xtp;
   int                 size;
   char               *s;

   EDBUG(6, "ICCCM_GetInfo");

   if (atom_change == 0 || atom_change == E_XA_WM_CLASS)
     {
	FREE_AND_CLEAR(ewin->icccm.wm_res_name);
	FREE_AND_CLEAR(ewin->icccm.wm_res_class);

	if (XGetClassHint(disp, ewin->client.win, &hint) ||
	    XGetClassHint(disp, ewin->client.group, &hint))
	  {
	     ewin->icccm.wm_res_name = Estrdup(hint.res_name);
	     ewin->icccm.wm_res_class = Estrdup(hint.res_class);
	     XFree(hint.res_name);
	     XFree(hint.res_class);
	  }
     }

   if (atom_change == 0 || atom_change == E_XA_WM_COMMAND)
     {
	FREE_AND_CLEAR(ewin->icccm.wm_command);

	ewin->icccm.wm_command = WinGetWMCommand(ewin->client.win);

	if (!ewin->icccm.wm_command && ewin->client.win != ewin->client.group)
	   ewin->icccm.wm_command = WinGetWMCommand(ewin->client.group);
     }

   if (atom_change == 0 || atom_change == E_XA_WM_CLIENT_MACHINE)
     {
	FREE_AND_CLEAR(ewin->icccm.wm_machine);

	if (XGetWMClientMachine(disp, ewin->client.win, &xtp) ||
	    XGetWMClientMachine(disp, ewin->client.group, &xtp))
	  {
	     ewin->icccm.wm_machine = Estrdup((char *)xtp.value);
	     XFree(xtp.value);
	  }
     }

   if (atom_change == 0 || atom_change == E_XA_WM_ICON_NAME)
     {
	FREE_AND_CLEAR(ewin->icccm.wm_icon_name);

	if (XGetWMIconName(disp, ewin->client.win, &xtp) ||
	    XGetWMIconName(disp, ewin->client.group, &xtp))
	  {
	     if (xtp.encoding == XA_STRING)
	       {
		  ewin->icccm.wm_icon_name = Estrdup((char *)xtp.value);
	       }
	     else
	       {
		  char              **cl;
		  Status              status;
		  int                 n;

		  status = XmbTextPropertyToTextList(disp, &xtp, &cl, &n);
		  if (status >= Success && n > 0 && cl[0])
		    {
		       ewin->icccm.wm_icon_name = Estrdup(cl[0]);
		       XFreeStringList(cl);
		    }
		  else
		     ewin->icccm.wm_icon_name = Estrdup((char *)xtp.value);
	       }
	     XFree(xtp.value);
	  }
     }

   if (atom_change == 0 || atom_change == E_XA_WM_WINDOW_ROLE)
     {
	FREE_AND_CLEAR(ewin->icccm.wm_role);

	s = AtomGet(ewin->client.win, E_XA_WM_WINDOW_ROLE, XA_STRING, &size);
	if (s)
	  {
	     ewin->icccm.wm_role = Estrndup(s, size);
	     Efree(s);
	  }
     }

   EDBUG_RETURN_;
}

void
ICCCM_GetHints(EWin * ewin, Atom atom_change)
{
   XWMHints           *hint;
   Window              win;
   Atom               *prop;
   Window             *cleader;
   int                 i, num;

   EDBUG(6, "ICCCM_GetHints");

   if (EwinIsInternal(ewin))
      EDBUG_RETURN_;

   MWM_GetHints(ewin, atom_change);

   hint = NULL;
   if (atom_change == 0 || atom_change == E_XA_WM_HINTS)
      hint = XGetWMHints(disp, ewin->client.win);
   if (hint)
     {
	/* I have to make sure the thing i'm docking is a dock app */
	if ((hint->flags & StateHint)
	    && (hint->initial_state == WithdrawnState))
	  {
	     if (hint->flags & (StateHint | IconWindowHint | IconPositionHint |
				WindowGroupHint))
	       {
		  if ((hint->icon_x == 0) && (hint->icon_y == 0)
		      && hint->window_group == ewin->client.win)
		     ewin->docked = 1;
	       }
	  }

	if (hint->flags & InputHint)
	  {
	     if (hint->input)
	       {
		  ewin->client.need_input = 1;
	       }
	     else
	       {
		  ewin->client.need_input = 0;
	       }
	  }
	else
	  {
	     ewin->client.need_input = 1;
	  }

	if (hint->flags & StateHint)
	  {
	     if (hint->initial_state == IconicState)
	       {
		  ewin->client.start_iconified = 1;
	       }
	     else
	       {
		  ewin->client.start_iconified = 0;
	       }
	  }
	else
	  {
	     ewin->client.start_iconified = 0;
	  }

	if (hint->flags & IconPixmapHint)
	  {
	     ewin->client.icon_pmap = hint->icon_pixmap;
	     EwinChange(ewin, EWIN_CHANGE_ICON_PMAP);
	  }
	else
	  {
	     ewin->client.icon_pmap = 0;
	  }

	if (hint->flags & IconMaskHint)
	  {
	     ewin->client.icon_mask = hint->icon_mask;
	  }
	else
	  {
	     ewin->client.icon_mask = 0;
	  }

	if (hint->flags & IconWindowHint)
	  {
	     ewin->client.icon_win = hint->icon_window;
	  }
	else
	  {
	     ewin->client.icon_win = 0;
	  }

	if (hint->flags & WindowGroupHint)
	  {
	     ewin->client.group = hint->window_group;
	  }
	else
	  {
	     ewin->client.group = 0;
	  }

	XFree(hint);
     }

   if (atom_change == 0 || atom_change == ECORE_X_ATOM_WM_PROTOCOLS)
     {
	if (XGetWMProtocols(disp, ewin->client.win, &prop, &num))
	  {
	     ewin->client.take_focus = 0;
	     ewin->client.delete_window = 0;
	     for (i = 0; i < num; i++)
	       {
		  if (prop[i] == ECORE_X_ATOM_WM_TAKE_FOCUS)
		     ewin->client.take_focus = ewin->client.need_input = 1;
		  else if (prop[i] == ECORE_X_ATOM_WM_DELETE_WINDOW)
		     ewin->client.delete_window = 1;
	       }
	     XFree(prop);
	  }
     }

   if (!ewin->client.need_input)
     {
	ewin->skipfocus = 1;
     }

   if (atom_change == 0 || atom_change == E_XA_WM_TRANSIENT_FOR)
     {
	ewin->client.transient = 0;
	ewin->client.transient_for = None;
	if (XGetTransientForHint(disp, ewin->client.win, &win))
	  {
	     ewin->client.transient = 1;
	     ewin->client.transient_for = win;
	  }
     }

   if (ewin->client.group == ewin->client.win)
     {
	ewin->client.is_group_leader = 1;
     }
   else
     {
	ewin->client.is_group_leader = 0;
     }

   if (atom_change == 0 || atom_change == E_XA_WM_CLIENT_LEADER)
     {
	cleader =
	   AtomGet(ewin->client.win, E_XA_WM_CLIENT_LEADER, XA_WINDOW, &num);
	if (cleader)
	  {
	     ewin->client.client_leader = *cleader;
	     if (!ewin->client.group)
		ewin->client.group = *cleader;
	     Efree(cleader);
	  }
     }

   EDBUG_RETURN_;
}

void
ICCCM_GetShapeInfo(EWin * ewin)
{
   XRectangle         *rl = NULL;
   int                 rn = 0, ord;
   int                 x, y;
   unsigned int        w, h, d;
   Window              rt;

   EDBUG(6, "ICCCM_GetShapeInfo");

   ecore_x_grab();
   EGetGeometry(disp, ewin->client.win, &rt, &x, &y, &w, &h, &d, &d);
   rl = EShapeGetRectangles(disp, ewin->client.win, ShapeBounding, &rn, &ord);
   ecore_x_ungrab();

   if (rn < 1)
     {
	ewin->client.shaped = 0;
	EShapeCombineMask(disp, ewin->win_container, ShapeBounding, 0, 0, None,
			  ShapeSet);
     }
   else if (rn == 1)
     {
	if ((rl[0].x <= 0) && (rl[0].y <= 0) && (rl[0].width >= w)
	    && (rl[0].height >= h))
	  {
	     ewin->client.shaped = 0;
	     EShapeCombineMask(disp, ewin->win_container, ShapeBounding, 0, 0,
			       None, ShapeSet);
	  }
	else
	  {
	     ewin->client.shaped = 1;
	     EShapeCombineShape(disp, ewin->win_container, ShapeBounding, 0, 0,
				ewin->client.win, ShapeBounding, ShapeSet);
	  }
     }
   else
     {
	ewin->client.shaped = 1;
	EShapeCombineShape(disp, ewin->win_container, ShapeBounding, 0, 0,
			   ewin->client.win, ShapeBounding, ShapeSet);
     }
   if (rl)
      XFree(rl);

   EDBUG_RETURN_;
}

void
ICCCM_SetIconSizes()
{
   XIconSize          *is;

   EDBUG(6, "ICCCM_SetIconSizes");
   is = XAllocIconSize();
   is->min_width = 8;
   is->min_height = 8;
   is->max_width = 48;
   is->max_height = 48;
   is->width_inc = 1;
   is->height_inc = 1;
   XSetIconSizes(disp, VRoot.win, is, 1);
   XFree(is);
   EDBUG_RETURN_;
}

/*
 * Process received window property change
 */
void
ICCCM_ProcessPropertyChange(EWin * ewin, Atom atom_change)
{
   EDBUG(6, "ICCCM_ProcessPropertyChange");

   ICCCM_GetTitle(ewin, atom_change);
   ICCCM_GetHints(ewin, atom_change);
   ICCCM_GetInfo(ewin, atom_change);
   ICCCM_Cmap(ewin);
   ICCCM_GetGeoms(ewin, atom_change);

   EDBUG_RETURN_;
}

void
ICCCM_SetEInfo(EWin * ewin)
{
   static Atom         a = 0, aa = 0;
   CARD32              c[8];

   EDBUG(6, "ICCCM_SetEInfo");

   if (EwinIsInternal(ewin))
      EDBUG_RETURN_;

   if (!a)
      a = XInternAtom(disp, "ENL_INTERNAL_DATA", False);
   if (!aa)
      aa = XInternAtom(disp, "ENL_INTERNAL_DATA_BORDER", False);
   c[0] = EoGetDesk(ewin);
   c[1] = EoIsSticky(ewin);
   c[2] = EoGetX(ewin);
   c[3] = EoGetY(ewin);
   c[4] = ewin->iconified;
   if (ewin->iconified)
      ICCCM_DeIconify(ewin);
   c[5] = ewin->shaded;
   c[6] = ewin->client.w;
   c[7] = ewin->client.h;
   XChangeProperty(disp, ewin->client.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)c, 8);
   XChangeProperty(disp, ewin->client.win, aa, XA_STRING, 8, PropModeReplace,
		   (unsigned char *)ewin->normal_border->name,
		   strlen(ewin->normal_border->name) + 1);
   if (EventDebug(EDBUG_TYPE_SNAPS))
      Eprintf("Snap set einf  %#lx: %4d+%4d %4dx%4d: %s\n",
	      ewin->client.win, ewin->client.x, ewin->client.y,
	      ewin->client.w, ewin->client.h, EwinGetName(ewin));
   EDBUG_RETURN_;
}

void
ICCCM_SetMainEInfo(void)
{
   Atom                a;
   int                 i, ax, ay, n_desks;
   CARD32             *cc;

   n_desks = DesksGetNumber();
   cc = Emalloc(2 * n_desks * sizeof(CARD32));
   if (!cc)
      return;

   for (i = 0; i < n_desks; i++)
     {
	DeskGetArea(i, &ax, &ay);
	cc[(i * 2)] = ax;
	cc[(i * 2) + 1] = ay;
     }
   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   XChangeProperty(disp, VRoot.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)cc, n_desks * 2);

   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   cc[0] = DesksGetCurrent();
   XChangeProperty(disp, VRoot.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)cc, 1);

   Efree(cc);
}

void
ICCCM_GetMainEInfo(void)
{
   Atom                a, a2;
   CARD32             *c;
   unsigned long       lnum, ldummy;
   int                 num, dummy, i, n_desks;
   unsigned char      *puc;

   n_desks = DesksGetNumber();

   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   puc = NULL;
   XGetWindowProperty(disp, VRoot.win, a, 0, 0xffff, False, XA_CARDINAL, &a2,
		      &dummy, &lnum, &ldummy, &puc);
   c = (CARD32 *) puc;
   num = (int)lnum;
   if ((num > 0) && (c))
     {
	for (i = 0; i < (num / 2); i++)
	  {
	     if (i < n_desks)
		DeskSetArea(i, c[(i * 2)], c[(i * 2) + 1]);
	  }
	XFree(c);
     }

   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   puc = NULL;
   XGetWindowProperty(disp, VRoot.win, a, 0, 1, False, XA_CARDINAL, &a2,
		      &dummy, &lnum, &ldummy, &puc);
   c = (CARD32 *) puc;
   num = (int)lnum;
   if ((num > 0) && (c))
     {
	DesksSetCurrent(*c);
	XFree(c);
     }
}

int
ICCCM_GetEInfo(EWin * ewin)
{
   static Atom         a = 0, aa = 0;
   Atom                a2;
   CARD32             *c;
   char               *str;
   unsigned long       lnum, ldummy;
   int                 num, dummy;
   unsigned char      *puc;

   EDBUG(6, "ICCCM_GetEInfo");

   if (EwinIsInternal(ewin))
      EDBUG_RETURN(0);

   if (!a)
      a = XInternAtom(disp, "ENL_INTERNAL_DATA", False);
   if (!aa)
      aa = XInternAtom(disp, "ENL_INTERNAL_DATA_BORDER", False);

   puc = NULL;
   XGetWindowProperty(disp, ewin->client.win, a, 0, 10, True, XA_CARDINAL, &a2,
		      &dummy, &lnum, &ldummy, &puc);
   c = (CARD32 *) puc;
   num = (int)lnum;
   if ((num >= 8) && (c))
     {
	EoSetSticky(ewin, c[1]);
	EoSetDesk(ewin, c[0]);
	ewin->client.x = c[2];
	ewin->client.y = c[3];
	ewin->client.grav = NorthWestGravity;
	ewin->iconified = c[4];
	ewin->shaded = c[5];
	if (ewin->iconified)
	  {
	     ewin->client.start_iconified = 1;
	     ewin->iconified = 0;
	  }
	ewin->client.already_placed = 1;
	ewin->client.w = c[6];
	ewin->client.h = c[7];
	XFree(c);

	puc = NULL;
	XGetWindowProperty(disp, ewin->client.win, aa, 0, 0xffff, True,
			   XA_STRING, &a2, &dummy, &lnum, &ldummy, &puc);
	str = (char *)puc;
	num = (int)lnum;
	if ((num > 0) && (str))
	   EwinSetBorderByName(ewin, str, 0);
	XFree(str);
	if (EventDebug(EDBUG_TYPE_SNAPS))
	   Eprintf("Snap get einf  %#lx: %4d+%4d %4dx%4d: %s\n",
		   ewin->client.win, ewin->client.x, ewin->client.y,
		   ewin->client.w, ewin->client.h, EwinGetName(ewin));
     }
   EDBUG_RETURN(0);
}

void
ICCCM_SetEInfoOnAll(void)
{
   int                 i, num;
   EWin               *const *lst;

   EDBUG(5, "SetEInfoOnAll");

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("SetEInfoOnAll\n");

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
      if (!EwinIsInternal(lst[i]))
	 ICCCM_SetEInfo(lst[i]);

   ICCCM_SetMainEInfo();

   EDBUG_RETURN_;
}
