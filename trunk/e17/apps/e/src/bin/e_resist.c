#include "e.h"

typedef struct _E_Resist_Rect E_Resist_Rect;

struct _E_Resist_Rect
{
   int x, y, w, h;
   int v1, v2, v3, v4;
};

int
e_resist_container_position(E_Container *con, Evas_List *skiplist,
			    int px, int py, int pw, int ph,
			    int x, int y, int w, int h,
			    int *rx, int *ry)
{
   int resist = 1;
   int desk_resist = 32;
   int win_resist = 12;
   int dx, dy, d, pd;
   int resist_x = 0, resist_y = 0;
   Evas_List *l, *ll, *rects = NULL;
   E_Resist_Rect *r;

   /* FIXME: get resist values from config */
   
   if (!resist)
     {
	*rx = x;
	*ry = y;
	return 0;
     }
   dx = x - px;
   dy = y - py;
     /* edges of screen */
#define OBSTACLE(_x, _y, _w, _h, _resist) \
   { \
      r = E_NEW(E_Resist_Rect, 1); \
      r->x = _x; r->y = _y; r->w = _w; r->h = _h; r->v1 = _resist; \
      rects = evas_list_append(rects, r); \
   }
   OBSTACLE(-1000000, -1000000, 2000000 + con->w, 1000000,
	    desk_resist);
   OBSTACLE(-1000000, -1000000, 1000000, 2000000 + con->h,
	    desk_resist);
   OBSTACLE(-1000000, con->h, 2000000 + con->w, 1000000,
	    desk_resist);
   OBSTACLE(con->w, -1000000, 1000000, 2000000 + con->h,
	    desk_resist);
   /* FIXME: need to add resist or complete BLOCKS for things like ibar */
   /* can add code here to add more fake obstacles with custom resist values */
   /* here if need be - ie xinerama middle between screens and panels etc. */
   
   for (l = con->clients; l; l = l->next)
     {
        E_Border           *bd;
	
	bd = l->data;
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
		  r = E_NEW(E_Resist_Rect, 1);
		  
		  r->x = bd->x;
		  r->y = bd->y;
		  r->w = bd->w;
		  r->h = bd->h;
		  r->v1 = win_resist;
		  rects = evas_list_append(rects, r);
	       }
	  }
     }
   
   for (l = rects; l; l = l->next)
     {
	r = l->data;
	if (E_SPANS_COMMON(r->y, r->h, y, h))
	  {
	     if (dx > 0)
	       {
		  /* moving right - check left edge of windows against right */
		  d = r->x - (x + w);
		  pd = r->x - (px + pw);
		  if ((d < 0) && (pd >= 0) && (d >= -r->v1))
		    {
		       if (resist_x > d)
			 resist_x = d;
		    }
	       }
	     else if (dx < 0)
	       {
		  /* moving left - check right edge of windows against left */
		  d = x - (r->x + r->w);
		  pd = px - (r->x + r->w);
		  if ((d < 0) && (pd >= 0) && (d >= -r->v1))
		    {
		       if (-resist_x > d)
			 resist_x = -d;
		    }
	       }
	  }
	if (E_SPANS_COMMON(r->x, r->w, x, w))
	  {
	     if (dy > 0)
	       {
		  /* moving down - check top edge of windows against bottom */
		  d = r->y - (y + h);
		  pd = r->y - (py + ph);
		  if ((d < 0) && (pd >= 0) && (d >= -r->v1))
		    {
		       if (resist_y > d)
			 resist_y = d;
		    }
	       }
	     else if (dy < 0)
	       {
		  /* moving up - check bottom edge of windows against top */
		  d = y - (r->y + r->h);
		  pd = py - (r->y + r->h);
		  if ((d < 0) && (pd >= 0) && (d >= -r->v1))
		    {
		       if (-resist_y > d)
			 resist_y = -d;
		    }
	       }
	  }
     }
   if (rects)
     {
	for (l = rects; l; l = l->next)
	  {
	     E_FREE(l->data);
	  }
	evas_list_free(rects);
     }
   if (dx != 0)
     *rx = x + resist_x;
   else
     *rx = x;
   if (dy != 0)
     *ry = y + resist_y;
   else
     *ry = y;
   return 1;
}
