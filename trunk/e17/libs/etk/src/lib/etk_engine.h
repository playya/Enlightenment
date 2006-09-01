/** @file etk_engine.h */
#ifndef _ETK_ENGINE_H_
#define _ETK_ENGINE_H_

#include "Evas.h"
#include "etk_toplevel_widget.h"
#include "etk_window.h"
#include "etk_types.h"

struct Etk_Engine
{
   void *engine_data;
   char *engine_name;
   Etk_Engine *super;
   void *handle;
   
   Etk_Bool (*engine_init)();
   void (*engine_shutdown)();
   
   void (*window_constructor)(Etk_Window *window);
   void (*window_destructor)(Etk_Window *window);
   void (*window_show)(Etk_Window *window);
   void (*window_hide)(Etk_Window *window);
   Evas *(*window_evas_get)(Etk_Window *window);
   void (*window_title_set)(Etk_Window *window, const char *title);
   const char *(*window_title_get)(Etk_Window *window);
   void (*window_wmclass_set)(Etk_Window *window, const char *window_name, const char *window_class);
   void (*window_move)(Etk_Window *window, int x, int y);
   void (*window_resize)(Etk_Window *window, int w, int h);
   void (*window_size_min_set)(Etk_Window *window, int w, int h);
   void (*window_evas_position_get)(Etk_Window *window, int *x, int *y);
   void (*window_screen_position_get)(Etk_Window *window, int *x, int *y);
   void (*window_size_get)(Etk_Window *window, int *w, int *h);
   void (*window_screen_geometry_get)(Etk_Window *window, int *x, int *y, int *w, int *h);
   void (*window_modal_for_window)(Etk_Window *window_to_modal, Etk_Window *window);
   void (*window_iconified_set)(Etk_Window *window, Etk_Bool iconified);
   Etk_Bool (*window_iconified_get)(Etk_Window *window);
   void (*window_maximized_set)(Etk_Window *window, Etk_Bool maximized);
   Etk_Bool (*window_maximized_get)(Etk_Window *window);
   void (*window_fullscreen_set)(Etk_Window *window, Etk_Bool fullscreen);
   Etk_Bool (*window_fullscreen_get)(Etk_Window *window);
   void (*window_raise)(Etk_Window *window);
   void (*window_lower)(Etk_Window *window);
   void (*window_stacking_set)(Etk_Window *window, Etk_Window_Stacking stacking);
   Etk_Window_Stacking (*window_stacking_get)(Etk_Window *window);
   void (*window_sticky_set)(Etk_Window *window, Etk_Bool sticky);
   Etk_Bool (*window_sticky_get)(Etk_Window *window);
   void (*window_focused_set)(Etk_Window *window, Etk_Bool focused);
   Etk_Bool (*window_focused_get)(Etk_Window *window);
   void (*window_decorated_set)(Etk_Window *window, Etk_Bool decorated);
   Etk_Bool (*window_decorated_get)(Etk_Window *window);
   void (*window_shaped_set)(Etk_Window *window, Etk_Bool shaped);
   Etk_Bool (*window_shaped_get)(Etk_Window *window);
   void (*window_skip_taskbar_hint_set)(Etk_Window *window, Etk_Bool skip_taskbar_hint);
   Etk_Bool (*window_skip_taskbar_hint_get)(Etk_Window *window);
   void (*window_skip_pager_hint_set)(Etk_Window *window, Etk_Bool skip_pager_hint);
   Etk_Bool (*window_skip_pager_hint_get)(Etk_Window *window);
   void (*window_pointer_set)(Etk_Window *window, Etk_Pointer_Type pointer_type);
   
   void (*popup_window_constructor)(Etk_Popup_Window *popup_window);
   void (*popup_window_popup_at_xy)(Etk_Popup_Window *popup_window, int x, int y);
   void (*popup_window_popdown)(Etk_Popup_Window *popup_window);
   Evas_List **(*popup_window_popped_get)(void);
   
   void (*mouse_position_get)(int *x, int *y);
   void (*mouse_screen_geometry_get)(int *x, int *y, int *w, int *h);
   
   void (*drag_constructor)(Etk_Drag *drag);
   void (*drag_begin)(Etk_Drag *drag);
   
   Etk_Bool (*dnd_init)(void);
   void (*dnd_shutdown)(void);
   
   void (*clipboard_text_request)(Etk_Widget *widget);
   void (*clipboard_text_set)(Etk_Widget *widget, const char *text, int length);
   
   void (*selection_text_request)(Etk_Widget *widget);
   void (*selection_text_set)(Etk_Widget *widget, const char *text, int length);
   void (*selection_clear)(void);
};

Etk_Bool etk_engine_init();
void etk_engine_shutdown();

Evas_List *etk_engine_list_get();
Etk_Bool etk_engine_exists(const char *engine_name);
Etk_Engine *etk_engine_get();

Etk_Engine *etk_engine_load(const char *engine_name);
void etk_engine_unload(Etk_Engine *engine);
Etk_Bool etk_engine_inherit_from(Etk_Engine *engine, const char * inherit_name);

void etk_engine_window_constructor(Etk_Window *window);
void etk_engine_window_destructor(Etk_Window *window);
void etk_engine_window_show(Etk_Window *window);
void etk_engine_window_hide(Etk_Window *window);
Evas *etk_engine_window_evas_get(Etk_Window *window);
void etk_engine_window_title_set(Etk_Window *window, const char *title);
const char *etk_engine_window_title_get(Etk_Window *window);
void etk_engine_window_wmclass_set(Etk_Window *window, const char *window_name, const char *window_class);
void etk_engine_window_move(Etk_Window *window, int x, int y);
void etk_engine_window_resize(Etk_Window *window, int w, int h);
void etk_engine_window_size_min_set(Etk_Window *window, int w, int h);
void etk_engine_window_evas_position_get(Etk_Window *window, int *x, int *y);
void etk_engine_window_screen_position_get(Etk_Window *window, int *x, int *y);
void etk_engine_window_size_get(Etk_Window *window, int *w, int *h);
void etk_engine_window_screen_geometry_get(Etk_Window *window, int *x, int *y, int *w, int *h);
void etk_engine_window_modal_for_window(Etk_Window *window_to_modal, Etk_Window *window);
void etk_engine_window_iconified_set(Etk_Window *window, Etk_Bool iconified);
Etk_Bool etk_engine_window_iconified_get(Etk_Window *window);
void etk_engine_window_maximized_set(Etk_Window *window, Etk_Bool maximized);
Etk_Bool etk_engine_window_maximized_get(Etk_Window *window);
void etk_engine_window_fullscreen_set(Etk_Window *window, Etk_Bool fullscreen);
Etk_Bool etk_engine_window_fullscreen_get(Etk_Window *window);
void etk_engine_window_raise(Etk_Window *window);
void etk_engine_window_lower(Etk_Window *window);
void etk_engine_window_stacking_set(Etk_Window *window, Etk_Window_Stacking stacking);
Etk_Window_Stacking etk_engine_window_stacking_get(Etk_Window *window);
void etk_engine_window_sticky_set(Etk_Window *window, Etk_Bool on);
Etk_Bool etk_engine_window_sticky_get(Etk_Window *window);
void etk_engine_window_focused_set(Etk_Window *window, Etk_Bool focused);
Etk_Bool etk_engine_window_focused_get(Etk_Window *window);
void etk_engine_window_decorated_set(Etk_Window *window, Etk_Bool decorated);
Etk_Bool etk_engine_window_decorated_get(Etk_Window *window);
void etk_engine_window_shaped_set(Etk_Window *window, Etk_Bool shaped);
Etk_Bool etk_engine_window_shaped_get(Etk_Window *window);  
void etk_engine_window_skip_taskbar_hint_set(Etk_Window *window, Etk_Bool skip_taskbar_hint);
Etk_Bool etk_engine_window_skip_taskbar_hint_get(Etk_Window *window);
void etk_engine_window_skip_pager_hint_set(Etk_Window *window, Etk_Bool skip_pager_hint);
Etk_Bool etk_engine_window_skip_pager_hint_get(Etk_Window *window);
void etk_engine_window_pointer_set(Etk_Window *window, Etk_Pointer_Type pointer_type);
  
void etk_engine_popup_window_constructor(Etk_Popup_Window *popup_window);
void etk_engine_popup_window_popup_at_xy(Etk_Popup_Window *popup_window, int x, int y);
void etk_engine_popup_window_popdown(Etk_Popup_Window *popup_window);
Evas_List **etk_engine_popup_window_popped_get();

void etk_engine_mouse_position_get(int *x, int *y);
void etk_engine_mouse_screen_geometry_get(int *x, int *y, int *w, int *h);

void etk_engine_drag_constructor(Etk_Drag *drag);
void etk_engine_drag_begin(Etk_Drag *drag);  

Etk_Bool etk_engine_dnd_init();
void etk_engine_dnd_shutdown();

void etk_engine_clipboard_text_request(Etk_Widget *widget);
void etk_engine_clipboard_text_set(Etk_Widget *widget, const char *text, int length);

void etk_engine_selection_text_request(Etk_Widget *widget);
void etk_engine_selection_text_set(Etk_Widget *widget, const char *text, int length);
void etk_engine_selection_clear();

/** @} */

#endif
