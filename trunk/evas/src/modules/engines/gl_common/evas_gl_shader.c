#include "evas_gl_private.h"

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_frag_bin[] =
{
# include "shader/rect_frag_bin_s3c6410.h"
};
#endif

const char rect_frag_glsl[] =
#include "shader/rect_frag.h"
  ;
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     rect_frag_bin, sizeof(rect_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int rect_vert_bin[] =
{
# include "shader/rect_vert_bin_s3c6410.h"
};
#endif
const char rect_vert_glsl[] =
#include "shader/rect_vert.h"
  ;
Evas_GL_Program_Source shader_rect_vert_src =
{
   rect_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     rect_vert_bin, sizeof(rect_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int font_frag_bin[] =
{
# include "shader/font_frag_bin_s3c6410.h"
};
#endif

const char font_frag_glsl[] =
#include "shader/font_frag.h"
  ;
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     font_frag_bin, sizeof(font_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int font_vert_bin[] =
{
# include "shader/font_vert_bin_s3c6410.h"
};
#endif
const char font_vert_glsl[] =
#include "shader/font_vert.h"
  ;
Evas_GL_Program_Source shader_font_vert_src =
{
   font_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     font_vert_bin, sizeof(font_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_frag_bin[] =
{
# include "shader/yuv_frag_bin_s3c6410.h"
};
#endif

const char yuv_frag_glsl[] =
#include "shader/yuv_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_frag_bin, sizeof(yuv_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_vert_bin[] =
{
# include "shader/yuv_vert_bin_s3c6410.h"
};
#endif
const char yuv_vert_glsl[] =
#include "shader/yuv_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_vert_src =
{
   yuv_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_vert_bin, sizeof(yuv_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_nomul_frag_bin[] =
{
# include "shader/yuv_nomul_frag_bin_s3c6410.h"
};
#endif

const char yuv_nomul_frag_glsl[] =
#include "shader/yuv_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_nomul_frag_bin, sizeof(yuv_nomul_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int yuv_nomul_vert_bin[] =
{
# include "shader/yuv_nomul_vert_bin_s3c6410.h"
};
#endif
const char yuv_nomul_vert_glsl[] =
#include "shader/yuv_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_yuv_nomul_vert_src =
{
   yuv_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     yuv_nomul_vert_bin, sizeof(yuv_nomul_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_frag_bin[] =
{
# include "shader/tex_frag_bin_s3c6410.h"
};
#endif

const char tex_frag_glsl[] =
#include "shader/tex_frag.h"
  ;
Evas_GL_Program_Source shader_tex_frag_src =
{
   tex_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_frag_bin, sizeof(tex_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_vert_bin[] =
{
# include "shader/tex_vert_bin_s3c6410.h"
};
#endif
const char tex_vert_glsl[] =
#include "shader/tex_vert.h"
  ;
Evas_GL_Program_Source shader_tex_vert_src =
{
   tex_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_vert_bin, sizeof(tex_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_nomul_frag_bin[] =
{
# include "shader/tex_nomul_frag_bin_s3c6410.h"
};
#endif

const char tex_nomul_frag_glsl[] =
#include "shader/tex_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_frag_src =
{
   tex_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_nomul_frag_bin, sizeof(tex_nomul_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int tex_nomul_vert_bin[] =
{
# include "shader/tex_nomul_vert_bin_s3c6410.h"
};
#endif
const char tex_nomul_vert_glsl[] =
#include "shader/tex_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_tex_nomul_vert_src =
{
   tex_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     tex_nomul_vert_bin, sizeof(tex_nomul_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_frag_bin[] =
{
# include "shader/img_frag_bin_s3c6410.h"
};
#endif

const char img_frag_glsl[] =
#include "shader/img_frag.h"
  ;
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_frag_bin, sizeof(img_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_vert_bin[] =
{
# include "shader/img_vert_bin_s3c6410.h"
};
#endif
const char img_vert_glsl[] =
#include "shader/img_vert.h"
  ;
Evas_GL_Program_Source shader_img_vert_src =
{
   img_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_vert_bin, sizeof(img_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_nomul_frag_bin[] =
{
# include "shader/img_nomul_frag_bin_s3c6410.h"
};
#endif

const char img_nomul_frag_glsl[] =
#include "shader/img_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_nomul_frag_src =
{
   img_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_nomul_frag_bin, sizeof(img_nomul_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_nomul_vert_bin[] =
{
# include "shader/img_nomul_vert_bin_s3c6410.h"
};
#endif
const char img_nomul_vert_glsl[] =
#include "shader/img_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_nomul_vert_src =
{
   img_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_nomul_vert_bin, sizeof(img_nomul_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_frag_bin[] =
{
# include "shader/img_bgra_frag_bin_s3c6410.h"
};
#endif

const char img_bgra_frag_glsl[] =
#include "shader/img_bgra_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_frag_src =
{
   img_bgra_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_frag_bin, sizeof(img_bgra_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_vert_bin[] =
{
# include "shader/img_bgra_vert_bin_s3c6410.h"
};
#endif
const char img_bgra_vert_glsl[] =
#include "shader/img_bgra_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_vert_src =
{
   img_bgra_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_vert_bin, sizeof(img_bgra_vert_bin)
#else
     NULL, 0
#endif
};


/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_mask_frag_bin[] =
{
# include "shader/img_mask_frag_bin_s3c6410.h"
};
#endif

const char img_mask_frag_glsl[] =
#include "shader/img_mask_frag.h"
  ;
Evas_GL_Program_Source shader_img_mask_frag_src =
{
   img_mask_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_mask_frag_bin, sizeof(img_mask_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_mask_vert_bin[] =
{
# include "shader/img_mask_vert_bin_s3c6410.h"
};
#endif
const char img_mask_vert_glsl[] =
#include "shader/img_mask_vert.h"
  ;
Evas_GL_Program_Source shader_img_mask_vert_src =
{
   img_mask_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_mask_vert_bin, sizeof(img_mask_vert_bin)
#else
     NULL, 0
#endif
};


/////////////////////////////////////////////
#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_nomul_frag_bin[] =
{
# include "shader/img_bgra_nomul_frag_bin_s3c6410.h"
};
#endif

const char img_bgra_nomul_frag_glsl[] =
#include "shader/img_bgra_nomul_frag.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_frag_src =
{
   img_bgra_nomul_frag_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_nomul_frag_bin, sizeof(img_bgra_nomul_frag_bin)
#else
     NULL, 0
#endif
};

#if defined (GLES_VARIETY_S3C6410)
const unsigned int img_bgra_nomul_vert_bin[] =
{
# include "shader/img_bgra_nomul_vert_bin_s3c6410.h"
};
#endif
const char img_bgra_nomul_vert_glsl[] =
#include "shader/img_bgra_nomul_vert.h"
  ;
Evas_GL_Program_Source shader_img_bgra_nomul_vert_src =
{
   img_bgra_nomul_vert_glsl,
#if defined (GLES_VARIETY_S3C6410)
     img_bgra_nomul_vert_bin, sizeof(img_bgra_nomul_vert_bin)
#else
     NULL, 0
#endif
};

/////////////////////////////////////////////
const char filter_invert_frag_glsl[] =
#include "shader/filter_invert.h"
  ;
Evas_GL_Program_Source shader_filter_invert_frag_src =
{
   filter_invert_frag_glsl,
   NULL, 0
};

const char filter_invert_nomul_frag_glsl[] =
#include "shader/filter_invert_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_invert_nomul_frag_src =
{
   filter_invert_nomul_frag_glsl,
   NULL, 0
};

const char filter_invert_bgra_frag_glsl[] =
#include "shader/filter_invert_bgra.h"
  ;
Evas_GL_Program_Source shader_filter_invert_bgra_frag_src =
{
   filter_invert_bgra_frag_glsl,
   NULL, 0
};
const char filter_invert_bgra_nomul_frag_glsl[] =
#include "shader/filter_invert_bgra_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_invert_bgra_nomul_frag_src =
{
   filter_invert_bgra_nomul_frag_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char filter_greyscale_frag_glsl[] =
#include "shader/filter_greyscale.h"
  ;
Evas_GL_Program_Source shader_filter_greyscale_frag_src =
{
   filter_greyscale_frag_glsl,
   NULL, 0
};

const char filter_greyscale_nomul_frag_glsl[] =
#include "shader/filter_greyscale_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_greyscale_nomul_frag_src =
{
   filter_greyscale_nomul_frag_glsl,
   NULL, 0
};

const char filter_greyscale_bgra_frag_glsl[] =
#include "shader/filter_greyscale_bgra.h"
  ;
Evas_GL_Program_Source shader_filter_greyscale_bgra_frag_src =
{
   filter_greyscale_bgra_frag_glsl,
   NULL, 0
};
const char filter_greyscale_bgra_nomul_frag_glsl[] =
#include "shader/filter_greyscale_bgra_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_greyscale_bgra_nomul_frag_src =
{
   filter_greyscale_bgra_nomul_frag_glsl,
   NULL, 0
};

/////////////////////////////////////////////
const char filter_sepia_frag_glsl[] =
#include "shader/filter_sepia.h"
  ;
Evas_GL_Program_Source shader_filter_sepia_frag_src =
{
   filter_sepia_frag_glsl,
   NULL, 0
};

const char filter_sepia_nomul_frag_glsl[] =
#include "shader/filter_sepia_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_sepia_nomul_frag_src =
{
   filter_sepia_nomul_frag_glsl,
   NULL, 0
};

const char filter_sepia_bgra_frag_glsl[] =
#include "shader/filter_sepia_bgra.h"
  ;
Evas_GL_Program_Source shader_filter_sepia_bgra_frag_src =
{
   filter_sepia_bgra_frag_glsl,
   NULL, 0
};
const char filter_sepia_bgra_nomul_frag_glsl[] =
#include "shader/filter_sepia_bgra_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_sepia_bgra_nomul_frag_src =
{
   filter_sepia_bgra_nomul_frag_glsl,
   NULL, 0
};

/////////////////////////////////////////////
#if 0
	Blur is a work in progress currently.
	Mostly because GPUs are so hopeless.
const char filter_blur_vert_glsl[] =
#include "shader/filter_blur_vert.h"
  ;

Evas_GL_Program_Source shader_filter_blur_vert_src =
{
   filter_blur_vert_glsl,
   NULL, 0
};

const char filter_blur_frag_glsl[] =
#include "shader/filter_blur.h"
  ;
Evas_GL_Program_Source shader_filter_blur_frag_src =
{
   filter_blur_frag_glsl,
   NULL, 0
};

const char filter_blur_nomul_frag_glsl[] =
#include "shader/filter_blur_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_blur_nomul_frag_src =
{
   filter_blur_nomul_frag_glsl,
   NULL, 0
};

const char filter_blur_bgra_frag_glsl[] =
#include "shader/filter_blur_bgra.h"
  ;
Evas_GL_Program_Source shader_filter_blur_bgra_frag_src =
{
   filter_blur_bgra_frag_glsl,
   NULL, 0
};
const char filter_blur_bgra_nomul_frag_glsl[] =
#include "shader/filter_blur_bgra_nomul.h"
  ;
Evas_GL_Program_Source shader_filter_blur_bgra_nomul_frag_src =
{
   filter_blur_bgra_nomul_frag_glsl,
   NULL, 0
};

#endif



/////////////////////////////////////////////
static void
gl_compile_link_error(GLuint target, const char *action)
{
   int loglen = 0, chars = 0;
   char *logtxt;

   /* Shader info log */
   glGetShaderiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetShaderInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }

   /* Program info log */
   glGetProgramiv(target, GL_INFO_LOG_LENGTH, &loglen);
   if (loglen > 0)
     {
        logtxt = calloc(loglen, sizeof(char));
        if (logtxt)
          {
             glGetProgramInfoLog(target, loglen, &chars, logtxt);
             ERR("Failed to %s: %s", action, logtxt);
             free(logtxt);
          }
     }
}

static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

static Eina_Bool
_evas_gl_shader_file_is_dir(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return EINA_FALSE;
   if (S_ISDIR(st.st_mode)) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_evas_gl_shader_file_mkdir(const char *dir)
{
   /* evas gl shader only call this function when the dir is not exist */
   if (mkdir(dir, default_mode) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_evas_gl_shader_file_exists(const char *file)
{
   struct stat st;
   if (!file) return EINA_FALSE;
   if (stat(file, &st) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
_evas_gl_shader_file_mkpath_if_not_exists(const char *path)
{
   struct stat st;

   if (stat(path, &st) < 0)
     return _evas_gl_shader_file_mkdir(path);
   else if (!S_ISDIR(st.st_mode))
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

static Eina_Bool
_evas_gl_shader_file_mkpath(const char *path)
{
   char ss[PATH_MAX];
   unsigned int i;

   if (_evas_gl_shader_file_is_dir(path)) return EINA_TRUE;

   for (i = 0; path[i]; ss[i] = path[i], i++)
     {
        if (i == sizeof(ss) - 1) return EINA_FALSE;
        if ((path[i] == '/') && (i > 0))
          {
             ss[i] = 0;
             if (!_evas_gl_shader_file_mkpath_if_not_exists(ss))
               return EINA_FALSE;
          }
     }
   ss[i] = 0;
   return _evas_gl_shader_file_mkpath_if_not_exists(ss);
}

static int
_evas_gl_shader_dir_check(char *bin_shader_dir, int num)
{
   char *home = NULL;
   char *subdir = ".cache/evas_gl_common_shaders";

   home = getenv("HOME");
   if ((!home) || (!home[0])) return 0;

   snprintf(bin_shader_dir, num, "%s/%s", home, subdir);
   return _evas_gl_shader_file_exists(bin_shader_dir);
}

static int
_evas_gl_shader_file_check(const char *bin_shader_dir, char *bin_shader_file, int dir_num)
{
   char before_name[PATH_MAX];
   char after_name[PATH_MAX];
   int new_path_len = 0;
   int i = 0, j = 0;

   char *vendor = NULL;
   char *driver = NULL;
   char *version = NULL;

   vendor = (char *)glGetString(GL_VENDOR);
   driver = (char *)glGetString(GL_RENDERER);
   version = (char *)glGetString(GL_VERSION);

   new_path_len = snprintf(before_name, sizeof(before_name), "%s::%s::%s::%s::binary_shader.eet", vendor, version, driver, MODULE_ARCH);

   /* remove '/' from file name */
   for (i = 0; i < new_path_len; i++)
     {
        if (before_name[i] != '/')
          {
             after_name[j] = before_name[i];
             j++;
          }
     }
   after_name[j] = 0;

   snprintf(bin_shader_file, dir_num, "%s/%s", bin_shader_dir, after_name);

   return _evas_gl_shader_file_exists(bin_shader_file);
}

static int
_evas_gl_common_shader_program_binary_init(Evas_GL_Program *p,
                                           const char *pname,
                                           Eet_File *ef)
{
   int res = 0, num = 0, length = 0;
   int *formats = NULL;
   void *data = NULL;

   if (!ef) return res;

   data = eet_read(ef, pname, &length);
   if ((!data) || (length <= 0)) goto finish;

   glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num);
   if (num <= 0) goto finish;

   formats = calloc(num, sizeof(int));
   if (!formats) goto finish;

   glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats);
   if (!formats[0]) goto finish;

   p->prog = glCreateProgram();

#if 0
   // TODO: invalid rendering error occurs when attempting to use a
   // glProgramBinary. in order to render correctly we should create a dummy
   // vertex shader.
   p->vert = glCreateShader(GL_VERTEX_SHADER);
   glAttachShader(p->prog, p->vert);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);
   glAttachShader(p->prog, p->frag);
#endif
   glsym_glProgramBinary(p->prog, formats[0], data, length);

   glBindAttribLocation(p->prog, SHAD_VERTEX, "vertex");
   glBindAttribLocation(p->prog, SHAD_COLOR,  "color");
   glBindAttribLocation(p->prog, SHAD_TEXUV,  "tex_coord");
   glBindAttribLocation(p->prog, SHAD_TEXUV2, "tex_coord2");
   glBindAttribLocation(p->prog, SHAD_TEXUV3, "tex_coord3");
   glBindAttribLocation(p->prog, SHAD_TEXM,   "tex_coordm");
   res = 1;

finish:
   if (formats) free(formats);
   if (data) free(data);
   if ((!res) && (p->prog))
     {
        glDeleteProgram(p->prog);
        p->prog = 0;
     }
   return res;
}

static int
_evas_gl_common_shader_program_binary_save(Evas_GL_Program *p,
                                           const char *pname,
                                           Eet_File *ef)
{
   void* data = NULL;
   GLenum format;
   int length = 0;

   if (!glsym_glGetProgramBinary) return 0;

   glGetProgramiv(p->prog, GL_PROGRAM_BINARY_LENGTH, &length);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (length <= 0) return 0;

   data = malloc(length);
   if (!data) return 0;

   glsym_glGetProgramBinary(p->prog, length, NULL, &format, data);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   if (eet_write(ef, pname, data, length, 0) < 0)
     {
        if (data) free(data);
        return 0;
     }
   if (data) free(data);
   return 1;
}

static int
_evas_gl_common_shader_program_source_init(Evas_GL_Program *p,
                                           Evas_GL_Program_Source *vert,
                                           Evas_GL_Program_Source *frag,
                                           const char *name)
{
   GLint ok;

   p->vert = glCreateShader(GL_VERTEX_SHADER);
   p->frag = glCreateShader(GL_FRAGMENT_SHADER);
#if defined (GLES_VARIETY_S3C6410)
   glShaderBinary(1, &(p->vert), 0, vert->bin, vert->bin_size);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glShaderBinary(1, &(p->frag), 0, frag->bin, frag->bin_size);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#else
   glShaderSource(p->vert, 1,
                  (const char **)&(vert->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->vert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetShaderiv(p->vert, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->vert, "compile vertex shader");
        ERR("Abort compile of shader vert (%s): %s", name, vert->src);
        return 0;
     }
   glShaderSource(p->frag, 1,
                  (const char **)&(frag->src), NULL);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glCompileShader(p->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetShaderiv(p->frag, GL_COMPILE_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->frag, "compile fragment shader");
        ERR("Abort compile of shader frag (%s): %s", name, frag->src);
        return 0;
     }
#endif
   p->prog = glCreateProgram();
#if defined(GLES_VARIETY_S3C6410) || defined(GLES_VARIETY_SGX)
#else
   if ((glsym_glGetProgramBinary) && (glsym_glProgramParameteri))
      glsym_glProgramParameteri(p->prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
                                GL_TRUE);
#endif
   glAttachShader(p->prog, p->vert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glAttachShader(p->prog, p->frag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glBindAttribLocation(p->prog, SHAD_VERTEX, "vertex");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_COLOR,  "color");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV,  "tex_coord");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV2, "tex_coord2");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXUV3, "tex_coord3");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindAttribLocation(p->prog, SHAD_TEXM, "tex_coordm");
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glLinkProgram(p->prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   ok = 0;
   glGetProgramiv(p->prog, GL_LINK_STATUS, &ok);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   if (!ok)
     {
        gl_compile_link_error(p->prog, "link fragment and vertex shaders");
        ERR("Abort compile of shader frag (%s): %s", name, frag->src);
        ERR("Abort compile of shader vert (%s): %s", name, vert->src);
        return 0;
     }
   return 1;
}

static int
_evas_gl_common_shader_source_init(Evas_GL_Shared *shared)
{
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.rect),
                                                   &(shader_rect_vert_src),
                                                   &(shader_rect_frag_src),
                                                   "rect")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.font),
                                                   &(shader_font_vert_src),
                                                   &(shader_font_frag_src),
                                                   "font")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.img),
                                                   &(shader_img_vert_src),
                                                   &(shader_img_frag_src),
                                                   "img")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.img_nomul),
                                                   &(shader_img_nomul_vert_src),
                                                   &(shader_img_nomul_frag_src),
                                                   "img_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.img_bgra),
                                                   &(shader_img_bgra_vert_src),
                                                &(shader_img_bgra_frag_src),
                                                   "img_bgra")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.img_bgra_nomul),
                                                   &(shader_img_bgra_nomul_vert_src),
                                                   &(shader_img_bgra_nomul_frag_src),
                                                   "img_bgra_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.img_mask),
                                                   &(shader_img_mask_vert_src),
                                                   &(shader_img_mask_frag_src),
                                                   "img_mask")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.tex),
                                                   &(shader_tex_vert_src),
                                                   &(shader_tex_frag_src),
                                                   "tex")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.tex_nomul),
                                                   &(shader_tex_nomul_vert_src),
                                                   &(shader_tex_nomul_frag_src),
                                                   "tex_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.yuv),
                                                   &(shader_yuv_vert_src),
                                                   &(shader_yuv_frag_src),
                                                   "yuv")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.yuv_nomul),
                                                   &(shader_yuv_nomul_vert_src),
                                                   &(shader_yuv_nomul_frag_src),
                                                   "yuv_nomul")) return 0;

   /* Most of the filters use the image fragment shader */
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_invert),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_invert_frag_src),
                                                   "filter_invert")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_invert_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_invert_nomul_frag_src),
                                                   "filter_invert_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_invert_bgra),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_invert_bgra_frag_src),
                                                   "filter_invert_bgra")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_invert_bgra_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_invert_bgra_nomul_frag_src),
                                                   "filter_invert_bgra_nomul")) return 0;

   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_sepia),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_sepia_frag_src),
                                                   "filter_sepia")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_sepia_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_sepia_nomul_frag_src),
                                                   "filter_sepia_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_sepia_bgra),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_sepia_bgra_frag_src),
                                                   "filter_sepia_bgra")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_sepia_bgra_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_sepia_bgra_nomul_frag_src),
                                                   "filter_sepia_bgra_nomul")) return 0;

   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_greyscale),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_greyscale_frag_src),
                                                   "filter_greyscale")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_greyscale_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_greyscale_nomul_frag_src),
                                                   "filter_greyscale_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_greyscale_bgra),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_greyscale_bgra_frag_src),
                                                   "filter_greyscale_bgra")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_greyscale_bgra_nomul),
                                                   &(shader_img_vert_src),
                                                   &(shader_filter_greyscale_bgra_nomul_frag_src),
                                                   "filter_greyscale_bgra_nomul")) return 0;
#if 0
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_blur),
                                                   &(shader_filter_blur_vert_src),
                                                   &(shader_filter_blur_frag_src),
                                                   "filter_blur")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_blur_nomul),
                                                   &(shader_filter_blur_vert_src),
                                                   &(shader_filter_blur_nomul_frag_src),
                                                   "filter_blur_nomul")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_blur_bgra),
                                                   &(shader_filter_blur_vert_src),
                                                   &(shader_filter_blur_bgra_frag_src),
                                                   "filter_blur_bgra")) return 0;
   if (!_evas_gl_common_shader_program_source_init(&(shared->shader.filter_blur_bgra_nomul),
                                                   &(shader_filter_blur_vert_src),
                                                   &(shader_filter_blur_bgra_nomul_frag_src),
                                                   "filter_blur_bgra_nomul")) return 0;
#endif

   return 1;
}

static int
_evas_gl_common_shader_binary_init(Evas_GL_Shared *shared)
{
   /* check eet */
   Eet_File *et = NULL;
   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];

   if (!_evas_gl_shader_dir_check(bin_dir_path, sizeof(bin_dir_path)))
      return 0;

   if (!_evas_gl_shader_file_check(bin_dir_path, bin_file_path,
                                   sizeof(bin_dir_path)))
      return 0;

   /* use eet */
   if (!eet_init()) return 0;
   et = eet_open(bin_file_path, EET_FILE_MODE_READ);
   if (!et) goto error;

   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.rect), "rect", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.font), "font", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.img), "img", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.img_nomul), "img_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.img_bgra), "img_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.img_bgra_nomul), "img_bgra_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.img_mask), "img_mask", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.tex), "tex", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.tex_nomul),"tex_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.yuv), "yuv", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.yuv_nomul), "yuv_nomul", et)) goto error;

   /* Most of the filters use the image fragment shader */
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_invert), "filter_invert", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_invert_nomul), "filter_invert_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_invert_bgra), "filter_invert_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_invert_bgra_nomul), "filter_invert_bgra_nomul", et)) goto error;

   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_sepia), "filter_sepia", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_sepia_nomul), "filter_sepia_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_sepia_bgra), "filter_sepia_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_sepia_bgra_nomul), "filter_sepia_bgra_nomul", et)) goto error;

   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_greyscale), "filter_greyscale", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_greyscale_nomul), "filter_greyscale_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_greyscale_bgra), "filter_greyscale_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_greyscale_bgra_nomul), "filter_greyscale_bgra_nomul", et)) goto error;
#if 0
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_blur), "filter_blur", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_blur_nomul), "filter_blur_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_blur_bgra), "filter_blur_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_init(&(shared->shader.filter_blur_bgra_nomul), "filter_blur_bgra_nomul", et)) goto error;
#endif

   if (et) eet_close(et);
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   eet_shutdown();
   return 0;
}

static int
_evas_gl_common_shader_binary_save(Evas_GL_Shared *shared)
{
   /* check eet */
   Eet_File *et = NULL; //check eet file
   int tmpfd;
   int res = 0;
   char bin_dir_path[PATH_MAX];
   char bin_file_path[PATH_MAX];
   char tmp_file[PATH_MAX];

   if (!_evas_gl_shader_dir_check(bin_dir_path, sizeof(bin_dir_path)))
      res = _evas_gl_shader_file_mkpath(bin_dir_path);
   if (!res) return 0; /* we can't make directory */

   _evas_gl_shader_file_check(bin_dir_path, bin_file_path,
                              sizeof(bin_dir_path));

   /* use mkstemp for writing */
   snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", bin_file_path);
   tmpfd = mkstemp(tmp_file);
   if (tmpfd < 0) goto error;
   close(tmpfd);

   /* use eet */
   if (!eet_init()) goto error;

   et = eet_open(tmp_file, EET_FILE_MODE_WRITE);
   if (!et) goto error;

   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.rect), "rect", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.font), "font", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.img), "img", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.img_nomul), "img_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.img_bgra), "img_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.img_bgra_nomul), "img_bgra_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.img_mask), "img_mask", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.tex), "tex", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.tex_nomul),"tex_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.yuv), "yuv", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.yuv_nomul), "yuv_nomul", et)) goto error;
   /* Most of the filters use the image fragment shader */
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_invert), "filter_invert", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_invert_nomul), "filter_invert_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_invert_bgra), "filter_invert_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_invert_bgra_nomul), "filter_invert_bgra_nomul", et)) goto error;

   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_sepia), "filter_sepia", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_sepia_nomul), "filter_sepia_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_sepia_bgra), "filter_sepia_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_sepia_bgra_nomul), "filter_sepia_bgra_nomul", et)) goto error;

   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_greyscale), "filter_greyscale", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_greyscale_nomul), "filter_greyscale_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_greyscale_bgra), "filter_greyscale_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_greyscale_bgra_nomul), "filter_greyscale_bgra_nomul", et)) goto error;
#if 0
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_blur), "filter_blur", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_blur_nomul), "filter_blur_nomul", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_blur_bgra), "filter_blur_bgra", et)) goto error;
   if (!_evas_gl_common_shader_program_binary_save(&(shared->shader.filter_blur_bgra_nomul), "filter_blur_bgra_nomul", et)) goto error;
#endif

   if (eet_close(et) != EET_ERROR_NONE) goto error;
   if (rename(tmp_file,bin_file_path) < 0) goto error;
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   if (_evas_gl_shader_file_exists(tmp_file)) unlink(tmp_file);
   eet_shutdown();
   return 0;
}

int
evas_gl_common_shader_program_init(Evas_GL_Shared *shared)
{
   // gl support binary shader and get env of binary shader path
   if (shared->info.bin_program &&
       _evas_gl_common_shader_binary_init(shared)) return 1;
   /* compile all shader.*/
   if (!_evas_gl_common_shader_source_init(shared)) return 0;
   /* success compile all shader. if gl support binary shader, we need to save */
   if (shared->info.bin_program) _evas_gl_common_shader_binary_save(shared);
   return 1;
}

void
evas_gl_common_shader_program_init_done(void)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glReleaseShaderCompiler();
#endif
}

void
evas_gl_common_shader_program_shutdown(Evas_GL_Program *p)
{
   if (p->vert) glDeleteShader(p->vert);
   if (p->frag) glDeleteShader(p->frag);
   if (p->prog) glDeleteProgram(p->prog);
}
