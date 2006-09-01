/** @file etk_engine.c */
#include "etk_engine.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>
#include <Ecore_Data.h>
#include <Ecore_File.h>
#include "etk_object.h"
#include "etk_window.h"
#include "etk_popup_window.h"
#include "etk_drag.h"
#include "etk_widget.h"
#include "etk_utils.h"
#include "config.h"

/**
 * @addtogroup Etk_Engine
 * @{
 */
 
#define INHERIT(func) \
   if (!inherit_to->func) \
      inherit_to->func = inherit_from->func;

static void _etk_engine_inheritance_set(Etk_Engine *inherit_to, Etk_Engine *inherit_from);
 
static Etk_Engine *_engine = NULL;
static Evas_List *_loaded_engines = NULL;
static Evas_List *_etk_engines = NULL;

/**************************
 *
 * Implementation
 *
 **************************/
 
/**
 * @internal
 * @brief Initializes the engine subsystem
 * @return Returns ETK_TRUE on success, ETK_FALSE on failure
 */
Etk_Bool etk_engine_init()
{
   char *file;
   Ecore_List *files;
   
   if (!ecore_file_exists(PACKAGE_LIB_DIR "/etk/engines"))
      return ETK_FALSE;
   
   files = ecore_file_ls(PACKAGE_LIB_DIR "/etk/engines");
   if (!files || (ecore_list_nodes(files) == 0))
      return ETK_FALSE;
   
   ecore_list_goto_first(files);
   while ((file = ecore_list_next(files)))
   {
      int len;
      
      len = strlen(file);
      if (len > 3 && strncmp(&file[len - 3], ".so", 3) == 0)
      {
	 file[len - 3] = '\0';
	 _etk_engines = evas_list_append(_etk_engines, strdup(file));
      }
   }
   ecore_list_destroy(files);
   
   return ETK_TRUE;
}

/**
 * @internal
 * @brief Shutdowns the engine subsystem
 */
void etk_engine_shutdown()
{
   /* Shutdowns all the loaded engines */
   while (_loaded_engines)
   {
      Etk_Engine *engine;
      void *(*engine_close)();
      
      engine = _loaded_engines->data;
      _loaded_engines = evas_list_remove(_loaded_engines, engine);
      
      if (engine->engine_shutdown)
         engine->engine_shutdown();

      if (engine->handle)
      {
         if ((engine_close = dlsym(engine->handle, "engine_close")))
            engine_close();
         dlclose(engine->handle);
      }
   }
   
   while (_etk_engines)
   {
      free(_etk_engines->data);
      _etk_engines = evas_list_remove_list(_etk_engines, _etk_engines);
   }
   
   _engine = NULL;
}

/**
 * @brief Gets the list of the available engines
 * @return Returns the list of the available engines
 * @warning The returned list should not be modified or freed
 */
Evas_List *etk_engine_list_get()
{
   return _etk_engines;
}

/**
 * @brief Checks whether the engine exists
 * @param engine_name the name of the engine to find
 * @return Return ETK_TRUE if the engine exists, ETK_FALSE otherwise
 */
Etk_Bool etk_engine_exists(const char *engine_name)
{
   Evas_List *l;
   
   if (!engine_name)
      return ETK_FALSE;
   
   for (l = _etk_engines; l; l = l->next)   
   {
      if (strcmp(l->data, engine_name) == 0)
         return ETK_TRUE;
   }

   return ETK_FALSE;
}

/**
 * @brief Gets the engine currently used by Etk
 * @return Returns the engine currently used by Etk or NULL if no engine is loaded yet
 */
Etk_Engine *etk_engine_get()
{
   return _engine;
}

/**
 * @internal
 * @brief Loads an engine. The loaded engine will automatically become the engine used by Etk
 * @param engine_name the name of the engine to load
 * @return Returns the loaded engine, or NULL if the engine could not be loaded
 */
Etk_Engine *etk_engine_load(const char *engine_name)
{
   Etk_Engine *engine;
   Etk_Engine *(*engine_open)();   
   char filename[PATH_MAX];
   void *handle;
   
   if (!engine_name)
      return NULL;
   
   snprintf(filename, sizeof(filename), "%s/etk/engines/%s.so", PACKAGE_LIB_DIR, engine_name);
   
   if (!ecore_file_exists(filename))
   {
      ETK_WARNING("Etk can not find the requested engine!");
      return NULL;
   }
   
   handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);
   if (!handle)
   {
      ETK_WARNING("Etk can not dlopen the requested engine!");
      return NULL;
   }

   engine_open = dlsym(handle, "engine_open");
   if (!engine_open)
   {
      ETK_WARNING("Etk can not find an open method for this engine!");
      dlclose(handle);
      return NULL;
   }
   
   if (!(engine = engine_open()))
   {
      ETK_WARNING("Etk can not open the requested engine!");
      dlclose(handle);
      return NULL;
   }
   
   if (engine->engine_init && !engine->engine_init())
   {
      ETK_WARNING("Etk can not initialize the requested engine!");
      dlclose(handle);
      return NULL;
   }
   
   _loaded_engines = evas_list_prepend(_loaded_engines, engine);
   engine->handle = handle;
   _engine = engine;
   
   return engine;
}

/**
 * @brief Makes the engine inherit from another existing engine: the methods of the base engine
 * will be used by the inheriting engine
 * @param engine the engine which will inherit from the methods of the base engine
 * @param inherit_name the name of the engine from which @a engine will inherit
 * @return Returns ETK_TRUE on success, ETK_FALSE otherwise
 */
Etk_Bool etk_engine_inherit_from(Etk_Engine *engine, const char *inherit_name)
{
   Etk_Engine *inherit_from;
   
   if (!engine || !inherit_name)
      return ETK_FALSE;
   
   if (!(inherit_from = etk_engine_load(inherit_name)))
     return ETK_FALSE;
   
   _etk_engine_inheritance_set(engine, inherit_from);
   return ETK_TRUE;
}

/**************************
 *
 * Etk Engine's methods
 *
 **************************/

/**
 * @brief Calls the engine's method to create a window
 * @param window a window
 */
void etk_engine_window_constructor(Etk_Window *window)
{
   if (!_engine || !_engine->window_constructor)
      return;
   _engine->window_constructor(window);
}

/**
 * @brief Calls the engine's method to destroy a window
 * @param window a window
 */
void etk_engine_window_destructor(Etk_Window *window)
{
   if (!_engine || !_engine->window_destructor)
      return;   
   _engine->window_destructor(window);
}

/**
 * @brief Calls the engines's method to show a window
 * @param window a window
 */
void etk_engine_window_show(Etk_Window *window)
{
   if (!_engine || !_engine->window_show)
      return;   
   _engine->window_show(window);
}

/**
 * @brief Calls the engines's method to hide a window
 * @param window a window
 */
void etk_engine_window_hide(Etk_Window *window)
{
   if (!_engine || !_engine->window_hide)
      return;   
   _engine->window_hide(window);
}

/**
 * @brief Calls the engines's method to get the evas of a window
 * @param window a window
 * @return Returns the evas of the window, or NULL on failure
 */
Evas *etk_engine_window_evas_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_evas_get)
      return NULL;
   return _engine->window_evas_get(window);
}

/**
 * @brief Calls the engines's method to set the title of a window
 * @param window a window
 * @param title the title to set to the window
 */
void etk_engine_window_title_set(Etk_Window *window, const char *title)
{
   if (!_engine || !_engine->window_title_set)
      return;   
   _engine->window_title_set(window, title);
}

/**
 * @brief Calls the engines's method to get the title of a window
 * @param window a window
 * @return Returns the title of the window, or NULL on failure
 */
const char *etk_engine_window_title_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_title_get)
      return NULL;   
   return _engine->window_title_get(window);
}

/**
 * @brief Calls the engines's method to set the window name and the window class of a window
 * @param window a window
 * @param window_name the window name to set
 * @param window_class the window class to set
 */
void etk_engine_window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class)
{
   if (!_engine || !_engine->window_wmclass_set)
      return;   
   _engine->window_wmclass_set(window, window_name, window_class);
}

/**
 * @brief Calls the engines's method to move a window
 * @param window a window
 * @param x the x position
 * @param y the y position
 */
void etk_engine_window_move(Etk_Window *window, int x, int y)
{
   if (!_engine || !_engine->window_move)
      return;   
   _engine->window_move(window, x, y);
}

/**
 * @brief Calls the engines's method to resize a window
 * @param window a window
 * @param w the new width of the window
 * @param h the new height of the window
 */
void etk_engine_window_resize(Etk_Window *window, int w, int h)
{
   if (!_engine || !_engine->window_resize)
      return;   
   _engine->window_resize(window, w, h);
}

/**
 * @brief Calls the engines's method to set the minimum size of a window
 * @param window a window
 * @param w the minimum width of the window
 * @param h the minimum height of the window
 */
void etk_engine_window_size_min_set(Etk_Window *window, int w, int h)
{
   if (!_engine || !_engine->window_size_min_set)
      return;   
   _engine->window_size_min_set(window, w, h);
}

/**
 * @brief Calls the engines's method to get the position of a window, relative to the evas where it is drawn
 * @param window a window
 * @param x the location where to store the x evas position of the window
 * @param y the location where to store the y evas position of the window
 */
void etk_engine_window_evas_position_get(Etk_Window *window, int *x, int *y)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
   
   if (!_engine || !_engine->window_evas_position_get)
      return;
   _engine->window_evas_position_get(window, x, y);
}

/**
 * @brief Calls the engines's method to get the position of a window, relative to the screen
 * @param window a window
 * @param x the location where to store the x screen position of the window
 * @param y the location where to store the y screen position of the window
 */
void etk_engine_window_screen_position_get(Etk_Window *window, int *x, int *y)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
   
   if (!_engine || !_engine->window_screen_position_get)
      return;
   _engine->window_screen_position_get(window, x, y);
}

/**
 * @brief Calls the engines's method to get the size of a window
 * @param window a window
 * @param w the location where to store the width of the window
 * @param h the location where to store the height of the window
 */
void etk_engine_window_size_get(Etk_Window *window, int *w, int *h)
{
   if (w)   *w = 0;
   if (h)   *h = 0;
   
   if (!_engine || !_engine->window_size_get)
      return;
   _engine->window_size_get(window, w, h);
}

/**
 * @brief Calls the engines's method to get the geometry of the screen containing @a window
 * @param window a window
 * @param x the location where to store x position of the screen
 * @param y the location where to store y position of the screen
 * @param w the location where to store the width of the screen
 * @param h the location where to store the height of the screen
 */
void etk_engine_window_screen_geometry_get(Etk_Window *window, int *x, int *y, int *w, int *h)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
   if (w)   *w = 0;
   if (h)   *h = 0;
   
   if (!_engine || !_engine->window_screen_geometry_get)
      return;
   _engine->window_screen_geometry_get(window, x, y, w, h);
   
}

/* TODO: remove */
/*void etk_engine_window_center_on_window(Etk_Window *window_to_center, Etk_Window *window)
{
   if (!_engine || !_engine->window_center_on_window)
      return;   
   _engine->window_center_on_window(window_to_center, window);
}

void etk_engine_window_move_to_mouse(Etk_Window *window)
{
   if (!_engine || !_engine->window_move_to_mouse)
      return;   
   _engine->window_move_to_mouse(window);
}*/

void etk_engine_window_modal_for_window(Etk_Window *window_to_modal, Etk_Window *window)
{
   if (!_engine || !_engine->window_modal_for_window)
      return;   
   _engine->window_modal_for_window(window_to_modal, window);
}

void etk_engine_window_iconified_set(Etk_Window *window, Etk_Bool iconified)
{
   if (!_engine || !_engine->window_iconified_set)
      return;
   _engine->window_iconified_set(window, iconified);
}

Etk_Bool etk_engine_window_iconified_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_iconified_get)
      return ETK_FALSE;
   return _engine->window_iconified_get(window);
}

void etk_engine_window_maximized_set(Etk_Window *window, Etk_Bool maximized)
{
   if (!_engine || !_engine->window_maximized_set)
      return;
   _engine->window_maximized_set(window, maximized);
}

Etk_Bool etk_engine_window_maximized_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_maximized_get)
      return ETK_FALSE;   
   return _engine->window_maximized_get(window);
}

void etk_engine_window_fullscreen_set(Etk_Window *window, Etk_Bool fullscreen)
{
   if (!_engine || !_engine->window_fullscreen_set)
      return;   
   _engine->window_fullscreen_set(window, fullscreen);
}

Etk_Bool etk_engine_window_fullscreen_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_fullscreen_get)
      return ETK_FALSE;
   return _engine->window_fullscreen_get(window);
}

void etk_engine_window_raise(Etk_Window *window)
{
   if (!_engine || !_engine->window_raise)
      return;   
   _engine->window_raise(window);
}

void etk_engine_window_lower(Etk_Window *window)
{
   if (!_engine || !_engine->window_lower)
      return;   
   _engine->window_lower(window);
}

void etk_engine_window_sticky_set(Etk_Window *window, Etk_Bool sticky)
{
   if (!_engine || !_engine->window_sticky_set)
      return;   
   _engine->window_sticky_set(window, sticky);
}

Etk_Bool etk_engine_window_sticky_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_sticky_get)
      return ETK_FALSE;
   return _engine->window_sticky_get(window);
}

void etk_engine_window_focused_set(Etk_Window *window, Etk_Bool focused)
{
   if (!_engine || !_engine->window_focused_set)
      return;   
   _engine->window_focused_set(window, focused);
}

Etk_Bool etk_engine_window_focused_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_focused_get)
      return ETK_FALSE;   
   return _engine->window_focused_get(window);
}

void etk_engine_window_decorated_set(Etk_Window *window, Etk_Bool decorated)
{
   if (!_engine || !_engine->window_decorated_set)
      return;   
   _engine->window_decorated_set(window, decorated);
}

Etk_Bool etk_engine_window_decorated_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_decorated_get)
      return ETK_FALSE;   
   return _engine->window_decorated_get(window);
}

void etk_engine_window_shaped_set(Etk_Window *window, Etk_Bool shaped)
{
   if (!_engine || !_engine->window_shaped_set)
      return;   
   _engine->window_shaped_set(window, shaped);
}

Etk_Bool etk_engine_window_shaped_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_shaped_get)
      return ETK_FALSE;   
   return _engine->window_shaped_get(window);
}

void etk_engine_window_skip_taskbar_hint_set(Etk_Window *window, Etk_Bool skip_taskbar_hint)
{
   if (!_engine || !_engine->window_skip_taskbar_hint_set)
      return;   
   _engine->window_skip_taskbar_hint_set(window, skip_taskbar_hint);
}

Etk_Bool etk_engine_window_skip_taskbar_hint_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_skip_taskbar_hint_get)
      return ETK_FALSE;   
   return _engine->window_skip_taskbar_hint_get(window);
}

void etk_engine_window_skip_pager_hint_set(Etk_Window *window, Etk_Bool skip_pager_hint)
{
   if (!_engine || !_engine->window_skip_pager_hint_set)
      return;   
   _engine->window_skip_pager_hint_set(window, skip_pager_hint);
}

Etk_Bool etk_engine_window_skip_pager_hint_get(Etk_Window *window)
{
   if (!_engine || !_engine->window_skip_pager_hint_get)
      return ETK_FALSE;   
   return _engine->window_skip_pager_hint_get(window);
}

void etk_engine_window_pointer_set(Etk_Window *window, Etk_Pointer_Type pointer_type)
{
   if (!_engine || !_engine->window_pointer_set)
      return;   
   _engine->window_pointer_set(window, pointer_type);
}

void etk_engine_popup_window_constructor(Etk_Popup_Window *popup_window)
{
   if (!_engine || !_engine->popup_window_constructor)
      return;   
   _engine->popup_window_constructor(popup_window);
}

void etk_engine_popup_window_popup_at_xy(Etk_Popup_Window *popup_window, int x, int y)
{
   if (!_engine || !_engine->popup_window_popup_at_xy)
      return;
   _engine->popup_window_popup_at_xy(popup_window, x, y);
}

/* TODO: remove */
/*void etk_engine_popup_window_popup(Etk_Popup_Window *popup_window)
{
   if (!_engine || !_engine->popup_window_popup)
      return;   
   _engine->popup_window_popup(popup_window);
}*/

void etk_engine_popup_window_popdown(Etk_Popup_Window *popup_window)
{
   if (!_engine || !_engine->popup_window_popdown)
      return;   
   _engine->popup_window_popdown(popup_window);
}

/* TODO: remove?? */
Evas_List **etk_engine_popup_window_popped_get()
{
   if (!_engine || !_engine->popup_window_popped_get)
      return NULL;   
   return _engine->popup_window_popped_get();
}

/**
 * @brief Calls the engines's method to get the position of the mouse pointer, relative to the screen
 * @param window a window
 * @param x the location where to store the x position of the mouse pointer
 * @param y the location where to store the y position of the mouse pointer
 */
void etk_engine_mouse_position_get(int *x, int *y)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
   
   if (!_engine || !_engine->mouse_position_get)
      return;
   return _engine->mouse_position_get(x, y);
}

void etk_engine_mouse_screen_geometry_get(int *x, int *y, int *w, int *h)
{
   if (x)   *x = 0;
   if (y)   *y = 0;
   if (w)   *w = 0;
   if (h)   *h = 0;
   
   if (!_engine || !_engine->mouse_screen_geometry_get)
      return;
   return _engine->mouse_screen_geometry_get(x, y, w, h);
}

void etk_engine_drag_constructor(Etk_Drag *drag)
{
   if (!_engine || !_engine->drag_constructor)
      return;
   return _engine->drag_constructor(drag);
}
  
void etk_engine_drag_begin(Etk_Drag *drag)
{
   if (!_engine || !_engine->drag_begin)
      return;
   return _engine->drag_begin(drag);
}

Etk_Bool etk_engine_dnd_init()
{
   if (!_engine || !_engine->dnd_init)
           return ETK_TRUE;
   return _engine->dnd_init();
}

void etk_engine_dnd_shutdown()
{
   if (!_engine || !_engine->dnd_shutdown)
      return;
   _engine->dnd_shutdown();
}

void etk_engine_clipboard_text_request(Etk_Widget *widget)
{
   if (!_engine || !_engine->clipboard_text_request)
      return;
   _engine->clipboard_text_request(widget);
}

void etk_engine_clipboard_text_set(Etk_Widget *widget, const char *text, int length)
{
   if (!_engine || !_engine->clipboard_text_set)
      return;
   _engine->clipboard_text_set(widget, text, length);
}

void etk_engine_selection_text_request(Etk_Widget *widget)
{
   if (!_engine || !_engine->clipboard_text_request)
      return;
   _engine->selection_text_request(widget);
}

void etk_engine_selection_text_set(Etk_Widget *widget, const char *text, int length)
{
   if (!_engine || !_engine->clipboard_text_set)
      return;
   _engine->selection_text_set(widget, text, length);
}

void etk_engine_selection_clear()
{
   if (!_engine || !_engine->selection_clear)
      return;
   _engine->selection_clear();
}

/**************************
 *
 * Private functions
 *
 **************************/

/* Copies the methods of "inherit_from" to "inherit_to" */
static void _etk_engine_inheritance_set(Etk_Engine *inherit_to, Etk_Engine *inherit_from)
{
   if (!inherit_to || !inherit_from)
      return;
   
   inherit_to->super = inherit_from;
   
   INHERIT(window_constructor);
   INHERIT(window_destructor);
   INHERIT(window_show);
   INHERIT(window_hide);
   INHERIT(window_evas_get);
   INHERIT(window_title_set);
   INHERIT(window_title_get);
   INHERIT(window_wmclass_set);
   INHERIT(window_move);
   INHERIT(window_resize);
   INHERIT(window_size_min_set);
   INHERIT(window_evas_position_get);
   INHERIT(window_screen_position_get);
   INHERIT(window_size_get);
   INHERIT(window_screen_geometry_get);
   INHERIT(window_modal_for_window);
   INHERIT(window_iconified_set);
   INHERIT(window_iconified_get);
   INHERIT(window_maximized_set);
   INHERIT(window_maximized_get);
   INHERIT(window_fullscreen_set);
   INHERIT(window_fullscreen_get);
   INHERIT(window_raise);
   INHERIT(window_lower);   
   INHERIT(window_sticky_set);
   INHERIT(window_sticky_get);
   INHERIT(window_focused_set);
   INHERIT(window_focused_get);
   INHERIT(window_decorated_set);
   INHERIT(window_decorated_get);
   INHERIT(window_shaped_set);
   INHERIT(window_shaped_get);
   INHERIT(window_skip_taskbar_hint_set);
   INHERIT(window_skip_taskbar_hint_get);
   INHERIT(window_skip_pager_hint_set);
   INHERIT(window_skip_pager_hint_get);
   INHERIT(window_pointer_set);
   
   INHERIT(popup_window_constructor);
   INHERIT(popup_window_popup_at_xy);
   INHERIT(popup_window_popdown);
   INHERIT(popup_window_popped_get);
   
   INHERIT(mouse_position_get);
   INHERIT(mouse_screen_geometry_get);
   
   INHERIT(drag_constructor);
   INHERIT(drag_begin);
   
   INHERIT(dnd_init);
   INHERIT(dnd_shutdown);
   
   INHERIT(clipboard_text_request);
   INHERIT(clipboard_text_set);
   
   INHERIT(selection_text_request);
   INHERIT(selection_text_set);
   INHERIT(selection_clear);
}

/** @} */
