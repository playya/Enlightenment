#include "evas_common.h"
#include "evas_engine.h"
#include "evas_macros.h"
#include <xcb/shm.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_atom.h>
#include <pixman.h>
#include <sys/time.h>
#include <sys/utsname.h>


static Evas_List *shmpool = NULL;
static int shmsize = 0;
static int shmmemlimit = 10 * 1024 * 1024;
static int shmcountlimit = 32;

static Xcb_Output_Buffer *
_find_xcbob(xcb_connection_t *conn, int depth, int w, int h, int shm, void *data)
{
   Evas_List         *l;
   Evas_List         *xl;
   Xcb_Output_Buffer *xcbob = NULL;
   int                fitness = 0x7fffffff;
   int                sz;
   int                lbytes;
   int                bpp;

//   return evas_software_xcb_x_output_buffer_new(d, v, depth, w, h, shm, data);
   if (!shm)
     return evas_software_xcb_x_output_buffer_new(conn, depth, w, h, shm, data);
   if (depth > 1)
     {
	bpp = depth / 8;
	if (bpp == 3) bpp = 4;
	lbytes = (((w * bpp) + 3) / 4) * 4;
     }
   else
     lbytes = ((w + 31) / 32) * 4;
   sz = lbytes * h;
   for (l = shmpool; l; l = l->next)
     {
	Xcb_Output_Buffer *xcbob2;
	int                szdif;

	xcbob2 = l->data;
	if ((xcbob2->image->depth != depth) ||
	    (xcbob2->connection != conn))
	  continue;
	szdif = xcbob2->psize - sz;
	if (szdif < 0) continue;
	if (szdif == 0)
	  {
	     xcbob = xcbob2;
	     xl = l;
	     goto have_xcbob;
	  }
	if (szdif < fitness)
	  {
	     fitness = szdif;
	     xcbob = xcbob2;
	     xl = l;
	  }
     }
   if ((fitness > (100 * 100)) || (!xcbob))
     return evas_software_xcb_x_output_buffer_new(conn, depth, w, h, shm, data);

   have_xcbob:
   shmpool = evas_list_remove_list(shmpool, xl);
   xcbob->w = w;
   xcbob->h = h;
   xcbob->bpl = lbytes;
   xcbob->image->width = xcbob->w;
   xcbob->image->height = xcbob->h;
   xcbob->image->stride = xcbob->bpl;
   shmsize -= xcbob->psize * (xcbob->image->depth / 8);
   return xcbob;
}

static void
_unfind_xcbob(Xcb_Output_Buffer *xcbob, int sync)
{
//   evas_software_xcb_x_output_buffer_free(xcbob, sync); return;
   if (xcbob->shm_info)
     {
	shmpool = evas_list_prepend(shmpool, xcbob);
	shmsize += xcbob->psize * xcbob->image->depth / 8;
	while ((shmsize > (shmmemlimit)) ||
	       (evas_list_count(shmpool) > shmcountlimit))
	  {
	     Evas_List *xl;

	     xl = evas_list_last(shmpool);
	     if (!xl)
	       {
		  shmsize = 0;
		  break;
	       }
	     xcbob = xl->data;
	     shmpool = evas_list_remove_list(shmpool, xl);
	     evas_software_xcb_x_output_buffer_free(xcbob, sync);
	  }
     }
   else
     evas_software_xcb_x_output_buffer_free(xcbob, sync);
}

static void
_clear_xcbob(int sync)
{
   while (shmpool)
     {
	Xcb_Output_Buffer *xcbob;

	xcbob = shmpool->data;
	shmpool = evas_list_remove_list(shmpool, shmpool);
	evas_software_xcb_x_output_buffer_free(xcbob, sync);
     }
   shmsize = 0;
}

void
evas_software_xcb_outbuf_init(void)
{
}

void
evas_software_xcb_outbuf_free(Outbuf * buf)
{
   while (buf->priv.pending_writes)
     {
	RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.pending_writes->data;
	buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	obr = im->extended_info;
	evas_cache_image_drop(im);
	if (obr->xcbob) _unfind_xcbob(obr->xcbob, 0);
	if (obr->mxcbob) _unfind_xcbob(obr->mxcbob, 0);
	free(obr);
     }
   evas_software_xcb_outbuf_idle_flush(buf);
   evas_software_xcb_outbuf_flush(buf);
   if (buf->priv.x.gc)
      xcb_free_gc(buf->priv.x.conn, buf->priv.x.gc);
   if (buf->priv.x.gcm)
      xcb_free_gc(buf->priv.x.conn, buf->priv.x.gcm);
   if (buf->priv.pal)
     evas_software_xcb_x_color_deallocate(buf->priv.x.conn,
					  buf->priv.x.cmap,
					  buf->priv.x.vis,
					  buf->priv.pal);
   free(buf);
   _clear_xcbob(0);
}

void
evas_software_xcb_outbuf_rotation_set(Outbuf *buf, int rot)
{
   buf->rot = rot;
}

Outbuf *
evas_software_xcb_outbuf_setup_x(int               w,
				 int               h,
				 int               rot,
				 Outbuf_Depth      depth,
				 xcb_connection_t *conn,
                                 xcb_screen_t     *screen,
				 xcb_drawable_t    draw,
				 xcb_visualtype_t *vis,
				 xcb_colormap_t    cmap,
				 int               x_depth,
				 int               grayscale,
				 int               max_colors,
				 xcb_drawable_t    mask,
				 int               shape_dither,
				 int               destination_alpha)
{
   Outbuf *buf;

   buf = calloc(1, sizeof(Outbuf));
   if (!buf)
      return NULL;

   buf->w = w;
   buf->h = h;
   buf->depth = depth;
   buf->rot = rot;

   buf->priv.x.conn = conn;
   buf->priv.x.screen = screen;
   buf->priv.x.vis = vis;
   buf->priv.x.cmap = cmap;
   buf->priv.x.depth = x_depth;

   buf->priv.mask_dither = shape_dither;
   buf->priv.destination_alpha = destination_alpha;

   {
      Gfx_Func_Convert    conv_func;
      Xcb_Output_Buffer  *xcbob;

      buf->priv.x.shm = evas_software_xcb_x_can_do_shm(buf->priv.x.conn, buf->priv.x.screen);
      xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
						    buf->priv.x.depth,
						    1, 1,
						    buf->priv.x.shm, NULL);

      conv_func = NULL;
      if (xcbob)
	{
#ifdef WORDS_BIGENDIAN
	   if (evas_software_xcb_x_output_buffer_byte_order(xcbob) == XCB_IMAGE_ORDER_LSB_FIRST)
	     buf->priv.x.swap = 1;
	   if (evas_software_xcb_x_output_buffer_bit_order(xcbob) == XCB_IMAGE_ORDER_MSB_FIRST)
	     buf->priv.x.bit_swap = 1;
#else
	   if (evas_software_xcb_x_output_buffer_byte_order(xcbob) == XCB_IMAGE_ORDER_MSB_FIRST)
	     buf->priv.x.swap = 1;
	   if (evas_software_xcb_x_output_buffer_bit_order(xcbob) == XCB_IMAGE_ORDER_MSB_FIRST)
	     buf->priv.x.bit_swap = 1;
#endif
	   if (((vis->_class == XCB_VISUAL_CLASS_TRUE_COLOR) ||
                (vis->_class == XCB_VISUAL_CLASS_DIRECT_COLOR)) &&
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
	   else if ((vis->_class == XCB_VISUAL_CLASS_STATIC_GRAY)  ||
		    (vis->_class == XCB_VISUAL_CLASS_GRAY_SCALE)   ||
		    (vis->_class == XCB_VISUAL_CLASS_STATIC_COLOR) ||
		    (vis->_class == XCB_VISUAL_CLASS_PSEUDO_COLOR) ||
                    (x_depth <= 8))
	     {
		Convert_Pal_Mode pm = PAL_MODE_RGB332;

		if ((vis->_class == XCB_VISUAL_CLASS_GRAY_SCALE) ||
                    (vis->_class == XCB_VISUAL_CLASS_STATIC_GRAY))
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
		buf->priv.pal =
		   evas_software_xcb_x_color_allocate(conn,
						      cmap,
						      vis,
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
		   conv_func = evas_common_convert_func_get(0,
							    buf->w,
							    buf->h,
							    evas_software_xcb_x_output_buffer_depth (xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    buf->priv.pal->colors,
							    buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth (xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    buf->priv.pal->colors,
							    buf->rot);
	     }
	   else
	     {
		if (buf->rot == 0 || buf->rot == 180)
		   conv_func = evas_common_convert_func_get(0,
							    buf->w,
							    buf->h,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    PAL_MODE_NONE,
							    buf->rot);
		else if (buf->rot == 90 || buf->rot == 270)
		   conv_func = evas_common_convert_func_get(0,
							    buf->h,
							    buf->w,
							    evas_software_xcb_x_output_buffer_depth(xcbob),
							    buf->priv.mask.r,
							    buf->priv.mask.g,
							    buf->priv.mask.b,
							    PAL_MODE_NONE,
							    buf->rot);
	     }
	   evas_software_xcb_x_output_buffer_free(xcbob, 1);
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
		       buf->priv.mask.b,
		       buf->priv.pal->colors);
	     }
	}
      evas_software_xcb_outbuf_drawable_set(buf, draw);
      evas_software_xcb_outbuf_mask_set(buf, mask);
   }

   return buf;
}

RGBA_Image *
evas_software_xcb_outbuf_new_region_for_update(Outbuf *buf,
					       int     x,
					       int     y,
					       int     w,
					       int     h,
					       int    *cx,
					       int    *cy,
					       int    *cw,
					       int    *ch)
{
   RGBA_Image    *im;
   Outbuf_Region *obr;
   int            bpl = 0;
   int            use_shm = 1;

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
                  /* we sync */
                  free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
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
	     obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
                                                                buf->priv.x.depth,
                                                                buf->w, buf->h,
                                                                use_shm,
                                                                NULL);
	     im->image->data = (DATA32 *) evas_software_xcb_x_output_buffer_data(obr->xcbob, &bpl);
	     if (buf->priv.x.mask)
	       obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
                                                                   1,
                                                                   buf->w, buf->h,
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
	       obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
                                                                  buf->priv.x.depth,
                                                                  buf->w, buf->h,
                                                                  use_shm,
                                                                  NULL);
	     else if ((buf->rot == 90) || (buf->rot == 270))
	       obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
                                                                  buf->priv.x.depth,
                                                                  buf->h, buf->w,
                                                                  use_shm,
                                                                  NULL);
	     if (buf->priv.x.mask)
	       obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn,
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
//   use_shm = 0; /* 630 -> 1006 fps */
//   if ((w * h) < (200 * 200)) use_shm = 0; /* 630 -> 962 fps */

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
	obr->xcbob = _find_xcbob(buf->priv.x.conn,
                                 buf->priv.x.depth,
                                 w, h,
                                 use_shm,
                                 NULL);
/*	obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn, */
/*							   buf->priv.x.depth, */
/*							   w, */
/*							   h, */
/*							   use_shm, */
/*							   NULL); */
	im->image->data = (DATA32 *)evas_software_xcb_x_output_buffer_data(obr->xcbob, &bpl);
	if (buf->priv.x.mask)
	  obr->mxcbob = _find_xcbob(buf->priv.x.conn,
                                    1, w, h,
                                    use_shm,
                                    NULL);
/*	  obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn, */
/*							      1, */
/*							      w, */
/*							      h, */
/*							      use_shm, */
/*							      NULL); */
     }
   else
     {
        im = evas_cache_image_empty(evas_common_image_cache_get());
	im->image->w = w;
	im->image->h = h;
        evas_common_image_surface_alloc(im->image);
	im->extended_info = obr;
	if ((buf->rot == 0) || (buf->rot == 180))
	  obr->xcbob = _find_xcbob(buf->priv.x.conn,
                                   buf->priv.x.depth,
                                   w, h,
                                   use_shm,
                                   NULL);
/*	   obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn, */
/*							      buf->priv.x.depth, */
/*							      w, */
/*							      h, */
/*							      use_shm, */
/*							      NULL); */
	else if ((buf->rot == 90) || (buf->rot == 270))
	  obr->xcbob = _find_xcbob(buf->priv.x.conn,
                                   buf->priv.x.depth,
                                   h, w,
                                   use_shm,
                                   NULL);
/*	  obr->xcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn, */
/*							     buf->priv.x.depth, */
/*							     h, */
/*							     w, */
/*							     use_shm, */
/*							     NULL); */
	if (buf->priv.x.mask)
	  obr->mxcbob = _find_xcbob(buf->priv.x.conn,
                                    1, w, h,
                                    use_shm,
                                    NULL);
/*	  obr->mxcbob = evas_software_xcb_x_output_buffer_new(buf->priv.x.conn, */
/*							      1, */
/*							      w, */
/*							      h, */
/*							      use_shm, */
/*							      NULL); */
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
evas_software_xcb_outbuf_free_region_for_update(Outbuf     *buf,
						RGBA_Image *update)
{
   /* no need to do anything - they are cleaned up on flush */
}

void
evas_software_xcb_outbuf_flush(Outbuf *buf)
{
   Evas_List           *l;

   if ((buf->priv.onebuf) && (buf->priv.onebuf_regions))
     {
	RGBA_Image *im;
	Outbuf_Region *obr;
	pixman_region16_t tmpr;

	im = buf->priv.onebuf;
	obr = im->extended_info;
        pixman_region_init(&tmpr);
	while (buf->priv.onebuf_regions)
	  {
	     Evas_Rectangle *rect;

	     rect = buf->priv.onebuf_regions->data;
	     buf->priv.onebuf_regions = evas_list_remove_list(buf->priv.onebuf_regions, buf->priv.onebuf_regions);
             pixman_region_union_rect(&tmpr, &tmpr,
                                      rect->x, rect->y,
                                      rect->w, rect->h);
	     if (buf->priv.debug)
	       evas_software_xcb_outbuf_debug_show(buf, buf->priv.x.win,
						   rect->x, rect->y, rect->w, rect->h);
	     free(rect);
	  }
        xcb_set_clip_rectangles(buf->priv.x.conn, XCB_CLIP_ORDERING_YX_BANDED,
                                buf->priv.x.gc,
                                0, 0, pixman_region_n_rects(&tmpr),
                                (const xcb_rectangle_t *)pixman_region_rectangles(&tmpr, NULL));
	evas_software_xcb_x_output_buffer_paste(obr->xcbob, buf->priv.x.win,
						buf->priv.x.gc,
						0, 0, 0);
	if (obr->mxcbob)
	  {
             xcb_set_clip_rectangles(buf->priv.x.conn, XCB_CLIP_ORDERING_YX_BANDED,
                                     buf->priv.x.gcm,
                                     0, 0, pixman_region_n_rects(&tmpr),
                                     (const xcb_rectangle_t *)pixman_region_rectangles(&tmpr, NULL));
	     evas_software_xcb_x_output_buffer_paste(obr->mxcbob,
						     buf->priv.x.mask,
						     buf->priv.x.gcm,
						     0, 0, 0);
	  }
	buf->priv.synced = 0;
     }
   else
     {
#if 1
	/* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
        for (l = buf->priv.pending_writes; l; l = l->next)
          {
             RGBA_Image *im;
             Outbuf_Region      *obr;

             im = l->data;
             obr = im->extended_info;
             if (buf->priv.debug)
               evas_software_xcb_outbuf_debug_show(buf,
                                                   buf->priv.x.win,
                                                   obr->x,
                                                   obr->y,
                                                   obr->w,
                                                   obr->h);
             evas_software_xcb_x_output_buffer_paste(obr->xcbob,
                                                     buf->priv.x.win,
                                                     buf->priv.x.gc,
                                                     obr->x,
                                                     obr->y, 0);
             if (obr->mxcbob)
               evas_software_xcb_x_output_buffer_paste(obr->mxcbob,
                                                       buf->priv.x.mask,
                                                       buf->priv.x.gcm,
                                                       obr->x,
                                                       obr->y, 0);
          }
        while (buf->priv.pending_writes)
          {
             RGBA_Image    *im;
             Outbuf_Region *obr;

             im = buf->priv.pending_writes->data;
             buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes,
                                                              buf->priv.pending_writes);
             obr = im->extended_info;
             evas_cache_image_drop(im);
	     if (obr->xcbob) _unfind_xcbob(obr->xcbob, 0);
	     if (obr->mxcbob) _unfind_xcbob(obr->mxcbob, 0);
/*              if (obr->xcbob) evas_software_xcb_x_output_buffer_free(obr->xcbob, 0); */
/*              if (obr->mxcbob) evas_software_xcb_x_output_buffer_free(obr->mxcbob, 0); */
             free(obr);
          }
	buf->priv.prev_pending_writes = buf->priv.pending_writes;
	buf->priv.pending_writes = NULL;
	xcb_flush(buf->priv.x.conn);
#else
	/* XX async push - disable */
	/*
	for (l = buf->priv.pending_writes; l; l = l->next)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;

	     im = l->data;
	     obr = im->extended_info;
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
	 */
	/* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));

	while (buf->priv.pending_writes)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;

	     im = buf->priv.pending_writes->data;
	     buf->priv.pending_writes = evas_list_remove_list(buf->priv.pending_writes, buf->priv.pending_writes);
	     obr = im->extended_info;
	     evas_cache_image_drop(im);
	     if (obr->xob) _unfind_xob(obr->xob, 0);
	     if (obr->mxob) _unfind_xob(obr->mxob, 0);
/*
	     if (obr->xob) evas_software_x11_x_output_buffer_free(obr->xob, 0);
	     if (obr->mxob) evas_software_x11_x_output_buffer_free(obr->mxob, 0);
 */
	     free(obr);
	  }
#endif
     }
   evas_common_cpu_end_opt();
}

void
evas_software_xcb_outbuf_idle_flush(Outbuf *buf)
{
   if (buf->priv.onebuf)
     {
        RGBA_Image *im;
	Outbuf_Region *obr;

	im = buf->priv.onebuf;
	buf->priv.onebuf = NULL;
	obr = im->extended_info;
	evas_cache_image_drop(im);
	if (obr->xcbob) evas_software_xcb_x_output_buffer_free(obr->xcbob, 0);
	if (obr->mxcbob) evas_software_xcb_x_output_buffer_free(obr->mxcbob, 0);
	free(obr);
     }
   else
     {
	if (buf->priv.prev_pending_writes)
          free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
	while (buf->priv.prev_pending_writes)
	  {
	     RGBA_Image *im;
	     Outbuf_Region *obr;

	     im = buf->priv.prev_pending_writes->data;
	     buf->priv.prev_pending_writes =
	       evas_list_remove_list(buf->priv.prev_pending_writes,
				     buf->priv.prev_pending_writes);
	     obr = im->extended_info;
	     evas_cache_image_drop(im);
	     if (obr->xcbob) _unfind_xcbob(obr->xcbob, 0);
	     if (obr->mxcbob) _unfind_xcbob(obr->mxcbob, 0);
	     free(obr);
	  }
	_clear_xcbob(0);
     }
}

void
evas_software_xcb_outbuf_push_updated_region(Outbuf     *buf,
					     RGBA_Image *update,
					     int         x,
					     int         y,
					     int         w,
					     int         h)
{
   Gfx_Func_Convert conv_func = NULL;
   Outbuf_Region   *obr;
   DATA32          *src_data;
   void            *data;
   int              bpl = 0;
   int              yy;

   obr = update->extended_info;
   if (buf->priv.pal)
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   buf->priv.pal->colors, buf->rot);
     }
   else
     {
	if ((buf->rot == 0) || (buf->rot == 180))
	  conv_func = evas_common_convert_func_get(0, w, h,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
	else if ((buf->rot == 90) || (buf->rot == 270))
	  conv_func = evas_common_convert_func_get(0, h, w,
						   evas_software_xcb_x_output_buffer_depth
						   (obr->xcbob), buf->priv.mask.r,
						   buf->priv.mask.g, buf->priv.mask.b,
						   PAL_MODE_NONE, buf->rot);
     }
   if (!conv_func) return;

   data = evas_software_xcb_x_output_buffer_data(obr->xcbob, &bpl);
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
		    ((evas_software_xcb_x_output_buffer_depth(obr->xcbob) / 8)) - obr->w,
                    obr->w, obr->h, x, y,
		    buf->priv.pal->lookup);
     }
   else
     {
	if (data != src_data)
	  conv_func(src_data, data,
		    0,
		    bpl /
		    ((evas_software_xcb_x_output_buffer_depth(obr->xcbob) / 8)) - obr->w,
                    obr->w, obr->h, x, y, NULL);
     }
#if 1
#else
   /* XX async push */
   if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions)))
     {
	if (buf->priv.debug)
	  evas_software_xcb_outbuf_debug_show(buf, buf->priv.x.win,
					      obr->x, obr->y, obr->w, obr->h);
	evas_software_xcb_x_output_buffer_paste(obr->xcbob, buf->priv.x.win,
						buf->priv.x.gc,
						obr->x, obr->y, 0);
     }
#endif
   if (obr->mxcbob)
     {
	for (yy = 0; yy < obr->h; yy++)
	  evas_software_xcb_x_write_mask_line(buf, obr->mxcbob,
					      src_data +
					      (yy * obr->w), obr->w, yy);
#if 1
#else
	/* XX async push */
	if (!((buf->priv.onebuf) && (buf->priv.onebuf_regions)))
	  evas_software_xcb_x_output_buffer_paste(obr->mxcbob,
						  buf->priv.x.mask,
						  buf->priv.x.gcm,
						  obr->x, obr->y, 0);
#endif
     }
#if 1
#else
   xcb_flush(buf->priv.x.disp);
#endif
}

void
evas_software_xcb_outbuf_reconfigure(Outbuf      *buf,
				     int          w,
				     int          h,
				     int          rot,
				     Outbuf_Depth depth)
{
   if ((w == buf->w) &&
       (h == buf->h) &&
       (rot == buf->rot) &&
       (depth == buf->depth)) return;
   buf->w = w;
   buf->h = h;
   buf->rot = rot;
   evas_software_xcb_outbuf_idle_flush(buf);
}

int
evas_software_xcb_outbuf_get_width(Outbuf * buf)
{
   return buf->w;
}

int
evas_software_xcb_outbuf_get_height(Outbuf * buf)
{
   return buf->h;
}

Outbuf_Depth
evas_software_xcb_outbuf_get_depth(Outbuf * buf)
{
   return buf->depth;
}

int
evas_software_xcb_outbuf_get_rot(Outbuf * buf)
{
   return buf->rot;
}

void
evas_software_xcb_outbuf_drawable_set(Outbuf        *buf,
				      xcb_drawable_t draw)
{
   if (buf->priv.x.win == draw) return;
   if (buf->priv.x.gc)
     {
	xcb_free_gc(buf->priv.x.conn, buf->priv.x.gc);
	buf->priv.x.gc = 0;
     }
   buf->priv.x.win = draw;
   buf->priv.x.gc = xcb_generate_id(buf->priv.x.conn);
   xcb_create_gc(buf->priv.x.conn, buf->priv.x.gc, buf->priv.x.win, 0, NULL);
}

void
evas_software_xcb_outbuf_mask_set(Outbuf        *buf,
				  xcb_drawable_t mask)
{
   if (buf->priv.x.mask == mask) return;
   if (buf->priv.x.gcm)
     {
	xcb_free_gc(buf->priv.x.conn, buf->priv.x.gcm);
	buf->priv.x.gcm = 0;
     }
   buf->priv.x.mask = mask;
   if (buf->priv.x.mask)
      {
	 buf->priv.x.gcm = xcb_generate_id(buf->priv.x.conn);
	 xcb_create_gc(buf->priv.x.conn, buf->priv.x.gcm, buf->priv.x.win, 0, NULL);
      }
}

void
evas_software_xcb_outbuf_debug_set(Outbuf *buf,
				   int     debug)
{
   buf->priv.debug = debug;
}
void
evas_software_xcb_outbuf_debug_show(Outbuf        *buf,
				    xcb_drawable_t draw,
				    int            x,
				    int            y,
				    int            w,
				    int            h)
{
   int                          i;
   xcb_screen_t                *screen = NULL;

   {
      xcb_get_geometry_reply_t *geom;
      xcb_drawable_t            root;
      xcb_screen_iterator_t     i;

      geom = xcb_get_geometry_reply (buf->priv.x.conn, xcb_get_geometry_unchecked(buf->priv.x.conn, draw), 0);
      root = geom->root;
      free (geom);
      geom = xcb_get_geometry_reply (buf->priv.x.conn, xcb_get_geometry_unchecked(buf->priv.x.conn, root), 0);

      i = xcb_setup_roots_iterator((xcb_setup_t *)xcb_get_setup(buf->priv.x.conn));
      for (; i.rem; xcb_screen_next(&i))
	 if (i.data->root == geom->root)
	    {
	       screen = i.data;
	       break;
	    }
      free (geom);
   }
   for (i = 0; i < 20; i++)
     {
/* 	xcb_image_t    *image; */
	xcb_rectangle_t rect = { x, y, w, h};
	uint32_t        mask;
	uint32_t        value[2];

	mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	value[0] = screen->black_pixel;
	value[1] = XCB_EXPOSURES_NOT_ALLOWED; /* no graphics exposures allowed */
	xcb_change_gc(buf->priv.x.conn, buf->priv.x.gc, mask, value);
	xcb_poly_fill_rectangle (buf->priv.x.conn, draw, buf->priv.x.gc, 1, &rect);
        /* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
//	image = xcb_image_get(buf->priv.x.conn, draw, x, y, w, h, XCB_ALL_PLANES, XCB_IMAGE_FORMAT_Z_PIXMAP);
//	if (image)
//	   xcb_image_destroy(image);
        /* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
	mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	value[0] = screen->white_pixel;
	value[1] = XCB_EXPOSURES_NOT_ALLOWED; /* no graphics exposures allowed */
	xcb_change_gc(buf->priv.x.conn, buf->priv.x.gc, mask, value);
	xcb_poly_fill_rectangle (buf->priv.x.conn, draw, buf->priv.x.gc, 1, &rect);
        /* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
//	image = xcb_image_get(buf->priv.x.conn, draw, x, y, w, h, XCB_ALL_PLANES, XCB_IMAGE_FORMAT_Z_PIXMAP);
//	if (image)
//	   xcb_image_destroy(image);
        /* we sync */
        free(xcb_get_input_focus_reply(buf->priv.x.conn, xcb_get_input_focus_unchecked(buf->priv.x.conn), NULL));
     }
}
