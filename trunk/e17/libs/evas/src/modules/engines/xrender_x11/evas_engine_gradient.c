#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"
#include <math.h>

XR_Gradient *
_xre_gradient_new(Ximage_Info *xinf)
{
   XR_Gradient  *gr;

   if (!xinf) return NULL;
   gr = calloc(1, sizeof(XR_Gradient));
   if (!gr) return NULL;
   gr->grad = evas_common_gradient_new();
   if (!gr->grad)
     {
	free(gr);
	return NULL;
     }
   gr->xinf = xinf;
   gr->xinf->references++;
   gr->changed = 1;
   return gr;
}

void
_xre_gradient_free(XR_Gradient *gr)
{
   if (!gr) return;
   if (gr->grad)
	evas_common_gradient_free(gr->grad);
   if (gr->surface)
	_xr_render_surface_free(gr->surface);
   _xr_image_info_free(gr->xinf);
   free(gr);
}

void
_xre_gradient_color_stop_add(XR_Gradient *gr, int r, int g, int b, int a, int delta)
{
   if (!gr) return;
   evas_common_gradient_color_stop_add(gr->grad, r, g, b, a, delta);
   gr->changed = 1;
}

void
_xre_gradient_alpha_stop_add(XR_Gradient *gr, int a, int delta)
{
   if (!gr) return;
   evas_common_gradient_alpha_stop_add(gr->grad, a, delta);
   gr->changed = 1;
}

void
_xre_gradient_clear(XR_Gradient *gr)
{
   if (!gr) return;
   evas_common_gradient_clear(gr->grad);
   gr->changed = 1;
}

void
_xre_gradient_color_data_set(XR_Gradient *gr, void *map, int len, int has_alpha)
{
   evas_common_gradient_color_data_set(gr->grad, map, len, has_alpha);
   gr->changed = 1;
}

void
_xre_gradient_alpha_data_set(XR_Gradient *gr, void *amap, int len)
{
   evas_common_gradient_alpha_data_set(gr->grad, amap, len);
   gr->changed = 1;
}

void
_xre_gradient_fill_set(XR_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   evas_common_gradient_fill_set(gr->grad, x, y, w, h);
   gr->changed = 1;
}

void
_xre_gradient_fill_angle_set(XR_Gradient *gr, double angle)
{
   if (!gr) return;
   evas_common_gradient_fill_angle_set(gr->grad, angle);
   gr->changed = 1;
}

void
_xre_gradient_fill_spread_set(XR_Gradient *gr, int spread)
{
   if (!gr) return;
   evas_common_gradient_fill_spread_set(gr->grad, spread);
   gr->changed = 1;
}

void
_xre_gradient_angle_set(XR_Gradient *gr, double angle)
{
   if (!gr) return;
   evas_common_gradient_map_angle_set(gr->grad, angle);
   gr->changed = 1;
}

void
_xre_gradient_offset_set(XR_Gradient *gr, float offset)
{
   if (!gr) return;
   evas_common_gradient_map_offset_set(gr->grad, offset);
   gr->changed = 1;
}

void
_xre_gradient_direction_set(XR_Gradient *gr, int direction)
{
   if (!gr) return;
   evas_common_gradient_map_direction_set(gr->grad, direction);
   gr->changed = 1;
}

void
_xre_gradient_type_set(XR_Gradient *gr, char *name, char *params)
{
   if (!gr) return;
   evas_common_gradient_type_set(gr->grad, name, params);
   gr->changed = 1;
}

void
_xre_gradient_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h)
{
   int alpha = 0;

   if ((w < 1) || (h < 1)) return;
   if (!rs || !dc || !gr) return;
   if (!gr->xinf || !gr->grad || !gr->grad->type.geometer) return;

   if (gr->grad->type.geometer->has_alpha(gr->grad, dc->render_op) ||
	gr->grad->type.geometer->has_mask(gr->grad, dc->render_op))
	alpha = 1;
   if (((gr->sw != w) || (gr->sh != h)) && gr->surface)
     {
	_xr_render_surface_free(gr->surface);
	gr->surface = NULL;
	gr->changed = 1;
     }
   if (!gr->surface)
     {
	gr->surface = _xr_render_surface_new(gr->xinf, w, h, gr->xinf->fmt32, 1);
	if (!gr->surface) return;
	gr->changed = 1;
     }
   if (gr->changed)
     {
	int op = dc->render_op, cuse = dc->clip.use;
	RGBA_Image    *im;
	Ximage_Image  *xim;

	im = evas_common_image_new();
	if (!im) 
	  {
	    _xr_render_surface_free(gr->surface);
	    gr->surface = NULL;
	    return;
	  }
	im->image = evas_common_image_surface_new(im);
	if (!im->image) 
	  {
	    evas_common_image_free(im);
	    _xr_render_surface_free(gr->surface);
	    gr->surface = NULL;
	    return;
	  }
	xim = _xr_image_new(gr->xinf, w, h, gr->surface->depth);
	if (!xim)
	  {
	    evas_common_image_free(im);
	    _xr_render_surface_free(gr->surface);
	    gr->surface = NULL;
	    return;
	  }
	im->image->data = (DATA32 *)xim->data;
	im->image->w = w;  im->image->h = h;
	im->image->no_free = 1;
	dc->render_op = _EVAS_RENDER_FILL;
	dc->clip.use = 0;
	evas_common_gradient_draw(im, dc, 0, 0, w, h, gr->grad);
	if
#ifdef WORDS_BIGENDIAN
	   (xim->xim->byte_order == LSBFirst)
#else
	   (xim->xim->byte_order == MSBFirst)
#endif
	  {
	     DATA32  *p = im->image->data, *pe = p + (w * h);
	     while (p < pe)
	       {
		  *p = (*p << 24) + ((*p << 8) & 0xff0000) + ((*p >> 8) & 0xff00) + (*p >> 24);
		  p++;
	       }
	  }
	_xr_image_put(xim, gr->surface->draw, 0, 0, w, h);
	evas_common_image_free(im);
	dc->render_op = op;
	dc->clip.use = cuse;
     }
   gr->surface->alpha = alpha;
   _xr_render_surface_composite(gr->surface, rs, dc, 0, 0, gr->surface->w, gr->surface->h, x, y, w, h, 0);
   gr->changed = 0;
   gr->sw = w;  gr->sh = h;
}
