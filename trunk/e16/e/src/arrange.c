/*
 * Copyright (C) 2000 Carsten Haitzler, Geoff Harrison and various contributors
 * *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 * *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"

int                 ArrangeAddToList(int **array, int current_size, int value);
void                ArrangeSwapList(RectBox * list, int a, int b);

int
ArrangeAddToList(int **array, int current_size, int value)
{
   int                 i, j;

   EDBUG(8, "ArrangeAddToList");
   for (i = 0; i < current_size; i++)
     {
	if (value < (*array)[i])
	  {
	     for (j = current_size; j > i; j--)
		(*array)[j] = (*array)[j - 1];
	     (*array)[i] = value;
	     EDBUG_RETURN(current_size + 1);
	  }
	else if (value == (*array)[i])
	   EDBUG_RETURN(current_size);
     }
   (*array)[current_size] = value;
   EDBUG_RETURN(current_size + 1);
}

void
ArrangeSwapList(RectBox * list, int a, int b)
{
   RectBox             bb;

   EDBUG(8, "ArrangeSwapList");
   bb.data = list[a].data;
   bb.x = list[a].x;
   bb.y = list[a].y;
   bb.w = list[a].w;
   bb.h = list[a].h;
   list[a].data = list[b].data;
   list[a].x = list[b].x;
   list[a].y = list[b].y;
   list[a].w = list[b].w;
   list[a].h = list[b].h;
   list[b].data = bb.data;
   list[b].x = bb.x;
   list[b].y = bb.y;
   list[b].w = bb.w;
   list[b].h = bb.h;
   EDBUG_RETURN_;
}

void
ArrangeRects(RectBox * fixed, int fixed_count, RectBox * floating,
	     int floating_count, RectBox * sorted, int startx, int starty,
	     int width, int height, int policy)
{
   int                 num_sorted = 0;
   int                 xsize = 0, ysize = 0;
   int                *xarray = NULL, *yarray = NULL;
   int                *leftover = NULL;
   int                 i, j, k, x, y, x1, x2, y1, y2;
   unsigned char      *filled = NULL;
   RectBox            *spaces = NULL;
   int                 num_spaces = 0;
   int                 sort;
   int                 a1, a2;
   int                 num_leftover = 0;

   EDBUG(7, "ArrangeRects");

   switch (policy)
     {
     case ARRANGE_VERBATIM:
	break;
     case ARRANGE_BY_SIZE:
	sort = 0;
	while (!sort)
	  {
	     sort = 1;
	     for (i = 0; i < floating_count - 1; i++)
	       {
		  a1 = floating[i].w * floating[i].h;
		  a2 = floating[i + 1].w * floating[i + 1].h;
		  if (a2 > a1)
		    {
		       sort = 0;
		       ArrangeSwapList(floating, i, i + 1);
		    }
	       }
	  }
	break;
     case ARRANGE_BY_POSITION:
	sort = 0;
	while (!sort)
	  {
	     sort = 1;
	     for (i = 0; i < floating_count - 1; i++)
	       {
		  a1 = floating[i].x + floating[i].y;
		  a2 = (floating[i + 1].x + (floating[i + 1].w >> 1)) +
		     (floating[i + 1].y + (floating[i + 1].h >> 1));
		  if (a2 < a1)
		    {
		       sort = 0;
		       ArrangeSwapList(floating, i, i + 1);
		    }
	       }
	  }
	break;
     default:
	break;
     }
/* for every floating rect in order, "fit" it into the sorted list */
   i = ((fixed_count + floating_count) * 2) + 2;
   xarray = Emalloc(i * sizeof(int));
   yarray = Emalloc(i * sizeof(int));
   filled = Emalloc(i * i * sizeof(char));

   spaces = Emalloc(i * i * sizeof(RectBox));
   if (floating_count)
      leftover = Emalloc(floating_count * sizeof(int));

   if (!xarray)
     {
	if (yarray)
	   Efree(yarray);
	if (filled)
	   Efree(filled);
	if (spaces)
	   Efree(spaces);
	if (leftover)
	   Efree(leftover);
	EDBUG_RETURN_;
     }
   if (!yarray)
     {
	Efree(xarray);
	if (filled)
	   Efree(filled);
	if (spaces)
	   Efree(spaces);
	if (leftover)
	   Efree(leftover);
	EDBUG_RETURN_;
     }
   if (!filled)
     {
	Efree(xarray);
	Efree(yarray);
	if (spaces)
	   Efree(spaces);
	if (leftover)
	   Efree(leftover);
	EDBUG_RETURN_;
     }
   if (!spaces)
     {
	Efree(xarray);
	Efree(yarray);
	Efree(filled);
	if (leftover)
	   Efree(leftover);
	EDBUG_RETURN_;
     }
/* copy "fixed" rects into the sorted list */
   for (i = 0; i < fixed_count; i++)
     {
	sorted[num_sorted].data = fixed[i].data;
	sorted[num_sorted].x = fixed[i].x;
	sorted[num_sorted].y = fixed[i].y;
	sorted[num_sorted].w = fixed[i].w;
	sorted[num_sorted].h = fixed[i].h;
	sorted[num_sorted].p = fixed[i].p;
	num_sorted++;
     }
/* go through each floating rect in order and "fit" it in */
   for (i = 0; i < floating_count; i++)
     {
	xsize = 0;
	ysize = 0;
/* put all the sorted rects into the xy arrays */
	xsize = ArrangeAddToList(&xarray, xsize, startx);
	xsize = ArrangeAddToList(&xarray, xsize, width);
	ysize = ArrangeAddToList(&yarray, ysize, starty);
	ysize = ArrangeAddToList(&yarray, ysize, height);
	for (j = 0; j < num_sorted; j++)
	  {
	     if (sorted[j].x < width)
		xsize = ArrangeAddToList(&xarray, xsize, sorted[j].x);
	     if ((sorted[j].x + sorted[j].w) < width)
		xsize =
		   ArrangeAddToList(&xarray, xsize, sorted[j].x + sorted[j].w);
	     if (sorted[j].y < height)
		ysize = ArrangeAddToList(&yarray, ysize, sorted[j].y);
	     if ((sorted[j].y + sorted[j].h) < height)
		ysize =
		   ArrangeAddToList(&yarray, ysize, sorted[j].y + sorted[j].h);
	  }
/* fill the allocation array */
	for (j = 0; j < (xsize - 1) * (ysize - 1); filled[j++] = 0);
	for (j = 0; j < num_sorted; j++)
	  {
	     x1 = -1;
	     x2 = -1;
	     y1 = -1;
	     y2 = -1;
	     for (k = 0; k < xsize - 1; k++)
	       {
		  if (sorted[j].x == xarray[k])
		    {
		       x1 = k;
		       x2 = k;
		    }
		  if (sorted[j].x + sorted[j].w == xarray[k + 1])
		     x2 = k;
	       }
	     for (k = 0; k < ysize - 1; k++)
	       {
		  if (sorted[j].y == yarray[k])
		    {
		       y1 = k;
		       y2 = k;
		    }
		  if (sorted[j].y + sorted[j].h == yarray[k + 1])
		     y2 = k;
	       }
	     if ((x1 >= 0) && (x2 >= 0) && (y1 >= 0) && (y2 >= 0))
	       {
		  for (y = y1; y <= y2; y++)
		    {
		       for (x = x1; x <= x2; x++)
			 {
			    if (filled[(y * (xsize - 1)) + x] <
				(sorted[j].p + 1))
			       filled[(y * (xsize - 1)) + x] = sorted[j].p + 1;
			 }
		    }
	       }
	  }
	num_spaces = 0;
/* create list of all "spaces" */
	for (y = 0; y < ysize - 1; y++)
	  {
	     for (x = 0; x < xsize - 1; x++)
	       {
/* if the square is empty (lowe prioiryt suares filled) "grow" the space */
		  if (filled[(y * (xsize - 1)) + x] < (floating[i].p + 1))
		    {
		       int                 can_expand_x = 1;
		       int                 can_expand_y = 1;

		       x1 = x + 1;
		       y1 = y + 1;
		       filled[(y * (xsize - 1)) + x] = 100;
		       if (x >= xsize - 2)
			  can_expand_x = 0;
		       if (y >= ysize - 2)
			  can_expand_y = 0;
		       while ((can_expand_x) || (can_expand_y))
			 {
			    if (x1 >= xsize - 1)
			       can_expand_x = 0;
			    if (y1 >= ysize - 1)
			       can_expand_y = 0;
			    if (can_expand_x)
			      {
				 for (j = y; j < y1; j++)
				   {
				      if (filled[(j * (xsize - 1)) + x1] >=
					  (floating[i].p + 1))
					 can_expand_x = 0;
				   }
			      }
			    if (can_expand_x)
			       x1++;
			    if (can_expand_y)
			      {
				 for (j = x; j < x1; j++)
				   {
				      if (filled[(y1 * (xsize - 1)) + j] >=
					  (floating[i].p + 1))
					 can_expand_y = 0;
				   }
			      }
			    if (can_expand_y)
			       y1++;
			 }
		       spaces[num_spaces].x = xarray[x];
		       spaces[num_spaces].y = yarray[y];
		       spaces[num_spaces].w = xarray[x1] - xarray[x];
		       spaces[num_spaces].h = yarray[y1] - yarray[y];
		       spaces[num_spaces].p = 0;
		       num_spaces++;
		    }
	       }
	  }
/* find the first space that fits */
	k = -1;
	sort = 0x7fffffff;
	for (j = 0; j < num_spaces; j++)
	  {
	     if ((spaces[j].w >= floating[i].w) &&
		 (spaces[j].h >= floating[i].h))
	       {
		  if (policy == ARRANGE_BY_POSITION)
		    {
		       a1 = (spaces[j].x + (spaces[j].w >> 1)) -
			  (floating[i].x + (floating[i].w >> 1));
		       a2 = (spaces[j].y + (spaces[j].h >> 1)) -
			  (floating[i].y + (floating[i].h >> 1));
		       if (a1 < 0)
			  a1 = -a1;
		       if (a2 < 0)
			  a2 = -a2;
		       if ((a1 + a2) < sort)
			 {
			    sort = a1 + a2;
			    k = j;
			 }
		    }
		  else
		    {
		       k = j;
		       j = num_spaces;
		    }
	       }
	  }
	if (k >= 0)
	  {
	     if (policy == ARRANGE_BY_POSITION)
	       {
		  a1 = (spaces[k].x + (spaces[k].w >> 1)) -
		     (floating[i].x + (floating[i].w >> 1));
		  a2 = (spaces[k].y + (spaces[k].h >> 1)) -
		     (floating[i].y + (floating[i].h >> 1));
		  if (a1 >= 0)
		     sorted[num_sorted].x = spaces[k].x;
		  else
		     sorted[num_sorted].x =
			spaces[k].x + spaces[k].w - floating[i].w;
		  if (a2 >= 0)
		     sorted[num_sorted].y = spaces[k].y;
		  else
		     sorted[num_sorted].y =
			spaces[k].y + spaces[k].h - floating[i].h;
	       }
	     else
	       {
		  sorted[num_sorted].x = spaces[k].x;
		  sorted[num_sorted].y = spaces[k].y;
	       }
	     sorted[num_sorted].data = floating[i].data;
	     sorted[num_sorted].w = floating[i].w;
	     sorted[num_sorted].h = floating[i].h;
	     sorted[num_sorted].p = floating[i].p;
	     num_sorted++;
	  }
	else
	   leftover[num_leftover++] = i;
     }
/* ok we cant fit everything in this baby.... time fit the leftovers into the */
/* leftover space */
   for (i = 0; i < num_leftover; i++)
     {
	xsize = 0;
	ysize = 0;
/* put all the sorted rects into the xy arrays */
	xsize = ArrangeAddToList(&xarray, xsize, 0);
	xsize = ArrangeAddToList(&xarray, xsize, width);
	ysize = ArrangeAddToList(&yarray, ysize, 0);
	ysize = ArrangeAddToList(&yarray, ysize, height);
	for (j = 0; j < num_sorted; j++)
	  {
	     if (sorted[j].x < width)
		xsize = ArrangeAddToList(&xarray, xsize, sorted[j].x);
	     if ((sorted[j].x + sorted[j].w) < width)
		xsize =
		   ArrangeAddToList(&xarray, xsize, sorted[j].x + sorted[j].w);
	     if (sorted[j].y < height)
		ysize = ArrangeAddToList(&yarray, ysize, sorted[j].y);
	     if ((sorted[j].y + sorted[j].h) < height)
		ysize =
		   ArrangeAddToList(&yarray, ysize, sorted[j].y + sorted[j].h);
	  }
/* fill the allocation array */
	for (j = 0; j < (xsize - 1) * (ysize - 1); filled[j++] = 0);
	for (j = 0; j < num_sorted; j++)
	  {
	     x1 = -1;
	     x2 = -1;
	     y1 = -1;
	     y2 = -1;
	     for (k = 0; k < xsize - 1; k++)
	       {
		  if (sorted[j].x == xarray[k])
		    {
		       x1 = k;
		       x2 = k;
		    }
		  if (sorted[j].x + sorted[j].w == xarray[k + 1])
		     x2 = k;
	       }
	     for (k = 0; k < ysize - 1; k++)
	       {
		  if (sorted[j].y == yarray[k])
		    {
		       y1 = k;
		       y2 = k;
		    }
		  if (sorted[j].y + sorted[j].h == yarray[k + 1])
		     y2 = k;
	       }
	     if ((x1 >= 0) && (x2 >= 0) && (y1 >= 0) && (y2 >= 0))
	       {
		  for (y = y1; y <= y2; y++)
		    {
		       for (x = x1; x <= x2; x++)
			 {
			    if (filled[(y * (xsize - 1)) + x] <
				(sorted[j].p + 1))
			       filled[(y * (xsize - 1)) + x] = sorted[j].p + 1;
			 }
		    }
	       }
	  }
	num_spaces = 0;
/* create list of all "spaces" */
	for (y = 0; y < ysize - 1; y++)
	  {
	     for (x = 0; x < xsize - 1; x++)
	       {
/* if the square is empty "grow" the space */
		  if (!filled[(y * (xsize - 1)) + x])
		    {
		       int                 can_expand_x = 1;
		       int                 can_expand_y = 1;
		       char                fitswin = 1;

		       x1 = x + 1;
		       y1 = y + 1;
		       if (x >= xsize - 2)
			  can_expand_x = 0;
		       if (y >= ysize - 2)
			  can_expand_y = 0;
		       while ((can_expand_x) || (can_expand_y))
			 {
			    if (x1 >= xsize - 1)
			       can_expand_x = 0;
			    if (y1 >= ysize - 1)
			       can_expand_y = 0;
			    if (can_expand_x)
			      {
				 for (j = y; j < y1; j++)
				   {
				      if (filled[(j * (xsize - 1)) + x1] >=
					  (floating[leftover[i]].p + 1))
					{
					   if (filled[(j * (xsize - 1)) + x1] >
					       (floating[leftover[i]].p + 1))
					      fitswin = 0;
					   can_expand_x = 0;
					}
				   }
			      }
			    if (can_expand_x)
			       x1++;
			    if (can_expand_y)
			      {
				 for (j = x; j < x1; j++)
				   {
				      if (filled[(y1 * (xsize - 1)) + j] >=
					  (floating[leftover[i]].p + 1))
					{
					   if (filled[(y1 * (xsize - 1)) + j] >
					       (floating[leftover[i]].p + 1))
					      fitswin = 0;
					   can_expand_y = 0;
					}
				   }
			      }
			    if (can_expand_y)
			       y1++;
			 }
		       spaces[num_spaces].x = xarray[x];
		       spaces[num_spaces].y = yarray[y];
		       spaces[num_spaces].w = xarray[x1] - xarray[x];
		       spaces[num_spaces].h = yarray[y1] - yarray[y];
		       spaces[num_spaces].p = fitswin;
		       num_spaces++;
		    }
	       }
	  }
/* find the first space that fits */
	k = -1;
	sort = 0x7fffffff;
	a1 = floating[leftover[i]].w * floating[leftover[i]].h;
	k = -1;
	for (j = 0; j < num_spaces; j++)
	  {
	     a2 = spaces[j].w * spaces[j].h;
	     if ((a2 != 0) && ((a1 - a2) < sort) && (spaces[j].p))
	       {
		  k = j;
		  sort = a1 - a2;
	       }
	  }
/* if there's a small space ... */
	if (k >= 0)
	  {
	     sorted[num_sorted].x = spaces[k].x;
	     sorted[num_sorted].y = spaces[k].y;
	     sorted[num_sorted].data = floating[leftover[i]].data;
	     sorted[num_sorted].w = floating[leftover[i]].w;
	     sorted[num_sorted].h = floating[leftover[i]].h;
	     if ((sorted[num_sorted].x + sorted[num_sorted].w) > width)
		sorted[num_sorted].x = width - sorted[num_sorted].w;
	     if ((sorted[num_sorted].y + sorted[num_sorted].h) > height)
		sorted[num_sorted].y = height - sorted[num_sorted].h;
	     if (sorted[num_sorted].x < startx)
		sorted[num_sorted].x = startx;
	     if (sorted[num_sorted].y < starty)
		sorted[num_sorted].y = starty;
	     num_sorted++;
	  }
/* there is no room - put it centered (but dont put top left off screen) */
	else
	  {
	     sorted[num_sorted].data = floating[leftover[i]].data;
	     sorted[num_sorted].x = (width - floating[leftover[i]].w) / 2;
	     sorted[num_sorted].y = (height - floating[leftover[i]].h) / 2;
	     sorted[num_sorted].w = floating[leftover[i]].w;
	     sorted[num_sorted].h = floating[leftover[i]].h;
	     if (sorted[num_sorted].x < startx)
		sorted[num_sorted].x = startx;
	     if (sorted[num_sorted].y < starty)
		sorted[num_sorted].y = starty;
	     num_sorted++;
	  }
     }
/* free up memory */
   Efree(xarray);
   Efree(yarray);
   Efree(filled);
   Efree(spaces);
   if (leftover)
      Efree(leftover);
   for (i = 0; i < num_sorted; i++)
     {
	if ((sorted[i].x + sorted[i].w) > width)
	   sorted[i].x = root.w - sorted[i].w;
	if ((sorted[i].y + sorted[i].h) > height)
	   sorted[i].y = root.h - sorted[i].h;
	if (sorted[i].x < startx)
	   sorted[i].x = startx;
	if (sorted[i].y < starty)
	   sorted[i].y = starty;
     }
   EDBUG_RETURN_;
}

void
SnapEwin(EWin * ewin, int dx, int dy, int *new_dx, int *new_dy)
{
   EWin              **lst, **gwins;
   int                 gnum, num, i, j, screen_snap_dist, odx, ody;
   static char         last_res = 0;

   EDBUG(5, "SnapEwin");
   if (!ewin)
      EDBUG_RETURN_;

   if (!mode.snap)
     {
	*new_dx = dx;
	*new_dy = dy;
	EDBUG_RETURN_;
     }
   screen_snap_dist = mode.constrained ? (root.w + root.h)
      : mode.screen_snap_dist;
   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   gwins = ListWinGroupMembersForEwin(ewin, ACTION_MOVE, mode.nogroup, &gnum);
   if (gwins)
     {
	for (i = 0; i < gnum; i++)
	  {
	     for (j = 0; j < num; j++)
	       {
		  if ((lst[j] == gwins[i]) || (lst[j] == ewin))
		     lst[j] = NULL;
	       }
	  }
	Efree(gwins);
     }
   odx = dx;
   ody = dy;
   if (dx < 0)
     {
	if (IN_BELOW(ewin->x + dx, 0, screen_snap_dist) && (ewin->x >= 0))
	   dx = 0 - ewin->x;
	else if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i])
		    {
		       if (((ewin->desktop == lst[i]->desktop) ||
			    (lst[i]->sticky)) &&
			   (!(lst[i]->floating)) &&
			 (!(lst[i]->iconified)) && (!(lst[i]->ignorearrange)))
			 {
			    if (IN_BELOW
				(ewin->x + dx, lst[i]->x + lst[i]->w - 1,
				 mode.edge_snap_dist)
				&& SPANS_COMMON(ewin->y, ewin->h, lst[i]->y,
						lst[i]->h)
				&& (ewin->x >= (lst[i]->x + lst[i]->w)))
			      {
				 dx = (lst[i]->x + lst[i]->w) - ewin->x;
				 break;
			      }
			 }
		    }
	       }
	  }
	if ((ewin->reqx - ewin->x) > 0)
	   dx = 0;
     }
   else if (dx > 0)
     {
	if (IN_ABOVE(ewin->x + ewin->w + dx, root.w, screen_snap_dist)
	    && ((ewin->x + ewin->w) <= root.w))
	   dx = root.w - (ewin->x + ewin->w);
	else if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i])
		    {
		       if (((ewin->desktop == lst[i]->desktop) ||
			    (lst[i]->sticky)) &&
			   (!(lst[i]->floating)) &&
			 (!(lst[i]->iconified)) && (!(lst[i]->ignorearrange)))
			 {
			    if (IN_ABOVE(ewin->x + ewin->w + dx - 1, lst[i]->x,
					 mode.edge_snap_dist) &&
				SPANS_COMMON(ewin->y, ewin->h, lst[i]->y,
					     lst[i]->h)
				&& ((ewin->x + ewin->w) <= lst[i]->x))
			      {
				 dx = lst[i]->x - (ewin->x + ewin->w);
				 break;
			      }
			 }
		    }
	       }
	  }
	if ((ewin->reqx - ewin->x) < 0)
	   dx = 0;
     }
   if (dy < 0)
     {
	if (IN_BELOW(ewin->y + dy, 0, screen_snap_dist) && (ewin->y >= 0))
	   dy = 0 - ewin->y;
	else if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i])
		    {
		       if (((ewin->desktop == lst[i]->desktop) ||
			    (lst[i]->sticky)) &&
			   (!(lst[i]->floating)) &&
			 (!(lst[i]->iconified)) && (!(lst[i]->ignorearrange)))
			 {
			    if (IN_BELOW
				(ewin->y + dy, lst[i]->y + lst[i]->h - 1,
				 mode.edge_snap_dist)
				&& SPANS_COMMON(ewin->x, ewin->w, lst[i]->x,
						lst[i]->w)
				&& (ewin->y >= (lst[i]->y + lst[i]->h)))
			      {
				 dy = (lst[i]->y + lst[i]->h) - ewin->y;
				 break;
			      }
			 }
		    }
	       }
	  }
	if ((ewin->reqy - ewin->y) > 0)
	   dy = 0;
     }
   else if (dy > 0)
     {
	if (IN_ABOVE(ewin->y + ewin->h + dy, root.h, screen_snap_dist)
	    && ((ewin->y + ewin->h) <= root.h))
	   dy = root.h - (ewin->y + ewin->h);
	else if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i])
		    {
		       if (((ewin->desktop == lst[i]->desktop) ||
			    (lst[i]->sticky)) &&
			   (!(lst[i]->floating)) &&
			 (!(lst[i]->iconified)) && (!(lst[i]->ignorearrange)))
			 {
			    if (IN_ABOVE(ewin->y + ewin->h + dy - 1, lst[i]->y,
					 mode.edge_snap_dist) &&
				SPANS_COMMON(ewin->x, ewin->w, lst[i]->x,
					     lst[i]->w)
				&& ((ewin->y + ewin->h) <= lst[i]->y))
			      {
				 dy = lst[i]->y - (ewin->y + ewin->h);
				 break;
			      }
			 }
		    }
	       }
	  }
	if ((ewin->reqy - ewin->y) < 0)
	   dy = 0;
     }
   if (lst)
      Efree(lst);
   if ((odx != dx) || (ody != dy))
     {
	if (!last_res)
	  {
/*           AUDIO_PLAY("SOUND_MOVE_RESIST"); */
	     last_res = 1;
	  }
     }
   else
      last_res = 0;
   *new_dx = dx;
   *new_dy = dy;
   EDBUG_RETURN_;
}

void
ArrangeEwin(EWin * ewin)
{
   EWin              **lst;
   Button            **blst;
   int                 i, j, num;
   RectBox            *fixed, *ret, newrect;

   fixed = NULL;
   ret = NULL;
   ewin->client.already_placed = 1;
   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if ((lst) && (num > 0))
     {
	fixed = Emalloc(sizeof(RectBox) * num);
	ret = Emalloc(sizeof(RectBox) * (num + 1));
	j = 0;
	for (i = 0; i < num; i++)
	  {
	     if ((lst[i] != ewin) && (!lst[i]->iconified) &&
		 (!lst[i]->ignorearrange) && (lst[i]->layer != 0) &&
		 (((lst
		    [i]->area_x == desks.desk[ewin->desktop].current_area_x)
		   && (lst[i]->area_y ==
		       desks.desk[ewin->desktop].current_area_y)
		   && (lst[i]->desktop == ewin->desktop)) || (lst[i]->sticky)))
	       {
		  fixed[j].data = lst[i];
		  fixed[j].x = (lst[i])->x;
		  fixed[j].y = (lst[i])->y;
		  fixed[j].w = (lst[i])->w;
		  fixed[j].h = (lst[i])->h;
		  if (fixed[j].x < 0)
		    {
		       fixed[j].w += fixed[j].x;
		       fixed[j].x = 0;
		    }
		  if ((fixed[j].x + fixed[j].w) > root.w)
		     fixed[j].w = root.w - fixed[j].x;
		  if (fixed[j].y < 0)
		    {
		       fixed[j].h += fixed[j].y;
		       fixed[j].y = 0;
		    }
		  if ((fixed[j].y + fixed[j].h) > root.h)
		     fixed[j].h = root.h - fixed[j].y;
		  if ((fixed[j].w > 0) && (fixed[j].h > 0))
		    {
		       if (!(lst[i])->never_use_area)
			  fixed[j].p = (lst[i])->layer;
		       else
			  fixed[j].p = 50;
		       j++;
		    }
	       }
	  }
	blst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	if (blst)
	  {
	     fixed = Erealloc(fixed, sizeof(RectBox) * (num + j));
	     ret = Erealloc(ret, sizeof(RectBox) * ((num + j) + 1));
	     for (i = 0; i < num; i++)
	       {
		  if (((blst[i]->desktop == ewin->desktop) ||
		       ((blst[i]->desktop == 0) && (blst[i]->sticky))) &&
		      (blst[i]->visible))
		    {
		       fixed[j].data = NULL;
		       fixed[j].x = blst[i]->x;
		       fixed[j].y = blst[i]->y;
		       fixed[j].w = blst[i]->w;
		       fixed[j].h = blst[i]->h;
		       if (fixed[j].x < 0)
			 {
			    fixed[j].w += fixed[j].x;
			    fixed[j].x = 0;
			 }
		       if ((fixed[j].x + fixed[j].w) > root.w)
			  fixed[j].w = root.w - fixed[j].x;
		       if (fixed[j].y < 0)
			 {
			    fixed[j].h += fixed[j].y;
			    fixed[j].y = 0;
			 }
		       if ((fixed[j].y + fixed[j].h) > root.h)
			  fixed[j].h = root.h - fixed[j].y;
		       if ((fixed[j].w > 0) && (fixed[j].h > 0))
			 {
			    if (blst[i]->sticky)
			       fixed[j].p = 50;
			    else
			       fixed[j].p = 0;
			    j++;
			 }
		    }
	       }
	     Efree(blst);
	  }
	newrect.data = ewin;
	newrect.x = 0;
	newrect.y = 0;
	newrect.w = ewin->w;
	newrect.h = ewin->h;
	newrect.p = ewin->layer;
	if (mode.kde_support)
	  {
	     ArrangeRects(fixed, j, &newrect, 1, ret, mode.kde_x1, mode.kde_y1,
			  mode.kde_x2, mode.kde_y2, ARRANGE_BY_SIZE);
	  }
	else
	  {
	     ArrangeRects(fixed, j, &newrect, 1, ret, 0, 0, root.w, root.h,
			  ARRANGE_BY_SIZE);
	  }
	for (i = 0; i < j + 1; i++)
	  {
	     if (ret[i].data == ewin)
	       {
		  ewin->x = ret[i].x;
		  ewin->y = ret[i].y;
		  i = j + 1;
	       }
	  }
	Efree(lst);
	if (ret)
	   Efree(ret);
	if (fixed)
	   Efree(fixed);
     }
   else
     {
	ewin->x = (root.w - ewin->w) >> 1;
	ewin->y = (root.h - ewin->h) >> 1;
     }
   MoveEwin(ewin, ewin->x, ewin->y);
}
