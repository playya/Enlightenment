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
#include "ecore-e16.h"
#include <X11/Xutil.h>
#include <X11/Xresource.h>

typedef struct
{
   EventCallbackFunc  *func;
   void               *prm;
} EventCallbackItem;

typedef struct
{
   int                 num;
   EventCallbackItem  *lst;
} EventCallbackList;

typedef struct _exid
{
   EventCallbackList   cbl;
   Window              parent;
   Window              win;
   int                 x, y, w, h;
   char                mapped;
   char                in_use;
   char                do_del;
   char                attached;
   int                 num_rect;
   int                 ord;
   XRectangle         *rects;
   int                 depth;
   Pixmap              bgpmap;
   int                 bgcol;
} EXID;

static XContext     xid_context = 0;

static EXID        *
EXidCreate(void)
{
   EXID               *xid;

   xid = Ecalloc(1, sizeof(EXID));
   xid->bgcol = -1;

   return xid;
}

static void
EXidDestroy(EXID * xid)
{
#if 0
   Eprintf("EXidDestroy: %p %#lx\n", xid, xid->win);
#endif
   if (xid->rects)
      XFree(xid->rects);
   if (xid->cbl.lst)
      Efree(xid->cbl.lst);
   Efree(xid);
}

static void
EXidAdd(EXID * xid)
{
   if (!xid_context)
      xid_context = XUniqueContext();

   XSaveContext(disp, xid->win, xid_context, (XPointer) xid);
   AddItem(xid, "", xid->win, LIST_TYPE_XID);
}

static void
EXidDelete(Window win)
{
   EXID               *xid;

#if 0
   Eprintf("EXidDelete: %p %#lx\n", xid, xid->win);
#endif
   xid = RemoveItem("", win, LIST_FINDBY_ID, LIST_TYPE_XID);
   if (!xid)
      return;

   XDeleteContext(disp, win, xid_context);
   if (xid->in_use)
      xid->do_del = 1;
   else
      EXidDestroy(xid);
}

static EXID        *
EXidFind(Window win)
{
   EXID               *xid;
   XPointer            xp;

   if (xid_context == 0)
      xid_context = XUniqueContext();

   xp = NULL;
   if (XFindContext(disp, win, xid_context, &xp) == XCNOENT)
      xp = NULL;
   xid = (EXID *) xp;

   return xid;
}

static EXID        *
EXidSet(Window win, Window parent, int x, int y, int w, int h, int depth)
{
   EXID               *xid;

   xid = EXidCreate();
   xid->parent = parent;
   xid->win = win;
   xid->x = x;
   xid->y = y;
   xid->w = w;
   xid->h = h;
   xid->depth = depth;
#if 0
   Eprintf("EXidSet: %#lx\n", xid->win);
#endif
   EXidAdd(xid);

   return xid;
}

void
EventCallbackRegister(Window win, int type __UNUSED__, EventCallbackFunc * func,
		      void *prm)
{
   EXID               *xid;
   EventCallbackItem  *eci;

   xid = EXidFind(win);
   if (!xid)
      ERegisterWindow(win);
   xid = EXidFind(win);
#if 0
   Eprintf("EventCallbackRegister: %p %#lx\n", xid, win);
#endif
   if (!xid)
     {
	Eprintf("EventCallbackRegister win=%#lx ???\n", win);
	return;
     }

   xid->cbl.num++;
   xid->cbl.lst =
      Erealloc(xid->cbl.lst, xid->cbl.num * sizeof(EventCallbackItem));
   eci = xid->cbl.lst + xid->cbl.num - 1;
   eci->func = func;
   eci->prm = prm;
}

/* Not used/tested */
void
EventCallbackUnregister(Window win, int type __UNUSED__,
			EventCallbackFunc * func, void *prm)
{
   EXID               *xid;
   EventCallbackList  *ecl;
   EventCallbackItem  *eci;
   int                 i;

   xid = EXidFind(win);
#if 0
   Eprintf("EventCallbackUnregister: %p %#lx\n", xid, win);
#endif
   if (xid == NULL)
      return;

   ecl = &xid->cbl;
   eci = ecl->lst;
   for (i = 0; i < ecl->num; i++, eci++)
      if (eci->func == func && eci->prm == prm)
	{
	   ecl->num--;
	   if (ecl->num)
	     {
		for (; i < ecl->num; i++, eci++)
		   *eci = *(eci + 1);
		xid->cbl.lst =
		   Erealloc(xid->cbl.lst, ecl->num * sizeof(EventCallbackItem));
	     }
	   else
	     {
		Efree(xid->cbl.lst);
		xid->cbl.lst = NULL;
	     }
	   return;
	}
}

void
EventCallbacksProcess(XEvent * ev)
{
   EXID               *xid;
   EventCallbackList  *ecl;
   EventCallbackItem  *eci;
   int                 i;

   xid = EXidFind(ev->xany.window);
   if (xid == NULL)
      return;

   xid->in_use = 1;
   ecl = &xid->cbl;
   eci = ecl->lst;
   for (i = 0; i < ecl->num; i++, eci++)
     {
	if (EventDebug(EDBUG_TYPE_DISPATCH))
	   Eprintf("EventDispatch: type=%d win=%#lx func=%p prm=%p\n",
		   ev->type, ev->xany.window, eci->func, eci->prm);
	eci->func(ev, eci->prm);
	if (xid->do_del)
	  {
	     EXidDestroy(xid);
	     return;
	  }
     }
   xid->in_use = 0;
}

Window
ECreateWindow(Window parent, int x, int y, int w, int h, int saveunder)
{
   Window              win;
   XSetWindowAttributes attr;

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.colormap = VRoot.cmap;
   attr.border_pixel = 0;
/*   attr.background_pixel = 0; */
   attr.background_pixmap = None;
   if ((saveunder == 1) && (Conf.save_under))
      attr.save_under = True;
   else if (saveunder == 2)
      attr.save_under = True;
   else
      attr.save_under = False;
   win = XCreateWindow(disp, parent, x, y, w, h, 0,
		       VRoot.depth, InputOutput, VRoot.vis,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixmap | CWBorderPixel, &attr);
   EXidSet(win, parent, x, y, w, h, VRoot.depth);

   return win;
}

/* Creates a window, but takes the visual, depth and the colormap from c_attr. */
Window
ECreateVisualWindow(Window parent, int x, int y, int w, int h, int saveunder,
		    XWindowAttributes * c_attr)
{
   Window              win;
   XSetWindowAttributes attr;

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.border_pixel = 0;
   attr.colormap = c_attr->colormap;
/*   attr.background_pixel = 0; */
   attr.background_pixmap = None;
   if ((saveunder == 1) && (Conf.save_under))
      attr.save_under = True;
   else if (saveunder == 2)
      attr.save_under = True;
   else
      attr.save_under = False;
   win = XCreateWindow(disp, parent, x, y, w, h, 0,
		       c_attr->depth, InputOutput, c_attr->visual,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixmap | CWBorderPixel, &attr);
   EXidSet(win, parent, x, y, w, h, VRoot.depth);

   return win;
}

Window
ECreateEventWindow(Window parent, int x, int y, int w, int h)
{
   Window              win;
   XSetWindowAttributes attr;

   attr.override_redirect = False;
   win = XCreateWindow(disp, parent, x, y, w, h, 0, 0, InputOnly,
		       CopyFromParent, CWOverrideRedirect, &attr);
   EXidSet(win, parent, x, y, w, h, VRoot.depth);

   return win;
}

#if 0				/* Not used */
/*
 * create a window which will accept the keyboard focus when no other 
 * windows have it
 */
Window
ECreateFocusWindow(Window parent, int x, int y, int w, int h)
{
   Window              win;
   XSetWindowAttributes attr;

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.colormap = VRoot.cmap;
   attr.border_pixel = 0;
   attr.background_pixel = 0;
   attr.save_under = False;
   attr.event_mask = KeyPressMask | FocusChangeMask;

   win = XCreateWindow(disp, parent, x, y, w, h, 0, 0, InputOnly,
		       CopyFromParent,
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixel | CWBorderPixel | CWEventMask,
		       &attr);

   XSetWindowBackground(disp, win, 0);
   XMapWindow(disp, win);
   XSetInputFocus(disp, win, RevertToParent, CurrentTime);

   return win;
}
#endif

void
EMoveWindow(Window win, int x, int y)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
#if 0
	Eprintf("EMoveWindow: %p %#lx: %d,%d %dx%d -> %d,%d\n",
		xid, xid->win, xid->x, xid->y, xid->w, xid->h, x, y);
#endif
	if ((x != xid->x) || (y != xid->y))
	  {
	     xid->x = x;
	     xid->y = y;
	     XMoveWindow(disp, win, x, y);
	  }
     }
   else
      XMoveWindow(disp, win, x, y);
}

void
EResizeWindow(Window win, int w, int h)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if ((w != xid->w) || (h != xid->h))
	  {
	     xid->w = w;
	     xid->h = h;
	     XResizeWindow(disp, win, w, h);
	  }
     }
   else
      XResizeWindow(disp, win, w, h);
}

void
EMoveResizeWindow(Window win, int x, int y, int w, int h)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
#if 0
	Eprintf("EMoveResizeWindow: %p %#lx: %d,%d %dx%d -> %d,%d %dx%d\n",
		xid, xid->win, xid->x, xid->y, xid->w, xid->h, x, y, w, h);
#endif
	if ((w != xid->w) || (h != xid->h) || (x != xid->x) || (y != xid->y))
	  {
	     xid->x = x;
	     xid->y = y;
	     xid->w = w;
	     xid->h = h;
	     XMoveResizeWindow(disp, win, x, y, w, h);
	  }
     }
   else
      XMoveResizeWindow(disp, win, x, y, w, h);
}

void
EDestroyWindow(Window win)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	EXID              **lst;
	int                 i, num;

	if (xid->parent != None)
	   XDestroyWindow(disp, win);
	EXidDelete(win);

	lst = (EXID **) ListItemType(&num, LIST_TYPE_XID);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->parent != win)
		     continue;
		  lst[i]->parent = None;
		  EDestroyWindow(lst[i]->win);
	       }
	     Efree(lst);
	  }
     }
   else
      XDestroyWindow(disp, win);
}

void
EWindowSync(Window win)
{
   EXID               *xid;
   Window              rr;
   int                 x, y;
   unsigned int        w, h, bw, depth;

   xid = EXidFind(win);
   if (!xid)
      return;

   XGetGeometry(disp, win, &rr, &x, &y, &w, &h, &bw, &depth);
#if 0
   Eprintf("EWindowSync: %p %#lx: %d,%d %dx%d -> %d,%d %dx%d\n",
	   xid, xid->win, xid->x, xid->y, xid->w, xid->h, x, y, w, h);
#endif
   xid->x = x;
   xid->y = y;
   xid->w = w;
   xid->h = h;
   xid->depth = depth;
}

void
EWindowSetMapped(Window win, int mapped)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (!xid)
      return;

   xid->mapped = mapped;
}

Window
EWindowGetParent(Window win)
{
   EXID               *xid;
   Window              parent, rt;
   Window             *pch = NULL;
   unsigned int        nch = 0;

   parent = None;
   if (!XQueryTree(disp, win, &rt, &parent, &pch, &nch))
      parent = None;
   else if (pch)
      XFree(pch);

   xid = EXidFind(win);
   if (xid)
      xid->parent = parent;

   return parent;
}

void
ERegisterWindow(Window win)
{
   EXID               *xid;
   Window              rr;
   int                 x, y;
   unsigned int        w, h, bw, depth;

   xid = EXidFind(win);
   if (xid)
      return;

   XGetGeometry(disp, win, &rr, &x, &y, &w, &h, &bw, &depth);
#if 0
   Eprintf("ERegisterWindow %#lx %d+%d %dx%d\n", win, x, y, w, h);
#endif
   xid = EXidSet(win, None, x, y, w, h, depth);
   xid->attached = 1;
}

void
EUnregisterWindow(Window win)
{
   EXidDelete(win);
}

void
EMapWindow(Window win)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if (!xid->mapped)
	  {
	     xid->mapped = 1;
	     XMapWindow(disp, win);
	  }
     }
   else
      XMapWindow(disp, win);
}

void
EUnmapWindow(Window win)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if (xid->mapped)
	  {
	     xid->mapped = 0;
	     XUnmapWindow(disp, win);
	  }
     }
   else
      XUnmapWindow(disp, win);
}

void
EReparentWindow(Window win, Window parent, int x, int y)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
#if 0
	Eprintf
	   ("EReparentWindow: %p %#lx: %d %#lx->%#lx %d,%d %dx%d -> %d,%d\n",
	    xid, xid->win, xid->mapped, xid->parent, parent, xid->x, xid->y,
	    xid->w, xid->h, x, y);
#endif
	if (parent == xid->parent)
	  {
	     if ((x != xid->x) || (y != xid->y))
	       {
		  xid->x = x;
		  xid->y = y;
		  XMoveWindow(disp, win, x, y);
	       }
	  }
	else
	  {
	     xid->parent = parent;
	     xid->x = x;
	     xid->y = y;
	     XReparentWindow(disp, win, parent, x, y);
	  }
     }
   else
      XReparentWindow(disp, win, parent, x, y);
}

void
EMapRaised(Window win)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if (xid->mapped)
	   XRaiseWindow(disp, win);
	else
	  {
	     xid->mapped = 1;
	     XMapRaised(disp, win);
	  }
     }
   else
      XMapRaised(disp, win);
}

int
EGetGeometry(Window win, Window * root_return, int *x, int *y,
	     int *w, int *h, int *bw, int *depth)
{
   int                 ok;
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if (x)
	   *x = xid->x;
	if (y)
	   *y = xid->y;
	if (w)
	   *w = xid->w;
	if (h)
	   *h = xid->h;
	if (bw)
	   *bw = 0;
	if (depth)
	   *depth = xid->depth;
	if (root_return)
	   *root_return = VRoot.win;
	ok = 1;
     }
   else
     {
	Window              rr;
	int                 xx, yy;
	unsigned int        ww, hh, bb, dd;

	ok = XGetGeometry(disp, win, &rr, &xx, &yy, &ww, &hh, &bb, &dd);
	if (ok)
	  {
	     if (root_return)
		*root_return = rr;
	     if (x)
		*x = xx;
	     if (y)
		*y = yy;
	     if (w)
		*w = ww;
	     if (h)
		*h = hh;
	     if (bw)
		*bw = bb;
	     if (depth)
		*depth = dd;
	  }
     }
#if 0				/* Debug */
   if (!ok)
      Eprintf("EGetGeometry win=%#x, error %d\n", (unsigned)win, ok);
#endif
   return ok;
}

void
EConfigureWindow(Window win, unsigned int mask, XWindowChanges * wc)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	char                doit = 0;

	if ((mask & CWX) && (wc->x != xid->x))
	  {
	     xid->x = wc->x;
	     doit = 1;
	  }
	if ((mask & CWY) && (wc->y != xid->y))
	  {
	     xid->y = wc->y;
	     doit = 1;
	  }
	if ((mask & CWWidth) && (wc->width != xid->w))
	  {
	     xid->w = wc->width;
	     doit = 1;
	  }
	if ((mask & CWHeight) && (wc->height != xid->h))
	  {
	     xid->h = wc->height;
	     doit = 1;
	  }
	if ((doit) || (mask & (CWBorderWidth | CWSibling | CWStackMode)))
	   XConfigureWindow(disp, win, mask, wc);
     }
   else
     {
	XConfigureWindow(disp, win, mask, wc);
     }
}

void
ESetWindowBackgroundPixmap(Window win, Pixmap pmap)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	xid->bgpmap = pmap;
	XSetWindowBackgroundPixmap(disp, win, pmap);
     }
   else
      XSetWindowBackgroundPixmap(disp, win, pmap);
}

void
ESetWindowBackground(Window win, int col)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	if (xid->bgpmap)
	  {
	     xid->bgpmap = 0;
	     xid->bgcol = col;
	     XSetWindowBackground(disp, win, col);
	  }
	else if (xid->bgcol != col)
	  {
	     xid->bgcol = col;
	     XSetWindowBackground(disp, win, col);
	  }
     }
   else
      XSetWindowBackground(disp, win, col);
}

void
ESelectInputAdd(Window win, long mask)
{
   XWindowAttributes   xwa;

   XGetWindowAttributes(disp, win, &xwa);
   xwa.your_event_mask |= mask;
   XSelectInput(disp, win, xwa.your_event_mask);
}

#define DEBUG_SHAPE_OPS 0
#define DEBUG_SHAPE_PROPAGATE 0

#if DEBUG_SHAPE_OPS
static void
EShapeShow(const char *txt, XRectangle * pr, int nr)
{
   int                 i;

   Eprintf("%s nr=%d\n", txt, nr);
   for (i = 0; i < nr; i++)
      Eprintf(" %d - %4d,%4d %4dx%4d\n", i,
	      pr[i].x, pr[i].y, pr[i].width, pr[i].height);
}
#endif

static void
EXidShapeUpdate(EXID * xid)
{
   if (xid->rects)
     {
	XFree(xid->rects);
	xid->num_rect = 0;
     }

   xid->rects =
      XShapeGetRectangles(disp, xid->win, ShapeBounding, &(xid->num_rect),
			  &(xid->ord));
   if (xid->rects)
     {
	if (xid->num_rect == 1)
	  {
	     if ((xid->rects[0].x == 0) && (xid->rects[0].y == 0)
		 && (xid->rects[0].width == xid->w)
		 && (xid->rects[0].height == xid->h))
	       {
		  xid->num_rect = 0;
		  XFree(xid->rects);
		  xid->rects = NULL;
	       }
	  }
     }
   else
     {
	xid->num_rect = -1;
     }
#if DEBUG_SHAPE_OPS
   EShapeShow("EXidShapeUpdate", xid->rects, xid->num_rect);
#endif
}

void
EShapeCombineMask(Window win, int dest, int x, int y, Pixmap pmap, int op)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	char                wasshaped = 0;

	if (xid->rects)
	  {
	     xid->num_rect = 0;
	     XFree(xid->rects);
	     xid->rects = NULL;
	     wasshaped = 1;
	  }
#if DEBUG_SHAPE_OPS
	Eprintf("EShapeCombineMask %#lx wassh=%d\n", win, wasshaped);
#endif
	if (pmap)
	  {
	     XShapeCombineMask(disp, win, dest, x, y, pmap, op);
	     EXidShapeUpdate(xid);
	  }
	else if (wasshaped)
	   XShapeCombineMask(disp, win, dest, x, y, pmap, op);
     }
   else
      XShapeCombineMask(disp, win, dest, x, y, pmap, op);
}

void
EShapeCombineMaskTiled(Window win, int dest, int x, int y,
		       Pixmap pmap, int op, int w, int h)
{
   XGCValues           gcv;
   GC                  gc;
   Window              tm;

   gcv.fill_style = FillTiled;
   gcv.tile = pmap;
   gcv.ts_x_origin = 0;
   gcv.ts_y_origin = 0;
   tm = ECreatePixmap(win, w, h, 1);
   gc = ECreateGC(tm, GCFillStyle | GCTile |
		  GCTileStipXOrigin | GCTileStipYOrigin, &gcv);
   XFillRectangle(disp, tm, gc, 0, 0, w, h);
   EFreeGC(gc);
   EShapeCombineMask(win, dest, x, y, tm, op);
   EFreePixmap(tm);
}

void
EShapeCombineRectangles(Window win, int dest, int x, int y,
			XRectangle * rect, int n_rects, int op, int ordering)
{
   EXID               *xid;

#if DEBUG_SHAPE_OPS
   Eprintf("EShapeCombineRectangles %#lx %d\n", win, n_rects);
#endif

   xid = EXidFind(win);
   if (xid)
     {
	if (n_rects == 1 && op == ShapeSet)
	  {
	     if ((rect[0].x == 0) && (rect[0].y == 0) &&
		 (rect[0].width == xid->w) && (rect[0].height == xid->h))
	       {
		  xid->num_rect = 0;
		  XFree(xid->rects);
		  xid->rects = NULL;
		  XShapeCombineMask(disp, win, dest, x, y, None, op);
		  return;
	       }
	  }
	XShapeCombineRectangles(disp, win, dest, x, y, rect, n_rects, op,
				ordering);
	if (n_rects > 1)
	  {
	     /* Limit shape to window extents */
	     XRectangle          r;

	     r.x = r.y = 0;
	     r.width = xid->w;
	     r.height = xid->h;
	     XShapeCombineRectangles(disp, win, ShapeBounding, 0, 0, &r,
				     1, ShapeIntersect, Unsorted);
	  }
	EXidShapeUpdate(xid);
     }
   else
      XShapeCombineRectangles(disp, win, dest, x, y, rect, n_rects, op,
			      ordering);
}

void
EShapeCombineShape(Window win, int dest, int x, int y,
		   Window src_win, int src_kind, int op)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid)
     {
	XShapeCombineShape(disp, win, dest, x, y, src_win, src_kind, op);
	EXidShapeUpdate(xid);
     }
   else
      XShapeCombineShape(disp, win, dest, x, y, src_win, src_kind, op);
}

XRectangle         *
EShapeGetRectangles(Window win, int dest, int *rn, int *ord)
{
   EXID               *xid;

   xid = EXidFind(win);
   if (xid && !xid->attached)
     {
	XRectangle         *r;

#if DEBUG_SHAPE_OPS
	Eprintf("EShapeGetRectangles-A %#lx nr=%d\n", win, xid->num_rect);
#endif
	*rn = xid->num_rect;
	*ord = xid->ord;
	if (xid->num_rect > 0)
	  {
	     r = Emalloc(sizeof(XRectangle) * xid->num_rect);
	     memcpy(r, xid->rects, sizeof(XRectangle) * xid->num_rect);
	     return r;
	  }
     }
   else
     {
	XRectangle         *r, *rr;

#if DEBUG_SHAPE_OPS
	Eprintf("EShapeGetRectangles-B %#lx nr=%d\n", win, xid->num_rect);
#endif
	r = XShapeGetRectangles(disp, win, dest, rn, ord);
	if (r)
	  {
	     rr = Emalloc(sizeof(XRectangle) * *rn);
	     memcpy(rr, r, sizeof(XRectangle) * *rn);
	     XFree(r);
	     return rr;
	  }
     }
   return NULL;
}

int
EShapeCopy(Window dst, Window src)
{
   XRectangle         *rl;
   int                 rn = 0, ord;
   int                 x, y, w, h, d;
   Window              rt;

   EGrabServer();
   EGetGeometry(src, &rt, &x, &y, &w, &h, &d, &d);
   rl = EShapeGetRectangles(src, ShapeBounding, &rn, &ord);
   EUngrabServer();

   if (rn < 0)
     {
	/* Source has empty shape */
	EShapeCombineShape(dst, ShapeBounding, 0, 0,
			   src, ShapeBounding, ShapeSet);
     }
   else if (rn == 0)
     {
	/* Source has default shape (no shape) */
	EShapeCombineMask(dst, ShapeBounding, 0, 0, None, ShapeSet);
     }
   else if (rn == 1)
     {
	if ((rl[0].x <= 0) && (rl[0].y <= 0) && (rl[0].width >= w)
	    && (rl[0].height >= h))
	  {
	     rn = 0;
	     EShapeCombineMask(dst, ShapeBounding, 0, 0, None, ShapeSet);
	  }
	else
	  {
	     EShapeCombineShape(dst, ShapeBounding, 0, 0,
				src, ShapeBounding, ShapeSet);
	  }
     }
   else
     {
	EShapeCombineShape(dst, ShapeBounding, 0, 0,
			   src, ShapeBounding, ShapeSet);
     }
   if (rl)
      XFree(rl);

   return rn != 0;
}

void
EShapePropagate(Window win)
{
   Window              rt, par, *list = NULL;
   unsigned int        i, num, num_rects;
   int                 k, rn, ord;
   int                 x, y, w, h, xx, yy, ww, hh, d;
   XRectangle         *rects, *rl;
   XWindowAttributes   att;

   if (!EGetGeometry(win, &rt, &xx, &yy, &ww, &hh, &d, &d))
      return;
   if ((ww <= 0) || (hh <= 0))
      return;

#if DEBUG_SHAPE_PROPAGATE
   Eprintf("EShapePropagate %#lx %d,%d %dx%d\n", win, xx, yy, ww, hh);
#endif

   XQueryTree(disp, win, &rt, &par, &list, &num);
   if (!list)
      return;

   num_rects = 0;
   rects = NULL;

   /* go through all child windows and create/inset spans */
   for (i = 0; i < num; i++)
     {
	XGetWindowAttributes(disp, list[i], &att);
#if DEBUG_SHAPE_PROPAGATE > 1
	Eprintf("%3d %#lx(%d): %4d,%4d %4dx%4d\n", i, list[i], att.map_state,
		att.x, att.y, att.width, att.height);
#endif
	if ((att.class != InputOutput) || (att.map_state == IsUnmapped))
	   continue;

	x = att.x;
	y = att.y;
	w = att.width;
	h = att.height;
	if (x >= ww || y >= hh || x + w < 0 || y + h < 0)
	   continue;

	rl = EShapeGetRectangles(list[i], ShapeBounding, &rn, &ord);

	if (rn > 0)
	  {
	     rects = Erealloc(rects, (num_rects + rn) * sizeof(XRectangle));
	     /* go through all clip rects in thsi window's shape */
	     for (k = 0; k < rn; k++)
	       {
		  /* for each clip rect, add it to the rect list */
		  rects[num_rects + k].x = x + rl[k].x;
		  rects[num_rects + k].y = y + rl[k].y;
		  rects[num_rects + k].width = rl[k].width;
		  rects[num_rects + k].height = rl[k].height;
#if DEBUG_SHAPE_PROPAGATE > 1
		  Eprintf(" - %d: %4d,%4d %4dx%4d\n", k,
			  rects[num_rects + k].x,
			  rects[num_rects + k].y, rects[num_rects + k].width,
			  rects[num_rects + k].height);
#endif
	       }
	     num_rects += rn;
	     Efree(rl);
	  }
	else if (rn == 0)
	  {
	     /* Unshaped */
	     rects = Erealloc(rects, (num_rects + 1) * sizeof(XRectangle));

	     rects[num_rects].x = x;
	     rects[num_rects].y = y;
	     rects[num_rects].width = w;
	     rects[num_rects].height = h;
	     num_rects++;
	  }
     }

#if DEBUG_SHAPE_PROPAGATE > 1
   Eprintf("EShapePropagate %#lx nr=%d\n", win, num_rects);
   for (i = 0; i < num_rects; i++)
      Eprintf("%3d %4d,%4d %4dx%4d\n", i, rects[i].x, rects[i].y,
	      rects[i].width, rects[i].height);
#endif

   /* set the rects as the shape mask */
   if (rects)
     {
	EShapeCombineRectangles(win, ShapeBounding, 0, 0, rects,
				num_rects, ShapeSet, Unsorted);
	Efree(rects);
     }
   else
     {
	/* Empty shape */
	EShapeCombineRectangles(win, ShapeBounding, 0, 0, NULL, 0, ShapeSet,
				Unsorted);
     }
   XFree(list);
}

GC
ECreateGC(Drawable d, unsigned long mask, XGCValues * val)
{
   XGCValues           xgcv;

   if (val)
     {
	mask |= GCGraphicsExposures;
	val->graphics_exposures = False;
     }
   else
     {
	mask = GCGraphicsExposures;
	val = &xgcv;
	val->graphics_exposures = False;
     }
   return XCreateGC(disp, d, mask, val);
}

int
EFreeGC(GC gc)
{
   return XFreeGC(disp, gc);
}

void
ESetColor(XColor * pxc, int r, int g, int b)
{
   pxc->red = (r << 8) | r;
   pxc->green = (g << 8) | g;
   pxc->blue = (b << 8) | b;
}

void
EGetColor(const XColor * pxc, int *pr, int *pg, int *pb)
{
   *pr = pxc->red >> 8;
   *pg = pxc->green >> 8;
   *pb = pxc->blue >> 8;
}

/* Build mask from window shape rects */
/* Snatched from imlib_create_scaled_image_from_drawable() */
Pixmap
EWindowGetShapePixmap(Window win)
{
   Pixmap              mask;
   GC                  gc;
   XRectangle         *rect;
   int                 i, w, h;
   int                 rect_num, rect_ord;

   EGetGeometry(win, NULL, NULL, NULL, &w, &h, NULL, NULL);
   mask = ECreatePixmap(win, w, h, 1);

   gc = XCreateGC(disp, mask, 0, NULL);
   XSetForeground(disp, gc, 0);

   rect = XShapeGetRectangles(disp, win, ShapeBounding, &rect_num, &rect_ord);
   XFillRectangle(disp, mask, gc, 0, 0, w, h);
   if (rect)
     {
	XSetForeground(disp, gc, 1);
	for (i = 0; i < rect_num; i++)
	   XFillRectangle(disp, mask, gc, rect[i].x, rect[i].y,
			  rect[i].width, rect[i].height);
	XFree(rect);
     }

   XFreeGC(disp, gc);

   return mask;
}

void
EGrabServer(void)
{
   if (Mode.server_grabbed <= 0)
      XGrabServer(disp);
   Mode.server_grabbed++;
}

void
EUngrabServer(void)
{
   if (Mode.server_grabbed == 1)
     {
	XUngrabServer(disp);
	XFlush(disp);
     }
   Mode.server_grabbed--;
   if (Mode.server_grabbed < 0)
      Mode.server_grabbed = 0;
}

int
EServerIsGrabbed(void)
{
   return Mode.server_grabbed;
}

void
EFlush(void)
{
   XFlush(disp);
}

void
ESync(void)
{
   XSync(disp, False);
}

Window
WindowAtXY_0(Window base, int bx, int by, int x, int y)
{
   Window             *list = NULL;
   XWindowAttributes   att;
   Window              child = 0, parent_win = 0, root_win = 0;
   int                 i;
   unsigned int        ww, wh, num;
   int                 wx, wy;

   if (!XGetWindowAttributes(disp, base, &att))
      return 0;
   if (att.class == InputOnly)
      return 0;
   if (att.map_state != IsViewable)
      return 0;
   wx = att.x;
   wy = att.y;
   ww = att.width;
   wh = att.height;

   wx += bx;
   wy += by;

   if (!((x >= wx) && (y >= wy) && (x < (int)(wx + ww))
	 && (y < (int)(wy + wh))))
      return 0;

   if (!XQueryTree(disp, base, &root_win, &parent_win, &list, &num))
      return base;
   if (list)
     {
	for (i = num - 1;; i--)
	  {
	     if ((child = WindowAtXY_0(list[i], wx, wy, x, y)) != 0)
	       {
		  XFree(list);
		  return child;
	       }
	     if (!i)
		break;
	  }
	XFree(list);
     }
   return base;
}

Window
WindowAtXY(int x, int y)
{
   Window             *list = NULL;
   Window              child = 0, parent_win = 0, root_win = 0;
   unsigned int        num;
   int                 i;

   EGrabServer();
   if (!XQueryTree(disp, VRoot.win, &root_win, &parent_win, &list, &num))
     {
	EUngrabServer();
	return VRoot.win;
     }
   if (list)
     {
	i = num - 1;
	do
	  {
	     XWindowAttributes   xwa;

	     XGetWindowAttributes(disp, list[i], &xwa);
	     if (xwa.map_state != IsViewable)
		continue;

	     if ((child = WindowAtXY_0(list[i], 0, 0, x, y)) == 0)
		continue;

	     XFree(list);
	     EUngrabServer();
	     return child;
	  }
	while (--i > 0);
	XFree(list);
     }
   EUngrabServer();
   return VRoot.win;
}

Bool
PointerAt(int *x, int *y)
{
   Window              dw;
   int                 dd;
   unsigned int        mm;

   if (!x || !y)
      x = y = &dd;

   /* Return True if pointer is on "our" screen */
   return XQueryPointer(disp, VRoot.win, &dw, &dw, &dd, &dd, x, y, &mm);
}

Display            *
EDisplayOpen(const char *dstr, int scr)
{
   char                dbuf[256], *s;
   Display            *dpy;

   if (scr >= 0)
     {
	/* Override screen */
	Esnprintf(dbuf, sizeof(dbuf) - 10, dstr);
	s = strchr(dbuf, ':');
	if (s)
	  {
	     s = strchr(s, '.');
	     if (s)
		*s = '\0';
	  }
	Esnprintf(dbuf + strlen(dbuf), 10, ".%d", scr);
	dstr = dbuf;
     }

#ifdef USE_ECORE_X
   ecore_x_init(dstr);
   dpy = ecore_x_display_get();
#else
   dpy = XOpenDisplay(dstr);
#endif

   return dpy;
}

void
EDisplayClose(void)
{
   if (!disp)
      return;
#ifdef USE_ECORE_X
   ecore_x_shutdown();
#else
   XCloseDisplay(disp);
#endif
   XSetErrorHandler((XErrorHandler) NULL);
   XSetIOErrorHandler((XIOErrorHandler) NULL);
   disp = NULL;
}

void
EDisplayDisconnect(void)
{
   if (!disp)
      return;
#ifdef USE_ECORE_X
   ecore_x_disconnect();
#else
   close(ConnectionNumber(disp));
#endif
   XSetErrorHandler((XErrorHandler) NULL);
   XSetIOErrorHandler((XIOErrorHandler) NULL);

   disp = NULL;
}

Time
EGetTimestamp(void)
{
   static Window       win_ts = None;
   XSetWindowAttributes attr;
   XEvent              ev;

   if (win_ts == None)
     {
	attr.override_redirect = 1;
	win_ts = XCreateWindow(disp, VRoot.win, -100, -100, 1, 1, 0,
			       CopyFromParent, InputOnly, CopyFromParent,
			       CWOverrideRedirect, &attr);
	XSelectInput(disp, win_ts, PropertyChangeMask);
     }

   XChangeProperty(disp, win_ts, XA_WM_NAME, XA_STRING, 8,
		   PropModeAppend, (unsigned char *)"", 0);
   XWindowEvent(disp, win_ts, PropertyChangeMask, &ev);

   return ev.xproperty.time;
}

void
EDrawableDumpImage(Drawable draw, const char *txt)
{
   static int          seqn = 0;
   char                buf[1024];
   Imlib_Image        *im;
   int                 w, h;

   w = h = 0;
   EGetGeometry(draw, NULL, NULL, NULL, &w, &h, NULL, NULL);
   if (w <= 0 || h <= 0)
      return;
   imlib_context_set_drawable(draw);
   im = imlib_create_image_from_drawable(None, 0, 0, w, h, !EServerIsGrabbed());
   imlib_context_set_image(im);
   imlib_image_set_format("png");
   sprintf(buf, "%s-%#lx-%d", txt, draw, seqn++);
   Eprintf("EDrawableDumpImage: %s\n", buf);
   imlib_save_image(buf);
   imlib_free_image_and_decache();
}
