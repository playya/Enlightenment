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
#define DECLARE_STRUCT_BUTTON
#include "E.h"
#include <time.h>
#include <sys/time.h>

char               *
GetUniqueBGString(Background * bg)
{
   char                s[256];
   const char         *chmap =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
   int                 r, g, b;
   int                 n1, n2, n3, n4, n5, f1, f2, f3, f4, f5, f6;

   EGetColor(&(bg->bg_solid), &r, &g, &b);
   n1 = (r << 24) | (g << 16) | (b << 8) | (bg->bg_tile << 7)
      | (bg->bg.keep_aspect << 6) | (bg->top.keep_aspect << 5);
   n2 = (bg->bg.xjust << 16) | (bg->bg.yjust);
   n3 = (bg->bg.xperc << 16) | (bg->bg.yperc);
   n4 = (bg->top.xjust << 16) | (bg->top.yjust);
   n5 = (bg->top.xperc << 16) | (bg->top.yperc);
   f1 = 0;
   f2 = 0;
   f3 = 0;
   f4 = 0;
   f5 = 0;
   f6 = 0;
   if (bg->bg.file)
     {
	char               *f;

	f = FindFile(bg->bg.file);
	if (f)
	  {
	     f1 = fileinode(f);
	     f2 = filedev(f);
	     f3 = (int)moddate(f);
	     Efree(f);
	  }
     }
   if (bg->top.file)
     {
	char               *f;

	f = FindFile(bg->top.file);
	if (f)
	  {
	     f4 = fileinode(f);
	     f5 = filedev(f);
	     f6 = (int)moddate(f);
	     Efree(f);
	  }
     }
   Esnprintf(s, sizeof(s),
	     "%c%c%c%c%c%c" "%c%c%c%c%c%c" "%c%c%c%c%c%c" "%c%c%c%c%c%c"
	     "%c%c%c%c%c%c" "%c%c%c%c%c%c" "%c%c%c%c%c%c" "%c%c%c%c%c%c"
	     "%c%c%c%c%c%c" "%c%c%c%c%c%c" "%c%c%c%c%c%c",
	     chmap[(n1 >> 0) & 0x3f], chmap[(n1 >> 6) & 0x3f],
	     chmap[(n1 >> 12) & 0x3f], chmap[(n1 >> 18) & 0x3f],
	     chmap[(n1 >> 24) & 0x3f], chmap[(n1 >> 28) & 0x3f],
	     chmap[(n2 >> 0) & 0x3f], chmap[(n2 >> 6) & 0x3f],
	     chmap[(n2 >> 12) & 0x3f], chmap[(n2 >> 18) & 0x3f],
	     chmap[(n2 >> 24) & 0x3f], chmap[(n2 >> 28) & 0x3f],
	     chmap[(n3 >> 0) & 0x3f], chmap[(n3 >> 6) & 0x3f],
	     chmap[(n3 >> 12) & 0x3f], chmap[(n3 >> 18) & 0x3f],
	     chmap[(n3 >> 24) & 0x3f], chmap[(n3 >> 28) & 0x3f],
	     chmap[(n4 >> 0) & 0x3f], chmap[(n4 >> 6) & 0x3f],
	     chmap[(n4 >> 12) & 0x3f], chmap[(n4 >> 18) & 0x3f],
	     chmap[(n4 >> 24) & 0x3f], chmap[(n4 >> 28) & 0x3f],
	     chmap[(n5 >> 0) & 0x3f], chmap[(n5 >> 6) & 0x3f],
	     chmap[(n5 >> 12) & 0x3f], chmap[(n5 >> 18) & 0x3f],
	     chmap[(n5 >> 24) & 0x3f], chmap[(n5 >> 28) & 0x3f],
	     chmap[(f1 >> 0) & 0x3f], chmap[(f1 >> 6) & 0x3f],
	     chmap[(f1 >> 12) & 0x3f], chmap[(f1 >> 18) & 0x3f],
	     chmap[(f1 >> 24) & 0x3f], chmap[(f1 >> 28) & 0x3f],
	     chmap[(f2 >> 0) & 0x3f], chmap[(f2 >> 6) & 0x3f],
	     chmap[(f2 >> 12) & 0x3f], chmap[(f2 >> 18) & 0x3f],
	     chmap[(f2 >> 24) & 0x3f], chmap[(f2 >> 28) & 0x3f],
	     chmap[(f3 >> 0) & 0x3f], chmap[(f3 >> 6) & 0x3f],
	     chmap[(f3 >> 12) & 0x3f], chmap[(f3 >> 18) & 0x3f],
	     chmap[(f3 >> 24) & 0x3f], chmap[(f3 >> 28) & 0x3f],
	     chmap[(f4 >> 0) & 0x3f], chmap[(f4 >> 6) & 0x3f],
	     chmap[(f4 >> 12) & 0x3f], chmap[(f4 >> 18) & 0x3f],
	     chmap[(f4 >> 24) & 0x3f], chmap[(f4 >> 28) & 0x3f],
	     chmap[(f5 >> 0) & 0x3f], chmap[(f5 >> 6) & 0x3f],
	     chmap[(f5 >> 12) & 0x3f], chmap[(f5 >> 18) & 0x3f],
	     chmap[(f5 >> 24) & 0x3f], chmap[(f5 >> 28) & 0x3f],
	     chmap[(f6 >> 0) & 0x3f], chmap[(f6 >> 6) & 0x3f],
	     chmap[(f6 >> 12) & 0x3f], chmap[(f6 >> 18) & 0x3f],
	     chmap[(f6 >> 24) & 0x3f], chmap[(f6 >> 28) & 0x3f]);
   return Estrdup(s);
}

void
ChangeNumberOfDesktops(int quantity)
{
   int                 pnum, i, num;
   EWin              **lst;

   pnum = conf.desks.num;
   for (i = quantity; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
      LowerDesktop(i);
   conf.desks.num = quantity;

   if (conf.desks.num <= 0)
      conf.desks.num = 1;
   else if (conf.desks.num > ENLIGHTENMENT_CONF_NUM_DESKTOPS)
      conf.desks.num = ENLIGHTENMENT_CONF_NUM_DESKTOPS;

   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     if (lst[i]->desktop >= conf.desks.num)
		MoveEwinToDesktop(lst[i], conf.desks.num - 1);
	  }
	Efree(lst);
     }
   if (conf.desks.num > pnum)
     {
	for (i = pnum; i < conf.desks.num; i++)
	   NewPagerForDesktop(i);
     }
   else if (conf.desks.num < pnum)
     {
	for (i = conf.desks.num; i < pnum; i++)
	   DisablePagersForDesktop(i);
     }
   if (desks.current >= conf.desks.num)
      GotoDesktop(conf.desks.num - 1);

   HintsSetDesktopConfig();
}

void
ShowDesktopControls()
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 1);
   if ((blst) && (num > 0))
     {
	for (i = 0; i < num; i++)
	   ButtonShow(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}

void
ShowDesktopTabs()
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 2);
   if ((blst) && (num > 0))
     {
	for (i = 0; i < num; i++)
	   ButtonShow(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}

void
HideDesktopTabs()
{
   Button            **blst;
   int                 num, i;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 2);
   if ((blst) && (num > 0))
     {
	for (i = 0; i < num; i++)
	   ButtonHide(blst[i]);
	Efree(blst);
	StackDesktops();
     }
}

void
ShowDesktopButtons(void)
{
   Button            **blst;
   int                 i, num;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
   if (blst)
     {
	for (i = 0; i < num; i++)
	  {
	     if ((!blst[i]->internal) && (blst[i]->default_show))
		ButtonShow(blst[i]);
	  }
	Efree(blst);
     }
}

void
MoveToDeskTop(int num)
{
   int                 i, j;

   EDBUG(6, "MoveToDeskTop");
   j = -1;
   i = 0;
   while ((j < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
     {
	if (desks.order[i] == num)
	   j = i;
	i++;
     }
   if (j < 0)
      EDBUG_RETURN_;
   if (j > 0)
     {
	for (i = j - 1; i >= 0; i--)
	   desks.order[i + 1] = desks.order[i];
	desks.order[0] = num;
     }
   EDBUG_RETURN_;
}

void
MoveToDeskBottom(int num)
{
   int                 i, j;

   EDBUG(6, "MoveToDeskBottom");
   j = -1;
   i = 0;
   while ((j < 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
     {
	if (desks.order[i] == num)
	   j = i;
	i++;
     }
   if (j < 0)
      EDBUG_RETURN_;
   if (j < ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1)
     {
	for (i = j; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1; i++)
	   desks.order[i] = desks.order[i + 1];
	desks.order[ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1] = num;
     }
   EDBUG_RETURN_;
}

void
SlideWindowTo(Window win, int fx, int fy, int tx, int ty, int speed)
{
   int                 k, spd, x, y, min;
   struct timeval      timev1, timev2;
   int                 dsec, dusec;
   double              tm;

   EDBUG(5, "SlideWindowTo");
   spd = 16;
   min = 2;
   GrabX();
   for (k = 0; k <= 1024; k += spd)
     {
	gettimeofday(&timev1, NULL);
	x = ((fx * (1024 - k)) + (tx * k)) >> 10;
	y = ((fy * (1024 - k)) + (ty * k)) >> 10;
	EMoveWindow(disp, win, x, y);
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
   EMoveWindow(disp, win, tx, ty);
   UngrabX();
   EDBUG_RETURN_;
}

static void
FreeBGimages(Background * bg, int free_pmap)
{
   if (bg->bg.im)
     {
	imlib_context_set_image(bg->bg.im);
	imlib_free_image();
	bg->bg.im = NULL;
     }
   if (bg->top.im)
     {
	imlib_context_set_image(bg->top.im);
	imlib_free_image();
	bg->top.im = NULL;
     }
   if (free_pmap && bg->pmap)
     {
	imlib_free_pixmap_and_mask(bg->pmap);
	bg->pmap = 0;
     }
}

void
KeepBGimages(Background * bg, char onoff)
{
   if (onoff)
     {
	bg->keepim = 1;
     }
   else
     {
	bg->keepim = 0;
	FreeBGimages(bg, 0);
     }
}

void
RemoveImagesFromBG(Background * bg)
{
   if (bg->bg.file)
      Efree(bg->bg.file);
   bg->bg.file = NULL;

   if (bg->bg.real_file)
      Efree(bg->bg.real_file);
   bg->bg.real_file = NULL;

   FreeBGimages(bg, 1);

   bg->keepim = 0;
}

void
FreeDesktopBG(Background * bg)
{
   EDBUG(6, "FreeDesktopBG");

   if (!bg)
      EDBUG_RETURN_;

   if (bg->ref_count > 0)
     {
	DialogOK(_("Background Error!"), _("%u references remain\n"),
		 bg->ref_count);
	EDBUG_RETURN_;
     }

   RemoveImagesFromBG(bg);

   if (bg->name)
      Efree(bg->name);

   Efree(bg);

   EDBUG_RETURN_;
}

Background         *
CreateDesktopBG(char *name, XColor * solid, char *bg, char tile,
		char keep_aspect, int xjust, int yjust, int xperc,
		int yperc, char *top, char tkeep_aspect, int txjust,
		int tyjust, int txperc, int typerc)
{
   Background         *d;

   EDBUG(6, "CreateDesktopBG");

   d = Emalloc(sizeof(Background));
   if (!d)
      EDBUG_RETURN(NULL);
   d->name = Estrdup(name);
   d->pmap = 0;
   d->last_viewed = 0;

   ESetColor(&(d->bg_solid), 160, 160, 160);
   if (solid)
      d->bg_solid = *solid;
   d->bg.file = NULL;
   if (bg)
      d->bg.file = Estrdup(bg);
   d->bg.real_file = NULL;
   d->bg.im = NULL;
   d->bg_tile = tile;
   d->bg.keep_aspect = keep_aspect;
   d->bg.xjust = xjust;
   d->bg.yjust = yjust;
   d->bg.xperc = xperc;
   d->bg.yperc = yperc;

   d->top.file = NULL;
   if (top)
      d->top.file = Estrdup(top);
   d->top.real_file = NULL;
   d->top.im = NULL;
   d->top.keep_aspect = tkeep_aspect;
   d->top.xjust = txjust;
   d->top.yjust = tyjust;
   d->top.xperc = txperc;
   d->top.yperc = typerc;

   d->cmclass = NULL;
   d->keepim = 0;
   d->ref_count = 0;

   EDBUG_RETURN(d);
}

void
RefreshCurrentDesktop()
{
   EDBUG(5, "RefreshCurrentDesktop");
   RefreshDesktop(desks.current);
   EDBUG_RETURN_;
}

void
RefreshDesktop(int desk)
{
   Background         *dsk;

   EDBUG(4, "RefreshDesktop");

   desk = desk % ENLIGHTENMENT_CONF_NUM_DESKTOPS;
   if (!desks.desk[desk].viewable)
      EDBUG_RETURN_;

   dsk = desks.desk[desk].bg;
   if (!dsk)
      EDBUG_RETURN_;

   SetBackgroundTo(desks.desk[desk].win, dsk, 1);
   EDBUG_RETURN_;
}

static void
BgFindImageSize(BgPart * bgp, int rw, int rh, int *pw, int *ph, int setbg)
{
   int                 w, h;

   if (bgp->xperc > 0)
     {
	w = (rw * bgp->xperc) >> 10;
     }
   else
     {
	if (!setbg)
	   w = (imlib_image_get_width() * rw) / root.w;
	else
	   w = imlib_image_get_width();
     }

   if (bgp->yperc > 0)
     {
	h = (rh * bgp->yperc) >> 10;
     }
   else
     {
	if (!setbg)
	  {
	     h = (imlib_image_get_height() * rh) / root.h;
	  }
	else
	  {
	     h = imlib_image_get_height();
	  }
     }

   if (w <= 0)
      w = 1;
   if (h <= 0)
      h = 1;

   if (bgp->keep_aspect)
     {
	if (bgp->yperc <= 0)
	  {
	     if (((w << 10) / h) !=
		 ((imlib_image_get_width() << 10) / imlib_image_get_height()))
		h = ((w * imlib_image_get_height()) / imlib_image_get_width());
	  }
	else
	  {
	     if (((h << 10) / w) !=
		 ((imlib_image_get_height() << 10) / imlib_image_get_width()))
		w = ((h * imlib_image_get_width()) / imlib_image_get_height());
	  }
     }

   *pw = w;
   *ph = h;
}

void
SetBackgroundTo(Window win, Background * dsk, char setbg)
{
   unsigned int        rw, rh;
   Pixmap              dpmap;
   GC                  gc;
   XGCValues           gcv;
   int                 rt, depth;

   EDBUG(4, "SetBackgroundTo");

   if (!WinExists(win))
      EDBUG_RETURN_;

   IMLIB1_SET_CONTEXT(win == root.win);

   GetWinWH(win, &rw, &rh);
   depth = GetWinDepth(win);
   imlib_context_set_drawable(win);

   EAllocColor(&dsk->bg_solid);
   gc = 0;
   rt = imlib_context_get_dither();

   if (conf.backgrounds.hiquality)
     {
	imlib_context_set_dither(1);
#if 0				/* ??? */
	imlib_context_set_anti_alias(1);
#endif
     }

   dpmap = dsk->pmap;
   if (!setbg && dpmap)
     {
	/* Always regenerate if setting non-desktop window (?) */
	imlib_free_pixmap_and_mask(dpmap);
	dpmap = 0;
     }

   if (!dpmap)
     {
	unsigned int        w, h, x, y;
	char                hasbg, hasfg;
	Pixmap              pmap, mask;
	ColorModifierClass *cm;

	if (dsk->bg.file && !dsk->bg.im)
	  {
	     if (!dsk->bg.real_file)
		dsk->bg.real_file = FindFile(dsk->bg.file);
	     dsk->bg.im = ELoadImage(dsk->bg.real_file);
	  }

	if (dsk->top.file && !dsk->top.im)
	  {
	     if (!dsk->top.real_file)
		dsk->top.real_file = FindFile(dsk->top.file);
	     dsk->top.im = ELoadImage(dsk->top.real_file);
	  }

	cm = dsk->cmclass;
	if (cm)
	   cm->ref_count--;
	else
	   cm = (ColorModifierClass *) FindItem("BACKGROUND", 0,
						LIST_FINDBY_NAME,
						LIST_TYPE_COLORMODIFIER);

	if (cm)
	  {
	     cm->ref_count++;
#if !USE_IMLIB2
	     if (dsk->top.im)
	       {
		  Imlib_set_image_red_curve(pImlib_Context, dsk->top.im,
					    cm->red.map);
		  Imlib_set_image_green_curve(pImlib_Context, dsk->top.im,
					      cm->green.map);
		  Imlib_set_image_blue_curve(pImlib_Context, dsk->top.im,
					     cm->blue.map);
	       }
	     if (dsk->bg.im)
	       {
		  Imlib_set_image_red_curve(pImlib_Context, dsk->bg.im,
					    cm->red.map);
		  Imlib_set_image_green_curve(pImlib_Context, dsk->bg.im,
					      cm->green.map);
		  Imlib_set_image_blue_curve(pImlib_Context, dsk->bg.im,
					     cm->blue.map);
	       }
#endif
	  }

	hasbg = hasfg = 0;
	if (dsk->top.im)
	   hasfg = 1;
	if (dsk->bg.im)
	   hasbg = 1;

	w = h = x = y = 0;

	if (hasbg)
	  {
	     imlib_context_set_image(dsk->bg.im);

	     BgFindImageSize(&(dsk->bg), rw, rh, &w, &h, setbg);
	     x = ((rw - w) * dsk->bg.xjust) >> 10;
	     y = ((rh - h) * dsk->bg.yjust) >> 10;

	     imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask, w, h);
	  }

	if (hasbg && !hasfg && setbg && x == 0 && y == 0 && w == rw && h == rh)
	  {
	     /* Put image 1:1 onto the current root window */
	     dpmap = pmap;
	  }
	else if (hasbg && !hasfg && dsk->bg_tile && !conf.theme.transparency)
	  {
	     /* BG only, tiled */
	     dpmap = ECreatePixmap(disp, win, w, h, depth);
	     gc = XCreateGC(disp, dpmap, 0, &gcv);
	  }
	else
	  {
	     /* The rest that require some more work */
	     dpmap = ECreatePixmap(disp, win, rw, rh, depth);
	     gc = XCreateGC(disp, dpmap, 0, &gcv);
	     if (!dsk->bg_tile)
	       {
		  XSetForeground(disp, gc, dsk->bg_solid.pixel);
		  XFillRectangle(disp, dpmap, gc, 0, 0, rw, rh);
	       }
	  }

	if (hasbg && dpmap != pmap)
	  {
	     XSetTile(disp, gc, pmap);
	     XSetTSOrigin(disp, gc, x, y);
	     XSetFillStyle(disp, gc, FillTiled);
	     if (dsk->bg_tile)
		XFillRectangle(disp, dpmap, gc, 0, 0, rw, rh);
	     else
		XFillRectangle(disp, dpmap, gc, x, y, w, h);
	     IMLIB_FREE_PIXMAP_AND_MASK(pmap, mask);
	  }

	if (hasfg)
	  {
	     int                 ww, hh;

	     imlib_context_set_image(dsk->top.im);

	     BgFindImageSize(&(dsk->top), rw, rh, &ww, &hh, setbg);
	     x = ((rw - ww) * dsk->top.xjust) >> 10;
	     y = ((rh - hh) * dsk->top.yjust) >> 10;

	     imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask, ww, hh);
	     XSetTile(disp, gc, pmap);
	     XSetTSOrigin(disp, gc, x, y);
	     XSetFillStyle(disp, gc, FillTiled);
	     if (mask)
	       {
		  XSetClipMask(disp, gc, mask);
		  XSetClipOrigin(disp, gc, x, y);
	       }
	     XFillRectangle(disp, dpmap, gc, x, y, ww, hh);
	     IMLIB_FREE_PIXMAP_AND_MASK(pmap, mask);
	  }

	if (!dsk->keepim)
	   FreeBGimages(dsk, 0);
     }

   if (setbg)
     {
	if (dpmap)
	  {
	     HintsSetRootInfo(win, dpmap, 0);
	     XSetWindowBackgroundPixmap(disp, win, dpmap);
	  }
	else
	  {
	     HintsSetRootInfo(win, 0, dsk->bg_solid.pixel);
	     XSetWindowBackground(disp, win, dsk->bg_solid.pixel);
	  }
	XClearWindow(disp, win);
     }
   else
     {
	if (dpmap)
	  {
	     if (!gc)
		gc = XCreateGC(disp, dpmap, 0, &gcv);
	     XSetClipMask(disp, gc, 0);
	     XSetTile(disp, gc, dpmap);
	     XSetTSOrigin(disp, gc, 0, 0);
	     XSetFillStyle(disp, gc, FillTiled);
	     XFillRectangle(disp, win, gc, 0, 0, rw, rh);
	     imlib_free_pixmap_and_mask(dpmap);
	     dpmap = 0;
	  }
	else
	  {
	     if (!gc)
		gc = XCreateGC(disp, win, 0, &gcv);
	     XSetClipMask(disp, gc, 0);
	     XSetFillStyle(disp, gc, FillSolid);
	     XSetForeground(disp, gc, dsk->bg_solid.pixel);
	     XFillRectangle(disp, win, gc, 0, 0, rw, rh);
	  }
	XSync(disp, False);
     }
   dsk->pmap = dpmap;

   if (gc)
      XFreeGC(disp, gc);

   imlib_context_set_dither(rt);

   IMLIB1_SET_CONTEXT(0);

   EDBUG_RETURN_;
}

void
InitDesktopBgs()
{
   int                 i;
   Desk               *d;
   Atom                at;

   EDBUG(6, "InitDesktopBgs");
   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	d = &desks.desk[i];
	d->bg = NULL;
	desks.order[i] = i;
	d->tag = NULL;
	d->x = 0;
	d->y = 0;
	d->current_area_x = 0;
	d->current_area_y = 0;
	if (i == 0)
	  {
	     d->win = root.win;
	     d->viewable = 0;
	  }
	else
	  {
	     d->win =
		ECreateWindow(root.win, -root.w, -root.h, root.w, root.h, 0);
	     XSelectInput(disp, d->win,
			  SubstructureNotifyMask | ButtonPressMask |
			  ButtonReleaseMask | EnterWindowMask | LeaveWindowMask
			  | ButtonMotionMask | PropertyChangeMask |
			  SubstructureRedirectMask | KeyPressMask |
			  KeyReleaseMask | PointerMotionMask);
	     d->viewable = 0;
	  }
	at = XInternAtom(disp, "ENLIGHTENMENT_DESKTOP", False);
	XChangeProperty(disp, d->win, at, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *)&i, 1);
/* I don't believe it.. this property causes xv and Xscreensaver to barf
 * stupid bloody clients - I cant' believe peope write such shitty code
 */
/*      
 * at = XInternAtom(disp, "__SWM_VROOT", False);
 * XChangeProperty(disp, d->win, at, XA_CARDINAL, 32, PropModeReplace,
 * (unsigned char *)&i, 1);
 */
     }

   EDBUG_RETURN_;

}

void
InitDesktopControls()
{
   int                 i;
   ActionClass        *ac, *ac2, *ac3;
   ImageClass         *ic, *ic2, *ic3, *ic4;
   Button             *b;
   Action             *a;
   int                 x[3], y[3], w[3], h[3], m, n, o;
   char                s[512], *param;

   EDBUG(6, "InitDesktopControls");

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	Esnprintf(s, sizeof(s), "DRAGBAR_DESKTOP_%i", i);
	ac = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac)
	  {
	     ac = CreateAclass(s);
	     AddItem(ac, ac->name, 0, LIST_TYPE_ACLASS);
	     a = CreateAction(EVENT_MOUSE_DOWN, 0, 0, 0, 1, 0, NULL, NULL);
	     AddAction(ac, a);
	     param = Emalloc(3);
	     Esnprintf(param, 3, "%i", i);
	     AddToAction(a, ACTION_DESKTOP_DRAG, param);
	     a = CreateAction(EVENT_MOUSE_DOWN, 0, 0, 0, 3, 0, NULL, NULL);
	     AddAction(ac, a);
	     Esnprintf(s, sizeof(s), "deskmenu");
	     AddToAction(a, ACTION_SHOW_MENU, Estrdup(s));
	     a = CreateAction(EVENT_MOUSE_DOWN, 0, 0, 0, 2, 0, NULL, NULL);
	     AddAction(ac, a);
	     Esnprintf(s, sizeof(s), "taskmenu");
	     AddToAction(a, ACTION_SHOW_MENU, Estrdup(s));
	     if (i > 0)
	       {
		  ac->tooltipstring =
		     Estrdup(_
			     ("Hold down the mouse button and drag\n"
			      "the mouse to be able to drag the desktop\n"
			      "back and forth.\n"
			      "Click right mouse button for a list of all\n"
			      "Desktops and their applications.\n"
			      "Click middle mouse button for a list of all\n"
			      "applications currently running.\n"));
	       }
	     else
	       {
		  ac->tooltipstring =
		     Estrdup(_
			     ("This is the Root desktop.\n"
			      "You cannot drag the root desktop around.\n"
			      "Click right mouse button for a list of all\n"
			      "Desktops and their applications.\n"
			      "Click middle mouse button for a list of all\n"
			      "applications currently running.\n"));
	       }
	  }
	Esnprintf(s, sizeof(s), "RAISEBUTTON_DESKTOP_%i", i);
	ac2 = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac2)
	  {
	     ac2 = CreateAclass(s);
	     AddItem(ac2, ac2->name, 0, LIST_TYPE_ACLASS);
	     a = CreateAction(EVENT_MOUSE_UP, 1, 0, 1, 0, 0, NULL, NULL);
	     AddAction(ac2, a);
	     param = Emalloc(3);
	     Esnprintf(param, 3, "%i", i);
	     AddToAction(a, ACTION_DESKTOP_RAISE, param);
	     ac2->tooltipstring =
		Estrdup(_
			("Click here to raise this desktop\n" "to the top.\n"));
	  }
	Esnprintf(s, sizeof(s), "LOWERBUTTON_DESKTOP_%i", i);
	ac3 = FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	if (!ac3)
	  {
	     ac3 = CreateAclass(s);
	     AddItem(ac3, ac3->name, 0, LIST_TYPE_ACLASS);
	     a = CreateAction(EVENT_MOUSE_UP, 1, 0, 1, 0, 0, NULL, NULL);
	     AddAction(ac3, a);
	     param = Emalloc(3);
	     Esnprintf(param, 3, "%i", i);
	     AddToAction(a, ACTION_DESKTOP_LOWER, param);
	     ac3->tooltipstring =
		Estrdup(_
			("Click here to lower this desktop\n"
			 "to the bottom.\n"));
	  }
	b = NULL;

	if (conf.desks.dragdir < 2)
	  {
	     ic = FindItem("DESKTOP_DRAGBUTTON_VERT", 0, LIST_FINDBY_NAME,
			   LIST_TYPE_ICLASS);
	     ic2 =
		FindItem("DESKTOP_RAISEBUTTON_VERT", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	     ic3 =
		FindItem("DESKTOP_LOWERBUTTON_VERT", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	     ic4 =
		FindItem("DESKTOP_DESKRAY_VERT", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	  }
	else
	  {
	     ic = FindItem("DESKTOP_DRAGBUTTON_HORIZ", 0, LIST_FINDBY_NAME,
			   LIST_TYPE_ICLASS);
	     ic2 =
		FindItem("DESKTOP_RAISEBUTTON_HORIZ", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	     ic3 =
		FindItem("DESKTOP_LOWERBUTTON_HORIZ", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	     ic4 =
		FindItem("DESKTOP_DESKRAY_HORIZ", 0, LIST_FINDBY_NAME,
			 LIST_TYPE_ICLASS);
	  }

	switch (conf.desks.dragbar_ordering)
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

	switch (conf.desks.dragdir)
	  {
	  case 0:
	     w[0] = w[1] = w[2] = h[0] = h[1] = conf.desks.dragbar_width;
	     if (conf.desks.dragbar_length == 0)
		h[2] = root.h - (conf.desks.dragbar_width * 2);
	     else
		h[2] = conf.desks.dragbar_length;
	     x[0] = x[1] = x[2] = 0;
	     y[m] = 0;
	     y[n] = y[m] + h[m];
	     y[o] = y[n] + h[n];
	     break;
	  case 1:
	     w[0] = w[1] = w[2] = h[0] = h[1] = conf.desks.dragbar_width;
	     if (conf.desks.dragbar_length == 0)
		h[2] = root.h - (conf.desks.dragbar_width * 2);
	     else
		h[2] = conf.desks.dragbar_length;
	     x[0] = x[1] = x[2] = root.w - conf.desks.dragbar_width;
	     y[m] = 0;
	     y[n] = y[m] + h[m];
	     y[o] = y[n] + h[n];
	     break;
	  case 2:
	     h[0] = h[1] = h[2] = w[0] = w[1] = conf.desks.dragbar_width;
	     if (conf.desks.dragbar_length == 0)
		w[2] = root.w - (conf.desks.dragbar_width * 2);
	     else
		w[2] = conf.desks.dragbar_length;
	     y[0] = y[1] = y[2] = 0;
	     x[m] = 0;
	     x[n] = x[m] + w[m];
	     x[o] = x[n] + w[n];
	     break;
	  case 3:
	     h[0] = h[1] = h[2] = w[0] = w[1] = conf.desks.dragbar_width;
	     if (conf.desks.dragbar_length == 0)
		w[2] = root.w - (conf.desks.dragbar_width * 2);
	     else
		w[2] = conf.desks.dragbar_length;
	     y[0] = y[1] = y[2] = root.h - conf.desks.dragbar_width;
	     x[m] = 0;
	     x[n] = x[m] + w[m];
	     x[o] = x[n] + w[n];
	     break;
	  default:
	     break;
	  }

	if (conf.desks.dragbar_width > 0)
	  {
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", ic2, ac2, NULL, NULL, -1,
			      FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[0], 0,
			      y[0], 0, 0, w[0], 0, h[0], 0, i, 0);
	     AddItem(b, b->name, 1, LIST_TYPE_BUTTON);
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", ic3, ac3, NULL, NULL, -1,
			      FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[1], 0,
			      y[1], 0, 0, w[1], 0, h[1], 0, i, 0);
	     AddItem(b, b->name, 1, LIST_TYPE_BUTTON);
	     b = ButtonCreate("_DESKTOP_DRAG_CONTROL", ic, ac, NULL, NULL, -1,
			      FLAG_FIXED, 1, 99999, 1, 99999, 0, 0, x[2], 0,
			      y[2], 0, 0, w[2], 0, h[2], 0, i, 0);
	     AddItem(b, b->name, 1, LIST_TYPE_BUTTON);
	  }
	if (i > 0)
	  {
	     if (conf.desks.dragdir == 0)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_VERT, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y, 0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else if (conf.desks.dragdir == 1)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_VERT, 1, 99999, 1,
				   99999, 0, 0,
				   desks.desk[i].x + root.w -
				   conf.desks.dragbar_width, 0, desks.desk[i].y,
				   0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else if (conf.desks.dragdir == 2)
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_HORIZ, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y, 0, 0, 0, 0, 0, 1, 0, 1);
	       }
	     else
	       {
		  b = ButtonCreate("_DESKTOP_DESKRAY_DRAG_CONTROL", ic4, ac,
				   NULL, NULL, 1, FLAG_FIXED_HORIZ, 1, 99999, 1,
				   99999, 0, 0, desks.desk[i].x, 0,
				   desks.desk[i].y + root.h -
				   conf.desks.dragbar_width, 0, 0, 0, 0, 0, 1,
				   0, 1);
	       }
	     AddItem(b, b->name, 2, LIST_TYPE_BUTTON);
	     desks.desk[i].tag = b;
	  }
	else
	   desks.desk[i].tag = NULL;
     }
   EDBUG_RETURN_;
}

void
SetDesktopBg(int desk, Background * bg)
{
   EDBUG(5, "SetDesktopBg");

   if (desk < 0)
      EDBUG_RETURN_;
   if (desk >= ENLIGHTENMENT_CONF_NUM_DESKTOPS)
      EDBUG_RETURN_;

   if (desks.desk[desk].bg)
     {
	if (desks.desk[desk].bg != bg)
	  {
	     desks.desk[desk].bg->ref_count--;
	     if (desks.desk[desk].bg->ref_count < 1)
	       {
		  desks.desk[desk].bg->last_viewed = 0;
		  DesktopAccounting();
	       }
	     if (bg)
		bg->ref_count++;
	  }
     }
   desks.desk[desk].bg = bg;
   if (desks.desk[desk].viewable)
      RefreshDesktop(desk);
   if (desk == desks.current)
     {
	RedrawPagersForDesktop(desk, 2);
	ForceUpdatePagersForDesktop(desk);
     }
   else
      RedrawPagersForDesktop(desk, 1);
   EDBUG_RETURN_;
}

void
ConformEwinToDesktop(EWin * ewin)
{
   int                 xo, yo;

   EDBUG(3, "ConformEwinToDesktop");

   if ((ewin->iconified) && (ewin->parent != desks.desk[ewin->desktop].win))
     {
	ewin->parent = desks.desk[ewin->desktop].win;
	DesktopAddEwinToTop(ewin);
	EReparentWindow(disp, ewin->win, desks.desk[ewin->desktop].win, ewin->x,
			ewin->y);
	ICCCM_Configure(ewin);
	StackDesktops();
     }
   else if (ewin->floating)
     {
	xo = desks.desk[ewin->desktop].x;
	yo = desks.desk[ewin->desktop].y;
	if ((ewin->parent != root.win) && (ewin->floating == 2))
	  {
	     ewin->parent = root.win;
	     EReparentWindow(disp, ewin->win, root.win, ewin->x, ewin->y);
	     ewin->desktop = 0;
	  }
	XRaiseWindow(disp, ewin->win);
	ShowEdgeWindows();
	ICCCM_Configure(ewin);
     }
   else if (ewin->parent != desks.desk[ewin->desktop].win)
     {
	ewin->parent = desks.desk[ewin->desktop].win;
	DesktopAddEwinToTop(ewin);
	EReparentWindow(disp, ewin->win, desks.desk[ewin->desktop].win, ewin->x,
			ewin->y);
	StackDesktops();
	MoveEwin(ewin, ewin->x, ewin->y);
     }
   else
     {
	MoveEwin(ewin, ewin->x, ewin->y);
     }
   HintsSetWindowDesktop(ewin);
   EDBUG_RETURN_;
}

int
DesktopAt(int x, int y)
{
   int                 i;

   EDBUG(3, "DesktopAt");

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	if ((x >= desks.desk[desks.order[i]].x)
	    && (x < (desks.desk[desks.order[i]].x + root.w))
	    && (y >= desks.desk[desks.order[i]].y)
	    && (y < (desks.desk[desks.order[i]].y + root.h)))
	   EDBUG_RETURN(desks.order[i]);
     }

   EDBUG_RETURN(0);
}

static void
MoveStickyWindowsToCurrentDesk(void)
{
   EWin              **lst, *ewin, *last_ewin;
   int                 i, num;

   lst = EwinListGetStacking(&num);
   last_ewin = NULL;
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];
	if (!ewin->sticky)
	   continue;

	ewin->desktop = desks.current;
	ewin->parent = desks.desk[ewin->desktop].win;
	EReparentWindow(disp, ewin->win,
			desks.desk[ewin->desktop].win, root.w, root.h);
	EMoveWindow(disp, ewin->win, ewin->x, ewin->y);
	HintsSetWindowArea(ewin);
	HintsSetWindowDesktop(ewin);
	last_ewin = ewin;
     }
   if (last_ewin)
      RestackEwin(last_ewin);
}

void
GotoDesktop(int desk)
{
   int                 x, y, pdesk;

   EDBUG(2, "GotoDesktop");

   if (conf.desks.wraparound)
     {
	if (desk >= conf.desks.num)
	   desk = 0;
	else if (desk < 0)
	   desk = conf.desks.num - 1;
     }
   if (desk < 0 || desk >= conf.desks.num || desk == desks.current)
      EDBUG_RETURN_;

   pdesk = desks.current;

   SlideoutsHide();

   {
      ToolTip           **lst;
      int                 i, j;

      lst = (ToolTip **) ListItemType(&j, LIST_TYPE_TOOLTIP);
      if (lst)
	{
	   for (i = 0; i < j; i++)
	     {
		HideToolTip(lst[i]);
	     }
	   Efree(lst);
	}
   }

   ActionsSuspend();

   FocusNewDeskBegin();

   if (mode.mode == MODE_NONE)
      mode.mode = MODE_DESKSWITCH;

   if (desk > 0)
     {
	if (conf.desks.slidein)
	  {
	     if (!desks.desk[desk].viewable)
	       {
		  switch (conf.desks.dragdir)
		    {
		    case 0:
		       MoveDesktop(desk, root.w, 0);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, root.w, 0, 0, 0,
				     conf.desks.slidespeed);
		       break;
		    case 1:
		       MoveDesktop(desk, -root.w, 0);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, -root.w, 0, 0, 0,
				     conf.desks.slidespeed);
		       break;
		    case 2:
		       MoveDesktop(desk, 0, root.h);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, 0, root.h, 0, 0,
				     conf.desks.slidespeed);
		       break;
		    case 3:
		       MoveDesktop(desk, 0, -root.h);
		       RaiseDesktop(desk);
		       SlideWindowTo(desks.desk[desk].win, 0, -root.h, 0, 0,
				     conf.desks.slidespeed);
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
				conf.desks.slidespeed);
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

   if (mode.mode == MODE_DESKSWITCH)
      mode.mode = MODE_NONE;

   RedrawPagersForDesktop(pdesk, 0);
   RedrawPagersForDesktop(desk, 3);
   ForceUpdatePagersForDesktop(desk);
   HandleDrawQueue();

   EDBUG_RETURN_;
}

void
MoveDesktop(int desk, int x, int y)
{
   int                 i;
   EWin              **lst;
   int                 n, v, dx, dy;

   EDBUG(3, "MoveDesktop");
   if (desk < 0)
      EDBUG_RETURN_;
   if (desk >= conf.desks.num)
      EDBUG_RETURN_;
   if (desk == 0)
      EDBUG_RETURN_;
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
		  if ((desks.desk[desks.order[i]].viewable)
		      && (desks.desk[desks.order[i]].bg))
		     desks.desk[desks.order[i]].bg->last_viewed = time(NULL);
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
		       if ((!v) && (desks.desk[desks.order[i]].viewable)
			   && (desks.desk[desks.order[i]].bg))
			 {
			    desks.desk[desks.order[i]].bg->last_viewed =
			       time(NULL);
			 }
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

   lst = (EWin **) ListItemType(&n, LIST_TYPE_EWIN);
   if (lst)
     {
	for (i = 0; i < n; i++)
	   if (lst[i]->desktop == desk)
	      ICCCM_Configure(lst[i]);
	Efree(lst);
     }
   EDBUG_RETURN_;

}

void
RaiseDesktop(int desk)
{
   int                 i;

   EDBUG(3, "RaiseDesktop");

   if ((desk < 0) || (desk >= conf.desks.num))
      EDBUG_RETURN_;

   FocusNewDeskBegin();
   CloneDesktop(desks.order[0]);
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
   FocusNewDesk();
   FX_DeskChange();
   RemoveClones();
   RedrawPagersForDesktop(desk, 3);
   ForceUpdatePagersForDesktop(desk);
   UpdatePagerSel();
   HandleDrawQueue();
   HintsSetCurrentDesktop();
   EMapWindow(disp, desks.desk[desk].win);
   XSync(disp, False);

   EDBUG_RETURN_;

}

void
LowerDesktop(int desk)
{
   EDBUG(3, "LowerDesktop");

   if ((desk <= 0) || (desk >= conf.desks.num))
      EDBUG_RETURN_;

   FocusNewDeskBegin();
   CloneDesktop(desk);
   MoveToDeskBottom(desk);
   UncoverDesktop(desks.order[0]);
   HideDesktop(desk);
   StackDesktops();
   desks.current = desks.order[0];
   MoveStickyWindowsToCurrentDesk();
   FocusNewDesk();
   FX_DeskChange();
   RemoveClones();
   RedrawPagersForDesktop(desks.order[0], 3);
   ForceUpdatePagersForDesktop(desks.order[0]);
   UpdatePagerSel();
   HandleDrawQueue();
   HintsSetCurrentDesktop();
   XSync(disp, False);

   EDBUG_RETURN_;

}

void
HideDesktop(int desk)
{
   EDBUG(3, "HideDesktop");

   if ((desk < 0) || (desk >= conf.desks.num))
      EDBUG_RETURN_;
   if (desk == 0)
      EDBUG_RETURN_;

   if ((desks.desk[desk].viewable) && (desks.desk[desk].bg))
      desks.desk[desk].bg->last_viewed = time(NULL);
   desks.desk[desk].viewable = 0;
   EMoveWindow(disp, desks.desk[desk].win, root.w, 0);

   EDBUG_RETURN_;

}

void
ShowDesktop(int desk)
{
   int                 i;

   EDBUG(3, "ShowDesktop");

   if (desk < 0)
      EDBUG_RETURN_;
   if (desk >= conf.desks.num)
      EDBUG_RETURN_;

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

   EDBUG_RETURN_;
}

void
StackDesktops()
{
   EDBUG(2, "StackDesktops");

   StackDesktop(0);

   EDBUG_RETURN_;
}

#define _APPEND_TO_WIN_LIST(win) \
  { \
     wl = Erealloc(wl, ++tot * sizeof(Window)); \
     wl[tot - 1] = win; \
  }
void
StackDesktop(int desk)
{
   Window             *wl, *wl2;
   int                 i, wnum, tot, bnum;
   EWin              **lst, *ewin;
   Button            **blst;

   EDBUG(2, "StackDesktop");
   tot = 0;
   wl = NULL;

   /*
    * Build the window stack, top to bottom
    */

   wl2 = ListProgressWindows(&wnum);
   if (wl2)
     {
	for (i = 0; i < wnum; i++)
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

   lst = EwinListGetStacking(&wnum);
   blst = (Button **) ListItemType(&bnum, LIST_TYPE_BUTTON);

   /* Sticky buttons */
   if (blst)
     {
	for (i = 0; i < bnum; i++)
	  {
	     if (!blst[i]->sticky || blst[i]->internal)
		continue;

	     _APPEND_TO_WIN_LIST(blst[i]->win);
	  }
     }

   /* Floating EWins */
   if (lst)
     {
	for (i = 0; i < wnum; i++)
	  {
	     if (!lst[i]->floating)
		continue;

	     _APPEND_TO_WIN_LIST(lst[i]->win);
	  }
     }

   /* The virtual desktop windows */
   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	if (desks.order[i] == 0)
	   break;

	_APPEND_TO_WIN_LIST(desks.desk[desks.order[i]].win);
     }

   /* Non-sticky, "above" buttons */
   if (blst)
     {
	for (i = 0; i < bnum; i++)
	  {
	     if (blst[i]->desktop != desk || blst[i]->ontop != 1 ||
		 blst[i]->sticky || blst[i]->internal)
		continue;

	     _APPEND_TO_WIN_LIST(blst[i]->win);
	  }
     }

   /* Normal EWins on this desk */
   for (i = 0; i < wnum; i++)
     {
	ewin = lst[i];
	if (EwinGetDesk(ewin) != desk || ewin->floating)
	   continue;

	_APPEND_TO_WIN_LIST(ewin->win);
	if (ewin->win == mode.menu_win_covered)
	   _APPEND_TO_WIN_LIST(mode.menu_cover_win);
     }

   /* Non-sticky, "below" buttons */
   if (blst)
     {
	for (i = 0; i < bnum; i++)
	  {
	     if (blst[i]->desktop != desk || blst[i]->ontop != -1 ||
		 blst[i]->sticky || blst[i]->internal)
		continue;

	     _APPEND_TO_WIN_LIST(blst[i]->win);
	  }
     }

   /* The current (virtual) root window */
   _APPEND_TO_WIN_LIST(desks.desk[desk].win);

   XRestackWindows(disp, wl, tot);
   ShowEdgeWindows();
   RaiseProgressbars();
   HintsSetClientList();

   if (wl)
      Efree(wl);
   if (blst)
      Efree(blst);

   EDBUG_RETURN_;
}

void
UncoverDesktop(int desk)
{
   EDBUG(3, "UncoverDesktop");
   if (desk < 0)
      EDBUG_RETURN_;
   if (desk >= conf.desks.num)
      EDBUG_RETURN_;
   desks.desk[desk].viewable = 1;
   RefreshDesktop(desk);
   if (desk != 0)
      EMapWindow(disp, desks.desk[desk].win);
   EDBUG_RETURN_;
}

void
MoveEwinToDesktop(EWin * ewin, int desk)
{
   int                 pdesk;

   EDBUG(3, "MoveEwinToDesktop");
/*   ewin->sticky = 0; */
   ewin->floating = 0;
   pdesk = ewin->desktop;
   ewin->desktop = DESKTOPS_WRAP_NUM(desk);
   DesktopAddEwinToTop(ewin);
   ConformEwinToDesktop(ewin);
   if (ewin->has_transients)
     {
	EWin              **lst;
	int                 i, nn;

	lst = ListTransientsFor(ewin->client.win, &nn);
	if (lst)
	  {
	     for (i = 0; i < nn; i++)
	       {
		  MoveEwinToDesktop(lst[i], desk);
	       }
	     Efree(lst);
	  }
     }
   ForceUpdatePagersForDesktop(pdesk);
   ForceUpdatePagersForDesktop(ewin->desktop);
   EDBUG_RETURN_;
}

void
DesktopAddEwinToTop(EWin * ewin)
{
   EDBUG(5, "DesktopAddEwinToTop");

   if ((ewin->desktop < 0)
       || (ewin->desktop > ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1))
      EDBUG_RETURN_;

   EwinListStackingRaise(ewin);
   ForceUpdatePagersForDesktop(ewin->desktop);

   EDBUG_RETURN_;
}

void
DesktopAddEwinToBottom(EWin * ewin)
{
   EDBUG(5, "DesktopAddEwinToBottom");

   if ((ewin->desktop < 0)
       || (ewin->desktop > ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1))
      EDBUG_RETURN_;

   EwinListStackingLower(ewin);
   ForceUpdatePagersForDesktop(ewin->desktop);

   EDBUG_RETURN_;
}

void
MoveEwinToDesktopAt(EWin * ewin, int desk, int x, int y)
{
   int                 dx, dy;

   EDBUG(3, "MoveEwinToDesktopAt");
/*   ewin->sticky = 0; */
   ewin->floating = 0;
   if (desk != ewin->desktop && !ewin->sticky)
     {
	ForceUpdatePagersForDesktop(ewin->desktop);
	ewin->desktop = DESKTOPS_WRAP_NUM(desk);
	DesktopAddEwinToTop(ewin);
     }
   dx = x - ewin->x;
   dy = y - ewin->y;
   ewin->x = x;
   ewin->y = y;
   ConformEwinToDesktop(ewin);
   if (ewin->has_transients)
     {
	EWin              **lst;
	int                 i, nn;

	lst = ListTransientsFor(ewin->client.win, &nn);
	if (lst)
	  {
	     for (i = 0; i < nn; i++)
	       {
		  MoveEwinToDesktopAt(lst[i], desk, lst[i]->x + dx,
				      lst[i]->y + dy);
	       }
	     Efree(lst);
	  }
     }
   ForceUpdatePagersForDesktop(desk);
   EDBUG_RETURN_;
}

void
GotoDesktopByEwin(EWin * ewin)
{
   if (!ewin->sticky)
     {
	GotoDesktop(ewin->desktop);
	SetCurrentArea(ewin->area_x, ewin->area_y);
     }
}

#if 0				/* Unused */
void
FloatEwinAboveDesktops(EWin * ewin)
{
   int                 xo, yo;

   EDBUG(2, "FloatEwinAboveDesktops");
   xo = desks.desk[ewin->desktop].x;
   yo = desks.desk[ewin->desktop].y;
   ewin->desktop = 0;
   ewin->floating = 1;
   ConformEwinToDesktop(ewin);
   if (ewin->has_transients)
     {
	EWin              **lst;
	int                 i, num;

	lst = ListTransientsFor(ewin->client.win, &num);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
		FloatEwinAboveDesktops(lst[i]);
	     Efree(lst);
	  }
     }
   EDBUG_RETURN_;
}
#endif

void
DesktopAccounting()
{
   time_t              now;
   int                 i, j, num;
   Background        **lst;

   EDBUG(3, "DesktopAccounting");
   now = time(NULL);
   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
     {
	if ((desks.desk[i].bg) && (desks.desk[i].viewable))
	   desks.desk[i].bg->last_viewed = now;
     }

   lst = (Background **) ListItemType(&num, LIST_TYPE_BACKGROUND);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     if ((lst[i]->pmap == 0) ||
		 ((now - lst[i]->last_viewed) <= conf.backgrounds.timeout))
		continue;

	     for (j = 0; j < ENLIGHTENMENT_CONF_NUM_DESKTOPS; j++)
	       {
		  if ((desks.desk[j].bg == lst[i]) && (!desks.desk[j].viewable))
		    {
		       Window              win = desks.desk[j].win;

		       HintsSetRootInfo(win, 0, 0);
		       XSetWindowBackground(disp, win, 0);
		       XClearWindow(disp, win);

		       IMLIB1_SET_CONTEXT(lst[i] == desks.desk[0].bg);
		       imlib_free_pixmap_and_mask(lst[i]->pmap);
		       lst[i]->pmap = 0;
		    }
	       }

	  }
	Efree(lst);
	IMLIB1_SET_CONTEXT(0);
     }

   EDBUG_RETURN_;
}
