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

//~ #define _GNU_SOURCE
#include "main.h"

/***   Parts Tree 'model'   ***/
Elm_Genlist_Item_Class parts_class;
Elm_Genlist_Item_Class states_class;
Elm_Genlist_Item_Class progs_class;

static char*
_tree_model_label_get(const void *data, Evas_Object *obj, const char *source)
{
   //~ printf("LABEL_GET: %s\n", (char*) data);
   return strdup(data); // NOTE this will be free() by the caller
}

static Evas_Object*
_tree_model_part_icon_get(const void *data, Evas_Object *obj, const char *source)
{
   Evas_Object *ic;
   char *part = (char*)data;

   //~ printf("ICON GET for part[%d]: %s (source: %s)\n", type, part, source);

   if (!strcmp(source, "elm.swallow.icon"))
   {
      ic = elm_icon_add(obj);
      elm_icon_file_set(ic, EdjeFile, part_type_image_get2(part));
      evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      evas_object_size_hint_max_set(ic, 16, 16);
      evas_object_show(ic);
      return ic;
   }

   return NULL;
}

static Evas_Object*
_tree_model_state_icon_get(const void *data, Evas_Object *obj, const char *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon"))
      return NULL;

   ic = elm_icon_add(obj);
   elm_icon_file_set(ic, EdjeFile, "DESC.PNG");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_size_hint_max_set(ic, 16, 16);
   evas_object_show(ic);
   return ic;
}

static Evas_Object*
_tree_model_prog_icon_get(const void *data, Evas_Object *obj, const char *source)
{
   Evas_Object *ic;

   if (strcmp(source, "elm.swallow.icon"))
      return NULL;

   ic = elm_icon_add(obj);
   elm_icon_file_set(ic, EdjeFile, "PROG.PNG");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_size_hint_max_set(ic, 16, 16);
   evas_object_show(ic);
   return ic;
}

static Eina_Bool 
_tree_model_state_get(const void *data, Evas_Object *obj, const char *source)
{
   return EINA_FALSE;
}

static void
_tree_model_del(const void *data, Evas_Object *obj)
{
   printf("DEL DATA\n");
   eina_stringshare_del(data);
}


static void
_tree_model_part_sel(void *data, Evas_Object *obj, void *event_info)
{
   const char *part = elm_genlist_item_data_get(event_info);

   printf("SELECTED PART: %s\n", part);
   
   set_current_part(part);
   set_current_state("default 0.0");
   set_current_prog(NULL);
   set_current_tween(NULL);


   window_update_frames_visibility();
   
   part_frame_update();
   state_frame_update();
   position_frame_update();
   canvas_redraw();
}

static void
_tree_model_state_sel(void *data, Evas_Object *obj, void *event_info)
{
   const char *state = elm_genlist_item_data_get(event_info);
   const char *parent = elm_genlist_item_data_get(
                           elm_genlist_item_parent_get(event_info));

   printf("SELECTED STATE: %s [parent: %s]\n", state, parent);
   
   set_current_part(parent);
   set_current_state(state);
   set_current_prog(NULL);
   set_current_tween(NULL);
   
   edje_edit_part_selected_state_set(ui.edje_o, cur.part, cur.state);  

   state_frame_update();
   position_frame_update();
   //~ position_comboboxes_update();

   
   canvas_redraw();
}

static void
_tree_model_prog_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("SEL PROG\n");
}

static void
_tree_model_expand(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Evas_Object *tree = data;
   const char *part = elm_genlist_item_data_get(it);
   Eina_List *states, *l;
   char *state;

   printf("EXPAND %s\n", part);

   states = edje_edit_part_states_list_get(ui.edje_o, part);
   EINA_LIST_FOREACH(states, l, state)
   {
      printf("  state: %s\n", state);
      elm_genlist_item_append(tree, &states_class,
                              eina_stringshare_add(state)/* item data */,
                              it/* parent */,
                              ELM_GENLIST_ITEM_NONE,
                              _tree_model_state_sel/* func */,
                              NULL/* func data */);
   }
   edje_edit_string_list_free(states);
}

static void
_tree_model_contract(void *data, Evas_Object *obj, void *event_info)
{
   printf("CONTRACT %s\n", (char*)data);
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
_tree_model_expand_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}

static void
_tree_model_contract_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

/***   Parts Tree   ***/
static void
_tree_btn_click_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("Click\n");
   tree_groups_create();
}

static void
_tree_emitter_populate(Evas_Object *o)
{
   Eina_List *progs, *l;
   const char *name;

   //~ hoversel_clear(o);

   progs = edje_edit_programs_list_get(ui.edje_o);
   EINA_LIST_FOREACH(progs, l, name)
     {
        const char *signal, *source;
        char buf[1024];
        //~ Hoversel_Item_Data *data;

        signal = edje_edit_program_signal_get(ui.edje_o, name); //TODO free?
        if (!signal) continue;
        source = edje_edit_program_source_get(ui.edje_o, name);//TODO free?

        snprintf(buf, sizeof(buf), "%s : %s", signal, source);
        //~ data = calloc(1, sizeof(Hoversel_Item_Data));
        //~ data->grp = grp;
        //~ data->signal = signal;
        //~ data->source = source;
        elm_hoversel_item_add(o, buf, NULL, ELM_ICON_NONE, /*on_hover_signal_select*/ NULL, NULL);
        //~ data->label = eina_stringshare_add(buf);

        //~ grp->v->hoversel_items = eina_list_append(
            //~ grp->v->hoversel_items, data);
     }
   edje_edit_string_list_free(progs);
}

void 
tree_parts_create(void)
{
   Evas_Object *bt, *tree, *table, *label;
   Eina_List *parts, *progs, *l;
   char *name;
   
   printf("populate parts\n");
   
   table = elm_table_add(ui.win);
   elm_table_homogenous_set(table, 0);

   // 'Change Group' Button
   bt = elm_button_add(ui.win);
   elm_button_label_set(bt, "Change");
   evas_object_smart_callback_add(bt, "clicked", _tree_btn_click_cb, NULL);
   evas_object_size_hint_align_set(bt, 0.0, 0.0);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0); // TODO this doesn't work as expected
   elm_table_pack(table, bt, 0, 0, 1, 2);
   evas_object_show(bt);
   
   label = elm_label_add(ui.win);
   elm_label_label_set(label, "<b>Current group:</b>");
   evas_object_size_hint_align_set(label, 0.0, 1.0);
   evas_object_size_hint_weight_set(label, 1.0, 0.0); // This should expand the label
   elm_table_pack(table, label, 1, 0, 1, 1);
   evas_object_show(label);

   label = elm_label_add(ui.win);
   elm_label_label_set(label, cur.group);
   evas_object_size_hint_align_set(label, 0.0, 0.0);
   evas_object_size_hint_weight_set(label, 1.0, 0.0); // This should expand the label
   elm_table_pack(table, label, 1, 1, 1, 1);
   evas_object_show(label);
   
   parts_class.item_style     = "default";
   parts_class.func.label_get = _tree_model_label_get;
   parts_class.func.icon_get  = _tree_model_part_icon_get;
   parts_class.func.state_get = _tree_model_state_get;
   parts_class.func.del       = _tree_model_del;
   
   states_class.item_style     = "default";
   states_class.func.label_get = _tree_model_label_get;
   states_class.func.icon_get  = _tree_model_state_icon_get;
   states_class.func.state_get = _tree_model_state_get;
   states_class.func.del       = _tree_model_del;
   
   progs_class.item_style     = "default";
   progs_class.func.label_get = _tree_model_label_get;
   progs_class.func.icon_get  = _tree_model_prog_icon_get;
   progs_class.func.state_get = _tree_model_state_get;
   progs_class.func.del       = _tree_model_del;
   
   tree = elm_genlist_add(ui.win);
   evas_object_size_hint_align_set(tree, -1.0, -1.0);
   evas_object_size_hint_weight_set(tree, 1.0, 1.0);
   elm_table_pack(table, tree, 0, 2, 2, 1);
   evas_object_show(tree);
   ui.parts_tree = tree;

   
   //Signal emitter
   Evas_Object *o;

   o = elm_hoversel_add(ui.win);
   elm_hoversel_hover_parent_set(o, ui.win);
   elm_hoversel_label_set(o, "Signal Emitter");
   evas_object_size_hint_weight_set(o, 1.0, 0.0);
   elm_table_pack(table, o, 0, 3, 2, 1);
   evas_object_show(o);
   
   _tree_emitter_populate(o);
   //~ elm_hoversel_item_add(o, "asd", NULL, ELM_ICON_NONE, /*on_hover_signal_select*/NULL, NULL);
   //~ elm_hoversel_item_add(o, "asd3", NULL, ELM_ICON_NONE, /*on_hover_signal_select*/NULL, NULL);


   parts = edje_edit_parts_list_get(ui.edje_o);
   EINA_LIST_FOREACH(parts, l, name)
   {
      printf("  PART: %s\n", name);
      elm_genlist_item_append(tree, &parts_class,
                           eina_stringshare_add(name), /* item data */ //NOTE free() by _tree_model_del()
                           NULL/* parent */,
                           ELM_GENLIST_ITEM_SUBITEMS,
                           _tree_model_part_sel /* func */,
                           NULL/* func data */);
   
   }
   edje_edit_string_list_free(parts);
   
   progs = edje_edit_programs_list_get(ui.edje_o);
   EINA_LIST_FOREACH(progs, l, name)
   {
      //~ printf("  PROG: %s\n", name);
      elm_genlist_item_append(tree, &progs_class,
                           eina_stringshare_add(name), /* item data */ //NOTE free() by _tree_model_del()
                           NULL/* parent */,
                           ELM_GENLIST_ITEM_NONE,
                           _tree_model_prog_sel /* func */,
                           NULL/* func data */);
   }
   edje_edit_string_list_free(progs);

   evas_object_smart_callback_add(tree, "expand,request", _tree_model_expand_req, tree);
   evas_object_smart_callback_add(tree, "contract,request", _tree_model_contract_req, tree);
   evas_object_smart_callback_add(tree, "expanded", _tree_model_expand, tree);
   evas_object_smart_callback_add(tree, "contracted", _tree_model_contract, tree);

   // Push the tree in the tree pager (aka: show it)
   elm_pager_content_pop(ui.tree_pager);
   elm_pager_content_push(ui.tree_pager, table);
}

/***   Groups Tree   ***/
void _tree_group_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *item;
   
   item = elm_list_selected_item_get(obj);
   printf("CLICK on: %s\n", elm_list_item_label_get(item));
   change_to_group(elm_list_item_label_get(item));
}

void
tree_groups_create(void)
{
   Eina_List *groups, *l;
   char *name;
   Evas_Object *box, *lb, *list;

   box = elm_box_add(ui.win);
   evas_object_size_hint_align_set(box, 1.0, 1.0);
   evas_object_show(box);
   
   // label
   lb = elm_label_add(ui.win);
   elm_label_label_set(lb, "<b>Select a group</b>");
   elm_box_pack_end(box, lb);
   evas_object_show(lb);
   
   // list
   list = elm_list_add(ui.win);
   evas_object_size_hint_align_set(list, -1.0, -1.0);
   evas_object_size_hint_weight_set(list, 1.0, 1.0);
   elm_box_pack_end(box, list);


   // Populate the list
   Elm_List_Item *item;
   groups = edje_file_collection_list(cur.open_temp_name);
   EINA_LIST_FOREACH(groups, l, name)
      item = elm_list_item_append(list, name, NULL, NULL, _tree_group_sel_cb, NULL);
   edje_file_collection_list_free(groups);
   
   // Run the list
   elm_list_go(list);
   evas_object_show(list);
   
   
   // Push the list in the tree pager (aka: show it)
   elm_pager_content_pop(ui.tree_pager);
   elm_pager_content_push(ui.tree_pager, box);
   

   //Select the first group and load it
   //~ etk_combobox_entry_active_item_set(ETK_COMBOBOX_ENTRY(UI_GroupsComboBox),
      //~ etk_combobox_entry_first_item_get(ETK_COMBOBOX_ENTRY(UI_GroupsComboBox)));
   //~ 
   //~ etk_combobox_entry_autosearch_set(ETK_COMBOBOX_ENTRY(UI_GroupsComboBox),
                                     //~ GROUP_COMBO_AUTOSEARCH_COL, NULL);
}
//~ 
//~ Etk_Tree_Row *
//~ tree_part_add(const char *part_name, Etk_Tree_Row *after)
//~ {
   //~ /* If after=0 then append to the tree (but before programs)
      //~ If after=1 then prepend to the tree
      //~ If after>1 then prepend relative to after
      //~ 
      //~ I hope no one get a real row pointer == 1  :P
   //~ */
   //~ Etk_Tree_Row *row = NULL;
   //~ char *buf;
//~ 
   //~ //printf("Add Part to tree: %s\n",par->name);
   //~ 
   //~ /* Search for the last row that isn't a program */
   //~ if (after == 0)
   //~ {
      //~ int row_type;
      //~ 
      //~ after = etk_tree_last_row_get(ETK_TREE(UI_PartsTree));
      //~ etk_tree_row_fields_get(after, COL_TYPE, &row_type, NULL);
      //~ 
      //~ while (after && row_type && row_type == ROW_PROG)
      //~ {
         //~ after = etk_tree_row_prev_get(after);
         //~ etk_tree_row_fields_get(after, COL_TYPE, &row_type, NULL);
      //~ }
   //~ }
//~ 
   //~ /* Add the row in the right position */
   //~ buf = part_type_image_get(part_name);
   //~ if ((int)(long)after > 1)
      //~ row = etk_tree_row_insert(ETK_TREE(UI_PartsTree),
                                //~ NULL,
                                //~ after,
                                //~ COL_NAME, EdjeFile, buf, part_name,
                                //~ COL_TYPE, ROW_PART,
                                //~ NULL);
   //~ else if ((int)(long)after == 1)
      //~ row = etk_tree_row_prepend(ETK_TREE(UI_PartsTree),
                                //~ NULL,
                                //~ COL_NAME, EdjeFile, buf, part_name,
                                //~ COL_TYPE, ROW_PART,
                                //~ NULL);
   //~ else
      //~ row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
                                //~ NULL,
                                //~ COL_NAME, EdjeFile, buf, part_name,
                                //~ COL_TYPE, ROW_PART,
                                //~ NULL);
//~ 
   //~ if (!Parts_Hash) Parts_Hash = eina_hash_string_superfast_new(NULL);
   //~ eina_hash_add(Parts_Hash, part_name, row);
//~ 
   //~ /* also add all state to the tree */
   //~ Eina_List *states, *sp;
   //~ states = sp = edje_edit_part_states_list_get(edje_o, part_name);
   //~ while(sp)
   //~ {
      //~ tree_state_add(part_name, (char*)sp->data);
      //~ sp = sp->next;
   //~ }
   //~ edje_edit_string_list_free(states);
   //~ free(buf);
//~ 
   //~ return row;
//~ }
//~ 
//~ Etk_Tree_Row *
//~ tree_state_add(const char *part_name, const char *state_name)
//~ {
   //~ Etk_Tree_Row *row;
//~ 
   //~ const char *stock_key;
//~ 
   //~ stock_key = etk_stock_key_get(ETK_STOCK_TEXT_X_GENERIC, ETK_STOCK_SMALL);
   //~ row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
            //~ eina_hash_find(Parts_Hash,part_name),
            //~ COL_NAME, EdjeFile, "DESC.PNG", state_name,
            //~ COL_VIS, TRUE,
            //~ COL_TYPE, ROW_DESC,
            //~ COL_PARENT, part_name, NULL);
   //~ return row;
//~ }
//~ 
//~ Etk_Tree_Row *
//~ tree_program_add(const char* prog)
//~ {
   //~ Etk_Tree_Row *row = NULL;
//~ 
   //~ //printf("Add Program to tree: %s\n",prog->name);
   //~ row = etk_tree_row_append(ETK_TREE(UI_PartsTree),
               //~ NULL,
               //~ COL_NAME, EdjeFile,"PROG.PNG", prog,
               //~ COL_TYPE,ROW_PROG,
               //~ NULL);
//~ 
   //~ return row;
//~ }
//~ 
//~ 
//~ /***   Tree callbacks   ***/
//~ Etk_Bool
//~ _tree_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
//~ {
   //~ int row_type=0;
   //~ char *name;
   //~ char *parent_name;
//~ 
   //~ printf("Row Selected Signal on one of the Tree EMITTED \n");
//~ 
   //~ //get the info from the tree cols of the selected row
   //~ etk_tree_row_fields_get(row,
                           //~ COL_TYPE, &row_type,
                           //~ COL_NAME,NULL, NULL, &name,
                           //~ COL_PARENT, &parent_name,
                           //~ NULL);
//~ 
   //~ switch (row_type)
   //~ {
      //~ case ROW_PART:

      //~ case ROW_DESC:
         //~ Cur.state = etk_string_set(Cur.state, name);
         //~ Cur.part = etk_string_set(Cur.part, parent_name);
         //~ Cur.tween = etk_string_clear(Cur.tween);
         //~ Cur.prog = etk_string_clear(Cur.prog);
//~ 
         //~ edje_edit_part_selected_state_set(edje_o, Cur.part->string,
                                           //~ Cur.state->string);  
//~ 
         //~ state_frame_update();
         //~ position_frame_update();
         //~ position_comboboxes_update();
//~ 
         //~ switch(edje_edit_part_type_get(edje_o, Cur.part->string))
         //~ {
            //~ case EDJE_PART_TYPE_RECTANGLE:
               //~ rectangle_frame_update();
               //~ edje_object_signal_emit(edje_ui,"rect_frame_show","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"fill_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"gradient_frame_hide","edje_editor");
               //~ break;
            //~ case EDJE_PART_TYPE_IMAGE:
               //~ image_frame_update();
               //~ fill_frame_update();
               //~ edje_object_signal_emit(edje_ui,"image_frame_show","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"fill_frame_show","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"gradient_frame_hide","edje_editor");
               //~ break;
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
            //~ case EDJE_PART_TYPE_TEXT:
               //~ text_frame_update();
               //~ edje_object_signal_emit(edje_ui,"text_frame_show","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"fill_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
               //~ edje_object_signal_emit(edje_ui,"gradient_frame_hide","edje_editor");
               //~ break;
         //~ }
//~ 
         //~ edje_object_signal_emit(edje_ui,"part_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"program_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"script_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"state_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"position_frame_show","edje_editor");
         //~ break;
//~ 
      //~ case ROW_PROG:
         //~ Cur.prog = etk_string_set(Cur.prog, name);
         //~ Cur.part = etk_string_clear(Cur.part);
         //~ Cur.state = etk_string_clear(Cur.state);
         //~ Cur.tween = etk_string_clear(Cur.tween);
//~ 
         //~ edje_object_signal_emit(edje_ui,"state_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"position_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"rect_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"image_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"gradient_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"fill_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"text_frame_hide","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"group_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"part_frame_hide","edje_editor");
//~ 
         //~ edje_object_signal_emit(edje_ui,"program_frame_show","edje_editor");
         //~ edje_object_signal_emit(edje_ui,"script_frame_show_small","edje_editor");
//~ 
         //~ script_frame_update();
         //~ program_frame_update();
        //~ // PopulateSourceComboBox();
         //~ break;
   //~ }
//~ 
   //~ canvas_redraw();
   //~ return ETK_TRUE;
//~ }
//~ 
//~ Etk_Bool
//~ _tree_click_cb(Etk_Tree *tree, Etk_Tree_Row *row, Etk_Event_Mouse_Up *event, void *data)
//~ {
   //~ if ((event->flags == ETK_MOUSE_DOUBLE_CLICK) && etk_string_length_get(Cur.prog))
      //~ edje_edit_program_run(edje_o, Cur.prog->string);
//~ 
   //~ return ETK_TRUE;
//~ }
//~ 
//~ 
//~ /***   Group combobox callback   ***/
//~ Etk_Bool
//~ _tree_combobox_active_item_changed_cb(Etk_Combobox_Entry *combobox_entry, void *data)
//~ {
   //~ char *gr;
   //~ Etk_Combobox_Entry_Item *item;
   //~ 
   //~ item = etk_combobox_entry_active_item_get(combobox_entry);
   //~ gr = etk_combobox_entry_item_field_get(item, 0);
   //~ printf("Group combo activated: %s\n",gr);
   //~ change_group(gr);
   //~ 
   //~ etk_entry_text_set(ETK_ENTRY(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_GroupsComboBox))), gr);
//~ 
   //~ return ETK_TRUE;
//~ }
