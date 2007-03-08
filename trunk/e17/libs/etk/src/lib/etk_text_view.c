/** @file etk_text_view.c */
#include "etk_text_view.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "etk_textblock.h"
#include "etk_event.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_utils.h"
#include "etk_selection.h"
#include "etk_string.h"

/**
 * @addtogroup Etk_Text_View
 * @{
 */

enum Etk_Text_View_Signal_Id
{
   ETK_TEXT_VIEW_TEXT_CHANGED_SIGNAL,
   ETK_TEXT_VIEW_NUM_SIGNALS
};

static void _etk_text_view_constructor(Etk_Text_View *text_view);
static void _etk_text_view_destructor(Etk_Text_View *text_view);
static void _etk_text_view_size_allocate(Etk_Widget *widget, Etk_Geometry geometry);
static void _etk_text_view_realize_cb(Etk_Object *object, void *data);
static void _etk_text_view_unrealize_cb(Etk_Object *object, void *data);
static void _etk_text_view_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data);
static void _etk_text_view_mouse_up_cb(Etk_Object *object, Etk_Event_Mouse_Up *event, void *data);
static void _etk_text_view_scroll_size_get(Etk_Widget *widget, Etk_Size scrollview_size, Etk_Size scrollbar_size, Etk_Size *scroll_size);
static void _etk_text_view_scroll(Etk_Widget *widget, int x, int y);
static void _etk_text_view_selection_copy(Etk_Text_View *tv, Etk_Selection_Type selection, Etk_Bool cut);


static Etk_Signal *_etk_text_view_signals[ETK_TEXT_VIEW_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @internal
 * @brief Gets the type of an Etk_Text_View
 * @return Returns the type of an Etk_Text_View
 */
Etk_Type *etk_text_view_type_get()
{
   static Etk_Type *text_view_type = NULL;

   if (!text_view_type)
   {
      text_view_type = etk_type_new("Etk_Text_View", ETK_WIDGET_TYPE, sizeof(Etk_Text_View),
         ETK_CONSTRUCTOR(_etk_text_view_constructor), ETK_DESTRUCTOR(_etk_text_view_destructor));

      _etk_text_view_signals[ETK_TEXT_VIEW_TEXT_CHANGED_SIGNAL] = etk_signal_new("text_changed",
         text_view_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
   }

   return text_view_type;
}

/**
 * @brief Creates a new text view
 * @return Returns the new text view widget
 */
Etk_Widget *etk_text_view_new()
{
   return etk_widget_new(ETK_TEXT_VIEW_TYPE, "theme_group", "text_view", "focusable", ETK_TRUE,
      "focus_on_click", ETK_TRUE, NULL);
}

/**
 * @brief Gets the textblock of the text view
 * @param text_view a text view
 * @return Returns the textblock of the textview
 */
Etk_Textblock *etk_text_view_textblock_get(Etk_Text_View *text_view)
{
   if (!text_view)
      return NULL;
   return text_view->textblock;
}

/**
 * @brief Gets the cursor's iterator of the text view
 * @param text_view a text view
 * @param Returns the cursor's iterator of the text view
 * @warning The text view has to be realized, otherwise it returns NULL
 */
Etk_Textblock_Iter *etk_text_view_cursor_get(Etk_Text_View *text_view)
{
   if (!text_view || !text_view->textblock_object)
      return NULL;
   return etk_textblock_object_cursor_get(text_view->textblock_object);
}

/**
 * @brief Gets the selection bound's iterator of the text view
 * @param text_view a text view
 * @param Returns the selection bound's iterator of the text view
 * @warning The text view has to be realized, otherwise it returns NULL
 */
Etk_Textblock_Iter *etk_text_view_selection_bound_get(Etk_Text_View *text_view)
{
   if (!text_view || !text_view->textblock_object)
      return NULL;
   return etk_textblock_object_selection_bound_get(text_view->textblock_object);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the default values of the text view */
static void _etk_text_view_constructor(Etk_Text_View *text_view)
{
   if (!text_view)
      return;

   text_view->textblock = etk_textblock_new();
   text_view->textblock_object = NULL;
   
   ETK_WIDGET(text_view)->size_allocate = _etk_text_view_size_allocate;

   ETK_WIDGET(text_view)->scroll = _etk_text_view_scroll;
   ETK_WIDGET(text_view)->scroll_size_get = _etk_text_view_scroll_size_get;

   etk_signal_connect("realize", ETK_OBJECT(text_view), ETK_CALLBACK(_etk_text_view_realize_cb), NULL);
   etk_signal_connect("unrealize", ETK_OBJECT(text_view), ETK_CALLBACK(_etk_text_view_unrealize_cb), NULL);
   etk_signal_connect("key_down", ETK_OBJECT(text_view), ETK_CALLBACK(_etk_text_view_key_down_cb), NULL);
   etk_signal_connect("mouse_up", ETK_OBJECT(text_view), ETK_CALLBACK(_etk_text_view_mouse_up_cb), NULL);
}

/* Destroys the text view */
static void _etk_text_view_destructor(Etk_Text_View *text_view)
{
   if (!text_view)
      return;
   etk_object_destroy(ETK_OBJECT(text_view->textblock));
}

/* TODO: size_allocate doc, swallow?? */
static void _etk_text_view_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   Etk_Text_View *text_view;
   
   if (!(text_view = ETK_TEXT_VIEW(widget)))
      return;
   
   evas_object_move(text_view->textblock_object, geometry.x, geometry.y);
   evas_object_resize(text_view->textblock_object, geometry.w, geometry.h);
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the text view is realized */
static void _etk_text_view_realize_cb(Etk_Object *object, void *data)
{
   Etk_Text_View *text_view;
   Evas *evas;

   if (!(text_view = ETK_TEXT_VIEW(object)) || !(evas = etk_widget_toplevel_evas_get(ETK_WIDGET(text_view))))
      return;

   text_view->textblock_object = etk_textblock_object_add(text_view->textblock, evas);
   etk_widget_member_object_add(ETK_WIDGET(text_view), text_view->textblock_object);
   evas_object_repeat_events_set(text_view->textblock_object, 1);
   evas_object_show(text_view->textblock_object);
}

/* Called when the text view is unrealized */
static void _etk_text_view_unrealize_cb(Etk_Object *object, void *data)
{
   Etk_Text_View *text_view;

   if (!(text_view = ETK_TEXT_VIEW(object)))
      return;
   
   if (text_view->textblock_object)
   {
      etk_widget_member_object_del(ETK_WIDGET(text_view), text_view->textblock_object);
      evas_object_del(text_view->textblock_object);
      text_view->textblock_object = NULL;
   }
}

/* Called when a key is pressed */
static void _etk_text_view_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data)
{
   Etk_Text_View *text_view;
   Etk_Textblock *tb;
   Etk_Textblock_Iter *cursor;
   Etk_Textblock_Iter *selection;
   int compare_res;
   Etk_Bool selecting;
   
   if (!(text_view = ETK_TEXT_VIEW(object)) || !text_view->textblock_object)
      return;
   
   tb = text_view->textblock;
   cursor = etk_textblock_object_cursor_get(text_view->textblock_object);
   selection = etk_textblock_object_selection_bound_get(text_view->textblock_object);
   compare_res = etk_textblock_iter_compare(cursor, selection);
   selecting = (compare_res != 0);
   
   if (strcmp(event->keyname, "Left") == 0)
   {
      if (event->modifiers & ETK_MODIFIER_SHIFT)
         etk_textblock_iter_backward_char(cursor);
      else if (selecting)
      {
         if (compare_res < 0)
            etk_textblock_iter_copy(selection, cursor);
         else
            etk_textblock_iter_copy(cursor, selection);
      }
      else
      {
         etk_textblock_iter_backward_char(cursor);
         etk_textblock_iter_copy(selection, cursor);
      }
   }
   else if (strcmp(event->keyname, "Right") == 0)
   {
      if (event->modifiers & ETK_MODIFIER_SHIFT)
         etk_textblock_iter_forward_char(cursor);
      else if (selecting)
      {
         if (compare_res < 0)
            etk_textblock_iter_copy(cursor, selection);
         else
            etk_textblock_iter_copy(selection, cursor);
      }
      else
      {
         etk_textblock_iter_forward_char(cursor);
         etk_textblock_iter_copy(selection, cursor);
      }
   }
   else if (strcmp(event->keyname, "BackSpace") == 0)
   {
      if (selecting)
         etk_textblock_delete_range(tb, cursor, selection);
      else
         etk_textblock_delete_before(tb, cursor);
   }
   else if (strcmp(event->keyname, "Delete") == 0)
   {
      if (selecting)
         etk_textblock_delete_range(tb, cursor, selection);
      else
         etk_textblock_delete_after(tb, cursor);
   }
   else if (strcmp(event->keyname, "Return") == 0 || strcmp(event->keyname, "KP_Enter") == 0)
   {
      if (selecting)
         etk_textblock_delete_range(tb, cursor, selection);
      etk_textblock_insert(tb, cursor, "\n", -1);
   }
   else if (event->string && !(strlen(event->string) == 1 && event->string[0] < 0x20))
   {
      if (selecting)
         etk_textblock_delete_range(tb, cursor, selection);
      etk_textblock_insert(tb, cursor, event->string, -1);
   }
}

static void _etk_text_view_mouse_up_cb(Etk_Object *object, Etk_Event_Mouse_Up *event, void *data)
{
   if (event->button == 1)
   {
      _etk_text_view_selection_copy(ETK_TEXT_VIEW(object), ETK_SELECTION_PRIMARY, ETK_FALSE);
   }
}

/* Size of all the text_view for scrolling ability. */
static void _etk_text_view_scroll_size_get(Etk_Widget *widget, Etk_Size scrollview_size, Etk_Size scrollbar_size, Etk_Size *scroll_size)
{
   Etk_Text_View *text_view;
   
   if(!(text_view = ETK_TEXT_VIEW(widget)) || !scroll_size )
      return;

   etk_textblock_object_full_geometry_get( text_view->textblock_object, NULL, NULL, &(scroll_size->w), &(scroll_size->h) );
}

static void _etk_text_view_scroll(Etk_Widget *widget, int x, int y)
{
   Etk_Text_View *text_view;

   if( !( text_view = ETK_TEXT_VIEW(widget) ) )
      return;

   etk_textblock_object_xoffset_set( text_view->textblock_object, x );
   etk_textblock_object_yoffset_set( text_view->textblock_object, y );

   etk_widget_redraw_queue( widget );
}

/* Copies the selected text of the entry to the given selection */
static void _etk_text_view_selection_copy(Etk_Text_View *tv, Etk_Selection_Type selection, Etk_Bool cut)
{
   Etk_Textblock_Iter *cursor_pos, *selection_pos;
   Etk_Bool selecting;
   Etk_String *range;

   cursor_pos = etk_textblock_object_cursor_get(tv->textblock_object);
   selection_pos = etk_textblock_object_selection_bound_get(tv->textblock_object);
   selecting = etk_textblock_iter_compare(cursor_pos, selection_pos);

   if (!selecting)
      return;

   range = etk_textblock_range_text_get(tv->textblock, cursor_pos, selection_pos, ETK_FALSE);
   if (range)
   {
      etk_selection_text_set(selection, etk_string_get(range));
      etk_object_destroy(ETK_OBJECT(range));
      if (cut)
      {
         etk_textblock_delete_range(tv->textblock, cursor_pos, selection_pos);
         etk_signal_emit(_etk_text_view_signals[ETK_TEXT_VIEW_TEXT_CHANGED_SIGNAL], ETK_OBJECT(tv), NULL);
      }
   }
}
/** @} */
