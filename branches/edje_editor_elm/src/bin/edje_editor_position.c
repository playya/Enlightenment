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

static Evas_Object *_rel1x_entry;
static Evas_Object *_rel1y_entry;
static Evas_Object *_off1x_entry;
static Evas_Object *_off1y_entry;
static Evas_Object *_to1x_combo;
static Evas_Object *_to1y_combo;

static Evas_Object *_rel2x_entry;
static Evas_Object *_rel2y_entry;
static Evas_Object *_off2x_entry;
static Evas_Object *_off2y_entry;
static Evas_Object *_to2x_combo;
static Evas_Object *_to2y_combo;


/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   double f;
   int i;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, probably a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   printf("Apply entry [%s]\n", txt);

   // Rel 1
   if (o == _rel1x_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_rel1_relative_x_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   if (o == _rel1y_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_rel1_relative_y_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   if (o == _off1x_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
	 edje_edit_state_rel1_offset_x_set(ui.edje_o, cur.part, cur.state, i);
      else
	 dialog_alert_show(MSG_INT);
   }
   if (o == _off1y_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
	 edje_edit_state_rel1_offset_y_set(ui.edje_o, cur.part, cur.state, i);
      else
	 dialog_alert_show(MSG_INT);
   }

   // Rel 2
   if (o == _rel2x_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_rel2_relative_x_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   if (o == _rel2y_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_rel2_relative_y_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   if (o == _off2x_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
	 edje_edit_state_rel2_offset_x_set(ui.edje_o, cur.part, cur.state, i);
      else
	 dialog_alert_show(MSG_INT);
   }
   if (o == _off2y_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
	 edje_edit_state_rel2_offset_y_set(ui.edje_o, cur.part, cur.state, i);
      else
	 dialog_alert_show(MSG_INT);
   }

   position_frame_update();
   canvas_redraw();
}

static void
_entry_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   
   //~ printf("KEY DOWN %s\n", ev->key);
   if (ecore_str_equal(ev->key, "Return"))
      _entry_apply(obj);
   else if(ecore_str_equal(ev->key, "Escape"))
      position_frame_update();
}

static void
_to_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Hoversel_Item *it = event_info;
   const char *to;

   to = elm_hoversel_item_label_get(it);
   if (ecore_str_equal(to, cur.part))
   {
      dialog_alert_show("A part can't be related to itself.");
      return;
   }

   if (obj == _to1x_combo)
      edje_edit_state_rel1_to_x_set(ui.edje_o, cur.part, cur.state, 
				    ecore_str_equal(to, "unset") ? NULL : to);
   else if (obj == _to1y_combo)
      edje_edit_state_rel1_to_y_set(ui.edje_o, cur.part, cur.state, 
				    ecore_str_equal(to, "unset") ? NULL : to);
   else if (obj == _to2x_combo)
      edje_edit_state_rel2_to_x_set(ui.edje_o, cur.part, cur.state, 
				    ecore_str_equal(to, "unset") ? NULL : to);
   else if (obj == _to2y_combo)
      edje_edit_state_rel2_to_y_set(ui.edje_o, cur.part, cur.state, 
				    ecore_str_equal(to, "unset") ? NULL : to);
   
   position_frame_update();
   edje_edit_part_selected_state_set(ui.edje_o, cur.part, cur.state);  //this make edje redraw (need to update in lib)
   canvas_redraw();
}

/***   Implementation   ***/
Evas_Object*
position_frame_create(Evas_Object *parent)
{
   Evas_Object *vbox, *fr, *tb, *_o;

   vbox = elm_box_add(parent);
   elm_box_horizontal_set(vbox, 0);
   evas_object_size_hint_align_set(vbox, 0.0, 0.0);
   evas_object_size_hint_weight_set(vbox, 1.0, 1.0);
   evas_object_show(vbox);

   fr = elm_frame_add(parent);
   elm_frame_label_set(fr, "Top-Left point");
   evas_object_size_hint_align_set(fr, 0.0, 0.0);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_show(fr);
   elm_box_pack_end(vbox, fr);

   tb = elm_table_add(parent);
   evas_object_size_hint_align_set(tb, 0.0, 0.0);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   elm_frame_content_set(fr,tb);

   NEW_DOUBLE_ENTRY_TO_TABLE("relative:", 0, 0, _rel1x_entry, _rel1y_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("offset:", 0, 1, _off1x_entry, _off1y_entry, EINA_TRUE)
   NEW_COMBO_TO_TABLE(_to1x_combo, "to x", 0, 2, 2, part_populate_combo_with_parts, _to_combo_sel)
   NEW_COMBO_TO_TABLE(_to1y_combo, "to y", 0, 3, 2, part_populate_combo_with_parts, _to_combo_sel)
   
   fr = elm_frame_add(parent);
   elm_frame_label_set(fr, "Bottom-Right point");
   evas_object_size_hint_align_set(fr, 0.0, 0.0);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_show(fr);
   elm_box_pack_end(vbox, fr);

   tb = elm_table_add(parent);
   evas_object_size_hint_align_set(tb, 0.0, 0.0);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   elm_frame_content_set(fr,tb);

   NEW_DOUBLE_ENTRY_TO_TABLE("relative:", 0, 0, _rel2x_entry, _rel2y_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("offset:", 0, 1, _off2x_entry, _off2y_entry, EINA_TRUE)
   NEW_COMBO_TO_TABLE(_to2x_combo, "to x", 0, 2, 2, part_populate_combo_with_parts, _to_combo_sel)
   NEW_COMBO_TO_TABLE(_to2y_combo, "to y", 0, 3, 2, part_populate_combo_with_parts, _to_combo_sel)

   return vbox;
}

void
position_frame_update(void)
{
   const char *to;

   if (!cur.state || !cur.part) return;
   //~ printf("Update Position of: %s\n", cur.state);

   // Set position spinners
   elm_entry_printf(_rel1x_entry, "%.2f", edje_edit_state_rel1_relative_x_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_rel1y_entry, "%.2f", edje_edit_state_rel1_relative_y_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_off1x_entry, "%d", edje_edit_state_rel1_offset_x_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_off1y_entry, "%d", edje_edit_state_rel1_offset_y_get(
					  ui.edje_o, cur.part, cur.state));

   elm_entry_printf(_rel2x_entry, "%.2f", edje_edit_state_rel2_relative_x_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_rel2y_entry, "%.2f", edje_edit_state_rel2_relative_y_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_off2x_entry, "%d", edje_edit_state_rel2_offset_x_get(
					  ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_off2y_entry, "%d", edje_edit_state_rel2_offset_y_get(
					  ui.edje_o, cur.part, cur.state));


   // Set comboboxes
   if ((to = edje_edit_state_rel1_to_x_get(ui.edje_o, cur.part, cur.state)))
   {
      elm_hoversel_label_set(_to1x_combo, to);
      edje_edit_string_free(to);
   }
   else elm_hoversel_label_set(_to1x_combo, "unset");
   
   if ((to = edje_edit_state_rel1_to_y_get(ui.edje_o, cur.part, cur.state)))
   {
      elm_hoversel_label_set(_to1y_combo, to);
      edje_edit_string_free(to);
   }
   else elm_hoversel_label_set(_to1y_combo, "unset");
   
   if ((to = edje_edit_state_rel2_to_x_get(ui.edje_o, cur.part, cur.state)))
   {
      elm_hoversel_label_set(_to2x_combo, to);
      edje_edit_string_free(to);
   }
   else elm_hoversel_label_set(_to2x_combo, "unset");
   
   if ((to = edje_edit_state_rel2_to_y_get(ui.edje_o, cur.part, cur.state)))
   {
      elm_hoversel_label_set(_to2y_combo, to);
      edje_edit_string_free(to);
   }
   else elm_hoversel_label_set(_to2y_combo, "unset");
}
