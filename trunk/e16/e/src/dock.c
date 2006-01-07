/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
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
#include "E.h"
#include "ewins.h"
#include "iclass.h"

static void
DockappFindEmptySpotFor(EWin * eapp)
{
   EWin               *const *lst, *ewin;
   int                 num, i, j, x, y, w, h, done;
   int                 step_right, step_down;

   x = EoGetX(eapp);
   y = EoGetY(eapp);
   w = eapp->client.w;
   h = eapp->client.h;
   if (!eapp->state.placed)
     {
	x = Conf.dock.startx;
	if (x < 0)
	   x = 0;
	else if (x > VRoot.w - EoGetW(eapp))
	   x = VRoot.w - EoGetW(eapp);

	y = Conf.dock.starty;
	if (y < 0)
	   y = 0;
	else if (y > VRoot.h - EoGetH(eapp))
	   y = VRoot.h - EoGetH(eapp);
     }

   step_right = Conf.dock.startx < VRoot.w;
   step_down = Conf.dock.starty < VRoot.h;

   lst = EwinListGetAll(&num);
   for (j = 0; j < num; j++)
      for (i = 0; i < num; i++)
	{
	   ewin = lst[i];

	   /* Skip self and non-dockapps */
	   if (ewin == eapp || !ewin->state.docked)
	      continue;

	   if ((x + w) <= EoGetX(ewin) || x >= (EoGetX(ewin) + EoGetW(ewin)))
	      done = 1;
	   else if ((y + h) <= EoGetY(ewin)
		    || y > (EoGetY(ewin) + EoGetH(ewin)))
	      done = 1;
	   else
	      done = 0;

	   if (!done)
	     {
		switch (Conf.dock.dirmode)
		  {
		  case DOCK_RIGHT:
		     x = EoGetX(ewin) + EoGetW(ewin);
		     if (x + w >= VRoot.w)
		       {
			  x = Conf.dock.startx;
			  y += (step_down) ? h : -h;
		       }
		     break;
		  case DOCK_LEFT:
		     x = EoGetX(ewin) - w;
		     if (x < 0)
		       {
			  x = Conf.dock.startx - w;
			  y += (step_down) ? h : -h;
		       }
		     break;
		  case DOCK_DOWN:
		     y = EoGetY(ewin) + EoGetH(ewin);
		     if (y + h >= VRoot.h)
		       {
			  y = Conf.dock.starty;
			  x += (step_right) ? w : -w;
		       }
		     break;
		  case DOCK_UP:
		     y = EoGetY(ewin) - h;
		     if (y < 0)
		       {
			  y = VRoot.h - h;
			  x += (step_right) ? w : -w;
		       }
		     break;
		  }
	     }
	}

   if (x < 0 || y < 0 || x + w >= VRoot.w || y + h >= VRoot.h)
     {
	x = VRoot.w - w / 2;
	y = VRoot.h - h / 2;
     }

   EoMove(eapp, x, y);
}

void
DockIt(EWin * ewin)
{
   ImageClass         *ic;

   ic = FindItem("DEFAULT_DOCK_BUTTON", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);

   if (Conf.dock.sticky)
      EoSetSticky(ewin, 1);

   ewin->props.donthide = 1;
   ewin->props.focusclick = 1;

   DockappFindEmptySpotFor(ewin);
   ewin->state.placed = 1;

   if (ewin->icccm.icon_win)
     {
	XSetWindowBorderWidth(disp, ewin->icccm.icon_win, 0);
	XMoveWindow(disp, ewin->icccm.icon_win, 0, 0);
	XMapWindow(disp, ewin->icccm.icon_win);
     }

   ImageclassApply(ic, EoGetWin(ewin), ewin->client.w, ewin->client.h,
		   0, 0, STATE_NORMAL, 0, ST_BUTTON);
}
