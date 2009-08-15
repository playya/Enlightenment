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

#ifndef _EDJE_EDITOR_WINDOW_H_
#define _EDJE_EDITOR_WINDOW_H_


/*** Public */
void window_main_create(void);
void window_update_frames_visibility(void);
//~ Etk_Widget* window_color_button_create (char* label_text, int color_button_enum,int w,int h, Evas *evas);


/*** Private */
void _window_delete_cb(void *data, Evas_Object *obj, void *event_info);
void _window_logo_key_press(void *data, Evas *e, Evas_Object *obj, void *event_info);
//~ void     _window_resize_cb           (Ecore_Evas *ecore_evas);
//~ 
//~ void     _window_edit_obj_signal_cb  (void *data, Evas_Object *o, const char *sig, const char *src);
//~ void     _window_edit_obj_click      (void *data, Evas *e, Evas_Object *obj, void *event_info);
//~ 
//~ Etk_Bool _window_all_button_click_cb (Etk_Button *button, void *data);
//~ void     _window_logo_key_press      (void *data, Evas *e, Evas_Object *obj, void *event_info);
//~ void     _window_color_canvas_click  (void *data, Evas *e, Evas_Object *obj, void *event_info);
//~ 
//~ void     _window_confirm_save        (Etk_Dialog *dialog, int response_id, void *data);

//TODO describe all this
#define NEW_ENTRY_TO_TABLE(TITLE, X, Y, W, ENTRY, EDITABLE) \
   _o = elm_label_add(parent); \
   elm_label_label_set(_o, TITLE"  "); \
   elm_table_pack(tb, _o, X, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 0.0, 0.0); \
   evas_object_size_hint_align_set(_o, 1.0, 0.5); \
   evas_object_show(_o); \
    \
   _o = elm_entry_add(parent); \
   elm_entry_single_line_set(_o, 1); \
   elm_entry_editable_set(_o, EDITABLE); \
   elm_entry_entry_set(_o, "----"); \
   elm_table_pack(tb, _o, X+1, Y, W, 1); \
   evas_object_size_hint_weight_set(_o, 1.0, 0.0); \
   evas_object_size_hint_align_set(_o, 0.0, 0.5); \
   evas_object_show(_o); \
   ENTRY = _o; \
   evas_object_event_callback_add(_o, EVAS_CALLBACK_KEY_DOWN, _entry_key_down, NULL);
   
#define NEW_DOUBLE_ENTRY_TO_TABLE(TITLE, X, Y, ENTRY1, ENTRY2, EDITABLE) \
   _o = elm_label_add(parent); \
   elm_label_label_set(_o, TITLE"  "); \
   elm_table_pack(tb, _o, X, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 0.0, 0.0); \
   evas_object_size_hint_align_set(_o, 1.0, 0.5); \
   evas_object_show(_o); \
    \
   _o = elm_entry_add(parent); \
   elm_entry_single_line_set(_o, 1); \
   elm_entry_editable_set(_o, EDITABLE); \
   elm_entry_entry_set(_o, "----"); \
   elm_table_pack(tb, _o, X+1, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 1.0, 0.0); \
   evas_object_size_hint_align_set(_o, 0.0, 0.5); \
   evas_object_show(_o); \
   ENTRY1 = _o; \
   evas_object_event_callback_add(_o, EVAS_CALLBACK_KEY_DOWN, _entry_key_down, NULL); \
   \
   _o = elm_entry_add(parent); \
   elm_entry_single_line_set(_o, 1); \
   elm_entry_editable_set(_o, EDITABLE); \
   elm_entry_entry_set(_o, "----"); \
   elm_table_pack(tb, _o, X+2, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 1.0, 0.0); \
   evas_object_size_hint_align_set(_o, 0.0, 0.5); \
   evas_object_show(_o); \
   ENTRY2 = _o; \
   evas_object_event_callback_add(_o, EVAS_CALLBACK_KEY_DOWN, _entry_key_down, NULL);

#define NEW_COMBO_TO_TABLE(OBJ, TITLE, X, Y, W, POPULATE_CB, SEL_CB) \
   _o = elm_label_add(parent); \
   elm_label_label_set(_o, TITLE"  "); \
   elm_table_pack(tb, _o, X, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 0.0, 0.0); \
   evas_object_size_hint_align_set(_o, 1.0, 0.5); \
   evas_object_show(_o); \
    \
   _o = elm_hoversel_add(parent); \
   elm_hoversel_hover_parent_set(_o, ui.win); \
   evas_object_size_hint_weight_set(_o, 1.0, 0.0); \
   evas_object_size_hint_align_set(_o, 0.0, 0.5); \
   elm_table_pack(tb, _o, X+1, Y, W, 1); \
   evas_object_show(_o); \
   OBJ = _o; \
   if (POPULATE_CB) { \
      evas_object_smart_callback_add(_o, "clicked", POPULATE_CB, NULL); \
      evas_object_smart_callback_add(_o, "dismissed", elm_hoversel_clear, _o); \
   } \
   if (SEL_CB) \
      evas_object_smart_callback_add(_o, "selected", SEL_CB, _o);

#define NEW_SLIDER_TO_TABLE(TITLE, X, Y, W, SLIDER, CB) \
   _o = elm_label_add(parent); \
   elm_label_label_set(_o, TITLE"  "); \
   elm_table_pack(tb, _o, X, Y, 1, 1); \
   evas_object_size_hint_weight_set(_o, 0.0, 0.0); \
   evas_object_size_hint_align_set(_o, 1.0, 0.5); \
   evas_object_show(_o); \
   \
   _o = elm_slider_add(parent); \
   elm_slider_min_max_set(_o, 0.0, 255.0); \
   elm_slider_indicator_format_set(_o, "%3.0f"); \
   elm_slider_value_set(_o, 255); \
   elm_slider_span_size_set(_o, 120); \
   elm_table_pack(tb, _o, X+1, Y, W, 1); \
   evas_object_size_hint_weight_set(_o, 1.0, 0.0); \
   evas_object_size_hint_align_set(_o, 0.0, 0.5); \
   evas_object_show(_o); \
   SLIDER = _o; \
   evas_object_smart_callback_add(_o, "changed", CB, NULL);

#define NEW_TITLE_TO_TABLE(TITLE, X, Y, W) \
   _o = elm_label_add(parent); \
   elm_label_label_set(_o, "<b>"TITLE"</b>"); \
   elm_table_pack(tb, _o, X, Y, W, 1); \
   evas_object_show(_o);



#endif
