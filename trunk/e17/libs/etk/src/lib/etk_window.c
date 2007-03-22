/** @file etk_window.c */
#include "etk_window.h"
#include <stdlib.h>
#include <string.h>

#include "etk_engine.h"
#include "etk_utils.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "config.h"

/**
 * @addtogroup Etk_Window
 * @{
 */

enum Etk_Widget_Signal_Id
{
   ETK_WINDOW_MOVED_SIGNAL,
   ETK_WINDOW_RESIZED_SIGNAL,
   ETK_WINDOW_FOCUSED_IN_SIGNAL,
   ETK_WINDOW_FOCUSED_OUT_SIGNAL,
   ETK_WINDOW_DELETE_EVENT_SIGNAL,
   ETK_WINDOW_NUM_SIGNALS
};

enum Etk_Window_Property_Id
{
   ETK_WINDOW_TITLE_PROPERTY,
   ETK_WINDOW_ICONIFIED_PROPERTY,
   ETK_WINDOW_MAXIMIZED_PROPERTY,
   ETK_WINDOW_FULLSCREEN_PROPERTY,
   ETK_WINDOW_STICKY_PROPERTY,
   ETK_WINDOW_FOCUSED_PROPERTY,
   ETK_WINDOW_DECORATED_PROPERTY,
   ETK_WINDOW_SHAPED_PROPERTY,
   ETK_WINDOW_HAS_ALPHA_PROPERTY,     
   ETK_WINDOW_SKIP_TASKBAR_PROPERTY,
   ETK_WINDOW_SKIP_PAGER_PROPERTY
};

static void _etk_window_constructor(Etk_Window *window);
static void _etk_window_destructor(Etk_Window *window);
static void _etk_window_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_window_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_window_shown_cb(Etk_Object *object, void *data);
static void _etk_window_hidden_cb(Etk_Object *object, void *data);
static void _etk_window_size_requested_cb(Etk_Object *object, Etk_Size *requested_size, void *data);
static Etk_Bool _etk_window_delete_event_handler(Etk_Window *window);
static void _etk_window_evas_position_get(Etk_Toplevel *toplevel, int *x, int *y);
static void _etk_window_screen_position_get(Etk_Toplevel *toplevel, int *x, int *y);
static void _etk_window_size_get(Etk_Toplevel *toplevel, int *w, int *h);
static void _etk_window_pointer_set(Etk_Toplevel *toplevel, Etk_Pointer_Type pointer_type);

static Etk_Signal *_etk_window_signals[ETK_WINDOW_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @internal
 * @brief Gets the type of an Etk_Window
 * @return Returns the type of an Etk_Window
 */
Etk_Type *etk_window_type_get(void)
{
   static Etk_Type *window_type = NULL;

   if (!window_type)
   {
       window_type = etk_type_new("Etk_Window", ETK_TOPLEVEL_TYPE, sizeof(Etk_Window), ETK_CONSTRUCTOR(_etk_window_constructor), ETK_DESTRUCTOR(_etk_window_destructor));
   
      _etk_window_signals[ETK_WINDOW_MOVED_SIGNAL] = etk_signal_new("moved", window_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_window_signals[ETK_WINDOW_RESIZED_SIGNAL] = etk_signal_new("resized", window_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_window_signals[ETK_WINDOW_FOCUSED_IN_SIGNAL] = etk_signal_new("focused-in", window_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_window_signals[ETK_WINDOW_FOCUSED_OUT_SIGNAL] = etk_signal_new("focused-out", window_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_window_signals[ETK_WINDOW_DELETE_EVENT_SIGNAL] = etk_signal_new("delete-event", window_type, ETK_MEMBER_OFFSET(Etk_Window, delete_event), etk_marshaller_BOOL__VOID, etk_accumulator_bool_or, NULL);
   
      etk_type_property_add(window_type, "title", ETK_WINDOW_TITLE_PROPERTY, ETK_PROPERTY_STRING, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_string(NULL));
      etk_type_property_add(window_type, "iconified", ETK_WINDOW_ICONIFIED_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "maximized", ETK_WINDOW_MAXIMIZED_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "fullscreen", ETK_WINDOW_FULLSCREEN_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "sticky", ETK_WINDOW_STICKY_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "focused", ETK_WINDOW_FOCUSED_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_TRUE));
      etk_type_property_add(window_type, "decorated", ETK_WINDOW_DECORATED_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "shaped", ETK_WINDOW_SHAPED_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "has_alpha", ETK_WINDOW_HAS_ALPHA_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));      
      etk_type_property_add(window_type, "skip-taskbar", ETK_WINDOW_SKIP_TASKBAR_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(window_type, "skip-pager", ETK_WINDOW_SKIP_PAGER_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      
      window_type->property_set = _etk_window_property_set;
      window_type->property_get = _etk_window_property_get;
   }

   return window_type;
}

/**
 * @brief Creates a new window
 * @return Returns the new window widget
 */
Etk_Widget *etk_window_new(void)
{
   return etk_widget_new(ETK_WINDOW_TYPE, "theme-group", "window", NULL);
}

/**
 * @brief Emits a delete-request on the window: it will call all the callbacks connected to the "delete_event" signal
 * and if all these callbacks return ETK_FALSE, the window will be destroyed. It has the same effect as if the user
 * had clicked on the "close" button of the window
 * @param window a window
 */
void etk_window_delete_request(Etk_Window *window)
{
   Etk_Bool result;
   
   etk_signal_emit(_etk_window_signals[ETK_WINDOW_DELETE_EVENT_SIGNAL], ETK_OBJECT(window), &result);
   if (!result)
      etk_object_destroy(ETK_OBJECT(window));
}

/**
 * @brief Sets the title of the window
 * @param window the window
 * @param title the title to set
 */ 
void etk_window_title_set(Etk_Window *window, const char *title)
{
   if (!window)
      return;
   etk_engine_window_title_set(window, title);
}

/**
 * @brief Gets the title of the window
 * @param window the window
 * @return Returns the title of the window
 */ 
const char *etk_window_title_get(Etk_Window *window)
{
   if (!window)
      return NULL;
   return etk_engine_window_title_get(window);
}

/**
 * @brief Sets the window name and the window class of a window
 * @param window a window
 * @param window_name the window name to set
 * @param window_class the window class to set
 */
void etk_window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class)
{
   if (!window)
      return;
   etk_engine_window_wmclass_set(window, window_name, window_class);
}

/**
 * @brief Moves the window to the position (x, y)
 * @param window a window
 * @param x the x position
 * @param y the y position
 */
void etk_window_move(Etk_Window *window, int x, int y)
{
   if (!window)
      return;
   etk_engine_window_move(window, x, y);
}

/**
 * @brief Resizes the window
 * @param window a window
 * @param w the new width of the window
 * @param h the new height of the window
 */
void etk_window_resize(Etk_Window *window, int w, int h)
{   
   if (!window)
      return;
   etk_engine_window_resize(window, w, h);
}

/**
 * @brief Gets the geometry of the window
 * @param window a window
 * @param x the location where to set the x position the window
 * @param y the location where to set the y position the window
 * @param w the location where to set the width of the window
 * @param h the location where to set the height of the window
 */
void etk_window_geometry_get(Etk_Window *window, int *x, int *y, int *w, int *h)
{
   if (!window)
      return;
   etk_toplevel_screen_position_get(ETK_TOPLEVEL(window), x, y);
   etk_toplevel_size_get(ETK_TOPLEVEL(window), w, h);
}

/**
 * @brief Centers a window on another window
 * @param window_to_center the window to center
 * @param window the window on which @a window_to_center will be centered. If NULL, the window will be centered on the screen
 */
void etk_window_center_on_window(Etk_Window *window_to_center, Etk_Window *window)
{
   int x, y, w, h;
   int cw, ch;
   
   if (!window_to_center)
      return;
   
   if (window_to_center->center_on_window)
   {
      etk_object_weak_pointer_remove(ETK_OBJECT(window), (void **)(&window_to_center->center_on_window));
      window_to_center->center_on_window = NULL;
   }
   
   /* If the window has not been been resized yet, we can't center it already,
    * so we queue the center-action for later */
   if (window_to_center->wait_size_request)
   {
      window_to_center->center_on_window = window;
      if (window)
         etk_object_weak_pointer_add(ETK_OBJECT(window), (void **)(&window_to_center->center_on_window));
   }
   /* Otherwise we center it right now */
   else
   {
      if (window)
         etk_window_geometry_get(window, &x, &y, &w, &h);
      else
         etk_engine_mouse_screen_geometry_get(&x, &y, &w, &h);
      etk_window_geometry_get(window_to_center, NULL, NULL, &cw, &ch);
      etk_window_move(window_to_center, x + (w - cw) / 2, y + (h - ch) / 2);
   }
}

/**
 * @brief Moves the window at the mouse position
 * @param window a window
 */
void etk_window_move_to_mouse(Etk_Window *window)
{
   int x, y;
   
   if (!window)
      return;
   
   etk_engine_mouse_position_get(&x, &y);
   etk_window_move(window, x, y);
}

/**
 * @brief Makes a window modal for another window
 * @param window_to_modal the window to make modal
 * @param window the window on which @a window_to_modal will modal'ed on, or NULL to disable the modal state
 */
/* TODO: grab the input */
void etk_window_modal_for_window(Etk_Window *window_to_modal, Etk_Window *window)
{
   if (window_to_modal)
      return;
   etk_engine_window_modal_for_window(window_to_modal, window);
}

/**
 * @brief Raises the window.
 * @param window a window
 */
void etk_window_raise(Etk_Window *window)
{
   if (!window)
      return;
   etk_engine_window_raise(window);
}

/**
 * @brief Lowers the window.
 * @param window a window
 */
void etk_window_lower(Etk_Window *window)
{
   if (!window)
      return;
   etk_engine_window_lower(window);
}

/**
 * @brief Sets the stacking layer of the window (normal, "always on top" or "always below")
 * @param window a window
 * @param stacking the stacking layer to use
 */
void etk_window_stacking_set(Etk_Window *window, Etk_Window_Stacking stacking)
{
   if (!window)
      return;
   etk_engine_window_stacking_set(window, stacking);
}

/**
 * @brief Gets the stacking layer of the window
 * @param window a window
 * @return Returns the stacking layer of the window
 */
Etk_Window_Stacking etk_window_stacking_get(Etk_Window *window)
{
   if (!window)
      return ETK_WINDOW_NORMAL;
   return etk_engine_window_stacking_get(window);
}

/**
 * @brief Sets whether or not the window is iconified (i.e. minimized)
 * @param window a window
 * @param iconified ETK_TRUE to iconify the window, ETK_FALSE to deiconify it
 */
void etk_window_iconified_set(Etk_Window *window, Etk_Bool iconified)
{
   if (!window)
      return;
   etk_engine_window_iconified_set(window, iconified);
}

/**
 * @brief Gets whether the window is iconified
 * @param window a window
 * @return Returns ETK_TRUE if the window is iconified, ETK_FALSE otherwise
 */
Etk_Bool etk_window_iconified_get(Etk_Window *window)
{
   if (!window)
      return ETK_FALSE;
   return etk_engine_window_iconified_get(window);
}

/**
 * @brief Sets whether or not the window is maximized
 * @param window a window
 * @param maximized ETK_TRUE to maximize the window, ETK_FALSE to unmaximize it
 */
void etk_window_maximized_set(Etk_Window *window, Etk_Bool maximized)
{
   if (!window)
      return;
   etk_engine_window_maximized_set(window, maximized);
}

/**
 * @brief Gets whether the window is maximized
 * @param window a window
 * @return Returns ETK_TRUE if the window is maximized, ETK_FALSE otherwise
 */
Etk_Bool etk_window_maximized_get(Etk_Window *window)
{
   if (!window)
      return ETK_FALSE;
   return etk_engine_window_maximized_get(window);
}

/**
 * @brief Places the window in the fullscreen state
 * @param window a window
 * @param fullscreen ETK_TRUE to place the window in the fullscreen state, ETK_FALSE to toggle off the fullscreen state
 */
void etk_window_fullscreen_set(Etk_Window *window, Etk_Bool fullscreen)
{
   if (!window)
      return;
   etk_engine_window_fullscreen_set(window, fullscreen);
}

/**
 * @brief Gets whether the window is in the fullscreen state
 * @param window a window
 * @return Returns ETK_TRUE if the window is in the fullscreen state, ETK_FALSE otherwise
 */
Etk_Bool etk_window_fullscreen_get(Etk_Window *window)
{
   if (!window)
      return ETK_FALSE;
   return etk_engine_window_fullscreen_get(window);
}

/**
 * @brief Sets whether or not the window is focused
 * @param window a window
 * @param focused ETK_TRUE to focus the window, ETK_FALSE to unfocus it
 */
void etk_window_focused_set(Etk_Window *window, Etk_Bool focused)
{
   if (!window)
      return;
   etk_engine_window_focused_set(window, focused);
}

/**
 * @brief Gets whether the window is focused
 * @param window a window
 * @return Returns ETK_TRUE if the window is focused, ETK_FALSE otherwise
 */
Etk_Bool etk_window_focused_get(Etk_Window *window)
{
   if (!window)
      return ETK_FALSE;
   return etk_engine_window_focused_get(window);
}

/**
 * @brief Sticks / unsticks the window: it will appear / disappear on all the virtual desktops
 * @param window a window
 * @param sticky ETK_TRUE to stick the window, ETK_FALSE to unstick it
 */
void etk_window_sticky_set(Etk_Window *window, Etk_Bool sticky)
{
   if (!window)
      return;
   etk_engine_window_sticky_set(window, sticky);
}

/**
 * @brief Gets whether the window is sticky (i.e. if it appears on all the virtual desktops)
 * @param window a window
 * @return Returns ETK_TRUE if the window is sticky, ETK_FALSE otherwise
 */
Etk_Bool etk_window_sticky_get(Etk_Window *window)
{
   if (!window)
      return ETK_FALSE;
   return etk_engine_window_sticky_get(window);
}

/**
 * @brief Sets wheter the window is decorated
 * @param window a window
 * @param decorated if @a decorated is ETK_FALSE, the border of the window will be hidden
 */
void etk_window_decorated_set(Etk_Window *window, Etk_Bool decorated)
{
   if (!window)
      return;
   etk_engine_window_decorated_set(window, decorated);
}

/**
 * @brief Gets whether the window is decorated (i.e. whether the border of the window is shown)
 * @param window a window
 * @return Returns ETK_TRUE if the window is decorated
 */
Etk_Bool etk_window_decorated_get(Etk_Window *window)
{
   if (!window)
      return ETK_TRUE;
   return etk_engine_window_decorated_get(window);
}

/**
 * @brief Sets wheter the window is shaped
 * @param window a window
 * @param shaped the shaped setting
 */
void etk_window_shaped_set(Etk_Window *window, Etk_Bool shaped)
{
   if (!window)
      return;
   etk_engine_window_shaped_set(window, shaped);
}

/**
 * @brief Gets whether the window is shaped
 * @param window a window
 * @return Returns ETK_TRUE if the window is shaped
 */
Etk_Bool etk_window_shaped_get(Etk_Window *window)
{
   if (!window)
      return ETK_TRUE;
   return etk_engine_window_shaped_get(window);
}

/**
 * @brief Sets wheter the window has an alpha channel (supports translucency)
 * @param window a window
 * @param has_alpha the alpha setting
 */
void etk_window_has_alpha_set(Etk_Window *window, Etk_Bool has_alpha)
{
   if (!window)
      return;
   etk_engine_window_has_alpha_set(window, has_alpha);
}

/**
 * @brief Gets whether the window has an alpha channel (supports translucency)
 * @param window a window
 * @return Returns ETK_TRUE if the window has an alpha channel
 */
Etk_Bool etk_window_has_alpha_get(Etk_Window *window)
{
   if (!window)
      return ETK_TRUE;
   return etk_engine_window_has_alpha_get(window);
}

/**
 * @brief Sets whether the window should not be shown in the taskbar
 * @param window a window
 * @param skip_taskbar_hint if @a skip_taskbar_hint == ETK_TRUE, the window should not be shown in the taskbar
 */
void etk_window_skip_taskbar_hint_set(Etk_Window *window, Etk_Bool skip_taskbar_hint)
{
   etk_engine_window_skip_taskbar_hint_set(window, skip_taskbar_hint);
}

/**
 * @brief Gets whether the window should not be shown in the taskbar
 * @param window a window
 * @return Returns ETK_FALSE if the window is shown in the taskbar
 */
Etk_Bool etk_window_skip_taskbar_hint_get(Etk_Window *window)
{
   return etk_engine_window_skip_taskbar_hint_get(window);
}

/**
 * @brief Sets whether the window should not be shown in the pager
 * @param window a window
 * @param skip_pager_hint if @a skip_pager_hint == ETK_TRUE, the window should not be shown in the pager
 */
void etk_window_skip_pager_hint_set(Etk_Window *window, Etk_Bool skip_pager_hint)
{
   etk_engine_window_skip_pager_hint_set(window, skip_pager_hint);
}

/**
 * @brief Gets whether the window should not be shown in the pager
 * @param window a window
 * @return Returns ETK_TRUE if the window should not be shown in the pager
 */
Etk_Bool etk_window_skip_pager_hint_get(Etk_Window *window)
{
   return etk_engine_window_skip_pager_hint_get(window);
}

/**
 * @brief A utility function to use as a callback for the "delete_event" signal.
 * It will hide the window and return ETK_TRUE to prevent the program from quitting
 * @param window the window to hide
 * @param data the data passed when the signal is emitted - unused
 * @return Return ETK_TRUE so the the program won't quit
 */
Etk_Bool etk_window_hide_on_delete(Etk_Object *window, void *data)
{   
   etk_widget_hide(ETK_WIDGET(window));
   return ETK_TRUE;
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the members and build the window */
static void _etk_window_constructor(Etk_Window *window)
{
   if (!window)
      return;
 
   window->wait_size_request = ETK_TRUE;
   window->center_on_window = NULL;
   window->delete_event = _etk_window_delete_event_handler;
   
   etk_engine_window_constructor(window);
   
   ETK_TOPLEVEL(window)->pointer_set = _etk_window_pointer_set;
   ETK_TOPLEVEL(window)->evas_position_get = _etk_window_evas_position_get;
   ETK_TOPLEVEL(window)->screen_position_get = _etk_window_screen_position_get;
   ETK_TOPLEVEL(window)->size_get = _etk_window_size_get;
   ETK_TOPLEVEL(window)->evas = etk_engine_window_evas_get(window);
   
   /* TODO: remove the font path */
   evas_font_path_append(ETK_TOPLEVEL(window)->evas, PACKAGE_DATA_DIR "/fonts/");
   etk_signal_connect("size-requested", ETK_OBJECT(window), ETK_CALLBACK(_etk_window_size_requested_cb), NULL);
   etk_signal_connect("shown", ETK_OBJECT(window), ETK_CALLBACK(_etk_window_shown_cb), NULL);
   etk_signal_connect("hidden", ETK_OBJECT(window), ETK_CALLBACK(_etk_window_hidden_cb), NULL);
   etk_object_notify(ETK_OBJECT(window), "evas");
}

/* Destroys the window */
static void _etk_window_destructor(Etk_Window *window)
{
   if (!window)
      return;
   
   etk_engine_window_destructor(window);
   
   if (window->center_on_window)
      etk_object_weak_pointer_remove(ETK_OBJECT(window->center_on_window), (void **)(&window->center_on_window));
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_window_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Window *window;

   if (!(window = ETK_WINDOW(object)) || !value)
      return;
   
   switch (property_id)
   {
      case ETK_WINDOW_TITLE_PROPERTY:
         etk_window_title_set(window, etk_property_value_string_get(value));
         break;
      case ETK_WINDOW_ICONIFIED_PROPERTY:
         etk_window_iconified_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_MAXIMIZED_PROPERTY:
         etk_window_maximized_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_FULLSCREEN_PROPERTY:
         etk_window_fullscreen_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_STICKY_PROPERTY:
         etk_window_sticky_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_FOCUSED_PROPERTY:
         etk_window_focused_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_DECORATED_PROPERTY:
         etk_window_decorated_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_SHAPED_PROPERTY:
         etk_window_shaped_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_HAS_ALPHA_PROPERTY:
         etk_window_has_alpha_set(window, etk_property_value_bool_get(value));
         break;      
      case ETK_WINDOW_SKIP_TASKBAR_PROPERTY:
         etk_window_skip_taskbar_hint_set(window, etk_property_value_bool_get(value));
         break;
      case ETK_WINDOW_SKIP_PAGER_PROPERTY:
         etk_window_skip_pager_hint_set(window, etk_property_value_bool_get(value));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_window_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Window *window;

   if (!(window = ETK_WINDOW(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_WINDOW_TITLE_PROPERTY:
         etk_property_value_string_set(value, etk_window_title_get(window));
         break;
      case ETK_WINDOW_ICONIFIED_PROPERTY:
         etk_property_value_bool_set(value, etk_window_iconified_get(window));
         break;
      case ETK_WINDOW_MAXIMIZED_PROPERTY:
         etk_property_value_bool_set(value, etk_window_maximized_get(window));
         break;
      case ETK_WINDOW_FULLSCREEN_PROPERTY:
         etk_property_value_bool_set(value, etk_window_fullscreen_get(window));
         break;
      case ETK_WINDOW_STICKY_PROPERTY:
         etk_property_value_bool_set(value, etk_window_sticky_get(window));
         break;
      case ETK_WINDOW_FOCUSED_PROPERTY:
         etk_property_value_bool_set(value, etk_window_focused_get(window));
         break;
      case ETK_WINDOW_DECORATED_PROPERTY:
         etk_property_value_bool_set(value, etk_window_decorated_get(window));
         break;
      case ETK_WINDOW_SHAPED_PROPERTY:
         etk_property_value_bool_set(value, etk_window_shaped_get(window));
         break;
      case ETK_WINDOW_HAS_ALPHA_PROPERTY:
         etk_property_value_bool_set(value, etk_window_has_alpha_get(window));
         break;      
      case ETK_WINDOW_SKIP_TASKBAR_PROPERTY:
         etk_property_value_bool_set(value, etk_window_skip_taskbar_hint_get(window));
         break;
      case ETK_WINDOW_SKIP_PAGER_PROPERTY:
         etk_property_value_bool_set(value, etk_window_skip_pager_hint_get(window));
         break;
      default:
         break;
   }
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the window is shown */
static void _etk_window_shown_cb(Etk_Object *object, void *data)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(object)) || window->wait_size_request)
      return;
   etk_engine_window_show(window);
}

/* Called when the window is hidden */
static void _etk_window_hidden_cb(Etk_Object *object, void *data)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(object)) || window->wait_size_request)
      return;
   etk_engine_window_hide(window);
}

/* Called when the "size-requested" signal is emitted */
static void _etk_window_size_requested_cb(Etk_Object *object, Etk_Size *requested_size, void *data)
{
   Etk_Window *window;
   
   if (!(window = ETK_WINDOW(object)) || !requested_size)
      return;
   
   if (requested_size->w >= 0 && requested_size->h >= 0)
   {
      etk_engine_window_size_min_set(window, requested_size->w, requested_size->h);
      
      if (window->wait_size_request)
      {
         window->wait_size_request = ETK_FALSE;
         if (etk_widget_is_visible(ETK_WIDGET(window)))
            etk_engine_window_show(window);
         if (window->center_on_window)
            etk_window_center_on_window(window, window->center_on_window);
      }
   }
}

/* Default handler for the "delete_event" signal */
static Etk_Bool _etk_window_delete_event_handler(Etk_Window *window)
{
   return ETK_FALSE;
}

/* Gets the evas position of the window */
static void _etk_window_evas_position_get(Etk_Toplevel *toplevel, int *x, int *y)
{
   etk_engine_window_evas_position_get(ETK_WINDOW(toplevel), x, y);
}

/* Gets the screen position of the window */
static void _etk_window_screen_position_get(Etk_Toplevel *toplevel, int *x, int *y)
{
   etk_engine_window_screen_position_get(ETK_WINDOW(toplevel), x, y);
}

/* Gets the size of the window */
static void _etk_window_size_get(Etk_Toplevel *toplevel, int *w, int *h)
{
   etk_engine_window_size_get(ETK_WINDOW(toplevel), w, h);
}

/* Sets the mouse pointer of the window */
static void _etk_window_pointer_set(Etk_Toplevel *toplevel, Etk_Pointer_Type pointer_type)
{
   etk_engine_window_pointer_set(ETK_WINDOW(toplevel), pointer_type);
}

/** @} */
