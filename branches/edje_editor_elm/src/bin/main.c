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
elm_entry_printf(Evas_Object *obj, const char *format, ...)
{
   char buf[4096];
   va_list fmtargs;

   va_start(fmtargs,format);
   vsnprintf(buf, sizeof(buf), format, fmtargs);
   va_end(fmtargs);
   
   elm_entry_entry_set(obj, buf);
}

Eina_Bool
ecore_str_equal(const char *s1, const char *s2)
{
   if (!strcmp(s1, s2)) return EINA_TRUE;
   return EINA_FALSE;
}

int
save_edje(const char *file)
{
   const char *compiler = NULL;
  
   // TODO: port warning dialog
   compiler = edje_edit_compiler_get(ui.edje_o);
   if (strcmp(compiler, "edje_edit"))
   {
      //~ dialog = etk_message_dialog_new(ETK_MESSAGE_DIALOG_WARNING,
                                    //~ ETK_MESSAGE_DIALOG_OK_CANCEL,
               //~ "<b>Warning</b><br>"
               //~ "This file has been compiled from EDC sources.<br>"
               //~ "Saving the file means that the original source file will<br>"
               //~ "be replaced by a new generated EDC.<br><br>"
               //~ "This will cause the lost of the following features:<br>"
               //~ " - All the MACRO (#define) will be lost.<br>"
               //~ " - All the comments in the original sources will be lost.<br>"
               //~ " - All the sources file will be merged in a single EDC.<br><br>"
               //~ "Are you sure you want to save the file?"
               //~ );
      //~ etk_signal_connect("response", ETK_OBJECT(dialog),
                      //~ ETK_CALLBACK(_window_confirm_save), NULL);
      //~ etk_widget_show_all(dialog);
      //~ break;
   }
   edje_edit_string_free(compiler);
 
   //~ _window_confirm_save(NULL, ETK_RESPONSE_OK, NULL);

   int save_success = edje_edit_save(ui.edje_o);
   
   if (save_success)
   {
      if (file)
      {
         if (!ecore_file_cp(cur.open_temp_name, file))
         {
            save_success = 0;
         }
         //TODO: overwrite cur.open_file_name with file
      }
      else
      {
         if (!ecore_file_cp(cur.open_temp_name, cur.open_file_name))
         {
            save_success = 0;
         }
      }

   }
   if (!save_success)
   {
      dialog_alert_show("<b>ERROR</b>:<br>Can't save to Edje file.");
      return 0;
   }
  
  return 1;
}

int
load_edje(const char *file)
{
   unsigned char new_file = 0;
   char *realp = NULL;
   const char *old_temp;

   if (!file)
   {
      file = PACKAGE_DATA_DIR"/blank.edj";
      new_file = 1;
   }

   printf("** Load EDJ: '%s'\n", file);

   realp = ecore_file_realpath(file);
   if (!ecore_file_exists(realp))
   {
      dialog_alert_show("<b>ERROR:</b><br>File not exists.");
      return 0;
   }
   if (!ecore_file_can_read(realp))
   {
      dialog_alert_show("<b>ERROR</b>:<br>Can't read file.");
      return 0;
   }
   if (!ecore_str_has_suffix(realp, ".edj"))  //TODO: better check
   {
      dialog_alert_show("<b>ERROR</b>:<br>File is not an edje file.");
      return 0;
   }

   //Create temp file
   if (cur.open_temp_name)
      old_temp = eina_stringshare_add(cur.open_temp_name);
   else
      old_temp = NULL;

   char tmpn[1024];
   int fd = 0;
   strcpy(tmpn, "/tmp/edje_editor_tmp.edj-XXXXXX");
   fd = mkstemp(tmpn);
   if (fd < 0)
   {
      printf("Can't create temp file '%s'\nError: %s\n", tmpn, strerror(errno));
      return 0;
   }
   
   if (cur.open_temp_name) eina_stringshare_del(cur.open_temp_name);
   cur.open_temp_name = eina_stringshare_add(tmpn);


   if (!ecore_file_cp(realp, cur.open_temp_name))
   {
      dialog_alert_show("<b>ERROR</b>:<br>Can't copy to temp file.");
      return 0;
   }

   if (new_file)
   {
      if (cur.open_file_name) eina_stringshare_del(cur.open_file_name);
      cur.open_file_name = NULL;
      elm_win_title_set(ui.win, "Untitled");
   }
   else
   {
      if (cur.open_file_name) eina_stringshare_del(cur.open_file_name);
      cur.open_file_name = eina_stringshare_add(realp);
      elm_win_title_set(ui.win, cur.open_file_name);
   }


   set_current_group(NULL);
   set_current_part(NULL);
   set_current_state(NULL);
   set_current_prog(NULL);

   //~ Parts_Hash = NULL;

   tree_groups_create();
   //~ text_font_combo_populate();
   //~ gradient_spectra_combo_populate();


   //Delete old temp file
   if (old_temp && !ecore_file_unlink(old_temp))
   {
      dialog_alert_show("<b>ERROR</b>:<br>Can't remove temp file.");
      return 0;
   }
   eina_stringshare_del(old_temp);
   return 1;
}

void
change_to_group(const char *group)
{
   if (!group) return;

   //~ if (cur.group && !strcmp(group, cur.group)) return; TODO
   
   printf("Change to group: %s\n", group);

   //~ toolbar_play_button_toggle(1);

   if (cur.group && ui.edje_o)
      edje_edit_save(ui.edje_o);

   evas_object_hide(ui.edje_o);
   consolle_clear();

   //evas_object_del(edje_o);
   //edje_o = NULL;
   //edje_o = edje_object_add(UI_evas);

   if (!edje_object_file_set(ui.edje_o, cur.open_temp_name, group))
   {
      dialog_alert_show("Error loading group");
      return;
   }
   
   evas_object_show(ui.edje_o);
   evas_object_move(ui.edje_o, 300, 100);
   evas_object_resize(ui.edje_o, 300, 300);
   //~ Parts_Hash = NULL;         //TODO FREE

   set_current_group(group);
   set_current_part(NULL);
   set_current_state(NULL);
   set_current_prog(NULL);

   tree_parts_create();
   //~ position_comboboxes_populate();
   //~ program_source_combo_populate();
   //~ program_signal_combo_populate();

   group_frame_update();
   part_frame_update();
   
   window_update_frames_visibility();

   //update FakeWin
   edje_object_part_text_set(EV_fakewin, "title", group);
   canvas_redraw();
}

void set_current_group(const char *group)
{
   printf("Set group: %s to CURRENT\n", group);
   if (cur.group) eina_stringshare_del(cur.group);
   cur.group = group ? eina_stringshare_add(group) : NULL;
}

void set_current_part(const char *part)
{
   printf("Set part: %s to CURRENT\n", part);
   if (cur.part) eina_stringshare_del(cur.part);
   cur.part = part ? eina_stringshare_add(part) : NULL;
}

void set_current_state(const char *state)
{
   printf("Set state: %s to CURRENT\n", state);
   if (cur.state) eina_stringshare_del(cur.state);
   cur.state = state ? eina_stringshare_add(state) : NULL;
}

void set_current_prog(const char *prog)
{
   printf("Set prog: %s to CURRENT\n", prog);
   if (cur.prog) eina_stringshare_del(cur.prog);
   cur.prog = prog ? eina_stringshare_add(prog) : NULL;
}

EAPI int
elm_main(int argc, char **argv)
{
   
   setlocale(LC_NUMERIC,"C");
   //printf("LOCALE TEST:\n");
   //double val = 1.2;
   //sscanf("3.2","%lf", &val);
   //printf("%f\n", val);
   
   
   //Init Globals
   EdjeFile = PACKAGE_DATA_DIR"/edje_editor.edj";
   
   ui.edje_o = NULL;
   cur.group = NULL;
   cur.part = NULL;
   cur.state = NULL;
   cur.prog = NULL;
   cur.fullscreen = 0;
   cur.open_temp_name = NULL;
   cur.open_file_name = NULL;
   consolle_count = 0;
   stack = NULL;
   

   // Add a custom theme to elementary
   elm_theme_overlay_add(EdjeFile);

   elm_finger_size_set(1);
   window_main_create();
   canvas_prepare();
   
   // Open a file from the command line
   if (argc == 2)
   {
      printf("Opening edje file: '%s'\n",argv[1]);
      load_edje(argv[1]);
   }
   else
   // Or open blank.edj
   {
      load_edje(NULL);
   }
   
   
   elm_run();
   elm_shutdown();
   return 0; 
}
ELM_MAIN()
