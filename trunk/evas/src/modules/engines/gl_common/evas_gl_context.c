#include "evas_gl_private.h"

static int sym_done = 0;

void (*glsym_glGenFramebuffers)      (GLsizei a, GLuint *b) = NULL;
void (*glsym_glBindFramebuffer)      (GLenum a, GLuint b) = NULL;
void (*glsym_glFramebufferTexture2D) (GLenum a, GLenum b, GLenum c, GLuint d, GLint e) = NULL;
void (*glsym_glDeleteFramebuffers)   (GLsizei a, const GLuint *b) = NULL;

static void
sym_missing(void)
{
   printf("EVAS ERROR - GL symbols missing!\n");
}

static void
gl_symbols(void)
{
   if (sym_done) return;
   sym_done = 1;

#ifdef _EVAS_ENGINE_SDL_H
# define FINDSYM(dst, sym) if (!dst) dst = SDL_GL_GetProcAddress(sym)
#else
# define FINDSYM(dst, sym) if (!dst) dst = dlsym(RTLD_DEFAULT, sym)
#endif
#define FALLBAK(dst) if (!dst) dst = (void *)sym_missing;
   
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffers");
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersEXT");
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersARB");
   FALLBAK(glsym_glGenFramebuffers);
   
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebuffer");
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferEXT");
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferARB");
   FALLBAK(glsym_glBindFramebuffer);
   
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2D");
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DEXT");
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DARB");
   FALLBAK(glsym_glFramebufferTexture2D);

   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffers");
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersEXT");
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersARB");
   FALLBAK(glsym_glDeleteFramebuffers);
}

static void shader_array_flush(Evas_GL_Context *gc);

static Evas_GL_Context *_evas_gl_common_context = NULL;
static Evas_GL_Shared *shared = NULL;

void
glerr(int err, const char *file, const char *func, int line, const char *op)
{
   fprintf(stderr, "GLERR: %s:%i %s(), %s: ", file, line, func, op);
   switch (err)
     {
     case GL_INVALID_ENUM:
        fprintf(stderr, "GL_INVALID_ENUM\n");
        break;
     case GL_INVALID_VALUE:
        fprintf(stderr, "GL_INVALID_VALUE\n");
        break;
     case GL_INVALID_OPERATION:
        fprintf(stderr, "GL_INVALID_OPERATION\n");
        break;
     case GL_OUT_OF_MEMORY:
        fprintf(stderr, "GL_OUT_OF_MEMORY\n");
        break;
     default:
        fprintf(stderr, "0x%x\n", err);
     }
}

static void
matrix_ident(GLfloat *m)
{
   memset(m, 0, 16 * sizeof(GLfloat));
   m[0] = m[5] = m[10] = m[15] = 1.0;
}

static void
matrix_ortho(GLfloat *m, 
             GLfloat l, GLfloat r, 
             GLfloat t, GLfloat b, 
             GLfloat near, GLfloat far)
{
   m[0] = 2.0 / (r - l);
   m[1] = m[2] = m[3] = 0.0;
   
   m[4] = 0.0;
   m[5] = 2.0 / (t - b);
   m[6] = m[7] = 0.0;
   
   m[8] = m[9] = 0.0;
   m[10] = -(2.0 / (far - near));
   m[11] = 0.0;
   
   m[12] = -((r + l)/(r - l));
   m[13] = -((t + b)/(t - b));
   m[14] = -((near + far)/(far - near));
   m[15] = 1.0;
}

static void
_evas_gl_common_viewport_set(Evas_GL_Context *gc)
{
   GLfloat proj[16];
   int w = 1, h = 1, m = 1;

   if ((gc->shader.surface == gc->def_surface) ||
       (!gc->shader.surface))
     {
        w = gc->w;
        h = gc->h;
     }
   else
     {
        w = gc->shader.surface->w;
        h = gc->shader.surface->h;
        m = -1;
     }

   if ((!gc->change.size) || 
       ((gc->shared->w == w) && (gc->shared->h == h)))
     return;
   
   gc->shared->w = w;
   gc->shared->h = h;
   gc->change.size = 0;
   
   glViewport(0, 0, w, h);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
   matrix_ident(proj);
   if (m == 1) matrix_ortho(proj, 0, w, 0, h, -1.0, 1.0);
   else matrix_ortho(proj, 0, w, h, 0, -1.0, 1.0);
   
   glUseProgram(gc->shared->shader.rect.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.rect.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.font.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.font.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.yuv.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.yuv.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.tex.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.tex.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   
   glUseProgram(gc->shared->shader.img.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_nomul.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_nomul.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_solid.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_solid.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_solid_nomul.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_solid_nomul.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glUseProgram(gc->shared->shader.img_bgra.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_bgra.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_bgra_nomul.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_bgra_nomul.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_bgra_solid.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_bgra_solid.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->shared->shader.img_bgra_solid_nomul.prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader.img_bgra_solid_nomul.prog, "mvp"), 1,
                      GL_FALSE, proj);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glUseProgram(gc->shader.cur_prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
}

Evas_GL_Context *
evas_gl_common_context_new(void)
{
   Evas_GL_Context *gc;

#if 1
   if (_evas_gl_common_context)
     {
	_evas_gl_common_context->references++;
	return _evas_gl_common_context;
     }
#endif   
   gc = calloc(1, sizeof(Evas_GL_Context));
   if (!gc) return NULL;

   gl_symbols();
   
   gc->references = 1;
   
   _evas_gl_common_context = gc;

   if (!shared)
     {
        GLint linked;
        unsigned int pixel = 0xffffffff;
        const GLubyte *ext;

        shared = calloc(1, sizeof(Evas_GL_Shared));
        ext = glGetString(GL_EXTENSIONS);
        if (ext)
          {
             fprintf(stderr, "EXT:\n%s\n", ext);
             if ((strstr((char*) ext, "GL_ARB_texture_non_power_of_two")) ||
                 (strstr((char*) ext, "OES_texture_npot")) ||
                 (strstr((char*) ext, "GL_IMG_texture_npot")))
               shared->info.tex_npo2 = 1;
             if ((strstr((char*) ext, "GL_NV_texture_rectangle")) ||
                 (strstr((char*) ext, "GL_EXT_texture_rectangle")) ||
                 (strstr((char*) ext, "GL_ARB_texture_rectangle")))
               shared->info.tex_rect = 1;
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
             if ((strstr((char*) ext, "GL_EXT_texture_filter_anisotropic")))
               glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 
                           &(shared->info.anisotropic));
#endif
#ifdef GL_BGRA
             if ((strstr((char*) ext, "GL_EXT_bgra")) ||
                 (strstr((char*) ext, "GL_EXT_texture_format_BGRA8888")))
               shared->info.bgra = 1;
#endif             
          }
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,
                      &(shared->info.max_texture_units));
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,
                      &(shared->info.max_texture_size));
        
        fprintf(stderr, "max tex size %ix%i\n"
                "max units %i\n"
                "non-power-2 tex %i\n"
                "rect tex %i\n"
                "bgra : %i\n"
                "max ansiotropic filtering: %3.3f\n"
                , 
                shared->info.max_texture_size, shared->info.max_texture_size,
                shared->info.max_texture_units,
                (int)shared->info.tex_npo2,
                (int)shared->info.tex_rect,
                (int)shared->info.bgra,
                (double)shared->info.anisotropic
                );
        
        glDisable(GL_DEPTH_TEST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glEnable(GL_DITHER);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDisable(GL_BLEND);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        // no dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ???
        glDepthMask(GL_FALSE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
        if (shared->info.anisotropic > 0.0)
          {
             glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
#endif
        
        glEnableVertexAttribArray(SHAD_VERTEX);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glEnableVertexAttribArray(SHAD_COLOR);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        
        evas_gl_common_shader_program_init(&(shared->shader.img),
                                           &(shader_img_vert_src),
                                           &(shader_img_frag_src),
                                           "img");
        evas_gl_common_shader_program_init(&(shared->shader.img_nomul),
                                           &(shader_img_nomul_vert_src),
                                           &(shader_img_nomul_frag_src),
                                           "img_nomul");
        evas_gl_common_shader_program_init(&(shared->shader.img_solid),
                                           &(shader_img_solid_vert_src),
                                           &(shader_img_solid_frag_src),
                                           "img_solid");
        evas_gl_common_shader_program_init(&(shared->shader.img_solid_nomul),
                                           &(shader_img_solid_nomul_vert_src),
                                           &(shader_img_solid_nomul_frag_src),
                                           "img_solid_nomul");

        evas_gl_common_shader_program_init(&(shared->shader.img_bgra),
                                           &(shader_img_bgra_vert_src),
                                           &(shader_img_bgra_frag_src),
                                           "img_bgra");
        evas_gl_common_shader_program_init(&(shared->shader.img_bgra_nomul),
                                           &(shader_img_bgra_nomul_vert_src),
                                           &(shader_img_bgra_nomul_frag_src),
                                           "img_bgra_nomul");
        evas_gl_common_shader_program_init(&(shared->shader.img_bgra_solid),
                                           &(shader_img_bgra_solid_vert_src),
                                           &(shader_img_bgra_solid_frag_src),
                                           "img_bgra_solid");
        evas_gl_common_shader_program_init(&(shared->shader.img_bgra_solid_nomul),
                                           &(shader_img_bgra_solid_nomul_vert_src),
                                           &(shader_img_bgra_solid_nomul_frag_src),
                                           "img_bgra_solid_nomul");
        
        evas_gl_common_shader_program_init(&(shared->shader.rect), 
                                           &(shader_rect_vert_src), 
                                           &(shader_rect_frag_src),
                                           "rect");
        evas_gl_common_shader_program_init(&(shared->shader.font),
                                           &(shader_font_vert_src), 
                                           &(shader_font_frag_src),
                                           "font");
        evas_gl_common_shader_program_init(&(shared->shader.tex),
                                           &(shader_tex_vert_src), 
                                           &(shader_tex_frag_src),
                                           "tex");
        
        evas_gl_common_shader_program_init(&(shared->shader.yuv),
                                           &(shader_yuv_vert_src), 
                                           &(shader_yuv_frag_src),
                                           "yuv");
        glUseProgram(shared->shader.yuv.prog);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glUniform1i(glGetUniformLocation(shared->shader.yuv.prog, "tex"), 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glUniform1i(glGetUniformLocation(shared->shader.yuv.prog, "texu"), 1);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glUniform1i(glGetUniformLocation(shared->shader.yuv.prog, "texv"), 2);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glUseProgram(gc->shader.cur_prog);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        // in shader:
        // uniform sampler2D tex[8];
        // 
        // in code:
        // GLuint texes[8];
        // GLint loc = glGetUniformLocation(prog, "tex");
        // glUniform1iv(loc, 8, texes);
     }
   gc->shared = shared;
   gc->shared->references++;
   _evas_gl_common_viewport_set(gc);
   
   gc->def_surface = evas_gl_common_image_surface_new(gc, 1, 1, 1);
   
   return gc;
}

void
evas_gl_common_context_free(Evas_GL_Context *gc)
{
   int i, j;
   
   gc->references--;
   if (gc->references > 0) return;
   gc->shared->references--;
   
   evas_gl_common_image_free(gc->def_surface);
   
   if (gc->shared->references == 0)
     {
        while (gc->shared->images)
          {
             evas_gl_common_image_free(gc->shared->images->data);
          }
        while (gc->shared->tex.whole)
          {
             evas_gl_common_texture_free(gc->shared->tex.whole->data);
          }
        for (i = 0; i < 33; i++)
          {
             for (j = 0; j < 3; j++)
               {
                  while (gc->shared->tex.atlas[i][j])
                    evas_gl_common_texture_free
                    ((Evas_GL_Texture *)gc->shared->tex.atlas[i][j]);
               }
          }
        free(gc->shared);
        shared = NULL;
     }
   

   if (gc->array.vertex) free(gc->array.vertex);
   if (gc->array.color) free(gc->array.color);
   if (gc->array.texuv) free(gc->array.texuv);
   if (gc->array.texuv2) free(gc->array.texuv2);
   if (gc->array.texuv3) free(gc->array.texuv3);
   
   if (gc == _evas_gl_common_context) _evas_gl_common_context = NULL;
   free(gc);
}

void
evas_gl_common_context_use(Evas_GL_Context *gc)
{
//   if (_evas_gl_common_context == gc) return;
   _evas_gl_common_context = gc;
   _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h)
{
   if ((gc->w == w) && (gc->h == h)) return;
   gc->change.size = 1;
   gc->w = w;
   gc->h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_context_target_surface_set(Evas_GL_Context *gc,
                                          Evas_GL_Image *surface)
{
   if (surface == gc->shader.surface) return;
   
   evas_gl_common_context_flush(gc);

   gc->shader.surface = surface;
   gc->change.size = 1;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif   
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif   
#endif   
   if (gc->shader.surface == gc->def_surface)
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   else
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, surface->tex->pt->fb);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   _evas_gl_common_viewport_set(gc);
}

#define PUSH_VERTEX(x, y, z) \
   gc->array.vertex[nv++] = x; \
   gc->array.vertex[nv++] = y; \
   gc->array.vertex[nv++] = z
#define PUSH_COLOR(r, g, b, a) \
   gc->array.color[nc++] = r; \
   gc->array.color[nc++] = g; \
   gc->array.color[nc++] = b; \
   gc->array.color[nc++] = a
#define PUSH_TEXUV(u, v) \
   gc->array.texuv[nu++] = u; \
   gc->array.texuv[nu++] = v
#define PUSH_TEXUV2(u, v) \
   gc->array.texuv2[nu2++] = u; \
   gc->array.texuv2[nu2++] = v
#define PUSH_TEXUV3(u, v) \
   gc->array.texuv3[nu3++] = u; \
   gc->array.texuv3[nu3++] = v

static inline void
_evas_gl_common_context_array_alloc(Evas_GL_Context *gc)
{
   if (gc->array.num <= gc->array.alloc) return;
   gc->array.alloc += 6 * 1024;
   if (gc->array.use_vertex)
     gc->array.vertex = realloc(gc->array.vertex,
                                gc->array.alloc * sizeof(GLshort) * 3);
   if (gc->array.use_color)
     gc->array.color  = realloc(gc->array.color,
                                gc->array.alloc * sizeof(GLubyte) * 4);
   if (gc->array.use_texuv)
     gc->array.texuv  = realloc(gc->array.texuv,
                                gc->array.alloc * sizeof(GLfloat) * 2);
   if (gc->array.use_texuv2)
     gc->array.texuv2  = realloc(gc->array.texuv2,
                               gc->array.alloc * sizeof(GLfloat) * 2);
   if (gc->array.use_texuv3)
     gc->array.texuv3  = realloc(gc->array.texuv3,
                                 gc->array.alloc * sizeof(GLfloat) * 2);
}

void
evas_gl_common_context_line_push(Evas_GL_Context *gc, 
                                 int x1, int y1, int x2, int y2,
                                 int clip, int cx, int cy, int cw, int ch,
                                 int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   Eina_Bool blend = 0;
   GLuint prog = gc->shared->shader.rect.prog;
   
   shader_array_flush(gc);
   
   if (a < 255) blend = 1;
   if (gc->dc->render_op == EVAS_RENDER_COPY) blend = 0;
   gc->shader.cur_tex = 0;
   gc->shader.cur_prog = prog;
   gc->shader.blend = blend;
   gc->shader.render_op = gc->dc->render_op;
   gc->shader.clip = clip;
   gc->shader.cx = cx;
   gc->shader.cy = cy;
   gc->shader.cw = cw;
   gc->shader.ch = ch;
   
   gc->array.line = 1;
   gc->array.use_vertex = 1;
   gc->array.use_color = 1;
   gc->array.use_texuv = 0;
   gc->array.use_texuv2 = 0;
   gc->array.use_texuv3 = 0;
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 1;
   _evas_gl_common_context_array_alloc(gc);
  
   PUSH_VERTEX(x1, y1, 0);
   PUSH_VERTEX(x2, y2, 0);
   
   for (i = 0; i < 2; i++)
     {
        PUSH_COLOR(r, g, b, a);
     }
   
   shader_array_flush(gc);
   gc->array.line = 0;
   gc->array.use_vertex = 0;
   gc->array.use_color = 0;
   gc->array.use_texuv = 0;
   gc->array.use_texuv2 = 0;
   gc->array.use_texuv3 = 0;
}

void
evas_gl_common_context_rectangle_push(Evas_GL_Context *gc, 
                                      int x, int y, int w, int h,
                                      int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   Eina_Bool blend = 0;
   GLuint prog = gc->shared->shader.rect.prog;
   
   if (a < 255) blend = 1;
   if (gc->dc->render_op == EVAS_RENDER_COPY) blend = 0;
   
   if ((gc->shader.cur_tex != 0)
       || (gc->shader.cur_prog != prog)
       || (gc->shader.blend != blend)
       || (gc->shader.render_op != gc->dc->render_op)
       || (gc->shader.clip != 0)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = 0;
        gc->shader.cur_prog = prog;
        gc->shader.blend = blend;
        gc->shader.render_op = gc->dc->render_op;
        gc->shader.clip = 0;

     }
   gc->array.line = 0;
   gc->array.use_vertex = 1;
   gc->array.use_color = 1;
   gc->array.use_texuv = 0;
   gc->array.use_texuv2 = 0;
   gc->array.use_texuv3 = 0;
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);
  
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(r, g, b, a);
     }
}

void
evas_gl_common_context_image_push(Evas_GL_Context *gc,
                                  Evas_GL_Texture *tex,
                                  double sx, double sy, double sw, double sh,
                                  int x, int y, int w, int h,
                                  int r, int g, int b, int a,
                                  Eina_Bool smooth, Eina_Bool tex_only)
{
   int pnum, nv, nc, nu, nu2, nt, i;
   GLfloat tx1, tx2, ty1, ty2;
   GLfloat bl = 1.0;
   Eina_Bool blend = 1;
   GLuint prog = gc->shared->shader.img.prog;

   if (!tex->alpha) blend = 0;
   if (a < 255) blend = 1;
   
   if (tex_only)
     {
        if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
          prog = gc->shared->shader.tex.prog; // fixme: nomul
        else
          prog = gc->shared->shader.tex.prog;
     }
   else
     {
        if (tex->gc->shared->info.bgra)
          {
             if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
               prog = gc->shared->shader.img_bgra_nomul.prog;
             else
               prog = gc->shared->shader.img_bgra.prog;
          }
        else
          {
             if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
               prog = gc->shared->shader.img_nomul.prog;
             else
               prog = gc->shared->shader.img.prog;
          }
     }

   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != prog)
       || (gc->shader.smooth != smooth)
       || (gc->shader.blend != blend)
       || (gc->shader.render_op != gc->dc->render_op)
       || (gc->shader.clip != 0)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_prog = prog;
        gc->shader.smooth = smooth;
        gc->shader.blend = blend;
        gc->shader.render_op = gc->dc->render_op;
        gc->shader.clip = 0;
     } 
   if ((tex->im) && (tex->im->native.data))
     {
        if (gc->array.im != tex->im)
          {
             shader_array_flush(gc);
             gc->array.im = tex->im;
          }
     }
   
   gc->array.line = 0;
   gc->array.use_vertex = 1;
   // if nomul... dont need this
   gc->array.use_color = 1;
   gc->array.use_texuv = 1;
   // if not solid, don't need this
   gc->array.use_texuv2 = 1;
   gc->array.use_texuv3 = 0;
  
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nu2 = pnum * 2;
   nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy) / (double)tex->pt->h;
     }
   else
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
     }
   if (blend) bl = 0.0;

   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx1, ty1);
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx1, ty2);
   
   PUSH_TEXUV2(bl, 0.0);
   PUSH_TEXUV2(bl, 0.0);
   PUSH_TEXUV2(bl, 0.0);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx2, ty2);
   PUSH_TEXUV(tx1, ty2);

   PUSH_TEXUV2(bl, 0.0);
   PUSH_TEXUV2(bl, 0.0);
   PUSH_TEXUV2(bl, 0.0);

   // if nomul... dont need this
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(r, g, b, a);
     }
}

void
evas_gl_common_context_font_push(Evas_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   GLfloat tx1, tx2, ty1, ty2;

   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != gc->shared->shader.font.prog)
       || (gc->shader.smooth != 0)
       || (gc->shader.blend != 1)
       || (gc->shader.render_op != gc->dc->render_op)
       || (gc->shader.clip != 0)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_prog = gc->shared->shader.font.prog;
        gc->shader.smooth = 0;
        gc->shader.blend = 1;
        gc->shader.render_op = gc->dc->render_op;
        gc->shader.clip = 0;
     }
   gc->array.line = 0;
   gc->array.use_vertex = 1;
   gc->array.use_color = 1;
   gc->array.use_texuv = 1;
   gc->array.use_texuv2 = 0;
   gc->array.use_texuv3 = 0;
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   if (sw == 0.0)
     {
        tx1 = tex->sx1;
        ty1 = tex->sy1;
        tx2 = tex->sx2;
        ty2 = tex->sy2;
     }
   else
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
     }
   
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx1, ty1);
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx1, ty2);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx2, ty2);
   PUSH_TEXUV(tx1, ty2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(r, g, b, a);
     }
}

void
evas_gl_common_context_yuv_push(Evas_GL_Context *gc,
                                Evas_GL_Texture *tex, 
                                double sx, double sy, double sw, double sh,
                                int x, int y, int w, int h,
                                int r, int g, int b, int a,
                                Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nu2, nu3, nt, i;
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Eina_Bool blend = 0;

   if (a < 255) blend = 1;
   
   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != gc->shared->shader.yuv.prog)
       || (gc->shader.smooth != smooth)
       || (gc->shader.blend != blend)
       || (gc->shader.render_op != gc->dc->render_op)
       || (gc->shader.clip != 0)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_texu = tex->ptu->texture;
        gc->shader.cur_texv = tex->ptv->texture;
        gc->shader.cur_prog = gc->shared->shader.yuv.prog;
        gc->shader.smooth = smooth;
        gc->shader.blend = blend;
        gc->shader.render_op = gc->dc->render_op;
        gc->shader.clip = 0;
     }
   gc->array.line = 0;
   gc->array.use_vertex = 1;
   gc->array.use_color = 1;
   gc->array.use_texuv = 1;
   gc->array.use_texuv2 = 1;
   gc->array.use_texuv3 = 1;
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; 
   nu2 = pnum * 2; nu3 = pnum * 2; nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;
   
   t2x1 = ((sx) / 2) / (double)tex->ptu->w;
   t2y1 = ((sy) / 2) / (double)tex->ptu->h;
   t2x2 = ((sx + sw) / 2) / (double)tex->ptu->w;
   t2y2 = ((sy + sh) / 2) / (double)tex->ptu->h;
   
   PUSH_VERTEX(x    , y    , 0);
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx1, ty1);
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx1, ty2);
   
   PUSH_TEXUV2(t2x1, t2y1);
   PUSH_TEXUV2(t2x2, t2y1);
   PUSH_TEXUV2(t2x1, t2y2);
   
   PUSH_TEXUV3(t2x1, t2y1);
   PUSH_TEXUV3(t2x2, t2y1);
   PUSH_TEXUV3(t2x1, t2y2);
   
   PUSH_VERTEX(x + w, y    , 0);
   PUSH_VERTEX(x + w, y + h, 0);
   PUSH_VERTEX(x    , y + h, 0);
   
   PUSH_TEXUV(tx2, ty1);
   PUSH_TEXUV(tx2, ty2);
   PUSH_TEXUV(tx1, ty2);

   PUSH_TEXUV2(t2x2, t2y1);
   PUSH_TEXUV2(t2x2, t2y2);
   PUSH_TEXUV2(t2x1, t2y2);

   PUSH_TEXUV3(t2x2, t2y1);
   PUSH_TEXUV3(t2x2, t2y2);
   PUSH_TEXUV3(t2x1, t2y2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(r, g, b, a);
     }
}

void
evas_gl_common_context_image_map4_push(Evas_GL_Context *gc,
                                       Evas_GL_Texture *tex,
                                       RGBA_Map_Point *p,
                                       int clip, int cx, int cy, int cw, int ch,
                                       int r, int g, int b, int a,
                                       Eina_Bool smooth, Eina_Bool tex_only)
{
   int pnum, nv, nc, nu, nu2, nt, i;
   const int points[6] = { 0, 1, 2, 0, 2, 3 };
   GLfloat tx[4], ty[4];
   GLfloat bl = 1.0;
   Eina_Bool blend = 1;
   RGBA_Map_Point *pt;
   DATA32 cmul;
   GLuint prog = gc->shared->shader.img.prog;

   if (!tex->alpha) blend = 0;
   if (a < 255) blend = 1;
   
   if (tex_only)
     {
        if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
          prog = gc->shared->shader.tex.prog; // fixme: nomul
        else
          prog = gc->shared->shader.tex.prog;
     }
   else
     {
        if (tex->gc->shared->info.bgra)
          {
             if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
               prog = gc->shared->shader.img_bgra_nomul.prog;
             else
               prog = gc->shared->shader.img_bgra.prog;
          }
        else
          {
             if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
               prog = gc->shared->shader.img_nomul.prog;
             else
               prog = gc->shared->shader.img.prog;
          }
     }
   
   if ((gc->shader.cur_tex != tex->pt->texture)
       || (gc->shader.cur_prog != prog)
       || (gc->shader.smooth != smooth)
       || (gc->shader.blend != blend)
       || (gc->shader.render_op != gc->dc->render_op)
       || (gc->shader.clip != clip)
       || (gc->shader.cx != cx)
       || (gc->shader.cy != cy)
       || (gc->shader.cw != cw)
       || (gc->shader.ch != ch)
       )
     {
        shader_array_flush(gc);
        gc->shader.cur_tex = tex->pt->texture;
        gc->shader.cur_prog = prog;
        gc->shader.smooth = smooth;
        gc->shader.blend = blend;
        gc->shader.render_op = gc->dc->render_op;
        gc->shader.clip = clip;
        gc->shader.cx = cx;
        gc->shader.cy = cy;
        gc->shader.cw = cw;
        gc->shader.ch = ch;
     }
   if ((tex->im) && (tex->im->native.data))
     {
        if (gc->array.im != tex->im)
          {
             shader_array_flush(gc);
             gc->array.im = tex->im;
          }
     }
   gc->array.line = 0;
   gc->array.use_vertex = 1;
   gc->array.use_color = 1;
   gc->array.use_texuv = 1;
   gc->array.use_texuv2 = 1;
   gc->array.use_texuv3 = 0;
   
   pnum = gc->array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nu2 = pnum * 2;
   nt = pnum * 4;
   gc->array.num += 6;
   _evas_gl_common_context_array_alloc(gc);

   for (i = 0; i < 4; i++)
     {
        tx[i] = ((double)(tex->x) + (((double)p[i].u) / FP1)) /
          (double)tex->pt->w;
        ty[i] = ((double)(tex->y) + (((double)p[i].v) / FP1)) / 
          (double)tex->pt->h;
     }
   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
     {
        // FIXME: handle yinvert
     }
   
   if (blend) bl = 0.0;
   
   cmul = ARGB_JOIN(a, r, g, b);
   for (i = 0; i < 6; i++)
     {
        DATA32 cl = MUL4_SYM(cmul, p[points[i]].col);
        PUSH_VERTEX((p[points[i]].x >> FP), 
                    (p[points[i]].y >> FP),
                    0);
//                    (p[points[i]].y >> FP) + 4096);
//                    (p[points[i]].z >> FP));
        PUSH_TEXUV(tx[points[i]],
                   ty[points[i]]);
        
        PUSH_TEXUV2(bl, 0.0);
   
        PUSH_COLOR(R_VAL(&cl),
                   G_VAL(&cl),
                   B_VAL(&cl),
                   A_VAL(&cl));
     }
}

void
evas_gl_common_context_flush(Evas_GL_Context *gc)
{
   shader_array_flush(gc);
//   fprintf(stderr, "------------FRAME: done\n");
}

static void
shader_array_flush(Evas_GL_Context *gc)
{
   if (gc->array.num <= 0) return;

//   fprintf(stderr, "  flush array %i\n", gc->array.num);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "<flush err>");
   if (gc->shader.cur_prog != gc->shader.current.cur_prog)
     {
        glUseProgram(gc->shader.cur_prog);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }

   if (gc->shader.cur_tex != gc->shader.current.cur_tex)
     {
        if (gc->shader.cur_tex)
          {
             glEnable(GL_TEXTURE_2D);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             glDisable(GL_TEXTURE_2D);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        glActiveTexture(GL_TEXTURE0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBindTexture(GL_TEXTURE_2D, gc->shader.cur_tex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   if (gc->array.im)
     {
        if (!gc->array.im->native.loose)
          {
             if (gc->array.im->native.func.bind)
               gc->array.im->native.func.bind(gc->array.im->native.func.data, 
                                              gc->array.im);
          }
     }
   if (gc->shader.render_op != gc->shader.current.render_op)
     {
        switch (gc->shader.render_op)
          {
          case EVAS_RENDER_BLEND: /**< default op: d = d*(1-sa) + s */
             glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             break;
          case EVAS_RENDER_COPY: /**< d = s */
             gc->shader.blend = 0;
             glBlendFunc(GL_ONE, GL_ONE);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             break;
             // FIXME: fix blend funcs below!
          case EVAS_RENDER_BLEND_REL: /**< d = d*(1 - sa) + s*da */
          case EVAS_RENDER_COPY_REL: /**< d = s*da */
          case EVAS_RENDER_ADD: /**< d = d + s */
          case EVAS_RENDER_ADD_REL: /**< d = d + s*da */
          case EVAS_RENDER_SUB: /**< d = d - s */
          case EVAS_RENDER_SUB_REL: /**< d = d - s*da */
          case EVAS_RENDER_TINT: /**< d = d*s + d*(1 - sa) + s*(1 - da) */
          case EVAS_RENDER_TINT_REL: /**< d = d*(1 - sa + s) */
          case EVAS_RENDER_MASK: /**< d = d*sa */
          case EVAS_RENDER_MUL: /**< d = d*s */
          default:
             glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             break;
          }
     }
   if (gc->shader.blend != gc->shader.current.blend)
     {
        if (gc->shader.blend)
          {
             glEnable(GL_BLEND);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             glDisable(GL_BLEND);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
     }
   if (gc->shader.smooth != gc->shader.current.smooth)
     {
        if (gc->shader.smooth)
          {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
             if (shared->info.anisotropic > 0.0)
               {
                  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
#endif
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
             if (shared->info.anisotropic > 0.0)
               {
                  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
#endif
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
     }
/* hmmm this breaks things. must find out why!   
   if (gc->shader.clip != gc->shader.current.clip)
     {
        if (gc->shader.clip)
          glEnable(GL_SCISSOR_TEST);
        else
          {
             glDisable(GL_SCISSOR_TEST);
//             glScissor(0, 0, 0, 0);
          }
     }
   if (gc->shader.clip)
     {
        if ((gc->shader.cx != gc->shader.current.cx) ||
            (gc->shader.cx != gc->shader.current.cx) ||
            (gc->shader.cx != gc->shader.current.cx) ||
            (gc->shader.cx != gc->shader.current.cx))
          {
             glScissor(gc->shader.cx, 
                       gc->h - gc->shader.cy - gc->shader.ch,
                       gc->shader.cw,
                       gc->shader.ch);
          }
//                    gc->clip.x,
//                    gc->h - gc->clip.y - gc->clip.h,
//                    gc->clip.w,
//                    gc->clip.h);
        
     }
 */
   glVertexAttribPointer(SHAD_VERTEX, 3, GL_SHORT, GL_FALSE, 0, gc->array.vertex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glVertexAttribPointer(SHAD_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, gc->array.color);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (gc->array.use_texuv)
     {
        glEnableVertexAttribArray(SHAD_TEXUV);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glVertexAttribPointer(SHAD_TEXUV, 2, GL_FLOAT, GL_FALSE, 0, gc->array.texuv);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   else
     {
        glDisableVertexAttribArray(SHAD_TEXUV);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }

   if (gc->array.line)
     {
        glDisableVertexAttribArray(SHAD_TEXUV);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDisableVertexAttribArray(SHAD_TEXUV2); 
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDisableVertexAttribArray(SHAD_TEXUV3);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDrawArrays(GL_LINES, 0, gc->array.num);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   else
     {
        if ((gc->array.use_texuv2) && (gc->array.use_texuv3))
          {
             glEnableVertexAttribArray(SHAD_TEXUV2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glEnableVertexAttribArray(SHAD_TEXUV3);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glVertexAttribPointer(SHAD_TEXUV2, 2, GL_FLOAT, GL_FALSE, 0, gc->array.texuv2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glVertexAttribPointer(SHAD_TEXUV3, 2, GL_FLOAT, GL_FALSE, 0, gc->array.texuv3);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glActiveTexture(GL_TEXTURE1);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glBindTexture(GL_TEXTURE_2D, gc->shader.cur_texu);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glActiveTexture(GL_TEXTURE2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glBindTexture(GL_TEXTURE_2D, gc->shader.cur_texv);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glActiveTexture(GL_TEXTURE0);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else if (gc->array.use_texuv2)
          {
             glEnableVertexAttribArray(SHAD_TEXUV2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glVertexAttribPointer(SHAD_TEXUV2, 2, GL_FLOAT, GL_FALSE, 0, gc->array.texuv2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             glDisableVertexAttribArray(SHAD_TEXUV2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glDisableVertexAttribArray(SHAD_TEXUV3);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
   
        glDrawArrays(GL_TRIANGLES, 0, gc->array.num);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   if (gc->array.im)
     {
        if (!gc->array.im->native.loose)
          {
             if (gc->array.im->native.func.unbind)
               gc->array.im->native.func.unbind(gc->array.im->native.func.data, 
                                                gc->array.im);
          }
        gc->array.im = NULL;
     }

   gc->shader.current.cur_prog = gc->shader.cur_prog;
   gc->shader.current.cur_tex = gc->shader.cur_tex;
   gc->shader.current.blend = gc->shader.blend;
   gc->shader.current.smooth = gc->shader.smooth;
   gc->shader.current.render_op = gc->shader.render_op;
   gc->shader.current.clip = gc->shader.clip;
   gc->shader.current.cx = gc->shader.cx;
   gc->shader.current.cy = gc->shader.cy;
   gc->shader.current.cw = gc->shader.cw;
   gc->shader.current.ch = gc->shader.ch;
   
   if (gc->array.vertex) free(gc->array.vertex);
   if (gc->array.color) free(gc->array.color);
   if (gc->array.texuv) free(gc->array.texuv);
   if (gc->array.texuv2) free(gc->array.texuv2);
   if (gc->array.texuv3) free(gc->array.texuv3);
   
   gc->array.vertex = NULL;
   gc->array.color = NULL;
   gc->array.texuv = NULL;
   gc->array.texuv2 = NULL;
   gc->array.texuv3 = NULL;
   
   gc->array.num = 0;
   gc->array.alloc = 0;
}
