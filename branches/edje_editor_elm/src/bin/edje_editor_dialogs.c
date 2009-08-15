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

/***   FileChooser Dialog ***/
static void
_dialog_filechooser_done(void *data, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
   const char *selected = event_info;
   
   int FileChooserType = (long)evas_object_data_get(obj, "FileChooserType");

   if (!selected)
   {
      evas_object_del(data); /* delete the test window */
      return;
   }

   printf("Selected file: %s [type: %d]\n", selected, FileChooserType);

   switch(FileChooserType)
   {
      case FILECHOOSER_OPEN:
         if (load_edje(selected))
            evas_object_del(data);
         break;
      //~ case FILECHOOSER_SAVE_EDJ:
            //~ printf("SAVE EDJ\n");
            //~ snprintf(cmd,4096,"%s/%s",
               //~ etk_filechooser_widget_current_folder_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)),
               //~ etk_filechooser_widget_selected_file_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)));
            //~ edje_edit_save(edje_o);
            //~ if(!ecore_file_cp(Cur.edj_temp_name->string, cmd))
            //~ {
               //~ dialog_alert_show("<b>ERROR:<\b><br>Can't write file");
            //~ }
            //~ else
            //~ {
               //~ Cur.edj_file_name = etk_string_set(Cur.edj_file_name, cmd);
               //~ ecore_evas_title_set(UI_ecore_MainWin, cmd);
            //~ }
         //~ break;
         //~ case FILECHOOSER_SAVE_EDC:
              //~ dialog_alert_show("Not yet implemented.");
         //~ break;
         case FILECHOOSER_IMAGE:
            if (!edje_edit_image_add(ui.edje_o, selected))
            {
               dialog_alert_show("ERROR: Can't import image file.");
               break;
            }
            if (cur.part && cur.state)
               edje_edit_state_image_set(ui.edje_o, cur.part, cur.state,
                                         ecore_file_file_get(selected));
            // close the dialog
            evas_object_del(data);
            image_browser_show(EINA_TRUE);
            //TODO show the imported image
            break;
      case FILECHOOSER_FONT:
         if (!ecore_str_has_suffix(selected, ".ttf") ||
            (!edje_edit_font_add(ui.edje_o, selected)))
           dialog_alert_show("ERROR: Can't import font file.");
         evas_object_del(data);
         fonts_browser_show(ui.win);
         break;
   }
}

void
dialog_filechooser_show(int FileChooserType)
{
   Evas_Object *inwin, *fs;
   
   // InWin
   inwin = elm_win_inwin_add(ui.win);
   //~ elm_object_style_set(inwin, "minimal");
   elm_object_style_set(inwin, "default"); // TODO Should be minimal, but the genlist will not show in minimal mode :(
   evas_object_show(inwin);

   // File Selector
   fs = elm_fileselector_add(ui.win);
   elm_fileselector_path_set(fs, getenv("HOME"));
   elm_win_inwin_content_set(inwin, fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(fs, "done", _dialog_filechooser_done, inwin);
   evas_object_data_set(fs, "FileChooserType", (void*)(long)FileChooserType);


   //~ FileChooserOperation = FileChooserType;
   //~ switch(FileChooserOperation){
      //~ case FILECHOOSER_OPEN:
         //~ etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an EDJ or EDC file to open");
         //~ etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         //~ etk_widget_hide(UI_FilechooserSaveButton);
      //~ break;
      //~ case FILECHOOSER_IMAGE:
         //~ etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an image to import");
         //~ etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         //~ etk_widget_hide(UI_FilechooserSaveButton);
      //~ break;
      //~ case FILECHOOSER_FONT:
         //~ etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an font to import");
         //~ etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         //~ etk_widget_hide(UI_FilechooserSaveButton);
      //~ break;
      //~ case FILECHOOSER_SAVE_EDJ:
         //~ etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose the new edje name");
         //~ etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_TRUE);
         //~ etk_widget_hide(UI_FilechooserLoadButton);
      //~ break;
      //~ case FILECHOOSER_SAVE_EDC:
         //~ etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose the new edc name");
         //~ etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_TRUE);
         //~ etk_widget_hide(UI_FilechooserLoadButton);
      //~ break;
      //~ default:
      //~ break;
   //~ }
}


/***   Color Picker Dialog   
Etk_Widget *
dialog_colorpicker_create(void)
{
   UI_ColorWin = etk_window_new();
   etk_object_name_set(ETK_OBJECT(UI_ColorWin), "ColorDialog");
   etk_signal_connect("delete-event", ETK_OBJECT(UI_ColorWin),
                      ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   UI_ColorPicker = etk_colorpicker_new();
   etk_colorpicker_use_alpha_set (ETK_COLORPICKER(UI_ColorPicker), TRUE);
   etk_container_add(ETK_CONTAINER(UI_ColorWin), UI_ColorPicker);
   etk_signal_connect("color-changed", ETK_OBJECT(UI_ColorPicker),
                        ETK_CALLBACK(_dialog_colorpicker_change_cb), NULL);
   return UI_ColorWin;
}

Etk_Bool
_dialog_colorpicker_change_cb(Etk_Object *object, void *data)
{
  // printf("ColorChangeSignal on ColorDialog EMITTED\n");
   Etk_Color color;
   Etk_Color premuled;

   color = etk_colorpicker_current_color_get(ETK_COLORPICKER(object));
 //  printf("Color: %d %d %d %d\n",color.r,color.g,color.b,color.a);

   if (color.r > 255) color.r = 255;
   if (color.g > 255) color.g = 255;
   if (color.b > 255) color.b = 255;
   if (color.a > 255) color.a = 255;

   if (color.r < 0) color.r = 0;
   if (color.g < 0) color.g = 0;
   if (color.b < 0) color.b = 0;
   if (color.a < 0) color.a = 0;

   premuled = color;
   evas_color_argb_premul(premuled.a,&premuled.r,&premuled.g,&premuled.b);

   switch (current_color_object)
   {
    case COLOR_OBJECT_RECT:
      evas_object_color_set(RectColorObject,premuled.r,premuled.g,premuled.b,premuled.a);
      edje_edit_state_color_set(edje_o, Cur.part->string, Cur.state->string,
                                premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_TEXT:
      evas_object_color_set(TextColorObject,premuled.r,premuled.g,premuled.b,premuled.a);
      edje_edit_state_color_set(edje_o, Cur.part->string, Cur.state->string,
                                premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_SHADOW:
      evas_object_color_set(ShadowColorObject,premuled.r,premuled.g,premuled.b,premuled.a);
      edje_edit_state_color3_set(edje_o, Cur.part->string, Cur.state->string,
                                premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_OUTLINE:
      evas_object_color_set(OutlineColorObject,premuled.r,premuled.g,premuled.b,premuled.a);
      edje_edit_state_color2_set(edje_o, Cur.part->string, Cur.state->string,
                                premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_BG:
      edje_object_color_class_set(edje_ui, "cc_background",
                                  color.r, color.g, color.b, 255,
                                  0, 0, 0, 255, 0, 0, 0, 255);
    case COLOR_OBJECT_CC1:
      evas_object_color_set(ColorClassC1,premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_CC2:
      evas_object_color_set(ColorClassC2,premuled.r,premuled.g,premuled.b,premuled.a);
      break;
    case COLOR_OBJECT_CC3:
      evas_object_color_set(ColorClassC3,premuled.r,premuled.g,premuled.b,premuled.a);
      break;
   }

   canvas_redraw();
   return ETK_TRUE;
} ***/

/***   Alert Dialog   ***/
static void
_dialog_alert_ok_click_cb(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_del(data);
}

void
dialog_alert_show(char* text)
{
   Evas_Object *inwin, *lb, *table, *icon, *ok;
   
   printf("ALERT: %s\n", text);
   
   // InWin
   inwin = elm_win_inwin_add(ui.win);
   elm_object_style_set(inwin, "minimal");
   evas_object_show(inwin);
   
   // Table
   table = elm_table_add(ui.win);
   elm_table_homogenous_set(table, 0);
   elm_win_inwin_content_set(inwin, table);
   evas_object_show(table);

   // Icon
   icon = elm_icon_add(ui.win);
   elm_icon_file_set(icon, EdjeFile, "WARN.PNG");
   evas_object_size_hint_min_set(icon, 50, 50);
   elm_table_pack(table, icon, 0, 0, 1, 1);
   evas_object_show(icon);
   
   // Label
   lb = elm_label_add(ui.win);
   elm_label_label_set(lb, text);
   evas_object_size_hint_padding_set(lb, 50, 50, 50, 50);
   elm_table_pack(table, lb, 1, 0, 1, 1);
   evas_object_show(lb);
   
   // OK Button
   ok = elm_button_add(ui.win);
   elm_button_label_set(ok, "Ok");
   elm_table_pack(table, ok, 0, 1, 2, 1);
   evas_object_smart_callback_add(ok, "clicked", _dialog_alert_ok_click_cb, inwin);
   evas_object_show(ok);

}

