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

void
MakeWindowUnSticky(EWin * ewin)
{

   EDBUG(5, "MakeWindowUnSticky");
   if (!ewin)
      EDBUG_RETURN_;

   ewin->sticky = 2;		/* Grrr: we are "unsticking" (hack to get the desktop right) */
   ewin->desktop = desks.current;
   FloatEwinAt(ewin, ewin->x, ewin->y);
   DrawEwinShape(ewin, 0, ewin->x, ewin->y, ewin->client.w, ewin->client.h, 0);
   MoveEwinToDesktopAt(ewin, desks.current, ewin->x, ewin->y);
   ewin->sticky = 0;
   RaiseEwin(ewin);
   EwinBorderDraw(ewin, 0, 0);

   HintsSetWindowState(ewin);

   SoundPlay("SOUND_WINDOW_UNSTICK");

   EDBUG_RETURN_;
}

void
MakeWindowSticky(EWin * ewin)
{
   int                 x, y, dx, dy;

   EDBUG(5, "MakeWindowSticky");
   if (!ewin)
      EDBUG_RETURN_;

   ewin->sticky = 1;

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

   if (x != ewin->x || y != ewin->y)
     {
	ewin->x = x;
	ewin->y = y;
	FloatEwinAt(ewin, ewin->x, ewin->y);
	DrawEwinShape(ewin, 0, ewin->x, ewin->y, ewin->client.w, ewin->client.h,
		      0);
     }
   MoveEwinToDesktopAt(ewin, desks.current, ewin->x, ewin->y);
   RaiseEwin(ewin);
   EwinBorderDraw(ewin, 0, 0);

   HintsSetWindowState(ewin);

   SoundPlay("SOUND_WINDOW_STICK");

   EDBUG_RETURN_;
}
