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


/***   Callbacks   ***/
//~ Etk_Bool
//~ _consolle_button_click_cb(Etk_Button *button, void *data)
//~ {
   //~ const char *sig, *sou;
//~ 
   //~ sig = etk_entry_text_get(ETK_ENTRY(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SignalEmitEntry))));
   //~ sou = etk_entry_text_get(ETK_ENTRY(etk_combobox_entry_entry_get(ETK_COMBOBOX_ENTRY(UI_SourceEmitEntry))));
//~ 
   //~ edje_object_signal_emit(edje_o, sig, sou);
//~ 
   //~ _consolle_entry_item_append_ifnotexist(UI_SignalEmitEntry, sig);
   //~ _consolle_entry_item_append_ifnotexist(UI_SourceEmitEntry, sou);
//~ 
   //~ return ETK_TRUE;
//~ }

/***   Implementation   ***/

void
consolle_clear(void)
{
   //~ printf("CLEAR\n");
   edje_object_part_text_set(ui.edje_ui, "line1", "");
   edje_object_part_text_set(ui.edje_ui, "line2", "");
   edje_object_part_text_set(ui.edje_ui, "line3", "");
   edje_object_part_text_set(ui.edje_ui, "line4", "");
   edje_object_part_text_set(ui.edje_ui, "line5", "");
   edje_object_part_text_set(ui.edje_ui, "line6", "");
   edje_object_part_text_set(ui.edje_ui, "line7", "");
   edje_object_part_text_set(ui.edje_ui, "line8", "");

   while(stack)
   {
      eina_stringshare_del(eina_list_data_get(stack));
      stack = eina_list_remove_list(stack, stack);
   }
   consolle_count = 0;
}

void
consolle_log(char *text)
{
   //~ printf("LOG: %s\n", text);

   stack = eina_list_prepend(stack, eina_stringshare_add(text));

   while (eina_list_count(stack) > 8)
   {
      eina_stringshare_del(eina_list_data_get(eina_list_last(stack)));
      stack = eina_list_remove_list(stack, eina_list_last(stack));
   }

   edje_object_part_text_set(ui.edje_ui, "line1", eina_list_nth(stack, 0));
   edje_object_part_text_set(ui.edje_ui, "line2", eina_list_nth(stack, 1));
   edje_object_part_text_set(ui.edje_ui, "line3", eina_list_nth(stack, 2));
   edje_object_part_text_set(ui.edje_ui, "line4", eina_list_nth(stack, 3));
   edje_object_part_text_set(ui.edje_ui, "line5", eina_list_nth(stack, 4));
   edje_object_part_text_set(ui.edje_ui, "line6", eina_list_nth(stack, 4));
   edje_object_part_text_set(ui.edje_ui, "line7", eina_list_nth(stack, 4));
   edje_object_part_text_set(ui.edje_ui, "line8", eina_list_nth(stack, 4));
}
