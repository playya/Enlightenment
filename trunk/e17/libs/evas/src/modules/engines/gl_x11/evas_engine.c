#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_GL_X11.h"
#include "evas_gl_common.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static Visual *eng_best_visual_get(Display *disp, int screen);
static Colormap eng_best_colormap_get(Display *disp, int screen);
static int eng_best_depth_get(Display *disp, int screen);

static void eng_font_hinting_set(void *data, void *font, int hinting);
static int eng_font_hinting_can_hint(void *data, int hinting);

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_GL_X11_Window *win;
   int                 end;
};

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_GL_X11 *in;

   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;
   int eb, evb;

   info = (Evas_Engine_Info_GL_X11 *)in;
   if (!e->engine.data.output)
     {
	if (!glXQueryExtension(info->info.display, &eb, &evb)) return;
	re = calloc(1, sizeof(Render_Engine));
	if (!re) return;
	e->engine.data.output = re;
	re->win = eng_window_new(info->info.display,
				 info->info.drawable,
				 0 /* FIXME: screen 0 assumption */,
				 info->info.visual,
				 info->info.colormap,
				 info->info.depth,
				 e->output.w,
				 e->output.h);
	if (!re->win)
	  {
	     free(re);
	     e->engine.data.output = NULL;
	     return;
	  }

	evas_common_cpu_init();
	
	evas_common_blend_init();
	evas_common_image_init();
	evas_common_convert_init();
	evas_common_scale_init();
	evas_common_rectangle_init();
	evas_common_gradient_init();
	evas_common_polygon_init();
	evas_common_line_init();
	evas_common_font_init();
	evas_common_draw_init();
	evas_common_tilebuf_init();
     }
   else
     {
	re = e->engine.data.output;
	eng_window_free(re->win);
	re->win = eng_window_new(info->info.display,
				 info->info.drawable,
				 0,/* FIXME: screen 0 assumption */
				 info->info.visual,
				 info->info.colormap,
				 info->info.depth,
				 e->output.w,
				 e->output.h);
     }
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_free(re->win);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;
   evas_gl_common_context_resize(re->win->gl_context, w, h);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* smple bounding box */
   if (!re->win->draw.redraw)
     {
#if 0
	re->win->draw.x1 = x;
	re->win->draw.y1 = y;
	re->win->draw.x2 = x + w - 1;
	re->win->draw.y2 = y + h - 1;
#else
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
#endif
     }
   else
     {
	if (x < re->win->draw.x1) re->win->draw.x1 = x;
	if (y < re->win->draw.y1) re->win->draw.y1 = y;
	if ((x + w - 1) > re->win->draw.x2) re->win->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->win->draw.y2) re->win->draw.y2 = y + h - 1;
     }
   re->win->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
//   printf("GL: finish update cycle!\n");
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* get the upate rect surface - return engine data as dummy */
   if (!re->win->draw.redraw)
     {
//	printf("GL: NO updates!\n");
	return NULL;
     }
//   printf("GL: update....!\n");
   if (x) *x = re->win->draw.x1;
   if (y) *y = re->win->draw.y1;
   if (w) *w = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (h) *h = re->win->draw.y2 - re->win->draw.y1 + 1;
   if (cx) *cx = re->win->draw.x1;
   if (cy) *cy = re->win->draw.y1;
   if (cw) *cw = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (ch) *ch = re->win->draw.y2 - re->win->draw.y1 + 1;
   return re;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
//   printf("GL: update done.\n");
   re->win->draw.redraw = 0;
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
//   printf("GL: flush your mush!\n");
   eng_window_use(re->win);

/* SLOW AS ALL HELL! */
#if 0
   evas_gl_common_swap_rect(re->win->gl_context,
			    re->win->draw.x1, re->win->draw.y1,
			    re->win->draw.x2 - re->win->draw.x1 + 1,
			    re->win->draw.y2 - re->win->draw.y1 + 1);
#else
#if 0
   glFlush();
     {
	unsigned int rc;
   
	glXGetVideoSyncSGI(&rc);
	glXWaitVideoSyncSGI(2, (rc + 1) % 2, &rc);
     }
#endif   
   glXSwapBuffers(re->win->disp, re->win->win);
#endif
//   glFlush();
//   glXWaitGL();
//   XSync(re->win->disp, False);
//   printf("SYNC! %i\n", fr++);
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in gl engine */
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in gl engine */
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
   evas_gl_common_rect_draw(re->win->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->gl_context->dc = context;
   evas_gl_common_line_draw(re->win->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_point_add(polygon, x, y);

}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->gl_context->dc = context;
   evas_gl_common_poly_draw(re->win->gl_context, polygon);
}

static void *
eng_gradient_new(void *data)
{
   return evas_gl_common_gradient_new();
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   evas_gl_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   evas_gl_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   evas_gl_common_gradient_clear(gradient);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   evas_gl_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   evas_gl_common_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_free(void *data, void *gradient)
{
   evas_gl_common_gradient_free(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   evas_gl_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   evas_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   evas_gl_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   evas_gl_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   evas_gl_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   evas_gl_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   evas_gl_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->win->gl_context->dc = context;
   return evas_gl_common_gradient_is_opaque(re->win->gl_context, gradient, x, y, w, h);
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->win->gl_context->dc = context;
   return evas_gl_common_gradient_is_visible(re->win->gl_context, gradient, x, y, w, h);
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
   Render_Engine *re = (Render_Engine *)data;

   re->win->gl_context->dc = context;
   evas_gl_common_gradient_render_pre(re->win->gl_context, gradient);
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
   evas_gl_common_gradient_render_post(gradient);
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
   evas_gl_common_gradient_draw(re->win->gl_context, gradient, x, y, w, h);
}

static void *
eng_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = 0;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (w) *w = ((Evas_GL_Image *)image)->im->image->w;
   if (h) *h = ((Evas_GL_Image *)image)->im->image->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im_old;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   if (!image) return NULL;
   im_old = image;
   if ((im_old) && (im_old->im->image->w == w) && (im_old->im->image->h == h))
     return image;
   im = evas_gl_common_image_new(re->win->gl_context, w, h);
   if (im_old)
     {
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
	evas_gl_common_image_free(im_old);
     }
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_image_dirty(image);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   evas_common_load_image_data_from_file(im->im);
   if (to_write)
     {
	if (im->references > 1)
	  {
	     Evas_GL_Image *im_new;

	     im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->image->w, im->im->image->h, im->im->image->data);
	     if (!im_new)
	       {
		  return im;
		  *image_data = NULL;
	       }
	     im = im_new;
	  }
	else
	  evas_gl_common_image_dirty(im);
     }
   *image_data = im->im->image->data;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   if (image_data != im->im->image->data)
     {
	int w, h;

	w = im->im->image->w;
	h = im->im->image->h;
	evas_gl_common_image_free(im);
	return eng_image_new_from_data(data, w, h, image_data);
     }
   /* hmmm - but if we wrote... why bother? */
   evas_gl_common_image_dirty(im);
   return im;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   im = image;
   if ((has_alpha) && (im->im->flags & RGBA_IMAGE_HAS_ALPHA)) return image;
   else if ((!has_alpha) && (!(im->im->flags & RGBA_IMAGE_HAS_ALPHA))) return image;
   if (im->references > 1)
     {
	Evas_GL_Image *im_new;

	im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->image->w, im->im->image->h, im->im->image->data);
	if (!im_new) return im;
	evas_gl_common_image_free(im);
	im = im_new;
     }
   else
     evas_gl_common_image_dirty(im);
   if (has_alpha)
     im->im->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
     im->im->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   return image;
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   if (im->im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
   return 0;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   re->win->gl_context->dc = context;
   evas_gl_common_image_draw(re->win->gl_context, image,
			     src_x, src_y, src_w, src_h,
			     dst_x, dst_y, dst_w, dst_h,
			     smooth);
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   return EVAS_COLORSPACE_ARGB8888;
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
}

static void *
eng_image_native_get(void *data, void *image)
{
   return NULL;
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
     {
	static RGBA_Image *im = NULL;

	if (!im)
	  {
	     im = evas_common_image_new();
	     im->image = evas_common_image_surface_new(im);
	     im->image->no_free = 1;
	  }
	im->image->w = re->win->w;
	im->image->h = re->win->h;
	im->image->data = NULL;
	evas_common_draw_context_font_ext_set(context,
					      re->win->gl_context,
					      evas_gl_font_texture_new,
					      evas_gl_font_texture_free,
					      evas_gl_font_texture_draw);
	evas_common_font_draw(im, context, font, x, y, text);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
}

/* private engine functions the calling prog can use */

static Visual *
eng_best_visual_get(Display *disp, int screen)
{
   if (!disp) return NULL;
   if (!_evas_gl_x11_vi)
     _evas_gl_x11_vi = glXChooseVisual(disp, screen,
				       _evas_gl_x11_configuration);
   if (!_evas_gl_x11_vi) return NULL;
   return _evas_gl_x11_vi->visual;
}

static Colormap
eng_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi)
     eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   _evas_gl_x11_cmap = XCreateColormap(disp, RootWindow(disp, screen),
				_evas_gl_x11_vi->visual, 0);
   return _evas_gl_x11_cmap;
}

static int
eng_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi)
     eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   return _evas_gl_x11_vi->depth;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(output_flush);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);
   ORD(gradient_new);
   ORD(gradient_free);
   ORD(gradient_color_stop_add);
   ORD(gradient_alpha_stop_add);
   ORD(gradient_color_data_set);
   ORD(gradient_alpha_data_set);
   ORD(gradient_clear);
   ORD(gradient_fill_set);
   ORD(gradient_fill_angle_set);
   ORD(gradient_fill_spread_set);
   ORD(gradient_angle_set);
   ORD(gradient_offset_set);
   ORD(gradient_direction_set);
   ORD(gradient_type_set);
   ORD(gradient_is_opaque);
   ORD(gradient_is_visible);
   ORD(gradient_render_pre);
   ORD(gradient_render_post);
   ORD(gradient_draw);
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(font_draw);
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_ENGINE,
     "gl_x11",
     "none"
};
