#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <Evas.h>
#include <Edje.h>
#include <Ecore_Evas.h>
#include <Ecore_Fb.h>
#include <Ecore_File.h>

#include "etk_types.h"
#include "etk_engine.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_theme.h"
#include "etk_utils.h"
#include "config.h"

#include "Etk_Engine_Ecore_Evas.h"
#include "Etk_Engine_Ecore_Fb.h"

#define POINTER_SIZE 64

/* fb data */
int fb_width = 0;
int fb_height = 0;

/* TODO: prop notify ? */

/* Engine specific data for Etk_Window
 * We do this to shorten the name for internal use */
typedef Etk_Engine_Ecore_Fb_Window_Data Etk_Engine_Window_Data;


/* General engine functions */
Etk_Engine *engine_open();

static Etk_Bool _engine_init();
static void _engine_shutdown();

/* Etk_Window functions */
static void _window_constructor(Etk_Window *window);
static void _window_show(Etk_Window *window);
static void _window_hide(Etk_Window *window);
static Evas *_window_evas_get(Etk_Window *window);
static void _window_move(Etk_Window *window, int x, int y);
static void _window_resize(Etk_Window *window, int w, int h);
static void _window_min_size_set(Etk_Window *window, int w, int h);
static void _window_evas_position_get(Etk_Window *window, int *x, int *y);
static void _window_screen_position_get(Etk_Window *window, int *x, int *y);
static void _window_size_get(Etk_Window *window, int *w, int *h);

/* Handlers and callbacks */
static int _mouse_move_handler(void *data, int ev_type, void *ev);

static void _window_realized_cb(Etk_Object *object, void *data);
static void _window_titlebar_mouse_down_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _window_titlebar_mouse_up_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

static Etk_Engine engine_info = {
   
   NULL, /* engine specific data */
   NULL, /* engine name */
   NULL, /* super (parent) engine */
   NULL, /* DL handle */
   
   _engine_init,
   _engine_shutdown,
   
   _window_constructor,
   NULL, /* window_destructor */
   _window_show,
   _window_hide,
   _window_evas_get,
   NULL, /* window_title_set */
   NULL, /* window_title_get */
   NULL, /* window_wmclass_set */
   _window_move,
   _window_resize,
   _window_min_size_set,
   _window_evas_position_get,
   _window_screen_position_get,
   _window_size_get,
   NULL, /* window_center_on_window */
   NULL, /* window_move_to_mouse */
   NULL, /* window_modal_for_window */
   NULL, /* window_iconified_set */
   NULL, /* window_iconified_get */
   NULL, /* window_maximized_set */
   NULL, /* window_maximized_get */
   NULL, /* window_fullscreen_set */
   NULL, /* window_fullscreen_get */
   NULL, /* window_raise */
   NULL, /* window_lower */
   NULL, /* window_sticky_set */
   NULL, /* window_sticky_get */
   NULL, /* window_focused_set */
   NULL, /* window_focused_get */
   NULL, /* window_decorated_set */
   NULL, /* window_decorated_get */
   NULL, /* window_shaped_set */
   NULL, /* window_shaped_get */
   NULL, /* window_skip_taskbar_hint_set */
   NULL, /* window_skip_taskbar_hint_get */
   NULL, /* window_skip_pager_hint_set */
   NULL, /* window_skip_pager_hint_get */
   NULL, /* window_dnd_aware_set */
   NULL, /* window_dnd_aware_get */
   NULL, /* window_pointer_set */

   NULL, /* popup_window_constructor */
   NULL, /* popup_window_popup_at_xy */
   NULL, /* popup_window_popup */
   NULL, /* popup_window_popdown */
   NULL, /* popup_window_popped_get */
   
   NULL, /* drag_constructor */
   NULL, /* drag_begin */
   
   NULL, /* dnd_init */
   NULL, /* dnd_shutdown */
   
   NULL, /* clipboard_text_request */
   NULL, /* clipboard_text_set */
   
   NULL, /* selection_text_request */
   NULL, /* selection_text_set */
   NULL, /* _selection_clear */
};

static Ecore_Evas *_ecore_evas = NULL;
static Evas *_evas = NULL;
static Evas_Object *_background_object = NULL;
static Evas_Object *_pointer_object = NULL;

static Etk_Window *_drag_window = NULL;
static int _drag_offset_x = 0;
static int _drag_offset_y = 0;

/**************************
 *
 * General engine functions
 *
 **************************/

/* Opens the engine */
Etk_Engine *engine_open()
{
   engine_info.engine_data = NULL;
   engine_info.engine_name = strdup("ecore_fb");
   return &engine_info;
}

/* Initializes the engine */
static Etk_Bool _engine_init()
{
   Ecore_List *files;
   char *filename;
   char device_path[PATH_MAX];
   
   if (!ecore_evas_init())
   {
      ETK_WARNING("Ecore_Evas initialization failed!");
      return ETK_FALSE;
   }
   if (!ecore_fb_init(NULL))
   {
      ETK_WARNING("Ecore_Fb initialization failed!");
      return ETK_FALSE;
   }
   /* needed for the pointer */
   ecore_event_handler_add(ECORE_FB_EVENT_MOUSE_MOVE, _mouse_move_handler, NULL);
   ecore_fb_size_get(&fb_width, &fb_height);
   /* Create the evas where all the windows will be drawn */
   _ecore_evas = ecore_evas_fb_new(NULL, 0, fb_width, fb_height);
   if(!_ecore_evas)
   {
      ETK_WARNING("Ecore_Evas_Fb initialization failed!");
      return ETK_FALSE;
   }
   if (!(_evas = ecore_evas_get(_ecore_evas)))
   {
      ETK_WARNING("Unable to create a FB evas");
      return ETK_FALSE;
   }
   ecore_evas_show(_ecore_evas);
   
   #if 0
   ecore_evas_ignore_events_set(_ecore_evas, 1);
   evas_event_feed_mouse_in(_evas, ecore_time_get(), NULL);
   #endif
   
   /* Create the background */
   _background_object = etk_theme_object_load(_evas, etk_theme_widget_theme_get(), "wm_background");
   evas_object_resize(_background_object, fb_width, fb_height);
   evas_object_show(_background_object);
   
   /* Create the mouse pointer */
   _pointer_object = evas_object_image_add(_evas);
   evas_object_image_file_set(_pointer_object, PACKAGE_DATA_DIR "/pointers/default_pointer.png", NULL);
   evas_object_image_fill_set(_pointer_object, 0, 0, POINTER_SIZE, POINTER_SIZE);
   evas_object_pass_events_set(_pointer_object, 1);
   evas_object_move(_pointer_object, fb_width - (POINTER_SIZE / 2), fb_height - (POINTER_SIZE / 2));
   evas_object_resize(_pointer_object, POINTER_SIZE, POINTER_SIZE);
   evas_object_show(_pointer_object);
  
   return ETK_TRUE;
}

/* Shutdowns the engine */
static void _engine_shutdown()
{
   ecore_evas_free(_ecore_evas);
   _ecore_evas = NULL;
   _evas = NULL;
   _background_object = NULL;
   _pointer_object = NULL;
   _drag_window = NULL;
   
   ecore_evas_shutdown();
   ecore_fb_shutdown();
}

/**************************
 *
 * Etk_Window's functions
 *
 **************************/

/* Initializes the new window */
static void _window_constructor(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   /* TODO: free? */
   engine_data = malloc(sizeof(Etk_Engine_Window_Data));
   engine_data->border_position.x = 0;
   engine_data->border_position.y = 0;
   engine_data->size.w = 32;
   engine_data->size.h = 32;
   engine_data->min_size.w = 0;
   engine_data->min_size.h = 0;
   engine_data->visible = ETK_FALSE;
   engine_data->border = NULL;
   window->engine_data = engine_data;
   
   ETK_TOPLEVEL_WIDGET(window)->evas = _evas;
   etk_signal_connect("realize", ETK_OBJECT(window), ETK_CALLBACK(_window_realized_cb), NULL);
}

/* Shows the window */
static void _window_show(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   engine_data->visible = ETK_TRUE;
   if (engine_data->border)
      evas_object_show(engine_data->border);
}

/* Hides the window */
static void _window_hide(Etk_Window *window)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   engine_data->visible = ETK_FALSE;
   if (engine_data->border)
      evas_object_hide(engine_data->border);
}

/* Gets the evas where the window is drawn */
static Evas *_window_evas_get(Etk_Window *window)
{
   return _evas;
}

/* Moves the window at the given position */
static void _window_move(Etk_Window *window, int x, int y)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   engine_data->border_position.x = x;
   engine_data->border_position.y = y;
   if (engine_data->border)
      evas_object_move(engine_data->border, x, y);
}

/* Resizes the window */
static void _window_resize(Etk_Window *window, int w, int h)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   engine_data->size.w = ETK_MAX(engine_data->min_size.w, w);
   engine_data->size.h = ETK_MAX(engine_data->min_size.h, h);
   if (engine_data->border && ETK_WIDGET(window)->smart_object)
   {
      int border_w, border_h;
      
      edje_extern_object_min_size_set(ETK_WIDGET(window)->smart_object, engine_data->size.w, engine_data->size.h);
      edje_object_part_swallow(engine_data->border, "content", ETK_WIDGET(window)->smart_object);
      edje_object_size_min_calc(engine_data->border, &border_w, &border_h);
      evas_object_resize(engine_data->border, border_w, border_h);
   }
}

/* Sets the min size of the window */
static void _window_min_size_set(Etk_Window *window, int w, int h)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   engine_data->min_size.w = w;
   engine_data->min_size.h = h;
   if (engine_data->size.w < w || engine_data->size.h < h)
      _window_resize(window, engine_data->size.w, engine_data->size.h);
}

/* Gets the position of the window, relative to the top left corner of the evas where the window is drawn */
static void _window_evas_position_get(Etk_Window *window, int *x, int *y)
{
   if (ETK_WIDGET(window)->smart_object)
      evas_object_geometry_get(ETK_WIDGET(window)->smart_object, x, y, NULL, NULL);
   else
   {
      if (x)   *x = 0;
      if (y)   *y = 0;
   }
}

/* Gets the position of the window, relative to the screen */
static void _window_screen_position_get(Etk_Window *window, int *x, int *y)
{
   if (ETK_WIDGET(window)->smart_object)
      evas_object_geometry_get(ETK_WIDGET(window)->smart_object, x, y, NULL, NULL);
   else
   {
      if (x)   *x = 0;
      if (y)   *y = 0;
   }
}

/* Gets the size of the window */
static void _window_size_get(Etk_Window *window, int *w, int *h)
{
   Etk_Engine_Window_Data *engine_data;
   
   engine_data = window->engine_data;
   if (w)   *w = engine_data->size.w;
   if (h)   *h = engine_data->size.h;
}

/**************************
 *
 * Handlers and callbacks
 *
 **************************/

/* Called when the window is realized: it creates the border */
static void _window_realized_cb(Etk_Object *object, void *data)
{
   Etk_Window *window;
   Etk_Engine_Window_Data *engine_data;
   int border_w, border_h;
   
   if (!(window = ETK_WINDOW(object)))
      return;
   engine_data = window->engine_data;
   
   engine_data->border = etk_theme_object_load_from_parent(_evas, ETK_WIDGET(window), NULL, "wm_border");
   edje_extern_object_min_size_set(ETK_WIDGET(window)->smart_object, engine_data->size.w, engine_data->size.h);
   edje_object_part_swallow(engine_data->border, "content", ETK_WIDGET(window)->smart_object);
   edje_object_size_min_calc(engine_data->border, &border_w, &border_h);
   
   evas_object_move(engine_data->border, engine_data->border_position.x, engine_data->border_position.y);
   evas_object_resize(engine_data->border, border_w, border_h);
   if (engine_data->visible)
      evas_object_show(engine_data->border);
   
   edje_object_signal_callback_add(engine_data->border, "mouse,down,1*", "titlebar", _window_titlebar_mouse_down_cb, window);
   edje_object_signal_callback_add(engine_data->border, "mouse,up,1*", "titlebar", _window_titlebar_mouse_up_cb, window);
   
   evas_object_raise(_pointer_object);
}

/* Called when the titlebar of the window is pressed */
static void _window_titlebar_mouse_down_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   /* TODO rasie the window */
   _drag_window = ETK_WINDOW(data);
}

/* Called when the titlebar of the window is released */
static void _window_titlebar_mouse_up_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   _drag_window = NULL;
}

/* Called when the mouse is moved */
static int _mouse_move_handler(void *data, int ev_type, void *ev)
{
   Ecore_Fb_Event_Mouse_Move *event = ev;
   
   /* Move the window to drag */
   if (_drag_window)
      etk_window_move(_drag_window, event->x - _drag_offset_x, event->y - _drag_offset_y);
   
   evas_object_move(_pointer_object, event->x - (POINTER_SIZE / 2), event->y - (POINTER_SIZE / 2));
   evas_event_feed_mouse_move(_evas, event->x, event->y, ecore_time_get(), NULL);
   
   return 1;
}


