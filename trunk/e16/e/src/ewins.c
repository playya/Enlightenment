/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004 Kim Woelders
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
#include "ecompmgr.h"
#include "snaps.h"
#include <sys/time.h>

#define EWIN_TOP_EVENT_MASK \
  (ButtonPressMask | ButtonReleaseMask | \
   EnterWindowMask | LeaveWindowMask | PointerMotionMask /* | \
   StructureNotifyMask */)

#define EWIN_CONTAINER_EVENT_MASK \
  (/* ButtonPressMask | ButtonReleaseMask | */ \
   /* StructureNotifyMask | ResizeRedirectMask | */ \
   SubstructureNotifyMask | SubstructureRedirectMask)

#define EWIN_CLIENT_EVENT_MASK \
  (EnterWindowMask | LeaveWindowMask | FocusChangeMask | \
   /* StructureNotifyMask | */ ResizeRedirectMask | \
   PropertyChangeMask | ColormapChangeMask | VisibilityChangeMask)

static void         EwinHandleEventsToplevel(XEvent * ev, void *prm);
static void         EwinHandleEventsContainer(XEvent * ev, void *prm);
static void         EwinHandleEventsClient(XEvent * ev, void *prm);

static void
EwinEventsConfigure(EWin * ewin, int mode)
{
   long                emask;

   emask = (mode) ? ~((long)0) : ~(EnterWindowMask | LeaveWindowMask);

   ESelectInput(EoGetWin(ewin), EWIN_TOP_EVENT_MASK & emask);
   ESelectInput(ewin->client.win, ewin->client.event_mask & emask);
   EwinBorderEventsConfigure(ewin, mode);
}

static EWin        *
EwinCreate(Window win, int type)
{
   EWin               *ewin;
   XSetWindowAttributes att;

   ewin = Ecalloc(1, sizeof(EWin));

   ewin->type = type;
   ewin->state = (Mode.wm.startup) ? EWIN_STATE_STARTUP : EWIN_STATE_NEW;
   ewin->ld = -1;
   ewin->lx = -1;
   ewin->ly = -1;
   ewin->lw = -1;
   ewin->lh = -1;
   ewin->client.x = -1;
   ewin->client.y = -1;
   ewin->client.w = -1;
   ewin->client.h = -1;
   ewin->client.need_input = 1;
   ewin->client.aspect_min = 0.0;
   ewin->client.aspect_max = 65535.0;
   ewin->client.w_inc = 1;
   ewin->client.h_inc = 1;
   ewin->client.width.max = 65535;
   ewin->client.height.max = 65535;
   ewin->client.mwm_decor_border = 1;
   ewin->client.mwm_decor_resizeh = 1;
   ewin->client.mwm_decor_title = 1;
   ewin->client.mwm_decor_menu = 1;
   ewin->client.mwm_decor_minimize = 1;
   ewin->client.mwm_decor_maximize = 1;
   ewin->client.mwm_func_resize = 1;
   ewin->client.mwm_func_move = 1;
   ewin->client.mwm_func_minimize = 1;
   ewin->client.mwm_func_maximize = 1;
   ewin->client.mwm_func_close = 1;
   EoSetWin(ewin, ECreateWindow(VRoot.win, -10, -10, 1, 1, 1));
   ewin->win_container = ECreateWindow(EoGetWin(ewin), 0, 0, 1, 1, 0);
#if 0				/* ENABLE_GNOME - Not actually used */
   ewin->expanded_width = -1;
   ewin->expanded_height = -1;
#endif
   ewin->area_x = -1;
   ewin->area_y = -1;

   EobjInit(&ewin->o, EOBJ_TYPE_EWIN, -1, -1, -1, -1);
   EoSetDesk(ewin, DesksGetCurrent());
   EoSetLayer(ewin, 4);
   ewin->props.opacity = 0xFFFFFFFF;

   att.event_mask = EWIN_CONTAINER_EVENT_MASK;
   att.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
   EChangeWindowAttributes(ewin->win_container,
			   CWEventMask | CWDontPropagate, &att);
   EMapWindow(ewin->win_container);

   att.event_mask = EWIN_TOP_EVENT_MASK;
   att.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
   EChangeWindowAttributes(EoGetWin(ewin), CWEventMask | CWDontPropagate, &att);
   ewin->client.win = win;
   FocusEwinSetGrabs(ewin);
   GrabButtonGrabs(ewin);

   EobjListStackAdd(&ewin->o, 0);
   EobjListFocusAdd(&ewin->o, 0);

   ewin->client.event_mask = EWIN_CLIENT_EVENT_MASK;
   AddItem(ewin, "EWIN", win, LIST_TYPE_EWIN);

   XShapeSelectInput(disp, win, ShapeNotifyMask);

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinCreate %#lx frame=%#lx state=%d\n", ewin->client.win,
	      EoGetWin(ewin), ewin->state);

   EventCallbackRegister(EoGetWin(ewin), 0, EwinHandleEventsToplevel, ewin);
   EventCallbackRegister(ewin->win_container, 0, EwinHandleEventsContainer,
			 ewin);
   ERegisterWindow(ewin->client.win);
   EventCallbackRegister(ewin->client.win, 0, EwinHandleEventsClient, ewin);

   if (!EwinIsInternal(ewin))
     {
	XSetWindowBorderWidth(disp, win, 0);
	ewin->client.bw = 0;
     }

   ModulesSignal(ESIGNAL_EWIN_CREATE, ewin);

   return ewin;
}

static void
EwinCleanup(EWin * ewin)
{
   EwinBorderDetach(ewin);
}

static void
EwinDestroy(EWin * ewin)
{
   EWin              **lst;
   int                 i, num;

   if (!ewin)
      return;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinDestroy %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   /* FIXME - Fading */
   ECompMgrWinDel(&ewin->o, True, False);

   RemoveItem(NULL, ewin->client.win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
   EventCallbackUnregister(EoGetWin(ewin), 0, EwinHandleEventsToplevel, ewin);
   EventCallbackUnregister(ewin->win_container, 0, EwinHandleEventsContainer,
			   ewin);
   EventCallbackUnregister(ewin->client.win, 0, EwinHandleEventsClient, ewin);
   if (!EwinIsInternal(ewin))
      EUnregisterWindow(ewin->client.win);

   EobjListStackDel(&ewin->o);
   EobjListFocusDel(&ewin->o);

   HintsSetClientList();

   SnapshotEwinUnmatch(ewin);

   ModulesSignal(ESIGNAL_EWIN_DESTROY, ewin);

   lst = EwinListTransientFor(ewin, &num);
   for (i = 0; i < num; i++)
     {
	lst[i]->has_transients--;
	if (lst[i]->has_transients < 0)	/* Paranoia? */
	   lst[i]->has_transients = 0;
     }
   if (lst)
      Efree(lst);

   EwinCleanup(ewin);

   if (ewin->icccm.wm_name)
      Efree(ewin->icccm.wm_name);
   if (ewin->icccm.wm_res_class)
      Efree(ewin->icccm.wm_res_class);
   if (ewin->icccm.wm_res_name)
      Efree(ewin->icccm.wm_res_name);
   if (ewin->icccm.wm_role)
      Efree(ewin->icccm.wm_role);
   if (ewin->icccm.wm_command)
      Efree(ewin->icccm.wm_command);
   if (ewin->icccm.wm_machine)
      Efree(ewin->icccm.wm_machine);
#if ENABLE_EWMH
   if (ewin->ewmh.wm_name)
      Efree(ewin->ewmh.wm_name);
   if (ewin->ewmh.wm_icon_name)
      Efree(ewin->ewmh.wm_icon_name);
#endif
   if (ewin->icccm.wm_icon_name)
      Efree(ewin->icccm.wm_icon_name);
   if (EoGetWin(ewin))
      EDestroyWindow(EoGetWin(ewin));
   if (ewin->bits)
      Efree(ewin->bits);
   if (ewin->session_id)
      Efree(ewin->session_id);
   FreePmapMask(&ewin->mini_pmm);
   if (ewin->icon_image)
     {
	imlib_context_set_image(ewin->icon_image);
	imlib_free_image_and_decache();
     }
   GroupsEwinRemove(ewin);
   Efree(ewin);
}

void
DetermineEwinFloat(EWin * ewin, int dx, int dy)
{
   char                dofloat = 0;
   int                 desk, x, y, w, h, xd, yd;

   desk = EoGetDesk(ewin);
   x = EoGetX(ewin);
   y = EoGetY(ewin);
   w = EoGetW(ewin);
   h = EoGetH(ewin);

   xd = DeskGetX(desk);
   yd = DeskGetY(desk);

   if ((desk != 0) && (EoIsFloating(ewin) < 2) &&
       ((xd != 0) || (yd != 0) || (DesksGetCurrent() != desk)))
     {
	switch (Conf.desks.dragdir)
	  {
	  case 0:
	     if (((x + dx < 0) ||
		  ((x + dx + w <= VRoot.w) &&
		   ((DesktopAt(xd + x + dx + w - 1, yd) != desk)))))
		dofloat = 1;
	     break;
	  case 1:
	     if (((x + dx + w > VRoot.w) ||
		  ((x + dx >= 0) && ((DesktopAt(xd + x + dx, yd) != desk)))))
		dofloat = 1;
	     break;
	  case 2:
	     if (((y + dy < 0) ||
		  ((y + dy + h <= VRoot.h) &&
		   ((DesktopAt(xd, yd + y + dy + h - 1) != desk)))))
		dofloat = 1;
	     break;
	  case 3:
	     if (((y + dy + h > VRoot.h) ||
		  ((y + dy >= 0) && ((DesktopAt(xd, yd + y + dy) != desk)))))
		dofloat = 1;
	     break;
	  }

	if (dofloat)
	   FloatEwinAt(ewin, x + xd, y + yd);
     }
}

EWin               *
GetEwinByCurrentPointer(void)
{
   Window              rt, ch;
   int                 dum, x, y;
   unsigned int        mr;

   XQueryPointer(disp, DeskGetWin(DesksGetCurrent()), &rt, &ch, &x, &y, &dum,
		 &dum, &mr);

   return FindEwinByBase(ch);
}

EWin               *
GetEwinPointerInClient(void)
{
   Window              rt, ch;
   int                 dum, px, py, desk;
   EWin               *const *lst, *ewin;
   int                 i, num;

   desk = DesktopAt(Mode.x, Mode.y);
   XQueryPointer(disp, DeskGetWin(desk), &rt, &ch, &dum, &dum, &px, &py,
		 (unsigned int *)&dum);
   px -= DeskGetX(desk);
   py -= DeskGetY(desk);

   lst = EwinListGetForDesk(&num, desk);
   for (i = 0; i < num; i++)
     {
	int                 x, y, w, h;

	ewin = lst[i];
	x = EoGetX(ewin);
	y = EoGetY(ewin);
	w = EoGetW(ewin);
	h = EoGetH(ewin);
	if ((px >= x) && (py >= y) && (px < (x + w)) && (py < (y + h)) &&
	    EwinIsMapped(ewin))
	   return ewin;
     }

   return NULL;
}

EWin               *
GetFocusEwin(void)
{
   return Mode.focuswin;
}

EWin               *
GetContextEwin(void)
{
   EWin               *ewin;

#if 0
   ewin = Mode.mouse_over_ewin;
   if (ewin && ewin->type != EWIN_TYPE_MENU)
      return ewin;
#endif

   ewin = Mode.context_ewin;
   if (ewin)
      goto done;

   ewin = Mode.focuswin;
   if (ewin && ewin->type != EWIN_TYPE_MENU)
      goto done;

   ewin = NULL;

 done:
#if 0
   Eprintf("GetContextEwin %#lx %s\n", EwinGetClientWin(ewin),
	   EwinGetName(ewin));
#endif
   return ewin;
}

void
SetContextEwin(EWin * ewin)
{
   if (ewin && ewin->type == EWIN_TYPE_MENU)
      return;
#if 0
   Eprintf("SetContextEwin %#lx %s\n", EwinGetClientWin(ewin),
	   EwinGetName(ewin));
#endif
   Mode.context_ewin = ewin;
}

void
EwinDetermineArea(EWin * ewin)
{
   int                 ax, ay;

   DeskGetArea(EoGetDesk(ewin), &ax, &ay);
   ax = (EoGetX(ewin) + (EoGetW(ewin) / 2) + (ax * VRoot.w)) / VRoot.w;
   ay = (EoGetY(ewin) + (EoGetH(ewin) / 2) + (ay * VRoot.h)) / VRoot.h;

   AreaFix(&ax, &ay);

   if (ax != ewin->area_x || ay != ewin->area_y)
     {
	ewin->area_x = ax;
	ewin->area_y = ay;
	HintsSetWindowArea(ewin);
     }
}

/*
 * Derive frame window position from client window and border properties
 */
static void
EwinGetPosition(const EWin * ewin, int *px, int *py)
{
   int                 x, y, bw, frame_lr, frame_tb;

   x = ewin->client.x;
   y = ewin->client.y;
   bw = ewin->client.bw;
   frame_lr = ewin->border->border.left + ewin->border->border.right;
   frame_tb = ewin->border->border.top + ewin->border->border.bottom;

   switch (ewin->client.grav)
     {
     case NorthWestGravity:
     case SouthWestGravity:
     case WestGravity:
	x -= bw;
	break;
     case NorthEastGravity:
     case EastGravity:
     case SouthEastGravity:
	x -= frame_lr / 2;
	break;
     case NorthGravity:
     case CenterGravity:
     case SouthGravity:
	x -= frame_lr - bw;
	break;
     case StaticGravity:
	x -= ewin->border->border.left;
	break;
     default:
	break;
     }

   switch (ewin->client.grav)
     {
     case NorthWestGravity:
     case NorthGravity:
     case NorthEastGravity:
	y -= bw;
	break;
     case WestGravity:
     case CenterGravity:
     case EastGravity:
	y -= frame_tb / 2;
	break;
     case SouthWestGravity:
     case SouthGravity:
     case SouthEastGravity:
	y -= frame_tb - bw;
	break;
     case StaticGravity:
	y -= ewin->border->border.top;
	break;
     default:
	break;
     }

   *px = x;
   *py = y;
}

/*
 * Derive frame window geometry from client window properties
 */
static void
EwinGetGeometry(EWin * ewin)
{
   int                 x, y;

   EwinGetPosition(ewin, &x, &y);

   ewin->client.x = x + ewin->border->border.left;
   ewin->client.y = y + ewin->border->border.top;

   EoSetX(ewin, ewin->shape_x = x);
   EoSetY(ewin, ewin->shape_y = y);

   EoSetW(ewin, ewin->client.w +
	  ewin->border->border.left + ewin->border->border.right);
   EoSetH(ewin, ewin->client.h +
	  ewin->border->border.top + ewin->border->border.bottom);
}

void
EwinPropagateShapes(EWin * ewin)
{
   if (!ewin->docked)
      PropagateShapes(EoGetWin(ewin));
}

static void
EwinAdopt(EWin * ewin)
{
   /* We must reparent after getting original window position */
   EReparentWindow(ewin->client.win, ewin->win_container, 0, 0);
   ICCCM_Adopt(ewin);
}

static EWin        *
Adopt(EWin * ewin, Window win)
{
   if (ewin)
      EwinCleanup(ewin);
   else
      ewin = EwinCreate(win, EWIN_TYPE_NORMAL);

   ICCCM_AdoptStart(ewin);
   ICCCM_GetTitle(ewin, 0);
   ICCCM_GetHints(ewin, 0);
   ICCCM_GetInfo(ewin, 0);
   ICCCM_GetColormap(ewin);
   ICCCM_GetShapeInfo(ewin);
   ICCCM_GetGeoms(ewin, 0);
   HintsGetWindowHints(ewin);
   SessionGetInfo(ewin, 0);
#if 0				/* Do we want this? */
   MatchEwinToSM(ewin);
#endif
   WindowMatchEwinOps(ewin);	/* Window matches */
   SnapshotEwinMatch(ewin);	/* Saved settings */
   if (Mode.wm.startup)
      EHintsGetInfo(ewin);	/* E restart hints */
   ICCCM_MatchSize(ewin);

   EwinAdopt(ewin);

   EwinBorderSelect(ewin);
   EwinBorderSetTo(ewin, NULL);

   EwinGetGeometry(ewin);
   EwinEventsConfigure(ewin, 1);

   if (ewin->shaded)
      EwinInstantShade(ewin, 1);

   HintsSetWindowState(ewin);
   HintsSetClientList();

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("Adopt %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   return ewin;
}

static EWin        *
AdoptInternal(Window win, Border * border, int type, void (*init) (EWin *
								   ewin,
								   void *ptr),
	      void *ptr)
{
   EWin               *ewin;

   ewin = EwinCreate(win, type);

   ewin->border = border;

   if (init)
      init(ewin, ptr);		/* Type specific initialisation */

   ICCCM_AdoptStart(ewin);
   ICCCM_GetTitle(ewin, 0);
   ICCCM_GetInfo(ewin, 0);
   ICCCM_GetShapeInfo(ewin);
   ICCCM_GetGeoms(ewin, 0);

   WindowMatchEwinOps(ewin);	/* Window matches */
   SnapshotEwinMatch(ewin);	/* Saved settings */
   ICCCM_MatchSize(ewin);

   EwinAdopt(ewin);

   EwinBorderSelect(ewin);
   EwinBorderSetTo(ewin, NULL);

   EwinGetGeometry(ewin);
   EwinEventsConfigure(ewin, 1);

   if (ewin->shaded)
      EwinInstantShade(ewin, 1);

   HintsSetWindowState(ewin);
   HintsSetClientList();

   return ewin;
}

void
AddToFamily(EWin * ewin, Window win)
{
   EWin               *ewin2;
   EWin              **lst;
   int                 i, k, num, fx, fy, x, y, desk;
   char                doslide, manplace;

   ecore_x_grab();

   if (!WinExists(win))
     {
	Eprintf("Window is gone %#lx\n", win);
	ecore_x_ungrab();
	return;
     }

   /* adopt the new baby */
   ewin = Adopt(ewin, win);

   /* if it hasn't been planted on a desktop - assign it the current desktop */
   desk = EoGetDesk(ewin);

   /* if is an afterstep/windowmaker dock app - dock it */
   if (Conf.dock.enable && ewin->docked)
     {
	DockIt(ewin);
	ewin->props.donthide = 1;
     }

   doslide = Conf.mapslide && !Mode.wm.startup;
   /* if set for borderless then dont slide it in */
   if ((!ewin->client.mwm_decor_title) && (!ewin->client.mwm_decor_border))
      doslide = 0;

   ewin2 = NULL;
   if (ewin->client.transient)
     {
	if (ewin->client.transient_for == None ||
	    ewin->client.transient_for == VRoot.win)
	  {
	     /* Group transient */
	     ewin->client.transient_for = VRoot.win;
#if 0				/* Maybe? */
	     ewin->layer++;
#endif
	     /* Don't treat this as a normal transient */
	     ewin->client.transient = -1;
	  }
	else if (ewin->client.transient_for == ewin->client.win)
	  {
	     /* Some apps actually do this. Why? */
	     ewin->client.transient = 0;
	  }
	else
	  {
	     /* Regular transient */
	  }

	if (ewin->client.transient)
	  {
	     /* Tag the parent window if this is a transient */
	     lst = EwinListTransientFor(ewin, &num);
	     for (i = 0; i < num; i++)
	       {
		  lst[i]->has_transients++;
		  if (EoGetLayer(ewin) < EoGetLayer(lst[i]))
		     EoSetLayer(ewin, EoGetLayer(lst[i]));
	       }
	     if (lst)
	       {
		  ewin2 = lst[0];
		  EoSetSticky(ewin, EoIsSticky(lst[0]));
		  Efree(lst);
	       }
	     else
	       {
		  /* No parents? - not a transient */
		  ewin->client.transient = 0;
	       }
	  }
     }

   if (ewin->client.transient && Conf.focus.transientsfollowleader)
     {
	EWin               *const *lst2;

	if (!ewin2)
	   ewin2 = FindItem(NULL, ewin->client.group, LIST_FINDBY_ID,
			    LIST_TYPE_EWIN);

	if (!ewin2)
	  {
	     lst2 = EwinListGetAll(&num);
	     for (i = 0; i < num; i++)
	       {
		  if ((lst2[i]->iconified) ||
		      (ewin->client.group != lst2[i]->client.group))
		     continue;

		  ewin2 = lst2[i];
		  break;
	       }
	  }

	if (ewin2)
	  {
	     desk = EoGetDesk(ewin2);
	     if (!Mode.wm.startup && Conf.focus.switchfortransientmap &&
		 !ewin->iconified)
		DeskGotoByEwin(ewin2);
	  }
     }

   if (ewin->st.fullscreen)
     {
	ewin->st.fullscreen = 0;
	EwinSetFullscreen(ewin, 1);
	ewin->client.already_placed = 1;
	ShowEwin(ewin);
	ecore_x_ungrab();
	return;
     }

   ResizeEwin(ewin, ewin->client.w, ewin->client.h);

   manplace = 0;
   if ((!ewin->client.transient) && (Conf.place.manual)
       && (!ewin->client.already_placed) && (!Mode.wm.startup) && (!Mode.place))
     {
	char                cangrab;

	cangrab = GrabPointerSet(VRoot.win, ECSR_GRAB, 0);
	if (cangrab == GrabSuccess)
	   manplace = 1;
     }

   /* if it hasn't been placed yet.... find a spot for it */
   x = EoGetX(ewin);
   y = EoGetY(ewin);
   if ((!ewin->client.already_placed) && (!manplace))
     {
	/* Place the window below the mouse pointer */
	if (Conf.place.manual_mouse_pointer)
	  {
	     int                 rx, ry, wx, wy;
	     unsigned int        mask;
	     Window              junk, root_return;
	     int                 newWinX = 0, newWinY = 0;

	     /* if the loser has manual placement on and the app asks to be on */
	     /*  a desktop, then send E to that desktop so the user can place */
	     /* the window there */
	     DeskGoto(desk);

	     XQueryPointer(disp, VRoot.win, &root_return, &junk, &rx, &ry, &wx,
			   &wy, &mask);
	     Mode.x = rx;
	     Mode.y = ry;
	     ewin->client.already_placed = 1;

	     /* try to center the window on the mouse pointer */
	     newWinX = rx;
	     newWinY = ry;
	     if (EoGetW(ewin))
		newWinX -= EoGetW(ewin) / 2;
	     if (EoGetH(ewin))
		newWinY -= EoGetH(ewin) / 2;

	     /* keep it all on this screen if possible */
	     newWinX = MIN(newWinX, VRoot.w - EoGetW(ewin));
	     newWinY = MIN(newWinY, VRoot.h - EoGetH(ewin));
	     newWinX = MAX(newWinX, 0);
	     newWinY = MAX(newWinY, 0);

	     /* this works for me... */
	     EoSetX(ewin, x = newWinX);
	     EoSetY(ewin, y = newWinY);
	  }
	else
	  {
	     ewin->client.already_placed = 1;
	     ArrangeEwinXY(ewin, &x, &y);
	  }
     }

   /* Force reparent if not on desk 0 */
   EoSetDesk(ewin, 0);

   /* if the window asked to be iconified at the start */
   if (ewin->client.start_iconified)
     {
#if 0				/* FIXME - Remove? */
	EwinBorderDraw(ewin, 1, 1, 0);
#endif
	MoveEwinToDesktopAt(ewin, desk, x, y);
	ecore_x_ungrab();
	ewin->state = EWIN_STATE_MAPPED;
	EwinIconify(ewin);
	ewin->state = EWIN_STATE_ICONIC;
	return;
     }

   /* if we should slide it in and are not currently in the middle of a slide */
   if ((manplace) && (!ewin->client.already_placed))
     {
	int                 rx, ry, wx, wy;
	unsigned int        mask;
	Window              junk, root_return;

#if 0				/* FIXME: Disable for now */
	/* if the loser has manual placement on and the app asks to be on */
	/*  a desktop, then send E to that desktop so the user can place */
	/* the window there */
	DeskGoto(desk);
#endif

	XQueryPointer(disp, VRoot.win, &root_return, &junk, &rx, &ry, &wx, &wy,
		      &mask);
	Mode.x = rx;
	Mode.y = ry;
	ewin->client.already_placed = 1;
	x = Mode.x + 1;
	y = Mode.y + 1;
#if 0				/* FIXME - Remove? */
	EwinBorderDraw(ewin, 1, 1, 0);
#endif
	MoveEwinToDesktop(ewin, desk);
	RaiseEwin(ewin);
	MoveEwin(ewin, x, y);
	RaiseEwin(ewin);
	ShowEwin(ewin);
	GrabPointerSet(VRoot.win, ECSR_GRAB, 0);
	Mode.have_place_grab = 1;
	Mode.place = 1;
	ecore_x_ungrab();
	EoSetFloating(ewin, 1);	/* Causes reparenting to root */
	ActionMoveStart(ewin, 1, 0, 0);
	return;
     }
   else if ((doslide) && (!Mode.doingslide))
     {
	MoveEwin(ewin, VRoot.w, VRoot.h);
	k = rand() % 4;
	if (k == 0)
	  {
	     fx = (rand() % (VRoot.w)) - EoGetW(ewin);
	     fy = -EoGetH(ewin);
	  }
	else if (k == 1)
	  {
	     fx = (rand() % (VRoot.w));
	     fy = VRoot.h;
	  }
	else if (k == 2)
	  {
	     fx = -EoGetW(ewin);
	     fy = (rand() % (VRoot.h));
	  }
	else
	  {
	     fx = VRoot.w;
	     fy = (rand() % (VRoot.h)) - EoGetH(ewin);
	  }
#if 0				/* FIXME - Remove? */
	EwinBorderDraw(ewin, 1, 1, 0);
#endif
	MoveEwinToDesktop(ewin, desk);
	RaiseEwin(ewin);
	MoveEwin(ewin, fx, fy);
	ShowEwin(ewin);
	SlideEwinTo(ewin, fx, fy, x, y, Conf.slidespeedmap);
	MoveEwinToDesktopAt(ewin, desk, x, y);
     }
   else
     {
#if 0				/* FIXME - Remove? */
	EwinBorderDraw(ewin, 1, 1, 0);
#endif
	MoveEwinToDesktopAt(ewin, desk, x, y);
	RaiseEwin(ewin);
	ShowEwin(ewin);
     }

   EwinDetermineArea(ewin);

   ecore_x_ungrab();
}

EWin               *
AddInternalToFamily(Window win, const char *bname, int type, void *ptr,
		    void (*init) (EWin * ewin, void *ptr))
{
   EWin               *ewin;
   Border             *b;

   b = NULL;
   if (bname)
     {
	b = FindItem(bname, 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
	if (!b)
	   b = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
     }

   ecore_x_grab();

   ewin = AdoptInternal(win, b, type, init, ptr);

#if 0
   Eprintf("Desk=%d, layer=%d, sticky=%d, floating=%d\n",
	   EoGetDesk(ewin), EoGetLayer(ewin), EoIsSticky(ewin),
	   EoIsFloating(ewin));
#endif

#if 0				/* FIXME - Remove? */
   EwinBorderDraw(ewin, 1, 1, 1);
#endif

   EwinConformToDesktop(ewin);
   EwinDetermineArea(ewin);

   ecore_x_ungrab();

   return ewin;
}

static void
EwinWithdraw(EWin * ewin)
{
   Window              win;
   int                 x, y;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinWithdraw %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   ecore_x_grab();

   /* Park the client window on the root */
   x = ewin->client.x;
   y = ewin->client.y;
   XTranslateCoordinates(disp, ewin->client.win, VRoot.win,
			 -ewin->border->border.left,
			 -ewin->border->border.top, &x, &y, &win);
   EReparentWindow(ewin->client.win, VRoot.win, x, y);
   ICCCM_Withdraw(ewin);
   HintsDelWindowHints(ewin);

   ecore_x_sync();
   ecore_x_ungrab();

   if (EwinIsInternal(ewin))
      EwinDestroy(ewin);
}

void
EwinConformToDesktop(EWin * ewin)
{
   Window              dwin;

   dwin = DeskGetWin(EoGetDesk(ewin));
   if ((ewin->iconified) && (ewin->parent != dwin))
     {
	ewin->parent = dwin;
	EReparentWindow(EoGetWin(ewin), dwin, EoGetX(ewin), EoGetY(ewin));
	RaiseEwin(ewin);
	ICCCM_Configure(ewin);
     }
   else if (EoIsFloating(ewin))
     {
	if ((ewin->parent != VRoot.win) && (EoIsFloating(ewin) == 2))
	  {
	     ewin->parent = VRoot.win;
	     EReparentWindow(EoGetWin(ewin), VRoot.win, EoGetX(ewin),
			     EoGetY(ewin));
	     EoSetDesk(ewin, 0);
	  }
	RaiseEwin(ewin);
	ICCCM_Configure(ewin);
	EdgeWindowsShow();
     }
   else if (ewin->parent != dwin)
     {
	ewin->parent = dwin;
	EReparentWindow(EoGetWin(ewin), dwin, EoGetX(ewin), EoGetY(ewin));
	RaiseEwin(ewin);
	MoveEwin(ewin, EoGetX(ewin), EoGetY(ewin));
     }
   else
     {
	MoveEwin(ewin, EoGetX(ewin), EoGetY(ewin));
     }

   /* FIXME - This should not be necessary. It is when a new window is added as
    * the only one in the lowest layer (e.g. desktop type).
    * In stead EobjListStackAdd() should mark the object stack dirty. */
   StackDesktop(EoGetDesk(ewin));

   EwinDetermineArea(ewin);
   HintsSetWindowDesktop(ewin);
}

void
EwinReparent(EWin * ewin, Window parent)
{
   EReparentWindow(ewin->client.win, parent, 0, 0);
}

static void
EwinEventMapRequest(EWin * ewin, Window win)
{
   if (ewin)
     {
	if (ewin->state == EWIN_STATE_ICONIC)
	   EwinDeIconify(ewin);
	if (ewin->state == EWIN_STATE_WITHDRAWN)
	   AddToFamily(ewin, win);
	else
	   Eprintf("AddToFamily: Already managing %s %#lx\n", "A",
		   ewin->client.win);
     }
   else
     {
	/* Check if we are already managing it */
	ewin = FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_EWIN);

	/* Some clients MapRequest more than once ?!? */
	if (ewin)
	  {
	     Eprintf("AddToFamily: Already managing %s %#lx\n", "B",
		     ewin->client.win);
	     ShowEwin(ewin);
	  }
	else
	   AddToFamily(NULL, win);
     }
}

static void
EwinEventDestroy(EWin * ewin)
{
   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventDestroy %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   EwinDestroy(ewin);
}

static void
EwinEventMap(EWin * ewin)
{
   int                 old_state = ewin->state;

   ewin->state = EWIN_STATE_MAPPED;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventMap %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   /* If first time we may want to focus it (unless during startup) */
   if (old_state == EWIN_STATE_NEW)
      FocusToEWin(ewin, FOCUS_EWIN_NEW);
   else
      FocusToEWin(ewin, FOCUS_SET);

   ModulesSignal(ESIGNAL_EWIN_CHANGE, ewin);
}

static void
EwinEventUnmap(EWin * ewin)
{
   if (GetZoomEWin() == ewin)
      Zoom(NULL);

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventUnmap %#lx %s state=%d\n", ewin->client.win,
	      EwinGetName(ewin), ewin->state);

   if (ewin->state == EWIN_STATE_WITHDRAWN)
      return;

   ActionsEnd(ewin);

   if (ewin == GetContextEwin())
      SlideoutsHide();

   if (ewin == Mode.focuswin)
      FocusToEWin(ewin, FOCUS_EWIN_GONE);
   if (ewin == Mode.mouse_over_ewin)
      Mode.mouse_over_ewin = NULL;
   if (ewin == Mode.context_ewin)
      Mode.context_ewin = NULL;

   if (Mode.doingslide)
     {
	DrawEwinShape(ewin, Conf.slidemode, ewin->shape_x, ewin->shape_y,
		      ewin->client.w, ewin->client.h, 2);
	Mode.doingslide = 0;
     }

   ewin->shown = 0;
   /* FIXME - This is to sync the client.win EXID mapped state */
   EUnmapWindow(ewin->client.win);
   EUnmapWindow(EoGetWin(ewin));

   ModulesSignal(ESIGNAL_EWIN_UNMAP, ewin);

   if (ewin->iconified)
     {
	ewin->state = EWIN_STATE_ICONIC;
	return;
     }

   ewin->state = EWIN_STATE_WITHDRAWN;

   if (ewin->Close)
      ewin->Close(ewin);

   if (WinGetParent(ewin->client.win) == ewin->win_container)
      EwinWithdraw(ewin);
}

static void
EwinEventConfigureRequest(EWin * ewin, XEvent * ev)
{
   Window              win, winrel;
   EWin               *ewin2;
   int                 x = 0, y = 0, w = 0, h = 0;
   XWindowChanges      xwc;

   win = ev->xconfigurerequest.window;

   if (ewin)
     {
	x = EoGetX(ewin);
	y = EoGetY(ewin);
	w = ewin->client.w;
	h = ewin->client.h;
	winrel = 0;
	if (ev->xconfigurerequest.value_mask & CWX)
	   x = ev->xconfigurerequest.x;
	if (ev->xconfigurerequest.value_mask & CWY)
	   y = ev->xconfigurerequest.y;
	if (ev->xconfigurerequest.value_mask & CWWidth)
	   w = ev->xconfigurerequest.width;
	if (ev->xconfigurerequest.value_mask & CWHeight)
	   h = ev->xconfigurerequest.height;
	if (ev->xconfigurerequest.value_mask & CWSibling)
	   winrel = ev->xconfigurerequest.above;
	if (ev->xconfigurerequest.value_mask & CWStackMode)
	  {
	     ewin2 = FindItem(NULL, winrel, LIST_FINDBY_ID, LIST_TYPE_EWIN);
	     if (ewin2)
		winrel = EoGetWin(ewin2);
	     xwc.sibling = winrel;
	     xwc.stack_mode = ev->xconfigurerequest.detail;
	     if (Mode.mode == MODE_NONE)
	       {
		  if (xwc.stack_mode == Above)
		     RaiseEwin(ewin);
		  else if (xwc.stack_mode == Below)
		     LowerEwin(ewin);
	       }
	     /*        else
	      * XConfigureWindow(disp, EoGetWin(ewin),
	      * ev->xconfigurerequest.value_mask &
	      * (CWSibling | CWStackMode), &xwc); */
	  }
#if 0				/* Let's try disabling this */
	/* this ugly workaround here is because x11amp is very brain-dead */
	/* and sets its minunum and maximm sizes the same - fair enough */
	/* to ensure it doesnt get resized - BUT hwne it shades itself */
	/* it resizes down to a smaller size - of course keeping the */
	/* minimum and maximim size same - E unconditionally disallows any */
	/* client window to be resized outside of its constraints */
	/* (any client could do this resize - not just x11amp thus E is */
	/* imposing the hints x11amp set up - this works around by */
	/* modifying the constraints to fit what the app requested */
	if (w < ewin->client.width.min)
	   ewin->client.width.min = w;
	if (w > ewin->client.width.max)
	   ewin->client.width.max = w;
	if (h < ewin->client.height.min)
	   ewin->client.height.min = h;
	if (h > ewin->client.height.max)
	   ewin->client.height.max = h;
#endif

	if (ev->xconfigurerequest.value_mask & (CWX | CWY))
	  {
	     /* Correct position taking gravity into account */
	     ewin->client.x = x;
	     ewin->client.y = y;
	     EwinGetPosition(ewin, &x, &y);
	  }

	Mode.move.check = 0;	/* Don't restrict client requests */
	MoveResizeEwin(ewin, x, y, w, h);
	Mode.move.check = 1;

	{
	   char                pshaped;

	   pshaped = ewin->client.shaped;
	   ICCCM_GetShapeInfo(ewin);
	   if (pshaped != ewin->client.shaped)
	     {
		SyncBorderToEwin(ewin);
		EwinPropagateShapes(ewin);
	     }
	}
	ReZoom(ewin);
     }
   else
     {
	xwc.x = ev->xconfigurerequest.x;
	xwc.y = ev->xconfigurerequest.y;
	xwc.width = ev->xconfigurerequest.width;
	xwc.height = ev->xconfigurerequest.height;
	xwc.border_width = ev->xconfigurerequest.border_width;
	xwc.sibling = ev->xconfigurerequest.above;
	xwc.stack_mode = ev->xconfigurerequest.detail;
	EConfigureWindow(win, ev->xconfigurerequest.value_mask, &xwc);
     }
}

static void
EwinEventResizeRequest(EWin * ewin, XEvent * ev)
{
   Window              win;
   int                 w, h;

   win = ev->xresizerequest.window;

   if (ewin)
     {
	w = ev->xresizerequest.width;
	h = ev->xresizerequest.height;
	ResizeEwin(ewin, w, h);
	{
	   char                pshaped;

	   pshaped = ewin->client.shaped;
	   ICCCM_GetShapeInfo(ewin);
	   if (pshaped != ewin->client.shaped)
	     {
		SyncBorderToEwin(ewin);
		EwinPropagateShapes(ewin);
	     }
	}
	ReZoom(ewin);
     }
   else
     {
	EResizeWindow(win, ev->xresizerequest.width, ev->xresizerequest.height);
     }
}

static void
EwinEventCirculateRequest(EWin * ewin, XEvent * ev)
{
   Window              win;

   win = ev->xcirculaterequest.window;

   if (ewin)
     {
	if (ev->xcirculaterequest.place == PlaceOnTop)
	   RaiseEwin(ewin);
	else
	   LowerEwin(ewin);
     }
   else
     {
	if (ev->xcirculaterequest.place == PlaceOnTop)
	   ERaiseWindow(win);
	else
	   ELowerWindow(win);
     }
}

static void
EwinEventPropertyNotify(EWin * ewin, XEvent * ev)
{
   ecore_x_grab();
   EwinChangesStart(ewin);

   HintsProcessPropertyChange(ewin, ev->xproperty.atom);
   SessionGetInfo(ewin, ev->xproperty.atom);
   SyncBorderToEwin(ewin);

   EwinChangesProcess(ewin);
   ecore_x_ungrab();
}

static void
EwinEventShapeChange(EWin * ewin)
{
   const Border       *b;

   b = ewin->border;
   SyncBorderToEwin(ewin);
   if (ewin->border == b)
      EwinPropagateShapes(ewin);
}

static void
EwinEventVisibility(EWin * ewin, int state)
{
   ewin->visibility = state;
}

void
EwinRefresh(EWin * ewin)
{
   if (!ewin)
      return;

   if (TransparencyEnabled())
      EwinBorderDraw(ewin, 0, 1, 0);	/* Update the border */

   if (ewin->Refresh)
      ewin->Refresh(ewin);
}

void
EwinUpdateAfterMoveResize(EWin * ewin, int resize)
{
   if (!ewin)
      return;

   EwinDetermineArea(ewin);

   if (TransparencyEnabled())
      EwinBorderDraw(ewin, resize, 1, 0);	/* Update the border */

   if (ewin->MoveResize)
      ewin->MoveResize(ewin, resize);

   SnapshotEwinUpdate(ewin, SNAP_USE_POS | SNAP_USE_SIZE);

   ModulesSignal(ESIGNAL_EWIN_CHANGE, ewin);
}

#if 0				/* Unused */
void
FloatEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   call_depth++;
   if (call_depth > 256)
      return;

   EoSetFloating(ewin, 1);
   EoSetDesk(ewin, 0);
   EwinConformToDesktop(ewin);
   RaiseEwin(ewin);

   lst = EwinListTransients(ewin, &num, 0);
   for (i = 0; i < num; i++)
      FloatEwin(lst[i]);
   if (lst)
      Efree(lst);

   call_depth--;
}
#endif

void
FloatEwinAt(EWin * ewin, int x, int y)
{
   static int          call_depth = 0;
   int                 dx, dy;
   EWin              **lst;
   int                 i, num;

   call_depth++;
   if (call_depth > 256)
      return;

   if (EoIsFloating(ewin))
     {
	EoSetFloating(ewin, 2);
     }
   else
     {
	ewin->ld = EoGetDesk(ewin);
	EoSetFloating(ewin, 1);
     }

   dx = x - EoGetX(ewin);
   dy = y - EoGetY(ewin);
   EoSetX(ewin, x);
   EoSetY(ewin, y);
   EwinConformToDesktop(ewin);

   lst = EwinListTransients(ewin, &num, 0);
   for (i = 0; i < num; i++)
      FloatEwinAt(lst[i], EoGetX(lst[i]) + dx, EoGetY(lst[i]) + dy);
   if (lst)
      Efree(lst);

   call_depth--;
}

/*
 * Place particular EWin at appropriate location in the window stack
 */
static void
RestackEwin(EWin * ewin)
{
   EWin               *const *lst;
   int                 i, num;
   XWindowChanges      xwc;
   unsigned int        value_mask;

   if (EventDebug(EDBUG_TYPE_STACKING))
      Eprintf("RestackEwin %#lx %s\n", ewin->client.win, EwinGetName(ewin));

#if 0				/* FIXME - remove? */
   if (EoIsFloating(ewin))
     {
	ERaiseWindow(EoGetWin(ewin));
	goto done;
     }
#endif

   lst = EwinListGetForDesk(&num, EoGetDesk(ewin));
   if (num < 2)
      goto done;

   for (i = 0; i < num; i++)
      if (lst[i] == ewin)
	 break;
   if (i < num - 1)
     {
	xwc.stack_mode = Above;
	xwc.sibling = EoGetWin(lst[i + 1]);
     }
   else
     {
	xwc.stack_mode = Below;
	xwc.sibling = EoGetWin(lst[i - 1]);
     }
   value_mask = CWSibling | CWStackMode;
   if (EventDebug(EDBUG_TYPE_STACKING))
      Eprintf("RestackEwin %#10lx %s %#10lx\n", EoGetWin(ewin),
	      (xwc.stack_mode == Above) ? "Above" : "Below", xwc.sibling);
   XConfigureWindow(disp, EoGetWin(ewin), value_mask, &xwc);
   HintsSetClientStacking();
   ModulesSignal(ESIGNAL_EWIN_CHANGE, ewin);

 done:
   ;
}

void
RaiseEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   if (call_depth > 256)
      return;
   call_depth++;

   if (EventDebug(EDBUG_TYPE_RAISELOWER))
      Eprintf("RaiseEwin(%d) %#lx %s\n", call_depth, ewin->client.win,
	      EwinGetName(ewin));

   if (EoGetWin(ewin))
     {
#if 0				/* FIXME - remove? */
	if (EoIsFloating(ewin))
	  {
	     ERaiseWindow(EoGetWin(ewin));
	     goto done;
	  }
#endif

	num = EwinListStackRaise(ewin);
	if (num == 0)		/* Quit if stacking is unchanged */
	   goto done;

	lst = EwinListTransients(ewin, &num, 1);
	for (i = 0; i < num; i++)
	   RaiseEwin(lst[i]);
	if (lst)
	   Efree(lst);

	if (call_depth == 1)
	  {
	     if (num > 0)
		StackDesktop(EoGetDesk(ewin));	/* Do the full stacking */
	     else
		RestackEwin(ewin);	/* Restack this one only */
	  }
     }

 done:
   call_depth--;
}

void
LowerEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   if (call_depth > 256)
      return;
   call_depth++;

   if (EventDebug(EDBUG_TYPE_RAISELOWER))
      Eprintf("LowerEwin(%d) %#lx %s\n", call_depth, ewin->client.win,
	      EwinGetName(ewin));

#if 0				/* FIXME - remove? */
   if ((EoGetWin(ewin)) && (!EoIsFloating(ewin)))
#else
   if (EoGetWin(ewin))
#endif
     {
	num = EwinListStackLower(ewin);
	if (num == 0)		/* Quit if stacking is unchanged */
	   goto done;

	lst = EwinListTransientFor(ewin, &num);
	for (i = 0; i < num; i++)
	   LowerEwin(lst[i]);
	if (lst)
	   Efree(lst);

	if (call_depth == 1)
	  {
	     if (num > 0)
		StackDesktop(EoGetDesk(ewin));	/* Do the full stacking */
	     else
		RestackEwin(ewin);	/* Restack this one only */
	  }
     }

 done:
   call_depth--;
}

void
ShowEwin(EWin * ewin)
{
   if (ewin->shown)
      return;
   ewin->shown = 1;

   if (ewin->client.win)
     {
	if (ewin->shaded)
	   EMoveResizeWindow(ewin->win_container, -30, -30, 1, 1);
	EMapWindow(ewin->client.win);
     }

   if (EoGetWin(ewin))
      EMapWindow(EoGetWin(ewin));
}

void
HideEwin(EWin * ewin)
{
   if (!ewin->shown || !EwinIsMapped(ewin))
      return;
   ewin->shown = 0;

   if (GetZoomEWin() == ewin)
      Zoom(NULL);

   EUnmapWindow(ewin->client.win);

   if (EoGetWin(ewin))
      EUnmapWindow(EoGetWin(ewin));
}

Window
EwinGetClientWin(const EWin * ewin)
{
   return (ewin) ? ewin->client.win : None;
}

const char         *
EwinGetName(const EWin * ewin)
{
   const char         *name;

   if (!ewin)
      return NULL;
#if ENABLE_EWMH
   name = ewin->ewmh.wm_name;
   if (name)
      goto done;
#endif
   name = ewin->icccm.wm_name;
   if (name)
      goto done;

 done:
   return (name && name[0]) ? name : NULL;
}

const char         *
EwinGetIconName(const EWin * ewin)
{
   const char         *name;

#if ENABLE_EWMH
   name = ewin->ewmh.wm_icon_name;
   if (name)
      goto done;
#endif
   name = ewin->icccm.wm_icon_name;
   if (name)
      goto done;

   return EwinGetName(ewin);

 done:
   return (name && strlen(name)) ? name : NULL;
}

int
EwinIsOnScreen(EWin * ewin)
{
   int                 x, y, w, h;

   if (EoIsSticky(ewin))
      return 1;
   if (EoGetDesk(ewin) != DesksGetCurrent())
      return 0;

   x = EoGetX(ewin);
   y = EoGetY(ewin);
   w = EoGetW(ewin);
   h = EoGetH(ewin);

   if (x + w <= 0 || x >= VRoot.w || y + h <= 0 || y >= VRoot.h)
      return 0;

   return 1;
}

/*
 * Save current position in absolute viewport coordinates
 */
void
EwinRememberPositionSet(EWin * ewin)
{
   int                 ax, ay;

   ewin->req_x = EoGetX(ewin);
   ewin->req_y = EoGetY(ewin);
   if (!EoIsSticky(ewin))
     {
	DeskGetArea(EoGetDesk(ewin), &ax, &ay);
	ewin->req_x += ax * VRoot.w;
	ewin->req_y += ay * VRoot.h;
     }
}

/*
 * Get saved position in relative viewport coordinates
 */
void
EwinRememberPositionGet(EWin * ewin, int *px, int *py)
{
   int                 x, y, ax, ay;

   x = ewin->req_x;
   y = ewin->req_y;
   if (!EoIsSticky(ewin))
     {
	DeskGetArea(EoGetDesk(ewin), &ax, &ay);
	x -= ax * VRoot.w;
	y -= ay * VRoot.h;
     }

   *px = x;
   *py = y;
}

/*
 * Change requests
 */
static struct
{
   unsigned int        flags;
   EWin                ewin_old;
} EWinChanges;

void
EwinChange(EWin * ewin, unsigned int flag)
{
   EWinChanges.flags |= flag;
   return;
   ewin = NULL;
}

void
EwinChangesStart(EWin * ewin)
{
   EWinChanges.flags = 0;
   /* Brute force :) */
   EWinChanges.ewin_old = *ewin;
}

void
EwinChangesProcess(EWin * ewin)
{
   if (!EWinChanges.flags)
      return;

   if (EWinChanges.flags & EWIN_CHANGE_NAME)
     {
	EwinBorderUpdateInfo(ewin);
	EwinBorderCalcSizes(ewin);
     }

   if (EWinChanges.flags & EWIN_CHANGE_DESKTOP)
     {
	int                 desk, pdesk;

	desk = EoGetDesk(ewin);
	pdesk = EoGetDesk(&EWinChanges.ewin_old);
	if (desk != pdesk && !EoIsSticky(ewin))
	  {
	     EoSetDesk(ewin, pdesk);
	     MoveEwinToDesktop(ewin, desk);
	  }
     }

   if (EWinChanges.flags & EWIN_CHANGE_ICON_PMAP)
     {
	ModulesSignal(ESIGNAL_EWIN_CHANGE_ICON, ewin);
     }

   EWinChanges.flags = 0;
}

void
EwinsEventsConfigure(int mode)
{
   EWin               *const *lst, *ewin;
   int                 i, num;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];

	EwinEventsConfigure(lst[i], mode);

	/* This is a hack. Maybe we should do something with expose events. */
	if (mode)
	   if (Mode.mode == MODE_DESKSWITCH && EoIsSticky(ewin) && ewin->shown)
	      EwinRefresh(ewin);
     }
}

static void
EwinsTouch(void)
{
   int                 i, num;
   EWin               *const *lst, *ewin;

   lst = EwinListStackGet(&num);
   for (i = num - 1; i >= 0; i--)
     {
	ewin = lst[i];
	if (EwinIsMapped(ewin))
	   MoveEwin(ewin, EoGetX(ewin), EoGetY(ewin));
     }
}

void
EwinsSetFree(void)
{
   int                 i, num;
   EWin               *const *lst, *ewin;

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("EwinsSetFree\n");

   lst = EwinListStackGet(&num);
   for (i = num - 1; i >= 0; i--)
     {
	ewin = lst[i];
	if (EwinIsInternal(ewin))
	   continue;

	/* This makes E determine the client window stacking at exit */
	EwinInstantUnShade(ewin);
	EReparentWindow(ewin->client.win, VRoot.win,
			ewin->client.x, ewin->client.y);
     }
}

/*
 * Event handlers
 */

static int
ActionsCheck(const char *which, EWin * ewin, XEvent * ev)
{
   ActionClass        *ac;

   if (Mode.action_inhibit)	/* Probably not here */
      return 0;

   ac = FindItem(which, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
   if (!ac)
      return 0;

   if (ev->type == ButtonPress)
      GrabPointerSet(EoGetWin(ewin), ECSR_GRAB, 0);
   else if (ev->type == ButtonRelease)
      GrabPointerRelease();

   return EventAclass(ev, ewin, ac);
}

#define DEBUG_EWIN_EVENTS 0
static void
EwinHandleEventsToplevel(XEvent * ev, void *prm)
{
   EWin               *ewin = (EWin *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	ActionsCheck("BUTTONBINDINGS", ewin, ev);
	break;
     case ButtonRelease:
	ActionsCheck("BUTTONBINDINGS", ewin, ev);
	break;
     case EnterNotify:
	FocusHandleEnter(ewin, ev);
	break;
     case LeaveNotify:
	FocusHandleLeave(ewin, ev);
	break;
     case MotionNotify:
	break;
     default:
#if DEBUG_EWIN_EVENTS
	Eprintf("EwinHandleEventsToplevel: type=%2d win=%#lx: %s\n",
		ev->type, ewin->client.win, EwinGetName(ewin));
#endif
	break;
     }
}

static void
EwinHandleEventsContainer(XEvent * ev, void *prm)
{
   EWin               *ewin = (EWin *) prm;

#if 0
   Eprintf("EwinHandleEventsContainer: type=%2d win=%#lx: %s\n",
	   ev->type, ewin->client.win, EwinGetName(ewin));
#endif
   switch (ev->type)
     {
     case ButtonPress:
	FocusHandleClick(ewin, ev->xany.window);
	break;
     case MapRequest:
	EwinEventMapRequest(ewin, ev->xmaprequest.window);
	break;
     case ConfigureRequest:
	EwinEventConfigureRequest(ewin, ev);
	break;
     case ResizeRequest:
	EwinEventResizeRequest(ewin, ev);
	break;
     case CirculateRequest:
	EwinEventCirculateRequest(ewin, ev);
	break;

     case DestroyNotify:
	EwinEventDestroy(ewin);
	break;
     case UnmapNotify:
#if 0
	if (ewin->state == EWIN_STATE_NEW)
	  {
	     Eprintf("EwinEventUnmap %#lx: Ignoring bogus Unmap event\n",
		     ewin->client.win);
	     break;
	  }
#endif
	EwinEventUnmap(ewin);
	break;
     case MapNotify:
	EwinEventMap(ewin);
	break;
     case ReparentNotify:
	/* Check if window parent hasn't changed already (compress?) */
	if (WinGetParent(ev->xreparent.window) != ev->xreparent.parent)
	   break;
	if (ev->xreparent.parent != ewin->win_container)
	   EwinEventDestroy(ewin);
	break;

     case GravityNotify:
     case ConfigureNotify:
	break;

     default:
	Eprintf("EwinHandleEventsContainer: type=%2d win=%#lx: %s\n",
		ev->type, ewin->client.win, EwinGetName(ewin));
	break;
     }
}

static void
EwinHandleEventsClient(XEvent * ev, void *prm)
{
   EWin               *ewin = (EWin *) prm;

   switch (ev->type)
     {
     case ButtonPress:
     case ButtonRelease:
     case MotionNotify:
     case EnterNotify:
     case LeaveNotify:
     case FocusIn:
     case FocusOut:
     case ConfigureNotify:
     case GravityNotify:
	break;
     case VisibilityNotify:
	EwinEventVisibility(ewin, ev->xvisibility.state);
	break;

#if 0				/* FIXME - Remove? */
     case DestroyNotify:
	EwinEventDestroy(ewin);
	break;
     case UnmapNotify:
#if 0
	if (ewin->state == EWIN_STATE_NEW)
	  {
	     Eprintf("EwinEventUnmap %#lx: Ignoring bogus Unmap event\n",
		     ewin->client.win);
	     break;
	  }
#endif
	EwinEventUnmap(ewin);
	break;
     case MapNotify:
	EwinEventMap(ewin);
	break;
     case ReparentNotify:
	/* Check if window parent hasn't changed already (compress?) */
	if (WinGetParent(ev->xreparent.window) != ev->xreparent.parent)
	   break;
	if (ev->xreparent.parent == VRoot.win)
	   EwinEventDestroy(ewin);
	break;
#endif

#if 0
     case ConfigureRequest:
	if (ev->xconfigurerequest.window == ewin->client.win)
	   EwinEventConfigureRequest(ewin, ev);
	break;
     case ResizeRequest:
	if (ev->xresizerequest.window == ewin->client.win)
	   EwinEventResizeRequest(ewin, ev);
	break;
     case CirculateRequest:
	EwinEventCirculateRequest(ewin, ev);
	break;
#endif
     case PropertyNotify:
	EwinEventPropertyNotify(ewin, ev);
	break;
     case EX_EVENT_SHAPE_NOTIFY:
	EwinEventShapeChange(ewin);
     default:
#if DEBUG_EWIN_EVENTS
	Eprintf("EwinHandleEventsClient: type=%2d win=%#lx: %s\n",
		ev->type, ewin->client.win, EwinGetName(ewin));
#endif
	break;
     }
}

static void
EwinHandleEventsRoot(XEvent * ev, void *prm __UNUSED__)
{
   EWin               *ewin;

   switch (ev->type)
     {
     case EnterNotify:
	FocusHandleEnter(NULL, ev);
	break;
     case LeaveNotify:
	FocusHandleLeave(NULL, ev);
	break;

     case MapRequest:
	EwinEventMapRequest(NULL, ev->xmaprequest.window);
	break;
     case ConfigureRequest:
#if 0
	Eprintf("EwinHandleEventsRoot ConfigureRequest %#lx\n",
		ev->xconfigurerequest.window);
#endif
	ewin = FindItem(NULL, ev->xconfigurerequest.window, LIST_FINDBY_ID,
			LIST_TYPE_EWIN);
	EwinEventConfigureRequest(ewin, ev);
	break;
     case ResizeRequest:
#if 0
	Eprintf("EwinHandleEventsRoot ResizeRequest %#lx\n",
		ev->xresizerequest.window);
#endif
	ewin = FindItem(NULL, ev->xresizerequest.window, LIST_FINDBY_ID,
			LIST_TYPE_EWIN);
	EwinEventResizeRequest(ewin, ev);
	break;
     case CirculateRequest:
#if 0
	Eprintf("EwinHandleEventsRoot CirculateRequest %#lx\n",
		ev->xcirculaterequest.window);
#endif
	EwinEventCirculateRequest(NULL, ev);
	break;

     case UnmapNotify:
	/* Catch clients unmapped after MapRequest but before being reparented */
	ewin = FindItem(NULL, ev->xunmap.window, LIST_FINDBY_ID,
			LIST_TYPE_EWIN);
	if (ewin)
	   EwinEventUnmap(ewin);
	break;

     case DestroyNotify:
	/* Catch clients destroyed after MapRequest but before being reparented */
	ewin = FindItem(NULL, ev->xdestroywindow.window, LIST_FINDBY_ID,
			LIST_TYPE_EWIN);
	if (!ewin)
	   ewin = FindEwinByBase(ev->xdestroywindow.window);
	if (ewin)
	   EwinEventDestroy(ewin);
	break;

     default:
#if 0
	Eprintf("EwinHandleEventsRoot: type=%2d win=%#lx\n",
		ev->type, ev->xany.window);
#endif
	break;
     }
}

static void
EwinsInit(void)
{
   EventCallbackRegister(VRoot.win, 0, EwinHandleEventsRoot, NULL);
}

/*
 * Ewins module
 * This is the WM.
 */

static void
EwinsSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	EwinsInit();
	break;
#if 0
     case ESIGNAL_CONFIGURE:
	if (!Conf.mapslide || Mode.wm.restart)
	   MapUnmap(1);
	break;
     case ESIGNAL_START:
	if (Conf.mapslide && !Mode.wm.restart)
	   MapUnmap(1);
	break;
#endif
     case ESIGNAL_DESK_RESIZE:
	EwinsTouch();
	break;
     }
}

#if 0
IpcItem             EwinsIpcArray[] = {
};
#define N_IPC_FUNCS (sizeof(EwinsIpcArray)/sizeof(IpcItem))
#else
#define N_IPC_FUNCS   0
#define EwinsIpcArray NULL
#endif

/*
 * Module descriptor
 */
EModule             ModEwins = {
   "ewins", NULL,
   EwinsSighan,
   {N_IPC_FUNCS, EwinsIpcArray}
   ,
   {0, NULL}
};
