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
#include <sys/time.h>

#define EWIN_TOP_EVENT_MASK \
  (ButtonPressMask | ButtonReleaseMask | \
   EnterWindowMask | LeaveWindowMask | PointerMotionMask /* | \
   StructureNotifyMask */)
#define EWIN_CONTAINER_EVENT_MASK \
  (/* ButtonPressMask | ButtonReleaseMask | */ \
   /* StructureNotifyMask | ResizeRedirectMask | */ \
   /* SubstructureNotifyMask | */ SubstructureRedirectMask)
#define EWIN_BORDER_PART_EVENT_MASK \
  (KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
   EnterWindowMask | LeaveWindowMask | PointerMotionMask | ExposureMask)
#define EWIN_BORDER_TITLE_EVENT_MASK \
  (EWIN_BORDER_PART_EVENT_MASK | ExposureMask)

#define EWIN_CLIENT_EVENT_MASK \
  (EnterWindowMask | LeaveWindowMask | FocusChangeMask | \
   StructureNotifyMask | ResizeRedirectMask | \
   PropertyChangeMask | ColormapChangeMask)

static void         EwinBorderSetTo(EWin * ewin, Border * b);
static EWin        *EwinCreate(Window win);
static EWin        *Adopt(Window win);
static EWin        *AdoptInternal(Window win, Border * border, int type);
static void         EwinEventsConfigure(EWin * ewin, int mode);
static void         EwinBorderDraw(EWin * ewin, int do_shape, int queue_off);

void
DetermineEwinFloat(EWin * ewin, int dx, int dy)
{
   char                dofloat = 0;

   EDBUG(5, "DetermineEwinFloat");

   if ((ewin->desktop != 0) && (ewin->floating < 2) &&
       ((desks.desk[ewin->desktop].x != 0) ||
	(desks.desk[ewin->desktop].y != 0) || (desks.current != ewin->desktop)))
     {
	switch (Conf.desks.dragdir)
	  {
	  case 0:
	     if (((ewin->x + dx < 0) ||
		  ((ewin->x + dx + ewin->w <= VRoot.w) &&
		   ((DesktopAt
		     (desks.desk[ewin->desktop].x + ewin->x + dx + ewin->w - 1,
		      desks.desk[ewin->desktop].y) != ewin->desktop)))))
		dofloat = 1;
	     break;
	  case 1:
	     if (((ewin->x + dx + ewin->w > VRoot.w) ||
		  ((ewin->x + dx >= 0) &&
		   ((DesktopAt
		     (desks.desk[ewin->desktop].x + ewin->x + dx,
		      desks.desk[ewin->desktop].y) != ewin->desktop)))))
		dofloat = 1;
	     break;
	  case 2:
	     if (((ewin->y + dy < 0) ||
		  ((ewin->y + dy + ewin->h <= VRoot.h) &&
		   ((DesktopAt
		     (desks.desk[ewin->desktop].x,
		      desks.desk[ewin->desktop].y + ewin->y + dy + ewin->h -
		      1) != ewin->desktop)))))
		dofloat = 1;
	     break;
	  case 3:
	     if (((ewin->y + dy + ewin->h > VRoot.h) ||
		  ((ewin->y + dy >= 0) &&
		   ((DesktopAt
		     (desks.desk[ewin->desktop].x,
		      desks.desk[ewin->desktop].y + ewin->y + dy) !=
		     ewin->desktop)))))
		dofloat = 1;
	     break;
	  }

	if (dofloat)
	   FloatEwinAt(ewin, ewin->x + desks.desk[ewin->desktop].x,
		       ewin->y + desks.desk[ewin->desktop].y);
     }
   EDBUG_RETURN_;
}

EWin               *
GetEwinByCurrentPointer(void)
{
   Window              rt, ch;
   int                 dum, x, y;
   unsigned int        mr;

   EDBUG(5, "GetEwinByCurrentPointer");

   XQueryPointer(disp, desks.desk[desks.current].win, &rt, &ch, &x, &y, &dum,
		 &dum, &mr);

   EDBUG_RETURN(FindEwinByBase(ch));
}

EWin               *
GetEwinPointerInClient(void)
{
   Window              rt, ch;
   int                 dum, px, py, d;
   EWin               *const *lst, *ewin;
   int                 i, num;

   EDBUG(5, "GetEwinPointerInClient");

   d = DesktopAt(Mode.x, Mode.y);
   XQueryPointer(disp, desks.desk[d].win, &rt, &ch, &dum, &dum, &px, &py,
		 (unsigned int *)&dum);
   px -= desks.desk[d].x;
   py -= desks.desk[d].y;

   lst = EwinListGetForDesktop(d, &num);
   for (i = 0; i < num; i++)
     {
	int                 x, y, w, h;

	ewin = lst[i];
	x = ewin->x;
	y = ewin->y;
	w = ewin->w;
	h = ewin->h;
	if ((px >= x) && (py >= y) && (px < (x + w)) && (py < (y + h)) &&
	    (ewin->visible) && (ewin->state == EWIN_STATE_MAPPED))
	   EDBUG_RETURN(ewin);
     }

   EDBUG_RETURN(NULL);
}

EWin               *
GetFocusEwin(void)
{
   EDBUG(4, "GetFocusEwin");
   EDBUG_RETURN(Mode.focuswin);
}

EWin               *
GetContextEwin(void)
{
   EWin               *ewin;

   EDBUG(4, "GetContextEwin");

#if 0
   ewin = Mode.mouse_over_ewin;
   if (ewin && !ewin->menu)
      EDBUG_RETURN(ewin);
#endif

   ewin = Mode.focuswin;
   if (ewin && !ewin->menu)
      EDBUG_RETURN(ewin);

   return NULL;
}

void
SlideEwinTo(EWin * ewin, int fx, int fy, int tx, int ty, int speed)
{
   int                 k, spd, x, y, min, tmpx, tmpy, tmpw, tmph;
   struct timeval      timev1, timev2;
   int                 dsec, dusec;
   double              tm;
   char                firstlast;

   EDBUG(3, "SlideEwinTo");
   spd = 16;
   min = 2;
   firstlast = 0;
   Mode.doingslide = 1;
   SoundPlay("SOUND_WINDOW_SLIDE");

   if (Conf.slidemode > 0)
      GrabX();

   for (k = 0; k <= 1024; k += spd)
     {
	gettimeofday(&timev1, NULL);
	x = ((fx * (1024 - k)) + (tx * k)) >> 10;
	y = ((fy * (1024 - k)) + (ty * k)) >> 10;
	tmpx = x;
	tmpy = y;
	tmpw = ewin->client.w;
	tmph = ewin->client.h;
	if (Conf.slidemode == 0)
	   EMoveWindow(disp, ewin->win, tmpx, tmpy);
	else
	   DrawEwinShape(ewin, Conf.slidemode, tmpx, tmpy, tmpw, tmph,
			 firstlast);
	if (firstlast == 0)
	   firstlast = 1;
	XSync(disp, False);
	gettimeofday(&timev2, NULL);
	dsec = timev2.tv_sec - timev1.tv_sec;
	dusec = timev2.tv_usec - timev1.tv_usec;
	if (dusec < 0)
	  {
	     dsec--;
	     dusec += 1000000;
	  }
	tm = (double)dsec + (((double)dusec) / 1000000);
	spd = (int)((double)speed * tm);
	if (spd < min)
	   spd = min;
     }
   DrawEwinShape(ewin, Conf.slidemode, x, y, ewin->client.w, ewin->client.h, 2);
   MoveEwin(ewin, tx, ty);
   Mode.doingslide = 0;
   if (Conf.slidemode > 0)
      UngrabX();
   SoundPlay("SOUND_WINDOW_SLIDE_END");
   EDBUG_RETURN_;
}

void
SlideEwinsTo(EWin ** ewin, int *fx, int *fy, int *tx, int *ty, int num_wins,
	     int speed)
{
   int                 k, spd, *x = NULL, *y =
      NULL, min, tmpx, tmpy, tmpw, tmph, i;
   struct timeval      timev1, timev2;
   int                 dsec, dusec;
   double              tm;
   char                firstlast;

   EDBUG(3, "SlideEwinsTo");

   if (num_wins)
     {
	x = Emalloc(sizeof(int) * num_wins);
	y = Emalloc(sizeof(int) * num_wins);
     }
   spd = 16;
   min = 2;
   firstlast = 0;
   Mode.doingslide = 1;
   SoundPlay("SOUND_WINDOW_SLIDE");
   if (Conf.slidemode > 0)
      GrabX();
   for (k = 0; k <= 1024; k += spd)
     {
	for (i = 0; i < num_wins; i++)
	  {
	     if (ewin[i])
	       {
		  gettimeofday(&timev1, NULL);
		  x[i] = ((fx[i] * (1024 - k)) + (tx[i] * k)) >> 10;
		  y[i] = ((fy[i] * (1024 - k)) + (ty[i] * k)) >> 10;
		  tmpx = x[i];
		  tmpy = y[i];
		  tmpw = ewin[i]->client.w;
		  tmph = ewin[i]->client.h;
		  if (ewin[i]->menu)
		     EMoveWindow(disp, ewin[i]->win, tmpx, tmpy);
		  else
		     DrawEwinShape(ewin[i], 0, tmpx, tmpy, tmpw, tmph,
				   firstlast);
		  if (firstlast == 0)
		     firstlast = 1;
		  XSync(disp, False);
		  gettimeofday(&timev2, NULL);
		  dsec = timev2.tv_sec - timev1.tv_sec;
		  dusec = timev2.tv_usec - timev1.tv_usec;
		  if (dusec < 0)
		    {
		       dsec--;
		       dusec += 1000000;
		    }
		  tm = (double)dsec + (((double)dusec) / 1000000);
		  spd = (int)((double)speed * tm);
		  if (spd < min)
		     spd = min;
	       }
	  }
     }

   for (i = 0; i < num_wins; i++)
     {
	if (ewin[i])
	  {
	     DrawEwinShape(ewin[i], 0, x[i], y[i], ewin[i]->client.w,
			   ewin[i]->client.h, 2);
	     MoveEwin(ewin[i], tx[i], ty[i]);
	  }
     }

   Mode.doingslide = 0;
   if (Conf.slidemode > 0)
      UngrabX();
   SoundPlay("SOUND_WINDOW_SLIDE_END");
   if (x)
      Efree(x);
   if (y)
      Efree(y);
   EDBUG_RETURN_;
}

void
EwinDetermineArea(EWin * ewin)
{
   int                 ax, ay;

   EDBUG(4, "EwinDetermineArea");

   ax = (ewin->x + (ewin->w / 2) +
	 (desks.desk[ewin->desktop].current_area_x * VRoot.w)) / VRoot.w;
   ay = (ewin->y + (ewin->h / 2) +
	 (desks.desk[ewin->desktop].current_area_y * VRoot.h)) / VRoot.h;

   AreaFix(&ax, &ay);

   if (ax != ewin->area_x || ay != ewin->area_y)
     {
	ewin->area_x = ax;
	ewin->area_y = ay;
	HintsSetWindowArea(ewin);
     }

   EDBUG_RETURN_;
}

void
AddToFamily(Window win)
{
   EWin               *ewin, *ewin2;
   EWin              **lst;
   int                 i, k, num, speed, fx, fy, x, y;
   char                doslide, manplace;
   char                cangrab = 0;

   EDBUG(3, "AddToFamily");

   /* find the client window if it's already managed */
   ewin = FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_EWIN);

   /* it's already managed */
   if (ewin)
     {
	/* if its iconified - de-iconify */
	if (ewin->iconified)
	  {
	     RemoveMiniIcon(ewin);
	     ewin->desktop = desks.current;
	     MoveEwinToArea(ewin, desks.desk[desks.current].current_area_x,
			    desks.desk[desks.current].current_area_y);
	     RaiseEwin(ewin);
	     ConformEwinToDesktop(ewin);
	     ShowEwin(ewin);
	     ICCCM_DeIconify(ewin);
	     ewin->iconified = 0;
	  }
	EDBUG_RETURN_;
     }

   /* grab that server */
   GrabX();
   speed = Conf.slidespeedmap;
   doslide = Conf.mapslide;
   manplace = 0;
   /* adopt the new baby */
   ewin = Adopt(win);

   /* if is an afterstep/windowmaker dock app - dock it */
   if (Conf.dockapp_support && ewin->docked)
     {
	DockIt(ewin);
	EDBUG_RETURN_;
     }

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
		  if (ewin->layer < lst[i]->layer)
		     ewin->layer = lst[i]->layer;
	       }
	     if (lst)
	       {
		  ewin2 = lst[0];
		  ewin->sticky = lst[0]->sticky;
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
	     ewin->desktop = ewin2->desktop;
	     if ((Conf.focus.switchfortransientmap) && (!ewin->iconified))
		GotoDesktopByEwin(ewin2);
	  }
     }

   ResizeEwin(ewin, ewin->client.w, ewin->client.h);

   /* if it hasn't been planted on a desktop - assign it the current desktop */
   if (ewin->desktop < 0)
     {
	ewin->desktop = desks.current;
     }
   else
     {
	/* assign it the desktop it asked for (modulo the number of desks) */
	ewin->desktop = DESKTOPS_WRAP_NUM(ewin->desktop);
     }

   if ((!ewin->client.transient) && (Conf.manual_placement)
       && (!ewin->client.already_placed) && (!Mode.wm.startup) && (!Mode.place))
     {
	cangrab = GrabThePointer(VRoot.win);
	if ((cangrab == GrabNotViewable) || (cangrab == AlreadyGrabbed)
	    || (cangrab == GrabFrozen))
	  {
	     XUngrabPointer(disp, CurrentTime);
	     cangrab = 0;
	  }
	else
	  {
	     manplace = 1;
	     cangrab = 1;
	  }
     }

   /* if it hasn't been placed yet.... find a spot for it */
   x = ewin->x;
   y = ewin->y;
   if ((!ewin->client.already_placed) && (!manplace))
     {

	/* Place the window below the mouse pointer */
	if (Conf.manual_placement_mouse_pointer)
	  {
	     int                 rx, ry, wx, wy;
	     unsigned int        mask;
	     Window              junk, root_return;
	     int                 newWinX = 0, newWinY = 0;

	     /* if the loser has manual placement on and the app asks to be on */
	     /*  a desktop, then send E to that desktop so the user can place */
	     /* the window there */
	     if (ewin->desktop >= 0)
		GotoDesktop(ewin->desktop);

	     GrabThePointer(VRoot.win);
	     XQueryPointer(disp, VRoot.win, &root_return, &junk, &rx, &ry, &wx,
			   &wy, &mask);
	     XUngrabPointer(disp, CurrentTime);
	     Mode.x = rx;
	     Mode.y = ry;
	     ewin->client.already_placed = 1;

	     /* try to center the window on the mouse pointer */
	     newWinX = rx;
	     newWinY = ry;
	     if (ewin->w)
		newWinX -= ewin->w / 2;
	     if (ewin->h)
		newWinY -= ewin->h / 2;

	     /* keep it all on this screen if possible */
	     newWinX = MIN(newWinX, VRoot.w - ewin->w);
	     newWinY = MIN(newWinY, VRoot.h - ewin->h);
	     newWinX = MAX(newWinX, 0);
	     newWinY = MAX(newWinY, 0);

	     /* this works for me... */
	     x = ewin->x = newWinX;
	     y = ewin->y = newWinY;
	  }
	else
	  {
	     ewin->client.already_placed = 1;
	     ArrangeEwinXY(ewin, &x, &y);
	  }			/* (Conf.manual_placement_mouse_pointer) */
     }				/* ((!ewin->client.already_placed) && (!manplace)) */

   /* if the window asked to be iconified at the start */
   if (ewin->client.start_iconified)
     {
	EwinBorderDraw(ewin, 1, 0);
	MoveEwinToDesktopAt(ewin, ewin->desktop, x, y);
	UngrabX();
	IconifyEwin(ewin);
	ewin->state = EWIN_STATE_ICONIC;
	EDBUG_RETURN_;
     }

   /* if we should slide it in and are not currently in the middle of a slide */
   if ((manplace) && (!ewin->client.already_placed))
     {
	int                 rx, ry, wx, wy;
	unsigned int        mask;
	Window              junk, root_return;

	/* if the loser has manual placement on and the app asks to be on */
	/*  a desktop, then send E to that desktop so the user can place */
	/* the window there */
	if (ewin->desktop >= 0)
	   GotoDesktop(ewin->desktop);
	XQueryPointer(disp, VRoot.win, &root_return, &junk, &rx, &ry, &wx, &wy,
		      &mask);
	Mode.x = rx;
	Mode.y = ry;
	ewin->client.already_placed = 1;
	x = Mode.x + 1;
	y = Mode.y + 1;
	ICCCM_Configure(ewin);
	EwinBorderDraw(ewin, 1, 0);
	MoveEwinToDesktop(ewin, ewin->desktop);
	RaiseEwin(ewin);
	MoveEwin(ewin, x, y);
	RaiseEwin(ewin);
	ShowEwin(ewin);
	GrabThePointer(VRoot.win);
	Mode.have_place_grab = 1;
	Mode.place = 1;
	ICCCM_Configure(ewin);
	UngrabX();
	ewin->floating = 1;	/* Causes reparenting to root */
	ActionsCall(ACTION_MOVE, ewin, NULL);
	EDBUG_RETURN_;
     }
   else if ((doslide) && (!Mode.doingslide))
     {
	MoveEwin(ewin, VRoot.w, VRoot.h);
	k = rand() % 4;
	if (k == 0)
	  {
	     fx = (rand() % (VRoot.w)) - ewin->w;
	     fy = -ewin->h;
	  }
	else if (k == 1)
	  {
	     fx = (rand() % (VRoot.w));
	     fy = VRoot.h;
	  }
	else if (k == 2)
	  {
	     fx = -ewin->w;
	     fy = (rand() % (VRoot.h));
	  }
	else
	  {
	     fx = VRoot.w;
	     fy = (rand() % (VRoot.h)) - ewin->h;
	  }
	EwinBorderDraw(ewin, 1, 0);
	MoveEwinToDesktop(ewin, ewin->desktop);
	RaiseEwin(ewin);
	MoveEwin(ewin, fx, fy);
	ShowEwin(ewin);
	SlideEwinTo(ewin, fx, fy, x, y, speed);
	MoveEwinToDesktopAt(ewin, ewin->desktop, x, y);
     }
   else
     {
	EwinBorderDraw(ewin, 1, 0);
	MoveEwinToDesktopAt(ewin, ewin->desktop, x, y);
	RaiseEwin(ewin);
	ShowEwin(ewin);
     }

   /* send synthetic configure notifies and configure the window */
   ICCCM_Configure(ewin);

   EwinDetermineArea(ewin);
   UngrabX();

   EDBUG_RETURN_;
}

EWin               *
AddInternalToFamily(Window win, const char *bname, int type, void *ptr,
		    void (*init) (EWin * ewin, void *ptr))
{
   EWin               *ewin;
   Border             *b;

   EDBUG(3, "AddInternalToFamily");

   b = NULL;
   if (bname)
     {
	b = FindItem(bname, 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
	if (!b)
	   b = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
     }
   ewin = AdoptInternal(win, b, type);

   if (ewin->desktop < 0)
      ewin->desktop = desks.current;
   else
      ewin->desktop = DESKTOPS_WRAP_NUM(ewin->desktop);
   ConformEwinToDesktop(ewin);

   if (init)
      init(ewin, ptr);		/* Type specific initialisation */

   ICCCM_Configure(ewin);
   EwinBorderDraw(ewin, 1, 1);

   EwinDetermineArea(ewin);
   UngrabX();

   EDBUG_RETURN(ewin);
}

void
SyncBorderToEwin(EWin * ewin)
{
   Border             *b;

   EDBUG(4, "SyncBorderToEwin");
   b = ewin->border;
   ICCCM_GetShapeInfo(ewin);
   EwinSetBorder(ewin, b, 1);
   EDBUG_RETURN_;
}

void
EwinBorderUpdateState(EWin * ewin)
{
   EwinBorderDraw(ewin, 0, 0);
}

static void
BorderWinpartRealise(EWin * ewin, int i)
{
   EWinBit            *ewb = &ewin->bits[i];

   EDBUG(4, "BorderWinpartRealise");

   if ((ewb->cx != ewb->x) || (ewb->cy != ewb->y) ||
       (ewb->cw != ewb->w) || (ewb->ch != ewb->h))
     {
	if ((ewb->w < 0) || (ewb->h < 0))
	  {
	     EUnmapWindow(disp, ewb->win);
	  }
	else
	  {
	     EMapWindow(disp, ewb->win);
	     EMoveResizeWindow(disp, ewb->win, ewb->x, ewb->y, ewb->w, ewb->h);
	  }
     }
   EDBUG_RETURN_;
}

static void
BorderWinpartITclassApply(EWin * ewin, int i, int force)
{
   EWinBit            *ewb = &ewin->bits[i];
   ImageState         *is;
   const char         *title;

   if (ewb->win == None)
      return;

   is = IclassGetImageState(ewin->border->part[i].iclass, ewin->bits[i].state,
			    ewin->active, ewin->sticky);
   if (!force && ewin->bits[i].is == is)
      return;
   ewin->bits[i].is = is;

   IclassApply(ewin->border->part[i].iclass, ewb->win,
	       ewb->w, ewb->h, ewin->active,
	       ewin->sticky, ewb->state, ewb->expose, ST_BORDER);

   switch (ewin->border->part[i].flags)
     {
     case FLAG_TITLE:
	title = EwinGetTitle(ewin);
	if (title)
	   TclassApply(ewin->border->part[i].iclass, ewb->win,
		       ewb->w, ewb->h, ewin->active,
		       ewin->sticky, ewb->state, ewb->expose,
		       ewin->border->part[i].tclass, title);
	break;
     case FLAG_MINIICON:
	break;
     default:
	break;
     }
}

static int
BorderWinpartDraw(EWin * ewin, int i)
{
   EWinBit            *ewb = &ewin->bits[i];
   int                 move = 0, resize = 0, ret = 0;

   EDBUG(4, "BorderWinpartDraw");

   if ((ewb->x != ewb->cx) || (ewb->y != ewb->cy))
     {
	move = 1;
	ewb->cx = ewb->x;
	ewb->cy = ewb->y;
	ret = 1;
     }

   if ((ewb->w != ewb->cw) || (ewb->h != ewb->ch))
     {
	resize = 1;
	ewb->cw = ewb->w;
	ewb->ch = ewb->h;
     }

   if ((resize) || (ewb->expose))
     {
	BorderWinpartITclassApply(ewin, i, 1);
	ewb->expose = 0;
	ret = 1;
     }

   EDBUG_RETURN(ret);
}

void
BorderWinpartChange(EWin * ewin, int i, int force)
{
   EDBUG(3, "BorderWinpartChange");

   BorderWinpartITclassApply(ewin, i, force);

   if (!ewin->shapedone || ewin->border->changes_shape)
      PropagateShapes(ewin->win);
   ewin->shapedone = 1;

   EDBUG_RETURN_;
}

static void
EwinBorderDraw(EWin * ewin, int do_shape, int queue_off)
{
   int                 i, pq;

   EDBUG(4, "EwinBorderDraw");

   if (!ewin)
      EDBUG_RETURN_;

   pq = Mode.queue_up;
   if (queue_off)
      Mode.queue_up = 0;

   for (i = 0; i < ewin->border->num_winparts; i++)
      BorderWinpartITclassApply(ewin, i, do_shape);

   if (do_shape || !ewin->shapedone || ewin->border->changes_shape)
      PropagateShapes(ewin->win);
   ewin->shapedone = 1;

   if (queue_off)
      Mode.queue_up = pq;

   EDBUG_RETURN_;
}

void
EwinBorderUpdateInfo(EWin * ewin)
{
   int                 i;

   for (i = 0; i < ewin->border->num_winparts; i++)
     {
	if (ewin->border->part[i].flags == FLAG_TITLE)
	   BorderWinpartITclassApply(ewin, i, 1);
     }
}

static void
BorderWinpartCalc(EWin * ewin, int i)
{
   int                 x, y, w, h, ox, oy, max, min;
   int                 topleft, bottomright;

   EDBUG(4, "BorderWinpartCalc");
   topleft = ewin->border->part[i].geom.topleft.originbox;
   bottomright = ewin->border->part[i].geom.bottomright.originbox;
   if (topleft >= 0)
      BorderWinpartCalc(ewin, topleft);
   if (bottomright >= 0)
      BorderWinpartCalc(ewin, bottomright);
   x = y = 0;
   if (topleft == -1)
     {
	x = ((ewin->border->part[i].geom.topleft.x.percent * ewin->w) >> 10) +
	   ewin->border->part[i].geom.topleft.x.absolute;
	y = ((ewin->border->part[i].geom.topleft.y.percent * ewin->h) >> 10) +
	   ewin->border->part[i].geom.topleft.y.absolute;
     }
   else if (topleft >= 0)
     {
	x = ((ewin->border->part[i].geom.topleft.x.percent *
	      ewin->bits[topleft].w) >> 10) +
	   ewin->border->part[i].geom.topleft.x.absolute +
	   ewin->bits[topleft].x;
	y = ((ewin->border->part[i].geom.topleft.y.percent *
	      ewin->bits[topleft].h) >> 10) +
	   ewin->border->part[i].geom.topleft.y.absolute +
	   ewin->bits[topleft].y;
     }
   ox = oy = 0;
   if (bottomright == -1)
     {
	ox = ((ewin->border->
	       part[i].geom.bottomright.x.percent * ewin->w) >> 10) +
	   ewin->border->part[i].geom.bottomright.x.absolute;
	oy = ((ewin->border->
	       part[i].geom.bottomright.y.percent * ewin->h) >> 10) +
	   ewin->border->part[i].geom.bottomright.y.absolute;
     }
   else if (bottomright >= 0)
     {
	ox = ((ewin->border->part[i].geom.bottomright.x.percent *
	       ewin->bits[bottomright].w) >> 10) +
	   ewin->border->part[i].geom.bottomright.x.absolute +
	   ewin->bits[bottomright].x;
	oy = ((ewin->border->part[i].geom.bottomright.y.percent *
	       ewin->bits[bottomright].h) >> 10) +
	   ewin->border->part[i].geom.bottomright.y.absolute +
	   ewin->bits[bottomright].y;
     }
   /*
    * calculate height before width, because we may need it in order to
    * determine the font size. But we might do it the other way around for
    * side borders :-)
    */

   h = (oy - y) + 1;
   max = ewin->border->part[i].geom.height.max;
   min = ewin->border->part[i].geom.height.min;

   /*
    * If the title bar max size is set to zero, then set the title bar size to
    * just a little bit more than the size of the title text.
    */

   if (max == 0 && ewin->border->part[i].flags == FLAG_TITLE)
     {
	int                 dummywidth, wmax, wmin;
	ImageClass         *iclass;
	TextClass          *tclass;

	/*
	 * calculate width before height, because we need it in order to
	 * determine the font size.
	 */

	w = (ox - x) + 1;
	wmax = ewin->border->part[i].geom.width.max;
	wmin = ewin->border->part[i].geom.width.min;
	if (w > wmax)
	  {
	     w = wmax;
	     x = ((x + ox) - w) >> 1;
	  }
	else if (w < wmin)
	  {
	     w = wmin;
	  }
	iclass = ewin->border->part[i].iclass;
	tclass = ewin->border->part[i].tclass;
	TextSize(tclass, ewin->active, ewin->sticky, ewin->bits[i].state,
		 EwinGetTitle(ewin), &max, &dummywidth,
		 w - (iclass->padding.top + iclass->padding.bottom));
	max += iclass->padding.left + iclass->padding.right;
	if (h > max)
	  {
	     y = y + (((h - max) * tclass->justification) >> 10);
	     h = max;
	  }
	if (h < min)
	  {
	     h = min;
	  }
     }
   else
     {
	if (h > max)
	  {
	     h = max;
	     y = ((y + oy) - h) >> 1;
	  }
	else if (h < min)
	  {
	     h = min;
	  }
	/*
	 * and now the width.
	 */

	w = (ox - x) + 1;
	max = ewin->border->part[i].geom.width.max;
	min = ewin->border->part[i].geom.width.min;

	/*
	 * If the title bar max size is set to zero, then set the title bar
	 * size to just a little bit more than the size of the title text.
	 */

	if (max == 0 && ewin->border->part[i].flags == FLAG_TITLE)
	  {
	     int                 dummyheight;

	     ImageClass         *iclass;
	     TextClass          *tclass;

	     iclass = ewin->border->part[i].iclass;
	     tclass = ewin->border->part[i].tclass;
	     TextSize(tclass, ewin->active, ewin->sticky, ewin->bits[i].state,
		      EwinGetTitle(ewin), &max, &dummyheight,
		      h - (iclass->padding.top + iclass->padding.bottom));
	     max += iclass->padding.left + iclass->padding.right;

	     if (w > max)
	       {
		  x = x + (((w - max) * tclass->justification) >> 10);
		  w = max;
	       }
	  }
	if (w > max)
	  {
	     w = max;
	     x = ((x + ox) - w) >> 1;
	  }
	else if (w < min)
	  {
	     w = min;
	  }
     }
   if ((ewin->shaded) && (!ewin->border->part[i].keep_for_shade))
     {
	ewin->bits[i].x = -100;
	ewin->bits[i].y = -100;
	ewin->bits[i].w = -1;
	ewin->bits[i].h = -1;
     }
   else
     {
	ewin->bits[i].x = x;
	ewin->bits[i].y = y;
	ewin->bits[i].w = w;
	ewin->bits[i].h = h;
     }
   EDBUG_RETURN_;
}

static void
CalcEwinSizes(EWin * ewin)
{
   int                 i;
   char                reshape;

   EDBUG(4, "CalcEwinSizes");

   if (!ewin)
      EDBUG_RETURN_;
   if (!ewin->border)
      EDBUG_RETURN_;

   for (i = 0; i < ewin->border->num_winparts; i++)
      ewin->bits[i].w = -2;
   for (i = 0; i < ewin->border->num_winparts; i++)
      if (ewin->bits[i].w == -2)
	 BorderWinpartCalc(ewin, i);
   for (i = 0; i < ewin->border->num_winparts; i++)
      BorderWinpartRealise(ewin, i);

   reshape = 0;
   for (i = 0; i < ewin->border->num_winparts; i++)
     {
	reshape |= BorderWinpartDraw(ewin, i);
	ewin->bits[i].no_expose = 1;
     }

   if ((reshape) || (Mode.have_place_grab))
     {
	if (Mode.have_place_grab)
	  {
	     char                pq;

	     pq = Mode.queue_up;
	     Mode.queue_up = 0;
	     PropagateShapes(ewin->win);
	     Mode.queue_up = pq;
	  }
	else
	   PropagateShapes(ewin->win);
	ewin->shapedone = 1;
     }

   EDBUG_RETURN_;
}

void
HonorIclass(char *s, int id)
{
   AwaitIclass        *a;
   EWin               *ewin;

   EDBUG(4, "HonorIclass");

   a = RemoveItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_AWAIT_ICLASS);
   if (!a)
      EDBUG_RETURN_;

   ewin = FindItem(NULL, a->client_win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
   if (ewin)
     {
	if (a->ewin_bit < ewin->border->num_winparts)
	  {
	     if ((ewin->border->part[a->ewin_bit].iclass->external)
		 && (!ewin->bits[a->ewin_bit].win) && (id))
	       {
		  ewin->bits[a->ewin_bit].win = id;
		  BorderWinpartRealise(ewin, a->ewin_bit);
		  EMapWindow(disp, id);
		  ewin->shapedone = 0;
		  if (!ewin->shapedone)
		    {
		       PropagateShapes(ewin->win);
		    }
		  else
		    {
		       if (ewin->border->changes_shape)
			  PropagateShapes(ewin->win);
		    }
		  ewin->shapedone = 1;
	       }
	  }
     }
   if (a->iclass)
      a->iclass->ref_count--;

   Efree(a);

   EDBUG_RETURN_;
}

static void
EwinBorderSelect(EWin * ewin)
{
   Border             *b;

   ewin->normal_border = ewin->border;

   /* Quit if we allready have a border that isn't the fallback one */
   b = ewin->border;
   if (b && strcmp(b->name, "__FALLBACK_BORDER"))
      return;

   ICCCM_GetShapeInfo(ewin);

   if ((!ewin->client.mwm_decor_title) && (!ewin->client.mwm_decor_border))
      b = (Border *) FindItem("BORDERLESS", 0, LIST_FINDBY_NAME,
			      LIST_TYPE_BORDER);
   else
      b = MatchEwinByFunction(ewin,
			      (void
			       *(*)(EWin *, WindowMatch *))(MatchEwinBorder));
   if (Conf.dockapp_support && ewin->docked)
      b = (Border *) FindItem("BORDERLESS", 0, LIST_FINDBY_NAME,
			      LIST_TYPE_BORDER);
   if (!b)
      b = (Border *) FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);

   if (!b)
      b = FindItem("__FALLBACK_BORDER", 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);

   ewin->border = b;
}

/*
 * Derive frame window position from client window and border properties
 */
void
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

   ewin->shape_x = ewin->x = x;
   ewin->shape_y = ewin->y = y;

   ewin->w = ewin->client.w +
      ewin->border->border.left + ewin->border->border.right;
   ewin->h = ewin->client.h +
      ewin->border->border.top + ewin->border->border.bottom;
}

static EWin        *
Adopt(Window win)
{
   EWin               *ewin;

   EDBUG(4, "Adopt");

   GrabX();
   ewin = EwinCreate(win);

   ICCCM_AdoptStart(ewin);
   ICCCM_GetTitle(ewin, 0);
   ICCCM_GetHints(ewin, 0);
   ICCCM_GetInfo(ewin, 0);
   ICCCM_GetColormap(ewin);
   ICCCM_GetShapeInfo(ewin);
   ICCCM_GetGeoms(ewin, 0);
   HintsGetWindowHints(ewin);
   SessionGetInfo(ewin, 0);
   MatchEwinToSM(ewin);
   MatchEwinToSnapInfo(ewin);
   if (Mode.wm.startup)
      ICCCM_GetEInfo(ewin);
   ICCCM_MatchSize(ewin);
   ICCCM_Adopt(ewin);

   EwinBorderSelect(ewin);
   EwinBorderSetTo(ewin, NULL);

   EwinGetGeometry(ewin);
   EwinEventsConfigure(ewin, 0);

   UngrabX();

   if (ewin->shaded)
      EwinInstantShade(ewin, 1);

   HintsSetWindowState(ewin);
   HintsSetClientList();

   EDBUG_RETURN(ewin);
}

static EWin        *
AdoptInternal(Window win, Border * border, int type)
{
   EWin               *ewin;

   EDBUG(4, "AdoptInternal");

   GrabX();
   ewin = EwinCreate(win);

   ewin->border = border;
   ewin->internal = 1;
   ewin->type = type;
   switch (type)
     {
     case EWIN_TYPE_DIALOG:
	break;
     case EWIN_TYPE_MENU:
	ewin->layer = 99;
	ewin->skiptask = 1;
	ewin->skip_ext_pager = 1;
	ewin->no_actions = 1;
	ewin->skipfocus = 1;
	break;
     case EWIN_TYPE_ICONBOX:
	ewin->sticky = 1;
	ewin->skiptask = 1;
	ewin->skip_ext_pager = 1;
	ewin->skipfocus = 1;
	break;
     case EWIN_TYPE_PAGER:
	ewin->sticky = 1;
	ewin->skiptask = 1;
	ewin->skip_ext_pager = 1;
	ewin->skipfocus = 1;
	break;
     }

   ICCCM_AdoptStart(ewin);
   ICCCM_GetTitle(ewin, 0);
   ICCCM_GetInfo(ewin, 0);
   ICCCM_GetShapeInfo(ewin);
   ICCCM_GetGeoms(ewin, 0);
   switch (type)
     {
     case EWIN_TYPE_DIALOG:
     case EWIN_TYPE_MENU:
	ewin->client.width.min = ewin->client.width.max = ewin->client.w;
	ewin->client.height.min = ewin->client.height.max = ewin->client.h;
	ewin->client.no_resize_h = ewin->client.no_resize_v = 1;
	break;
     }
   MatchEwinToSnapInfo(ewin);
   ICCCM_MatchSize(ewin);
   ICCCM_Adopt(ewin);

   EwinBorderSelect(ewin);
   EwinBorderSetTo(ewin, NULL);

   EwinGetGeometry(ewin);
   EwinEventsConfigure(ewin, 0);

   UngrabX();

   if (ewin->shaded)
      EwinInstantShade(ewin, 1);

   HintsSetWindowState(ewin);
   HintsSetClientList();

   EDBUG_RETURN(ewin);
}

static EWin        *
EwinCreate(Window win)
{
   EWin               *ewin;
   XSetWindowAttributes att;

   EDBUG(5, "EwinCreate");

   ewin = Ecalloc(1, sizeof(EWin));

   ewin->state = (Mode.wm.startup) ? EWIN_STATE_STARTUP : EWIN_STATE_NEW;
   ewin->x = -1;
   ewin->y = -1;
   ewin->w = -1;
   ewin->h = -1;
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
   ewin->desktop = desks.current;
   ewin->layer = 4;
   ewin->win = ECreateWindow(VRoot.win, -10, -10, 1, 1, 1);
   ewin->win_container = ECreateWindow(ewin->win, 0, 0, 1, 1, 0);
#if 0				/* ENABLE_GNOME - Not actually used */
   ewin->expanded_width = -1;
   ewin->expanded_height = -1;
#endif
   ewin->area_x = -1;
   ewin->area_y = -1;

   att.event_mask = EWIN_CONTAINER_EVENT_MASK;
   att.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
   XChangeWindowAttributes(disp, ewin->win_container,
			   CWEventMask | CWDontPropagate, &att);
   EMapWindow(disp, ewin->win_container);

   att.event_mask = EWIN_TOP_EVENT_MASK;
   att.do_not_propagate_mask = ButtonPressMask | ButtonReleaseMask;
   XChangeWindowAttributes(disp, ewin->win,
			   CWEventMask | CWDontPropagate, &att);
   FocusEwinSetGrabs(ewin);
   GrabButtonGrabs(ewin);
   EwinListAdd(&EwinListStack, ewin, 0);
   EwinListAdd(&EwinListFocus, ewin, 0);

   ewin->client.win = win;
   ewin->client.event_mask = EWIN_CLIENT_EVENT_MASK;
   AddItem(ewin, "EWIN", win, LIST_TYPE_EWIN);

   XShapeSelectInput(disp, win, ShapeNotifyMask);

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinCreate %#lx state=%d\n", ewin->client.win, ewin->state);

   EDBUG_RETURN(ewin);
}

static void
EwinDestroy(EWin * ewin)
{
   EWin              **lst;
   int                 i, num;

   EDBUG(5, "FreeEwin");
   if (!ewin)
      EDBUG_RETURN_;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinDestroy %#lx state=%d\n", ewin->client.win, ewin->state);

   RemoveItem(NULL, ewin->client.win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
   EwinListDelete(&EwinListStack, ewin);
   EwinListDelete(&EwinListFocus, ewin);

   XSelectInput(disp, ewin->client.win, 0);

   HintsSetClientList();

   UnmatchEwinToSnapInfo(ewin);

   if (ewin->iconified > 0)
      RemoveMiniIcon(ewin);

   lst = EwinListTransientFor(ewin, &num);
   for (i = 0; i < num; i++)
     {
	lst[i]->has_transients--;
	if (lst[i]->has_transients < 0)	/* Paranoia? */
	   lst[i]->has_transients = 0;
     }
   if (lst)
      Efree(lst);

   if (ewin->border)
      ewin->border->ref_count--;
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
   if (ewin->win)
      EDestroyWindow(disp, ewin->win);
   if (ewin->bits)
      Efree(ewin->bits);
   if (ewin->session_id)
      Efree(ewin->session_id);
   FreePmapMask(&ewin->mini_pmm);
   FreePmapMask(&ewin->icon_pmm);
   GroupsEwinRemove(ewin);
   Efree(ewin);

   EDBUG_RETURN_;
}

void
EwinWithdraw(EWin * ewin)
{
   Window              win;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinWithdraw %#lx state=%d\n", ewin->client.win, ewin->state);

   /* Park the client window on the root */
   XTranslateCoordinates(disp, ewin->client.win, VRoot.win,
			 -ewin->border->border.left,
			 -ewin->border->border.top, &ewin->client.x,
			 &ewin->client.y, &win);
   EReparentWindow(disp, ewin->client.win, VRoot.win, ewin->client.x,
		   ewin->client.y);

   ICCCM_Withdraw(ewin);
   HintsDelWindowHints(ewin);
   EwinDestroy(ewin);
}

void
EwinReparent(EWin * ewin, Window parent)
{
   EReparentWindow(disp, ewin->client.win, parent, 0, 0);
   EwinDestroy(ewin);
}

void
EwinEventDestroy(EWin * ewin)
{
   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventDestroy %#lx state=%d\n", ewin->client.win,
	      ewin->state);

   EwinDestroy(ewin);
}

void
EwinEventMap(EWin * ewin)
{
   int                 old_state = ewin->state;

   ewin->state = EWIN_STATE_MAPPED;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventMap %#lx state=%d\n", ewin->client.win, ewin->state);

   /* If first time we may want to focus it (unless during startup) */
   if (old_state == EWIN_STATE_NEW)
      FocusToEWin(ewin, FOCUS_EWIN_NEW);
}

void
EwinEventUnmap(EWin * ewin)
{
   if (GetZoomEWin() == ewin)
      Zoom(NULL);

   if (ewin->state == EWIN_STATE_NEW)
     {
	Eprintf("EwinEventUnmap %#lx: Ignoring bogus Unmap event\n",
		ewin->client.win);
	return;
     }

   /* Set state to unknown until we can set the correct one */
   ewin->state = (ewin->iconified) ? EWIN_STATE_ICONIC : EWIN_STATE_WITHDRAWN;

   if (EventDebug(EDBUG_TYPE_EWINS))
      Eprintf("EwinEventUnmap %#lx state=%d\n", ewin->client.win, ewin->state);

   ActionsEnd(ewin);

   if (ewin->pager)
      PagerEventUnmap(ewin->pager);

   if (Conf.dockapp_support && ewin->docked)
      DockDestroy(ewin);

   if (ewin == GetContextEwin())
      SlideoutsHide();

   if (ewin == Mode.focuswin)
      FocusToEWin(ewin, FOCUS_EWIN_GONE);
   if (ewin == Mode.mouse_over_ewin)
      Mode.mouse_over_ewin = NULL;

   /* hide any menus this ewin has brought up if they are still up when we */
   /* destroy this ewin */
   if (ewin->shownmenu)
      MenusHideByWindow(ewin->shownmenu);

   if (Mode.doingslide)
     {
	DrawEwinShape(ewin, Conf.slidemode, ewin->shape_x, ewin->shape_y,
		      ewin->client.w, ewin->client.h, 2);
	Mode.doingslide = 0;
     }

   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }

   if (ewin->iconified)
     {
	HideEwin(ewin);
	return;
     }

   if (ewin->Close)
      ewin->Close(ewin);

   EwinWithdraw(ewin);
}

static void
EwinBorderSetTo(EWin * ewin, Border * b)
{
   int                 i;
   int                 px = 0, py = 0;
   char                s[1024];

   AwaitIclass        *await;

   EDBUG(4, "EwinBorderSetTo");

   if (ewin->border == b)
      EDBUG_RETURN_;

   if (b == NULL)
     {
	b = ewin->border;
	ewin->border = NULL;
     }

   if (ewin->border)
     {
	px = ewin->border->border.left;
	py = ewin->border->border.top;
	for (i = 0; i < ewin->border->num_winparts; i++)
	  {
	     if (ewin->bits[i].win)
		EDestroyWindow(disp, ewin->bits[i].win);
	  }
	if (ewin->bits)
	   Efree(ewin->bits);
	ewin->bits = NULL;
	ewin->border->ref_count--;
     }

   ewin->border = b;
   b->ref_count++;
   HintsSetWindowBorder(ewin);

   if (b->num_winparts > 0)
      ewin->bits = Emalloc(sizeof(EWinBit) * b->num_winparts);
   for (i = 0; i < b->num_winparts; i++)
     {
	if (b->part[i].iclass->external)
	  {
	     ewin->bits[i].win = 0;
	     Esnprintf(s, sizeof(s), "request imageclass %s",
		       b->part[i].iclass->name);
	     CommsBroadcast(s);
	     await = Emalloc(sizeof(AwaitIclass));
	     await->client_win = ewin->client.win;
	     await->ewin_bit = i;

	     await->iclass = b->part[i].iclass;
	     if (await->iclass)
		await->iclass->ref_count++;

	     AddItem(await, b->part[i].iclass->name, 0, LIST_TYPE_AWAIT_ICLASS);
	  }
	else
	  {
	     ewin->bits[i].win = ECreateWindow(ewin->win, -10, -10, 1, 1, 0);
	     ApplyECursor(ewin->bits[i].win, b->part[i].ec);
	     EMapWindow(disp, ewin->bits[i].win);
	     /*
	      * KeyPressMask KeyReleaseMask ButtonPressMask 
	      * ButtonReleaseMask
	      * EnterWindowMask LeaveWindowMask PointerMotionMask 
	      * PointerMotionHintMask Button1MotionMask 
	      * Button2MotionMask
	      * Button3MotionMask Button4MotionMask Button5MotionMask
	      * ButtonMotionMask KeymapStateMask ExposureMask 
	      * VisibilityChangeMask StructureNotifyMask 
	      * ResizeRedirectMask 
	      * SubstructureNotifyMask SubstructureRedirectMask 
	      * FocusChangeMask PropertyChangeMas ColormapChangeMask 
	      * OwnerGrabButtonMask
	      */
	     if (b->part[i].flags & FLAG_TITLE)
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_TITLE_EVENT_MASK);
	     else
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_PART_EVENT_MASK);
	     ewin->bits[i].x = -10;
	     ewin->bits[i].y = -10;
	     ewin->bits[i].w = -10;
	     ewin->bits[i].h = -10;
	     ewin->bits[i].cx = -99;
	     ewin->bits[i].cy = -99;
	     ewin->bits[i].cw = -99;
	     ewin->bits[i].ch = -99;
	     ewin->bits[i].state = 0;
	     ewin->bits[i].expose = 0;
	     ewin->bits[i].no_expose = 0;
	     ewin->bits[i].left = 0;
	     ewin->bits[i].is = NULL;
	  }
     }

   {
      Window             *wl;
      int                 j = 0;

      wl = Emalloc((b->num_winparts + 1) * sizeof(Window));
      for (i = b->num_winparts - 1; i >= 0; i--)
	{
	   if (b->part[i].ontop)
	      wl[j++] = ewin->bits[i].win;
	}
      wl[j++] = ewin->win_container;
      for (i = b->num_winparts - 1; i >= 0; i--)
	{
	   if (!b->part[i].ontop)
	      wl[j++] = ewin->bits[i].win;
	}
      XRestackWindows(disp, wl, j);
      Efree(wl);
   }

   if (!ewin->shaded)
      EMoveWindow(disp, ewin->win_container, b->border.left, b->border.top);

   CalcEwinSizes(ewin);
   PropagateShapes(ewin->win);

   EDBUG_RETURN_;
}

void
EwinSetBorder(EWin * ewin, Border * b, int apply)
{
   if (!b)
      return;

   if (apply)
     {
	if (ewin->border != b)
	  {
	     EwinBorderSetTo(ewin, b);
	     ICCCM_MatchSize(ewin);
	     MoveResizeEwin(ewin, ewin->x, ewin->y,
			    ewin->client.w, ewin->client.h);
	  }
     }
   else
     {
	ewin->border = b;
     }
}

void
EwinSetBorderByName(EWin * ewin, const char *name, int apply)
{
   Border             *b;

   b = (Border *) FindItem(name, 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);

   EwinSetBorder(ewin, b, apply);
}

void
EwinRefresh(EWin * ewin)
{
   if (!ewin)
      return;

   if (Conf.theme.transparency)
      EwinBorderDraw(ewin, 0, 0);	/* Update the border */

   if (ewin->Refresh)
      ewin->Refresh(ewin);
}

void
EwinUpdateAfterMoveResize(EWin * ewin, int resize)
{
   if (!ewin)
      return;

   EwinDetermineArea(ewin);

   if (Conf.theme.transparency)
      EwinBorderDraw(ewin, 1, 0);	/* Update the border */

   if (ewin->MoveResize)
      ewin->MoveResize(ewin, resize);

   PagerEwinOutsideAreaUpdate(ewin);
   ForceUpdatePagersForDesktop(ewin->desktop);
}

void
EwinFixPosition(EWin * ewin)
{
   int                 x, y;

   if (ewin->state != EWIN_STATE_MAPPED)
      return;

   x = ewin->x;
   y = ewin->y;
   if ((ewin->x + ewin->border->border.left + 1) > VRoot.w)
      x = VRoot.w - ewin->border->border.left - 1;
   else if ((ewin->x + ewin->w - ewin->border->border.right - 1) < 0)
      x = 0 - ewin->w + ewin->border->border.right + 1;
   if ((ewin->y + ewin->border->border.top + 1) > VRoot.h)
      y = VRoot.h - ewin->border->border.top - 1;
   else if ((ewin->y + ewin->h - ewin->border->border.bottom - 1) < 0)
      y = 0 - ewin->h + ewin->border->border.bottom + 1;

   if (x != ewin->x || y != ewin->y)
      MoveEwin(ewin, x, y);
}

#define MR_FLAGS_MOVE   1
#define MR_FLAGS_RESIZE 2

static void
doMoveResizeEwin(EWin * ewin, int x, int y, int w, int h, int flags)
{
   static int          call_depth = 0;
   int                 dx = 0, dy = 0, sw, sh, x0, y0;
   char                move = 0, resize = 0;
   EWin              **lst;
   int                 i, num;

   EDBUG(3, "doMoveResizeEwin");
   if (call_depth > 256)
      EDBUG_RETURN_;
   call_depth++;

   if (EventDebug(EDBUG_TYPE_MOVERESIZE))
      Eprintf("doMoveResizeEwin(%d) %#lx %d+%d %d*%d %d %s\n", call_depth,
	      ewin->client.win, x, y, w, h, flags, EwinGetTitle(ewin));

   if (Mode.mode == MODE_NONE && Mode.move.check)
     {
	/* Don't throw windows offscreen */
	sw = VRoot.w;
	sh = VRoot.h;
	if (ewin->sticky)
	  {
	     x0 = y0 = 0;
	  }
	else
	  {
	     int                 ax, ay;

	     ax = desks.desk[ewin->desktop].current_area_x;
	     ay = desks.desk[ewin->desktop].current_area_y;
	     x0 = -ax * sw;
	     y0 = -ay * sh;
	     sw *= Conf.areas.nx;
	     sh *= Conf.areas.ny;
	  }
	dx = ewin->w / 4;
	if (dx > 8)
	   dx = 8;
	dy = ewin->h / 4;
	if (dy > 8)
	   dy = 8;

	if (x < x0 - ewin->w + dx)
	   x = x0 - ewin->w + dx;
	else if (x > x0 + sw - dx)
	   x = x0 + sw - dx;
	if (y < y0 - ewin->h + dy)
	   y = y0 - ewin->h + dy;
	else if (y > y0 + sh - dy)
	   y = y0 + sh - dy;
     }

   if (flags & MR_FLAGS_MOVE)
     {
	dx = x - ewin->x;
	dy = y - ewin->y;
	if ((dx != 0) || (dy != 0))
	   move = 1;
	ewin->x = x;
	ewin->y = y;
	ewin->client.x = x + ewin->border->border.left;
	ewin->client.y = y + ewin->border->border.top;
     }

   if (flags & MR_FLAGS_RESIZE)
     {
	if ((w != ewin->client.w) || (h != ewin->client.h))
	   resize = 2;
	ewin->client.w = w;
	ewin->client.h = h;
	ICCCM_MatchSize(ewin);

	if (!ewin->shaded)
	  {
	     ewin->w = ewin->client.w + ewin->border->border.left +
		ewin->border->border.right;
	     ewin->h = ewin->client.h + ewin->border->border.top +
		ewin->border->border.bottom;
	  }
     }

   EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);

   if ((Mode.mode != MODE_MOVE_PENDING && Mode.mode != MODE_MOVE)
       || (Mode.have_place_grab))
      ICCCM_Configure(ewin);

   CalcEwinSizes(ewin);

   if (move)
     {

	lst = EwinListTransients(ewin, &num, 0);
	for (i = 0; i < num; i++)
	  {
#if 0				/* Why? */
	     if (!((Mode.flipp) && (lst[i]->floating))
		 && (lst[i]->client.mwm_decor_border
		     || lst[i]->client.mwm_decor_resizeh
		     || lst[i]->client.mwm_decor_title
		     || lst[i]->client.mwm_decor_menu
		     || lst[i]->client.mwm_decor_minimize
		     || lst[i]->client.mwm_decor_maximize))
#endif
		MoveEwin(lst[i], lst[i]->x + dx, lst[i]->y + dy);
	  }
	if (lst)
	   Efree(lst);
     }

   if ((Mode.mode == MODE_NONE) /* && (move || resize) */ )
     {
	ewin->shape_x = x;
	ewin->shape_y = y;
	EwinUpdateAfterMoveResize(ewin, resize);
     }

   call_depth--;
   EDBUG_RETURN_;
}

void
MoveEwin(EWin * ewin, int x, int y)
{
   doMoveResizeEwin(ewin, x, y, 0, 0, MR_FLAGS_MOVE);
}

void
ResizeEwin(EWin * ewin, int w, int h)
{
   doMoveResizeEwin(ewin, 0, 0, w, h, MR_FLAGS_RESIZE);
}

void
MoveResizeEwin(EWin * ewin, int x, int y, int w, int h)
{
   doMoveResizeEwin(ewin, x, y, w, h, MR_FLAGS_MOVE | MR_FLAGS_RESIZE);
}

#if 0				/* Unused */
void
FloatEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   EDBUG(3, "FloatEwin");
   call_depth++;
   if (call_depth > 256)
      EDBUG_RETURN_;
   ewin->floating = 1;
   ewin->desktop = 0;
   ConformEwinToDesktop(ewin);
   RaiseEwin(ewin);

   lst = EwinListTransients(ewin, &num, 0);
   for (i = 0; i < num; i++)
      FloatEwin(lst[i]);
   if (lst)
      Efree(lst);

   call_depth--;
   EDBUG_RETURN_;
}
#endif

void
FloatEwinAt(EWin * ewin, int x, int y)
{
   static int          call_depth = 0;
   int                 dx, dy;
   EWin              **lst;
   int                 i, num;

   EDBUG(3, "FloatEwinAt");
   call_depth++;
   if (call_depth > 256)
      EDBUG_RETURN_;
   if (ewin->floating)
      ewin->floating = 2;
   else
      ewin->floating = 1;
   dx = x - ewin->x;
   dy = y - ewin->y;
   ewin->x = x;
   ewin->y = y;
   ConformEwinToDesktop(ewin);

   lst = EwinListTransients(ewin, &num, 0);
   for (i = 0; i < num; i++)
      FloatEwinAt(lst[i], lst[i]->x + dx, lst[i]->y + dy);
   if (lst)
      Efree(lst);

   call_depth--;
   EDBUG_RETURN_;
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

   EDBUG(3, "RestackEwin");

   if (EventDebug(EDBUG_TYPE_STACKING))
      Eprintf("RestackEwin %#lx %s\n", ewin->client.win, EwinGetTitle(ewin));

   if (ewin->floating)
     {
	XRaiseWindow(disp, ewin->win);
	goto done;
     }

   lst = EwinListGetForDesktop(ewin->desktop, &num);
   if (num < 2)
      goto done;

   for (i = 0; i < num; i++)
      if (lst[i] == ewin)
	 break;
   if (i < num - 1)
     {
	xwc.stack_mode = Above;
	xwc.sibling = lst[i + 1]->win;
     }
   else
     {
	xwc.stack_mode = Below;
	xwc.sibling = lst[i - 1]->win;
     }
   value_mask = CWSibling | CWStackMode;
   if (EventDebug(EDBUG_TYPE_STACKING))
      Eprintf("RestackEwin %#10lx %s %#10lx\n", ewin->win,
	      (xwc.stack_mode == Above) ? "Above" : "Below", xwc.sibling);
   XConfigureWindow(disp, ewin->win, value_mask, &xwc);
   HintsSetClientStacking();

   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }

 done:
   EDBUG_RETURN_;
}

void
RaiseEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   EDBUG(3, "RaiseEwin");
   if (call_depth > 256)
      EDBUG_RETURN_;
   call_depth++;

   if (EventDebug(EDBUG_TYPE_RAISELOWER))
      Eprintf("RaiseEwin(%d) %#lx %s\n", call_depth, ewin->client.win,
	      EwinGetTitle(ewin));

   if (ewin->win)
     {
	if (ewin->floating)
	  {
	     XRaiseWindow(disp, ewin->win);
	     goto done;
	  }

	num = EwinListStackingRaise(ewin);
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
		StackDesktop(ewin->desktop);	/* Do the full stacking */
	     else
		RestackEwin(ewin);	/* Restack this one only */
	  }
     }

 done:
   call_depth--;
   EDBUG_RETURN_;
}

void
LowerEwin(EWin * ewin)
{
   static int          call_depth = 0;
   EWin              **lst;
   int                 i, num;

   EDBUG(3, "LowerEwin");
   if (call_depth > 256)
      EDBUG_RETURN_;
   call_depth++;

   if (EventDebug(EDBUG_TYPE_RAISELOWER))
      Eprintf("LowerEwin(%d) %#lx %s\n", call_depth, ewin->client.win,
	      EwinGetTitle(ewin));

   if ((ewin->win) && (!ewin->floating))
     {
	num = EwinListStackingLower(ewin);
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
		StackDesktop(ewin->desktop);	/* Do the full stacking */
	     else
		RestackEwin(ewin);	/* Restack this one only */
	  }
     }

 done:
   call_depth--;
   EDBUG_RETURN_;
}

void
ShowEwin(EWin * ewin)
{
   EDBUG(3, "ShowEwin");

   if (ewin->visible)
      EDBUG_RETURN_;
   ewin->visible = 1;

   if (ewin->client.win)
     {
	if (ewin->shaded)
	   EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	EMapWindow(disp, ewin->client.win);
     }

   if (ewin->win)
      EMapWindow(disp, ewin->win);

   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }

   EDBUG_RETURN_;
}

void
HideEwin(EWin * ewin)
{
   EDBUG(3, "HideEwin");

   if (ewin->state != EWIN_STATE_MAPPED || !ewin->visible)
      EDBUG_RETURN_;
   ewin->visible = 0;

   if (GetZoomEWin() == ewin)
      Zoom(NULL);

   if (ewin->win)
      EUnmapWindow(disp, ewin->win);

   EDBUG_RETURN_;
}

void
FreeBorder(Border * b)
{
   int                 i;

   EDBUG(3, "FreeBorder");

   if (!b)
      EDBUG_RETURN_;

   if (b->ref_count > 0)
     {
	DialogOK(_("Border Error!"), _("%u references remain\n"), b->ref_count);
	EDBUG_RETURN_;
     }

   while (RemoveItemByPtr(b, LIST_TYPE_BORDER));

   for (i = 0; i < b->num_winparts; i++)
     {
	if (b->part[i].iclass)
	   b->part[i].iclass->ref_count--;
	if (b->part[i].tclass)
	   b->part[i].tclass->ref_count--;
	if (b->part[i].aclass)
	   b->part[i].aclass->ref_count--;
	if (b->part[i].ec)
	   b->part[i].ec->ref_count--;
     }

   if (b->num_winparts > 0)
      Efree(b->part);

   if (b->name)
      Efree(b->name);
   if (b->group_border_name)
      Efree(b->group_border_name);

   EDBUG_RETURN_;
}

Border             *
CreateBorder(const char *name)
{
   Border             *b;

   EDBUG(5, "CreateBorder");

   b = Emalloc(sizeof(Border));
   if (!b)
      EDBUG_RETURN(NULL);

   b->name = Estrdup(name);
   b->group_border_name = NULL;
   b->border.left = 0;
   b->border.right = 0;
   b->border.top = 0;
   b->border.bottom = 0;
   b->num_winparts = 0;
   b->part = NULL;
   b->changes_shape = 0;
   b->shadedir = 2;
   b->ref_count = 0;

   EDBUG_RETURN(b);
}

void
AddBorderPart(Border * b, ImageClass * iclass, ActionClass * aclass,
	      TextClass * tclass, ECursor * ec, char ontop, int flags,
	      char isregion, int wmin, int wmax, int hmin, int hmax,
	      int torigin, int txp, int txa, int typ, int tya, int borigin,
	      int bxp, int bxa, int byp, int bya, char keep_for_shade)
{
   int                 n;

   EDBUG(6, "AddBorderPart");

   b->num_winparts++;
   n = b->num_winparts;

   isregion = 0;
   if (!b->part)
     {
	b->part = Emalloc(n * sizeof(WinPart));
     }
   else
     {
	b->part = Erealloc(b->part, n * sizeof(WinPart));
     }

   if (!iclass)
      iclass =
	 FindItem("__FALLBACK_ICLASS", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);

   b->part[n - 1].iclass = iclass;
   if (iclass)
      iclass->ref_count++;

   b->part[n - 1].aclass = aclass;
   if (aclass)
      aclass->ref_count++;

   b->part[n - 1].tclass = tclass;
   if (tclass)
      tclass->ref_count++;

   b->part[n - 1].ec = ec;
   if (ec)
      ec->ref_count++;

   b->part[n - 1].ontop = ontop;
   b->part[n - 1].flags = flags;
   b->part[n - 1].keep_for_shade = keep_for_shade;
   b->part[n - 1].geom.width.min = wmin;
   b->part[n - 1].geom.width.max = wmax;
   b->part[n - 1].geom.height.min = hmin;
   b->part[n - 1].geom.height.max = hmax;
   b->part[n - 1].geom.topleft.originbox = torigin;
   b->part[n - 1].geom.topleft.x.percent = txp;
   b->part[n - 1].geom.topleft.x.absolute = txa;
   b->part[n - 1].geom.topleft.y.percent = typ;
   b->part[n - 1].geom.topleft.y.absolute = tya;
   b->part[n - 1].geom.bottomright.originbox = borigin;
   b->part[n - 1].geom.bottomright.x.percent = bxp;
   b->part[n - 1].geom.bottomright.x.absolute = bxa;
   b->part[n - 1].geom.bottomright.y.percent = byp;
   b->part[n - 1].geom.bottomright.y.absolute = bya;

   EDBUG_RETURN_;
}

void
EwinUnStick(EWin * ewin)
{

   EDBUG(4, "EwinUnStick");
   if (!ewin)
      EDBUG_RETURN_;

   ewin->sticky = 0;
   MoveEwinToDesktopAt(ewin, desks.current, ewin->x, ewin->y);
   EwinBorderUpdateState(ewin);
   HintsSetWindowState(ewin);

   EDBUG_RETURN_;
}

void
EwinStick(EWin * ewin)
{
   int                 x, y, dx, dy;

   EDBUG(4, "EwinStick");
   if (!ewin)
      EDBUG_RETURN_;

   /* Avoid "losing" windows made sticky while not in the current viewport */
   dx = ewin->w / 2;
   dy = ewin->h / 2;
   x = (ewin->x + dx) % VRoot.w;
   if (x < 0)
      x += VRoot.w;
   x -= dx;
   y = (ewin->y + dy) % VRoot.h;
   if (y < 0)
      y += VRoot.h;
   y -= dy;

   MoveEwinToDesktopAt(ewin, desks.current, x, y);
   ewin->sticky = 1;
   EwinBorderUpdateState(ewin);
   HintsSetWindowState(ewin);

   EDBUG_RETURN_;
}

static void
MinShadeSize(EWin * ewin, int *mw, int *mh)
{
   int                 i, pw, ph, w, h, min_w, min_h;
   int                 leftborderwidth, rightborderwidth;
   int                 topborderwidth, bottomborderwidth;

   min_w = 32;
   min_h = 32;

   if (!ewin)
      goto done;

   pw = ewin->w;
   ph = ewin->h;

   for (i = 0; i < ewin->border->num_winparts; i++)
      ewin->bits[i].w = -2;
   for (i = 0; i < ewin->border->num_winparts; i++)
      if (ewin->bits[i].w == -2)
	 BorderWinpartCalc(ewin, i);

   switch (ewin->border->shadedir)
     {
     case 0:
     case 1:
	/* get the correct width, based on the borderparts that */
	/*are remaining visible */
	leftborderwidth = rightborderwidth = 0;
	for (i = 0; i < ewin->border->num_winparts; i++)
	  {
	     if (!ewin->border->part[i].keep_for_shade)
		continue;

	     w = ewin->border->border.left - ewin->bits[i].x;
	     if (leftborderwidth < w)
		leftborderwidth = w;

	     w = ewin->bits[i].x + ewin->bits[i].w -
		(ewin->w - ewin->border->border.right);
	     if (rightborderwidth < w)
		rightborderwidth = w;
	  }
	ewin->w = rightborderwidth + leftborderwidth;
	break;
     case 2:
     case 3:
	topborderwidth = bottomborderwidth = 0;
	for (i = 0; i < ewin->border->num_winparts; i++)
	  {
	     if (!ewin->border->part[i].keep_for_shade)
		continue;

	     h = ewin->border->border.top - ewin->bits[i].y;
	     if (topborderwidth < h)
		topborderwidth = h;

	     h = ewin->bits[i].y + ewin->bits[i].h -
		(ewin->h - ewin->border->border.bottom);
	     if (bottomborderwidth < h)
		bottomborderwidth = h;
	  }
	ewin->h = bottomborderwidth + topborderwidth;
	break;
     default:
	break;
     }

   for (i = 0; i < ewin->border->num_winparts; i++)
      ewin->bits[i].w = -2;
   for (i = 0; i < ewin->border->num_winparts; i++)
      if (ewin->bits[i].w == -2)
	 BorderWinpartCalc(ewin, i);

   min_w = 0;
   min_h = 0;
   for (i = 0; i < ewin->border->num_winparts; i++)
     {
	if (!ewin->border->part[i].keep_for_shade)
	   continue;

	w = ewin->bits[i].x + ewin->bits[i].w;
	if (min_w < w)
	   min_w = w;

	h = ewin->bits[i].y + ewin->bits[i].h;
	if (min_h < h)
	   min_h = h;
     }

   ewin->w = pw;
   ewin->h = ph;

 done:
   *mw = min_w;
   *mh = min_h;
}

void
EwinInstantShade(EWin * ewin, int force)
{
   XSetWindowAttributes att;
   int                 b, d;
   char                pq;

   EDBUG(4, "EwinInstantShade");

   if ((ewin->border->border.left == 0) && (ewin->border->border.right == 0)
       && (ewin->border->border.top == 0) && (ewin->border->border.bottom == 0))
      EDBUG_RETURN_;
   if (GetZoomEWin() == ewin)
      EDBUG_RETURN_;
   if (ewin->shaded && !force)
      EDBUG_RETURN_;

   pq = Mode.queue_up;
   Mode.queue_up = 0;
   switch (ewin->border->shadedir)
     {
     case 0:
	att.win_gravity = EastGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	ewin->shaded = 2;
	ewin->w = b;
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 1:
	att.win_gravity = WestGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	d = ewin->x + ewin->w - b;
	ewin->shaded = 2;
	ewin->w = b;
	ewin->x = d;
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 2:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	b = d;
	ewin->shaded = 2;
	ewin->h = b;
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 3:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	b = d;
	d = ewin->y + ewin->h - b;
	ewin->shaded = 2;
	ewin->h = b;
	ewin->y = d;
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     default:
	break;
     }
   PropagateShapes(ewin->win);
   Mode.queue_up = pq;
   HintsSetWindowState(ewin);
   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }
   EDBUG_RETURN_;
}

void
EwinInstantUnShade(EWin * ewin)
{
   XSetWindowAttributes att;
   int                 b, d;
   char                pq;

   EDBUG(4, "EwinInstantUnShade");
   if (GetZoomEWin() == ewin)
      EDBUG_RETURN_;
   if (!ewin->shaded)
      EDBUG_RETURN_;
   pq = Mode.queue_up;
   Mode.queue_up = 0;
   switch (ewin->border->shadedir)
     {
     case 0:
	att.win_gravity = EastGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	b = ewin->client.w + ewin->border->border.left +
	   ewin->border->border.right;
	ewin->shaded = 0;
	ewin->w = b;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 1:
	att.win_gravity = WestGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	b = ewin->client.w + ewin->border->border.left +
	   ewin->border->border.right;
	d = ewin->x + ewin->w - (ewin->border->border.right + ewin->client.w +
				 ewin->border->border.left);
	ewin->shaded = 0;
	ewin->w = b;
	ewin->x = d;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 2:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	b = ewin->client.h + ewin->border->border.top +
	   ewin->border->border.bottom;
	ewin->shaded = 0;
	ewin->h = b;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 3:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	b = ewin->client.h + ewin->border->border.top +
	   ewin->border->border.bottom;
	d = ewin->y + ewin->h - (ewin->border->border.bottom +
				 ewin->client.h + ewin->border->border.top);
	ewin->shaded = 0;
	ewin->h = b;
	ewin->y = d;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     default:
	break;
     }
   PropagateShapes(ewin->win);
   Mode.queue_up = pq;
   HintsSetWindowState(ewin);
   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }
   EDBUG_RETURN_;
}

void
EwinShade(EWin * ewin)
{
   XSetWindowAttributes att;
   int                 i, j, speed, a, b, c, d, ww, hh;
   int                 k, spd, min;
   struct timeval      timev1, timev2;
   int                 dsec, dusec;
   double              tm;
   char                pq;

   EDBUG(4, "EwinShade");

   if ((ewin->border->border.left == 0) && (ewin->border->border.right == 0)
       && (ewin->border->border.top == 0) && (ewin->border->border.bottom == 0))
      EDBUG_RETURN_;
   if (GetZoomEWin() == ewin)
      EDBUG_RETURN_;
   if (ewin->shaded)
      EDBUG_RETURN_;
   if ((ewin->border) && (!strcmp(ewin->border->name, "BORDERLESS")))
      EDBUG_RETURN_;
   pq = Mode.queue_up;
   Mode.queue_up = 0;
   speed = Conf.shadespeed;
   spd = 32;
   min = 2;
   GrabX();
   switch (ewin->border->shadedir)
     {
     case 0:
	att.win_gravity = EastGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	a = ewin->w;
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		ewin->w = i;
		if (ewin->w < 1)
		   ewin->w = 1;
		ww = ewin->w - ewin->border->border.left -
		   ewin->border->border.right;
		if (ww < 1)
		   ww = 1;
		hh = ewin->client.h;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ww, hh);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, -(ewin->client.w - ww),
				      0, ewin->client.win, ShapeBounding,
				      ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->shaded = 2;
	ewin->w = b;
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 1:
	att.win_gravity = WestGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	a = ewin->w;
	c = ewin->x;
	d = ewin->x + ewin->w - b;
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		j = ((c * (1024 - k)) + (d * k)) >> 10;
		ewin->w = i;
		ewin->x = j;
		if (ewin->w < 1)
		   ewin->w = 1;
		ww = ewin->w - ewin->border->border.left -
		   ewin->border->border.right;
		if (ww < 1)
		   ww = 1;
		hh = ewin->client.h;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ww, hh);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0, 0, ewin->client.win,
				      ShapeBounding, ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->shaded = 2;
	ewin->w = b;
	ewin->x = d;
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 2:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	a = ewin->h;
	MinShadeSize(ewin, &b, &d);
	b = d;
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		ewin->h = i;
		if (ewin->h < 1)
		   ewin->h = 1;
		hh = ewin->h - ewin->border->border.top -
		   ewin->border->border.bottom;
		if (hh < 1)
		   hh = 1;
		ww = ewin->client.w;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ww, hh);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0,
				      -(ewin->client.h - hh),
				      ewin->client.win, ShapeBounding,
				      ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->shaded = 2;
	ewin->h = b;
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     case 3:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	MinShadeSize(ewin, &b, &d);
	a = ewin->h;
	b = d;
	c = ewin->y;
	d = ewin->y + ewin->h - b;
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		j = ((c * (1024 - k)) + (d * k)) >> 10;
		ewin->h = i;
		ewin->y = j;
		if (ewin->h < 1)
		   ewin->h = 1;
		hh = ewin->h - ewin->border->border.top -
		   ewin->border->border.bottom;
		if (hh < 1)
		   hh = 1;
		ww = ewin->client.w;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ww, hh);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0, 0, ewin->client.win,
				      ShapeBounding, ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->shaded = 2;
	ewin->h = b;
	ewin->y = d;
	EMoveResizeWindow(disp, ewin->win_container, -30, -30, 1, 1);
	EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w, ewin->h);
	CalcEwinSizes(ewin);
	XSync(disp, False);
	break;
     default:
	break;
     }
   UngrabX();
   if (ewin->client.shaped)
      EShapeCombineShape(disp, ewin->win_container, ShapeBounding, 0, 0,
			 ewin->client.win, ShapeBounding, ShapeSet);
   PropagateShapes(ewin->win);
   Mode.queue_up = pq;
   HintsSetWindowState(ewin);
   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }
   EDBUG_RETURN_;
}

void
EwinUnShade(EWin * ewin)
{
   XSetWindowAttributes att;
   int                 i, j, speed, a, b, c, d;
   int                 k, spd, min;
   struct timeval      timev1, timev2;
   int                 dsec, dusec;
   double              tm;
   char                pq;

   EDBUG(4, "EwinUnShade");
   if (GetZoomEWin() == ewin)
      EDBUG_RETURN_;
   if (!ewin->shaded)
      EDBUG_RETURN_;
   pq = Mode.queue_up;
   Mode.queue_up = 0;
   speed = Conf.shadespeed;
   spd = 32;
   min = 2;
   GrabX();
   switch (ewin->border->shadedir)
     {
     case 0:
	att.win_gravity = EastGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	a = ewin->border->border.left;
	b = ewin->client.w + ewin->border->border.left +
	   ewin->border->border.right;
	ewin->shaded = 0;
	EMoveResizeWindow(disp, ewin->win_container,
			  ewin->border->border.left, ewin->border->border.top,
			  1, ewin->client.h);
	EMoveResizeWindow(disp, ewin->client.win, -ewin->client.w, 0,
			  ewin->client.w, ewin->client.h);
	EMapWindow(disp, ewin->client.win);
	EMapWindow(disp, ewin->win_container);
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		ewin->w = i;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top,
				  ewin->w - ewin->border->border.left -
				  ewin->border->border.right, ewin->client.h);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding,
				      -(ewin->client.w -
					(ewin->w - ewin->border->border.left -
					 ewin->border->border.right)), 0,
				      ewin->client.win, ShapeBounding,
				      ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->w = b;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 1:
	att.win_gravity = WestGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	a = ewin->border->border.right;
	b = ewin->client.w + ewin->border->border.left +
	   ewin->border->border.right;
	c = ewin->x;
	d = ewin->x + ewin->w - (ewin->border->border.right + ewin->client.w +
				 ewin->border->border.left);
	ewin->shaded = 0;
	EMoveResizeWindow(disp, ewin->win_container,
			  ewin->border->border.left, ewin->border->border.top,
			  1, ewin->client.h);
	EMoveResizeWindow(disp, ewin->client.win, 0, 0, ewin->client.w,
			  ewin->client.h);
	EMapWindow(disp, ewin->client.win);
	EMapWindow(disp, ewin->win_container);
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		j = ((c * (1024 - k)) + (d * k)) >> 10;
		ewin->w = i;
		ewin->x = j;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top,
				  ewin->w - ewin->border->border.left -
				  ewin->border->border.right, ewin->client.h);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0, 0, ewin->client.win,
				      ShapeBounding, ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->w = b;
	ewin->x = d;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 2:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	a = ewin->border->border.top;
	b = ewin->client.h + ewin->border->border.top +
	   ewin->border->border.bottom;
	ewin->shaded = 0;
	EMoveResizeWindow(disp, ewin->win_container,
			  ewin->border->border.left, ewin->border->border.top,
			  ewin->client.w, 1);
	EMoveResizeWindow(disp, ewin->client.win, 0, -ewin->client.h,
			  ewin->client.w, ewin->client.h);
	EMapWindow(disp, ewin->client.win);
	EMapWindow(disp, ewin->win_container);
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		ewin->h = i;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ewin->client.w,
				  ewin->h - ewin->border->border.top -
				  ewin->border->border.bottom);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0,
				      -(ewin->client.h -
					(ewin->h - ewin->border->border.top -
					 ewin->border->border.bottom)),
				      ewin->client.win, ShapeBounding,
				      ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->h = b;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     case 3:
	att.win_gravity = SouthGravity;
	XChangeWindowAttributes(disp, ewin->client.win, CWWinGravity, &att);
	a = ewin->border->border.bottom;
	b = ewin->client.h + ewin->border->border.top +
	   ewin->border->border.bottom;
	c = ewin->y;
	d = ewin->y + ewin->h - (ewin->border->border.bottom +
				 ewin->client.h + ewin->border->border.top);
	ewin->shaded = 0;
	EMoveResizeWindow(disp, ewin->win_container,
			  ewin->border->border.left, ewin->border->border.top,
			  ewin->client.w, 1);
	EMoveResizeWindow(disp, ewin->client.win, 0, 0, ewin->client.w,
			  ewin->client.h);
	EMapWindow(disp, ewin->client.win);
	EMapWindow(disp, ewin->win_container);
	if ((Conf.animate_shading) || (ewin->menu))
	   for (k = 0; k <= 1024; k += spd)
	     {
		gettimeofday(&timev1, NULL);
		i = ((a * (1024 - k)) + (b * k)) >> 10;
		j = ((c * (1024 - k)) + (d * k)) >> 10;
		ewin->h = i;
		ewin->y = j;
		EMoveResizeWindow(disp, ewin->win_container,
				  ewin->border->border.left,
				  ewin->border->border.top, ewin->client.w,
				  ewin->h - ewin->border->border.top -
				  ewin->border->border.bottom);
		EMoveResizeWindow(disp, ewin->win, ewin->x, ewin->y, ewin->w,
				  ewin->h);
		CalcEwinSizes(ewin);
		if (ewin->client.shaped)
		   EShapeCombineShape(disp, ewin->win_container,
				      ShapeBounding, 0, 0, ewin->client.win,
				      ShapeBounding, ShapeSet);
		PropagateShapes(ewin->win);
		gettimeofday(&timev2, NULL);
		dsec = timev2.tv_sec - timev1.tv_sec;
		dusec = timev2.tv_usec - timev1.tv_usec;
		if (dusec < 0)
		  {
		     dsec--;
		     dusec += 1000000;
		  }
		tm = (double)dsec + (((double)dusec) / 1000000);
		spd = (int)((double)speed * tm);
		if (spd < min)
		   spd = min;
	     }
	ewin->h = b;
	ewin->y = d;
	MoveResizeEwin(ewin, ewin->x, ewin->y, ewin->client.w, ewin->client.h);
	XSync(disp, False);
	break;
     default:
	break;
     }
   UngrabX();
   if (ewin->client.shaped)
      EShapeCombineShape(disp, ewin->win_container, ShapeBounding, 0, 0,
			 ewin->client.win, ShapeBounding, ShapeSet);
   PropagateShapes(ewin->win);
   Mode.queue_up = pq;
   HintsSetWindowState(ewin);
   if (Mode.mode == MODE_NONE)
     {
	PagerEwinOutsideAreaUpdate(ewin);
	ForceUpdatePagersForDesktop(ewin->desktop);
     }
   EDBUG_RETURN_;
}

void
EwinSetFullscreen(EWin * ewin, int on)
{
   int                 x, y, w, h;
   Border             *b;

   if (ewin->st.fullscreen == on)
      return;

   if (on)
     {
	ewin->lx = ewin->x;
	ewin->ly = ewin->y;
	ewin->lw = ewin->client.w;
	ewin->lh = ewin->client.h;
	ewin->ll = ewin->layer;
	ScreenGetGeometry(ewin->x, ewin->y, &x, &y, &w, &h);
#if 0
	ewin->layer = 10;
#endif
	ewin->fixedpos = 1;
	ewin->st.fullscreen = 1;
        b = (Border *) FindItem("BORDERLESS", 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
     }
   else
     {
	x = ewin->lx;
	y = ewin->ly;
	w = ewin->lw;
	h = ewin->lh;
	ewin->layer = ewin->ll;
	ewin->fixedpos = 0;	/* Yeah - well */
	ewin->st.fullscreen = 0;
	b = ewin->normal_border;
     }
   RaiseEwin(ewin);
   MoveResizeEwin(ewin, x, y, w, h);
   HintsSetWindowState(ewin);
   EwinSetBorder(ewin, b, 1);
}

void
MoveEwinToArea(EWin * ewin, int ax, int ay)
{
   EDBUG(4, "MoveEwinToArea");
   AreaFix(&ax, &ay);
   MoveEwin(ewin, ewin->x + (VRoot.w * (ax - ewin->area_x)),
	    ewin->y + (VRoot.h * (ay - ewin->area_y)));
   EDBUG_RETURN_;
}

int
EwinGetDesk(const EWin * ewin)
{
   return (ewin->sticky) ? desks.current : ewin->desktop;
}

const char         *
EwinGetTitle(const EWin * ewin)
{
   const char         *name;

#if ENABLE_EWMH
   name = ewin->ewmh.wm_name;
   if (name)
      goto done;
#endif
   name = ewin->icccm.wm_name;
   if (name)
      goto done;

 done:
   return (name && strlen(name)) ? name : NULL;
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

   return EwinGetTitle(ewin);

 done:
   return (name && strlen(name)) ? name : NULL;
}

int
EwinIsOnScreen(EWin * ewin)
{
   int                 x, y, w, h;

   if (ewin->sticky)
      return 1;
   if (ewin->desktop != desks.current)
      return 0;

   x = ewin->x;
   y = ewin->y;
   w = ewin->w;
   h = ewin->h;

   if (x + w <= 0 || x >= VRoot.w || y + h <= 0 || y >= VRoot.h)
      return 0;

   return 1;
}

int
BorderWinpartIndex(EWin * ewin, Window win)
{
   int                 i;

   for (i = 0; i < ewin->border->num_winparts; i++)
     {
	if (win == ewin->bits[i].win)
	   return i;
     }

   return -1;			/* Not found */
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
	CalcEwinSizes(ewin);
     }

   if (EWinChanges.flags & EWIN_CHANGE_DESKTOP)
     {
	int                 desk = ewin->desktop;

	if (desk != EWinChanges.ewin_old.desktop && !ewin->sticky)
	  {
	     ewin->desktop = EWinChanges.ewin_old.desktop;
	     MoveEwinToDesktop(ewin, desk);
	  }
     }

   if (EWinChanges.flags & EWIN_CHANGE_ICON_PMAP)
     {
	if (ewin->iconified)
	   IconboxesUpdateEwinIcon(ewin, 1);
     }

   EWinChanges.flags = 0;
}

static void
EwinEventsConfigure(EWin * ewin, int mode)
{
   int                 i;
   long                emask;

   if (mode)
     {
	emask = ~(EnterWindowMask | LeaveWindowMask);

	XSelectInput(disp, ewin->win, EWIN_TOP_EVENT_MASK & emask);
	XSelectInput(disp, ewin->client.win, ewin->client.event_mask & emask);

	for (i = 0; i < ewin->border->num_winparts; i++)
	  {
	     if (ewin->border->part[i].flags & FLAG_TITLE)
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_TITLE_EVENT_MASK & emask);
	     else
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_PART_EVENT_MASK & emask);
	  }
     }
   else
     {
	XSelectInput(disp, ewin->win, EWIN_TOP_EVENT_MASK);
	XSelectInput(disp, ewin->client.win, ewin->client.event_mask);

	for (i = 0; i < ewin->border->num_winparts; i++)
	  {
	     if (ewin->border->part[i].flags & FLAG_TITLE)
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_TITLE_EVENT_MASK);
	     else
		XSelectInput(disp, ewin->bits[i].win,
			     EWIN_BORDER_PART_EVENT_MASK);
	  }
     }
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
	if (mode == 0)
	   if (Mode.mode == MODE_DESKSWITCH && ewin->sticky && ewin->visible)
	      EwinRefresh(ewin);
     }
}

void
EwinsSetFree(void)
{
   int                 i, num;
   EWin               *const *lst, *ewin;

   if (EventDebug(EDBUG_TYPE_SESSION))
      Eprintf("EwinsSetFree\n");

   lst = EwinListGetStacking(&num);
   for (i = num - 1; i >= 0; i--)
     {
	ewin = lst[i];
	if (ewin->internal)
	   continue;

	/* This makes E determine the client window stacking at exit */
	EReparentWindow(disp, ewin->client.win, VRoot.win,
			ewin->client.x, ewin->client.y);
     }
}

/*
 * Border event handlers
 */
typedef void        (border_event_func_t) (XEvent * ev, EWin * ewin, int part);

static void
BorderWinpartEventExpose(XEvent * ev, EWin * ewin, int j)
{
   ewin->bits[j].no_expose = 0;
   ewin->bits[j].expose = 1;
   if (BorderWinpartDraw(ewin, j) && IsPropagateEwinOnQueue(ewin))
      PropagateShapes(ewin->win);
   return;
   ev = NULL;
}

static void
BorderWinpartEventMouseDown(XEvent * ev, EWin * ewin, int j)
{
   GrabThePointer(ewin->bits[j].win);

   ewin->bits[j].state = STATE_CLICKED;
   BorderWinpartChange(ewin, j, 0);

   if (ewin->border->part[j].aclass)
      EventAclass(ev, ewin, ewin->border->part[j].aclass);
}

static void
BorderWinpartEventMouseUp(XEvent * ev, EWin * ewin, int j)
{
   if ((ewin->bits[j].state == STATE_CLICKED) && (!ewin->bits[j].left))
      ewin->bits[j].state = STATE_HILITED;
   else
      ewin->bits[j].state = STATE_NORMAL;
   ewin->bits[j].left = 0;
   BorderWinpartChange(ewin, j, 0);

   if (ewin->bits[j].win == Mode.context_win && ewin->border->part[j].aclass)
      EventAclass(ev, ewin, ewin->border->part[j].aclass);
}

static void
BorderWinpartEventEnter(XEvent * ev, EWin * ewin, int j)
{
   if (ewin->bits[j].state == STATE_CLICKED)
      ewin->bits[j].left = 0;
   else
     {
	ewin->bits[j].state = STATE_HILITED;
	BorderWinpartChange(ewin, j, 0);
	if (ewin->border->part[j].aclass)
	   EventAclass(ev, ewin, ewin->border->part[j].aclass);
     }
}

static void
BorderWinpartEventLeave(XEvent * ev, EWin * ewin, int j)
{
   if (ewin->bits[j].state == STATE_CLICKED)
      ewin->bits[j].left = 1;
   else
     {
	ewin->bits[j].state = STATE_NORMAL;
	BorderWinpartChange(ewin, j, 0);
	if (ewin->border->part[j].aclass)
	   EventAclass(ev, ewin, ewin->border->part[j].aclass);
     }
}

static void
BorderWinpartEventLeave2(XEvent * ev, EWin * ewin, int j)
{
   ewin->bits[j].left = 0;
   ewin->bits[j].state = STATE_NORMAL;
   BorderWinpartChange(ewin, j, 0);
   return;
   ev = NULL;
}

static int
BordersEvent(XEvent * ev, border_event_func_t * func)
{
   Window              win = ev->xany.window;
   EWin               *const *ewins;
   int                 i, j, num, used = 0;

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < ewins[i]->border->num_winparts; j++)
	  {
	     if (win == ewins[i]->bits[j].win)
	       {
		  func(ev, ewins[i], j);

		  used = 1;
		  goto done;
	       }
	  }
     }

 done:

   return used;
}

int
BordersEventExpose(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventExpose);
}

int
BordersEventMouseDown(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventMouseDown);
}

int
BordersEventMouseUp(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventMouseUp);
}

int
BordersEventMouseIn(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventEnter);
}

int
BordersEventMouseOut(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventLeave);
}

int
BordersEventMouseOut2(XEvent * ev)
{
   return BordersEvent(ev, BorderWinpartEventLeave2);
}
