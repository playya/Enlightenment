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
static Evas_Object *_aspect_min_entry;
static Evas_Object *_aspect_max_entry;
static Evas_Object *_aspect_combo;
static Evas_Object *_size_min_entry;
static Evas_Object *_size_max_entry;
static Evas_Object *_align_x_entry;
static Evas_Object *_align_y_entry;
static Evas_Object *_color_class_entry;
static Evas_Object *_visible_toggle;

/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   double f;
   int w, h;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, maybe a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   printf("Apply entry [%s]\n", txt);

   if (!txt || !cur.state) return;
   
   // Apply Name
   if (o == _name_entry)
   {
      if (ecore_str_equal(txt, cur.state)) return;

      if (ecore_str_equal(cur.state, "default 0.0"))
      {
	 dialog_alert_show("You can't rename default 0.0");
	 state_frame_update();
	 return;
      }

      if (edje_edit_state_name_set(ui.edje_o, cur.part, cur.state, txt))
      {
	 /* update tree  TODO */
	 //~ Etk_Tree_Row *row;
	 //~ row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
	 //~ etk_tree_row_fields_set(row,TRUE,
                                    //~ COL_NAME, EdjeFile, "DESC.PNG", name,
                                    //~ NULL);
	 set_current_state(txt);
      }
      else
	 dialog_alert_show("<b>Wrong name format</b><br>Name must be in the form:<br>\"default 0.00\"");
   }

   // Apply Aspect 
   else if (o == _aspect_min_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_aspect_min_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _aspect_max_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_aspect_max_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }

   // Apply SizeMin
   else if (o == _size_min_entry)
   {
      if (ecore_str_equal(txt, "unset"))
	 w = h = 0;
      else if (sscanf(txt,"%dx%d", &w, &h) != 2)
      {
	 dialog_alert_show(MSG_SIZE);
	 return;
      }
      edje_edit_state_min_w_set(ui.edje_o, cur.part, cur.state, w);
      edje_edit_state_min_h_set(ui.edje_o, cur.part, cur.state, h);
   }

   // Apply SizeMax
   else if (o == _size_max_entry )
   {
      if (ecore_str_equal(txt, "unset"))
	 w = h = -1;
      else if (sscanf(txt,"%dx%d", &w, &h) != 2)
      {
	 dialog_alert_show(MSG_SIZE);
	 return;
      }
      edje_edit_state_max_w_set(ui.edje_o, cur.part, cur.state, w);
      edje_edit_state_max_h_set(ui.edje_o, cur.part, cur.state, h);
   }

   // Apply Align
   else if (o == _align_x_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_align_x_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _align_y_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
	 edje_edit_state_align_y_set(ui.edje_o, cur.part, cur.state, f);
      else
	 dialog_alert_show(MSG_FLOAT);
   }

   // Apply ColorClass
   else if (o == _color_class_entry)
   {
      if (ecore_str_equal(txt, "unset") || strlen(txt) < 1)
	 edje_edit_state_color_class_set(ui.edje_o, cur.part, cur.state, NULL);
      else
	 edje_edit_state_color_class_set(ui.edje_o, cur.part, cur.state, txt);
   }
   
   canvas_redraw();
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
      state_frame_update();
   }
}

static void
_aspect_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   edje_edit_state_aspect_pref_set(ui.edje_o, cur.part, cur.state,
				   (int)(long)data);
   state_frame_update();
}

static void
_visible_toggle_changed(void *data, Evas_Object *obj, void *event_info)
{
   if (!cur.part || !cur.state) return;
   edje_edit_state_visible_set(ui.edje_o, cur.part, cur.state,
			       elm_toggle_state_get(_visible_toggle));
   canvas_redraw();
}

/***   Implementation   ***/
Evas_Object*
state_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *_o, *p;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("name:", 0, 0, _name_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("aspect:", 0, 1, _aspect_min_entry, _aspect_max_entry, EINA_TRUE)
   NEW_COMBO_TO_TABLE(_aspect_combo,"preference:", 0, 2, 2, NULL, NULL);
   elm_hoversel_item_add(_aspect_combo, "None", NULL, ELM_ICON_NONE,
                         _aspect_combo_sel, (void*)EDJE_ASPECT_PREFER_NONE);
   elm_hoversel_item_add(_aspect_combo, "Vertical", NULL, ELM_ICON_NONE,
                         _aspect_combo_sel, (void*)EDJE_ASPECT_PREFER_VERTICAL);
   elm_hoversel_item_add(_aspect_combo, "Horizontal", NULL, ELM_ICON_NONE,
                         _aspect_combo_sel, (void*)EDJE_ASPECT_PREFER_HORIZONTAL);
   elm_hoversel_item_add(_aspect_combo, "Both", NULL, ELM_ICON_NONE,
                         _aspect_combo_sel, (void*)EDJE_ASPECT_PREFER_BOTH);
   
   NEW_ENTRY_TO_TABLE("min:", 0, 3, _size_min_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("max:", 0, 4, _size_max_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("align:", 0, 5, _align_x_entry, _align_y_entry, EINA_TRUE)

   //// NEW_TOGGLE_TO_TABLE            // TODO
   _o = elm_label_add(parent);
   elm_label_label_set(_o, "visible:  ");
   evas_object_size_hint_weight_set(_o, 0.0, 0.0);
   evas_object_size_hint_align_set(_o, 1.0, 0.0);
   elm_table_pack(tb, _o, 0, 6, 1, 1);
   evas_object_show(_o);
   
   p = elm_frame_add(parent);
   elm_frame_style_set(p, "pad_small");
   evas_object_size_hint_weight_set(p, 1.0, 1.0);
   evas_object_size_hint_align_set(p, 0.0, 0.0);
   elm_table_pack(tb, p, 1, 6, 2, 1);
   evas_object_show(p);

   _o = elm_toggle_add(parent);
   elm_toggle_states_labels_set(_o, "visible", "hidden");
   
   evas_object_size_hint_weight_set(_o, 1.0, 0.0);
   evas_object_size_hint_align_set(_o, 0.0, 0.0);
   elm_frame_content_set(p, _o);
   _visible_toggle = _o;
   evas_object_show(_o);
   evas_object_smart_callback_add(_o, "changed", _visible_toggle_changed, NULL);
   ////
   
   NEW_ENTRY_TO_TABLE("color class:", 0, 7, _color_class_entry, EINA_TRUE)

   return tb;
}

void
state_frame_update(void)
{
   const char* cc;


   if (!cur.state) return;
   
   //Set description name & index
   elm_entry_entry_set(_name_entry, cur.state);

   //TODO reenable this
   //~ if (!strcmp(Cur.state->string, "default 0.00"))
      //~ etk_widget_disabled_set(ETK_WIDGET(UI_StateEntry), ETK_TRUE);
   //~ else
      //~ etk_widget_disabled_set(ETK_WIDGET(UI_StateEntry), ETK_FALSE);

   //Set aspect min & max
   elm_entry_printf(_aspect_min_entry, "%.1f",
		    edje_edit_state_aspect_min_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_aspect_max_entry, "%.1f",
		    edje_edit_state_aspect_max_get(ui.edje_o, cur.part, cur.state));
   
   //Set aspect preference
   switch (edje_edit_state_aspect_pref_get(ui.edje_o, cur.part, cur.state))
   {
      case EDJE_ASPECT_PREFER_HORIZONTAL:
	 elm_hoversel_label_set(_aspect_combo, "horizontal");
	 break;
      case EDJE_ASPECT_PREFER_VERTICAL:
	 elm_hoversel_label_set(_aspect_combo, "vertical");
	 break;
      case EDJE_ASPECT_PREFER_BOTH:
	 elm_hoversel_label_set(_aspect_combo, "both");
	 break;
      case EDJE_ASPECT_PREFER_NONE: default:
	 elm_hoversel_label_set(_aspect_combo, "none");
	 break;
   }

   //Set SizeMin
   int w, h;
   w = edje_edit_state_min_w_get(ui.edje_o, cur.part, cur.state);
   h = edje_edit_state_min_h_get(ui.edje_o, cur.part, cur.state);
   if (w == 0 && h == 0)
      elm_entry_entry_set(_size_min_entry, "unset");
   else
      elm_entry_printf(_size_min_entry, "%dx%d", w, h);

   //Set SizeMax
   w = edje_edit_state_max_w_get(ui.edje_o, cur.part, cur.state);
   h = edje_edit_state_max_h_get(ui.edje_o, cur.part, cur.state);
   if (w == -1 && h == -1)
      elm_entry_entry_set(_size_max_entry, "unset");
   else
      elm_entry_printf(_size_max_entry, "%dx%d", w, h);

   //Set align & valign
   elm_entry_printf(_align_x_entry, "%.3f",
	 edje_edit_state_align_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_align_y_entry, "%.3f",
	 edje_edit_state_align_y_get(ui.edje_o, cur.part, cur.state));

   //Set visible checkbox
   elm_toggle_state_set(_visible_toggle,
		edje_edit_state_visible_get(ui.edje_o, cur.part, cur.state));

   //Set Color Class Entry
   cc = edje_edit_state_color_class_get(ui.edje_o, cur.part, cur.state);
   elm_entry_entry_set(_color_class_entry, cc ? cc : "unset");
   edje_edit_string_free(cc);
   
}
