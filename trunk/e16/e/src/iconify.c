/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2005 Kim Woelders
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
#include "desktops.h"
#include "ecore-e16.h"
#include "emodule.h"
#include "eobj.h"
#include "ewins.h"
#include "iclass.h"
#include "icons.h"
#include "menus.h"
#include "tooltips.h"
#include "xwin.h"
#include <math.h>

typedef struct _iconbox Iconbox;

static void         IconboxesConfigSave(void);
static Iconbox     *SelectIconboxForEwin(EWin * ewin);
static void         IconboxObjEwinDel(Iconbox * ib, EWin * ewin);

/* Systray stuff */
typedef struct
{
   Window              win;
   char                mapped;
} SWin;

static void         SystrayInit(Iconbox * ib, Window win, int screen);
static void         SystrayExit(Iconbox * ib);
static void         IconboxObjSwinFree(Iconbox * ib, SWin * swin);

typedef struct
{
   union
   {
      void               *obj;
      EWin               *ewin;
      SWin               *swin;
   } u;
   int                 xo, yo, wo, ho;	/* Outer */
   int                 xi, yi, wi, hi;	/* Inner */
   Imlib_Image        *im;
} IboxOject;

struct _iconbox
{
   /* user settings */
   char               *name;
   char                type;
   char                orientation;
   char                animate;
   char                scrollbar_side;
   char                arrow_side;
   char                shownames;
   char                nobg;
   int                 iconsize;
   int                 icon_mode;

   char                auto_resize;
   char                draw_icon_base;
   char                scrollbar_hide;
   char                cover_hide;
   int                 auto_resize_anchor;

   /* internally set stuff */
   EWin               *ewin;
   int                 w, h;
   int                 pos;
   int                 max, max_min;
   ImageClass         *ic_box;
   ImageClass         *ic_item_base;
   Imlib_Image        *im_item_base;

   char                arrow1_hilited;
   char                arrow1_clicked;
   char                arrow2_hilited;
   char                arrow2_clicked;
   char                icon_clicked;
   char                scrollbar_hilited;
   char                scrollbar_clicked;
   char                scrollbox_clicked;

   Window              win;
   Window              cover_win;
   Window              icon_win;
   Window              scroll_win;
   Window              arrow1_win;
   Window              arrow2_win;
   Window              scrollbar_win;
   Window              scrollbarknob_win;

   int                 num_objs;
   IboxOject          *objs;

   /* these are theme-settable parameters */
   int                 scroll_thickness;
   int                 arrow_thickness;
   int                 bar_thickness;
   int                 knob_length;

   /* State flags */
   char                do_update;
};

/* Silly hack to avoid name clash warning when using -Wshadow */
#define y1 y1_

static void         IconboxLayout(Iconbox * ib, int *px, int *py, int *pw,
				  int *ph);
static void         IconboxDraw(Iconbox * ib);
static void         IconboxRedraw(Iconbox * ib);
static void         IboxEventScrollWin(XEvent * ev, void *prm);
static void         IboxEventScrollbarWin(XEvent * ev, void *prm);
static void         IboxEventCoverWin(XEvent * ev, void *prm);
static void         IboxEventArrow1Win(XEvent * ev, void *prm);
static void         IboxEventArrow2Win(XEvent * ev, void *prm);
static void         IboxEventIconWin(XEvent * ev, void *prm);

#define IB_ANIM_TIME 0.25

#define IB_TYPE_ICONBOX     0
#define IB_TYPE_SYSTRAY     1

static void
IB_Animate(char iconify, EWin * from, EWin * to)
{
   double              t1, t2, t, i, spd, ii;
   int                 x, y, x1, y1, x2, y2, x3, y3, x4, y4, w, h, fx, fy, fw,
      fh, dx, dy, dw, dh;
   Window              root = VRoot.win;
   GC                  gc;
   XGCValues           gcv;
   Desk               *dskf, *dskt;

   if (Mode.wm.startup)
      return;

   dskf = EoGetDesk(from);
   dskt = EoGetDesk(to);

   EobjsRepaint();
   EGrabServer();

   spd = 0.00001;
   gcv.subwindow_mode = IncludeInferiors;
   gcv.function = GXxor;
   gcv.fill_style = FillOpaqueStippled;
   gcv.foreground = WhitePixel(disp, VRoot.scr);
   if (gcv.foreground == 0)
      gcv.foreground = BlackPixel(disp, VRoot.scr);
   gc = ECreateGC(root,
		  GCFunction | GCForeground | GCSubwindowMode | GCFillStyle,
		  &gcv);
   t1 = GetTime();
   if (iconify)
     {
	fw = EoGetW(from) + 4;
	fh = EoGetH(from) + 4;
	fx = EoGetX(from) + EoGetX(dskf) - 2;
	fy = EoGetY(from) + EoGetY(dskf) - 2;
	dw = 4;
	dh = 4;
	dx = EoGetX(to) + EoGetX(dskt) + (EoGetW(to) / 2) - 2;
	dy = EoGetY(to) + EoGetY(dskt) + (EoGetH(to) / 2) - 2;
	for (i = 0.0; i < 1.0; i += spd)
	  {
	     ii = 1.0 - i;

	     x = (fx * ii) + (dx * i);
	     y = (fy * ii) + (dy * i);
	     w = (fw * ii) + (dw * i);
	     h = (fh * ii) + (dh * i);

	     x = (2 * x + w) / 2;	/* x middle */
	     y = (2 * y + h) / 2;	/* y middle */
	     w /= 2;		/* width/2 */
	     h /= 2;		/* height/2 */

	     x1 = x + w * (1 - .5 * sin(3.14159 + i * 6.2831853072));
	     y1 = y + h * cos(i * 6.2831853072);
	     x2 = x + w * (1 - .5 * sin(i * 6.2831853072));
	     y2 = y - h * cos(i * 6.2831853072);
	     x3 = x - w * (1 - .5 * sin(3.14159 + i * 6.2831853072));
	     y3 = y - h * cos(i * 6.2831853072);
	     x4 = x - w * (1 - .5 * sin(i * 6.2831853072));
	     y4 = y + h * cos(i * 6.2831853072);

	     XDrawLine(disp, root, gc, x1, y1, x2, y2);
	     XDrawLine(disp, root, gc, x2, y2, x3, y3);
	     XDrawLine(disp, root, gc, x3, y3, x4, y4);
	     XDrawLine(disp, root, gc, x4, y4, x1, y1);

	     XDrawLine(disp, root, gc, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	     XDrawLine(disp, root, gc, x2 + 1, y2 + 1, x3 - 1, y3 - 1);
	     XDrawLine(disp, root, gc, x3 + 1, y3 + 1, x4 - 1, y4 - 1);
	     XDrawLine(disp, root, gc, x4 + 1, y4 + 1, x1 - 1, y1 - 1);

	     XDrawLine(disp, root, gc, x1 + 2, y1 + 2, x2 - 2, y2 - 2);
	     XDrawLine(disp, root, gc, x2 + 2, y2 + 2, x3 - 2, y3 - 2);
	     XDrawLine(disp, root, gc, x3 + 2, y3 + 2, x4 - 2, y4 - 2);
	     XDrawLine(disp, root, gc, x4 + 2, y4 + 2, x1 - 2, y1 - 2);

	     ESync();
	     t2 = GetTime();
	     t = t2 - t1;
	     t1 = t2;
	     spd = t / IB_ANIM_TIME;

	     XDrawLine(disp, root, gc, x1, y1, x2, y2);
	     XDrawLine(disp, root, gc, x2, y2, x3, y3);
	     XDrawLine(disp, root, gc, x3, y3, x4, y4);
	     XDrawLine(disp, root, gc, x4, y4, x1, y1);

	     XDrawLine(disp, root, gc, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	     XDrawLine(disp, root, gc, x2 + 1, y2 + 1, x3 - 1, y3 - 1);
	     XDrawLine(disp, root, gc, x3 + 1, y3 + 1, x4 - 1, y4 - 1);
	     XDrawLine(disp, root, gc, x4 + 1, y4 + 1, x1 - 1, y1 - 1);

	     XDrawLine(disp, root, gc, x1 + 2, y1 + 2, x2 - 2, y2 - 2);
	     XDrawLine(disp, root, gc, x2 + 2, y2 + 2, x3 - 2, y3 - 2);
	     XDrawLine(disp, root, gc, x3 + 2, y3 + 2, x4 - 2, y4 - 2);
	     XDrawLine(disp, root, gc, x4 + 2, y4 + 2, x1 - 2, y1 - 2);
	  }
     }
   else
     {
	fw = EoGetW(from) + 4;
	fh = EoGetH(from) + 4;
	fx = EoGetX(from) + EoGetX(dskf) - 2;
	fy = EoGetY(from) + EoGetY(dskf) - 2;
	dw = 4;
	dh = 4;
	dx = EoGetX(to) + EoGetX(dskt) + (EoGetW(to) / 2) - 2;
	dy = EoGetY(to) + EoGetY(dskt) + (EoGetH(to) / 2) - 2;
	for (i = 1.0; i >= 0.0; i -= spd)
	  {
	     ii = 1.0 - i;

	     x = (fx * ii) + (dx * i);
	     y = (fy * ii) + (dy * i);
	     w = (fw * ii) + (dw * i);
	     h = (fh * ii) + (dh * i);

	     x = (2 * x + w) / 2;	/* x middle */
	     y = (2 * y + h) / 2;	/* y middle */
	     w /= 2;		/* width/2 */
	     h /= 2;		/* height/2 */

	     x1 = x + w * (1 - .5 * sin(3.14159 + i * 6.2831853072));
	     y1 = y + h * cos(i * 6.2831853072);
	     x2 = x + w * (1 - .5 * sin(i * 6.2831853072));
	     y2 = y - h * cos(i * 6.2831853072);
	     x3 = x - w * (1 - .5 * sin(3.14159 + i * 6.2831853072));
	     y3 = y - h * cos(i * 6.2831853072);
	     x4 = x - w * (1 - .5 * sin(i * 6.2831853072));
	     y4 = y + h * cos(i * 6.2831853072);

	     XDrawLine(disp, root, gc, x1, y1, x2, y2);
	     XDrawLine(disp, root, gc, x2, y2, x3, y3);
	     XDrawLine(disp, root, gc, x3, y3, x4, y4);
	     XDrawLine(disp, root, gc, x4, y4, x1, y1);

	     XDrawLine(disp, root, gc, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	     XDrawLine(disp, root, gc, x2 + 1, y2 + 1, x3 - 1, y3 - 1);
	     XDrawLine(disp, root, gc, x3 + 1, y3 + 1, x4 - 1, y4 - 1);
	     XDrawLine(disp, root, gc, x4 + 1, y4 + 1, x1 - 1, y1 - 1);

	     XDrawLine(disp, root, gc, x1 + 2, y1 + 2, x2 - 2, y2 - 2);
	     XDrawLine(disp, root, gc, x2 + 2, y2 + 2, x3 - 2, y3 - 2);
	     XDrawLine(disp, root, gc, x3 + 2, y3 + 2, x4 - 2, y4 - 2);
	     XDrawLine(disp, root, gc, x4 + 2, y4 + 2, x1 - 2, y1 - 2);

	     ESync();
	     t2 = GetTime();
	     t = t2 - t1;
	     t1 = t2;
	     spd = t / IB_ANIM_TIME;

	     XDrawLine(disp, root, gc, x1, y1, x2, y2);
	     XDrawLine(disp, root, gc, x2, y2, x3, y3);
	     XDrawLine(disp, root, gc, x3, y3, x4, y4);
	     XDrawLine(disp, root, gc, x4, y4, x1, y1);

	     XDrawLine(disp, root, gc, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	     XDrawLine(disp, root, gc, x2 + 1, y2 + 1, x3 - 1, y3 - 1);
	     XDrawLine(disp, root, gc, x3 + 1, y3 + 1, x4 - 1, y4 - 1);
	     XDrawLine(disp, root, gc, x4 + 1, y4 + 1, x1 - 1, y1 - 1);

	     XDrawLine(disp, root, gc, x1 + 2, y1 + 2, x2 - 2, y2 - 2);
	     XDrawLine(disp, root, gc, x2 + 2, y2 + 2, x3 - 2, y3 - 2);
	     XDrawLine(disp, root, gc, x3 + 2, y3 + 2, x4 - 2, y4 - 2);
	     XDrawLine(disp, root, gc, x4 + 2, y4 + 2, x1 - 2, y1 - 2);
	  }
     }
   EFreeGC(gc);
   EUngrabServer();
}

static Iconbox     *
IconboxCreate(const char *name)
{
   Iconbox            *ib;

   ib = Ecalloc(1, sizeof(Iconbox));
   ib->name = Estrdup(name);
   ib->type = (!strncmp(name, "_ST_", 4)) ? IB_TYPE_SYSTRAY : IB_TYPE_ICONBOX;
   ib->orientation = 0;
   ib->scrollbar_side = 1;
   ib->arrow_side = 1;
   ib->nobg = 0;
   ib->shownames = 1;
   ib->iconsize = (ib->type == IB_TYPE_ICONBOX) ? 48 : 24;
   ib->icon_mode = 2;
   ib->auto_resize = 0;
   ib->draw_icon_base = 0;
   ib->scrollbar_hide = 0;
   ib->cover_hide = 0;
   ib->auto_resize_anchor = 0;
   /* FIXME: need to have theme settable params for this and get them */
   ib->scroll_thickness = 12;
   ib->arrow_thickness = 12;
   ib->bar_thickness = 8;
   ib->knob_length = 8;
   ib->animate = 1;

   ib->w = 0;
   ib->h = 0;
   ib->pos = 0;
   ib->max = 1;
   ib->arrow1_hilited = 0;
   ib->arrow1_clicked = 0;
   ib->arrow2_hilited = 0;
   ib->arrow2_clicked = 0;
   ib->icon_clicked = 0;
   ib->scrollbar_hilited = 0;
   ib->scrollbar_clicked = 0;
   ib->scrollbox_clicked = 0;

   ib->win = ECreateWindow(VRoot.win, 0, 0, 1, 1, 0);
   ib->icon_win = ECreateWindow(ib->win, 0, 0, 128, 26, 0);
   EventCallbackRegister(ib->icon_win, 0, IboxEventIconWin, ib);
   ib->cover_win = ECreateWindow(ib->win, 0, 0, 128, 26, 0);
   EventCallbackRegister(ib->cover_win, 0, IboxEventCoverWin, ib);
   ib->scroll_win = ECreateWindow(ib->win, 6, 26, 116, 6, 0);
   EventCallbackRegister(ib->scroll_win, 0, IboxEventScrollWin, ib);
   ib->arrow1_win = ECreateWindow(ib->win, 0, 26, 6, 6, 0);
   EventCallbackRegister(ib->arrow1_win, 0, IboxEventArrow1Win, ib);
   ib->arrow2_win = ECreateWindow(ib->win, 122, 26, 6, 6, 0);
   EventCallbackRegister(ib->arrow2_win, 0, IboxEventArrow2Win, ib);
   ib->scrollbar_win = ECreateWindow(ib->scroll_win, 122, 26, 6, 6, 0);
   EventCallbackRegister(ib->scrollbar_win, 0, IboxEventScrollbarWin, ib);
   ib->scrollbarknob_win = ECreateWindow(ib->scrollbar_win, -20, -20, 4, 4, 0);

   ESelectInput(ib->icon_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask | PointerMotionMask);
   ESelectInput(ib->scroll_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask);
   ESelectInput(ib->cover_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask);
   ESelectInput(ib->arrow1_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask);
   ESelectInput(ib->arrow2_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask);
   ESelectInput(ib->scrollbar_win,
		EnterWindowMask | LeaveWindowMask | ButtonPressMask |
		ButtonReleaseMask | PointerMotionMask);

   EMapWindow(ib->icon_win);
   EMapWindow(ib->scrollbar_win);

   ib->ewin = NULL;
   ib->num_objs = 0;
   ib->objs = NULL;

   AddItem(ib, ib->name, 0, LIST_TYPE_ICONBOX);

   if (ib->type == IB_TYPE_SYSTRAY)
      SystrayInit(ib, ib->icon_win, VRoot.scr);

   return ib;
}

static void
IconboxDestroy(Iconbox * ib, int exiting)
{
   int                 i;

   RemoveItemByPtr(ib, LIST_TYPE_ICONBOX);

   if (ib->name)
      Efree(ib->name);

   switch (ib->type)
     {
     case IB_TYPE_ICONBOX:
	break;
     case IB_TYPE_SYSTRAY:
	SystrayExit(ib);
	break;
     }

   for (i = 0; i < ib->num_objs; i++)
     {
	switch (ib->type)
	  {
	  case IB_TYPE_ICONBOX:
	     if (!exiting)
		EwinDeIconify(ib->objs[i].u.ewin);
	     IconboxObjEwinDel(ib, ib->objs[i].u.ewin);
	     break;
	  case IB_TYPE_SYSTRAY:
	     IconboxObjSwinFree(ib, ib->objs[i].u.swin);
	     break;
	  }
     }

   EDestroyWindow(ib->win);

   if (ib->objs)
      Efree(ib->objs);

   Efree(ib);

   if (!exiting)
      IconboxesConfigSave();
}

static void
IconboxReconfigure(Iconbox * ib)
{
   ImageClass         *ic, *ic2;
   Imlib_Border       *pad;
   EWin               *ewin;
   int                 extra;

   ewin = ib->ewin;

   ewin->client.width.min = 8;
   ewin->client.height.min = 8;
   ewin->client.width.max = 16384;
   ewin->client.height.max = 16384;
   ewin->client.no_resize_h = 0;
   ewin->client.no_resize_v = 0;

   extra = 0;
   if (ib->orientation)
     {
	ic = ImageclassFind("ICONBOX_VERTICAL", 0);
	pad = ImageclassGetPadding(ic);
	if (ic)
	   extra = pad->left + pad->right;
	if (ib->draw_icon_base)
	  {
	     ic2 = ImageclassFind("DEFAULT_ICON_BUTTON", 0);
	     pad = ImageclassGetPadding(ic2);
	     if (ic2)
		extra += pad->left + pad->right;
	  }
	ewin->client.width.max = ewin->client.width.min =
	   ib->iconsize + ib->scroll_thickness + extra;
	ewin->client.no_resize_h = 1;
	ib->max_min = ewin->client.height.min;
     }
   else
     {
	ic = ImageclassFind("ICONBOX_HORIZONTAL", 0);
	pad = ImageclassGetPadding(ic);
	if (ic)
	   extra = pad->top + pad->bottom;
	if (ib->draw_icon_base)
	  {
	     ic2 = ImageclassFind("DEFAULT_ICON_BUTTON", 0);
	     pad = ImageclassGetPadding(ic2);
	     if (ic2)
		extra += pad->top + pad->bottom;
	  }
	ewin->client.height.max = ewin->client.height.min =
	   ib->iconsize + ib->scroll_thickness + extra;
	ewin->client.no_resize_v = 1;
	ib->max_min = ewin->client.width.min;
     }
}

static void
IconboxEwinLayout(EWin * ewin, int *px, int *py, int *pw, int *ph)
{
   Iconbox            *ib = ewin->data;

   IconboxLayout(ib, px, py, pw, ph);

   if (*pw != ib->w || *ph != ib->h)
      ib->do_update = 1;
}

static void
IconboxEwinMoveResize(EWin * ewin, int resize)
{
   Iconbox            *ib = ewin->data;

   if (!resize && !ib->do_update && !TransparencyUpdateNeeded())
      return;

   ib->w = ewin->client.w;
   ib->h = ewin->client.h;

   IconboxDraw(ib);
   ib->do_update = 0;
}

static void
IconboxEwinClose(EWin * ewin)
{
   IconboxDestroy(ewin->data, 0);
   ewin->data = NULL;
}

static void
IconboxEwinInit(EWin * ewin, void *ptr)
{
   ewin->data = (Iconbox *) ptr;

   ewin->Layout = IconboxEwinLayout;
   ewin->MoveResize = IconboxEwinMoveResize;
   ewin->Close = IconboxEwinClose;

   ewin->props.skip_ext_task = 1;
   ewin->props.skip_ext_pager = 1;
   ewin->props.skip_focuslist = 1;
   ewin->props.skip_winlist = 1;
   ewin->props.never_focus = 1;
   ewin->props.never_iconify = 1;
   ewin->props.autosave = 1;

   EoSetSticky(ewin, 1);
}

static void
IconboxShow(Iconbox * ib)
{
   EWin               *ewin;

   if (ib->type == IB_TYPE_ICONBOX)
     {
	HintsSetWindowName(ib->win, "Iconbox");
	HintsSetWindowClass(ib->win, ib->name, "Enlightenment_IconBox");
     }
   else
     {
	HintsSetWindowName(ib->win, "Systray");
	HintsSetWindowClass(ib->win, ib->name, "Enlightenment_IconBox");
     }

   ewin = AddInternalToFamily(ib->win, "ICONBOX", EWIN_TYPE_ICONBOX, ib,
			      IconboxEwinInit);
   if (!ewin)
      return;

   ib->ewin = ewin;

   IconboxReconfigure(ib);

   if (ewin->state.placed)
     {
	EwinMoveToDesktop(ewin, EoGetDesk(ewin));
	EwinMoveResize(ewin, EoGetX(ewin), EoGetY(ewin), ewin->client.w,
		       ewin->client.h);
     }
   else
     {
	/* The first one */
	EwinStick(ewin);
	EwinMoveToDesktop(ewin, EoGetDesk(ewin));
	EwinResize(ewin, 128, 32);
	EwinMove(ewin, VRoot.w - EoGetW(ewin), VRoot.h - EoGetH(ewin));
     }

   ShowEwin(ewin);
}

/*
 * Return index, -1 if not found.
 */
static int
IconboxObjectFind(Iconbox * ib, void *obj)
{
   int                 i;

   for (i = 0; i < ib->num_objs; i++)
      if (ib->objs[i].u.obj == obj)
	 return i;

   return -1;
}

static int
IconboxObjectAdd(Iconbox * ib, void *obj)
{
   /* Not if already there */
   if (IconboxObjectFind(ib, obj) >= 0)
      return -1;

   ib->num_objs++;
   ib->objs = Erealloc(ib->objs, sizeof(IboxOject) * ib->num_objs);
   ib->objs[ib->num_objs - 1].u.obj = obj;

   return ib->num_objs - 1;	/* Success */
}

static int
IconboxObjectDel(Iconbox * ib, void *obj)
{
   int                 i, j;

   /* Quit if not there */
   i = IconboxObjectFind(ib, obj);
   if (i < 0)
      return -1;

   for (j = i; j < ib->num_objs - 1; j++)
      ib->objs[j] = ib->objs[j + 1];
   ib->num_objs--;
   if (ib->num_objs > 0)
      ib->objs = Erealloc(ib->objs, sizeof(IboxOject) * ib->num_objs);
   else
     {
	Efree(ib->objs);
	ib->objs = NULL;
     }

   return 0;			/* Success */
}

static int
IconboxObjEwinFind(Iconbox * ib, EWin * ewin)
{
   return IconboxObjectFind(ib, ewin);
}

static void
IconboxObjEwinAdd(Iconbox * ib, EWin * ewin)
{
   int                 i;

   i = IconboxObjectAdd(ib, ewin);
   if (i < 0)
      return;

   ib->objs[i].im = EwinIconImageGet(ewin, ib->iconsize, ib->icon_mode);
   IconboxRedraw(ib);
}

static void
IconboxObjEwinDel(Iconbox * ib, EWin * ewin)
{
   int                 i;

   i = IconboxObjEwinFind(ib, ewin);
   if (i < 0)
      return;

   if (ib->objs[i].im)
     {
	imlib_context_set_image(ib->objs[i].im);
	imlib_free_image();
     }
   IconboxObjectDel(ib, ewin);
}

static void
IconboxesEwinIconify(EWin * ewin)
{
   Iconbox            *ib;

   SoundPlay("SOUND_ICONIFY");

   ib = SelectIconboxForEwin(ewin);
   if (!ib)
      return;

   IconboxObjEwinAdd(ib, ewin);

   if (ib->animate && !ewin->state.showingdesk)
      IB_Animate(1, ewin, ib->ewin);
}

static void
IconboxesEwinDeIconify(EWin * ewin)
{
   Iconbox            *ib;

   SoundPlay("SOUND_DEICONIFY");

   ib = SelectIconboxForEwin(ewin);
   if (!ib)
      return;

   if (ib->animate && !ewin->state.showingdesk)
      IB_Animate(0, ewin, ib->ewin);

   IconboxObjEwinDel(ib, ewin);
   IconboxRedraw(ib);
   EobjsRepaint();
}

static void
RemoveMiniIcon(EWin * ewin)
{
   Iconbox            *ib;

   ib = SelectIconboxForEwin(ewin);
   if (!ib)
      return;

   IconboxObjEwinDel(ib, ewin);
   IconboxRedraw(ib);
}

static Iconbox    **
IconboxesList(int *num)
{
   /* list all currently available Iconboxes */
   return (Iconbox **) ListItemType(num, LIST_TYPE_ICONBOX);
}

static Iconbox     *
SelectIconboxForEwin(EWin * ewin)
{
   /* find the appropriate iconbox from all available ones for this app */
   /* if it is to be iconified, or if it is alreayd return which iconbox */
   /* it's in */
   Iconbox           **lst, *ib, *ib_sel = NULL;
   int                 i, num = 0;

   if (!ewin)
      return NULL;

   lst = IconboxesList(&num);
   if (!lst)
      return NULL;

   if (ewin->state.iconified)
     {
	/* find the iconbox this window got iconifed into */
	for (i = 0; i < num; i++)
	  {
	     ib = lst[i];
	     if (ib->type != IB_TYPE_ICONBOX)
		continue;

	     if (IconboxObjEwinFind(ib, ewin) < 0)
		continue;
	     ib_sel = ib;
	     break;
	  }
     }
   else
     {
	/* pick the closest iconbox physically on screen to put it in */
	int                 min_dist;
	int                 dx, dy, dist;

	min_dist = 0x7fffffff;
	for (i = 0; i < num; i++)
	  {
	     ib = lst[i];
	     if (ib->ewin == NULL || ib->type != IB_TYPE_ICONBOX)
		continue;

	     dx = (EoGetX(ib->ewin) + (EoGetW(ib->ewin) / 2)) -
		(EoGetX(ewin) + (EoGetW(ewin) / 2));
	     dy = (EoGetY(ib->ewin) + (EoGetH(ib->ewin) / 2)) -
		(EoGetY(ewin) + (EoGetH(ewin) / 2));
	     dist = (dx * dx) + (dy * dy);
	     if ((!EoIsSticky(ib->ewin)) &&
		 (EoGetDesk(ib->ewin) != EoGetDesk(ewin)))
		dist += (VRoot.w * VRoot.w) + (VRoot.h * VRoot.h);
	     if (dist < min_dist)
	       {
		  min_dist = dist;
		  ib_sel = ib;
	       }
	  }
     }

   Efree(lst);

   return ib_sel;
}

static void
IconboxUpdateEwinIcon(Iconbox * ib, EWin * ewin, int icon_mode)
{
   int                 i;

   if (ib->icon_mode != icon_mode)
      return;

   i = IconboxObjEwinFind(ib, ewin);
   if (i < 0)
      return;

   if (ib->objs[i].im)
     {
	imlib_context_set_image(ib->objs[i].im);
	imlib_free_image();
     }
   ib->objs[i].im = EwinIconImageGet(ewin, ib->iconsize, icon_mode);

   IconboxRedraw(ib);
}

static void
IconboxesUpdateEwinIcon(EWin * ewin, int icon_mode)
{
   Iconbox           **ib;
   int                 i, num = 0;

   ib = IconboxesList(&num);
   if (!ib)
      return;

   for (i = 0; i < num; i++)
      IconboxUpdateEwinIcon(ib[i], ewin, icon_mode);
   Efree(ib);
}

static void
IconboxFindIconSize(Imlib_Image * im, int *pw, int *ph, int size)
{
   int                 w, h, minsz, maxwh;

   imlib_context_set_image(im);
   w = imlib_image_get_width();
   h = imlib_image_get_height();

   maxwh = (w > h) ? w : h;
   if (maxwh <= 1)
      goto done;

   minsz = (size * 3) / 4;

   if (maxwh < minsz || maxwh > size)
     {
	w = (w * size) / maxwh;
	h = (h * size) / maxwh;
     }

 done:
   *pw = w;
   *ph = h;
}

static void
IconboxLayoutImageWin(Iconbox * ib)
{
   int                 i, xo, yo, wo, ho, wi, hi;
   int                 item_pad, padl, padr, padt, padb;
   IboxOject          *ibo;
   Imlib_Border       *pad;

   if (ib->orientation)
      ib->ic_box = ImageclassFind("ICONBOX_VERTICAL", 0);
   else
      ib->ic_box = ImageclassFind("ICONBOX_HORIZONTAL", 0);

   if (ib->draw_icon_base && !ib->im_item_base)
     {
	ib->ic_item_base = ImageclassFind("DEFAULT_ICON_BUTTON", 0);
	if (ib->ic_item_base)
	   ib->im_item_base =
	      ImageclassGetImage(ib->ic_item_base, 0, 0, STATE_NORMAL);
	if (!ib->im_item_base)
	  {
	     ib->ic_item_base = NULL;
	     ib->draw_icon_base = 0;
	  }
     }

   if (ib->draw_icon_base)
     {
	pad = ImageclassGetPadding(ib->ic_item_base);
	padl = pad->left;
	padr = pad->right;
	padt = pad->top;
	padb = pad->bottom;

	item_pad = 0;
     }
   else
     {
	padl = padr = padt = padb = 0;

	item_pad = 2;
     }

   xo = 0;
   yo = 0;
   if (ib->ic_box)
     {
	pad = ImageclassGetPadding(ib->ic_box);
	xo += pad->left;
	yo += pad->top;
     }

   for (i = 0; i < ib->num_objs; i++)
     {
	ibo = &ib->objs[i];

	/* Inner size */
	if (ib->type == IB_TYPE_ICONBOX)
	  {
	     wi = hi = 8;
	     if (ibo->im)
		IconboxFindIconSize(ibo->im, &wi, &hi, ib->iconsize);
	  }
	else
	  {
	     wi = hi = ib->iconsize;
	  }

	/* Outer size */
	if (ib->draw_icon_base && ib->im_item_base)
	  {
	     if (ib->type == IB_TYPE_ICONBOX)
	       {
		  wo = ib->iconsize + padl + padr;
		  ho = ib->iconsize + padt + padb;
	       }
	     else
	       {
		  if (ib->objs[i].u.swin->mapped)
		    {
		       wo = ib->iconsize + padl + padr;
		       ho = ib->iconsize + padt + padb;
		    }
		  else
		     wo = ho = 0;
	       }
	  }
	else
	  {
	     if (ib->orientation)
	       {
		  wo = ib->iconsize;
		  ho = hi;
	       }
	     else
	       {
		  wo = wi;
		  ho = ib->iconsize;
	       }
	  }

	ibo->xo = xo;
	ibo->yo = yo;
	ibo->wo = wo;
	ibo->ho = ho;
	ibo->xi = xo + (wo - wi) / 2;
	ibo->yi = yo + (ho - hi) / 2;
	ibo->wi = wi;
	ibo->hi = hi;
#if 0
	Eprintf("xo,yo=%d,%d wo,ho=%d,%d  xi,yi=%d,%d wi,hi=%d,%d\n",
		ibo->xo, ibo->yo, ibo->wo, ibo->ho, ibo->xi, ibo->yi, ibo->wi,
		ibo->hi);
#endif

	if (ib->orientation)
	   yo += ho + item_pad;
	else
	   xo += wo + item_pad;
     }

   if (ib->ic_box)
     {
	pad = ImageclassGetPadding(ib->ic_box);
	xo += pad->right;
	yo += pad->bottom;
     }

   if (ib->orientation)
      ib->max = yo - item_pad;
   else
      ib->max = xo - item_pad;

   if (ib->max < ib->max_min)
      ib->max = ib->max_min;
}

static EWin        *
IB_FindIcon(Iconbox * ib, int px, int py)
{
   int                 i;
   IboxOject          *ibo;

   if (ib->type != IB_TYPE_ICONBOX)
      return NULL;

   for (i = 0; i < ib->num_objs; i++)
     {
	ibo = &ib->objs[i];

	if (px >= ibo->xo - 1 && py >= ibo->yo - 1 &&
	    px < ibo->xo + ibo->wo + 1 && py < ibo->yo + ibo->ho + 1)
	   return ibo->u.ewin;
     }
   return NULL;
}

static void
IB_DrawScroll(Iconbox * ib)
{
   ImageClass         *ic;
   Imlib_Border       *pad;
   int                 arrow_mode = ib->arrow_side;
   int                 bs, bw, bx;
   int                 state;

   switch (ib->orientation)
     {
     default:
	if (ib->h < 2 * ib->arrow_thickness + ib->knob_length)
	   arrow_mode = 3;	/* No arrows */

	ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_VERTICAL", 0);
	pad = ImageclassGetPadding(ic);
	if (arrow_mode < 3)
	   bs = ib->h - (ib->arrow_thickness * 2);
	else
	   bs = ib->h;
	if (ic)
	   bs -= pad->top + pad->bottom;
	bw = (ib->h * bs) / ib->max;
	if (bs < 1)
	   bs = 1;
	if (bw > bs)
	   bw = bs;
	if (bw < 1)
	   bw = 1;
	bx = ((ib->pos * bs) / ib->max);
	if (ic)
	   bx += pad->top;
	if ((ib->scrollbar_hide) && (bw == bs))
	   goto do_hide_sb;

	EMapWindow(ib->scroll_win);
	if (arrow_mode < 3)
	  {
	     EMapWindow(ib->arrow1_win);
	     EMapWindow(ib->arrow2_win);
	  }
	else
	  {
	     EUnmapWindow(ib->arrow1_win);
	     EUnmapWindow(ib->arrow2_win);
	  }

	/* fix this area */
	if (ib->scrollbar_side == 1)
	   /* right */
	  {
	     /* start */
	     if (arrow_mode == 0)
	       {
		  EMoveResizeWindow(ib->arrow1_win,
				    ib->w - ib->scroll_thickness, 0,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->scroll_thickness,
				    ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->w - ib->scroll_thickness,
				    ib->arrow_thickness * 2,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* both ends */
	     else if (arrow_mode == 1)
	       {
		  EMoveResizeWindow(ib->arrow1_win,
				    ib->w - ib->scroll_thickness, 0,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->scroll_thickness,
				    ib->h - ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->w - ib->scroll_thickness,
				    ib->arrow_thickness,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* end */
	     else if (arrow_mode == 2)
	       {
		  EMoveResizeWindow(ib->arrow1_win,
				    ib->w - ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2),
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->scroll_thickness,
				    ib->h - ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->w - ib->scroll_thickness, 0,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* no arrows */
	     else
	       {
		  EMoveResizeWindow(ib->scroll_win,
				    ib->w - ib->scroll_thickness, 0,
				    ib->scroll_thickness, ib->h);
	       }
	  }
	else
	   /* left */
	  {
	     /* start */
	     if (arrow_mode == 0)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0, 0,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win, 0,
				    ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win, 0,
				    ib->arrow_thickness * 2,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* both ends */
	     else if (arrow_mode == 1)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0, 0,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win, 0,
				    ib->h - ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win, 0,
				    ib->arrow_thickness,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* end */
	     else if (arrow_mode == 2)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0,
				    ib->h - (ib->arrow_thickness * 2),
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->arrow2_win, 0,
				    ib->h - ib->arrow_thickness,
				    ib->scroll_thickness, ib->arrow_thickness);
		  EMoveResizeWindow(ib->scroll_win, 0, 0,
				    ib->scroll_thickness,
				    ib->h - (ib->arrow_thickness * 2));
	       }
	     /* no arrows */
	     else
	       {
		  EMoveResizeWindow(ib->scroll_win, 0, 0,
				    ib->scroll_thickness, ib->h);
	       }
	  }

	ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_VERTICAL", 0);
	if (ic)
	   ImageclassApply(ic, ib->scroll_win, -1, -1, 0, 0, STATE_NORMAL, 0,
			   ST_ICONBOX);

	EMoveResizeWindow(ib->scrollbar_win,
			  (ib->scroll_thickness - ib->bar_thickness) / 2, bx,
			  ib->bar_thickness, bw);

	ic = ImageclassFind("ICONBOX_SCROLLBAR_KNOB_VERTICAL", 0);
	if (ic)
	  {
	     state = STATE_NORMAL;
	     if (ib->scrollbar_hilited)
		state = STATE_HILITED;
	     if (ib->scrollbar_clicked)
		state = STATE_CLICKED;
	     ImageclassApply(ic, ib->scrollbar_win, -1, -1, 0, 0, state, 0,
			     ST_ICONBOX);
	  }

	ic = ImageclassFind("ICONBOX_SCROLLKNOB_VERTICAL", 0);
	if ((ic) && (bw > ib->knob_length))
	  {
	     EMapWindow(ib->scrollbarknob_win);
	     EMoveResizeWindow(ib->scrollbarknob_win, 0,
			       (bw - ib->knob_length) / 2, ib->bar_thickness,
			       ib->knob_length);

	     state = STATE_NORMAL;
	     if (ib->scrollbar_hilited)
		state = STATE_HILITED;
	     if (ib->scrollbar_clicked)
		state = STATE_CLICKED;
	     ImageclassApply(ic, ib->scrollbarknob_win, -1, -1, 0, 0, state, 0,
			     ST_ICONBOX);
	  }
	else
	  {
	     EUnmapWindow(ib->scrollbarknob_win);
	  }

	if (arrow_mode < 3)
	  {
	     ic = ImageclassFind("ICONBOX_ARROW_UP", 0);
	     if (ic)
	       {
		  state = STATE_NORMAL;
		  if (ib->arrow1_hilited)
		     state = STATE_HILITED;
		  if (ib->arrow1_clicked)
		     state = STATE_CLICKED;
		  ImageclassApply(ic, ib->arrow1_win, -1, -1, 0, 0, state, 0,
				  ST_ICONBOX);
	       }

	     ic = ImageclassFind("ICONBOX_ARROW_DOWN", 0);
	     if (ic)
	       {
		  state = STATE_NORMAL;
		  if (ib->arrow2_hilited)
		     state = STATE_HILITED;
		  if (ib->arrow2_clicked)
		     state = STATE_CLICKED;
		  ImageclassApply(ic, ib->arrow2_win, -1, -1, 0, 0, state, 0,
				  ST_ICONBOX);
	       }
	  }
	break;

     case 0:
	if (ib->w < 2 * ib->arrow_thickness + ib->knob_length)
	   arrow_mode = 3;	/* No arrows */

	ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_HORIZONTAL", 0);
	pad = ImageclassGetPadding(ic);
	if (arrow_mode < 3)
	   bs = ib->w - (ib->arrow_thickness * 2);
	else
	   bs = ib->w;
	if (ic)
	   bs -= pad->left + pad->right;
	bw = (ib->w * bs) / ib->max;
	if (bs < 1)
	   bs = 1;
	if (bw > bs)
	   bw = bs;
	if (bw < 1)
	   bw = 1;
	bx = ((ib->pos * bs) / ib->max);
	if (ic)
	   bx += pad->left;
	if ((ib->scrollbar_hide) && (bw == bs))
	   goto do_hide_sb;

	EMapWindow(ib->scroll_win);
	if (arrow_mode < 3)
	  {
	     EMapWindow(ib->arrow1_win);
	     EMapWindow(ib->arrow2_win);
	  }
	else
	  {
	     EUnmapWindow(ib->arrow1_win);
	     EUnmapWindow(ib->arrow2_win);
	  }

	if (ib->scrollbar_side == 1)
	   /* bottom */
	  {
	     /* start */
	     if (arrow_mode == 0)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0,
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->arrow_thickness,
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->arrow_thickness * 2,
				    ib->h - ib->scroll_thickness,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* both ends */
	     else if (arrow_mode == 1)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0,
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->arrow_thickness,
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->arrow_thickness,
				    ib->h - ib->scroll_thickness,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* end */
	     else if (arrow_mode == 2)
	       {
		  EMoveResizeWindow(ib->arrow1_win,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->arrow_thickness,
				    ib->h - ib->scroll_thickness,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win, 0,
				    ib->h - ib->scroll_thickness,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* no arrows */
	     else
	       {
		  EMoveResizeWindow(ib->scroll_win, 0,
				    ib->h - ib->scroll_thickness, ib->w,
				    ib->scroll_thickness);
	       }
	  }
	else
	   /* top */
	  {
	     /* start */
	     if (arrow_mode == 0)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0, 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->arrow_thickness, 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->arrow_thickness * 2, 0,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* both ends */
	     else if (arrow_mode == 1)
	       {
		  EMoveResizeWindow(ib->arrow1_win, 0, 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->arrow_thickness, 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win,
				    ib->arrow_thickness, 0,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* end */
	     else if (arrow_mode == 2)
	       {
		  EMoveResizeWindow(ib->arrow1_win,
				    ib->w - (ib->arrow_thickness * 2), 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->arrow2_win,
				    ib->w - ib->arrow_thickness, 0,
				    ib->arrow_thickness, ib->scroll_thickness);
		  EMoveResizeWindow(ib->scroll_win, 0, 0,
				    ib->w - (ib->arrow_thickness * 2),
				    ib->scroll_thickness);
	       }
	     /* no arrows */
	     else
	       {
		  EMoveResizeWindow(ib->scroll_win, 0, 0, ib->w,
				    ib->scroll_thickness);
	       }
	  }

	EMoveResizeWindow(ib->scrollbar_win, bx,
			  (ib->scroll_thickness - ib->bar_thickness) / 2, bw,
			  ib->bar_thickness);

	ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_HORIZONTAL", 0);
	if (ic)
	   ImageclassApply(ic, ib->scroll_win, -1, -1, 0, 0, STATE_NORMAL, 0,
			   ST_ICONBOX);

	ic = ImageclassFind("ICONBOX_SCROLLBAR_KNOB_HORIZONTAL", 0);
	if (ic)
	  {
	     state = STATE_NORMAL;
	     if (ib->scrollbar_hilited)
		state = STATE_HILITED;
	     if (ib->scrollbar_clicked)
		state = STATE_CLICKED;
	     ImageclassApply(ic, ib->scrollbar_win, -1, -1, 0, 0, state, 0,
			     ST_ICONBOX);
	  }

	ic = ImageclassFind("ICONBOX_SCROLLKNOB_HORIZONTAL", 0);
	if ((ic) && (bw > ib->knob_length))
	  {
	     EMapWindow(ib->scrollbarknob_win);
	     EMoveResizeWindow(ib->scrollbarknob_win,
			       (bw - ib->knob_length) / 2, 0, ib->knob_length,
			       ib->bar_thickness);

	     state = STATE_NORMAL;
	     if (ib->scrollbar_hilited)
		state = STATE_HILITED;
	     if (ib->scrollbar_clicked)
		state = STATE_CLICKED;
	     ImageclassApply(ic, ib->scrollbarknob_win, -1, -1, 0, 0, state, 0,
			     ST_ICONBOX);
	  }
	else
	  {
	     EUnmapWindow(ib->scrollbarknob_win);
	  }

	if (arrow_mode < 3)
	  {
	     ic = ImageclassFind("ICONBOX_ARROW_LEFT", 0);
	     if (ic)
	       {
		  state = STATE_NORMAL;
		  if (ib->arrow1_hilited)
		     state = STATE_HILITED;
		  if (ib->arrow1_clicked)
		     state = STATE_CLICKED;
		  ImageclassApply(ic, ib->arrow1_win, -1, -1, 0, 0, state, 0,
				  ST_ICONBOX);
	       }

	     ic = ImageclassFind("ICONBOX_ARROW_RIGHT", 0);
	     if (ic)
	       {
		  state = STATE_NORMAL;
		  if (ib->arrow2_hilited)
		     state = STATE_HILITED;
		  if (ib->arrow2_clicked)
		     state = STATE_CLICKED;
		  ImageclassApply(ic, ib->arrow2_win, -1, -1, 0, 0, state, 0,
				  ST_ICONBOX);
	       }
	  }
	break;

      do_hide_sb:
	EUnmapWindow(ib->scroll_win);
	EUnmapWindow(ib->arrow1_win);
	EUnmapWindow(ib->arrow2_win);
	break;
     }
}

static void
IB_FixPos(Iconbox * ib)
{
   int                 v;

   if (ib->orientation)
      v = ib->max - ib->h;
   else
      v = ib->max - ib->w;

   if (ib->pos > v)
      ib->pos = v;
   if (ib->pos < 0)
      ib->pos = 0;
}

static void
IconboxLayout(Iconbox * ib, int *px, int *py, int *pw, int *ph)
{
   int                 x, y, w, h;
   EWin               *ewin = ib->ewin;

   x = *px;
   y = *py;
   w = *pw;
   h = *ph;
   ICCCM_SizeMatch(ewin, w, h, &w, &h);

   IconboxLayoutImageWin(ib);

   if (ib->auto_resize)
     {
	int                 add = 0;

	if (ib->orientation)
	  {
	     add = ib->max;
	     if (ib->ewin->border)
	       {
		  if ((ib->ewin->border->border.top +
		       ib->ewin->border->border.bottom + add) > VRoot.h)
		     add =
			VRoot.h - (ib->ewin->border->border.top +
				   ib->ewin->border->border.bottom);
	       }
	     y += (((ib->ewin->client.h - add) * ib->auto_resize_anchor) >> 10);
	     h = add;
	     if (ib->ewin->border)
	       {
		  if ((EoGetY(ib->ewin) + ib->ewin->border->border.top +
		       ib->ewin->border->border.bottom + add) > VRoot.h)
		     y = VRoot.h - (ib->ewin->border->border.top +
				    ib->ewin->border->border.bottom + add);
	       }
	  }
	else
	  {
	     add = ib->max;
	     if (ib->ewin->border)
	       {
		  if ((ib->ewin->border->border.left +
		       ib->ewin->border->border.right + add) > VRoot.w)
		     add =
			VRoot.w - (ib->ewin->border->border.left +
				   ib->ewin->border->border.right);
	       }
	     x += (((ib->ewin->client.w - add) * ib->auto_resize_anchor) >> 10);
	     w = add;
	     if (ib->ewin->border)
	       {
		  if ((EoGetX(ib->ewin) + ib->ewin->border->border.left +
		       ib->ewin->border->border.right + add) > VRoot.w)
		     x = VRoot.w - (ib->ewin->border->border.left +
				    ib->ewin->border->border.right + add);
	       }
	  }
     }

   IB_FixPos(ib);

   *px = x;
   *py = y;
   *pw = w;
   *ph = h;
}

static void
IconboxDraw(Iconbox * ib)
{
   int                 i, x, y, w, h;
   ImageClass         *ib_ic_cover;
   int                 ib_xlt, ib_ylt, ib_ww, ib_hh;
   int                 ib_x0, ib_y0, ib_w0, ib_h0;
   Imlib_Image        *im, *im2;
   int                 ww, hh;
   Pixmap              pmap, mask;

   x = EoGetX(ib->ewin);
   y = EoGetY(ib->ewin);
   w = ib->w;
   h = ib->h;

   IB_DrawScroll(ib);

   /* Geometry of iconbox window, excluding scrollbar */
   ib_xlt = 0;
   ib_ylt = 0;
   ib_ww = w;
   ib_hh = h;
   if (ib->orientation)
     {
	ib_ic_cover = ImageclassFind("ICONBOX_COVER_VERTICAL", 0);
	if (ib->scrollbar_side == 0)
	   ib_xlt = ib->scroll_thickness;
	ib_ww -= ib->scroll_thickness;

	/* Geometry of icon window (including invisible parts) */
	ib_x0 = ib_xlt;
	ib_y0 = ib_ylt - ib->pos;
	ib_w0 = ib_ww;
	ib_h0 = ib_hh;
	if (ib_h0 < ib->max)
	   ib_h0 = ib->max;
     }
   else
     {
	ib_ic_cover = ImageclassFind("ICONBOX_COVER_HORIZONTAL", 0);
	if (ib->scrollbar_side == 0)
	   ib_ylt = ib->scroll_thickness;
	ib_hh -= ib->scroll_thickness;

	/* Geometry of icon window (including invisible parts) */
	ib_x0 = ib_xlt - ib->pos;
	ib_y0 = ib_ylt;
	ib_w0 = ib_ww;
	if (ib_w0 < ib->max)
	   ib_w0 = ib->max;
	ib_h0 = ib_hh;
     }

   EMoveResizeWindow(ib->icon_win, ib_x0, ib_y0, ib_w0, ib_h0);

   if (ib_ic_cover && !ib->cover_hide)
     {
	EMoveResizeWindow(ib->cover_win, ib_xlt, ib_ylt, ib_ww, ib_hh);
	EMapWindow(ib->cover_win);
	ImageclassApply(ib_ic_cover, ib->cover_win, -1, -1, 0, 0, STATE_NORMAL,
			0, ST_ICONBOX);
     }
   else
     {
	EMoveResizeWindow(ib->cover_win, -30000, -30000, 2, 2);
	EUnmapWindow(ib->cover_win);
     }

   if (ib->nobg && ib->num_objs == 0)
     {
	im = NULL;
     }
   else if (ib->ic_box &&
	    (!ib->nobg || (ib->type == IB_TYPE_SYSTRAY && !ib->draw_icon_base)))
     {
	/* Start out with iconbox image class image */
	im2 = ImageclassGetImage(ib->ic_box, 0, 0, STATE_NORMAL);
	imlib_context_set_image(im2);
	ww = imlib_image_get_width();
	hh = imlib_image_get_height();
	im = imlib_create_cropped_scaled_image(0, 0, ww, hh, ib_w0, ib_h0);
	imlib_free_image();
     }
   else
     {
	/* Start out with blank image */
	im = imlib_create_image(ib_w0, ib_h0);
	imlib_context_set_image(im);
	imlib_image_set_has_alpha(1);
	imlib_context_set_blend(0);
	imlib_context_set_color(0, 0, 0, 0);
	imlib_image_fill_rectangle(0, 0, ib_w0, ib_h0);
     }

   for (i = 0; i < ib->num_objs; i++)
     {
	IboxOject          *ibo;

	ibo = &ib->objs[i];

	if (ib->draw_icon_base && ib->im_item_base)
	  {
	     imlib_context_set_image(ib->im_item_base);
	     ww = imlib_image_get_width();
	     hh = imlib_image_get_height();
	     imlib_context_set_image(im);
	     imlib_context_set_blend(1);
	     imlib_blend_image_onto_image(ib->im_item_base, 1, 0, 0,
					  ww, hh, ibo->xo, ibo->yo,
					  ibo->wo, ibo->ho);
	     imlib_context_set_blend(0);
	  }

	if (ib->type == IB_TYPE_ICONBOX)
	  {
	     if (ibo->im)
	       {
		  imlib_context_set_image(ibo->im);
		  ww = imlib_image_get_width();
		  hh = imlib_image_get_height();
		  imlib_context_set_image(im);
		  imlib_context_set_anti_alias(1);
		  imlib_context_set_blend(1);
		  imlib_blend_image_onto_image(ibo->im, 1, 0, 0, ww, hh,
					       ibo->xi, ibo->yi, ibo->wi,
					       ibo->hi);
		  imlib_context_set_blend(0);
		  imlib_context_set_anti_alias(0);
	       }
	  }
	else
	  {
	     if (ibo->u.swin->mapped)
	       {
		  EMoveResizeWindow(ibo->u.swin->win,
				    ibo->xi, ibo->yi, ibo->wi, ibo->hi);
	       }
	  }
     }

   if (im)
     {
	EMapWindow(ib->icon_win);
	imlib_context_set_drawable(ib->icon_win);
	imlib_context_set_image(im);
	imlib_image_set_has_alpha(1);
	pmap = mask = None;
	imlib_render_pixmaps_for_whole_image(&pmap, &mask);
	ESetWindowBackgroundPixmap(ib->icon_win, pmap);
	EShapeCombineMask(ib->icon_win, ShapeBounding, 0, 0, mask, ShapeSet);
	imlib_free_pixmap_and_mask(pmap);
	imlib_free_image();
	EClearWindow(ib->icon_win);

	if (ib->type == IB_TYPE_SYSTRAY && ib->nobg && !ib->draw_icon_base)
	   EShapePropagate(ib->icon_win);
     }
   else
     {
	/* Transparent and no objects */
	EUnmapWindow(ib->icon_win);
     }
   EShapePropagate(ib->win);
   EwinUpdateShapeInfo(ib->ewin);
   ib->ewin->update.shape = 1;
   EwinPropagateShapes(ib->ewin);
}

static void
IconboxRedraw(Iconbox * ib)
{
   EWin               *ewin = ib->ewin;

   ib->do_update = 1;
   EwinResize(ib->ewin, ewin->client.w, ewin->client.h);
}

static int
IB_Scroll(Iconbox * ib, int dir)
{
   int                 ppos;

   ppos = ib->pos;
   ib->pos += dir;
   IB_FixPos(ib);
   if (ib->pos == ppos)
      return 0;

   IconboxDraw(ib);
   return 1;
}

static void
IB_ShowMenu(Iconbox * ib, int x __UNUSED__, int y __UNUSED__)
{
   static Menu        *p_menu = NULL;
   MenuItem           *mi;
   char                s[1024];

   if (p_menu)
      MenuDestroy(p_menu);

   if (ib->type == IB_TYPE_ICONBOX)
     {
	p_menu = MenuCreate("__IBOX_MENU", _("Iconbox Options"), NULL, NULL);

	Esnprintf(s, sizeof(s), "ibox cfg %s", ib->name);
	mi = MenuItemCreate(_("This Iconbox Settings..."), NULL, s, NULL);
	MenuAddItem(p_menu, mi);

	Esnprintf(s, sizeof(s), "wop %#lx cl", ib->win);
	mi = MenuItemCreate(_("Close Iconbox"), NULL, s, NULL);
	MenuAddItem(p_menu, mi);

	mi = MenuItemCreate(_("Create New Iconbox"), NULL, "ibox new", NULL);
	MenuAddItem(p_menu, mi);

	EFunc(NULL, "menus show __IBOX_MENU");
     }
   else
     {
	p_menu = MenuCreate("__TRAY_MENU", _("Systray Options"), NULL, NULL);

	Esnprintf(s, sizeof(s), "ibox cfg %s", ib->name);
	mi = MenuItemCreate(_("Systray Settings..."), NULL, s, NULL);
	MenuAddItem(p_menu, mi);

	Esnprintf(s, sizeof(s), "wop %#lx cl", ib->win);
	mi = MenuItemCreate(_("Close Systray"), NULL, s, NULL);
	MenuAddItem(p_menu, mi);

	EFunc(NULL, "menus show __TRAY_MENU");
     }
}

static void
IconboxesShow(void)
{
   int                 i, num;
   Iconbox           **ibl;

   ibl = IconboxesList(&num);
   if (ibl)
     {
	for (i = 0; i < num; i++)
	   IconboxShow(ibl[i]);
	Efree(ibl);
     }
   else if (Conf.startup.firsttime)
     {
	Iconbox            *ib;

	ib = IconboxCreate("_IB_0");
	IconboxShow(ib);
	IconboxesConfigSave();
     }
}

static void
IconboxesDestroy(void)
{
   int                 i, num;
   Iconbox           **ibl;

   ibl = IconboxesList(&num);
   if (!ibl)
      return;

   for (i = 0; i < num; i++)
      IconboxDestroy(ibl[i], 1);
   Efree(ibl);
}

static void
IboxEventScrollWin(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;
   int                 x, y, w, h;

   switch (ev->type)
     {
     case ButtonPress:
	if (ev->xbutton.button == 1)
	   ib->scrollbox_clicked = 1;
	else if (ev->xbutton.button == 3)
	   IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;

     case ButtonRelease:
	if (!ib->scrollbox_clicked)
	   break;
	ib->scrollbox_clicked = 0;
	EGetGeometry(ib->scrollbar_win, NULL, &x, &y, &w, &h, NULL, NULL);
	if (ib->orientation)
	  {
	     if (ev->xbutton.y < y)
		IB_Scroll(ib, -8);
	     else if (ev->xbutton.y > (y + h))
		IB_Scroll(ib, 8);
	  }
	else
	  {
	     if (ev->xbutton.x < x)
		IB_Scroll(ib, -8);
	     else if (ev->xbutton.x > (x + w))
		IB_Scroll(ib, 8);
	  }
	break;
     }
}

static void
IboxEventScrollbarWin(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;
   static int          px, py, pos0;
   int                 bs, dp;
   ImageClass         *ic;
   Imlib_Border       *pad;

   switch (ev->type)
     {
     case ButtonPress:
	if (ev->xbutton.button == 1)
	  {
	     px = ev->xbutton.x_root;
	     py = ev->xbutton.y_root;
	     pos0 = ib->pos;
	     ib->scrollbar_clicked = 1;
	  }
	else if (ev->xbutton.button == 3)
	   IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;

     case ButtonRelease:
	if (ib->scrollbar_clicked)
	   ib->scrollbar_clicked = 0;
	break;

     case EnterNotify:
	ib->scrollbar_hilited = 1;
	break;

     case LeaveNotify:
	ib->scrollbar_hilited = 0;
	break;

     case MotionNotify:
	if (!ib->scrollbar_clicked)
	   break;

	if (ib->orientation)
	  {
	     ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_VERTICAL", 0);
	     pad = ImageclassGetPadding(ic);
	     bs = ib->h - (ib->arrow_thickness * 2);
	     if (ic)
		bs -= pad->top + pad->bottom;
	     if (bs < 1)
		bs = 1;
	     dp = ev->xmotion.y_root - py;
	  }
	else
	  {
	     ic = ImageclassFind("ICONBOX_SCROLLBAR_BASE_HORIZONTAL", 0);
	     pad = ImageclassGetPadding(ic);
	     bs = ib->w - (ib->arrow_thickness * 2);
	     if (ic)
		bs -= pad->left + pad->right;
	     if (bs < 1)
		bs = 1;
	     dp = ev->xmotion.x_root - px;
	  }
	dp = pos0 + (dp * ib->max) / bs - ib->pos;
	if (dp)
	   IB_Scroll(ib, dp);
	return;
     }
   IB_DrawScroll(ib);
}

static void
IboxEventCoverWin(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;
     case ButtonRelease:
	break;
     }
}

static void
IboxEventArrow1Win(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	if (ev->xbutton.button == 1)
	   ib->arrow1_clicked = 1;
	else if (ev->xbutton.button == 3)
	   IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;

     case ButtonRelease:
	if (!ib->arrow1_clicked)
	   break;
	ib->arrow1_clicked = 0;
	if (IB_Scroll(ib, -8))
	   return;
	break;

     case EnterNotify:
	ib->arrow1_hilited = 1;
	break;

     case LeaveNotify:
	ib->arrow1_hilited = 0;
	break;
     }
   IB_DrawScroll(ib);
}

static void
IboxEventArrow2Win(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	if (ev->xbutton.button == 1)
	   ib->arrow2_clicked = 1;
	else if (ev->xbutton.button == 3)
	   IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;

     case ButtonRelease:
	if (!ib->arrow2_clicked)
	   break;
	ib->arrow2_clicked = 0;
	if (IB_Scroll(ib, 8))
	   return;
	break;

     case EnterNotify:
	ib->arrow2_hilited = 1;
	break;

     case LeaveNotify:
	ib->arrow2_hilited = 0;
	break;
     }
   IB_DrawScroll(ib);
}

static void
IboxEventIconWin(XEvent * ev, void *prm)
{
   Iconbox            *ib = (Iconbox *) prm;
   static EWin        *name_ewin = NULL;
   ToolTip            *tt;
   EWin               *ewin;
   int                 x, y;
   const char         *name;

   switch (ev->type)
     {
     case ButtonPress:
	if (ev->xbutton.button == 1)
	   ib->icon_clicked = 1;
	else if (ev->xbutton.button == 3)
	   IB_ShowMenu(ib, ev->xbutton.x, ev->xbutton.y);
	break;

     case ButtonRelease:
	if (!ib->icon_clicked)
	   break;

	ib->icon_clicked = 0;
	ewin = IB_FindIcon(ib, ev->xbutton.x, ev->xbutton.y);
	if (!ewin)
	   break;

	tt = FindItem("ICONBOX", 0, LIST_FINDBY_NAME, LIST_TYPE_TOOLTIP);
	if (tt)
	   TooltipHide(tt);

	EwinOpIconify(ewin, 0);
	break;

     case MotionNotify:
	x = ev->xmotion.x;
	y = ev->xmotion.y;
	goto do_motion;

     case EnterNotify:
	x = ev->xcrossing.x;
	y = ev->xcrossing.y;
	goto do_motion;

      do_motion:
	if (!ib->shownames)
	   break;

	ewin = IB_FindIcon(ib, x, y);
	if (ewin == name_ewin)
	   break;
	name_ewin = ewin;

	tt = FindItem("ICONBOX", 0, LIST_FINDBY_NAME, LIST_TYPE_TOOLTIP);
	if (!tt)
	   break;

	TooltipHide(tt);
	if (!ewin)
	   break;

	name = EwinGetIconName(ewin);
	if (name)
	   TooltipShow(tt, name, NULL, Mode.events.x, Mode.events.y);
	break;

     case LeaveNotify:
	tt = FindItem("ICONBOX", 0, LIST_FINDBY_NAME, LIST_TYPE_TOOLTIP);
	if (tt)
	  {
	     TooltipHide(tt);
	     name_ewin = NULL;
	  }
	break;
     }
}

/*
 * Configuration dialog
 */
static char        *tmp_ib_name = NULL;
static char         tmp_ib_nobg;
static char         tmp_ib_shownames;
static int          tmp_ib_vert;
static int          tmp_ib_side;
static int          tmp_ib_arrows;
static int          tmp_ib_iconsize;
static int          tmp_ib_mode;
static char         tmp_ib_auto_resize;
static char         tmp_ib_draw_icon_base;
static char         tmp_ib_scrollbar_hide;
static char         tmp_ib_cover_hide;
static int          tmp_ib_autoresize_anchor;
static char         tmp_ib_animate;

static void
CB_ConfigureIconbox(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	Iconbox            *ib;

	if (!tmp_ib_name)
	   return;
	ib = FindItem(tmp_ib_name, 0, LIST_FINDBY_NAME, LIST_TYPE_ICONBOX);
	if (!ib)
	   return;

	ib->nobg = tmp_ib_nobg;
	ib->shownames = tmp_ib_shownames;
	ib->orientation = tmp_ib_vert;
	ib->scrollbar_side = tmp_ib_side;
	ib->arrow_side = tmp_ib_arrows;
	ib->iconsize = tmp_ib_iconsize;
	ib->icon_mode = tmp_ib_mode;
	ib->auto_resize = tmp_ib_auto_resize;
	ib->draw_icon_base = tmp_ib_draw_icon_base;
	ib->scrollbar_hide = tmp_ib_scrollbar_hide;
	ib->cover_hide = tmp_ib_cover_hide;
	ib->auto_resize_anchor = tmp_ib_autoresize_anchor;
	ib->animate = tmp_ib_animate;

	IconboxReconfigure(ib);
	IconboxRedraw(ib);
	IconboxesConfigSave();
     }
}

static void
CB_IconSizeSlider(Dialog * d, int val __UNUSED__, void *data)
{
   DItem              *di;
   char                s[256];

   di = data;
   Esnprintf(s, sizeof(s), _("Icon size: %2d"), tmp_ib_iconsize);
   DialogItemSetText(di, s);
   DialogDrawItems(d, di, 0, 0, 99999, 99999);
}

static void
IconboxConfigure(Iconbox * ib)
{
   Dialog             *d;
   DItem              *di, *table, *table2;
   DItem              *radio1, *radio2, *radio3, *radio4, *label;
   char                s[256];

   if (!ib)
      return;

   if ((d =
	FindItem("CONFIGURE_ICONBOX", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_ICONBOX");

   tmp_ib_nobg = ib->nobg;
   tmp_ib_shownames = ib->shownames;
   tmp_ib_vert = ib->orientation;
   tmp_ib_side = ib->scrollbar_side;
   tmp_ib_arrows = ib->arrow_side;
   tmp_ib_iconsize = ib->iconsize;
   tmp_ib_mode = ib->icon_mode;
   tmp_ib_auto_resize = ib->auto_resize;
   tmp_ib_draw_icon_base = ib->draw_icon_base;
   tmp_ib_scrollbar_hide = ib->scrollbar_hide;
   tmp_ib_cover_hide = ib->cover_hide;
   tmp_ib_autoresize_anchor = ib->auto_resize_anchor;
   tmp_ib_animate = ib->animate;
   if (tmp_ib_name)
      Efree(tmp_ib_name);
   tmp_ib_name = Estrdup(ib->name);

   d = DialogCreate("CONFIGURE_ICONBOX");
   if (ib->type == IB_TYPE_ICONBOX)
      DialogSetTitle(d, _("Iconbox Settings"));
   else
      DialogSetTitle(d, _("Systray Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 1, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	table2 = DialogAddItem(table, DITEM_TABLE);
	DialogItemTableSetOptions(table2, 2, 0, 0, 0);

	di = DialogAddItem(table2, DITEM_IMAGE);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemImageSetFile(di, "pix/iconbox.png");

	di = DialogAddItem(table2, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Enlightenment Iconbox\n" "Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Transparent background"));
   DialogItemCheckButtonSetState(di, tmp_ib_nobg);
   DialogItemCheckButtonSetPtr(di, &tmp_ib_nobg);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Hide inner border"));
   DialogItemCheckButtonSetState(di, tmp_ib_cover_hide);
   DialogItemCheckButtonSetPtr(di, &tmp_ib_cover_hide);

   if (ib->type == IB_TYPE_ICONBOX)
     {
	di = DialogAddItem(table, DITEM_CHECKBUTTON);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Show icon names"));
	DialogItemCheckButtonSetState(di, tmp_ib_shownames);
	DialogItemCheckButtonSetPtr(di, &tmp_ib_shownames);
     }

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Draw base image behind Icons"));
   DialogItemCheckButtonSetState(di, tmp_ib_draw_icon_base);
   DialogItemCheckButtonSetPtr(di, &tmp_ib_draw_icon_base);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Hide scrollbar when not needed"));
   DialogItemCheckButtonSetState(di, tmp_ib_scrollbar_hide);
   DialogItemCheckButtonSetPtr(di, &tmp_ib_scrollbar_hide);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Automatically resize to fit Icons"));
   DialogItemCheckButtonSetState(di, tmp_ib_auto_resize);
   DialogItemCheckButtonSetPtr(di, &tmp_ib_auto_resize);

   if (ib->type == IB_TYPE_ICONBOX)
     {
	di = DialogAddItem(table, DITEM_CHECKBUTTON);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Animate when iconifying to this Iconbox"));
	DialogItemCheckButtonSetState(di, tmp_ib_animate);
	DialogItemCheckButtonSetPtr(di, &tmp_ib_animate);
     }

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 0);
   DialogItemSetText(di,
		     _("Alignment of anchoring when automatically resizing:"));

   di = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 0, 1024);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 8);
   DialogItemSliderSetVal(di, tmp_ib_autoresize_anchor);
   DialogItemSliderSetValPtr(di, &tmp_ib_autoresize_anchor);

   if (ib->type == IB_TYPE_ICONBOX)
     {
	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);

	di = DialogAddItem(table, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 0, 0);
	DialogItemSetAlign(di, 0, 0);
	DialogItemSetText(di,
			  _
			  ("Icon image display policy (if one operation fails, try the next):"));

	radio4 = di = DialogAddItem(table, DITEM_RADIOBUTTON);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di,
			  _
			  ("Snapshot Windows, Use application icon, Use Enlightenment Icon"));
	DialogItemRadioButtonSetFirst(di, radio4);
	DialogItemRadioButtonGroupSetVal(di, 0);

	di = DialogAddItem(table, DITEM_RADIOBUTTON);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di,
			  _
			  ("Use application icon, Use Enlightenment Icon, Snapshot Window"));
	DialogItemRadioButtonSetFirst(di, radio4);
	DialogItemRadioButtonGroupSetVal(di, 1);

	di = DialogAddItem(table, DITEM_RADIOBUTTON);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Use Enlightenment Icon, Snapshot Window"));
	DialogItemRadioButtonSetFirst(di, radio4);
	DialogItemRadioButtonGroupSetVal(di, 2);
	DialogItemRadioButtonGroupSetValPtr(radio4, &tmp_ib_mode);
     }

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   label = di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   Esnprintf(s, sizeof(s), _("Icon size: %2d"), tmp_ib_iconsize);
   DialogItemSetText(di, s);

   di = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 4, 128);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 8);
   DialogItemSliderSetVal(di, tmp_ib_iconsize);
   DialogItemSliderSetValPtr(di, &tmp_ib_iconsize);
   DialogItemSetCallback(di, CB_IconSizeSlider, 0, label);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   table2 = DialogAddItem(table, DITEM_TABLE);
   DialogItemTableSetOptions(table2, 3, 0, 0, 0);

   di = DialogAddItem(table2, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Orientation:"));

   di = DialogAddItem(table2, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Scrollbar side:"));

   di = DialogAddItem(table2, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Scrollbar arrows:"));

   radio1 = di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Horizontal"));
   DialogItemRadioButtonSetFirst(di, radio1);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio2 = di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Left / Top"));
   DialogItemRadioButtonSetFirst(di, radio2);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio3 = di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Start"));
   DialogItemRadioButtonSetFirst(di, radio3);
   DialogItemRadioButtonGroupSetVal(di, 0);

   di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Vertical"));
   DialogItemRadioButtonSetFirst(di, radio1);
   DialogItemRadioButtonGroupSetVal(di, 1);
   DialogItemRadioButtonGroupSetValPtr(radio1, &tmp_ib_vert);

   di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Right / Bottom"));
   DialogItemRadioButtonSetFirst(di, radio2);
   DialogItemRadioButtonGroupSetVal(di, 1);
   DialogItemRadioButtonGroupSetValPtr(radio2, &tmp_ib_side);

   di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("Both ends"));
   DialogItemRadioButtonSetFirst(di, radio3);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table2, DITEM_NONE);
   di = DialogAddItem(table2, DITEM_NONE);

   di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("End"));
   DialogItemRadioButtonSetFirst(di, radio3);
   DialogItemRadioButtonGroupSetVal(di, 2);

   di = DialogAddItem(table2, DITEM_NONE);
   di = DialogAddItem(table2, DITEM_NONE);

   di = DialogAddItem(table2, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetText(di, _("None"));
   DialogItemRadioButtonSetFirst(di, radio3);
   DialogItemRadioButtonGroupSetVal(di, 3);
   DialogItemRadioButtonGroupSetValPtr(radio3, &tmp_ib_arrows);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureIconbox, 1, DIALOG_BUTTON_OK);
   DialogAddButton(d, _("Apply"), CB_ConfigureIconbox, 0, DIALOG_BUTTON_APPLY);
   DialogAddButton(d, _("Close"), CB_ConfigureIconbox, 1, DIALOG_BUTTON_CLOSE);
   DialogSetExitFunction(d, CB_ConfigureIconbox, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureIconbox, 0);
   ShowDialog(d);
}

/*
 * Configuration load/save
 */
#include "conf.h"

static void
IconboxesConfigLoad(void)
{
   int                 err = 0;
   FILE               *fs;
   char                s[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1, i2;
   int                 fields;
   Iconbox            *ib;

   Esnprintf(s, sizeof(s), "%s.ibox", EGetSavePrefix());
   fs = fopen(s, "r");
   if (!fs)
      return;

   ib = NULL;
   while (fgets(s, sizeof(s), fs))
     {
	s2[0] = 0;
	i1 = CONFIG_INVALID;
	fields = sscanf(s, "%i %4000s", &i1, s2);

	if (fields < 1)
	   i1 = CONFIG_INVALID;
	else if (i1 == CONFIG_CLOSE)
	  {
	     if (fields != 1)
		Alert(_("CONFIG: ignoring extra data in \"%s\"\n"), s);
	  }

	switch (i1)
	  {
	  case CONFIG_IBOX:
	     err = -1;
	     i2 = atoi(s2);
	     if (i2 != CONFIG_OPEN)
		goto done;
	     break;
	  case CONFIG_CLOSE:
	     ib = NULL;
	     err = 0;
	     break;

	  case CONFIG_CLASSNAME:	/* __NAME %s */
	     ib = FindItem(s2, 0, LIST_FINDBY_NAME, LIST_TYPE_ICONBOX);
	     if (ib)
		IconboxDestroy(ib, 0);
	     ib = IconboxCreate(s2);
	     break;
	  case TEXT_ORIENTATION:	/* __ORIENTATION [ __HORIZONTAL | __VERTICAL ] */
	     if (ib)
		ib->orientation = (char)atoi(s2);
	     break;
	  case CONFIG_TRANSPARENCY:	/* __TRANSPARENCY [ __ON | __OFF ] */
	     if (ib)
		ib->nobg = (char)atoi(s2);
	     break;
	  case CONFIG_SHOW_NAMES:	/* __SHOW_NAMES [ __ON | __OFF ] */
	     if (ib)
		ib->shownames = (char)atoi(s2);
	     break;
	  case CONFIG_ICON_SIZE:	/* __ICON_SIZE %i */
	     if (ib)
		ib->iconsize = (int)atoi(s2);
	     break;
	  case CONFIG_ICON_MODE:	/* __ICON_MODE [ 0 | 1 | 2 | 3 | 4 ] */
	     if (ib)
		ib->icon_mode = (int)atoi(s2);
	     break;
	  case CONFIG_SCROLLBAR_SIDE:	/* __SCROLLBAR_SIDE [ __BAR_LEFT/__BAR_TOP | __BAR_RIGHT/__BAR_BOTTOM ] */
	     if (ib)
		ib->scrollbar_side = (char)atoi(s2);
	     break;
	  case CONFIG_SCROLLBAR_ARROWS:	/* __SCROLLBAR_ARROWS [ __START | __BOTH | __FINISH | __NEITHER ] */
	     if (ib)
		ib->arrow_side = (char)atoi(s2);
	     break;
	  case CONFIG_AUTOMATIC_RESIZE:	/* __AUTOMATIC_RESIZE [ __ON | __OFF ] */
	     if (ib)
		ib->auto_resize = (char)atoi(s2);
	     break;
	  case CONFIG_SHOW_ICON_BASE:	/* __SHOW_ICON_BASE [ __ON | __OFF ] */
	     if (ib)
		ib->draw_icon_base = (char)atoi(s2);
	     break;
	  case CONFIG_SCROLLBAR_AUTOHIDE:	/* __SCROLLBAR_AUTOHIDE [ __ON | __OFF ] */
	     if (ib)
		ib->scrollbar_hide = (char)atoi(s2);
	     break;
	  case CONFIG_COVER_HIDE:	/* __COVER_HIDE [ __ON | __OFF ] */
	     if (ib)
		ib->cover_hide = (char)atoi(s2);
	     break;
	  case CONFIG_RESIZE_ANCHOR:	/* __COVER_HIDE 0-1024 */
	     if (ib)
		ib->auto_resize_anchor = atoi(s2);
	     break;
	  case CONFIG_IB_ANIMATE:	/* __COVER_HIDE 0-1024 */
	     if (ib)
		ib->animate = (char)atoi(s2);
	     break;
	  default:
	     Eprintf("Warning: Iconbox configuration, ignoring: %s\n", s);
	     break;
	  }
     }
   if (err)
      Eprintf("Error: Iconbox configuration file load problem.\n");

 done:
   fclose(fs);
}

static void
IconboxesConfigSave(void)
{
   char                s[FILEPATH_LEN_MAX];
   FILE               *fs;
   int                 i, num;
   Iconbox           **iblist;

   iblist = IconboxesList(&num);

   Esnprintf(s, sizeof(s), "%s.ibox", EGetSavePrefix());
   fs = fopen(s, "w");
   if (!fs)
      return;

   /* We should check for errors... */
   for (i = num - 1; i >= 0; i--)
     {
	fprintf(fs, "19 999\n");
	fprintf(fs, "100 %s\n", iblist[i]->name);
	fprintf(fs, "200 %i\n", iblist[i]->orientation);
	fprintf(fs, "2001 %i\n", iblist[i]->nobg);
	fprintf(fs, "2002 %i\n", iblist[i]->shownames);
	fprintf(fs, "2003 %i\n", iblist[i]->iconsize);
	fprintf(fs, "2004 %i\n", iblist[i]->icon_mode);
	fprintf(fs, "2005 %i\n", iblist[i]->scrollbar_side);
	fprintf(fs, "2006 %i\n", iblist[i]->arrow_side);
	fprintf(fs, "2007 %i\n", iblist[i]->auto_resize);
	fprintf(fs, "2008 %i\n", iblist[i]->draw_icon_base);
	fprintf(fs, "2009 %i\n", iblist[i]->scrollbar_hide);
	fprintf(fs, "2010 %i\n", iblist[i]->cover_hide);
	fprintf(fs, "2011 %i\n", iblist[i]->auto_resize_anchor);
	fprintf(fs, "2012 %i\n", iblist[i]->animate);
	fprintf(fs, "1000\n");
     }

   fclose(fs);

   if (iblist)
      Efree(iblist);
}

/*
 * Iconboxes Module
 */

static void
IconboxesSighan(int sig, void *prm)
{
   EWin               *ewin;

   switch (sig)
     {
     case ESIGNAL_CONFIGURE:
	break;
     case ESIGNAL_START:
	IconboxesConfigLoad();
	IconboxesShow();
	break;
     case ESIGNAL_EXIT:
	IconboxesDestroy();
	break;
     case ESIGNAL_EWIN_ICONIFY:
	ewin = (EWin *) prm;
	IconboxesEwinIconify(ewin);
	break;
     case ESIGNAL_EWIN_DEICONIFY:
	ewin = (EWin *) prm;
	IconboxesEwinDeIconify(ewin);
	break;
     case ESIGNAL_EWIN_DESTROY:
	ewin = (EWin *) prm;
	if (ewin->state.iconified > 0)
	   RemoveMiniIcon(ewin);
	break;
     case ESIGNAL_EWIN_CHANGE_ICON:
	ewin = (EWin *) prm;
	if (ewin->state.iconified)
	   IconboxesUpdateEwinIcon(ewin, 1);
	break;
     }
}

static void
IconboxesConfigure(const char *params)
{
   Iconbox            *ib;

   if (!params || !params[0])
      params = "DEFAULT";

   ib = FindItem(params, 0, LIST_FINDBY_NAME, LIST_TYPE_ICONBOX);
   if (ib)
      IconboxConfigure(ib);
}

/*
 * IPC functions
 */
static void
IboxIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[128];
   int                 len, num;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %100s %n", cmd, prm, &len);
	p += len;
     }

   if (!p || cmd[0] == '?')
     {
	/* List iconboxes */
     }
   else if (!strncmp(cmd, "cfg", 3))
     {
	IconboxesConfigure(prm);
     }
   else if (!strncmp(cmd, "new", 3))
     {
	Iconbox            *ib, **ibl;

	if (!prm[0])
	  {
	     ibl = IconboxesList(&num);
	     if (ibl)
		Efree(ibl);
	     Esnprintf(prm, sizeof(prm), "_IB_%i", num);
	  }
	ib = IconboxCreate(prm);
	IconboxShow(ib);
	IconboxesConfigSave();
     }
}

IpcItem             IconboxesIpcArray[] = {
   {
    IboxIpc,
    "iconbox", "ibox",
    "Iconbox functions",
    "  iconbox new <name>   Create new iconbox\n"
    "  iconbox cfg          Configure iconboxes\n"}
};
#define N_IPC_FUNCS (sizeof(IconboxesIpcArray)/sizeof(IpcItem))

#if 0
/*
 * Configuration items
 */
static const CfgItem IconboxesCfgItems[] = {
   CFG_ITEM_BOOL(Conf.iconboxes, enable, 1),
};
#define N_CFG_ITEMS (sizeof(IconboxesCfgItems)/sizeof(CfgItem))
#endif

/*
 * Module descriptor
 */
EModule             ModIconboxes = {
   "iconboxes", "ibox",
   IconboxesSighan,
   {N_IPC_FUNCS, IconboxesIpcArray},
   {0, NULL}
};

/*
 * System tray functions
 */

/* Selection atoms */
static Atom         E_XA_MANAGER = 0;

/* XEmbed atoms */
static Atom         E_XA__XEMBED = 0;
static Atom         E_XA__XEMBED_INFO = 0;

/* Systray atoms */
static Atom         _NET_SYSTEM_TRAY_Sx = 0;
static Atom         _NET_SYSTEM_TRAY_OPCODE = 0;
static Atom         _NET_SYSTEM_TRAY_MESSAGE_DATA = 0;
static Window       systray_sel_win = None;
static Time         systray_sel_time = 0;

static void         SystrayEvent(XEvent * ev, void *prm);

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

/* _XEMBED client message */
#define XEMBED_EMBEDDED_NOTIFY      0

/* _XEMBED_INFO property */
#define XEMBED_MAPPED               (1 << 0)

static int
SystrayGetXembedInfo(Window win, int *info)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;

   prop_ret = NULL;
   if (XGetWindowProperty(disp, win, E_XA__XEMBED_INFO, 0, 0x7fffffff,
			  False, E_XA__XEMBED_INFO, &type_ret, &format_ret,
			  &num_ret, &bytes_after, &prop_ret) != Success)
      return -1;

   if (prop_ret && type_ret == E_XA__XEMBED_INFO && format_ret == 32
       && num_ret >= 2)
     {
	info[0] = ((unsigned long *)prop_ret)[0];
	info[1] = ((unsigned long *)prop_ret)[1];
     }
   else
     {
	/* Property invalid or not there. I doubt we ever get here */
	info[0] = 0;		/* Set protocol version 0 */
	info[1] = 1;		/* Set mapped */
	num_ret = 0;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num_ret;
}

/*
 * Return index, -1 if not found.
 */
static int
IconboxObjSwinFind(Iconbox * ib, Window win)
{
   int                 i;

   for (i = 0; i < ib->num_objs; i++)
      if (ib->objs[i].u.swin->win == win)
	 return i;

   return -1;
}

static void
IconboxObjSwinAdd(Iconbox * ib, Window win)
{
   SWin               *swin = NULL;
   int                 xembed_info[2];

   /* Not if already there */
   if (IconboxObjSwinFind(ib, win) >= 0)
      return;

   EGrabServer();

   switch (SystrayGetXembedInfo(win, xembed_info))
     {
     case -1:			/* Error - assume invalid window */
	Eprintf("IconboxObjSwinAdd: Hmm.. Invalid window? Ignoring %#lx\n",
		win);
	goto bail_out;
     case 0:			/* Assume broken - proceed anyway */
	Eprintf("IconboxObjSwinAdd: Hmm.. No _XEMBED_INFO?\n");
	break;
     default:
	if (EventDebug(EDBUG_TYPE_ICONBOX))
	   Eprintf("IconboxObjSwinAdd: _XEMBED_INFO: %#lx: %d %d\n", win,
		   xembed_info[0], xembed_info[1]);
	break;
     }

   swin = Emalloc(sizeof(SWin));
   if (!swin)
      goto bail_out;

   if (IconboxObjectAdd(ib, swin) < 0)
      goto bail_out;

   swin->win = win;
   swin->mapped = (xembed_info[1] & XEMBED_MAPPED) != 0;

   ERegisterWindow(win);
   ESelectInput(win, PropertyChangeMask);
   EventCallbackRegister(win, 0, SystrayEvent, ib);
   EReparentWindow(win, ib->icon_win, 0, 0);

   EUngrabServer();

   if (swin->mapped)
     {
	XMapWindow(disp, win);
	IconboxRedraw(ib);
     }

   /* TBD - Always set protocol version as reported by client */
   ecore_x_client_message32_send(win, E_XA__XEMBED, NoEventMask,
				 CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0,
				 win, xembed_info[0]);

   return;			/* Success */

 bail_out:
   EUngrabServer();
   if (swin)
      Efree(swin);
}

static void
IconboxObjSwinDel(Iconbox * ib, Window win)
{
   int                 i;
   SWin               *swin;

   i = IconboxObjSwinFind(ib, win);
   if (i < 0)
      return;

   swin = ib->objs[i].u.swin;

   if (IconboxObjectDel(ib, swin) == 0)
      IconboxRedraw(ib);

   Efree(swin);
}

static void
IconboxObjSwinMapUnmap(Iconbox * ib, Window win)
{
   int                 i, map;
   SWin               *swin;
   int                 xembed_info[2];

   i = IconboxObjSwinFind(ib, win);
   if (i < 0)
      return;

   swin = ib->objs[i].u.swin;

   if (SystrayGetXembedInfo(win, xembed_info) >= 0)
     {
	if (EventDebug(EDBUG_TYPE_ICONBOX))
	   Eprintf("IconboxObjSwinMapUnmap: _XEMBED_INFO: %#lx: %d %d\n", win,
		   xembed_info[0], xembed_info[1]);

	map = (xembed_info[1] & XEMBED_MAPPED) != 0;
	if (map == swin->mapped)
	   return;

	if (map)
	   EMapWindow(win);
	else
	   EUnmapWindow(win);
     }
   else
     {
	Eprintf("IconboxObjSwinMapUnmap: _XEMBED_INFO: %#lx: gone?\n", win);

	map = 0;
	if (map == swin->mapped)
	   return;
     }

   swin->mapped = map;
   IconboxRedraw(ib);
}

static void
IconboxObjSwinFree(Iconbox * ib, SWin * swin)
{
   if (EventDebug(EDBUG_TYPE_ICONBOX))
      Eprintf("IconboxObjSwinFree %#lx\n", swin->win);

   if (disp)
     {
	EventCallbackUnregister(swin->win, 0, SystrayEvent, ib);
	EUnregisterWindow(swin->win);

	EUnmapWindow(swin->win);
	EReparentWindow(swin->win, VRoot.win, 0, 0);
	ESync();
     }

   Efree(swin);
}

static void
SystrayEventClientMessage(Iconbox * ib, XClientMessageEvent * ev)
{
   Window              win;

   if (EventDebug(EDBUG_TYPE_ICONBOX))
      Eprintf
	 ("SystrayEventClientMessage: ev->type=%ld ev->data.l: %#lx %#lx %#lx %#lx\n",
	  ev->message_type, ev->data.l[0], ev->data.l[1], ev->data.l[2],
	  ev->data.l[3]);

   if (ev->message_type == _NET_SYSTEM_TRAY_OPCODE)
     {
	win = ev->data.l[2];
	if (win == None)
	   goto done;

	IconboxObjSwinAdd(ib, win);
     }
   else if (ev->message_type == _NET_SYSTEM_TRAY_MESSAGE_DATA)
     {
	if (EventDebug(EDBUG_TYPE_ICONBOX))
	   Eprintf("SystrayEventClientMessage: Got data message\n");
     }
 done:
   ;
}

static void
SystrayEventClientProperty(Iconbox * ib, XPropertyEvent * ev)
{
   if (EventDebug(EDBUG_TYPE_ICONBOX))
      Eprintf("SystrayEventClientProperty %#lx %ld\n", ev->window, ev->atom);

   if (ev->atom == E_XA__XEMBED_INFO)
     {
	IconboxObjSwinMapUnmap(ib, ev->window);
     }
}

static void
SystrayEvent(XEvent * ev, void *prm)
{
   Window              win;

   if (EventDebug(EDBUG_TYPE_ICONBOX))
      Eprintf("SystrayEvent %2d %#lx\n", ev->type, ev->xany.window);

   switch (ev->type)
     {
     case MapNotify:
	EWindowSync(ev->xmap.window);
	break;

     case DestroyNotify:
	win = ev->xdestroywindow.window;
	goto do_terminate;
     case ReparentNotify:
	/* Terminate if reparenting away from systray */
	if (ev->xreparent.parent == ev->xreparent.event)
	   break;
	win = ev->xreparent.window;
	goto do_terminate;

     case ClientMessage:
	SystrayEventClientMessage(prm, &(ev->xclient));
	break;

     case PropertyNotify:
	SystrayEventClientProperty(prm, &(ev->xproperty));
	break;

      do_terminate:
	IconboxObjSwinDel(prm, win);
	break;
     }
}

static void
SystrayInit(Iconbox * ib, Window win, int screen)
{
   char                buf[32];

   Esnprintf(buf, sizeof(buf), "_NET_SYSTEM_TRAY_S%d", screen);

   E_XA_MANAGER = XInternAtom(disp, "MANAGER", False);
   E_XA__XEMBED = XInternAtom(disp, "_XEMBED", False);
   E_XA__XEMBED_INFO = XInternAtom(disp, "_XEMBED_INFO", False);
   _NET_SYSTEM_TRAY_Sx = XInternAtom(disp, buf, False);
   _NET_SYSTEM_TRAY_OPCODE =
      XInternAtom(disp, "_NET_SYSTEM_TRAY_OPCODE", False);
   _NET_SYSTEM_TRAY_MESSAGE_DATA =
      XInternAtom(disp, "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);

   /* Acquire selection */
   if (systray_sel_win != None)
     {
	DialogOK(_("Systray Error!"), _("Only one systray is allowed"));
	return;
     }
   systray_sel_win = ECreateEventWindow(VRoot.win, -100, -100, 1, 1);
   systray_sel_time = EGetTimestamp();
   XSetSelectionOwner(disp, _NET_SYSTEM_TRAY_Sx, systray_sel_win,
		      systray_sel_time);
   if (XGetSelectionOwner(disp, _NET_SYSTEM_TRAY_Sx) != systray_sel_win)
     {
	DialogOK(_("Systray Error!"), _("Could not activate systray"));
	Eprintf("Failed to acquire selection %s\n", buf);
	EDestroyWindow(systray_sel_win);
	systray_sel_win = None;
	return;
     }
   if (EventDebug(EDBUG_TYPE_ICONBOX))
      Eprintf("Window %#lx is now %s owner, time=%ld\n",
	      systray_sel_win, buf, systray_sel_time);

   ESelectInput(systray_sel_win,
		SubstructureRedirectMask | SubstructureNotifyMask);
   EventCallbackRegister(systray_sel_win, 0, SystrayEvent, ib);

   ESelectInputAdd(win, SubstructureRedirectMask | SubstructureNotifyMask);
   EventCallbackRegister(win, 0, SystrayEvent, ib);

   ecore_x_client_message32_send(VRoot.win, E_XA_MANAGER, StructureNotifyMask,
				 CurrentTime, _NET_SYSTEM_TRAY_Sx,
				 systray_sel_win, 0, 0);
}

static void
SystrayExit(Iconbox * ib)
{
   XSetSelectionOwner(disp, _NET_SYSTEM_TRAY_Sx, None, systray_sel_time);
   EventCallbackUnregister(systray_sel_win, 0, SystrayEvent, ib);
   EventCallbackUnregister(ib->win, 0, SystrayEvent, ib);
   EDestroyWindow(systray_sel_win);
   systray_sel_win = None;
}
