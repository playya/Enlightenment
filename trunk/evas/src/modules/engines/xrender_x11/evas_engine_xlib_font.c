#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_XRender_X11.h"

static Eina_Hash *_xr_fg_pool = NULL;

XR_Font_Surface *
_xre_xlib_font_surface_new(Ximage_Info *xinf, RGBA_Font_Glyph *fg)
{
   XR_Font_Surface *fs;
   DATA8 *data;
   int w, h, j;
   XRenderPictureAttributes att;
   XRenderPictFormat *fmt;
   Ximage_Image  *xim;
   Eina_Hash *pool;
   char buf[256], buf2[256];

   data = fg->glyph_out->bitmap.buffer;
   w = fg->glyph_out->bitmap.width;
   h = fg->glyph_out->bitmap.rows;
   j = fg->glyph_out->bitmap.pitch;
   if (j < w) j = w;
   if ((w <= 0) || (h <= 0)) return NULL;

   if (fg->ext_dat)
     {
	fs = fg->ext_dat;
	if ((fs->xinf->x11.connection == xinf->x11.connection) && (fs->xinf->x11.root == xinf->x11.root))
	  return fs;
	snprintf(buf, sizeof(buf), "@%p@/@%lx@", fs->xinf->x11.connection, (unsigned long int)fs->xinf->x11.root);
	pool = eina_hash_find(_xr_fg_pool, buf);
	if (pool)
	  {
	     snprintf(buf, sizeof(buf), "%p", fg);
	     fs = eina_hash_find(pool, buf);
	     if (fs) return fs;
	  }
     }

   fs = calloc(1, sizeof(XR_Font_Surface));
   if (!fs) return NULL;

   fs->xinf = xinf;
   fs->fg = fg;
   fs->xinf->references++;
   fs->w = w;
   fs->h = h;

   snprintf(buf, sizeof(buf), "@%p@/@%lx@", fs->xinf->x11.connection, (unsigned long int)fs->xinf->x11.root);
   pool = eina_hash_find(_xr_fg_pool, buf);
   if (!pool) pool = eina_hash_string_superfast_new(NULL);
   snprintf(buf2, sizeof(buf2), "%p", fg);
   eina_hash_add(pool, buf2, fs);
   if (!_xr_fg_pool) _xr_fg_pool = eina_hash_string_superfast_new(NULL);
   eina_hash_add(_xr_fg_pool, buf, pool);

   /* FIXME: maybe use fmt4? */
   fmt = xinf->x11.fmt8;
   fs->draw = XCreatePixmap(xinf->x11.connection, xinf->x11.root, w, h,fmt->depth);
   att.dither = 0;
   att.component_alpha = 0;
   att.repeat = 0;
   fs->pic = XRenderCreatePicture(xinf->x11.connection, fs->draw,fmt,
				  CPRepeat | CPDither | CPComponentAlpha, &att);

   /* FIXME: handle if fmt->depth != 8 */
   xim = _xr_xlib_image_new(fs->xinf, w, h,fmt->depth);
   if ((fg->glyph_out->bitmap.num_grays == 256) &&
       (fg->glyph_out->bitmap.pixel_mode == ft_pixel_mode_grays))
     {
	int x, y;
	DATA8 *p1, *p2;

	for (y = 0; y < h; y++)
	  {
	     p1 = data + (j * y);
	     p2 = ((DATA8 *)xim->data) + (xim->line_bytes * y);
	     for (x = 0; x < w; x++)
	       {
		  *p2 = *p1;
		  p1++;
		  p2++;
	       }
	  }

     }
   else
     {
        DATA8 *tmpbuf = NULL, *dp, *tp, bits;
	int bi, bj, end;
	const DATA8 bitrepl[2] = {0x0, 0xff};

	tmpbuf = alloca(w);
	  {
	     int x, y;
	     DATA8 *p1, *p2;

	     for (y = 0; y < h; y++)
	       {
		  p1 = tmpbuf;
		  p2 = ((DATA8 *)xim->data) + (xim->line_bytes * y);
		  tp = tmpbuf;
		  dp = data + (y * fg->glyph_out->bitmap.pitch);
		  for (bi = 0; bi < w; bi += 8)
		    {
		       bits = *dp;
		       if ((w - bi) < 8) end = w - bi;
		       else end = 8;
		       for (bj = 0; bj < end; bj++)
			 {
			    *tp = bitrepl[(bits >> (7 - bj)) & 0x1];
			    tp++;
			 }
		       dp++;
		    }
		  for (x = 0; x < w; x++)
		    {
		       *p2 = *p1;
		       p1++;
		       p2++;
		    }
	       }
	  }
     }
   _xr_xlib_image_put(xim, fs->draw, 0, 0, w, h);
   return fs;
}

static Eina_Bool
_xre_xlib_font_pool_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   char buf[256];
   Eina_Hash *pool;
   XR_Font_Surface *fs;

   fs = fdata;
   pool = data;
   snprintf(buf, sizeof(buf), "@%p@/@%lx@", fs->xinf->x11.connection, (unsigned long int)fs->xinf->x11.root);
   eina_hash_del(pool, buf, fs);
   return 1;
}

void
_xre_xlib_font_surface_free(XR_Font_Surface *fs)
{
   if (!fs) return;
   eina_hash_foreach(_xr_fg_pool, _xre_xlib_font_pool_cb, fs);
   XFreePixmap(fs->xinf->x11.connection, fs->draw);
   XRenderFreePicture(fs->xinf->x11.connection, fs->pic);
   _xr_xlib_image_info_free(fs->xinf);
   free(fs);
}

void
_xre_xlib_font_surface_draw(Ximage_Info *xinf __UNUSED__, RGBA_Image *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y)
{
   XR_Font_Surface *fs;
   Xrender_Surface *target_surface;
   XRectangle rect;
   int r, g, b, a;

   fs = fg->ext_dat;
   if (!fs || !fs->xinf || !dc || !dc->col.col) return;
   if (!surface || !surface->image.data) return;
   target_surface = (Xrender_Surface *)(surface->image.data);
   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   if ((fs->xinf->mul_r != r) || (fs->xinf->mul_g != g) ||
       (fs->xinf->mul_b != b) || (fs->xinf->mul_a != a))
     {
	fs->xinf->mul_r = r;
	fs->xinf->mul_g = g;
	fs->xinf->mul_b = b;
	fs->xinf->mul_a = a;
	_xr_xlib_render_surface_solid_rectangle_set(fs->xinf->mul, r, g, b, a, 0, 0, 1, 1);
     }
   rect.x = x; rect.y = y; rect.width = fs->w; rect.height = fs->h;
   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(rect.x, rect.y, rect.width, rect.height,
			   dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
     }
   XRenderSetPictureClipRectangles(target_surface->xinf->x11.connection,
				   target_surface->x11.xlib.pic, 0, 0, &rect, 1);
   XRenderComposite(fs->xinf->x11.connection, PictOpOver, fs->xinf->mul->x11.xlib.pic,
		    fs->pic, target_surface->x11.xlib.pic,
		    0, 0, 0, 0, x, y, fs->w, fs->h);
}
