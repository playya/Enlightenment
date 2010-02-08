/*  Copyright (C) 2006-2008 Davide Andreoli (see AUTHORS)
 *
 *  This file is part of Edje_editor.
 *  Edje_editor is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Edje_editor is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Edje_editor.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <Etk.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include "main.h"



/***   Implementation   ***/
Etk_Widget*
part_frame_create(void)
{
   Etk_Widget *table, *table2;
   Etk_Widget *label;
   Etk_Widget *frame;
   Etk_Widget *hbox;

   //table
   table = etk_table_new(2, 5, ETK_TABLE_NOT_HOMOGENEOUS);

   //PartNameEntry
   label = etk_label_new("<b>Name</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 0, 0,ETK_TABLE_NONE,0,0);
   UI_PartNameEntry = etk_entry_new();
   UI_PartNameEntryImage = etk_image_new_from_stock(ETK_STOCK_DIALOG_OK,
                                                    ETK_STOCK_SMALL);
   etk_entry_image_set(ETK_ENTRY(UI_PartNameEntry), ETK_ENTRY_IMAGE_SECONDARY,
                       ETK_IMAGE(UI_PartNameEntryImage));
   etk_table_attach_default(ETK_TABLE(table),UI_PartNameEntry, 1, 1, 0, 0);

   //UI_CliptoComboBox
   label = etk_label_new("<b>Clip to</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 1, 1,ETK_TABLE_NONE,0,0);

   UI_CliptoComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_CliptoComboBox),
                           ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_CliptoComboBox),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_CliptoComboBox));
   etk_table_attach_default(ETK_TABLE(table), UI_CliptoComboBox, 1, 1, 1, 1);

   //UI_PartSourceComboBox
   UI_PartSourceLabel = etk_label_new("<b>Source</b>");
   etk_table_attach(ETK_TABLE(table), UI_PartSourceLabel,
                    0, 0, 2, 2, ETK_TABLE_NONE, 0, 0);

   UI_PartSourceComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartSourceComboBox),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_PartSourceComboBox));
   etk_table_attach_default(ETK_TABLE(table), UI_PartSourceComboBox, 1, 1, 2, 2);

   //events frame
   frame = etk_frame_new("Mouse events");
   etk_table_attach(ETK_TABLE(table), frame, 0, 1, 3, 3,
                     ETK_TABLE_EXPAND_FILL, 0, 0);
   //events hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_widget_padding_set(hbox, 10, 0, 0, 0);
   etk_container_add(ETK_CONTAINER(frame), hbox); 

   //PartEventsCheck
   UI_PartEventsCheck = etk_check_button_new_with_label("Accept");
   etk_box_append(ETK_BOX(hbox), UI_PartEventsCheck,
                  ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //PartEventRepeatCheck
   UI_PartEventsRepeatCheck = etk_check_button_new_with_label("Repeat");
   etk_box_append(ETK_BOX(hbox), UI_PartEventsRepeatCheck,
                  ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //drag frame
   frame = etk_frame_new("Dragable");
   etk_table_attach(ETK_TABLE(table), frame, 0, 1, 4, 4, 
                     ETK_TABLE_EXPAND_FILL, 0, 0);
   //drag table
   table2 = etk_table_new(5, 4, ETK_TABLE_NOT_HOMOGENEOUS);
   etk_container_add(ETK_CONTAINER(frame), table2);

   //PartDragXCheck
   UI_PartDragXCheck = etk_check_button_new_with_label("Horiz");
   etk_table_attach_default(ETK_TABLE(table2), UI_PartDragXCheck, 0, 0, 0, 0);
   
   //PartStepXSpinner
   label = etk_label_new("step");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   etk_table_attach_default(ETK_TABLE(table2), label, 1, 1, 0, 0);
   UI_PartStepXSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   etk_widget_size_request_set(UI_PartStepXSpinner, 35, 20);
   etk_table_attach_default(ETK_TABLE(table2), UI_PartStepXSpinner, 2, 2, 0, 0);

   //PartCountXSpinner
   label = etk_label_new("count");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   etk_table_attach_default(ETK_TABLE(table2), label, 3, 3, 0, 0);
   UI_PartCountXSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   etk_widget_size_request_set(UI_PartCountXSpinner, 35, 20);
   etk_table_attach_default(ETK_TABLE(table2), UI_PartCountXSpinner, 4, 4, 0, 0);
   
   //PartDragYCheck
   UI_PartDragYCheck = etk_check_button_new_with_label("Vert");
   etk_table_attach_default(ETK_TABLE(table2), UI_PartDragYCheck, 0, 0, 1, 1);

   //PartStepYSpinner
   label = etk_label_new("step");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   etk_table_attach_default(ETK_TABLE(table2), label, 1, 1, 1, 1);
   UI_PartStepYSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   etk_widget_size_request_set(UI_PartStepYSpinner, 35, 20);
   etk_table_attach_default(ETK_TABLE(table2), UI_PartStepYSpinner, 2, 2, 1, 1);

   //PartCountYSpinner
   label = etk_label_new("count");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   etk_table_attach_default(ETK_TABLE(table2), label, 3, 3, 1, 1);
   UI_PartCountYSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   etk_widget_size_request_set(UI_PartCountYSpinner, 35, 20);
   etk_table_attach_default(ETK_TABLE(table2), UI_PartCountYSpinner, 4, 4, 1, 1);

   //PartConfineCombo
   label = etk_label_new("Confine");
   etk_table_attach_default(ETK_TABLE(table2), label, 0, 0, 2, 2);
   
   UI_PartConfineCombo = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartConfineCombo),
                           ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartConfineCombo),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_PartConfineCombo));
   etk_table_attach_default(ETK_TABLE(table2), UI_PartConfineCombo, 1, 4, 2, 2);

   //PartEventCombo
   label = etk_label_new("Events");
   etk_table_attach_default(ETK_TABLE(table2), label, 0, 0, 3, 3);
   
   UI_PartEventCombo = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartEventCombo),
                           ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartEventCombo),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_PartEventCombo));
   etk_table_attach_default(ETK_TABLE(table2), UI_PartEventCombo, 1, 4, 3, 3);


   etk_signal_connect("text-changed", ETK_OBJECT(UI_PartNameEntry),
         ETK_CALLBACK(_group_NamesEntry_text_changed_cb), NULL);   
   etk_signal_connect("key-down", ETK_OBJECT(UI_PartNameEntry),
         ETK_CALLBACK(_part_NameEntry_key_down_cb), NULL);
   etk_signal_connect("mouse-click", ETK_OBJECT(UI_PartNameEntryImage),
                      ETK_CALLBACK(_part_NameEntryImage_clicked_cb), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsCheck),
                      ETK_CALLBACK(_part_EventsCheck_toggled_cb), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsRepeatCheck),
                      ETK_CALLBACK(_part_EventsRepeatCheck_toggled_cb), NULL);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                     ETK_CALLBACK(_part_CliptoComboBox_item_activated_cb), NULL);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                      ETK_CALLBACK(_part_SourceComboBox_item_activated_cb), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartDragXCheck),
                      ETK_CALLBACK(_part_DragXCheck_toggled_cb), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartDragYCheck),
                      ETK_CALLBACK(_part_DragYCheck_toggled_cb), NULL);
   
   etk_signal_connect("value-changed", ETK_OBJECT(UI_PartStepXSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)STEPX_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_PartStepYSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)STEPY_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_PartCountXSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)COUNTX_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_PartCountYSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)COUNTY_SPINNER);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_PartConfineCombo),
                    ETK_CALLBACK(_part_ConfineCombo_item_activated_cb), NULL);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_PartEventCombo),
                    ETK_CALLBACK(_part_EventCombo_item_activated_cb), NULL);
   return table;
}

void
part_frame_update(void)
{
   Etk_Combobox_Item *item = NULL;
   const char *clipto;
   int i, status;
   char *p;

   if (!etk_string_length_get(Cur.part))
      return;

   //Stop signal propagation
   etk_signal_block("text-changed",ETK_OBJECT(UI_PartNameEntry),
                    _group_NamesEntry_text_changed_cb, NULL);
   etk_signal_block("toggled",ETK_OBJECT(UI_PartEventsCheck),
                    _part_EventsCheck_toggled_cb, NULL);
   etk_signal_block("toggled",ETK_OBJECT(UI_PartEventsRepeatCheck),
                    _part_EventsRepeatCheck_toggled_cb, NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                    ETK_CALLBACK(_part_CliptoComboBox_item_activated_cb), NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                    ETK_CALLBACK(_part_SourceComboBox_item_activated_cb), NULL);
   etk_signal_block("toggled", ETK_OBJECT(UI_PartDragXCheck),
                    ETK_CALLBACK(_part_DragXCheck_toggled_cb), NULL);
   etk_signal_block("toggled", ETK_OBJECT(UI_PartDragYCheck),
                    ETK_CALLBACK(_part_DragYCheck_toggled_cb), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_PartStepXSpinner),
                    ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                    (void *)STEPX_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_PartStepYSpinner),
                    ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                    (void *)STEPY_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_PartCountXSpinner),
                    ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                    (void *)COUNTX_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_PartCountYSpinner),
                    ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                    (void *)COUNTY_SPINNER);
   etk_signal_block("item-activated", ETK_OBJECT(UI_PartConfineCombo),
                    ETK_CALLBACK(_part_ConfineCombo_item_activated_cb), NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_PartEventCombo),
                    ETK_CALLBACK(_part_EventCombo_item_activated_cb), NULL);
   //Set name
   etk_entry_text_set(ETK_ENTRY(UI_PartNameEntry), Cur.part->string);
   etk_widget_hide(ETK_WIDGET(UI_PartNameEntryImage));

   //Set mouse events
   etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartEventsCheck),
                  edje_edit_part_mouse_events_get(edje_o, Cur.part->string));
   etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartEventsRepeatCheck),
                  edje_edit_part_repeat_events_get(edje_o, Cur.part->string));


   /* Update clip_to combobox */
   clipto = edje_edit_part_clip_to_get(edje_o, Cur.part->string);

   if (clipto)
   {
      //Loop for all the item in the Combobox
      i=1;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_CliptoComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (!strcmp(p, clipto))
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_CliptoComboBox),item);
         i++;
      }
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_CliptoComboBox),
            etk_combobox_first_item_get(ETK_COMBOBOX(UI_CliptoComboBox)));
   edje_edit_string_free(clipto);


   /* Update PartSource combobox */
   const char *source;
   source = edje_edit_part_source_get(edje_o, Cur.part->string);

   if (source)
   {
      //Loop for all the item in the Combobox
      i=1;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartSourceComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 0);
         if (!strcmp(p, source))
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),item);
         i++;
      }
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),
            etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartSourceComboBox)));

   edje_edit_string_free(source);

   //Update dragables
   status = edje_edit_part_drag_x_get(edje_o, Cur.part->string) == 0 ? 0 : 1;
   etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartDragXCheck), status);
   etk_widget_disabled_set(UI_PartStepXSpinner, !status);
   etk_widget_disabled_set(UI_PartCountXSpinner, !status);

   status = edje_edit_part_drag_y_get(edje_o, Cur.part->string) == 0 ? 0 : 1;
   etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartDragYCheck), status);
   etk_widget_disabled_set(UI_PartStepYSpinner, !status);
   etk_widget_disabled_set(UI_PartCountYSpinner, !status);

   etk_range_value_set(ETK_RANGE(UI_PartStepXSpinner),
            (float)edje_edit_part_drag_step_x_get(edje_o, Cur.part->string));
   etk_range_value_set(ETK_RANGE(UI_PartStepYSpinner),
            (float)edje_edit_part_drag_step_y_get(edje_o, Cur.part->string));
   etk_range_value_set(ETK_RANGE(UI_PartCountXSpinner),
            (float)edje_edit_part_drag_count_x_get(edje_o, Cur.part->string));
   etk_range_value_set(ETK_RANGE(UI_PartCountYSpinner),
            (float)edje_edit_part_drag_count_y_get(edje_o, Cur.part->string));

   //Update drag confine combo
   source = edje_edit_part_drag_confine_get(edje_o, Cur.part->string);
   if (source)
   {
      //Loop for all the item in the Combobox
      i = 1;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartConfineCombo),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (!strcmp(p, source))
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartConfineCombo),item);
         i++;
      }
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartConfineCombo),
            etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartConfineCombo)));
   edje_edit_string_free(source);

   //Update drag events combo
   source = edje_edit_part_drag_event_get(edje_o, Cur.part->string);
   if (source)
   {
      //Loop for all the item in the Combobox
      i = 1;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartEventCombo),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (!strcmp(p, source))
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartEventCombo),item);
         i++;
      }
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartEventCombo),
            etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartEventCombo)));
   edje_edit_string_free(source);

   //Show/hide Sourcecombo for part EDJE_PART_TYPE_GROUP
   if (edje_edit_part_type_get(edje_o, Cur.part->string) == EDJE_PART_TYPE_GROUP)
   {
      etk_widget_show(UI_PartSourceComboBox);
      etk_widget_show(UI_PartSourceLabel);
   }
   else
   {
      etk_widget_hide(UI_PartSourceComboBox);
      etk_widget_hide(UI_PartSourceLabel);
   }


   //ReEnable Signal Propagation
   etk_signal_unblock("text-changed",ETK_OBJECT(UI_PartNameEntry),
                      _group_NamesEntry_text_changed_cb, NULL);
   etk_signal_unblock("toggled",ETK_OBJECT(UI_PartEventsCheck),
                      _part_EventsCheck_toggled_cb, NULL);
   etk_signal_unblock("toggled",ETK_OBJECT(UI_PartEventsRepeatCheck),
                      _part_EventsRepeatCheck_toggled_cb, NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                      ETK_CALLBACK(_part_CliptoComboBox_item_activated_cb), NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                      ETK_CALLBACK(_part_SourceComboBox_item_activated_cb), NULL);
   etk_signal_unblock("toggled", ETK_OBJECT(UI_PartDragXCheck),
                      ETK_CALLBACK(_part_DragXCheck_toggled_cb), NULL);
   etk_signal_unblock("toggled", ETK_OBJECT(UI_PartDragYCheck),
                      ETK_CALLBACK(_part_DragYCheck_toggled_cb), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_PartStepXSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)STEPX_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_PartStepYSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)STEPY_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_PartCountXSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)COUNTX_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_PartCountYSpinner),
                      ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      (void *)COUNTY_SPINNER);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_PartConfineCombo),
                      ETK_CALLBACK(_part_ConfineCombo_item_activated_cb), NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_PartEventCombo),
                      ETK_CALLBACK(_part_EventCombo_item_activated_cb), NULL);
}

char*
part_type_image_get(const char *part)
{
   /* Get the name of the group in edje_editor.edj that
    * correspond to the given EDJE_PART_TYPE.
    * Remember to free the returned string.
    */
   static char buf[20];
   
   switch (edje_edit_part_type_get(edje_o, part))
   {
      case EDJE_PART_TYPE_IMAGE:     strcpy(buf, "IMAGE.PNG"); break;
      case EDJE_PART_TYPE_GRADIENT:  strcpy(buf, "GRAD_LINEAR.PNG"); break;
      case EDJE_PART_TYPE_TEXT:      strcpy(buf, "TEXT.PNG"); break;
      case EDJE_PART_TYPE_RECTANGLE: strcpy(buf, "RECT.PNG"); break;
      case EDJE_PART_TYPE_SWALLOW:   strcpy(buf, "SWAL.PNG"); break;
      case EDJE_PART_TYPE_GROUP:     strcpy(buf, "GROUP.PNG"); break;
      default:                       strcpy(buf, "NONE.PNG"); break;
   }
   
   return strdup(buf);
}
/***   Callbacks   ***/
Etk_Bool
_part_EventsCheck_toggled_cb(Etk_Object *object, void *data)
{
   printf("Toggled Signal on EventsCheck EMITTED\n");
   if (etk_string_length_get(Cur.part))
   {
      edje_edit_part_mouse_events_set(edje_o, Cur.part->string,
                     etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object)));
   }
   return ETK_TRUE;
}

Etk_Bool
_part_EventsRepeatCheck_toggled_cb(Etk_Object *object, void *data)
{
   printf("Toggled Signal on EventsRepeatCheck EMITTED\n");
   if (etk_string_length_get(Cur.part))
   {
      edje_edit_part_repeat_events_set(edje_o, Cur.part->string,
                     etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object)));
   }
   return ETK_TRUE;
}

Etk_Bool
_part_CliptoComboBox_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   char *to;
   printf("Item Activated Signal on CliptoCombobox EMITTED\n");
   
   to = etk_combobox_item_field_get(item, 1);
   if (strcmp(to, "None"))
      edje_edit_part_clip_to_set(edje_o, Cur.part->string, to);
   else
      edje_edit_part_clip_to_set(edje_o, Cur.part->string, NULL);

   return ETK_TRUE;
}

Etk_Bool
_part_SourceComboBox_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   char *gr;
   printf("Item Activated Signal on PartSourceCombobox EMITTED\n");

   gr = etk_combobox_item_field_get(item,0);

   if (!strcmp(gr, Cur.group->string))
   {
      dialog_alert_show("A group can't contain itself");
      return ETK_TRUE;
   }

   if (strcmp(gr, "None"))
      edje_edit_part_source_set(edje_o, Cur.part->string, gr);
   else
      edje_edit_part_source_set(edje_o, Cur.part->string, NULL);

   reload_edje();
   return ETK_TRUE;
}

Etk_Bool
_part_NameEntry_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data)
{
   printf("PRESSED %s\n", event->keyname);

   if (!strcmp(event->keyname, "Return"))
      _part_NameEntryImage_clicked_cb(
                                 ETK_OBJECT(ETK_ENTRY(object)->secondary_image),
                                 NULL);
   return ETK_TRUE;
}

Etk_Bool
_part_NameEntryImage_clicked_cb(Etk_Object *object, void *data)
{
   const char *name;
   char *image_name;
   Etk_Tree_Row *row;
   Etk_Tree_Row *child;

   printf("Mouse Click Signal on PartNameEntryImage Emitted\n");

   name = etk_entry_text_get(ETK_ENTRY(UI_PartNameEntry));

   if (!name || !etk_string_length_get(Cur.part)) return ETK_TRUE;

   if (!strcmp(name, Cur.part->string))
   {
      etk_widget_hide(ETK_WIDGET(UI_PartNameEntryImage));
      return ETK_TRUE;
   }

   /* change the name in edje */
   if (!edje_edit_part_name_set(edje_o, Cur.part->string, name))
   {
      dialog_alert_show("Can't set part name.<br>Another name with this name exist? ");
      return ETK_TRUE;
   }

   /* Set new Current name */
   Cur.part = etk_string_set(Cur.part, name);

   //Update PartTree
   row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
   image_name = part_type_image_get(Cur.part->string);
   etk_tree_row_fields_set(row,TRUE,
                           COL_NAME, EdjeFile, image_name, name, NULL);
   free(image_name);


   /* Update hidden colon on every child */
   child = etk_tree_row_first_child_get(row);
   etk_tree_row_fields_set(child, TRUE, COL_PARENT, name, NULL);
   while ((child = etk_tree_row_next_get(child)))
      etk_tree_row_fields_set(child, TRUE, COL_PARENT, name, NULL);

   /* Update Parts_Hash */
   eina_hash_del(Parts_Hash, Cur.part->string, NULL);
   if (!Parts_Hash) Parts_Hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(Parts_Hash, name, row);

   /* Recreate rel combobox */
   position_comboboxes_populate();
   program_source_combo_populate();

   /* Hide the image */
   etk_widget_hide(ETK_WIDGET(UI_PartNameEntryImage));

   return ETK_TRUE;
}

Etk_Bool
_part_DragXCheck_toggled_cb(Etk_Object *object, void *data)
{
   int status;

   if (!etk_string_length_get(Cur.part))
      return ETK_TRUE;

   //printf("Toggled Signal on DragX checkbox EMITTED\n");
   status = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object));
   edje_edit_part_drag_x_set(edje_o, Cur.part->string, status);

   etk_widget_disabled_set(UI_PartStepXSpinner, !status);
   etk_widget_disabled_set(UI_PartCountXSpinner, !status);

   return ETK_TRUE;
}

Etk_Bool
_part_DragYCheck_toggled_cb(Etk_Object *object, void *data)
{
   int status;

   if (!etk_string_length_get(Cur.part))
      return ETK_TRUE;

   //printf("Toggled Signal on DragY checkbox EMITTED\n");
   status = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object));
   edje_edit_part_drag_y_set(edje_o, Cur.part->string, status);

   etk_widget_disabled_set(UI_PartStepYSpinner, !status);
   etk_widget_disabled_set(UI_PartCountYSpinner, !status);

   return ETK_TRUE;
}

Etk_Bool
_part_drag_spinners_value_changed_cb(Etk_Range *range, double value, void *data)
{
   printf("Part Drag Spinners value changed signal EMIT\n");
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   switch ((int)(long)data)
   {
      case STEPX_SPINNER:
         edje_edit_part_drag_step_x_set(edje_o, Cur.part->string,
                                        (int)etk_range_value_get(range));
         break;
      case STEPY_SPINNER:
         edje_edit_part_drag_step_y_set(edje_o, Cur.part->string,
                                        (int)etk_range_value_get(range));
         break;
      case COUNTX_SPINNER:
         edje_edit_part_drag_count_x_set(edje_o, Cur.part->string,
                                         (int)etk_range_value_get(range));
         break;
      case COUNTY_SPINNER:
         edje_edit_part_drag_count_y_set(edje_o, Cur.part->string,
                                         (int)etk_range_value_get(range));
         break;
      default:
         break;
   }
   return ETK_TRUE;
}

Etk_Bool
_part_ConfineCombo_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   char *confine;
   printf("Item Activated Signal on DragConfineCombo EMITTED\n");
   
   confine = etk_combobox_item_field_get(item, 1);
   if (strcmp(confine, "None"))
      edje_edit_part_drag_confine_set(edje_o, Cur.part->string, confine);
   else
      edje_edit_part_drag_confine_set(edje_o, Cur.part->string, NULL);

   return ETK_TRUE;
}

Etk_Bool
_part_EventCombo_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   char *event;
   printf("Item Activated Signal on DragEventCombo EMITTED\n");
   
   event = etk_combobox_item_field_get(item, 1);
   if (strcmp(event, "None"))
      edje_edit_part_drag_event_set(edje_o, Cur.part->string, event);
   else
      edje_edit_part_drag_event_set(edje_o, Cur.part->string, NULL);

   return ETK_TRUE;
}
