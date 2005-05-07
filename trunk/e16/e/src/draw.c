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
#include <sys/ipc.h>
#include <sys/shm.h>

void
HandleDrawQueue()
{
   DrawQueue         **lst = NULL, *dq;
   int                 i, num;
   char                already, p_queue;

   switch (Mode.mode)
     {
     case MODE_MOVE_PENDING:
     case MODE_MOVE:
	if (Conf.movres.mode_move > 0)
	   return;
	break;
     case MODE_RESIZE:
     case MODE_RESIZE_H:
     case MODE_RESIZE_V:
	if (Conf.movres.mode_resize > 0)
	   return;
	break;
     }

   p_queue = Mode.queue_up;
   Mode.queue_up = 0;
   num = 0;
   /* find all DRAW queue entries most recent first and add them to the */
   /* end of the draw list array if there are no previous entries for that */
   /* draw type and that window in the array */
   while ((dq =
	   (DrawQueue *) RemoveItem(NULL, 0, LIST_FINDBY_NONE, LIST_TYPE_DRAW)))
     {
	already = 0;

	if (dq->shape_propagate)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->win == dq->win) && (lst[i]->shape_propagate))
		    {
		       already = 1;
		       break;
		    }
	       }
	  }
#if USE_DQ_TCLASS
	else if (dq->text)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->win == dq->win) && (lst[i]->text))
		    {
		       already = 1;
		       break;
		    }
	       }
	  }
#endif
#if USE_DQ_ICLASS
	else if (dq->iclass)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->win == dq->win) && (!lst[i]->shape_propagate)
		      && (!lst[i]->text))
		    {
		       already = 1;
		       break;
		    }
	       }
	  }
#endif
	else if (dq->pager)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->win == dq->win) && (lst[i]->pager))
		    {
		       already = 1;
		       break;
		    }
	       }
	  }
	else if (dq->d)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->d == dq->d) && DialogItem(dq->d) &&
		      /*(dq->d->item == dq->di) && */ (lst[i]->di == dq->di))
		    {
		       if (dq->x < lst[i]->x)
			 {
			    lst[i]->w += (lst[i]->x - dq->x);
			    lst[i]->x = dq->x;
			 }
		       if ((lst[i]->x + lst[i]->w) < (dq->x + dq->w))
			  lst[i]->w +=
			     (dq->x + dq->w) - (lst[i]->x + lst[i]->w);
		       if (dq->y < lst[i]->y)
			 {
			    lst[i]->h += (lst[i]->y - dq->y);
			    lst[i]->y = dq->y;
			 }
		       if ((lst[i]->y + lst[i]->h) < (dq->y + dq->h))
			  lst[i]->h +=
			     (dq->y + dq->h) - (lst[i]->y + lst[i]->h);
		       already = 1;
		       break;
		    }
	       }
	  }
	else if (dq->redraw_pager)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if ((lst[i]->win == dq->win) && (lst[i]->redraw_pager))
		    {
		       switch (lst[i]->newbg)
			 {
			 case 0:
			    if (dq->newbg == 1)
			       lst[i]->newbg = 1;
			    else if (dq->newbg == 2)
			       lst[i]->newbg = 1;
			    break;
			 case 1:
			    break;
			 case 2:
			    if (dq->newbg == 1)
			       lst[i]->newbg = 1;
			    else if (dq->newbg == 0)
			       lst[i]->newbg = 1;
			    break;
			 case 3:
			    if (dq->newbg == 1)
			       lst[i]->newbg = 1;
			    else if (dq->newbg == 0)
			       lst[i]->newbg = 0;
			    else if (dq->newbg == 2)
			       lst[i]->newbg = 2;
			    break;
			 default:
			    break;
			 }
		       already = 1;
		       break;
		    }
	       }
	  }

	if (already)
	  {
	     if (dq)
	       {
#if USE_DQ_ICLASS
		  if (dq->iclass)
		     dq->iclass->ref_count--;
#endif
#if USE_DQ_TCLASS
		  if (dq->tclass)
		     dq->tclass->ref_count--;
		  if (dq->text)
		     Efree(dq->text);
#endif
		  Efree(dq);
	       }
	  }
	else
	  {
	     num++;
	     lst = Erealloc(lst, num * sizeof(DrawQueue *));
	     lst[num - 1] = dq;
	  }
     }
   /* go thru the list in chronological order (ie reverse) and do the draws */
   if (lst)
     {
	for (i = num - 1; i >= 0; i--)
	  {
	     dq = lst[i];
	     if (dq->shape_propagate)
	       {
/*            printf("S %x\n", dq->win); */
		  if (WinExists(dq->win))
		     PropagateShapes(dq->win);
	       }
#if USE_DQ_TCLASS
	     else if (dq->text)
	       {
/*            printf("T %x\n", dq->win); */
		  if (WinExists(dq->win))
		     TextclassApply(dq->iclass, dq->win, dq->w, dq->h,
				    dq->active, dq->sticky, dq->state,
				    dq->expose, dq->tclass, dq->text);
		  Efree(dq->text);
	       }
#endif
#if USE_DQ_ICLASS
	     else if (dq->iclass)
	       {
/*            printf("I %x\n", dq->win); */
		  if (WinExists(dq->win))
		     ImageclassApply(dq->iclass, dq->win, dq->w, dq->h,
				     dq->active, dq->sticky, dq->state, 0,
				     dq->image_type);
	       }
#endif
	     else if (dq->pager)
	       {
/*            printf("P %x\n", dq->win); */
		  if (FindItem
		      ((char *)(dq->pager), 0, LIST_FINDBY_POINTER,
		       LIST_TYPE_PAGER))
		     dq->func(dq);
	       }
	     else if (dq->d)
	       {
/*            printf("D %x\n", dq->d->ewin->client.win); */
		  if (FindItem
		      ((char *)(dq->d), 0, LIST_FINDBY_POINTER,
		       LIST_TYPE_DIALOG))
		     DialogDrawItems(dq->d, dq->di, dq->x, dq->y, dq->w, dq->h);
	       }
	     else if (dq->redraw_pager)
	       {
/*            printf("p %x\n", dq->win); */
		  if (FindItem
		      ((char *)(dq->redraw_pager), 0, LIST_FINDBY_POINTER,
		       LIST_TYPE_PAGER))
		     dq->func(dq);
	       }
#if USE_DQ_ICLASS
	     if (dq->iclass)
		dq->iclass->ref_count--;
#endif
#if USE_DQ_TCLASS
	     if (dq->tclass)
		dq->tclass->ref_count--;
#endif
	     Efree(dq);
	  }
	Efree(lst);
     }

   Mode.queue_up = p_queue;
}

char
IsPropagateEwinOnQueue(EWin * ewin)
{
   if (FindItem(NULL, EoGetWin(ewin), LIST_FINDBY_ID, LIST_TYPE_DRAW))
      return 1;
   return 0;
}

static void
EFillPixmap(Window win, Pixmap pmap, int x, int y, int w, int h)
{
   XGCValues           gcv;
   GC                  gc;

   gcv.subwindow_mode = IncludeInferiors;
   gc = ECreateGC(win, GCSubwindowMode, &gcv);
   XCopyArea(disp, win, pmap, gc, x, y, w, h, x, y);
   EFreeGC(gc);
}

static void
EPastePixmap(Window win, Pixmap pmap, int x, int y, int w, int h)
{
   XGCValues           gcv;
   GC                  gc;

   gcv.subwindow_mode = IncludeInferiors;
   gc = ECreateGC(win, GCSubwindowMode, &gcv);
   XCopyArea(disp, pmap, win, gc, x, y, w, h, x, y);
   EFreeGC(gc);
}

typedef struct _PixImg
{
   XImage             *xim;
   XShmSegmentInfo    *shminfo;
   Pixmap              pmap;
   GC                  gc;
}
PixImg;

static PixImg      *
ECreatePixImg(Window win, int w, int h)
{
   XGCValues           gcv;
   int                 bpp;
   PixImg             *pi;

   if (VRoot.depth <= 8)
      bpp = 1;
   else if (VRoot.depth <= 16)
      bpp = 2;
   else if (VRoot.depth <= 24)
      bpp = 3;
   else
      bpp = 4;

   pi = Emalloc(sizeof(PixImg));
   if (!pi)
      return NULL;

   pi->shminfo = Emalloc(sizeof(XShmSegmentInfo));
   if (pi->shminfo)
     {
	pi->xim = XShmCreateImage(disp, VRoot.vis, VRoot.depth, ZPixmap, NULL,
				  pi->shminfo, w, h);
	if (pi->xim)
	  {
	     pi->shminfo->shmid =
		shmget(IPC_PRIVATE, pi->xim->bytes_per_line * pi->xim->height,
		       IPC_CREAT | 0666);
	     if (pi->shminfo->shmid >= 0)
	       {
		  pi->shminfo->shmaddr = pi->xim->data =
		     shmat(pi->shminfo->shmid, 0, 0);
		  if (pi->shminfo->shmaddr)
		    {
		       pi->shminfo->readOnly = False;
		       XShmAttach(disp, pi->shminfo);
		       pi->pmap =
			  XShmCreatePixmap(disp, win, pi->shminfo->shmaddr,
					   pi->shminfo, w, h, VRoot.depth);
		       if (pi->pmap)
			 {
			    gcv.subwindow_mode = IncludeInferiors;
			    pi->gc = ECreateGC(win, GCSubwindowMode, &gcv);
			    if (pi->gc)
			       return pi;

			    EFreePixmap(pi->pmap);
			 }
		       XShmDetach(disp, pi->shminfo);
		       shmdt(pi->shminfo->shmaddr);
		    }
		  shmctl(pi->shminfo->shmid, IPC_RMID, 0);
	       }
	     XDestroyImage(pi->xim);
	  }
	Efree(pi->shminfo);
     }
   Efree(pi);
   return NULL;
}

static void
EDestroyPixImg(PixImg * pi)
{
   if (!pi)
      return;
   ecore_x_sync();
   XShmDetach(disp, pi->shminfo);
   shmdt(pi->shminfo->shmaddr);
   shmctl(pi->shminfo->shmid, IPC_RMID, 0);
   XDestroyImage(pi->xim);
   Efree(pi->shminfo);
   EFreePixmap(pi->pmap);
   EFreeGC(pi->gc);
   Efree(pi);
}

static void
EBlendRemoveShape(EWin * ewin, Pixmap pmap, int x, int y)
{
   XGCValues           gcv;
   int                 i, w, h;
   static GC           gc = 0, gcm = 0;
   static int          rn, ord;
   static XRectangle  *rl = NULL;
   static Pixmap       mask = 0;

   if (!ewin)
     {
	if (rl)
	   XFree(rl);
	if (gc)
	   EFreeGC(gc);
	if (gcm)
	   EFreeGC(gcm);
	if (mask)
	   EFreePixmap(mask);
	mask = 0;
	gc = 0;
	gcm = 0;
	rl = NULL;
	return;
     }

   w = EoGetW(ewin);
   h = EoGetH(ewin);
   if (!rl)
     {
	rl = EShapeGetRectangles(EoGetWin(ewin), ShapeBounding, &rn, &ord);
	if (rn < 1)
	   return;
	else if (rn == 1)
	  {
	     if ((rl[0].x == 0) && (rl[0].y == 0)
		 && (rl[0].width == EoGetW(ewin))
		 && (rl[0].height == EoGetH(ewin)))
	       {
		  if (rl)
		     XFree(rl);
		  rl = NULL;
		  return;
	       }
	  }
     }
   if (!mask)
      mask = ECreatePixmap(VRoot.win, w, h, 1);
   if (!gcm)
      gcm = ECreateGC(mask, 0, &gcv);
   if (!gc)
     {
	gcv.subwindow_mode = IncludeInferiors;
	gc = ECreateGC(VRoot.win, GCSubwindowMode, &gcv);
	XSetForeground(disp, gcm, 1);
	XFillRectangle(disp, mask, gcm, 0, 0, w, h);
	XSetForeground(disp, gcm, 0);
	for (i = 0; i < rn; i++)
	   XFillRectangle(disp, mask, gcm, rl[i].x, rl[i].y, rl[i].width,
			  rl[i].height);
	XSetClipMask(disp, gc, mask);
     }
   XSetClipOrigin(disp, gc, x, y);
   XCopyArea(disp, pmap, VRoot.win, gc, x, y, w, h, x, y);
}

static void
EBlendPixImg(EWin * ewin, PixImg * s1, PixImg * s2, PixImg * dst, int x, int y,
	     int w, int h)
{
   int                 ox, oy;
   int                 i, j;
   XGCValues           gcv;
   static int          rn, ord;
   static XRectangle  *rl = NULL;
   static GC           gc = 0;

   if (!s1)
     {
	if (gc)
	   EFreeGC(gc);
	if (rl > (XRectangle *) 1)
	   XFree(rl);
	gc = 0;
	rl = NULL;
	return;
     }
   if (!gc)
     {
	gcv.subwindow_mode = IncludeInferiors;
	gc = ECreateGC(VRoot.win, GCSubwindowMode, &gcv);
     }
   if (!rl)
     {
	rl = EShapeGetRectangles(EoGetWin(ewin), ShapeBounding, &rn, &ord);
	if (rl)
	   XSetClipRectangles(disp, gc, x, y, rl, rn, ord);
	if (!rl)
	   rl = (XRectangle *) 1;
     }
   else
      XSetClipOrigin(disp, gc, x, y);
   ox = 0;
   oy = 0;
   if ((x >= VRoot.w) || (y >= VRoot.h))
      return;
   if (x + w > VRoot.w)
      w -= ((x + w) - VRoot.w);
   if (x < 0)
     {
	ox = -x;
	w -= ox;
	x = 0;
     }
   if (y + h > VRoot.h)
      h -= ((y + h) - VRoot.h);
   if (y < 0)
     {
	oy = -y;
	h -= oy;
	y = 0;
     }
   if ((w <= 0) || (h <= 0))
      return;
   ecore_x_sync();
   if (dst)
     {
	switch (dst->xim->bits_per_pixel)
	  {
	  case 32:
	     for (j = 0; j < h; j++)
	       {
		  unsigned int       *ptr1, *ptr2, *ptr3;

		  ptr1 =
		     (unsigned int *)(s1->xim->data +
				      ((x) *
				       ((s1->xim->bits_per_pixel) >> 3)) +
				      ((j + y) * s1->xim->bytes_per_line));
		  ptr2 =
		     (unsigned int *)(s2->xim->data +
				      ((ox) *
				       ((s2->xim->bits_per_pixel) >> 3)) +
				      ((j + oy) * s2->xim->bytes_per_line));
		  ptr3 =
		     (unsigned int *)(dst->xim->data +
				      ((ox) *
				       ((dst->xim->bits_per_pixel) >> 3)) +
				      ((j + oy) * dst->xim->bytes_per_line));
		  for (i = 0; i < w; i++)
		    {
		       unsigned int        p1, p2;

		       p1 = *ptr1++;
		       p2 = *ptr2++;
		       *ptr3++ =
			  ((p1 >> 1) & 0x7f7f7f7f) +
			  ((p2 >> 1) & 0x7f7f7f7f) + (p1 & p2 & 0x01010101);
		    }
	       }
	     break;
	  case 24:
	     for (j = 0; j < h; j++)
	       {
		  for (i = 0; i < w; i++)
		    {
		       unsigned int        p1, p2;

		       p1 = XGetPixel(s1->xim, (i + x), (j + y));
		       p2 = XGetPixel(s2->xim, (i + ox), (j + oy));
		       XPutPixel(dst->xim, (i + ox), (j + oy),
				 (((p1 >> 1) & 0x7f7f7f7f) +
				  ((p2 >> 1) & 0x7f7f7f7f) +
				  (p1 & p2 & 0x01010101)));
		    }
	       }
	     break;
	  case 16:
	     if (DefaultDepth(disp, VRoot.scr) != 15)
	       {
		  for (j = 0; j < h; j++)
		    {
		       unsigned int       *ptr1, *ptr2, *ptr3;

		       ptr1 =
			  (unsigned int *)(s1->xim->data +
					   ((x) *
					    ((s1->xim->bits_per_pixel) >> 3)) +
					   ((j + y) * s1->xim->bytes_per_line));
		       ptr2 =
			  (unsigned int *)(s2->xim->data +
					   ((ox) *
					    ((s2->xim->bits_per_pixel) >> 3)) +
					   ((j +
					     oy) * s2->xim->bytes_per_line));
		       ptr3 =
			  (unsigned int *)(dst->xim->data +
					   ((ox) *
					    ((dst->xim->bits_per_pixel) >> 3)) +
					   ((j +
					     oy) * dst->xim->bytes_per_line));
		       if (!(w & 0x1))
			 {
			    for (i = 0; i < w; i += 2)
			      {
				 unsigned int        p1, p2;

				 p1 = *ptr1++;
				 p2 = *ptr2++;
				 *ptr3++ =
				    ((p1 >> 1) &
				     ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)
				      | (0x78 << 24) | (0x7c << 19) | (0x78
								       <<
								       13)))
				    +
				    ((p2 >> 1) &
				     ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)
				      | (0x78 << 24) | (0x7c << 19) | (0x78
								       <<
								       13)))
				    +
				    (p1 & p2 &
				     ((0x1 << 11) | (0x1 << 5) | (0x1) |
				      (0x1 << 27) | (0x1 << 21) | (0x1 << 16)));
			      }
			 }
		       else
			 {
			    for (i = 0; i < (w - 1); i += 2)
			      {
				 unsigned int        p1, p2;

				 p1 = *ptr1++;
				 p2 = *ptr2++;
				 *ptr3++ =
				    ((p1 >> 1) &
				     ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)
				      | (0x78 << 24) | (0x7c << 19) | (0x78
								       <<
								       13)))
				    +
				    ((p2 >> 1) &
				     ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)
				      | (0x78 << 24) | (0x7c << 19) | (0x78
								       <<
								       13)))
				    +
				    (p1 & p2 &
				     ((0x1 << 11) | (0x1 << 5) | (0x1) |
				      (0x1 << 27) | (0x1 << 21) | (0x1 << 16)));
			      }
			    {
			       unsigned short     *pptr1, *pptr2, *pptr3;
			       unsigned short      pp1, pp2;

			       pptr1 = (unsigned short *)ptr1;
			       pptr2 = (unsigned short *)ptr2;
			       pptr3 = (unsigned short *)ptr3;
			       pp1 = *pptr1;
			       pp2 = *pptr2;
			       *pptr3 =
				  ((pp1 >> 1) &
				   ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)))
				  +
				  ((pp2 >> 1) &
				   ((0x78 << 8) | (0x7c << 3) | (0x78 >> 3)))
				  +
				  (pp1 & pp2 &
				   ((0x1 << 11) | (0x1 << 5) | (0x1)));
			    }
			 }
		    }
	       }
	     else
	       {
		  for (j = 0; j < h; j++)
		    {
		       unsigned int       *ptr1, *ptr2, *ptr3;

		       ptr1 =
			  (unsigned int *)(s1->xim->data +
					   ((x) *
					    ((s1->xim->
					      bits_per_pixel) >> 3)) + ((j +
									 y) *
									s1->
									xim->
									bytes_per_line));
		       ptr2 =
			  (unsigned int *)(s2->xim->data +
					   ((ox) *
					    ((s2->xim->
					      bits_per_pixel) >> 3)) + ((j +
									 oy)
									*
									s2->
									xim->
									bytes_per_line));
		       ptr3 =
			  (unsigned int *)(dst->xim->data +
					   ((ox) *
					    ((dst->xim->
					      bits_per_pixel) >> 3)) + ((j +
									 oy)
									*
									dst->
									xim->
									bytes_per_line));
		       if (!(w & 0x1))
			 {
			    for (i = 0; i < w; i += 2)
			      {
				 unsigned int        p1, p2;

				 p1 = *ptr1++;
				 p2 = *ptr2++;
				 *ptr3++ =
				    ((p1 >> 1) &
				     ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)
				      | (0x78 << 23) | (0x78 << 18) | (0x78
								       <<
								       13)))
				    +
				    ((p2 >> 1) &
				     ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)
				      | (0x78 << 23) | (0x78 << 18) | (0x78
								       <<
								       13)))
				    +
				    (p1 & p2 &
				     ((0x1 << 10) | (0x1 << 5) | (0x1) |
				      (0x1 << 26) | (0x1 << 20) | (0x1 << 16)));
			      }
			 }
		       else
			 {
			    for (i = 0; i < (w - 1); i += 2)
			      {
				 unsigned int        p1, p2;

				 p1 = *ptr1++;
				 p2 = *ptr2++;
				 *ptr3++ =
				    ((p1 >> 1) &
				     ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)
				      | (0x78 << 23) | (0x78 << 18) | (0x78
								       <<
								       13)))
				    +
				    ((p2 >> 1) &
				     ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)
				      | (0x78 << 23) | (0x78 << 18) | (0x78
								       <<
								       13)))
				    +
				    (p1 & p2 &
				     ((0x1 << 10) | (0x1 << 5) | (0x1) |
				      (0x1 << 26) | (0x1 << 20) | (0x1 << 16)));
			      }
			    {
			       unsigned short     *pptr1, *pptr2, *pptr3;
			       unsigned short      pp1, pp2;

			       pptr1 = (unsigned short *)ptr1;
			       pptr2 = (unsigned short *)ptr2;
			       pptr3 = (unsigned short *)ptr3;
			       pp1 = *pptr1;
			       pp2 = *pptr2;
			       *pptr3++ =
				  ((pp1 >> 1) &
				   ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)))
				  +
				  ((pp2 >> 1) &
				   ((0x78 << 7) | (0x78 << 2) | (0x78 >> 3)))
				  +
				  (pp1 & pp2 &
				   ((0x1 << 10) | (0x1 << 5) | (0x1)));
			    }
			 }
		    }
	       }
	     break;
	  default:
	     for (j = 0; j < h; j++)
	       {
		  unsigned char      *ptr1, *ptr2, *ptr3;

		  ptr1 =
		     (unsigned char *)(s1->xim->data +
				       ((x) *
					((s1->xim->bits_per_pixel) >> 3)) +
				       ((j + y) * s1->xim->bytes_per_line));
		  ptr2 =
		     (unsigned char *)(s2->xim->data +
				       ((ox) *
					((s2->xim->bits_per_pixel) >> 3)) +
				       ((j + oy) * s2->xim->bytes_per_line));
		  ptr3 =
		     (unsigned char *)(dst->xim->data +
				       ((ox) *
					((dst->xim->bits_per_pixel) >> 3)) +
				       ((j + oy) * dst->xim->bytes_per_line));
		  if (!(w & 0x1))
		    {
		       if (j & 0x1)
			 {
			    ptr2++;
			    for (i = 0; i < w; i += 2)
			      {
				 unsigned char       p1;

				 p1 = *ptr1;
				 ptr1 += 2;
				 *ptr3++ = p1;
				 p1 = *ptr2;
				 ptr2 += 2;
				 *ptr3++ = p1;
			      }
			 }
		       else
			 {
			    ptr1++;
			    for (i = 0; i < w; i += 2)
			      {
				 unsigned char       p1;

				 p1 = *ptr2;
				 ptr2 += 2;
				 *ptr3++ = p1;
				 p1 = *ptr1;
				 ptr1 += 2;
				 *ptr3++ = p1;
			      }
			 }
		    }
		  else
		    {
		       if (j & 0x1)
			 {
			    ptr2++;
			    for (i = 0; i < (w - 1); i += 2)
			      {
				 unsigned char       p1;

				 p1 = *ptr1;
				 ptr1 += 2;
				 *ptr3++ = p1;
				 p1 = *ptr2;
				 ptr2 += 2;
				 *ptr3++ = p1;
			      }
			    *ptr3 = *ptr1;
			 }
		       else
			 {
			    ptr1++;
			    for (i = 0; i < (w - 1); i += 2)
			      {
				 unsigned char       p1;

				 p1 = *ptr2;
				 ptr2 += 2;
				 *ptr3++ = p1;
				 p1 = *ptr1;
				 ptr1 += 2;
				 *ptr3++ = p1;
			      }
			    *ptr3 = *ptr2;
			 }
		    }
	       }
	     break;
	  }
/* workaround since XCopyArea doesnt always work with shared pixmaps */
	XShmPutImage(disp, VRoot.win, gc, dst->xim, ox, oy, x, y, w, h, False);
/*      XCopyArea(disp, dst->pmap, VRoot.win, gc, ox, oy, w, h, x, y); */
     }
/* I dont believe it - you cannot do this to a shared pixmaps to the screen */
/* XCopyArea(disp, dst->pmap, VRoot.win, dst->gc, x, y, w, h, x, y); */
}

#include <X11/bitmaps/flipped_gray>
#include <X11/bitmaps/gray>
#include <X11/bitmaps/gray3>

void
DrawEwinShape(EWin * ewin, int md, int x, int y, int w, int h, char firstlast)
{
   static GC           gc = 0;
   XGCValues           gcv;
   int                 x1, y1, w1, h1, i, j, pw, ph, dx, dy;
   static Pixmap       b1 = 0, b2 = 0, b3 = 0;
   static Font         font = 0;
   int                 bpp;
   char                str[32], pq;
   char                check_move = 0;

   for (i = 0; i < ewin->num_groups; i++)
     {
	check_move |= ewin->groups[i]->cfg.move;
	if (check_move)
	   break;
     }

   if ((md == 5)
       && ((Mode.mode == MODE_RESIZE) || (Mode.mode == MODE_RESIZE_H)
	   || (Mode.mode == MODE_RESIZE_V) || (ewin->groups && check_move)))
      md = 0;

   if (md == 5)
     {
	if (VRoot.depth <= 8)
	   bpp = 1;
	else if (VRoot.depth <= 16)
	   bpp = 2;
	else if (VRoot.depth <= 24)
	   bpp = 3;
	else
	   bpp = 4;
     }

   pw = w;
   ph = h;
   pq = Mode.queue_up;
   Mode.queue_up = 0;

   switch (md)
     {
     case 0:
	MoveResizeEwin(ewin, x, y, w, h);
	EwinShapeSet(ewin);
	if (Mode.mode != MODE_NONE)
	   CoordsShow(ewin);
	break;
     case 1:
     case 2:
     case 3:
     case 4:
     case 5:
	if (!b1)
	   b1 = XCreateBitmapFromData(disp, VRoot.win, flipped_gray_bits,
				      flipped_gray_width, flipped_gray_height);
	if (!b2)
	   b2 = XCreateBitmapFromData(disp, VRoot.win, gray_bits, gray_width,
				      gray_height);
	if (!b3)
	   b3 = XCreateBitmapFromData(disp, VRoot.win, gray3_bits, gray3_width,
				      gray3_height);

	if ((Mode.mode == MODE_RESIZE) || (Mode.mode == MODE_RESIZE_H)
	    || (Mode.mode == MODE_RESIZE_V))
	  {
	     w1 = ewin->client.w;
	     h1 = ewin->client.h;
	     ewin->client.w = w;
	     ewin->client.h = h;
	     ICCCM_MatchSize(ewin);
	     i = (x - ewin->shape_x) / ewin->client.w_inc;
	     j = (y - ewin->shape_y) / ewin->client.h_inc;
	     x = ewin->shape_x + (i * ewin->client.w_inc);
	     y = ewin->shape_y + (j * ewin->client.h_inc);
	     ewin->client.w = w1;
	     ewin->client.h = h1;
	  }

	dx = DeskGetX(EoGetDesk(ewin));
	dy = DeskGetY(EoGetDesk(ewin));
	x1 = ewin->shape_x + dx;
	y1 = ewin->shape_y + dy;

	w1 = ewin->shape_w;
	h1 = ewin->shape_h;

	ewin->shape_x = x;
	ewin->shape_y = y;
	x += dx;
	y += dy;

	if ((w != ewin->client.w) || (h != ewin->client.h))
	  {
	     ewin->client.w = w;
	     ewin->client.h = h;
	     ICCCM_MatchSize(ewin);
	     if (!ewin->shaded)
	       {
		  ewin->shape_w = ewin->client.w;
		  ewin->shape_h = ewin->client.h;
	       }
	  }
	w = ewin->shape_w;
	h = ewin->shape_h;

	if (!gc)
	  {
	     gcv.function = GXxor;
	     gcv.foreground = WhitePixel(disp, VRoot.scr);
	     if (gcv.foreground == 0)
		gcv.foreground = BlackPixel(disp, VRoot.scr);
	     gcv.subwindow_mode = IncludeInferiors;
	     gc = ECreateGC(VRoot.win,
			    GCFunction | GCForeground | GCSubwindowMode, &gcv);
	  }
#define DRAW_H_ARROW(x1, x2, y1) \
      if (((x2) - (x1)) >= 12) \
        { \
          XDrawLine(disp, VRoot.win, gc, x1, y1, (x1) + 6, (y1) - 3); \
          XDrawLine(disp, VRoot.win, gc, x1, y1, (x1) + 6, (y1) + 3); \
          XDrawLine(disp, VRoot.win, gc, x2, y1, (x2) - 6, (y1) - 3); \
          XDrawLine(disp, VRoot.win, gc, x2, y1, (x2) - 6, (y1) + 3); \
        } \
      if ((x2) >= (x1)) \
        { \
          XDrawLine(disp, VRoot.win, gc, x1, y1, x2, y1); \
          Esnprintf(str, sizeof(str), "%i", (x2) - (x1) + 1); \
          XDrawString(disp, VRoot.win, gc, ((x1) + (x2)) / 2, (y1) - 10, str, strlen(str)); \
        }
#define DRAW_V_ARROW(y1, y2, x1) \
      if (((y2) - (y1)) >= 12) \
        { \
          XDrawLine(disp, VRoot.win, gc, x1, y1, (x1) + 3, (y1) + 6); \
          XDrawLine(disp, VRoot.win, gc, x1, y1, (x1) - 3, (y1) + 6); \
          XDrawLine(disp, VRoot.win, gc, x1, y2, (x1) + 3, (y2) - 6); \
          XDrawLine(disp, VRoot.win, gc, x1, y2, (x1) - 3, (y2) - 6); \
        } \
      if ((y2) >= (y1)) \
        { \
          XDrawLine(disp, VRoot.win, gc, x1, y1, x1, y2); \
          Esnprintf(str, sizeof(str), "%i", (y2) - (y1) + 1); \
          XDrawString(disp, VRoot.win, gc, x1 + 10, ((y1) + (y2)) / 2, str, strlen(str)); \
        }
#define DO_DRAW_MODE_1(aa, bb, cc, dd) \
      if (!font) \
        font = XLoadFont(disp, "-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*"); \
      XSetFont(disp, gc, font); \
      if (cc < 3) cc = 3; \
      if (dd < 3) dd = 3; \
      DRAW_H_ARROW(aa + ewin->border->border.left, \
                   aa + ewin->border->border.left + cc - 1, \
                   bb + ewin->border->border.top + dd - 16); \
      DRAW_H_ARROW(0, \
                   aa - 1, \
                   bb + ewin->border->border.top + (dd / 2)); \
      DRAW_H_ARROW(aa + cc + ewin->border->border.left + ewin->border->border.right, \
                   VRoot.w - 1, \
                   bb + ewin->border->border.top + (dd / 2)); \
      DRAW_V_ARROW(bb + ewin->border->border.top, \
                   bb + ewin->border->border.top + dd - 1, \
                   aa + ewin->border->border.left + 16); \
      DRAW_V_ARROW(0, \
                   bb - 1, \
                   aa + ewin->border->border.left + (cc / 2)); \
      DRAW_V_ARROW(bb + dd + ewin->border->border.top + ewin->border->border.bottom, \
                   VRoot.h - 1, \
                   aa + ewin->border->border.left + (cc / 2)); \
      XDrawLine(disp, VRoot.win, gc, aa, 0, aa, VRoot.h); \
      XDrawLine(disp, VRoot.win, gc, \
		aa + cc + ewin->border->border.left + \
		ewin->border->border.right - 1, 0, \
		aa + cc + ewin->border->border.left + \
		ewin->border->border.right - 1, VRoot.h); \
      XDrawLine(disp, VRoot.win, gc, 0, bb, VRoot.w, bb); \
      XDrawLine(disp, VRoot.win, gc, 0, \
		bb + dd + ewin->border->border.top + \
		ewin->border->border.bottom - 1, VRoot.w, \
		bb + dd + ewin->border->border.top + \
		ewin->border->border.bottom - 1); \
      XDrawRectangle(disp, VRoot.win, gc, aa + ewin->border->border.left + 1, \
		     bb + ewin->border->border.top + 1, cc - 3, dd - 3);

#define DO_DRAW_MODE_2(aa, bb, cc, dd) \
      if (cc < 3) cc = 3; \
      if (dd < 3) dd = 3; \
      XDrawRectangle(disp, VRoot.win, gc, aa, bb, \
                     cc + ewin->border->border.left + \
                     ewin->border->border.right - 1, \
                     dd + ewin->border->border.top + \
                     ewin->border->border.bottom - 1); \
      XDrawRectangle(disp, VRoot.win, gc, aa + ewin->border->border.left + 1, \
		     bb + ewin->border->border.top + 1, cc - 3, dd - 3);

#define DO_DRAW_MODE_3(aa, bb, cc, dd) \
      XSetFillStyle(disp, gc, FillStippled); \
      XSetStipple(disp, gc, b2); \
      if ((cc + ewin->border->border.left + ewin->border->border.right > 0) && \
          (ewin->border->border.top > 0)) \
      XFillRectangle(disp, VRoot.win, gc, aa, bb, \
                     cc + ewin->border->border.left + \
                     ewin->border->border.right, \
                     ewin->border->border.top); \
      if ((cc + ewin->border->border.left + ewin->border->border.right > 0) && \
          (ewin->border->border.bottom > 0)) \
      XFillRectangle(disp, VRoot.win, gc, aa, bb + dd + \
                     ewin->border->border.top, \
                     cc + ewin->border->border.left + \
                     ewin->border->border.right, \
                     ewin->border->border.bottom); \
      if ((dd > 0) && (ewin->border->border.left > 0)) \
      XFillRectangle(disp, VRoot.win, gc, aa, bb + ewin->border->border.top, \
                     ewin->border->border.left, \
                     dd); \
      if ((dd > 0) && (ewin->border->border.right > 0)) \
      XFillRectangle(disp, VRoot.win, gc, aa + cc + ewin->border->border.left, \
                     bb + ewin->border->border.top, \
                     ewin->border->border.right, \
                     dd); \
      XSetStipple(disp, gc, b3); \
      if ((cc > 0) && (dd > 0)) \
        XFillRectangle(disp, VRoot.win, gc, aa + ewin->border->border.left + 1, \
  		       bb + ewin->border->border.top + 1, cc - 3, dd - 3);

#define DO_DRAW_MODE_4(aa, bb, cc, dd) \
      XSetFillStyle(disp, gc, FillStippled); \
      XSetStipple(disp, gc, b2); \
      XFillRectangle(disp, VRoot.win, gc, aa, bb, \
                     cc + ewin->border->border.left + \
                     ewin->border->border.right, \
                     dd + ewin->border->border.top + \
                     ewin->border->border.bottom);
	if (md == 1)
	  {
	     if (firstlast > 0)
	       {
		  DO_DRAW_MODE_1(x1, y1, w1, h1);
	       }
	     if ((Mode.mode != MODE_NONE)
		 && (!ewin->groups || (ewin->groups && !check_move)))
		CoordsShow(ewin);
	     if (firstlast < 2)
	       {
		  DO_DRAW_MODE_1(x, y, w, h);
	       }
	  }
	else if (md == 2)
	  {
	     if (firstlast > 0)
	       {
		  DO_DRAW_MODE_2(x1, y1, w1, h1);
	       }
	     if ((Mode.mode != MODE_NONE)
		 && (!ewin->groups || (ewin->groups && !check_move)))
		CoordsShow(ewin);
	     if (firstlast < 2)
	       {
		  DO_DRAW_MODE_2(x, y, w, h);
	       }
	  }
	else if (md == 3)
	  {
	     if (firstlast > 0)
	       {
		  DO_DRAW_MODE_3(x1, y1, w1, h1);
	       }
	     if ((Mode.mode != MODE_NONE)
		 && (!ewin->groups || (ewin->groups && !check_move)))
		CoordsShow(ewin);
	     if (firstlast < 2)
	       {
		  DO_DRAW_MODE_3(x, y, w, h);
	       }
	  }
	else if (md == 4)
	  {
	     if (firstlast > 0)
	       {
		  DO_DRAW_MODE_4(x1, y1, w1, h1);
	       }
	     if ((Mode.mode != MODE_NONE)
		 && (!ewin->groups || (ewin->groups && !check_move)))
		CoordsShow(ewin);
	     if (firstlast < 2)
	       {
		  DO_DRAW_MODE_4(x, y, w, h);
	       }
	  }
	else if (md == 5)
	  {
	     static PixImg      *ewin_pi = NULL;
	     static PixImg      *root_pi = NULL;
	     static PixImg      *draw_pi = NULL;

	     if (firstlast == 0)
	       {
		  XGCValues           gcv2;
		  GC                  gc2;

		  if (ewin_pi)
		     EDestroyPixImg(ewin_pi);
		  if (root_pi)
		     EDestroyPixImg(root_pi);
		  if (draw_pi)
		     EDestroyPixImg(draw_pi);
		  EBlendRemoveShape(NULL, 0, 0, 0);
		  EBlendPixImg(NULL, NULL, NULL, NULL, 0, 0, 0, 0);
		  ewin_pi = NULL;
		  root_pi = NULL;
		  draw_pi = NULL;
		  root_pi = ECreatePixImg(VRoot.win, VRoot.w, VRoot.h);
		  ewin_pi =
		     ECreatePixImg(VRoot.win, EoGetW(ewin), EoGetH(ewin));
		  draw_pi =
		     ECreatePixImg(VRoot.win, EoGetW(ewin), EoGetH(ewin));
		  if ((!root_pi) || (!ewin_pi) || (!draw_pi))
		    {
		       Conf.movres.mode_move = 0;
		       ecore_x_ungrab();
		       DrawEwinShape(ewin, Conf.movres.mode_move, x, y, w, h,
				     firstlast);
		       goto done;
		    }
		  EFillPixmap(VRoot.win, root_pi->pmap, x1, y1, EoGetW(ewin),
			      EoGetH(ewin));
		  gc2 = ECreateGC(root_pi->pmap, 0, &gcv2);
		  XCopyArea(disp, root_pi->pmap, ewin_pi->pmap, gc2, x1, y1,
			    EoGetW(ewin), EoGetH(ewin), 0, 0);
		  EFreeGC(gc2);
		  EBlendPixImg(ewin, root_pi, ewin_pi, draw_pi, x, y,
			       EoGetW(ewin), EoGetH(ewin));
	       }
	     else if (firstlast == 1)
	       {
		  int                 wt, ht;
		  int                 adx, ady;

		  dx = x - x1;
		  dy = y - y1;
		  if (dx < 0)
		     adx = -dx;
		  else
		     adx = dx;
		  if (dy < 0)
		     ady = -dy;
		  else
		     ady = dy;
		  wt = EoGetW(ewin);
		  ht = EoGetH(ewin);
		  if ((adx <= wt) && (ady <= ht))
		    {
		       if (dx < 0)
			  EFillPixmap(VRoot.win, root_pi->pmap, x, y, -dx, ht);
		       else if (dx > 0)
			  EFillPixmap(VRoot.win, root_pi->pmap, x + wt - dx, y,
				      dx, ht);
		       if (dy < 0)
			  EFillPixmap(VRoot.win, root_pi->pmap, x, y, wt, -dy);
		       else if (dy > 0)
			  EFillPixmap(VRoot.win, root_pi->pmap, x, y + ht - dy,
				      wt, dy);
		    }
		  else
		     EFillPixmap(VRoot.win, root_pi->pmap, x, y, wt, ht);
		  if ((adx <= wt) && (ady <= ht))
		    {
		       EBlendPixImg(ewin, root_pi, ewin_pi, draw_pi, x, y,
				    EoGetW(ewin), EoGetH(ewin));
		       if (dx > 0)
			  EPastePixmap(VRoot.win, root_pi->pmap, x1, y1, dx,
				       ht);
		       else if (dx < 0)
			  EPastePixmap(VRoot.win, root_pi->pmap, x1 + wt + dx,
				       y1, -dx, ht);
		       if (dy > 0)
			  EPastePixmap(VRoot.win, root_pi->pmap, x1, y1, wt,
				       dy);
		       else if (dy < 0)
			  EPastePixmap(VRoot.win, root_pi->pmap, x1,
				       y1 + ht + dy, wt, -dy);
		    }
		  else
		    {
		       EPastePixmap(VRoot.win, root_pi->pmap, x1, y1, wt, ht);
		       EBlendPixImg(ewin, root_pi, ewin_pi, draw_pi, x, y,
				    EoGetW(ewin), EoGetH(ewin));
		    }
		  EBlendRemoveShape(ewin, root_pi->pmap, x, y);
	       }
	     else if (firstlast == 2)
	       {
		  EPastePixmap(VRoot.win, root_pi->pmap, x1, y1, EoGetW(ewin),
			       EoGetH(ewin));
		  if (ewin_pi)
		     EDestroyPixImg(ewin_pi);
		  if (root_pi)
		     EDestroyPixImg(root_pi);
		  if (draw_pi)
		     EDestroyPixImg(draw_pi);
		  EBlendRemoveShape(NULL, 0, 0, 0);
		  EBlendPixImg(NULL, NULL, NULL, NULL, 0, 0, 0, 0);
		  ewin_pi = NULL;
		  root_pi = NULL;
		  draw_pi = NULL;
	       }
	     else if (firstlast == 3)
	       {
		  EPastePixmap(VRoot.win, root_pi->pmap, x, y, EoGetW(ewin),
			       EoGetH(ewin));
		  if (root_pi)
		     EDestroyPixImg(root_pi);
		  root_pi->pmap = 0;
	       }
	     else if (firstlast == 4)
	       {
		  int                 wt, ht;

		  wt = EoGetW(ewin);
		  ht = EoGetH(ewin);
		  root_pi = ECreatePixImg(VRoot.win, VRoot.w, VRoot.h);
		  EFillPixmap(VRoot.win, root_pi->pmap, x, y, wt, ht);
		  EBlendPixImg(ewin, root_pi, ewin_pi, draw_pi, x, y,
			       EoGetW(ewin), EoGetH(ewin));
	       }
	     else if (firstlast == 5)
	       {
		  if (root_pi)
		     EDestroyPixImg(root_pi);
		  root_pi->pmap = 0;
	       }
	     if (Mode.mode != MODE_NONE)
		CoordsShow(ewin);
	  }
	if (firstlast == 2)
	  {
#if 1				/* FIXME - Here? */
	     /* If we're moving a group, don't do this,
	      * otherwise we have a lot of garbage onscreen */
	     if (!EoIsFloating(ewin) || !ewin->groups
		 || (ewin->groups && !check_move))
	       {
		  if (ewin->shaded)
		     MoveEwin(ewin, ewin->shape_x, ewin->shape_y);
		  else
		     MoveResizeEwin(ewin, ewin->shape_x, ewin->shape_y, pw, ph);
	       }
#endif
	     EFreeGC(gc);
	     gc = 0;
	  }
	break;
     default:
	break;
     }

   if (firstlast == 0 || firstlast == 2 || firstlast == 4)
     {
	ewin->req_x = ewin->shape_x;
	ewin->req_y = ewin->shape_y;
	if (firstlast == 2)
	   CoordsHide();
     }

 done:
   Mode.queue_up = pq;
}

Imlib_Image        *
ELoadImage(const char *file)
{
   Imlib_Image        *im;
   char               *f = NULL;

   if (!file)
      return NULL;

   if (file[0] == '/')
     {
	im = imlib_load_image(file);
	return im;
     }

   f = ThemeFileFind(file);
   if (f)
     {
	im = imlib_load_image(f);
	Efree(f);
	return im;
     }

   return NULL;
}

void
PropagateShapes(Window win)
{
   Window              rt, par, *list = NULL;
   int                 k, i, num = 0, num_rects = 0, rn = 0, ord;
   int                 x, y, ww, hh, w, h, d;
   XRectangle         *rects = NULL, *rl = NULL;
   XWindowAttributes   att;

   if (Mode.queue_up)
     {
	DrawQueue          *dq;

	dq = Ecalloc(1, sizeof(DrawQueue));
	dq->win = win;
	dq->shape_propagate = 1;
	AddItem(dq, "DRAW", dq->win, LIST_TYPE_DRAW);
	return;
     }
   EGetGeometry(win, &rt, &x, &y, &w, &h, &d, &d);
   if ((w <= 0) || (h <= 0))
      return;

   ww = w;
   hh = h;

   XQueryTree(disp, win, &rt, &par, &list, (unsigned int *)&num);
   if (list)
     {
	/* go through all child windows and create/inset spans */
	for (i = 0; i < num; i++)
	  {
	     XGetWindowAttributes(disp, list[i], &att);
	     x = att.x;
	     y = att.y;
	     w = att.width;
	     h = att.height;
	     if ((att.class == InputOutput) && (att.map_state != IsUnmapped))
	       {
		  rl = NULL;
		  rl = EShapeGetRectangles(list[i], ShapeBounding, &rn, &ord);
		  if (rl)
		    {
		       num_rects += rn;
		       if (rn > 0)
			 {
			    rects =
			       Erealloc(rects, num_rects * sizeof(XRectangle));
			    /* go through all clip rects in thsi window's shape */
			    for (k = 0; k < rn; k++)
			      {
				 /* for each clip rect, add it to the rect list */
				 rects[num_rects - rn + k].x = x + rl[k].x;
				 rects[num_rects - rn + k].y = y + rl[k].y;
				 rects[num_rects - rn + k].width = rl[k].width;
				 rects[num_rects - rn + k].height =
				    rl[k].height;
			      }
			 }
		       Efree(rl);
		    }
		  else
		    {
		       num_rects++;
		       rects = Erealloc(rects, num_rects * sizeof(XRectangle));

		       rects[num_rects - 1].x = x;
		       rects[num_rects - 1].y = y;
		       rects[num_rects - 1].width = w;
		       rects[num_rects - 1].height = h;
		    }
	       }
	  }
	/* set the rects as the shape mask */
	if (rects)
	  {
	     EShapeCombineRectangles(win, ShapeBounding, 0, 0, rects,
				     num_rects, ShapeSet, Unsorted);
	     Efree(rects);
	     rl = NULL;
	     rl = EShapeGetRectangles(win, ShapeBounding, &rn, &ord);
	     if (rl)
	       {
		  if (rn < 1)
		     EShapeCombineMask(win, ShapeBounding, 0, 0, None,
				       ShapeSet);
		  else if (rn == 1)
		    {
		       if ((rl[0].x == 0) && (rl[0].y == 0)
			   && (rl[0].width == ww) && (rl[0].height == hh))
			  EShapeCombineMask(win, ShapeBounding, 0, 0,
					    None, ShapeSet);
		    }
		  Efree(rl);
	       }
	     else
		EShapeCombineMask(win, ShapeBounding, 0, 0, None, ShapeSet);
	  }
	XFree(list);
     }
}
