/*
 * Copyright (C) 2004-2006 Kim Woelders
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
 * This code was originally derived from xcompmgr.c,  see original copyright
 * notice at end.
 * It has been mostly rewritten since, only the shadow code is more or less
 * intact.
 */

#include "E.h"
#if USE_COMPOSITE
#include "desktops.h"
#include "ecompmgr.h"
#include "emodule.h"
#include "eobj.h"
#include "hints.h"
#include "settings.h"
#include "timers.h"
#include "xwin.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>

#define ENABLE_SHADOWS      1

#define USE_DESK_EXPOSE     0
#define USE_DESK_VISIBILITY 1

#define USE_CLIP_RELATIVE_TO_DESK 1

#define ENABLE_DEBUG   1
#if ENABLE_DEBUG
#define EDBUG_TYPE_COMPMGR  161
#define EDBUG_TYPE_COMPMGR2 162
#define EDBUG_TYPE_COMPMGR3 163
#define EDBUG_TYPE_COMPMGR4 164
#define D1printf(fmt...) if(EventDebug(EDBUG_TYPE_COMPMGR))Eprintf(fmt)
#define D2printf(fmt...) if(EventDebug(EDBUG_TYPE_COMPMGR2))Eprintf(fmt)
#define D3printf(fmt...) if(EventDebug(EDBUG_TYPE_COMPMGR3))Eprintf(fmt)
#define D4printf(fmt...) if(EventDebug(EDBUG_TYPE_COMPMGR4))Eprintf(fmt)
#else
#define D1printf(fmt...)
#define D2printf(fmt...)
#endif /* ENABLE_DEBUG */

#define DEBUG_OPACITY 0

#define INV_POS     0x01
#define INV_SIZE    0x02
#define INV_CLIP    0x04
#define INV_OPACITY 0x08
#define INV_SHADOW  0x10
#define INV_PIXMAP  0x20
#define INV_PICTURE 0x40
#define INV_GEOM    (INV_POS | INV_SIZE)
#define INV_ALL     (INV_POS | INV_SIZE | INV_CLIP | INV_OPACITY | INV_SHADOW | INV_PIXMAP)

typedef struct
{
   EObj               *next;	/* Paint order */
   EObj               *prev;	/* Paint order */
   Pixmap              pixmap;
   int                 rcx, rcy, rcw, rch;
   int                 mode;
   unsigned            damaged:1;
   unsigned            fading:1;
   unsigned            fadeout:1;
   unsigned            has_shadow:1;
   Damage              damage;
   Picture             picture;
   Picture             pict_alpha;	/* Solid, current opacity */
   XserverRegion       shape;
   XserverRegion       extents;
   XserverRegion       clip;
#if ENABLE_SHADOWS
   Picture             shadow_alpha;	/* Solid, sharp * current opacity */
   Picture             shadow_pict;	/* Blurred shaped shadow */
   int                 shadow_dx;
   int                 shadow_dy;
   int                 shadow_width;
   int                 shadow_height;
#endif
   unsigned int        opacity;

   unsigned long       damage_sequence;	/* sequence when damage was created */
} ECmWinInfo;

/*
 * Configuration
 */
#if ENABLE_SHADOWS
#define ECM_SHADOWS_OFF      0
#define ECM_SHADOWS_SHARP    1	/* use window alpha for shadow; sharp, but precise */
#define ECM_SHADOWS_ECHO     3	/* use window for shadow; sharp, but precise */
#define ECM_SHADOWS_BLURRED  2	/* use window extents for shadow, blurred */
#endif

#define ECM_OR_UNREDIRECTED  0
#define ECM_OR_ON_MAP        1
#define ECM_OR_ON_MAPUNMAP   2
#define ECM_OR_ON_CREATE     3

static struct
{
   char                enable;
   char                resize_fix_enable;
   char                use_name_pixmap;
   int                 mode;
   struct
   {
      int                 mode;
      int                 offset_x, offset_y;
      struct
      {
	 int                 opacity;
	 int                 radius;
      } blur;
      struct
      {
	 int                 opacity;
      } sharp;
   } shadows;
   struct
   {
      char                enable;
      int                 dt_us;	/* us between updates */
      unsigned int        step;
   } fading;
   struct
   {
      int                 mode;
      int                 opacity;
   } override_redirect;
} Conf_compmgr;

/*
 * State
 */
#define ECM_MODE_OFF    0
#define ECM_MODE_ROOT   1
#define ECM_MODE_WINDOW 2
#define ECM_MODE_AUTO   3

static struct
{
   char                active;
   char                use_pixmap;
   char                reorder;
   EObj               *eo_first;
   EObj               *eo_last;
   XserverRegion       rgn_screen;
   int                 shadow_mode;
   unsigned int        opac_or;	/* 0 -> 0xffffffff */
   double              opac_blur;	/* 0. -> 1. */
   double              opac_sharp;	/* 0. -> 1. */
} Mode_compmgr;

#define _ECM_SET_CLIP_CHANGED()  Mode_compmgr.reorder = 1
#define _ECM_SET_STACK_CHANGED() Mode_compmgr.reorder = 1

static Picture      rootPicture;
static Picture      rootBuffer;

static XserverRegion allDamage;

static ESelection  *wm_cm_sel = NULL;

#define OPAQUE          0xffffffff
#define OP32(op) ((double)(op)/OPAQUE)

#define WINDOW_UNREDIR  0
#define WINDOW_SOLID    1
#define WINDOW_TRANS    2
#define WINDOW_ARGB     3

static void         ECompMgrDamageAll(void);
static void         ECompMgrHandleRootEvent(Win win, XEvent * ev, void *prm);
static void         ECompMgrHandleWindowEvent(Win win, XEvent * ev, void *prm);
static void         doECompMgrWinFade(int val, void *data);
static void         ECompMgrWinInvalidate(EObj * eo, int what);
static void         ECompMgrWinSetPicts(EObj * eo);
static void         ECompMgrWinFadeOutEnd(EObj * eo);
static int          ECompMgrDetermineOrder(EObj * const *lst, int num,
					   EObj ** first, EObj ** last,
					   Desk * dsk, XserverRegion clip);

/*
 * Regions
 */

static              XserverRegion
ERegionCreate(void)
{
   return XFixesCreateRegion(disp, NULL, 0);
}

static              XserverRegion
ERegionCreateRect(int x, int y, int w, int h)
{
   XserverRegion       rgn;
   XRectangle          rct;

   rct.x = x;
   rct.y = y;
   rct.width = w;
   rct.height = h;
   rgn = XFixesCreateRegion(disp, &rct, 1);

   return rgn;
}

#if USE_DESK_EXPOSE
static              XserverRegion
ERegionCreateFromRects(XRectangle * rectangles, int nrectangles)
{
   return XFixesCreateRegion(disp, rectangles, nrectangles);
}
#endif

static              XserverRegion
ERegionCreateFromWindow(Window win)
{
   return XFixesCreateRegionFromWindow(disp, win, WindowRegionBounding);
}

static void
ERegionCopy(XserverRegion rgn, XserverRegion src)
{
   XFixesCopyRegion(disp, rgn, src);
}

static              XserverRegion
ERegionClone(XserverRegion src)
{
   XserverRegion       rgn;

   rgn = ERegionCreate();
   ERegionCopy(rgn, src);

   return rgn;
}

static void
ERegionDestroy(XserverRegion rgn)
{
   XFixesDestroyRegion(disp, rgn);
}

static void
ERegionTranslate(XserverRegion rgn, int dx, int dy)
{
   if (dx == 0 && dy == 0)
      return;
   XFixesTranslateRegion(disp, rgn, dx, dy);
}

static void
ERegionIntersect(XserverRegion dst, XserverRegion src)
{
   XFixesIntersectRegion(disp, dst, dst, src);
}

static void
ERegionUnion(XserverRegion dst, XserverRegion src)
{
   XFixesUnionRegion(disp, dst, dst, src);
}

static void
ERegionSubtract(XserverRegion dst, XserverRegion src)
{
   XFixesSubtractRegion(disp, dst, dst, src);
}

static void
ERegionLimit(XserverRegion rgn)
{
   XserverRegion       screen;

   screen = Mode_compmgr.rgn_screen;
   if (screen == None)
      Mode_compmgr.rgn_screen = screen =
	 ERegionCreateRect(0, 0, VRoot.w, VRoot.h);

   ERegionIntersect(rgn, screen);
}

static void
ERegionSubtractOffset(XserverRegion dst, int dx, int dy, XserverRegion src)
{
   Display            *dpy = disp;
   XserverRegion       rgn;

   rgn = src;
   if (dx != 0 || dy != 0)
     {
	rgn = ERegionClone(src);
	XFixesTranslateRegion(dpy, rgn, dx, dy);
     }
   XFixesSubtractRegion(dpy, dst, dst, rgn);
   if (rgn != src)
      ERegionDestroy(rgn);
}

static void
ERegionUnionOffset(XserverRegion dst, int dx, int dy, XserverRegion src)
{
   Display            *dpy = disp;
   XserverRegion       rgn;

   rgn = src;
   if (dx != 0 || dy != 0)
     {
	rgn = ERegionClone(src);
	XFixesTranslateRegion(dpy, rgn, dx, dy);
     }
   XFixesUnionRegion(dpy, dst, dst, rgn);
   if (rgn != src)
      ERegionDestroy(rgn);
}

#if 0				/* Unused (for debug) */
static int
ERegionIsEmpty(XserverRegion rgn)
{
   int                 nr;
   XRectangle         *pr;

   pr = XFixesFetchRegion(disp, rgn, &nr);
   if (pr)
      XFree(pr);
   return nr == 0;
}
#endif

static void
ERegionShow(const char *txt, XserverRegion rgn)
{
   int                 i, nr;
   XRectangle         *pr;

   if (rgn == None)
     {
	Eprintf(" - region: %s %#lx is None\n", txt, rgn);
	return;
     }

   pr = XFixesFetchRegion(disp, rgn, &nr);
   if (!pr || nr <= 0)
     {
	Eprintf(" - region: %s %#lx is empty\n", txt, rgn);
	goto done;
     }

   Eprintf(" - region: %s %#lx:\n", txt, rgn);
   for (i = 0; i < nr; i++)
      Eprintf("%4d: %4d+%4d %4dx%4d\n", i, pr[i].x, pr[i].y, pr[i].width,
	      pr[i].height);

 done:
   if (pr)
      XFree(pr);
}

/*
 * Pictures
 */

static              Picture
EPictureCreateSolid(Bool argb, double a, double r, double g, double b)
{
   Display            *dpy = disp;
   Pixmap              pmap;
   Picture             pict;
   XRenderPictFormat  *pictfmt;
   XRenderPictureAttributes pa;
   XRenderColor        c;

   pmap = XCreatePixmap(dpy, VRoot.xwin, 1, 1, argb ? 32 : 8);
   pictfmt = XRenderFindStandardFormat(dpy,
				       argb ? PictStandardARGB32 :
				       PictStandardA8);
   pa.repeat = True;
   pict = XRenderCreatePicture(dpy, pmap, pictfmt, CPRepeat, &pa);

   c.alpha = a * 0xffff;
   c.red = r * 0xffff;
   c.green = g * 0xffff;
   c.blue = b * 0xffff;
   XRenderFillRectangle(dpy, PictOpSrc, pict, &c, 0, 0, 1, 1);

   XFreePixmap(dpy, pmap);

   return pict;
}

static              Picture
EPictureCreateBuffer(Window win, int w, int h, int depth, Visual * vis)
{
   Picture             pict;
   Pixmap              pmap;
   XRenderPictFormat  *pictfmt;

   pmap = XCreatePixmap(disp, win, w, h, depth);
   pictfmt = XRenderFindVisualFormat(disp, vis);
   pict = XRenderCreatePicture(disp, pmap, pictfmt, 0, 0);
   XFreePixmap(disp, pmap);

   return pict;
}

#if 0
static              Picture
EPictureCreate(Window win, int depth, Visual * vis)
{
   Picture             pict;
   XRenderPictFormat  *pictfmt;

   pictfmt = XRenderFindVisualFormat(disp, vis);
   pict = XRenderCreatePicture(disp, win, pictfmt, 0, 0);

   return pict;
}
#endif

/* Hack to fix redirected window resize bug(?) */
void
ECompMgrMoveResizeFix(EObj * eo, int x, int y, int w, int h)
{
   Picture             pict;
   int                 wo, ho;
   ECmWinInfo         *cw = eo->cmhook;

   if (!cw || !Conf_compmgr.resize_fix_enable)
     {
	EMoveResizeWindow(eo->win, x, y, w, h);
	return;
     }

   wo = ho = 0;
   EGetGeometry(eo->win, NULL, NULL, NULL, &wo, &ho, NULL, NULL);
   if (wo <= 0 || ho <= 0 || (wo == w && ho == h))
     {
	EMoveResizeWindow(eo->win, x, y, w, h);
	return;
     }

   /* Resizing - grab old contents */
   pict = EPictureCreateBuffer(EobjGetXwin(eo), wo, ho, WinGetDepth(eo->win),
			       WinGetVisual(eo->win));
   XRenderComposite(disp, PictOpSrc, cw->picture, None, pict, 0, 0, 0, 0, 0, 0,
		    wo, ho);

   /* Resize (+move) */
   EMoveResizeWindow(eo->win, x, y, w, h);

   /* Paste old contents back in */
   if (w < wo)
      w = wo;
   if (h < ho)
      h = ho;
   XRenderComposite(disp, PictOpSrc, pict, None, cw->picture,
		    0, 0, 0, 0, 0, 0, w, h);
   XRenderFreePicture(disp, pict);
}

#if !USE_BG_WIN_ON_ALL_DESKS
/*
 * Desk background
 */

int
ECompMgrDeskConfigure(Desk * dsk)
{
   EObj               *eo;
   ECmWinInfo         *cw;
   Picture             pict;
   XRenderPictFormat  *pictfmt;
   XRenderPictureAttributes pa;
   Pixmap              pmap;

   if (!Mode_compmgr.active)
      return 0;

   eo = dsk->bg.o;
   if (!eo)
      return 1;

   ECompMgrWinInvalidate(eo, INV_PICTURE);

   if (!dsk->viewable && dsk->bg.bg)
      return 1;

   if (dsk->bg.pmap == None)
     {
	GC                  gc;

	pmap = XCreatePixmap(disp, VRoot.xwin, 1, 1, VRoot.depth);
	gc = EXCreateGC(pmap, 0, NULL);
	XSetClipMask(disp, gc, 0);
	XSetFillStyle(disp, gc, FillSolid);
	XSetForeground(disp, gc, dsk->bg.pixel);
	XFillRectangle(disp, pmap, gc, 0, 0, 1, 1);
	EXFreeGC(gc);
     }
   else
     {
	pmap = dsk->bg.pmap;
     }

   pa.repeat = True;
   pictfmt = XRenderFindVisualFormat(disp, VRoot.vis);
   pict = XRenderCreatePicture(disp, pmap, pictfmt, CPRepeat, &pa);

   if (pmap != dsk->bg.pmap)
      XFreePixmap(disp, pmap);

   /* New background, all must be repainted */
   ECompMgrDamageAll();

   cw = eo->cmhook;
   cw->picture = pict;

   D1printf
      ("ECompMgrDeskConfigure: Desk %d: using pixmap %#lx picture=%#lx\n",
       dsk->num, pmap, cw->picture);

   return 1;
}
#endif

#if USE_DESK_VISIBILITY
static void
ECompMgrDeskVisibility(EObj * eo, XEvent * ev)
{
   Desk               *dsk;
   int                 visible;

   switch (eo->type)
     {
     default:
	return;
     case EOBJ_TYPE_DESK:
	dsk = (Desk *) eo;
	break;
     case EOBJ_TYPE_ROOT_BG:
	dsk = DeskGet(0);
	break;
     }

   visible = dsk->viewable && ev->xvisibility.state != VisibilityFullyObscured;
   if (dsk->visible == visible)
      return;
   dsk->visible = visible;
   if (!visible)
      return;

   /*
    * A viewable desk is no longer fully obscured. Assume this happened due
    * to a VT switch to our display and repaint all. This may happen in other
    * situations as well, but most likely when we must repaint everything
    * anyway.
    */
   ECompMgrDamageAll();
}
#endif

/*
 * Root (?)
 */

static void
ECompMgrDamageMerge(XserverRegion damage, int destroy)
{
   if (allDamage != None)
     {
	if (EventDebug(EDBUG_TYPE_COMPMGR3))
	   ERegionShow("ECompMgrDamageMerge add:", damage);

	ERegionUnion(allDamage, damage);
	if (destroy)
	   ERegionDestroy(damage);
     }
   else if (!destroy)
     {
	allDamage = ERegionClone(damage);
     }
   else
     {
	allDamage = damage;
     }

   if (EventDebug(EDBUG_TYPE_COMPMGR3))
      ERegionShow("ECompMgrDamageMerge all:", allDamage);
}

static void
ECompMgrDamageMergeObject(EObj * eo, XserverRegion damage, int destroy)
{
   ECmWinInfo         *cw = eo->cmhook;
   Desk               *dsk = eo->desk;

   if (!Mode_compmgr.active || damage == None)
      return;

   if (dsk->num > 0 && !dsk->viewable && eo->ilayer < 512)
     {
	if (destroy)
	   ERegionDestroy(damage);
	return;
     }

   if (Mode_compmgr.reorder)
      ECompMgrDetermineOrder(NULL, 0, &Mode_compmgr.eo_first,
			     &Mode_compmgr.eo_last, DeskGet(0), None);

#if 0				/* FIXME - Remove? */
   if (cw->clip == None)
     {
	/* Clip may be None if window is not in paint list */
	if (destroy)
	   ERegionDestroy(damage);
	return;
     }
#endif

   if (!destroy)
      damage = ERegionClone(damage);

#if USE_CLIP_RELATIVE_TO_DESK
   if (cw->clip != None && eo->type != EOBJ_TYPE_DESK)
      ERegionSubtract(damage, cw->clip);
#endif

   if (EoGetX(dsk) != 0 || EoGetY(dsk) != 0)
      ERegionTranslate(damage, EoGetX(dsk), EoGetY(dsk));

#if !USE_CLIP_RELATIVE_TO_DESK
   if (cw->clip != None && eo->type != EOBJ_TYPE_DESK)
      ERegionSubtract(damage, cw->clip);
#endif

   ECompMgrDamageMerge(damage, 1);
}

static void
ECompMgrDamageAll(void)
{
   ECompMgrDamageMerge(ERegionCreateRect(0, 0, VRoot.w, VRoot.h), 1);
}

#if ENABLE_SHADOWS

static Picture      transBlackPicture;

typedef struct
{
   int                 size;
   double             *data;
} conv;

static conv        *gaussianMap = NULL;

static double
gaussian(double r, double x, double y)
{
   return ((1 / (sqrt(2 * M_PI * r))) * exp((-(x * x + y * y)) / (2 * r * r)));
}

static conv        *
make_gaussian_map(double r)
{
   conv               *c;
   int                 size = ((int)ceil((r * 3)) + 1) & ~1;
   int                 center = size / 2;
   int                 x, y;
   double              t;
   double              g;

   c = malloc(sizeof(conv) + size * size * sizeof(double));
   c->size = size;
   c->data = (double *)(c + 1);
   t = 0.0;
   for (y = 0; y < size; y++)
      for (x = 0; x < size; x++)
	{
	   g = gaussian(r, (double)(x - center), (double)(y - center));
	   t += g;
	   c->data[y * size + x] = g;
	}
/*    printf ("gaussian total %f\n", t); */
   for (y = 0; y < size; y++)
      for (x = 0; x < size; x++)
	{
	   c->data[y * size + x] /= t;
	}
   return c;
}

/*
 * A picture will help
 *
 *      -center   0                width  width+center
 *  -center +-----+-------------------+-----+
 *          |     |                   |     |
 *          |     |                   |     |
 *        0 +-----+-------------------+-----+
 *          |     |                   |     |
 *          |     |                   |     |
 *          |     |                   |     |
 *   height +-----+-------------------+-----+
 *          |     |                   |     |
 * height+  |     |                   |     |
 *  center  +-----+-------------------+-----+
 */

static unsigned char
sum_gaussian(conv * map, double opacity, int x, int y, int width, int height)
{
   int                 fx, fy;
   double             *g_data;
   double             *g_line = map->data;
   int                 g_size = map->size;
   int                 center = g_size / 2;
   int                 fx_start, fx_end;
   int                 fy_start, fy_end;
   double              v;

   /*
    * Compute set of filter values which are "in range",
    * that's the set with:
    *  0 <= x + (fx-center) && x + (fx-center) < width &&
    *  0 <= y + (fy-center) && y + (fy-center) < height
    *
    *  0 <= x + (fx - center)  x + fx - center < width
    *  center - x <= fx        fx < width + center - x
    */

   fx_start = center - x;
   if (fx_start < 0)
      fx_start = 0;
   fx_end = width + center - x;
   if (fx_end > g_size)
      fx_end = g_size;

   fy_start = center - y;
   if (fy_start < 0)
      fy_start = 0;
   fy_end = height + center - y;
   if (fy_end > g_size)
      fy_end = g_size;

   g_line = g_line + fy_start * g_size + fx_start;

   v = 0;
   for (fy = fy_start; fy < fy_end; fy++)
     {
	g_data = g_line;
	g_line += g_size;

	for (fx = fx_start; fx < fx_end; fx++)
	   v += *g_data++;
     }
   if (v > 1)
      v = 1;

   return ((unsigned char)(v * opacity * 255.0));
}

static XImage      *
make_shadow(double opacity, int width, int height)
{
   Display            *dpy = disp;
   XImage             *ximage;
   unsigned char      *data;
   int                 gsize = gaussianMap->size;
   int                 ylimit, xlimit;
   int                 swidth = width + gsize;
   int                 sheight = height + gsize;
   int                 center = gsize / 2;
   int                 x, y;
   unsigned char       d;
   int                 x_diff;

   data = calloc(swidth * sheight, sizeof(unsigned char));
   if (!data)
      return NULL;

   ximage = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
			 8, ZPixmap, 0,
			 (char *)data,
			 swidth, sheight, 8, swidth * sizeof(unsigned char));
   if (!ximage)
     {
	free(data);
	return NULL;
     }

   /*
    * Build the gaussian in sections
    */

#if 1
   /* FIXME - Handle properly - shaped/non-shaped/offset */
   /*
    * center (fill the complete data array)
    */
   d = sum_gaussian(gaussianMap, opacity, center, center, width, height);
   memset(data, d, sheight * swidth);
#endif

   /*
    * corners
    */
   ylimit = gsize;
   if (ylimit > sheight / 2)
      ylimit = (sheight + 1) / 2;
   xlimit = gsize;
   if (xlimit > swidth / 2)
      xlimit = (swidth + 1) / 2;

   for (y = 0; y < ylimit; y++)
      for (x = 0; x < xlimit; x++)
	{
	   d = sum_gaussian(gaussianMap, opacity, x - center, y - center, width,
			    height);
	   data[y * swidth + x] = d;
	   data[(sheight - y - 1) * swidth + x] = d;
	   data[(sheight - y - 1) * swidth + (swidth - x - 1)] = d;
	   data[y * swidth + (swidth - x - 1)] = d;
	}

   /*
    * top/bottom
    */
   x_diff = swidth - (gsize * 2);
   if (x_diff > 0 && ylimit > 0)
     {
	for (y = 0; y < ylimit; y++)
	  {
	     d = sum_gaussian(gaussianMap, opacity, center, y - center, width,
			      height);
	     memset(&data[y * swidth + gsize], d, x_diff);
	     memset(&data[(sheight - y - 1) * swidth + gsize], d, x_diff);
	  }
     }

   /*
    * sides
    */
   for (x = 0; x < xlimit; x++)
     {
	d = sum_gaussian(gaussianMap, opacity, x - center, center, width,
			 height);
	for (y = gsize; y < sheight - gsize; y++)
	  {
	     data[y * swidth + x] = d;
	     data[y * swidth + (swidth - x - 1)] = d;
	  }
     }

   return ximage;
}

static              Picture
shadow_picture(double opacity, int width, int height, int *wp, int *hp)
{
   Display            *dpy = disp;
   XImage             *shadowImage;
   Pixmap              shadowPixmap;
   Picture             shadowPicture;
   GC                  gc;

   shadowImage = make_shadow(opacity, width, height);
   if (!shadowImage)
      return None;

   shadowPixmap = XCreatePixmap(dpy, VRoot.xwin,
				shadowImage->width, shadowImage->height, 8);
   shadowPicture = XRenderCreatePicture(dpy, shadowPixmap,
					XRenderFindStandardFormat(dpy,
								  PictStandardA8),
					0, 0);
   gc = XCreateGC(dpy, shadowPixmap, 0, 0);

   XPutImage(dpy, shadowPixmap, gc, shadowImage, 0, 0, 0, 0,
	     shadowImage->width, shadowImage->height);
   *wp = shadowImage->width;
   *hp = shadowImage->height;
   XFreeGC(dpy, gc);
   XDestroyImage(shadowImage);
   XFreePixmap(dpy, shadowPixmap);

   return shadowPicture;
}

#endif /* ENABLE_SHADOWS */

/* Region of window in screen coordinates, including shadows */
static              XserverRegion
win_extents(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;
   XRectangle          r, sr;
   XserverRegion       rgn;
   unsigned int        bw;

   /* FIXME - Get this right */
   bw = EobjGetBW(eo);
   if (Mode_compmgr.use_pixmap)
     {
	cw->rcx = EobjGetX(eo);
	cw->rcy = EobjGetY(eo);
	cw->rcw = EobjGetW(eo) + 2 * bw;
	cw->rch = EobjGetH(eo) + 2 * bw;
     }
   else
     {
	cw->rcx = EobjGetX(eo) + bw;
	cw->rcy = EobjGetY(eo) + bw;
	cw->rcw = EobjGetW(eo);
	cw->rch = EobjGetH(eo);
     }

   if (eo->noredir && bw)
     {
	r.x = EobjGetX(eo);
	r.y = EobjGetY(eo);
	r.width = EobjGetW(eo) + 2 * bw;
	r.height = EobjGetH(eo) + 2 * bw;
     }
   else
     {
	r.x = cw->rcx;
	r.y = cw->rcy;
	r.width = cw->rcw;
	r.height = cw->rch;
     }

#if ENABLE_SHADOWS
   cw->has_shadow = (Mode_compmgr.shadow_mode != ECM_SHADOWS_OFF) &&
      eo->shadow && (EShapeCheck(eo->win) >= 0);
   if (!cw->has_shadow)
      goto skip_shadow;

   switch (Mode_compmgr.shadow_mode)
     {
     default:
	goto skip_shadow;

     case ECM_SHADOWS_SHARP:
     case ECM_SHADOWS_ECHO:
	cw->shadow_dx = Conf_compmgr.shadows.offset_x;
	cw->shadow_dy = Conf_compmgr.shadows.offset_y;
	cw->shadow_width = cw->rcw;
	cw->shadow_height = cw->rch;
	break;

     case ECM_SHADOWS_BLURRED:
	if (EobjIsShaped(eo) /* || cw->mode == WINDOW_ARGB */ )
	   goto skip_shadow;

	if (!gaussianMap)
	  {
	     gaussianMap =
		make_gaussian_map((double)Conf_compmgr.shadows.blur.radius);
	     if (!gaussianMap)
		goto skip_shadow;
	  }

	cw->shadow_dx = Conf_compmgr.shadows.offset_x - gaussianMap->size / 2;
	cw->shadow_dy = Conf_compmgr.shadows.offset_y - gaussianMap->size / 2;
	if (!cw->shadow_pict)
	   cw->shadow_pict = shadow_picture(Mode_compmgr.opac_blur,
					    cw->rcw, cw->rch,
					    &cw->shadow_width,
					    &cw->shadow_height);
	break;
     }
   sr.x = cw->rcx + cw->shadow_dx;
   sr.y = cw->rcy + cw->shadow_dy;
   sr.width = cw->shadow_width;
   sr.height = cw->shadow_height;
   if (sr.x < r.x)
     {
	r.width = (r.x + r.width) - sr.x;
	r.x = sr.x;
     }
   if (sr.y < r.y)
     {
	r.height = (r.y + r.height) - sr.y;
	r.y = sr.y;
     }
   if (sr.x + sr.width > r.x + r.width)
      r.width = sr.x + sr.width - r.x;
   if (sr.y + sr.height > r.y + r.height)
      r.height = sr.y + sr.height - r.y;

 skip_shadow:
#endif

   D2printf("extents %#lx %d %d %d %d\n", EobjGetXwin(eo), r.x, r.y, r.width,
	    r.height);

   rgn = ERegionCreateRect(r.x, r.y, r.width, r.height);

   if (EventDebug(EDBUG_TYPE_COMPMGR3))
      ERegionShow("extents", rgn);

#if 0				/* FIXME - Set picture clip region */
   if (cw->shadow_pict)
     {
	XserverRegion       clip;

	clip = ERegionClone(cw->extents);
	ERegionSubtractOffset(clip, 0, 0, cw->shape);
	XFixesSetPictureClipRegion(disp, cw->shadow_pict, 0, 0, clip);
	ERegionDestroy(clip);
     }
#endif
   return rgn;
}

/* Region of shaped window in screen coordinates */
static              XserverRegion
win_shape(EObj * eo)
{
   XserverRegion       border;
   int                 x, y;

   border = ERegionCreateFromWindow(EobjGetXwin(eo));

   if (1 /* eo->shaped */ )	/* FIXME - Track shaped state */
     {
	XserverRegion       rgn;

	/* Intersect with window size to get effective bounding region */
	rgn = ERegionCreateRect(0, 0, EobjGetW(eo), EobjGetH(eo));
	ERegionIntersect(border, rgn);
	ERegionDestroy(rgn);
     }

   /* translate this */
   x = EobjGetX(eo) + EobjGetBW(eo);
   y = EobjGetY(eo) + EobjGetBW(eo);
   ERegionTranslate(border, x, y);

   D2printf("shape %#lx: %d %d\n", EobjGetXwin(eo), x, y);
   if (EventDebug(EDBUG_TYPE_COMPMGR3))
      ERegionShow("shape", border);

   return border;
}

Pixmap
ECompMgrWinGetPixmap(const EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   if (!cw)
      return None;

   if (cw->pixmap != None)
      return cw->pixmap;

   if (eo->noredir)
      return None;

   cw->pixmap = XCompositeNameWindowPixmap(disp, EobjGetXwin(eo));

   return cw->pixmap;
}

static void
ECompMgrWinInvalidate(EObj * eo, int what)
{
   ECmWinInfo         *cw = eo->cmhook;
   Display            *dpy = disp;

   if (!cw)
      return;

   D2printf("ECompMgrWinInvalidate %#lx: %#x\n", EobjGetXwin(eo), what);

   if ((what & (INV_SIZE | INV_PIXMAP)) && cw->pixmap != None)
     {
	XFreePixmap(dpy, cw->pixmap);
	cw->pixmap = None;
	if (Mode_compmgr.use_pixmap)
	   what |= INV_PICTURE;
     }

   if ((what & INV_GEOM) && cw->shape != None)
     {
	ERegionDestroy(cw->shape);
	cw->shape = None;
     }

   if ((what & INV_PICTURE) && cw->picture != None)
     {
	XRenderFreePicture(dpy, cw->picture);
	cw->picture = None;
     }

   if ((what & INV_OPACITY) && cw->pict_alpha != None)
     {
	XRenderFreePicture(dpy, cw->pict_alpha);
	cw->pict_alpha = None;
     }

   if ((what & (INV_CLIP | INV_GEOM)) && cw->clip != None)
     {
	ERegionDestroy(cw->clip);
	cw->clip = None;
     }

#if ENABLE_SHADOWS
   if ((what & (INV_SIZE | INV_SHADOW)) && cw->shadow_pict != None)
     {
	XRenderFreePicture(dpy, cw->shadow_pict);
	cw->shadow_pict = None;
	what |= INV_GEOM;
     }
   if ((what & (INV_OPACITY | INV_SHADOW)) && cw->shadow_alpha != None)
     {
	XRenderFreePicture(dpy, cw->shadow_alpha);
	cw->shadow_alpha = None;
     }
#endif

   if ((what & (INV_GEOM | INV_SHADOW)) && cw->extents != None)
     {
	ERegionDestroy(cw->extents);
	cw->extents = None;
     }
}

static void
ECompMgrWinSetOpacity(EObj * eo, unsigned int opacity)
{
   ECmWinInfo         *cw = eo->cmhook;
   int                 mode;

   if (!cw || cw->opacity == opacity)
      return;

   cw->opacity = opacity;

   D1printf("ECompMgrWinSetOpacity: %#lx opacity=%#x\n", EobjGetXwin(eo),
	    cw->opacity);

   if (eo->shown || cw->fadeout)
      ECompMgrDamageMergeObject(eo, cw->extents, 0);

   /* Invalidate stuff changed by opacity */
   ECompMgrWinInvalidate(eo, INV_OPACITY);

   if (eo->noredir)
      mode = WINDOW_UNREDIR;
   else if (eo->argb)
      mode = WINDOW_ARGB;
   else if (cw->opacity != OPAQUE)
      mode = WINDOW_TRANS;
   else
      mode = WINDOW_SOLID;

   if (mode != cw->mode)
      _ECM_SET_CLIP_CHANGED();

   cw->mode = mode;
}

static void
ECompMgrWinFadeDoIn(EObj * eo, unsigned int op)
{
   char                s[128];

   Esnprintf(s, sizeof(s), "Fade-%#lx", EobjGetXwin(eo));
   DoIn(s, 1e-6 * Conf_compmgr.fading.dt_us, doECompMgrWinFade, op, eo);
}

static void
ECompMgrWinFadeCancel(EObj * eo)
{
   char                s[128];

   Esnprintf(s, sizeof(s), "Fade-%#lx", EobjGetXwin(eo));
   RemoveTimerEvent(s);
}

static void
doECompMgrWinFade(int val, void *data)
{
   EObj               *eo = data;
   ECmWinInfo         *cw;
   unsigned int        op = (unsigned int)val;

   /* May be gone */
   if (EobjListStackCheck(eo) < 0)
      return;

   cw = eo->cmhook;

#if DEBUG_OPACITY
   Eprintf("doECompMgrWinFade %#lx, %d/%d, %#x->%#x\n", EobjGetXwin(eo),
	   cw->fading, cw->fadeout, cw->opacity, op);
#endif
   if (!cw->fading)
      return;

   cw->fading = cw->fadeout;

   if (op == cw->opacity)
     {
	op = eo->opacity;
	if (cw->fadeout)
	   ECompMgrWinFadeOutEnd(eo);
	cw->fading = 0;
     }
   else if (op > cw->opacity)
     {
	if (op - cw->opacity > Conf_compmgr.fading.step)
	  {
	     op = cw->opacity + Conf_compmgr.fading.step;
	     cw->fading = 1;
	  }
     }
   else
     {
	if (cw->opacity - op > Conf_compmgr.fading.step)
	  {
	     op = cw->opacity - Conf_compmgr.fading.step;
	     cw->fading = 1;
	  }
     }

#if DEBUG_OPACITY
   Eprintf("doECompMgrWinFade %#lx, %#x\n", EobjGetXwin(eo), op);
#endif
   if (cw->fading)
      ECompMgrWinFadeDoIn(eo, (unsigned int)val);
   else if (eo->type == EOBJ_TYPE_EWIN)
      ModulesSignal(eo->shown ? ESIGNAL_EWIN_CHANGE : ESIGNAL_EWIN_UNMAP, eo);
   ECompMgrWinSetOpacity(eo, op);
}

static void
ECompMgrWinFade(EObj * eo, unsigned int op_from, unsigned int op_to)
{
   ECmWinInfo         *cw = eo->cmhook;

   cw->fading = 1;
   ECompMgrWinFadeDoIn(eo, op_to);
   ECompMgrWinSetOpacity(eo, op_from);
}

static void
ECompMgrWinFadeIn(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

#if DEBUG_OPACITY
   Eprintf("ECompMgrWinFadeIn  %#lx %#x -> %#x\n", EobjGetXwin(eo), 0x10000000,
	   eo->opacity);
#endif
   if (cw->fadeout)
      ECompMgrWinFadeOutEnd(eo);
   ECompMgrWinFade(eo, 0x10000000, eo->opacity);
}

static void
ECompMgrWinFadeOut(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

#if DEBUG_OPACITY
   Eprintf("ECompMgrWinFadeOut %#lx %#x -> %#x\n", EobjGetXwin(eo), cw->opacity,
	   0x10000000);
#endif
   cw->fadeout = 1;
   ECompMgrWinInvalidate(eo, INV_PICTURE);
   ECompMgrWinSetPicts(eo);
   ECompMgrDamageMergeObject(eo, cw->extents, 0);
   ECompMgrWinFade(eo, cw->opacity, 0x10000000);
}

static void
ECompMgrWinFadeOutEnd(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

#if DEBUG_OPACITY
   Eprintf("ECompMgrWinFadeOutEnd %#lx\n", EobjGetXwin(eo));
#endif
   cw->fadeout = 0;
   ECompMgrWinInvalidate(eo, INV_PIXMAP | INV_PICTURE);
   ECompMgrDamageMergeObject(eo, cw->extents, 0);
   _ECM_SET_CLIP_CHANGED();
}

void
ECompMgrWinChangeOpacity(EObj * eo, unsigned int opacity)
{
   ECmWinInfo         *cw = eo->cmhook;

   if (!cw)
      return;

   if (eo->shown && Conf_compmgr.fading.enable && eo->fade)
      ECompMgrWinFade(eo, cw->opacity, opacity);
   else
      ECompMgrWinSetOpacity(eo, opacity);
}

void
ECompMgrWinMap(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   if (!cw)
     {
	ECompMgrWinNew(eo);
	cw = eo->cmhook;
	if (!cw)
	   return;
     }

   D1printf("ECompMgrWinMap %#lx\n", EobjGetXwin(eo));

   if (cw->extents == None)
      cw->extents = win_extents(eo);

   _ECM_SET_STACK_CHANGED();
   ECompMgrDamageMergeObject(eo, cw->extents, 0);

   if (Conf_compmgr.fading.enable && eo->fade)
      ECompMgrWinFadeIn(eo);
}

void
ECompMgrWinUnmap(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   D1printf("ECompMgrWinUnmap %#lx shown=%d\n", EobjGetXwin(eo), eo->shown);
   if (!eo->shown)		/* Sometimes we get a synthetic one too */
      return;

   if (Conf_compmgr.fading.enable && eo->fade && !eo->gone)
      ECompMgrWinFadeOut(eo);
   else
     {
	ECompMgrDamageMergeObject(eo, cw->extents, 0);
	_ECM_SET_STACK_CHANGED();
	ECompMgrWinInvalidate(eo, INV_PIXMAP);
     }
}

static void
ECompMgrWinSetPicts(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   if (cw->pixmap == None && eo->shown && !eo->noredir &&
       (Mode_compmgr.use_pixmap || (eo->fade && Conf_compmgr.fading.enable)))
     {
	cw->pixmap = XCompositeNameWindowPixmap(disp, EobjGetXwin(eo));
	D2printf("ECompMgrWinSetPicts %#lx: Pmap=%#lx\n", EobjGetXwin(eo),
		 cw->pixmap);
     }

   if (cw->picture == None)
     {
	XRenderPictFormat  *pictfmt;
	XRenderPictureAttributes pa;
	Drawable            draw = EobjGetXwin(eo);

	if ((cw->pixmap && Mode_compmgr.use_pixmap) || (cw->fadeout))
	   draw = cw->pixmap;
	if (draw == None)
	   return;

	pictfmt = XRenderFindVisualFormat(disp, WinGetVisual(eo->win));
	pa.subwindow_mode = IncludeInferiors;
	cw->picture = XRenderCreatePicture(disp, draw,
					   pictfmt, CPSubwindowMode, &pa);
	D2printf("ECompMgrWinSetPicts %#lx: Pict=%#lx (drawable=%#lx)\n",
		 EobjGetXwin(eo), cw->picture, draw);

	if (draw == cw->pixmap)
	  {
	     XserverRegion       clip;

	     clip = ERegionCreateFromWindow(EobjGetXwin(eo));
	     XFixesSetPictureClipRegion(disp, cw->picture, 0, 0, clip);
	     ERegionDestroy(clip);
	  }
     }
}

void
ECompMgrWinNew(EObj * eo)
{
   ECmWinInfo         *cw;

   if (!Mode_compmgr.active)	/* FIXME - Here? */
      return;

   if (eo->inputonly || eo->win == VRoot.win)
      return;

   cw = Ecalloc(1, sizeof(ECmWinInfo));
   if (!cw)
      return;

   D1printf("ECompMgrWinNew %#lx\n", EobjGetXwin(eo));

   eo->cmhook = cw;

   if (eo->type == EOBJ_TYPE_EXT &&
       Conf_compmgr.override_redirect.mode == ECM_OR_UNREDIRECTED)
     {
	eo->noredir = 1;
	eo->fade = 0;
	eo->shadow = 0;
     }

   if (!eo->noredir)
     {
	if (Conf_compmgr.mode == ECM_MODE_WINDOW)
	   XCompositeRedirectWindow(disp, EobjGetXwin(eo),
				    CompositeRedirectManual);
	cw->damage_sequence = NextRequest(disp);
	cw->damage =
	   XDamageCreate(disp, EobjGetXwin(eo), XDamageReportNonEmpty);
     }

   if (eo->type == EOBJ_TYPE_EXT)
      eo->opacity = Mode_compmgr.opac_or;
   if (eo->opacity == 0)
      eo->opacity = 0xFFFFFFFF;

   if (eo->type == EOBJ_TYPE_DESK || eo->type == EOBJ_TYPE_ROOT_BG)
     {
	ESelectInputAdd(eo->win, VisibilityChangeMask);
     }

   cw->opacity = 0xdeadbeef;
   ECompMgrWinSetOpacity(eo, eo->opacity);

   EventCallbackRegister(eo->win, 0, ECompMgrHandleWindowEvent, eo);
}

void
ECompMgrWinMoveResize(EObj * eo, int change_xy, int change_wh, int change_bw)
{
   ECmWinInfo         *cw = eo->cmhook;
   XserverRegion       damage;
   int                 invalidate;

   D1printf("ECompMgrWinMoveResize %#lx xy=%d wh=%d bw=%d\n",
	    EobjGetXwin(eo), change_xy, change_wh, change_bw);

   invalidate = 0;
   if (change_xy || change_bw)
      invalidate |= INV_POS;
   if (change_wh || change_bw)
      invalidate |= INV_SIZE;

   if (!invalidate)
      return;

   if (cw->fadeout)
     {
	ECompMgrWinFadeCancel(eo);
	ECompMgrWinFadeOutEnd(eo);
	cw->fading = 0;
     }

   if (!eo->shown)
     {
	ECompMgrWinInvalidate(eo, invalidate);
	return;
     }

   /* Invalidate old window region */
   damage = cw->extents;
   cw->extents = None;
   if (EventDebug(EDBUG_TYPE_COMPMGR3))
      ERegionShow("old-extents:", damage);

#if 0				/* FIXME - We shouldn't have to update clip if transparent */
   if (cw->mode == WINDOW_UNREDIR || cw->mode == WINDOW_SOLID)
#endif
      _ECM_SET_CLIP_CHANGED();
   ECompMgrWinInvalidate(eo, invalidate);

   /* Invalidate new window region */
   cw->extents = win_extents(eo);
   if (damage != None)
     {
	ERegionUnion(damage, cw->extents);
	ECompMgrDamageMergeObject(eo, damage, 1);
     }
   else
     {
	ECompMgrDamageMergeObject(eo, cw->extents, 0);
     }
}

void
ECompMgrWinDamageArea(EObj * eo, int x __UNUSED__, int y __UNUSED__,
		      int w __UNUSED__, int h __UNUSED__)
{
   ECmWinInfo         *cw = eo->cmhook;

   ECompMgrDamageMergeObject(eo, cw->shape, 0);
}

static void
ECompMgrWinConfigure(EObj * eo, XEvent * ev)
{
   int                 x, y, w, h, bw;
   int                 change_xy, change_wh, change_bw;

   x = ev->xconfigure.x;
   y = ev->xconfigure.y;
   w = ev->xconfigure.width;
   h = ev->xconfigure.height;
   bw = ev->xconfigure.border_width;

   change_xy = EobjGetX(eo) != x || EobjGetY(eo) != y;
   change_wh = EobjGetW(eo) != w || EobjGetH(eo) != h;
   change_bw = EobjGetBW(eo) != bw;

   EWindowSetGeometry(eo->win, x, y, w, h, bw);

   ECompMgrWinMoveResize(eo, change_xy, change_wh, change_bw);
}

void
ECompMgrWinReparent(EObj * eo, Desk * dsk, int change_xy)
{
   ECmWinInfo         *cw = eo->cmhook;

   D1printf("ECompMgrWinReparent %#lx %#lx d=%d->%d x,y=%d,%d %d\n",
	    EobjGetXwin(eo), cw->extents, eo->desk->num, dsk->num,
	    EobjGetX(eo), EobjGetY(eo), change_xy);

   /* Invalidate old window region */
   if (EventDebug(EDBUG_TYPE_COMPMGR3))
      ERegionShow("old-extents:", cw->extents);
   ECompMgrDamageMergeObject(eo, cw->extents, change_xy);
   if (change_xy)
     {
	cw->extents = None;
	ECompMgrWinInvalidate(eo, INV_POS);

	/* Find new window region */
	cw->extents = win_extents(eo);
     }
   eo->desk = dsk;
   _ECM_SET_STACK_CHANGED();
   ECompMgrDamageMergeObject(eo, cw->extents, 0);
   ECompMgrWinInvalidate(eo, INV_PIXMAP);
}

static void
ECompMgrWinCirculate(EObj * eo, XEvent * ev)
{
   D1printf("ECompMgrWinCirculate %#lx %#lx\n", ev->xany.window,
	    EobjGetXwin(eo));

   _ECM_SET_STACK_CHANGED();
}

void
ECompMgrWinChangeShape(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   D1printf("ECompMgrWinChangeShape %#lx\n", EobjGetXwin(eo));

   ECompMgrDamageMergeObject(eo, cw->extents, 1);
   cw->extents = None;

   _ECM_SET_CLIP_CHANGED();
   ECompMgrWinInvalidate(eo, INV_SIZE);
}

void
ECompMgrWinRaiseLower(EObj * eo, int delta)
{
   ECmWinInfo         *cw = eo->cmhook;

   D1printf("ECompMgrWinRaiseLower %#lx delta=%d\n", EobjGetXwin(eo), delta);

   if (delta < 0)		/* Raise */
      _ECM_SET_STACK_CHANGED();
   ECompMgrDamageMergeObject(eo, cw->extents, 0);
   if (delta > 0)		/* Lower */
      _ECM_SET_STACK_CHANGED();
}

void
ECompMgrWinDel(EObj * eo)
{
   ECmWinInfo         *cw = eo->cmhook;

   if (!cw)
      return;

   D1printf("ECompMgrWinDel %#lx\n", EobjGetXwin(eo));

   if (cw->fading)
     {
	ECompMgrWinFadeCancel(eo);
	if (cw->fadeout)
	   ECompMgrWinFadeOutEnd(eo);
     }

   EventCallbackUnregister(eo->win, 0, ECompMgrHandleWindowEvent, eo);

   if (!eo->gone)
      ECompMgrWinInvalidate(eo, INV_PICTURE);

   if (!eo->noredir && !eo->gone)
     {
	if (cw->damage != None)
	   XDamageDestroy(disp, cw->damage);

	if (Conf_compmgr.mode == ECM_MODE_WINDOW)
	   XCompositeUnredirectWindow(disp, EobjGetXwin(eo),
				      CompositeRedirectManual);
     }

   ECompMgrWinInvalidate(eo, INV_ALL);

   _EFREE(eo->cmhook);

   _ECM_SET_STACK_CHANGED();
}

static void
ECompMgrWinDamage(EObj * eo, XEvent * ev __UNUSED__)
{
   ECmWinInfo         *cw = eo->cmhook;
   Display            *dpy = disp;
   XDamageNotifyEvent *de = (XDamageNotifyEvent *) ev;
   XserverRegion       parts;

   D1printf("ECompMgrWinDamage %#lx %#lx damaged=%d %d,%d %dx%d\n",
	    ev->xany.window, EobjGetXwin(eo), cw->damaged,
	    de->area.x, de->area.y, de->area.width, de->area.height);

   if (!cw->damaged)
     {
	parts = win_extents(eo);
	XDamageSubtract(dpy, cw->damage, None, None);
	cw->damaged = 1;
     }
   else
     {
	parts = ERegionCreate();
	XDamageSubtract(dpy, cw->damage, None, parts);
	ERegionTranslate(parts, EobjGetX(eo) + EobjGetBW(eo),
			 EobjGetY(eo) + EobjGetBW(eo));
#if 0				/* ENABLE_SHADOWS - FIXME - This is not right, remove? */
	if (Mode_compmgr.shadow_mode == ECM_SHADOWS_SHARP)
	  {
	     XserverRegion       o;

	     o = ERegionClone(parts);
	     ERegionTranslate(o, cw->shadow_dx, cw->shadow_dy);
	     ERegionUnion(parts, o);
	     ERegionDestroy(o);
	  }
#endif
     }
   eo->serial = ev->xany.serial;
   ECompMgrDamageMergeObject(eo, parts, 1);
}

static void
ECompMgrWinDumpInfo(const char *txt, EObj * eo, XserverRegion rgn, int force)
{
   ECmWinInfo         *cw = eo->cmhook;

   Eprintf("%s %#lx: %d,%d %dx%d: %s\n", txt, EobjGetXwin(eo),
	   EobjGetX(eo), EobjGetY(eo), EobjGetW(eo), EobjGetH(eo), eo->name);
   if (!cw)
     {
	Eprintf("Not managed\n");
	return;
     }

   if (force || EventDebug(EDBUG_TYPE_COMPMGR3))
     {
	Eprintf(" - pict=%#lx pmap=%#lx\n", cw->picture, cw->pixmap);

	ERegionShow("win extents", cw->extents);
	ERegionShow("win shape  ", cw->shape);
	ERegionShow("win clip   ", cw->clip);
	if (rgn != None)
	   ERegionShow("region", rgn);
     }
}

static void
ECompMgrDestroyClip(void)
{
   EObj               *eo, *const *lst;
   ECmWinInfo         *cw;
   int                 i, num;

   lst = EobjListStackGet(&num);
   for (i = 0; i < num; i++)
     {
	eo = lst[i];
	cw = eo->cmhook;
	if (!cw)
	   continue;
	if (cw->clip != None)
	   ERegionDestroy(cw->clip);
	cw->clip = None;
     }
}

static int
ECompMgrDetermineOrder(EObj * const *lst, int num, EObj ** first,
		       EObj ** last, Desk * dsk, XserverRegion clip)
{
   EObj               *eo, *eo_prev, *eo_first;
   int                 i, stop, destroy_clip;
   ECmWinInfo         *cw;

   D1printf("ECompMgrDetermineOrder %d\n", dsk->num);
   if (!lst)
      lst = EobjListStackGet(&num);
   if (clip == None)
     {
	destroy_clip = 1;
	ECompMgrDestroyClip();
	clip = ERegionCreate();
     }
   else
     {
	destroy_clip = 0;
     }

   /* Determine overall paint order, top to bottom */
   stop = 0;
   eo_first = eo_prev = NULL;

   for (i = 0; i < num; i++)
     {
	eo = lst[i];

	cw = eo->cmhook;

	if (!cw)
	   continue;

	if ((!eo->shown && !cw->fading) || eo->desk != dsk)
	   continue;

	/* Region of shaped window in screen coordinates */
	if (cw->shape == None)
	   cw->shape = win_shape(eo);

	/* Region of window in screen coordinates, including shadows */
	if (cw->extents == None)
	   cw->extents = win_extents(eo);

	D4printf(" - %#lx desk=%d shown=%d fading=%d fadeout=%d\n",
		 EobjGetXwin(eo), eo->desk->num, eo->shown, cw->fading,
		 cw->fadeout);

	if (eo->type == EOBJ_TYPE_DESK)
	  {
	     EObj               *eo1, *eo2;
	     Desk               *d = (Desk *) eo;

	     if (!d->viewable)
		continue;

#if USE_CLIP_RELATIVE_TO_DESK
	     ERegionTranslate(clip, -EoGetX(d), -EoGetY(d));
#endif
	     stop = ECompMgrDetermineOrder(lst, num, &eo1, &eo2, d, clip);
#if USE_CLIP_RELATIVE_TO_DESK
	     ERegionTranslate(clip, EoGetX(d), EoGetY(d));
#endif
	     if (eo1)
	       {
		  if (!eo_first)
		     eo_first = eo1;
		  if (eo_prev)
		     ((ECmWinInfo *) (eo_prev->cmhook))->next = eo1;
		  ((ECmWinInfo *) (eo1->cmhook))->prev = eo_prev;
		  eo_prev = eo2;
	       }

#if USE_BG_WIN_ON_ALL_DESKS	/* Only if using per desk bg overlay */
	     /* FIXME - We should break when the clip region becomes empty */
	     if (EobjGetX(eo) == 0 && EobjGetY(eo) == 0)
		stop = 1;
	     if (stop)
		break;
#endif
	  }

	cw->clip = ERegionClone(clip);

	ECompMgrWinSetPicts(eo);

	D4printf(" - %#lx desk=%d shown=%d dam=%d pict=%#lx\n",
		 EobjGetXwin(eo), eo->desk->num, eo->shown, cw->damaged,
		 cw->picture);

#if 0				/* FIXME - Need this? */
	if (!cw->damaged)
	   continue;
#endif
	if (cw->picture == None && !eo->noredir)
	   continue;

	D4printf
	   ("ECompMgrDetermineOrder hook in %d - %#lx desk=%d shown=%d\n",
	    dsk->num, EobjGetXwin(eo), eo->desk->num, eo->shown);

	if (!eo_first)
	   eo_first = eo;
	cw->prev = eo_prev;
	if (eo_prev)
	   ((ECmWinInfo *) (eo_prev->cmhook))->next = eo;
	eo_prev = eo;

	switch (cw->mode)
	  {
	  case WINDOW_UNREDIR:
	  case WINDOW_SOLID:
	     D4printf("-   clip %#lx %#lx %d,%d %dx%d: %s\n", EobjGetXwin(eo),
		      cw->clip, EobjGetX(eo), EobjGetY(eo), EobjGetW(eo),
		      EobjGetH(eo), eo->name);
#if USE_CLIP_RELATIVE_TO_DESK
	     ERegionUnionOffset(clip, 0, 0, cw->shape);
#else
	     ERegionUnionOffset(clip, EoGetX(dsk), EoGetY(dsk), cw->shape);
#endif
	     break;

	  default:
	     D4printf("- noclip %#lx %#lx %d,%d %dx%d: %s\n", EobjGetXwin(eo),
		      cw->clip, EobjGetX(eo), EobjGetY(eo), EobjGetW(eo),
		      EobjGetH(eo), eo->name);
	     break;
	  }

#if !USE_BG_WIN_ON_ALL_DESKS	/* Not if using per desk bg overlay */
	/* FIXME - We should break when the clip region becomes empty */
	if (eo->type == EOBJ_TYPE_DESK &&
	    EobjGetX(eo) == 0 && EobjGetY(eo) == 0)
	   stop = 1;
	if (stop)
	   break;
#endif
     }
   if (eo_prev)
      ((ECmWinInfo *) (eo_prev->cmhook))->next = NULL;

   *first = eo_first;
   *last = eo_prev;

   if (destroy_clip)
      ERegionDestroy(clip);
   Mode_compmgr.reorder = 0;
   return stop;
}

static              XserverRegion
ECompMgrRepaintObjSetClip(XserverRegion rgn, XserverRegion damage,
			  XserverRegion clip, int x, int y)
{
   ERegionCopy(rgn, damage);
#if USE_CLIP_RELATIVE_TO_DESK
   ERegionSubtractOffset(rgn, x, y, clip);
#else
   ERegionSubtractOffset(rgn, 0, 0, clip);
   x = y = 0;
#endif
   return rgn;
}

static void
ECompMgrRepaintObj(Picture pbuf, XserverRegion region, EObj * eo, int mode)
{
   static XserverRegion rgn_clip = None;
   Display            *dpy = disp;
   ECmWinInfo         *cw;
   Desk               *dsk = eo->desk;
   int                 x, y;
   XserverRegion       clip;
   Picture             alpha;

   cw = eo->cmhook;

#if 0				/* FIXME - Remove? */
   if (!cw->shape)
      cw->shape = win_shape(eo);
   if (!cw->extents)
      cw->extents = win_extents(eo);
#endif

   if (rgn_clip == None)
      rgn_clip = ERegionCreate();

   x = EoGetX(dsk);
   y = EoGetY(dsk);

   if (mode == 0)
     {
	/* Painting opaque windows top down. */
#if 0
	if (ERegionIsEmpty(clip))
	  {
	     D2printf(" - Quit - repaint region is empty\n");
	     return;
	  }
#endif

	switch (cw->mode)
	  {
	  case WINDOW_UNREDIR:
	  case WINDOW_SOLID:
	     clip = ECompMgrRepaintObjSetClip(rgn_clip, region, cw->clip, x, y);
	     if (EventDebug(EDBUG_TYPE_COMPMGR2))
		ECompMgrWinDumpInfo("ECompMgrRepaintObj solid", eo, clip, 0);
	     XFixesSetPictureClipRegion(dpy, pbuf, 0, 0, clip);
	     XRenderComposite(dpy, PictOpSrc, cw->picture, None, pbuf,
			      0, 0, 0, 0, x + cw->rcx, y + cw->rcy, cw->rcw,
			      cw->rch);
	     break;
	  }
     }
   else
     {
	/* Painting trans stuff bottom up. */

	switch (cw->mode)
	  {
	  default:
	     clip = None;
	     break;

	  case WINDOW_TRANS:
	  case WINDOW_ARGB:
	     clip = ECompMgrRepaintObjSetClip(rgn_clip, region, cw->clip, x, y);
	     if (EventDebug(EDBUG_TYPE_COMPMGR2))
		ECompMgrWinDumpInfo("ECompMgrRepaintObj trans", eo, clip, 0);
	     XFixesSetPictureClipRegion(dpy, pbuf, 0, 0, clip);
	     if (cw->opacity != OPAQUE && !cw->pict_alpha)
		cw->pict_alpha =
		   EPictureCreateSolid(True, OP32(cw->opacity), 0., 0., 0.);
	     XRenderComposite(dpy, PictOpOver, cw->picture, cw->pict_alpha,
			      pbuf, 0, 0, 0, 0, x + cw->rcx, y + cw->rcy,
			      cw->rcw, cw->rch);
	     break;
	  }

#if ENABLE_SHADOWS
	if (!cw->has_shadow)
	   return;

	if (clip == None)
	   clip = ECompMgrRepaintObjSetClip(rgn_clip, region, cw->clip, x, y);
	ERegionSubtractOffset(clip, x, y, cw->shape);
	XFixesSetPictureClipRegion(dpy, pbuf, 0, 0, clip);

	switch (Mode_compmgr.shadow_mode)
	  {
	  case ECM_SHADOWS_SHARP:
	  case ECM_SHADOWS_ECHO:
	     if (cw->opacity != OPAQUE && !cw->shadow_alpha)
		cw->shadow_alpha =
		   EPictureCreateSolid(True,
				       OP32(cw->opacity) *
				       Mode_compmgr.opac_sharp, 0., 0., 0.);
	     alpha = cw->shadow_alpha ? cw->shadow_alpha : transBlackPicture;
	     if (Mode_compmgr.shadow_mode == ECM_SHADOWS_SHARP)
		XRenderComposite(dpy, PictOpOver, alpha, cw->picture, pbuf,
				 0, 0, 0, 0,
				 x + cw->rcx + cw->shadow_dx,
				 y + cw->rcy + cw->shadow_dy,
				 cw->shadow_width, cw->shadow_height);
	     else
		XRenderComposite(dpy, PictOpOver, cw->picture, alpha, pbuf,
				 0, 0, 0, 0,
				 x + cw->rcx + cw->shadow_dx,
				 y + cw->rcy + cw->shadow_dy,
				 cw->shadow_width, cw->shadow_height);
	     break;

	  case ECM_SHADOWS_BLURRED:
	     if (cw->shadow_pict == None)
		break;

	     if (cw->opacity != OPAQUE && !cw->pict_alpha)
		cw->pict_alpha =
		   EPictureCreateSolid(True, OP32(cw->opacity), 0., 0., 0.);
	     alpha = (cw->pict_alpha) ? cw->pict_alpha : transBlackPicture;
	     XRenderComposite(dpy, PictOpOver, alpha, cw->shadow_pict, pbuf,
			      0, 0, 0, 0,
			      x + cw->rcx + cw->shadow_dx,
			      y + cw->rcy + cw->shadow_dy,
			      cw->shadow_width, cw->shadow_height);
	     break;
	  }
#endif
     }
}

void
ECompMgrRepaint(void)
{
   Display            *dpy = disp;
   EObj               *eo;
   Picture             pbuf;
   Desk               *dsk = DeskGet(0);

   if (!Mode_compmgr.active || allDamage == None)
      return;

   ERegionLimit(allDamage);

   D2printf("ECompMgrRepaint rootBuffer=%#lx rootPicture=%#lx\n",
	    rootBuffer, rootPicture);
   if (EventDebug(EDBUG_TYPE_COMPMGR))
      ERegionShow("allDamage", allDamage);

   if (!rootBuffer)
      rootBuffer = EPictureCreateBuffer(VRoot.xwin, VRoot.w, VRoot.h,
					VRoot.depth, VRoot.vis);
   pbuf = rootBuffer;

   if (!dsk)
      return;

   /* Do paint order list linking */
   if (Mode_compmgr.reorder)
      ECompMgrDetermineOrder(NULL, 0, &Mode_compmgr.eo_first,
			     &Mode_compmgr.eo_last, dsk, None);

   /* Paint opaque windows top down */
   for (eo = Mode_compmgr.eo_first; eo;
	eo = ((ECmWinInfo *) (eo->cmhook))->next)
      ECompMgrRepaintObj(pbuf, allDamage, eo, 0);

#if 0				/* FIXME - NoBg? */
   Picture             pict;

   if (EventDebug(EDBUG_TYPE_COMPMGR2))
      ERegionShow("after opaque", region);

   /* Repaint background, clipped by damage region and opaque windows */
   pict = ((ECmWinInfo *) (dsk->o.cmhook))->picture;
   D1printf("ECompMgrRepaint desk picture=%#lx\n", pict);
   XFixesSetPictureClipRegion(dpy, pbuf, 0, 0, region);
   XRenderComposite(dpy, PictOpSrc, pict, None, pbuf,
		    0, 0, 0, 0, 0, 0, VRoot.w, VRoot.h);
#endif

   /* Paint trans windows and shadows bottom up */
   for (eo = Mode_compmgr.eo_last; eo; eo = ((ECmWinInfo *) (eo->cmhook))->prev)
      ECompMgrRepaintObj(pbuf, allDamage, eo, 1);

   if (pbuf != rootPicture)
     {
	XFixesSetPictureClipRegion(dpy, pbuf, 0, 0, allDamage);
	XRenderComposite(dpy, PictOpSrc, pbuf, None, rootPicture,
			 0, 0, 0, 0, 0, 0, VRoot.w, VRoot.h);
     }

   ERegionDestroy(allDamage);
   allDamage = None;
}

static void
_ECompMgrIdler(void *data __UNUSED__)
{
   /* Do we get here on auto? */
   if (!allDamage /* || Conf_compmgr.mode == ECM_MODE_AUTO */ )
      return;
   ECompMgrRepaint();
#if 0				/* FIXME - Was here - Why? */
   XSync(disp, False);
#endif
}

static void
ECompMgrRootConfigure(void *prm __UNUSED__, XEvent * ev)
{
   Display            *dpy = disp;

   D1printf("ECompMgrRootConfigure root\n");
   if (ev->xconfigure.window == VRoot.xwin)
     {
	if (rootBuffer != None)
	  {
	     XRenderFreePicture(dpy, rootBuffer);
	     rootBuffer = None;
	  }

	if (Mode_compmgr.rgn_screen != None)
	   ERegionDestroy(Mode_compmgr.rgn_screen);
	Mode_compmgr.rgn_screen = None;
     }
   return;
}

#if USE_DESK_EXPOSE		/* FIXME - Remove? */
static void
ECompMgrRootExpose(void *prm __UNUSED__, XEvent * ev)
{
   static XRectangle  *expose_rects = 0;
   static int          size_expose = 0;
   static int          n_expose = 0;
   int                 more = ev->xexpose.count + 1;

   if (ev->xexpose.window != VRoot.xwin)
      return;

   D1printf("ECompMgrRootExpose %d %d %d\n", n_expose, size_expose,
	    ev->xexpose.count);

   if (n_expose == size_expose)
     {
	if (expose_rects)
	  {
	     expose_rects = realloc(expose_rects,
				    (size_expose + more) * sizeof(XRectangle));
	     size_expose += more;
	  }
	else
	  {
	     expose_rects = malloc(more * sizeof(XRectangle));
	     size_expose = more;
	  }
     }
   expose_rects[n_expose].x = ev->xexpose.x;
   expose_rects[n_expose].y = ev->xexpose.y;
   expose_rects[n_expose].width = ev->xexpose.width;
   expose_rects[n_expose].height = ev->xexpose.height;
   n_expose++;
   if (ev->xexpose.count == 0)
     {
	XserverRegion       region;

	region = ERegionCreateFromRects(expose_rects, n_expose);

	ECompMgrDamageMerge(region, 1);
	n_expose = 0;
     }
}
#endif

#if ENABLE_SHADOWS
static void
ECompMgrShadowsInit(int mode, int cleanup)
{
   Mode_compmgr.shadow_mode = mode;

   Conf_compmgr.shadows.blur.opacity =
      OpacityFix(Conf_compmgr.shadows.blur.opacity);
   Mode_compmgr.opac_blur = .01 * Conf_compmgr.shadows.blur.opacity;
   Conf_compmgr.shadows.sharp.opacity =
      OpacityFix(Conf_compmgr.shadows.sharp.opacity);
   Mode_compmgr.opac_sharp = .01 * Conf_compmgr.shadows.sharp.opacity;

   if (gaussianMap)
      free(gaussianMap);
   gaussianMap = NULL;

   if (mode != ECM_SHADOWS_OFF)
     {
	if (mode == ECM_SHADOWS_BLURRED)
	   transBlackPicture = EPictureCreateSolid(True, 1., 0., 0., 0.);
	else
	   transBlackPicture =
	      EPictureCreateSolid(True, Mode_compmgr.opac_sharp, 0., 0., 0.);
     }
   else
     {
	if (transBlackPicture)
	   XRenderFreePicture(disp, transBlackPicture);
	transBlackPicture = None;
     }

   if (cleanup)
     {
	EObj               *const *lst;
	int                 i, num;

	lst = EobjListStackGet(&num);
	for (i = 0; i < num; i++)
	   ECompMgrWinInvalidate(lst[i], INV_SHADOW);
     }
}
#else
#define ECompMgrShadowsInit(mode, cleanup)
#endif

int
ECompMgrIsActive(void)
{
   return Mode_compmgr.active;
}

static void
ECompMgrStart(void)
{
   EObj               *const *lst;
   int                 i, num;
   XRenderPictFormat  *pictfmt;
   XRenderPictureAttributes pa;

   if (Mode_compmgr.active || Conf_compmgr.mode == ECM_MODE_OFF)
      return;
   Conf_compmgr.enable = Mode_compmgr.active = 1;

   Conf_compmgr.override_redirect.opacity =
      OpacityFix(Conf_compmgr.override_redirect.opacity);
   Mode_compmgr.opac_or =
      OpacityFromPercent(Conf_compmgr.override_redirect.opacity);

   pa.subwindow_mode = IncludeInferiors;
   pictfmt = XRenderFindVisualFormat(disp, VRoot.vis);
   rootPicture =
      XRenderCreatePicture(disp, VRoot.xwin, pictfmt, CPSubwindowMode, &pa);

   ECompMgrShadowsInit(Conf_compmgr.shadows.mode, 0);

   EGrabServer();

   switch (Conf_compmgr.mode)
     {
     case ECM_MODE_ROOT:
	XCompositeRedirectSubwindows(disp, VRoot.xwin, CompositeRedirectManual);
#if USE_DESK_EXPOSE		/* FIXME - Remove? */
	ESelectInputAdd(VRoot.xwin, ExposureMask);
#endif
	break;
     case ECM_MODE_WINDOW:
#if USE_DESK_EXPOSE		/* FIXME - Remove? */
	ESelectInputAdd(VRoot.xwin, ExposureMask);
#endif
	break;
     case ECM_MODE_AUTO:
	XCompositeRedirectSubwindows(disp, VRoot.xwin,
				     CompositeRedirectAutomatic);
	break;
     }

   allDamage = None;

   EventCallbackRegister(VRoot.win, 0, ECompMgrHandleRootEvent, NULL);

   wm_cm_sel = SelectionAcquire("_NET_WM_CM_S", NULL, NULL);

   lst = EobjListStackGet(&num);
   for (i = 0; i < num; i++)
     {
	ECompMgrWinNew(lst[i]);
	if (lst[i]->shown)
	   ECompMgrWinMap(lst[i]);
     }

#if !USE_BG_WIN_ON_ALL_DESKS
   DesksBackgroundRefresh(NULL, DESK_BG_RECONFIGURE_ALL);
#endif
   _ECM_SET_CLIP_CHANGED();
   EUngrabServer();
   ESync();
}

static void
ECompMgrStop(void)
{
   EObj               *const *lst1, **lst;
   int                 i, num;

   if (!Mode_compmgr.active)
      return;
   Conf_compmgr.enable = Mode_compmgr.active = 0;

   EGrabServer();

   SelectionRelease(wm_cm_sel);
   wm_cm_sel = NULL;

   if (rootPicture)
      XRenderFreePicture(disp, rootPicture);
   rootPicture = None;

   if (rootBuffer)
      XRenderFreePicture(disp, rootBuffer);
   rootBuffer = None;

   ECompMgrShadowsInit(ECM_SHADOWS_OFF, 0);

   lst1 = EobjListStackGet(&num);
   if (num > 0)
     {
	lst = Emalloc(num * sizeof(EObj *));
	if (lst)
	  {
	     memcpy(lst, lst1, num * sizeof(EObj *));
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->type == EOBJ_TYPE_EXT)
		     EobjUnregister(lst[i]);	/* Modifies the object stack! */
		  else
		     ECompMgrWinDel(lst[i]);
	       }
	     Efree(lst);
	  }
     }

   if (allDamage != None)
      ERegionDestroy(allDamage);
   allDamage = None;

   if (Mode_compmgr.rgn_screen != None)
      ERegionDestroy(Mode_compmgr.rgn_screen);
   Mode_compmgr.rgn_screen = None;

   if (Conf_compmgr.mode == ECM_MODE_ROOT)
      XCompositeUnredirectSubwindows(disp, VRoot.xwin, CompositeRedirectManual);

   EventCallbackUnregister(VRoot.win, 0, ECompMgrHandleRootEvent, NULL);

#if !USE_BG_WIN_ON_ALL_DESKS
   DesksBackgroundRefresh(NULL, DESK_BG_RECONFIGURE_ALL);
#endif
   EUngrabServer();
   ESync();
}

void
ECompMgrConfigGet(cfg_composite * cfg)
{
   cfg->enable = Conf_compmgr.enable;
   cfg->shadow = Conf_compmgr.shadows.mode;
   cfg->fading = Conf_compmgr.fading.enable;
   cfg->fade_speed = 100 - (Conf_compmgr.fading.dt_us / 1000);
}

void
ECompMgrConfigSet(const cfg_composite * cfg)
{
   if (Conf_compmgr.mode == ECM_MODE_OFF)
     {
	if (cfg->enable)
	   DialogOK("Enable Composite Error",
		    _("Cannot enable Composite Manager.\n"
		      "Use xdpyinfo to check that\n"
		      "Composite, Damage, Fixes, and Render\n"
		      "extensions are loaded."));
	return;
     }

   if (cfg->enable != Conf_compmgr.enable)
     {
	Conf_compmgr.enable = cfg->enable;
	Conf_compmgr.shadows.mode = cfg->shadow;
	if (cfg->enable)
	   ECompMgrStart();
	else
	   ECompMgrStop();
     }
   else
     {
	if (cfg->shadow != Conf_compmgr.shadows.mode)
	  {
	     Conf_compmgr.shadows.mode = cfg->shadow;
	     if (Conf_compmgr.enable)
	       {
		  ECompMgrShadowsInit(Conf_compmgr.shadows.mode, 1);
		  ECompMgrDamageAll();
	       }
	  }
     }

   Conf_compmgr.fading.enable = cfg->fading;
   Conf_compmgr.fading.dt_us = (100 - cfg->fade_speed) * 1000;

   autosave();
}

/*
 * Event handlers
 */
#define USE_WINDOW_EVENTS 0

static void
ECompMgrHandleWindowEvent(Win win __UNUSED__, XEvent * ev, void *prm)
{
   EObj               *eo = prm;

   D2printf("ECompMgrHandleWindowEvent: type=%d\n", ev->type);

   switch (ev->type)
     {
#if USE_WINDOW_EVENTS
     case ConfigureNotify:
	ECompMgrWinConfigure(eo, ev);
	break;

     case MapNotify:
	ECompMgrWinMap(eo);
	break;
     case UnmapNotify:
	if (eo->type == EOBJ_TYPE_EXT && eo->cmhook)
	  {
	     ECompMgrWinUnmap(eo);
	     eo->shown = 0;
	  }
	break;

     case CirculateNotify:
	ECompMgrWinCirculate(eo, ev);
	break;
#endif

#if USE_DESK_VISIBILITY
     case VisibilityNotify:
	ECompMgrDeskVisibility(eo, ev);
	break;
#endif

     case EX_EVENT_DAMAGE_NOTIFY:
	ECompMgrWinDamage(eo, ev);
	break;
     }
}

static void
ECompMgrHandleRootEvent(Win win __UNUSED__, XEvent * ev, void *prm)
{
   Window              xwin;
   EObj               *eo;

   D2printf("ECompMgrHandleRootEvent: type=%d\n", ev->type);

   switch (ev->type)
     {
     case CreateNotify:
	xwin = ev->xcreatewindow.window;
      case_CreateNotify:
	if (Conf_compmgr.override_redirect.mode != ECM_OR_ON_CREATE)
	   break;
	eo = EobjListStackFind(xwin);
	if (!eo)
	   EobjRegister(xwin, EOBJ_TYPE_EXT);
	break;

     case DestroyNotify:
	xwin = ev->xdestroywindow.window;
      case_DestroyNotify:
	eo = EobjListStackFind(xwin);
	if (eo && eo->type == EOBJ_TYPE_EXT)
	  {
	     if (ev->type == DestroyNotify)
		eo->gone = 1;
	     EobjUnregister(eo);
	  }
	break;

     case ReparentNotify:
     case EX_EVENT_REPARENT_GONE:
	xwin = ev->xreparent.window;
	if (ev->xreparent.parent == VRoot.xwin)
	   goto case_CreateNotify;
	else
	   goto case_DestroyNotify;

     case ConfigureNotify:
	if (ev->xconfigure.window == VRoot.xwin)
	  {
	     ECompMgrRootConfigure(prm, ev);
	  }
	else
	  {
	     eo = EobjListStackFind(ev->xconfigure.window);
	     if (eo && eo->type == EOBJ_TYPE_EXT && eo->cmhook)
	       {
		  ECompMgrWinConfigure(eo, ev);
	       }
	  }
	break;

     case MapNotify:
	eo = EobjListStackFind(ev->xmap.window);
	if (!eo)
	   eo = EobjRegister(ev->xmap.window, EOBJ_TYPE_EXT);
	if (eo && eo->type == EOBJ_TYPE_EXT && eo->cmhook)
	  {
	     eo->shown = 1;
	     EobjListStackRaise(eo, 0);
	     ECompMgrWinMap(eo);
	  }
	break;

     case UnmapNotify:
     case EX_EVENT_UNMAP_GONE:
	eo = EobjListStackFind(ev->xunmap.window);
	if (eo && eo->type == EOBJ_TYPE_EXT && eo->cmhook)
	  {
	     if (ev->type == EX_EVENT_UNMAP_GONE)
		eo->gone = 1;
#if 0
	     /* No. Unredirection seems to cause map/unmap => loop */
	     if (Conf_compmgr.override_redirect.mode == ECM_OR_ON_MAPUNMAP)
	       {
		  EobjUnregister(eo);
	       }
	     else
#endif
	       {
		  ECompMgrWinUnmap(eo);
		  eo->shown = 0;
	       }
	  }
	break;

     case CirculateNotify:
	eo = EobjListStackFind(ev->xcirculate.window);
	if (eo && eo->cmhook)
	   ECompMgrWinCirculate(eo, ev);
	break;

#if USE_DESK_EXPOSE		/* FIXME - Remove? */
     case Expose:
	if (Mode_compmgr.shadow_mode != ECM_SHADOWS_OFF)
	   ECompMgrRootExpose(prm, ev);
	break;
#endif
     }
}

/*
 * Module functions
 */

static void
ECompMgrInit(void)
{
   if (!XEXT_AVAILABLE(XEXT_CM_ALL))
     {
	Conf_compmgr.mode = ECM_MODE_OFF;
	goto done;
     }

   Mode_compmgr.use_pixmap = Conf_compmgr.use_name_pixmap;

   if (Conf_compmgr.mode == ECM_MODE_OFF)
      Conf_compmgr.mode = ECM_MODE_ROOT;

   /* FIXME - Hardcode for now. */
   Conf_compmgr.mode = ECM_MODE_WINDOW;

 done:
   if (Conf_compmgr.mode == ECM_MODE_OFF)
      Conf_compmgr.enable = 0;
   D1printf("ECompMgrInit: enable=%d mode=%d\n", Conf_compmgr.enable,
	    Conf_compmgr.mode);
}

static void
ECompMgrSighan(int sig, void *prm __UNUSED__)
{
   if (sig != ESIGNAL_INIT && Conf_compmgr.mode == ECM_MODE_OFF)
      return;

   switch (sig)
     {
     case ESIGNAL_INIT:
	ECompMgrInit();
	if (Conf_compmgr.enable)
	   ECompMgrStart();
	IdlerAdd(50, _ECompMgrIdler, NULL);
	break;
     }
}

static void
CompMgrIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 len;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %4000s %n", cmd, prm, &len);
	p += len;
     }

   if (!p || cmd[0] == '?')
     {
	IpcPrintf("CompMgr - on=%d\n", Mode_compmgr.active);
     }
   else if (!strcmp(cmd, "cfg"))
     {
	SettingsComposite();
     }
   else if (!strcmp(cmd, "start"))
     {
	ECompMgrStart();
	autosave();
     }
   else if (!strcmp(cmd, "stop"))
     {
	ECompMgrStop();
	autosave();
     }
   else if (!strncmp(cmd, "list", 2))
     {
	/* TBD */
     }
   else if (!strncmp(cmd, "oi", 2))
     {
	Window              win;
	EObj               *eo;

	win = None;
	sscanf(prm, "%lx", &win);
	eo = EobjListStackFind(win);
	if (eo)
	   ECompMgrWinDumpInfo("EObj", eo, None, 1);
     }
}

static const IpcItem CompMgrIpcArray[] = {
   {
    CompMgrIpc,
    "compmgr", "cm",
    "Composite manager functions",
    "  cm ?                     Show info\n"
    "  cm cfg                   Configure\n"
    "  cm start                 Start composite manager\n"
    "  cm stop                  Stop composite manager\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(CompMgrIpcArray)/sizeof(IpcItem))

static const CfgItem CompMgrCfgItems[] = {
   CFG_ITEM_BOOL(Conf_compmgr, enable, 0),
   CFG_ITEM_INT(Conf_compmgr, mode, 1),
   CFG_ITEM_INT(Conf_compmgr, shadows.mode, 0),
   CFG_ITEM_INT(Conf_compmgr, shadows.offset_x, 3),
   CFG_ITEM_INT(Conf_compmgr, shadows.offset_y, 5),
   CFG_ITEM_INT(Conf_compmgr, shadows.blur.radius, 5),
   CFG_ITEM_INT(Conf_compmgr, shadows.blur.opacity, 75),
   CFG_ITEM_INT(Conf_compmgr, shadows.sharp.opacity, 30),
   CFG_ITEM_BOOL(Conf_compmgr, resize_fix_enable, 0),
   CFG_ITEM_BOOL(Conf_compmgr, use_name_pixmap, 0),
   CFG_ITEM_BOOL(Conf_compmgr, fading.enable, 1),
   CFG_ITEM_INT(Conf_compmgr, fading.dt_us, 10000),
   CFG_ITEM_HEX(Conf_compmgr, fading.step, 0x10000000),
   CFG_ITEM_INT(Conf_compmgr, override_redirect.mode, 1),
   CFG_ITEM_INT(Conf_compmgr, override_redirect.opacity, 90),
};
#define N_CFG_ITEMS (sizeof(CompMgrCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
const EModule       ModCompMgr = {
   "compmgr", "cm",
   ECompMgrSighan,
   {N_IPC_FUNCS, CompMgrIpcArray},
   {N_CFG_ITEMS, CompMgrCfgItems}
};

#endif /* USE_COMPOSITE */

/*
 * $Id: xcompmgr.c,v 1.26 2004/08/14 21:39:51 keithp Exp $
 *
 * Copyright © 2003 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Modified by Matthew Hawn. I don't know what to say here so follow what it 
 * says above. Not that I can really do anything about it
 */
