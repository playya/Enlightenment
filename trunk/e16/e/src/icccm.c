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

#define FREE_AND_CLEAR(ptr) if (ptr) { Efree(ptr); ptr = NULL; }

static Atom         E_XA_WM_STATE = 0;
static Atom         E_XA_WM_CHANGE_STATE = 0;
static Atom         E_XA_WM_NAME = 0;
static Atom         E_XA_WM_COLORMAP_WINDOWS = 0;
static Atom         E_XA_WM_DELETE_WINDOW = 0;
static Atom         E_XA_WM_PROTOCOLS = 0;
static Atom         E_XA_WM_SAVE_YOURSELF = 0;
static Atom         E_XA_WM_TAKE_FOCUS = 0;
static Atom         E_XA_WM_NORMAL_HINTS = 0;
static Atom         E_XA_WM_CLASS = 0;
static Atom         E_XA_WM_COMMAND = 0;
static Atom         E_XA_WM_CLIENT_MACHINE = 0;
static Atom         E_XA_WM_ICON_NAME = 0;
static Atom         E_XA_WM_WINDOW_ROLE = 0;
static Atom         E_XA_WM_HINTS = 0;
static Atom         E_XA_WM_CLIENT_LEADER = 0;

void
ICCCM_Init(void)
{
   E_XA_WM_STATE = XInternAtom(disp, "WM_STATE", False);
   E_XA_WM_CHANGE_STATE = XInternAtom(disp, "WM_CHANGE_STATE", False);
   E_XA_WM_NAME = XInternAtom(disp, "WM_NAME", False);
   E_XA_WM_COLORMAP_WINDOWS = XInternAtom(disp, "WM_COLORMAP_WINDOWS", False);
   E_XA_WM_DELETE_WINDOW = XInternAtom(disp, "WM_DELETE_WINDOW", False);
   E_XA_WM_PROTOCOLS = XInternAtom(disp, "WM_PROTOCOLS", False);
   E_XA_WM_SAVE_YOURSELF = XInternAtom(disp, "WM_SAVE_YOURSELF", False);
   E_XA_WM_TAKE_FOCUS = XInternAtom(disp, "WM_TAKE_FOCUS", False);
   E_XA_WM_NORMAL_HINTS = XInternAtom(disp, "WM_NORMAL_HINTS", False);
   E_XA_WM_CLASS = XInternAtom(disp, "WM_CLASS", False);
   E_XA_WM_COMMAND = XInternAtom(disp, "WM_COMMAND", False);
   E_XA_WM_CLIENT_MACHINE = XInternAtom(disp, "WM_CLIENT_MACHINE", False);
   E_XA_WM_ICON_NAME = XInternAtom(disp, "WM_ICON_NAME", False);
   E_XA_WM_WINDOW_ROLE = XInternAtom(disp, "WM_WINDOW_ROLE", False);
   E_XA_WM_HINTS = XInternAtom(disp, "WM_HINTS", False);
   E_XA_WM_CLIENT_LEADER = XInternAtom(disp, "WM_CLIENT_LEADER", False);
}

void
ICCCM_ProcessClientMessage(XClientMessageEvent * event)
{
   EWin               *ewin;

   if (event->message_type == E_XA_WM_CHANGE_STATE)
     {
	ewin = FindItem(NULL, event->window, LIST_FINDBY_ID, LIST_TYPE_EWIN);
	if (ewin == NULL)
	   return;

	if (event->data.l[0] == IconicState)
	  {
	     if (!(ewin->iconified))
		IconifyEwin(ewin);
	  }
     }
}

void
ICCCM_GetTitle(EWin * ewin, Atom atom_change)
{
   XTextProperty       xtp;

   EDBUG(6, "ICCCM_GetTitle");

   if (atom_change && atom_change != E_XA_WM_NAME)
      EDBUG_RETURN_;

   if (ewin->icccm.wm_name)
      Efree(ewin->icccm.wm_name);

   if (XGetWMName(disp, ewin->client.win, &xtp))
     {
	int                 items;
	char              **list;
	Status              s;

	if (xtp.format == 8)
	  {
	     s = XmbTextPropertyToTextList(disp, &xtp, &list, &items);
	     if ((s == Success) && (items > 0))
	       {
		  ewin->icccm.wm_name = Estrdup(*list);
		  XFreeStringList(list);
	       }
	     else
	       {
		  ewin->icccm.wm_name = Estrdup((char *)xtp.value);
	       }
	  }
	else
	  {
	     ewin->icccm.wm_name = Estrdup((char *)xtp.value);
	  }
	XFree(xtp.value);
     }
   else if (!ewin->internal)
     {
	ewin->icccm.wm_name = Estrdup("No Title");
     }

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

   if (ewin->internal)
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
   XClientMessageEvent ev;
   Atom               *prop;
   int                 num, i, del;

   EDBUG(6, "ICCCM_Delete");

   if (ewin->internal)
     {
	EUnmapWindow(disp, ewin->client.win);
	EDBUG_RETURN_;
     }

   del = 0;
   if (XGetWMProtocols(disp, ewin->client.win, &prop, &num))
     {
	for (i = 0; i < num; i++)
	   if (prop[i] == E_XA_WM_DELETE_WINDOW)
	      del = 1;
	XFree(prop);
     }
   if (del)
     {
	ev.type = ClientMessage;
	ev.window = ewin->client.win;
	ev.message_type = E_XA_WM_PROTOCOLS;
	ev.format = 32;
	ev.data.l[0] = E_XA_WM_DELETE_WINDOW;
	ev.data.l[1] = CurrentTime;
	XSendEvent(disp, ewin->client.win, False, 0, (XEvent *) & ev);
     }
   else
     {
	XKillClient(disp, (XID) ewin->client.win);
     }

   EDBUG_RETURN_;
}

void
ICCCM_Save(EWin * ewin)
{
   XClientMessageEvent ev;

   EDBUG(6, "ICCCM_Save");

   if (ewin->internal)
      EDBUG_RETURN_;

   ev.type = ClientMessage;
   ev.window = ewin->client.win;
   ev.message_type = E_XA_WM_PROTOCOLS;
   ev.format = 32;
   ev.data.l[0] = E_XA_WM_SAVE_YOURSELF;
   ev.data.l[1] = CurrentTime;
   XSendEvent(disp, ewin->client.win, False, 0, (XEvent *) & ev);

   EDBUG_RETURN_;
}

void
ICCCM_Iconify(EWin * ewin)
{
   unsigned long       c[2] = { IconicState, 0 };

   EDBUG(6, "ICCCM_Iconify");

   XChangeProperty(disp, ewin->client.win, E_XA_WM_STATE, E_XA_WM_STATE,
		   32, PropModeReplace, (unsigned char *)c, 2);
   AddItem(ewin, "ICON", ewin->client.win, LIST_TYPE_ICONIFIEDS);
   EUnmapWindow(disp, ewin->client.win);

   EDBUG_RETURN_;
}

void
ICCCM_DeIconify(EWin * ewin)
{
   unsigned long       c[2] = { NormalState, 0 };

   EDBUG(6, "ICCCM_DeIconify");

   XChangeProperty(disp, ewin->client.win, E_XA_WM_STATE, E_XA_WM_STATE,
		   32, PropModeReplace, (unsigned char *)c, 2);
   RemoveItem("ICON", ewin->client.win, LIST_FINDBY_BOTH, LIST_TYPE_ICONIFIEDS);
   EMapWindow(disp, ewin->client.win);

   EDBUG_RETURN_;
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
   XWindowChanges      xwc;
   int                 d;

   EDBUG(6, "ICCCM_Configure");

   d = ewin->desktop;
   if (d < 0)
      d = desks.current;

   if (ewin->shaded == 0)
     {
	xwc.x = ewin->border->border.left;
	xwc.y = ewin->border->border.top;
	xwc.width = ewin->client.w;
	xwc.height = ewin->client.h;
	XConfigureWindow(disp, ewin->win_container,
			 CWX | CWY | CWWidth | CWHeight, &xwc);
     }
   else
     {
	xwc.x = -30;
	xwc.y = -30;
	xwc.width = 1;
	xwc.height = 1;
	XConfigureWindow(disp, ewin->win_container,
			 CWX | CWY | CWWidth | CWHeight, &xwc);
	xwc.width = ewin->client.w;
	xwc.height = ewin->client.h;
     }
   xwc.x = 0;
   xwc.y = 0;
   XConfigureWindow(disp, ewin->client.win, CWX | CWY | CWWidth | CWHeight,
		    &xwc);
   if ((ewin->menu) || (ewin->dialog))
      EDBUG_RETURN_;

   ev.type = ConfigureNotify;
   ev.xconfigure.display = disp;
   ev.xconfigure.event = ewin->client.win;
   ev.xconfigure.window = ewin->client.win;
   ev.xconfigure.x = desks.desk[d].x + ewin->x + ewin->border->border.left;
   ev.xconfigure.y = desks.desk[d].y + ewin->y + ewin->border->border.top;
   ev.xconfigure.width = ewin->client.w;
   ev.xconfigure.height = ewin->client.h;
   ev.xconfigure.border_width = 0;
   ev.xconfigure.above = ewin->win;
   ev.xconfigure.override_redirect = False;
   XSendEvent(disp, ewin->client.win, False, StructureNotifyMask, &ev);

   EDBUG_RETURN_;
}

void
ICCCM_AdoptStart(EWin * ewin)
{
   Window              win = ewin->client.win;

   EDBUG(6, "ICCCM_AdoptStart");

   if (!ewin->internal)
      XAddToSaveSet(disp, win);

   EDBUG_RETURN_;
}

void
ICCCM_Adopt(EWin * ewin)
{
   Window              win = ewin->client.win;
   unsigned long       c[2] = { 0, 0 };

   EDBUG(6, "ICCCM_Adopt");

   if (!ewin->internal)
      XSetWindowBorderWidth(disp, win, 0);
   EReparentWindow(disp, win, ewin->win_container, 0, 0);
   c[0] = (ewin->client.start_iconified) ? IconicState : NormalState;
   XChangeProperty(disp, win, E_XA_WM_STATE, E_XA_WM_STATE, 32, PropModeReplace,
		   (unsigned char *)c, 2);
   ewin->x = ewin->client.x;
   ewin->y = ewin->client.y;
   ewin->w = ewin->client.w +
      ewin->border->border.left + ewin->border->border.right;
   ewin->h = ewin->client.h +
      ewin->border->border.top + ewin->border->border.bottom;

   EDBUG_RETURN_;
}

void
ICCCM_Withdraw(EWin * ewin)
{
   unsigned long       c[2] = { WithdrawnState, 0 };

   EDBUG(6, "ICCCM_Withdraw");

   /* We have a choice of deleting the WM_STATE property
    * or changing the value to Withdrawn. Since twm/fvwm does
    * it that way, we change it to Withdrawn.
    */
   XChangeProperty(disp, ewin->client.win, E_XA_WM_STATE, E_XA_WM_STATE,
		   32, PropModeReplace, (unsigned char *)c, 2);
   XRemoveFromSaveSet(disp, ewin->client.win);

   EDBUG_RETURN_;
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

   if (Mode.cur_menu_mode)
      EDBUG_RETURN_;

   ICCCM_GetColormap(ewin);

   if (ewin->internal)
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
   XClientMessageEvent ev;
   Atom               *prop;
   int                 num, i, foc;

   EDBUG(6, "ICCCM_Focus");

   if (!ewin)
     {
	XSetInputFocus(disp, VRoot.win, RevertToPointerRoot, CurrentTime);
	HintsSetActiveWindow(None);
	EDBUG_RETURN_;
     }

   foc = 0;
   if (XGetWMProtocols(disp, ewin->client.win, &prop, &num))
     {
	for (i = 0; i < num; i++)
	   if (prop[i] == E_XA_WM_TAKE_FOCUS)
	      foc = 1;
	XFree(prop);
     }
   if (foc)
     {
	ev.type = ClientMessage;
	ev.window = ewin->client.win;
	ev.message_type = E_XA_WM_PROTOCOLS;
	ev.format = 32;
	ev.data.l[0] = E_XA_WM_TAKE_FOCUS;
	ev.data.l[1] = CurrentTime;
	XSendEvent(disp, ewin->client.win, False, 0, (XEvent *) & ev);
     }
/*   else */
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
		  if ((hint.flags & PPosition) && (!ewin->sticky))
		    {
		       int                 dsk;

		       dsk = ewin->desktop;
		       if ((dsk < 0) || (dsk >= Conf.desks.num))
			  dsk = desks.current;
		       ewin->client.x -= desks.desk[dsk].x;
		       ewin->client.y -= desks.desk[dsk].y;
		       if (ewin->client.x + ewin->client.w >= VRoot.w)
			 {
			    ewin->client.x += desks.desk[dsk].x;
			 }
		       else if (ewin->client.x < 0)
			 {
			    ewin->client.x += desks.desk[dsk].x;
			 }
		       if (ewin->client.y + ewin->client.h >= VRoot.h)
			 {
			    ewin->client.y += desks.desk[dsk].y;
			 }
		       else if (ewin->client.y < 0)
			 {
			    ewin->client.y += desks.desk[dsk].y;
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
   if (ewin->client.width.min == 0)
     {
	if (ewin->internal)
	  {
	     ewin->client.width.min = w;
	     ewin->client.height.min = h;
	     ewin->client.width.max = w;
	     ewin->client.height.max = h;
	  }
     }
   ewin->client.no_resize_h = 0;
   ewin->client.no_resize_v = 0;
   if (ewin->client.width.min == ewin->client.width.max)
      ewin->client.no_resize_h = 1;
   if (ewin->client.height.min == ewin->client.height.max)
      ewin->client.no_resize_v = 1;

   EDBUG_RETURN_;
}

void
ICCCM_GetInfo(EWin * ewin, Atom atom_change)
{
   XClassHint          hint;
   XTextProperty       xtp;
   int                 cargc, i, size;
   char              **cargv, *s;

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

	if (XGetCommand(disp, ewin->client.win, &cargv, &cargc))
	  {
	     if (cargc > 0)
	       {
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
		  ewin->icccm.wm_command = s;
	       }
	  }
	else if (XGetCommand(disp, ewin->client.group, &cargv, &cargc))
	  {
	     EWin               *const *lst;
	     int                 lnum, ok = 1;

	     lst = EwinListGetAll(&lnum);
	     for (i = 0; i < lnum; i++)
	       {
		  if ((lst[i] != ewin)
		      && (lst[i]->client.group == ewin->client.group))
		    {
		       ok = 0;
		       i = lnum;
		    }
	       }

	     if (cargc > 0)
	       {
		  if (ok)
		    {
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
		       ewin->icccm.wm_command = s;
		    }
		  XFreeStringList(cargv);
	       }
	  }
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
   Window              w;
   Atom               *prop;
   Window             *cleader;
   int                 i, num;

   EDBUG(6, "ICCCM_GetHints");

   if (ewin->internal)
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

   if (atom_change == 0 || atom_change == E_XA_WM_PROTOCOLS)
     {
	if (XGetWMProtocols(disp, ewin->client.win, &prop, &num))
	  {
	     for (i = 0; i < num; i++)
		if (prop[i] == E_XA_WM_TAKE_FOCUS)
		   ewin->client.need_input = 1;
	     XFree(prop);
	  }
     }

   if (!ewin->client.need_input)
     {
	ewin->skipfocus = 1;
     }

   if (XGetTransientForHint(disp, ewin->client.win, &w))
     {
	ewin->client.transient = 1;
	ewin->client.transient_for = w;
     }
   else
     {
	ewin->client.transient = 0;
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

   GrabX();
   EGetGeometry(disp, ewin->client.win, &rt, &x, &y, &w, &h, &d, &d);
   rl = EShapeGetRectangles(disp, ewin->client.win, ShapeBounding, &rn, &ord);
   UngrabX();

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

void
ICCCM_SetEInfo(EWin * ewin)
{
   static Atom         a = 0, aa = 0;
   CARD32              c[8];

   EDBUG(6, "ICCCM_SetEInfo");
   if (ewin->internal)
      EDBUG_RETURN_;
   if (!a)
      a = XInternAtom(disp, "ENL_INTERNAL_DATA", False);
   if (!aa)
      aa = XInternAtom(disp, "ENL_INTERNAL_DATA_BORDER", False);
   c[0] = ewin->desktop;
   c[1] = ewin->sticky;
   c[2] = ewin->x;
   c[3] = ewin->y;
   c[4] = ewin->iconified;
   if (ewin->iconified)
      ICCCM_DeIconify(ewin);
   c[5] = ewin->shaded;
   c[6] = ewin->client.w;
   c[7] = ewin->client.h;
   XChangeProperty(disp, ewin->client.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)c, 9);
   XChangeProperty(disp, ewin->client.win, aa, XA_STRING, 8, PropModeReplace,
		   (unsigned char *)ewin->border->name,
		   strlen(ewin->border->name) + 1);
   EDBUG_RETURN_;
}

void
ICCCM_SetMainEInfo(void)
{
   Atom                a;
   int                 i;
   CARD32              cc[ENLIGHTENMENT_CONF_NUM_DESKTOPS * 2];

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	cc[(i * 2)] = desks.desk[i].current_area_x;
	cc[(i * 2) + 1] = desks.desk[i].current_area_y;
     }
   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   XChangeProperty(disp, VRoot.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)cc, ENLIGHTENMENT_CONF_NUM_DESKTOPS * 2);
   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   XChangeProperty(disp, VRoot.win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)(&desks.current), 1);
}

void
ICCCM_GetMainEInfo(void)
{
   Atom                a, a2;
   CARD32             *c;
   unsigned long       lnum, ldummy;
   int                 num, dummy, i;
   unsigned char      *puc;

   a = XInternAtom(disp, "ENL_INTERNAL_AREA_DATA", False);
   puc = NULL;
   XGetWindowProperty(disp, VRoot.win, a, 0, 10, False, XA_CARDINAL, &a2,
		      &dummy, &lnum, &ldummy, &puc);
   c = (CARD32 *) puc;
   num = (int)lnum;
   if ((num > 0) && (c))
     {
	for (i = 0; i < (num / 2); i++)
	  {
	     if (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS)
	       {
		  desks.desk[i].current_area_x = c[(i * 2)];
		  desks.desk[i].current_area_y = c[(i * 2) + 1];
	       }
	  }
	XFree(c);
     }

   a = XInternAtom(disp, "ENL_INTERNAL_DESK_DATA", False);
   puc = NULL;
   XGetWindowProperty(disp, VRoot.win, a, 0, 10, False, XA_CARDINAL, &a2,
		      &dummy, &lnum, &ldummy, &puc);
   c = (CARD32 *) puc;
   num = (int)lnum;
   if ((num > 0) && (c))
     {
	GotoDesktop(*c);
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
   if (ewin->internal)
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
	ewin->desktop = c[0];
	ewin->sticky = c[1];
	ewin->client.x = c[2];
	ewin->client.y = c[3];
	ewin->iconified = c[4];
	ewin->shaded = c[5];
	if (ewin->sticky)
	   ewin->desktop = -1;
	if (ewin->iconified)
	  {
	     ewin->client.start_iconified = 1;
	     ewin->iconified = 0;
	  }
	ewin->client.already_placed = 1;
	if (num >= 9)
	  {
	     ewin->client.w = c[6];
	     ewin->client.h = c[7];
	  }
	XFree(c);

	puc = NULL;
	XGetWindowProperty(disp, ewin->client.win, aa, 0, 0xffff, True,
			   XA_STRING, &a2, &dummy, &lnum, &ldummy, &puc);
	str = (char *)puc;
	num = (int)lnum;
	if ((num > 0) && (str))
	   EwinSetBorderByName(ewin, str, 0);
	XFree(str);
     }
   EDBUG_RETURN(0);
}
