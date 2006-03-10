#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_Xcb.h"
#include <math.h>

XR_Gradient *
_xre_gradient_color_add(XCBimage_Info *xcbinf, XR_Gradient *gr, int r, int g, int b, int a, int distance)
{
   if (!gr)
     {
	gr = calloc(1, sizeof(XR_Gradient));
	if (!gr) return NULL;
	gr->xcbinf = xcbinf;
	gr->xcbinf->references++;
	gr->grad = evas_common_gradient_new();
	if (!gr->grad)
	  {
	     gr->xcbinf->references--;
	     free(gr);
	     return NULL;
	  }
     }
   evas_common_gradient_color_add(gr->grad, r, g, b, a, distance);
   if (gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
     }
   gr->changed = 1;
   return gr;
}

XR_Gradient *
_xre_gradient_colors_clear(XR_Gradient *gr)
{
   if (!gr) return NULL;
   evas_common_gradient_colors_clear(gr->grad);
   if (gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
     }
   gr->changed = 1;
   return gr;
}

void
_xre_gradient_free(XR_Gradient *gr)
{
   if (!gr) return;
   if (gr->grad)
     {
	evas_common_gradient_free(gr->grad);
	gr->grad = NULL;
     }
   if (gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
     }
   _xr_image_info_free(gr->xcbinf);
   free(gr);
}

void
_xre_gradient_fill_set(XR_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   evas_common_gradient_fill_set(gr->grad, x, y, w, h);
   gr->changed = 1;
}

void
_xre_gradient_type_set(XR_Gradient *gr, char *name)
{
   if (!gr) return;
   evas_common_gradient_type_set(gr->grad, name);
   gr->changed = 1;
}

void
_xre_gradient_type_params_set(XR_Gradient *gr, char *params)
{
   if (!gr) return;
   evas_common_gradient_type_params_set(gr->grad, params);
   gr->changed = 1;
}

void *
_xre_gradient_geometry_init(XR_Gradient *gr, int spread)
{
   if (!gr) return NULL;
   gr->grad = evas_common_gradient_geometry_init(gr->grad, spread);
   return gr;
}

int
_xre_gradient_alpha_get(XR_Gradient *gr, int spread)
{
   if (!gr) return 0;
   return evas_common_gradient_has_alpha(gr->grad, spread);
}

void
_xre_gradient_map(RGBA_Draw_Context *dc, XR_Gradient *gr, int spread)
{
   if (!gr) return;
   evas_common_gradient_map(dc, gr->grad, spread);
   evas_common_cpu_end_opt();
   gr->changed = 1;
}

void
_xre_gradient_draw(XCBrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h, double angle, int spread)
{
   RGBA_Image *im;
   
   if ((w <= 0) || (h <= 0)) return;
   
   if ((angle != gr->angle) || (spread != gr->spread) || (gr->changed))
     {
	if (gr->surface)
	  {
	     _xr_render_surface_free(gr->surface);
	     gr->surface = NULL;
	  }
     }
   if (!gr->surface)
     {
	im = evas_common_image_create(w, h);
	if (im)
	  {
	     RGBA_Draw_Context *dc2;
	     
	     dc2 = evas_common_draw_context_new();
	     if (dc2)
	       {
		  im->flags |= RGBA_IMAGE_HAS_ALPHA;
		  memset(im->image->data, 0, im->image->w * im->image->h * sizeof(DATA32));
		  dc2->anti_alias = dc->anti_alias;
		  dc2->interpolation.color_space = dc->interpolation.color_space;
		  evas_common_gradient_draw(im, dc2, 0, 0, w, h, gr->grad, angle, spread);
		  gr->surface = _xr_render_surface_new(gr->xcbinf, w, h, gr->xcbinf->fmt32, 1);
		  if (gr->surface)
		    _xr_render_surface_argb_pixels_fill(gr->surface, w, h, im->image->data, 0, 0, w, h);
		  evas_common_draw_context_free(dc2);
		  gr->angle = angle;
		  gr->spread = spread;
	       }
	     evas_common_image_free(im);
	  }
     }
   if (gr->surface)
     _xr_render_surface_composite(gr->surface, rs, dc, 0, 0, gr->surface->w, gr->surface->h, x, y, w, h, 1);
   gr->changed = 0;
}
