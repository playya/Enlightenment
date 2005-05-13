/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Resist_Rect E_Resist_Rect;

struct _E_Resist_Rect
{
   int x, y, w, h;
   int v1;
   int resist_out;
};

static void _e_resist_rects(Evas_List *rects, int px, int py, int pw, int ph, int x, int y, int w, int h, int *rx, int *ry, int *rw, int *rh);

int
e_resist_container_border_position(E_Container *con, Evas_List *skiplist,
				   int px, int py, int pw, int ph,
				   int x, int y, int w, int h,
				   int *rx, int *ry, int *rw, int *rh)
{
   int resist = 1;
   int desk_resist = 32;
   int win_resist = 12;
   int gad_resist = 32;
   Evas_List *l, *ll, *rects = NULL;
   E_Resist_Rect *r;
   E_Border_List *bl;
   E_Border *bd;

   /* FIXME: get resist values from config */
   if (!resist)
     {
	*rx = x;
	*ry = y;
	*rw = w;
	*rw = h;
	return 0;
     }
   /* edges of screen */
#define OBSTACLE(_x, _y, _w, _h, _resist) \
   { \
      r = E_NEW(E_Resist_Rect, 1); \
      r->x = _x; r->y = _y; r->w = _w; r->h = _h; r->v1 = _resist; \
      r->resist_out = 0; \
      rects = evas_list_append(rects, r); \
   }
#define HOLDER(_x, _y, _w, _h, _resist) \
   { \
      r = E_NEW(E_Resist_Rect, 1); \
      r->x = _x; r->y = _y; r->w = _w; r->h = _h; r->v1 = _resist; \
      r->resist_out = 1; \
      rects = evas_list_append(rects, r); \
   }

   for (l = con->zones; l; l = l->next)
     {
	E_Zone *zone;

	zone = l->data;
	HOLDER(zone->x, zone->y, zone->w, zone->h, desk_resist);
     }
   /* FIXME: need to add resist or complete BLOCKS for things like ibar */
   /* can add code here to add more fake obstacles with custom resist values */
   /* here if need be - ie xinerama middle between screens and panels etc. */

   bl = e_container_border_list_first(con);
   while ((bd = e_container_border_list_next(bl)))
     {
        E_Border *bd;

	if (bd->visible)
	  {
	     int ok;

	     ok = 1;
	     for (ll = skiplist; ll; ll = ll->next)
	       {
		  if (ll->data == bd)
		    {
		       ok = 0;
		       break;
		    }
	       }
	     if (ok)
	       {
		  OBSTACLE(bd->x, bd->y, bd->w, bd->h, win_resist);
	       }
	  }
     }
   e_container_border_list_free(bl);

   for (l = con->gadman->clients; l; l = l->next)
     {
        E_Gadman_Client *gmc;

	gmc = l->data;
	OBSTACLE(gmc->x, gmc->y, gmc->w, gmc->h, gad_resist);
     }
   if (rects)
     {
	_e_resist_rects(rects,
			px, py, pw, ph,
		       	x, y, w, h,
		       	rx, ry, rw, rh);

	for (l = rects; l; l = l->next)
	  {
	     E_FREE(l->data);
	  }
	evas_list_free(rects);
     }
   return 1;
}

int
e_resist_container_gadman_position(E_Container *con, Evas_List *skiplist,
				   int px, int py, int pw, int ph,
				   int x, int y, int w, int h,
				   int *rx, int *ry)
{
   int resist = 1;
   int gad_resist = 32;
   Evas_List *l, *ll, *rects = NULL;
   E_Resist_Rect *r;

   /* FIXME: get resist values from config */
   if (!resist)
     {
	*rx = x;
	*ry = y;
	return 0;
     }

   for (l = con->gadman->clients; l; l = l->next)
     {
        E_Gadman_Client *gmc;
	int ok;

	gmc = l->data;
	ok = 1;
	for (ll = skiplist; ll; ll = ll->next)
	  {
	     if (ll->data == gmc)
	       {
		  ok = 0;
		  break;
	       }
	  }
	if (ok)
	  {
	     r = E_NEW(E_Resist_Rect, 1);

	     r->x = gmc->x;
	     r->y = gmc->y;
	     r->w = gmc->w;
	     r->h = gmc->h;
	     r->v1 = gad_resist;
	     rects = evas_list_append(rects, r);
	  }
     }

   if (rects)
     {
	_e_resist_rects(rects,
			px, py, pw, ph,
		       	x, y, w, h,
		       	rx, ry, NULL, NULL);

	for (l = rects; l; l = l->next)
	  {
	     E_FREE(l->data);
	  }
	evas_list_free(rects);
     }
   return 1;
}

static void
_e_resist_rects(Evas_List *rects,
		int px, int py, int pw, int ph,
	       	int x, int y, int w, int h,
	       	int *rx, int *ry, int *rw, int *rh)
{
   int dx, dy, dw, dh, d, pd;
   int resist_x = 0, resist_y = 0;
   int resist_w = 0, resist_h = 0;
   Evas_List *l;
   E_Resist_Rect *r;

   dx = x - px;
   dy = y - py;
   dw = w - pw;
   dh = h - ph;

   for (l = rects; l; l = l->next)
     {
	r = l->data;
	if (E_SPANS_COMMON(r->y, r->h, y, h))
	  {
	     if (dx > 0)
	       {
		  /* moving right */
		  if (r->resist_out)
		    {
		       /* check right edge of windows against left */
		       d = x + w - (r->x + r->w);
		       pd = px + pw - (r->x + r->w);
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (-resist_x < d)
			      resist_x = -d;
			 }
		    }
		  else
		    {
		       /* check left edge of windows against right */
		       d = r->x - (x + w);
		       pd = r->x - (px + pw);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (resist_x > d)
			      resist_x = d;
			 }
		    }
	       }
	     else if (dx < 0)
	       {
		  /* moving left */
		  if (r->resist_out)
		    {
		       /* check left edge of windows against right */
		       d = r->x - x;
		       pd = r->x - px;
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (resist_x < d)
			      {
				 resist_x = d;
				 resist_w = -d;
			      }
			 }
		    }
		  else
		    {
		       /* check right edge of windows against left */
		       d = x - (r->x + r->w);
		       pd = px - (r->x + r->w);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (-resist_x > d)
			    {
			       resist_x = -d;
			       resist_w = d;
			    }
			 }
		    }
	       }
	     if ((dw > 0) && (dx == 0))
	       {
		  /* enlarging window by moving lower corner */
		  if (r->resist_out)
		    {
		       /* check right edge of windows against left */
		       d = x + w - (r->x + r->w);
		       pd = px + pw - (r->x + r->w);
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (-resist_w < d)
			      resist_w = -d;
			 }
		    }
		  else
		    {
		       /* check left edge of windows against right */
		       d = r->x - (x + w);
		       pd = r->x - (px + pw);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (resist_w > d)
			      resist_w = d;
			 }
		    }
	       }
	  }
	if (E_SPANS_COMMON(r->x, r->w, x, w))
	  {
	     if (dy > 0)
	       {
		  /* moving down */
		  if (r->resist_out)
		    {
		       /* check bottom edge of windows against top */
		       d = y + h - (r->y + r->h);
		       pd = py + ph - (r->y + r->h);
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (-resist_y < d)
			      resist_y = -d;
			 }
		    }
		  else
		    {
		       /* check top edge of windows against bottom */
		       d = r->y - (y + h);
		       pd = r->y - (py + ph);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (resist_y > d)
			      resist_y = d;
			 }
		    }
	       }
	     else if (dy < 0)
	       {
		  /* moving up */
		  if (r->resist_out)
		    {
		       /* check top edge of windows against bottom */
		       d = r->y - y;
		       pd = r->y - py;
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (resist_y < d)
			      {
				 resist_y = d;
				 resist_h = -d;
			      }
			 }
		    }
		  else
		    {
		       /* moving up - check bottom edge of windows against top */
		       d = y - (r->y + r->h);
		       pd = py - (r->y + r->h);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (-resist_y > d)
			      {
				 resist_y = -d;
				 resist_h = d;
			      }
			 }
		    }
	       }
	     if ((dh > 0) && (dy == 0))
	       {
		  /* enlarging window by moving lower corner */
		  if (r->resist_out)
		    {
		       /* check bottom edge of windows against top */
		       d = y + h - (r->y + r->h);
		       pd = py + ph - (r->y + r->h);
		       if ((d > 0) && (pd <= 0) && (d <= r->v1))
			 {
			    if (-resist_h < d)
			      resist_h = -d;
			 }
		    }
		  else
		    {
		       /* check top edge of windows against bottom */
		       d = r->y - (y + h);
		       pd = r->y - (py + ph);
		       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
			 {
			    if (resist_h > d)
			      resist_h = d;
			 }
		    }
	       }
	  }
     }
   if (rx)
     {
	if (dx != 0)
	  *rx = x + resist_x;
	else
	  *rx = x;
     }
   if (ry)
     {
	if (dy != 0)
	  *ry = y + resist_y;
	else
	  *ry = y;
     }
   if (rw)
     {
	if (dw != 0)
	  *rw = w + resist_w;
	else
	  *rw = w;
     }
   if (rh)
     {
	if (dh != 0)
	  *rh = h + resist_h;
	else
	  *rh = h;
     }
}
