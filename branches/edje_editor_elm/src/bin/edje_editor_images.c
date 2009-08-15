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

static Evas_Object *_border_entry;
static Evas_Object *_image_entry;
static Evas_Object *_alpha_slider;
static Evas_Object *_middle_check;

static Evas_Object *_b_inwin;
static Evas_Object *_b_title;
static Evas_Object *_b_twl; //tween list
static Evas_Object *_b_iml; //image list
static Evas_Object *_b_radio; //image type radio


/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   int l, r, t, b;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, maybe a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   //~ printf("Apply entry [%s]\n", txt);

   if (!txt || !cur.group || !cur.part || !cur.state)
     return;

   // apply border
   if (o == _border_entry)
   {
      if (sscanf(txt,"%d %d %d %d", &l, &r, &t, &b) != 4)
      {
         dialog_alert_show(MSG_COLOR); //TODO make a border message??
         return;
      }
      edje_edit_state_image_border_set(ui.edje_o, cur.part, cur.state, l, r, t, b);
   }

   image_frame_update();
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
      image_frame_update();
   }
}

static void
_browse_clicked(void *data, Evas_Object *obj, void *event_info)
{
   image_browser_show(1);
}

static void
_alpha_slider_changed(void *data, Evas_Object *obj, void *event_info)
{
   if (!cur.part || !cur.state) return;
   edje_edit_state_color_set(ui.edje_o, cur.part, cur.state,
                             -1, -1, -1, elm_slider_value_get(obj));
   canvas_redraw();
}

static void
_middle_changed(void *data, Evas_Object *obj, void *event_info)
{
   if (!cur.part || !cur.state) return;
   edje_edit_state_image_border_fill_set(ui.edje_o, cur.part, cur.state,
                                         elm_check_state_get(_middle_check));
   canvas_redraw();
}
/***   Implementation   ***/
Evas_Object *
image_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *bt, *_o;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("image:", 0, 0, 1, _image_entry, EINA_FALSE)

   bt = elm_button_add(parent);
   elm_button_label_set(bt, "browse");
   evas_object_size_hint_align_set(bt, 1.0, 0.5);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_smart_callback_add(bt, "clicked", _browse_clicked, NULL);
   evas_object_show(bt);
   
   NEW_SLIDER_TO_TABLE("alpha", 0, 1, 2, _alpha_slider, _alpha_slider_changed)
   NEW_ENTRY_TO_TABLE("border:", 0, 2, 2, _border_entry, EINA_TRUE)
   
   _middle_check = elm_check_add(parent);
   elm_check_label_set(_middle_check, "middle");
   elm_table_pack(tb, _middle_check, 2, 2, 1, 1);
   evas_object_smart_callback_add(_middle_check, "changed", _middle_changed, NULL);
   evas_object_show(_middle_check);

   return tb;

   //~ //ImageTweenVBox
   //~ UI_ImageTweenVBox = etk_vbox_new(ETK_TRUE, 0);
   //~ etk_table_attach_default(ETK_TABLE(table), UI_ImageTweenVBox, 0, 1, 1, 1);
//~ 
   //~ //AddTweenButton
   //~ UI_AddTweenButton = etk_button_new_from_stock(ETK_STOCK_LIST_ADD);
   //~ etk_button_style_set(ETK_BUTTON(UI_AddTweenButton), ETK_BUTTON_ICON);
   //~ etk_signal_connect("clicked", ETK_OBJECT(UI_AddTweenButton),
      //~ ETK_CALLBACK(_window_all_button_click_cb), (void*)IMAGE_TWEEN_ADD);
   //~ etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_AddTweenButton,
                     //~ ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
//~ 
   //~ //DeleteTweenButton
   //~ UI_DeleteTweenButton = etk_button_new_from_stock(ETK_STOCK_LIST_REMOVE);
   //~ etk_button_style_set(ETK_BUTTON(UI_DeleteTweenButton), ETK_BUTTON_ICON);
   //~ etk_signal_connect("clicked", ETK_OBJECT(UI_DeleteTweenButton),
      //~ ETK_CALLBACK(_window_all_button_click_cb), (void*)IMAGE_TWEEN_DELETE);
   //~ etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_DeleteTweenButton,
                  //~ ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
//~ 
   //~ //MoveUpTweenButton
   //~ UI_MoveUpTweenButton = etk_button_new_from_stock(ETK_STOCK_GO_UP);
   //~ etk_button_style_set(ETK_BUTTON(UI_MoveUpTweenButton),  ETK_BUTTON_ICON);
   //~ etk_signal_connect("clicked", ETK_OBJECT(UI_MoveUpTweenButton),
      //~ ETK_CALLBACK(_window_all_button_click_cb), (void*)IMAGE_TWEEN_UP);
   //~ etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_MoveUpTweenButton, 
                  //~ ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
//~ 
   //~ //MoveDownTweenButton
   //~ UI_MoveDownTweenButton = etk_button_new_from_stock(ETK_STOCK_GO_DOWN);
   //~ etk_button_style_set(ETK_BUTTON(UI_MoveDownTweenButton),  ETK_BUTTON_ICON);
   //~ etk_signal_connect("clicked", ETK_OBJECT(UI_MoveDownTweenButton),
      //~ ETK_CALLBACK(_window_all_button_click_cb), (void*)IMAGE_TWEEN_DOWN);
   //~ etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_MoveDownTweenButton,
                  //~ ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
//~ 
   //~ //ImageTweenList
   //~ UI_ImageTweenList = etk_tree_new();
   //~ etk_tree_mode_set(ETK_TREE(UI_ImageTweenList), ETK_TREE_MODE_LIST);
   //~ etk_tree_headers_visible_set(ETK_TREE(UI_ImageTweenList), FALSE);
  //~ // etk_tree_multiple_select_set(ETK_TREE(UI_ImageTweenList), ETK_TRUE);
   //~ col1 = etk_tree_col_new(ETK_TREE(UI_ImageTweenList), "Tween", 130, 0.0);
   //~ etk_tree_col_model_add(col1, etk_tree_model_image_new());
   //~ etk_tree_col_model_add(col1, etk_tree_model_text_new());
   //~ etk_tree_build(ETK_TREE(UI_ImageTweenList));
   //~ etk_table_attach_default(ETK_TABLE(table),UI_ImageTweenList, 2, 5, 1, 1);
//~ 
}

void
image_frame_update(void)
{
   //~ //Etk_Combobox_Item *item = NULL;
   const char *im;
   int alpha, l, r, t, b;


   if (!cur.part || !cur.state) return;

   //~ image_tweenlist_populate();
   //~ etk_widget_disabled_set(UI_DeleteTweenButton, TRUE);
   //~ etk_widget_disabled_set(UI_MoveDownTweenButton, TRUE);
   //~ etk_widget_disabled_set(UI_MoveUpTweenButton, TRUE);

   //Set the images label for normal image
   im = edje_edit_state_image_get(ui.edje_o, cur.part, cur.state);
   if (im)
   {
      elm_entry_entry_set(_image_entry, im);
      edje_edit_string_free(im);
   }
   else
      elm_entry_entry_set(_image_entry, "unset");

   //Set alpha and borders   
   edje_edit_state_color_get(ui.edje_o, cur.part, cur.state, NULL, NULL, NULL, &alpha);
   edje_edit_state_image_border_get(ui.edje_o, cur.part, cur.state, &l, &r, &t, &b);
   elm_entry_printf(_border_entry, "%d %d %d %d", l, r, t, b);
   elm_slider_value_set(_alpha_slider, (double)alpha);
   elm_check_state_set(_middle_check,
         edje_edit_state_image_border_fill_get(ui.edje_o, cur.part, cur.state));

}


/***  Image Browser Callbacks  ***/
static void
_canc_clicked(void *data, Evas_Object *obj, void *event_info)
{
   //Close the dialog
   evas_object_del(_b_inwin);
}

static void
_ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it;
   Eina_List *tweens, *l;
   const char *name;

   if (!cur.part || !cur.state)
   {
      // close the dialog and exit
      evas_object_del(_b_inwin);
      return;
   }
   
   it = elm_list_selected_item_get(_b_iml);
   if (it)
      edje_edit_state_image_set(ui.edje_o, cur.part, cur.state,
                                elm_list_item_label_get(it));

   // clear all tweens from state
   tweens = edje_edit_state_tweens_list_get(ui.edje_o, cur.part, cur.state);
   EINA_LIST_FOREACH(tweens, l, name)
      edje_edit_state_tween_del(ui.edje_o, cur.part, cur.state, name);
   edje_edit_string_list_free(tweens);

   // re-set all new tween frames 
   tweens = (Eina_List *)elm_list_items_get(_b_twl);
   EINA_LIST_FOREACH(tweens, l, it)
      edje_edit_state_tween_add(ui.edje_o, cur.part, cur.state,
                                elm_list_item_label_get(it));
   
   image_frame_update();
   // close the dialog
   evas_object_del(_b_inwin);
}

static void
_import_clicked(void *data, Evas_Object *obj, void *event_info)
{
   dialog_filechooser_show(FILECHOOSER_IMAGE);
   //Close the dialog
   evas_object_del(_b_inwin);
}

static void
_plus_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it, *after, *new;

   it = elm_list_selected_item_get(_b_iml);
   if (!it)
   {
      dialog_alert_show("You must first choose an image from the list");
      return;
   }

   after = elm_list_selected_item_get(_b_twl);
   if (after)
     new = elm_list_item_insert_after(_b_twl, after,
                                         elm_list_item_label_get(it),
                                         NULL, NULL, NULL, NULL);
   else
     new = elm_list_item_append(_b_twl, elm_list_item_label_get(it),
                                   NULL, NULL, NULL, NULL);
   elm_list_go(_b_twl);
   elm_list_item_selected_set(new, EINA_TRUE);
   elm_list_item_show(new);
}

static void
_minus_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it, *next;

   it = elm_list_selected_item_get(_b_twl);
   if (!it)
   {
      dialog_alert_show("You must first choose a frame from the tween list");
      return;
   }

   next = elm_list_item_next(it);
   if (!next) next = elm_list_item_prev(it);
   elm_list_item_del(it);
   if (next) elm_list_item_selected_set(next, EINA_TRUE);
}

static void
_up_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it, *prev, *new;
   const char *name;

   it = elm_list_selected_item_get(_b_twl);
   if (!it)
   {
      dialog_alert_show("You must first choose a frame from the tween list");
      return;
   }

   prev = elm_list_item_prev(it);
   if (!prev) return; //we are the first item yet
   
   name = elm_list_item_label_get(it);
   new = elm_list_item_insert_before(_b_twl, prev, name, NULL, NULL, NULL, NULL);
   elm_list_item_del(it);
   elm_list_item_selected_set(new, EINA_TRUE);
   elm_list_go(_b_twl);
   elm_list_item_show(new);
}

static void
_down_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it, *next, *new;
   const char *name;

   it = elm_list_selected_item_get(_b_twl);
   if (!it)
   {
      dialog_alert_show("You must first choose a frame from the tween list");
      return;
   }

   next = elm_list_item_next(it);
   if (!next) return; //we are the last item yet
   
   name = elm_list_item_label_get(it);
   new = elm_list_item_insert_after(_b_twl, next, name, NULL, NULL, NULL, NULL);
   elm_list_item_del(it);
   elm_list_item_selected_set(new, EINA_TRUE);
   elm_list_go(_b_twl);
   elm_list_item_show(new);
}

/***  Image Browser  ***/
void
image_browser_show(int UpdateCurrent)
{
   Evas_Object *table, *tb, *vbox, *hbox, *o, *ic, *fr;

   // inwin
   _b_inwin = elm_win_inwin_add(ui.win);
   evas_object_show(_b_inwin);

   // main table
   table = elm_table_add(ui.win);
   elm_table_homogenous_set(table, EINA_FALSE);
   evas_object_size_hint_weight_set(table, 1.0, 1.0);
   evas_object_size_hint_align_set(table, -1.0, -1.0);
   evas_object_show(table);
   elm_win_inwin_content_set(_b_inwin, table);

   // title
   _b_title = elm_label_add(ui.win);
   elm_label_label_set(_b_title, "<b>Available Images</b>");
   elm_table_pack(table, _b_title, 0, 0, 2, 1);
   evas_object_show(_b_title);

   // images frame
   fr = elm_frame_add(ui.win);
   elm_frame_label_set(fr, "Available images");
   evas_object_size_hint_align_set(fr, -1.0, -1.0);
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   elm_table_pack(table, fr, 0, 1, 1, 3);
   evas_object_show(fr);

   // images list
   _b_iml = elm_list_add(ui.win);
   evas_object_show(_b_iml);
   elm_frame_content_set(fr, _b_iml);

   // Tween frame 1
   fr = elm_frame_add(ui.win);
   elm_frame_label_set(fr, "Image type");
   evas_object_size_hint_align_set(fr, -1.0, 0.0);
   evas_object_size_hint_weight_set(fr, 1.0, 0.0);
   elm_table_pack(table, fr, 1, 1, 1, 1);
   evas_object_show(fr);

   vbox = elm_box_add(ui.win);
   elm_frame_content_set(fr, vbox);
   evas_object_show(vbox);

   _b_radio = elm_radio_add(ui.win);
   elm_radio_label_set(_b_radio, "Normal image");
   elm_radio_state_value_set(_b_radio, 0);
   elm_box_pack_end(vbox, _b_radio);
   evas_object_show(_b_radio);

   o = elm_radio_add(ui.win);
   elm_radio_group_add(o, _b_radio);
   elm_radio_label_set(o, "Tween animation");
   elm_radio_state_value_set(o, 1);
   elm_box_pack_end(vbox, o);
   evas_object_show(o);

   // tween frame 2 (editor)
   fr = elm_frame_add(ui.win);
   elm_frame_label_set(fr, "Tween editor");
   evas_object_size_hint_align_set(fr, -1.0, -1.0);
   elm_table_pack(table, fr, 1, 2, 1, 1);
   evas_object_show(fr);

   tb = elm_table_add(ui.win);
   elm_frame_content_set(fr, tb);
   evas_object_show(tb);
   
   // tween add
   o = elm_button_add(ui.win);
   elm_button_label_set(o, ""); //TODO this is a bug in the button theme
   ic = elm_icon_add(ui.win);
   elm_icon_file_set(ic, EdjeFile, "ADD.PNG");
   elm_button_icon_set(o, ic);
   elm_table_pack(tb, o, 0, 0, 1, 1);
   evas_object_smart_callback_add(o, "clicked", _plus_clicked, NULL);
   evas_object_show(o);

   // tween del
   o = elm_button_add(ui.win);
   elm_button_label_set(o, ""); //TODO this is a bug in the button theme
   ic = elm_icon_add(ui.win);
   elm_icon_file_set(ic, EdjeFile, "REMOVE.PNG");
   elm_button_icon_set(o, ic);
   elm_table_pack(tb, o, 0, 1, 1, 1);
   evas_object_smart_callback_add(o, "clicked", _minus_clicked, NULL);
   evas_object_show(o);

   // tween up
   o = elm_button_add(ui.win);
   elm_button_label_set(o, ""); //TODO this is a bug in the button theme
   ic = elm_icon_add(ui.win);
   elm_icon_file_set(ic, EdjeFile, "UP.PNG");
   elm_button_icon_set(o, ic);
   elm_table_pack(tb, o, 0, 2, 1, 1);
   evas_object_smart_callback_add(o, "clicked", _up_clicked, NULL);
   evas_object_show(o);

   // tween down
   o = elm_button_add(ui.win);
   elm_button_label_set(o, ""); //TODO this is a bug in the button theme
   ic = elm_icon_add(ui.win);
   elm_icon_file_set(ic, EdjeFile, "DOWN.PNG");
   elm_button_icon_set(o, ic);
   elm_table_pack(tb, o, 0, 3, 1, 1);
   evas_object_smart_callback_add(o, "clicked", _down_clicked, NULL);
   evas_object_show(o);

   // tween list
   _b_twl = elm_list_add(ui.win);
   evas_object_size_hint_weight_set(_b_twl, 1.0, 1.0);
   evas_object_size_hint_align_set(_b_twl, -1.0, -1.0);
   elm_list_multi_select_set(_b_twl, EINA_FALSE);
   elm_table_pack(tb, _b_twl, 1, 0, 1, 4);
   evas_object_show(_b_twl);

   // text frame
   fr = elm_frame_add(ui.win);
   elm_frame_label_set(fr, "Info");
   evas_object_size_hint_weight_set(fr, 1.0, 1.0);
   evas_object_size_hint_align_set(fr, -1.0, -1.0);
   elm_table_pack(table, fr, 1, 3, 1, 1);
   evas_object_show(fr);
   
   o = elm_anchorblock_add(ui.win);
   elm_anchorblock_text_set(o, "Put here<br>many info...<br>info..."); //TODO Doc here
   
   elm_frame_content_set(fr, o);
   evas_object_show(o);
   
   // hbox (buttons)
   hbox = elm_box_add(ui.win);
   elm_box_horizontal_set(hbox, 1);
   evas_object_show(hbox);
   elm_table_pack(table, hbox, 0, 4, 2, 1);

   // Add Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Import image...");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _import_clicked, NULL);
   evas_object_show(o);

   // Cancel Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Cancel");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _canc_clicked, NULL);
   evas_object_show(o);
   
   // OK Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Ok");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _ok_clicked, NULL);
   evas_object_show(o);
   
   image_browser_update();
   //~ ImageBroserUpdate = UpdateCurrent;
}

void
image_browser_update(void)
{
   Evas_Object *ic;
   Eina_List *tweens, *images, *l;
   Elm_List_Item *selected = NULL;
   const char *name, *normal;
   char buf[1024];

   if (!cur.part || !cur.state) return;
   
   // update window title
   snprintf(buf, sizeof(buf), "<b>Choose image for part: \"%s\" state: \"%s\"</b>",
                              cur.part, cur.state);
   elm_label_label_set(_b_title, buf);

   // fill images list
   normal = edje_edit_state_image_get(ui.edje_o, cur.part, cur.state);
   images = edje_edit_images_list_get(ui.edje_o);
   EINA_LIST_FOREACH(images, l, name)
   {
      Elm_List_Item *it;
      snprintf(buf, sizeof(buf), "images/%d", edje_edit_image_id_get(ui.edje_o, name));
      printf("(%s) IM: %s\n", normal, buf);

      ic = elm_icon_add(ui.win);
      elm_icon_file_set(ic, cur.open_temp_name, buf);
      evas_object_size_hint_min_set(ic, 50, 50);
      
      it = elm_list_item_append(_b_iml, name, ic, NULL, NULL, NULL);
      if (ecore_str_equal(name, normal))
      {
         elm_list_item_selected_set(it, EINA_TRUE);
         selected = it;
      }
   }
   edje_edit_string_list_free(images);
   edje_edit_string_free(normal);
   
   elm_list_go(_b_iml);
   if (selected) elm_list_item_show(selected);

   
   // populate tweens list
   tweens = edje_edit_state_tweens_list_get(ui.edje_o, cur.part, cur.state);
   EINA_LIST_FOREACH(tweens, l, name)
      elm_list_item_append(_b_twl, name, NULL, NULL, NULL, NULL);
   elm_list_go(_b_twl);
   edje_edit_string_list_free(tweens);
   
   // update radio button
   elm_radio_value_set(_b_radio, tweens ? 1 : 0);
}
