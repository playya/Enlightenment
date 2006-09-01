/** @file etk_window.h */
#ifndef _ETK_WINDOW_H_
#define _ETK_WINDOW_H_

#include <Ecore.h>
#include "etk_toplevel_widget.h"
#include "etk_types.h"

/**
 * @defgroup Etk_Window Etk_Window
 * @{
 */

/** @brief Gets the type of a window */
#define ETK_WINDOW_TYPE       (etk_window_type_get())
/** @brief Casts the object to an Etk_Window */
#define ETK_WINDOW(obj)       (ETK_OBJECT_CAST((obj), ETK_WINDOW_TYPE, Etk_Window))
/** @brief Check if the object is an Etk_Window */
#define ETK_IS_WINDOW(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_WINDOW_TYPE))

/**
 * @struct Etk_Window
 * @brief A window which can be borderless, decorated or not, sticky, ...
 */
struct _Etk_Window
{
   /* private: */
   /* Inherit from Etk_Toplevel_Widget */
   Etk_Toplevel_Widget toplevel_widget;

   void *engine_data;
   
   int width;
   int height;
   
   Etk_Bool (*delete_event)(Etk_Window *window);
   Etk_Bool wait_size_request;
   Etk_Bool center;
   Etk_Window *center_on_window;
   Etk_Bool modal;
   Etk_Window *modal_for_window;

   void (*move_cb)(Etk_Window *window);
   void (*resize_cb)(Etk_Window *window);   
   void (*focus_in_cb)(Etk_Window *window);
   void (*focus_out_cb)(Etk_Window *window);
   void (*sticky_changed_cb)(Etk_Window *window);  
   void (*delete_request_cb)(Etk_Window *window);   
};

Etk_Type *etk_window_type_get();
Etk_Widget *etk_window_new();

void etk_window_title_set(Etk_Window *window, const char *title);
const char *etk_window_title_get(Etk_Window *window);
void etk_window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class);

void etk_window_move(Etk_Window *window, int x, int y);
void etk_window_resize(Etk_Window *window, int w, int h);
void etk_window_geometry_get(Etk_Window *window, int *x, int *y, int *w, int *h);
void etk_window_center_on_window(Etk_Window *window_to_center, Etk_Window *window);
void etk_window_move_to_mouse(Etk_Window *window);
void etk_window_modal_for_window(Etk_Window *window_to_modal, Etk_Window *window);

void etk_window_raise(Etk_Window *window);
void etk_window_lower(Etk_Window *window);

void etk_window_iconified_set(Etk_Window *window, Etk_Bool iconified);
Etk_Bool etk_window_iconified_get(Etk_Window *window);
void etk_window_maximized_set(Etk_Window *window, Etk_Bool maximized);
Etk_Bool etk_window_maximized_get(Etk_Window *window);
void etk_window_fullscreen_set(Etk_Window *window, Etk_Bool fullscreen);
Etk_Bool etk_window_fullscreen_get(Etk_Window *window);

void etk_window_focused_set(Etk_Window *window, Etk_Bool focused);
Etk_Bool etk_window_focused_get(Etk_Window *window);

void etk_window_sticky_set(Etk_Window *window, Etk_Bool sticky);
Etk_Bool etk_window_sticky_get(Etk_Window *window);
void etk_window_decorated_set(Etk_Window *window, Etk_Bool decorated);
Etk_Bool etk_window_decorated_get(Etk_Window *window);
void etk_window_shaped_set(Etk_Window *window, Etk_Bool shaped);
Etk_Bool etk_window_shaped_get(Etk_Window *window);

void etk_window_skip_taskbar_hint_set(Etk_Window *window, Etk_Bool skip_taskbar_hint);
Etk_Bool etk_window_skip_taskbar_hint_get(Etk_Window *window);
void etk_window_skip_pager_hint_set(Etk_Window *window, Etk_Bool skip_pager_hint);
Etk_Bool etk_window_skip_pager_hint_get(Etk_Window *window);

/* Misc */
Etk_Bool etk_window_hide_on_delete(Etk_Object *window, void *data);

  
/** @} */

#endif
