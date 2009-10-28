/*  Copyright (C) 2006-2008 Davide Andreoli (see AUTHORS)
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

static Evas_Object *_name_entry;
static Evas_Object *_delay_entry;
static Evas_Object *_delayrandom_entry;
static Evas_Object *_source_combo;
static Evas_Object *_signal_combo;

/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   int i;
   char val_buf[128];
   
   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, probably a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
   {
      txt[strlen(txt) - 4] = '\0';
   }
   printf("Apply entry [%s]\n", txt);

   if (!txt || !cur.prog) return;
   
   /* Apply Name */
   else if (o  == _name_entry)
   {
      // TODO: error handling of return
      edje_edit_program_name_set(ui.edje_o, cur.prog, txt);
      set_current_prog(txt);
      // TODO: should also change the label in the parts selection on the left
      //       currently a group change is needed to update the label
   }
   /* Apply Delay */
   else if (o == _delay_entry)
   {
      if (sscanf(val_buf, "%f", elm_entry_entry_get(o)) != 1)
      {
        dialog_alert_show(MSG_INT);
      }
      else
      {
        // TODO: error handling of return
        edje_edit_program_in_from_set (ui.edje_o, cur.prog, 0.0);
      }
     
   /*snprintf (val_buf, sizeof (val_buf), "%.4f", edje_edit_program_in_from_get(ui.edje_o, cur.prog));
   elm_entry_entry_set(_delay_entry, val_buf);
   snprintf (val_buf, sizeof (val_buf), "%.4f", edje_edit_program_in_range_get(ui.edje_o, cur.prog));
   elm_entry_entry_set(_delayrandom_entry, val_buf);*/
   }
  
   if (txt)
   { 
      free (txt);
   }

   canvas_redraw();
   program_frame_update();
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
      text_frame_update();
   }
}

static void
_source_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   const char *source = (const char*) data;

   edje_edit_program_source_set (ui.edje_o, cur.prog, source);
  
   canvas_redraw();
   program_frame_update();
}

static void
_signal_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   const char *signal = (const char*) data;

   edje_edit_program_signal_set (ui.edje_o, cur.prog, signal);
  
   canvas_redraw();
   program_frame_update();
}

static void
_run_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
   // TODO: run script
   //fonts_browser_show(ui.win);
}

/***   Implementation   ***/
Evas_Object*
program_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *bt, *_o;
   Elm_Hoversel_Item *it;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("name:", 0, 0, 1, _name_entry, EINA_TRUE)

   bt = elm_button_add(parent);
   elm_button_label_set(bt, "run");
   evas_object_size_hint_align_set(bt, 1.0, 0.5);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_smart_callback_add(bt, "clicked", _run_button_clicked, NULL);
   evas_object_show(bt);
   
   // create source
   NEW_COMBO_TO_TABLE( _source_combo, "source:", 0, 3, 2, NULL, NULL)
   
   // create signal
   NEW_COMBO_TO_TABLE( _signal_combo, "signal:", 0, 4, 2, NULL, NULL)
    
   it = elm_hoversel_item_add(_signal_combo, "program,start", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "program,start");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);

   it = elm_hoversel_item_add(_signal_combo, "program,stop", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "program,stop");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "load", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "load");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "show", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "show");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "hide", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "hide");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "resize", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "resize");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "mouse,in", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,in");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "mouse,out", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,out");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
   
   it = elm_hoversel_item_add(_signal_combo, "mouse,move", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,move");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "mouse,down,1", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,down,1");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "mouse,up,1", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,up,1");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);
  
   it = elm_hoversel_item_add(_signal_combo, "mouse,clicked,1", NULL, ELM_ICON_NONE,
                              _signal_combo_sel, "mouse,clicked,1");
   elm_hoversel_item_icon_set(it, EdjeFile, "DESC.PNG", ELM_ICON_FILE);

   NEW_DOUBLE_ENTRY_TO_TABLE("delay:", 0, 5, _delay_entry, _delayrandom_entry, EINA_TRUE)

   return tb;
}

void
program_frame_update(void)
{
   const char *s;
   char val_buf[128];

   if (!cur.prog) return;
   
   //Set name
   elm_entry_entry_set(_name_entry, cur.prog);

   // Set source
   s = edje_edit_program_source_get (ui.edje_o, cur.prog);
   if (!s)
   {
      elm_hoversel_label_set(_source_combo, "none"); // TODO: minimal width
   }
   else
   {
      elm_hoversel_label_set(_source_combo, s); // TODO: minimal width
   }
   edje_edit_string_free(s);
  
   //Update Signal
   s = edje_edit_program_signal_get(ui.edje_o, cur.prog);
   if (!s)
   {
      elm_hoversel_label_set(_signal_combo, "none"); // TODO: minimal width
   }
   else
   {
      elm_hoversel_label_set(_signal_combo, s); // TODO: minimal width
   }
   edje_edit_string_free(s);
  
   //Update Delay
   snprintf (val_buf, sizeof (val_buf), "%.4f", edje_edit_program_in_from_get(ui.edje_o, cur.prog));
   elm_entry_entry_set(_delay_entry, val_buf);
   snprintf (val_buf, sizeof (val_buf), "%.4f", edje_edit_program_in_range_get(ui.edje_o, cur.prog));
   elm_entry_entry_set(_delayrandom_entry, val_buf);

}

void
program_source_combo_populate(void)
{
   Eina_List *l;
   Elm_Hoversel_Item *it;
   char *image_name;
   printf("Populate Program Source ComboEntry\n");
  
   // delete label and items before adding new
   elm_hoversel_label_set(_source_combo, "");
   l = elm_hoversel_items_get (_source_combo);
   while (l)
   {
      Elm_Hoversel_Item *it = (Elm_Hoversel_Item*) l->data;
     
      elm_hoversel_item_del (it);
     
      l = l->next;
   }

   l = edje_edit_parts_list_get(ui.edje_o);
   while (l)
   {
      image_name = part_type_image_get((char*)l->data);
      it = elm_hoversel_item_add(_source_combo, (char*)l->data, NULL, ELM_ICON_NONE,
                                 _source_combo_sel, l->data);
      elm_hoversel_item_icon_set(it, EdjeFile, image_name, ELM_ICON_FILE);
      free(image_name);

      l = l->next;
   }
   edje_edit_string_list_free(l);
}

