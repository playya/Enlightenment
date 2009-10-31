#include "evas_common.h"
#include "evas_private.h"

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */
static int cpunum = 0;
static int _evas_soft_gen_log_dom = -1;
static void *
eng_context_new(void *data __UNUSED__)
{
   return evas_common_draw_context_new();
}

static void
eng_context_free(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_free(context);
}

static void
eng_context_clip_set(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
eng_context_clip_clip(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
eng_context_clip_unset(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_unset_clip(context);
}

static int
eng_context_clip_get(void *data __UNUSED__, void *context, int *x, int *y, int *w, int *h)
{
   *x = ((RGBA_Draw_Context *)context)->clip.x;
   *y = ((RGBA_Draw_Context *)context)->clip.y;
   *w = ((RGBA_Draw_Context *)context)->clip.w;
   *h = ((RGBA_Draw_Context *)context)->clip.h;
   return ((RGBA_Draw_Context *)context)->clip.use;
}

static void
eng_context_color_set(void *data __UNUSED__, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
eng_context_color_get(void *data __UNUSED__, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
eng_context_multiplier_set(void *data __UNUSED__, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
eng_context_multiplier_unset(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_unset_multiplier(context);
}

static int
eng_context_multiplier_get(void *data __UNUSED__, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
}

static void
eng_context_cutout_add(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_context_anti_alias_set(void *data __UNUSED__, void *context, unsigned char aa)
{
   evas_common_draw_context_set_anti_alias(context, aa);
}

static unsigned char
eng_context_anti_alias_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->anti_alias;
}

static void
eng_context_color_interpolation_set(void *data __UNUSED__, void *context, int color_space)
{
   evas_common_draw_context_set_color_interpolation(context, color_space);
}

static int
eng_context_color_interpolation_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->interpolation.color_space;
}

static void
eng_context_render_op_set(void *data __UNUSED__, void *context, int op)
{
   evas_common_draw_context_set_render_op(context, op);
}

static int
eng_context_render_op_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->render_op;
}



static void
eng_rectangle_draw(void *data __UNUSED__, void *context, void *surface, int x, int y, int w, int h)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_rectangle_draw(surface, context, x, y, w, h);
   else
#endif
     {
	evas_common_rectangle_draw(surface, context, x, y, w, h);
	evas_common_cpu_end_opt();
     }
}

static void
eng_line_draw(void *data __UNUSED__, void *context, void *surface, int x1, int y1, int x2, int y2)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_line_draw(surface, context, x1, y1, x2, y2);
   else
#endif   
     {
	evas_common_line_draw(surface, context, x1, y1, x2, y2);
	evas_common_cpu_end_opt();
     }
}

static void *
eng_polygon_point_add(void *data __UNUSED__, void *context __UNUSED__, void *polygon, int x, int y)
{
   return evas_common_polygon_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data __UNUSED__, void *context __UNUSED__, void *polygon)
{
   return evas_common_polygon_points_clear(polygon);
}

static void
eng_polygon_draw(void *data __UNUSED__, void *context, void *surface, void *polygon)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_poly_draw(surface, context, polygon);
   else
#endif
     {
	evas_common_polygon_draw(surface, context, polygon);
	evas_common_cpu_end_opt();
     }
}

static void
eng_gradient2_color_np_stop_insert(void *data __UNUSED__, void *gradient, int r, int g, int b, int a, float pos)
{
   evas_common_gradient2_color_np_stop_insert(gradient, r, g, b, a, pos);
}

static void
eng_gradient2_clear(void *data __UNUSED__, void *gradient)
{
   evas_common_gradient2_clear(gradient);
}

static void
eng_gradient2_fill_transform_set(void *data __UNUSED__, void *gradient, void *transform)
{
   evas_common_gradient2_fill_transform_set(gradient, transform);
}

static void
eng_gradient2_fill_spread_set(void *data __UNUSED__, void *gradient, int spread)
{
   evas_common_gradient2_fill_spread_set(gradient, spread);
}

static void *
eng_gradient2_linear_new(void *data __UNUSED__)
{
   return evas_common_gradient2_linear_new();
}

static void
eng_gradient2_linear_free(void *data __UNUSED__, void *linear_gradient)
{
   evas_common_gradient2_free(linear_gradient);
}

static void
eng_gradient2_linear_fill_set(void *data __UNUSED__, void *linear_gradient, float x0, float y0, float x1, float y1)
{
   evas_common_gradient2_linear_fill_set(linear_gradient, x0, y0, x1, y1);
}

static int
eng_gradient2_linear_is_opaque(void *data __UNUSED__, void *context, void *linear_gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)linear_gradient;

   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
              gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient2_linear_is_visible(void *data __UNUSED__, void *context, void *linear_gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !linear_gradient)  return 0;
   return 1;
}

static void
eng_gradient2_linear_render_pre(void *data __UNUSED__, void *context, void *linear_gradient)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)linear_gradient;
   int  len;

   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_update(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient2_map(dc, gr, len);
}

static void
eng_gradient2_linear_render_post(void *data __UNUSED__, void *linear_gradient __UNUSED__)
{
}

static void
eng_gradient2_linear_draw(void *data __UNUSED__, void *context, void *surface, void *linear_gradient, int x, int y, int w, int h)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_grad2_draw(surface, context, x, y, w, h, linear_gradient);
   else
#endif
     evas_common_gradient2_draw(surface, context, x, y, w, h, linear_gradient);
}

static void *
eng_gradient2_radial_new(void *data __UNUSED__)
{
   return evas_common_gradient2_radial_new();
}

static void
eng_gradient2_radial_free(void *data __UNUSED__, void *radial_gradient)
{
   evas_common_gradient2_free(radial_gradient);
}

static void
eng_gradient2_radial_fill_set(void *data __UNUSED__, void *radial_gradient, float cx, float cy, float rx, float ry)
{
   evas_common_gradient2_radial_fill_set(radial_gradient, cx, cy, rx, ry);
}

static int
eng_gradient2_radial_is_opaque(void *data __UNUSED__, void *context, void *radial_gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)radial_gradient;

   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
              gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient2_radial_is_visible(void *data __UNUSED__, void *context, void *radial_gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !radial_gradient)  return 0;
   return 1;
}

static void
eng_gradient2_radial_render_pre(void *data __UNUSED__, void *context, void *radial_gradient)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient2 *gr = (RGBA_Gradient2 *)radial_gradient;
   int  len;

   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_update(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient2_map(dc, gr, len);
}

static void
eng_gradient2_radial_render_post(void *data __UNUSED__, void *radial_gradient __UNUSED__)
{
}

static void
eng_gradient2_radial_draw(void *data __UNUSED__, void *context, void *surface, void *radial_gradient, int x, int y, int w, int h)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_grad2_draw(surface, context, x, y, w, h, radial_gradient);
   else
#endif
     evas_common_gradient2_draw(surface, context, x, y, w, h, radial_gradient);
}

static void *
eng_gradient_new(void *data __UNUSED__)
{
   return evas_common_gradient_new();
}

static void
eng_gradient_free(void *data __UNUSED__, void *gradient)
{
   evas_common_gradient_free(gradient);
}

static void
eng_gradient_color_stop_add(void *data __UNUSED__, void *gradient, int r, int g, int b, int a, int delta)
{
   evas_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data __UNUSED__, void *gradient, int a, int delta)
{
   evas_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_color_data_set(void *data __UNUSED__, void *gradient, void *map, int len, int has_alpha)
{
   evas_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data __UNUSED__, void *gradient, void *alpha_map, int len)
{
   evas_common_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_clear(void *data __UNUSED__, void *gradient)
{
   evas_common_gradient_clear(gradient);
}

static void
eng_gradient_fill_set(void *data __UNUSED__, void *gradient, int x, int y, int w, int h)
{
   evas_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data __UNUSED__, void *gradient, double angle)
{
   evas_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data __UNUSED__, void *gradient, int spread)
{
   evas_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data __UNUSED__, void *gradient, double angle)
{
   evas_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data __UNUSED__, void *gradient, float offset)
{
   evas_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data __UNUSED__, void *gradient, int direction)
{
   evas_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data __UNUSED__, void *gradient, char *name, char *params)
{
   evas_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data __UNUSED__, void *context, void *gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;

   if (!dc || !gr || !gr->type.geometer)  return 0;
   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
              gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient_is_visible(void *data __UNUSED__, void *context, void *gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !gradient)  return 0;
   return 1;
}

static void
eng_gradient_render_pre(void *data __UNUSED__, void *context, void *gradient)
{
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;
   int  len;

   if (!dc || !gr || !gr->type.geometer)  return;
   gr->type.geometer->geom_set(gr);
   len = gr->type.geometer->get_map_len(gr);
   evas_common_gradient_map(dc, gr, len);
}

static void
eng_gradient_render_post(void *data __UNUSED__, void *gradient __UNUSED__)
{
}

static void
eng_gradient_draw(void *data __UNUSED__, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_grad_draw(surface, context, x, y, w, h, gradient);
   else
#endif   
     {
	evas_common_gradient_draw(surface, context, x, y, w, h, gradient);
	evas_common_cpu_end_opt();
     }
}

static int
eng_image_alpha_get(void *data __UNUSED__, void *image)
{
   Image_Entry *im;

   if (!image) return 1;
   im = image;
   switch (im->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->flags.alpha) return 1;
      default:
	break;
     }
   return 0;
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image)
{
   Image_Entry *im;

   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->space;
}

static void *
eng_image_alpha_set(void *data __UNUSED__, void *image, int has_alpha)
{
   RGBA_Image *im;

   if (!image) return NULL;
   im = image;
   if (im->cache_entry.space != EVAS_COLORSPACE_ARGB8888)
     {
	im->cache_entry.flags.alpha = 0;
	return im;
     }
   im = (RGBA_Image *) evas_cache_image_alone(&im->cache_entry);
   evas_common_image_colorspace_dirty(im);

   im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   return im;
}

static void *
eng_image_border_set(void *data __UNUSED__, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
   RGBA_Image *im;

   im = image;
   return im;
}

static void
eng_image_border_get(void *data __UNUSED__, void *image, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
   RGBA_Image *im;

   im = image;
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image, char *key __UNUSED__)
{
   RGBA_Image *im;

   if (!image) return NULL;
   im = image;
   return im->info.comment;
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return NULL;
}

static void
eng_image_colorspace_set(void *data __UNUSED__, void *image, int cspace)
{
   Image_Entry *im;

   if (!image) return;
   im = image;
   evas_cache_image_colorspace(im, cspace);
}

static void
eng_image_native_set(void *data __UNUSED__, void *image __UNUSED__, void *native __UNUSED__)
{
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return NULL;
}

static void *
eng_image_load(void *data __UNUSED__, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   *error = 0;
   return evas_common_load_image_from_file(file, key, lo);
}

static void *
eng_image_new_from_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   return evas_cache_image_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   return evas_cache_image_copied_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data __UNUSED__, void *image)
{
   evas_cache_image_drop(image);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   Image_Entry *im;

   im = image;
   if (w) *w = im->w;
   if (h) *h = im->h;
}

static void *
eng_image_size_set(void *data __UNUSED__, void *image, int w, int h)
{
   Image_Entry *im;

   im = image;
   return evas_cache_image_size_set(image, w, h);
}

static void *
eng_image_dirty_region(void *data __UNUSED__, void *image, int x, int y, int w, int h)
{
   Image_Entry *im = image;

   if (!image) return NULL;
   return evas_cache_image_dirty(im, x, y, w, h);
}

static void *
eng_image_data_get(void *data __UNUSED__, void *image, int to_write, DATA32 **image_data)
{
   RGBA_Image *im;

   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = image;
   evas_cache_image_load_data(&im->cache_entry);
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
          im = (RGBA_Image *) evas_cache_image_alone(&im->cache_entry);
	*image_data = im->image.data;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	*image_data = im->cs.data;
        break;
      default:
	abort();
	break;
     }
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   RGBA_Image *im, *im2;

   if (!image) return NULL;
   im = image;
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (image_data != im->image.data)
	  {
	     int w, h;

	     w = im->cache_entry.w;
	     h = im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
             evas_cache_image_drop(&im->cache_entry);
	     im = im2;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (image_data != im->cs.data)
	  {
	     if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	     evas_common_image_colorspace_dirty(im);
	  }
        break;
      default:
	abort();
	break;
     }
   return im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   RGBA_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   RGBA_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data __UNUSED__, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   RGBA_Image *im;

   if (!image) return;
   im = image;
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     {
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          evas_cache_image_load_data(&im->cache_entry);
        evas_common_image_colorspace_normalize(im);
        evas_common_rgba_image_scalecache_prepare(im, surface, context, smooth,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);
        
        evas_common_pipe_image_draw(im, surface, context, smooth,
                                    src_x, src_y, src_w, src_h,
                                    dst_x, dst_y, dst_w, dst_h);
     }
   else
#endif
     {
//        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
//          evas_cache_image_load_data(&im->cache_entry);
//        evas_common_image_colorspace_normalize(im);
        evas_common_rgba_image_scalecache_prepare(im, surface, context, smooth,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);
        evas_common_rgba_image_scalecache_do(im, surface, context, smooth,
                                             src_x, src_y, src_w, src_h,
                                             dst_x, dst_y, dst_w, dst_h);
/*        
	if (smooth)
	  evas_common_scale_rgba_in_to_out_clip_smooth(im, surface, context,
						       src_x, src_y, src_w, src_h,
						       dst_x, dst_y, dst_w, dst_h);
	else
	  evas_common_scale_rgba_in_to_out_clip_sample(im, surface, context,
						       src_x, src_y, src_w, src_h,
						       dst_x, dst_y, dst_w, dst_h);
 */
	evas_common_cpu_end_opt();
     }
}

static void
eng_image_map4_draw(void *data __UNUSED__, void *context, void *surface, void *image, RGBA_Map_Point *p, int smooth, int level)
{
   RGBA_Image *im;

   if (!image) return;
   im = image;
   evas_common_map4_rgba(im, surface, context, p, smooth, level);
   evas_common_cpu_end_opt();
}

static void *
eng_image_map_surface_new(void *data __UNUSED__, int w, int h, int alpha)
{
   void *surface;
   DATA32 *pixels;
   surface = evas_cache_image_copied_data(evas_common_image_cache_get(), 
                                          w, h, NULL, alpha, 
                                          EVAS_COLORSPACE_ARGB8888);
   pixels = evas_cache_image_pixels(surface);
   return surface;
}

static void
eng_image_map_surface_free(void *data __UNUSED__, void *surface)
{
   evas_cache_image_drop(surface);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image, int hint)
{
   Image_Entry *im;

   if (!image) return;
   im = image;
   im->scale_hint = hint;
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image)
{
   Image_Entry *im;

   if (!image) return EVAS_IMAGE_SCALE_HINT_NONE;
   im = image;
   return im->scale_hint;
}

static void
eng_image_cache_flush(void *data __UNUSED__)
{
   int tmp_size;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data __UNUSED__, int bytes)
{
   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
}

static int
eng_image_cache_get(void *data __UNUSED__)
{
   return evas_common_image_get_cache();
}

static void *
eng_font_load(void *data __UNUSED__, const char *name, int size)
{
   return evas_common_font_load(name, size);
}

static void *
eng_font_memory_load(void *data __UNUSED__, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void *
eng_font_add(void *data __UNUSED__, void *font, const char *name, int size)
{
   return evas_common_font_add(font, name, size);
}

static void *
eng_font_memory_add(void *data __UNUSED__, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_add(font, name, size, fdata, fdata_size);
}

static void
eng_font_free(void *data __UNUSED__, void *font)
{
   evas_common_font_free(font);
}

static int
eng_font_ascent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_ascent_get(font);
}

static int
eng_font_descent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_descent_get(font);
}

static int
eng_font_max_ascent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_max_ascent_get(font);
}

static int
eng_font_max_descent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_max_descent_get(font);
}

static void
eng_font_string_size_get(void *data __UNUSED__, void *font, const char *text, int *w, int *h)
{
   evas_common_font_query_size(font, text, w, h);
}

static int
eng_font_inset_get(void *data __UNUSED__, void *font, const char *text)
{
   return evas_common_font_query_inset(font, text);
}

static int
eng_font_h_advance_get(void *data __UNUSED__, void *font, const char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data __UNUSED__, void *font, const char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

static int
eng_font_char_coords_get(void *data __UNUSED__, void *font, const char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data __UNUSED__, void *font, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch);
}

static int
eng_font_last_up_to_pos(void *data __UNUSED__, void *font, const char *text, int x, int y)
{
   return evas_common_font_query_last_up_to_pos(font, text, x, y);
}

static void
eng_font_draw(void *data __UNUSED__, void *context, void *surface, void *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const char *text)
{
#ifdef BUILD_PIPE_RENDER
   if (cpunum > 1)
     evas_common_pipe_text_draw(surface, context, font, x, y, text);
   else
#endif   
     {
	evas_common_font_draw(surface, context, font, x, y, text);
	evas_common_cpu_end_opt();
     }
}

static void
eng_font_cache_flush(void *data __UNUSED__)
{
   evas_common_font_flush();
}

static void
eng_font_cache_set(void *data __UNUSED__, int bytes)
{
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *data __UNUSED__)
{
   return evas_common_font_cache_get();
}

static void
eng_font_hinting_set(void *data __UNUSED__, void *font, int hinting)
{
   evas_common_font_hinting_set(font, hinting);
}

static int
eng_font_hinting_can_hint(void *data __UNUSED__, int hinting)
{
   return evas_common_hinting_available(hinting);
}

static Eina_Bool
eng_canvas_alpha_get(void *data __UNUSED__, void *info __UNUSED__)
{
   return EINA_TRUE;
}

/*
 *****
 **
 ** ENGINE API
 **
 *****
 */

static Evas_Func func =
{
   NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     /* draw context virtual methods */
     eng_context_new,
     eng_canvas_alpha_get,
     eng_context_free,
     eng_context_clip_set,
     eng_context_clip_clip,
     eng_context_clip_unset,
     eng_context_clip_get,
     eng_context_color_set,
     eng_context_color_get,
     eng_context_multiplier_set,
     eng_context_multiplier_unset,
     eng_context_multiplier_get,
     eng_context_cutout_add,
     eng_context_cutout_clear,
     eng_context_anti_alias_set,
     eng_context_anti_alias_get,
     eng_context_color_interpolation_set,
     eng_context_color_interpolation_get,
     eng_context_render_op_set,
     eng_context_render_op_get,
     /* rect draw funcs */
     eng_rectangle_draw,
     /* line draw funcs */
     eng_line_draw,
     /* polygon draw funcs */
     eng_polygon_point_add,
     eng_polygon_points_clear,
     eng_polygon_draw,
     /* gradient draw funcs */
     eng_gradient2_color_np_stop_insert,
     eng_gradient2_clear,
     eng_gradient2_fill_transform_set,
     eng_gradient2_fill_spread_set,

     eng_gradient2_linear_new,
     eng_gradient2_linear_free,
     eng_gradient2_linear_fill_set,
     eng_gradient2_linear_is_opaque,
     eng_gradient2_linear_is_visible,
     eng_gradient2_linear_render_pre,
     eng_gradient2_linear_render_post,
     eng_gradient2_linear_draw,

     eng_gradient2_radial_new,
     eng_gradient2_radial_free,
     eng_gradient2_radial_fill_set,
     eng_gradient2_radial_is_opaque,
     eng_gradient2_radial_is_visible,
     eng_gradient2_radial_render_pre,
     eng_gradient2_radial_render_post,
     eng_gradient2_radial_draw,

     eng_gradient_new,
     eng_gradient_free,
     eng_gradient_color_stop_add,
     eng_gradient_alpha_stop_add,
     eng_gradient_color_data_set,
     eng_gradient_alpha_data_set,
     eng_gradient_clear,
     eng_gradient_fill_set,
     eng_gradient_fill_angle_set,
     eng_gradient_fill_spread_set,
     eng_gradient_angle_set,
     eng_gradient_offset_set,
     eng_gradient_direction_set,
     eng_gradient_type_set,
     eng_gradient_is_opaque,
     eng_gradient_is_visible,
     eng_gradient_render_pre,
     eng_gradient_render_post,
     eng_gradient_draw,
     /* image draw funcs */
     eng_image_load,
     eng_image_new_from_data,
     eng_image_new_from_copied_data,
     eng_image_free,
     eng_image_size_get,
     eng_image_size_set,
     NULL,
     eng_image_dirty_region,
     eng_image_data_get,
     eng_image_data_put,
     eng_image_data_preload_request,
     eng_image_data_preload_cancel,
     eng_image_alpha_set,
     eng_image_alpha_get,
     eng_image_border_set,
     eng_image_border_get,
     eng_image_draw,
     eng_image_comment_get,
     eng_image_format_get,
     eng_image_colorspace_set,
     eng_image_colorspace_get,
     eng_image_native_set,
     eng_image_native_get,
     /* image cache funcs */
     eng_image_cache_flush,
     eng_image_cache_set,
     eng_image_cache_get,
     /* font draw functions */
     eng_font_load,
     eng_font_memory_load,
     eng_font_add,
     eng_font_memory_add,
     eng_font_free,
     eng_font_ascent_get,
     eng_font_descent_get,
     eng_font_max_ascent_get,
     eng_font_max_descent_get,
     eng_font_string_size_get,
     eng_font_inset_get,
     eng_font_h_advance_get,
     eng_font_v_advance_get,
     eng_font_char_coords_get,
     eng_font_char_at_coords_get,
     eng_font_draw,
     /* font cache functions */
     eng_font_cache_flush,
     eng_font_cache_set,
     eng_font_cache_get,
     /* font hinting functions */
     eng_font_hinting_set,
     eng_font_hinting_can_hint,
     eng_image_scale_hint_set,
     eng_image_scale_hint_get,
     /* more font draw functions */
     eng_font_last_up_to_pos,
     /* FUTURE software generic calls go here (done) */
     eng_image_map4_draw,
     eng_image_map_surface_new,
     eng_image_map_surface_free
     /* FUTURE software generic calls go here */
};

/*
 *****
 **
 ** MODULE ACCESSIBLE API API
 **
 *****
 */

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_soft_gen_log_dom = eina_log_domain_register("EvasSoftGeneric", EVAS_DEFAULT_LOG_COLOR);
   if(_evas_soft_gen_log_dom<0)
     {
       EINA_LOG_ERR("Evas SoftGen : Impossible to create a log domain for the software generic engine.\n");
       return 0;
     }
   em->functions = (void *)(&func);
   cpunum = eina_cpu_count();
   return 1;
}

static void
module_close(Evas_Module *em)
{
  eina_log_domain_unregister(_evas_soft_gen_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "software_generic",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_generic);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_GENERIC
EVAS_EINA_MODULE_DEFINE(engine, software_generic);
#endif
