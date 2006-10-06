/** @file etk_entry.c */
#include "etk_entry.h"
#include <stdlib.h>
#include <string.h>
#include "etk_editable.h"
#include "etk_toplevel.h"
#include "etk_selection.h"
#include "etk_event.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_utils.h"

/**
 * @addtogroup Etk_Entry
 * @{
 */

enum Etk_Entry_Signal_Id
{
   ETK_ENTRY_TEXT_CHANGED_SIGNAL,
   ETK_ENTRY_NUM_SIGNALS
};

enum Etk_Entry_Propery_Id
{
   ETK_ENTRY_PASSWORD_MODE_PROPERTY
};

static void _etk_entry_constructor(Etk_Entry *entry);
static void _etk_entry_destructor(Etk_Entry *entry);
static void _etk_entry_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_entry_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_entry_realize_cb(Etk_Object *object, void *data);
static void _etk_entry_unrealize_cb(Etk_Object *object, void *data);
static void _etk_entry_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data);
static void _etk_entry_mouse_in_cb(Etk_Object *object, Etk_Event_Mouse_In *event, void *data);
static void _etk_entry_mouse_out_cb(Etk_Object *object, Etk_Event_Mouse_Out *event, void *data);
static void _etk_entry_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Down *event, void *data);
static void _etk_entry_mouse_up_cb(Etk_Object *object, Etk_Event_Mouse_Up *event, void *data);
static void _etk_entry_mouse_move_cb(Etk_Object *object, Etk_Event_Mouse_Move *event, void *data);
static void _etk_entry_focus_cb(Etk_Object *object, void *data);
static void _etk_entry_unfocus_cb(Etk_Object *object, void *data);
static void _etk_entry_selection_received_cb(Etk_Object *object, void *event, void *data);
static void _etk_entry_selection_copy(Etk_Entry *entry, Etk_Selection_Type selection, Etk_Bool cut);
  
static Etk_Signal *_etk_entry_signals[ETK_ENTRY_NUM_SIGNALS];


/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @internal
 * @brief Gets the type of an Etk_Entry
 * @return Returns the type of an Etk_Entry
 */
Etk_Type *etk_entry_type_get()
{
   static Etk_Type *entry_type = NULL;

   if (!entry_type)
   {
      entry_type = etk_type_new("Etk_Entry", ETK_WIDGET_TYPE, sizeof(Etk_Entry),
         ETK_CONSTRUCTOR(_etk_entry_constructor), ETK_DESTRUCTOR(_etk_entry_destructor));

      _etk_entry_signals[ETK_ENTRY_TEXT_CHANGED_SIGNAL] = etk_signal_new("text_changed",
         entry_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      
      etk_type_property_add(entry_type, "password_mode", ETK_ENTRY_PASSWORD_MODE_PROPERTY,
         ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE,  etk_property_value_bool(ETK_FALSE));
      
      entry_type->property_set = _etk_entry_property_set;
      entry_type->property_get = _etk_entry_property_get;      
   }

   return entry_type;
}

/**
 * @brief Creates a new entry
 * @return Returns the new entry widget
 */
Etk_Widget *etk_entry_new()
{
   return etk_widget_new(ETK_ENTRY_TYPE, "theme_group", "entry",
      "focusable", ETK_TRUE, "focus_on_click", ETK_TRUE, NULL);
}

/**
 * @brief Sets the text of the entry
 * @param entry an entry
 * @param text the text to set
 */
void etk_entry_text_set(Etk_Entry *entry, const char *text)
{
   if (!entry)
      return;
   
   if (!entry->editable_object)
   {
      if (entry->text != text)
      {
         free(entry->text);
         entry->text = text ? strdup(text) : NULL;
      }
   }
   else
      etk_editable_text_set(entry->editable_object, text);
   
   etk_signal_emit(_etk_entry_signals[ETK_ENTRY_TEXT_CHANGED_SIGNAL], ETK_OBJECT(entry), NULL);
}

/**
 * @brief Gets the text of the entry
 * @param entry an entry
 * @return Returns the text of the entry
 */
const char *etk_entry_text_get(Etk_Entry *entry)
{
   if (!entry)
      return NULL;
   
   if (!entry->editable_object)
      return entry->text;
   else
      return etk_editable_text_get(entry->editable_object);
}

/**
 * @brief Clears the text of the entry
 * @param entry the entry to clear
 */
void etk_entry_clear(Etk_Entry *entry)
{
   etk_entry_text_set(entry, NULL);
}

/**
 * @brief Sets whether or not the entry is in password mode
 * @param entry an entry
 * @param password_mode ETK_TRUE to turn the entry into a password entry, ETK_FALSE to turn it into a normal entry
 */
void etk_entry_password_mode_set(Etk_Entry *entry, Etk_Bool password_mode)
{
   if (!entry || entry->password_mode == password_mode)
      return;
   
   if (entry->editable_object)
      etk_editable_password_mode_set(entry->editable_object, password_mode);
   entry->password_mode = password_mode;
   etk_object_notify(ETK_OBJECT(entry), "password_mode");
}

/**
 * @brief Gets whether or not the entry is in password mode
 * @param entry an entry
 * @return Returns ETK_TRUE if the entry is in password mode, ETK_FALSE otherwise
 */
Etk_Bool etk_entry_password_mode_get(Etk_Entry *entry)
{
   if (!entry)
      return ETK_FALSE;
   return entry->password_mode;
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the default values of the entry */
static void _etk_entry_constructor(Etk_Entry *entry)
{
   if (!entry)
      return;

   entry->editable_object = NULL;
   entry->password_mode = ETK_FALSE;
   entry->selection_dragging = ETK_FALSE;
   entry->text = NULL;   
   
   etk_signal_connect("realize", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_realize_cb), NULL);
   etk_signal_connect("unrealize", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_unrealize_cb), NULL);
   etk_signal_connect("key_down", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_key_down_cb), NULL);
   etk_signal_connect("mouse_in", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_in_cb), NULL);
   etk_signal_connect("mouse_out", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_out_cb), NULL);
   etk_signal_connect("mouse_down", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_down_cb), NULL);
   etk_signal_connect("mouse_up", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_up_cb), NULL);
   etk_signal_connect("mouse_move", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_move_cb), NULL);
   etk_signal_connect("focus", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_focus_cb), NULL);
   etk_signal_connect("unfocus", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_unfocus_cb), NULL);
   etk_signal_connect("selection_received", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_selection_received_cb), NULL);
}

/* Destroys the entry */
static void _etk_entry_destructor(Etk_Entry *entry)
{
   if (!entry)
      return;
   free(entry->text);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_entry_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Entry *entry;
   
   if (!(entry = ETK_ENTRY(object)) || !value)
      return;
   
   switch (property_id)
   {
      case ETK_ENTRY_PASSWORD_MODE_PROPERTY:
	 etk_entry_password_mode_set(entry, etk_property_value_bool_get(value));
	 break;
      default:
	 break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_entry_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Entry *entry;
   
   if (!(entry = ETK_ENTRY(object)) || !value)
      return;
   
   switch (property_id)
   {
      case ETK_ENTRY_PASSWORD_MODE_PROPERTY:
         etk_property_value_bool_set(value, entry->password_mode);
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

/* Called when the entry is realized */
static void _etk_entry_realize_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;
   Evas *evas;

   if (!(entry = ETK_ENTRY(object)) || !(evas = etk_widget_toplevel_evas_get(ETK_WIDGET(entry))))
      return;

   entry->editable_object = etk_editable_add(evas);
   etk_editable_theme_set(entry->editable_object, etk_widget_theme_file_get(ETK_WIDGET(entry)),
      etk_widget_theme_group_get(ETK_WIDGET(entry)));
   etk_editable_text_set(entry->editable_object, entry->text);
   etk_editable_password_mode_set(entry->editable_object, entry->password_mode);
   if (!etk_widget_focused_get(ETK_WIDGET(entry)))
   {
      etk_editable_cursor_hide(entry->editable_object);
      etk_editable_selection_hide(entry->editable_object);
   }
   evas_object_show(entry->editable_object);
   etk_widget_swallow_object(ETK_WIDGET(entry), "etk.swallow.text", entry->editable_object);
}

/* Called when the entry is unrealized */
static void _etk_entry_unrealize_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;
   const char *text;

   if (!(entry = ETK_ENTRY(object)))
      return;
   
   free(entry->text);
   if ((text = etk_editable_text_get(entry->editable_object)))
      entry->text = strdup(text);
   else
      entry->text = NULL;
   
   evas_object_del(entry->editable_object);
   entry->editable_object = NULL;
}

/* Called when the user presses a key */
static void _etk_entry_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data)
{
   Etk_Entry *entry;
   Evas_Object *editable;
   int cursor_pos, selection_pos;
   int start_pos, end_pos;
   Etk_Bool selecting;
   Etk_Bool changed = ETK_FALSE;
   Etk_Bool selection_changed = ETK_FALSE;
   Etk_Bool stop_signal = ETK_TRUE;
   
   if (!(entry = ETK_ENTRY(object)))
     return;

   editable = entry->editable_object;
   cursor_pos = etk_editable_cursor_pos_get(editable);
   selection_pos = etk_editable_selection_pos_get(editable);
   start_pos = ETK_MIN(cursor_pos, selection_pos);
   end_pos = ETK_MAX(cursor_pos, selection_pos);
   selecting = (start_pos != end_pos);
   
   /* Move the cursor/selection to the left */
   if (strcmp(event->keyname, "Left") == 0)
   {
      if (event->modifiers & ETK_MODIFIER_SHIFT)
      {
         etk_editable_cursor_move_left(editable);
         selection_changed = ETK_TRUE;
      }
      else if (selecting)
      {
         if (cursor_pos < selection_pos)
            etk_editable_selection_pos_set(editable, cursor_pos);
         else
            etk_editable_cursor_pos_set(editable, selection_pos);
      }
      else
      {
         etk_editable_cursor_move_left(editable);
         etk_editable_selection_pos_set(editable, etk_editable_cursor_pos_get(editable));
      }
   }
   /* Move the cursor/selection to the right */
   else if (strcmp(event->keyname, "Right") == 0)
   {
      if (event->modifiers & ETK_MODIFIER_SHIFT)
      {
         etk_editable_cursor_move_right(editable);
         selection_changed = ETK_TRUE;
      }
      else if (selecting)
      {
         if (cursor_pos > selection_pos)
            etk_editable_selection_pos_set(editable, cursor_pos);
         else
            etk_editable_cursor_pos_set(editable, selection_pos);
      }
      else
      {
         etk_editable_cursor_move_right(editable);
         etk_editable_selection_pos_set(editable, etk_editable_cursor_pos_get(editable));
      }
   }
   /* Move the cursor/selection to the start of the entry */
   else if (strcmp(event->keyname, "Home") == 0)
   {
      etk_editable_cursor_move_to_start(editable);
      if (!(event->modifiers & ETK_MODIFIER_SHIFT))
         etk_editable_selection_pos_set(editable, etk_editable_cursor_pos_get(editable));
      else
         selection_changed = ETK_TRUE;
   }
   /* Move the cursor/selection to the end of the entry */
   else if (strcmp(event->keyname, "End") == 0)
   {
      etk_editable_cursor_move_to_end(editable);
      if (!(event->modifiers & ETK_MODIFIER_SHIFT))
         etk_editable_selection_pos_set(editable, etk_editable_cursor_pos_get(editable));
      else
         selection_changed = ETK_TRUE;
   }
   /* Delete the previous character */
   else if (strcmp(event->keyname, "BackSpace") == 0)
   {
      if (selecting)
         changed = etk_editable_delete(editable, start_pos, end_pos);
      else
         changed = etk_editable_delete(editable, cursor_pos - 1, cursor_pos);
   }
   /* Delete the next character */
   else if (strcmp(event->keyname, "Delete") == 0)
   {
      if (selecting)
         changed = etk_editable_delete(editable, start_pos, end_pos);
      else
         changed = etk_editable_delete(editable, cursor_pos, cursor_pos + 1);
   }
   /* Ctrl + A,C,X,V */
   else if (event->modifiers & ETK_MODIFIER_CTRL)
   {
      if (strcmp(event->keyname, "a") == 0)
      {
         etk_editable_select_all(editable);
         selection_changed = ETK_TRUE;
      }
      else if (strcmp(event->keyname, "x") == 0 || strcmp(event->keyname, "c") == 0)
         _etk_entry_selection_copy(entry, ETK_SELECTION_CLIPBOARD, (strcmp(event->keyname, "x") == 0));
      else if (strcmp(event->keyname, "v") == 0)
         etk_selection_text_request(ETK_SELECTION_CLIPBOARD, ETK_WIDGET(entry));
      else
         stop_signal = ETK_FALSE;
   }
   /* Otherwise, we insert the corresponding character */
   else if (event->string && *event->string && (strlen(event->string) != 1 || event->string[0] >= 0x20))
   {
      if (selecting)
         changed |= etk_editable_delete(editable, start_pos, end_pos);
      changed |= etk_editable_insert(editable, start_pos, event->string);
   }
   else
      stop_signal = ETK_FALSE;
   
   
   if (changed)
      etk_signal_emit(_etk_entry_signals[ETK_ENTRY_TEXT_CHANGED_SIGNAL], ETK_OBJECT(entry), NULL);
   if (selection_changed)
      _etk_entry_selection_copy(entry, ETK_SELECTION_PRIMARY, ETK_FALSE);
   if (stop_signal)
      etk_signal_stop();
}

/* Called when the mouse enters the entry */
static void _etk_entry_mouse_in_cb(Etk_Object *object, Etk_Event_Mouse_In *event, void *data)
{
   Etk_Widget *entry_widget;

   if (!(entry_widget = ETK_WIDGET(object)))
      return;
   etk_toplevel_pointer_push(etk_widget_toplevel_parent_get(entry_widget), ETK_POINTER_TEXT_EDIT);
}

/* Called when the mouse leaves the entry */
static void _etk_entry_mouse_out_cb(Etk_Object *object, Etk_Event_Mouse_Out *event, void *data)
{
   Etk_Widget *entry_widget;

   if (!(entry_widget = ETK_WIDGET(object)))
      return;
   etk_toplevel_pointer_pop(entry_widget->toplevel_parent, ETK_POINTER_TEXT_EDIT);
}

/* Called when the entry is pressed by the mouse */
static void _etk_entry_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Down *event, void *data)
{
   Etk_Entry *entry;
   Evas_Coord ox, oy;
   int pos;
   
   if (!(entry = ETK_ENTRY(object)))
      return;
   
   evas_object_geometry_get(entry->editable_object, &ox, &oy, NULL, NULL);
   pos = etk_editable_pos_get_from_coords(entry->editable_object, event->canvas.x - ox, event->canvas.y - oy);
   if (event->button == 1)
   {
      if (event->flags & ETK_MOUSE_DOUBLE_CLICK)
         etk_editable_select_all(entry->editable_object);
      else
      {
         etk_editable_cursor_pos_set(entry->editable_object, pos);
         if (!(event->modifiers & ETK_MODIFIER_SHIFT))
            etk_editable_selection_pos_set(entry->editable_object, pos);
         
         entry->selection_dragging = ETK_TRUE;
      }
   }
   else if (event->button == 2)
   {
      etk_editable_cursor_pos_set(entry->editable_object, pos);
      etk_editable_selection_pos_set(entry->editable_object, pos);
      
      etk_selection_text_request(ETK_SELECTION_PRIMARY, ETK_WIDGET(entry));
   }
}

/* Called when the entry is released by the mouse */
static void _etk_entry_mouse_up_cb(Etk_Object *object, Etk_Event_Mouse_Up *event, void *data)
{
   Etk_Entry *entry;
   
   if (!(entry = ETK_ENTRY(object)))
      return;
   entry->selection_dragging = ETK_FALSE;
   if (event->button == 1)
   {
      entry->selection_dragging = ETK_FALSE;
      _etk_entry_selection_copy(entry, ETK_SELECTION_PRIMARY, ETK_FALSE);
   }
}

/* Called when the mouse moves over the entry */
static void _etk_entry_mouse_move_cb(Etk_Object *object, Etk_Event_Mouse_Move *event, void *data)
{
   Etk_Entry *entry;
   Evas_Coord ox, oy;
   int pos;
   
   if (!(entry = ETK_ENTRY(object)))
      return;
   
   if (entry->selection_dragging)
   {
      evas_object_geometry_get(entry->editable_object, &ox, &oy, NULL, NULL);
      pos = etk_editable_pos_get_from_coords(entry->editable_object, event->cur.canvas.x - ox, event->cur.canvas.y - oy);
      if (pos >= 0)
         etk_editable_cursor_pos_set(entry->editable_object, pos);
   }
}

/* Called when the entry is focused */
static void _etk_entry_focus_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)) || !entry->editable_object)
      return;
   
   etk_editable_cursor_show(entry->editable_object);
   etk_editable_selection_show(entry->editable_object);
}

/* Called when the entry is unfocused */
static void _etk_entry_unfocus_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)) || !entry->editable_object)
      return;
   
   etk_editable_cursor_move_to_end(entry->editable_object);
   etk_editable_selection_move_to_end(entry->editable_object);
   etk_editable_cursor_hide(entry->editable_object);
   etk_editable_selection_hide(entry->editable_object);
}

/* Called when the selection/clipboard content is received */
static void _etk_entry_selection_received_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Entry *entry;
   Evas_Object *editable;
   Etk_Selection_Event *ev = event;
   const char *text;
   
   if (!(entry = ETK_ENTRY(object)) || !(editable = entry->editable_object))
      return;
   
   if (ev->type == ETK_SELECTION_TEXT && (text = ev->data.text) && *text && (strlen(text) != 1 || text[0] >= 0x20))
   {
      int cursor_pos, selection_pos;
      int start_pos, end_pos;
      Etk_Bool selecting;
      Etk_Bool changed = ETK_FALSE;
      
      cursor_pos = etk_editable_cursor_pos_get(editable);
      selection_pos = etk_editable_selection_pos_get(editable);
      start_pos = ETK_MIN(cursor_pos, selection_pos);
      end_pos = ETK_MAX(cursor_pos, selection_pos);
      selecting = (start_pos != end_pos);
      
      if (selecting)
         changed |= etk_editable_delete(editable, start_pos, end_pos);
      changed |= etk_editable_insert(editable, start_pos, text);
      
      if (changed)
         etk_signal_emit(_etk_entry_signals[ETK_ENTRY_TEXT_CHANGED_SIGNAL], ETK_OBJECT(entry), NULL);
   }
}

/**************************
 *
 * Private function
 *
 **************************/

/* Copies the selected text of the entry to the given selection */
static void _etk_entry_selection_copy(Etk_Entry *entry, Etk_Selection_Type selection, Etk_Bool cut)
{
   Evas_Object *editable;
   int cursor_pos, selection_pos;
   int start_pos, end_pos;
   Etk_Bool selecting;
   char *range;
   
   if (!entry)
     return;

   editable = entry->editable_object;
   cursor_pos = etk_editable_cursor_pos_get(editable);
   selection_pos = etk_editable_selection_pos_get(editable);
   start_pos = ETK_MIN(cursor_pos, selection_pos);
   end_pos = ETK_MAX(cursor_pos, selection_pos);
   selecting = (start_pos != end_pos);
   
   if (!selecting)
      return;
   
   range = etk_editable_text_range_get(editable, start_pos, end_pos);
   if (range)
   {
      etk_selection_text_set(selection, range);
      free(range);
      if (cut)
      {
         if (etk_editable_delete(editable, start_pos, end_pos))
            etk_signal_emit(_etk_entry_signals[ETK_ENTRY_TEXT_CHANGED_SIGNAL], ETK_OBJECT(entry), NULL);
      }
   }
}

/** @} */

/**
 * @addtogroup Etk_Entry
 *
 * @image html widgets/entry.png
 * You can add an empty entry with etk_entry_new(). @n
 * You can change the text of the entry with etk_entry_text_set() or etk_entry_clear(),
 * and get the text with etk_entry_text_get(). @n
 * An entry can work in two modes: the normal mode (the text is visible) and the password mode
 * (the text is replaced by '*'). To change the mode of the entry, use etk_entry_password_mode_set().
 * 
 * \par Object Hierarchy:
 * - Etk_Object
 *   - Etk_Widget
 *     - Etk_Entry
 *
 * \par Signals:
 * @signal_name "text_changed": Emitted when the text of the entry is changed
 * @signal_cb void callback(Etk_Entry *entry, void *data)
 * @signal_arg entry: the entry whose text has been changed
 * @signal_data
 *
 * \par Properties:
 * @prop_name "password_mode": The height of an item of the combobox (should be > 0)
 * @prop_type Boolean
 * @prop_rw
 * @prop_val ETK_FALSE
 */
