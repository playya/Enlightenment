#include <dirent.h>
#include <string.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include <Etk.h>
#include "main.h"
#include "callbacks.h"
#include "interface.h"
#include "evas.h"

void
ConsolleClear(void)
{
   edje_object_part_text_set(Consolle, "line1", "");
   edje_object_part_text_set(Consolle, "line2", "");
   edje_object_part_text_set(Consolle, "line3", "");
   edje_object_part_text_set(Consolle, "line4", "");
   edje_object_part_text_set(Consolle, "line5", "");
   
   while(stack)
   {
      evas_stringshare_del(evas_list_data(stack));
      stack = evas_list_remove_list(stack, stack);
   }
   consolle_count = 0;
}
void
ConsolleLog(char *text)
{
   //printf("LOG: %s\n", text);
   
   stack = evas_list_prepend(stack, evas_stringshare_add(text));
   
   while (evas_list_count(stack) > 5)
   {
      evas_stringshare_del(evas_list_data(evas_list_last(stack)));
      stack = evas_list_remove_list(stack, evas_list_last(stack));
   }

   edje_object_part_text_set(Consolle, "line1", evas_list_nth(stack, 0));
   edje_object_part_text_set(Consolle, "line2", evas_list_nth(stack, 1));
   edje_object_part_text_set(Consolle, "line3", evas_list_nth(stack, 2));
   edje_object_part_text_set(Consolle, "line4", evas_list_nth(stack, 3));
   edje_object_part_text_set(Consolle, "line5", evas_list_nth(stack, 4));
}

void
TogglePlayButton(int set)
{
   /* set    -1 = toggle   0 = pause   1 = play   */
   if (set == -1)
      set = !edje_object_play_get(edje_o);
   
   if (set == 0)
   {
      edje_object_play_set(edje_o, 0);
      etk_button_image_set(ETK_BUTTON(UI_PlayButton), ETK_IMAGE(UI_PlayImage));
      etk_object_properties_set(ETK_OBJECT(UI_PlayButton),"label","Play Edje",NULL);
   }
   else if (set == 1)
   {
      edje_object_play_set(edje_o, 1);
      etk_button_image_set(ETK_BUTTON(UI_PlayButton), ETK_IMAGE(UI_PauseImage));
      etk_object_properties_set(ETK_OBJECT(UI_PlayButton),"label","Pause Edje",NULL);
   }
}

void
ShowAlert(char* text)
{
   etk_message_dialog_text_set(ETK_MESSAGE_DIALOG(UI_AlertDialog), text);
   etk_widget_show_all(UI_AlertDialog);
}

void
ShowFilechooser(int FileChooserType)
{
   etk_widget_show_all(UI_FileChooserDialog);
   
   FileChooserOperation = FileChooserType;
   switch(FileChooserType){
      case FILECHOOSER_OPEN:
         etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an EDJ or EDC file to open");
         etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         etk_widget_hide(UI_FilechooserSaveButton);
      break;
      case FILECHOOSER_IMAGE:
         etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an image to import");
         etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         etk_widget_hide(UI_FilechooserSaveButton);
      break;
      case FILECHOOSER_FONT:
         etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose an font to import");
         etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_FALSE);
         etk_widget_hide(UI_FilechooserSaveButton);
      break;
      case FILECHOOSER_SAVE_EDJ:
         etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose the new edje name");
         etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_TRUE);
         etk_widget_hide(UI_FilechooserLoadButton);
      break;
      case FILECHOOSER_SAVE_EDC:
         etk_window_title_set(ETK_WINDOW(UI_FileChooserDialog), "Choose the new edc name");
         etk_filechooser_widget_is_save_set(ETK_FILECHOOSER_WIDGET(UI_FileChooser),ETK_TRUE);
         etk_widget_hide(UI_FilechooserLoadButton);
      break;
      default:
      break;
   }
}
void
ShowImageBrowser(int UpdateCurrent)
{
   ImageBroserUpdate = UpdateCurrent;
   PopulateImageBrowser();
   UpdateImageBrowser();
   etk_widget_show_all(UI_ImageBrowserWin);
}
/***********************************
 *
 * functions to update interface
 *
 ***********************************/

char*
GetPartTypeImage(int part_type)
{
   /* Get the name ot the group in edje_editor.edj that
    * correspond to the given EDJE_PART_TYPE.
    * Remember to free the returned string.
    */
   
   static char buf[20];
   char *ret;
   
   switch (part_type)
   {
      case EDJE_PART_TYPE_IMAGE:     strcpy(buf,"IMAGE.PNG"); break;
      case EDJE_PART_TYPE_TEXT:      strcpy(buf,"TEXT.PNG");  break;
      case EDJE_PART_TYPE_RECTANGLE: strcpy(buf,"RECT.PNG");  break;
      case EDJE_PART_TYPE_SWALLOW:   strcpy(buf,"SWAL.PNG");  break;
      case EDJE_PART_TYPE_GROUP:     strcpy(buf,"GROUP.PNG");  break;
      default:                       strcpy(buf,"NONE.PNG");  break;
   }
   ret = strdup(buf);
   printf("IMAGE: %s\n",ret);
   return ret;
}
Etk_Tree_Row *
AddPartToTree(const char *part_name, Etk_Tree_Row *after)
{
   /* If after=0 then append to the tree (but before programs)
      If after=1 then prepend to the tree
      If after>1 then prepend relative to after
      
      I hope no one get a real row pointer == 1  :P
   */
   Etk_Tree_Row *row = NULL;
   char *buf;

   //printf("Add Part to tree: %s\n",par->name);
   
   /* Search for the last row that isn't a program */
   if (after == 0)
   {
      int row_type;
      
      after = etk_tree_last_row_get(ETK_TREE(UI_PartsTree));
      etk_tree_row_fields_get(after, COL_TYPE, &row_type, NULL);
      
      while (after && row_type && row_type == ROW_PROG)
      {
         after = etk_tree_row_prev_get(after);
         etk_tree_row_fields_get(after, COL_TYPE, &row_type, NULL);
      }
   }
   
   /* Add the row in the right position */
   buf = GetPartTypeImage(edje_edit_part_type_get(edje_o, part_name));
   if ((int)after > 1)
      row = etk_tree_row_insert(ETK_TREE(UI_PartsTree),
                                NULL,
                                after,
                                COL_NAME, EdjeFile, buf, part_name,
                                COL_TYPE, ROW_PART,
                                NULL);
   else if ((int)after == 1)
      row = etk_tree_row_prepend(ETK_TREE(UI_PartsTree),
                                NULL,
                                COL_NAME, EdjeFile, buf, part_name,
                                COL_TYPE, ROW_PART,
                                NULL);
   else
      row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
                                NULL,
                                COL_NAME, EdjeFile, buf, part_name,
                                COL_TYPE, ROW_PART,
                                NULL);

   Parts_Hash = evas_hash_add(Parts_Hash, part_name, row);
   
   /* also add all state to the tree */
   Evas_List *states, *sp;
   states = sp = edje_edit_part_states_list_get(edje_o, part_name);
   while(sp)
   {
      AddStateToTree(part_name, (char*)sp->data);
      sp = sp->next;
   }
   edje_edit_string_list_free(states);
   free(buf);
   
   return row;
}

Etk_Tree_Row *
AddStateToTree(const char *part_name, const char *state_name)
{
   Etk_Tree_Row *row;

   const char *stock_key;

   stock_key = etk_stock_key_get(ETK_STOCK_TEXT_X_GENERIC, ETK_STOCK_SMALL);
   row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
            evas_hash_find(Parts_Hash,part_name),
            COL_NAME, EdjeFile, "DESC.PNG", state_name,
            COL_VIS, TRUE,
            COL_TYPE, ROW_DESC,
            COL_PARENT, part_name, NULL);
   return row;
}

Etk_Tree_Row *
AddProgramToTree(const char* prog)
{
   Etk_Tree_Row *row = NULL;

   //printf("Add Program to tree: %s\n",prog->name);
   row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
               NULL,
               COL_NAME, EdjeFile,"PROG.PNG", prog,
               COL_TYPE,ROW_PROG,
               NULL);

   return row;
}
void 
PopulateTree(void)
{
   Evas_List *parts, *pp;
   Evas_List *progs;
   
   etk_tree_freeze(ETK_TREE(UI_PartsTree));
   etk_tree_clear(ETK_TREE(UI_PartsTree));
        
   parts = pp = edje_edit_parts_list_get(edje_o);
   while(pp)
   {
      printf("  P: %s\n", (char*)pp->data);
      AddPartToTree((char*)pp->data, NULL);
      pp = pp->next;
   }
   edje_edit_string_list_free(parts);
   
   progs = pp = edje_edit_programs_list_get(edje_o);
   while(pp)
   {
      AddProgramToTree((char*)pp->data);
      pp = pp->next;
   }
   edje_edit_string_list_free(progs);
   
   etk_tree_row_select(etk_tree_first_row_get (ETK_TREE(UI_PartsTree)));
   etk_tree_thaw(ETK_TREE(UI_PartsTree));
   
}
void
PopulateGroupsComboBox(void)
{
   Evas_List *groups, *l;
   
   //Stop signal propagation
   etk_signal_block("item-activated",ETK_OBJECT(UI_GroupsComboBox),
                    on_GroupsComboBox_activated, NULL);
   etk_signal_block("item-activated",ETK_OBJECT(UI_PartSourceComboBox),
                    on_PartSourceComboBox_item_activated, NULL);
   
   //Clear the combos
   etk_combobox_clear(ETK_COMBOBOX(UI_GroupsComboBox));
   etk_combobox_clear(ETK_COMBOBOX(UI_PartSourceComboBox));
   etk_combobox_item_append(ETK_COMBOBOX(UI_PartSourceComboBox), "None");
   
   //Populate UI_GroupsComboBox & UI_PartSourceComboBox
   groups = edje_file_collection_list(Cur.edj_temp_name->string);
   for(l = groups; l; l = l->next)
   {
      etk_combobox_item_append(ETK_COMBOBOX(UI_GroupsComboBox), (char*)l->data);
      etk_combobox_item_append(ETK_COMBOBOX(UI_PartSourceComboBox), (char*)l->data);
   }
   edje_file_collection_list_free(groups);
    
   //Renable  signal propagation
   etk_signal_unblock("item-activated",ETK_OBJECT(UI_GroupsComboBox),
                      on_GroupsComboBox_activated, NULL);
   etk_signal_unblock("item-activated",ETK_OBJECT(UI_PartSourceComboBox),
                      on_PartSourceComboBox_item_activated, NULL);
   
   //Select the first group and load it
   etk_combobox_active_item_set(ETK_COMBOBOX(UI_GroupsComboBox),
      etk_combobox_first_item_get(ETK_COMBOBOX(UI_GroupsComboBox)));
}

void 
PopulateFontsComboBox(void)
{
   Evas_List *l;
   Etk_Combobox_Item *ComboItem;

   //Stop signal propagation
   etk_signal_disconnect("item-activated", ETK_OBJECT(UI_FontComboBox), ETK_CALLBACK(on_FontComboBox_item_activated), NULL);

   printf("Populate Fonts Combo\n");

   etk_combobox_clear(ETK_COMBOBOX(UI_FontComboBox));

   Evas_List *fonts;
   fonts = l = edje_edit_fonts_list_get(edje_o);
   while (l)
   {
      ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_FontComboBox),
                     etk_image_new_from_stock(ETK_STOCK_PREFERENCES_DESKTOP_FONT,ETK_STOCK_SMALL), 
                     l->data);
      l = l->next;
   }
   edje_edit_string_list_free(fonts);

   //Renable  signal propagation
   etk_signal_connect("item-activated", ETK_OBJECT(UI_FontComboBox), ETK_CALLBACK(on_FontComboBox_item_activated), NULL);

}

void
PopulateRelComboBoxes(void)
{
   Evas_List *l;
   char *image_name;
   
   printf("Populate 4 Rel Comboboxs\n");
   //Stop signal propagation
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel1ToXComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1X_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel1ToYComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1Y_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel2ToXComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2X_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel2ToYComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2Y_SPINNER);
   etk_signal_block("item-activated", ETK_OBJECT(UI_CliptoComboBox), ETK_CALLBACK(on_CliptoComboBox_item_activated), NULL);


   etk_combobox_clear(ETK_COMBOBOX(UI_Rel1ToXComboBox));
   etk_combobox_clear(ETK_COMBOBOX(UI_Rel1ToYComboBox));
   etk_combobox_clear(ETK_COMBOBOX(UI_Rel2ToXComboBox));
   etk_combobox_clear(ETK_COMBOBOX(UI_Rel2ToYComboBox));
   etk_combobox_clear(ETK_COMBOBOX(UI_CliptoComboBox));

   if (etk_string_length_get(Cur.group))
   {
      // Add first element 'Interface' to all the 4 combobox
      etk_combobox_item_append(ETK_COMBOBOX(UI_Rel1ToXComboBox),
                               etk_image_new_from_edje(EdjeFile,"NONE.PNG"),
                               "Interface");
      etk_combobox_item_append(ETK_COMBOBOX(UI_Rel1ToYComboBox),
                               etk_image_new_from_edje(EdjeFile,"NONE.PNG"),
                               "Interface");
      etk_combobox_item_append(ETK_COMBOBOX(UI_Rel2ToXComboBox),
                               etk_image_new_from_edje(EdjeFile,"NONE.PNG"),
                               "Interface");
      etk_combobox_item_append(ETK_COMBOBOX(UI_Rel2ToYComboBox),
                               etk_image_new_from_edje(EdjeFile,"NONE.PNG"),
                               "Interface");
      etk_combobox_item_append(ETK_COMBOBOX(UI_CliptoComboBox), 
                               etk_image_new_from_edje(EdjeFile,"NONE.PNG"),
                               "None");
   
      // Add all the part to all the 4 combobox
      Evas_List *parts;
      int type;
      
      parts = l = edje_edit_parts_list_get(edje_o);
      while (l)
      {
         //printf("-- %s\n", (char *)l->data);
         type = edje_edit_part_type_get(edje_o,(char *)l->data);
         image_name = GetPartTypeImage(type);
         
         etk_combobox_item_append(ETK_COMBOBOX(UI_Rel1ToXComboBox),
                                  etk_image_new_from_edje(EdjeFile, image_name),
                                  (char *)l->data);
         etk_combobox_item_append(ETK_COMBOBOX(UI_Rel1ToYComboBox),
                                  etk_image_new_from_edje(EdjeFile, image_name),
                                  (char *)l->data);
         etk_combobox_item_append(ETK_COMBOBOX(UI_Rel2ToXComboBox),
                                  etk_image_new_from_edje(EdjeFile, image_name),
                                  (char *)l->data);
         etk_combobox_item_append(ETK_COMBOBOX(UI_Rel2ToYComboBox),
                                  etk_image_new_from_edje(EdjeFile, image_name),
                                  (char *)l->data);
         etk_combobox_item_append(ETK_COMBOBOX(UI_CliptoComboBox),
                                  etk_image_new_from_edje(EdjeFile, image_name),
                                  (char *)l->data);
         free(image_name);
         l = l->next;
      }
      edje_edit_string_list_free(parts);
   }

   //Reenable signal propagation
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel1ToXComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1X_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel1ToYComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1Y_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel2ToXComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2X_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel2ToYComboBox), ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2Y_SPINNER);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_CliptoComboBox), ETK_CALLBACK(on_CliptoComboBox_item_activated), NULL);
}

void
PopulateTweenList(void)
{
   Evas_List *tweens, *l;
   Etk_Tree_Col *col;
   
   if (!etk_string_length_get(Cur.state)) return;
   if (!etk_string_length_get(Cur.part)) return;
   
   col = etk_tree_nth_col_get(ETK_TREE(UI_ImageTweenList), 0);
   
   etk_tree_clear(ETK_TREE(UI_ImageTweenList));
   
   tweens = l = edje_edit_state_tweens_list_get(edje_o, Cur.part->string, Cur.state->string);
   while (l)
   {
      //printf("RET: %s (id: %d)\n", l->data, edje_edit_image_id_get(edje_o, l->data));
      //snprintf(buf, sizeof(buf), "images/%d", edje_edit_image_id_get(edje_o, l->data)); TODO: find a way to append image directly from the edje file.
      etk_tree_row_append(ETK_TREE(UI_ImageTweenList), NULL,
                          col, NULL, NULL, l->data,
                          NULL);
      l = l->next;
   }
   edje_edit_string_list_free(tweens);
}

void
PopulateSourceComboEntry(void)
{
   Evas_List *l;
   char *image_name;
   printf("Populate Program Source ComboEntry\n");
    
   //Stop signal propagation
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_SourceEntry),
                  ETK_CALLBACK(on_SourceEntry_item_changed), NULL);
   
   etk_combobox_entry_clear(ETK_COMBOBOX_ENTRY(UI_SourceEntry));
   
   l = edje_edit_parts_list_get(edje_o);
   while (l)
   {
      image_name = GetPartTypeImage(edje_edit_part_type_get(edje_o, (char*)l->data));
      etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SourceEntry),
                  etk_image_new_from_edje(EdjeFile, image_name),
                  (char *)l->data);
      free(image_name);
      
      l = l->next;
   }
   edje_edit_string_list_free(l);
   
   //Renable  signal propagation
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_SourceEntry),
                     ETK_CALLBACK(on_SourceEntry_item_changed), NULL);
}

void
PopulateSignalComboEntry(void)
{
   printf("Populate Program Signal ComboEntry\n");
   
   //Stop signal propagation
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_SignalEntry),
                  ETK_CALLBACK(on_SignalEntry_item_changed), NULL);
   
   etk_combobox_entry_clear(ETK_COMBOBOX_ENTRY(UI_SignalEntry));
      
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "program,start");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "program,stop");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "load");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "show");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "hide");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "resize");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,in");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,out");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,move");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,down,1");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,up,1");
   etk_combobox_entry_item_append(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
            etk_image_new_from_edje(EdjeFile, "DESC.PNG"), "mouse,clicked,1");
   
   //Renable  signal propagation
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_SignalEntry),
                     ETK_CALLBACK(on_SignalEntry_item_changed), NULL);
}



void
PopulateImageBrowser(void)
{
   Evas_List *l, *images;
   char buf[4096];
   
   etk_iconbox_clear(ETK_ICONBOX(UI_ImageBrowserIconbox));
   
   etk_iconbox_freeze(ETK_ICONBOX(UI_ImageBrowserIconbox));
   images = l = edje_edit_images_list_get(edje_o);
   while (l)
   {
      snprintf(buf,4096,"images/%d",edje_edit_image_id_get(edje_o, (char*)l->data));
      etk_iconbox_append(ETK_ICONBOX(UI_ImageBrowserIconbox),
                         Cur.edj_temp_name->string, buf, (char*)l->data);
      l = l->next;
   }
   edje_edit_string_list_free(images);
   etk_iconbox_thaw(ETK_ICONBOX(UI_ImageBrowserIconbox));
}

void
UpdateGroupFrame(void)
{
   //Stop signal propagation
   etk_signal_block("text-changed", ETK_OBJECT(UI_GroupNameEntry),
                    on_NamesEntry_text_changed, NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_GroupMinWSpinner),
                    ETK_CALLBACK(on_GroupSpinner_value_changed),
                    (void *)MINW_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_GroupMinHSpinner),
                    ETK_CALLBACK(on_GroupSpinner_value_changed),
                    (void *)MINH_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_GroupMaxWSpinner),
                    ETK_CALLBACK(on_GroupSpinner_value_changed),
                    (void *)MAXW_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_GroupMaxHSpinner),
                    ETK_CALLBACK(on_GroupSpinner_value_changed),
                    (void *)MAXH_SPINNER);

   if (etk_string_length_get(Cur.group))
   {
      //Update name
      etk_entry_text_set(ETK_ENTRY(UI_GroupNameEntry),Cur.group->string);
      etk_widget_hide(ETK_WIDGET(UI_GroupNameEntryImage));
   
      //Update min e max spinners
      etk_range_value_set(ETK_RANGE(UI_GroupMinWSpinner),
                          (float)edje_edit_group_min_w_get(edje_o));
      etk_range_value_set(ETK_RANGE(UI_GroupMinHSpinner),
                          (float)edje_edit_group_min_h_get(edje_o));
      etk_range_value_set(ETK_RANGE(UI_GroupMaxWSpinner),
                          (float)edje_edit_group_max_w_get(edje_o));
      etk_range_value_set(ETK_RANGE(UI_GroupMaxHSpinner),
                          (float)edje_edit_group_max_h_get(edje_o));
   }

   //ReEnable Signal Propagation
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_GroupNameEntry),
                      on_NamesEntry_text_changed, NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_GroupMinWSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MINW_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_GroupMinHSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MINH_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_GroupMaxWSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MAXW_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_GroupMaxHSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MAXH_SPINNER);

}

void
UpdatePartFrame(void)
{
   //Stop signal propagation
   etk_signal_block("text-changed",ETK_OBJECT(UI_PartNameEntry),
                    on_NamesEntry_text_changed, NULL);
   etk_signal_block("toggled",ETK_OBJECT(UI_PartEventsCheck),
                    on_PartEventsCheck_toggled, NULL);
   etk_signal_block("toggled",ETK_OBJECT(UI_PartEventsRepeatCheck),
                    on_PartEventsRepeatCheck_toggled, NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                    ETK_CALLBACK(on_CliptoComboBox_item_activated), NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                    ETK_CALLBACK(on_PartSourceComboBox_item_activated), NULL);

   if (etk_string_length_get(Cur.part))
   {
      etk_entry_text_set(ETK_ENTRY(UI_PartNameEntry), Cur.part->string);
      etk_widget_hide(ETK_WIDGET(UI_PartNameEntryImage));
      
      etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartEventsCheck),
                     edje_edit_part_mouse_events_get(edje_o, Cur.part->string));
      etk_toggle_button_active_set(ETK_TOGGLE_BUTTON(UI_PartEventsRepeatCheck),
                     edje_edit_part_repeat_events_get(edje_o, Cur.part->string));

   
      /* Update clip_to combobox */
      Etk_Combobox_Item *item = NULL;
      const char *clipto;
      int i;
      char *p;
      
      clipto = edje_edit_part_clip_to_get(edje_o, Cur.part->string);
         
      if (clipto)
      {
         //Loop for all the item in the Combobox
         i=1;
         while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_CliptoComboBox),i)))
         {
            p = etk_combobox_item_field_get(item, 1);
            if (!strcmp(p, clipto))
               etk_combobox_active_item_set(ETK_COMBOBOX(UI_CliptoComboBox),item);
            i++;
         }
      }
      else
         etk_combobox_active_item_set(ETK_COMBOBOX(UI_CliptoComboBox),
               etk_combobox_first_item_get(ETK_COMBOBOX(UI_CliptoComboBox)));
      edje_edit_string_free(clipto);
      
      
      /* Update PartSource combobox */
      const char *source;
      source = edje_edit_part_source_get(edje_o, Cur.part->string);
         
      if (source)
      {
         //Loop for all the item in the Combobox
         i=1;
         while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_PartSourceComboBox),i)))
         {
            p = etk_combobox_item_field_get(item, 0);
            if (!strcmp(p, source))
               etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),item);
            i++;
         }
      }
      else
         etk_combobox_active_item_set(ETK_COMBOBOX(UI_PartSourceComboBox),
               etk_combobox_first_item_get(ETK_COMBOBOX(UI_PartSourceComboBox)));
      
      edje_edit_string_free(source);
   }
   
   //Show/hide Sourcecombo for part EDJE_PART_TYPE_GROUP
   if (edje_edit_part_type_get(edje_o, Cur.part->string) == EDJE_PART_TYPE_GROUP)
   {
      etk_widget_show(UI_PartSourceComboBox);
      etk_widget_show(UI_PartSourceLabel);
   }
   else
   {
      etk_widget_hide(UI_PartSourceComboBox);
      etk_widget_hide(UI_PartSourceLabel);
   }
   
   //ReEnable Signal Propagation
   etk_signal_unblock("text-changed",ETK_OBJECT(UI_PartNameEntry),
                      on_NamesEntry_text_changed, NULL);
   etk_signal_unblock("toggled",ETK_OBJECT(UI_PartEventsCheck),
                      on_PartEventsCheck_toggled, NULL);
   etk_signal_unblock("toggled",ETK_OBJECT(UI_PartEventsRepeatCheck),
                      on_PartEventsRepeatCheck_toggled, NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                      ETK_CALLBACK(on_CliptoComboBox_item_activated), NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                      ETK_CALLBACK(on_PartSourceComboBox_item_activated), NULL);


}

void
UpdateDescriptionFrame(void)
{
  //Stop signal propagation
   etk_signal_block("text-changed", ETK_OBJECT(UI_StateEntry),
                    on_NamesEntry_text_changed, NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_AspectMinSpinner),
                    ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_AspectMaxSpinner),
                    ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_AspectComboBox),
                    ETK_CALLBACK(on_AspectComboBox_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateMinWSpinner),
                    ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateMinHSpinner),
                    ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateMaxWSpinner),
                    ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateMaxHSpinner),
                    ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateAlignVSpinner),
                    ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                    (void*)STATE_ALIGNV_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_StateAlignHSpinner),
                    ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                    (void*)STATE_ALIGNH_SPINNER);

   if (etk_string_length_get(Cur.state))
   {
      //Set description name & index
      etk_entry_text_set(ETK_ENTRY(UI_StateEntry),Cur.state->string);
      etk_widget_hide(ETK_WIDGET(UI_StateEntryImage));
      if (!strcmp(Cur.state->string, "default 0.00"))
         etk_widget_disabled_set(ETK_WIDGET(UI_StateEntry), ETK_TRUE);
      else
         etk_widget_disabled_set(ETK_WIDGET(UI_StateEntry), ETK_FALSE);
      
      //Set aspect min & max
      etk_range_value_set(ETK_RANGE(UI_AspectMinSpinner),
         edje_edit_state_aspect_min_get(edje_o, Cur.part->string, Cur.state->string));
      etk_range_value_set(ETK_RANGE(UI_AspectMaxSpinner),
         edje_edit_state_aspect_max_get(edje_o, Cur.part->string, Cur.state->string));
      
      //Set aspect pref Combo
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_AspectComboBox),
         etk_combobox_nth_item_get(ETK_COMBOBOX(UI_AspectComboBox), 
            edje_edit_state_aspect_pref_get(edje_o, Cur.part->string, Cur.state->string)));
      
      //Set min e max size
      etk_range_value_set(ETK_RANGE(UI_StateMinWSpinner),
         edje_edit_state_min_w_get(edje_o, Cur.part->string, Cur.state->string));
      etk_range_value_set(ETK_RANGE(UI_StateMinHSpinner),
         edje_edit_state_min_h_get(edje_o, Cur.part->string, Cur.state->string));
      etk_range_value_set(ETK_RANGE(UI_StateMaxWSpinner),
         edje_edit_state_max_w_get(edje_o, Cur.part->string, Cur.state->string));
      etk_range_value_set(ETK_RANGE(UI_StateMaxHSpinner),
         edje_edit_state_max_h_get(edje_o, Cur.part->string, Cur.state->string));
      
      
      //Set description align & valign
      etk_range_value_set(ETK_RANGE(UI_StateAlignHSpinner),
         edje_edit_state_align_x_get(edje_o, Cur.part->string, Cur.state->string));
      etk_range_value_set(ETK_RANGE(UI_StateAlignVSpinner),
         edje_edit_state_align_y_get(edje_o, Cur.part->string, Cur.state->string));
   }

   //ReEnable Signal Propagation
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_StateEntry),
                      on_NamesEntry_text_changed, NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_AspectMinSpinner),
                      ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_AspectMaxSpinner),
                      ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_AspectComboBox),
                      ETK_CALLBACK(on_AspectComboBox_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateMinWSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateMinHSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateMaxWSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateMaxHSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateAlignVSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                      (void*)STATE_ALIGNV_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_StateAlignHSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                      (void*)STATE_ALIGNH_SPINNER);

}

void
UpdateRectFrame(void)
{
   Etk_Color color;

   if (etk_string_length_get(Cur.state))
   {
      edje_edit_state_color_get(edje_o, Cur.part->string, Cur.state->string,
                                &color.r, &color.g, &color.b, &color.a);

      etk_signal_block("color-changed", ETK_OBJECT(UI_ColorPicker),
                       ETK_CALLBACK(on_ColorDialog_change), NULL);

      //Set ColorPicker
      etk_colorpicker_current_color_set (ETK_COLORPICKER(UI_ColorPicker), color);
      //Set Color rect
      evas_color_argb_premul(color.a,&color.r,&color.g,&color.b);
      evas_object_color_set(RectColorObject,color.r,color.g,color.b,color.a);

      etk_signal_unblock("color-changed", ETK_OBJECT(UI_ColorPicker),
                         ETK_CALLBACK(on_ColorDialog_change), NULL);
   }

}

void
UpdateImageBrowser(void)
{
   const char *pi;
   char buf[4096];
   Etk_Iconbox_Icon *icon;
   
   if (ImageBroserUpdate)
   {
      //window title
      snprintf(buf, 4096, "Choose image for part \"%s\" state: \"%s\"",Cur.part->string, Cur.state->string);
      etk_window_title_set(ETK_WINDOW(UI_ImageBrowserWin), buf);
      
      //iconbox
      etk_iconbox_unselect_all(ETK_ICONBOX(UI_ImageBrowserIconbox));
      pi = edje_edit_state_image_get(edje_o, Cur.part->string, Cur.state->string);
      if (pi)
      {
         icon = etk_iconbox_icon_get_by_label( ETK_ICONBOX(UI_ImageBrowserIconbox), pi);
         etk_iconbox_icon_select(icon);
      }
      //preview
      //~ snprintf(buf, 4096, "images/%d", edje_edit_image_id_get(edje_o, pi));
      //~ printf("IM: %s\n", buf);
      //~ etk_image_set_from_file(UI_ImageBrowserPreview, Cur.edj_temp_name->string, buf);
     
      //~ edje_edit_string_free(pi);
      
      //int w, h;
      //etk_image_size_get (etk_image_new_from_file(icon->filename, icon->edje_group), &w, &h);
      //printf("W: %d H: %d (%d)\n", w, h, etk_image_new_from_file(icon->filename, icon->edje_group));
      //etk_label_set(UI_ImageBrowserSizeLabel, "ciccio");
      
   }
   else
   {
      etk_window_title_set(ETK_WINDOW(UI_ImageBrowserWin), "Image Browser");
   }

}

void
UpdateImageFrame(void)
{
   //Etk_Combobox_Item *item = NULL;
   const char *im;
    
   //Stop signal propagation
   etk_signal_block("value-changed",ETK_OBJECT(UI_BorderLeftSpinner),
                    ETK_CALLBACK(on_BorderSpinner_value_changed), (void*)BORDER_LEFT);
   etk_signal_block("value-changed",ETK_OBJECT(UI_BorderRightSpinner),
                    ETK_CALLBACK(on_BorderSpinner_value_changed), (void*)BORDER_RIGHT);
   etk_signal_block("value-changed",ETK_OBJECT(UI_BorderTopSpinner),
                    ETK_CALLBACK(on_BorderSpinner_value_changed), (void*)BORDER_TOP);
   etk_signal_block("value-changed",ETK_OBJECT(UI_BorderBottomSpinner),
                    ETK_CALLBACK(on_BorderSpinner_value_changed), (void*)BORDER_BOTTOM);
   etk_signal_block("value-changed",ETK_OBJECT(UI_ImageAlphaSlider),
                    ETK_CALLBACK(on_ImageAlphaSlider_value_changed), NULL);
   etk_signal_block("icon-selected", ETK_OBJECT(UI_ImageBrowserIconbox),
                    ETK_CALLBACK(on_ImageBrowserIconbox_selected), NULL);
   
   if (!etk_string_length_get(Cur.state)) return;
   if (!etk_string_length_get(Cur.part)) return;
   
   PopulateTweenList();
   etk_widget_disabled_set(UI_DeleteTweenButton, TRUE);
   etk_widget_disabled_set(UI_MoveDownTweenButton, TRUE);
   etk_widget_disabled_set(UI_MoveUpTweenButton, TRUE);

   /* Update Image Browser */
   UpdateImageBrowser();
   
   //Set the images label for normal image
   im = edje_edit_state_image_get(edje_o, Cur.part->string, Cur.state->string);
   if (im)
   {
      etk_label_set(ETK_LABEL(UI_ImageNameLabel), im);
      edje_edit_string_free(im);
   }
   else
   {
      etk_label_set(ETK_LABEL(UI_ImageNameLabel), "none");
   }

   //Set alpha and borders
   int alpha, t, l, r, b;
   edje_edit_state_color_get(edje_o, Cur.part->string, Cur.state->string,
                             NULL, NULL, NULL, &alpha);
   edje_edit_state_image_border_get(edje_o, Cur.part->string, Cur.state->string,
                                    &l, &r, &t, &b);
   etk_range_value_set(ETK_RANGE(UI_ImageAlphaSlider), alpha);
   etk_range_value_set(ETK_RANGE(UI_BorderLeftSpinner), l);
   etk_range_value_set(ETK_RANGE(UI_BorderRightSpinner), r);
   etk_range_value_set(ETK_RANGE(UI_BorderTopSpinner), t);
   etk_range_value_set(ETK_RANGE(UI_BorderBottomSpinner), b);
   
   //ReEnable Signal Propagation
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_BorderLeftSpinner),
                      on_BorderSpinner_value_changed, (void*)BORDER_LEFT);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_BorderRightSpinner),
                      on_BorderSpinner_value_changed, (void*)BORDER_RIGHT);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_BorderTopSpinner),
                      on_BorderSpinner_value_changed, (void*)BORDER_TOP);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_BorderBottomSpinner),
                      on_BorderSpinner_value_changed, (void*)BORDER_BOTTOM);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_ImageAlphaSlider),
                      on_ImageAlphaSlider_value_changed, NULL);
   etk_signal_unblock("icon-selected", ETK_OBJECT(UI_ImageBrowserIconbox),
                    ETK_CALLBACK(on_ImageBrowserIconbox_selected), NULL);

}

void
UpdateTextFrame(void)
{
   int eff_num = 0;
   //int alpha;
   int r, g, b;
   Etk_Combobox_Item *item = NULL;
   char *combo_data;
   const char *t;
   const char *font;

   if (!etk_string_length_get(Cur.state)) return;
   if (!etk_string_length_get(Cur.part)) return;
   
   printf("Update Text Frame: %s\n",Cur.state->string);

   //Stop signal propagation
   etk_signal_block("text-changed", ETK_OBJECT(UI_TextEntry),
                    on_TextEntry_text_changed, NULL);
   etk_signal_block("item-activated", ETK_OBJECT(UI_FontComboBox),
                    ETK_CALLBACK(on_FontComboBox_item_activated), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FontAlignHSpinner),
                    ETK_CALLBACK(on_FontAlignSpinner_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FontAlignVSpinner),
                    ETK_CALLBACK(on_FontAlignSpinner_value_changed), NULL);
   
 
   //Set Text Text in Cur.eps
   t = edje_edit_state_text_get(edje_o,Cur.part->string,Cur.state->string);
   printf("TEXT: %s\n",t);
   etk_entry_text_set(ETK_ENTRY(UI_TextEntry), t);
   edje_edit_string_free(t);

   //Set the font size spinner
   etk_range_value_set(ETK_RANGE(UI_FontSizeSpinner), 
      (float)edje_edit_state_text_size_get(edje_o, Cur.part->string, Cur.state->string));
   
   //Set the font align spinners
   etk_range_value_set(ETK_RANGE(UI_FontAlignHSpinner),
                        edje_edit_state_text_align_x_get(edje_o,
                                                         Cur.part->string,
                                                         Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FontAlignVSpinner),
                        edje_edit_state_text_align_y_get(edje_o,
                                                         Cur.part->string,
                                                         Cur.state->string));
   
   //Set the font combobox
   font = edje_edit_state_font_get(edje_o, Cur.part->string, Cur.state->string);
   if (font)
   {
      //Loop for all the item in the Combobox
      item = etk_combobox_first_item_get(ETK_COMBOBOX(UI_FontComboBox));
      while (item)
      {
         combo_data = etk_combobox_item_field_get(item, 1);
         printf("COMBODATA: %s\n",combo_data);
         if (combo_data && (strcmp(combo_data, font) == 0))
         {
            //If we found the item set active and break
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_FontComboBox),item);
            break;
         }
         item = etk_combobox_item_next_get(item);
      }
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_FontComboBox),
                  etk_combobox_first_item_get(ETK_COMBOBOX(UI_FontComboBox)));//TODO change all combobox like this one
   
   edje_edit_string_free(font);

   //Set Effect ComboBox
   eff_num = edje_edit_part_effect_get(edje_o, Cur.part->string);
   eff_num--;
   if (eff_num < 0) eff_num = 0;
   
   etk_combobox_active_item_set(ETK_COMBOBOX(UI_EffectComboBox),
      etk_combobox_nth_item_get(ETK_COMBOBOX(UI_EffectComboBox), eff_num));
   
   //Set Text color Rects
   edje_edit_state_color_get(edje_o, Cur.part->string, Cur.state->string,&r,&g,&b,NULL);
   evas_object_color_set(TextColorObject, r, g, b, 255);
   edje_edit_state_color2_get(edje_o, Cur.part->string, Cur.state->string,&r,&g,&b,NULL);
   evas_object_color_set(ShadowColorObject, r, g, b, 255);
   edje_edit_state_color3_get(edje_o, Cur.part->string, Cur.state->string,&r,&g,&b,NULL);
   evas_object_color_set(OutlineColorObject, r, g, b, 255);

   //Renable  signal propagation
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_TextEntry),
                      ETK_CALLBACK(on_TextEntry_text_changed), NULL);
   etk_signal_unblock("item-activated", ETK_OBJECT(UI_FontComboBox),
                      ETK_CALLBACK(on_FontComboBox_item_activated), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FontAlignHSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FontAlignVSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed), NULL);
}

void
UpdatePositionFrame(void)
{
   //printf("Update Position: %s (offset: %d)\n",Cur.eps->name,Cur.eps->rel1.offset.x);
   //Stop signal propagation
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel1XSpinner),
                    ETK_CALLBACK(on_RelSpinner_value_changed),
                    (void*)REL1X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel1YSpinner),
                    ETK_CALLBACK(on_RelSpinner_value_changed),
                    (void*)REL1Y_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel2XSpinner),
                    ETK_CALLBACK(on_RelSpinner_value_changed),
                    (void*)REL2X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel2YSpinner),
                    ETK_CALLBACK(on_RelSpinner_value_changed),
                    (void*)REL2Y_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel1XOffsetSpinner),
                    ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                    (void*)REL1X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel1YOffsetSpinner),
                    ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                    (void*)REL1Y_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel2XOffsetSpinner),
                    ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                    (void*)REL2X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Rel2YOffsetSpinner),
                    ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                    (void*)REL2Y_SPINNER);

   if (!etk_string_length_get(Cur.state)) return;
   if (!etk_string_length_get(Cur.part)) return;
    //Set relative position spinners
   etk_range_value_set(ETK_RANGE(UI_Rel1XSpinner),
      edje_edit_state_rel1_relative_x_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel1YSpinner),
      edje_edit_state_rel1_relative_y_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel2XSpinner),
      edje_edit_state_rel2_relative_x_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel2YSpinner),
      edje_edit_state_rel2_relative_y_get(edje_o, Cur.part->string,Cur.state->string));
   
   etk_range_value_set(ETK_RANGE(UI_Rel1XOffsetSpinner),
         edje_edit_state_rel1_offset_x_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel1YOffsetSpinner),
         edje_edit_state_rel1_offset_y_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel2XOffsetSpinner),
         edje_edit_state_rel2_offset_x_get(edje_o, Cur.part->string,Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_Rel2YOffsetSpinner),
         edje_edit_state_rel2_offset_y_get(edje_o, Cur.part->string,Cur.state->string));

   //Reenable signal propagation
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel1XSpinner),
                      ETK_CALLBACK(on_RelSpinner_value_changed),
                      (void*)REL1X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel1YSpinner),
                      ETK_CALLBACK(on_RelSpinner_value_changed),
                      (void*)REL1Y_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel2XSpinner),
                      ETK_CALLBACK(on_RelSpinner_value_changed),
                      (void*)REL2X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel2YSpinner),
                      ETK_CALLBACK(on_RelSpinner_value_changed),
                      (void*)REL2Y_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel1XOffsetSpinner),
                      ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                      (void*)REL1X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel1YOffsetSpinner),
                      ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                      (void*)REL1Y_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel2XOffsetSpinner),
                      ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                      (void*)REL2X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Rel2YOffsetSpinner),
                      ETK_CALLBACK(on_RelOffsetSpinner_value_changed),
                      (void*)REL2Y_SPINNER);
}

void
UpdateComboPositionFrame(void)
{
   int i=0;
   Etk_Combobox_Item *item = NULL;
   
   printf("SETTING COMBOS\n");
   
   //Stop signal propagation
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel1ToXComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL1X_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel1ToYComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL1Y_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel2ToXComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL2X_SPINNER);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_Rel2ToYComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL2Y_SPINNER);

   if (!etk_string_length_get(Cur.part)) return;
   if (!etk_string_length_get(Cur.state)) return;
   const char *rel;
   char *p;
   
   //If rel1_to_x is know set the combobox
   if ((rel = edje_edit_state_rel1_to_x_get(edje_o, Cur.part->string, Cur.state->string)))
   {
      //Loop for all the item in the Combobox
      i=0;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel1ToXComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (strcmp(p,rel) == 0)
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel1ToXComboBox),item);
         i++;
      }
      edje_edit_string_free(rel);
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel1ToXComboBox), 
            etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel1ToXComboBox),0));
   
   //If rel1_to_y is know set the combobox
   if ((rel = edje_edit_state_rel1_to_y_get(edje_o, Cur.part->string, Cur.state->string)))
   {
      //Loop for all the item in the Combobox
      i=0;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel1ToYComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (strcmp(p,rel) == 0)
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel1ToYComboBox),item);
         i++;
      }
      edje_edit_string_free(rel);
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel1ToYComboBox), 
            etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel1ToYComboBox),0));
   
   //If rel2_to_x is know set the combobox
   if ((rel = edje_edit_state_rel2_to_x_get(edje_o, Cur.part->string, Cur.state->string)))
   {
      //Loop for all the item in the Combobox
      i=0;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel2ToXComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (strcmp(p,rel) == 0)
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel2ToXComboBox),item);
         i++;
      }
      edje_edit_string_free(rel);
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel2ToXComboBox), 
            etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel2ToXComboBox),0));
   
   //If rel2_to_y is know set the combobox
   if ((rel = edje_edit_state_rel2_to_y_get(edje_o, Cur.part->string, Cur.state->string)))
   {
      //Loop for all the item in the Combobox
      i=0;
      while ((item = etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel2ToYComboBox),i)))
      {
         p = etk_combobox_item_field_get(item, 1);
         if (strcmp(p,rel) == 0)
            etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel2ToYComboBox),item);
         i++;
      }
      edje_edit_string_free(rel);
   }
   else
      etk_combobox_active_item_set(ETK_COMBOBOX(UI_Rel2ToYComboBox), 
            etk_combobox_nth_item_get(ETK_COMBOBOX(UI_Rel2ToYComboBox),0));
   
   //Reenable signal propagation
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel1ToXComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL1X_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel1ToYComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL1Y_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel2ToXComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL2X_SPINNER);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_Rel2ToYComboBox),
         ETK_CALLBACK(on_RelToComboBox_changed), (void*)REL2Y_SPINNER);

}

void
UpdateProgFrame(void)
{
   const char *s;
   Evas_List *l;
   
   if (!etk_string_length_get(Cur.prog)) return;
   
   //Stop signal propagation
   etk_signal_block("text-changed", ETK_OBJECT(UI_ProgramEntry),
         ETK_CALLBACK(on_NamesEntry_text_changed), NULL);
   etk_signal_block("text-changed",
         ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SourceEntry))),
         ETK_CALLBACK(on_SourceEntry_text_changed), UI_SourceEntry);
   etk_signal_block("text-changed",
         ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SignalEntry))),
         ETK_CALLBACK(on_SignalEntry_text_changed), UI_SignalEntry);
   etk_signal_block("value-changed", ETK_OBJECT(UI_DelayFromSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_DelayRangeSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed), NULL);
   etk_signal_block("text-changed", ETK_OBJECT(UI_TargetEntry),
         ETK_CALLBACK(on_TargetEntry_text_changed), NULL);
   etk_signal_block("text-changed", ETK_OBJECT(UI_Param1Entry),
         ETK_CALLBACK(on_Param1Entry_text_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Param1Spinner),
         ETK_CALLBACK(on_Param1Spinner_value_changed), NULL);
   etk_signal_block("active-item-changed", ETK_OBJECT(UI_TransiComboBox),
         ETK_CALLBACK(on_TransitionComboBox_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_DurationSpinner),
         ETK_CALLBACK(on_DurationSpinner_value_changed), NULL);
   etk_signal_block("text-changed", ETK_OBJECT(UI_AfterEntry),
         ETK_CALLBACK(on_AfterEntry_text_changed), NULL);
   etk_signal_block("text-changed", ETK_OBJECT(UI_Param2Entry),
         ETK_CALLBACK(on_Param2Entry_text_changed), NULL);
   etk_signal_block("value-changed", ETK_OBJECT(UI_Param2Spinner),
         ETK_CALLBACK(on_Param2Spinner_value_changed), NULL);
   
   //Update Program Entry (name)
   etk_entry_text_set(ETK_ENTRY(UI_ProgramEntry), Cur.prog->string);
   etk_widget_hide(ETK_WIDGET(UI_ProgramEntryImage));

   //Update Source
   s = edje_edit_program_source_get(edje_o, Cur.prog->string);
   etk_entry_text_set(ETK_ENTRY(etk_combobox_entry_entry_get(
                      ETK_COMBOBOX_ENTRY(UI_SourceEntry))),s);
   edje_edit_string_free(s);
   
   //Update Signal
   s = edje_edit_program_signal_get(edje_o, Cur.prog->string);
   etk_entry_text_set(ETK_ENTRY(etk_combobox_entry_entry_get(
                      ETK_COMBOBOX_ENTRY(UI_SignalEntry))),s);
   edje_edit_string_free(s);
    
   //Update Delay
   etk_range_value_set(ETK_RANGE(UI_DelayFromSpinner),
                       edje_edit_program_in_from_get(edje_o, Cur.prog->string));
   etk_range_value_set(ETK_RANGE(UI_DelayRangeSpinner),
                      edje_edit_program_in_range_get(edje_o, Cur.prog->string));

   //Update Action
   int action;
   action = edje_edit_program_action_get(edje_o, Cur.prog->string);
   etk_combobox_active_item_set(ETK_COMBOBOX(UI_ActionComboBox),
         etk_combobox_nth_item_get(ETK_COMBOBOX(UI_ActionComboBox),action));


   //Update Target(s)
   Etk_String *str = etk_string_new("");
   l = edje_edit_program_targets_get(edje_o, Cur.prog->string);
   for (; l; l = l->next)
   {
      str = etk_string_append(str, l->data);
      str = etk_string_append(str, "|");
   }
   if (str->length > 1)
      str = etk_string_truncate(str, str->length - 1);
   etk_entry_text_set(ETK_ENTRY(UI_TargetEntry),str->string);
   
   //Update Afters(s)
   str = etk_string_clear(str);
   l = edje_edit_program_afters_get(edje_o, Cur.prog->string);
   for (; l; l = l->next)
   {
      str = etk_string_append(str,l->data);
      str = etk_string_append(str,"|");
   }
   if (str->length > 1)
      str = etk_string_truncate(str, str->length - 1);
   etk_entry_text_set(ETK_ENTRY(UI_AfterEntry),str->string);
   

   //Update state & state value
   s = edje_edit_program_state_get(edje_o, Cur.prog->string);
   etk_entry_text_set(ETK_ENTRY(UI_Param1Entry),s);
   etk_range_value_set(ETK_RANGE(UI_Param1Spinner),
                       edje_edit_program_value_get(edje_o, Cur.prog->string));
   edje_edit_string_free(s);
   
   s = edje_edit_program_state2_get(edje_o, Cur.prog->string);
   etk_entry_text_set(ETK_ENTRY(UI_Param2Entry),s);
   etk_range_value_set(ETK_RANGE(UI_Param2Spinner),
                        edje_edit_program_value2_get(edje_o, Cur.prog->string));
   edje_edit_string_free(s);
   
   //Update Transition
   etk_combobox_active_item_set(ETK_COMBOBOX(UI_TransiComboBox),
      etk_combobox_nth_item_get(ETK_COMBOBOX(UI_TransiComboBox),
         edje_edit_program_transition_get(edje_o, Cur.prog->string)-1));
   
   //Update Transition Duration
   etk_range_value_set(ETK_RANGE(UI_DurationSpinner),
               edje_edit_program_transition_time_get(edje_o, Cur.prog->string));
   
   
   //Reenable signal propagation
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_ProgramEntry),
         ETK_CALLBACK(on_NamesEntry_text_changed), NULL);
   etk_signal_unblock("text-changed",
         ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SourceEntry))),
         ETK_CALLBACK(on_SourceEntry_text_changed), UI_SourceEntry);
   etk_signal_unblock("text-changed",
         ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SignalEntry))),
         ETK_CALLBACK(on_SignalEntry_text_changed), UI_SignalEntry);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_DelayFromSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_DelayRangeSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed), NULL);
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_TargetEntry),
         ETK_CALLBACK(on_TargetEntry_text_changed), NULL);
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_Param1Entry),
         ETK_CALLBACK(on_Param1Entry_text_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Param1Spinner),
         ETK_CALLBACK(on_Param1Spinner_value_changed), NULL);
   etk_signal_unblock("active-item-changed", ETK_OBJECT(UI_TransiComboBox),
         ETK_CALLBACK(on_TransitionComboBox_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_DurationSpinner),
         ETK_CALLBACK(on_DurationSpinner_value_changed), NULL);
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_AfterEntry),
         ETK_CALLBACK(on_AfterEntry_text_changed), NULL);
   etk_signal_unblock("text-changed", ETK_OBJECT(UI_Param2Entry),
         ETK_CALLBACK(on_Param2Entry_text_changed), NULL);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_Param2Spinner),
         ETK_CALLBACK(on_Param2Spinner_value_changed), NULL);
   etk_object_destroy(ETK_OBJECT(str));
}

void
UpdateScriptFrame(void)
{
   if (etk_string_length_get(Cur.group))
   {
      edje_edit_script_get(edje_o);
      //printf("Update group script: %s\n",edje_edit_script_get(edje_o));
      
   }
}


/***********************************
 *
 * functions to create interface
 *
 ***********************************/
static Etk_Widget*
create_filechooser_dialog(void)
{
   //Dialog
   UI_FileChooserDialog = etk_dialog_new();
   etk_object_properties_set (ETK_OBJECT(UI_FileChooserDialog), "action-area-homogeneous",ETK_FALSE,NULL);
   etk_signal_connect("delete-event", ETK_OBJECT(UI_FileChooserDialog), ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   etk_signal_connect("response", ETK_OBJECT(UI_FileChooserDialog), ETK_CALLBACK(on_FileChooserDialog_response), NULL);

   //Filechooser
   UI_FileChooser = etk_filechooser_widget_new();
   etk_dialog_pack_in_main_area(ETK_DIALOG(UI_FileChooserDialog), UI_FileChooser,
      ETK_BOX_START, ETK_BOX_EXPAND_FILL,0);
   etk_signal_connect("selected", ETK_OBJECT(UI_FileChooser), ETK_CALLBACK(on_FileChooser_selected), NULL);

   etk_dialog_button_add_from_stock(ETK_DIALOG(UI_FileChooserDialog),
      ETK_STOCK_DIALOG_CANCEL, ETK_RESPONSE_CANCEL );
   UI_FilechooserLoadButton = etk_dialog_button_add_from_stock(ETK_DIALOG(UI_FileChooserDialog),
      ETK_STOCK_DOCUMENT_OPEN ,ETK_RESPONSE_OK );
   UI_FilechooserSaveButton = etk_dialog_button_add_from_stock(ETK_DIALOG(UI_FileChooserDialog),
      ETK_STOCK_DOCUMENT_SAVE ,ETK_RESPONSE_OK );

   return UI_FileChooserDialog;
}

static Etk_Widget*
create_a_color_button(char* label_text, int color_button_enum,int w,int h, Evas *evas)
{
   Etk_Widget *vbox;
   Etk_Widget *label;
   Etk_Widget *shadow;
   Evas_Object* rect;
   Etk_Widget *etk_evas_object;

   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 3);

   //shadow
   shadow = etk_shadow_new();
   etk_shadow_border_set(ETK_SHADOW(shadow), 0);
   //etk_widget_size_request_set(shadow, 45, 45);
   etk_shadow_shadow_set(ETK_SHADOW(shadow),ETK_SHADOW_OUTSIDE, ETK_SHADOW_ALL, 10, 2, 2, 200);
   //etk_shadow_shadow_set(Etk_Shadow *shadow, Etk_Shadow_Type type, Etk_Shadow_Edges edges, int radius, int offset_x, int offset_y, int opacity);
   etk_box_append(ETK_BOX(vbox), shadow, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Add the colored rectangle
   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 255,100,100,255);
   evas_object_resize(rect, w, h);
   etk_evas_object = etk_evas_object_new();
   etk_evas_object_set_object(ETK_EVAS_OBJECT(etk_evas_object), rect);
   evas_object_show(rect);
   etk_widget_show_all(etk_evas_object);
   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN,
                                  on_ColorCanvas_click, (void*)color_button_enum);
   etk_container_add(ETK_CONTAINER(shadow), etk_evas_object);
   etk_widget_size_request_set(etk_evas_object, w, h);
   switch (color_button_enum)
   {
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
   


   //Label
   if (label_text){
      label = etk_label_new(label_text);
      etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
      etk_box_append(ETK_BOX(vbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   }
   return vbox;
}

static Etk_Widget*
create_toolbar(Etk_Toolbar_Orientation o)
{
   Etk_Widget *button;
   Etk_Widget *sep;
   Etk_Widget *menu_item;
   Etk_Widget *image;
   
   //ToolBar
   UI_Toolbar = etk_toolbar_new();
   etk_toolbar_orientation_set(ETK_TOOLBAR(UI_Toolbar), o);
   //etk_object_properties_set(ETK_OBJECT(UI_Toolbar),"theme-group","etk/iconbox/label");
   //etk_theme_edje_object_set(Evas_Object *object, const char *file, const char *group, const char *parent_group)
   
   //NewButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_NEW);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_NEW);

   //OpenButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_OPEN);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPEN);

   //SaveButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_SAVE);
   etk_object_properties_set(ETK_OBJECT(button),"label","Save",NULL);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_SAVE);
   
   //SaveEDJButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_SAVE_AS);
   etk_object_properties_set(ETK_OBJECT(button),"label","Save as",NULL);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_SAVE_EDJ);

   //SaveEDCButton
 /*  button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_SAVE_AS);
   etk_object_properties_set(ETK_OBJECT(button),"label","Export edc",NULL);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_SAVE_EDC);
   */ 

   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);

   //AddButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_LIST_ADD);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                        ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_ADD);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

   //AddMenu
   UI_AddMenu = etk_menu_new();

   //New Rectangle
   menu_item = etk_menu_item_image_new_with_label("Rectangle");
   image = etk_image_new_from_edje(EdjeFile,"RECT.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_RECT);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));

   //New Image
   menu_item = etk_menu_item_image_new_with_label("Image");
   image = etk_image_new_from_edje(EdjeFile,"IMAGE.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated),(void*) NEW_IMAGE);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));
   
   //New Text
   menu_item = etk_menu_item_image_new_with_label("Text");
   image = etk_image_new_from_edje(EdjeFile,"TEXT.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_TEXT);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));
   
   //New Swallow
   menu_item = etk_menu_item_image_new_with_label("Swallow");
   image = etk_image_new_from_edje(EdjeFile,"SWAL.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_SWAL);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));
   
   //New GroupSwallow
   menu_item = etk_menu_item_image_new_with_label("Group swallow");
   image = etk_image_new_from_edje(EdjeFile,"GROUP.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_GROUPSWAL);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));
   
   //New Program
   menu_item = etk_menu_item_image_new_with_label("Program");
   image = etk_image_new_from_edje(EdjeFile,"PROG.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_PROG);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));
   
   //Separator
   menu_item = etk_menu_item_separator_new();
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));

   //New Description
   UI_AddStateButton = etk_menu_item_image_new_with_label("A new state to part");
   image = etk_image_new_from_edje(EdjeFile,"DESC.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(UI_AddStateButton), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(UI_AddStateButton),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_DESC);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(UI_AddStateButton));

   //New Group
   menu_item = etk_menu_item_image_new_with_label("A new group to edj");
   image = etk_image_new_from_edje(EdjeFile,"NONE.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
                     ETK_CALLBACK(on_AddMenu_item_activated), (void*)NEW_GROUP);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_AddMenu), ETK_MENU_ITEM(menu_item));

   //RemoveButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_LIST_REMOVE);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_REMOVE);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

   //RemoveMenu
   UI_RemoveMenu = etk_menu_new();

   //description
   UI_RemoveStateButton = etk_menu_item_image_new_with_label("Selected State");
   image = etk_image_new_from_edje(EdjeFile,"DESC.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(UI_RemoveStateButton), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(UI_RemoveStateButton),
         ETK_CALLBACK(on_RemoveMenu_item_activated), (void*)REMOVE_DESCRIPTION);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_RemoveMenu), ETK_MENU_ITEM(UI_RemoveStateButton));

   //part
   UI_RemovePartButton = etk_menu_item_image_new_with_label("Selected Part");
   etk_signal_connect("activated", ETK_OBJECT(UI_RemovePartButton),
               ETK_CALLBACK(on_RemoveMenu_item_activated), (void*)REMOVE_PART);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_RemoveMenu), ETK_MENU_ITEM(UI_RemovePartButton));

   //program 
   UI_RemoveProgramButton = etk_menu_item_image_new_with_label("Selected Program");
   image = etk_image_new_from_edje(EdjeFile,"PROG.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(UI_RemoveProgramButton), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(UI_RemoveProgramButton),
               ETK_CALLBACK(on_RemoveMenu_item_activated), (void*)REMOVE_PROG);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_RemoveMenu), ETK_MENU_ITEM(UI_RemoveProgramButton));
    
   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);

   //group
   menu_item = etk_menu_item_image_new_with_label("Current Group");
   image = etk_image_new_from_edje(EdjeFile,"NONE.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
               ETK_CALLBACK(on_RemoveMenu_item_activated), (void*)REMOVE_GROUP);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_RemoveMenu), ETK_MENU_ITEM(menu_item));

   //MoveUp Button
   button = etk_tool_button_new_from_stock( ETK_STOCK_GO_UP);
   etk_object_properties_set(ETK_OBJECT(button),"label","Lower",NULL);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_MOVE_UP);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

   //MoveDown Button
   button = etk_tool_button_new_from_stock( ETK_STOCK_GO_DOWN);
   etk_object_properties_set(ETK_OBJECT(button),"label","Raise",NULL);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                  ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_MOVE_DOWN);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);

   //Images Browser Button
   button = etk_tool_button_new_from_stock( ETK_STOCK_IMAGE_X_GENERIC);
   etk_object_properties_set(ETK_OBJECT(button),"label","Images",NULL);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                  ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_IMAGE_BROWSER);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

#if DEBUG_MODE
   //Font Browser Button
   button = etk_tool_button_new_from_stock( ETK_STOCK_PREFERENCES_DESKTOP_FONT);
   etk_object_properties_set(ETK_OBJECT(button),"label","Fonts",NULL);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                  ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_FONT_BROWSER);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
#endif
   
   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);
   
   //OptionsButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_PREFERENCES_SYSTEM);
   etk_object_properties_set(ETK_OBJECT(button),"label","Options",NULL);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTIONS);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);

   //OptionsMenu
   UI_OptionsMenu = etk_menu_new();

   //set bg1
   menu_item = etk_menu_item_image_new_with_label("White squared background");
   image = etk_image_new_from_edje(EdjeFile,"BG1_I.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
         ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTION_BG1);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));

   //set bg2
   menu_item = etk_menu_item_image_new_with_label("Gray squared background");
   image = etk_image_new_from_edje(EdjeFile,"BG2_I.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
               ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTION_BG2);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));

   //set bg3
   menu_item = etk_menu_item_image_new_with_label("White background");
   image = etk_image_new_from_edje(EdjeFile,"BG3_I.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
               ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTION_BG3);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));

   //set bg4
   menu_item = etk_menu_item_image_new_with_label("Black background");
   image = etk_image_new_from_edje(EdjeFile,"BG4_I.PNG");
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
               ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTION_BG4);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));
   
   //Separator
   menu_item = etk_menu_item_separator_new();
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));

   //Fullscreen
   menu_item = etk_menu_item_image_new_with_label("Toggle fullscreen");
   image = etk_image_new_from_stock(ETK_STOCK_VIDEO_DISPLAY, ETK_STOCK_SMALL);
   etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item), ETK_IMAGE(image));
   etk_signal_connect("activated", ETK_OBJECT(menu_item),
               ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_OPTION_FULLSCREEN);
   etk_menu_shell_append(ETK_MENU_SHELL(UI_OptionsMenu), ETK_MENU_ITEM(menu_item));
  
   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);

   //UI_PlayButton
   UI_PlayImage = etk_image_new_from_stock(ETK_STOCK_MEDIA_PLAYBACK_START, ETK_STOCK_MEDIUM);
   UI_PauseImage = etk_image_new_from_stock(ETK_STOCK_MEDIA_PLAYBACK_PAUSE, ETK_STOCK_MEDIUM);
   UI_PlayButton = etk_tool_button_new();
   etk_button_image_set (ETK_BUTTON(UI_PlayButton), ETK_IMAGE(UI_PauseImage));
   etk_object_properties_set(ETK_OBJECT(UI_PlayButton),"label","Pause Edje",NULL);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), UI_PlayButton, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(UI_PlayButton),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_PLAY);
   
#if DEBUG_MODE
   //Separator
   sep = etk_vseparator_new();
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), sep, ETK_BOX_START);
   
   //DebugButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_DOCUMENT_PROPERTIES);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                        ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_DEBUG);
   etk_object_properties_set(ETK_OBJECT(button),"label","Debug",NULL);
#endif
   
   //QuitButton
   button = etk_tool_button_new_from_stock(ETK_STOCK_SYSTEM_LOG_OUT);
   etk_object_properties_set(ETK_OBJECT(button),"label","Quit",NULL);
   etk_toolbar_append(ETK_TOOLBAR(UI_Toolbar), button, ETK_BOX_START);
   etk_signal_connect("clicked", ETK_OBJECT(button),
                     ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_QUIT);
   
   return UI_Toolbar;
}

static Etk_Widget*
create_group_frame(void)
{
   Etk_Widget *label;
   Etk_Widget *hbox, *vbox;

   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);

   //GroupNameEntry
   label = etk_label_new("Name");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);
   UI_GroupNameEntry = etk_entry_new();
   UI_GroupNameEntryImage = etk_image_new_from_stock(ETK_STOCK_DIALOG_OK,
                                                     ETK_STOCK_SMALL);
   etk_entry_image_set(ETK_ENTRY(UI_GroupNameEntry), ETK_ENTRY_IMAGE_SECONDARY,
                       ETK_IMAGE(UI_GroupNameEntryImage));
   etk_box_append(ETK_BOX(hbox), UI_GroupNameEntry, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //CurrentSizeLabel
   UI_CurrentGroupSizeLabel = etk_label_new("Current size: 0 x 0");
   etk_box_append(ETK_BOX(vbox), UI_CurrentGroupSizeLabel, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("min");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_GroupMinWSpinner
   UI_GroupMinWSpinner = etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_GroupMinWSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_GroupMinWSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("x");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_GroupMinHSpinner
   UI_GroupMinHSpinner = etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_GroupMinHSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_GroupMinHSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("max");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //UI_GroupMaxWSpinner
   UI_GroupMaxWSpinner =  etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_GroupMaxWSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_GroupMaxWSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("x");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_GroupMaxHspinner
   UI_GroupMaxHSpinner = etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_GroupMaxHSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_GroupMaxHSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   
   etk_signal_connect("text-changed", ETK_OBJECT(UI_GroupNameEntry),
         ETK_CALLBACK(on_NamesEntry_text_changed), NULL);   
   etk_signal_connect("key-down", ETK_OBJECT(UI_GroupNameEntry),
         ETK_CALLBACK(on_GroupNameEntry_key_down), NULL);
   etk_signal_connect("mouse-click", ETK_OBJECT(UI_GroupNameEntryImage),
                      ETK_CALLBACK(on_GroupNameEntryImage_mouse_clicked), NULL);
   
   etk_signal_connect("value-changed", ETK_OBJECT(UI_GroupMinWSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MINW_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_GroupMinHSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MINH_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_GroupMaxWSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                      (void *)MAXW_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_GroupMaxHSpinner),
                      ETK_CALLBACK(on_GroupSpinner_value_changed),
                     (void *)MAXH_SPINNER);

   return vbox;
}

static Etk_Widget*
create_tree(void)
{
   //UI_GroupsComboBox
   UI_GroupsComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_GroupsComboBox), ETK_COMBOBOX_LABEL, 30, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_GroupsComboBox));
   //etk_combobox_items_height_set(ETK_COMBOBOX(UI_GroupsComboBox), 18);
   
   etk_signal_connect("item-activated", ETK_OBJECT(UI_GroupsComboBox),
                      ETK_CALLBACK(on_GroupsComboBox_activated), NULL);
   
   //UI_PartsTree
   Etk_Tree_Col *col;
   UI_PartsTree = etk_tree_new();
   etk_widget_padding_set(UI_PartsTree,2,2,2,2);
   etk_tree_mode_set(ETK_TREE(UI_PartsTree), ETK_TREE_MODE_TREE);
   etk_tree_headers_visible_set(ETK_TREE(UI_PartsTree), ETK_FALSE);
   etk_widget_size_request_set(UI_PartsTree, 260, 300);
   //Main column
   col = etk_tree_col_new(ETK_TREE(UI_PartsTree), "File contents",100,0);
   etk_tree_col_model_add(col,etk_tree_model_image_new());
   etk_tree_col_model_add(col,etk_tree_model_text_new());
   etk_tree_col_resizable_set(col, ETK_FALSE);
   etk_tree_col_expand_set(col,ETK_TRUE);
   //Visibility column
   col = etk_tree_col_new(ETK_TREE(UI_PartsTree), "vis", 10,0);
   etk_tree_col_visible_set(col, DEBUG_MODE);
   etk_tree_col_model_add(col,etk_tree_model_checkbox_new());
   etk_tree_col_resizable_set(col, ETK_FALSE);
   etk_tree_col_expand_set(col,ETK_FALSE);
   //RowType column
   col = etk_tree_col_new(ETK_TREE(UI_PartsTree), "type",10, 0);
   etk_tree_col_model_add(col,etk_tree_model_int_new());
   etk_tree_col_visible_set(col, DEBUG_MODE);
   etk_tree_col_resizable_set(col, ETK_FALSE);
   etk_tree_col_expand_set(col,ETK_FALSE);
   //Parent part row
   col = etk_tree_col_new(ETK_TREE(UI_PartsTree), "parent",100, 0);
   etk_tree_col_model_add(col,etk_tree_model_text_new());
   etk_tree_col_visible_set(col, DEBUG_MODE);
   etk_tree_col_resizable_set(col, ETK_FALSE);
   etk_tree_col_expand_set(col,ETK_FALSE);
   etk_tree_build(ETK_TREE(UI_PartsTree));

   etk_signal_connect("row-selected", ETK_OBJECT(UI_PartsTree),
                      ETK_CALLBACK(on_PartsTree_row_selected), NULL);
   etk_signal_connect("row-clicked", ETK_OBJECT(UI_PartsTree),
                      ETK_CALLBACK(on_PartsTree_click), NULL);
   
   //vbox
   Etk_Widget *vbox;
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), UI_GroupsComboBox, ETK_BOX_START, ETK_BOX_NONE, 0);
   etk_box_append(ETK_BOX(vbox), UI_PartsTree, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   return vbox;
}

static Etk_Widget*
create_description_frame(void)
{
   Etk_Widget *vbox;
   Etk_Widget *hbox;
   Etk_Widget *label;
   Etk_Combobox_Item *ComboItem;

   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 0);
  // etk_container_add(ETK_CONTAINER(UI_DescriptionFrame), vbox);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<b>Name</b>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //StateEntry
   UI_StateEntry = etk_entry_new();
   UI_StateEntryImage = etk_image_new_from_stock(ETK_STOCK_DIALOG_OK,
                                                   ETK_STOCK_SMALL);
   etk_entry_image_set(ETK_ENTRY(UI_StateEntry), ETK_ENTRY_IMAGE_SECONDARY,
                       ETK_IMAGE(UI_StateEntryImage));
   etk_box_append(ETK_BOX(hbox), UI_StateEntry, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //UI_StateIndexSpinner
   //~ UI_StateIndexSpinner = etk_spinner_new(0.0, 1.0, 0.0, 0.1, 1.0);
   //~ etk_spinner_digits_set(ETK_SPINNER(UI_StateIndexSpinner), 1);
   //~ etk_widget_size_request_set(UI_StateIndexSpinner,45, 20);
   //~ etk_box_append(ETK_BOX(hbox),UI_StateIndexSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<b>Aspect</b>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("min:");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_AspectMinSpinner
   UI_AspectMinSpinner = etk_spinner_new(0.0, 100.0, 0.0, 0.1, 1.0);
   etk_spinner_digits_set(ETK_SPINNER(UI_AspectMinSpinner), 1);
   etk_widget_size_request_set(UI_AspectMinSpinner,45, 20);
   etk_box_append(ETK_BOX(hbox),UI_AspectMinSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("max:");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_AspectMaxSpinner
   UI_AspectMaxSpinner = etk_spinner_new(0.0, 100.0, 0.0, 0.1, 1.0);
   etk_spinner_digits_set(ETK_SPINNER(UI_AspectMaxSpinner), 1);
   etk_widget_size_request_set(UI_AspectMaxSpinner,45, 20);
   etk_box_append(ETK_BOX(hbox),UI_AspectMaxSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_AspectCombo
   UI_AspectComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_AspectComboBox), ETK_COMBOBOX_LABEL, 30, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_AspectComboBox));
   etk_box_append(ETK_BOX(hbox),UI_AspectComboBox, ETK_BOX_START, ETK_BOX_NONE, 0);

   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_AspectComboBox), "None");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_ASPECT_PREFER_NONE);

   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_AspectComboBox), "Vertical");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_ASPECT_PREFER_VERTICAL);

   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_AspectComboBox), "Horizontal");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_ASPECT_PREFER_HORIZONTAL);

   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_AspectComboBox), "Both");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_ASPECT_PREFER_BOTH);
   
   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("min");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_StateMinWSpinner
   UI_StateMinWSpinner = etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_StateMinWSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateMinWSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("x");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_StateMinHSpinner
   UI_StateMinHSpinner =  etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_StateMinHSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateMinHSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("max");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //UI_StateMaxWSpinner
   UI_StateMaxWSpinner =  etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_StateMaxWSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateMaxWSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("x");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //UI_StateMaxHspinner
   UI_StateMaxHSpinner =  etk_spinner_new(0, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_StateMaxHSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateMaxHSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   //UI_StateAlignHSpinner
   label = etk_label_new("Align");
   etk_box_append(ETK_BOX(hbox),label, ETK_BOX_START, ETK_BOX_NONE, 0);
    
   UI_StateAlignHSpinner =  etk_spinner_new(0, 1, 0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_StateAlignHSpinner), 2);
   etk_widget_size_request_set(UI_StateAlignHSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateAlignHSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   //UI_StateAlignVSpinner
   label = etk_label_new("V Align");
   etk_box_append(ETK_BOX(hbox),label, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   UI_StateAlignVSpinner =  etk_spinner_new(0, 1, 0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_StateAlignVSpinner), 2);
   etk_widget_size_request_set(UI_StateAlignVSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_StateAlignVSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);
    
   
   etk_signal_connect("key-down", ETK_OBJECT(UI_StateEntry),
                      ETK_CALLBACK(on_StateEntry_key_down), NULL);
   etk_signal_connect("mouse-click", ETK_OBJECT(UI_StateEntryImage),
                      ETK_CALLBACK(on_StateEntryImage_mouse_clicked), NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_StateEntry),
                      ETK_CALLBACK(on_NamesEntry_text_changed), NULL);
   
   etk_signal_connect("value-changed", ETK_OBJECT(UI_AspectMinSpinner),
                      ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_AspectMaxSpinner),
                      ETK_CALLBACK(on_AspectSpinner_value_changed), NULL);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_AspectComboBox),
                      ETK_CALLBACK(on_AspectComboBox_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateMinWSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateMinHSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateMaxWSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateMaxHSpinner),
                      ETK_CALLBACK(on_StateMinMaxSpinner_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateAlignVSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                      (void*)STATE_ALIGNV_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_StateAlignHSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed),
                      (void*)STATE_ALIGNH_SPINNER);
   return vbox;
}

static Etk_Widget*
create_rectangle_frame(Evas *evas)
{
   return create_a_color_button("Color",COLOR_OBJECT_RECT,100,30, evas);
}

static Etk_Widget*
create_image_frame(void)
{
   Etk_Widget *label;
   Etk_Widget *table;
   Etk_Tree_Col *col1;

   //table
   table = etk_table_new(5, 4, ETK_TABLE_NOT_HOMOGENEOUS);

   //AddImageButton
   UI_ImageAddButton = etk_button_new_from_stock(ETK_STOCK_IMAGE_X_GENERIC);
   etk_object_properties_set(ETK_OBJECT(UI_ImageAddButton), "label", "Choose Image", NULL);
   etk_table_attach_default(ETK_TABLE(table),UI_ImageAddButton, 0, 1, 0, 0);
   
   //ImageName
   UI_ImageNameLabel = etk_label_new("none");
   etk_object_properties_set(ETK_OBJECT(UI_ImageNameLabel), "xalign", 0.5, NULL);
   etk_table_attach_default(ETK_TABLE(table),UI_ImageNameLabel, 2, 4, 0, 0);

   //ImageTweenVBox
   UI_ImageTweenVBox = etk_vbox_new(ETK_TRUE, 0);
   etk_table_attach_default(ETK_TABLE(table),UI_ImageTweenVBox, 0, 0, 1, 1);
   
   //AddTweenButton
   UI_AddTweenButton = etk_button_new_from_stock(ETK_STOCK_LIST_ADD);
   etk_button_style_set(ETK_BUTTON(UI_AddTweenButton), ETK_BUTTON_ICON);
   etk_signal_connect("clicked", ETK_OBJECT(UI_AddTweenButton), 
      ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_TWEEN_ADD);
   etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_AddTweenButton, 
                     ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   //DeleteTweenButton
   UI_DeleteTweenButton = etk_button_new_from_stock(ETK_STOCK_LIST_REMOVE);
   etk_button_style_set(ETK_BUTTON(UI_DeleteTweenButton), ETK_BUTTON_ICON);
   etk_signal_connect("clicked", ETK_OBJECT(UI_DeleteTweenButton), 
      ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_TWEEN_DELETE);
   etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_DeleteTweenButton, 
                     ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   //MoveUpTweenButton
   UI_MoveUpTweenButton = etk_button_new_from_stock(ETK_STOCK_GO_UP);
   etk_button_style_set(ETK_BUTTON(UI_MoveUpTweenButton),  ETK_BUTTON_ICON);
   etk_signal_connect("clicked", ETK_OBJECT(UI_MoveUpTweenButton), 
      ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_TWEEN_UP);
   etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_MoveUpTweenButton, 
                     ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   //MoveDownTweenButton
   UI_MoveDownTweenButton = etk_button_new_from_stock(ETK_STOCK_GO_DOWN);
   etk_button_style_set(ETK_BUTTON(UI_MoveDownTweenButton),  ETK_BUTTON_ICON);
   etk_signal_connect("clicked", ETK_OBJECT(UI_MoveDownTweenButton), 
      ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_TWEEN_DOWN);
   etk_box_append(ETK_BOX(UI_ImageTweenVBox), UI_MoveDownTweenButton, 
                     ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //ImageTweenList
   UI_ImageTweenList = etk_tree_new();
   etk_tree_mode_set(ETK_TREE(UI_ImageTweenList), ETK_TREE_MODE_LIST);
   etk_tree_headers_visible_set(ETK_TREE(UI_ImageTweenList), FALSE);
  // etk_tree_multiple_select_set(ETK_TREE(UI_ImageTweenList), ETK_TRUE);
   col1 = etk_tree_col_new(ETK_TREE(UI_ImageTweenList), "Tween", 130, 0.0);
   etk_tree_col_model_add(col1, etk_tree_model_image_new());
   etk_tree_col_model_add(col1, etk_tree_model_text_new());
   etk_tree_build(ETK_TREE(UI_ImageTweenList));
   etk_table_attach_default(ETK_TABLE(table),UI_ImageTweenList, 1, 4, 1, 1);

   label = etk_label_new("Alpha");
   etk_table_attach_default(ETK_TABLE(table),label, 0, 0, 2, 2);

   //ImageAlphaSlider
   UI_ImageAlphaSlider = etk_hslider_new(0, 255, 15, 1,20);
   etk_table_attach_default(ETK_TABLE(table),UI_ImageAlphaSlider, 1, 4, 2, 2);

   label = etk_label_new("Left");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 1, 1, 3, 3);

   label = etk_label_new("Right");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 2, 2, 3, 3);

   label = etk_label_new("Top");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 3, 3, 3, 3);

   label = etk_label_new("Bottom");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 4, 4, 3, 3);

   label = etk_label_new("Border");
   //etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 0, 0, 4, 4);

   //UI_BorderLeftSpinner
   UI_BorderLeftSpinner = etk_spinner_new(0, 500, 0, 1, 10);
   etk_widget_size_request_set(UI_BorderLeftSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_BorderLeftSpinner, 1, 1, 4, 4);

   //UI_BorderRightSpinner
   UI_BorderRightSpinner = etk_spinner_new(0, 500, 0, 1, 10);
   etk_widget_size_request_set(UI_BorderRightSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_BorderRightSpinner, 2, 2, 4, 4);

   //UI_BorderTopSpinner
   UI_BorderTopSpinner = etk_spinner_new(0, 500, 0, 1, 10);
   etk_widget_size_request_set(UI_BorderTopSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_BorderTopSpinner, 3, 3, 4, 4);

   //UI_BorderBottomSpinner
   UI_BorderBottomSpinner = etk_spinner_new(0, 500, 0, 1, 10);
   etk_widget_size_request_set(UI_BorderBottomSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_BorderBottomSpinner, 4, 4, 4, 4);

   etk_signal_connect("row-selected", ETK_OBJECT(UI_ImageTweenList),
            ETK_CALLBACK(on_ImageTweenList_row_selected), NULL);
   etk_signal_connect("clicked", ETK_OBJECT(UI_ImageAddButton),
            ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_BROWSER_SHOW);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_ImageAlphaSlider),
            ETK_CALLBACK(on_ImageAlphaSlider_value_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_BorderLeftSpinner),
            ETK_CALLBACK(on_BorderSpinner_value_changed), (void *)BORDER_LEFT);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_BorderRightSpinner),
            ETK_CALLBACK(on_BorderSpinner_value_changed), (void *)BORDER_RIGHT);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_BorderTopSpinner),
            ETK_CALLBACK(on_BorderSpinner_value_changed), (void *)BORDER_TOP);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_BorderBottomSpinner),
            ETK_CALLBACK(on_BorderSpinner_value_changed), (void *)BORDER_BOTTOM);

   return table;
}

static Etk_Widget*
create_image_browser(void)
{
   Etk_Widget *hbox, *vbox, *vbox2;
   Etk_Widget *button;
   Etk_Widget *frame;
   Etk_Widget *label;
   Etk_Widget *image;
   
   //ImageBrowserWin
   UI_ImageBrowserWin = etk_window_new();
   etk_window_title_set(ETK_WINDOW(UI_ImageBrowserWin), "Image Browser");
   etk_window_resize(ETK_WINDOW(UI_ImageBrowserWin), 590, 370);
   etk_signal_connect("delete-event", ETK_OBJECT(UI_ImageBrowserWin),
                      ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   etk_container_border_width_set(ETK_CONTAINER(UI_ImageBrowserWin), 5);

   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_container_add (ETK_CONTAINER(UI_ImageBrowserWin), hbox);
   
   //ImageBrowserIconbox
   UI_ImageBrowserIconbox = etk_iconbox_new();
   etk_signal_connect("icon-selected", ETK_OBJECT(UI_ImageBrowserIconbox),
                      ETK_CALLBACK(on_ImageBrowserIconbox_selected), NULL);
   etk_box_append(ETK_BOX(hbox), UI_ImageBrowserIconbox, 0, ETK_BOX_EXPAND_FILL, 0);
   
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_box_append (ETK_BOX(hbox), vbox, 0, ETK_BOX_NONE, 5);
   
   //AddImageButton
   button = etk_button_new_from_stock(ETK_STOCK_DOCUMENT_OPEN);
   etk_object_properties_set(ETK_OBJECT(button), "label","Import image",NULL);
   etk_button_style_set(ETK_BUTTON(button), ETK_BUTTON_BOTH_VERT);
   etk_signal_connect("clicked", ETK_OBJECT(button),
            ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_IMAGE_FILE_ADD);
   etk_box_append(ETK_BOX(vbox), button, 0, ETK_BOX_NONE, 0);
   
   //DeleteImageButton
   button = etk_button_new_from_stock(ETK_STOCK_EDIT_DELETE);
   etk_object_properties_set(ETK_OBJECT(button), "label","Remove image",NULL);
   etk_button_style_set(ETK_BUTTON(button), ETK_BUTTON_BOTH_VERT);
   etk_widget_disabled_set(button, 1);
   etk_box_append(ETK_BOX(vbox), button, 0, ETK_BOX_NONE, 0);
   
   //EditImageButton
   button = etk_button_new_with_label("Edit Image");
   image = etk_image_new_from_edje(EdjeFile, "GIMP.PNG");
   etk_button_image_set(ETK_BUTTON(button), ETK_IMAGE(image));
   etk_button_style_set(ETK_BUTTON(button), ETK_BUTTON_BOTH_VERT);
   etk_widget_disabled_set(button, 1);
   etk_box_append(ETK_BOX(vbox), button, 0, ETK_BOX_NONE, 0);
   
   //ImageInfo
   frame = etk_frame_new("Image Info");
   etk_box_append(ETK_BOX(vbox), frame, 0, ETK_BOX_EXPAND_FILL, 10);
   
   vbox2 = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(frame), vbox2);
   
   label = etk_label_new("<b>name:</b>");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 0, NULL);
   etk_box_append(ETK_BOX(vbox2), label, 0, ETK_BOX_NONE, 0);
   
   label = etk_label_new("Unknow");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 0, NULL);
   etk_box_append(ETK_BOX(vbox2), label, 0, ETK_BOX_NONE, 0);
   
   label = etk_label_new(" ");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 0, NULL);
   etk_box_append(ETK_BOX(vbox2), label, 0, ETK_BOX_NONE, 0);
   
   UI_ImageBrowserSizeLabel = etk_label_new("<b>size:</b> Unknow");
   etk_object_properties_set(ETK_OBJECT(UI_ImageBrowserSizeLabel), "xalign", 0, NULL);
   etk_box_append(ETK_BOX(vbox2), UI_ImageBrowserSizeLabel, 0, ETK_BOX_NONE, 0);
   
   label = etk_label_new("<b>comp:</b> Unknow");
   etk_object_properties_set(ETK_OBJECT(label), "xalign", 0, NULL);
   etk_box_append(ETK_BOX(vbox2), label, 0, ETK_BOX_NONE, 0);
   
   //CloseButton
   button = etk_button_new_from_stock(ETK_STOCK_DIALOG_CLOSE);
   etk_button_style_set(ETK_BUTTON(button), ETK_BUTTON_BOTH_VERT);
   etk_signal_connect("clicked", ETK_OBJECT(button),
            ETK_CALLBACK(on_AllButton_click), (void*)IMAGE_BROWSER_CLOSE);
   etk_box_append(ETK_BOX(vbox), button, 0, ETK_BOX_NONE, 0);
   
   return UI_ImageBrowserWin;
}

static Etk_Widget*
create_text_frame(Evas *evas)
{
   Etk_Widget *vbox;
   Etk_Widget *hbox;
   Etk_Widget *label;
   Etk_Widget *table;
   Etk_Combobox_Item *ComboItem;

   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 5);

   //table
   table = etk_table_new(5, 4, ETK_TABLE_NOT_HOMOGENEOUS);
   etk_box_append(ETK_BOX(vbox), table, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("Text");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 0, 0, 0,0);

   //Text Entry
   UI_TextEntry = etk_entry_new ();
   //etk_widget_size_request_set(UI_TextEntry,30, 30);
   etk_table_attach_default(ETK_TABLE(table),UI_TextEntry, 1, 4, 0,0);

   label = etk_label_new("Font");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 0, 0, 1,1);

   //FontComboBox
   UI_FontComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_FontComboBox),
                           ETK_COMBOBOX_IMAGE, 20, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_FontComboBox),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_FontComboBox));
   etk_table_attach_default(ETK_TABLE(table),UI_FontComboBox, 1, 1, 1,1);

   //FontAddButton
   UI_FontAddButton = etk_button_new_from_stock(ETK_STOCK_DOCUMENT_OPEN);
   etk_object_properties_set(ETK_OBJECT(UI_FontAddButton), "label","",NULL);
   etk_table_attach_default(ETK_TABLE(table),UI_FontAddButton, 2, 2, 1, 1);


   label = etk_label_new("Size");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_table_attach_default(ETK_TABLE(table),label, 3, 3, 1,1);

   //FontSizeSpinner
   UI_FontSizeSpinner = etk_spinner_new(0, 200, 0, 1, 10);
   etk_widget_size_request_set(UI_FontSizeSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_FontSizeSpinner, 4, 4, 1,1);

   //FontAlignHSpinner
   label = etk_label_new("Align");
   etk_table_attach_default(ETK_TABLE(table),label,0,0, 2,2);
    
   UI_FontAlignHSpinner = etk_spinner_new(0, 1, 0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FontAlignHSpinner), 2);
   etk_widget_size_request_set(UI_FontAlignHSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_FontAlignHSpinner, 1, 1, 2,2);
   
   //FontAlignVSpinner
   label = etk_label_new("V Align");
   etk_table_attach_default(ETK_TABLE(table),label, 2, 2, 2,2);
   
   UI_FontAlignVSpinner =  etk_spinner_new (0, 1, 0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FontAlignVSpinner), 2);
   etk_widget_size_request_set(UI_FontAlignVSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_FontAlignVSpinner, 3, 4, 2,2);
   
   //PartEffectComboBox
   label = etk_label_new("Effect");
   etk_table_attach_default(ETK_TABLE(table),label, 0, 0, 3,3);
   
   UI_EffectComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_EffectComboBox), 
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_EffectComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_EffectComboBox));
   
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Plain");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_PLAIN);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Outline");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_OUTLINE);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Soft Outline");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_SOFT_OUTLINE);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Soft Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_SOFT_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Outline Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_OUTLINE_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Outline Soft Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Far Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_FAR_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Far Soft Shadow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW);
   ComboItem = etk_combobox_item_append(ETK_COMBOBOX(UI_EffectComboBox),
      etk_image_new_from_edje(EdjeFile,"NONE.PNG"), "Glow");
   etk_combobox_item_data_set(ComboItem, (void*)EDJE_TEXT_EFFECT_GLOW);
   
   etk_table_attach_default(ETK_TABLE(table),UI_EffectComboBox,1,4,3,3);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 10);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Color buttons
   etk_box_append(ETK_BOX(hbox), create_a_color_button("Text",COLOR_OBJECT_TEXT,30,30, evas),
                  ETK_BOX_START, ETK_BOX_EXPAND, 0);
   etk_box_append(ETK_BOX(hbox), create_a_color_button("Shadow",COLOR_OBJECT_SHADOW,30,30, evas),
                  ETK_BOX_START, ETK_BOX_EXPAND, 0);
   etk_box_append(ETK_BOX(hbox), create_a_color_button("Outline",COLOR_OBJECT_OUTLINE,30,30, evas),
                  ETK_BOX_START, ETK_BOX_EXPAND, 0);

   etk_signal_connect("clicked", ETK_OBJECT(UI_FontAddButton),
                      ETK_CALLBACK(on_AllButton_click), (void*)TOOLBAR_FONT_FILE_ADD);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_FontComboBox),
                      ETK_CALLBACK(on_FontComboBox_item_activated), NULL);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_EffectComboBox),
                      ETK_CALLBACK(on_EffectComboBox_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FontSizeSpinner),
                      ETK_CALLBACK(on_FontSizeSpinner_value_changed), NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_TextEntry),
                      ETK_CALLBACK(on_TextEntry_text_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FontAlignVSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed), (void*)TEXT_ALIGNV_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FontAlignHSpinner),
                      ETK_CALLBACK(on_FontAlignSpinner_value_changed), (void*)TEXT_ALIGNH_SPINNER);
    
   return vbox;
}

static Etk_Widget*
create_position_frame(void)
{
   Etk_Widget *vbox;
   Etk_Widget *hbox;
   Etk_Widget *label;

/*    //Position Notebook
   notebook = etk_notebook_new();
   etk_container_add(ETK_CONTAINER(UI_PositionFrame), notebook);
 */
/*          //Simple TAB
         vbox = etk_vbox_new(ETK_FALSE, 0);
         etk_notebook_page_append(ETK_NOTEBOOK(notebook), "Simple", vbox);

            //hbox
            hbox = etk_hbox_new(ETK_FALSE, 0);
            etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

               label = etk_label_new("Container");
               etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
               etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

               //SimpleParentCombobox
               SimpleParentComboBox = etk_combobox_new();
               etk_combobox_column_add(ETK_COMBOBOX(SimpleParentComboBox), ETK_COMBOBOX_LABEL, 75, ETK_TRUE, ETK_FALSE, ETK_FALSE, 0.0, 0.5);
               etk_combobox_build(ETK_COMBOBOX(SimpleParentComboBox));
               etk_box_append(ETK_BOX(hbox), SimpleParentComboBox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
 */
   //ADVANCED TAB
   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 0);
   // etk_notebook_page_append(ETK_NOTEBOOK(notebook), "Advanced", vbox);

   label = etk_label_new("<color=#FF0000><b>First_Point</b></>");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(vbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<color=#FF0000><b>X</b></>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel1XSpinner
   UI_Rel1XSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_Rel1XSpinner), 2);
   etk_widget_size_request_set(UI_Rel1XSpinner,45, 20);
   etk_box_append(ETK_BOX(hbox),UI_Rel1XSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#FF0000>+</>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //Rel1XOffsetEntry
   UI_Rel1XOffsetSpinner =  etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_Rel1XOffsetSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox),UI_Rel1XOffsetSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#FF0000>to</>");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel1ToXCombobox
   UI_Rel1ToXComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel1ToXComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel1ToXComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_Rel1ToXComboBox));
   etk_box_append(ETK_BOX(hbox), UI_Rel1ToXComboBox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<color=#FF0000><b>Y</b></>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel1YSpinner
   UI_Rel1YSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_Rel1YSpinner), 2);
   etk_widget_size_request_set(UI_Rel1YSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel1YSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#FF0000>+</>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //Rel1YOffsetSpinner
   UI_Rel1YOffsetSpinner =  etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_Rel1YOffsetSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel1YOffsetSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#FF0000>to</> ");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel1ToYCombobox
   UI_Rel1ToYComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel1ToYComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel1ToYComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_Rel1ToYComboBox));
   etk_box_append(ETK_BOX(hbox), UI_Rel1ToYComboBox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<color=#0000FF><b>Second_Point</b></>");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",0.5,NULL);
   etk_box_append(ETK_BOX(vbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<color=#0000FF><b>X</b></>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel2XSpinner
   UI_Rel2XSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_Rel2XSpinner), 2);
   etk_widget_size_request_set(UI_Rel2XSpinner,45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel2XSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#0000FF>+</>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //Rel2XOffsetSpinner
   UI_Rel2XOffsetSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_Rel2XOffsetSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel2XOffsetSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#0000FF>to</>");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel2ToXCombobox
   UI_Rel2ToXComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel2ToXComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel2ToXComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_Rel2ToXComboBox));
   etk_box_append(ETK_BOX(hbox), UI_Rel2ToXComboBox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   label = etk_label_new("<color=#0000FF><b>Y</b></>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel2YSpinner
   UI_Rel2YSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_Rel2YSpinner), 2);
   etk_widget_size_request_set(UI_Rel2YSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel2YSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#0000FF>+</>");
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

   //Rel2YOffsetSpinner
   UI_Rel2YOffsetSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_Rel2YOffsetSpinner, 45, 20);
   etk_box_append(ETK_BOX(hbox), UI_Rel2YOffsetSpinner, ETK_BOX_START, ETK_BOX_NONE, 0);

   label = etk_label_new("<color=#0000FF>to</> ");
   etk_object_properties_set(ETK_OBJECT(label), "xalign",1.0,NULL);
   etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //Rel1ToYCombobox
   UI_Rel2ToYComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel2ToYComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_Rel2ToYComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_Rel2ToYComboBox));
   etk_box_append(ETK_BOX(hbox), UI_Rel2ToYComboBox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel1XOffsetSpinner),
      ETK_CALLBACK(on_RelOffsetSpinner_value_changed), (void *)REL1X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel1YOffsetSpinner),
      ETK_CALLBACK(on_RelOffsetSpinner_value_changed), (void *)REL1Y_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel2XOffsetSpinner),
      ETK_CALLBACK(on_RelOffsetSpinner_value_changed), (void *)REL2X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel2YOffsetSpinner),
      ETK_CALLBACK(on_RelOffsetSpinner_value_changed), (void *)REL2Y_SPINNER);

   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel1XSpinner),
      ETK_CALLBACK(on_RelSpinner_value_changed), (void *)REL1X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel1YSpinner),
      ETK_CALLBACK(on_RelSpinner_value_changed), (void *)REL1Y_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel2XSpinner),
      ETK_CALLBACK(on_RelSpinner_value_changed), (void *)REL2X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Rel2YSpinner),
      ETK_CALLBACK(on_RelSpinner_value_changed), (void *)REL2Y_SPINNER);

   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_Rel1ToXComboBox),
               ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1X_SPINNER);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_Rel1ToYComboBox),
               ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL1Y_SPINNER);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_Rel2ToXComboBox),
               ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2X_SPINNER);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_Rel2ToYComboBox),
               ETK_CALLBACK(on_RelToComboBox_changed), (void *)REL2Y_SPINNER);

   return vbox;
}

static Etk_Widget*
create_part_frame(void)
{
   Etk_Widget *table;
   Etk_Widget *label;
   Etk_Widget *frame;
   Etk_Widget *hbox;

   //table
   table = etk_table_new(2, 4, ETK_TABLE_NOT_HOMOGENEOUS);

   //PartNameEntry
   label = etk_label_new("<b>Name</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 0, 0,ETK_TABLE_NONE,0,0);
   UI_PartNameEntry = etk_entry_new();
   UI_PartNameEntryImage = etk_image_new_from_stock(ETK_STOCK_DIALOG_OK,
                                                    ETK_STOCK_SMALL);
   etk_entry_image_set(ETK_ENTRY(UI_PartNameEntry), ETK_ENTRY_IMAGE_SECONDARY,
                       ETK_IMAGE(UI_PartNameEntryImage));
   etk_table_attach_default(ETK_TABLE(table),UI_PartNameEntry, 1, 1, 0, 0);
   
   //UI_CliptoComboBox
   label = etk_label_new("<b>Clip to</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 1, 1,ETK_TABLE_NONE,0,0);

   UI_CliptoComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_CliptoComboBox),
                           ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_CliptoComboBox),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_CliptoComboBox));
   etk_table_attach_default(ETK_TABLE(table), UI_CliptoComboBox, 1, 1, 1, 1);
   
   //events frame
   frame = etk_frame_new("Mouse events");
   etk_table_attach(ETK_TABLE(table), frame, 0, 1, 2, 2, 
                     ETK_TABLE_EXPAND_FILL, 0, 0);
   //events hbox
   hbox = etk_hbox_new(ETK_FALSE, 0);
   etk_widget_padding_set(hbox, 10, 0, 0, 0);
   etk_container_add(ETK_CONTAINER(frame), hbox); 
   
   //PartEventsCheck
   UI_PartEventsCheck = etk_check_button_new_with_label("Accept</b>");
   etk_box_append(ETK_BOX(hbox), UI_PartEventsCheck,
                  ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   //PartEventRepeatCheck
   UI_PartEventsRepeatCheck = etk_check_button_new_with_label("Repeat</b>");
   etk_box_append(ETK_BOX(hbox), UI_PartEventsRepeatCheck,
                  ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   //UI_PartSourceComboBox
   UI_PartSourceLabel = etk_label_new("<b>Source</b>");
   etk_table_attach(ETK_TABLE(table), UI_PartSourceLabel,
                    0, 0, 3, 3, ETK_TABLE_NONE, 0, 0);

   UI_PartSourceComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_PartSourceComboBox),
                           ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_PartSourceComboBox));
   etk_table_attach_default(ETK_TABLE(table), UI_PartSourceComboBox, 1, 1, 3, 3);
   
   
   
   etk_signal_connect("text-changed", ETK_OBJECT(UI_PartNameEntry),
         ETK_CALLBACK(on_NamesEntry_text_changed), NULL);   
   etk_signal_connect("key-down", ETK_OBJECT(UI_PartNameEntry),
         ETK_CALLBACK(on_PartNameEntry_key_down), NULL);
   etk_signal_connect("mouse-click", ETK_OBJECT(UI_PartNameEntryImage),
                      ETK_CALLBACK(on_PartNameEntryImage_mouse_clicked), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsCheck),
                      ETK_CALLBACK(on_PartEventsCheck_toggled), NULL);
   etk_signal_connect("toggled", ETK_OBJECT(UI_PartEventsRepeatCheck),
                      ETK_CALLBACK(on_PartEventsRepeatCheck_toggled), NULL);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_CliptoComboBox),
                     ETK_CALLBACK(on_CliptoComboBox_item_activated), NULL);
   etk_signal_connect("item-activated", ETK_OBJECT(UI_PartSourceComboBox),
                      ETK_CALLBACK(on_PartSourceComboBox_item_activated), NULL);
   return table;
}

static Etk_Widget*
create_program_frame(void)
{
   Etk_Widget *table;
   Etk_Widget *label;
   
   
   //table
   table = etk_table_new(4, 10, ETK_TABLE_HHOMOGENEOUS);

   //UI_ProgramEntry + image
   label = etk_label_new("<b>Name</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 0, 0,ETK_TABLE_NONE,0,0);
   UI_ProgramEntry = etk_entry_new();
   etk_tooltips_tip_set(UI_ProgramEntry, "Symbolic <b>name</b> of "
                                         "program as a unique identifier.");
   UI_ProgramEntryImage = etk_image_new_from_stock(ETK_STOCK_DIALOG_OK,
                                                   ETK_STOCK_SMALL);
   etk_entry_image_set(ETK_ENTRY(UI_ProgramEntry), ETK_ENTRY_IMAGE_SECONDARY,
                       ETK_IMAGE(UI_ProgramEntryImage));
   etk_table_attach_default(ETK_TABLE(table),UI_ProgramEntry, 1, 2, 0, 0);
   
   
   //ScriptSaveButton
   UI_RunProgButton = etk_button_new_from_stock(ETK_STOCK_MEDIA_PLAYBACK_START);
   etk_object_properties_set(ETK_OBJECT(UI_RunProgButton),
                              "label","Run",NULL);
   etk_table_attach_default(ETK_TABLE(table),UI_RunProgButton, 3, 3, 0, 0);
   
   etk_signal_connect("clicked", ETK_OBJECT(UI_RunProgButton),
            ETK_CALLBACK(on_AllButton_click), (void*)RUN_PROG);
   
   //UI_SourceEntry
   label = etk_label_new("<b>Source</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 1, 1,ETK_TABLE_NONE,0,0);
   //etk_tooltips_tip_set(UI_SourceEntry, "<b>Source(s)</b> of the signal.<br>The Part or Program that emit the signal<br>Wildcards can be used to widen the scope, ex: \"button-*\"");
   UI_SourceEntry = etk_combobox_entry_new();
   etk_combobox_entry_column_add(ETK_COMBOBOX_ENTRY(UI_SourceEntry),
                  ETK_COMBOBOX_ENTRY_IMAGE, 24, ETK_COMBOBOX_ENTRY_NONE, 0.0);
   etk_combobox_entry_column_add(ETK_COMBOBOX_ENTRY(UI_SourceEntry),
                  ETK_COMBOBOX_ENTRY_LABEL, 75, ETK_COMBOBOX_ENTRY_EXPAND, 0.0);
   etk_combobox_entry_build(ETK_COMBOBOX_ENTRY(UI_SourceEntry));
   etk_table_attach_default(ETK_TABLE(table), UI_SourceEntry, 1, 3, 1, 1);

   //UI_SignalEntry
   label = etk_label_new("<b>Signal</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 2, 2,ETK_TABLE_NONE,0,0);
   
   UI_SignalEntry = etk_combobox_entry_new();
   etk_combobox_entry_column_add(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
                  ETK_COMBOBOX_ENTRY_IMAGE, 24, ETK_COMBOBOX_ENTRY_NONE, 0.0);
   etk_combobox_entry_column_add(ETK_COMBOBOX_ENTRY(UI_SignalEntry),
                  ETK_COMBOBOX_ENTRY_LABEL, 75, ETK_COMBOBOX_ENTRY_EXPAND, 0.0);
   etk_combobox_entry_build(ETK_COMBOBOX_ENTRY(UI_SignalEntry));
   etk_tooltips_tip_set(UI_SignalEntry, "The name of the <b>signal</b> that will trigger the program.<br>Wildcards can be used to widen the scope, ex: \"mouse,down,*\"<br>Can be a mouse signal ( mouse,in ; mouse,up,1 )<br>Or a user defined signal (emitted by the application)");
   etk_table_attach_default(ETK_TABLE(table),UI_SignalEntry, 1, 3, 2, 2);

   //UI_DelayFromSpinner
   label = etk_label_new("<b>Delay</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 3, 3,ETK_TABLE_NONE,0,0);
   UI_DelayFromSpinner = etk_spinner_new(0.0, 999.0, 0.0, 0.1, 1.0);
   etk_tooltips_tip_set(UI_DelayFromSpinner, "The number of seconds to wait before running the program");
   etk_spinner_digits_set(ETK_SPINNER(UI_DelayFromSpinner), 3);
   etk_widget_size_request_set(UI_DelayFromSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_DelayFromSpinner, 1, 1, 3, 3);

   //UI_DelayRangeSpinner
   label = etk_label_new("<b>+random</b>");
   etk_table_attach(ETK_TABLE(table), label, 2, 2, 3, 3,ETK_TABLE_NONE,0,0);
   UI_DelayRangeSpinner = etk_spinner_new(0.0, 999.0, 0.0, 0.1, 1.0);
   etk_tooltips_tip_set(UI_DelayRangeSpinner, "The maximum <b>random</b> seconds which is added to <b>Delay</b>");
   etk_spinner_digits_set(ETK_SPINNER(UI_DelayRangeSpinner), 3);
   etk_widget_size_request_set(UI_DelayRangeSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_DelayRangeSpinner, 3, 3, 3, 3);

   //UI_AfterEntry
   label = etk_label_new("<b>After(s)</b>");
   etk_table_attach (ETK_TABLE(table), label, 0, 0, 4, 4,ETK_TABLE_NONE,0,0);
   UI_AfterEntry = etk_entry_new();
   etk_tooltips_tip_set(UI_AfterEntry, "Specifies program(s) to run after the current program completes.<br>The <i>source</i> and <i>signal</i> parameters of a program run as an <i>after</j> are ignored.<br>Multiple programs can be specified separated by ',' (comma).");
   etk_table_attach_default(ETK_TABLE(table),UI_AfterEntry, 1, 3, 4, 4);

   //UI_ActionComboBox
   Etk_Combobox_Item *item = NULL;
   label = etk_label_new("<b>Action</b>");
   etk_table_attach(ETK_TABLE(table), label, 0, 0, 5, 5, ETK_TABLE_NONE, 0, 0);
   UI_ActionComboBox = etk_combobox_new();
   //etk_tooltips_tip_set(UI_ActionComboBox, "<b>Action</b> to be performed by the program.<br>STATE_SET is used to change the state of one or more targets parts<br>, ACTION_STOP and SIGNAL_EMIT.<br>");
   etk_combobox_column_add(ETK_COMBOBOX(UI_ActionComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_ActionComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_ActionComboBox));
   etk_table_attach_default(ETK_TABLE(table),UI_ActionComboBox, 1, 3, 5, 5);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "None");
   etk_combobox_item_data_set(item, (void*)EDJE_ACTION_TYPE_NONE);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "State Set");
   etk_combobox_item_data_set(item, (void*)EDJE_ACTION_TYPE_STATE_SET);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Action Stop");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_ACTION_STOP);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Signal Emit");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_SIGNAL_EMIT);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Drag val set");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_DRAG_VAL_SET);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Drag val step");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_DRAG_VAL_STEP);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Drag val page");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_DRAG_VAL_PAGE);

   item = etk_combobox_item_append(ETK_COMBOBOX(UI_ActionComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Embryo script");
   etk_combobox_item_data_set(ETK_COMBOBOX_ITEM(item),
                              (void*)EDJE_ACTION_TYPE_SCRIPT);
   
   //UI_TargetEntry
   UI_TargetLabel = etk_label_new("<b>Target(s)</b>");
   etk_table_attach(ETK_TABLE(table),UI_TargetLabel,0,0,6,6,ETK_TABLE_NONE,0,0);
   UI_TargetEntry = etk_entry_new();
   etk_tooltips_tip_set(UI_TargetEntry, "Program or part on which the specified action acts.<br>Multiple target keywords may be specified, separated by ','(comma).");
   etk_table_attach_default(ETK_TABLE(table), UI_TargetEntry, 1, 4, 6, 6);

   //UI_Param1Entry
   UI_Param1Label = etk_label_new("<b>State</b>");
   etk_table_attach(ETK_TABLE(table), UI_Param1Label, 0, 0, 7, 7,ETK_TABLE_NONE,0,0);
   UI_Param1Entry = etk_entry_new();
   etk_tooltips_tip_set(UI_Param1Entry, "Description to set in the specified targets");
   etk_table_attach_default(ETK_TABLE(table), UI_Param1Entry, 1, 2, 7, 7);

   //Param1Spinner
   UI_Param1Spinner = etk_spinner_new(-999.0, 999.0, 0.0, 0.1, 1.0);
   etk_tooltips_tip_set(UI_Param1Spinner, "Description to set in the specified targets");
   etk_spinner_digits_set(ETK_SPINNER(UI_Param1Spinner), 2);
   etk_widget_size_request_set(UI_Param1Spinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_Param1Spinner, 3, 3, 7, 7);


   //UI_TransiComboBox
   UI_TransiLabel = etk_label_new("<b>Transition</b>");
   etk_table_attach(ETK_TABLE(table), UI_TransiLabel, 0, 0, 8, 8,ETK_TABLE_NONE,0,0);
   UI_TransiComboBox = etk_combobox_new();
   etk_combobox_column_add(ETK_COMBOBOX(UI_TransiComboBox),
      ETK_COMBOBOX_IMAGE, 24, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_column_add(ETK_COMBOBOX(UI_TransiComboBox),
      ETK_COMBOBOX_LABEL, 75, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_TransiComboBox));
   etk_table_attach_default(ETK_TABLE(table),UI_TransiComboBox, 1, 2, 8, 8);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_TransiComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Linear");
   etk_combobox_item_data_set(item, (void*)EDJE_TWEEN_MODE_LINEAR);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_TransiComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Sinusoidal");
   etk_combobox_item_data_set(item, (void*)EDJE_TWEEN_MODE_SINUSOIDAL);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_TransiComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Accelerate");
   etk_combobox_item_data_set(item, (void*)EDJE_TWEEN_MODE_ACCELERATE);
   
   item = etk_combobox_item_append(ETK_COMBOBOX(UI_TransiComboBox),
                  etk_image_new_from_edje(EdjeFile,"DESC.PNG"), "Decelerate");
   etk_combobox_item_data_set(item, (void*)EDJE_TWEEN_MODE_DECELERATE);
   

   //DurationSpinner
   UI_DurationSpinner = etk_spinner_new(0.0, 999.0, 0.0, 0.1, 1.0);
   etk_tooltips_tip_set(UI_DurationSpinner, "The duration of the transition. In seconds.");
   etk_spinner_digits_set(ETK_SPINNER(UI_DurationSpinner), 3);
   etk_widget_size_request_set(UI_DurationSpinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_DurationSpinner, 3, 3, 8, 8);

   //UI_Param2Entry
   UI_Param2Label = etk_label_new("<b>Source</b>");
   etk_table_attach(ETK_TABLE(table), UI_Param2Label, 0, 0, 9, 9,ETK_TABLE_NONE,0,0);
   UI_Param2Entry = etk_entry_new();
   etk_tooltips_tip_set(UI_Param2Entry, "!!!!!!");
   etk_table_attach_default(ETK_TABLE(table), UI_Param2Entry, 1, 2, 9, 9);
   
   //Param2Spinner
   UI_Param2Spinner = etk_spinner_new(-999.0, 999.0, 0.0, 0.1, 1.0);
   etk_tooltips_tip_set(UI_Param2Spinner, "Used for drag operation");
   etk_spinner_digits_set(ETK_SPINNER(UI_Param2Spinner), 2);
   etk_widget_size_request_set(UI_Param2Spinner,45, 20);
   etk_table_attach_default(ETK_TABLE(table),UI_Param2Spinner, 3, 3, 9, 9);

   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_ActionComboBox),
         ETK_CALLBACK(on_ActionComboBox_changed), NULL);
   
   etk_signal_connect("text-changed", ETK_OBJECT(UI_ProgramEntry),
         ETK_CALLBACK(on_NamesEntry_text_changed), NULL);   
   etk_signal_connect("key-down", ETK_OBJECT(UI_ProgramEntry),
         ETK_CALLBACK(on_ProgramEntry_key_down), NULL);
   etk_signal_connect("mouse-click", ETK_OBJECT(UI_ProgramEntryImage),
                      ETK_CALLBACK(on_ProgramEntryImage_mouse_clicked), NULL);
   
   etk_signal_connect("text-changed", ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SourceEntry))),
         ETK_CALLBACK(on_SourceEntry_text_changed), UI_SourceEntry);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_SourceEntry),
         ETK_CALLBACK(on_SourceEntry_item_changed), NULL);
   
   etk_signal_connect("text-changed", ETK_OBJECT(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SignalEntry))),
         ETK_CALLBACK(on_SignalEntry_text_changed), UI_SignalEntry);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_SignalEntry),
         ETK_CALLBACK(on_SignalEntry_item_changed), NULL);
   
   etk_signal_connect("value-changed", ETK_OBJECT(UI_DelayFromSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed),NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_DelayRangeSpinner),
         ETK_CALLBACK(on_DelaySpinners_value_changed),NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_TargetEntry),
         ETK_CALLBACK(on_TargetEntry_text_changed), NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_Param1Entry),
         ETK_CALLBACK(on_Param1Entry_text_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Param1Spinner),
         ETK_CALLBACK(on_Param1Spinner_value_changed), NULL);
   etk_signal_connect("active-item-changed", ETK_OBJECT(UI_TransiComboBox),
         ETK_CALLBACK(on_TransitionComboBox_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_DurationSpinner),
         ETK_CALLBACK(on_DurationSpinner_value_changed), NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_AfterEntry),
         ETK_CALLBACK(on_AfterEntry_text_changed), NULL);
   etk_signal_connect("text-changed", ETK_OBJECT(UI_Param2Entry),
         ETK_CALLBACK(on_Param2Entry_text_changed), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_Param2Spinner),
         ETK_CALLBACK(on_Param2Spinner_value_changed), NULL);


   return table;
}

static Etk_Widget*
create_script_frame(void)
{
   Etk_Widget *sv;
   Etk_Widget *vbox;
   
   //vbox
   vbox = etk_vbox_new(ETK_FALSE, 0);

   //ScriptBox
   UI_ScriptBox = etk_text_view_new();
   sv = etk_scrolled_view_new();
   etk_scrolled_view_policy_set(ETK_SCROLLED_VIEW(sv),
                                 ETK_POLICY_AUTO, ETK_POLICY_AUTO);
   etk_bin_child_set(ETK_BIN(sv), UI_ScriptBox);
   etk_box_append(ETK_BOX(vbox), sv, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   //ScriptSaveButton
   UI_ScriptSaveButton = etk_button_new_from_stock(ETK_STOCK_DOCUMENT_SAVE);
   etk_object_properties_set(ETK_OBJECT(UI_ScriptSaveButton),
                              "label","Save script",NULL);
   etk_box_append(ETK_BOX(vbox), UI_ScriptSaveButton,
                  ETK_BOX_START, ETK_BOX_NONE, 0);

   etk_signal_connect("clicked", ETK_OBJECT(UI_ScriptSaveButton),
            ETK_CALLBACK(on_AllButton_click), (void*)SAVE_SCRIPT);
 
   return vbox;
}

static void 
_embed_position_set(void *position_data, int *x, int *y)
{
   ecore_evas_geometry_get(position_data, x, y, NULL, NULL);
}

void
create_main_window(void)
{
   //Create the main ecore_evas window
#if USE_GL_ENGINE
   UI_ecore_MainWin = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 32, 32);
#else
   UI_ecore_MainWin = ecore_evas_software_x11_new(NULL, 0, 0, 0, 32, 32);
#endif
   ecore_evas_title_set(UI_ecore_MainWin, "Edje Editor");
   ecore_evas_callback_resize_set(UI_ecore_MainWin, ecore_resize_callback);
   ecore_evas_callback_delete_request_set(UI_ecore_MainWin, ecore_delete_cb);
   ecore_evas_resize(UI_ecore_MainWin, 950, 500);
   ecore_evas_size_min_set(UI_ecore_MainWin, 600, 350);
   UI_evas = ecore_evas_get(UI_ecore_MainWin);
   
   //Load main edje interface
   edje_ui = edje_object_add(UI_evas);
   edje_object_file_set(edje_ui, EdjeFile, "MainUI");
   evas_object_move(edje_ui, 0, 0);
   evas_object_show(edje_ui);
   
   //Tooltips
   etk_tooltips_init();
   etk_tooltips_enable();

   //Create the evas objects needed by the canvas (fakewin, handlers)
   prepare_canvas();

   //Toolbar
   create_toolbar(ETK_TOOLBAR_HORIZ);
   UI_ToolbarEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_ToolbarEmbed), UI_Toolbar);
   etk_embed_position_method_set(ETK_EMBED(UI_ToolbarEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_ToolbarEmbed);
   evas_object_move(etk_embed_object_get(ETK_EMBED(UI_ToolbarEmbed)), 130, 0);
   evas_object_resize(etk_embed_object_get(ETK_EMBED(UI_ToolbarEmbed)), 0, 50);
    
   //Tree
   UI_PartsTreeEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_PartsTreeEmbed), create_tree());
   etk_embed_position_method_set(ETK_EMBED(UI_PartsTreeEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_PartsTreeEmbed);
   
   //Logo (keygrabber)
   Evas_Object *logo;
   logo = edje_object_add(UI_evas);
   edje_object_file_set(logo, EdjeFile, "Logo");
   evas_object_event_callback_add(logo, EVAS_CALLBACK_KEY_DOWN,
                                  on_Mainwin_key_press, NULL);
   Evas_Modifier_Mask mask;
   mask = evas_key_modifier_mask_get(UI_evas, "Control");
   evas_object_key_grab(logo, "q", mask, 0, 0); // quit
   evas_object_key_grab(logo, "f", mask, 0, 0); // fullscreen
   evas_object_key_grab(logo, "s", mask, 0, 0); // save
   evas_object_key_grab(logo, "c", mask, 0, 0); // copy selection (TODO)
   evas_object_key_grab(logo, "v", mask, 0, 0); // paste selection (TODO)
   evas_object_key_grab(logo, "x", mask, 0, 0); // cut selection (TODO)
   evas_object_key_grab(logo, "d", mask, 0, 0); // duplicate selection (TODO)
   evas_object_key_grab(logo, "n", mask, 0, 0); // new object (TODO)
   
   evas_object_show(logo);
   
   //GroupEmbed
   UI_GroupEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_GroupEmbed), create_group_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_GroupEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_GroupEmbed);
   edje_object_part_swallow(edje_ui,"group_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_GroupEmbed)));
    
   //PartEmbed
   UI_PartEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_PartEmbed), create_part_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_PartEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_PartEmbed);
   edje_object_part_swallow(edje_ui,"part_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_PartEmbed)));

   //DescriptionEmbed
   UI_DescriptionEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_DescriptionEmbed), create_description_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_DescriptionEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_DescriptionEmbed);
   edje_object_part_swallow(edje_ui,"description_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_DescriptionEmbed)));

   //RectEmbed
   UI_RectEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_RectEmbed), create_rectangle_frame(UI_evas));
   etk_embed_position_method_set(ETK_EMBED(UI_RectEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_RectEmbed);
   edje_object_part_swallow(edje_ui,"rect_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_RectEmbed)));

   //TextEmbed
   UI_TextEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_TextEmbed), create_text_frame(UI_evas));
   etk_embed_position_method_set(ETK_EMBED(UI_TextEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_TextEmbed);
   edje_object_part_swallow(edje_ui,"text_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_TextEmbed)));

   //ImageEmbed
   UI_ImageEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_ImageEmbed), create_image_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_ImageEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_ImageEmbed);
   edje_object_part_swallow(edje_ui,"image_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_ImageEmbed)));

   //PositionEmbed
   UI_PositionEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_PositionEmbed), create_position_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_PositionEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_PositionEmbed);
   edje_object_part_swallow(edje_ui,"position_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_PositionEmbed)));

   //ProgramEmbed
   UI_ProgramEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_ProgramEmbed), create_program_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_ProgramEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_ProgramEmbed);
   edje_object_part_swallow(edje_ui,"program_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_ProgramEmbed)));

   //ScriptEmbed
   UI_ScriptEmbed = etk_embed_new(UI_evas);
   etk_container_add(ETK_CONTAINER(UI_ScriptEmbed), create_script_frame());
   etk_embed_position_method_set(ETK_EMBED(UI_ScriptEmbed),
                                 _embed_position_set, UI_ecore_MainWin);
   etk_widget_show_all(UI_ScriptEmbed);
   edje_object_part_swallow(edje_ui,"script_frame_swallow",
                            etk_embed_object_get(ETK_EMBED(UI_ScriptEmbed)));
   
   //Filechooser
   UI_FileChooserDialog = create_filechooser_dialog();

   //Alert Dialog
   UI_AlertDialog = etk_message_dialog_new (ETK_MESSAGE_DIALOG_INFO,
                                       ETK_MESSAGE_DIALOG_OK, "Hallo world!");
   etk_widget_size_request_set(UI_AlertDialog, 240, 100);
   etk_signal_connect("delete-event", ETK_OBJECT(UI_AlertDialog),
                      ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   etk_signal_connect("response", ETK_OBJECT(UI_AlertDialog),
                      ETK_CALLBACK(on_AlertDialog_response), NULL);

   //ColorPicker
   UI_ColorWin = etk_window_new();
   etk_signal_connect("delete-event", ETK_OBJECT(UI_ColorWin),
                      ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   UI_ColorPicker = etk_colorpicker_new();
   etk_colorpicker_use_alpha_set (ETK_COLORPICKER(UI_ColorPicker), TRUE);
   etk_container_add(ETK_CONTAINER(UI_ColorWin), UI_ColorPicker);
   etk_signal_connect("color-changed", ETK_OBJECT(UI_ColorPicker),
                        ETK_CALLBACK(on_ColorDialog_change), NULL);

   //Image Browser
   create_image_browser();
   
   //Create the main edje object to edit
   edje_o = edje_object_add(UI_evas);
   edje_object_signal_callback_add(edje_o, "*", "*", signal_cb, NULL);
   evas_object_event_callback_add(edje_o, EVAS_CALLBACK_MOUSE_DOWN,
                                  on_Editing_click, NULL);


   /*edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"part_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"description_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"rect_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"text_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"position_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"image_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"program_frame_show","edje_editor");
   edje_object_signal_emit(edje_ui,"script_frame_show","edje_editor");*/
}
