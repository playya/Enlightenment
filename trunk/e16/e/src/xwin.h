/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2007 Kim Woelders
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
#ifndef _XWIN_H_
#define _XWIN_H_

Display            *EDisplayOpen(const char *dstr, int scr);
void                EDisplayClose(void);
void                EDisplayDisconnect(void);

void                EGrabServer(void);
void                EUngrabServer(void);
int                 EServerIsGrabbed(void);
void                EFlush(void);
void                ESync(void);

#if USE_COMPOSITE
int                 EVisualIsARGB(Visual * vis);
Visual             *EVisualFindARGB(void);
#endif

Time                EGetTimestamp(void);

typedef struct _xwin *Win;
typedef void        (EventCallbackFunc) (Win win, XEvent * ev, void *prm);

#define NoWin ((Win)0)

#define EXPOSE_WIN 1
#if EXPOSE_WIN || DECLARE_WIN
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

struct _xwin
{
   struct _xwin       *next;
   struct _xwin       *prev;
   EventCallbackList   cbl;
   Window              xwin;
   Win                 parent;
   int                 x, y, w, h;
   short               depth;
   unsigned short      bw;
   char                argb;
   char                mapped;
   char                in_use;
   signed char         do_del;
   char                attached;
   int                 num_rect;
   int                 ord;
   XRectangle         *rects;
   Visual             *visual;
   Colormap            cmap;
   Pixmap              bgpmap;
   unsigned int        bgcol;
};
#endif

Win                 ELookupXwin(Window xwin);

#if EXPOSE_WIN
#define             WinGetXwin(win)		((win)->xwin)
#define             WinGetX(win)		((win)->x)
#define             WinGetY(win)		((win)->y)
#define             WinGetW(win)		((win)->w)
#define             WinGetH(win)		((win)->h)
#define             WinGetBorderWidth(win)	((win)->bw)
#define             WinGetDepth(win)		((win)->depth)
#define             WinGetVisual(win)		((win)->visual)
#define             WinGetCmap(win)		((win)->cmap)
#else
Window              WinGetXwin(const Win win);
int                 WinGetX(const Win win);
int                 WinGetY(const Win win);
int                 WinGetW(const Win win);
int                 WinGetH(const Win win);
int                 WinGetBorderWidth(const Win win);
int                 WinGetDepth(const Win win);
Visual             *WinGetVisual(const Win win);
Colormap            WinGetCmap(const Win win);
#endif

Win                 ECreateWinFromXwin(Window xwin);
void                EDestroyWin(Win win);

Win                 ERegisterWindow(Window xwin, XWindowAttributes * pxwa);
void                EUnregisterWindow(Win win);
void                EUnregisterXwin(Window xwin);
void                EventCallbackRegister(Win win, int type,
					  EventCallbackFunc * func, void *prm);
void                EventCallbackUnregister(Win win, int type,
					    EventCallbackFunc * func,
					    void *prm);
void                EventCallbacksProcess(Win win, XEvent * ev);

Win                 ECreateWindow(Win parent, int x, int y, int w, int h,
				  int saveunder);
Win                 ECreateArgbWindow(Win parent, int x, int y, int w, int h,
				      Win cwin);
Win                 ECreateWindowVD(Win parent, int x, int y, int w, int h,
				    Visual * vis, unsigned int depth);
Win                 ECreateClientWindow(Win parent, int x, int y, int w, int h);
Win                 ECreateObjectWindow(Win parent, int x, int y, int w,
					int h, int saveunder, int type,
					Win cwin);
Win                 ECreateEventWindow(Win parent, int x, int y, int w, int h);
Win                 ECreateFocusWindow(Win parent, int x, int y, int w, int h);
void                EWindowSync(Win win);
void                EWindowSetGeometry(Win win, int x, int y, int w, int h,
				       int bw);
void                EWindowSetMapped(Win win, int mapped);
void                ESelectInputAdd(Win win, long mask);

void                EMoveWindow(Win win, int x, int y);
void                EResizeWindow(Win win, int w, int h);
void                EMoveResizeWindow(Win win, int x, int y, int w, int h);
void                EDestroyWindow(Win win);
void                EMapWindow(Win win);
void                EMapRaised(Win win);
void                EUnmapWindow(Win win);
void                EReparentWindow(Win win, Win parent, int x, int y);
int                 EGetGeometry(Win win, Window * root_return,
				 int *x, int *y, int *w, int *h, int *bw,
				 int *depth);
void                EGetWindowAttributes(Win win, XWindowAttributes * pxwa);
void                EConfigureWindow(Win win, unsigned int mask,
				     XWindowChanges * wc);
void                ESetWindowBackgroundPixmap(Win win, Pixmap pmap);
void                ESetWindowBackground(Win win, unsigned int col);
int                 ETranslateCoordinates(Win src_w, Win dst_w,
					  int src_x, int src_y,
					  int *dest_x_return,
					  int *dest_y_return,
					  Window * child_return);
int                 EDrawableCheck(Drawable draw, int grab);

#define ESelectInput(win, event_mask) \
	XSelectInput(disp, WinGetXwin(win), event_mask)

#define EChangeWindowAttributes(win, mask, attr) \
	XChangeWindowAttributes(disp, WinGetXwin(win), mask, attr)
#define ESetWindowBorderWidth(win, bw) \
	XSetWindowBorderWidth(disp, WinGetXwin(win), bw)

#define ERaiseWindow(win) \
	XRaiseWindow(disp, WinGetXwin(win))
#define ELowerWindow(win) \
	XLowerWindow(disp, WinGetXwin(win))

#define EClearWindow(win) \
	XClearWindow(disp, WinGetXwin(win))
#define EClearArea(win, x, y, w, h, exp) \
	XClearArea(disp, WinGetXwin(win), x, y, w, h, exp)

Pixmap              ECreatePixmap(Win win, unsigned int width,
				  unsigned int height, unsigned int depth);
void                EFreePixmap(Pixmap pixmap);

void                EShapeCombineMask(Win win, int dest, int x, int y,
				      Pixmap pmap, int op);
void                EShapeCombineMaskTiled(Win win, int dest, int x, int y,
					   Pixmap pmap, int op, int w, int h);
void                EShapeCombineRectangles(Win win, int dest, int x, int y,
					    XRectangle * rect, int n_rects,
					    int op, int ordering);
void                EShapeCombineShape(Win win, int dest, int x, int y,
				       Win src_win, int src_kind, int op);
XRectangle         *EShapeGetRectangles(Win win, int dest, int *rn, int *ord);
int                 EShapeCopy(Win dst, Win src);
int                 EShapePropagate(Win win);
int                 EShapeCheck(Win win);
Pixmap              EWindowGetShapePixmap(Win win);

Bool                EQueryPointer(Win win, int *px, int *py,
				  Window * pchild, unsigned int *pmask);

void                EAllocColor(Colormap colormap, XColor * pxc);
void                ESetColor(XColor * pxc, int r, int g, int b);
void                EGetColor(const XColor * pxc, int *pr, int *pg, int *pb);

Window              EXWindowGetParent(Window xwin);
int                 EXGetGeometry(Window xwin, Window * root_return,
				  int *x, int *y, int *w, int *h, int *bw,
				  int *depth);
#define EXGetWindowAttributes(win, xwa) \
	XGetWindowAttributes(disp, WinGetXwin(win), xwa)

void                EXCopyArea(Drawable src, Drawable dst, int sx, int sy,
			       unsigned int w, unsigned int h, int dx, int dy);

void                EXWarpPointer(Window xwin, int x, int y);

#define EXCreatePixmap(win, w, h, d) \
	XCreatePixmap(disp, win, w, h, d)
#define EXFreePixmap(pmap) \
	XFreePixmap(disp, pmap)
Pixmap              EXCreatePixmapCopy(Pixmap src, unsigned int w,
				       unsigned int h, unsigned int depth);

GC                  EXCreateGC(Drawable draw, unsigned long mask,
			       XGCValues * val);
int                 EXFreeGC(GC gc);

typedef struct
{
   char                type;
   Pixmap              pmap;
   Pixmap              mask;
   int                 w, h;
} PmapMask;

void                FreePmapMask(PmapMask * pmm);

#endif /* _XWIN_H_ */
