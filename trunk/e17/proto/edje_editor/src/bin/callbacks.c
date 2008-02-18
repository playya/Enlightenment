#include <string.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include <Etk.h>
#include <Ecore_Evas.h>
#include "callbacks.h"
#include "interface.h"
#include "inout.h"
#include "main.h"
#include "evas.h"


static int current_color_object;

/* Called when the window is destroyed */
void
ecore_delete_cb(Ecore_Evas *ee)
{
   etk_main_quit();
}

/* Called when the window is resized */
void
ecore_resize_callback(Ecore_Evas *ecore_evas)
{
   Evas_Object *embed_object;
   int win_w, win_h;
   
   //Get window size
   ecore_evas_geometry_get(UI_ecore_MainWin, NULL, NULL, &win_w, &win_h);
   
   //Resize main edje interface
   evas_object_resize(edje_ui, win_w, win_h);
   
   //Resize tree
   embed_object = etk_embed_object_get(ETK_EMBED(UI_PartsTreeEmbed));
   evas_object_move(embed_object, 0, 55);
   evas_object_resize(embed_object, TREE_WIDTH, win_h - 55);
}
/* Catch all the signal from the editing edje object */
void
signal_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
   printf("CALLBACK for \"%s\" \"%s\"\n", sig, src);
}

/* Group combobox callback */
Etk_Bool
on_GroupsComboBox_activated(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   char *gr;
   gr = etk_combobox_item_field_get(item,0);
   //printf("Group combo activated: %s\n",gr);
   ChangeGroup(gr);
   
   return ETK_TRUE;
}

/* All the buttons Callback */
Etk_Bool
on_AllButton_click(Etk_Button *button, void *data)
{
   char cmd[1024];
   Etk_String *text;
   char *tween;
   char *name;
   Etk_Tree_Row *row, *next, *prev;
   Etk_Combobox_Item *item;

   switch ((int)data)
      {
   case TOOLBAR_NEW:
      system("edje_editor &");
      break;
   case TOOLBAR_OPEN:
      ShowFilechooser(FILECHOOSER_OPEN);
      break;
   case TOOLBAR_SAVE:
      if (!etk_string_length_get(Cur.edj_file_name))
      {
         ShowFilechooser(FILECHOOSER_SAVE_EDJ);
         break;
      }
      
      edje_edit_save(edje_o);
      if (!ecore_file_cp(Cur.edj_temp_name->string, Cur.edj_file_name->string))
      {
         ShowAlert("<b>ERROR:<\b><br>Can't write file");
      }
      break;
   case TOOLBAR_SAVE_EDC:
      ShowAlert("Not yet reimplemented ;)");
      break;
   case TOOLBAR_SAVE_EDJ:
      ShowFilechooser(FILECHOOSER_SAVE_EDJ);
      break;
   case TOOLBAR_ADD:
      etk_menu_popup(ETK_MENU(UI_AddMenu));
      break;
   case TOOLBAR_REMOVE:
      etk_menu_popup(ETK_MENU(UI_RemoveMenu));
      break;
   
   case TOOLBAR_MOVE_UP: //Lower
      if (!etk_string_length_get(Cur.part))
      {
         ShowAlert("You must select a part to lower");
         break;
      }
      row = evas_hash_find(Parts_Hash, Cur.part->string);
      prev = etk_tree_row_prev_get(row);
      if (!prev) break;
      prev = etk_tree_row_prev_get(prev);
      if (prev)
         etk_tree_row_fields_get(prev, COL_NAME, NULL, NULL, &name, NULL);
      else
         name = NULL;
      if (!edje_edit_part_restack(edje_o, Cur.part->string, name))
         break;
      Parts_Hash = evas_hash_del(Parts_Hash, NULL, row);
      etk_tree_row_delete(row);
      
      if (prev)
         row = AddPartToTree(Cur.part->string, prev);
      else
         row = AddPartToTree(Cur.part->string, (void*)1);
      etk_tree_row_select(row);
      break;
   
   case TOOLBAR_MOVE_DOWN: //Raise
      if (!etk_string_length_get(Cur.part))
      {
         ShowAlert("You must select a part to lower");
         break;
      }
      row = evas_hash_find(Parts_Hash, Cur.part->string);
      next = etk_tree_row_next_get(row);
      if (!next)
         break;
      
      etk_tree_row_fields_get(next, COL_NAME, NULL, NULL, &name, NULL);
      if (!edje_edit_part_restack(edje_o, Cur.part->string, name))
         break;
      Parts_Hash = evas_hash_del(Parts_Hash, NULL, row);
      etk_tree_row_delete(row);
      row = AddPartToTree(Cur.part->string, next);
      etk_tree_row_select(row);
      
      break;
      
   case TOOLBAR_IMAGE_FILE_ADD:
      ShowFilechooser(FILECHOOSER_IMAGE);
      break;
   case TOOLBAR_FONT_FILE_ADD:
      ShowFilechooser(FILECHOOSER_FONT);
      break;
   case IMAGE_TWEEN_ADD:
      item = etk_combobox_active_item_get(ETK_COMBOBOX(UI_ImageComboBox));
      tween = etk_combobox_item_field_get(item, 1);
      if (!tween)
      {
         ShowAlert("You must choose an image to add from the combobox below");
      }
      
      if(edje_edit_state_tween_add(edje_o, Cur.part->string, Cur.state->string, tween))
      {
         PopulateTweenList();
         row = etk_tree_last_row_get(ETK_TREE(UI_ImageTweenList));
         etk_tree_row_select(row);
         etk_tree_row_scroll_to(row, ETK_FALSE);
      }
      break;
   case IMAGE_TWEEN_DELETE:
      //TODO delete the correct tween (not the first with that name)
      if (!etk_string_length_get(Cur.tween)) break;
      printf("REMOVE TWEEN %s\n", Cur.tween->string);
      edje_edit_state_tween_del(edje_o, Cur.part->string, Cur.state->string,
                                   Cur.tween->string);
      row = etk_tree_selected_row_get(ETK_TREE(UI_ImageTweenList));
      next = etk_tree_row_next_get(row);
      if (!next) 
         next = etk_tree_row_prev_get(row);
      if (next)
         etk_tree_row_select(next);
      else
      {
         Cur.tween = etk_string_clear(Cur.tween);
         etk_widget_disabled_set(UI_DeleteTweenButton, TRUE);
         etk_widget_disabled_set(UI_MoveDownTweenButton, TRUE);
         etk_widget_disabled_set(UI_MoveUpTweenButton, TRUE);
      }
      etk_tree_row_delete(row);
      break;
   case TOOLBAR_OPTIONS:
      etk_menu_popup(ETK_MENU(UI_OptionsMenu));
      //etk_menu_popup_at_xy (ETK_MENU(AddMenu), 10, 10);
      break;
   case TOOLBAR_OPTION_BG1:
      printf("SET_BG1\n");
      edje_object_signal_emit(edje_ui,"set_bg1","edje_editor");
      break;
    case TOOLBAR_OPTION_BG2:
      printf("SET_BG2\n");
      edje_object_signal_emit(edje_ui,"set_bg2","edje_editor");
      break;
   case TOOLBAR_OPTION_BG3:
      printf("SET_BG3\n");
      edje_object_signal_emit(edje_ui,"set_bg3","edje_editor");
      break;
   case TOOLBAR_OPTION_BG4:
      printf("SET_BG4\n");
      edje_object_signal_emit(edje_ui,"set_bg4","edje_editor");
      break;
   case TOOLBAR_PLAY:
      printf("Clicked signal on Toolbar Button 'Play' EMITTED\n");
      if (!Cur.eg)
         ShowAlert("You must select a group to test.");
      else if (!Cur.open_file_name) 
         ShowAlert("You need to save the file before testing it.");
      else
      {
         snprintf(cmd,1024,"edje_editor -t \"%s\" \"%s\" &",
                  Cur.open_file_name,Cur.eg->name);
         printf("TESTING EDJE. cmd: %s\n",cmd);
         system(cmd);
      }
      break;
   case TOOLBAR_DEBUG:
      DebugInfo(FALSE);
      break;
   case IMAGE_TWEEN_UP:
         ShowAlert("Up not yet implemented.");
      break;
   case IMAGE_TWEEN_DOWN:
         ShowAlert("Down not yet implemented.");
      break;
   case SAVE_SCRIPT:
         text = etk_textblock_text_get(ETK_TEXT_VIEW(UI_ScriptBox)->textblock,
                                       ETK_TRUE);
         ShowAlert("Script not yet implemented.");
         etk_object_destroy(ETK_OBJECT(text));
      break;
   default:
      break;
      }

   return ETK_TRUE;
}

/* Tree callbacks */
Etk_Bool
on_PartsTree_row_selected(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   int row_type=0;
   char *name;
   char *parent_name;

   printf("Row Selected Signal on one of the Tree EMITTED \n");

   //get the info from the tree cols of the selected row
   etk_tree_row_fields_get(row,
                           COL_TYPE, &row_type,
                           COL_NAME,NULL, NULL, &name,
                           COL_PARENT, &parent_name,
                           NULL);

   switch (row_type)
   {
      case ROW_PART:
         Cur.part = etk_string_set(Cur.part, name);
         Cur.state = etk_string_clear(Cur.state);
         Cur.tween = etk_string_clear(Cur.tween);
         Cur.prog = etk_string_clear(Cur.prog);
         
         edje_object_signal_emit(edje_ui,"description_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"position_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"program_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"part_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"script_frame_hide","edje_editor");
         
         UpdatePartFrame();
         break;

      case ROW_DESC:
         Cur.state = etk_string_set(Cur.state, name);
         Cur.part = etk_string_set(Cur.part, parent_name);
         Cur.tween = etk_string_clear(Cur.tween);
         Cur.prog = etk_string_clear(Cur.prog);
       
         edje_edit_part_selected_state_set(edje_o, Cur.part->string, Cur.state->string);  
         
         UpdateDescriptionFrame();
         UpdatePositionFrame();
         UpdateComboPositionFrame();
       
         switch(edje_edit_part_type_get(edje_o, Cur.part->string))
         {
            case EDJE_PART_TYPE_RECTANGLE:
               UpdateRectFrame();
               edje_object_signal_emit(edje_ui,"rect_frame_show","edje_editor");
               edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
               edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
               break;
            case EDJE_PART_TYPE_IMAGE:
               UpdateImageFrame();
               edje_object_signal_emit(edje_ui,"image_frame_show","edje_editor");
               edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
               edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
               break;
            case EDJE_PART_TYPE_TEXT:
               UpdateTextFrame();
               edje_object_signal_emit(edje_ui,"text_frame_show","edje_editor");
               edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
               edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
               break;
         }
         
         edje_object_signal_emit(edje_ui,"part_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"program_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"script_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"description_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"position_frame_show","edje_editor");
         break;
      
      case ROW_PROG:
         Cur.prog = etk_string_set(Cur.prog, name);
         Cur.part = etk_string_clear(Cur.part);
         Cur.state = etk_string_clear(Cur.state);
         Cur.tween = etk_string_clear(Cur.tween);
       
         edje_object_signal_emit(edje_ui,"description_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"position_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
         edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"part_frame_hide","edje_editor");
         
         edje_object_signal_emit(edje_ui,"program_frame_show","edje_editor");
         edje_object_signal_emit(edje_ui,"script_frame_show_small","edje_editor");
      
         UpdateScriptFrame();
         UpdateProgFrame();
        // PopulateSourceComboBox();
         break;
   }

   ev_redraw();
   return ETK_TRUE;
}

/* Group frame callbacks */
Etk_Bool
on_GroupNameEntry_text_changed(Etk_Object *object, void *data)
{
   //printf("Text Changed Signal on PartNameEntry EMITTED (text: %s)\n",etk_entry_text_get(ETK_ENTRY(object)));

   const char *name;
   name = etk_entry_text_get(ETK_ENTRY(object));
   
   edje_edit_group_name_set(edje_o, name);
   
   //Update Group Combobox
   Etk_Combobox_Item *item;
   item = etk_combobox_active_item_get(ETK_COMBOBOX(UI_GroupsComboBox));
   etk_signal_block("item-activated",ETK_OBJECT(UI_GroupsComboBox), on_GroupsComboBox_activated, NULL);
   etk_combobox_item_fields_set(item, name);
   etk_signal_unblock("item-activated",ETK_OBJECT(UI_GroupsComboBox), on_GroupsComboBox_activated, NULL);
    
   //Update FakeWin title
   edje_object_part_text_set(EV_fakewin, "title", name);

   return ETK_TRUE;
}

Etk_Bool
on_GroupSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Group Spinners value changed signal EMIT\n");
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   switch ((int)data)
   {
      case MINW_SPINNER:
         edje_edit_group_min_w_set(edje_o,
            (int)etk_range_value_get(ETK_RANGE(UI_GroupMinWSpinner)));
         break;
      case MINH_SPINNER:
         edje_edit_group_min_h_set(edje_o,
            (int)etk_range_value_get(ETK_RANGE(UI_GroupMinHSpinner)));
         break;
      case MAXW_SPINNER:
         edje_edit_group_max_w_set(edje_o,
            (int)etk_range_value_get(ETK_RANGE(UI_GroupMaxWSpinner)));
         break;
      case MAXH_SPINNER:
         edje_edit_group_max_h_set(edje_o,
            (int)etk_range_value_get(ETK_RANGE(UI_GroupMaxHSpinner)));
         break;
   }
   return ETK_TRUE;
}

/* Parts & Descriptions Callbacks*/
Etk_Bool
on_PartNameEntry_text_changed(Etk_Object *object, void *data)
{
   Etk_Tree_Row *row;
   const char *text;
   //printf("Text Changed Signal on PartNameEntry EMITTED (text: %s)\n",etk_entry_text_get(ETK_ENTRY(object)));
   if (etk_string_length_get(Cur.part))
   {
      //Update PartTree
      row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
      text = etk_entry_text_get(ETK_ENTRY(object));
      
      printf("** TYPE: %d\n", edje_edit_part_type_get(edje_o, Cur.part->string));
      switch (edje_edit_part_type_get(edje_o, Cur.part->string))
      {
         case EDJE_PART_TYPE_IMAGE:
            etk_tree_row_fields_set(row,TRUE,
                                    COL_NAME, EdjeFile, "IMAGE.PNG", text,
                                    NULL); 
            break;
         case EDJE_PART_TYPE_RECTANGLE:
            etk_tree_row_fields_set(row,TRUE,
                                    COL_NAME, EdjeFile, "RECT.PNG", text,
                                    NULL); 
            break;
         case EDJE_PART_TYPE_TEXT:
            etk_tree_row_fields_set(row,TRUE,
                                    COL_NAME, EdjeFile, "TEXT.PNG", text,
                                    NULL); 
            break;
         default:
            etk_tree_row_fields_set(row,TRUE,
                                    COL_NAME, EdjeFile, "NONE.PNG", text,
                                    NULL);
            break;
      }
      
      /* Update hidden colon on every child */
      Etk_Tree_Row *child;
      child = etk_tree_row_first_child_get(row);
      etk_tree_row_fields_set(child, TRUE, COL_PARENT, text, NULL);
      while ((child = etk_tree_row_next_get(child)))
         etk_tree_row_fields_set(child, TRUE, COL_PARENT, text, NULL);
       
      /* Update Parts_Hash */
      Parts_Hash = evas_hash_del(Parts_Hash, Cur.part->string, NULL);
      Parts_Hash = evas_hash_add(Parts_Hash, text, row);
       
      /* change the name in edje */
      edje_edit_part_name_set(edje_o, Cur.part->string, text);
      
      /* Set new Current name */
      Cur.part = etk_string_set(Cur.part, text);
      
      /* Recreate rel combobox */
      PopulateRelComboBoxes();  //TODO do a focus-out callback for this (don't need to do on every key!!)
   }
   return ETK_TRUE;
}

Etk_Bool
on_PartEventsCheck_toggled(Etk_Object *object, void *data)
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
on_PartEventsRepeatCheck_toggled(Etk_Object *object, void *data)
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
on_StateEntry_text_changed(Etk_Object *object, void *data)
{
   const char *nn;   //new name
   printf("Text Changed Signal on StateEntry EMITTED\n");

   if (etk_string_length_get(Cur.state))
   {
      if (strcmp("default 0.00", Cur.state->string))
      {
         nn = etk_entry_text_get(ETK_ENTRY(object));
         if (edje_edit_state_name_set(edje_o, Cur.part->string, Cur.state->string, nn))
         {
            /* update tree */
            Etk_Tree_Row *row;
            row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
            etk_tree_row_fields_set(row,TRUE,
                                       COL_NAME, EdjeFile, "DESC.PNG", nn,
                                       NULL);
            /* update Cur */
            Cur.state = etk_string_set(Cur.state, nn);
             
         }
         else
         {
            ShowAlert("<b>Wrong name format</b><br>Name must be in the form:<br>\"default 0.00\"");
         }
      }
      else
      {
         ShowAlert("You can't rename default 0.0");
         etk_signal_block("text-changed",ETK_OBJECT(object),
                          on_StateEntry_text_changed, NULL);
         etk_entry_text_set(ETK_ENTRY(object), Cur.state->string);
         etk_signal_unblock("text-changed",ETK_OBJECT(object),
                            on_StateEntry_text_changed, NULL);
      }
   }
   
   return ETK_TRUE;
}

Etk_Bool
on_StateIndexSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   //TODO remove this function when switch to edje internal
   char buf[4096];
   Etk_Tree_Col *col1=NULL;

   printf("Value Changed Signal on StateIndexSpinner EMITTED\n");
   if (Cur.eps)
   {
      snprintf(buf,4096,"%s",engrave_part_state_name_get(Cur.eps,NULL));
      //RenameDescription(selected_desc,NULL,etk_range_value_get(range));
      if ((strcmp("default", buf)) || Cur.eps->value)
      {
         engrave_part_state_name_set(Cur.eps,buf,etk_range_value_get(range));
      }else
      {
         ShowAlert("You can't rename default 0.0");
      }
      //Update PartTree
      col1 = etk_tree_nth_col_get(ETK_TREE(UI_PartsTree), 0);
      snprintf(buf,4095,"%s %.2f",Cur.eps->name,Cur.eps->value);
      etk_tree_row_fields_set(ecore_hash_get(hash,Cur.eps),TRUE,
         col1,EdjeFile,"DESC.PNG",buf,NULL);
   }

   return ETK_TRUE;
}

Etk_Bool
on_AspectSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on AspectMinSpinner EMITTED\n");
   edje_edit_state_aspect_min_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_AspectMinSpinner)));
   edje_edit_state_aspect_max_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_AspectMaxSpinner)));
   return ETK_TRUE;
}

Etk_Bool
on_AspectComboBox_changed(Etk_Combobox *combobox, void *data)
{
   printf("Active Item Changed Signal on AspectComboBox EMITTED\n");
   int pref;
   pref = (int)etk_combobox_item_data_get(etk_combobox_active_item_get (combobox));
   edje_edit_state_aspect_pref_set(edje_o, Cur.part->string, Cur.state->string, pref);
   return ETK_TRUE;
}

Etk_Bool
on_StateMinMaxSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Active Item Changed Signal on MinMaxSpinners EMITTED\n");

   edje_edit_state_min_w_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_StateMinWSpinner)));
   edje_edit_state_min_h_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_StateMinHSpinner)));
   edje_edit_state_max_w_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_StateMaxWSpinner)));
   edje_edit_state_max_h_set(edje_o, Cur.part->string, Cur.state->string,
                           etk_range_value_get(ETK_RANGE(UI_StateMaxHSpinner)));

   ev_redraw();
   return ETK_TRUE;
}

/* Image Frame Callbacks */
Etk_Bool
on_ImageComboBox_item_activated(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   printf("Changed signal on Image Combo EMITTED\n");
   
   char *im;
   if (!etk_string_length_get(Cur.state)) return ETK_TRUE;
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   
   im = etk_combobox_item_field_get(item, 1);
   edje_edit_state_image_set(edje_o, Cur.part->string, Cur.state->string, im);

   return ETK_TRUE;
}

Etk_Bool
on_ImageTweenList_row_selected(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Tree_Col *col;
   char *selected = NULL;
   printf("Row selected signal on ImageTweenList EMITTED\n");
   
   col = etk_tree_nth_col_get(ETK_TREE(UI_ImageTweenList), 0);
   etk_tree_row_fields_get(row, col, NULL, NULL, &selected, NULL);
   if (!selected) return ETK_TRUE;
   
   Cur.tween = etk_string_set(Cur.tween, selected);
   etk_widget_disabled_set(UI_DeleteTweenButton, FALSE);
  // etk_widget_disabled_set(UI_MoveDownTweenButton, FALSE);
  // etk_widget_disabled_set(UI_MoveUpTweenButton, FALSE);
   
   return ETK_TRUE;
}

Etk_Bool
on_ImageAlphaSlider_value_changed(Etk_Object *object, double va, void *data)
{
   printf("ImageSlieder value_changed signale EMIT: %.2f\n",va);

   if (!etk_string_length_get(Cur.state)) return ETK_TRUE;
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   edje_edit_state_color_set(edje_o, Cur.part->string, Cur.state->string,
                             -1, -1, -1, (int)va);
   ev_redraw();

   return ETK_TRUE;
}

Etk_Bool
on_BorderSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed signal on BorderSpinner EMITTED (value: %f)\n",etk_range_value_get(range));

   if (!etk_string_length_get(Cur.state)) return ETK_TRUE;
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   edje_edit_state_image_border_set(edje_o, Cur.part->string, Cur.state->string,
      (int)etk_range_value_get(ETK_RANGE(UI_BorderLeftSpinner)),
      (int)etk_range_value_get(ETK_RANGE(UI_BorderRightSpinner)),
      (int)etk_range_value_get(ETK_RANGE(UI_BorderTopSpinner)),
      (int)etk_range_value_get(ETK_RANGE(UI_BorderBottomSpinner)));

   ev_redraw();

   return ETK_TRUE;
}

/* Position Frame Callbacks */
Etk_Bool
on_RelToComboBox_changed(Etk_Combobox *combobox, void *data)
{
   char *parent;
   parent = etk_combobox_item_field_get(etk_combobox_active_item_get(combobox), 1);
   
   if (strcmp(parent,"Interface") == 0)
        parent = NULL;
    
   if (parent && (strcmp(parent,Cur.part->string) == 0))
   {
      ShowAlert("A state can't rel to itself.");
      return ETK_TRUE;
   }
   
   switch ((int)data)
   {
      case REL1X_SPINNER:
         edje_edit_state_rel1_to_x_set(edje_o, Cur.part->string,
                                       Cur.state->string, parent);
         break;
      case REL1Y_SPINNER:
         edje_edit_state_rel1_to_y_set(edje_o, Cur.part->string,
                                       Cur.state->string, parent);
         break;
      case REL2X_SPINNER:
         edje_edit_state_rel2_to_x_set(edje_o, Cur.part->string,
                                       Cur.state->string, parent);
         break;
      case REL2Y_SPINNER:
        edje_edit_state_rel2_to_y_set(edje_o, Cur.part->string,
                                      Cur.state->string, parent);
         break;
   }

   edje_edit_part_selected_state_set(edje_o, Cur.part->string, Cur.state->string);  //this make edje redraw (need to update in lib)
   ev_redraw();
   return ETK_TRUE;
}

Etk_Bool
on_RelSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on RelSpinner EMITTED (value: %f)\n",etk_range_value_get(range));

   if (etk_string_length_get(Cur.state) && etk_string_length_get(Cur.part))
   {
      switch ((int)data)
      {
         case REL1X_SPINNER:
            edje_edit_state_rel1_relative_x_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
         case REL1Y_SPINNER:
            edje_edit_state_rel1_relative_y_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
           
            break;
         case REL2X_SPINNER:
            edje_edit_state_rel2_relative_x_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
         case REL2Y_SPINNER:
            edje_edit_state_rel2_relative_y_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
      }
      ev_redraw();
   }
   return ETK_TRUE;
}

Etk_Bool
on_RelOffsetSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on Offset Spinner EMITTED\n");

   if (etk_string_length_get(Cur.state) && etk_string_length_get(Cur.part))
   {
      switch ((int)data)
      {
         case REL1X_SPINNER:
            edje_edit_state_rel1_offset_x_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
         case REL1Y_SPINNER:
            edje_edit_state_rel1_offset_y_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
         case REL2X_SPINNER:
            edje_edit_state_rel2_offset_x_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
         case REL2Y_SPINNER:
            edje_edit_state_rel2_offset_y_set(edje_o, 
                                    Cur.part->string, Cur.state->string,
                                    etk_range_value_get(range));
            break;
      }
      ev_redraw();
      //ev_draw_focus();
   }

   return ETK_TRUE;
}

/* Text Frame Callbacks */
Etk_Bool
on_FontComboBox_item_activated(Etk_Combobox *combobox, Etk_Combobox_Item *item, void *data)
{
   printf("Changed Signal on FontComboBox EMITTED \n");

   char *font;
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   if (!etk_string_length_get(Cur.state)) return ETK_TRUE;
   
   font = etk_combobox_item_field_get(item, 1);
   
   edje_edit_state_font_set(edje_o, Cur.part->string, Cur.state->string, font);
   
   return ETK_TRUE;
}

Etk_Bool
on_EffectComboBox_changed(Etk_Combobox *combobox, void *data)
{
   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   
   edje_edit_part_effect_set(edje_o, Cur.part->string,
      (int)etk_combobox_item_data_get(etk_combobox_active_item_get(combobox)));
   
   ev_redraw();

   return ETK_TRUE;
}

Etk_Bool
on_FontSizeSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on FontSizeSpinner EMITTED (value: %d)\n",(int)etk_range_value_get(range));

   edje_edit_state_text_size_set(edje_o, Cur.part->string, Cur.state->string,
                                 (int)etk_range_value_get(range));

   ev_redraw();
   return ETK_TRUE;
}

Etk_Bool
on_TextEntry_text_changed(Etk_Object *object, void *data)
{
   printf("Text Changed Signal on TextEntry EMITTED (value %s)\n",etk_entry_text_get(ETK_ENTRY(object)));
   edje_edit_state_text_set(edje_o, Cur.part->string, Cur.state->string,
                            etk_entry_text_get(ETK_ENTRY(object)));

   ev_redraw();
   return ETK_TRUE;
}

Etk_Bool
on_FontAlignSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on AlignSpinner (h or v, text or part) EMITTED (value: %.2f)\n",etk_range_value_get(range));

   if (!etk_string_length_get(Cur.part)) return ETK_TRUE;
   if (!etk_string_length_get(Cur.state)) return ETK_TRUE;
   
   if ((int)data == TEXT_ALIGNH_SPINNER)
      edje_edit_state_text_align_x_set(edje_o, Cur.part->string, Cur.state->string,
                                       (double)etk_range_value_get(range));
   if ((int)data == TEXT_ALIGNV_SPINNER)
      edje_edit_state_text_align_y_set(edje_o, Cur.part->string, Cur.state->string,
                                       (double)etk_range_value_get(range));
   if ((int)data == STATE_ALIGNH_SPINNER)
      edje_edit_state_align_x_set(edje_o, Cur.part->string, Cur.state->string,
                                  (double)etk_range_value_get(range));
   if ((int)data == STATE_ALIGNV_SPINNER)
      edje_edit_state_align_y_set(edje_o, Cur.part->string, Cur.state->string,
                                  (double)etk_range_value_get(range));

   return ETK_TRUE;
}

/* Programs Callbacks */
Etk_Bool
on_ActionComboBox_changed(Etk_Combobox *combobox, void *data)
{
   int action;
   printf("Changed Signal on ActionComboBox EMITTED\n");
   
   if (!etk_string_length_get(Cur.prog)) return ETK_TRUE;

   //Get the new action from the combo data
   action = (int)etk_combobox_item_data_get(
               etk_combobox_active_item_get(combobox));

   edje_edit_program_action_set(edje_o, Cur.prog->string, action);
   
   if (action == EDJE_ACTION_TYPE_SIGNAL_EMIT)
   {
      etk_widget_hide(UI_TargetEntry);
      etk_widget_hide(UI_TargetLabel);
      etk_widget_hide(UI_TransiComboBox);
      etk_widget_hide(UI_TransiLabel);
      etk_widget_hide(UI_DurationSpinner);
      etk_widget_show(UI_Param1Entry);
      etk_widget_show(UI_Param1Label);
      etk_label_set(ETK_LABEL(UI_Param1Label), "<b>Signal</b>");
      etk_label_set(ETK_LABEL(UI_Param2Label), "<b>Source</b>");
      etk_widget_hide(UI_Param1Spinner);
      etk_widget_show(UI_Param2Label);
      etk_widget_show(UI_Param2Entry);
   }
   if (action == EDJE_ACTION_TYPE_STATE_SET)
   {
      etk_widget_show(UI_TargetEntry);
      etk_widget_show(UI_TargetLabel);
      etk_widget_show(UI_TransiComboBox);
      etk_widget_show(UI_TransiLabel);
      etk_widget_show(UI_DurationSpinner);
      etk_widget_show(UI_Param1Entry);
      etk_widget_show(UI_Param1Label);
      etk_label_set(ETK_LABEL(UI_Param1Label), "<b>State</b>");
      etk_widget_show(UI_Param1Spinner);
      etk_widget_hide(UI_Param2Label);
      etk_widget_hide(UI_Param2Entry);
      etk_widget_hide(UI_Param2Spinner);
   }
   if (action == EDJE_ACTION_TYPE_ACTION_STOP)
   {
      etk_widget_show(UI_TargetEntry);
      etk_widget_show(UI_TargetLabel);
      etk_widget_hide(UI_TransiComboBox);
      etk_widget_hide(UI_TransiLabel);
      etk_widget_hide(UI_DurationSpinner);
      etk_widget_hide(UI_Param1Entry);
      etk_widget_hide(UI_Param1Label);
      etk_widget_hide(UI_Param1Spinner);
      etk_widget_hide(UI_Param2Label);
      etk_widget_hide(UI_Param2Entry);
      etk_widget_hide(UI_Param2Spinner);
   }
   if (action == EDJE_ACTION_TYPE_NONE ||
       action == EDJE_ACTION_TYPE_SCRIPT)
   {
      etk_widget_hide(UI_TargetEntry);
      etk_widget_hide(UI_TargetLabel);
      etk_widget_hide(UI_TransiComboBox);
      etk_widget_hide(UI_TransiLabel);
      etk_widget_hide(UI_DurationSpinner);
      etk_widget_hide(UI_Param1Entry);
      etk_widget_hide(UI_Param1Label);
      etk_widget_hide(UI_Param1Spinner);
      etk_widget_hide(UI_Param2Label);
      etk_widget_hide(UI_Param2Entry);
      etk_widget_hide(UI_Param2Spinner);
   }
   if (action == EDJE_ACTION_TYPE_DRAG_VAL_SET ||
       action == EDJE_ACTION_TYPE_DRAG_VAL_STEP ||
       action == EDJE_ACTION_TYPE_DRAG_VAL_PAGE)
   {
      etk_widget_hide(UI_TargetEntry);
      etk_widget_hide(UI_TargetLabel);
      etk_widget_hide(UI_TransiComboBox);
      etk_widget_hide(UI_TransiLabel);
      etk_widget_hide(UI_DurationSpinner);
      etk_label_set(ETK_LABEL(UI_Param1Label), "<b>? ? ? ?</b>");
      etk_label_set(ETK_LABEL(UI_Param2Label), "<b>? ? ? ?</b>");
      etk_widget_hide(UI_Param1Entry);
      etk_widget_show(UI_Param1Label);
      etk_widget_show(UI_Param1Spinner);
      etk_widget_show(UI_Param2Label);
      etk_widget_hide(UI_Param2Entry);
      etk_widget_show(UI_Param2Spinner);
   }

   return ETK_TRUE;
}

Etk_Bool
on_ProgramEntry_text_changed(Etk_Object *object, void *data)
{
   const char *name;

   //printf("Text Changed Signal on ProgramEntry Emitted\n");
   
   if (!etk_string_length_get(Cur.prog)) return ETK_TRUE;
   
   name = etk_entry_text_get(ETK_ENTRY(UI_ProgramEntry));
   
   if (edje_edit_program_name_set(edje_o, Cur.prog->string, name))
   {
      /* update tree */
      Etk_Tree_Row *row;
      row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
      etk_tree_row_fields_set(row,TRUE,
                              COL_NAME, EdjeFile, "PROG.PNG", name,
                              NULL);
      /* update Cur */
      Cur.prog = etk_string_set(Cur.prog, name);
   }
   
   return ETK_TRUE;
}

Etk_Bool
on_SourceEntry_text_changed(Etk_Object *object, void *data)
{
   printf("Text Changed Signal on SourceEntry Emitted\n");
   const char *str = etk_entry_text_get(ETK_ENTRY(object));
   edje_edit_program_source_set(edje_o, Cur.prog->string, str);
   return ETK_TRUE;
}

Etk_Bool
on_SourceEntry_item_changed(Etk_Combobox_Entry *combo, void *data)
{
   Etk_Combobox_Entry_Item *active_item = NULL;
   char *pname;

   printf("Item Changed Signal on SourceEntry Emitted\n");

   if (!(active_item = etk_combobox_entry_active_item_get(combo)))
      return ETK_TRUE;

   etk_combobox_entry_item_fields_get(active_item, NULL, &pname, NULL);

   etk_entry_text_set(ETK_ENTRY(etk_combobox_entry_entry_get(
                      ETK_COMBOBOX_ENTRY(UI_SourceEntry))),pname);

   return ETK_TRUE;
}

Etk_Bool
on_SignalEntry_item_changed(Etk_Combobox_Entry *combo, void *data)
{
   Etk_Combobox_Entry_Item *active_item = NULL;
   char *pname;

   printf("Item Changed Signal on SignalEntry Emitted\n");

   if (!(active_item = etk_combobox_entry_active_item_get(combo)))
      return ETK_TRUE;

   etk_combobox_entry_item_fields_get(active_item, NULL, &pname, NULL);

   etk_entry_text_set(ETK_ENTRY(etk_combobox_entry_entry_get(
                      ETK_COMBOBOX_ENTRY(UI_SignalEntry))),pname);

   return ETK_TRUE;
}

Etk_Bool
on_SignalEntry_text_changed(Etk_Object *object, void *data)
{
   printf("Text Changed Signal on SignalEntry Emitted\n");
   const char *str = etk_entry_text_get(ETK_ENTRY(object));
   edje_edit_program_signal_set(edje_o, Cur.prog->string, str);
   return ETK_TRUE;
}

Etk_Bool
on_DelaySpinners_value_changed(Etk_Range *range, double value, void *data)
{
   printf("value Changed Signal on DelayFromSpinner Emitted\n");
   edje_edit_program_in_from_set(edje_o, Cur.prog->string,
                           etk_range_value_get(ETK_RANGE(UI_DelayFromSpinner)));
   
   edje_edit_program_in_range_set(edje_o, Cur.prog->string,
                           etk_range_value_get(ETK_RANGE(UI_DelayRangeSpinner)));
   
   return ETK_TRUE;
}

Etk_Bool
on_TargetEntry_text_changed(Etk_Object *object, void *data)
{
   char *text = strdup(etk_entry_text_get(ETK_ENTRY(object)));
   char *tok;

   printf("Text Changed Signal on TargetEntry Emitted (text: %s)\n",text);

   //Empty current targets list
   edje_edit_program_targets_clear(edje_o, Cur.prog->string);

   //Spit the string in token and add every targets
   tok = strtok(text,"|");
   while (tok != NULL)
   {
      printf("'%s'\n",tok);
      edje_edit_program_target_add(edje_o, Cur.prog->string, tok);
      tok = strtok(NULL, "|");
   }

   //TODO Check if all the targets exists in the group, otherwise make the text red

   free(text);
   return ETK_TRUE;
}

Etk_Bool
on_Param1Entry_text_changed(Etk_Object *object, void *data)
{
   printf("Text Changed Signal on Param1Entry Emitted\n");

   edje_edit_program_state_set(edje_o, Cur.prog->string,
                               etk_entry_text_get(ETK_ENTRY(UI_Param1Entry)));
   
   return ETK_TRUE;
}

Etk_Bool
on_Param2Entry_text_changed(Etk_Object *object, void *data)
{
   printf("Text Changed Signal on Param2Entry Emitted\n");

   edje_edit_program_state2_set(edje_o, Cur.prog->string,
                               etk_entry_text_get(ETK_ENTRY(UI_Param2Entry)));
   
   return ETK_TRUE;
}

Etk_Bool
on_Param1Spinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("value Changed Signal on Param1Spinner Emitted\n");
   edje_edit_program_value_set(edje_o, Cur.prog->string,
                              etk_range_value_get(ETK_RANGE(UI_Param1Spinner)));
   return ETK_TRUE;
}

Etk_Bool
on_Param2Spinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("value Changed Signal on Param2Spinner Emitted\n");
   edje_edit_program_value2_set(edje_o, Cur.prog->string,
                              etk_range_value_get(ETK_RANGE(UI_Param2Spinner)));
   return ETK_TRUE;
}

Etk_Bool
on_TransitionComboBox_changed(Etk_Combobox *combobox, void *data)
{
   int trans;
   printf("Changed Signal on TransitionComboBox Emitted\n");

   //get the transition from the combo data
   trans = (int)etk_combobox_item_data_get(etk_combobox_active_item_get(combobox));
   edje_edit_program_transition_set(edje_o, Cur.prog->string, trans);
   
   return ETK_TRUE;
}

Etk_Bool
on_DurationSpinner_value_changed(Etk_Range *range, double value, void *data)
{
   printf("value Changed Signal on DurationSpinner Emitted\n");
   edje_edit_program_transition_time_set(edje_o, Cur.prog->string,
                           etk_range_value_get(ETK_RANGE(UI_DurationSpinner)));
   
   return ETK_TRUE;
}

Etk_Bool
on_AfterEntry_text_changed(Etk_Object *object, void *data)
{
   char *text = strdup(etk_entry_text_get(ETK_ENTRY(object)));
   char *tok;

   printf("Text Changed Signal on AfterEntry Emitted (text: %s)\n",text);

   //Empty current afters list
   edje_edit_program_afters_clear(edje_o, Cur.prog->string);
      
   //Spit the string in token and add every afters
   tok = strtok (text,"|");
   while (tok != NULL)
   {
      printf ("'%s'\n",tok);
      edje_edit_program_after_add(edje_o, Cur.prog->string, tok);
      engrave_program_after_add(Cur.epr,tok);
      tok = strtok (NULL, "|");
   }

   //TODO Check if all the after exists in the group, otherwise make the text red

   free(text);
   return ETK_TRUE;
}

/* Colors Callbacks */
Etk_Bool
on_ColorCanvas_realize(Etk_Widget *canvas, void *data)
{
   //Must use the realize callback on the EtkCanvas object.
   //Because I can't add any object to the canvas before it is realized
   Evas_Object* rect;
   //Add the colored rectangle
   rect = evas_object_rectangle_add(etk_widget_toplevel_evas_get(canvas));
   etk_canvas_object_add(ETK_CANVAS(canvas), rect);
   evas_object_color_set(rect, 100,100,100,255);
   evas_object_resize(rect,300,300);
   etk_canvas_object_move(ETK_CANVAS(canvas),rect,0,0);
   evas_object_show(rect);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, on_ColorCanvas_click, data);
   switch ((int)data){
      case COLOR_OBJECT_RECT:
         RectColorObject = rect;
         break;
      case COLOR_OBJECT_TEXT:
         TextColorObject = rect;
         break;
      case COLOR_OBJECT_SHADOW:
         ShadowColorObject = rect;
         break;
      case COLOR_OBJECT_OUTLINE:
         OutlineColorObject = rect;
         break;
   }

   return ETK_TRUE;
}

void
on_ColorCanvas_click(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Etk_Color c;
   printf("Clik Signal on ColorCanvas Emitted\n");
   if (UI_ColorWin) etk_widget_show_all(UI_ColorWin);
   current_color_object = (int)data;

   etk_signal_block("color-changed", ETK_OBJECT(UI_ColorPicker), ETK_CALLBACK(on_ColorDialog_change), NULL);
   switch (current_color_object)
   {
      case COLOR_OBJECT_RECT:
         etk_window_title_set(ETK_WINDOW(UI_ColorWin), "Rectangle color");
         edje_edit_state_color_get(edje_o, Cur.part->string, Cur.state->string, &c.r,&c.g,&c.b,&c.a);
         etk_colorpicker_current_color_set(ETK_COLORPICKER(UI_ColorPicker), c);
         break;
      case COLOR_OBJECT_TEXT:
         etk_window_title_set(ETK_WINDOW(UI_ColorWin), "Text color");
         edje_edit_state_color_get(edje_o, Cur.part->string, Cur.state->string, &c.r,&c.g,&c.b,&c.a);
         etk_colorpicker_current_color_set(ETK_COLORPICKER(UI_ColorPicker), c);
         break;
      case COLOR_OBJECT_SHADOW:
         etk_window_title_set(ETK_WINDOW(UI_ColorWin), "Shadow color");
         edje_edit_state_color3_get(edje_o, Cur.part->string, Cur.state->string, &c.r,&c.g,&c.b,&c.a);
         etk_colorpicker_current_color_set(ETK_COLORPICKER(UI_ColorPicker), c);
         break;
      case COLOR_OBJECT_OUTLINE:
         etk_window_title_set(ETK_WINDOW(UI_ColorWin), "Outline color");
         edje_edit_state_color2_get(edje_o, Cur.part->string, Cur.state->string, &c.r,&c.g,&c.b,&c.a);
         etk_colorpicker_current_color_set(ETK_COLORPICKER(UI_ColorPicker), c);
         break;
   }
   etk_signal_unblock("color-changed", ETK_OBJECT(UI_ColorPicker), ETK_CALLBACK(on_ColorDialog_change), NULL);
}

Etk_Bool
on_ColorDialog_change(Etk_Object *object, void *data)
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

   switch (current_color_object){
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
   }

   ev_redraw();
   return ETK_TRUE;
}

/* Add/Remove Buttons Callbacks */
Etk_Bool
on_AddMenu_item_activated(Etk_Object *object, void *data)
{
   printf("Item Activated Signal on AddMenu EMITTED\n");
   Etk_Tree_Row *row;
   Etk_Combobox_Item *item;
   switch ((int)data)
   {
       case NEW_RECT:
         if (!etk_string_length_get(Cur.group))
         {
            ShowAlert("You must first select a group.");
            break;
         }
         if (!edje_edit_part_add(edje_o, "New rectangle", EDJE_PART_TYPE_RECTANGLE))
         {
            ShowAlert("Can't create part.");
            break;
         }
         //TODO generate a unique new name
         row = AddPartToTree("New rectangle", NULL);
         etk_tree_row_select(row);
         etk_tree_row_unfold(row);
         PopulateRelComboBoxes();
         break;
      
      case NEW_IMAGE:
         if (!etk_string_length_get(Cur.group))
         {
            ShowAlert("You must first select a group.");
            break;
         }
         if (!edje_edit_part_add(edje_o, "New image", EDJE_PART_TYPE_IMAGE))
         {
            ShowAlert("Can't create part.");
            break;
         }
         //TODO generate a unique new name
         row = AddPartToTree("New image", NULL);
         
         char *image;
         item = etk_combobox_first_item_get(ETK_COMBOBOX(UI_ImageComboBox));
         if (item)
         {
            image = etk_combobox_item_field_get(item, 1);
            if (image)
               edje_edit_state_image_set(edje_o, "New image",
                                         "default 0.00", image);
         }
      
         etk_tree_row_select(row);
         etk_tree_row_unfold(row);
         PopulateRelComboBoxes();
         break;
      
      case NEW_TEXT:
         if (!etk_string_length_get(Cur.group))
         {
            ShowAlert("You must first select a group.");
            break;
         }
         if (!edje_edit_part_add(edje_o, "New text", EDJE_PART_TYPE_TEXT))
         {
            ShowAlert("Can't create part.");
            break;
         }
         //TODO generate a unique new name
         row = AddPartToTree("New text", NULL);
         
         char *font;
         item = etk_combobox_first_item_get(ETK_COMBOBOX(UI_FontComboBox));
         if (item)
         {
            font = etk_combobox_item_field_get(item, 1);
            if (font)
               edje_edit_state_font_set(edje_o, "New text",
                                        "default 0.00", font);
         }
         edje_edit_state_text_size_set(edje_o, "New text",
                                       "default 0.00", 16);
         edje_edit_state_text_set(edje_o, "New text",
                                  "default 0.00", "Something to say !");
         edje_edit_part_effect_set(edje_o, "New text", EDJE_TEXT_EFFECT_GLOW);
      
         etk_tree_row_select(row);
         etk_tree_row_unfold(row);
         PopulateRelComboBoxes();
         break;
      
      case NEW_DESC:
         if (!etk_string_length_get(Cur.part))
         {
            ShowAlert("You must first select a part.");
            break;
         }
         
         //Create state
         edje_edit_state_add(edje_o, Cur.part->string, "New state");
         edje_edit_state_rel1_relative_x_set(edje_o, Cur.part->string,
                                    "New state 0.00", 0.1);
         edje_edit_state_rel1_relative_y_set(edje_o, Cur.part->string,
                                    "New state 0.00", 0.1);
         edje_edit_state_rel2_relative_x_set(edje_o, Cur.part->string,
                                    "New state 0.00", 0.9);
         edje_edit_state_rel2_relative_y_set(edje_o, Cur.part->string,
                                    "New state 0.00", 0.9);
         edje_edit_state_text_size_set(edje_o, Cur.part->string,
                                       "New state 0.00", 16);
         //Add state to tree
         row = AddStateToTree(Cur.part->string, "New state 0.00");
         etk_tree_row_select(row);
         etk_tree_row_unfold(evas_hash_find(Parts_Hash,Cur.part->string));
         break;
      
      case NEW_PROG:
         if (!etk_string_length_get(Cur.group))
         {
            ShowAlert("You must first select a group.");
            break;
         }
         if (!edje_edit_program_add(edje_o, "New program"))
         {
            ShowAlert("ERROR: can't add program");
            break;
         }
         row = AddProgramToTree("New program");
         etk_tree_row_select(row);
         etk_tree_row_scroll_to(row, ETK_FALSE);
         break;
      
      case NEW_GROUP:
         if (edje_edit_group_add(edje_o, "New group"))
         {
            PopulateGroupsComboBox();
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_GroupsComboBox),
               etk_combobox_last_item_get(ETK_COMBOBOX(UI_GroupsComboBox)));
         }
         else
         {
            ShowAlert("Can't create group.");
         }
         break;
   }
   ev_redraw();
   return ETK_TRUE;
}

Etk_Bool
on_RemoveMenu_item_activated(Etk_Object *object, void *data)
{
   Etk_Tree_Row *row, *next;
   printf("Item Activated Signal on RemoveMenu EMITTED\n");

   switch ((int)data)
   {
      case REMOVE_DESCRIPTION:
         if (!etk_string_length_get(Cur.state))
         {
            ShowAlert("No part state selected");
            break;
         }
         if (!strcmp(Cur.state->string,"default 0.00"))
         {
            ShowAlert("You can't remove default 0.0");
            break;
         }
         edje_edit_state_del(edje_o, Cur.part->string, Cur.state->string);
         
         // Select next row (if no exist select prev); and delete current.
         row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
         next = etk_tree_row_next_get(row);
         if (!next) 
            next = etk_tree_row_prev_get(row);
         etk_tree_row_select(next);
         etk_tree_row_delete(row);
      
         break;
      case REMOVE_PART:
         if (!etk_string_length_get(Cur.part))
         {
            ShowAlert("No part selected");
            break;
         }
         if (!edje_edit_part_del(edje_o, Cur.part->string))
         {
            ShowAlert("Can't delete part");
            break;
         }
         
         row = evas_hash_find(Parts_Hash, Cur.part->string);
         Parts_Hash = evas_hash_del(Parts_Hash, Cur.part->string, NULL);
         
         next = etk_tree_row_next_get(row);
         if (!next) 
            next = etk_tree_row_prev_get(row);
         etk_tree_row_delete(row);
         if (next)
            etk_tree_row_select(next);
         else
         {
            Cur.part = etk_string_clear(Cur.part);
            Cur.state = etk_string_clear(Cur.state);
         }
         
         PopulateRelComboBoxes();
         //ev_redraw();
         break;
      
      case REMOVE_GROUP:
         if (!edje_edit_group_del(edje_o))
         {
            ShowAlert("Can't delete group");
            break;
         }
         Etk_Combobox_Item *item, *nitem;
         item = etk_combobox_active_item_get(ETK_COMBOBOX(UI_GroupsComboBox));
         
         nitem = etk_combobox_item_next_get(item);
         if (!nitem)
            nitem = etk_combobox_item_prev_get(item);
         
         etk_combobox_active_item_set(ETK_COMBOBOX(UI_GroupsComboBox), nitem);
         etk_combobox_item_remove(item);
         break;
      
      case REMOVE_PROG:
         if (!etk_string_length_get(Cur.prog))
         {
            ShowAlert("You must first select a program");
         }
         if (!edje_edit_program_del(edje_o, Cur.prog->string))
         {
            ShowAlert("Can't delete program");
            break;
         }
         row = etk_tree_selected_row_get(ETK_TREE(UI_PartsTree));
         next = etk_tree_row_next_get(row);
         if (!next) 
            next = etk_tree_row_prev_get(row);
         etk_tree_row_delete(row);
         if (next)
            etk_tree_row_select(next);
      
         break;
   }
   return ETK_TRUE;
}

/* Dialogs Callbacks */
Etk_Bool
on_FileChooserDialog_response(Etk_Dialog *dialog, int response_id, void *data)
{
   char cmd[4096];

   printf("Response Signal on Filechooser EMITTED\n");

   if (response_id == ETK_RESPONSE_OK){

      switch(FileChooserOperation){
         case FILECHOOSER_OPEN:
            snprintf(cmd,4096,"%s/%s",
            etk_filechooser_widget_current_folder_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)),
            etk_filechooser_widget_selected_file_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)));
            LoadEDJ(cmd);
         break;
         case FILECHOOSER_SAVE_EDJ:
            printf("SAVE EDJ\n");
            snprintf(cmd,4096,"%s/%s",
               etk_filechooser_widget_current_folder_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)),
               etk_filechooser_widget_selected_file_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)));
            edje_edit_save(edje_o);
            if(!ecore_file_cp(Cur.edj_temp_name->string, cmd))
            {
               ShowAlert("<b>ERROR:<\b><br>Can't write file");
            }
            else
            {
               Cur.edj_file_name = etk_string_set(Cur.edj_file_name, cmd);
               ecore_evas_title_set(UI_ecore_MainWin, cmd);
            }
         break;
         case FILECHOOSER_SAVE_EDC:
              ShowAlert("Not yet implemented.");
         break;
         case FILECHOOSER_IMAGE:
            snprintf(cmd, 4096, "%s/%s", 
               etk_filechooser_widget_current_folder_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)),
               etk_filechooser_widget_selected_file_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)));
            if (!edje_edit_image_add(edje_o, cmd))
            {
               ShowAlert("ERROR: Can't import image file.");
               break;
            }
            PopulateImagesComboBox();
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_ImageComboBox),
                  etk_combobox_last_item_get(ETK_COMBOBOX(UI_ImageComboBox)));
         break;
         case FILECHOOSER_FONT:
            snprintf(cmd, 4096, "%s/%s", 
               etk_filechooser_widget_current_folder_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)),
               etk_filechooser_widget_selected_file_get(ETK_FILECHOOSER_WIDGET(UI_FileChooser)));
            if (!edje_edit_font_add(edje_o, cmd))
            {
               ShowAlert("ERROR: Can't import font file.");
               break;
            }
            PopulateFontsComboBox();
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_FontComboBox),
                  etk_combobox_last_item_get(ETK_COMBOBOX(UI_FontComboBox)));
         break;
      }
      etk_widget_hide(ETK_WIDGET(dialog));
   }
   else{
      etk_widget_hide(ETK_WIDGET(dialog));
   }

   return ETK_TRUE;
}

Etk_Bool
on_FileChooser_selected(Etk_Filechooser_Widget *filechooser)
{
   printf("*** FILECHOOSER SELECTD ON *** \n");
   on_FileChooserDialog_response(ETK_DIALOG(UI_FileChooserDialog), ETK_RESPONSE_OK, NULL);
   return ETK_TRUE;
}

Etk_Bool
on_AlertDialog_response(Etk_Dialog *dialog, int response_id, void *data)
{
   etk_widget_hide(ETK_WIDGET(dialog));
   return ETK_TRUE;
}
