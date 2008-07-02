#include <Etk.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include "main.h"
//#include "callbacks.h"
//#include "interface.h"




Etk_Widget*
create_tree(void)
{
   //UI_GroupsComboBox
   UI_GroupsComboBox = etk_combobox_new();
   etk_combobox_items_height_set(ETK_COMBOBOX(UI_GroupsComboBox), 18);
   etk_combobox_column_add(ETK_COMBOBOX(UI_GroupsComboBox), ETK_COMBOBOX_LABEL, 30, ETK_COMBOBOX_NONE, 0.0);
   etk_combobox_build(ETK_COMBOBOX(UI_GroupsComboBox));
   
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
                    _part_SourceComboBox_item_activated_cb, NULL);
   
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
                      _part_SourceComboBox_item_activated_cb, NULL);
   
   //Select the first group and load it
   etk_combobox_active_item_set(ETK_COMBOBOX(UI_GroupsComboBox),
      etk_combobox_first_item_get(ETK_COMBOBOX(UI_GroupsComboBox)));
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
         
         part_frame_update();
         break;

      case ROW_DESC:
         Cur.state = etk_string_set(Cur.state, name);
         Cur.part = etk_string_set(Cur.part, parent_name);
         Cur.tween = etk_string_clear(Cur.tween);
         Cur.prog = etk_string_clear(Cur.prog);
       
         edje_edit_part_selected_state_set(edje_o, Cur.part->string, Cur.state->string);  
         
         UpdateDescriptionFrame();
         position_frame_update();
         position_comboboxes_update();
       
         switch(edje_edit_part_type_get(edje_o, Cur.part->string))
         {
            case EDJE_PART_TYPE_RECTANGLE:
               rectangle_frame_update();
               edje_object_signal_emit(edje_ui,"rect_frame_show","edje_editor");
               edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
               edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
               break;
            case EDJE_PART_TYPE_IMAGE:
               image_frame_update();
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
      
         script_frame_update();
         program_frame_update();
        // PopulateSourceComboBox();
         break;
   }

   canvas_redraw();
   return ETK_TRUE;
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

Etk_Bool
on_PartsTree_click(Etk_Tree *tree, Etk_Tree_Row *row, Etk_Event_Mouse_Up *event, void *data)
{
   if ((event->flags == ETK_MOUSE_DOUBLE_CLICK) && etk_string_length_get(Cur.prog))
      edje_edit_program_run(edje_o, Cur.prog->string);
   
   return ETK_TRUE;
}
