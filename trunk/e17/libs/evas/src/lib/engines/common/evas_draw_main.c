#include "evas_common.h"

extern DATA8        *_evas_pow_lut;

void
evas_common_draw_init(void)
{
}

RGBA_Draw_Context *
evas_common_draw_context_new(void)
{
   RGBA_Draw_Context *dc;

   dc = calloc(1, sizeof(RGBA_Draw_Context));
   return dc;
}

void
evas_common_draw_context_free(RGBA_Draw_Context *dc)
{
   free(dc);
}

void
evas_common_draw_context_font_ext_set(RGBA_Draw_Context *dc,
				      void *data,
				      void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
				      void  (*gl_free) (void *ext_dat),
				      void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y))
{
   dc->font_ext.data = data;
   dc->font_ext.func.gl_new = gl_new;
   dc->font_ext.func.gl_free = gl_free;
   dc->font_ext.func.gl_draw = gl_draw;
}

void
evas_common_draw_context_clip_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h, 
			   x, y, w, h);
     }
   else
     evas_common_draw_context_set_clip(dc, x, y, w, h);
}

void
evas_common_draw_context_set_clip(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   dc->clip.use = 1;
   dc->clip.x = x;
   dc->clip.y = y;
   dc->clip.w = w;
   dc->clip.h = h;
}

void
evas_common_draw_context_unset_clip(RGBA_Draw_Context *dc)
{
   dc->clip.use = 0;
}

void
evas_common_draw_context_set_color(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   R_VAL(&(dc->col.col)) = (DATA8)r;
   G_VAL(&(dc->col.col)) = (DATA8)g;
   B_VAL(&(dc->col.col)) = (DATA8)b;
   A_VAL(&(dc->col.col)) = (DATA8)a;
}

void
evas_common_draw_context_set_multiplier(RGBA_Draw_Context *dc, int r, int g, int b, int a)
{
   dc->mul.use = 1;
   dc->mod.use = 0;
   R_VAL(&(dc->mul.col)) = (DATA8)r;
   G_VAL(&(dc->mul.col)) = (DATA8)g;
   B_VAL(&(dc->mul.col)) = (DATA8)b;
   A_VAL(&(dc->mul.col)) = (DATA8)a;
}

void
evas_common_draw_context_unset_multiplier(RGBA_Draw_Context *dc)
{
   dc->mul.use = 0;
}

void
evas_common_draw_context_set_modifiers(RGBA_Draw_Context *dc, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod)
{
   int i;
   
   dc->mod.use = 1;
   dc->mul.use = 0;
   if (rmod) memcpy(dc->mod.r, rmod, sizeof(DATA8) * 256);
   else
     {
	for (i = 0; i < 256; i++) dc->mod.r[i] = (DATA8)i;
     }
   if (gmod) memcpy(dc->mod.g, gmod, sizeof(DATA8) * 256);
   else
     {
	for (i = 0; i < 256; i++) dc->mod.g[i] = (DATA8)i;
     }
   if (bmod) memcpy(dc->mod.b, bmod, sizeof(DATA8) * 256);
   else
     {
	for (i = 0; i < 256; i++) dc->mod.b[i] = (DATA8)i;
     }
   if (amod) memcpy(dc->mod.a, amod, sizeof(DATA8) * 256);
   else
     {
	for (i = 0; i < 256; i++) dc->mod.a[i] = (DATA8)i;
     }
}

void
evas_common_draw_context_ununset_modifiers(RGBA_Draw_Context *dc)
{
   dc->mod.use = 0;
}

void
evas_common_draw_context_add_cutout(RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   Cutout_Rect *r;
   
   r = calloc(1, sizeof(Cutout_Rect));
   r->x = x;
   r->y = y;
   r->w = w;
   r->h = h;
   dc->cutout.rects = evas_object_list_append(dc->cutout.rects, r);
}

void
evas_common_draw_context_clear_cutouts(RGBA_Draw_Context *dc)
{
   evas_common_draw_context_apply_free_cutouts(dc->cutout.rects);
   dc->cutout.rects = NULL;
}

Cutout_Rect *
evas_common_draw_context_apply_cutouts(RGBA_Draw_Context *dc)
{
   Cutout_Rect *r, *rects;
   Evas_Object_List *l;
   
   if (!dc->clip.use) return NULL;
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0)) return NULL;
   r = calloc(1, sizeof(Cutout_Rect));
   r->x = dc->clip.x;
   r->y = dc->clip.y;
   r->w = dc->clip.w;
   r->h = dc->clip.h;
   rects = r;
   for (l = (Evas_Object_List *)dc->cutout.rects; l; l = l->next)
     {
	r = (Cutout_Rect *)l;
	rects = evas_common_draw_context_cutouts_split(rects, r);
     }
   return rects;
}

void
evas_common_draw_context_apply_free_cutouts(Cutout_Rect *rects)
{
   while (rects)
     {
	Cutout_Rect *r;
	
	r = rects;
	rects = evas_object_list_remove(rects, rects);
	free(r);
     }
}

Cutout_Rect *
evas_common_draw_context_cutouts_split(Cutout_Rect *in, Cutout_Rect *split)
{
   /* multiple rect in, multiple out */
   Cutout_Rect *out;
   Evas_Object_List *l;
   
   out = NULL;
   for (l = (Evas_Object_List *)in; l; l = l->next)
     {
	Cutout_Rect *r;

	r = (Cutout_Rect *)l;
	r = evas_common_draw_context_cutout_split(r, split);
	while (r)
	  {
	     Cutout_Rect *r2;
	     
	     r2 = r;
	     r = evas_object_list_remove(r, r);
	     out = evas_object_list_append(out, r2);
	  }
     }
   evas_common_draw_context_apply_free_cutouts(in);
   return out;
}

Cutout_Rect *
evas_common_draw_context_cutout_split(Cutout_Rect *in, Cutout_Rect *split)
{
   /* 1 input rect, multiple out */
   Cutout_Rect *out;
   Cutout_Rect *r;

   /* this is to save me a LOT of typing */
#define INX1 (in->x)
#define INX2 (in->x + in->w)
#define SPX1 (split->x)
#define SPX2 (split->x + split->w)   
#define INY1 (in->y)
#define INY2 (in->y + in->h)
#define SPY1 (split->y)
#define SPY2 (split->y + split->h)   
#define X1_IN (in->x < split->x)
#define X2_IN ((in->x + in->w) > (split->x + split->w))
#define Y1_IN (in->y < split->y)
#define Y2_IN ((in->y + in->h) > (split->y + split->h))
#define R_NEW(_r, _x, _y, _w, _h) {(_r) = calloc(1, sizeof(Cutout_Rect)); (_r)->x = (_x); (_r)->y = (_y); (_r)->w = (_w); (_r)->h = (_h);}   
   out = NULL;
   if (!RECTS_INTERSECT(in->x, in->y, in->w, in->h, 
			split->x, split->y, split->w, split->h))
     {
	R_NEW(r, in->x, in->y, in->w, in->h);
	out = evas_object_list_append(out, r);
	return out;
     }
   
   /* S    = split (ie cut out rect) */
   /* +--+ = in (rect to be cut) */
   
   /* 
    *  +---+
    *  |   |
    *  | S |
    *  |   |
    *  +---+
    * 
    */
   if (X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY1, SPX1 - in->x, SPY2 - SPY1);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    * S|SSS|S
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
	return NULL;
     }
   /* SSS
    * S+---+
    * S|S  |
    * S|S  |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
	R_NEW(r, SPX2, in->y, INX2 - SPX2, in->h);
	out = evas_object_list_append(out, r);
	return out;
     }
   /*    S
    *  +---+
    *  | S |
    *  | S |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && !Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, SPX1 - in->x, in->h);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, in->y, INX2 - SPX2, in->h);
	out = evas_object_list_append(out, r);
	return out;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |  S|S
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && !Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, SPX1 - in->x, in->h);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* SSSSSSS
    * S+---+S
    * S|SSS|S
    *  |   |
    *  |   |
    *  +---+
    * 
    */
   if (!X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    * S|SSS|S
    *  |   |
    *  +---+
    * 
    */
   if (!X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    *  |   |
    * S|SSS|S
    * S+---+S
    * SSSSSSS
    */
   if (!X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* SSS
    * S+---+
    * S|S  |
    *  |   |
    *  |   |
    *  +---+
    * 
    */
   if (!X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(r, SPX2, in->y, INX2 - SPX2, SPY2 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /*    S
    *  +---+
    *  | S |
    *  |   |
    *  |   |
    *  +---+
    * 
    */
   if (X1_IN && X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, SPX1 - in->x, SPY2 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, in->y, INX2 - SPX2, SPY2 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /*     SSS
    *  +---+S
    *  |  S|S
    *  |   |
    *  |   |
    *  +---+
    * 
    */
   if (X1_IN && !X2_IN && !Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, SPX1 - in->x, SPY2 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    * S|S  |
    *  |   |
    *  +---+
    * 
    */
   if (!X1_IN && X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, SPY1, INX2 - SPX2, SPY2 - SPY1);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    *  |  S|S
    *  |   |
    *  +---+
    * 
    */
   if (X1_IN && !X2_IN && Y1_IN && Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY1, SPX1 - in->x, SPY2 - SPY1);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY2, in->w, INY2 - SPY2);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    *  |   |
    * S|S  |
    * S+---+
    * SSS
    */
   if (!X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    *  |   |
    *  | S |
    *  +---+
    *    S
    */
   if (X1_IN && X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY1, SPX1 - in->x, INY2 - SPY1);
	out = evas_object_list_append(out, r);
	R_NEW(r, SPX2, SPY1, INX2 - SPX2, INY2 - SPY1);
	out = evas_object_list_append(out, r);
	return out;
     }
   /* 
    *  +---+
    *  |   |
    *  |   |
    *  |  S|S
    *  +---+S
    *     SSS
    */
   if (X1_IN && !X2_IN && Y1_IN && !Y2_IN)
     {
	R_NEW(r, in->x, in->y, in->w, SPY1 - in->y);
	out = evas_object_list_append(out, r);
	R_NEW(r, in->x, SPY1, SPX1 - in->x, INY2 - SPY1);
	out = evas_object_list_append(out, r);
	return out;
     }
   return NULL;
}

Gfx_Func_Blend_Src_Dst
evas_common_draw_func_blend_get(RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     return evas_common_blend_pixels_rgba_to_rgba_c;
	  }
	else
	  {
#ifdef BUILD_MMX
# ifdef BUILD_C
	     if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
# endif	       
	       return evas_common_blend_pixels_rgba_to_rgb_mmx;
# ifdef BUILD_C
	     else
# endif	       
#endif
#ifdef BUILD_C
	       return evas_common_blend_pixels_rgba_to_rgb_c;
#endif	     
	  }
     }
   else
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     return evas_common_copy_pixels_rgb_to_rgba_c;
	  }
	else
	  {
#ifdef BUILD_SSE	     
	     if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	       return evas_common_copy_pixels_rgba_to_rgba_sse;
# ifdef BUILD_MMX	     
	     else
# endif	       
#endif	       
#ifdef BUILD_MMX
# ifdef BUILD_C
	       if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
# endif	       
		 return evas_common_copy_pixels_rgba_to_rgba_mmx2;
# ifdef BUILD_C
	       else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
# endif	       
		 return evas_common_copy_pixels_rgba_to_rgba_mmx;
# ifdef BUILD_C
	     else
# endif	       
#endif
#ifdef BUILD_C
	       return evas_common_copy_pixels_rgba_to_rgba_c;
#endif	     
	  }
     }
   return evas_common_blend_pixels_rgba_to_rgba_c;
   pixels = 0;
}

Gfx_Func_Blend_Color_Dst
evas_common_draw_func_blend_color_get(DATA32 src, RGBA_Image *dst, int pixels)
{
   if (A_VAL(&src) != 0xff)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_blend_color_rgba_to_rgba_c;
	  }
	else
	  {
#ifdef BUILD_MMX
	     if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	       return evas_common_blend_color_rgba_to_rgb_mmx;
#endif	     
#ifdef BUILD_C	       
# ifdef BUILD_MMX
	     else
# endif
	       return evas_common_blend_color_rgba_to_rgb_c;
#endif	     
	  }
     }
   else
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_copy_color_rgb_to_rgba_c;
	  }
	else
	  {
#ifdef  BUILD_SSE	     
	     if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	       return evas_common_copy_color_rgba_to_rgba_sse;
#endif
#ifdef BUILD_MMX
# ifdef BUILD_SSE	     
	     else
# endif	       
	       if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
		 return evas_common_copy_color_rgba_to_rgba_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX	     
	     else
# endif	       
	       return evas_common_copy_color_rgba_to_rgba_c;
#endif	     
	  }
     }
   return evas_common_blend_color_rgba_to_rgba_c;
   pixels = 0;
}

Gfx_Func_Blend_Src_Cmod_Dst
evas_common_draw_func_blend_cmod_get(RGBA_Image *src, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_blend_pixels_cmod_rgba_to_rgba_c;
	  }
	else
	  {
	     return evas_common_blend_pixels_cmod_rgba_to_rgb_c;
	  }
     }
   else
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_copy_pixels_cmod_rgb_to_rgba_c;
	  }
	else
	  {
	     return evas_common_copy_pixels_cmod_rgba_to_rgba_c;
	  }
     }
   return evas_common_blend_pixels_cmod_rgba_to_rgba_c;
   pixels = 0;
}

Gfx_Func_Blend_Src_Mul_Dst
evas_common_draw_func_blend_mul_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels)
{
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_blend_pixels_mul_color_rgba_to_rgba_c;
	  }
	else
	  {
#ifdef BUILD_MMX	     
	     if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	       return evas_common_blend_pixels_mul_color_rgba_to_rgb_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX	     
	     else
# endif	       
	       return evas_common_blend_pixels_mul_color_rgba_to_rgb_c;
#endif	     
	  }
     }
   else
     {
	if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
	  {
	     if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	     return evas_common_blend_pixels_mul_color_rgba_to_rgba_c;
	  }
	else
	  {
#ifdef BUILD_MMX	     
	     if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	       return evas_common_blend_pixels_mul_color_rgba_to_rgb_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX	     
	     else
# endif	       
	       return evas_common_blend_pixels_mul_color_rgba_to_rgb_c;
#endif	     
	  }
     }
   return evas_common_blend_pixels_mul_color_rgba_to_rgba_c;
   col = 0;
   pixels = 0;
}

Gfx_Func_Blend_Src_Alpha_Mul_Dst
evas_common_draw_func_blend_alpha_get(RGBA_Image *dst)
{
   if (dst->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	if (!_evas_pow_lut) evas_common_blend_init_evas_pow_lut();
	return evas_common_blend_alpha_color_rgba_to_rgba_c;
     }
   else
     {
#ifdef BUILD_MMX	     
	if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	  return evas_common_blend_alpha_color_rgba_to_rgb_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX
	else
# endif
	  return evas_common_blend_alpha_color_rgba_to_rgb_c;
#endif
     }
#ifdef BUILD_C
   return evas_common_blend_alpha_color_rgba_to_rgba_c;
#else
   return NULL;
#endif   
}

Gfx_Func_Blend_Src_Dst
evas_common_draw_func_copy_get(int pixels, int reverse)
{
   if (reverse)
     {
#ifdef  BUILD_SSE	     
	if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	  return evas_common_copy_pixels_rev_rgba_to_rgba_sse;
#endif
#ifdef BUILD_MMX
# ifdef BUILD_SSE	     
	else
# endif	       
	  if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	    return evas_common_copy_pixels_rev_rgba_to_rgba_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX	     
	else
# endif	       
	  return evas_common_copy_pixels_rev_rgba_to_rgba_c;
#endif	       
     }
   else
     {
#ifdef  BUILD_SSE	     
	if (evas_common_cpu_has_feature(CPU_FEATURE_SSE) && (pixels > 256 * 256))
	  return evas_common_copy_pixels_rgba_to_rgba_sse;
#endif
#ifdef BUILD_MMX
# ifdef BUILD_SSE	     
	else
# endif	       
	  if (evas_common_cpu_has_feature(CPU_FEATURE_MMX2))
	    return evas_common_copy_pixels_rgba_to_rgba_mmx2;
	  else if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
	    return evas_common_copy_pixels_rgba_to_rgba_mmx;
#endif
#ifdef BUILD_C
# ifdef BUILD_MMX	     
	else
# endif	       
	  return evas_common_copy_pixels_rgba_to_rgba_c;
#endif	       
     }
#ifdef BUILD_C
   return evas_common_copy_pixels_rgba_to_rgba_c;
#else
   return NULL;
#endif
   pixels = 0;
}

