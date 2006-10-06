#include <stdio.h>
#include <string.h>
#include <Ecore_Evas.h>
#include <Etk.h>
#include "Etk_Engine_Ecore_Evas.h"

typedef Etk_Engine_Ecore_Evas_Window_Data Etk_Engine_Window_Data;

/* General engine functions */
Etk_Engine *engine_open();
void engine_close();

static Etk_Bool _engine_init();
static void _engine_shutdown();

/* Etk_Window functions */
static void _window_constructor(Etk_Window *window);
static void _window_destructor(Etk_Window *window);
static void _window_show(Etk_Window *window);
static void _window_hide(Etk_Window *window);
static Evas *_window_evas_get(Etk_Window *window);  
static void _window_title_set(Etk_Window *window, const char *title);
static const char *_window_title_get(Etk_Window *window);
static void _window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class);
static void _window_move(Etk_Window *window, int x, int y);
static void _window_resize(Etk_Window *window, int w, int h);
static void _window_size_min_set(Etk_Window *window, int w, int h);
static void _window_evas_position_get(Etk_Window *window, int *x, int *y);
static void _window_screen_position_get(Etk_Window *window, int *x, int *y);
static void _window_size_get(Etk_Window *window, int *w, int *h);
static void _window_iconified_set(Etk_Window *window, Etk_Bool iconified);
static Etk_Bool _window_iconified_get(Etk_Window *window);
static void _window_maximized_set(Etk_Window *windo, Etk_Bool maximized);
static Etk_Bool _window_maximized_get(Etk_Window *window);
static void _window_fullscreen_set(Etk_Window *window, Etk_Bool focused);
static Etk_Bool _window_fullscreen_get(Etk_Window *window);
static void _window_raise(Etk_Window *window);
static void _window_lower(Etk_Window *window);
static void _window_sticky_set(Etk_Window *window, Etk_Bool on);
static Etk_Bool _window_sticky_get(Etk_Window *window);
static void _window_focused_set(Etk_Window *window, Etk_Bool focused);
static Etk_Bool _window_focused_get(Etk_Window *window);
static void _window_decorated_set(Etk_Window *window, Etk_Bool decorated);
static Etk_Bool _window_decorated_get(Etk_Window *window);
static void _window_shaped_set(Etk_Window *window, Etk_Bool shaped);  
static Etk_Bool _window_shaped_get(Etk_Window *window);

static void _window_move_cb(Ecore_Evas *ecore_evas);
static void _window_resize_cb(Ecore_Evas *ecore_evas);
static void _window_focus_in_cb(Ecore_Evas *ecore_evas);
static void _window_focus_out_cb(Ecore_Evas *ecore_evas);
static void _window_sticky_changed_cb(Ecore_Evas *ecore_evas);
static void _window_delete_request_cb(Ecore_Evas *ecore_evas);


static Etk_Engine engine_info = {
   
   NULL, /* engine specific data */
   NULL, /* engine name */
   NULL, /* super (parent) engine */
   NULL, /* DL handle */
   
   _engine_init,
   _engine_shutdown,
   
   _window_constructor,
   _window_destructor,
   _window_show,
   _window_hide,
   _window_evas_get,
   _window_title_set,
   _window_title_get,
   _window_wmclass_set,
   _window_move,
   _window_resize,
   _window_size_min_set,
   _window_evas_position_get,
   _window_screen_position_get,
   _window_size_get,
   NULL, /* window_screen_geometry_get */
   NULL, /* window_modal_for_window */
   _window_iconified_set,
   _window_iconified_get,
   _window_maximized_set,
   _window_maximized_get,
   _window_fullscreen_set,
   _window_fullscreen_get,
   _window_raise,
   _window_lower,
   NULL, /* window_stacking_set */
   NULL, /* window_stacking_get */
   _window_sticky_set,
   _window_sticky_get,
   _window_focused_set,
   _window_focused_get,
   _window_decorated_set,
   _window_decorated_get,
   _window_shaped_set,
   _window_shaped_get,
   NULL, /* window_skip_taskbar_hint_set */
   NULL, /* window_skip_taskbar_hint_get */
   NULL, /* window_skip_pager_hint_set */
   NULL, /* window_skip_pager_hint_get */
   NULL, /* window_pointer_set */
   
   NULL, /* popup_window_constructor */
   NULL, /* popup_window_popup */
   NULL, /* popup_window_popdown */
   
   NULL, /* event_callback_set */
   NULL, /* mouse_position_get */
   NULL, /* mouse_screen_geometry_get */
   
   NULL, /* selection_text_set */
   NULL, /* selection_text_request */
   NULL, /* selection_clear */
   
   NULL, /* drag_constructor */
   NULL, /* drag_begin */
};

/**************************
 *
 * General engine functions
 *
 **************************/

/* Called when the engine is loaded */
Etk_Engine *engine_open()
{
   engine_info.engine_data = NULL;
   engine_info.engine_name = strdup("ecore_evas");
   return &engine_info;
}

/* Called when the engine is unloaded */
void engine_close()
{
   free(engine_info.engine_name);
}

/* Initializes the engine */
static Etk_Bool _engine_init()
{
   if (!ecore_evas_init())
   {
      ETK_WARNING("Ecore_Evas initialization failed!");
      return ETK_FALSE;
   }
   return ETK_TRUE;
}

/* Shutdowns the engine */
static void _engine_shutdown()
{
   ecore_evas_shutdown();
}

/**************************
 *
 * Etk_Window's functions
 *
 **************************/

/* Initializes the created window */
static void _window_constructor(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;

   ETK_TOPLEVEL(window)->evas = ecore_evas_get(engine_data->ecore_evas);
   ecore_evas_data_set(engine_data->ecore_evas, "_Etk_Engine::Window", window);
   ecore_evas_callback_move_set(engine_data->ecore_evas, _window_move_cb);
   ecore_evas_callback_resize_set(engine_data->ecore_evas, _window_resize_cb);
   ecore_evas_callback_focus_in_set(engine_data->ecore_evas, _window_focus_in_cb);
   ecore_evas_callback_focus_out_set(engine_data->ecore_evas, _window_focus_out_cb);
   ecore_evas_callback_sticky_set(engine_data->ecore_evas, _window_sticky_changed_cb);
   ecore_evas_callback_unsticky_set(engine_data->ecore_evas, _window_sticky_changed_cb);
   ecore_evas_callback_delete_request_set(engine_data->ecore_evas, _window_delete_request_cb);
}

/* Destroys the window */
static void _window_destructor(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   ecore_evas_free(engine_data->ecore_evas);
}

/* Shows the window */
static void _window_show(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   ecore_evas_show(engine_data->ecore_evas);
}

/* Hides the window */
static void _window_hide(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   ecore_evas_hide(engine_data->ecore_evas);
}

/* Returns the evas of the window */
static Evas *_window_evas_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   return ecore_evas_get(engine_data->ecore_evas);
}

/* Sets the title of the window */
static void _window_title_set(Etk_Window *window, const char *title)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_title_set(engine_data->ecore_evas, title);
   etk_object_notify(ETK_OBJECT(window), "title");
}

/* Returns the title of the window */
static const char *_window_title_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;  
   return ecore_evas_title_get(engine_data->ecore_evas);
}

/* Sets the WM class of the window */
static void _window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_name_class_set(engine_data->ecore_evas, window_name, window_class);
}

/* Moves the window to the given position */
static void _window_move(Etk_Window *window, int x, int y)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_move(engine_data->ecore_evas, x, y);
}

/* Resizes the window */
static void _window_resize(Etk_Window *window, int w, int h)
{
   Etk_Engine_Window_Data *engine_data;
   int min_w, min_h;
   
   engine_data = window->engine_data;            
   ecore_evas_size_min_get(engine_data->ecore_evas, &min_w, &min_h);
   window->width = ETK_MAX(w, min_w);
   window->height = ETK_MAX(h, min_h);
   ecore_evas_resize(engine_data->ecore_evas, window->width, window->height);
}

/* Sets the minimum size of the window */
static void _window_size_min_set(Etk_Window *window, int w, int h)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;      
   ecore_evas_size_min_set(engine_data->ecore_evas, w, h);
}

/* Gets the position of the window, relative to the Evas where the widgets are drawn */
static void _window_evas_position_get(Etk_Window *window, int *x, int *y)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
}

/* Gets the position of the window, relative to the screen */
static void _window_screen_position_get(Etk_Window *window, int *x, int *y)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_geometry_get(engine_data->ecore_evas, x, y, NULL, NULL);
}

/* Gets the size of the window */
static void _window_size_get(Etk_Window *window, int *w, int *h)
{
   if (w)   *w = window->width;
   if (h)   *h = window->height;
}

/* Sets whether or not the window is iconified */
static void _window_iconified_set(Etk_Window *window, Etk_Bool iconified)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_iconified_set(engine_data->ecore_evas, iconified);
}

/* Gets whether or not the window is iconified */
static Etk_Bool _window_iconified_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_iconified_get(engine_data->ecore_evas);
}

/* Sets whether or not the window is maximized */
static void _window_maximized_set(Etk_Window *window, Etk_Bool maximized)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_maximized_set(engine_data->ecore_evas, maximized);
   etk_object_notify(ETK_OBJECT(window), "maximized");
}

/* Gets whether or not the window is maximized */
Etk_Bool _window_maximized_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_maximized_get(engine_data->ecore_evas);
}

/* Sets whether or not the window is in fullscreen mode */
static void _window_fullscreen_set(Etk_Window *window, Etk_Bool fullscreen)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_fullscreen_set(engine_data->ecore_evas, fullscreen);
   etk_object_notify(ETK_OBJECT(window), "fullscreen");
}

/* Gets whether or not the window is in fullscreen mode */
static Etk_Bool _window_fullscreen_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_fullscreen_get(engine_data->ecore_evas);
}

/* Raises the window above the other windows */
static void _window_raise(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_raise(engine_data->ecore_evas);
}

/* Lowers the window below the other windows */
static void _window_lower(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_lower(engine_data->ecore_evas);
}

/* Sets whether or not the window is sticky (i.e. the window appears on all the virtual desks */
static void _window_sticky_set(Etk_Window *window, Etk_Bool on)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_sticky_set(engine_data->ecore_evas, on);
}

/* Gets whether or not the window is sticky */
static Etk_Bool _window_sticky_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_sticky_get(engine_data->ecore_evas);
}

/* Sets whether or not the window is focused */
static void _window_focused_set(Etk_Window *window, Etk_Bool focused)
{   
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_focus_set(engine_data->ecore_evas, focused);
}

/* Gets whether or not the window is focused */
static Etk_Bool _window_focused_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return ecore_evas_focus_get(engine_data->ecore_evas);
}

/* Sets whether or not the window is decorated (i.e. the window has a border) */
static void _window_decorated_set(Etk_Window *window, Etk_Bool decorated)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   ecore_evas_borderless_set(engine_data->ecore_evas, !decorated);
   etk_object_notify(ETK_OBJECT(window), "decorated");
}

/* Gets whether or not the window is decorated */
static Etk_Bool _window_decorated_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;   
   return !ecore_evas_borderless_get(engine_data->ecore_evas);   
}

/* Sets whether or not the window is shaped (i.e. the window is not rectangular) */
static void _window_shaped_set(Etk_Window *window, Etk_Bool shaped)
{   
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   ecore_evas_shaped_set(engine_data->ecore_evas, shaped);
   etk_object_notify(ETK_OBJECT(window), "shaped");
}

/* Gets whether or not the window is shaped */
static Etk_Bool _window_shaped_get(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   return ecore_evas_shaped_get(engine_data->ecore_evas);
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the window is moved */
static void _window_move_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   /* TODO: why use a func pointer here? */
   window->move_cb(window);
}

/* Called when the window is resized */
static void _window_resize_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   
   ecore_evas_geometry_get(ecore_evas, NULL, NULL, &window->width, &window->height);
   /* TODO: why use a func pointer here? */
   window->resize_cb(window);
}

/* Called when the window is focused in */
static void _window_focus_in_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   /* TODO: why use a func pointer here? We may just notify the "focused" property */
   window->focus_in_cb(window);
}

/* Called when the window is focused out */
static void _window_focus_out_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   /* TODO: why use a func pointer here? We may just notify the "focused" property */
   window->focus_out_cb(window);
}

/* Called when the window's sticky setting has changed */
static void _window_sticky_changed_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   /* TODO: why use a func pointer here? We may just notify the "sticky" property */
   window->sticky_changed_cb(window);
}

/* Called when the user wants to close the window */
static void _window_delete_request_cb(Ecore_Evas *ecore_evas)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(ecore_evas_data_get(ecore_evas, "_Etk_Engine::Window"))))
     return;
   /* TODO: why use a func pointer here? */
   window->delete_request_cb(window);   
}
