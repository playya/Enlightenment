#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include "common.h"
#include "scale.h"
#include "image.h"
#include "ximage.h"
#include "context.h"
#include "rend.h"
#include "rgba.h"
#include "color.h"
#include "grab.h"
#include "blend.h"

/* size of the lines per segment we scale / render at a time */
#define LINESIZE 16

/* useful macro */
#define CLIP(x, y, w, h, xx, yy, ww, hh) \
if (x < xx) {w += x; x = xx;} \
if (y < yy) {h += y; y = xx;} \
if ((x + w) > ww) {w = ww - x;} \
if ((y + h) > hh) {h = hh - y;}

void
__imlib_RenderImage(Display *d, ImlibImage *im, 
		    Drawable w, Drawable m, 
		    Visual *v, Colormap cm, int depth, 
		    int sx, int sy, int sw, int sh, 
		    int dx, int dy, int dw, int dh, 
		    char anitalias, char hiq, char blend, char dither_mask,
		    ImlibColorModifier *cmod)
{
   XImage   *xim, *mxim;
   Context *ct;
   DATA32   *buf = NULL, *pointer, *back = NULL;
   int       y, h, hh, jump;
   static GC gc = 0;
   static GC gcm = 0;
   XGCValues gcv;
   DATA32  **ypoints = NULL;
   int      *xpoints = NULL;
   int      *yapoints = NULL;
   int      *xapoints = NULL;
   int       scw, sch;
   int       psx, psy, psw, psh;
   char      xup = 0, yup = 0;
   char      shm = 0;
   
   /* dont do anything if we have a 0 widht or height image to render */
   if ((dw <= 0) || (dh <= 0))
      return;
   /* if the input rect size < 0 dont render either */
   if ((sw <= 0) || (sh <= 0))
      return;
   /* if the output is too big (8k arbitary limit here) dont bother */
   if ((dw > 8192) || (dh > 8192))
      return;
   /* clip the source rect to be within the actual image */
   psx = sx;
   psy = sy;
   psw = sw;
   psh = sh;
   CLIP(sx, sy, sw, sh, 0, 0, im->w, im->h);
   /* clip output coords to clipped input coords */
   if (psx != sx)
      dx = (dx * sx) / psx;
   if (psy != sy)
      dy = (dy * sy) / psy;
   if (psw != sw)
      dw = (dw * sw) / psw;
   if (psh != sh)
      dh = (dh * sh) / psh;
   /* do a second check to see if we now have invalid coords */
   /* dont do anything if we have a 0 widht or height image to render */
   if ((dw <= 0) || (dh <= 0))
      return;
   /* if the input rect size < 0 dont render either */
   if ((sw <= 0) || (sh <= 0))
      return;
   /* if the output is too big (8k arbitary limit here) dont bother */
   if ((dw > 8192) || (dh > 8192))
      return;
   /* calculate the scaling factors of width and height for a whole image */
   scw = dw * im->w / sw;
   sch = dh * im->h / sh;
   /* if we are scaling the image at all make a scaling buffer */
   if (!((sw == dw) && (sh == dh)))
     {
	/* need to calculate ypoitns and xpoints array */
	ypoints = __imlib_CalcYPoints(im->data, im->w, im->h, sch, im->border.top, im->border.bottom);
	if (!ypoints)
	   return;
	xpoints = __imlib_CalcXPoints(im->w, scw, im->border.left, im->border.right);
	if (!xpoints)
	  {
	     free(ypoints);
	     return;
	  }
	/* calculate aliasing counts */
	if (anitalias)
	  {
	     yapoints = __imlib_CalcApoints(im->h, sch, im->border.top, im->border.bottom);
	     if (!yapoints)
	       {
		  free(ypoints);
		  free(xpoints);
		  return;
	       }
	     xapoints = __imlib_CalcApoints(im->w, scw, im->border.left, im->border.right);
	     if (!xapoints)
	       {
		  free(yapoints);
		  free(ypoints);
		  free(xpoints);
		  return;
	       }
	  }
     }
   ct = __imlib_GetContext(d, v, cm, depth);
   __imlib_RGBASetupContext(ct);
   if ((blend) && (IMAGE_HAS_ALPHA(im)))
     {
	back = malloc(dw *dh *sizeof(DATA32));
        if (!__imlib_GrabDrawableToRGBA(back, 0, 0, dw, dh, d, w, 0, v, cm, depth, dx, dy, dw, dh, 0, 1))
	  {
	     free(back);
	     back = NULL;
	  }
     }
   /* get a new XImage - or get one from the cached list */
   xim = __imlib_ProduceXImage(d, v, depth, dw, dh, &shm);
   if (!xim)
     {
	if (anitalias)
	  {
	     free(xapoints);
	     free(yapoints);
	  }
	free(ypoints);
	free(xpoints);
	if (back)
	   free(back);
	return;
     }
   /* do a double check in 24/32bpp */
   if ((xim->bits_per_pixel == 32) && (depth == 24))
      depth = 32;
   if (m)
     {
	mxim = __imlib_ProduceXImage(d, v, 1, dw, dh, &shm);
	if (!mxim)
	  {
	     __imlib_ConsumeXImage(d, xim);
	     if (anitalias)
	       {
		  free(xapoints);
		  free(yapoints);
	       }
	     free(ypoints);
	     free(xpoints);
	     if (back)
		free(back);
	     return;
	  }
     }
   /* if we are scaling the image at all make a scaling buffer */
   if (!((sw == dw) && (sh == dh)))
     {
	/* allocate a buffer to render scaled RGBA data into */
	buf = malloc(dw * LINESIZE * sizeof(int));
	if (!buf)
	  {
	     __imlib_ConsumeXImage(d, xim);
	     if (m)
		__imlib_ConsumeXImage(d, mxim);
	     if (anitalias)
	       {
		  free(xapoints);
		  free(yapoints);
	       }
	     free(ypoints);
	     free(xpoints);
	     if (back)
		free(back);
	     return;
	  }
     }
   /* setup h */
   h = dh;
   /* set our scaling up in x / y dir flags */
   if (dw > sw)
      xup = 1;
   if (dh > sh)
      yup = 1;
   /* scale in LINESIZE Y chunks and convert to depth*/
   for (y = 0; y < dh; y += LINESIZE)
     {
	hh = LINESIZE;
	if (h < LINESIZE)
	   hh = h;
	/* if we're scaling it */
	if (buf)
	  {
	     /* scale the imagedata for this LINESIZE lines chunk of image data */
	     if (anitalias)
	       {
		  if (IMAGE_HAS_ALPHA(im))
		     __imlib_ScaleAARGBA(ypoints, xpoints, buf, xapoints, yapoints, xup, yup, dx, dy + y, 0, 0, dw, hh, dw, im->w);
		  else
		     __imlib_ScaleAARGB(ypoints, xpoints, buf, xapoints, yapoints, xup, yup, dx, dy + y, 0, 0, dw, hh, dw, im->w);
	       }
	     else
		__imlib_ScaleSampleRGBA(ypoints, xpoints, buf, dx, dy + y, 0, 0, dw, hh, dw);
	     jump = 0;
	     pointer = buf;
	  }
	else
	  {
	     jump = im->w - sw;
	     pointer = im->data + ((y + sy) * im->w) + sx;
	  }
	/* if we have a back buffer - we're blending to the bg */
	if (back)
	  {
	     __imlib_BlendRGBAToRGBA(pointer, jump, back + (y * dw), 0, dw, hh); 
	     pointer = back + (y * dw);
	     jump = 0;
	  }
	/* once scaled... convert chunk to bit depth into XImage bufer */
	/* NB - the order here may be random - but I chose it to select most */
	/* common depths first */
	if (depth == 16)
	  {
	     if (hiq)
		__imlib_RGBA_to_RGB565_dither(pointer, jump, 
					      ((DATA16 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA16))),
					      (xim->bytes_per_line / sizeof(DATA16)) - dw,
					      dw, hh, dx, dy + y); 
	     else
		__imlib_RGBA_to_RGB565_fast(pointer, jump, 
					    ((DATA16 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA16))),
					    (xim->bytes_per_line / sizeof(DATA16)) - dw,
					    dw, hh, dx, dy + y); 
	  }
	/* FIXME: need to handle different RGB ordering */
	else if (depth == 24)
	  {
	     __imlib_RGBA_to_RGB888_fast(pointer, jump, 
					 ((DATA8 *)xim->data) + (y * xim->bytes_per_line),
					 xim->bytes_per_line - (dw * 3),
					 dw, hh, dx, dy + y); 
	  }
	else if (depth == 32)
	  {
	     __imlib_RGBA_to_RGB8888_fast(pointer, jump, 
					  ((DATA32 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA32))),
					  (xim->bytes_per_line / sizeof(DATA32)) - dw,
					  dw, hh, dx, dy + y); 
	  }
	else if (depth == 8)
	  {
	     switch (ct->palette_type)
	       {
	       case 0:
		  if (hiq)
		     __imlib_RGBA_to_RGB332_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB332_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 1:
		  if (hiq)
		     __imlib_RGBA_to_RGB232_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB232_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 2:
		  if (hiq)
		     __imlib_RGBA_to_RGB222_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB222_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 3:
		  if (hiq)
		     __imlib_RGBA_to_RGB221_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB221_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 4:
		  if (hiq)
		     __imlib_RGBA_to_RGB121_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB121_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 5:
		  if (hiq)
		     __imlib_RGBA_to_RGB111_dither(pointer, jump, 
						   ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						   (xim->bytes_per_line / sizeof(DATA8)) - dw,
						   dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB111_fast(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  break;
	       case 6:
		  if (hiq)
		     __imlib_RGBA_to_RGB1_dither(pointer, jump, 
						 ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
						 (xim->bytes_per_line / sizeof(DATA8)) - dw,
						 dw, hh, dx, dy + y); 
		  else
		     __imlib_RGBA_to_RGB1_fast(pointer, jump, 
					       ((DATA8 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA8))),
					       (xim->bytes_per_line / sizeof(DATA8)) - dw,
					       dw, hh, dx, dy + y); 
		  break;
	       default:
		  break;
	       }
	  }
	else if (depth == 15)
	  {
	     if (hiq)
		__imlib_RGBA_to_RGB555_dither(pointer, jump, 
					      ((DATA16 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA16))),
					      (xim->bytes_per_line / sizeof(DATA16)) - dw,
					      dw, hh, dx, dy + y); 
	     else
		__imlib_RGBA_to_RGB555_fast(pointer, jump, 
					    ((DATA16 *)xim->data) + (y * (xim->bytes_per_line / sizeof(DATA16))),
					    (xim->bytes_per_line / sizeof(DATA16)) - dw,
					    dw, hh, dx, dy + y); 
	  }
	if (m)
	  {
	     memset(((DATA8 *)mxim->data) + (y * (mxim->bytes_per_line)),
		    0x0, mxim->bytes_per_line * hh);
	     if (dither_mask)
		__imlib_RGBA_to_A1_dither(pointer, jump, 
					  ((DATA8 *)mxim->data) + (y * (mxim->bytes_per_line)),
					  (mxim->bytes_per_line) - (dw >> 3),
					  dw, hh, dx, dy + y); 
	     else
		__imlib_RGBA_to_A1_fast(pointer, jump, 
					((DATA8 *)mxim->data) + (y * (mxim->bytes_per_line)),
					(mxim->bytes_per_line) - (dw >> 3),
					dw, hh, dx, dy + y); 
	  }
	/* FIXME: have to add code to generate mask if asked for */
	h -= LINESIZE;
     }
   /* free up our buffers and poit tables */
   if (buf)
     {
	free(buf);
	free(ypoints);
	free(xpoints);
     }
   if (anitalias)
     {
	free(yapoints);
	free(xapoints);
     }
   if (back)
      free(back);
   /* if we didnt have a gc... create it */
   if (!gc)
     {
	gcv.graphics_exposures = False;
	gc = XCreateGC(d, w, GCGraphicsExposures, &gcv);
     }
   if (m)
     {
	if (!gcm)
	  {
	     gcv.graphics_exposures = False;
	     gcm = XCreateGC(d, m, GCGraphicsExposures, &gcv);
	  }
	/* write the mask */
	if (shm)
	   /* write shm XImage */
	   XShmPutImage(d, m, gcm, mxim, 0, 0, dx, dy, dw, dh, False);
	/* write regular XImage */
	else
	   XPutImage(d, m, gcm, mxim, 0, 0, dx, dy, dw, dh);
     }
   /* write the image */
   if (shm)
      /* write shm XImage */
      XShmPutImage(d, w, gc, xim, 0, 0, dx, dy, dw, dh, False);
   /* write regular XImage */
   else
      XPutImage(d, w, gc, xim, 0, 0, dx, dy, dw, dh);
   /* free the XImage and put onto our free list */
   /* wait for the write to be done */
   if (shm)
      XSync(d, False);
   __imlib_ConsumeXImage(d, xim);
   if (m)
      __imlib_ConsumeXImage(d, mxim);    
}

