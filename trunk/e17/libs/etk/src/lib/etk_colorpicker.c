/** @file etk_colorpicker.c */
#include "etk_colorpicker.h"
#include <stdlib.h>
#include <stdint.h>
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_utils.h"
#include "etk_radio_button.h"

/**
 * @addtogroup Etk_Colorpicker
 * @{
 */

enum Etk_Combobox_Signal_Id
{
   ETK_CP_COLOR_CHANGED_SIGNAL,
   ETK_CP_NUM_SIGNALS
};

enum Etk_Colorpicker_Property_Id
{
   ETK_CP_MODE_PROPERTY
};

static void _etk_colorpicker_constructor(Etk_Colorpicker *cp);
static void _etk_colorpicker_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_colorpicker_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_colorpicker_size_request(Etk_Widget *widget, Etk_Size *size);
static void _etk_colorpicker_size_allocate(Etk_Widget *widget, Etk_Geometry geometry);

static void _etk_colorpicker_realize_cb(Etk_Object *object, void *data);
static void _etk_colorpicker_unrealize_cb(Etk_Object *object, void *data);
static void _etk_colorpicker_radio_toggled_cb(Etk_Object *object, void *data);

static void _etk_colorpicker_sp_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _etk_colorpicker_sp_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _etk_colorpicker_sp_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _etk_colorpicker_vp_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _etk_colorpicker_vp_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _etk_colorpicker_vp_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _etk_colorpicker_sp_move_resize(Etk_Colorpicker *cp, int x, int y, int w, int h);
static void _etk_colorpicker_vp_move_resize(Etk_Colorpicker *cp, int x, int y, int w, int h);

static void _etk_colorpicker_update(Etk_Colorpicker *cp, Etk_Bool sp_image, Etk_Bool sp_cursor, Etk_Bool vp_image, Etk_Bool vp_cursor);
static void _etk_colorpicker_sp_image_update(Etk_Colorpicker *cp);
static void _etk_colorpicker_sp_cursor_update(Etk_Colorpicker *cp);
static void _etk_colorpicker_vp_image_update(Etk_Colorpicker *cp);
static void _etk_colorpicker_vp_cursor_update(Etk_Colorpicker *cp);
static void _etk_colorpicker_sp_color_get(Etk_Colorpicker *cp, int i, int j, int *r, int *g, int *b);
static void _etk_colorpicker_vp_color_get(Etk_Colorpicker *cp, int i, int *r, int *g, int *b);
static void _etk_colorpicker_color_calc(Etk_Colorpicker_Mode mode, float sp_xpos, float sp_ypos, float vp_pos, int *r, int *g, int *b);
static Etk_Signal *_etk_colorpicker_signals[ETK_CP_NUM_SIGNALS];


/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Colorpicker
 * @return Returns the type of an Etk_Colorpicker
 */
Etk_Type *etk_colorpicker_type_get()
{
   static Etk_Type *cp_type = NULL;

   if (!cp_type)
   {
      cp_type = etk_type_new("Etk_Colorpicker", ETK_WIDGET_TYPE, sizeof(Etk_Colorpicker),
         ETK_CONSTRUCTOR(_etk_colorpicker_constructor), NULL);
   
      _etk_colorpicker_signals[ETK_CP_COLOR_CHANGED_SIGNAL] = etk_signal_new("color_changed",
         cp_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      
      etk_type_property_add(cp_type, "mode", ETK_CP_MODE_PROPERTY,
         ETK_PROPERTY_INT, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_int(ETK_COLORPICKER_H));
   
      cp_type->property_set = _etk_colorpicker_property_set;
      cp_type->property_get = _etk_colorpicker_property_get;
   }

   return cp_type;
}

/**
 * @brief Creates a new colorpicker
 * @return Returns the new colorpicker
 */
Etk_Widget *etk_colorpicker_new()
{
   return etk_widget_new(ETK_COLORPICKER_TYPE, "theme_group", "colorpicker", NULL);
}

/**
 * @brief Sets the current color mode of the colorpicker
 * @param cp a colorpicker
 * @param mode the color mode to use
 */
void etk_colorpicker_mode_set(Etk_Colorpicker *cp, Etk_Colorpicker_Mode mode)
{
   if (!cp || (cp->mode == mode))
      return;
   
   cp->mode = mode;
   etk_colorpicker_current_color_set(cp, cp->current_color);
   
   etk_object_notify(ETK_OBJECT(cp), "mode");
}

/**
 * @brief Gets the current color mode of colorpicker
 * @param cp a colorpicker
 * @return Returns the current color mode of colorpicker
 */
Etk_Colorpicker_Mode etk_colorpicker_mode_get(Etk_Colorpicker *cp)
{
   if (!cp)
      return ETK_COLORPICKER_H;
   return cp->mode;
}

/**
 * @brief Gets the color currently selected by the colorpicker
 * @param cp a colorpicker
 * @return Returns the color selected by the colorpicker
 */
Etk_Color etk_colorpicker_current_color_get(Etk_Colorpicker *cp)
{
   if (!cp)
   {
      Etk_Color black;
      black.r = 0;
      black.g = 0;
      black.b = 0;
      black.a = 255;
      return black;
   }
   
   return cp->current_color;
}

/* TODO: doc, signal */
void etk_colorpicker_current_color_set(Etk_Colorpicker *cp, Etk_Color color)
{
   int r, g, b;
   float h, s, v;
   
   r = color.r;
   g = color.g;
   b = color.b;
   evas_color_rgb_to_hsv(r, g, b, &h, &s, &v);
   
   switch (cp->mode)
   {
      case ETK_COLORPICKER_H:
         cp->sp_xpos = v;
         cp->sp_ypos = s;
         cp->vp_pos = h / 360.0;
         break;
      case ETK_COLORPICKER_S:
         cp->sp_xpos = v;
         cp->sp_ypos = h / 360.0;
         cp->vp_pos = s;
         break;
      case ETK_COLORPICKER_V:
         cp->sp_xpos = s;
         cp->sp_ypos = h / 360.0;
         cp->vp_pos = v;
         break;
      case ETK_COLORPICKER_R:
         cp->sp_xpos = b / 255.0;
         cp->sp_ypos = g / 255.0;
         cp->vp_pos = r / 255.0;
         break;
      case ETK_COLORPICKER_G:
         cp->sp_xpos = b / 255.0;
         cp->sp_ypos = r / 255.0;
         cp->vp_pos = g / 255.0;
         break;
      case ETK_COLORPICKER_B:
         cp->sp_xpos = g / 255.0;
         cp->sp_ypos = r / 255.0;
         cp->vp_pos = b / 255.0;
         break;
      default:
         break;
   }
   
   _etk_colorpicker_update(cp, ETK_TRUE, ETK_TRUE, ETK_TRUE, ETK_TRUE);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the colorpicker */
static void _etk_colorpicker_constructor(Etk_Colorpicker *cp)
{
   Etk_Widget *cp_widget;
   char *labels[6] = {"H", "S", "V", "R", "G", "B"};
   int i;

   if (!(cp_widget = ETK_WIDGET(cp)))
      return;
   
   cp->mode = ETK_COLORPICKER_H;
   cp->current_color.r = 0;
   cp->current_color.g = 0;
   cp->current_color.b = 0;
   cp->current_color.a = 255;
   
   cp->sp_image = NULL;
   cp->sp_hcursor = NULL;
   cp->sp_vcursor = NULL;
   cp->sp_res = 64;
   cp->sp_xpos = 0.0;
   cp->sp_ypos = 0.0;
   
   cp->vp_image = NULL;
   cp->vp_cursor = NULL;
   cp->vp_res = 256;
   cp->vp_pos = 0.0;
   
   cp->sp_dragging = ETK_FALSE;
   cp->vp_dragging = ETK_FALSE;
   cp->sp_image_needs_update = ETK_FALSE;
   cp->sp_cursor_needs_update = ETK_FALSE;
   cp->vp_image_needs_update = ETK_FALSE;
   cp->vp_cursor_needs_update = ETK_FALSE;
   
   cp->radio_vbox = etk_vbox_new(0, ETK_TRUE);
   etk_widget_parent_set(cp->radio_vbox, cp_widget);
   etk_widget_visibility_locked_set(cp->radio_vbox, ETK_TRUE);
   etk_widget_show(cp->radio_vbox);
   
   for (i = 0; i < 6; i++)
   {
      cp->radios[i] = etk_radio_button_new_with_label_from_widget(labels[i],
         (i == 0) ? NULL : ETK_RADIO_BUTTON(cp->radios[0]));
      etk_box_pack_start(ETK_BOX(cp->radio_vbox), cp->radios[i], ETK_TRUE, ETK_TRUE, 0);
      etk_widget_visibility_locked_set(cp->radios[i], ETK_TRUE);
      etk_widget_show(cp->radios[i]);

      etk_signal_connect("toggled", ETK_OBJECT(cp->radios[i]),
         ETK_CALLBACK(_etk_colorpicker_radio_toggled_cb), cp);
   }
   
   cp_widget->size_request = _etk_colorpicker_size_request;
   cp_widget->size_allocate = _etk_colorpicker_size_allocate;
   
   etk_signal_connect("realize", ETK_OBJECT(cp), ETK_CALLBACK(_etk_colorpicker_realize_cb), NULL);
   etk_signal_connect("unrealize", ETK_OBJECT(cp), ETK_CALLBACK(_etk_colorpicker_unrealize_cb), NULL);
   
   Etk_Color test;
   test.r = 161;
   test.g = 177;
   test.b = 0;
   etk_colorpicker_current_color_set(cp, test);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_colorpicker_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Colorpicker *cp;

   if (!(cp = ETK_COLORPICKER(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_CP_MODE_PROPERTY:
         etk_colorpicker_mode_set(cp, etk_property_value_int_get(value));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_colorpicker_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Colorpicker *cp;

   if (!(cp = ETK_COLORPICKER(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_CP_MODE_PROPERTY:
         etk_property_value_int_set(value, cp->mode);
         break;
      default:
         break;
   }
}

/* Calculates the ideal size of the colorpicker */
/* TODO: size_request */
static void _etk_colorpicker_size_request(Etk_Widget *widget, Etk_Size *size)
{
   if (!size)
      return;
   
   size->w = 480;
   size->h = 200;
}

/* Resizes the colorpicker to the allocated size */
/* TODO: size_allocate */
static void _etk_colorpicker_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   Etk_Colorpicker *cp;
   Etk_Geometry child_geometry;
   
   if (!(cp = ETK_COLORPICKER(widget)))
      return;
   
   /* First, updates the data of the images if needed */
   if (cp->sp_image_needs_update)
      _etk_colorpicker_sp_image_update(cp);
   if (cp->sp_cursor_needs_update)
      _etk_colorpicker_sp_cursor_update(cp);
   if (cp->vp_image_needs_update)
      _etk_colorpicker_vp_image_update(cp);
   if (cp->vp_cursor_needs_update)
      _etk_colorpicker_vp_cursor_update(cp);
   
   /* Then, moves and resizes the objects */
   _etk_colorpicker_sp_move_resize(cp, geometry.x, geometry.y, (geometry.w / 2) - 30, geometry.h);
   _etk_colorpicker_vp_move_resize(cp, geometry.x + (geometry.w / 2) - 25, geometry.y, 20, geometry.h);
   
   child_geometry.x = geometry.x + (geometry.w / 2);
   child_geometry.y = geometry.y;
   child_geometry.w = geometry.w / 2;
   child_geometry.h = geometry.h;
   etk_widget_size_allocate(cp->radio_vbox, child_geometry);
   
   cp->sp_image_needs_update = ETK_FALSE;
   cp->sp_cursor_needs_update = ETK_FALSE;
   cp->vp_image_needs_update = ETK_FALSE;
   cp->vp_cursor_needs_update = ETK_FALSE;
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the colorpicker is realized */
/* TODO: use smart objects! */
static void _etk_colorpicker_realize_cb(Etk_Object *object, void *data)
{
   Etk_Colorpicker *cp;
   Evas *evas;
   
   if (!(cp = ETK_COLORPICKER(object)) || !(evas = etk_widget_toplevel_evas_get(ETK_WIDGET(cp))))
      return;
   
   /* Square picker */
   cp->sp_image = evas_object_image_add(evas);
   evas_object_image_alpha_set(cp->sp_image, 0);
   evas_object_image_size_set(cp->sp_image, cp->sp_res, cp->sp_res);
   evas_object_show(cp->sp_image);
   etk_widget_member_object_add(ETK_WIDGET(cp), cp->sp_image);
   
   cp->sp_hcursor = evas_object_image_add(evas);
   evas_object_image_alpha_set(cp->sp_hcursor, 0);
   evas_object_image_size_set(cp->sp_hcursor, cp->sp_res, 1);
   evas_object_pass_events_set(cp->sp_hcursor, 1);
   evas_object_show(cp->sp_hcursor);
   etk_widget_member_object_add(ETK_WIDGET(cp), cp->sp_hcursor);
   
   cp->sp_vcursor = evas_object_image_add(evas);
   evas_object_image_alpha_set(cp->sp_vcursor, 0);
   evas_object_image_size_set(cp->sp_vcursor, 1, cp->sp_res);
   evas_object_pass_events_set(cp->sp_vcursor, 1);
   evas_object_show(cp->sp_vcursor);
   etk_widget_member_object_add(ETK_WIDGET(cp), cp->sp_vcursor);
   
   /* Vertical picker */
   cp->vp_image = evas_object_image_add(evas);
   evas_object_image_alpha_set(cp->vp_image, 0);
   evas_object_image_size_set(cp->vp_image, 1, cp->vp_res);
   evas_object_show(cp->vp_image);
   etk_widget_member_object_add(ETK_WIDGET(cp), cp->vp_image);
   
   cp->vp_cursor = evas_object_image_add(evas);
   evas_object_image_alpha_set(cp->vp_cursor, 0);
   evas_object_image_size_set(cp->vp_cursor, 1, 1);
   evas_object_pass_events_set(cp->vp_cursor, 1);
   evas_object_show(cp->vp_cursor);
   etk_widget_member_object_add(ETK_WIDGET(cp), cp->vp_cursor);
   
   /* Adds the mouse callbacks */
   evas_object_event_callback_add(cp->sp_image, EVAS_CALLBACK_MOUSE_DOWN, _etk_colorpicker_sp_mouse_down_cb, cp);
   evas_object_event_callback_add(cp->sp_image, EVAS_CALLBACK_MOUSE_UP, _etk_colorpicker_sp_mouse_up_cb, cp);
   evas_object_event_callback_add(cp->sp_image, EVAS_CALLBACK_MOUSE_MOVE, _etk_colorpicker_sp_mouse_move_cb, cp);
   evas_object_event_callback_add(cp->vp_image, EVAS_CALLBACK_MOUSE_DOWN, _etk_colorpicker_vp_mouse_down_cb, cp);
   evas_object_event_callback_add(cp->vp_image, EVAS_CALLBACK_MOUSE_UP, _etk_colorpicker_vp_mouse_up_cb, cp);
   evas_object_event_callback_add(cp->vp_image, EVAS_CALLBACK_MOUSE_MOVE, _etk_colorpicker_vp_mouse_move_cb, cp);
   
   /* Updates the colorpicker */
   _etk_colorpicker_update(cp, ETK_TRUE, ETK_TRUE, ETK_TRUE, ETK_TRUE);
}

/* Called when the colorpicker is unrealized */
static void _etk_colorpicker_unrealize_cb(Etk_Object *object, void *data)
{
   Etk_Colorpicker *cp;
   
   if (!(cp = ETK_COLORPICKER(object)))
      return;
   
   cp->sp_image = NULL;
   cp->sp_hcursor = NULL;
   cp->sp_vcursor = NULL;
   cp->vp_image = NULL;
   cp->vp_cursor = NULL;
}

/* Called when the color mode is changed with the radio buttons */
static void _etk_colorpicker_radio_toggled_cb(Etk_Object *object, void *data)
{
   Etk_Colorpicker *cp;
   Etk_Widget *radio;
   int i;
   
   if (!(radio = ETK_WIDGET(object)) || !(cp = ETK_COLORPICKER(data)))
      return;
   if (!etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(radio)))
      return;
   
   for (i = 0; i < 6; i++)
   {
      if (cp->radios[i] == radio)
      {
         etk_colorpicker_mode_set(cp, i);
         return;
      }
   }
}

/* Called when the square picker is pressed by the mouse */
static void _etk_colorpicker_sp_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   Evas_Event_Mouse_Down *event;
   int x, y, w, h;
   
   if (!(cp = ETK_COLORPICKER(data)) || !(event = event_info))
      return;
   
   evas_object_geometry_get(cp->sp_image, &x, &y, &w, &h);
   cp->sp_xpos = ETK_CLAMP((float)(event->canvas.x - x) / w, 0.0, 1.0);
   cp->sp_ypos = 1.0 - ETK_CLAMP((float)(event->canvas.y - y) / h, 0.0, 1.0);
   _etk_colorpicker_update(cp, ETK_FALSE, ETK_TRUE, ETK_FALSE, ETK_FALSE);
   
   cp->sp_dragging = ETK_TRUE;
}

/* Called when the square picker is released by the mouse */
static void _etk_colorpicker_sp_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   
   if (!(cp = ETK_COLORPICKER(data)))
      return;
   cp->sp_dragging = ETK_FALSE;
}

/* Called when the mouse is moved over the square picker */
static void _etk_colorpicker_sp_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   Evas_Event_Mouse_Move *event;
   int x, y, w, h;
   
   if (!(cp = ETK_COLORPICKER(data)) || !(event = event_info) || !cp->sp_dragging)
      return;
   
   evas_object_geometry_get(cp->sp_image, &x, &y, &w, &h);
   cp->sp_xpos = ETK_CLAMP((float)(event->cur.canvas.x - x) / w, 0.0, 1.0);
   cp->sp_ypos = 1.0 - ETK_CLAMP((float)(event->cur.canvas.y - y) / h, 0.0, 1.0);
   _etk_colorpicker_update(cp, ETK_FALSE, ETK_TRUE, ETK_FALSE, ETK_FALSE);
}

/* Called when the vertical picker is pressed by the mouse */
static void _etk_colorpicker_vp_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   Evas_Event_Mouse_Down *event;
   int y, h;
   
   if (!(cp = ETK_COLORPICKER(data)) || !(event = event_info))
      return;
   
   evas_object_geometry_get(cp->vp_image, NULL, &y, NULL, &h);
   cp->vp_pos = 1.0 - ETK_CLAMP((float)(event->canvas.y - y) / h, 0.0, 1.0);
   _etk_colorpicker_update(cp, ETK_TRUE, ETK_TRUE, ETK_TRUE, ETK_TRUE);
   
   cp->vp_dragging = ETK_TRUE;
}

/* Called when the vertical picker is released by the mouse */
static void _etk_colorpicker_vp_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   
   if (!(cp = ETK_COLORPICKER(data)))
      return;
   cp->vp_dragging = ETK_FALSE;
}

/* Called when the mouse is moved over the vertical picker */
static void _etk_colorpicker_vp_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Colorpicker *cp;
   Evas_Event_Mouse_Move *event;
   int y, h;
   
   if (!(cp = ETK_COLORPICKER(data)) || !(event = event_info) || !cp->vp_dragging)
      return;
   
   evas_object_geometry_get(cp->vp_image, NULL, &y, NULL, &h);
   cp->vp_pos = 1.0 - ETK_CLAMP((float)(event->cur.canvas.y - y) / h, 0.0, 1.0);
   _etk_colorpicker_update(cp, ETK_TRUE, ETK_TRUE, ETK_TRUE, ETK_TRUE);
}

/**************************
 *
 * Private functions
 *
 **************************/

/* Moves and resizes the square picker */
static void _etk_colorpicker_sp_move_resize(Etk_Colorpicker *cp, int x, int y, int w, int h)
{
   if (!cp || !cp->sp_image || !cp->sp_hcursor || !cp->sp_vcursor)
      return;
   
   evas_object_move(cp->sp_image, x, y);
   evas_object_resize(cp->sp_image, w, h);
   evas_object_image_fill_set(cp->sp_image, 0, 0, w, h);
   
   evas_object_move(cp->sp_hcursor, x, y + ((1.0 - cp->sp_ypos) * h));
   evas_object_resize(cp->sp_hcursor, w, 1);
   evas_object_image_fill_set(cp->sp_hcursor, 0, 0, w, 1);
   
   evas_object_move(cp->sp_vcursor, x + (cp->sp_xpos * w), y);
   evas_object_resize(cp->sp_vcursor, 1, h);
   evas_object_image_fill_set(cp->sp_vcursor, 0, 0, 1, h);
}

/* Moves and resizes the vertical picker */
static void _etk_colorpicker_vp_move_resize(Etk_Colorpicker *cp, int x, int y, int w, int h)
{
   if (!cp || !cp->vp_image || !cp->vp_cursor)
      return;
   
   evas_object_move(cp->vp_image, x, y);
   evas_object_resize(cp->vp_image, w, h);
   evas_object_image_fill_set(cp->vp_image, 0, 0, w, h);
   
   evas_object_move(cp->vp_cursor, x, y + ((1.0 - cp->vp_pos) * h));
   evas_object_resize(cp->vp_cursor, w, 1);
   evas_object_image_fill_set(cp->vp_cursor, 0, 0, w, 1);
}

/* Updates of the colorpicker */ 
static void _etk_colorpicker_update(Etk_Colorpicker *cp, Etk_Bool sp_image, Etk_Bool sp_cursor, Etk_Bool vp_image, Etk_Bool vp_cursor)
{
   int r, g, b;
   
   if (!cp)
      return;
   
   cp->sp_image_needs_update |= sp_image;
   cp->sp_cursor_needs_update |= sp_cursor;
   cp->vp_image_needs_update |= vp_image;
   cp->vp_cursor_needs_update |= vp_cursor;
   
   /* Updates the color */
   _etk_colorpicker_color_calc(cp->mode, cp->sp_xpos, cp->sp_ypos, cp->vp_pos, &r, &g, &b);
   if (cp->current_color.r != r || cp->current_color.g != g || cp->current_color.b != b)
   {
      cp->current_color.r = r;
      cp->current_color.g = g;
      cp->current_color.b = b;
      etk_signal_emit(_etk_colorpicker_signals[ETK_CP_COLOR_CHANGED_SIGNAL], ETK_OBJECT(cp), NULL);
   }
   
   etk_widget_redraw_queue(ETK_WIDGET(cp));
}

/* Updates the square picker image */
static void _etk_colorpicker_sp_image_update(Etk_Colorpicker *cp)
{
   uint32_t *data;
   int i, j;
   int r, g, b;
   
   if (!cp || !cp->sp_image)
      return;
   if (!(data = (uint32_t *)evas_object_image_data_get(cp->sp_image, 1)))
      return;
   
   for (i = 0; i < cp->sp_res; i++)
   {
      for (j = 0; j < cp->sp_res; j++)
      {
         _etk_colorpicker_sp_color_get(cp, i, j, &r, &g, &b);
         *data = ((r << 16) | (g << 8) | b);
         data++;
      }
   }
   
   evas_object_image_data_update_add(cp->sp_image, 0, 0, cp->sp_res, cp->sp_res);
}

/* Updates the cursor of the square picker */
static void _etk_colorpicker_sp_cursor_update(Etk_Colorpicker *cp)
{
   uint32_t *data;
   int i, j;
   int r, g, b;
   
   if (!cp)
      return;
   
   /* Updates the horizontal cursor */
   if (cp->sp_hcursor && (data = (uint32_t *)evas_object_image_data_get(cp->sp_hcursor, 1)))
   {
      j = cp->sp_res * (1.0 - cp->sp_ypos);
      for (i = 0; i < cp->sp_res; i++)
      {
         _etk_colorpicker_sp_color_get(cp, i, j, &r, &g, &b);
         *data = (((255 - r) << 16) | ((255 - g) << 8) | (255 - b));
         data++;
      }
      evas_object_image_data_update_add(cp->sp_hcursor, 0, 0, cp->sp_res, 1);
   }
   
   /* Updates the vertical cursor */
   if (cp->sp_vcursor && (data = (uint32_t *)evas_object_image_data_get(cp->sp_vcursor, 1)))
   {
      i = cp->sp_res * cp->sp_xpos;
      for (j = 0; j < cp->sp_res; j++)
      {
         _etk_colorpicker_sp_color_get(cp, i, j, &r, &g, &b);
         *data = (((255 - r) << 16) | ((255 - g) << 8) | (255 - b));
         data++;
      }
      evas_object_image_data_update_add(cp->sp_vcursor, 0, 0, 1, cp->sp_res);
   }
}

/* Updates the vertical picker image */
static void _etk_colorpicker_vp_image_update(Etk_Colorpicker *cp)
{
   uint32_t *data;
   int i;
   int r, g, b;
   
   if (!cp || !cp->vp_image)
      return;
   if (!(data = (uint32_t *)evas_object_image_data_get(cp->vp_image, 1)))
      return;
   
   for (i = 0; i < cp->vp_res; i++)
   {
      _etk_colorpicker_vp_color_get(cp, i, &r, &g, &b);
      *data = ((r << 16) | (g << 8) | b);
      data++;
   }
   
   evas_object_image_data_update_add(cp->vp_image, 0, 0, 1, cp->vp_res);
}

/* Updates the vertical picker cursor */
static void _etk_colorpicker_vp_cursor_update(Etk_Colorpicker *cp)
{
   uint32_t *data;
   int r, g, b;
   
   if (!cp || !cp->vp_cursor)
      return;
   if (!(data = (uint32_t *)evas_object_image_data_get(cp->vp_cursor, 1)))
      return;
   
   _etk_colorpicker_vp_color_get(cp, cp->vp_res * (1.0 - cp->vp_pos), &r, &g, &b);
   *data = (((255 - r) << 16) | ((255 - g) << 8) | (255 - b));
   
   evas_object_image_data_update_add(cp->vp_cursor, 0, 0, 1, 1);
}

/* Get the color of the square picker's image, at the point (i, j). (r, g, b) must not be NULL! */
static void _etk_colorpicker_sp_color_get(Etk_Colorpicker *cp, int i, int j, int *r, int *g, int *b)
{
   _etk_colorpicker_color_calc(cp->mode, 1.0 - ((float)i / cp->sp_res), (float)j / cp->sp_res, cp->vp_pos, r, g, b);
}

/* Get the color of the vertical picker's image, at the point i. (r, g, b) must not be NULL!  */
static void _etk_colorpicker_vp_color_get(Etk_Colorpicker *cp, int i, int *r, int *g, int *b)
{
   switch (cp->mode)
   {
      case ETK_COLORPICKER_H:
         evas_color_hsv_to_rgb(360.0 * (1.0 - ((float)i / cp->vp_res)), 1.0, 1.0, r, g, b);
         break;
      case ETK_COLORPICKER_S:
         *r = 255 - ((i * 255) / cp->vp_res);
         *g = 255 - ((i * 255) / cp->vp_res);
         *b = 255 - ((i * 255) / cp->vp_res);
         break;
      case ETK_COLORPICKER_V:
         *r = 255 - ((i * 255) / cp->vp_res);
         *g = 255 - ((i * 255) / cp->vp_res);
         *b = 255 - ((i * 255) / cp->vp_res);
         break;
      case ETK_COLORPICKER_R:
         *r = 255 - ((i * 255) / cp->vp_res);
         *g = 0;
         *b = 0;
         break;
      case ETK_COLORPICKER_G:
         *r = 0;
         *g = 255 - ((i * 255) / cp->vp_res);
         *b = 0;
         break;
      case ETK_COLORPICKER_B:
         *r = 0;
         *g = 0;
         *b = 255 - ((i * 255) / cp->vp_res);
         break;
      default:
         break;
   }
}

/* Calculates a color according to the color mode and the positions of the cursors */
static void _etk_colorpicker_color_calc(Etk_Colorpicker_Mode mode, float sp_xpos, float sp_ypos, float vp_pos, int *r, int *g, int *b)
{
   switch (mode)
   {
      case ETK_COLORPICKER_H:
         evas_color_hsv_to_rgb(vp_pos * 360.0, sp_xpos, sp_ypos, r, g, b);
         break;
      case ETK_COLORPICKER_S:
         evas_color_hsv_to_rgb(sp_xpos * 360.0, vp_pos, sp_ypos, r, g, b);
         break;
      case ETK_COLORPICKER_V:
         evas_color_hsv_to_rgb(sp_xpos * 360.0, sp_ypos, vp_pos, r, g, b);
         break;
      case ETK_COLORPICKER_R:
         *r = 255 * vp_pos;
         *g = 255 * sp_xpos;
         *b = 255 * sp_ypos;
         break;
      case ETK_COLORPICKER_G:
         *r = 255 * sp_xpos;
         *g = 255 * vp_pos;
         *b = 255 * sp_ypos;
         break;
      case ETK_COLORPICKER_B:
         *r = 255 * sp_xpos;
         *g = 255 * sp_ypos;
         *b = 255 * vp_pos;
         break;
      default:
         break;
   }
}

/** @} */
