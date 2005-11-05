/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
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
#include "desktops.h"
#include "eobj.h"
#include "ewins.h"
#include "hints.h"
#include "iclass.h"
#include "tclass.h"
#include "xwin.h"

static EObj        *coord_eo = NULL;

void
CoordsShow(EWin * ewin)
{
   TextClass          *tc;
   ImageClass         *ic;
   char                s[256];
   int                 md;
   int                 x, y;
   unsigned int        w, h;
   int                 cx, cy, cw, ch;
   EObj               *eo = coord_eo;
   Imlib_Border       *pad;

   if (!Conf.movres.mode_info)
      return;
   if (ewin == NULL || !ewin->state.show_coords)
      return;

   tc = TextclassFind("COORDS", 1);
   ic = ImageclassFind("COORDS", 1);
   if ((!ic) || (!tc))
      return;

   cx = cy = cw = ch = 0;

   x = ewin->shape_x;
   y = ewin->shape_y;
   ICCCM_GetIncrementalSize(ewin, ewin->shape_w, ewin->shape_h, &w, &h);

   Esnprintf(s, sizeof(s), "%i x %i (%i, %i)", w, h, x, y);
   TextSize(tc, 0, 0, 0, s, &cw, &ch, 17);
   pad = ImageclassGetPadding(ic);
   cw += pad->left + pad->right;
   ch += pad->top + pad->bottom;

   /* Width hysteresis (hack - assuming horizontal text) */
   cw += 8;
   if (eo && abs(eo->w - cw) < 8)
      cw = eo->w;

   if (Mode.mode == MODE_MOVE)
      md = Conf.movres.mode_move;
   else
      md = Conf.movres.mode_resize;

   if ((md == 0) || ((cw < ewin->shape_w - 2) && (ch < ewin->shape_h - 2)))
     {
	if (Conf.movres.mode_info == 1)
	  {
	     switch (md)
	       {
	       case 0:
	       case 1:
	       case 2:
		  cx =
		     x + (ewin->shape_w + ewin->border->border.left +
			  ewin->border->border.right - cw) / 2 +
		     EoGetX(EoGetDesk(ewin));
		  cy =
		     y + (ewin->shape_h + ewin->border->border.top +
			  ewin->border->border.bottom - ch) / 2 +
		     EoGetY(EoGetDesk(ewin));
		  break;
	       }
	  }
     }

   if (!eo)
     {
	eo = EobjWindowCreate(EOBJ_TYPE_MISC, 0, 0, 1, 1, 2, "Coord");
	if (!eo)
	   return;
	coord_eo = eo;

	/* Center text (override theme) */
	TextclassSetJustification(tc, 512);
     }

#define TEST_COORD_REPARENT_TO_FRAME 0
#if TEST_COORD_REPARENT_TO_FRAME
   cx -= x;
   cy -= y;
#endif
   EobjMoveResize(eo, cx, cy, cw, ch);

   if (!eo->shown)
     {
#if TEST_COORD_REPARENT_TO_FRAME
	EobjReparent(eo, EoObj(ewin), cx, cy);
#endif
	EobjMap(eo, 0);
     }

   ITApply(eo->win, ic, NULL, cw, ch, STATE_NORMAL, 1, 0, 0, ST_SOLID,
	   tc, NULL, s);

   EFlush();
}

void
CoordsHide(void)
{
   EObj               *eo = coord_eo;

   if (eo && eo->shown)
     {
	EobjUnmap(eo);
#if TEST_COORD_REPARENT_TO_FRAME
	EobjReparent(eo, EoObj(DeskGet(0)), 0, 0);
#endif
     }
}
