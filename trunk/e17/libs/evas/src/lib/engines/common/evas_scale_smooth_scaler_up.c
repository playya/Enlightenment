{
   int         srx = src_region_x, sry = src_region_y;
   int         srw = src_region_w, srh = src_region_h;
   int         drx = dst_region_x, dry = dst_region_y;
   int         drw = dst_region_w, drh = dst_region_h;

   int         dsxx, dsyy, sxx, syy, sx, sy;
   int         cx, cy;
   int         direct_scale = 0, buf_step = 0;

   DATA32      *psrc, *pdst, *pdst_end;
   DATA32      *buf, *pbuf, *pbuf_end;
   RGBA_Image  *im_buf;
   RGBA_Gfx_Func  func;

   /* a scanline buffer */
   pdst = dst_ptr;  // it's been set at (dst_clip_x, dst_clip_y)
   pdst_end = pdst + (dst_clip_h * dst_w);
   if (!dc->mul.use)
     {
	if ((dc->render_op == _EVAS_RENDER_BLEND) && !(src->flags & RGBA_IMAGE_HAS_ALPHA))
	  { direct_scale = 1;  buf_step = dst->image->w; }
	else if (dc->render_op == _EVAS_RENDER_COPY)
	  {
	    direct_scale = 1;  buf_step = dst->image->w;
	    if (src->flags & RGBA_IMAGE_HAS_ALPHA)
		dst->flags |= RGBA_IMAGE_HAS_ALPHA;
	  }
     }
   if (!direct_scale)
     {
	im_buf = evas_common_image_line_buffer_obtain(dst_clip_w);
	if (!im_buf)
	   return;
	buf = im_buf->image->data;
	if (dc->mul.use)
	   func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, dst_clip_w, dc->render_op);
	else
	   func  = evas_common_gfx_func_composite_pixel_span_get(src, dst, dst_clip_w, dc->render_op);
     }
   else
	buf = pdst;

   dsxx = (((srw == 1) ? (1 << 16) : ((srw - 1) << 16))) / ((drw == 1) ? 1 : (drw - 1));
   dsyy = (((srh == 1) ? (1 << 16) : ((srh - 1) << 16))) / ((drh == 1) ? 1 : (drh - 1));

   cx = dst_clip_x - drx;
   cy = dst_clip_y - dry;

   sxx = (dsxx * cx);
   syy = (dsyy * cy);

   sx = sxx >> 16;
   sy = syy >> 16;

   if (drh == srh)
     {
	int  sxx0 = sxx;

	psrc = src->image->data + (src_w * (sry + cy)) + srx;
	while (pdst < pdst_end)
	  {
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    sxx = sxx0;
#ifdef SCALE_USING_MMX
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
#endif
	    while (pbuf < pbuf_end)
	      {
		DATA32   p0, p1;
	        int      ax;

		sx = (sxx >> 16);
		ax = 1 + ((sxx - (sx << 16)) >> 8);
		p0 = p1 = *(psrc + sx);
		if ((sx + 1) < srw)
		    p1 = *(psrc + sx + 1);
#ifdef SCALE_USING_MMX
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p1)
		  {
		    MOV_A2R(ax, mm3)
		    MOV_P2R(p1, mm2, mm0)
		    INTERP_256_R2R(mm3, mm2, mm1, mm5)
		  }
		MOV_R2P(mm1, *pbuf, mm0)
		pbuf++;
#else
		if (p0 | p1)
		    p0 = INTERP_256(ax, p1, p0);
		*pbuf++ = p0;
#endif
		sxx += dsxx;
	      }
	        /* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
		func(buf, NULL, dc->mul.col, pdst, dst_clip_w);
	    pdst += dst_w;
	    psrc += src_w;
	    buf += buf_step;
	  }

	goto done_scale_up;
     }
   else if (drw == srw)
     {
	DATA32  *ps = src->image->data + (src_w * sry) + srx + cx;

	while (pdst < pdst_end)
	  {
	    int        ay;

	    sy = syy >> 16;
	    psrc = ps + (sy * src_w);
	    ay = 1 + ((syy - (sy << 16)) >> 8);
#ifdef SCALE_USING_MMX
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
	    MOV_A2R(ay, mm4)
#endif
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    while (pbuf < pbuf_end)
	      {
		DATA32  p0 = *psrc, p2 = p0;

		if ((sy + 1) < srh)
		   p2 = *(psrc + src_w);
#ifdef SCALE_USING_MMX
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p2)
		  {
		    MOV_P2R(p2, mm2, mm0)
		    INTERP_256_R2R(mm4, mm2, mm1, mm5)
		  }
		MOV_R2P(mm1, *pbuf, mm0)
		pbuf++;
#else
		if (p0 | p2)
		   p0 = INTERP_256(ay, p2, p0);
		*pbuf++ = p0;
#endif
		psrc++;
	      }
		/* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
	       func(buf, NULL, dc->mul.col, pdst, dst_clip_w);
	    pdst += dst_w;
	    syy += dsyy;
	    buf += buf_step;
	  }
	goto done_scale_up;
     }

     {
	DATA32  *ps = src->image->data + (src_w * sry) + srx;
	int     sxx0 = sxx;

	while (pdst < pdst_end)
	  {
	    int   ay;

	    sy = syy >> 16;
	    psrc = ps + (sy * src_w);
	    ay = 1 + ((syy - (sy << 16)) >> 8);
#ifdef SCALE_USING_MMX
	    MOV_A2R(ay, mm4)
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
#endif
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    sxx = sxx0;
	    while (pbuf < pbuf_end)
	      {
		int     ax;
		DATA32  *p, *q;
		DATA32  p0, p1, p2, p3;

		sx = sxx >> 16;
		ax = 1 + ((sxx - (sx << 16)) >> 8);
		p = psrc + sx;  q = p + src_w;
		p0 = p1 = p2 = p3 = *p;
		if ((sx + 1) < srw)
		   p1 = *(p + 1);
		if ((sy + 1) < srh)
		  {
		    p2 = *q;  p3 = p2;
		    if ((sx + 1) < srw)
		       p3 = *(q + 1);
		  }
#ifdef SCALE_USING_MMX
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p1)
		  {
		    MOV_P2R(p1, mm2, mm0)
		    INTERP_256_R2R(mm6, mm2, mm1, mm5)
		  }
		MOV_P2R(p2, mm2, mm0)
		if (p2 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
		INTERP_256_R2R(mm4, mm2, mm1, mm5)
		MOV_R2P(mm1, *pbuf, mm0)
		pbuf++;
#else
		if (p0 | p1)
		   p0 = INTERP_256(ax, p1, p0);
		if (p2 | p3)
		   p2 = INTERP_256(ax, p3, p2);
		if (p0 | p2)
		   p0 = INTERP_256(ay, p2, p0);
		*pbuf++ = p0;
#endif
		sxx += dsxx;
	      }
	     /* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
	       func(buf, NULL, dc->mul.col, pdst, dst_clip_w);
	    pdst += dst_w;
	    syy += dsyy;
	    buf += buf_step;
	  }
     }
   done_scale_up:
   if (!direct_scale)
	evas_common_image_line_buffer_release();
}
