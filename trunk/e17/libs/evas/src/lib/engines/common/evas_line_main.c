#include "evas_common.h"

extern DATA8        _evas_pow_lut[256][256];

void
evas_common_line_init(void)
{
}

#define ABS(_x) (((_x) < 0) ? - (_x) : (_x))
#define SGN(_x) (((_x) < 0) ? -1 : 1)

void
evas_common_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
   int dx, dy, ddx, ddy, ax, ay, sx, sy, x, y, d;
   DATA32 *im;
   DATA32 col;
   DATA32 *ptr;
   int ext_x, ext_y, ext_w, ext_h;
   int im_w, im_h;   
   int bx, by, bw, bh;

   /* FIXME: i didn't impliment bresnham to save cases in the code. should */
   /* impliment using bresnham. */
   
   dx = x2 - x1;
   dy = y2 - y1;
   ax = ABS(dx) << 1;
   ay = ABS(dy) << 1;
   sx = SGN(dx);
   sy = SGN(dy);
   
   if ((dx == 0) && (dy == 0))
     {
	/* point draw */
	return;
     }
   im = dst->image->data;
   im_w = dst->image->w;
   im_h = dst->image->h;   
   col = dc->col.col;

   ext_x = 0; ext_y = 0; ext_w = im_w; ext_h = im_h;
   if (dc->clip.use)
     {
	ext_x = dc->clip.x;
	ext_y = dc->clip.y;
	ext_w = dc->clip.w;
	ext_h = dc->clip.h;
	if (ext_x < 0) 
	  {
	     ext_w += ext_x;
	     ext_x = 0;
	  }
	if (ext_y < 0) 
	  {
	     ext_h += ext_y;
	     ext_y = 0;
	  }
	if ((ext_x + ext_w) > im_w)
	  ext_w = im_w - ext_x;
	if ((ext_y + ext_h) > im_h)
	  ext_h = im_h - ext_y;
     }
   if (ext_w <= 0) return;
   if (ext_h <= 0) return;

   if (x1 < x2) bx = x1;
   else bx = x2;
   if (y1 < y2) by = y1;
   else by = y2;

   ddx = dx; if (ddx < 0) ddx = -ddx;
   ddy = dy; if (ddy < 0) ddy = -ddy;
   
   bw = ddx + 1;
   bh = ddy + 1;
   
   if (!(RECTS_INTERSECT(ext_x, ext_y, ext_w, ext_h, bx, by, bw, bh))) return;

   x = x1;
   y = y1;
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (ax > ay)
	  {
	     d = ay - (ax >> 1);
	     for (;;) 
	       {
		  if ((y >= ext_y) && (y < (ext_y + ext_h)) &&
		      (x >= ext_x) && (x < (ext_x + ext_w)))
		    {
		       ptr = im + (y * im_w) + x;
			 {                                            
			    DATA32 __blend_tmp;                       
			    DATA8  __blend_a;                                  
			    
			    __blend_a = _evas_pow_lut[A_VAL(&(col))][A_VAL(ptr)]; 
			    
			    BLEND_COLOR(__blend_a, R_VAL(ptr), 
					R_VAL(&(col)), R_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(__blend_a, G_VAL(ptr), 
					G_VAL(&(col)), G_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(__blend_a, B_VAL(ptr), 
					B_VAL(&(col)), B_VAL(ptr), 
					__blend_tmp);                 
			    A_VAL(ptr) = A_VAL(ptr) + ((A_VAL(&(col)) * (255 - A_VAL(ptr))) / 255);
			 }
		    }
		  if (x == x2) return;
		  if (d >= 0) 
		    {
		       y += sy;
		       d -= ax;
		    }
		  x += sx;
		  d += ay;
	       }
	  }
	else
	  {
	     d = ax - (ay >> 1);
	     for (;;)
	       {
		  if ((y >= ext_y) && (y < (ext_y + ext_h)) &&
		      (x >= ext_x) && (x < (ext_x + ext_w)))
		    {
		       ptr = im + (y * im_w) + x;
			 {                                            
			    DATA32 __blend_tmp;                       
			    DATA8  __blend_a;                                  
			    
			    __blend_a = _evas_pow_lut[A_VAL(&(col))][A_VAL(ptr)]; 
			    
			    BLEND_COLOR(__blend_a, R_VAL(ptr), 
					R_VAL(&(col)), R_VAL(ptr), 
					__blend_tmp);
			    BLEND_COLOR(__blend_a, G_VAL(ptr), 
					G_VAL(&(col)), G_VAL(ptr), 
					__blend_tmp);
			    BLEND_COLOR(__blend_a, B_VAL(ptr), 
					B_VAL(&(col)), B_VAL(ptr), 
					__blend_tmp);
			    A_VAL(ptr) = A_VAL(ptr) + ((A_VAL(&(col)) * (255 - A_VAL(ptr))) / 255);
			 }
		    }
		  if (y == y2) return;
		  if (d >= 0)
		    {
		       x += sx;
		       d -= ay;
		    }
		  y += sy;
		  d += ax;
	       }
	  }
     }
   else
     {
	if (ax > ay)
	  {
	     d = ay - (ax >> 1);
	     for (;;) 
	       {
		  if ((y >= ext_y) && (y < (ext_y + ext_h)) &&
		      (x >= ext_x) && (x < (ext_x + ext_w)))
		    {
		       ptr = im + (y * im_w) + x;
			 {                                            
			    DATA32 __blend_tmp;                       
			    
			    BLEND_COLOR(A_VAL(&(col)), R_VAL(ptr), 
					R_VAL(&(col)), R_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(A_VAL(&(col)), G_VAL(ptr), 
					G_VAL(&(col)), G_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(A_VAL(&(col)), B_VAL(ptr), 
					B_VAL(&(col)), B_VAL(ptr), 
					__blend_tmp);                 
			 }
		    }
		  if (x == x2) return;
		  if (d >= 0) 
		    {
		       y += sy;
		       d -= ax;
		    }
		  x += sx;
		  d += ay;
	       }
	  }
	else
	  {
	     d = ax - (ay >> 1);
	     for (;;)
	       {
		  if ((y >= ext_y) && (y < (ext_y + ext_h)) &&
		      (x >= ext_x) && (x < (ext_x + ext_w)))
		    {
		       ptr = im + (y * im_w) + x;
			 {                                            
			    DATA32 __blend_tmp;                       
			    
			    BLEND_COLOR(A_VAL(&(col)), R_VAL(ptr), 
					R_VAL(&(col)), R_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(A_VAL(&(col)), G_VAL(ptr), 
					G_VAL(&(col)), G_VAL(ptr), 
					__blend_tmp);                 
			    BLEND_COLOR(A_VAL(&(col)), B_VAL(ptr), 
					B_VAL(&(col)), B_VAL(ptr), 
					__blend_tmp);                 
			 }
		    }
		  if (y == y2) return;
		  if (d >= 0)
		    {
		       x += sx;
		       d -= ay;
		    }
		  y += sy;
		  d += ax;
	       }
	  }
     }
}
