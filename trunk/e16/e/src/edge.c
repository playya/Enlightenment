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

static Window       w1 = 0, w2 = 0, w3 = 0, w4 = 0;

static void
EdgeTimeout(int val, void *data __UNUSED__)
{
   int                 ax, ay, aw, ah, dx, dy, dax, day;
   EWin               *ewin;

   if (MenusActive())
      return;
   if (!Conf.edge_flip_resistance)
      return;

   /* Quit if pointer has left screen */
   if (!PointerAt(NULL, NULL))
      return;

   /* Quit if in fullscreen window */
   ewin = GetEwinPointerInClient();
   if (ewin && ewin->st.fullscreen)
      return;

   DeskGetCurrentArea(&ax, &ay);
   GetAreaSize(&aw, &ah);
   dx = 0;
   dy = 0;
   dax = 0;
   day = 0;
   switch (val)
     {
     case 0:
	if (ax == 0 && !Conf.desks.areas_wraparound)
	   return;
	dx = VRoot.w - 2;
	dax = -1;
	break;
     case 1:
	if (ax == (aw - 1) && !Conf.desks.areas_wraparound)
	   return;
	dx = -(VRoot.w - 2);
	dax = 1;
	break;
     case 2:
	if (ay == 0 && !Conf.desks.areas_wraparound)
	   return;
	dy = VRoot.h - 2;
	day = -1;
	break;
     case 3:
	if (ay == (ah - 1) && !Conf.desks.areas_wraparound)
	   return;
	dy = -(VRoot.h - 2);
	day = 1;
	break;
     default:
	break;
     }
   if (aw == 1)
      dx = 0;
   if (ah == 1)
      dy = 0;
   Mode.px = Mode.x;
   Mode.py = Mode.y;
   Mode.x += dx;
   Mode.y += dy;
   XWarpPointer(disp, None, VRoot.win, 0, 0, 0, 0, Mode.x, Mode.y);
   Mode.flipp = 1;
   MoveCurrentAreaBy(dax, day);
   Mode.flipp = 0;
   Mode.px = Mode.x;
   Mode.py = Mode.y;
}

static void
EdgeEvent(int dir)
{
   static int          lastdir = -1;

#if 0
   Eprintf("EdgeEvent %d -> %d\n", lastdir, dir);
#endif
   if (lastdir == dir || !Conf.edge_flip_resistance)
      return;

   RemoveTimerEvent("EDGE_TIMEOUT");
   if (dir >= 0)
     {
	DoIn("EDGE_TIMEOUT",
	     ((double)Conf.edge_flip_resistance) / 100.0, EdgeTimeout,
	     dir, NULL);
     }
   lastdir = dir;
}

static void
EdgeHandleEvents(XEvent * ev, void *prm)
{
   static Time         last_time;
   int                 dir;
   unsigned long       dt;

   dir = (int)prm;

   switch (ev->type)
     {
     case EnterNotify:
	/* Avoid excessive flipping */
	dt = ev->xcrossing.time - last_time;
	if (dt < 500)
	   return;
	last_time = ev->xcrossing.time;
	EdgeEvent(dir);
	break;

     case LeaveNotify:
	EdgeEvent(-1);
	break;

#if 0
     case MotionNotify:
	if (Mode.mode != MODE_MOVE_PENDING && Mode.mode != MODE_MOVE)
	   break;

	EdgeEvent(dir);
	break;
#endif
     }
}

void
EdgeCheckMotion(int x, int y)
{
   int                 dir;

   if (x == 0)
      dir = 0;
   else if (x == VRoot.w - 1)
      dir = 1;
   else if (y == 0)
      dir = 2;
   else if (y == VRoot.h - 1)
      dir = 3;
   else
      dir = -1;
   EdgeEvent(dir);
}

void
EdgeWindowsShow(void)
{
   int                 ax, ay, cx, cy;

   if (Conf.edge_flip_resistance <= 0)
     {
	EdgeWindowsHide();
	return;
     }

   if (!w1)
     {
	w1 = ECreateEventWindow(VRoot.win, 0, 0, 1, VRoot.h);
	w2 = ECreateEventWindow(VRoot.win, VRoot.w - 1, 0, 1, VRoot.h);
	w3 = ECreateEventWindow(VRoot.win, 0, 0, VRoot.w, 1);
	w4 = ECreateEventWindow(VRoot.win, 0, VRoot.h - 1, VRoot.w, 1);
	ESelectInput(w1, EnterWindowMask | LeaveWindowMask);
	ESelectInput(w2, EnterWindowMask | LeaveWindowMask);
	ESelectInput(w3, EnterWindowMask | LeaveWindowMask);
	ESelectInput(w4, EnterWindowMask | LeaveWindowMask);
	EventCallbackRegister(w1, 0, EdgeHandleEvents, (void *)0);
	EventCallbackRegister(w2, 0, EdgeHandleEvents, (void *)1);
	EventCallbackRegister(w3, 0, EdgeHandleEvents, (void *)2);
	EventCallbackRegister(w4, 0, EdgeHandleEvents, (void *)3);
     }
   DeskGetCurrentArea(&cx, &cy);
   GetAreaSize(&ax, &ay);

   if (cx == 0 && !Conf.desks.areas_wraparound)
      EUnmapWindow(w1);
   else
      EMapRaised(w1);
   if (cx == (ax - 1) && !Conf.desks.areas_wraparound)
      EUnmapWindow(w2);
   else
      EMapRaised(w2);
   if (cy == 0 && !Conf.desks.areas_wraparound)
      EUnmapWindow(w3);
   else
      EMapRaised(w3);
   if (cy == (ay - 1) && !Conf.desks.areas_wraparound)
      EUnmapWindow(w4);
   else
      EMapRaised(w4);
}

void
EdgeWindowsHide(void)
{
   if (w1)
     {
	EUnmapWindow(w1);
	EUnmapWindow(w2);
	EUnmapWindow(w3);
	EUnmapWindow(w4);
     }
}
