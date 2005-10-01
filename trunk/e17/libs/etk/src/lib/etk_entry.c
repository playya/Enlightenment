/** @file etk_entry.c */
#include "etk_entry.h"
#include <stdlib.h>
#include <string.h>
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_editable_text_object.h"

/**
 * @addtogroup Etk_Entry
* @{
 */

static void _etk_entry_constructor(Etk_Entry *entry);
static void _etk_entry_realize_cb(Etk_Object *object, void *data);
static void _etk_entry_unrealize_cb(Etk_Object *object, void *data);
static void _etk_entry_key_down_cb(Etk_Object *object, void *event, void *data);
static void _etk_entry_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Up_Down *event, void *data);
static void _etk_entry_focus_cb(Etk_Object *object, void *data);
static void _etk_entry_unfocus_cb(Etk_Object *object, void *data);

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Entry
 * @return Returns the type on an Etk_Entry
 */
Etk_Type *etk_entry_type_get()
{
   static Etk_Type *entry_type = NULL;

   if (!entry_type)
   {
      entry_type = etk_type_new("Etk_Entry", ETK_WIDGET_TYPE, sizeof(Etk_Entry), ETK_CONSTRUCTOR(_etk_entry_constructor), NULL, NULL);
   }

   return entry_type;
}

/**
 * @brief Creates a new entry
 * @return Returns the new entry widget
 */
Etk_Widget *etk_entry_new()
{
   return etk_widget_new(ETK_ENTRY_TYPE, "theme_group", "entry", "focusable", TRUE, NULL);
}

/**
 * @brief Gets the text of an entry
 * @param entry an entry
 * @return Returns the text of the entry
 */
const char *etk_entry_text_get(Etk_Entry *entry)
{
   if (!entry || !entry->editable_object)
      return NULL;

   return etk_editable_text_object_text_get(entry->editable_object);
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

   etk_signal_connect_after("realize", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_realize_cb), NULL);
   etk_signal_connect("unrealize", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_unrealize_cb), NULL);
   etk_signal_connect("key_down", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_key_down_cb), NULL);
   etk_signal_connect("mouse_down", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_mouse_down_cb), NULL);
   etk_signal_connect("focus", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_focus_cb), NULL);
   etk_signal_connect("unfocus", ETK_OBJECT(entry), ETK_CALLBACK(_etk_entry_unfocus_cb), NULL);
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
   Etk_Widget *entry_widget;
   Evas *evas;

   if (!(entry_widget = ETK_WIDGET(object)) || !(evas = etk_widget_toplevel_evas_get(entry_widget)))
      return;

   entry = ETK_ENTRY(entry_widget);
   entry->editable_object = etk_editable_text_object_add(evas);
   etk_widget_theme_object_swallow(entry_widget, "text_area", entry->editable_object);
   etk_widget_member_object_add(entry_widget, entry->editable_object);
}

/* Called when the entry is unrealized */
static void _etk_entry_unrealize_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)))
      return;

   entry->editable_object = NULL;
}

/* Called when the user presses a key */
static void _etk_entry_key_down_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Key_Up_Down *key_event = event;
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)) || !entry->editable_object)
      return;

   /* printf("%s\n", key_event->key); */
   if (strcmp(key_event->key, "BackSpace") == 0)
      etk_editable_text_object_delete_char_before(entry->editable_object);
   else if (strcmp(key_event->key, "Delete") == 0)
      etk_editable_text_object_delete_char_after(entry->editable_object);
   else if (strcmp(key_event->key, "Left") == 0)
      etk_editable_text_object_cursor_move_left(entry->editable_object);
   else if (strcmp(key_event->key, "Right") == 0)
      etk_editable_text_object_cursor_move_right(entry->editable_object);
   else if (strcmp(key_event->key, "Home") == 0)
      etk_editable_text_object_cursor_move_at_start(entry->editable_object);
   else if (strcmp(key_event->key, "End") == 0)
      etk_editable_text_object_cursor_move_at_end(entry->editable_object);
   else
      etk_editable_text_object_insert(entry->editable_object, key_event->string);
}


/* Called when the user presses the entry with the mouse */
/* TODO: move the cursor under the mouse pointer */
static void _etk_entry_mouse_down_cb(Etk_Object *object, Etk_Event_Mouse_Up_Down *event, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)))
      return;

   etk_widget_focus(ETK_WIDGET(entry));
}

/* Called when the entry is focused */
static void _etk_entry_focus_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)) || !entry->editable_object)
      return;

   etk_editable_text_object_cursor_show(entry->editable_object);   
}

/* Called when the entry is unfocused */
static void _etk_entry_unfocus_cb(Etk_Object *object, void *data)
{
   Etk_Entry *entry;

   if (!(entry = ETK_ENTRY(object)) || !entry->editable_object)
      return;

   etk_editable_text_object_cursor_hide(entry->editable_object);
   etk_editable_text_object_cursor_move_at_start(entry->editable_object);
}

/** @} */
