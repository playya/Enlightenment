#include "evas_gl_private.h"

static const GLenum rgba_fmt   = GL_RGBA;
static const GLenum rgba_ifmt  = GL_RGBA;
//#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
//static const GLenum rgb_fmt    = GL_RGBA;
//static const GLenum rgb_ifmt   = GL_RGBA;
//#else
static const GLenum rgb_fmt    = GL_RGBA;
static const GLenum rgb_ifmt   = GL_RGB;
//#endif
#ifdef GL_BGRA
# if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_BGRA;
static const GLenum bgr_fmt    = GL_BGRA;
static const GLenum bgr_ifmt   = GL_BGRA;
# else
static const GLenum bgra_fmt   = GL_BGRA;
static const GLenum bgra_ifmt  = GL_RGBA;
static const GLenum bgr_fmt    = GL_BGRA;
static const GLenum bgr_ifmt   = GL_RGB;
# endif
#endif
static const GLenum alpha_fmt  = GL_ALPHA;
static const GLenum alpha_ifmt = GL_ALPHA;
static const GLenum lum_fmt    = GL_LUMINANCE;
static const GLenum lum_ifmt   = GL_LUMINANCE;

static int
_nearest_pow2(int num)
{
   unsigned int n = num - 1;
   n |= n >> 1;
   n |= n >> 2;
   n |= n >> 4;
   n |= n >> 8;
   n |= n >> 16;
   return n + 1;
}

static void
_tex_adjust(Evas_GL_Context *gc, int *w, int *h)
{
   if (gc->shared->info.tex_npo2) return;
   /*if (gc->shared->info.tex_rect) return;*/
   *w = _nearest_pow2(*w);
   *h = _nearest_pow2(*h);
}

static int
_tex_round_slot(Evas_GL_Context *gc, int h)
{
   if (!gc->shared->info.tex_npo2)
     h = _nearest_pow2(h);
   return (h + 15) >> 4;
}

static int
_tex_format_index(GLuint format)
{
   switch (format)
     {
     case GL_RGBA:
#ifdef GL_BGRA
     case GL_BGRA:
#endif        
        return 0;
     case GL_RGB:
        return 1;
     case GL_ALPHA:
        return 2;
     case GL_LUMINANCE:
        return 3;
     default:
        return 0;
     }
   return 0;
}

static void
_tex_2d(int intfmt, int w, int h, int fmt, int type)
{
   glTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, fmt, type, NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
}

static void
_tex_sub_2d(int x, int y, int w, int h, int fmt, int type, const void *pix)
{
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, type, pix);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
}

static Evas_GL_Texture_Pool *
_pool_tex_new(Evas_GL_Context *gc, int w, int h, int intformat, int format)
{
   Evas_GL_Texture_Pool *pt;
   
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   h = _tex_round_slot(gc, h) << 4;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->references = 0;
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_2d(pt->intformat, w, h, pt->format, pt->dataformat);
   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   return pt;
}

static int
_pool_tex_alloc(Evas_GL_Texture_Pool *pt, int w, int h, int *u, int *v, Eina_List **l_after)
{
   Eina_List *l;
   Evas_GL_Texture *tex, *tex2;
   int nx, d, b;

   if (pt->allocations)
     {
        tex = pt->allocations->data;
        // if firest tex is not at left edge...
        if (tex->x > (0 + 1))
          {
             if ((tex->x - 1) >= w)
               {
                  *u = 0;
                  *v = 0;
                  *l_after = NULL;
                  return 1;
               }
          }
     }
   EINA_LIST_FOREACH(pt->allocations, l, tex)
     {
        b = tex->x + tex->w + 2;
        if (l->next)
          {
             tex2 = l->next->data;
             nx = tex2->x - 1;
          }
        else
          nx = pt->w - 1;
        d = nx - b;
        if (d >= w)
          {
             *u = b;
             *v = 0;
             *l_after = l;
             return 1;
          }
     }
   *l_after = NULL;
   return 0;
}

static Evas_GL_Texture_Pool *
_pool_tex_find(Evas_GL_Context *gc, int w, int h, 
               int intformat, int format, int *u, int *v, 
               Eina_List **l_after, int atlas_w)
{
   Evas_GL_Texture_Pool *pt = NULL;
   Eina_List *l;
   int th, th2;
   
   if ((w > 512) || (h > 512))
     {
        pt = _pool_tex_new(gc, w, h, intformat, format);
        gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, pt);
        pt->slot = -1;
        pt->fslot = -1;
        pt->whole = 1;
        *u = 0;
        *v = 0;
        *l_after = NULL;
        return pt;
     }
   
   th = _tex_round_slot(gc, h);
   th2 = _tex_format_index(intformat);
   EINA_LIST_FOREACH(gc->shared->tex.atlas[th][th2], l, pt)
     {
        if (_pool_tex_alloc(pt, w, h, u, v, l_after))
          {
             gc->shared->tex.atlas[th][th2] = 
               eina_list_remove_list(gc->shared->tex.atlas[th][th2], l);
             gc->shared->tex.atlas[th][th2] = 
               eina_list_prepend(gc->shared->tex.atlas[th][th2], pt);
             return pt;
          }
     }
   pt = _pool_tex_new(gc, atlas_w, h, intformat, format);
   gc->shared->tex.atlas[th][th2] = 
     eina_list_prepend(gc->shared->tex.atlas[th][th2], pt);
   pt->slot = th;
   pt->fslot = th2;
   *u = 0;
   *v = 0;
   *l_after = NULL;
   return pt;
}

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;

   if (im->cache_entry.flags.alpha)
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_find(gc, im->cache_entry.w + 2,
                                   im->cache_entry.h + 1, bgra_ifmt, bgra_fmt, 
                                   &u, &v, &l_after, 1024);
        else
          tex->pt = _pool_tex_find(gc, im->cache_entry.w + 2,
                                   im->cache_entry.h + 1, rgba_ifmt, rgba_fmt, 
                                   &u, &v, &l_after, 1024);
        tex->alpha = 1;
     }
   else
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_find(gc, im->cache_entry.w + 3, 
                                 im->cache_entry.h + 1, bgr_ifmt, bgr_fmt,
                                 &u, &v, &l_after, 1024);
        else
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
          tex->pt = _pool_tex_find(gc, im->cache_entry.w + 3, 
                                 im->cache_entry.h + 1, rgba_ifmt, rgba_fmt,
                                 &u, &v, &l_after, 1024);
#else
          tex->pt = _pool_tex_find(gc, im->cache_entry.w + 3, 
                                 im->cache_entry.h + 1, rgb_ifmt, rgb_fmt,
                                 &u, &v, &l_after, 1024);
#endif
     }
   if (!tex->pt)
     {
        memset(tex, 0x11, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   tex->x = u + 1;
   tex->y = v;
   tex->w = im->cache_entry.w;
   tex->h = im->cache_entry.h;
   if (l_after)
     tex->pt->allocations = 
     eina_list_append_relative_list(tex->pt->allocations, tex, l_after);
   else
     tex->pt->allocations = 
     eina_list_prepend(tex->pt->allocations, tex);
   tex->pt->references++;
   evas_gl_common_texture_update(tex, im);
   return tex;
}

static Evas_GL_Texture_Pool *
_pool_tex_render_new(Evas_GL_Context *gc, int w, int h, int intformat, int format)
{
   Evas_GL_Texture_Pool *pt;
   
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   h = _tex_round_slot(gc, h) << 4;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->render = 1;
   pt->references = 0;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
# ifndef GL_COLOR_ATTACHMENT0
#  define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
# ifndef GL_COLOR_ATTACHMENT0
#  define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
# endif
#endif  
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_2d(pt->intformat, w, h, pt->format, pt->dataformat);
   
   glsym_glGenFramebuffers(1, &(pt->fb));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, pt->fb);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pt->texture, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   return pt;
}

static Evas_GL_Texture_Pool *
_pool_tex_native_new(Evas_GL_Context *gc, int w, int h, int intformat, int format, Evas_GL_Image *im)
{
   Evas_GL_Texture_Pool *pt;
   
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   pt->gc = gc;
#ifdef GL_TEXTURE_RECTANGLE_ARB   
   if (im->native.target == GL_TEXTURE_RECTANGLE_ARB)
     {
        printf("REEEEEEEEECT\n");
     }
   else
#endif     
     {
        // FIXME: handle po2 only textures
        pt->w = w;
        pt->h = h;
     }
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->references = 0;
   pt->native = 1;
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#else
   if (im->native.loose)
     {
        if (im->native.func.bind)
          im->native.func.bind(im->native.func.data, im);
     }
#endif
   
   glTexParameteri(im->native.target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(im->native.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(im->native.target, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   return pt;
}

static Evas_GL_Texture_Pool *
_pool_tex_dynamic_new(Evas_GL_Context *gc, int w, int h, int intformat, int format)
{
   Evas_GL_Texture_Pool *pt = NULL;
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   int fmt; // EGL_MAP_GL_TEXTURE_RGBA_SEC or EGL_MAP_GL_TEXTURE_RGB_SEC or bust
   int pixtype; // EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC or bust
   int attr[] =
     {
        EGL_MAP_GL_TEXTURE_WIDTH_SEC, 32,
        EGL_MAP_GL_TEXTURE_HEIGHT_SEC, 32,
        EGL_MAP_GL_TEXTURE_FORMAT_SEC, EGL_MAP_GL_TEXTURE_RGBA_SEC,
        EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC, EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC,
        EGL_NONE
     };
   void *egldisplay = pt->gc->egldisp;
   
   
   pt = calloc(1, sizeof(Evas_GL_Texture_Pool));
   if (!pt) return NULL;
   h = _tex_round_slot(gc, h) << 4;
   _tex_adjust(gc, &w, &h);
   pt->gc = gc;
   pt->w = w;
   pt->h = h;
   pt->intformat = intformat;
   pt->format = format;
   pt->dataformat = GL_UNSIGNED_BYTE;
   pt->render = 1;
   pt->references = 0;
   glGenTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   attr[1] = pt->w;
   attr[2] = pt->h;

   pt->dyn.img = secsym_eglCreateImage(egldisplay,
                                       EGL_NO_CONTEXT, 
                                       EGL_MAP_GL_TEXTURE_2D_SEC,
                                       0, attr);
   if (!pt->dyn.img)
     {
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   pt->dyn.data = secsym_eglMapImageSEC(egldisplay,
                                        pt->dyn.img);
   if (!pt->dyn.data)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_WIDTH_SEC,
                                   &(pt->dyn.w)) != EGL_TRUE)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_HEIGHT_SEC,
                                   &(pt->dyn.h)) != EGL_TRUE)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC,
                                   &(pt->dyn.stride)) != EGL_TRUE)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_FORMAT_SEC,
                                   &(fmt)) != EGL_TRUE)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (fmt != EGL_MAP_GL_TEXTURE_RGBA_SEC)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (secsym_eglGetImageAttribSEC(egldisplay,
                                   pt->dyn.img,
                                   EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC,
                                   &(pixtype)) != EGL_TRUE)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   if (pixtype != EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC)
     {
        secsym_eglDestroyImage(egldisplay, pt->dyn.img);
        pt->dyn.img = NULL;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        return pt;
     }
   
   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#endif  
   return pt;
}

static void
pt_unref(Evas_GL_Texture_Pool *pt)
{
   pt->references--;
   if (pt->references != 0) return;
   if (!((pt->render) || (pt->native)))
     {
        if (pt->whole)
          pt->gc->shared->tex.whole = eina_list_remove(pt->gc->shared->tex.whole, pt);
        else
          pt->gc->shared->tex.atlas [pt->slot][pt->fslot] =
          eina_list_remove(pt->gc->shared->tex.atlas[pt->slot][pt->fslot], pt);
     }
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (pt->dyn.img)
     {
        void *egldisplay = pt->gc->egldisp;
        
        secsym_eglDestroyImage(pt->gc->egldisp, pt->dyn.img);
        pt->dyn.img = NULL;
        pt->dyn.data = NULL;
        pt->dyn.w = 0;
        pt->dyn.h = 0;
        pt->dyn.stride = 0;
     }
#endif   
   
   glDeleteTextures(1, &(pt->texture));
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (pt->fb)
     {
        glsym_glDeleteFramebuffers(1, &(pt->fb));
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   memset(pt, 0x22, sizeof(Evas_GL_Texture_Pool)); // mark as freed
   free(pt);
}

Evas_GL_Texture *
evas_gl_common_texture_native_new(Evas_GL_Context *gc, int w, int h, int alpha, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   tex->alpha = alpha;
   if (alpha)
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_native_new(gc, w, h, rgba_ifmt, rgba_fmt, im);
        else
          tex->pt = _pool_tex_native_new(gc, w, h, rgba_ifmt, rgba_fmt, im);
     }
   else
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_native_new(gc, w, h, rgb_ifmt, rgb_fmt, im);
        else
          tex->pt = _pool_tex_native_new(gc, w, h, rgb_ifmt, rgb_fmt, im);
     }
   if (!tex->pt)
     {
        memset(tex, 0x33, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   tex->x = 0;
   tex->y = 0;
   tex->w = w;
   tex->h = h;
   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_render_new(Evas_GL_Context *gc, int w, int h, int alpha)
{
   Evas_GL_Texture *tex;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   tex->alpha = alpha;
   if (alpha)
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_render_new(gc, w, h, rgba_ifmt, rgba_fmt);
        else
          tex->pt = _pool_tex_render_new(gc, w, h, rgba_ifmt, rgba_fmt);
     }
   else
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_render_new(gc, w, h, rgb_ifmt, rgb_fmt);
        else
          tex->pt = _pool_tex_render_new(gc, w, h, rgb_ifmt, rgb_fmt);
     }
   if (!tex->pt)
     {
        memset(tex, 0x44, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   tex->x = 0;
   tex->y = 0;
   tex->w = w;
   tex->h = h;
   tex->pt->references++;
   return tex;
}

Evas_GL_Texture *
evas_gl_common_texture_dynamic_new(Evas_GL_Context *gc, Evas_GL_Image *im)
{
   Evas_GL_Texture *tex;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   tex->alpha = im->alpha;
   tex->x = 0;
   tex->y = 0;
   tex->w = im->w;
   tex->h = im->h;
   if (tex->alpha)
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_dynamic_new(gc, tex->w, tex->h, rgba_ifmt, rgba_fmt);
        else
          tex->pt = _pool_tex_dynamic_new(gc, tex->w, tex->h, rgba_ifmt, rgba_fmt);
     }
   else
     {
        if (gc->shared->info.bgra)
          tex->pt = _pool_tex_dynamic_new(gc, tex->w, tex->h, rgb_ifmt, rgb_fmt);
        else
          tex->pt = _pool_tex_dynamic_new(gc, tex->w, tex->h, rgb_ifmt, rgb_fmt);
     }
   if (!tex->pt)
     {
        memset(tex, 0x44, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   
   return tex;
}

void
evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im)
{
   GLuint fmt;
   
   if (tex->alpha != im->cache_entry.flags.alpha)
     {
        tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex);
        pt_unref(tex->pt);
        tex->alpha = im->cache_entry.flags.alpha;
        if (tex->alpha)
          {
             if (tex->gc->shared->info.bgra)
               tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h, bgra_ifmt, bgra_fmt);
             else
               tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h, rgba_ifmt, rgba_fmt);
          }
        else
          {
             if (tex->gc->shared->info.bgra)
               tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h, bgr_ifmt, bgr_fmt);
             else
               tex->pt = _pool_tex_render_new(tex->gc, tex->w, tex->h, rgb_ifmt, rgb_fmt);
          }
     }
   if (!tex->pt) return;
   fmt = tex->pt->format;
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_UNPACK_ROW_LENGTH   
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#endif   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   //  +-+
   //  +-+
   // 
   _tex_sub_2d(tex->x, tex->y, 
               im->cache_entry.w, im->cache_entry.h,
               fmt, tex->pt->dataformat,
               im->image.data);
   // |xxx
   // |xxx
   // 
   _tex_sub_2d(tex->x - 1, tex->y, 
               1, im->cache_entry.h,
               fmt, tex->pt->dataformat,
               im->image.data);
   //  xxx|
   //  xxx|
   // 
   _tex_sub_2d(tex->x + im->cache_entry.w, tex->y, 
               1, im->cache_entry.h,
               fmt, tex->pt->dataformat,
               im->image.data + (im->cache_entry.w - 1));
   //  xxx
   //  xxx
   //  ---
   _tex_sub_2d(tex->x, tex->y + im->cache_entry.h,
               im->cache_entry.w, 1,
               fmt, tex->pt->dataformat,
               im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w));
   //  xxx
   //  xxx
   // o
   _tex_sub_2d(tex->x - 1, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w));
   //  xxx
   //  xxx
   //     o
   _tex_sub_2d(tex->x + im->cache_entry.w, tex->y + im->cache_entry.h,
               1, 1,
               fmt, tex->pt->dataformat,
               im->image.data + ((im->cache_entry.h - 1) * im->cache_entry.w) + (im->cache_entry.w - 1));
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

void
evas_gl_common_texture_free(Evas_GL_Texture *tex)
{
   if (!tex) return;
   tex->references--;
   if (tex->references != 0) return;
   if (tex->pt)
     {
//        printf("tex->pt = %p\n", tex->pt);
//        printf("tex->pt->references = %i\n", tex->pt->references);
        tex->pt->allocations = eina_list_remove(tex->pt->allocations, tex);
        pt_unref(tex->pt);
     }
   if (tex->ptu) pt_unref(tex->ptu);
   if (tex->ptv) pt_unref(tex->ptv);
   memset(tex, 0x55, sizeof(Evas_GL_Texture)); // mark as freed
   free(tex);
}

Evas_GL_Texture *
evas_gl_common_texture_alpha_new(Evas_GL_Context *gc, DATA8 *pixels, 
                                 int w, int h, int fh)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;
   int tw = 4096;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   if (tw > gc->shared->info.max_texture_size)
     tw = gc->shared->info.max_texture_size;
   tex->pt = _pool_tex_find(gc, w + 3, fh, alpha_ifmt, alpha_fmt, &u, &v, 
                            &l_after, tw);
   if (!tex->pt)
     {
        memset(tex, 0x66, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   tex->x = u + 1;
   tex->y = v;
   tex->w = w;
   tex->h = h;
   if (l_after)
     tex->pt->allocations = 
     eina_list_append_relative_list(tex->pt->allocations, tex, l_after);
   else
     tex->pt->allocations = eina_list_prepend(tex->pt->allocations, tex);
   tex->pt->references++;
   evas_gl_common_texture_alpha_update(tex, pixels, w, h, fh);
   return tex;
}

void
evas_gl_common_texture_alpha_update(Evas_GL_Texture *tex, DATA8 *pixels, 
                                    int w, int h, int fh)
{
   if (!tex->pt) return;
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_UNPACK_ROW_LENGTH   
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#endif   
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_sub_2d(tex->x, tex->y, w, h, tex->pt->format, tex->pt->dataformat, 
               pixels);
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}

Evas_GL_Texture *
evas_gl_common_texture_yuv_new(Evas_GL_Context *gc, DATA8 **rows, int w, int h)
{
   Evas_GL_Texture *tex;
   Eina_List *l_after = NULL;
   int u = 0, v = 0;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tex->gc = gc;
   tex->references = 1;
   tex->pt = _pool_tex_new(gc, w + 1, h  + 1, lum_ifmt, lum_fmt);
   if (!tex->pt)
     {
        memset(tex, 0x77, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->pt);
   tex->pt->slot = -1;
   tex->pt->fslot = -1;
   tex->pt->whole = 1;
   tex->ptu = _pool_tex_new(gc, (w / 2) + 1, (h / 2)  + 1, lum_ifmt, lum_fmt);
   if (!tex->ptu)
     {
        pt_unref(tex->pt);
        memset(tex, 0x88, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptu);
   tex->ptu->slot = -1;
   tex->ptu->fslot = -1;
   tex->ptu->whole = 1;
   tex->ptv = _pool_tex_new(gc, (w / 2) + 1, (h / 2)  + 1, lum_ifmt, lum_fmt);
   if (!tex->ptv)
     {
        pt_unref(tex->pt);
        pt_unref(tex->ptu);
        memset(tex, 0x99, sizeof(Evas_GL_Texture)); // mark as freed
        free(tex);
        return NULL;
     }
   gc->shared->tex.whole = eina_list_prepend(gc->shared->tex.whole, tex->ptv);
   tex->ptv->slot = -1;
   tex->ptv->fslot = -1;
   tex->ptv->whole = 1;
   tex->x = 0;
   tex->y = 0;
   tex->w = w;
   tex->h = h;
   tex->pt->allocations = eina_list_prepend(tex->pt->allocations, tex);
   tex->ptu->allocations = eina_list_prepend(tex->ptu->allocations, tex);
   tex->ptv->allocations = eina_list_prepend(tex->ptv->allocations, tex);
   tex->pt->references++;
   tex->ptu->references++;
   tex->ptv->references++;
   evas_gl_common_texture_yuv_update(tex, rows, w, h);
   return tex;
}

void
evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, int w, int h)
{
   int y;

   if (!tex->pt) return;
   // FIXME: works on lowest size 4 pixel high buffers. must also be multiple of 2
#ifdef GL_UNPACK_ROW_LENGTH
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_sub_2d(0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
   glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + 1] - rows[h]);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_sub_2d(0, 0, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat, rows[h]);
   glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[h + (h / 2) + 1] - rows[h + (h / 2)]);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   _tex_sub_2d(0, 0, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2)]);
#else
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, tex->pt->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if ((rows[1] - rows[0]) == w)
     _tex_sub_2d(0, 0, w, h, tex->pt->format, tex->pt->dataformat, rows[0]);
   else
     {
        for (y = 0; y < h; y++)
          _tex_sub_2d(0, y, w, 1, tex->pt->format, tex->pt->dataformat, rows[y]);
     }

   glBindTexture(GL_TEXTURE_2D, tex->ptu->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if ((rows[h + 1] - rows[h]) == (w / 2))
     _tex_sub_2d(0, 0, w / 2, h / 2, tex->ptu->format, tex->ptu->dataformat, rows[h]);
   else
     {
        for (y = 0; y < (h / 2); y++)
          _tex_sub_2d(0, y, w / 2, 1, tex->ptu->format, tex->ptu->dataformat, rows[h + y]);
     }
   
   glBindTexture(GL_TEXTURE_2D, tex->ptv->texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if ((rows[h + (h / 2) + 1] - rows[h + (h / 2)]) == (w / 2))
     _tex_sub_2d(0, 0, w / 2, h / 2, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2)]);
   else
     {
        for (y = 0; y < (h / 2); y++)
          _tex_sub_2d(0, y, w / 2, 1, tex->ptv->format, tex->ptv->dataformat, rows[h + (h / 2) + y]);
     }
#endif   
   if (tex->pt->texture != tex->gc->pipe[0].shader.cur_tex)
     {
        glBindTexture(GL_TEXTURE_2D, tex->gc->pipe[0].shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
}
