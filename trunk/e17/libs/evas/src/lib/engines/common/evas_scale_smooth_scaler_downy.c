{
   int Cy, j;
   DATA32 *dptr, *pix, *pbuf, **yp;
   int r, g, b, a, rr, gg, bb, aa;
   int *xp, xap, yap, pos;
   int dyy, dxx;
   int w = dst_clip_w;

   dptr = dst_ptr;
   pos = (src_region_y * src_w) + src_region_x;
   dyy = dst_clip_y - dst_region_y;
   dxx = dst_clip_x - dst_region_x;

   xp = xpoints + dxx;
   yp = ypoints + dyy;
   xapp = xapoints + dxx;
   yapp = yapoints + dyy;
   pbuf = buf;

   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	while (dst_clip_h--)
	  {
	     Cy = *yapp >> 16;
	     yap = *yapp & 0xffff;

	     while (dst_clip_w--)
	       {
		  pix = *yp + *xp + pos;

		  a = (A_VAL(pix) * yap) >> 10;
		  r = (R_VAL(pix) * yap) >> 10;
		  g = (G_VAL(pix) * yap) >> 10;
		  b = (B_VAL(pix) * yap) >> 10;
		  for (j = (1 << 14) - yap; j > Cy; j -= Cy)
		    {
		       pix += src_w;
		       a += (A_VAL(pix) * Cy) >> 10;
		       r += (R_VAL(pix) * Cy) >> 10;
		       g += (G_VAL(pix) * Cy) >> 10;
		       b += (B_VAL(pix) * Cy) >> 10;
		    }
		  if (j > 0)
		    {
		       pix += src_w;
		       a += (A_VAL(pix) * j) >> 10;
		       r += (R_VAL(pix) * j) >> 10;
		       g += (G_VAL(pix) * j) >> 10;
		       b += (B_VAL(pix) * j) >> 10;
		    }
		  if ((xap = *xapp) > 0)
		    {
		       pix = *yp + *xp + 1 + pos;
		       aa = (A_VAL(pix) * yap) >> 10;
		       rr = (R_VAL(pix) * yap) >> 10;
		       gg = (G_VAL(pix) * yap) >> 10;
		       bb = (B_VAL(pix) * yap) >> 10;
		       for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			 {
			    pix += src_w;
			    aa += (A_VAL(pix) * Cy) >> 10;
			    rr += (R_VAL(pix) * Cy) >> 10;
			    gg += (G_VAL(pix) * Cy) >> 10;
			    bb += (B_VAL(pix) * Cy) >> 10;
			 }
		       if (j > 0)
			 {
			    pix += src_w;
			    aa += (A_VAL(pix) * j) >> 10;
			    rr += (R_VAL(pix) * j) >> 10;
			    gg += (G_VAL(pix) * j) >> 10;
			    bb += (B_VAL(pix) * j) >> 10;
			 }
		       a += ((aa - a) * xap) >> 8;
		       r += ((rr - r) * xap) >> 8;
		       g += ((gg - g) * xap) >> 8;
		       b += ((bb - b) * xap) >> 8;
		    }
		  *pbuf++ = ARGB_JOIN(a >> 4, r >> 4, g >> 4, b >> 4);
		  xp++;  xapp++;
	       }

	     if (dc->mod.use)
	       func_cmod(buf, dptr, w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
	     else if (dc->mul.use)
	       func_mul(buf, dptr, w, dc->mul.col);
	     else
	       func(buf, dptr, w);

	     pbuf = buf;
	     dptr += dst_w;  dst_clip_w = w;
	     yp++;  yapp++;
	     xp = xpoints + dxx;
	     xapp = xapoints + dxx;
	  }
     }
   else
     {
	while (dst_clip_h--)
	  {
	     Cy = *yapp >> 16;
	     yap = *yapp & 0xffff;

	     while (dst_clip_w--)
	       {
		  pix = *yp + *xp + pos;

		  r = (R_VAL(pix) * yap) >> 10;
		  g = (G_VAL(pix) * yap) >> 10;
		  b = (B_VAL(pix) * yap) >> 10;
		  for (j = (1 << 14) - yap; j > Cy; j -= Cy)
		    {
		       pix += src_w;
		       r += (R_VAL(pix) * Cy) >> 10;
		       g += (G_VAL(pix) * Cy) >> 10;
		       b += (B_VAL(pix) * Cy) >> 10;
		    }
		  if (j > 0)
		    {
		       pix += src_w;
		       r += (R_VAL(pix) * j) >> 10;
		       g += (G_VAL(pix) * j) >> 10;
		       b += (B_VAL(pix) * j) >> 10;
		    }
		  if ((xap = *xapp) > 0)
		    {
		       pix = *yp + *xp + 1 + pos;
		       rr = (R_VAL(pix) * yap) >> 10;
		       gg = (G_VAL(pix) * yap) >> 10;
		       bb = (B_VAL(pix) * yap) >> 10;
		       for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			 {
			    pix += src_w;
			    rr += (R_VAL(pix) * Cy) >> 10;
			    gg += (G_VAL(pix) * Cy) >> 10;
			    bb += (B_VAL(pix) * Cy) >> 10;
			 }
		       if (j > 0)
			 {
			    pix += src_w;
			    rr += (R_VAL(pix) * j) >> 10;
			    gg += (G_VAL(pix) * j) >> 10;
			    bb += (B_VAL(pix) * j) >> 10;
			 }
		       r += ((rr - r) * xap) >> 8;
		       g += ((gg - g) * xap) >> 8;
		       b += ((bb - b) * xap) >> 8;
		    }
		  *pbuf++ = ARGB_JOIN(0xff, r >> 4, g >> 4, b >> 4);
		  xp++;  xapp++;
	       }

	     if (dc->mod.use)
	       func_cmod(buf, dptr, w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
	     else if (dc->mul.use)
	       func_mul(buf, dptr, w, dc->mul.col);
	     else
	       func(buf, dptr, w);

	     pbuf = buf;
	     dptr += dst_w;  dst_clip_w = w;
	     yp++;  yapp++;
	     xp = xpoints + dxx;
	     xapp = xapoints + dxx;
	  }
     }
}
