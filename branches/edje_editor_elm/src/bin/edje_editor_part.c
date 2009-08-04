/*  Copyright (C) 2006-2009 Davide Andreoli (see AUTHORS)
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

#include "main.h"

static Evas_Object *_name_entry;
static Evas_Object *_clipto_combo;
static Evas_Object *_events_combo;


static void
_entry_apply(Evas_Object *o)
{
   char *txt;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, probably a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   printf("Apply entry [%s]\n", txt);

   /* Apply Part Name */
   if (o == _name_entry)
   {
      if (!txt || !cur.part || ecore_str_equal(txt, cur.part))
        return;

      // change the name in edje
      if (!edje_edit_part_name_set(ui.edje_o, cur.part, txt))
      {
         dialog_alert_show("Can't set part name.<br>Another name with this name exist? ");
         return;
      }

      // Set new Current name
      set_current_part(txt);

      // Update Parts Tree
      Elm_Genlist_Item *it;
      const char *old_data;
      it = elm_genlist_selected_item_get(ui.parts_tree);
      old_data = elm_genlist_item_data_get(it);
      elm_genlist_item_data_set(it, (void*)eina_stringshare_add(txt));
      eina_stringshare_del(old_data);
   }

   part_frame_update();
}

static void
_entry_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   
   //~ printf("KEY DOWN %s\n", ev->key);
   if (ecore_str_equal(ev->key, "Return"))
   {
      _entry_apply(obj);
   }
   else if(ecore_str_equal(ev->key, "Escape"))
   {
      part_frame_update();
   }
}

static void
_events_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   switch ((int)(long)data)
   {
      case EVENTS_YES:
         printf("YES\n");
         edje_edit_part_mouse_events_set(ui.edje_o, cur.part, EINA_TRUE);
         edje_edit_part_repeat_events_set(ui.edje_o, cur.part, EINA_FALSE);
         break;
      case EVENTS_YES_REPEAT:
         printf("YES++\n");
         edje_edit_part_mouse_events_set(ui.edje_o, cur.part, EINA_TRUE);
         edje_edit_part_repeat_events_set(ui.edje_o, cur.part, EINA_TRUE);
         break;
      case EVENTS_NO: default:
         printf("NO\n");
         edje_edit_part_mouse_events_set(ui.edje_o, cur.part, EINA_FALSE);
         edje_edit_part_repeat_events_set(ui.edje_o, cur.part, EINA_FALSE);
         break;
   }
   part_frame_update();
}

static void
_clipto_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Hoversel_Item *it = event_info;
   const char *to;
   
   printf("### %s\n",elm_hoversel_item_label_get(it));
   
   to = elm_hoversel_item_label_get(it);
   if (ecore_str_equal(to, "unset"))
      edje_edit_part_clip_to_set(ui.edje_o, cur.part, NULL);
   else
      edje_edit_part_clip_to_set(ui.edje_o, cur.part, to);

   part_frame_update();
}

/***   Implementation   ***/
Evas_Object*
part_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *_o;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("name:", 0, 0, _name_entry, EINA_TRUE)
   NEW_COMBO_TO_TABLE(_clipto_combo, "clip to:", 0, 1, 1, part_populate_combo_with_parts, _clipto_combo_sel)
   
   NEW_COMBO_TO_TABLE( _events_combo, "mouse events:", 0, 2, 1, NULL, NULL)
   elm_hoversel_item_add(_events_combo, "no", NULL, ELM_ICON_NONE,
                         _events_combo_sel, (void*)EVENTS_NO);
   elm_hoversel_item_add(_events_combo, "yes", NULL, ELM_ICON_NONE,
                         _events_combo_sel, (void*)EVENTS_YES);
   elm_hoversel_item_add(_events_combo, "yes + repeat", NULL, ELM_ICON_NONE,
                         _events_combo_sel, (void*)EVENTS_YES_REPEAT);
   
   return tb;
   //~ //UI_PartSourceComboBox
   //~ UI_PartSourceLabel = etk_label_new("<b>Source</b>");
   //~ etk_table_attach(ETK_TABLE(table), UI_PartSourceLabel,
                    //~ 0, 0, 2, 2, ETK_TABLE_NONE, 0, 0);
//~ 
   //~ UI_PartSourceComboBox = etk_combobox_new();
   //~ etk_combobox_column_add(ETK_COMBOBOX(UI_PartSourceComboBox),
                           //~ ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   //~ etk_combobox_build(ETK_COMBOBOX(UI_PartSourceComboBox));
   //~ etk_table_attach_default(ETK_TABLE(table), UI_PartSourceComboBox, 1, 1, 2, 2);

   //~ //drag frame
   //~ frame = etk_frame_new("Dragable");
   //~ etk_table_attach(ETK_TABLE(table), frame, 0, 1, 4, 4, 
                     //~ ETK_TABLE_EXPAND_FILL, 0, 0);
   //~ //drag table
   //~ table2 = etk_table_new(5, 4, ETK_TABLE_NOT_HOMOGENEOUS);
   //~ etk_container_add(ETK_CONTAINER(frame), table2);
//~ 
   //~ //PartDragXCheck
   //~ UI_PartDragXCheck = etk_check_button_new_with_label("Horiz");
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartDragXCheck, 0, 0, 0, 0);
   //~ 
   //~ //PartStepXSpinner
   //~ label = etk_label_new("step");
   //~ etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 1, 1, 0, 0);
   //~ UI_PartStepXSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   //~ etk_widget_size_request_set(UI_PartStepXSpinner, 35, 20);
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartStepXSpinner, 2, 2, 0, 0);
//~ 
   //~ //PartCountXSpinner
   //~ label = etk_label_new("count");
   //~ etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 3, 3, 0, 0);
   //~ UI_PartCountXSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   //~ etk_widget_size_request_set(UI_PartCountXSpinner, 35, 20);
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartCountXSpinner, 4, 4, 0, 0);
   //~ 
   //~ //PartDragYCheck
   //~ UI_PartDragYCheck = etk_check_button_new_with_label("Vert");
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartDragYCheck, 0, 0, 1, 1);
//~ 
   //~ //PartStepYSpinner
   //~ label = etk_label_new("step");
   //~ etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 1, 1, 1, 1);
   //~ UI_PartStepYSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   //~ etk_widget_size_request_set(UI_PartStepYSpinner, 35, 20);
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartStepYSpinner, 2, 2, 1, 1);
//~ 
   //~ //PartCountYSpinner
   //~ label = etk_label_new("count");
   //~ etk_object_properties_set(ETK_OBJECT(label), "xalign", 1.0, NULL);
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 3, 3, 1, 1);
   //~ UI_PartCountYSpinner = etk_spinner_new(0, 999, 0, 1, 10);
   //~ etk_widget_size_request_set(UI_PartCountYSpinner, 35, 20);
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartCountYSpinner, 4, 4, 1, 1);
//~ 
   //~ //PartConfineCombo
   //~ label = etk_label_new("Confine");
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 0, 0, 2, 2);
   //~ 
   //~ UI_PartConfineCombo = etk_combobox_new();
   //~ etk_combobox_column_add(ETK_COMBOBOX(UI_PartConfineCombo),
                           //~ ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   //~ etk_combobox_column_add(ETK_COMBOBOX(UI_PartConfineCombo),
                           //~ ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   //~ etk_combobox_build(ETK_COMBOBOX(UI_PartConfineCombo));
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartConfineCombo, 1, 4, 2, 2);
//~ 
   //~ //PartEventCombo
   //~ label = etk_label_new("Events");
   //~ etk_table_attach_default(ETK_TABLE(table2), label, 0, 0, 3, 3);
   //~ 
   //~ UI_PartEventCombo = etk_combobox_new();
   //~ etk_combobox_column_add(ETK_COMBOBOX(UI_PartEventCombo),
                           //~ ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   //~ etk_combobox_column_add(ETK_COMBOBOX(UI_PartEventCombo),
                           //~ ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   //~ etk_combobox_build(ETK_COMBOBOX(UI_PartEventCombo));
   //~ etk_table_attach_default(ETK_TABLE(table2), UI_PartEventCombo, 1, 4, 3, 3);
//~ 
//~ 
   //~ etk_signal_connect("text-changed", ETK_OBJECT(UI_PartNameEntry),
         //~ ETK_CALLBACK(_group_NamesEntry_text_changed_cb), NULL);   
   //~ etk_signal_connect("key-down", ETK_OBJECT(UI_PartNameEntry),
         //~ ETK_CALLBACK(_part_NameEntry_key_down_cb), NULL);
   //~ etk_signal_connect("mouse-click", ETK_OBJECT(UI_PartNameEntryImage),
                      //~ ETK_CALLBACK(_part_NameEntryImage_clicked_cb), NULL);
   //~ etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsCheck),
                      //~ ETK_CALLBACK(_part_EventsCheck_toggled_cb), NULL);
   //~ etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsRepeatCheck),
                      //~ ETK_CALLBACK(_part_EventsRepeatCheck_toggled_cb), NULL);
   //~ etk_signal_connect("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                     //~ ETK_CALLBACK(_part_CliptoComboBox_item_activated_cb), NULL);
   //~ etk_signal_connect("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                      //~ ETK_CALLBACK(_part_SourceComboBox_item_activated_cb), NULL);
   //~ etk_signal_connect("toggled", ETK_OBJECT(UI_PartDragXCheck),
                      //~ ETK_CALLBACK(_part_DragXCheck_toggled_cb), NULL);
   //~ etk_signal_connect("toggled", ETK_OBJECT(UI_PartDragYCheck),
                      //~ ETK_CALLBACK(_part_DragYCheck_toggled_cb), NULL);
   //~ 
   //~ etk_signal_connect("value-changed", ETK_OBJECT(UI_PartStepXSpinner),
                      //~ ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      //~ (void *)STEPX_SPINNER);
   //~ etk_signal_connect("value-changed", ETK_OBJECT(UI_PartStepYSpinner),
                      //~ ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      //~ (void *)STEPY_SPINNER);
   //~ etk_signal_connect("value-changed", ETK_OBJECT(UI_PartCountXSpinner),
                      //~ ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      //~ (void *)COUNTX_SPINNER);
   //~ etk_signal_connect("value-changed", ETK_OBJECT(UI_PartCountYSpinner),
                      //~ ETK_CALLBACK(_part_drag_spinners_value_changed_cb),
                      //~ (void *)COUNTY_SPINNER);
   //~ etk_signal_connect("item-activated", ETK_OBJECT(UI_PartConfineCombo),
                    //~ ETK_CALLBACK(_part_ConfineCombo_item_activated_cb), NULL);
   //~ etk_signal_connect("item-activated", ETK_OBJECT(UI_PartEventCombo),
                    //~ ETK_CALLBACK(_part_EventCombo_item_activated_cb), NULL);
   //~ return table;
}

void
part_frame_update(void)
{
   const char *clipto;

   if (!cur.part) return;

   printf("Update part frame %s\n", cur.part);

   // Set name
   elm_entry_entry_set(_name_entry, cur.part);

   // Set mouse events combo
   int ev = edje_edit_part_mouse_events_get(ui.edje_o, cur.part);
   int rp = edje_edit_part_repeat_events_get(ui.edje_o, cur.part);

   if (ev && rp) elm_hoversel_label_set(_events_combo, "yes + repeat");
   else if (ev)  elm_hoversel_label_set(_events_combo, "Yes");
   else          elm_hoversel_label_set(_events_combo, "No");

   // Set clip_to combobox
   clipto = edje_edit_part_clip_to_get(ui.edje_o, cur.part);
   if (clipto)
   {
      elm_hoversel_label_set(_clipto_combo, clipto);
      edje_edit_string_free(clipto);
   }
   else
      elm_hoversel_label_set(_clipto_combo, "unset");

   //~ /* Update PartSource combobox */
   //~ const char *source;
   //~ source = edje_edit_part_source_get(edje_o, Cur.part->string);

   //~ if (source)
   //~ {
      //~ //Loop for all the item in the Combobox
      //~ i=1;
      //~ while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartSourceComboBox),i)))
      //~ {
         //~ p = etk_combobox_item_field_get(item, 0);
         //~ if (!strcmp(p, source))
            //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),item);
         //~ i++;
      //~ }
   //~ }
   //~ else
      //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),
            //~ etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartSourceComboBox)));

   //~ edje_edit_string_free(source);

   //~ //Update dragables
   //~ status = edje_edit_part_drag_x_get(edje_o, Cur.part->string) == 0 ? 0 : 1;
   //~ etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartDragXCheck), status);
   //~ etk_widget_disabled_set(UI_PartStepXSpinner, !status);
   //~ etk_widget_disabled_set(UI_PartCountXSpinner, !status);

   //~ status = edje_edit_part_drag_y_get(edje_o, Cur.part->string) == 0 ? 0 : 1;
   //~ etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartDragYCheck), status);
   //~ etk_widget_disabled_set(UI_PartStepYSpinner, !status);
   //~ etk_widget_disabled_set(UI_PartCountYSpinner, !status);

   //~ etk_range_value_set(ETK_RANGE(UI_PartStepXSpinner),
            //~ (float)edje_edit_part_drag_step_x_get(edje_o, Cur.part->string));
   //~ etk_range_value_set(ETK_RANGE(UI_PartStepYSpinner),
            //~ (float)edje_edit_part_drag_step_y_get(edje_o, Cur.part->string));
   //~ etk_range_value_set(ETK_RANGE(UI_PartCountXSpinner),
            //~ (float)edje_edit_part_drag_count_x_get(edje_o, Cur.part->string));
   //~ etk_range_value_set(ETK_RANGE(UI_PartCountYSpinner),
            //~ (float)edje_edit_part_drag_count_y_get(edje_o, Cur.part->string));

   //~ //Update drag confine combo
   //~ source = edje_edit_part_drag_confine_get(edje_o, Cur.part->string);
   //~ if (source)
   //~ {
      //~ //Loop for all the item in the Combobox
      //~ i = 1;
      //~ while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartConfineCombo),i)))
      //~ {
         //~ p = etk_combobox_item_field_get(item, 1);
         //~ if (!strcmp(p, source))
            //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartConfineCombo),item);
         //~ i++;
      //~ }
   //~ }
   //~ else
      //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartConfineCombo),
            //~ etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartConfineCombo)));
   //~ edje_edit_string_free(source);

   //~ //Update drag events combo
   //~ source = edje_edit_part_drag_event_get(edje_o, Cur.part->string);
   //~ if (source)
   //~ {
      //~ //Loop for all the item in the Combobox
      //~ i = 1;
      //~ while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartEventCombo),i)))
      //~ {
         //~ p = etk_combobox_item_field_get(item, 1);
         //~ if (!strcmp(p, source))
            //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartEventCombo),item);
         //~ i++;
      //~ }
   //~ }
   //~ else
      //~ etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartEventCombo),
            //~ etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartEventCombo)));
   //~ edje_edit_string_free(source);

   //~ //Show/hide Sourcecombo for part EDJE_PART_TYPE_GROUP
   //~ if (edje_edit_part_type_get(edje_o, Cur.part->string) == EDJE_PART_TYPE_GROUP)
   //~ {
      //~ etk_widget_show(UI_PartSourceComboBox);
      //~ etk_widget_show(UI_PartSourceLabel);
   //~ }
   //~ else
   //~ {
      //~ etk_widget_hide(UI_PartSourceComboBox);
      //~ etk_widget_hide(UI_PartSourceLabel);
   //~ }
}


const char *
part_type_image_get2(const char *part)
{
   /* Get the name of the group in edje_editor.edj that
    * correspond to the given EDJE_PART_TYPE.
    */
   switch (edje_edit_part_type_get(ui.edje_o, part))
   {
      case EDJE_PART_TYPE_IMAGE:     return "IMAGE.PNG";
      case EDJE_PART_TYPE_GRADIENT:  return "GRAD_LINEAR.PNG";
      case EDJE_PART_TYPE_TEXT:      return "TEXT.PNG";
      case EDJE_PART_TYPE_RECTANGLE: return "RECT.PNG";
      case EDJE_PART_TYPE_SWALLOW:   return "SWAL.PNG";
      case EDJE_PART_TYPE_GROUP:     return "GROUP.PNG";
      default:                       return "NONE.PNG";
   }

   return NULL;
}

char *
part_type_image_get_OLD(const char *part)
{
   /* Get the name of the group in edje_editor.edj that
    * correspond to the given EDJE_PART_TYPE.
    * Remember to free the returned string.
    */
   static char buf[20];
   
   switch (edje_edit_part_type_get(ui.edje_o, part))
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


void
part_populate_combo_with_parts(void *data, Evas_Object *obj, void *event_info)
{
   Eina_List *parts, *l;
   const char *name;
   parts = edje_edit_parts_list_get(ui.edje_o);
   
   elm_hoversel_item_add(obj, "unset", NULL, ELM_ICON_NONE, NULL, NULL);
   EINA_LIST_FOREACH(parts, l, name)
      //TODO Fix icons
      elm_hoversel_item_add(obj, name, "home", ELM_ICON_STANDARD, NULL, NULL);
   edje_edit_string_list_free(parts);
}

/***   Callbacks   ***/
//~ Etk_Bool
//~ _part_SourceComboBox_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
//~ {
   //~ char *gr;
   //~ printf("Item Activated Signal on PartSourceCombobox EMITTED\n");
//~ 
   //~ gr = etk_combobox_item_field_get(item,0);
//~ 
   //~ if (!strcmp(gr, Cur.group->string))
   //~ {
      //~ dialog_alert_show("A group can't contain itself");
      //~ return ETK_TRUE;
   //~ }
//~ 
   //~ if (strcmp(gr, "None"))
      //~ edje_edit_part_source_set(edje_o, Cur.part->string, gr);
   //~ else
      //~ edje_edit_part_source_set(edje_o, Cur.part->string, NULL);
//~ 
   //~ reload_edje();
   //~ return ETK_TRUE;
//~ }


//~ Etk_Bool
//~ _part_DragXCheck_toggled_cb(Etk_Object *object, void *data)
//~ {
   //~ int status;
//~ 
   //~ if (!etk_string_length_get(Cur.part))
      //~ return ETK_TRUE;
//~ 
   //~ //printf("Toggled Signal on DragX checkbox EMITTED\n");
   //~ status = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object));
   //~ edje_edit_part_drag_x_set(edje_o, Cur.part->string, status);
//~ 
   //~ etk_widget_disabled_set(UI_PartStepXSpinner, !status);
   //~ etk_widget_disabled_set(UI_PartCountXSpinner, !status);
//~ 
   //~ return ETK_TRUE;
//~ }

//~ Etk_Bool
//~ _part_DragYCheck_toggled_cb(Etk_Object *object, void *data)
//~ {
   //~ int status;
//~ 
   //~ if (!etk_string_length_get(Cur.part))
      //~ return ETK_TRUE;
//~ 
   //~ //printf("Toggled Signal on DragY checkbox EMITTED\n");
   //~ status = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object));
   //~ edje_edit_part_drag_y_set(edje_o, Cur.part->string, status);
//~ 
   //~ etk_widget_disabled_set(UI_PartStepYSpinner, !status);
   //~ etk_widget_disabled_set(UI_PartCountYSpinner, !status);
//~ 
   //~ return ETK_TRUE;
//~ }

//~ Etk_Bool
//~ _part_drag_spinners_value_changed_cb(Etk_Range *range, double value, void *data)
//~ {
   //~ printf("Part Drag Spinners value changed signal EMIT\n");
   //~ if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   //~ switch ((int)(long)data)
   //~ {
      //~ case STEPX_SPINNER:
         //~ edje_edit_part_drag_step_x_set(edje_o, Cur.part->string,
                                        //~ (int)etk_range_value_get(range));
         //~ break;
      //~ case STEPY_SPINNER:
         //~ edje_edit_part_drag_step_y_set(edje_o, Cur.part->string,
                                        //~ (int)etk_range_value_get(range));
         //~ break;
      //~ case COUNTX_SPINNER:
         //~ edje_edit_part_drag_count_x_set(edje_o, Cur.part->string,
                                         //~ (int)etk_range_value_get(range));
         //~ break;
      //~ case COUNTY_SPINNER:
         //~ edje_edit_part_drag_count_y_set(edje_o, Cur.part->string,
                                         //~ (int)etk_range_value_get(range));
         //~ break;
      //~ default:
         //~ break;
   //~ }
   //~ return ETK_TRUE;
//~ }

//~ Etk_Bool
//~ _part_ConfineCombo_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
//~ {
   //~ char *confine;
   //~ printf("Item Activated Signal on DragConfineCombo EMITTED\n");
   //~ 
   //~ confine = etk_combobox_item_field_get(item, 1);
   //~ if (strcmp(confine, "None"))
      //~ edje_edit_part_drag_confine_set(edje_o, Cur.part->string, confine);
   //~ else
      //~ edje_edit_part_drag_confine_set(edje_o, Cur.part->string, NULL);
//~ 
   //~ return ETK_TRUE;
//~ }

//~ Etk_Bool
//~ _part_EventCombo_item_activated_cb(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
//~ {
   //~ char *event;
   //~ printf("Item Activated Signal on DragEventCombo EMITTED\n");
   //~ 
   //~ event = etk_combobox_item_field_get(item, 1);
   //~ if (strcmp(event, "None"))
      //~ edje_edit_part_drag_event_set(edje_o, Cur.part->string, event);
   //~ else
      //~ edje_edit_part_drag_event_set(edje_o, Cur.part->string, NULL);
//~ 
   //~ return ETK_TRUE;
//~ }
