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
#include <time.h>

#define EDESK_EVENT_MASK \
  (KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
   EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | \
   SubstructureNotifyMask | SubstructureRedirectMask | PropertyChangeMask)

struct _edesk
{
   char                viewable;
   Window              win;
   int                 x, y;
   Background         *bg;
   Button             *tag;
   int                 current_area_x;
   int                 current_area_y;
   long                event_mask;
};

#define ENLIGHTENMENT_CONF_NUM_DESKTOPS 32

typedef struct _desktops
{
   int                 current;
   Desk                desk[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
   int                 order[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
}
Desktops;

static void         DesktopHandleEvents(XEvent * ev, void *prm);

/* The desktops */
static Desktops     desks;

Window
DeskGetWin(int desk)
{
   return desks.desk[desk].win;
}

int
DeskGetX(int desk)
{
   return desks.desk[desk].x;
}

int
DeskGetY(int desk)
{
   return desks.desk[desk].y;
}

Background         *
DeskGetBackground(int desk)
{
   return desks.desk[desk].bg;
}

void
DeskGetArea(int desk, int *ax, int *ay)
{
   *ax = desks.desk[desk].current_area_x;
   *ay = desks.desk[desk].current_area_y;
}

void
DeskSetArea(int desk, int ax, int ay)
{
   desks.desk[desk].current_area_x = ax;
   desks.desk[desk].current_area_y = ay;
}

int
DeskIsViewable(int desk)
{
   return desks.desk[desk].viewable;
}

void
DeskSetViewable(int desk, int on)
{
   desks.desk[desk].viewable = on;
}

Window
DeskGetCurrentRoot(void)
{
   return DeskGetWin(desks.current);
}

void
DeskGetCurrentArea(int *ax, int *ay)
{
   DeskGetArea(desks.current, ax, ay);
}

void
DeskSetCurrentArea(int ax, int ay)
{
   DeskSetArea(desks.current, ax, ay);
}

int
DesksGetNumber(void)
{
   return Conf.desks.num;
}

int
DesksGetTotal(void)
{
   return ENLIGHTENMENT_CONF_NUM_DESKTOPS;
}

int
DesksGetCurrent(void)
{
   return desks.current;
}

void
DesksSetCurrent(int desk)
{
   desks.current = desk;
}

static void
DesktopInit(unsigned int dsk)
{
   Desk               *d;

   if (dsk >= ENLIGHTENMENT_CONF_NUM_DESKTOPS)
      return;

   d = &desks.desk[dsk];
   d->bg = NULL;
   desks.order[dsk] = dsk;
   d->tag = NULL;
   d->x = 0;
   d->y = 0;
   d->current_area_x = 0;
   d->current_area_y = 0;
   d->viewable = 0;

   if (dsk == 0)
     {
	d->win = VRoot.win;
     }
   else
     {
	d->win =
	   ECreateWindow(VRoot.win, -VRoot.w, -VRoot.h, VRoot.w, VRoot.h, 0);
#if 0				/* USE_COMPOSITE */
	EobjRegister(d->win, EOBJ_TYPE_DESK);
#endif
     }
   EventCallbackRegister(d->win, 0, DesktopHandleEvents, d);
}

static void
DesktopsInit(void)
{
   int                 i;

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
      DesktopInit(i);
}

static void
ChangeNumberOfDesktops(int quantity)
{
   int                 pnum, i, num;
   EWin               *const *lst;

   pnum = Conf.desks.num;
   for (i = quantity; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
      LowerDesktop(i);
   Conf.desks.num = quantity;

   if (Conf.desks.num <= 0)
      Conf.desks.num = 1;
   else if (Conf.desks.num > ENLIGHTENMENT_CONF_NUM_DESKTOPS)
      Conf.desks.num = ENLIGHTENMENT_CONF_NUM_DESKTOPS;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (EoGetDesk(lst[i]) >= Conf.desks.num)
	   MoveEwinToDesktop(lst[i], Conf.desks.num - 1);
     }

   if (Conf.desks.num > pnum)
     {
	for (i = pnum; i < Conf.desks.num; i++)
	   ModulesSignal(ESIGNAL_DESK_ADDED, (void *)i);
     }
   else if (Conf.desks.num < pnum)
     {
	for (i = Conf.desks.num; i < pnum; i++)
	   ModulesSignal(ESIGNAL_DESK_REMOVED, (void *)i);
     }
   if (DesksGetCurrent() >= Conf.desks.num)
      GotoDesktop(Conf.desks.num - 1);

   HintsSetDesktopConfig();
}

void
ShowDesktopControls(void)
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 1);
   if (blst)
     {
	for (i = 0; i < num; i++)
	   ButtonShow(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}

#if 0				/* Unused */
static void
ShowDesktopTabs(void)
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 2);
   if (blst)
     {
	for (i = 0; i < num; i++)
	   ButtonShow(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}

static void
HideDesktopTabs(void)
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 2);
   if (blst)
     {
	for (i = 0; i < num; i++)
	   ButtonHide(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}
#endif

static void
ShowDesktopButtons(void)
{
   Button            **blst;
   int                 i, num;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
   if (blst)
     {
	for (i = 0; i < num; i++)
	  {
	     if (ButtonDoShowDefault(blst[i]))
		ButtonShow(blst[i]);
	  }
	Efree(blst);
     }
}

static void
MoveToDeskTop(int num)
{
   int                 i, j;

   j = -1;
   i = 0;
   while ((j < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
     {
	if (desks.order[i] == num)
	   j = i;
	i++;
     }
   if (j < 0)
      return;
   if (j > 0)
     {
	for (i = j - 1; i >= 0; i--)
	   desks.order[i + 1] = desks.order[i];
	desks.order[0] = num;
     }
}

static void
MoveToDeskBottom(int num)
{
   int                 i, j;

   j = -1;
   i = 0;
   while ((j < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
     {
	if (desks.order[i] == num)
	   j = i;
	i++;
     }
   if (j < 0)
      return;
   if (j < ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1)
     {
	for (i = j; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1; i++)
	   desks.order[i] = desks.order[i + 1];
	desks.order[ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1] = num;
     }
}

void
SlideWindowTo(Window win, int fx, int fy, int tx, int ty, int speed)
{
   int                 k, x, y;

   ecore_x_grab();

   ETimedLoopInit(0, 1024, speed);
   for (k = 0; k <= 1024;)
     {
	x = ((fx * (1024 - k)) + (tx * k)) >> 10;
	y = ((fy * (1024 - k)) + (ty * k)) >> 10;
	EMoveWindow(disp, win, x, y);
	ecore_x_sync();

	k = ETimedLoopNext();
     }
   EMoveWindow(disp, win, tx, ty);

   ecore_x_ungrab();
}

void
RefreshDesktop(int desk)
{
   Background         *bg;

   desk = desk % ENLIGHTENMENT_CONF_NUM_DESKTOPS;
   if (!desks.desk[desk].viewable)
      return;

   if (EventDebug(EDBUG_TYPE_DESKS))
      Eprintf("RefreshDesktop %d\n", desk);

   bg = desks.desk[desk].bg;
   if (!bg)
      return;

   BackgroundApply(bg, desks.desk[desk].win, 1);
   HintsSetRootInfo(desks.desk[desk].win,
		    BackgroundGetPixmap(bg), BackgroundGetColor(bg));
}

void
DesktopsRefresh(void)
{
   int                 i;

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	if (desks.desk[i].bg)
	   DesktopSetBg(i, desks.desk[i].bg, 1);
     }
}

void
InitDesktopControls(void)
{
   int                 i;
   ActionClass        *ac, *ac2, *ac3;
   ImageClass         *ic, *ic2, *ic3, *ic4;
   Button             *b;
   Action             *a;
   int                 x[3], y[3], w[3], h[3], m, n, o;
   char                s[512];
   const char         *t;

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	Esnprintf(s, sizeof(s), "DRAGBAR_DESKTOP_%i", i);

	ac = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac)
	  {
	     ac = ActionclassCreate(s, 0);
	     a = ActionCreate(EVENT_MOUSE_DOWN, 0, 0, 0, 1, 0, NULL, NULL);
	     ActionclassAddAction(ac, a);

	     Esnprintf(s, sizeof(s), "desk drag %i", i);
	     ActionAddTo(a, s);

	     a = ActionCreate(EVENT_MOUSE_DOWN, 0, 0, 0, 3, 0, NULL, NULL);
	     ActionclassAddAction(ac, a);
	     ActionAddTo(a, "menus show deskmenu");

	     a = ActionCreate(EVENT_MOUSE_DOWN, 0, 0, 0, 2, 0, NULL, NULL);
	     ActionclassAddAction(ac, a);
	     ActionAddTo(a, "menus show taskmenu");

	     if (i > 0)
	       {
		  t = _("Hold down the mouse button and drag\n"
			"the mouse to be able to drag the desktop\n"
			"back and forth.\n"
			"Click right mouse button for a list of all\n"
			"Desktops and their applications.\n"
			"Click middle mouse button for a list of all\n"
			"applications currently running.\n");
		  ActionclassSetTooltipString(ac, t);
	       }
	     else
	       {
		  t = _("This is the Root desktop.\n"
			"You cannot drag the root desktop around.\n"
			"Click right mouse button for a list of all\n"
			"Desktops and their applications.\n"
			"Click middle mouse button for a list of all\n"
			"applications currently running.\n");
		  ActionclassSetTooltipString(ac, t);
	       }
	  }
	Esnprintf(s, sizeof(s), "RAISEBUTTON_DESKTOP_%i", i);
	ac2 = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac2)
	  {
	     ac2 = ActionclassCreate(s, 0);
	     a = ActionCreate(EVENT_MOUSE_UP, 1, 0, 1, 0, 0, NULL, NULL);
	     ActionclassAddAction(ac2, a);

	     Esnprintf(s, sizeof(s), "desk raise %i", i);
	     ActionAddTo(a, s);
	     t = _("Click here to raise this desktop\nto the top.\n");
	     ActionclassSetTooltipString(ac, t);
	  }
	Esnprintf(s, sizeof(s), "LOWERBUTTON_DESKTOP_%i", i);
	ac3 = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac3)
	  {
	     ac3 = ActionclassCreate(s, 0);
	     a = ActionCreate(EVENT_MOUSE_UP, 1, 0, 1, 0, 0, NULL, NULL);
	     ActionclassAddAction(ac3, a);

	     Esnprintf(s, sizeof(s), "desk lower %i", i);
	     ActionAddTo(a, s);
	     t = _("Click here to lower this desktop\nto the bottom.\n");
	     ActionclassSetTooltipString(ac, t);
	  }
	b = NULL;

	if (Conf.desks.dragdir < 2)
	  {
	     ic = ImageclassFind("DESKTOP_DRAGBUTTON_VERT", 0);
	     ic2 = ImageclassFind("DESKTOP_RAISEBUTTON_VERT", 0);
	     ic3 = ImageclassFind("DESKTOP_LOWERBUTTON_VERT", 0);
	     ic4 = ImageclassFind("DESKTOP_DESKRAY_VERT", 0);
	  }
	else
	  {
	     ic = ImageclassFind("DESKTOP_DRAGBUTTON_HORIZ", 0);
	     ic2 = ImageclassFind("DESKTOP_RAISEBUTTON_HORIZ", 0);
	     ic3 = ImageclassFind("DESKTOP_LOWERBUTTON_HORIZ", 0);
	     ic4 = ImageclassFind("DESKTOP_DESKRAY_HORIZ", 0);
	  }

	switch (Conf.desks.dragbar_ordering)
	  {
	  case 0:
	     m = 0;
	     n = 1;
	     o = 2;
	     break;
	  case 1:
	     m = 0;
	     n = 2;
	     o = 1;
	     break;
	  case 2:
	     m = 2;
	     n = 0;
	     o = 1;
	     break;
	  case 3:
	     m = 1;
	     n = 0;
	     o = 2;
	     break;
	  case 4:
	     m = 1;
	     n = 2;
	     o = 0;
	     break;
	  case 5:
	     m = 2;
	     n = 1;
	     o = 0;
	     break;
	  default:
	     m = 0;
	     n = 1;
	     o = 2;
	     break;
	  }

	switch (Conf.desks.dragdir)
	  {
	  case 0:
	     w[0] = w[1] = w[2] = h[0] = h[1] = Conf.desks.dragbar_width;
	     if (Conf.desks.dragbar_length == 0)
		h[2] = VRoot.h - (Conf.desks.dragbar_width * 2);
	     else
		h[2] = Conf.desks.dragbar_length;
	     x[0] = x[1] = x[2] = 0;
	     y[m] = 0;
	     y[n] = y[m] + h[m];
	     y[o] = y[n] + h[n];
	     break;
	  case 1:
	     w[0] = w[1] = w[2] = h[0] = h[1] = Conf.desks.dragbar_width;
	     if (Conf.desks.dragbar_length == 0)
		h[2] = VRoot.h - (Conf.desks.dragbar_width * 2);
	     else
		h[2] = Conf.desks.dragbar_length;
	     x[0] = x[1] = x[2] = VRoot.w - Conf.desks.dragbar_width;
	     y[m] = 0;
	     y[n] = y[m] + h[m];
	     y[o] = y[n] + h[n];
	     break;
	  case 2:
	     h[0] = h[1] = h[2] = w[0] = w[1] = Conf.desks.dragbar_width;
	     if (Conf.desks.dragbar_length == 0)
		w[2] = VRoot.w - (Conf.desks.dragbar_width * 2);
	     else
		w[2] = Conf.desks.dragbar_length;
	     y[0] = y[1] = y[2] = 0;
	     x[m] = 0;
	     x[n] = x[m] + w[m];
	     x[o] = x[n] + w[n];
	     break;
	  case 3:
	     h[0] = h[1] = h[2] = w[0] = w[1] = Conf.desks.dragbar_width;
	     if (Conf.desks.dragbar_length == 0)
		w[2] = VRoot.w - (Conf.desks.dragbar_width * 2);
	     else
		w[2] = Conf.desks.dragbar_length;
	     y[0] = y[1] = y[2] = VRoot.h - Conf.desks.dragbar_width;
	     x[m] = 0;
	     x[n] = x[m] + w[m];
	     x[o] = x[n] + w[n];
	     break;
	  default:
	     break;
	  }

	if (Conf.desks.dragbar_width > 0)
	  {
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", 1, ic2, ac2, NULL, NULL,
			      -1, FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[0], 0,
			      y[0], 0, 0, w[0], 0, h[0], 0, i, 0);
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", 1, ic3, ac3, NULL, NULL,
			      -1, FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[1], 0,
			      y[1], 0, 0, w[1], 0, h[1], 0, i, 0);
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", 1, ic, ac, NULL, NULL,
			      -1, FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[2], 0,
			      y[2], 0, 0, w[2], 0, h[2], 0, i, 0);
	  }
	if (i > 0)
	  {
	     if (Conf.desks.dragdir == 0)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", 2, ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_VERT, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y, 0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else if (Conf.desks.dragdir == 1)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", 2, ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_VERT, 1, 99999, 1,
				   99999, 0, 0,
				   desks.desk[i].x + VRoot.w -
				   Conf.desks.dragbar_width, 0, desks.desk[i].y,
				   0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else if (Conf.desks.dragdir == 2)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", 2, ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_HORIZ, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y, 0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", 2, ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_HORIZ, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y + VRoot.h -
				   Conf.desks.dragbar_width, 0, 0, 0, 0, 0, 1,
				   0, 1);
	       }
	     desks.desk[i].tag = b;
	  }
	else
	   desks.desk[i].tag = NULL;
     }
}

void
DesktopSetBg(int desk, Background * bg, int refresh)
{
   if (desk < 0 || desk >= ENLIGHTENMENT_CONF_NUM_DESKTOPS)
      return;

   if (refresh)
      BackgroundPixmapFree(desks.desk[desk].bg);

   if (bg && !strcmp(BackgroundGetName(bg), "NONE"))
      bg = NULL;

   if (desks.desk[desk].bg != bg)
     {
	if (desks.desk[desk].bg)
	   BackgroundDecRefcount(desks.desk[desk].bg);
	if (bg)
	   BackgroundIncRefcount(bg);
     }

   desks.desk[desk].bg = bg;

   if (desks.desk[desk].viewable)
      RefreshDesktop(desk);

   ModulesSignal(ESIGNAL_BACKGROUND_CHANGE, (void *)desk);
}

int
DesktopAt(int x, int y)
{
   int                 i;

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	if ((x >= desks.desk[desks.order[i]].x)
	    && (x < (desks.desk[desks.order[i]].x + VRoot.w))
	    && (y >= desks.desk[desks.order[i]].y)
	    && (y < (desks.desk[desks.order[i]].y + VRoot.h)))
	   return desks.order[i];
     }
   return 0;
}

static void
MoveStickyWindowsToCurrentDesk(void)
{
   EWin               *const *lst, *ewin;
   int                 i, num;

   lst = EwinListStackGet(&num);
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];
	if (!EoIsSticky(ewin))
	   continue;

	EoSetDesk(ewin, DesksGetCurrent());
	ewin->parent = desks.desk[DesksGetCurrent()].win;
	EReparentWindow(disp, EoGetWin(ewin), ewin->parent, VRoot.w, VRoot.h);
	EMoveWindow(disp, EoGetWin(ewin), EoGetX(ewin), EoGetY(ewin));
	HintsSetWindowArea(ewin);
	HintsSetWindowDesktop(ewin);
     }
}

static void
MoveStickyButtonsToCurrentDesk(void)
{
   Button            **lst, *btn;
   int                 i, num;

   lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
   for (i = 0; i < num; i++)
     {
	btn = lst[i];
	if (ButtonIsInternal(btn) || !EoIsSticky((EWin *) btn))
	   continue;

	ButtonMoveToDesktop(btn, desks.current);
     }
}

void
GotoDesktop(int desk)
{
   static int          pdesk = -1;
   int                 x, y;

   if (Conf.desks.desks_wraparound)
     {
	if (desk >= Conf.desks.num)
	   desk = 0;
	else if (desk < 0)
	   desk = Conf.desks.num - 1;
     }
   if (desk < 0 || desk >= Conf.desks.num || desk == pdesk)
      return;

   if (EventDebug(EDBUG_TYPE_DESKS))
      Eprintf("GotoDesktop %d\n", desk);

   ModulesSignal(ESIGNAL_DESK_SWITCH_START, NULL);

   ActionsSuspend();

   FocusNewDeskBegin();

   if (Mode.mode == MODE_NONE)
      Mode.mode = MODE_DESKSWITCH;

   if (desk > 0)
     {
	if (Conf.desks.slidein)
	  {
	     if (!desks.desk[desk].viewable)
	       {
		  switch (Conf.desks.dragdir)
		    {
		    case 0:
		       MoveDesktop(desk, VRoot.w, 0);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, VRoot.w, 0, 0, 0,
				     Conf.desks.slidespeed);
		       break;
		    case 1:
		       MoveDesktop(desk, -VRoot.w, 0);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, -VRoot.w, 0, 0, 0,
				     Conf.desks.slidespeed);
		       break;
		    case 2:
		       MoveDesktop(desk, 0, VRoot.h);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, 0, VRoot.h, 0, 0,
				     Conf.desks.slidespeed);
		       break;
		    case 3:
		       MoveDesktop(desk, 0, -VRoot.h);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, 0, -VRoot.h, 0, 0,
				     Conf.desks.slidespeed);
		       break;
		    default:
		       break;
		    }
	       }
	     else
	       {
		  GetWinXY(desks.desk[desk].win, &x, &y);
		  SlideWindowTo(desks.desk[desk].win, desks.desk[desk].x,
				desks.desk[desk].y, 0, 0,
				Conf.desks.slidespeed);
		  RaiseDesktop(desk);
	       }
	     StackDesktops();
	  }
	else
	  {
	     RaiseDesktop(desk);
	     StackDesktops();
	  }
	MoveDesktop(desk, 0, 0);
     }
   else
     {
	RaiseDesktop(desk);
     }

   ActionsResume();
   FocusNewDesk();

   if (Mode.mode == MODE_DESKSWITCH)
      Mode.mode = MODE_NONE;

   ModulesSignal(ESIGNAL_DESK_SWITCH_DONE, NULL);

   HandleDrawQueue();
   pdesk = DesksGetCurrent();
}

void
MoveDesktop(int desk, int x, int y)
{
   int                 i;
   EWin               *const *lst;
   int                 n, v, dx, dy;

   if (desk <= 0 || desk >= Conf.desks.num)
      return;

   dx = x - desks.desk[desk].x;
   dy = y - desks.desk[desk].y;

   if ((x == 0) && (y == 0))
     {
	n = -1;
	i = 0;
	while ((n < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
	  {
	     if (desks.order[i] == desk)
		n = i;
	     i++;
	  }
	if (n >= 0)
	  {
	     for (i = n + 1; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	       {
		  if (desks.desk[desks.order[i]].viewable)
		     BackgroundTouch(desks.desk[desks.order[i]].bg);
		  desks.desk[desks.order[i]].viewable = 0;
	       }
	  }
     }
   else
     {
	n = -1;
	i = 0;

	while ((n < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
	  {
	     if (desks.order[i] == desk)
		n = i;
	     i++;
	  }

	if (n >= 0)
	  {
	     if (desks.desk[desks.order[n]].viewable)
	       {
		  v = 1;
	       }
	     else
	       {
		  v = 0;
	       }

	     for (i = n + 1; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	       {

		  if ((!desks.desk[desks.order[i]].viewable) && (v))
		    {
		       desks.desk[desks.order[i]].viewable = v;
		       RefreshDesktop(desks.order[i]);
		    }
		  else
		    {
		       if ((!v) && (desks.desk[desks.order[i]].viewable))
			  BackgroundTouch(desks.desk[desks.order[i]].bg);
		       desks.desk[desks.order[i]].viewable = v;
		    }

		  if ((desks.desk[desks.order[i]].x == 0)
		      && (desks.desk[desks.order[i]].y == 0))
		    {
		       v = 0;
		    }
	       }
	  }
     }

   EMoveWindow(disp, desks.desk[desk].win, x, y);

   if (desks.desk[desk].tag)
      ButtonMoveRelative(desks.desk[desk].tag, dx, dy);

   desks.desk[desk].x = x;
   desks.desk[desk].y = y;

   lst = EwinListGetAll(&n);
   for (i = 0; i < n; i++)
      if (EoGetDesk(lst[i]) == desk)
	 ICCCM_Configure(lst[i]);
}

static void
UncoverDesktop(int desk)
{
   if (desk < 0 || desk >= Conf.desks.num)
      return;

   desks.desk[desk].viewable = 1;
   RefreshDesktop(desk);
   if (desk != 0)
      EMapWindow(disp, desks.desk[desk].win);
}

void
RaiseDesktop(int desk)
{
   int                 i;

   if (desk < 0 || desk >= Conf.desks.num)
      return;

   if (EventDebug(EDBUG_TYPE_DESKS))
      Eprintf("RaiseDesktop %d\n", desk);

   FocusNewDeskBegin();
   desks.desk[desk].viewable = 1;
   RefreshDesktop(desk);
   MoveToDeskTop(desk);

   if (desk == 0)
     {
	for (i = ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1; i > 0; i--)
	  {
	     HideDesktop(desks.order[i]);
	  }
     }
   StackDesktops();
   desks.current = desk;
   MoveStickyWindowsToCurrentDesk();
   MoveStickyButtonsToCurrentDesk();
   StackDesktop(DesksGetCurrent());
   FocusNewDesk();
#if 0				/* FIXME - TBD */
   ModulesSignal(ESIGNAL_DESK_SWITCH_DONE, NULL);
#endif
   HandleDrawQueue();
   HintsSetCurrentDesktop();
   EMapWindow(disp, desks.desk[desk].win);
   ecore_x_sync();
}

void
LowerDesktop(int desk)
{
   if ((desk <= 0) || (desk >= Conf.desks.num))
      return;

   FocusNewDeskBegin();
   MoveToDeskBottom(desk);
   UncoverDesktop(desks.order[0]);
   HideDesktop(desk);
   StackDesktops();
   desks.current = desks.order[0];
   MoveStickyWindowsToCurrentDesk();
   MoveStickyButtonsToCurrentDesk();
   StackDesktop(DesksGetCurrent());
   FocusNewDesk();
#if 0				/* FIXME - TBD */
   ModulesSignal(ESIGNAL_DESK_SWITCH_DONE, NULL);
#endif
   HandleDrawQueue();
   HintsSetCurrentDesktop();
   ecore_x_sync();
}

void
HideDesktop(int desk)
{
   if (desk <= 0 || desk >= Conf.desks.num)
      return;

   if (desks.desk[desk].viewable)
      BackgroundTouch(desks.desk[desk].bg);
   desks.desk[desk].viewable = 0;
   EMoveWindow(disp, desks.desk[desk].win, VRoot.w, 0);
}

void
ShowDesktop(int desk)
{
   int                 i;

   if (desk < 0 || desk >= Conf.desks.num)
      return;

   desks.desk[desk].viewable = 1;
   RefreshDesktop(desk);
   MoveToDeskTop(desk);

   if (desk == 0)
     {
	for (i = ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1; i > 0; i--)
	   HideDesktop(desks.order[i]);
     }
   else
     {
	StackDesktops();
	EMapWindow(disp, desks.desk[desk].win);
     }
}

void
StackDesktops(void)
{
   StackDesktop(0);
}

#define _APPEND_TO_WIN_LIST(win) \
  { \
     wl = Erealloc(wl, ++tot * sizeof(Window)); \
     wl[tot - 1] = win; \
  }
void
StackDesktop(int desk)
{
   Window             *wl;

#if 1				/* FIXME - Somehow */
   Window             *wl2;
#endif
   int                 i, num, tot;
   EObj               *const *lst, *eo;

   tot = 0;
   wl = NULL;

   /*
    * Build the window stack, top to bottom
    */

#if 1				/* FIXME - Somehow */
   if (desk == 0)
     {
	wl2 = ProgressbarsListWindows(&num);
	if (wl2)
	  {
	     for (i = 0; i < num; i++)
		_APPEND_TO_WIN_LIST(wl2[i]);
	     Efree(wl2);
	  }
	if (init_win_ext)
	  {
	     _APPEND_TO_WIN_LIST(init_win_ext);
	  }
	if (init_win1)
	  {
	     _APPEND_TO_WIN_LIST(init_win1);
	     _APPEND_TO_WIN_LIST(init_win2);
	  }
     }
#endif

   lst = EobjListStackGetForDesk(&num, desk);

   /* Make the X window list */

   /* Floating objects */
   for (i = 0; i < num; i++)
     {
	eo = lst[i];
	if (!eo->floating)
	   continue;

	_APPEND_TO_WIN_LIST(eo->win);
     }

   if (desk == 0)
     {
	/* The virtual desktop windows */
	for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	  {
	     if (desks.order[i] == 0)
		break;

	     _APPEND_TO_WIN_LIST(desks.desk[desks.order[i]].win);
	  }
     }

   /* Normal objects */
   for (i = 0; i < num; i++)
     {
	eo = lst[i];
#if 0				/* USE_COMPOSITE */
	if (eo->floating || eo->type == EOBJ_TYPE_DESK)
#else
	if (eo->floating)
#endif
	   continue;

	_APPEND_TO_WIN_LIST(eo->win);
#if 0				/* FIXME */
	if (EoGetWin(ewin) == Mode.menus.win_covered)
	   _APPEND_TO_WIN_LIST(Mode.menus.cover_win);
#endif
     }

   if (EventDebug(EDBUG_TYPE_STACKING))
     {
	Eprintf("StackDesktop %d:\n", desk);
	for (i = 0; i < tot; i++)
	   Eprintf(" win=%#10lx parent=%#10lx\n", wl[i], GetWinParent(wl[i]));
     }

   XRestackWindows(disp, wl, tot);
   EdgeWindowsShow();
#if 0				/* FIXME Is this necessary? */
   ProgressbarsRaise();
#endif
   HintsSetClientStacking();

   if (wl)
      Efree(wl);
}

void
GotoDesktopByEwin(EWin * ewin)
{
   if (!EoIsSticky(ewin))
     {
	GotoDesktop(EoGetDesk(ewin));
	SetCurrentArea(ewin->area_x, ewin->area_y);
     }
}

void
DesktopsEventsConfigure(int mode)
{
   int                 i;
   long                event_mask;
   XWindowAttributes   xwa;

   for (i = 0; i < Conf.desks.num; i++)
     {
	if (mode)
	  {
	     event_mask = desks.desk[i].event_mask;
	  }
	else
	  {
	     XGetWindowAttributes(disp, desks.desk[i].win, &xwa);
	     desks.desk[i].event_mask = xwa.your_event_mask | EDESK_EVENT_MASK;
	     event_mask =
		PropertyChangeMask | SubstructureRedirectMask |
		ButtonPressMask | ButtonReleaseMask;
#if USE_COMPOSITE
	     /* Handle ConfigureNotify's while sliding */
	     event_mask |= SubstructureNotifyMask;
#endif
	  }
	XSelectInput(disp, desks.desk[i].win, event_mask);
     }
}

static char         sentpress = 0;

static void
ButtonProxySendEvent(XEvent * ev)
{
   if (Mode.button_proxy_win)
      XSendEvent(disp, Mode.button_proxy_win, False, SubstructureNotifyMask,
		 ev);
}

void
DeskDragStart(int desk)
{
   Mode.deskdrag = desk;
   Mode.mode = MODE_DESKDRAG;
   Mode.start_x = Mode.x;
   Mode.start_y = Mode.y;
   Mode.win_x = DeskGetX(desk);
   Mode.win_y = DeskGetY(desk);
}

void
DeskDragMotion(void)
{
   int                 dx, dy;

   dx = Mode.x - Mode.px;
   dy = Mode.y - Mode.py;

   switch (Conf.desks.dragdir)
     {
     case 0:
	if ((desks.desk[Mode.deskdrag].x + dx) < 0)
	   dx = -desks.desk[Mode.deskdrag].x;
	MoveDesktop(Mode.deskdrag, desks.desk[Mode.deskdrag].x + dx,
		    desks.desk[Mode.deskdrag].y);
	break;
     case 1:
	if ((desks.desk[Mode.deskdrag].x + dx) > 0)
	   MoveDesktop(Mode.deskdrag, 0, desks.desk[Mode.deskdrag].y);
	else
	   MoveDesktop(Mode.deskdrag, desks.desk[Mode.deskdrag].x + dx,
		       desks.desk[Mode.deskdrag].y);
	break;
     case 2:
	if ((desks.desk[Mode.deskdrag].y + dy) < 0)
	   dy = -desks.desk[Mode.deskdrag].y;
	MoveDesktop(Mode.deskdrag, desks.desk[Mode.deskdrag].x,
		    desks.desk[Mode.deskdrag].y + dy);
	break;
     case 3:
	if ((desks.desk[Mode.deskdrag].y + dy) > 0)
	   MoveDesktop(Mode.deskdrag, desks.desk[Mode.deskdrag].x, 0);
	else
	   MoveDesktop(Mode.deskdrag, desks.desk[Mode.deskdrag].x,
		       desks.desk[Mode.deskdrag].y + dy);
	break;
     default:
	break;
     }
}

static void
DesktopEventButtonPress(Desk * d __UNUSED__, XEvent * ev)
{
   ActionClass        *ac;

   /* Don't handle desk bindings while doing stuff */
   if (Mode.mode)
      return;

   GrabPointerRelease();

   ac = FindItem("DESKBINDINGS", 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
   if (ac)
     {
	if (!EventAclass(ev, NULL, ac))
	   ButtonProxySendEvent(ev);
     }
}

static void
DesktopEventButtonRelease(Desk * d __UNUSED__, XEvent * ev)
{
   if (sentpress)
     {
	/* We never get here? */
	sentpress = 0;
	ButtonProxySendEvent(ev);
     }
}

static void
DesktopHandleEvents(XEvent * ev, void *prm)
{
   Desk               *d = (Desk *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	DesktopEventButtonPress(d, ev);
	break;
     case ButtonRelease:
	DesktopEventButtonRelease(d, ev);
	break;
     }
}

/* Settings */

#if 0				/* About to go */

static int
doDragdirSet(EWin * edummy, const char *params)
{
   char                pd;
   Button             *b;
   int                 i;

   pd = Conf.desks.dragdir;
   if (params)
      Conf.desks.dragdir = atoi(params);
   else
     {
	Conf.desks.dragdir++;
	if (Conf.desks.dragdir > 3)
	   Conf.desks.dragdir = 0;
     }
   if (pd != Conf.desks.dragdir)
     {
	GotoDesktop(DesksGetCurrent());
	for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	   MoveDesktop(i, 0, 0);
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
			       LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   ButtonDestroy(b);
	while ((b = RemoveItem("_DESKTOP_DESKRAY_DRAG_CONTROL", 0,
			       LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   ButtonDestroy(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   return 0;
}

static int
doDragbarOrderSet(EWin * edummy, const char *params)
{
   char                pd;
   Button             *b;

   pd = Conf.desks.dragbar_ordering;
   if (params)
      Conf.desks.dragbar_ordering = atoi(params);
   else
     {
	Conf.desks.dragbar_ordering++;
	if (Conf.desks.dragbar_ordering > 5)
	   Conf.desks.dragbar_ordering = 0;
     }
   if (pd != Conf.desks.dragbar_ordering)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
			       LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   ButtonDestroy(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   return 0;
}

static int
doDragbarWidthSet(EWin * edummy, const char *params)
{
   int                 pd;
   Button             *b;

   pd = Conf.desks.dragbar_width;
   if (params)
      Conf.desks.dragbar_width = atoi(params);
   if (pd != Conf.desks.dragbar_width)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
			       LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   ButtonDestroy(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   return 0;
}

static int
doDragbarLengthSet(EWin * edummy, const char *params)
{
   int                 pd;
   Button             *b;

   pd = Conf.desks.dragbar_length;
   if (params)
      Conf.desks.dragbar_length = atoi(params);
   if (pd != Conf.desks.dragbar_length)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
			       LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   ButtonDestroy(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   return 0;
}

static int
doDeskray(EWin * edummy, const char *params)
{
   if (params)
     {
	if (!atoi(params))
	  {
	     HideDesktopTabs();
	     Conf.deskmode = MODE_NONE;
	  }
	else
	  {
	     Conf.deskmode = MODE_DESKRAY;
	     ShowDesktopTabs();
	  }
     }
   else
     {
	if (Conf.deskmode == MODE_DESKRAY)
	  {
	     HideDesktopTabs();
	     Conf.deskmode = MODE_NONE;
	  }
	else
	  {
	     Conf.deskmode = MODE_DESKRAY;
	     ShowDesktopTabs();
	  }
     }
   return 0;
}
#endif

/*
 * Desktops Module
 */

static void
DesktopsSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	DesktopsInit();
	break;

     case ESIGNAL_CONFIGURE:
	SetAreaSize(Conf.desks.areas_nx, Conf.desks.areas_ny);

	DeskSetViewable(0, 1);
	RefreshDesktop(0);

	/* toss down the dragbar and related */
	InitDesktopControls();

	/* then draw all the buttons that belong on the desktop */
	ShowDesktopButtons();

	ShowDesktopControls();
	break;
     }
}

/*
 * Dialodgs
 */
static int          tmp_desktops;
static DItem       *tmp_desk_text;
static Dialog      *tmp_desk_dialog;
static char         tmp_desktop_wraparound;

static void
CB_ConfigureDesktops(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	ChangeNumberOfDesktops(tmp_desktops);
	Conf.desks.desks_wraparound = tmp_desktop_wraparound;
     }
   autosave();
}

static void
CB_DesktopDisplayRedraw(Dialog * d __UNUSED__, int val, void *data)
{
   static char         called = 0;
   DItem              *di;
   static Window       win, wins[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
   int                 i;
   int                 w, h;
   static int          prev_desktops = -1;
   char                s[64];

   if (val == 1)
      called = 0;

   if ((val != 1) && (prev_desktops == tmp_desktops))
      return;
   prev_desktops = tmp_desktops;
   di = (DItem *) data;
   win = DialogItemAreaGetWindow(di);
   DialogItemAreaGetSize(di, &w, &h);
   if (!called)
     {
	ImageClass         *ic;

	ic = ImageclassFind("SETTINGS_DESKTOP_AREA", 0);
	if (ic)
	   ImageclassApply(ic, win, w, h, 0, 0, STATE_NORMAL, 0, ST_UNKNWN);
	for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	   wins[i] = 0;
	called = 1;
     }
   for (i = 0; i < tmp_desktops; i++)
     {
	if (!wins[i])
	  {
	     wins[i] = ECreateWindow(win, 0, 0, 64, 48, 0);
	     XSetWindowBorderWidth(disp, wins[i], 1);
	     if (DeskGetBackground(i))
	       {
		  Pixmap              pmap;

		  pmap = ecore_x_pixmap_new(wins[i], 64, 48, VRoot.depth);
		  ESetWindowBackgroundPixmap(disp, wins[i], pmap);
		  BackgroundApply(DeskGetBackground(i), pmap, 0);
		  ecore_x_pixmap_del(pmap);
	       }
	  }
     }
   for (i = (tmp_desktops - 1); i >= 0; i--)
     {
	int                 num;

	num = tmp_desktops - 1;
	if (num < 1)
	   num = 1;
	XRaiseWindow(disp, wins[i]);
	EMoveWindow(disp, wins[i], (i * (w - 64 - 2)) / num,
		    (i * (h - 48 - 2)) / num);
	EMapWindow(disp, wins[i]);
     }
   for (i = tmp_desktops; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
      EUnmapWindow(disp, wins[i]);
   if (tmp_desktops > 1)
      Esnprintf(s, sizeof(s), _("%i Desktops"), tmp_desktops);
   else
      Esnprintf(s, sizeof(s), _("%i Desktop"), tmp_desktops);
   DialogItemTextSetText(tmp_desk_text, s);
   DialogDrawItems(tmp_desk_dialog, tmp_desk_text, 0, 0, 99999, 99999);
}

void
SettingsDesktops(void)
{
   Dialog             *d;
   DItem              *table, *di, *area, *slider;
   char                s[64];

   if ((d =
	FindItem("CONFIGURE_DESKTOPS", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_DESKTOPS");

   tmp_desktops = Conf.desks.num;
   tmp_desktop_wraparound = Conf.desks.desks_wraparound;

   d = tmp_desk_dialog = DialogCreate("CONFIGURE_DESKTOPS");
   DialogSetTitle(d, _("Multiple Desktop Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	di = DialogAddItem(table, DITEM_IMAGE);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemImageSetFile(di, "pix/desktops.png");

	di = DialogAddItem(table, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemTextSetText(di,
			      _("Enlightenment Multiple Desktop\n"
				"Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetColSpan(di, 2);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemTextSetText(di, _("Number of virtual desktops:\n"));

   di = tmp_desk_text = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   if (tmp_desktops > 1)
      Esnprintf(s, sizeof(s), _("%i Desktops"), tmp_desktops);
   else
      Esnprintf(s, sizeof(s), _("%i Desktop"), tmp_desktops);
   DialogItemTextSetText(di, s);

   di = slider = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 1, 32);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 1);
   DialogItemSetColSpan(di, 2);
   DialogItemSliderSetVal(di, tmp_desktops);
   DialogItemSliderSetValPtr(di, &tmp_desktops);

   di = area = DialogAddItem(table, DITEM_AREA);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemAreaSetSize(di, 128, 96);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemCheckButtonSetText(di, _("Wrap desktops around"));
   DialogItemCheckButtonSetState(di, tmp_desktop_wraparound);
   DialogItemCheckButtonSetPtr(di, &tmp_desktop_wraparound);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureDesktops, 1);
   DialogAddButton(d, _("Apply"), CB_ConfigureDesktops, 0);
   DialogAddButton(d, _("Close"), CB_ConfigureDesktops, 1);
   DialogSetExitFunction(d, CB_ConfigureDesktops, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureDesktops, 0);
   ShowDialog(d);
   DialogItemSetCallback(slider, CB_DesktopDisplayRedraw, 0, (void *)area);
   CB_DesktopDisplayRedraw(d, 1, area);
}

static int          tmp_area_x;
static int          tmp_area_y;
static int          tmp_edge_resist;
static char         tmp_edge_flip;
static DItem       *tmp_area_text;
static Dialog      *tmp_area_dialog;
static char         tmp_area_wraparound;

static void
CB_ConfigureAreas(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	SetNewAreaSize(tmp_area_x, 9 - tmp_area_y);
	Conf.desks.areas_wraparound = tmp_area_wraparound;
	if (tmp_edge_flip)
	  {
	     if (tmp_edge_resist < 1)
		tmp_edge_resist = 1;
	     Conf.edge_flip_resistance = tmp_edge_resist;
	  }
	else
	   Conf.edge_flip_resistance = 0;
	EdgeWindowsShow();
     }
   autosave();
}

static void
CB_AreaDisplayRedraw(Dialog * d __UNUSED__, int val, void *data)
{
   char                s[64];
   static char         called = 0;
   DItem              *di;
   static Window       win, awin;
   int                 w, h;
   static int          prev_ax = 0, prev_ay = 0;

   if (val == 1)
      called = 0;

   if ((val != 1) && ((prev_ax == tmp_area_x) && (prev_ay == tmp_area_y)))
      return;
   prev_ax = tmp_area_x;
   prev_ay = tmp_area_y;
   di = (DItem *) data;
   win = DialogItemAreaGetWindow(di);
   DialogItemAreaGetSize(di, &w, &h);
   if (!called)
     {
	ImageClass         *ic;
	PmapMask            pmm;

	ic = ImageclassFind("SETTINGS_AREA_AREA", 0);
	if (ic)
	   ImageclassApply(ic, win, w, h, 0, 0, STATE_NORMAL, 0, ST_UNKNWN);
	awin = ECreateWindow(win, 0, 0, 18, 14, 0);
	ic = ImageclassFind("SETTINGS_AREADESK_AREA", 0);
	if (ic)
	  {
	     ImageclassApplyCopy(ic, awin, 18, 14, 0, 0, STATE_NORMAL, &pmm, 0,
				 ST_UNKNWN);
	     ESetWindowBackgroundPixmap(disp, awin, pmm.pmap);
	     FreePmapMask(&pmm);
	  }
	XClearWindow(disp, awin);
	called = 1;
     }
   EMoveResizeWindow(disp, awin, ((w / 2) - (9 * tmp_area_x)),
		     ((h / 2) - (7 * (9 - tmp_area_y))), 18 * tmp_area_x,
		     14 * (9 - tmp_area_y));
   EMapWindow(disp, awin);

   if ((tmp_area_x > 1) || ((9 - tmp_area_y) > 1))
      Esnprintf(s, sizeof(s), _("%i x %i\nScreens in size"), tmp_area_x,
		9 - tmp_area_y);
   else
      Esnprintf(s, sizeof(s), _("1\nScreen in size"));
   DialogItemTextSetText(tmp_area_text, s);
   DialogDrawItems(tmp_area_dialog, tmp_area_text, 0, 0, 99999, 99999);
}

void
SettingsArea(void)
{
   Dialog             *d;
   DItem              *table, *di, *area, *slider, *slider2, *table2;
   char                s[64];

   if ((d = FindItem("CONFIGURE_AREA", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_AREA");

   tmp_area_wraparound = Conf.desks.areas_wraparound;
   tmp_edge_resist = Conf.edge_flip_resistance;
   if (tmp_edge_resist == 0)
      tmp_edge_flip = 0;
   else
      tmp_edge_flip = 1;
   GetAreaSize(&tmp_area_x, &tmp_area_y);
   tmp_area_y = 9 - tmp_area_y;

   d = tmp_area_dialog = DialogCreate("CONFIGURE_AREA");
   DialogSetTitle(d, _("Virtual Desktop Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 1, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	table2 = DialogAddItem(table, DITEM_TABLE);
	DialogItemTableSetOptions(table2, 2, 0, 0, 0);

	di = DialogAddItem(table2, DITEM_IMAGE);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemImageSetFile(di, "pix/areas.png");

	di = DialogAddItem(table2, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemTextSetText(di,
			      _("Enlightenment Virtual Desktop\n"
				"Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemTextSetText(di, _("Virtual Desktop size:\n"));

   di = tmp_area_text = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   if ((tmp_area_x > 1) || (tmp_area_y > 1))
      Esnprintf(s, sizeof(s), _("%i x %i\nScreens in size"), tmp_area_x,
		9 - tmp_area_y);
   else
      Esnprintf(s, sizeof(s), _("1\nScreen in size"));
   DialogItemTextSetText(di, s);

   table2 = DialogAddItem(table, DITEM_TABLE);
   DialogItemTableSetOptions(table2, 2, 0, 0, 0);

   di = DialogAddItem(table2, DITEM_NONE);

   di = slider = DialogAddItem(table2, DITEM_SLIDER);
   DialogItemSliderSetMinLength(di, 10);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 1, 8);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 1);
   DialogItemSliderSetVal(di, tmp_area_x);
   DialogItemSliderSetValPtr(di, &tmp_area_x);

   di = slider2 = DialogAddItem(table2, DITEM_SLIDER);
   DialogItemSliderSetMinLength(di, 10);
   DialogItemSliderSetOrientation(di, 0);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 1);
   DialogItemSliderSetBounds(di, 1, 8);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 1);
   DialogItemSliderSetVal(di, tmp_area_y);
   DialogItemSliderSetValPtr(di, &tmp_area_y);

   di = area = DialogAddItem(table2, DITEM_AREA);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemAreaSetSize(di, 160, 120);

   DialogItemSetCallback(slider, CB_AreaDisplayRedraw, 0, (void *)area);
   DialogItemSetCallback(slider2, CB_AreaDisplayRedraw, 0, (void *)area);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemCheckButtonSetText(di, _("Wrap virtual desktops around"));
   DialogItemCheckButtonSetState(di, tmp_area_wraparound);
   DialogItemCheckButtonSetPtr(di, &tmp_area_wraparound);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemCheckButtonSetText(di, _("Enable edge flip"));
   DialogItemCheckButtonSetState(di, tmp_edge_flip);
   DialogItemCheckButtonSetPtr(di, &tmp_edge_flip);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemTextSetText(di, _("Resistance at edge of screen:\n"));

   di = slider = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSliderSetMinLength(di, 10);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 1, 100);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 10);
   DialogItemSliderSetVal(di, tmp_edge_resist);
   DialogItemSliderSetValPtr(di, &tmp_edge_resist);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureAreas, 1);
   DialogAddButton(d, _("Apply"), CB_ConfigureAreas, 0);
   DialogAddButton(d, _("Close"), CB_ConfigureAreas, 1);
   DialogSetExitFunction(d, CB_ConfigureAreas, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureAreas, 0);
   ShowDialog(d);
   CB_AreaDisplayRedraw(d, 1, area);
}

/*
 * IPC functions
 */

static void
DesktopOpGoto(int desk)
{
   int                 pd = DesksGetCurrent();

   GotoDesktop(desk);

   if (DesksGetCurrent() != pd)
      SoundPlay("SOUND_DESKTOP_SHUT");
}

static void
DesktopOpDrag(int desk)
{
   DeskDragStart(desk);
}

static void
DesktopsIpcDesk(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[128];
   int                 len;
   int                 desk;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %100s %n", cmd, prm, &len);
	p += len;
     }

   desk = DesksGetCurrent();

   if (!p || cmd[0] == '?')
     {
	IpcPrintf("Current Desktop: %d\n", desk);
     }
   else if (!strncmp(cmd, "cfg", 3))
     {
	SettingsDesktops();
     }
   else if (!strncmp(cmd, "set", 3))
     {
	sscanf(prm, "%i", &desk);
	ChangeNumberOfDesktops(desk);
     }
   else if (!strncmp(cmd, "goto", 2))
     {
	sscanf(prm, "%i", &desk);
	DesktopOpGoto(desk);
     }
   else if (!strncmp(cmd, "next", 2))
     {
	DesktopOpGoto(DesksGetCurrent() + 1);
     }
   else if (!strncmp(cmd, "prev", 2))
     {
	DesktopOpGoto(DesksGetCurrent() - 1);
     }
   else if (!strncmp(cmd, "this", 2))
     {
	DesktopOpGoto(DesksGetCurrent());
     }
   else if (!strncmp(cmd, "raise", 2))
     {
	sscanf(prm, "%i", &desk);
	SoundPlay("SOUND_DESKTOP_RAISE");
	RaiseDesktop(desk);
     }
   else if (!strncmp(cmd, "lower", 2))
     {
	sscanf(prm, "%i", &desk);
	SoundPlay("SOUND_DESKTOP_LOWER");
	LowerDesktop(desk);
     }
   else if (!strncmp(cmd, "drag", 2))
     {
	if (prm[0])
	   desk = atoi(prm);
	DesktopOpDrag(desk);
     }
}

static void
DesktopsIpcArea(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[128];
   int                 len;
   int                 ax, ay, dx, dy;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %100s %n", cmd, prm, &len);
	p += len;
     }

   DeskGetCurrentArea(&ax, &ay);

   if (!p || cmd[0] == '?')
     {
	IpcPrintf("Current Area: %d %d\n", ax, ay);
     }
   else if (!strncmp(cmd, "cfg", 3))
     {
	SettingsArea();
     }
   else if (!strncmp(cmd, "set", 3))
     {
	sscanf(params, "%*s %i %i", &ax, &ay);
	SetNewAreaSize(ax, ay);
     }
   else if (!strncmp(cmd, "goto", 2))
     {
	sscanf(params, "%*s %i %i", &ax, &ay);
	SetCurrentArea(ax, ay);
     }
   else if (!strncmp(cmd, "move", 2))
     {
	dx = dy = 0;
	sscanf(params, "%*s %i %i", &dx, &dy);
	MoveCurrentAreaBy(dx, dy);
     }
   else if (!strncmp(cmd, "lgoto", 2))
     {
	sscanf(params, "%*s %i", &ax);
	SetCurrentLinearArea(ax);
     }
   else if (!strncmp(cmd, "lmove", 2))
     {
	dx = 0;
	sscanf(params, "%*s %i", &dx);
	MoveCurrentLinearAreaBy(dx);
     }
}

IpcItem             DesktopsIpcArray[] = {
   {
    DesktopsIpcDesk,
    "desk", NULL,
    "Desktop functions",
    "  desk ?               Desktop info\n"
    "  desk cfg             Configure desktops\n"
    "  desk set <nd>        Set number of desktops\n"
    "  desk goto <d>        Goto specified desktop\n"
    "  desk next            Goto next desktop\n"
    "  desk prev            Goto previous desktop\n"
    "  desk this            Goto this desktop\n"
    "  desk lower <d>       Lower desktop\n"
    "  desk raise <d>       Raise desktop\n"}
   ,
   {
    DesktopsIpcArea,
    "area", NULL,
    "Area functions",
    "  area ?               Area info\n"
    "  area cfg             Configure areas\n"
    "  area set <nx> <ny>   Set area size\n"
    "  area goto <ax> <ay>  Goto specified area\n"
    "  area move <dx> <dy>  Move relative to current area\n"
    "  area lgoto <al>      Goto specified linear area\n"
    "  area lmove <dl>      Move relative to current linear area\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(DesktopsIpcArray)/sizeof(IpcItem))

static const CfgItem DesktopsCfgItems[] = {
   CFG_ITEM_INT(Conf.desks, num, 2),
   CFG_ITEM_INT(Conf.desks, dragdir, 2),
   CFG_ITEM_INT(Conf.desks, dragbar_width, 16),
   CFG_ITEM_INT(Conf.desks, dragbar_ordering, 1),
   CFG_ITEM_INT(Conf.desks, dragbar_length, 0),
   CFG_ITEM_BOOL(Conf.desks, desks_wraparound, 0),
   CFG_ITEM_BOOL(Conf.desks, slidein, 1),
   CFG_ITEM_INT(Conf.desks, slidespeed, 6000),

   CFG_ITEM_INT(Conf.desks, areas_nx, 2),
   CFG_ITEM_INT(Conf.desks, areas_ny, 1),
   CFG_ITEM_BOOL(Conf.desks, areas_wraparound, 0),
};
#define N_CFG_ITEMS (sizeof(DesktopsCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
EModule             ModDesktops = {
   "desktops", "desk",
   DesktopsSighan,
   {N_IPC_FUNCS, DesktopsIpcArray},
   {N_CFG_ITEMS, DesktopsCfgItems}
};
