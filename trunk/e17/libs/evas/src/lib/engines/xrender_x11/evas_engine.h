#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xrender.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct _Ximage_Info      Ximage_Info;
typedef struct _Ximage_Image     Ximage_Image;
typedef struct _Xrender_Surface  Xrender_Surface;

struct _Ximage_Info
{
   Display           *disp;
   Drawable           root;
   Drawable           draw;
   int                depth;
   Visual            *vis;
   int                pool_mem;
   Evas_List         *pool;
   unsigned char      can_do_shm;
   XRenderPictFormat *fmt32;
   XRenderPictFormat *fmt24;
   XRenderPictFormat *fmt8;
   XRenderPictFormat *fmt4;
   XRenderPictFormat *fmt1;
   unsigned char      mul_r, mul_g, mul_b, mul_a;
   Xrender_Surface   *mul;
   int                references;
};

struct _Ximage_Image
{
   Ximage_Info     *xinf;
   XImage          *xim;
   XShmSegmentInfo *shm_info;
   int              w, h;
   int              depth;
   int              line_bytes;
   unsigned char   *data;
   unsigned char    available : 1;
};

struct _Xrender_Surface
{
   Ximage_Info       *xinf;
   int                w, h;
   int                depth;
   XRenderPictFormat *fmt;
   Drawable           draw;
   Picture            pic;
   unsigned char      alpha : 1;
   unsigned char      allocated : 1;
};

/* ximage support calls (ximage vs xshmimage, cache etc.) */
Ximage_Info  *_xr_image_info_get(Display *disp, Drawable draw, Visual *vis);
void          _xr_image_info_free(Ximage_Info *xinf);
void          _xr_image_info_pool_flush(Ximage_Info *xinf, int max_num, int max_mem);
Ximage_Image *_xr_image_new(Ximage_Info *xinf, int w, int h, int depth);
void          _xr_image_free(Ximage_Image *xim);
void          _xr_image_put(Ximage_Image *xim, Drawable draw, int x, int y, int w, int h);

/* xrender support calls */
Xrender_Surface *_xr_render_surface_new(Ximage_Info *xinf, int w, int h, XRenderPictFormat *fmt, int alpha);
Xrender_Surface *_xr_render_surface_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, int alpha);
Xrender_Surface *_xr_render_surface_format_adopt(Ximage_Info *xinf, Drawable draw, int w, int h, XRenderPictFormat *fmt, int alpha);
void             _xr_render_surface_free(Xrender_Surface *rs);
void             _xr_render_surface_repeat_set(Xrender_Surface *rs, int repeat);
void             _xr_render_surface_solid_rectangle_set(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
void             _xr_render_surface_argb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void             _xr_render_surface_rgb_pixels_fill(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h);
void             _xr_render_surface_composite(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
void             _xr_render_surface_copy(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h);
void             _xr_render_surface_rectangle_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void             _xr_render_surface_line_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void             _xre_poly_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);
  
    
typedef struct _XR_Image XR_Image;

struct _XR_Image
{
   Ximage_Info     *xinf;
   char            *file;
   char            *key;
   char            *fkey;
   RGBA_Image      *im;
   void            *data;
   int              w, h;
   Xrender_Surface *surface;
   int              references;
   char            *format;
   char            *comment;
   Tilebuf         *updates;
   unsigned char    alpha : 1;
   unsigned char    dirty : 1;
   unsigned char    free_data : 1;
};

XR_Image *_xre_image_load(Ximage_Info *xinf, char *file, char *key);
XR_Image *_xre_image_new_from_data(Ximage_Info *xinf, int w, int h, void *data);
XR_Image *_xre_image_new_from_copied_data(Ximage_Info *xinf, int w, int h, void *data);
XR_Image *_xre_image_new(Ximage_Info *xinf, int w, int h);
void      _xre_image_free(XR_Image *im);
void      _xre_image_region_dirty(XR_Image *im, int x, int y, int w, int h);
void      _xre_image_dirty(XR_Image *im);
XR_Image *_xre_image_copy(XR_Image *im);
void     *_xre_image_data_get(XR_Image *im);
XR_Image *_xre_image_data_find(void *data);
void      _xre_image_data_put(XR_Image *im, void *data);
void      _xre_image_alpha_set(XR_Image *im, int alpha);
int       _xre_image_alpha_get(XR_Image *im);
void      _xre_image_surface_gen(XR_Image *im);
void      _xre_image_cache_set(int size);
int       _xre_image_cache_get(void);

typedef struct _XR_Font_Surface XR_Font_Surface;

struct _XR_Font_Surface
{
   Ximage_Info     *xinf;
   RGBA_Font_Glyph *fg;
   int              w, h;
   Drawable         draw;
   Picture          pic;
};

XR_Font_Surface *_xre_font_surface_new(Ximage_Info *xinf, RGBA_Font_Glyph *fg);
void             _xre_font_surface_free(XR_Font_Surface *fs);
void             _xre_font_surface_draw(Ximage_Info *xinf, RGBA_Image *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);

typedef struct _XR_Gradient XR_Gradient;

struct _XR_Gradient
{
   Ximage_Info     *xinf;
   Xrender_Surface *surface;
   RGBA_Gradient   *grad;
   double           angle;
};

XR_Gradient *_xre_gradient_color_add(Ximage_Info *xinf, XR_Gradient *gr, int r, int g, int b, int a, int distance);
XR_Gradient *_xre_gradient_colors_clear(XR_Gradient *gr);
void         _xre_gradient_draw(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h, double angle);
    
#endif
