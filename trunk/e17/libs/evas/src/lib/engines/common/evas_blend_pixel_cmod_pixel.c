#include "evas_common.h"

#ifdef BUILD_MMX
#include "evas_mmx.h"
#endif

extern DATA8        pow_lut[256][256];
extern const DATA16 const_c1[4];

void
evas_common_blend_pixels_cmod_rgba_to_rgb_c(DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	
	a = amod[A_VAL(src_ptr)];
	if (a) /* hmmm - do we need this? */
	  {
	     BLEND_COLOR(a, R_VAL(dst_ptr), 
			 rmod[R_VAL(src_ptr)], R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, G_VAL(dst_ptr), 
			 gmod[G_VAL(src_ptr)], G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, B_VAL(dst_ptr), 
			 bmod[B_VAL(src_ptr)], B_VAL(dst_ptr), 
			 tmp);
	  }
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

void
evas_common_blend_pixels_cmod_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	DATA32 tmp;
	DATA8  a;
	
	a = pow_lut[amod[A_VAL(src_ptr)]][A_VAL(dst_ptr)];
	if (a) /* hmmm - do we need this? */
	  {
	     BLEND_COLOR(a, R_VAL(dst_ptr), 
			 rmod[R_VAL(src_ptr)], R_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, G_VAL(dst_ptr), 
			 gmod[G_VAL(src_ptr)], G_VAL(dst_ptr), 
			 tmp);
	     BLEND_COLOR(a, B_VAL(dst_ptr), 
			 bmod[B_VAL(src_ptr)], B_VAL(dst_ptr), 
			 tmp);	
	     A_VAL(dst_ptr) = A_VAL(dst_ptr) + ((amod[A_VAL(src_ptr)] * (255 - A_VAL(dst_ptr))) / 255);
	  }
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

void
evas_common_copy_pixels_cmod_rgba_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	R_VAL(dst_ptr) = rmod[R_VAL(src_ptr)];
	G_VAL(dst_ptr) = gmod[G_VAL(src_ptr)];
	B_VAL(dst_ptr) = bmod[B_VAL(src_ptr)];
	A_VAL(dst_ptr) = amod[A_VAL(src_ptr)];
	src_ptr++;
	dst_ptr++;
     }
}

/****************************************************************************/

void
evas_common_copy_pixels_cmod_rgb_to_rgba_c(DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod)
{
   DATA32 *src_ptr, *dst_ptr, *dst_end_ptr;
   
   src_ptr = src;
   dst_ptr = dst;
   dst_end_ptr = dst + len;

   while (dst_ptr < dst_end_ptr)
     {
	R_VAL(dst_ptr) = rmod[R_VAL(src_ptr)];
	G_VAL(dst_ptr) = gmod[G_VAL(src_ptr)];
	B_VAL(dst_ptr) = bmod[B_VAL(src_ptr)];
	A_VAL(dst_ptr) = amod[255];
	src_ptr++;
	dst_ptr++;
     }
}
