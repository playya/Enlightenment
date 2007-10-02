#include "evas_common.h"
#include "evas_engine.h"
#include "evas_macros.h"
#include <sys/time.h>
#include <sys/utsname.h>

void
evas_software_x11_outbuf_init(void)
{
}

void
evas_software_x11_outbuf_free(Outbuf * buf)
{
   evas_software_x11_outbuf_idle_flush(buf);
   evas_software_x11_outbuf_flush(buf);
   if (buf->priv.x.gc)
      XFreeGC(buf->priv.x.disp, buf->priv.x.gc);
   if (buf->priv.x.gcm)
      XFreeGC(buf->priv.x.disp, buf->priv.x.gcm);
   if (buf->priv.pal)
      evas_software_x11_x_color_deallocate(buf->priv.x.disp, buf->priv.x.cmap,
					   buf->priv.x.vis, buf->priv.pal);
   free(buf);
}

void
evas_software_x11_outbuf_rotation_set(Outbuf *buf, int rot)
{
   buf->rot = rot;
}

Outbuf             *
evas_software_x11_outbuf_setup_x(int w, int h, int rot, Outbuf_Depth depth,
				 Display * disp, Drawable draw, Visual * vis,
				 Colormap cmap, int x_depth,
				 int grayscale, int max_colors, Pixmap mask,
				 int shape_dither, int destination_alpha)
{
   Outbuf             *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->w = w;
   buf->h = h;
   buf->depth = depth;
   buf->rot = rot;

   buf->priv.x.disp = disp;
   buf->priv.x.vis = vis;
   buf->priv.x.cmap = cmap;
   buf->priv.x.depth = x_depth;

   buf->priv.mask_dither = shape_dither;
   buf->priv.destination_alpha = destination_alpha;

   {
      Gfx_Func_Convert    conv_func;
      X_Output_Buffer    *xob;

      buf->priv.x.shm = evas_software_x11_x_can_do_shm(buf->priv.x.disp);
      xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
						  buf->priv.x.vis,
						  buf->priv.x.depth,
						  1, 1, buf->priv.x.shm, NULL);

      conv_func = NULL;
      if (xob)
	{
#ifdef WORDS_BIGENDIAN
	   if (evas_software_x11_x_output_buffer_byte_order(xob) == LSBFirst)
	     buf->priv.x.swap = 1;
	   if (evas_software_x11_x_output_buffer_bit_order(xob) == MSBFirst)
	     buf->priv.x.bit_swap = 1;
#else
	   if (evas_software_x11_x_output_buffer_byte_order(xob) == MSBFirst)
	     buf->priv.x.swap = 1;
	   if (evas_software_x11_x_output_buffer_bit_order(xob) == MSBFirst)
	     buf->priv.x.bit_swap = 1;
#endif
	   if (((vis->class == TrueColor) || (vis->class == DirectColor)) &&
	       (x_depth > 8))
	     {
		buf->priv.mask.r = (DATA32) vis->red_mask;
		buf->priv.mask.g = (DATA32) vis->green_mask;
		buf->priv.mask.b = (DATA32) vis->blue_mask;
		if (buf->priv.x.swap)
		  {
		     SWAP32(buf->priv.mask.r);
		     SWAP32(buf->priv.mask.g);
		     SWAP32(buf->priv.mask.b);
		  }
	     }
	   else if ((vis->class == PseudoColor) ||
		    (vis->class == StaticColor) ||
		    (vis->class == GrayScale) ||
		    (vis->class == StaticGray) ||
		    (x_depth <= 8))
	     {
		Convert_Pal_Mode    pm = PAL_MODE_RGB332;

		if ((vis->class == GrayScale) || (vis->class == StaticGray))
		   grayscale = 1;
		if (grayscale)
		  {
		     if (max_colors >= 256)
			pm = PAL_MODE_GRAY256;
		     else if (max_colors >= 64)
			pm = PAL_MODE_GRAY64;
		     else if (max_colors >= 16)
			pm = PAL_MODE_GRAY16;
		     else if (max_colors >= 4)
			pm = PAL_MODE_GRAY4;
		     else
			pm = PAL_MODE_MONO;
		  }
		else
		  {
		     if (max_colors >= 256)
			pm = PAL_MODE_RGB332;
		     else if (max_colors >= 216)
			pm = PAL_MODE_RGB666;
		     else if (max_colors >= 128)
			pm = PAL_MODE_RGB232;
		     else if (max_colors >= 64)
			pm = PAL_MODE_RGB222;
		     else if (max_colors >= 32)
			pm = PAL_MODE_RGB221;
		     else if (max_colors >= 16)
			pm = PAL_MODE_RGB121;
		     else if (max_colors >= 8)
			pm = PAL_MODE_RGB111;
		     else if (max_colors >= 4)
			pm = PAL_MODE_GRAY4;
		     else
			pm = PAL_MODE_MONO;
		  }
		/* FIXME: only alloc once per display+cmap */
		buf->priv.pal = evas_software_x11_x_color_allocate(disp, cmap, vis,
								   PAL_MODE_RGB666);
		if (!buf->priv.pal)
		  {
		     free(buf);
		     return NULL;
		  }
	     }
	   if (buf->priv.pal)
	     {
		if (buf->rot == 0 || buf->rot == 180)
		  conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
							   evas_software_x11_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b,
							   buf->priv.pal->colors,
							   buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		  conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
							   evas_software_x11_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b,
							   buf->priv.pal->colors,
							   buf->rot);
	     }
	   else
	     {
		if (buf->rot == 0 || buf->rot == 180)
		  conv_func = evas_common_convert_func_get(0, buf->w, buf->h,
							   evas_software_x11_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
						buf->priv.mask.b, PAL_MODE_NONE,
							   buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		  conv_func = evas_common_convert_func_get(0, buf->h, buf->w,
							   evas_software_x11_x_output_buffer_depth
							   (xob), buf->priv.mask.r,
							   buf->priv.mask.g,
							   buf->priv.mask.b, PAL_MODE_NONE,
							   buf->rot);
	     }
	   evas_software_x11_x_output_buffer_free(xob, 1);
	   if (!conv_func)
	     {
		printf(".[ Evas Error ].\n"
		       " {\n"
		       "  At depth         %i:\n"
		       "  RGB format mask: %08x, %08x, %08x\n"
		       "  Palette mode:    %i\n"
		       "  Not supported by and compiled in converters!\n"
		       " }\n",
		       buf->priv.x.depth,
		       buf->priv.mask.r,
		       buf->priv.mask.g,
		       buf->priv.mask.b, buf->priv.pal->colors);
	     }
	}
      evas_software_x11_outbuf_drawable_set(buf, draw);
      evas_software_x11_outbuf_mask_set(buf, mask);
   }
   return buf;
}

RGBA_Image         *
evas_software_x11_outbuf_new_region_for_update(Outbuf *buf, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   RGBA_Image         *im;
   Outbuf_Region      *obr;
   int                 bpl = 0;
   int                 use_shm = 1;

   if ((buf->onebuf) && (buf->priv.x.shm))
     {
	Evas_Rectangle *rect;
	
	rect = malloc(sizeof(Evas_Rectangle));
	RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, buf->w, buf->h);
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
	buf->priv.onebuf_regions = evas_list_append(buf->priv.onebuf_regions, rect);
	if (buf->priv.onebuf)
	  {
	     *cx = x;
	     *cy = y;
	     *cw = w;
	     *ch = h;
	     if (!buf->priv.synced)
	       {
		  XSync(buf->priv.x.disp, False);
		  buf->priv.synced = 1;
	       }
	     if ((buf->priv.x.mask) || (buf->priv.destination_alpha))
	       {
		  int yy;
		  
		  im = buf->priv.onebuf;
		  for (yy = y; yy < (y + h); yy++)
		    {
		       memset(im->image->data + (im->image->w * yy) + x,
			      0, w * sizeof(DATA32));
		    }
	       }
	     return buf->priv.onebuf;
	  }
	obr = calloc(1, sizeof(Outbuf_Region));
	obr->x = 0;
	obr->y = 0;
	obr->w = buf->w;
	obr->h = buf->h;
	*cx = x;
	*cy = y;
	*cw = w;
	*ch = h;
	
	use_shm = buf->priv.x.shm;
	if ((buf->rot == 0) &&
	    (buf->priv.mask.r == 0xff0000) &&
	    (buf->priv.mask.g == 0x00ff00) &&
	    (buf->priv.mask.b == 0x0000ff))
	  {
	     im = evas_cache_image_empty(evas_common_image_cache_get());
	     im->image->w = buf->w;
	     im->image->h = buf->h;
	     im->image->data = NULL;
	     im->image->no_free = 1;
	     im->extended_info = obr;
	     obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							      buf->priv.x.vis,
							      buf->priv.x.depth,
							      buf->w, buf->h,
							      use_shm,
							      NULL);
	     im->image->data = (DATA32 *) evas_software_x11_x_output_buffer_data(obr->xob, &bpl);
	     if (buf->priv.x.mask)
	       obr->mxob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
								 buf->priv.x.vis,
								 1, buf->w, buf->h,
								 use_shm,
								 NULL);
	  }
	else
	  {
	     im = evas_cache_image_empty(evas_common_image_cache_get());
	     im->image->w = buf->w;
	     im->image->h = buf->h;
	     evas_common_image_surface_alloc(im->image);
	     im->extended_info = obr;
	     if ((buf->rot == 0) || (buf->rot == 180))
	       obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
								buf->priv.x.vis,
								buf->priv.x.depth,
								buf->w, buf->h,
								use_shm,
								NULL);
	     else if ((buf->rot == 90) || (buf->rot == 270))
	       obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
								buf->priv.x.vis,
								buf->priv.x.depth,
								buf->h, buf->w,
								use_shm,
								NULL);
	     if (buf->priv.x.mask)
	       obr->mxob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
								 buf->priv.x.vis,
								 1, buf->w, buf->h,
								 use_shm,
								 NULL);
	  }
	if ((buf->priv.x.mask) || (buf->priv.destination_alpha))
	  {
	     im->flags |= RGBA_IMAGE_HAS_ALPHA;
	     /* FIXME: faster memset! */
	     memset(im->image->data, 0, w * h * sizeof(DATA32));
	  }
	buf->priv.onebuf = im;
	return im;
     }
   
   
   
   
   
   
   
   obr = calloc(1, sizeof(Outbuf_Region));
   obr->x = x;
   obr->y = y;
   obr->w = w;
   obr->h = h;
   *cx = 0;
   *cy = 0;
   *cw = w;
   *ch = h;

   use_shm = buf->priv.x.shm;
   /* FIXME: magic - i found if shm regions are smaller than 200x200 its
    * faster to use ximages over unix sockets - trial and error
    */
   if ((w * h) < (200 * 200)) use_shm = 0;

   if ((buf->rot == 0) &&
       (buf->priv.mask.r == 0xff0000) &&
       (buf->priv.mask.g == 0x00ff00) &&
       (buf->priv.mask.b == 0x0000ff))
     {
	im = evas_cache_image_empty(evas_common_image_cache_get());
	im->image->w = w;
	im->image->h = h;
	im->image->data = NULL;
	im->image->no_free = 1;
	im->extended_info = obr;
	obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							 buf->priv.x.vis,
							 buf->priv.x.depth,
							 w, h,
							 use_shm,
							 NULL);
	im->image->data = (DATA32 *) evas_software_x11_x_output_buffer_data(obr->xob, &bpl);
	if (buf->priv.x.mask)
	  obr->mxob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							    buf->priv.x.vis,
							    1, w, h,
							    use_shm,
							    NULL);
     }
   else
     {
	im = evas_cache_image_empty(evas_common_image_cache_get());
        im->image->w = w;
        im->image->h = h;
        evas_common_image_surface_alloc(im->image);
	im->extended_info = obr;
	if ((buf->rot == 0) || (buf->rot == 180))
	  obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							   buf->priv.x.vis,
							   buf->priv.x.depth,
							   w, h,
							   use_shm,
							   NULL);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  obr->xob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							   buf->priv.x.vis,
							   buf->priv.x.depth,
							   h, w,
							   use_shm,
							   NULL);
	if (buf->priv.x.mask)
	  obr->mxob = evas_software_x11_x_output_buffer_new(buf->priv.x.disp,
							    buf->priv.x.vis,
							    1, w, h,
							    use_shm,
							    NULL);
     }
   if ((buf->priv.x.mask) || (buf->priv.destination_alpha))
     {
	im->flags |= RGBA_IMAGE_HAS_ALPHA;
	/* FIXME: faster memset! */
	memset(im->image->data, 0, w * h * sizeof(DATA32));
     }
   buf->priv.pending_writes = evas_list_append(buf->priv.pending_writes, im);
   return im;
}

void
evas_software_x11_outbuf_free_region_for_update(Outbuf * buf, RGBA_Image * update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_x11_outbuf_flush(Outbuf *buf)
{
   Evas_List *l;

   if ((buf->priv.onebuf) && (buf->priv.onebuf_regions))
     {
	RGBA_Image *im;
	Outbuf_Region *obr;
	Region tmpr;
	
	im = buf->priv.onebuf;
	obr = im->extended_info;
	tmpr = XCreateRegion();
	while (buf->priv.onebuf_regions)
	  {
	     Evas_Rectangle *rect;
	     XRectangle xr;
	     
	     rect = buf->priv.onebuf_regions->data;
	     buf->priv.onebuf_regions = evas_list_remove_list(buf->priv.onebuf_regions, buf->priv.onebuf_regions);
	     xr.x = rect->x;
	     xr.y = rect->y;
	     xr.width = rect->w;
	     xr.height = rect->h;
	     free(rect);
	     XUnionRectWithRegion(&xr, tmpr, tmpr);
	     if (buf->priv.debug)
	       evas_software_x11_outbuf_debug_show(buf, buf->priv.x.win,
						   rect->x, rect->y, rect->w, rect->h);
	  }
	XSetRegion(buf->priv.x.disp, buf->priv.x.gc, tmpr);
	evas_software_x11_x_output_buffer_paste(obr->xob, buf->priv.x.win,
						buf->priv.x.gc,
						0, 0, 0);
	if (obr->mxob)
	  {
	     XSetRegion(buf->priv.x.disp, buf->priv.x.gcm, tmpr);
	     evas_software_x11_x_output_buffer_paste(obr->mxob,
						     buf->priv.x.mask,
						     buf->priv.x.gcm,
						     0, 0, 0);
	  }
	XDestroyRegion(tmpr);
	buf->priv.synced = 0;
     }
   else
     {
	for (l = buf->priv.pending_writes; l; l = l->next)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;
	     
	     im = l->data;
	     obr = im->extended_info;
	     /* paste now */
	     if (buf->priv.debug)
	       evas_software_x11_outbuf_debug_show(buf, buf->priv.x.win,
						   obr->x, obr->y, obr->w, obr->h);
	     evas_software_x11_x_output_buffer_paste(obr->xob, buf->priv.x.win,
						     buf->priv.x.gc,
						     obr->x, obr->y, 0);
	     if (obr->mxob)
	       evas_software_x11_x_output_buffer_paste(obr->mxob,
						       buf->priv.x.mask,
						       buf->priv.x.gcm,
						       obr->x, obr->y, 0);
	  }
	XSync(buf->priv.x.disp, False);
	while (buf->priv.pending_writes)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;
	     
	     im = buf->priv.pending_writes->data;
	     buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	     obr = im->extended_info;
	     evas_cache_image_drop(im);
	     if (obr->xob) evas_software_x11_x_output_buffer_free(obr->xob, 0);
	     if (obr->mxob) evas_software_x11_x_output_buffer_free(obr->mxob, 0);
	     free(obr);
	  }
     }
   evas_common_cpu_end_opt();
}

void
evas_software_x11_outbuf_idle_flush(Outbuf *buf)
{
   if (buf->priv.onebuf)
     {
        RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.onebuf;
	buf->priv.onebuf = NULL;
	obr = im->extended_info;
	evas_cache_image_drop(im);
	if (obr->xob) evas_software_x11_x_output_buffer_free(obr->xob, 0);
	if (obr->mxob) evas_software_x11_x_output_buffer_free(obr->mxob, 0);
	free(obr);
     }
}

void
evas_software_x11_outbuf_push_updated_region(Outbuf * buf, RGBA_Image * update, int x, int y, int w, int h)
{
   Gfx_Func_Convert    conv_func = NULL;
   Outbuf_Region      *obr;
   DATA32             *src_data;
   void               *data;
   int                 bpl = 0, yy;

   obr = update->extended_info;
   if (buf->priv.pal)
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_x11_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_x11_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_x11_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_x11_x_output_buffer_depth
						   (obr->xob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
     }
   if (!conv_func) return;

   data = evas_software_x11_x_output_buffer_data(obr->xob, &bpl);
   src_data = update->image->data;
   if (buf->rot == 0)
     {
	obr->x = x;
	obr->y = y;
     }
   else if (buf->rot == 90)
     {
	obr->x = y;
	obr->y = buf->w - x - w;
     }
   else if (buf->rot == 180)
     {
	obr->x = buf->w - x - w;
	obr->y = buf->h - y - h;
     }
   else if (buf->rot == 270)
     {
	obr->x = buf->h - y - h;
	obr->y = x;
     }
   if ((buf->rot == 0) || (buf->rot == 180))
     {
	obr->w = w;
	obr->h = h;
     }
   else if ((buf->rot == 90) || (buf->rot == 270))
     {
	obr->w = h;
	obr->h = w;
     }
   if (buf->priv.pal)
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_x11_x_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y,
		    buf->priv.pal->lookup);
     }
   else
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_x11_x_output_buffer_depth(obr->xob) /
		      8)) - obr->w, obr->w, obr->h, x, y, NULL);
#if 0
	/* FIXME: this is evil - but it makes ARGB targets look correct */
	if ((buf->priv.destination_alpha) && (!obr->mxob) &&
	    (evas_software_x11_x_output_buffer_depth(obr->xob) == 32))
	  {
	     int i;
	     DATA32 a;
	     DATA32 *s, *e;
	     
	     for (i = 0; i < obr->h; i++)
	       {
		  s = ((DATA32 *)data) + ((bpl * i) / sizeof(DATA32));
		  e = s + obr->w;
		  while (s < e)
		    {
		       a = A_VAL(s) + 1;
		       R_VAL(s) = (R_VAL(s) * a) >> 8;
		       G_VAL(s) = (G_VAL(s) * a) >> 8;
		       B_VAL(s) = (B_VAL(s) * a) >> 8;
		       s++;
		    }
	       }
	  }
#endif
     }
   if (obr->mxob)
     {
	for (yy = 0; yy < obr->h; yy++)
	  evas_software_x11_x_write_mask_line(buf, obr->mxob,
					      src_data +
					      (yy * obr->w), obr->w, yy);
     }
}

void
evas_software_x11_outbuf_reconfigure(Outbuf * buf, int w, int h, int rot,
				     Outbuf_Depth depth)
{
   if ((w == buf->w) &&
       (h == buf->h) &&
       (rot == buf->rot) &&
       (depth == buf->depth)) return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   evas_software_x11_outbuf_idle_flush(buf);
}

int
evas_software_x11_outbuf_get_width(Outbuf * buf)
{
   return buf->w;
}

int
evas_software_x11_outbuf_get_height(Outbuf * buf)
{
   return buf->h;
}

Outbuf_Depth
evas_software_x11_outbuf_get_depth(Outbuf * buf)
{
   return buf->depth;
}

int
evas_software_x11_outbuf_get_rot(Outbuf * buf)
{
   return buf->rot;
}

void
evas_software_x11_outbuf_drawable_set(Outbuf * buf, Drawable draw)
{
   XGCValues           gcv;

   if (buf->priv.x.win == draw) return;
   if (buf->priv.x.gc)
     {
	XFreeGC(buf->priv.x.disp, buf->priv.x.gc);
	buf->priv.x.gc = NULL;
     }
   buf->priv.x.win = draw;
   buf->priv.x.gc = XCreateGC(buf->priv.x.disp, buf->priv.x.win, 0, &gcv);
}

void
evas_software_x11_outbuf_mask_set(Outbuf * buf, Pixmap mask)
{
   XGCValues           gcv;

   if (buf->priv.x.mask == mask) return;
   if (buf->priv.x.gcm)
     {
	XFreeGC(buf->priv.x.disp, buf->priv.x.gcm);
	buf->priv.x.gcm = NULL;
     }
   buf->priv.x.mask = mask;
   if (buf->priv.x.mask)
     buf->priv.x.gcm = XCreateGC(buf->priv.x.disp, buf->priv.x.mask, 0, &gcv);
}

void
evas_software_x11_outbuf_debug_set(Outbuf * buf, int debug)
{
   buf->priv.debug = debug;
}

void
evas_software_x11_outbuf_debug_show(Outbuf * buf, Drawable draw, int x, int y, int w,
			       int h)
{
   int                 i;
   int                 screen_num = 0;

     {
	int                 wx, wy;
	unsigned int        ww, wh, bd, dp;
	Window              wdum, root;
	XWindowAttributes   wattr;

	XGetGeometry(buf->priv.x.disp, draw, &root, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetGeometry(buf->priv.x.disp, root, &wdum, &wx, &wy, &ww, &wh, &bd, &dp);
	XGetWindowAttributes(buf->priv.x.disp, root, &wattr);
	screen_num = XScreenNumberOfScreen(wattr.screen);
     }
   for (i = 0; i < 20; i++)
     {
	XImage             *xim;

	XSetForeground(buf->priv.x.disp, buf->priv.x.gc,
		       BlackPixel(buf->priv.x.disp, screen_num));
	XFillRectangle(buf->priv.x.disp, draw, buf->priv.x.gc, x, y, w, h);
	XSync(buf->priv.x.disp, False);
//	xim =
//	  XGetImage(buf->priv.x.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
//	if (xim)
//	  XDestroyImage(xim);
	XSync(buf->priv.x.disp, False);
	XSetForeground(buf->priv.x.disp, buf->priv.x.gc,
		       WhitePixel(buf->priv.x.disp, screen_num));
	XFillRectangle(buf->priv.x.disp, draw, buf->priv.x.gc, x, y, w, h);
	XSync(buf->priv.x.disp, False);
//	xim =
//	  XGetImage(buf->priv.x.disp, draw, x, y, w, h, 0xffffffff, ZPixmap);
//	if (xim)
//	  XDestroyImage(xim);
	XSync(buf->priv.x.disp, False);
     }
}
