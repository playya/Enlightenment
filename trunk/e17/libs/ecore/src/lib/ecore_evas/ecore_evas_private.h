/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_EVAS_PRIVATE_H
#define _ECORE_EVAS_PRIVATE_H

#include "Ecore_Data.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <sys/mman.h>
#endif

#include <Evas.h>

#define ECORE_MAGIC_EVAS 0x76543211

#ifdef BUILD_ECORE_X
#include "Ecore_X.h"
#include <Evas_Engine_Software_X11.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef BUILD_ECORE_EVAS_GL
#include <Evas_Engine_GL_X11.h>
#endif
#ifdef BUILD_ECORE_EVAS_XRENDER
#include <Evas_Engine_XRender_X11.h>
#endif
#endif
#ifdef BUILD_ECORE_EVAS_FB
#include <Evas_Engine_FB.h>
#endif
#ifdef BUILD_ECORE_EVAS_BUFFER
#include <Evas_Engine_Buffer.h>
#endif

typedef struct _Ecore_Evas             Ecore_Evas;
typedef struct _Ecore_Evas_Engine      Ecore_Evas_Engine;
typedef struct _Ecore_Evas_Engine_Func Ecore_Evas_Engine_Func;

struct _Ecore_Evas_Engine_Func
{
   void        (*fn_free) (Ecore_Evas *ee);      
   void        (*fn_callback_resize_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_move_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_show_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_hide_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_delete_request_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_destroy_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_focus_in_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_focus_out_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_mouse_in_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_mouse_out_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_pre_render_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_callback_post_render_set) (Ecore_Evas *ee, void (*func) (Ecore_Evas *ee));
   void        (*fn_move) (Ecore_Evas *ee, int x, int y);
   void        (*fn_resize) (Ecore_Evas *ee, int w, int h);
   void        (*fn_move_resize) (Ecore_Evas *ee, int x, int y, int w, int h);
   void        (*fn_rotation_set) (Ecore_Evas *ee, int rot);
   void        (*fn_shaped_set) (Ecore_Evas *ee, int shaped);
   void        (*fn_show) (Ecore_Evas *ee);
   void        (*fn_hide) (Ecore_Evas *ee);
   void        (*fn_raise) (Ecore_Evas *ee);
   void        (*fn_lower) (Ecore_Evas *ee);
   void        (*fn_title_set) (Ecore_Evas *ee, const char *t);
   void        (*fn_name_class_set) (Ecore_Evas *ee, const char *n, const char *c);
   void        (*fn_size_min_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_max_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_base_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_size_step_set) (Ecore_Evas *ee, int w, int h);
   void        (*fn_cursor_set) (Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y);
   void        (*fn_layer_set) (Ecore_Evas *ee, int layer);
   void        (*fn_focus_set) (Ecore_Evas *ee, int on);
   void        (*fn_iconified_set) (Ecore_Evas *ee, int on);
   void        (*fn_borderless_set) (Ecore_Evas *ee, int on);
   void        (*fn_override_set) (Ecore_Evas *ee, int on);
   void        (*fn_maximized_set) (Ecore_Evas *ee, int on);
   void        (*fn_fullscreen_set) (Ecore_Evas *ee, int on);
   void        (*fn_avoid_damage_set) (Ecore_Evas *ee, int on);
   void        (*fn_withdrawn_set) (Ecore_Evas *ee, int withdrawn);
   void        (*fn_sticky_set) (Ecore_Evas *ee, int sticky);
};

struct _Ecore_Evas_Engine
{
   Ecore_Evas_Engine_Func *func;
   
#ifdef BUILD_ECORE_X
   struct {
      Ecore_X_Window win_root;
      Ecore_X_Window win_container;
      Ecore_X_Window win;
      Evas_List     *win_extra;
      Ecore_X_Pixmap pmap;
      Ecore_X_Pixmap mask;
      Ecore_X_GC     gc;
      Region         damages;
      unsigned char  direct_resize : 1;
      unsigned char  using_bg_pixmap : 1;
      struct {
	   /*
	   unsigned char modal : 1;
	   */
	   unsigned char sticky : 1;
	   /*
	   unsigned char maximized_v : 1;
	   unsigned char maximized_h : 1;
	   unsigned char shaded : 1;
	   unsigned char skip_taskbar : 1;
	   unsigned char skip_pager : 1;
	   unsigned char fullscreen : 1;
	   */
	   unsigned char above : 1;
	   unsigned char below : 1;
      } state;
   } x;
#endif   
#ifdef BUILD_ECORE_EVAS_FB
   struct {
      int real_w;
      int real_h;
   } fb;
#endif
#ifdef BUILD_ECORE_EVAS_BUFFER
   struct {
      void *pixels;
      Evas_Object *image;
   } buffer;
#endif
};
  
struct _Ecore_Evas
{
   Ecore_List  __list_data;
   ECORE_MAGIC;
   Evas       *evas;
   char       *driver;
   char       *name;
   int         x, y, w, h;
   short       rotation;
   char        shaped  : 1;
   char        visible : 1;
   char        should_be_visible : 1;

   Ecore_Idle_Enterer *delete_idle_enterer;
   
   Evas_Hash  *data;

   struct {
      int      x, y;
   } mouse;

   struct {
      int      w, h;
   } expecting_resize;
   
   struct {
      char           *title;
      char           *name;
      char           *clas;
      struct {
	 int          w, h;
      } min,
	max, 
	base, 
	step;
      struct {
	 Evas_Object *object;
	 char        *file;
	 int          layer;
	 struct {
	    int       x, y;
	 } hot;
      } cursor;
      int             layer;
      char            focused      : 1;
      char            iconified    : 1;
      char            borderless   : 1;
      char            override     : 1;
      char            maximized    : 1;
      char            fullscreen   : 1;
      char            avoid_damage : 1;
      char            withdrawn    : 1;
      char            sticky       : 1;
      char            request_pos  : 1;
   } prop;
   
   struct {
      void          (*fn_resize) (Ecore_Evas *ee);
      void          (*fn_move) (Ecore_Evas *ee);
      void          (*fn_show) (Ecore_Evas *ee);
      void          (*fn_hide) (Ecore_Evas *ee);
      void          (*fn_delete_request) (Ecore_Evas *ee);
      void          (*fn_destroy) (Ecore_Evas *ee);
      void          (*fn_focus_in) (Ecore_Evas *ee);
      void          (*fn_focus_out) (Ecore_Evas *ee);
      void          (*fn_mouse_in) (Ecore_Evas *ee);
      void          (*fn_mouse_out) (Ecore_Evas *ee);
      void          (*fn_pre_render) (Ecore_Evas *ee);
      void          (*fn_post_render) (Ecore_Evas *ee);
   } func;
   
   Ecore_Evas_Engine engine;
   Evas_List *sub_ecore_evas;
};

#ifdef BUILD_ECORE_X
int _ecore_evas_x_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_FB
int _ecore_evas_fb_shutdown(void);
#endif
#ifdef BUILD_ECORE_EVAS_BUFFER
int _ecore_evas_buffer_shutdown(void);
void _ecore_evas_buffer_render(Ecore_Evas *ee);
#endif

void _ecore_evas_fps_debug_init(void);
void _ecore_evas_fps_debug_shutdown(void);
void _ecore_evas_fps_debug_rendertime_add(double t);
void _ecore_evas_free(Ecore_Evas *ee);

#endif
