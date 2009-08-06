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

static Evas_Object *inwin;

/***  Callbacks  ***/

static void
_ok_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *item;

   if (data)
   {
      item = elm_list_selected_item_get(data);
      if (item && cur.part && cur.state)
	 edje_edit_state_font_set(ui.edje_o, cur.part, cur.state,
				 elm_list_item_label_get(item));
   
      text_frame_update();
   }

   //Close the dialog
   evas_object_del(inwin);
}

static void
_add_clicked(void *data, Evas_Object *obj, void *event_info)
{
   printf("ADD\n");
   dialog_filechooser_show(FILECHOOSER_FONT);
   
   //Close the dialog
   evas_object_del(inwin);
}

/***  Implementation  ***/

#define ICON_ADD() \
   icon = elm_icon_add(parent); \
   elm_icon_standard_set(icon, "home"); \
   evas_object_size_hint_min_set(icon, 20, 20); \
   evas_object_show(icon);

void
fonts_browser_show(Evas_Object *parent)
{
   Evas_Object *vbox, *hbox, *o, *icon, *list;
   Eina_List *fonts, *l;
   const char *name;

   printf("FONTS BROWSER SHOW: \n");

   // InWin
   inwin = elm_win_inwin_add(ui.win);
   //~ elm_object_style_set(inwin, "minimal");
   evas_object_show(inwin);

   // Vbox
   vbox = elm_box_add(parent);
   evas_object_show(vbox);
   elm_win_inwin_content_set(inwin, vbox);

   // Title
   o = elm_label_add(parent);
   elm_label_label_set(o, "<b>Available Fonts</b>");
   elm_box_pack_end(vbox, o);
   evas_object_show(o);
   

   // List
   list = elm_list_add(parent);
   evas_object_size_hint_align_set(list, -1.0, -1.0);
   evas_object_size_hint_weight_set(list, 1.0, 1.0);
   elm_box_pack_end(vbox, list);
   
   ICON_ADD()
   elm_list_item_append(list, "Sans", icon, NULL, NULL, NULL);
   ICON_ADD()
   elm_list_item_append(list, "Sans:style=Bold", icon, NULL, NULL, NULL);

   fonts = edje_edit_fonts_list_get(ui.edje_o);
   EINA_LIST_FOREACH(fonts, l, name)
      elm_list_item_append(list, name, NULL, NULL, NULL, NULL);
   edje_edit_string_list_free(fonts);

   elm_list_go(list);
   evas_object_show(list);

   // Hbox
   hbox = elm_box_add(parent);
   elm_box_horizontal_set(hbox, 1);
   evas_object_show(hbox);
   elm_box_pack_end(vbox, hbox);

   // Add Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Import font...");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _add_clicked, NULL);
   evas_object_show(o);

   // Cancel Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Cancel");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _ok_clicked, NULL);
   evas_object_show(o);
   
   // OK Button
   o = elm_button_add(ui.win);
   elm_button_label_set(o, "Ok");
   elm_box_pack_end(hbox, o);
   evas_object_smart_callback_add(o, "clicked", _ok_clicked, list);
   evas_object_show(o);
}
