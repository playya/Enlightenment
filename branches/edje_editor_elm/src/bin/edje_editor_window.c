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

void
_window_edit_obj_signal_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
   /* Catch all the signal from the editing edje object */
   char buf[1024];

   consolle_count++;
   snprintf(buf, sizeof(buf), "[%d]  SIGNAL = '%s'     SOURCE = '%s'",
            consolle_count, sig, src);
   consolle_log(buf);
}

/***   Implementation   ***/
void
window_main_create(void)
{
   Evas_Object *logo, *ly;
   Eina_Bool b;

   // Main Window
   ui.win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(ui.win, "Edje Editor");
   evas_object_smart_callback_add(ui.win, "delete-request", _window_delete_cb, NULL);
   evas_object_resize(ui.win, 900, 800);

   // Elementary Layout
   ly = elm_layout_add(ui.win);
   elm_layout_file_set(ly, EdjeFile, "layout");
   evas_object_size_hint_min_set(ly, 600, 350);
   evas_object_size_hint_weight_set(ly, 1.0, 1.0);
   elm_win_resize_object_add(ui.win, ly);
   evas_object_show(ly);
   ui.edje_ui = elm_layout_edje_get(ly);

   // Toolbar
   elm_layout_content_set(ly, "toolbar_swallow", toolbar_create(ui.win));

   // Tree Pager
   ui.tree_pager = elm_pager_add(ui.win);
   elm_layout_content_set(ly, "tree_swallow", ui.tree_pager);
   evas_object_show(ui.tree_pager);

   //Right pane
   elm_layout_content_set(ly, "group_frame_swallow", group_frame_create(ui.win));
   elm_layout_content_set(ly, "part_frame_swallow", part_frame_create(ui.win));
   elm_layout_content_set(ly, "position_frame_swallow", position_frame_create(ui.win));
   elm_layout_content_set(ly, "state_frame_swallow", state_frame_create(ui.win));
   elm_layout_content_set(ly, "text_frame_swallow", text_frame_create(ui.win));
   elm_layout_content_set(ly, "program_frame_swallow", program_frame_create(ui.win));
   elm_layout_content_set(ly, "rect_frame_swallow", rectangle_frame_create(ui.win));
   elm_layout_content_set(ly, "image_frame_swallow", image_frame_create(ui.win));
   elm_layout_content_set(ly, "fill_frame_swallow", fill_frame_create(ui.win));


   // Logo (keygrabber)
   logo = edje_object_add(evas_object_evas_get(ui.win));
   edje_object_file_set(logo, EdjeFile, "Logo");
   evas_object_event_callback_add(logo, EVAS_CALLBACK_KEY_DOWN,
                                  _window_logo_key_press, NULL);
   Evas_Modifier_Mask mask;
   mask = evas_key_modifier_mask_get(evas_object_evas_get(ui.win), "Control");
   b = evas_object_key_grab(logo, "q", mask, 0, 0); // quit
   b = evas_object_key_grab(logo, "f", mask, 0, 0); // fullscreen
   b = evas_object_key_grab(logo, "s", mask, 0, 0); // save
   b = evas_object_key_grab(logo, "c", mask, 0, 0); // copy selection (TODO)
   b = evas_object_key_grab(logo, "v", mask, 0, 0); // paste selection (TODO)
   b = evas_object_key_grab(logo, "x", mask, 0, 0); // cut selection (TODO)
   b = evas_object_key_grab(logo, "d", mask, 0, 0); // duplicate selection (TODO)
   b = evas_object_key_grab(logo, "n", mask, 0, 0); // new object (TODO)
   evas_object_show(logo);

   //Create the main edje object to edit
   ui.edje_o = edje_object_add(evas_object_evas_get(ui.win));
   edje_object_signal_callback_add(ui.edje_o, "*", "*",
                                   _window_edit_obj_signal_cb, NULL);
   //~ evas_object_event_callback_add(cur.edje_o, EVAS_CALLBACK_MOUSE_DOWN,
   //~ _window_edit_obj_click, NULL);

   evas_object_show(ui.win);
}

void
window_update_frames_visibility(void)
{
   if (cur.group)
      edje_object_signal_emit(ui.edje_ui, "group_frame_show", "edje_editor");
   else
      edje_object_signal_emit(ui.edje_ui, "group_frame_hide", "edje_editor");

   if (cur.part)
      edje_object_signal_emit(ui.edje_ui, "part_frame_show", "edje_editor");
   else
      edje_object_signal_emit(ui.edje_ui, "part_frame_hide", "edje_editor");

   if (cur.state)
   {
      edje_object_signal_emit(ui.edje_ui, "state_frame_show", "edje_editor");
      edje_object_signal_emit(ui.edje_ui, "position_frame_show", "edje_editor");
   }
   else
   {
      edje_object_signal_emit(ui.edje_ui, "state_frame_hide", "edje_editor");
      edje_object_signal_emit(ui.edje_ui, "position_frame_hide", "edje_editor");
   }

   if (cur.part && cur.state)
   {
      edje_object_signal_emit(ui.edje_ui,"state_frame_show","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"position_frame_show","edje_editor");

      switch (edje_edit_part_type_get(ui.edje_o, cur.part))
      {
      case EDJE_PART_TYPE_RECTANGLE:
         rectangle_frame_update();
         edje_object_signal_emit(ui.edje_ui,"rect_frame_show","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"fill_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"image_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"text_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"gradient_frame_hide","edje_editor");
         break;
      case EDJE_PART_TYPE_IMAGE:
         image_frame_update();
         fill_frame_update();
         edje_object_signal_emit(ui.edje_ui,"image_frame_show","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"fill_frame_show","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"rect_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"text_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"gradient_frame_hide","edje_editor");
         break;
         //~ case EDJE_PART_TYPE_GRADIENT:
         //~ gradient_frame_update();
         //~ fill_frame_update();
         //~ printf("GRAD\n");
         //~ edje_object_signal_emit(edje_ui,"gradient_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"fill_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
         //~ break;
      case EDJE_PART_TYPE_TEXT:
         text_frame_update();
         edje_object_signal_emit(ui.edje_ui,"text_frame_show","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"fill_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"image_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"rect_frame_hide","edje_editor");
         edje_object_signal_emit(ui.edje_ui,"gradient_frame_hide","edje_editor");
         break;
       // TODO: implement all part types
       default:
         break;
      }
   }
   else
   {
      edje_object_signal_emit(ui.edje_ui,"state_frame_hide","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"position_frame_hide","edje_editor");

      edje_object_signal_emit(ui.edje_ui,"rect_frame_hide","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"fill_frame_hide","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"image_frame_hide","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"text_frame_hide","edje_editor");
      edje_object_signal_emit(ui.edje_ui,"gradient_frame_hide","edje_editor");
   }

   if (cur.prog)
   {
      program_frame_update();
      edje_object_signal_emit(ui.edje_ui,"program_frame_show","edje_editor");
   }
   else
   {
      edje_object_signal_emit(ui.edje_ui,"program_frame_hide","edje_editor");
   }

   //TODO
   //~ edje_object_signal_emit(ui.edje_ui,"script_frame_hide","edje_editor");

}

/***   Callbacks   ***/
// generic callback - delete any window (close button/remove) and it just exits
void
_window_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

void
_window_logo_key_press(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   printf("*** Logo receive key pressed\n");
   printf("   keyname: %s\n", ev->keyname);
   printf("   key: %s\n", ev->key);
   printf("   string: %s\n", ev->string);
   printf("   compose: %s\n", ev->compose);

   /* NOTE: To add new bindings you must add a keygrab for the key
      you want in create_main_window(). And remember to update the README */

   /* quit */
   if (!strcmp(ev->key, "q") &&
         evas_key_modifier_is_set(ev->modifiers, "Control"))
      elm_exit();

   /* fullscreen */
   else if (!strcmp(ev->key, "f") &&
            evas_key_modifier_is_set(ev->modifiers, "Control"))
   {
      cur.fullscreen = !cur.fullscreen;
      elm_win_fullscreen_set(ui.win, cur.fullscreen);
   }

   //~ /* save (TODO make some sort of feedback for the user)*/
   //~ else if (!strcmp(ev->key, "s") &&
   //~ evas_key_modifier_is_set(ev->modifiers, "Control"))
   //~ _window_all_button_click_cb(NULL, (void *)TOOLBAR_SAVE);
}
