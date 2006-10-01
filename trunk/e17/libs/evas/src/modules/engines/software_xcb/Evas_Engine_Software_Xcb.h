#ifndef _EVAS_ENGINE_SOFTWARE_XCB_H
#define _EVAS_ENGINE_SOFTWARE_XCB_H

#include <xcb/xcb.h>

typedef struct _Evas_Engine_Info_Software_Xcb   Evas_Engine_Info_Software_Xcb;

struct _Evas_Engine_Info_Software_Xcb
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct {
      xcb_connection_t  *conn;
      xcb_drawable_t     drawable;
      xcb_drawable_t     mask;
      xcb_visualtype_t  *visual;
      xcb_colormap_t     colormap;
      int                depth;
      int                rotation;

      int                alloc_grayscale : 1;
      int                debug : 1;
      int                shape_dither : 1;
      int                destination_alpha : 1;
      int                track_mask_changes : 1;

      int                alloc_colors_max;
   } info;
   /* engine specific function calls to query stuff about the destination */
   /* engine (what visual & colormap & depth to use, performance info etc. */
   struct {
      xcb_visualtype_t * (*best_visual_get)   (xcb_connection_t *conn, int screen);
      xcb_colormap_t     (*best_colormap_get) (xcb_connection_t *conn, int screen);
      int                (*best_depth_get)    (xcb_connection_t *conn, int screen);

      Evas_Performance *(*performance_test)         (Evas             *e,
						     xcb_connection_t *conn,
						     xcb_visualtype_t *vis,
						     xcb_colormap_t    cmap,
						     xcb_drawable_t    draw,
						     int               depth);
      void              (*performance_free)         (Evas_Performance *perf);
      char *            (*performance_data_get)     (Evas_Performance *perf);
      char *            (*performance_key_get)      (Evas_Performance *perf);
      Evas_Performance *(*performance_new)          (Evas             *e,
						     xcb_connection_t *conn,
						     xcb_visualtype_t *vis,
						     xcb_colormap_t    cmap,
						     xcb_drawable_t    draw,
						     int               depth);
      void              (*performance_build)        (Evas_Performance *perf,
						     const char       *data);
      void              (*performance_device_store) (Evas_Performance *perf);
   } func;

   int mask_changed;
};

#endif /* _EVAS_ENGINE_SOFTWARE_XCB_H */
