/** @file etk_widget.c */
#include "etk_widget.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <Edje.h>
#include "etk_theme.h"
#include "etk_main.h"
#include "etk_toplevel_widget.h"
#include "etk_container.h"
#include "etk_utils.h"
#include "etk_marshallers.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_drag.h"

/**
 * @addtogroup Etk_Widget
 * @{
 */

typedef struct _Etk_Widget_Member_Object
{
   Evas_Object *object;
   Etk_Bool swallowed;
   Etk_Bool visible;
} Etk_Widget_Member_Object;

typedef struct _Etk_Widget_Swallowed_Object
{
   char *swallowing_part;
   Evas_Object *object;
   Etk_Widget *widget;
} Etk_Widget_Swallowed_Object;

enum _Etk_Widget_Signal_Id
{
   ETK_WIDGET_SHOW_SIGNAL,
   ETK_WIDGET_HIDE_SIGNAL,
   ETK_WIDGET_REALIZE_SIGNAL,
   ETK_WIDGET_UNREALIZE_SIGNAL,
   ETK_WIDGET_SIZE_REQUEST_SIGNAL,
   ETK_WIDGET_MOUSE_IN_SIGNAL,
   ETK_WIDGET_MOUSE_OUT_SIGNAL,
   ETK_WIDGET_MOUSE_MOVE_SIGNAL,
   ETK_WIDGET_MOUSE_DOWN_SIGNAL,
   ETK_WIDGET_MOUSE_UP_SIGNAL,
   ETK_WIDGET_MOUSE_CLICK_SIGNAL,
   ETK_WIDGET_MOUSE_WHEEL_SIGNAL,
   ETK_WIDGET_KEY_DOWN_SIGNAL,
   ETK_WIDGET_KEY_UP_SIGNAL,
   ETK_WIDGET_ENTER_SIGNAL,
   ETK_WIDGET_LEAVE_SIGNAL,
   ETK_WIDGET_FOCUS_SIGNAL,
   ETK_WIDGET_UNFOCUS_SIGNAL,
   ETK_WIDGET_SCROLL_SIZE_CHANGED_SIGNAL,
   ETK_WIDGET_DRAG_DROP_SIGNAL,
   ETK_WIDGET_DRAG_ENTER_SIGNAL,     
   ETK_WIDGET_DRAG_MOTION_SIGNAL,
   ETK_WIDGET_DRAG_LEAVE_SIGNAL,
   ETK_WIDGET_DRAG_BEGIN_SIGNAL,     
   ETK_WIDGET_DRAG_END_SIGNAL,
   ETK_WIDGET_SELECTION_RECEIVED_SIGNAL,
   ETK_WIDGET_CLIPBOARD_RECEIVED_SIGNAL,
   ETK_WIDGET_NUM_SIGNALS
};

enum _Etk_Widget_Property_Id
{
   ETK_WIDGET_NAME_PROPERTY,
   ETK_WIDGET_PARENT_PROPERTY,
   ETK_WIDGET_THEME_FILE_PROPERTY,
   ETK_WIDGET_THEME_GROUP_PROPERTY,
   ETK_WIDGET_THEME_PARENT_PROPERTY,
   ETK_WIDGET_GEOMETRY_PROPERTY,
   ETK_WIDGET_WIDTH_REQUEST_PROPERTY,
   ETK_WIDGET_HEIGHT_REQUEST_PROPERTY,
   ETK_WIDGET_VISIBLE_PROPERTY,
   ETK_WIDGET_VISIBILITY_LOCKED_PROPERTY,
   ETK_WIDGET_REPEAT_MOUSE_EVENTS_PROPERTY,
   ETK_WIDGET_PASS_MOUSE_EVENTS_PROPERTY,
   ETK_WIDGET_HAS_EVENT_OBJECT_PROPERTY,
   ETK_WIDGET_FOCUSABLE_PROPERTY,
   ETK_WIDGET_FOCUS_ON_PRESS_PROPERTY,
   ETK_WIDGET_CAN_PASS_FOCUS_PROPERTY
};

static void _etk_widget_constructor(Etk_Widget *widget);
static void _etk_widget_destructor(Etk_Widget *widget);
static void _etk_widget_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_widget_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);

static void _etk_widget_show_handler(Etk_Widget *widget);
static void _etk_widget_enter_handler(Etk_Widget *widget);
static void _etk_widget_leave_handler(Etk_Widget *widget);
static void _etk_widget_focus_handler(Etk_Widget *widget);
static void _etk_widget_unfocus_handler(Etk_Widget *widget);

static void _etk_widget_drag_drop_handler(Etk_Widget *widget);
static void _etk_widget_drag_motion_handler(Etk_Widget *widget);
static void _etk_widget_drag_enter_handler(Etk_Widget *widget);
static void _etk_widget_drag_leave_handler(Etk_Widget *widget);
static void _etk_widget_drag_begin_handler(Etk_Widget *widget);
static void _etk_widget_drag_end_handler(Etk_Widget *widget);

static void _etk_widget_mouse_in_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_signal_mouse_in_cb(Etk_Object *object, Etk_Event_Mouse_In_Out *event, void *data);
static void _etk_widget_mouse_out_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_signal_mouse_out_cb(Etk_Object *object, Etk_Event_Mouse_In_Out *event, void *data);
static void _etk_widget_mouse_move_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_mouse_down_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_signal_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Up_Down *event, void *data);
static void _etk_widget_mouse_up_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_mouse_wheel_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_key_down_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_signal_key_down_cb(Etk_Object *object, Etk_Event_Key_Up_Down *event, void *data);
static void _etk_widget_key_up_cb(void *data, Evas *evas, Evas_Object *object, void *event_info);
static void _etk_widget_toplevel_evas_changed_cb(Etk_Object *object, const char *property_name, void *data);

static void _etk_widget_realize(Etk_Widget *widget);
static void _etk_widget_unrealize(Etk_Widget *widget);

static void _etk_widget_toplevel_parent_set(Etk_Widget *widget, Etk_Toplevel_Widget *toplevel_parent);
static void _etk_widget_realize_all(Etk_Widget *widget);
static void _etk_widget_unrealize_all(Etk_Widget *widget);
static void _etk_widget_realize_on_theme_file_change(Etk_Widget *widget, Etk_Bool force_realize);
static void _etk_widget_realize_on_theme_change(Etk_Widget *widget);

static void _etk_widget_redraw_queue_recursive(Etk_Widget *widget);
static Etk_Bool _etk_widget_theme_object_swallow_full(Etk_Widget *swallowing_widget, const char *part, Evas_Object *object, Etk_Widget *widget);

static void _etk_widget_child_add(Etk_Widget *parent, Etk_Widget *child);
static void _etk_widget_child_remove(Etk_Widget *parent, Etk_Widget *child);
static void _etk_widget_object_add_to_smart(Etk_Widget *widget, Evas_Object *object);

static Evas_List *_etk_widget_member_object_find(Etk_Widget *widget, Evas_Object *object);
static void _etk_widget_member_object_intercept_show_cb(void *data, Evas_Object *obj);
static void _etk_widget_member_object_intercept_hide_cb(void *data, Evas_Object *obj);

static Evas_Object *_etk_widget_smart_object_add(Evas *evas, Etk_Widget *widget);
static void _etk_widget_smart_object_move_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _etk_widget_smart_object_resize_cb(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _etk_widget_smart_object_show_cb(Evas_Object *obj);
static void _etk_widget_smart_object_hide_cb(Evas_Object *obj);
static void _etk_widget_smart_object_clip_set_cb(Evas_Object *object, Evas_Object *clip);
static void _etk_widget_smart_object_clip_unset_cb(Evas_Object *object);
static void _etk_widget_dnd_drag_mouse_move_cb(Etk_Object *object, void *event, void *data);
static void _etk_widget_dnd_drag_end_cb(Etk_Object *object, void *data);  

static Etk_Signal *_etk_widget_signals[ETK_WIDGET_NUM_SIGNALS];
static Etk_Bool _etk_widget_propagate_key_event = ETK_TRUE;
static Etk_Bool _etk_widget_intercept_show_hide = ETK_TRUE;
static Evas_Smart *_etk_widget_smart_object_smart = NULL;
static Evas_List *_etk_widget_dnd_dest_widgets = NULL;
static Evas_List *_etk_widget_dnd_source_widgets = NULL;
static Etk_Bool _etk_dnd_drag_start = ETK_TRUE;

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Widget
 * @return Returns the type on an Etk_Widget
 */
Etk_Type *etk_widget_type_get()
{
   static Etk_Type *widget_type = NULL;

   if (!widget_type)
   {
      widget_type = etk_type_new("Etk_Widget", ETK_OBJECT_TYPE, sizeof(Etk_Widget), ETK_CONSTRUCTOR(_etk_widget_constructor), ETK_DESTRUCTOR(_etk_widget_destructor));

      _etk_widget_signals[ETK_WIDGET_SHOW_SIGNAL] =          etk_signal_new("show",          widget_type, ETK_MEMBER_OFFSET(Etk_Widget, show),    etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_HIDE_SIGNAL] =          etk_signal_new("hide",          widget_type, -1,                                     etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_REALIZE_SIGNAL] =       etk_signal_new("realize",       widget_type, -1,                                     etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_UNREALIZE_SIGNAL] =     etk_signal_new("unrealize",     widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_SIZE_REQUEST_SIGNAL] =  etk_signal_new("size_request",  widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_IN_SIGNAL] =      etk_signal_new("mouse_in",      widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_OUT_SIGNAL] =     etk_signal_new("mouse_out",     widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_MOVE_SIGNAL] =    etk_signal_new("mouse_move",    widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_DOWN_SIGNAL] =    etk_signal_new("mouse_down",    widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_UP_SIGNAL] =      etk_signal_new("mouse_up",      widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_CLICK_SIGNAL] =   etk_signal_new("mouse_click",   widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_MOUSE_WHEEL_SIGNAL] =   etk_signal_new("mouse_wheel",   widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_KEY_DOWN_SIGNAL] =      etk_signal_new("key_down",      widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_KEY_UP_SIGNAL] =        etk_signal_new("key_up",        widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_ENTER_SIGNAL] =         etk_signal_new("enter",         widget_type, ETK_MEMBER_OFFSET(Etk_Widget, enter),   etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_LEAVE_SIGNAL] =         etk_signal_new("leave",         widget_type, ETK_MEMBER_OFFSET(Etk_Widget, leave),   etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_FOCUS_SIGNAL] =         etk_signal_new("focus",         widget_type, ETK_MEMBER_OFFSET(Etk_Widget, focus),   etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_UNFOCUS_SIGNAL] =       etk_signal_new("unfocus",       widget_type, ETK_MEMBER_OFFSET(Etk_Widget, unfocus), etk_marshaller_VOID__VOID,    NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_DROP_SIGNAL] =     etk_signal_new("drag_drop",     widget_type, -1,                                     etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_MOTION_SIGNAL] =   etk_signal_new("drag_motion",   widget_type, ETK_MEMBER_OFFSET(Etk_Widget, drag_motion), etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_ENTER_SIGNAL] =    etk_signal_new("drag_enter",    widget_type, ETK_MEMBER_OFFSET(Etk_Widget, drag_enter),  etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_LEAVE_SIGNAL] =    etk_signal_new("drag_leave",    widget_type, ETK_MEMBER_OFFSET(Etk_Widget, drag_leave),  etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_BEGIN_SIGNAL] =    etk_signal_new("drag_begin",    widget_type, ETK_MEMBER_OFFSET(Etk_Widget, drag_begin),  etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_DRAG_END_SIGNAL] =      etk_signal_new("drag_end",      widget_type, ETK_MEMBER_OFFSET(Etk_Widget, drag_end),    etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_SELECTION_RECEIVED_SIGNAL] =   etk_signal_new("selection_received",    widget_type, -1,                      etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_CLIPBOARD_RECEIVED_SIGNAL] =   etk_signal_new("clipboard_received",    widget_type, -1,                      etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_widget_signals[ETK_WIDGET_SCROLL_SIZE_CHANGED_SIGNAL] =  etk_signal_new("scroll_size_changed",   widget_type, -1,                      etk_marshaller_VOID__VOID,    NULL, NULL);
      
      etk_type_property_add(widget_type, "name",              ETK_WIDGET_NAME_PROPERTY,              ETK_PROPERTY_STRING,  ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_string(NULL));
      etk_type_property_add(widget_type, "parent",            ETK_WIDGET_PARENT_PROPERTY,            ETK_PROPERTY_POINTER, ETK_PROPERTY_READABLE,          etk_property_value_pointer(NULL));
      etk_type_property_add(widget_type, "theme_file",        ETK_WIDGET_THEME_FILE_PROPERTY,        ETK_PROPERTY_STRING,  ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_string(NULL));
      etk_type_property_add(widget_type, "theme_group",       ETK_WIDGET_THEME_GROUP_PROPERTY,       ETK_PROPERTY_STRING,  ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_string(NULL));
      etk_type_property_add(widget_type, "theme_parent",      ETK_WIDGET_THEME_PARENT_PROPERTY,      ETK_PROPERTY_POINTER, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_pointer(NULL));
      etk_type_property_add(widget_type, "geometry",          ETK_WIDGET_GEOMETRY_PROPERTY,          ETK_PROPERTY_OTHER,   ETK_PROPERTY_NO_ACCESS,         NULL);
      etk_type_property_add(widget_type, "width_request",     ETK_WIDGET_WIDTH_REQUEST_PROPERTY,     ETK_PROPERTY_INT,     ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_int(-1));
      etk_type_property_add(widget_type, "height_request",    ETK_WIDGET_HEIGHT_REQUEST_PROPERTY,    ETK_PROPERTY_INT,     ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_int(-1));
      etk_type_property_add(widget_type, "visible",           ETK_WIDGET_VISIBLE_PROPERTY,           ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "visibility_locked", ETK_WIDGET_VISIBILITY_LOCKED_PROPERTY, ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "focusable",         ETK_WIDGET_FOCUSABLE_PROPERTY,         ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "focus_on_press",    ETK_WIDGET_FOCUS_ON_PRESS_PROPERTY,    ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "can_pass_focus",    ETK_WIDGET_CAN_PASS_FOCUS_PROPERTY,    ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_TRUE));
      etk_type_property_add(widget_type, "repeat_mouse_events", ETK_WIDGET_REPEAT_MOUSE_EVENTS_PROPERTY, ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "pass_mouse_events",   ETK_WIDGET_PASS_MOUSE_EVENTS_PROPERTY,   ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));
      etk_type_property_add(widget_type, "has_event_object",    ETK_WIDGET_HAS_EVENT_OBJECT_PROPERTY,    ETK_PROPERTY_BOOL,    ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(ETK_FALSE));

      widget_type->property_set = _etk_widget_property_set;
      widget_type->property_get = _etk_widget_property_get;
   }

   return widget_type;
}

/**
 * @brief Creates a new widget according to the type of the widget
 * @param widget_type the type of the widget to create
 * @param first_property the name of the first property value
 * @param ... the value of the first argument, followed by any number of name/argument-value pairs, terminated with NULL
 * @return Returns the new Etk_Widget
 */
Etk_Widget *etk_widget_new(Etk_Type *widget_type, const char *first_property, ...)
{
   Etk_Widget *new_widget;
   va_list args;

   if (!widget_type)
      return NULL;

   va_start(args, first_property);
   new_widget = ETK_WIDGET(etk_object_new_valist(widget_type, first_property, args));
   va_end(args);

   return new_widget;
}

/**
 * @brief Sets the name of the widget
 * @param widget a widget
 * @param name the name to set
 */
void etk_widget_name_set(Etk_Widget *widget, const char *name)
{
   if (!widget || widget->name == name)
      return;

   free(widget->name);
   widget->name = strdup(name);
   etk_object_notify(ETK_OBJECT(widget), "name");
}

/**
 * @brief Gets the name of the widget
 * @param widget a widget
 * @return Returns the name of the widget
 */
const char *etk_widget_name_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->name;
}

/**
 * @brief Gets the geometry of the widget, relative to the top left corner of its toplevel parent
 * @param widget a widget
 * @param x the location where to set the x position of the widget
 * @param y the location where to set the y position of the widget
 * @param w the location where to set the width of the widget
 * @param h the location where to set the height of the widget
 */
void etk_widget_geometry_get(Etk_Widget *widget, int *x, int *y, int *w, int *h)
{
   if (x)   *x = widget ? widget->geometry.x : 0;
   if (y)   *y = widget ? widget->geometry.y : 0;
   if (w)   *w = widget ? widget->geometry.w : 0;
   if (h)   *h = widget ? widget->geometry.h : 0;
}

/**
 * @brief Gets the inner geometry of the widget, relative to the top left corner of its toplevel parent. @n
 * The inner geometry takes account (TODO: english??) of the insets (horizontal and vertical paddings) defined by the theme
 * @param widget a widget
 * @param x the location where to set the inner x position of the widget
 * @param y the location where to set the inner y position of the widget
 * @param w the location where to set the inner width of the widget
 * @param h the location where to set the inner height of the widget
 */
void etk_widget_inner_geometry_get(Etk_Widget *widget, int *x, int *y, int *w, int *h)
{
   if (x)   *x = widget ? widget->inner_geometry.x : 0;
   if (y)   *y = widget ? widget->inner_geometry.y : 0;
   if (w)   *w = widget ? widget->inner_geometry.w : 0;
   if (h)   *h = widget ? widget->inner_geometry.h : 0;
}

/**
 * @brief Gets the toplevel widget that contains @a widget
 * @param widget a widget
 * @return Returns the toplevel widget if @a widget is contained in a toplevel widget, NULL on failure
 */
Etk_Toplevel_Widget *etk_widget_toplevel_parent_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->toplevel_parent;
}

/**
 * @brief Gets the evas of the toplevel widget that contains @a widget
 * @param widget a widget
 * @return Returns the evas if @a widget is contained in a toplevel widget, NULL on failure
 */
Evas *etk_widget_toplevel_evas_get(Etk_Widget *widget)
{
   if (!widget || !widget->toplevel_parent)
      return NULL;
   return etk_toplevel_widget_evas_get(widget->toplevel_parent);
}

/**
 * @brief Sets the theme file of the widget
 * @param widget a widget
 * @param theme_file the path of the .edj theme file. if NULL, it will use the theme file of its theme parent (or parent if is has no theme parent). @n
 * If none of its theme parents has a theme file, it will use the current theme of Etk. @n
 * Children of the widget will automatically inherit this theme file if they have no theme file already set
 */
void etk_widget_theme_file_set(Etk_Widget *widget, const char *theme_file)
{
   if (!widget)
      return;

   if (theme_file != widget->theme_file)
   {
      free(widget->theme_file);
      widget->theme_file = theme_file ? strdup(theme_file) : NULL;
      etk_object_notify(ETK_OBJECT(widget), "theme_file");
      _etk_widget_realize_on_theme_file_change(widget, ETK_TRUE);
   }
}

/**
 * @brief Gets the theme file of the widget
 * @param widget a widget
 * @return Returns the theme file used by the widget (if NULL, it uses the theme file of its parent)
 * @note To get the theme file really used for the theme of the widget, you have to walk through the parents of the widget @n
 * until you find the first parent that uses a non-null theme file. This is the theme file used of the widget. @n
 * If no parent has a non-null theme file, then it uses the current theme of Etk. You can get it with @a etk_theme_widget_theme_get()
 * @see etk_widget_theme_file_set()
 */
const char *etk_widget_theme_file_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->theme_file;
}

/**
 * @brief Sets the theme group of the widget. The theme group regroups all the theme parts of a "logical group of widgets". @n
 * For example, the theme group for an Etk_Tree is "tree". It regroups the theme parts for the tree background, @n
 * for the tree rows, for the expanders... So changing the theme group of a widget will change the whole appearance of the widget @n
 * including the appearance of the children of the widgets, if they share the same theme group
 * @param widget a widget
 * @param theme_context the theme group to use
 * @note Widgets will automatically inherit the theme group of their theme parent, or parent if they have no theme parent
 */
void etk_widget_theme_group_set(Etk_Widget *widget, const char *theme_group)
{
   if (!widget)
      return;

   if (theme_group != widget->theme_group)
   {
      free(widget->theme_group);
      widget->theme_group = theme_group ? strdup(theme_group) : NULL;
      etk_object_notify(ETK_OBJECT(widget), "theme_group");
      _etk_widget_realize_on_theme_change(widget);
   }
}

/**
 * @brief Sets the theme parent of the widget. The widget will use the theme file and theme group of its theme parent
 * @param widget a widget
 * @param theme_parent The theme parent to use
 */
void etk_widget_theme_parent_set(Etk_Widget *widget, Etk_Widget *theme_parent)
{
   if (!widget || widget->theme_parent == theme_parent)
      return;
   
   if (widget->theme_parent)
      widget->theme_parent->theme_children = evas_list_remove(widget->theme_parent->theme_children, widget);
   
   if (theme_parent)
      theme_parent->theme_children = evas_list_append(theme_parent->theme_children, widget);
   widget->theme_parent = theme_parent;
   
   _etk_widget_realize_on_theme_change(widget);
}

/**
 * @brief Gets the theme parent of the widget
 * @param widget a widget
 * @return Returns the theme parent of the widget. If it's NULL, it means that the widget gets its theme file and its @n
 * theme group from its "normal" parent (i.e. the widget that contains it)
 */
Etk_Widget *etk_widget_theme_parent_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->theme_parent;
}

/**
 * @brief Sets the parent of the widget. Used mainly by widget implementations. You do not have to call it yourself!
 * @param widget a widget
 * @param parent the new parent
 * @note etk_widget_parent_set(widget, parent) is equivalent to etk_widget_parent_set(widget, parent, ETK_TRUE)
 */
void etk_widget_parent_set(Etk_Widget *widget, Etk_Widget *parent)
{
   etk_widget_parent_set_full(widget, parent, ETK_TRUE);
}

/**
 * @brief Sets the parent of the widget. Used mainly by widget implementations. You do not have to call it yourself!
 * @param widget a widget
 * @param parent the new parent
 * @param remove_from_container if @a remove_from_container is ETK_TRUE and if the parent of the widget is a container, @n
 * the "remove_child" function of the container parent will be called. So @a remove_from_container should most of the time @n
 * set to ETK_TRUE, except when etk_widget_parent_set_full is called from remove_child" function of a container, in order @n
 * to avoid an infinite loop
 */
void etk_widget_parent_set_full(Etk_Widget *widget, Etk_Widget *parent, Etk_Bool remove_from_container)
{
   Etk_Toplevel_Widget *previous_toplevel;
   Evas *previous_evas, *new_evas;
   Etk_Widget *toplevel;

   if (!widget || widget->parent == parent)
      return;
   
   if (widget->parent)
   {
      if (remove_from_container && ETK_IS_CONTAINER(widget->parent))
         etk_container_remove(ETK_CONTAINER(widget->parent), widget);
      _etk_widget_child_remove(widget->parent, widget);
   }
   if (parent)
      _etk_widget_child_add(parent, widget);
   widget->parent = parent;

   previous_toplevel = widget->toplevel_parent;
   for (toplevel = widget; toplevel->parent; toplevel = toplevel->parent);
   if (ETK_IS_TOPLEVEL_WIDGET(toplevel))
      widget->toplevel_parent = ETK_TOPLEVEL_WIDGET(toplevel);
   else
      widget->toplevel_parent = NULL;

   if ((widget->toplevel_parent != previous_toplevel))
      _etk_widget_toplevel_parent_set(widget, widget->toplevel_parent);

   previous_evas = etk_toplevel_widget_evas_get(previous_toplevel);
   new_evas = etk_widget_toplevel_evas_get(widget);
   
   /* Realize/unrealize the children that need to be */
   /* TODO: Only realize children that needs to be? */
   if (new_evas)
      _etk_widget_realize_on_theme_change(widget);
   else if (widget->realized)
      _etk_widget_unrealize_all(widget);

   etk_object_notify(ETK_OBJECT(widget), "parent");
}

/**
 * @brief Gets the parent of the widget
 * @param widget a widget
 * @return Returns the parent of the widget
 */
Etk_Widget *etk_widget_parent_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->parent;
}

/**
 * @brief Sets whether the widget should have an event object. An event object is a simple invisible rectangle that grabs @n
 * mouse events. It can be useful for example if you want to know when a container with no theme object (a table, a box, ...) @n
 * is clicked. If a widget already has a theme object, then this function has no effect (the theme object will be used as @,
 * the event object.
 * @param widget a widget
 * @param has_event_object ETK_TRUE to add an event object to the widget
 */
void etk_widget_has_event_object_set(Etk_Widget *widget, Etk_Bool has_event_object)
{
   Evas *evas;
   
   if (!widget || widget->has_event_object == has_event_object)
      return;
   
   widget->has_event_object = has_event_object;
   
   if (has_event_object)
   {
      if (!widget->theme_object && (evas = etk_widget_toplevel_evas_get(widget)))
      {
         widget->event_object = evas_object_rectangle_add(evas);
         evas_object_color_set(widget->event_object, 255, 255, 255, 0);
         evas_object_repeat_events_set(widget->event_object, 1);
         evas_object_show(widget->event_object);
         etk_widget_member_object_add(widget, widget->event_object);
         etk_widget_redraw_queue(widget);
      }
   }
   else if (widget->event_object)
   {
      etk_widget_member_object_del(widget, widget->event_object);
      evas_object_del(widget->event_object);
      widget->event_object = NULL;
   }
   
   etk_object_notify(ETK_OBJECT(widget), "has_event_object");
}

/**
 * @brief Gets whether the widget has an event object
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget has an event object
 */
Etk_Bool etk_widget_has_event_object_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->has_event_object;
}

/**
 * @brief Sets whether the widget should repeat the mouse events it receives
 * @param widget a widget
 * @param repeat_mouse_events if @a repeat_mouse_events is ETK_TRUE, the parent widget will also receive the mouse events
 */
void etk_widget_repeat_mouse_events_set(Etk_Widget *widget, Etk_Bool repeat_mouse_events)
{
   if (!widget)
      return;

   widget->repeat_mouse_events = repeat_mouse_events;
   if (widget->smart_object)
      evas_object_repeat_events_set(widget->smart_object, repeat_mouse_events);
   etk_object_notify(ETK_OBJECT(widget), "repeat_mouse_events");
}

/**
 * @brief Checks whether the widget repeats the mouse events it receives
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget repeats the mouse events it receives
 */
Etk_Bool etk_widget_repeat_mouse_events_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->repeat_mouse_events;
}

/**
 * @brief Sets whether the widget should pass the mouse events it receives to its parent
 * @param widget a widget
 * @param pass_mouse_events if @a pass_mouse_events is ETK_TRUE, the mouse events will be directly sent to its parent @n
 * @a widget won't receive the mouse events anymore
 */
void etk_widget_pass_mouse_events_set(Etk_Widget *widget, Etk_Bool pass_mouse_events)
{
   if (!widget)
      return;

   widget->pass_mouse_events = pass_mouse_events;
   if (widget->smart_object)
      evas_object_pass_events_set(widget->smart_object, pass_mouse_events);
   etk_object_notify(ETK_OBJECT(widget), "pass_mouse_events");
}

/**
 * @brief Checks whether the widget passes the mouse events it receives to its parent
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget passes the mouse events it receives
 */
Etk_Bool etk_widget_pass_mouse_events_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->pass_mouse_events;
}

/**
 * @brief Stops the propagation of the current keyboard event so the parents of the widget @n
 * which has received the event won't be notified
 */
void etk_widget_key_event_propagation_stop()
{
   _etk_widget_propagate_key_event = ETK_FALSE;
}

/**
 * @brief Shows the widget. The widget will effectively be shown if all its parents are also shown
 * @param widget a widget
 */
void etk_widget_show(Etk_Widget *widget)
{
   if (!widget || widget->visible)
      return;

   widget->visible = ETK_TRUE;
   if (widget->smart_object && (!widget->parent || (widget->parent->smart_object && evas_object_visible_get(widget->parent->smart_object))))
      evas_object_show(widget->smart_object);
   etk_widget_size_recalc_queue(widget);
   etk_object_notify(ETK_OBJECT(widget), "visible");
   
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_SHOW_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Recursively shows the widget and its child
 * @param widget a widget
 */
void etk_widget_show_all(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;

   if (!widget->visibility_locked)
     etk_widget_show(widget);
   for (l = widget->children; l; l = l->next)
      etk_widget_show_all(ETK_WIDGET(l->data));
}

/**
 * @brief Hides the widget
 * @param widget a widget
 */
void etk_widget_hide(Etk_Widget *widget)
{
   if (!widget || !widget->visible)
      return;

   widget->visible = ETK_FALSE;
   if (widget->smart_object)
      evas_object_hide(widget->smart_object);
   etk_widget_size_recalc_queue(widget);
   etk_object_notify(ETK_OBJECT(widget), "visible");
   
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_HIDE_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Recursively hides the widget and its child
 * @param widget a widget
 */
void etk_widget_hide_all(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;

   if (!widget->visibility_locked)
      etk_widget_hide(widget);
   for (l = widget->children; l; l = l->next)
      etk_widget_hide_all(ETK_WIDGET(l->data));
}

/**
 * @brief Checks if the object is visible
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget is visible, ETK_FALSE otherwise
 */
Etk_Bool etk_widget_is_visible(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->visible;
}

/**
 * @brief Sets whether the widget is affected by etk_widget_show_all() and etk_widget_hide_all(). It's mainly used for implementation of widgets
 * @param widget a widget
 * @param visibility_locked has to be set to ETK_TRUE if etk_widget_show_all() and etk_widget_hide_all() should not affect the visibility of the widget
 */
void etk_widget_visibility_locked_set(Etk_Widget *widget, Etk_Bool visibility_locked)
{
   if (!widget || widget->visibility_locked == visibility_locked)
      return;
   widget->visibility_locked = visibility_locked;
   etk_object_notify(ETK_OBJECT(widget), "visibility_locked");
}

/**
 * @brief Gets whether the widget is affected by etk_widget_show_all() and etk_widget_hide_all()
 * @param widget a widget
 * @return Returns ETK_TRUE if etk_widget_show_all() and etk_widget_hide_all() does not affect the visibility of the widget
 */
Etk_Bool etk_widget_visibility_locked_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->visibility_locked;
}

/**
 * @brief Raises a widget: it will be above all the other widgets that have the same parent
 * @param widget the widget to raise
 */
void etk_widget_raise(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget || !widget->parent)
      return;
   
   if ((l = evas_list_find_list(widget->parent->children, widget)))
   {
      widget->parent->children = evas_list_remove_list(widget->parent->children, l);
      widget->parent->children = evas_list_append(widget->parent->children, widget);
   }
   
   if (widget->smart_object)
      evas_object_raise(widget->smart_object);
}

/**
 * @brief Lowers a widget: it will be below all the other widgets that have the same parent
 * @param widget the widget to lower
 */
void etk_widget_lower(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget || !widget->parent)
      return;
   
   if ((l = evas_list_find_list(widget->parent->children, widget)))
   {
      widget->parent->children = evas_list_remove_list(widget->parent->children, l);
      widget->parent->children = evas_list_prepend(widget->parent->children, widget);
   }
   
   if (widget->smart_object)
      evas_object_lower(widget->smart_object);
}

/**
 * @brief Queues a size recalculation request: during the next mainloop iteration, the widget size will be recalculated @n
 * It's mainly used in widget implementations
 * @param widget the widget to queue
 */
void etk_widget_size_recalc_queue(Etk_Widget *widget)
{
   Etk_Widget *w;

   if (!widget)
      return;

   for (w = widget; w; w = ETK_WIDGET(w->parent))
   {
      if (w->swallowed && w->parent)
         w->parent->need_theme_min_size_recalc = ETK_TRUE;
      w->need_size_recalc = ETK_TRUE;
      w->need_redraw = ETK_TRUE;
   }

   if (widget->toplevel_parent)
      etk_main_iteration_queue();
}

/**
 * @brief Queues a redraw request: during the next mainloop iteration, the widget will be redrawn @n
 * It's mainly used in widget implementations
 * @param widget the widget to queue
 */
void etk_widget_redraw_queue(Etk_Widget *widget)
{
   if (!widget)
      return;

   _etk_widget_redraw_queue_recursive(widget);
   if (widget->toplevel_parent)
      etk_main_iteration_queue();
}

/**
 * @brief Set the ideal size of the widget. The size set will be used for the next size requests instead of calculating it
 * @param widget a widget
 * @param w the ideal width (-1 will make the widget calculate it)
 * @param h the ideal height (-1 will make the widget calculate it)
 */
void etk_widget_size_request_set(Etk_Widget *widget, int w, int h)
{
   if (!widget)
      return;

   widget->requested_size.w = w;
   widget->requested_size.h = h;

   etk_object_notify(ETK_OBJECT(widget), "width_request");
   etk_object_notify(ETK_OBJECT(widget), "height_request");
   etk_widget_size_recalc_queue(widget);
}

/**
 * @brief Calculates the ideal size of the widget. Used mainly for widget implementation
 * @param widget a widget
 * @param size_requisition the location where to set the result
 */
void etk_widget_size_request(Etk_Widget *widget, Etk_Size *size_requisition)
{
   etk_widget_size_request_full(widget, size_requisition, ETK_TRUE);
}

/**
 * @brief Calculates the ideal size of the widget. Used mainly for widget implementation
 * @param widget a widget
 * @param size_requisition the location where to set the result
 * @param hidden_has_no_size if @a hidden_has_no_size is ETK_TRUE, then if the widget is hidden, the size requisition will be 0x0
 */
void etk_widget_size_request_full(Etk_Widget *widget, Etk_Size *size_requisition, Etk_Bool hidden_has_no_size)
{
   if (!widget || !size_requisition)
      return;

   size_requisition->w = -1;
   size_requisition->h = -1;

   if (!widget->visible && hidden_has_no_size)
      size_requisition->w = 0;
   else if (!widget->need_size_recalc && widget->last_size_requisition.w >= 0)
      size_requisition->w = widget->last_size_requisition.w;
   else if (widget->requested_size.w >= 0)
      size_requisition->w = widget->requested_size.w;
   
   if (!widget->visible && hidden_has_no_size)
      size_requisition->h = 0;
   else if (!widget->need_size_recalc && widget->last_size_requisition.h >= 0)
      size_requisition->h = widget->last_size_requisition.h;
   else if (widget->requested_size.h >= 0)
      size_requisition->h = widget->requested_size.h;

   if (size_requisition->w < 0 || size_requisition->h < 0)
   {
      int min_w, min_h;

      etk_widget_theme_object_min_size_calc(widget, &min_w, &min_h);
      if (widget->size_request && widget->realized)
      {
         Etk_Size widget_requisition;

         widget->size_request(widget, &widget_requisition);
         if (size_requisition->w < 0)
            size_requisition->w = ETK_MAX(min_w, widget_requisition.w + widget->left_inset + widget->right_inset);
         if (size_requisition->h < 0)
            size_requisition->h = ETK_MAX(min_h, widget_requisition.h + widget->top_inset + widget->bottom_inset);
      }
      else
      {
         if (size_requisition->w < 0)
            size_requisition->w = min_w;
         if (size_requisition->h < 0)
            size_requisition->h = min_h;
      }
   }

   if (widget->visible)
      widget->last_size_requisition = *size_requisition;
   widget->need_size_recalc = ETK_FALSE;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_SIZE_REQUEST_SIGNAL], ETK_OBJECT(widget), NULL, size_requisition);
}

/**
 * @brief Moves and resizes the theme object and sets the size of the widget according to @a geometry. Used mainly for widget implementation
 * @param widget a widget
 * @param geometry the size the widget should have
 */
void etk_widget_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   if (!widget || widget->swallowed || !widget->smart_object)
      return;

   if (geometry.x != widget->geometry.x || geometry.y != widget->geometry.y)
      evas_object_move(widget->smart_object, geometry.x, geometry.y);
   if (geometry.w != widget->geometry.w || geometry.h != widget->geometry.h || widget->need_redraw)
      evas_object_resize(widget->smart_object, geometry.w, geometry.h);
}

/**
 * @brief Emits the "enter" signal on the widget
 * @param widget a widget
 */
void etk_widget_enter(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_ENTER_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Emits the "leave" signal on the widget
 * @param widget a widget
 */
void etk_widget_leave(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_LEAVE_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Focuses the widget
 * @param widget a widget
 */
void etk_widget_focus(Etk_Widget *widget)
{
   Etk_Widget *focused;

   if (!widget || !widget->toplevel_parent || !widget->focusable)
      return;
   if ((focused = etk_toplevel_widget_focused_widget_get(widget->toplevel_parent)) && (widget == focused))
      return;

   if (focused)
      etk_widget_unfocus(focused);

   etk_toplevel_widget_focused_widget_set(widget->toplevel_parent, widget);
   if (widget->smart_object)
      evas_object_focus_set(widget->smart_object, 1);
   
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_FOCUS_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Unfocuses the widget
 * @param widget a widget
 */
void etk_widget_unfocus(Etk_Widget *widget)
{
   Etk_Widget *focused;

   if (!widget || !widget->toplevel_parent || !(focused = etk_toplevel_widget_focused_widget_get(widget->toplevel_parent)) || (focused != widget))
      return;

   etk_toplevel_widget_focused_widget_set(widget->toplevel_parent, NULL);
   if (widget->smart_object)
      evas_object_focus_set(widget->smart_object, 0);
   
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_UNFOCUS_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Makes the widget swallow another widget in a part of its theme object. Used mainly for widget implementation
 * @param swallowing_widget the widget that will swallow @a widget_to_swallow
 * @param part the name of the part of the theme object that will swallow @a widget_to_swallow
 * @param widget_to_swallow the widget to swallow
 * @return Returns ETK_TRUE on success, ETK_FALSE on failure (generally because the part doesn't exists, or because @a swallowing_widget isn't realized)
 * @note The swallowed widget has to be a child of the swallowing widget. Otherwise, it won't be swallowed and it will return ETK_FALSE
 */
Etk_Bool etk_widget_swallow_widget(Etk_Widget *swallowing_widget, const char *part, Etk_Widget *widget_to_swallow)
{
   Evas *evas;

   if (!swallowing_widget || !part || !widget_to_swallow || !swallowing_widget->theme_object || !widget_to_swallow->smart_object)
      return ETK_FALSE;
   if (!(evas = etk_widget_toplevel_evas_get(swallowing_widget)) || (evas != etk_widget_toplevel_evas_get(widget_to_swallow)))
      return ETK_FALSE;
   if (!edje_object_part_exists(swallowing_widget->theme_object, part))
      return ETK_FALSE;
   if (widget_to_swallow->parent != swallowing_widget)
   {
      ETK_WARNING("Unable to \"swallow\" the widget %p by the widget %p because the widget "
         "to swallow was not a child of the \"swallowing\" widget\n", widget_to_swallow, swallowing_widget);
      return ETK_FALSE;
   }

   if (widget_to_swallow->swallowed)
      etk_widget_unswallow_widget(widget_to_swallow->parent, widget_to_swallow);

   widget_to_swallow->swallowed = _etk_widget_theme_object_swallow_full(swallowing_widget, part, widget_to_swallow->smart_object, widget_to_swallow);
   return widget_to_swallow->swallowed;
}

/**
 * @brief Makes the widget unswallow another widget
 * @param swallowing_widget the widget that will unswallow @a widget
 * @param widget the widget to unswallow
 */
void etk_widget_unswallow_widget(Etk_Widget *swallowing_widget, Etk_Widget *widget)
{
   if (!swallowing_widget || !widget || !widget->smart_object)
      return;
   etk_widget_theme_object_unswallow(swallowing_widget, widget->smart_object);
}

/**
 * @brief Checks if @a widget is swallowing @a swallowed_widget
 * @param widget a widget
 * @param swallowed_widget the widget to checkif it is swallowed by @a widget
 * @return Returns ETK_TRUE if @a widget is swallowing @a swallowed_widget
 */
Etk_Bool etk_widget_is_swallowing_widget(Etk_Widget *widget, Etk_Widget *swallowed_widget)
{
   if (!widget || !swallowed_widget || !swallowed_widget->smart_object)
      return ETK_FALSE;
   return etk_widget_is_swallowing_object(widget, swallowed_widget->smart_object);
}

/**
 * @brief Checks if the widget is swallowed (by its parent)
 * @return Returns ETK_TRUE if the widget is swallowed (by its parent)
 */
Etk_Bool etk_widget_is_swallowed(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return widget->swallowed;
}

/**
 * @brief Makes the theme object of @a swallowing_widget swallow @a object in a part called @a part
 * @param swallowing_widget the widget that will swallow the object
 * @param part the name of the part
 * @param object the object to swallow
 * @return Returns ETK_TRUE on success, ETK_FALSE on failure (generally because the part doesn't exists, or because @a swallowing_widget isn't realized)
 */
Etk_Bool etk_widget_theme_object_swallow(Etk_Widget *swallowing_widget, const char *part, Evas_Object *object)
{
   return _etk_widget_theme_object_swallow_full(swallowing_widget, part, object, NULL);
}

/**
 * @brief Makes the theme object of @a swallowing_widget unswallow @a object
 * @param swallowing_widget the widget that will unswallow the object
 * @param object the object to unswallow
 */
void etk_widget_theme_object_unswallow(Etk_Widget *swallowing_widget, Evas_Object *object)
{
   Evas_List *l, *l2;
   Etk_Widget_Swallowed_Object *swallowed_object = NULL;
   Etk_Widget_Member_Object *member_object;

   if (!swallowing_widget || !object)
      return;

   if (swallowing_widget->theme_object)
      edje_object_part_unswallow(swallowing_widget->theme_object, object);

   for (l = swallowing_widget->swallowed_objects; l; l = l->next)
   {
      swallowed_object = l->data;
      if (swallowed_object->object == object)
         break;
      swallowed_object = NULL;
   }
   if (!swallowed_object)
      return;

   if (swallowed_object->widget)
      swallowed_object->widget->swallowed = ETK_FALSE;
   if ((l2 = _etk_widget_member_object_find(swallowing_widget, object)))
   {
      member_object = l2->data;
      member_object->swallowed = ETK_FALSE;
      _etk_widget_object_add_to_smart(swallowing_widget, object);
   }

   free(swallowed_object->swallowing_part);
   free(swallowed_object);
   swallowing_widget->swallowed_objects = evas_list_remove_list(swallowing_widget->swallowed_objects, l);
   
   etk_widget_size_recalc_queue(swallowing_widget);
}

/**
 * @brief Checks if the widget swallows the object
 * @param widget a widget
 * @param object an evas object
 * @return Returns ETK_TRUE if @a widget is swallowing @a object
 */
Etk_Bool etk_widget_is_swallowing_object(Etk_Widget *widget, Evas_Object *object)
{
   Evas_List *l;
   Etk_Widget_Swallowed_Object *swallowed_object;

   if (!widget || !object)
      return ETK_FALSE;

   for (l = widget->swallowed_objects; l; l = l->next)
   {
      swallowed_object = l->data;
      if (swallowed_object->object == object)
         return ETK_TRUE;
   }
   return ETK_FALSE;
}

/**
 * @brief Calculates the minimum size of the theme object of the widget. Used mainly for widget implementations
 * @param widget a widget
 * @param w the location where to set the calculated width
 * @param h the location where to set the calculated height
 */
void etk_widget_theme_object_min_size_calc(Etk_Widget *widget, int *w, int *h)
{
   if (!widget)
      return;

   if (widget->need_theme_min_size_recalc)
   {
      if (widget->theme_object)
      {
         int min_calc_width, min_calc_height;
         int min_get_width, min_get_height;
         Evas_List *l;
         Etk_Widget_Swallowed_Object *swallowed_object;

         for (l = widget->swallowed_objects; l; l = l->next)
         {
            swallowed_object = l->data;
            if (swallowed_object->widget)
            {
               Etk_Size swallow_size;

               etk_widget_size_request(swallowed_object->widget, &swallow_size);
               edje_extern_object_min_size_set(swallowed_object->object, swallow_size.w, swallow_size.h);
               edje_object_part_swallow(widget->theme_object, swallowed_object->swallowing_part, swallowed_object->object);
            }
         }
         edje_object_size_min_calc(widget->theme_object, &min_calc_width, &min_calc_height);
         edje_object_size_min_get(widget->theme_object, &min_get_width, &min_get_height);
         widget->theme_min_width = ETK_MAX(min_calc_width, min_get_width);
         widget->theme_min_height = ETK_MAX(min_calc_height, min_get_height);

         for (l = widget->swallowed_objects; l; l = l->next)
         {
            swallowed_object = l->data;
            if (swallowed_object->widget)
            {
               edje_extern_object_min_size_set(swallowed_object->object, 0, 0);
               edje_object_part_swallow(widget->theme_object, swallowed_object->swallowing_part, swallowed_object->object);
            }
         }
      }
      else
      {
         widget->theme_min_width = 0;
         widget->theme_min_height = 0;
      }
      
      widget->theme_min_width = ETK_MAX(widget->theme_min_width, widget->left_inset + widget->right_inset);
      widget->theme_min_height = ETK_MAX(widget->theme_min_height, widget->top_inset + widget->bottom_inset);
      widget->need_theme_min_size_recalc = ETK_FALSE;
   }

   if (w)
      *w = widget->theme_min_width;
   if (h)
      *h = widget->theme_min_height;
}

/**
 * @brief Sends a signal to the theme object of the widget
 * @param widget a widget
 * @param signal_name the name of the signal to send
 */
void etk_widget_theme_object_signal_emit(Etk_Widget *widget, const char *signal_name)
{
   if (!widget || !widget->theme_object)
      return;
   edje_object_signal_emit(widget->theme_object, signal_name, "");
   widget->need_theme_min_size_recalc = ETK_TRUE;
}

/**
 * @brief Sets a the text of a text part of the theme object of the widget
 * @param widget a widget
 * @param part_name the name of the text part
 * @param text the text to set
 */
void etk_widget_theme_object_part_text_set(Etk_Widget *widget, const char *part_name, const char *text)
{
   if (!widget || !widget->theme_object)
      return;
   edje_object_part_text_set(widget->theme_object, part_name, text);
   widget->need_theme_min_size_recalc = ETK_TRUE;
}

/**
 * @brief Gets the value of data from the theme of the widget. The widget has to be realized
 * @param widget a widget
 * @param data_name the name of the data you want to get the value
 * @param format the format of the data. Same format than the format argument of sscanf
 * @param ... the location of the variables where to set the values
 * @return Returns the number of the input items successfully matched and assigned, same as sscanf
 */
int etk_widget_theme_object_data_get(Etk_Widget *widget, const char *data_name, const char *format, ...)
{
   const char *data_string;
   int result;
   va_list args;
   
   if (!widget || !data_name || !format || !widget->theme_object)
      return 0;
   if (!(data_string = edje_object_data_get(widget->theme_object, data_name)))
      return 0;
   
   /* TODO: remove that vsscanf compiler warning */
   va_start(args, format);
   result = vsscanf(data_string, format, args);
   va_end(args);
   
   return result;
}

/**
 * @brief Adds an evas object to the list of objet members of the widget. Used for widget implementations mainly @n
 * The object will be automatically deleted when the object will be unrealized. Used for widget implementations mainly
 * @param widget a widget
 * @param object the evas object to add
 * @return Returns ETK_TRUE on success. ETK_FALSE on failure, probably because the widget and the object do not belong to the same evas
 * @note The object has to belong to the same evas than the widget
 */
Etk_Bool etk_widget_member_object_add(Etk_Widget *widget, Evas_Object *object)
{
   Etk_Widget_Member_Object *member_object;
   
   if (!widget || !object || !widget->smart_object || (evas_object_evas_get(object) != etk_widget_toplevel_evas_get(widget)))
      return ETK_FALSE;
   if (_etk_widget_member_object_find(widget, object))
      return ETK_TRUE;
   
   member_object = malloc(sizeof(Etk_Widget_Member_Object));
   member_object->object = object;
   member_object->visible = evas_object_visible_get(object);
   member_object->swallowed = ETK_FALSE;
   
   _etk_widget_object_add_to_smart(widget, object);
   evas_object_intercept_show_callback_add(object, _etk_widget_member_object_intercept_show_cb, widget);
   evas_object_intercept_hide_callback_add(object, _etk_widget_member_object_intercept_hide_cb, widget);
   
   widget->member_objects = evas_list_append(widget->member_objects, member_object);

   return ETK_TRUE;
}

/**
 * @brief Removes an evas object from the list of objet members of the widget. Used for widget implementations mainly
 * @param widget a widget
 * @param object the evas object to remove
 */
void etk_widget_member_object_del(Etk_Widget *widget, Evas_Object *object)
{
   Evas_List *l;
   Etk_Widget_Member_Object *member_object;

   if (!widget || !object)
      return;

   if ((l = _etk_widget_member_object_find(widget, object)))
   {
      member_object = l->data;
      
      if (member_object->swallowed)
         etk_widget_theme_object_unswallow(widget, object);
      if (widget->clip)
         evas_object_clip_unset(member_object->object);
      evas_object_intercept_show_callback_del(member_object->object, _etk_widget_member_object_intercept_show_cb);
      evas_object_intercept_hide_callback_del(member_object->object, _etk_widget_member_object_intercept_hide_cb);
         
      free(member_object);
      widget->member_objects = evas_list_remove_list(widget->member_objects, l);
      evas_object_smart_member_del(object);
   }
}

/**
 * @brief Stacks @a object above all the other member objects of the widget
 * @param widget a widget
 * @param object the object to raise
 */
void etk_widget_member_object_raise(Etk_Widget *widget, Evas_Object *object)
{
   if (!widget || !object)
      return;

   if (_etk_widget_member_object_find(widget, object))
      evas_object_raise(object);
}

/**
 * @brief Stacks @a object below all the other member objects of the widget
 * @param widget a widget
 * @param object the object to lower
 */
void etk_widget_member_object_lower(Etk_Widget *widget, Evas_Object *object)
{
   if (!widget || !object)
      return;

   if (_etk_widget_member_object_find(widget, object))
   {
      if (widget->theme_object && widget->theme_object != object)
         evas_object_stack_above(object, widget->theme_object);
      else
         evas_object_lower(object);
   }
}

/**
 * @brief Stacks the member object @a object above the member object @a above
 * @param widget a widget
 * @param object the object to restack
 * @param above the object above which @a object will be stacked 
 */
void etk_widget_member_object_stack_above(Etk_Widget *widget, Evas_Object *object, Evas_Object *above)
{
   if (!widget || !above || (object == above))
      return;

   if (_etk_widget_member_object_find(widget, object) && _etk_widget_member_object_find(widget, above))
      evas_object_stack_above(object, above);
}

/**
 * @brief Stacks the member object @a object below the member object @a below
 * @param widget a widget
 * @param object the object to restack
 * @param below the object below which @a object will be stacked 
 */
void etk_widget_member_object_stack_below(Etk_Widget *widget, Evas_Object *object, Evas_Object *below)
{
   if (!widget || !below || (object == below))
      return;
   
   if (_etk_widget_member_object_find(widget, object) && _etk_widget_member_object_find(widget, below))
      evas_object_stack_below(object, below);
}

/**
 * @brief Sets the clip object of the widget. Used mainly for widget implementations
 * @param widget a widget
 * @param clip the clip object to set
 * @warning when the clip is destoyed, you have to call etk_widget_clip_unset() on the widget
 */
void etk_widget_clip_set(Etk_Widget *widget, Evas_Object *clip)
{
   if (!widget)
      return;
   
   if (widget->clip)
      etk_widget_clip_unset(widget);
   
   if (widget->smart_object)
   {
      evas_object_show(clip);
      evas_object_clip_set(widget->smart_object, clip);
   }
   else
   {
      widget->clip = clip;
      evas_object_hide(widget->clip);
   }
}

/**
 * @brief Unsets the clip object of the widget. Used mainly for widget implementations
 * @param widget a widget
 */
void etk_widget_clip_unset(Etk_Widget *widget)
{
   if (!widget || !widget->clip)
      return;

   if (widget->smart_object)
      evas_object_clip_unset(widget->smart_object);
   widget->clip = NULL;
}

/**
 * @brief Gets the clip object of the widget
 * @param widget a widget
 * @return Returns the clip object of the widget
 */
Evas_Object *etk_widget_clip_get(Etk_Widget *widget)
{
   if (!widget)
      return NULL;
   return widget->clip;
}

/**
 * @brief Sets whether the widget is dnd destination
 * @param widget a widget
 * @param on ETK_TRUE to enable this widget as a dnd destination, ETK_FALSE to disable it
 */
void etk_widget_dnd_dest_set(Etk_Widget *widget, Etk_Bool on)
{
   if (!widget)
      return;
   
   if (on)
   {
      widget->accepts_dnd = ETK_TRUE;
      widget->dnd_dest = ETK_TRUE;
      _etk_widget_dnd_dest_widgets = evas_list_append(_etk_widget_dnd_dest_widgets, widget);
   }
   else
   {
      widget->accepts_dnd = ETK_FALSE;
      widget->dnd_dest = ETK_FALSE;
      _etk_widget_dnd_dest_widgets = evas_list_remove(_etk_widget_dnd_dest_widgets, widget);
   }
}

/**
 * @brief Checks whether the widget is a dnd destination
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget is a dnd destination, ETK_FALSE otherwise
 */
Etk_Bool etk_widget_dnd_dest_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return (widget->accepts_dnd && widget->dnd_dest);
}

/**
 * @brief Sets whether the widget is dnd source
 * @param widget a widget
 * @param on ETK_TRUE to enable this widget as a dnd source, ETK_FALSE to disable it
 */
void etk_widget_dnd_source_set(Etk_Widget *widget, Etk_Bool on)
{
   if (!widget)
      return;
   
   if (on)
   {      
      
      widget->accepts_dnd = ETK_TRUE;
      widget->dnd_source = ETK_TRUE;      
      _etk_widget_dnd_source_widgets = evas_list_append(_etk_widget_dnd_source_widgets, widget);
      
      if(!widget->dnd_internal)
      {
	 widget->drag = etk_drag_new(widget);
	 etk_signal_connect("mouse_move", ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_dnd_drag_mouse_move_cb), NULL);
	 etk_signal_connect("drag_end", ETK_OBJECT(widget->drag), ETK_CALLBACK(_etk_widget_dnd_drag_end_cb), NULL);
      }
   }
   else
   {
      widget->accepts_dnd = ETK_FALSE;
      widget->dnd_source = ETK_FALSE;
      _etk_widget_dnd_source_widgets = evas_list_remove(_etk_widget_dnd_source_widgets, widget);
      
      if(!widget->dnd_internal)
      {
	 etk_signal_disconnect("mouse_move", ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_dnd_drag_mouse_move_cb));
	 etk_signal_disconnect("drag_end", ETK_OBJECT(widget->drag), ETK_CALLBACK(_etk_widget_dnd_drag_end_cb));
      }
   }
}

/**
 * @brief Checks whether the widget is a dnd source
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget is a dnd source, ETK_FALSE otherwise
 */
Etk_Bool etk_widget_dnd_source_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return (widget->accepts_dnd && widget->dnd_source);
}

/**
 * @brief Set whether the widget wants to handle its own dnd or not
 * @param widget a widget
 * @param on ETK_TRUE if the widget handles its own dnd, ETK_FALSE otherwise
 */
void etk_widget_dnd_internal_set(Etk_Widget *widget, Etk_Bool on)
{
   if (!widget)
      return;
   widget->dnd_internal = on;
}

/**
 * @brief Checks whether the widget wants to handle its own dnd or not
 * @param widget a widget
 * @return Returns ETK_TRUE if the widget handles its own dnd, ETK_FALSE otherwise
 */
Etk_Bool etk_widget_dnd_internal_get(Etk_Widget *widget)
{
   if (!widget)
      return ETK_FALSE;
   return (widget->dnd_internal);
}

/**
 * @brief Sets the visual data for the drag (the widget to be displayed)
 * @param widget a widget
 * @param drag_widget the widget that will appear in the drag window
 */
void etk_widget_dnd_drag_widget_set(Etk_Widget *widget, Etk_Widget *drag_widget)
{
   if(!widget || !drag_widget)
     return;
   
   etk_container_add(ETK_CONTAINER(widget->drag), drag_widget);
}

/**
 * @brief Gets the visual data for the drag (the widget to be displayed)
 * @param widget a widget
 * @return Returns the widget that will appear in the drag window
 */
Etk_Widget *etk_widget_dnd_drag_widget_get(Etk_Widget *widget)
{
   Evas_List *children = NULL;
   
   if(!widget)
     return NULL;
   
   children = etk_container_children_get(ETK_CONTAINER(widget->drag));
   
   if(!children)
     return NULL;
   
   if(evas_list_count(children) < 1)
     return NULL;
   
   if(ETK_IS_WIDGET(children->data))
     return (Etk_Widget*)children->data;
   
   return NULL;
}

/* TODO: doc */
void etk_widget_dnd_drag_data_set(Etk_Widget *widget, const char **types, int num_types, void *data, int data_size)
{
   if(!widget)
     return;
   
   if(!widget->drag || !widget->dnd_source)
     return;
   
   etk_drag_types_set(ETK_DRAG(widget->drag), types, num_types);
   etk_drag_data_set(ETK_DRAG(widget->drag), data, data_size);   
}

/**
 * @brief Sets the possible types for dnd
 * @param widget a widget
 * @param types list of acceptable types
 * @param num number of types
 */
void etk_widget_dnd_types_set(Etk_Widget *widget, const char **types, int num)
{
   int i;
   
   if (!widget)
      return;
   
   /* free old data */
   if(num <= 0 || types == NULL || 
     (widget->dnd_types_num > 0 && widget->dnd_types != NULL))
   {
      
      for(i = 0; i < widget->dnd_types_num; i++)
	if(widget->dnd_types[i]) free(widget->dnd_types[i]);
      
      if(widget->dnd_types) free(widget->dnd_types);
      
      widget->dnd_types = NULL;
      widget->dnd_types_num = 0;
      return;
   }
   
   widget->dnd_types = calloc(num, sizeof(char*));
   for(i = 0; i < num; i++)
     widget->dnd_types[i] = strdup(types[i]);
   
   widget->dnd_types_num = num;
}

/**
 * @brief Gets the possible types for dnd
 * @param widget a widget
 * @param num number of types that the widget has
 * @return returns the dnd types this widget supports
 */
const char **etk_widget_dnd_types_get(Etk_Widget *widget, int *num)
{
   if (!widget)
      return ETK_FALSE;
   
   if(num)
     *num = widget->dnd_types_num;
   return (const char **)(widget->dnd_types);
}

/**
 * @brief Gets the list of the widgets that are dnd destinations
 * @return Returns the list of the dnd destination widgets
 */
Evas_List *etk_widget_dnd_dest_widgets_get()
{
   return _etk_widget_dnd_dest_widgets;
}

/**
 * @brief Sends the "drag_drop" signal
 * @param widget a widget
 */
void etk_widget_drag_drop(Etk_Widget *widget, Etk_Event_Selection_Request *event)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_DROP_SIGNAL], ETK_OBJECT(widget), NULL, event);
   /* FIXME: why isnt this being emitted automatically?!? */
   etk_widget_theme_object_signal_emit(widget, "drag_drop");   
}

/**
 * @brief Sends the "drag_motion" signal
 * @param widget a widget
 */
void etk_widget_drag_motion(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_MOTION_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Sends the "drag_enter" signal
 * @param widget a widget
 */
void etk_widget_drag_enter(Etk_Widget *widget)
{
   if (!widget)
      return;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_ENTER_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Sends the "drag_leave" signal
 * @param widget a widget
 */
void etk_widget_drag_leave(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_LEAVE_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Sends the "drag_begin" signal
 * @param widget a widget
 */
void etk_widget_drag_begin(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_BEGIN_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Sends the "drag_end" signal
 * @param widget a widget
 */
void etk_widget_drag_end(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_DRAG_END_SIGNAL], ETK_OBJECT(widget), NULL);
}

/**
 * @brief Sends the "selection_received" signal
 * @param widget a widget
 */
void etk_widget_selection_received(Etk_Widget *widget, Etk_Event_Selection_Request *event)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_SELECTION_RECEIVED_SIGNAL], ETK_OBJECT(widget), NULL, event);
}

/**
 * @brief Sends the "clipboard_received" signal
 * @param widget a widget
 */
void etk_widget_clipboard_received(Etk_Widget *widget, Etk_Event_Selection_Request *event)
{
   if (!widget)
      return;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_CLIPBOARD_RECEIVED_SIGNAL], ETK_OBJECT(widget), NULL, event);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the default values of the widget */
static void _etk_widget_constructor(Etk_Widget *widget)
{
   if (!widget)
      return;

   widget->name = NULL;
   widget->parent = NULL;
   widget->toplevel_parent = NULL;
   widget->child_properties = NULL;
   widget->children = NULL;
   widget->focus_order = NULL;

   widget->theme_object = NULL;
   widget->theme_min_width = 0;
   widget->theme_min_height = 0;
   widget->theme_file = NULL;
   widget->theme_group = NULL;
   widget->theme_parent = NULL;
   widget->theme_children = NULL;

   widget->smart_object = NULL;
   widget->event_object = NULL;
   widget->clip = NULL;
   widget->swallowed_objects = NULL;
   widget->member_objects = NULL;

   widget->show = _etk_widget_show_handler;
   widget->enter = _etk_widget_enter_handler;
   widget->leave = _etk_widget_leave_handler;
   widget->focus = _etk_widget_focus_handler;
   widget->unfocus = _etk_widget_unfocus_handler;
   widget->drag_drop = _etk_widget_drag_drop_handler;
   widget->drag_motion = _etk_widget_drag_motion_handler;
   widget->drag_enter = _etk_widget_drag_enter_handler;   
   widget->drag_leave = _etk_widget_drag_leave_handler;
   widget->drag_begin = _etk_widget_drag_begin_handler;   
   widget->drag_end = _etk_widget_drag_end_handler;

   widget->left_inset = 0;
   widget->right_inset = 0;
   widget->top_inset = 0;
   widget->bottom_inset = 0;

   memset(&widget->geometry, -1, sizeof(Etk_Geometry));
   memset(&widget->inner_geometry, -1, sizeof(Etk_Geometry));
   widget->size_request = NULL;
   widget->size_allocate = NULL;
   widget->requested_size.w = -1;
   widget->requested_size.h = -1;
   widget->last_size_requisition.w = 0;
   widget->last_size_requisition.h = 0;

   widget->scroll_size_get = NULL;
   widget->scroll_margins_get = NULL;
   widget->scroll = NULL;

   widget->realized = ETK_FALSE;
   widget->swallowed = ETK_FALSE;
   widget->visible = ETK_FALSE;
   widget->visibility_locked = ETK_FALSE;
   widget->focusable = ETK_FALSE;
   widget->focus_on_press = ETK_FALSE;
   widget->can_pass_focus = ETK_TRUE;
   widget->use_focus_order = ETK_FALSE;
   widget->has_event_object = ETK_FALSE;
   widget->repeat_mouse_events = ETK_FALSE;
   widget->pass_mouse_events = ETK_FALSE;
   widget->need_size_recalc = ETK_FALSE;
   widget->need_redraw = ETK_FALSE;
   widget->need_theme_min_size_recalc = ETK_FALSE;
   widget->swallowed = ETK_FALSE;
   widget->accepts_dnd = ETK_FALSE;
   widget->dnd_internal = ETK_FALSE;
   widget->dnd_source = ETK_FALSE;
   widget->dnd_dest = ETK_FALSE;
   widget->dnd_types = NULL;
   widget->dnd_types_num = 0;
   
   etk_signal_connect_full(_etk_widget_signals[ETK_WIDGET_MOUSE_IN_SIGNAL], ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_signal_mouse_in_cb), NULL, ETK_FALSE, ETK_FALSE);
   etk_signal_connect_full(_etk_widget_signals[ETK_WIDGET_MOUSE_OUT_SIGNAL], ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_signal_mouse_out_cb), NULL, ETK_FALSE, ETK_FALSE);
   etk_signal_connect_full(_etk_widget_signals[ETK_WIDGET_MOUSE_DOWN_SIGNAL], ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_signal_mouse_down_cb), NULL, ETK_FALSE, ETK_FALSE);
   etk_signal_connect_full(_etk_widget_signals[ETK_WIDGET_KEY_DOWN_SIGNAL], ETK_OBJECT(widget), ETK_CALLBACK(_etk_widget_signal_key_down_cb), NULL, ETK_FALSE, ETK_FALSE);
   
   if (ETK_IS_TOPLEVEL_WIDGET(widget))
      etk_object_notification_callback_add(ETK_OBJECT(widget), "evas", _etk_widget_toplevel_evas_changed_cb, NULL);
}

/* Destroys the widget */
static void _etk_widget_destructor(Etk_Widget *widget)
{
   if (!widget)
      return;

   _etk_widget_unrealize(widget);
   
   /* Remove children */
   while (widget->children)
      etk_object_destroy(ETK_OBJECT(widget->children->data));
   if (widget->parent)
   {
      if (ETK_IS_CONTAINER(widget->parent))
         etk_container_remove(ETK_CONTAINER(widget->parent), widget);
      etk_widget_parent_set(widget, NULL);
   }
   evas_list_free(widget->focus_order);
   
   while (widget->theme_children)
   {
      ETK_WIDGET(widget->theme_children->data)->theme_parent = NULL;
      widget->theme_children = evas_list_remove_list(widget->theme_children, widget->theme_children);
   }
   if (widget->theme_parent)
      widget->theme_parent->theme_children = evas_list_remove(widget->theme_parent->theme_children, widget);
   
   /* Remove the widget from the dnd lists */
   if (widget->accepts_dnd && widget->dnd_dest)
      _etk_widget_dnd_dest_widgets = evas_list_remove(_etk_widget_dnd_dest_widgets, widget);
   if (widget->accepts_dnd && widget->dnd_source)
     _etk_widget_dnd_source_widgets = evas_list_remove(_etk_widget_dnd_source_widgets, widget);
   
   free(widget->theme_file);
   free(widget->theme_group);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_widget_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Widget *widget;

   if (!(widget = ETK_WIDGET(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_WIDGET_THEME_FILE_PROPERTY:
         etk_widget_theme_file_set(widget, etk_property_value_string_get(value));
         break;
      case ETK_WIDGET_THEME_GROUP_PROPERTY:
         etk_widget_theme_group_set(widget, etk_property_value_string_get(value));
         break;
      case ETK_WIDGET_THEME_PARENT_PROPERTY:
         etk_widget_theme_parent_set(widget, ETK_WIDGET(etk_property_value_pointer_get(value)));
         break;
      case ETK_WIDGET_VISIBLE_PROPERTY:
         if (etk_property_value_bool_get(value))
            etk_widget_show(widget);
         else
            etk_widget_hide(widget);
         break;
      case ETK_WIDGET_VISIBILITY_LOCKED_PROPERTY:
         etk_widget_visibility_locked_set(widget, etk_property_value_bool_get(value));
         break;
      case ETK_WIDGET_WIDTH_REQUEST_PROPERTY:
         etk_widget_size_request_set(widget, etk_property_value_int_get(value), widget->requested_size.h);
         break;
      case ETK_WIDGET_HEIGHT_REQUEST_PROPERTY:
         etk_widget_size_request_set(widget, widget->requested_size.w, etk_property_value_int_get(value));
         break;
      case ETK_WIDGET_NAME_PROPERTY:
         etk_widget_name_set(widget, etk_property_value_string_get(value));
         break;
      case ETK_WIDGET_HAS_EVENT_OBJECT_PROPERTY:
         etk_widget_has_event_object_set(widget, etk_property_value_bool_get(value));
         break;
      case ETK_WIDGET_REPEAT_MOUSE_EVENTS_PROPERTY:
         etk_widget_repeat_mouse_events_set(widget, etk_property_value_bool_get(value));
         break;
      case ETK_WIDGET_PASS_MOUSE_EVENTS_PROPERTY:
         etk_widget_pass_mouse_events_set(widget, etk_property_value_bool_get(value));
         break;
      case ETK_WIDGET_FOCUSABLE_PROPERTY:
         /* TODO: is the widget is the one which is focused, we should change the focused widget */
         widget->focusable = etk_property_value_bool_get(value);
         etk_object_notify(object, "focusable");
      case ETK_WIDGET_FOCUS_ON_PRESS_PROPERTY:
         widget->focus_on_press = etk_property_value_bool_get(value);
         etk_object_notify(object, "focus_on_press");
         break;
      case ETK_WIDGET_CAN_PASS_FOCUS_PROPERTY:
         widget->can_pass_focus = etk_property_value_bool_get(value);
         etk_object_notify(object, "can_pass_focus");
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_widget_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Widget *widget;

   if (!(widget = ETK_WIDGET(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_WIDGET_NAME_PROPERTY:
         etk_property_value_string_set(value, widget->name);
         break;
      case ETK_WIDGET_PARENT_PROPERTY:
         etk_property_value_pointer_set(value, ETK_OBJECT(widget->parent));
         break;
      case ETK_WIDGET_THEME_FILE_PROPERTY:
         etk_property_value_string_set(value, widget->theme_file);
         break;
      case ETK_WIDGET_THEME_GROUP_PROPERTY:
         etk_property_value_string_set(value, widget->theme_group);
         break;
      case ETK_WIDGET_THEME_PARENT_PROPERTY:
         etk_property_value_pointer_set(value, widget->theme_parent);
         break;
      case ETK_WIDGET_WIDTH_REQUEST_PROPERTY:
         etk_property_value_int_set(value, widget->requested_size.w);
         break;
      case ETK_WIDGET_HEIGHT_REQUEST_PROPERTY:
         etk_property_value_int_set(value, widget->requested_size.h);
         break;
      case ETK_WIDGET_VISIBLE_PROPERTY:
         etk_property_value_bool_set(value, widget->visible);
         break;
      case ETK_WIDGET_VISIBILITY_LOCKED_PROPERTY:
         etk_property_value_bool_set(value, widget->visibility_locked);
         break;
      case ETK_WIDGET_HAS_EVENT_OBJECT_PROPERTY:
         etk_property_value_bool_set(value, widget->has_event_object);
         break;
      case ETK_WIDGET_REPEAT_MOUSE_EVENTS_PROPERTY:
         etk_property_value_bool_set(value, widget->repeat_mouse_events);
         break;
      case ETK_WIDGET_PASS_MOUSE_EVENTS_PROPERTY:
         etk_property_value_bool_set(value, widget->pass_mouse_events);
         break;
      case ETK_WIDGET_FOCUSABLE_PROPERTY:
         etk_property_value_bool_set(value, widget->focusable);
         break;
      case ETK_WIDGET_FOCUS_ON_PRESS_PROPERTY:
         etk_property_value_bool_set(value, widget->focus_on_press);
         break;
      case ETK_WIDGET_CAN_PASS_FOCUS_PROPERTY:
         etk_property_value_bool_set(value, widget->can_pass_focus);
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

/* Default handler for the "enter" signal */
static void _etk_widget_enter_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "enter");
}

/* Default handler for the "leave" signal */
static void _etk_widget_leave_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "leave");
}

/* Default handler for the "focus" signal */
static void _etk_widget_focus_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "focus");
}

/* Default handler for the "unfocus" signal */
static void _etk_widget_unfocus_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "unfocus");
}

/* Sets the widget as visible and queues a visibility update */
static void _etk_widget_show_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "show");
}

/* Default handler for the "drag_drop" signal */
static void _etk_widget_drag_drop_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   
   etk_widget_theme_object_signal_emit(widget, "drag_drop");
}

/* Default handler for the "drag_motion" signal */
static void _etk_widget_drag_motion_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "drag_motion");
}

/* Default handler for the "drag_enter" signal */
static void _etk_widget_drag_enter_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "drag_enter");
}

/* Default handler for the "drag_leave" signal */
static void _etk_widget_drag_leave_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "drag_leave");
}

/* Default handler for the "drag_begin" signal */
static void _etk_widget_drag_begin_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "drag_begin");
}

/* Default handler for the "drag_end" signal */
static void _etk_widget_drag_end_handler(Etk_Widget *widget)
{
   if (!widget)
      return;
   etk_widget_theme_object_signal_emit(widget, "drag_end");
}

/* Called when the mouse pointer enters the widget */
static void _etk_widget_mouse_in_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_In *evas_event = event_info;
   Etk_Event_Mouse_In_Out event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   event.buttons = evas_event->buttons;
   event.canvas.x = evas_event->canvas.x;
   event.canvas.y = evas_event->canvas.y;
   event.widget.x = evas_event->canvas.x - widget->inner_geometry.x;
   event.widget.y = evas_event->canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_IN_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (widget->parent)
      _etk_widget_mouse_in_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the signal "mouse_in" is emitted */
static void _etk_widget_signal_mouse_in_cb(Etk_Object *object, Etk_Event_Mouse_In_Out *event, void *data)
{
   if (!object)
      return;
   etk_widget_enter(ETK_WIDGET(object));
}

/* Called when the mouse pointer leaves the widget */
static void _etk_widget_mouse_out_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_Out *evas_event = event_info;
   Etk_Event_Mouse_In_Out event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   event.buttons = evas_event->buttons;
   event.canvas.x = evas_event->canvas.x;
   event.canvas.y = evas_event->canvas.y;
   event.widget.x = evas_event->canvas.x - widget->inner_geometry.x;
   event.widget.y = evas_event->canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_OUT_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (widget->parent)
      _etk_widget_mouse_out_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the signal "mouse_out" is emitted */
static void _etk_widget_signal_mouse_out_cb(Etk_Object *object, Etk_Event_Mouse_In_Out *event, void *data)
{
   if (!object)
      return;
   etk_widget_leave(ETK_WIDGET(object));
}

/* Called when the mouse pointer moves */
static void _etk_widget_mouse_move_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_Move *evas_event = event_info;
   Etk_Event_Mouse_Move event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   event.buttons = evas_event->buttons;
   event.cur.canvas.x = evas_event->cur.canvas.x;
   event.cur.canvas.y = evas_event->cur.canvas.y;
   event.cur.widget.x = evas_event->cur.canvas.x - widget->inner_geometry.x;
   event.cur.widget.y = evas_event->cur.canvas.y - widget->inner_geometry.y;
   event.prev.canvas.x = evas_event->prev.canvas.x;
   event.prev.canvas.y = evas_event->prev.canvas.y;
   event.prev.widget.x = evas_event->prev.canvas.x - widget->inner_geometry.x;
   event.prev.widget.y = evas_event->prev.canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_MOVE_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (widget->parent)
      _etk_widget_mouse_move_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the mouse presses the widget */
static void _etk_widget_mouse_down_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_Down *evas_event = event_info;
   Etk_Event_Mouse_Up_Down event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   event.button = evas_event->button;
   event.canvas.x = evas_event->canvas.x;
   event.canvas.y = evas_event->canvas.y;
   event.widget.x = evas_event->canvas.x - widget->inner_geometry.x;
   event.widget.y = evas_event->canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.flags = evas_event->flags;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_DOWN_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (widget->repeat_mouse_events && widget->parent)
      _etk_widget_mouse_down_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the widget is pressed */
static void _etk_widget_signal_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Up_Down *event, void *data)
{
   Etk_Widget *widget;
 
   if (!(widget = ETK_WIDGET(object)) || !widget->focus_on_press)
      return;
   etk_widget_focus(widget);
}

/* Called when the mouse releases the widget */
static void _etk_widget_mouse_up_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_Up *evas_event = event_info;
   Etk_Event_Mouse_Up_Down event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   event.button = evas_event->button;
   event.canvas.x = evas_event->canvas.x;
   event.canvas.y = evas_event->canvas.y;
   event.widget.x = evas_event->canvas.x - widget->inner_geometry.x;
   event.widget.y = evas_event->canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.flags = evas_event->flags;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_UP_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (evas_event->canvas.x >= widget->geometry.x && evas_event->canvas.x <= widget->geometry.x + widget->geometry.w &&
         evas_event->canvas.y >= widget->geometry.y && evas_event->canvas.y <= widget->geometry.y + widget->geometry.h)
      etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_CLICK_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (widget->repeat_mouse_events && widget->parent)
      _etk_widget_mouse_up_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the mouse wheel is used over the widget */
static void _etk_widget_mouse_wheel_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Mouse_Wheel *evas_event = event_info;
   Etk_Event_Mouse_Wheel event;

   if (!(widget = ETK_WIDGET(data)))
      return;
   
   /* TODO: _etk_widget_propagate_key_event for wheel? */
   if (object)
      _etk_widget_propagate_key_event = ETK_TRUE;
   
   event.direction = evas_event->direction;
   event.z = evas_event->z;
   event.canvas.x = evas_event->canvas.x;
   event.canvas.y = evas_event->canvas.y;
   event.widget.x = evas_event->canvas.x - widget->inner_geometry.x;
   event.widget.y = evas_event->canvas.y - widget->inner_geometry.y;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_MOUSE_WHEEL_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (_etk_widget_propagate_key_event && widget->parent)
      _etk_widget_mouse_wheel_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the user presses a key and if the widget is focused */
static void _etk_widget_key_down_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Key_Down *evas_event = event_info;
   Etk_Event_Key_Up_Down event;

   if (!(widget = ETK_WIDGET(data)))
      return;

   if (object)
      _etk_widget_propagate_key_event = ETK_TRUE;
   
   event.keyname = evas_event->keyname;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.key = evas_event->key;
   event.string = evas_event->string;
   event.compose = evas_event->compose;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_KEY_DOWN_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (_etk_widget_propagate_key_event && widget->parent)
      _etk_widget_key_down_cb(widget->parent, evas, NULL, event_info);
}

/* Called when a key is pressed */
static void _etk_widget_signal_key_down_cb(Etk_Object *object, Etk_Event_Key_Up_Down *event, void *data)
{
   Etk_Widget *widget;
   Etk_Toplevel_Widget *toplevel;

   if (!(widget = ETK_WIDGET(object)) || !event || !(toplevel = (widget->toplevel_parent)) || !event->key)
      return;

   if (object)
      _etk_widget_propagate_key_event = ETK_TRUE;

   if (strcmp(event->key, "Tab") == 0)
   {
      if (widget->can_pass_focus)
         etk_widget_focus(etk_toplevel_widget_focused_widget_next_get(toplevel));
      etk_widget_key_event_propagation_stop();
   }
   else if (strcmp(event->key, "ISO_Left_Tab") == 0)
   {
      if (widget->can_pass_focus)
         etk_widget_focus(etk_toplevel_widget_focused_widget_prev_get(toplevel));
      etk_widget_key_event_propagation_stop();
   }
}

/* Called when the user releases a key and if the widget is focused */
static void _etk_widget_key_up_cb(void *data, Evas *evas, Evas_Object *object, void *event_info)
{
   Etk_Widget *widget;
   Evas_Event_Key_Up *evas_event = event_info;
   Etk_Event_Key_Up_Down event;

   if (!(widget = ETK_WIDGET(data)))
      return;

   if (object)
      _etk_widget_propagate_key_event = ETK_TRUE;
   
   event.keyname = evas_event->keyname;
   event.modifiers = evas_event->modifiers;
   event.locks = evas_event->locks;
   event.key = evas_event->key;
   event.string = evas_event->string;
   event.compose = evas_event->compose;
   event.timestamp = evas_event->timestamp;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_KEY_UP_SIGNAL], ETK_OBJECT(widget), NULL, &event);
   
   if (_etk_widget_propagate_key_event && widget->parent)
      _etk_widget_key_up_cb(widget->parent, evas, NULL, event_info);
}

/* Called when the widget is a toplevel widget and when its evas is changed */
static void _etk_widget_toplevel_evas_changed_cb(Etk_Object *object, const char *property_name, void *data)
{
   Etk_Widget *widget;
   Evas *evas;
   
   if (!(widget = ETK_WIDGET(object)))
      return;
   
   evas = etk_toplevel_widget_evas_get(ETK_TOPLEVEL_WIDGET(widget));
   if (evas && (!widget->smart_object || evas_object_evas_get(widget->smart_object) != evas))
      _etk_widget_realize_all(widget);
   else if (!evas && widget->smart_object)
      _etk_widget_unrealize_all(widget);
}

/**************************
 *
 * Private functions
 *
 **************************/

/* Realizes the widget: it will load the theme and allocate the graphical ressources */
static void _etk_widget_realize(Etk_Widget *widget)
{
   Evas *evas = NULL;
   Evas_List *l;
   Etk_Widget *child;
   Etk_Widget *theme_parent;
   
   if (!widget || !(evas = etk_widget_toplevel_evas_get(widget)))
      return;
   
   if (widget->realized)
      _etk_widget_unrealize(widget);

   /* First, we create the smart object */
   widget->smart_object = _etk_widget_smart_object_add(evas, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_IN, _etk_widget_mouse_in_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_OUT, _etk_widget_mouse_out_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_MOVE, _etk_widget_mouse_move_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_DOWN, _etk_widget_mouse_down_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_UP, _etk_widget_mouse_up_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_MOUSE_WHEEL, _etk_widget_mouse_wheel_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_KEY_DOWN, _etk_widget_key_down_cb, widget);
   evas_object_event_callback_add(widget->smart_object, EVAS_CALLBACK_KEY_UP, _etk_widget_key_up_cb, widget);
   
   if (widget->parent && widget->parent->smart_object)
      _etk_widget_object_add_to_smart(widget->parent, widget->smart_object);
   for (l = widget->children; l; l = l->next)
   {
      child = ETK_WIDGET(l->data);
      if (child->smart_object)
         _etk_widget_object_add_to_smart(widget, child->smart_object);
   }
   
   if (ETK_IS_TOPLEVEL_WIDGET(widget) || (widget->parent && widget->parent->smart_object &&
      evas_object_visible_get(widget->parent->smart_object) && widget->visible))
   {
      evas_object_show(widget->smart_object);
   }
   else
      evas_object_hide(widget->smart_object);
   
   if (widget->clip)
   {
      evas_object_show(widget->clip);
      evas_object_clip_set(widget->smart_object, widget->clip);
   }

   evas_object_propagate_events_set(widget->smart_object, 0);
   evas_object_repeat_events_set(widget->smart_object, widget->repeat_mouse_events);
   evas_object_pass_events_set(widget->smart_object, widget->pass_mouse_events);
   
   if (widget->toplevel_parent && (widget == etk_toplevel_widget_focused_widget_get(widget->toplevel_parent)))
      evas_object_focus_set(widget->smart_object, 1);
   
   /* Then, we create the theme object */
   theme_parent = widget->theme_parent ? widget->theme_parent : widget->parent;
   if ((widget->theme_object = etk_theme_object_load_from_parent(evas, theme_parent, widget->theme_file, widget->theme_group)))
   {
      if (etk_widget_theme_object_data_get(widget, "inset", "%d %d %d %d",
         &widget->left_inset, &widget->right_inset, &widget->top_inset, &widget->bottom_inset) != 4)
      {
         widget->left_inset = 0;
         widget->right_inset = 0;
         widget->top_inset = 0;
         widget->bottom_inset = 0;
      }
      etk_widget_member_object_add(widget, widget->theme_object);
      evas_object_show(widget->theme_object);
      etk_widget_member_object_lower(widget, widget->theme_object);
   }
   /* And we create the event object if necessary */
   else if (widget->has_event_object)
   {
      widget->event_object = evas_object_rectangle_add(evas);
      evas_object_color_set(widget->event_object, 255, 255, 255, 0);
      evas_object_repeat_events_set(widget->event_object, 1);
      evas_object_show(widget->event_object);
      etk_widget_member_object_add(widget, widget->event_object);
   }

   widget->need_theme_min_size_recalc = ETK_TRUE;
   widget->realized = ETK_TRUE;

   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_REALIZE_SIGNAL], ETK_OBJECT(widget), NULL);
   etk_widget_size_recalc_queue(widget);
}

/* Unrealizes the widget: it will unload the theme and free the graphical ressources */
static void _etk_widget_unrealize(Etk_Widget *widget)
{
   Etk_Widget_Member_Object *m;
   Etk_Widget_Swallowed_Object *swallowed_object;

   if (!widget || !widget->realized)
      return;

   while (widget->swallowed_objects)
   {
      swallowed_object = widget->swallowed_objects->data;
      etk_widget_theme_object_unswallow(widget, swallowed_object->object);
   }
   
   widget->event_object = NULL;
   widget->theme_object = NULL;
   while (widget->member_objects)
   {
      m = widget->member_objects->data;
      evas_object_del(m->object);
      free(m);
      widget->member_objects = evas_list_remove_list(widget->member_objects, widget->member_objects);
   }
   
   evas_object_del(widget->smart_object);
   widget->smart_object = NULL;
   
   if (widget->clip)
      evas_object_hide(widget->clip);
   
   widget->left_inset = 0;
   widget->right_inset = 0;
   widget->top_inset = 0;
   widget->bottom_inset = 0;
   
   widget->realized = ETK_FALSE;
   etk_signal_emit(_etk_widget_signals[ETK_WIDGET_UNREALIZE_SIGNAL], ETK_OBJECT(widget), NULL);
}

/* Used by etk_widget_parent_set */
static void _etk_widget_toplevel_parent_set(Etk_Widget *widget, Etk_Toplevel_Widget *toplevel_parent)
{
   Evas_List *l;
   
   if (!widget)
      return;

   widget->toplevel_parent = toplevel_parent;
   for (l = widget->children; l; l = l->next)
      _etk_widget_toplevel_parent_set(ETK_WIDGET(l->data), toplevel_parent);
}

/* Realizes the widget and all its children */
static void _etk_widget_realize_all(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;

   _etk_widget_realize(widget);
   for (l = widget->children; l; l = l->next)
      _etk_widget_realize_all(ETK_WIDGET(l->data));
}

/* Unrealizes the widget and all its children */
static void _etk_widget_unrealize_all(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;

   _etk_widget_unrealize(widget);
   for (l = widget->children; l; l = l->next)
      _etk_widget_unrealize_all(ETK_WIDGET(l->data));
}

/* Realizes the children and theme children of the widget when the theme file is changed */
static void _etk_widget_realize_on_theme_file_change(Etk_Widget *widget, Etk_Bool force_realize)
{
   Evas_List *l;
   
   if (!widget)
      return;
   
   if (force_realize || !widget->theme_file)
   {
      _etk_widget_realize(widget);
      
      for (l = widget->children; l; l = l->next)
         _etk_widget_realize_on_theme_file_change(ETK_WIDGET(l->data), ETK_FALSE);
      for (l = widget->theme_children; l; l = l->next)
         _etk_widget_realize_on_theme_file_change(ETK_WIDGET(l->data), ETK_FALSE);
   }
}

/* Realizes the children and the theme children of the widget when the theme group or theme parent is changed */
static void _etk_widget_realize_on_theme_change(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;
   
   _etk_widget_realize(widget);
   
   for (l = widget->children; l; l = l->next)
      _etk_widget_realize_on_theme_change(ETK_WIDGET(l->data));
   for (l = widget->theme_children; l; l = l->next)
      _etk_widget_realize_on_theme_change(ETK_WIDGET(l->data));
}

/* Used by etk_widget_redraw_queue() */
static void _etk_widget_redraw_queue_recursive(Etk_Widget *widget)
{
   Evas_List *l;
   
   if (!widget)
      return;

   widget->need_redraw = ETK_TRUE;
   for (l = widget->children; l; l = l->next)
      _etk_widget_redraw_queue_recursive(ETK_WIDGET(l->data));
}

/* Makes the theme object of the widget swallow an object into one of its part */
/* TODO: object free_cb?? */
static Etk_Bool _etk_widget_theme_object_swallow_full(Etk_Widget *swallowing_widget, const char *part, Evas_Object *object, Etk_Widget *widget)
{
   Evas *evas;
   Evas_List *l;
   Evas_Object *previously_swallowed;
   Etk_Widget_Swallowed_Object *swallowed_object;
   Etk_Widget_Member_Object *member_object;

   if (!swallowing_widget || !part || !object || !swallowing_widget->theme_object)
      return ETK_FALSE;
   if (!(evas = etk_widget_toplevel_evas_get(swallowing_widget)) || (evas != evas_object_evas_get(object)))
      return ETK_FALSE;
   if (!edje_object_part_exists(swallowing_widget->theme_object, part))
      return ETK_FALSE;
   
   if ((previously_swallowed = edje_object_part_swallow_get(swallowing_widget->theme_object, part)))
      etk_widget_theme_object_unswallow(swallowing_widget, previously_swallowed);
   
   swallowed_object = malloc(sizeof(Etk_Widget_Swallowed_Object));
   swallowed_object->object = object;
   swallowed_object->swallowing_part = strdup(part);
   swallowed_object->widget = widget;
   swallowing_widget->swallowed_objects = evas_list_append(swallowing_widget->swallowed_objects, swallowed_object);
   edje_object_part_swallow(swallowing_widget->theme_object, part, object);
   etk_widget_size_recalc_queue(swallowing_widget);
   
   if (!(l = _etk_widget_member_object_find(swallowing_widget, object)))
      etk_widget_member_object_add(swallowing_widget, object);
   if (l || (l = _etk_widget_member_object_find(swallowing_widget, object)))
   {
      member_object = l->data;
      member_object->swallowed = ETK_TRUE;
   }

   return ETK_TRUE;
}

/* Adds a child to the widget */
static void _etk_widget_child_add(Etk_Widget *parent, Etk_Widget *child)
{
   if (!parent || !child)
      return;
   
   if (parent->clip)
      etk_widget_clip_set(child, parent->clip);
   
   parent->children = evas_list_append(parent->children, child);
}

/* Adds a child from the widget */
static void _etk_widget_child_remove(Etk_Widget *parent, Etk_Widget *child)
{
   Evas_List *l;
   
   if (!parent || !child)
      return;

   if ((l = evas_list_find_list(parent->children, child)))
   {
      if (child->swallowed)
         etk_widget_unswallow_widget(parent, child);
      if (child->clip)
         etk_widget_clip_unset(child);
      
      parent->children = evas_list_remove_list(parent->children, l);
      etk_widget_size_recalc_queue(parent);
   }
}

/* Adds an object to the widget smart object */
static void _etk_widget_object_add_to_smart(Etk_Widget *widget, Evas_Object *object)
{
   if (!widget || !widget->smart_object || !object)
      return;
   
   _etk_widget_intercept_show_hide = ETK_FALSE;
   if (!evas_object_visible_get(widget->smart_object))
      evas_object_hide(object);
   _etk_widget_intercept_show_hide = ETK_TRUE;
   
   if (widget->clip)
      evas_object_clip_set(object, widget->clip);
   evas_object_smart_member_add(object, widget->smart_object);
   evas_object_raise(object);
}

/* Finds if an evas object is a member object of the widget */
static Evas_List *_etk_widget_member_object_find(Etk_Widget *widget, Evas_Object *object)
{
   Evas_List *l;
   Etk_Widget_Member_Object *m;
   
   if (!widget || !object)
      return NULL;
   
   for (l = widget->member_objects; l; l = l->next)
   {
      m = l->data;
      if (m->object == object)
         return l;
   }
   return NULL;
}

/* Called when a member object requests to be shown. Shows it only if all the parents are visible */
static void _etk_widget_member_object_intercept_show_cb(void *data, Evas_Object *obj)
{
   Etk_Widget *widget;
   Evas_List *l;
   Etk_Widget_Member_Object *member_object;
   
   if (!_etk_widget_intercept_show_hide)
   {
      evas_object_show(obj);
      return;
   }
   
   if (!(widget = ETK_WIDGET(data)))
      return;
   
   if ((l = _etk_widget_member_object_find(widget, obj)))
   {
      member_object = l->data;
      member_object->visible = ETK_TRUE;
   }
   
   if (widget->smart_object && evas_object_visible_get(widget->smart_object))
      evas_object_show(obj);
}


/* Called when a member object of the widget requests to be hidden */
static void _etk_widget_member_object_intercept_hide_cb(void *data, Evas_Object *obj)
{
   Evas_List *l;
   Etk_Widget *widget;
   Etk_Widget_Member_Object *member_object;

   if (!_etk_widget_intercept_show_hide)
   {
      evas_object_hide(obj);
      return;
   }
   
   if (!(widget = ETK_WIDGET(data)))
      return;
   
   if ((l = _etk_widget_member_object_find(widget, obj)))
   {
      member_object = l->data;
      member_object->visible = ETK_FALSE;
   }
   evas_object_hide(obj);
}

/* Creates a new smart object for the widget */
static Evas_Object *_etk_widget_smart_object_add(Evas *evas, Etk_Widget *widget)
{
   Evas_Object *new_object;
   
   if (!evas || !widget)
      return NULL;

   if (!_etk_widget_smart_object_smart)
   {
      _etk_widget_smart_object_smart = evas_smart_new("etk_widget_smart_object",
         NULL, /* add */
         NULL, /* del */
         NULL, /* layer_set */
         NULL, /* raise */
         NULL, /* lower */
         NULL, /* stack_above */
         NULL, /* stack_below */
         _etk_widget_smart_object_move_cb, /* move */
         _etk_widget_smart_object_resize_cb, /* resize */
         _etk_widget_smart_object_show_cb, /* show */
         _etk_widget_smart_object_hide_cb, /* hide */
         NULL, /* color_set */
         _etk_widget_smart_object_clip_set_cb, /* clip_set */
         _etk_widget_smart_object_clip_unset_cb, /* clip_unset */
         NULL); /* data*/
   }

   new_object = evas_object_smart_add(evas, _etk_widget_smart_object_smart);
   evas_object_smart_data_set(new_object, widget);
   return new_object;
}

/* Called when the smart object is moved */
static void _etk_widget_smart_object_move_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Etk_Widget *widget, *child;

   if (!obj || !(widget = ETK_WIDGET(evas_object_smart_data_get(obj))))
      return;

   if (x != widget->geometry.x || y != widget->geometry.y)
   {
      Evas_List *l;
      Etk_Widget_Member_Object *m;
      Evas_Coord child_x, child_y;
      int x_offset, y_offset;

      x_offset = x - widget->geometry.x;
      y_offset = y - widget->geometry.y;
      widget->geometry.x = x;
      widget->geometry.y = y;
      widget->inner_geometry.x = widget->geometry.x + widget->left_inset;
      widget->inner_geometry.y = widget->geometry.y + widget->top_inset;

      for (l = widget->member_objects; l; l = l->next)
      {
         m = l->data;
         if (!m->swallowed)
         {
            evas_object_geometry_get(m->object, &child_x, &child_y, NULL, NULL);
            evas_object_move(m->object, child_x + x_offset, child_y + y_offset);
         }
      }
      for (l = widget->children; l; l = l->next)
      {
         child = l->data;
         if (!child->swallowed)
         {
            evas_object_geometry_get(child->smart_object, &child_x, &child_y, NULL, NULL);
            evas_object_move(child->smart_object, child_x + x_offset, child_y + y_offset);
         }
      }
      
      etk_object_notify(ETK_OBJECT(widget), "geometry");
   }
}

/* Called when the smart object is resized */
static void _etk_widget_smart_object_resize_cb(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Etk_Widget *widget;
   
   if (!obj || !(widget = ETK_WIDGET(evas_object_smart_data_get(obj))))
      return;

   if (w != widget->geometry.w || h != widget->geometry.h || widget->need_redraw)
   {
      widget->geometry.w = w;
      widget->geometry.h = h;
      widget->inner_geometry.w = widget->geometry.w - widget->left_inset - widget->right_inset;
      widget->inner_geometry.h = widget->geometry.h - widget->top_inset - widget->bottom_inset;

      if (widget->theme_object)
      {
         evas_object_move(widget->theme_object, widget->geometry.x, widget->geometry.y);
         evas_object_resize(widget->theme_object, widget->geometry.w, widget->geometry.h);
      }
      if (widget->event_object)
      {
         evas_object_move(widget->event_object, widget->geometry.x, widget->geometry.y);
         evas_object_resize(widget->event_object, widget->geometry.w, widget->geometry.h);
      }
      if (widget->size_allocate)
         widget->size_allocate(widget, widget->inner_geometry);
      
      widget->need_redraw = ETK_FALSE;
      etk_object_notify(ETK_OBJECT(widget), "geometry");
   }
}

/* Called when the smart object is shown */
static void _etk_widget_smart_object_show_cb(Evas_Object *obj)
{
   Evas_List *l;
   Etk_Widget_Member_Object *m;
   Etk_Widget *widget, *child;
   
   if (!obj || !(widget = ETK_WIDGET(evas_object_smart_data_get(obj))))
      return;
   
   for (l = widget->member_objects; l; l = l->next)
   {
      m = l->data;
      if (m->visible || m->swallowed)
      {
         _etk_widget_intercept_show_hide = ETK_FALSE;
         evas_object_show(m->object);
         _etk_widget_intercept_show_hide = ETK_TRUE;
      }
   }
   for (l = widget->children; l; l = l->next)
   {
      child = l->data;
      if (child->visible)
         evas_object_show(child->smart_object);
   }
}

/* Called when the smart object is hidden */
static void _etk_widget_smart_object_hide_cb(Evas_Object *obj)
{
   Evas_List *l;
   Etk_Widget_Member_Object *m;
   Etk_Widget *widget, *child;
   
   if (!obj || !(widget = ETK_WIDGET(evas_object_smart_data_get(obj))))
      return;
   
   for (l = widget->member_objects; l; l = l->next)
   {
      m = l->data;
      _etk_widget_intercept_show_hide = ETK_FALSE;
      evas_object_hide(m->object);
      _etk_widget_intercept_show_hide = ETK_TRUE;
   }
   for (l = widget->children; l; l = l->next)
   {
      child = l->data;
      evas_object_hide(child->smart_object);
   }
}

/* Called when a clip is set to the smart object */
static void _etk_widget_smart_object_clip_set_cb(Evas_Object *object, Evas_Object *clip)
{
   Evas_List *l;
   Etk_Widget_Member_Object *m;
   Etk_Widget *widget, *child;
   Evas_Object *o;

   if (!object || !clip || !(widget = ETK_WIDGET(evas_object_smart_data_get(object))))
      return;

   if (widget->clip)
      etk_widget_clip_unset(widget);
   widget->clip = clip;

   for (l = widget->member_objects; l; l = l->next)
   {
      m = l->data;
      for (o = m->object; o && (o != clip) && evas_object_clip_get(o); o = evas_object_clip_get(o));
      if (o != clip)
         evas_object_clip_set(o, clip);
   }
   for (l = widget->children; l; l = l->next)
   {
      child = ETK_WIDGET(l->data);
      if (child->clip)
         evas_object_clip_set(child->clip, clip);
      else
      {
         for (o = child->smart_object; o && (o != clip) && evas_object_clip_get(o); o = evas_object_clip_get(o));
         if (o != clip)
            evas_object_clip_set(o, clip);
      }
   }
}

/* Called when the clip of the smart object is unset */
static void _etk_widget_smart_object_clip_unset_cb(Evas_Object *object)
{
   Evas_List *l;
   Etk_Widget_Member_Object *m;
   Etk_Widget *widget, *child;

   if (!object || !(widget = ETK_WIDGET(evas_object_smart_data_get(object))) || !widget->clip)
      return;

   for (l = widget->member_objects; l; l = l->next)
   {
      m = l->data;
      if (evas_object_clip_get(m->object) == widget->clip)
         evas_object_clip_unset(m->object);
   }
   for (l = widget->children; l; l = l->next)
   {
      child = ETK_WIDGET(l->data);
      if (child->clip == widget->clip)
         etk_widget_clip_unset(child);
      else
      {
         Evas_Object *c;

         for (c = child->clip; c; c = evas_object_clip_get(c))
         {
            if (evas_object_clip_get(c) == widget->clip)
            {
               evas_object_clip_unset(c);
               break;
            }
         }
      }
   }
   widget->clip = NULL;
}

static void _etk_widget_dnd_drag_mouse_move_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Mouse_Move *ev;
   
   ev = event;

   if(ev->buttons & 0x001 && _etk_dnd_drag_start)
   {
      const char **types;
      unsigned int num_types;
      char *data;
      Etk_Widget *drag;
      
      drag = (ETK_WIDGET(object))->drag;
      
      _etk_dnd_drag_start = ETK_FALSE;
      types = calloc(1, sizeof(char*));
      num_types = 1;
      types[0] = strdup("text/plain");
      data = strdup("This is the drag data!");
      
      //etk_drag_types_set(drag, types, num_types);
      //etk_drag_data_set(drag, data, strlen(data) + 1);
      etk_drag_begin(ETK_DRAG(drag));
   }
}

static void _etk_widget_dnd_drag_end_cb(Etk_Object *object, void *data)
{
   _etk_dnd_drag_start = ETK_TRUE;   
}

/** @} */
