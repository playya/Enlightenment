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
#include "backgrounds.h"
#include "desktops.h"
#include "dialog.h"
#include "emodule.h"
#include "ewins.h"
#include "groups.h"
#include "hints.h"
#include "hiwin.h"
#include "iclass.h"
#include "menus.h"
#include "tooltips.h"
#include "xwin.h"

#define DEBUG_PAGER 0

#define EwinGetVX(ew) (ew->vx)
#define EwinGetVY(ew) (ew->vy)
#define EwinGetVX2(ew) (ew->vx + EoGetW(ew))
#define EwinGetVY2(ew) (ew->vy + EoGetH(ew))

static struct
{
   char                enable;
   char                zoom;
   char                title;
   char                hiq;
   char                snap;
   int                 scanspeed;
   int                 sel_button;
   int                 win_button;
   int                 menu_button;
} Conf_pagers;

static struct
{
   int                 zoom;
} Mode_pagers;

typedef struct
{
   char               *name;
   Window              win;
   Pixmap              pmap;
   Pixmap              bgpmap;
   Desk               *dsk;
   int                 w, h;
   int                 dw, dh;
   int                 update_phase;
   EWin               *ewin;
   Window              sel_win;

   /* State flags */
   char                scan_pending;
   char                do_newbg;
   char                do_update;
   int                 x1, y1, x2, y2;
} Pager;

static void         PagerScanCancel(Pager * p);
static void         PagerScanTimeout(int val, void *data);
static void         PagerUpdateTimeout(int val, void *data);
static void         PagerCheckUpdate(Pager * p);
static void         PagerEwinUpdateFromPager(Pager * p, EWin * ewin);
static void         PagerHiwinHide(void);
static void         PagerEwinGroupSet(void);
static void         PagerEvent(XEvent * ev, void *prm);
static void         PagerHiwinEvent(XEvent * ev, void *prm);

static char         pager_update_pending = 0;

static Hiwin       *hiwin = NULL;

static Pager       *
PagerCreate(void)
{
   Pager              *p;

   if (!Conf_pagers.enable)
      return NULL;

   p = Ecalloc(1, sizeof(Pager));
   p->name = NULL;
   p->win = ECreateWindow(VRoot.win, 0, 0, 1, 1, 0);
   EventCallbackRegister(p->win, 0, PagerEvent, p);
   p->sel_win = ECreateWindow(p->win, 0, 0, 1, 1, 0);

   return p;
}

static void
PagerDestroy(Pager * p)
{
   RemoveItem("PAGER", p->win, LIST_FINDBY_ID, LIST_TYPE_PAGER);
   PagerScanCancel(p);
   if (p->name)
      Efree(p->name);
   EDestroyWindow(p->win);
   PagerHiwinHide();
   if (p->pmap != None)
      EFreePixmap(p->pmap);
   if (p->bgpmap != None)
      EFreePixmap(p->bgpmap);

   Efree(p);
}

static void
ScaleRect(Window src, Pixmap dst, Pixmap * pdst, int sx, int sy, int sw, int sh,
	  int dx, int dy, int dw, int dh, int scale)
{
   Imlib_Image        *im;
   Pixmap              pmap, mask;

   scale = (scale) ? 2 : 1;

   imlib_context_set_drawable(src);
   im = imlib_create_scaled_image_from_drawable(None, sx, sy, sw, sh,
						scale * dw, scale * dh, 0, 0);
   imlib_context_set_image(im);
   imlib_context_set_anti_alias(1);
   if (pdst)
     {
	imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask, dw, dh);
	*pdst = pmap;
     }
   else
     {
	imlib_context_set_drawable(dst);
	imlib_render_image_on_drawable_at_size(dx, dy, dw, dh);
     }
   imlib_free_image();
}

static void
PagerScanTrig(Pager * p)
{
   char                s[128];

   if (p->scan_pending || Conf_pagers.scanspeed <= 0)
      return;

   Esnprintf(s, sizeof(s), "pg-scan.%x", (unsigned)p->win);
   DoIn(s, 1 / ((double)Conf_pagers.scanspeed), PagerScanTimeout, 0, p);
   p->scan_pending = 1;
}

static void
PagerScanCancel(Pager * p)
{
   char                s[128];

   if (!p->scan_pending)
      return;

   Esnprintf(s, sizeof(s), "pg-scan.%x", (unsigned)p->win);
   RemoveTimerEvent(s);
}

static void
PagerScanTimeout(int val __UNUSED__, void *data)
{
   Pager              *p;
   EWin               *ewin;
   int                 y, y2, phase, cx, cy, ww, hh, xx, yy;
   static int          offsets[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

   if (!Conf_pagers.snap)
      return;

   p = (Pager *) data;
   p->scan_pending = 0;

   ewin = p->ewin;
   if (!ewin || !EoIsShown(ewin))
      return;
   if (p->dsk != DesksGetCurrent())
      return;
   if (ewin->state.visibility == VisibilityFullyObscured)
      return;

   if (Conf_pagers.scanspeed > 0)
      PagerScanTrig(p);

   if (Mode.mode != MODE_NONE)
      return;

   DeskCurrentGetArea(&cx, &cy);
   ww = p->dw;
   hh = p->dh;
   xx = cx * ww;
   yy = cy * hh;
   phase = p->update_phase;
   if (ww <= 0 || hh <= 0)
      return;

#if 0
   /* Due to a bug in imlib2 <= 1.2.0 we have to scan left->right in stead
    * of top->bottom, at least for now. */
   y = ((phase & 0xfffffff8) + offsets[phase % 8]) % hh;
   y2 = (y * VRoot.h) / hh;

   ScaleRect(VRoot.win, p->pmap, NULL, 0, y2, VRoot.w, VRoot.h / hh,
	     xx, yy + y, ww, 1, Conf_pagers.hiq);
   EClearArea(p->win, xx, yy + y, ww, 1, False);
   y2 = p->h;
#else
   y = ((phase & 0xfffffff8) + offsets[phase % 8]) % ww;
   y2 = (y * VRoot.w) / ww;

   ScaleRect(VRoot.win, p->pmap, NULL, y2, 0, VRoot.w / ww, VRoot.h,
	     xx + y, yy, 1, hh, Conf_pagers.hiq);
   EClearArea(p->win, xx + y, yy, 1, hh, False);
   y2 = p->w;
#endif
   p->update_phase++;
   if (p->update_phase >= y2)
     {
	int                 i, num;
	EWin               *const *lst;

	lst = EwinListGetForDesk(&num, p->dsk);
	for (i = 0; i < num; i++)
	   PagerEwinUpdateFromPager(p, lst[i]);

	p->update_phase = 0;
     }
}

#if 0				/* FIXME - Remove? */
static void
PagerHiwinUpdate(Hiwin * phi, Pager * p __UNUSED__, EWin * ewin)
{
   Imlib_Image        *im;

   if (!EoIsShown(phi) || !ewin->mini_pmm.pmap)
      return;

   imlib_context_set_drawable(ewin->mini_pmm.pmap);
   im = imlib_create_image_from_drawable(0, 0, 0,
					 ewin->mini_w, ewin->mini_h, 0);
   imlib_context_set_image(im);
   imlib_context_set_drawable(EoGetWin(phi));
   imlib_render_image_on_drawable_at_size(0, 0, EoGetW(phi), EoGetH(phi));
   imlib_free_image_and_decache();
}
#endif

static void
PagerEwinUpdateMini(Pager * p, EWin * ewin)
{
   int                 w, h, use_iclass;
   Drawable            draw;

   w = (EoGetW(ewin) * p->dw) / VRoot.w;
   h = (EoGetH(ewin) * p->dh) / VRoot.h;

   if (w < 1)
      w = 1;
   if (h < 1)
      h = 1;

   if ((ewin->mini_w == w) && (ewin->mini_h == h))
      return;

   FreePmapMask(&ewin->mini_pmm);

   ewin->mini_w = w;
   ewin->mini_h = h;

   draw = None;
   if (Conf_pagers.snap)
     {
	draw = EoGetPixmap(ewin);
	if (draw == None && EwinIsOnScreen(ewin))
	   draw = EoGetWin(ewin);
     }
   use_iclass = draw == None;

   if (use_iclass)
     {
	ImageClass         *ic;

	ic = ImageclassFind("PAGER_WIN", 0);
	if (ic)
	  {
	     ewin->mini_pmm.type = 0;
	     ewin->mini_pmm.mask = None;
	     ewin->mini_pmm.pmap =
		ImageclassApplySimple(ic, p->win, None, STATE_NORMAL,
				      0, 0, w, h);
	  }
     }
   else
     {
	ewin->mini_pmm.type = 1;
	ewin->mini_pmm.mask = None;
	ScaleRect(draw, None, &ewin->mini_pmm.pmap, 0, 0,
		  EoGetW(ewin), EoGetH(ewin), 0, 0, w, h, Conf_pagers.hiq);
     }

#if 0				/* FIXME - Remove? */
   if (hiwin && ewin == hiwin->ewin)
      PagerHiwinUpdate(hiwin, p, ewin);
#endif
}

static void
doPagerUpdate(Pager * p)
{
   int                 x, y, ax, ay, cx, cy, vx, vy;
   GC                  gc;
   EWin               *const *lst;
   int                 i, num, update_screen_included, update_screen_only;

   p->update_phase = 0;
   DesksGetAreaSize(&ax, &ay);
   DeskGetArea(p->dsk, &cx, &cy);
   vx = cx * VRoot.w;
   vy = cy * VRoot.h;

   gc = ECreateGC(p->pmap, 0, NULL);
   if (gc == None)
      return;

   update_screen_included = update_screen_only = 0;
   if (Conf_pagers.snap && p->dsk == DesksGetCurrent())
     {
	/* Update from screen unless update area is entirely off-screen */
	if (!(p->x2 <= vx || p->y2 <= vy ||
	      p->x1 >= vx + VRoot.w || p->y1 >= vy + VRoot.h))
	   update_screen_included = 1;

	/* Check if update area is entirely on-screen */
	if (p->x1 >= vx && p->y1 >= vy &&
	    p->x2 <= vx + VRoot.w && p->y2 <= vy + VRoot.h)
	   update_screen_only = 1;
     }

   if (update_screen_only)
      goto do_screen_update;

   for (y = 0; y < ay; y++)
     {
	for (x = 0; x < ax; x++)
	  {
#if 0				/* Skip? */
	     if (update_screen_included && x == cx && y == cy)
		continue;
#endif
	     XCopyArea(disp, p->bgpmap, p->pmap, gc, 0, 0, p->dw, p->dh,
		       x * p->dw, y * p->dh);
	  }
     }

   lst = EwinListGetForDesk(&num, p->dsk);
   for (i = num - 1; i >= 0; i--)
     {
	EWin               *ewin;
	int                 wx, wy, ww, wh;

	ewin = lst[i];
	if (!EoIsShown(ewin))
	   continue;

	wx = (EwinGetVX(ewin) * p->dw) / VRoot.w;
	wy = (EwinGetVY(ewin) * p->dh) / VRoot.h;
	ww = (EoGetW(ewin) * p->dw) / VRoot.w;
	wh = (EoGetH(ewin) * p->dh) / VRoot.h;

	PagerEwinUpdateMini(p, ewin);

	if (ewin->mini_pmm.pmap)
	  {
	     if (ewin->mini_pmm.mask)
	       {
		  XSetClipMask(disp, gc, ewin->mini_pmm.mask);
		  XSetClipOrigin(disp, gc, wx, wy);
	       }
	     XCopyArea(disp, ewin->mini_pmm.pmap, p->pmap, gc, 0, 0,
		       ww, wh, wx, wy);
	     if (ewin->mini_pmm.mask)
		XSetClipMask(disp, gc, None);
	  }
	else
	  {
	     XSetForeground(disp, gc, BlackPixel(disp, VRoot.scr));
	     XDrawRectangle(disp, p->pmap, gc, wx - 1, wy - 1, ww + 1, wh + 1);
	     XSetForeground(disp, gc, WhitePixel(disp, VRoot.scr));
	     XFillRectangle(disp, p->pmap, gc, wx, wy, ww, wh);
	  }
     }

   if (!update_screen_included)
     {
	EClearWindow(p->win);
	goto done;
     }

 do_screen_update:
   /* Update pager area by snapshotting entire screen */
   ScaleRect(VRoot.win, p->pmap, NULL, 0, 0, VRoot.w, VRoot.h, cx * p->dw,
	     cy * p->dh, p->dw, p->dh, Conf_pagers.hiq);
   p->update_phase = 0;

   EClearWindow(p->win);

   /* Update ewin snapshots */
   lst = EwinListGetForDesk(&num, p->dsk);
   for (i = 0; i < num; i++)
      PagerEwinUpdateFromPager(p, lst[i]);

 done:
   p->x1 = p->y1 = 99999;
   p->x2 = p->y2 = -99999;

   EFreeGC(gc);
}

static void
PagerUpdate(Pager * p, int x1, int y1, int x2, int y2)
{
   if (p->x1 > x1)
      p->x1 = x1;
   if (p->y1 > y1)
      p->y1 = y1;
   if (p->x2 < x2)
      p->x2 = x2;
   if (p->y2 < y2)
      p->y2 = y2;

   p->do_update = 1;
   pager_update_pending = 1;

   if (!Conf_pagers.snap)
      return;

   RemoveTimerEvent("pg-upd");
   DoIn("pg-upd", .2, PagerUpdateTimeout, 0, NULL);
}

static void
PagerReconfigure(Pager * p)
{
   EWin               *ewin;
   int                 ax, ay;
   double              aspect;

   ewin = p->ewin;

   DesksGetAreaSize(&ax, &ay);

   aspect = ((double)VRoot.w) / ((double)VRoot.h);
   ICCCM_SetSizeConstraints(ewin, 10 * ax, 8 * ay, 320 * ax, 240 * ay,
			    0, 0, 4 * ax, 8 * ay,
			    aspect * ((double)ax / (double)ay),
			    aspect * ((double)ax / (double)ay));
}

static void
PagerUpdateBg(Pager * p)
{
   Pixmap              pmap;
   GC                  gc;
   Background         *bg;

   p->x1 = p->y1 = 0;
   p->x2 = p->y2 = 99999;

   pmap = p->bgpmap;
   if (pmap != None)
      EFreePixmap(pmap);
   pmap = p->bgpmap = ECreatePixmap(p->win, p->dw, p->dh, VRoot.depth);

   if (!Conf_pagers.snap)
     {
	ImageClass         *ic;

	ic = ImageclassFind("PAGER_BACKGROUND", 0);
	if (ic)
	   ImageclassApplySimple(ic, p->win, pmap, STATE_NORMAL,
				 0, 0, p->dw, p->dh);
	return;
     }

   bg = DeskGetBackground(p->dsk);
   if (bg)
     {
	char                s[4096];
	char               *uniq;
	Imlib_Image        *im;

	uniq = BackgroundGetUniqueString(bg);
	Esnprintf(s, sizeof(s), "%s/cached/pager/%s.%i.%i.%s.png",
		  EDirUserCache(), BackgroundGetName(bg), p->dw, p->dh, uniq);
	Efree(uniq);

	im = imlib_load_image(s);
	if (im)
	  {
	     imlib_context_set_image(im);
	     imlib_context_set_drawable(pmap);
	     imlib_render_image_on_drawable_at_size(0, 0, p->dw, p->dh);
	     imlib_free_image_and_decache();
	  }
	else
	  {
	     BackgroundApply(bg, pmap, p->dw, p->dh, 0);
	     imlib_context_set_drawable(pmap);
	     im = imlib_create_image_from_drawable(0, 0, 0, p->dw, p->dh, 1);
	     imlib_context_set_image(im);
	     imlib_image_set_format("png");
	     imlib_save_image(s);
	     imlib_free_image_and_decache();
	  }
	return;
     }

   gc = ECreateGC(pmap, 0, NULL);
   if (gc == None)
      return;

   XSetForeground(disp, gc, BlackPixel(disp, VRoot.scr));
   XDrawRectangle(disp, pmap, gc, 0, 0, p->dw, p->dh);
   XSetForeground(disp, gc, WhitePixel(disp, VRoot.scr));
   XFillRectangle(disp, pmap, gc, 1, 1, p->dw - 2, p->dh - 2);

   EFreeGC(gc);
}

static void
PagerEwinMoveResize(EWin * ewin, int resize __UNUSED__)
{
   Pager              *p = ewin->data;
   int                 w, h;
   int                 ax, ay, cx, cy;
   ImageClass         *ic;

   if (!Conf_pagers.enable || !p || Mode.mode != MODE_NONE)
      return;

   w = ewin->client.w;
   h = ewin->client.h;
   if ((w == p->w && h == p->h) || w <= 1 || h <= 1)
      return;

   DesksGetAreaSize(&ax, &ay);

   if (p->pmap != None)
      EFreePixmap(p->pmap);

   p->w = w;
   p->h = h;
   p->dw = w / ax;
   p->dh = h / ay;

   p->pmap = ECreatePixmap(p->win, p->w, p->h, VRoot.depth);
   ESetWindowBackgroundPixmap(p->win, p->pmap);
   p->do_newbg = 1;
   PagerCheckUpdate(p);

   ic = ImageclassFind("PAGER_SEL", 0);
   if (ic)
     {
	DeskGetArea(p->dsk, &cx, &cy);
	EMoveResizeWindow(p->sel_win, cx * p->dw, cy * p->dh, p->dw, p->dh);
	ImageclassApply(ic, p->sel_win, p->dw, p->dh, 0, 0, STATE_NORMAL, 0,
			ST_PAGER);
     }
}

static void
PagerEwinClose(EWin * ewin)
{
   PagerDestroy(ewin->data);
   ewin->data = NULL;
}

static void
PagerEwinInit(EWin * ewin, void *ptr)
{
   ewin->data = ptr;

   ewin->MoveResize = PagerEwinMoveResize;
   ewin->Close = PagerEwinClose;

   ewin->props.skip_ext_task = 1;
   ewin->props.skip_ext_pager = 1;
   ewin->props.skip_focuslist = 1;
   ewin->props.skip_winlist = 1;
   EwinInhSetWM(ewin, focus, 1);
   ewin->props.autosave = 1;

   EoSetSticky(ewin, 1);
}

static void
PagerShow(Pager * p)
{
   EWin               *ewin = NULL;
   char                s[128];
   int                 w, h;

   if (!Conf_pagers.enable)
      return;

   if (p->ewin)
     {
	ShowEwin(p->ewin);
	return;
     }

   Esnprintf(s, sizeof(s), "%i", p->dsk->num);
   HintsSetWindowClass(p->win, s, "Enlightenment_Pager");

   ewin =
      AddInternalToFamily(p->win, "PAGER", EWIN_TYPE_PAGER, p, PagerEwinInit);
   if (!ewin)
      return;

   p->ewin = ewin;

   PagerReconfigure(p);

   ewin->client.event_mask |=
      ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
   ESelectInput(p->win, ewin->client.event_mask);

   w = ewin->client.w;
   h = ewin->client.h;

   EwinMoveToDesktop(ewin, EoGetDesk(ewin));
   if (ewin->state.placed)
     {
	EwinMoveResize(ewin, EoGetX(ewin), EoGetY(ewin), w, h);
     }
   else
     {
	/* no snapshots ? first time ? make a row on the bottom left up */
	int                 ax, ay;

	DesksGetAreaSize(&ax, &ay);
	w = ((48 * VRoot.w) / VRoot.h) * ax;
	h = 48 * ay;
	EwinResize(ewin, w, h);	/* Does layout */
	EwinMove(ewin, 0,
		 VRoot.h - (DesksGetNumber() - p->dsk->num) * EoGetH(ewin));
     }

   ShowEwin(ewin);

   AddItem(p, "PAGER", p->win, LIST_TYPE_PAGER);
}

static Pager      **
PagersForDesktop(Desk * dsk, int *num)
{
   Pager             **pp = NULL;
   Pager             **pl = NULL;
   int                 i, pnum;

   if (!Conf_pagers.enable)
      return NULL;

   *num = 0;
   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   if (!pl)
      return NULL;

   for (i = 0; i < pnum; i++)
     {
	if (pl[i]->dsk == dsk)
	  {
	     (*num)++;
	     pp = Erealloc(pp, sizeof(Pager *) * (*num));
	     pp[(*num) - 1] = pl[i];
	  }
     }

   Efree(pl);

   return pp;
}

static void
PagersUpdate(Desk * dsk, int x1, int y1, int x2, int y2)
{
   Pager             **pl;
   int                 i, num;

   pl = PagersForDesktop(dsk, &num);
   if (!pl)
      return;

   for (i = 0; i < num; i++)
      PagerUpdate(pl[i], x1, y1, x2, y2);

   Efree(pl);
}

static void
PagerCheckUpdate(Pager * p)
{
   if (p->do_newbg)
     {
	PagerUpdateBg(p);
	p->do_update = 1;
     }

   if (p->do_update)
      doPagerUpdate(p);

   p->do_newbg = p->do_update = 0;
}

static void
PagersCheckUpdate(void)
{
   Pager             **pl;
   int                 i, num;

   if (!pager_update_pending || !Conf_pagers.enable)
      return;
   if (Mode.mode != MODE_NONE && Conf_pagers.snap)
      return;

   pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
   if (!pl)
      return;

   for (i = 0; i < num; i++)
      PagerCheckUpdate(pl[i]);

   pager_update_pending = 0;

   Efree(pl);
}

static void
PagerUpdateTimeout(int val __UNUSED__, void *data __UNUSED__)
{
   PagersCheckUpdate();
}

static void
PagerEwinUpdateFromPager(Pager * p, EWin * ewin)
{
   int                 x, y, w, h;
   static GC           gc = 0;

   if (!EoIsShown(ewin) || !EwinIsOnScreen(ewin))
      return;

   x = EwinGetVX(ewin);
   y = EwinGetVY(ewin);
   w = EoGetW(ewin);
   h = EoGetH(ewin);
   x = (x * p->dw) / VRoot.w;
   y = (y * p->dh) / VRoot.h;
   w = (w * p->dw) / VRoot.w;
   h = (h * p->dh) / VRoot.h;
   if (w <= 0)
      w = 1;
   if (h <= 0)
      h = 1;

   if (!gc)
      gc = ECreateGC(p->pmap, 0, NULL);

   /* NB! If the pixmap/mask was created by imlib, free it. Due to imlibs */
   /*     image/pixmap cache it may be in use elsewhere. */
   if (ewin->mini_pmm.pmap &&
       ((ewin->mini_pmm.type) || (ewin->mini_w != w) || (ewin->mini_h != h)))
      FreePmapMask(&ewin->mini_pmm);

   if (!ewin->mini_pmm.pmap)
     {
	ewin->mini_w = w;
	ewin->mini_h = h;
	ewin->mini_pmm.type = 0;
	ewin->mini_pmm.pmap = ECreatePixmap(p->win, w, h, VRoot.depth);
	ewin->mini_pmm.mask = None;
     }

   if (!ewin->mini_pmm.pmap)
      return;

   XCopyArea(disp, p->pmap, ewin->mini_pmm.pmap, gc, x, y, w, h, 0, 0);

#if 0				/* FIXME - Remove? */
   if (hiwin && ewin == hiwin->ewin)
      PagerHiwinUpdate(hiwin, p, ewin);
#endif
}

static void
PagersUpdateEwin(EWin * ewin, int gone)
{
   Desk               *dsk;

   if (!Conf_pagers.enable)
      return;

   if (!gone && (!EoIsShown(ewin) || ewin->state.animated))
      return;

   if (gone && ewin == HiwinGetEwin(hiwin, 0))
      PagerHiwinHide();

   dsk = (EoIsFloating(ewin)) ? DesksGetCurrent() : EoGetDesk(ewin);
   PagersUpdate(dsk, EwinGetVX(ewin), EwinGetVY(ewin),
		EwinGetVX2(ewin), EwinGetVY2(ewin));
}

static EWin        *
EwinInPagerAt(Pager * p, int px, int py)
{
   EWin               *const *lst, *ewin;
   int                 i, num, x, y, w, h;

   if (!Conf_pagers.enable)
      return NULL;

   lst = EwinListGetForDesk(&num, p->dsk);
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];
	if (!EoIsShown(ewin))
	   continue;

	x = (EwinGetVX(ewin) * p->dw) / VRoot.w;
	y = (EwinGetVY(ewin) * p->dh) / VRoot.h;
	w = (EoGetW(ewin) * p->dw) / VRoot.w;
	h = (EoGetH(ewin) * p->dh) / VRoot.h;

	if (px >= x && py >= y && px < (x + w) && py < (y + h))
	   return ewin;
     }

   return NULL;
}

static void
PagerMenuShow(Pager * p, int x, int y)
{
   static Menu        *p_menu = NULL, *pw_menu = NULL;
   MenuItem           *mi;
   EWin               *ewin;
   char                s[1024];

   if (!Conf_pagers.enable)
      return;

   ewin = EwinInPagerAt(p, x, y);
   if (ewin)
     {
	if (pw_menu)
	   MenuDestroy(pw_menu);

	pw_menu =
	   MenuCreate("__DESK_WIN_MENU", _("Window Options"), NULL, NULL);

	Esnprintf(s, sizeof(s), "wop %#lx ic", _EwinGetClientXwin(ewin));
	mi = MenuItemCreate(_("Iconify"), NULL, s, NULL);
	MenuAddItem(pw_menu, mi);

	Esnprintf(s, sizeof(s), "wop %#lx close", _EwinGetClientXwin(ewin));
	mi = MenuItemCreate(_("Close"), NULL, s, NULL);
	MenuAddItem(pw_menu, mi);

	Esnprintf(s, sizeof(s), "wop %#lx kill", _EwinGetClientXwin(ewin));
	mi = MenuItemCreate(_("Annihilate"), NULL, s, NULL);
	MenuAddItem(pw_menu, mi);

	Esnprintf(s, sizeof(s), "wop %#lx st", _EwinGetClientXwin(ewin));
	mi = MenuItemCreate(_("Stick / Unstick"), NULL, s, NULL);
	MenuAddItem(pw_menu, mi);

	EFunc(NULL, "menus show __DESK_WIN_MENU");
	return;
     }

   if (p_menu)
      MenuDestroy(p_menu);
   p_menu = MenuCreate("__DESK_MENU", _("Desktop Options"), NULL, NULL);

   mi = MenuItemCreate(_("Pager Settings..."), NULL, "pg cfg", NULL);
   MenuAddItem(p_menu, mi);

   if (Conf_pagers.snap)
     {
	mi = MenuItemCreate(_("Snapshotting Off"), NULL, "pg snap off", NULL);
	MenuAddItem(p_menu, mi);

	if (Conf_pagers.hiq)
	   mi = MenuItemCreate(_("High Quality Off"), NULL, "pg hiq off", NULL);
	else
	   mi = MenuItemCreate(_("High Quality On"), NULL, "pg hiq on", NULL);
	MenuAddItem(p_menu, mi);
     }
   else
     {
	mi = MenuItemCreate(_("Snapshotting On"), NULL, "pg snap on", NULL);
	MenuAddItem(p_menu, mi);
     }
   if (Conf_pagers.zoom)
      mi = MenuItemCreate(_("Zoom Off"), NULL, "pg zoom off", NULL);
   else
      mi = MenuItemCreate(_("Zoom On"), NULL, "pg zoom on", NULL);
   MenuAddItem(p_menu, mi);

   EFunc(NULL, "menus show __DESK_MENU");
}

static void
PagerClose(Pager * p)
{
   HideEwin(p->ewin);
}

static void
UpdatePagerSel(void)
{
   Pager             **pl;
   Pager              *p;
   int                 i, pnum, cx, cy;
   ImageClass         *ic;

   if (!Conf_pagers.enable)
      return;

   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   if (!pl)
      return;

   for (i = 0; i < pnum; i++)
     {
	p = pl[i];
	if (p->dsk != DesksGetCurrent())
	   EUnmapWindow(p->sel_win);
	else
	  {
	     DeskGetArea(p->dsk, &cx, &cy);
	     EMoveWindow(p->sel_win, cx * p->dw, cy * p->dh);
	     EMapWindow(p->sel_win);
	     ic = ImageclassFind("PAGER_SEL", 0);
	     if (ic)
		ImageclassApply(ic, p->sel_win, p->dw, p->dh, 0, 0,
				STATE_NORMAL, 0, ST_PAGER);
	  }
     }
   Efree(pl);
}

static void
PagerShowTt(EWin * ewin)
{
   static EWin        *tt_ewin = NULL;
   ToolTip            *tt;

#if DEBUG_PAGER
   Eprintf("PagerShowTt %s\n", (ewin) ? EwinGetIconName(ewin) : NULL);
#endif

   if (!Conf_pagers.title || (ewin == tt_ewin))
      return;

   if (MenusActive())		/* Don't show Tooltip when menu is up */
      return;

   tt = FindItem("PAGER", 0, LIST_FINDBY_NAME, LIST_TYPE_TOOLTIP);
   if (tt)
     {
	if (ewin)
	   TooltipShow(tt, EwinGetIconName(ewin), NULL, Mode.events.x,
		       Mode.events.y);
	else
	   TooltipHide(tt);
     }

   tt_ewin = ewin;
}

static void
PagerHiwinInit(Pager * p, EWin * ewin)
{
   Hiwin              *phi = hiwin;
   int                 wx, wy, ww, wh, px, py;

   if (!phi)
     {
	phi = HiwinCreate();
	if (!phi)
	   return;
	hiwin = phi;
     }

   wx = (EwinGetVX(ewin) * p->dw) / VRoot.w;
   wy = (EwinGetVY(ewin) * p->dh) / VRoot.h;
   ww = (EoGetW(ewin) * p->dw) / VRoot.w;
   wh = (EoGetH(ewin) * p->dh) / VRoot.h;
   ETranslateCoordinates(p->win, VRoot.win, 0, 0, &px, &py, NULL);

   HiwinInit(phi, ewin);
   HiwinSetGeom(phi, px + wx, py + wy, ww, wh);
   HiwinSetCallback(phi, PagerHiwinEvent, p);
}

static void
PagerHiwinHide(void)
{
#if DEBUG_PAGER
   Eprintf("PagerHiwinHide\n");
#endif
   HiwinHide(hiwin);
   PagerShowTt(NULL);
}

static void
PagerHiwinShow(Pager * p, EWin * ewin, int zoom, int confine)
{
   Hiwin              *phi = hiwin;

   if (MenusActive())		/* Don't show HiWin when menu is up */
      return;

   if (!phi || ewin)
     {
	PagerHiwinInit(p, ewin);
	phi = hiwin;
	if (!phi)
	   return;
     }

   HiwinShow(phi, ewin, zoom, confine);
}

static void
PagerZoomChange(Pager * p, int delta)
{
#if DEBUG_PAGER
   Eprintf("PagerZoomChange delta=%d\n", delta);
#endif

   if (delta == 0)
      return;

   if (delta > 0)
     {
	if (Mode_pagers.zoom >= 8)
	   return;
	Mode_pagers.zoom++;
     }
   else
     {
	if (Mode_pagers.zoom <= 2)
	   return;
	Mode_pagers.zoom--;
     }
   PagerHiwinShow(p, NULL, Mode_pagers.zoom, 0);
}

static void
PagerHandleMotion(Pager * p, int x, int y)
{
   EWin               *ewin;

   if (!Conf_pagers.enable)
      return;

   EQueryPointer(p->win, &x, &y, NULL, NULL);

   if (x >= 0 && x < p->w && y >= 0 && y < p->h)
      ewin = EwinInPagerAt(p, x, y);
   else
      ewin = NULL;

   if (!Conf_pagers.zoom)
     {
	PagerShowTt(ewin);
	return;
     }

   if (!ewin || EoGetLayer(ewin) <= 0)
     {
	PagerHiwinHide();
     }
   else if (!hiwin || ewin != HiwinGetEwin(hiwin, 0))
     {
	if (Mode_pagers.zoom < 2)
	   Mode_pagers.zoom = 2;
	PagerHiwinShow(p, ewin, Mode_pagers.zoom, 0);
     }
   if (Mode_pagers.zoom <= 2)
      PagerShowTt(ewin);
}

static void
NewPagerForDesktop(Desk * dsk)
{
   Pager              *p;
   char                s[128];

   p = PagerCreate();
   if (!p)
      return;

   p->dsk = dsk;
   Esnprintf(s, sizeof(s), "Pager-%i", dsk->num);
   HintsSetWindowName(p->win, s);
   PagerShow(p);
}

static void
PagersUpdateBackground(Desk * dsk)
{
   Pager             **pl;
   int                 i, num;

   if (dsk)
      pl = PagersForDesktop(dsk, &num);
   else
      pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
   if (!pl)
      return;

   for (i = 0; i < num; i++)
      pl[i]->do_newbg = 1;

   Efree(pl);

   pager_update_pending = 1;
}

static void
PagerSetHiQ(char onoff)
{
   EWin               *const *lst;
   int                 i, num;

   Conf_pagers.hiq = onoff;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	lst[i]->mini_w = 0;
	lst[i]->mini_h = 0;
     }

   PagersUpdateBackground(NULL);

   autosave();
}

static void
PagerSetSnap(char onoff)
{
   Pager             **pl;
   EWin               *const *lst;
   int                 i, num;

   Conf_pagers.snap = onoff;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	lst[i]->mini_w = 0;
	lst[i]->mini_h = 0;
     }

   PagersUpdateBackground(NULL);

   if (Conf_pagers.snap)
     {
	pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
	if (!pl)
	   return;

	for (i = 0; i < num; i++)
	  {
	     if (Conf_pagers.scanspeed > 0 && pl[i]->dsk == DesksGetCurrent())
		PagerScanTrig(pl[i]);
	  }

	Efree(pl);
     }

   autosave();
}

/*
 * Pager event handlers
 */

static int         *gwin_px, *gwin_py;

static void
PagerEwinGroupSet(void)
{
   int                 i, num;
   EWin               *ewin, **gwins;

   ewin = HiwinGetEwin(hiwin, 0);
   gwins = ListWinGroupMembersForEwin(ewin, GROUP_ACTION_MOVE,
				      Mode.nogroup, &num);
   if (!gwins)
      return;

   gwin_px = Emalloc(num * sizeof(int));
   gwin_py = Emalloc(num * sizeof(int));

   for (i = 0; i < num; i++)
     {
	gwin_px[i] = EoGetX(gwins[i]);
	gwin_py[i] = EoGetY(gwins[i]);
     }

   Efree(gwins);
}

static void
PagerEwinGroupUnset(void)
{
   _EFREE(gwin_px);
   _EFREE(gwin_py);
}

static void
PagerEventUnmap(Pager * p __UNUSED__)
{
   PagerHiwinHide();
}

static void
EwinGroupMove(EWin * ewin, Desk * dsk, int x, int y)
{
   int                 i, num, dx, dy, newdesk;
   EWin              **gwins;

   if (!ewin)
      return;

   /* Move all group members */
   newdesk = dsk != EoGetDesk(ewin);
   dx = x - EoGetX(ewin);
   dy = y - EoGetY(ewin);
   gwins =
      ListWinGroupMembersForEwin(ewin, GROUP_ACTION_MOVE, Mode.nogroup, &num);
   for (i = 0; i < num; i++)
     {
	if (gwins[i]->type == EWIN_TYPE_PAGER)
	   continue;

	if (newdesk)
	   EwinMoveToDesktopAt(gwins[i], dsk, EoGetX(gwins[i]) + dx,
			       EoGetY(gwins[i]) + dy);
	else
	   EwinMove(gwins[i], EoGetX(gwins[i]) + dx, EoGetY(gwins[i]) + dy);
     }
   if (gwins)
      Efree(gwins);
}

static void
PagerEwinMove(Pager * p __UNUSED__, Pager * pd, EWin * ewin)
{
   int                 x, y, dx, dy, px, py;
   int                 cx, cy;
   Hiwin              *phi = hiwin;

   /* Delta in pager coords */
   dx = Mode.events.x - Mode.events.px;
   dy = Mode.events.y - Mode.events.py;

   if (dx == 0 && dy == 0)
      return;

   /* Move mini window */
   HiwinGetXY(phi, &x, &y);
   HiwinMove(phi, x + dx, y + dy);

   /* Find real window position */
   ETranslateCoordinates(VRoot.win, pd->win, x, y, &px, &py, NULL);
   DeskGetArea(pd->dsk, &cx, &cy);
   x = (px * VRoot.w) / pd->dw - cx * VRoot.w;
   y = (py * VRoot.h) / pd->dh - cy * VRoot.h;

   /* Move all group members */
   EwinGroupMove(ewin, pd->dsk, x, y);
}

static void
PagerHandleMouseDown(Pager * p, int px, int py, int button)
{
   int                 in_pager;
   EWin               *ewin;

   in_pager = (px >= 0 && py >= 0 && px < p->w && py < p->h);
   if (!in_pager)
      return;

   if (button == Conf_pagers.menu_button)
     {
	PagerHiwinHide();
	PagerMenuShow(p, px, py);
     }
   else if (button == Conf_pagers.win_button)
     {
	ewin = EwinInPagerAt(p, px, py);
	if ((ewin) && (ewin->type != EWIN_TYPE_PAGER))
	  {
	     PagerHiwinShow(p, ewin, 1, !Mode.wm.window);
	     Mode.mode = MODE_PAGER_DRAG_PENDING;
	     PagerEwinGroupSet();
	  }
     }
}

static void
PagerHiwinHandleMouseDown(Pager * p, int px, int py, int button)
{
   PagerHandleMouseDown(p, px, py, button);
}

static void
PagerHandleMouseUp(Pager * p, int px, int py, int button)
{
   int                 in_pager;
   EWin               *ewin;

   in_pager = (px >= 0 && py >= 0 && px < p->w && py < p->h);
   if (!in_pager)
      return;

   if (button == Conf_pagers.sel_button)
     {
	DeskGoto(p->dsk);
	if (p->dsk != DesksGetCurrent())
	   SoundPlay("SOUND_DESKTOP_SHUT");
	DeskCurrentGotoArea(px / p->dw, py / p->dh);
     }
   else if (button == Conf_pagers.win_button)
     {
	DeskGoto(p->dsk);
	DeskCurrentGotoArea(px / p->dw, py / p->dh);
	ewin = EwinInPagerAt(p, px, py);
	if (ewin)
	  {
	     RaiseEwin(ewin);
	     FocusToEWin(ewin, FOCUS_SET);
	  }
     }
}

static void
PagerHiwinHandleMouseUp(Pager * p, int px, int py, int button)
{
   int                 i, num, in_vroot;
   EWin               *ewin, *ewin2, **gwins;
   int                 x, y;

#if DEBUG_PAGER
   Eprintf("PagerHiwinHandleMouseUp m=%d d=%d x,y=%d,%d\n", Mode.mode,
	   p->dsk, px, py);
#endif

   if (Mode.mode != MODE_PAGER_DRAG)
     {
	if (Mode.mode == MODE_PAGER_DRAG_PENDING)
	   Mode.mode = MODE_NONE;
	PagerHiwinHide();
	PagerHandleMouseUp(p, px, py, button);
	return;
     }

   Mode.mode = MODE_NONE;

   ewin = HiwinGetEwin(hiwin, 1);
   if (!ewin)
      goto done;

   in_vroot = (Mode.events.x >= 0 && Mode.events.x < VRoot.w &&
	       Mode.events.y >= 0 && Mode.events.y < VRoot.h);

   if (button == Conf_pagers.win_button)
     {
	/* Find which pager or iconbox we are in (if any) */
	ewin2 = GetEwinPointerInClient();
	if ((ewin2) && (ewin2->type == EWIN_TYPE_PAGER))
	  {
	     PagerEwinMove(p, ewin2->data, ewin);
	  }
	else if ((ewin2) && (ewin2->type == EWIN_TYPE_ICONBOX))
	  {
	     /* Pointer is in iconbox */

	     /* Iconify after moving back to pre-drag position */
	     gwins = ListWinGroupMembersForEwin(ewin, GROUP_ACTION_MOVE,
						Mode.nogroup, &num);
	     for (i = 0; i < num; i++)
	       {
		  if (gwins[i]->type != EWIN_TYPE_PAGER)
		    {
		       EwinMove(gwins[i], gwin_px[i], gwin_py[i]);
		       EwinIconify(gwins[i]);
		    }
	       }
	     if (gwins)
		Efree(gwins);
	  }
	else if (ewin2 && ewin2->props.vroot)
	  {
	     /* Dropping onto virtual root */
	     EwinReparent(ewin, _EwinGetClientXwin(ewin2));
	  }
	else if (!in_vroot)
	  {
	     /* Move back to real root */
	     EwinReparent(ewin, RRoot.win);
	  }
	else
	  {
	     /* Pointer is not in pager or iconbox */
	     /* Move window(s) to pointer location */
	     x = Mode.events.x - EoGetW(ewin) / 2;
	     y = Mode.events.y - EoGetH(ewin) / 2;
	     EwinGroupMove(ewin, DesksGetCurrent(), x, y);
	  }
     }

   PagerHiwinHide();

 done:
   /* unallocate the space that was holding the old positions of the */
   /* windows */
   PagerEwinGroupUnset();
}

static void
PagerEvent(XEvent * ev, void *prm)
{
   Pager              *p = (Pager *) prm;

#if DEBUG_PAGER
   Eprintf("PagerEvent ev=%d\n", ev->type);
#endif

   switch (ev->type)
     {
     case ButtonPress:
	PagerHandleMouseDown(p, ev->xbutton.x, ev->xbutton.y,
			     (int)ev->xbutton.button);
	break;
     case ButtonRelease:
	if (ev->xbutton.window != Mode.events.last_bpress)
	   break;
	PagerHandleMouseUp(p, ev->xbutton.x, ev->xbutton.y,
			   (int)ev->xbutton.button);
	break;

     case MotionNotify:
	PagerHandleMotion(p, ev->xmotion.x, ev->xmotion.y);
	break;

     case EnterNotify:
	break;
     case LeaveNotify:
	PagerShowTt(NULL);
	break;

     case UnmapNotify:
	PagerEventUnmap(p);
	break;

     case VisibilityNotify:
	if (ev->xvisibility.state != VisibilityFullyObscured)
	   PagerScanTrig(p);
	break;
     }
}

static void
PagerHiwinEvent(XEvent * ev, void *prm)
{
   Pager              *p = prm;
   int                 px, py;
   EWin               *ewin;

   if (!p)
      return;

#if DEBUG_PAGER
   Eprintf("PagerHiwinEvent ev=%d\n", ev->type);
#endif

   switch (ev->type)
     {
     case ButtonPress:
	switch (ev->xbutton.button)
	  {
	  case 4:
	     if (Mode.mode != MODE_NONE)
		break;
	     PagerZoomChange(p, 1);
	     break;
	  case 5:
	     if (Mode.mode != MODE_NONE)
		break;
	     PagerZoomChange(p, -1);
	     break;
	  default:
	     /* Translate x,y to pager window coordinates */
	     ETranslateCoordinates(ev->xbutton.window, p->win,
				   ev->xbutton.x, ev->xbutton.y, &px, &py,
				   NULL);
	     PagerHiwinHandleMouseDown(p, px, py, (int)ev->xbutton.button);
	     break;
	  }
	break;

     case ButtonRelease:
	switch (ev->xbutton.button)
	  {
	  case 4:
	  case 5:
	     break;
	  default:
	     /* Translate x,y to pager window coordinates */
	     ETranslateCoordinates(ev->xbutton.window, p->win,
				   ev->xbutton.x, ev->xbutton.y, &px, &py,
				   NULL);
	     PagerHiwinHandleMouseUp(p, px, py, (int)ev->xbutton.button);
	     break;
	  }
	break;

     case MotionNotify:
	switch (Mode.mode)
	  {
	  case MODE_NONE:
	     PagerHandleMotion(p, ev->xmotion.x, ev->xmotion.y);
	     break;

	  case MODE_PAGER_DRAG_PENDING:
	  case MODE_PAGER_DRAG:
	     ewin = HiwinGetEwin(hiwin, 1);
	     if (!ewin || ewin->type == EWIN_TYPE_PAGER)
	       {
		  Mode.mode = MODE_NONE;
		  break;
	       }

	     Mode.mode = MODE_PAGER_DRAG;
	     PagerEwinMove(p, p, ewin);
	     break;
	  }
	break;

     case LeaveNotify:
	PagerShowTt(NULL);
	break;
     }
}

/*
 * Pagers handling
 */

static void
PagersEnableForDesktop(Desk * dsk)
{
   Pager             **pl;
   int                 num;

   pl = PagersForDesktop(dsk, &num);
   if (!pl)
      NewPagerForDesktop(dsk);
   else
      Efree(pl);
}

static void
PagersDisableForDesktop(Desk * dsk)
{
   Pager             **pl;

   int                 i, num;

   pl = PagersForDesktop(dsk, &num);
   if (!pl)
      return;

   for (i = 0; i < num; i++)
      PagerClose(pl[i]);
   Efree(pl);
}

static void
PagersShow(int enable)
{
   unsigned int        i;

   if (enable && !Conf_pagers.enable)
     {
	Conf_pagers.enable = 1;
	for (i = 0; i < DesksGetNumber(); i++)
	   PagersEnableForDesktop(DeskGet(i));
	UpdatePagerSel();
     }
   else if (!enable && Conf_pagers.enable)
     {
	for (i = 0; i < DesksGetNumber(); i++)
	   PagersDisableForDesktop(DeskGet(i));
	Conf_pagers.enable = 0;
     }
}

static void
PagersReconfigure(void)
{
   Pager             **pl, *p;
   int                 i, num;

   if (!Conf_pagers.enable)
      return;

   pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
   if (!pl)
      return;

   for (i = 0; i < num; i++)
     {
	p = pl[i];
	PagerReconfigure(p);
	EwinResize(p->ewin, p->ewin->client.w, p->ewin->client.h);
     }
   Efree(pl);
}

/*
 * Configuration dialog
 */
static char         tmp_show_pagers;
static char         tmp_pager_hiq;
static char         tmp_pager_snap;
static char         tmp_pager_zoom;
static char         tmp_pager_title;
static char         tmp_pager_do_scan;
static int          tmp_pager_scan_speed;
static int          tmp_pager_sel_button;
static int          tmp_pager_win_button;
static int          tmp_pager_menu_button;
static DItem       *pager_scan_speed_label = NULL;
static Dialog      *pager_settings_dialog = NULL;

static void
CB_ConfigurePager(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	PagersShow(tmp_show_pagers);
	if (Conf_pagers.hiq != tmp_pager_hiq)
	   PagerSetHiQ(tmp_pager_hiq);
	Conf_pagers.zoom = tmp_pager_zoom;
	Conf_pagers.title = tmp_pager_title;
	Conf_pagers.sel_button = tmp_pager_sel_button;
	Conf_pagers.win_button = tmp_pager_win_button;
	Conf_pagers.menu_button = tmp_pager_menu_button;
	if ((Conf_pagers.scanspeed != tmp_pager_scan_speed)
	    || ((!tmp_pager_do_scan) && (Conf_pagers.scanspeed > 0))
	    || ((tmp_pager_do_scan) && (Conf_pagers.scanspeed == 0)))
	  {
	     if (tmp_pager_do_scan)
		Conf_pagers.scanspeed = tmp_pager_scan_speed;
	     else
		Conf_pagers.scanspeed = 0;
	     PagerSetSnap(tmp_pager_snap);
	  }
	if (Conf_pagers.snap != tmp_pager_snap)
	   PagerSetSnap(tmp_pager_snap);
     }
   autosave();
}

static void
CB_PagerScanSlide(Dialog * d __UNUSED__, int val __UNUSED__,
		  void *data __UNUSED__)
{
   char                s[256];

   Esnprintf(s, sizeof(s), "%s %03i %s", _("Pager scanning speed:"),
	     tmp_pager_scan_speed, _("lines per second"));
   DialogItemSetText(pager_scan_speed_label, s);
   DialogDrawItems(pager_settings_dialog, pager_scan_speed_label, 0, 0, 99999,
		   99999);
}

static void
SettingsPager(void)
{
   Dialog             *d;
   DItem              *table, *di, *radio;
   char                s[256];

   d = FindItem("CONFIGURE_PAGER", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG);
   if (d)
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_PAGER");

   tmp_show_pagers = Conf_pagers.enable;
   tmp_pager_hiq = Conf_pagers.hiq;
   tmp_pager_snap = Conf_pagers.snap;
   tmp_pager_zoom = Conf_pagers.zoom;
   tmp_pager_title = Conf_pagers.title;
   tmp_pager_sel_button = Conf_pagers.sel_button;
   tmp_pager_win_button = Conf_pagers.win_button;
   tmp_pager_menu_button = Conf_pagers.menu_button;
   if (Conf_pagers.scanspeed == 0)
      tmp_pager_do_scan = 0;
   else
      tmp_pager_do_scan = 1;
   tmp_pager_scan_speed = Conf_pagers.scanspeed;

   d = pager_settings_dialog = DialogCreate("CONFIGURE_PAGER");
   DialogSetTitle(d, _("Pager Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   if (Conf.dialogs.headers)
      DialogAddHeader(d, "pix/pager.png",
		      _("Enlightenment Desktop & Area\n"
			"Pager Settings Dialog\n"));

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Enable pager display"));
   DialogItemCheckButtonSetPtr(di, &tmp_show_pagers);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Make miniature snapshots of the screen"));
   DialogItemCheckButtonSetPtr(di, &tmp_pager_snap);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Smooth high quality snapshots in snapshot mode"));
   DialogItemCheckButtonSetPtr(di, &tmp_pager_hiq);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Zoom in on pager windows when mouse is over them"));
   DialogItemCheckButtonSetPtr(di, &tmp_pager_zoom);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _("Pop up window title when mouse is over the window"));
   DialogItemCheckButtonSetPtr(di, &tmp_pager_title);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Continuously scan screen to update pager"));
   DialogItemCheckButtonSetPtr(di, &tmp_pager_do_scan);

   di = pager_scan_speed_label = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   Esnprintf(s, sizeof(s), "%s %03i %s", _("Pager scanning speed:"),
	     tmp_pager_scan_speed, _("lines per second"));
   DialogItemSetText(di, s);

   di = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSliderSetBounds(di, 1, 256);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 1);
   DialogItemSetColSpan(di, 2);
   DialogItemSliderSetValPtr(di, &tmp_pager_scan_speed);
   DialogItemSetCallback(di, CB_PagerScanSlide, 0, NULL);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Mouse button to select and drag windows:"));

   radio = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Left"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Middle"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 2);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Right"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 3);
   DialogItemRadioButtonGroupSetValPtr(radio, &tmp_pager_win_button);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Mouse button to select desktops:"));

   radio = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Left"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Middle"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 2);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Right"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 3);
   DialogItemRadioButtonGroupSetValPtr(radio, &tmp_pager_sel_button);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Mouse button to display pager menu:"));

   radio = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Left"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Middle"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 2);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Right"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 3);
   DialogItemRadioButtonGroupSetValPtr(radio, &tmp_pager_menu_button);

   DialogAddFooter(d, DLG_OAC, CB_ConfigurePager);

   ShowDialog(d);
}

/*
 * Pagers Module
 */

static void
PagersSighan(int sig, void *prm)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	memset(&Mode_pagers, 0, sizeof(Mode_pagers));
	EDirMake(EDirUserCache(), "cached/pager");
	break;
     case ESIGNAL_CONFIGURE:
	break;
     case ESIGNAL_START:
	if (!Conf_pagers.enable)
	   break;
	Conf_pagers.enable = 0;
	PagersShow(1);
	PagersCheckUpdate();
	break;

     case ESIGNAL_IDLE:
	PagersCheckUpdate();
	break;

     case ESIGNAL_AREA_CONFIGURED:
	PagersReconfigure();
	break;
     case ESIGNAL_AREA_SWITCH_START:
	PagerHiwinHide();
	break;
     case ESIGNAL_AREA_SWITCH_DONE:
	PagersUpdate(DesksGetCurrent(), 0, 0, 99999, 99999);
	UpdatePagerSel();
	break;

     case ESIGNAL_DESK_ADDED:
	NewPagerForDesktop(prm);
	break;
     case ESIGNAL_DESK_REMOVED:
	PagersDisableForDesktop(prm);
	break;
     case ESIGNAL_DESK_SWITCH_START:
	PagerHiwinHide();
	break;
     case ESIGNAL_DESK_SWITCH_DONE:
	UpdatePagerSel();
	break;
     case ESIGNAL_DESK_RESIZE:
	PagersReconfigure();
	break;

     case ESIGNAL_BACKGROUND_CHANGE:
	PagersUpdateBackground(prm);
	break;

     case ESIGNAL_EWIN_UNMAP:
	PagersUpdateEwin(prm, 1);
	break;
     case ESIGNAL_EWIN_CHANGE:
	PagersUpdateEwin(prm, 0);
	break;
     }
}

static void
IPC_Pager(const char *params, Client * c __UNUSED__)
{
   const char         *p = params;
   char                prm1[128];
   int                 len, desk;
   Desk               *dsk;

   if (!p)
      return;

   prm1[0] = '\0';
   len = 0;
   sscanf(p, "%100s %n", prm1, &len);
   p += len;

   if (!strncmp(prm1, "cfg", 3))
     {
	SettingsPager();
     }
   else if (!strcmp(prm1, "on"))
     {
	PagersShow(1);
     }
   else if (!strcmp(prm1, "off"))
     {
	PagersShow(0);
     }
   else if (!strcmp(prm1, "desk"))
     {
	desk = -1;
	prm1[0] = '\0';
	sscanf(p, "%d %100s", &desk, prm1);
	dsk = DeskGet(desk);

	if (!dsk)
	  {
	     ;
	  }
	else if (!strcmp(prm1, "on"))
	  {
	     PagersEnableForDesktop(dsk);
	  }
	else if (!strcmp(prm1, "new"))
	  {
	     NewPagerForDesktop(dsk);
	  }
	else if (!strcmp(prm1, "off"))
	  {
	     PagersDisableForDesktop(dsk);
	  }
     }
   else if (!strcmp(prm1, "hiq"))
     {
	if (!strcmp(p, "on"))
	   PagerSetHiQ(1);
	else if (!strcmp(p, "off"))
	   PagerSetHiQ(0);
     }
   else if (!strcmp(prm1, "snap"))
     {
	if (!strcmp(p, "on"))
	   PagerSetSnap(1);
	else if (!strcmp(p, "off"))
	   PagerSetSnap(0);
     }
   else if (!strcmp(prm1, "zoom"))
     {
	if (!strcmp(p, "on"))
	   Conf_pagers.zoom = 1;
	else if (!strcmp(p, "off"))
	   Conf_pagers.zoom = 0;
     }
}

static const IpcItem PagersIpcArray[] = {
   {
    IPC_Pager,
    "pager", "pg",
    "Toggle the status of the Pager and various pager settings",
    "use \"pager <on/off>\" to set the current mode\nuse \"pager ?\" "
    "to get the current mode\n"
    "  pager <#> <on/off/?>   Toggle or test any desktop's pager\n"
    "  pager cfg              Configure pagers\n"
    "  pager hiq <on/off>     Toggle high quality pager\n"
    "  pager scanrate <#>     Toggle number of line updates per second\n"
    "  pager snap <on/off>    Toggle snapshotting in the pager\n"
    "  pager title <on/off>   Toggle title display in the pager\n"
    "  pager zoom <on/off>    Toggle zooming in the pager\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(PagersIpcArray)/sizeof(IpcItem))

/*
 * Configuration items
 */
static const CfgItem PagersCfgItems[] = {
   CFG_ITEM_BOOL(Conf_pagers, enable, 1),
   CFG_ITEM_BOOL(Conf_pagers, zoom, 1),
   CFG_ITEM_BOOL(Conf_pagers, title, 1),
   CFG_ITEM_BOOL(Conf_pagers, hiq, 1),
   CFG_ITEM_BOOL(Conf_pagers, snap, 1),
   CFG_ITEM_INT(Conf_pagers, scanspeed, 10),
   CFG_ITEM_INT(Conf_pagers, sel_button, 2),
   CFG_ITEM_INT(Conf_pagers, win_button, 1),
   CFG_ITEM_INT(Conf_pagers, menu_button, 3),
};
#define N_CFG_ITEMS (sizeof(PagersCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
EModule             ModPagers = {
   "pagers", "pg",
   PagersSighan,
   {N_IPC_FUNCS, PagersIpcArray},
   {N_CFG_ITEMS, PagersCfgItems}
};
