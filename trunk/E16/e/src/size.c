/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2003-2008 Kim Woelders
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
#include "hints.h"
#include "screen.h"

#define DEBUG_SIZE 0
#if DEBUG_SIZE
#define Dprintf Eprintf
#else
#define Dprintf(fmt...)
#endif

#define MAX_ABSOLUTE     0	/* Fill screen */
#define MAX_AVAILABLE    1	/* Expand until don't cover */
#define MAX_CONSERVATIVE 2	/* Expand until something */
#define MAX_XINERAMA     3	/* Fill Xinerama screen */

void
MaxSizeHV(EWin * ewin, const char *resize_type, int hor, int ver)
{
   int                 x, y, w, h, x1, x2, y1, y2, type, bl, br, bt, bb;
   EWin               *const *lst, *pe;
   int                 i, num;
   int                 old_hor = ewin->state.maximized_horz != 0;
   int                 old_ver = ewin->state.maximized_vert != 0;

   if (!ewin)
      return;

   if (ewin->state.inhibit_max_hor && hor)
      return;
   if (ewin->state.inhibit_max_ver && ver)
      return;

   if (!old_hor && !old_ver)
     {
	ewin->save_max.x = EoGetX(ewin);
	ewin->save_max.y = EoGetY(ewin);
	ewin->save_max.w = ewin->client.w;
	ewin->save_max.h = ewin->client.h;
     }

   /* Figure out target state */
   if (hor && ver)
     {
	hor = ver = (old_hor && old_ver) ? 0 : 1;
     }
   else
     {
	hor = (hor) ? !old_hor : old_hor;
	ver = (ver) ? !old_ver : old_ver;
     }

   ewin->state.maximizing = 1;
   ewin->state.maximized_horz = hor;
   ewin->state.maximized_vert = ver;

   Dprintf("h/v old = %d/%d new=%d/%d\n", old_hor, old_ver, hor, ver);
   if (!hor && !ver)
     {
	/* Restore regular state */
	EwinMoveResize(ewin, ewin->save_max.x, ewin->save_max.y,
		       ewin->save_max.w, ewin->save_max.h);
	goto done;
     }
   if (old_ver == ver && old_hor && !hor)
     {
	/* Turn off horizontal maxsize */
	EwinMoveResize(ewin, ewin->save_max.x, EoGetY(ewin),
		       ewin->save_max.w, ewin->client.h);
	goto done;
     }
   if (old_hor == hor && old_ver && !ver)
     {
	/* Turn off vertical maxsize */
	EwinMoveResize(ewin, EoGetX(ewin), ewin->save_max.y,
		       ewin->client.w, ewin->save_max.h);
	goto done;
     }

   type = MAX_ABSOLUTE;		/* Select default */
   if (!resize_type || !resize_type[0])
      type = Conf.movres.mode_maximize_default;
   else if (!strcmp(resize_type, "absolute"))
      type = MAX_ABSOLUTE;
   else if (!strcmp(resize_type, "available"))
      type = MAX_AVAILABLE;
   else if (!strcmp(resize_type, "conservative"))
      type = MAX_CONSERVATIVE;
   else if (!strcmp(resize_type, "xinerama"))
      type = MAX_XINERAMA;

   /* Default is no change */
   x = EoGetX(ewin);
   y = EoGetY(ewin);
   h = EoGetH(ewin);
   w = EoGetW(ewin);

   switch (type)
     {
     case MAX_XINERAMA:
	if (hor)
	  {
	     x = 0;
	     w = WinGetW(VROOT);
	  }
	if (ver)
	  {
	     y = 0;
	     h = WinGetH(VROOT);
	  }
	break;

     default:
     case MAX_ABSOLUTE:
     case MAX_AVAILABLE:
     case MAX_CONSERVATIVE:
	ScreenGetAvailableArea(x + w / 2, y + h / 2, &x1, &y1, &x2, &y2,
			       Conf.place.ignore_struts_maximize);
	x2 += x1;
	y2 += y1;

	if (Conf.movres.dragbar_nocover && type != MAX_ABSOLUTE)
	  {
	     /* Leave room for the dragbar */
	     switch (Conf.desks.dragdir)
	       {
	       case 0:		/* left */
		  if (x1 < Conf.desks.dragbar_width)
		     x1 = Conf.desks.dragbar_width;
		  break;

	       case 1:		/* right */
		  if (x2 > WinGetW(VROOT) - Conf.desks.dragbar_width)
		     x2 = WinGetW(VROOT) - Conf.desks.dragbar_width;
		  break;

	       case 2:		/* top */
		  if (y1 < Conf.desks.dragbar_width)
		     y1 = Conf.desks.dragbar_width;
		  break;

	       case 3:		/* bottom */
		  if (y2 > WinGetH(VROOT) - Conf.desks.dragbar_width)
		     y2 = WinGetH(VROOT) - Conf.desks.dragbar_width;
		  break;

	       default:
		  break;
	       }
	  }

	if (type == MAX_ABSOLUTE)
	  {
	     /* Simply ignore all windows */
	     lst = NULL;
	     num = 0;
	  }
	else
	  {
	     lst = EwinListGetAll(&num);
	  }

	if (ver && hor && !old_ver && !old_hor)
	  {
	     for (i = 0; i < num;)
	       {
		  int                 x1n, x2n, y1n, y2n;
		  int                 need_chop_y, need_chop_x;
		  int                 top, bottom, left, right;

		  pe = lst[i];
		  x1n = x1;
		  x2n = x2;
		  y1n = y1;
		  y2n = y2;

		  left = EoGetX(pe);
		  right = left + EoGetW(pe);
		  top = EoGetY(pe);
		  bottom = top + EoGetH(pe);

		  need_chop_x = need_chop_y = 0;

		  Dprintf
		     ("trying window #%d %s x:%d-%d y:%d-%d vs x:%d-%d y:%d-%d\n",
		      i, EoGetName(pe), left, right, top, bottom,
		      x1, x2, y1, y2);

		  if (pe == ewin || pe->state.iconified || EoIsFloating(pe) ||
		      pe->props.ignorearrange ||
		      (EoGetDesk(ewin) != EoGetDesk(pe) && !EoIsSticky(pe)) ||
		      (pe->type & (EWIN_TYPE_DIALOG | EWIN_TYPE_MENU)) ||
		      (type == MAX_AVAILABLE && !pe->props.never_use_area) ||
		      /* ignore windws that do not overlap with current search area */
		      !(SPANS_COMMON(x1, x2 - x1, EoGetX(pe), EoGetW(pe)) &&
			SPANS_COMMON(y1, y2 - y1, EoGetY(pe), EoGetH(pe))) ||
		      /* ignore windows that already overlap with the orig window */
		      (SPANS_COMMON(x + 1, w - 2, EoGetX(pe), EoGetW(pe)) &&
		       SPANS_COMMON(y + 1, h - 2, EoGetY(pe), EoGetH(pe))))
		    {
		       i++;
		       continue;
		    }

		  if (right <= x + w / 2)
		    {
		       need_chop_x = 1;
		       x1n = right;
		    }
		  if (left >= x + w / 2)
		    {
		       need_chop_x = 1;
		       x2n = left;
		    }
		  if (bottom <= y + h / 2)
		    {
		       need_chop_y = 1;
		       y1n = bottom;
		    }
		  if (top >= y + h / 2)
		    {
		       need_chop_y = 1;
		       y2n = top;
		    }
		  Dprintf("chop v: %d chop_x:%d\n",
			  (y2n - y1n) * (x2 - x1), (y2 - y1) * (x2n - x1n));

		  if (!(need_chop_y || need_chop_x))
		    {
		       Dprintf("no chop\n");
		       i++;
		       continue;
		    }
		  if (!need_chop_x)
		    {
		       Dprintf("chop_v\n");
		       y2 = y2n;
		       y1 = y1n;
		    }
		  else if (!need_chop_y)
		    {
		       Dprintf("chop_h\n");
		       x2 = x2n;
		       x1 = x1n;
		    }
		  /* greedily chop the minimum area either from the sides or top/bottom
		   * We may need to do a final cleanup pass below to escape from a
		   * local local minima of the area decision function */
		  else if ((y2 - y1) * (x2n - x1n) > (y2n - y1n) * (x2 - x1))
		    {
		       Dprintf("___chop_h\n");
		       x2 = x2n;
		       x1 = x1n;
		    }
		  else
		    {
		       Dprintf("___chop_v\n");
		       y2 = y2n;
		       y1 = y1n;
		    }
	       }
	     x = x1;
	     w = x2 - x1;
	     y = y1;
	     h = y2 - y1;
	     break;
	  }

	if (ver)
	  {
	     for (i = 0; i < num; i++)
	       {
		  pe = lst[i];
		  if (pe == ewin ||
		      pe->state.iconified || EoIsFloating(pe) ||
		      pe->props.ignorearrange ||
		      (EoGetDesk(ewin) != EoGetDesk(pe) && !EoIsSticky(pe)) ||
		      (pe->type & (EWIN_TYPE_DIALOG | EWIN_TYPE_MENU)) ||
		      (type == MAX_AVAILABLE && !pe->props.never_use_area) ||
		      !SPANS_COMMON(x, w, EoGetX(pe), EoGetW(pe)))
		     continue;

		  if (((EoGetY(pe) + EoGetH(pe)) <= y)
		      && ((EoGetY(pe) + EoGetH(pe)) >= y1))
		     y1 = EoGetY(pe) + EoGetH(pe);
		  else if (((y + h) <= EoGetY(pe)) && (y2 >= EoGetY(pe)))
		     y2 = EoGetY(pe);
	       }
	     y = y1;
	     h = y2 - y1;
	  }

	if (hor)
	  {
	     for (i = 0; i < num; i++)
	       {
		  pe = lst[i];
		  if (pe == ewin ||
		      pe->state.iconified || EoIsFloating(pe) ||
		      pe->props.ignorearrange ||
		      (EoGetDesk(ewin) != EoGetDesk(pe) && !EoIsSticky(pe)) ||
		      (pe->type & (EWIN_TYPE_DIALOG | EWIN_TYPE_MENU)) ||
		      (type == MAX_AVAILABLE && !pe->props.never_use_area) ||
		      !SPANS_COMMON(y, h, EoGetY(pe), EoGetH(pe)))
		     continue;

		  if (((EoGetX(pe) + EoGetW(pe)) <= x)
		      && ((EoGetX(pe) + EoGetW(pe)) >= x1))
		     x1 = EoGetX(pe) + EoGetW(pe);
		  else if (((x + w) <= EoGetX(pe)) && (x2 >= EoGetX(pe)))
		     x2 = EoGetX(pe);
	       }
	     x = x1;
	     w = x2 - x1;
	  }

	break;
     }

   EwinBorderGetSize(ewin, &bl, &br, &bt, &bb);
   w -= (bl + br);
   if (w < 10)
      w = 10;
   h -= (bt + bb);
   if (h < 10)
      h = 10;

   EwinMoveResize(ewin, x, y, w, h);
 done:
   ewin->state.maximizing = 0;
   HintsSetWindowState(ewin);
}
