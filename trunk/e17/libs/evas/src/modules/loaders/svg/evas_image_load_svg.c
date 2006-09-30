#include "evas_common.h"
#include "evas_private.h"
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

int evas_image_load_file_head_svg(RGBA_Image *im, const char *file, const char *key);
int evas_image_load_file_data_svg(RGBA_Image *im, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_svg_func =
{
  evas_image_load_file_head_svg,
  evas_image_load_file_data_svg
};

static int  rsvg_initialized = 0;
static void svg_loader_unpremul_data(DATA32 *data, unsigned int len);

static void
svg_loader_unpremul_data(DATA32 *data, unsigned int len)
{
   DATA32  *de = data + len;

   while (data < de)
     {
	DATA32   a = A_VAL(data);
	
	if (a && (a < 255))
	  {
	     R_VAL(data) = (R_VAL(data) * 255) / a;
	     G_VAL(data) = (G_VAL(data) * 255) / a;
	     B_VAL(data) = (B_VAL(data) * 255) / a;
	  }
	data++;
     }
}

int
evas_image_load_file_head_svg(RGBA_Image *im, const char *file, const char *key)
{
   char               cwd[PATH_MAX], pcwd[PATH_MAX], *p;
   
   RsvgHandle         *rsvg;
   RsvgDimensionData   dim;
   int                 w, h;
   
   if (!file) return 0;
   
   getcwd(pcwd, sizeof(pcwd));
   strncpy(cwd, file, sizeof(cwd) - 1);
   cwd[sizeof(cwd) - 1] = 0;
   p = strrchr(cwd, '/');
   if (p) *p = 0;
   chdir(cwd);
   
   rsvg = rsvg_handle_new_from_file(file, NULL);
   if (!rsvg)
     {
	chdir(pcwd);
	return 0;
     }
   if (!im->image)
     {
	im->image = evas_common_image_surface_new(im);
	if (!im->image)
	  {
	     rsvg_handle_free(rsvg);
	     chdir(pcwd);
	     return 0;
	  }
     }

   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if (im->load_opts.scale_down_by > 1)
     {
	w /= im->load_opts.scale_down_by;
	h /= im->load_opts.scale_down_by;
     }
   else if (im->load_opts.dpi > 0.0)
     {
	w = (w * im->load_opts.dpi) / 90.0;
	h = (h * im->load_opts.dpi) / 90.0;
     }
   else if ((im->load_opts.w > 0) &&
	    (im->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = im->load_opts.w;
	h2 = (im->load_opts.w * h) / w;
	if (h2 > im->load_opts.h)
	  {
	     h2 = im->load_opts.h;
	     w2 = (im->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   im->image->w = w;
   im->image->h = h;
   im->flags |= RGBA_IMAGE_HAS_ALPHA;
   rsvg_handle_free(rsvg);
   chdir(pcwd);
   return 1;
}

/** FIXME: All evas loaders need to be tightened up **/
int
evas_image_load_file_data_svg(RGBA_Image *im, const char *file, const char *key)
{
   char               cwd[PATH_MAX], pcwd[PATH_MAX], *p;
   
   RsvgHandle         *rsvg;
   RsvgDimensionData   dim;
   int                 w, h;

   cairo_surface_t    *surface;
   cairo_t            *cr;

   if (!file) return 0;
   if (!im->image) return 0;

   getcwd(pcwd, sizeof(pcwd));
   strncpy(cwd, file, sizeof(cwd) - 1);
   cwd[sizeof(cwd) - 1] = 0;
   p = strrchr(cwd, '/');
   if (p) *p = 0;
   chdir(cwd);
   
   rsvg = rsvg_handle_new_from_file(file, NULL);
   if (!rsvg)
     {
	evas_common_image_surface_free(im->image);
	im->image = NULL;
	chdir(pcwd);
	return 0;
     }

   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if (im->load_opts.scale_down_by > 1)
     {
	w /= im->load_opts.scale_down_by;
	h /= im->load_opts.scale_down_by;
     }
   else if (im->load_opts.dpi > 0.0)
     {
	w = (w * im->load_opts.dpi) / 90.0;
	h = (h * im->load_opts.dpi) / 90.0;
     }
   else if ((im->load_opts.w > 0) &&
	    (im->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = im->load_opts.w;
	h2 = (im->load_opts.w * h) / w;
	if (h2 > im->load_opts.h)
	  {
	     h2 = im->load_opts.h;
	     w2 = (im->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   im->image->w = w;
   im->image->h = h;
   im->flags |= RGBA_IMAGE_HAS_ALPHA;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_surface_free(im->image);
	im->image = NULL;
	rsvg_handle_free(rsvg);
	chdir(pcwd);
	return 0;
     }

   memset(im->image->data, 0, w * h * sizeof(DATA32));
   
   surface = cairo_image_surface_create_for_data((unsigned char *)im->image->data, CAIRO_FORMAT_ARGB32,
						 w, h, w * sizeof(DATA32));
   if (!surface)
     {
	evas_common_image_surface_free(im->image);
	im->image = NULL;
	rsvg_handle_free(rsvg);
	chdir(pcwd);
	return 0;
     }
   cr = cairo_create(surface);
   if (!cr)
     {
	cairo_surface_destroy(surface);
	evas_common_image_surface_free(im->image);
	im->image = NULL;
	rsvg_handle_free(rsvg);
	chdir(pcwd);
	return 0;
     }
   
   cairo_scale(cr, 
	       (double)im->image->w / dim.em, 
	       (double)im->image->h / dim.ex);
   rsvg_handle_render_cairo(rsvg, cr);
   cairo_surface_destroy(surface);
   /* need to check if this is required... */
   cairo_destroy(cr);
   rsvg_handle_free(rsvg);
   chdir(pcwd);
   evas_common_image_set_alpha_sparse(im);
   return 1;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_svg_func);
   if (!rsvg_initialized) rsvg_init();
   rsvg_initialized = 1;
   return 1;
}

EAPI void
module_close(void)
{
   if (!rsvg_initialized) return;
   rsvg_term();
   rsvg_initialized = 0;
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "svg",
     "none"
};

