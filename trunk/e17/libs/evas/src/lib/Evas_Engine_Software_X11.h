#ifndef _EVAS_ENGINE_SOFTWARE_X11_H 
#define _EVAS_ENGINE_SOFTWARE_X11_H 

#include <X11/Xlib.h>

typedef struct _Evas_Engine_Info_Software_X11              Evas_Engine_Info_Software_X11;

struct _Evas_Engine_Info_Software_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;
   
   /* engine specific data & parameters it needs to set up */
   struct {
      Display  *display;
      Drawable  drawable;
      Pixmap    mask;
      Visual   *visual;
      Colormap  colormap;
      int       depth;
      int       rotation;
      
      int       alloc_grayscale : 1;
      int       debug : 1;
      int       shape_dither : 1;

      int       alloc_colors_max;
   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
   struct {
      Visual *  (*best_visual_get)   (Display *disp, int screen);
      Colormap  (*best_colormap_get) (Display *disp, int screen);
      int       (*best_depth_get)    (Display *disp, int screen);
      
      Evas_Performance *(*performance_test)         (Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth);
      void              (*performance_free)         (Evas_Performance *perf);
      char *            (*performance_data_get)     (Evas_Performance *perf);
      char *            (*performance_key_get)      (Evas_Performance *perf);
      Evas_Performance *(*performance_new)          (Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth);
      void              (*performance_build)        (Evas_Performance *perf, const char *data);
      void              (*performance_device_store) (Evas_Performance *perf);
   } func;
};
#endif


