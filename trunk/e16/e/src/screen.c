/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2003-2006 Kim Woelders
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
#include "ewins.h"
#include "screen.h"
#include "xwin.h"
#ifdef HAS_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

typedef struct
{
   int                 type;
   int                 head;
   int                 x, y;
   int                 w, h;
} EScreen;

static EScreen     *p_screens = NULL;
static int          n_screens = 0;

void
ScreenAdd(int type, int head, int x, int y, unsigned int w, unsigned int h)
{
   EScreen            *es;

   n_screens++;
   p_screens = Erealloc(p_screens, n_screens * sizeof(EScreen));

   es = p_screens + n_screens - 1;
   es->type = type;
   es->head = head;
   es->x = x;
   es->y = y;
   es->w = w;
   es->h = h;
}

void
ScreenInit(void)
{
#ifdef HAS_XINERAMA
   XineramaScreenInfo *screens = NULL;
   int                 num_screens = 0;
   int                 i;

   if (Mode.wm.window)
      return;

   Mode.display.xinerama_active = XineramaIsActive(disp);
   Conf.extra_head = 0;
   if (Mode.display.xinerama_active)
      screens = XineramaQueryScreens(disp, &num_screens);

   for (i = 0; i < num_screens; i++)
      ScreenAdd(0, screens[i].screen_number, screens[i].x_org,
		screens[i].y_org, screens[i].width, screens[i].height);
#endif
}

void
ScreenShowInfo(const char *prm __UNUSED__)
{
   int                 i;

#ifdef HAS_XINERAMA
   if (XineramaIsActive(disp))
     {
	XineramaScreenInfo *scrns;
	int                 num;

	scrns = XineramaQueryScreens(disp, &num);

	IpcPrintf("Xinerama screens:\n");
	IpcPrintf("Head  Screen  X-Origin  Y-Origin     Width    Height\n");
	for (i = 0; i < num; i++)
	   IpcPrintf(" %2d     %2d       %5d     %5d     %5d     %5d\n",
		     i, scrns[i].screen_number,
		     scrns[i].x_org, scrns[i].y_org, scrns[i].width,
		     scrns[i].height);
	XFree(scrns);
     }
   else
     {
	IpcPrintf("Xinerama is not active\n");
     }
#endif

   IpcPrintf("E-screens:\n");
   IpcPrintf("Head  Screen  X-Origin  Y-Origin     Width    Height\n");
   if (n_screens)
     {
	for (i = 0; i < n_screens; i++)
	  {
	     EScreen            *ps = p_screens + i;

	     IpcPrintf(" %2d     %2d       %5d     %5d     %5d     %5d\n",
		       i, ps->head, ps->x, ps->y, ps->w, ps->h);
	  }
     }
   else
     {
	IpcPrintf(" %2d     %2d       %5d     %5d     %5d     %5d\n",
		  0, VRoot.scr, 0, 0, VRoot.w, VRoot.h);
     }
}

int
ScreenGetGeometry(int xi, int yi, int *px, int *py, int *pw, int *ph)
{
   int                 i, dx, dy, x, y, w, h, dist, ix;
   EScreen            *ps;

   ix = -1;
   dist = 2147483647;

   if (n_screens > 1)
     {
	for (i = 0; i < n_screens; i++)
	  {
	     ps = p_screens + i;

	     dx = xi - (ps->x + ps->w / 2);
	     dy = yi - (ps->y + ps->h / 2);
	     dx = dx * dx + dy * dy;
	     if (dx >= dist)
		continue;
	     dist = dx;
	     ix = i;
	  }
     }

   if (ix >= 0)
     {
	ps = p_screens + ix;
	ix = ps->head;
	x = ps->x;
	y = ps->y;
	w = ps->w;
	h = ps->h;
     }
   else
     {
	ix = VRoot.scr;
	x = 0;
	y = 0;
	w = VRoot.w;
	h = VRoot.h;
     }

   if (px)
      *px = x;
   if (py)
      *py = y;
   if (pw)
      *pw = w;
   if (ph)
      *ph = h;

   return ix;
}

static void
VRootGetAvailableArea(int *px, int *py, int *pw, int *ph)
{
   EWin               *const *lst, *ewin;
   int                 i, num, l, r, t, b;

   l = r = t = b = 0;
   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];

	if (l < ewin->strut.left)
	   l = ewin->strut.left;
	if (r < ewin->strut.right)
	   r = ewin->strut.right;
	if (t < ewin->strut.top)
	   t = ewin->strut.top;
	if (b < ewin->strut.bottom)
	   b = ewin->strut.bottom;
     }

   *px = l;
   *py = t;
   *pw = VRoot.w - (l + r);
   *ph = VRoot.h - (t + b);
}

int
ScreenGetAvailableArea(int xi, int yi, int *px, int *py, int *pw, int *ph)
{
   int                 x1, y1, w1, h1, x2, y2, w2, h2, head;

   head = ScreenGetGeometry(xi, yi, &x1, &y1, &w1, &h1);

   if (!Conf.place.ignore_struts)
     {
	VRootGetAvailableArea(&x2, &y2, &w2, &h2);
	if (x1 < x2)
	   x1 = x2;
	if (y1 < y2)
	   y1 = y2;
	if (w1 > w2)
	   w1 = w2;
	if (h1 > h2)
	   h1 = h2;
     }

   *px = x1;
   *py = y1;
   *pw = w1;
   *ph = h1;

   return head;
}

int
GetPointerScreenGeometry(int *px, int *py, int *pw, int *ph)
{
   int                 pointer_x, pointer_y;

   EXQueryPointer(VRoot.win, &pointer_x, &pointer_y, NULL, NULL);

   return ScreenGetGeometry(pointer_x, pointer_y, px, py, pw, ph);
}

int
GetPointerScreenAvailableArea(int *px, int *py, int *pw, int *ph)
{
   int                 pointer_x, pointer_y;

   EXQueryPointer(VRoot.win, &pointer_x, &pointer_y, NULL, NULL);

   return ScreenGetAvailableArea(pointer_x, pointer_y, px, py, pw, ph);
}
