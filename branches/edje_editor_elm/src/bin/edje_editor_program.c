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

static Evas_Object *_text_entry;
static Evas_Object *_name_entry;
static Evas_Object *_size_entry;
static Evas_Object *_alignx_entry;
static Evas_Object *_aligny_entry;
static Evas_Object *_source_combo;
static Evas_Object *_signal_combo;
static Evas_Object *_color_entry;
static Evas_Object *_color2_entry;
static Evas_Object *_color3_entry;
static Evas_Object *_elipsis_entry;

/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   int i, r, g, b, a;
   double f;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, probably a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   printf("Apply entry [%s]\n", txt);

   if (!txt || !cur.part || !cur.state) return;
   
   /* Apply Text */
   if (o == _text_entry)
   {
      if (ecore_str_equal(txt, "unset"))
        edje_edit_state_text_set(ui.edje_o, cur.part, cur.state, "");
      else
        edje_edit_state_text_set(ui.edje_o, cur.part, cur.state, txt);
   }
   /* Apply Font */
   else if (o  == _name_entry)
      edje_edit_state_font_set(ui.edje_o, cur.part, cur.state, txt);
   /* Apply Font Size */
   else if (o == _size_entry)
   {
      if (sscanf(txt, "%d", &i) != 1)
        dialog_alert_show(MSG_INT);
      else
        edje_edit_state_text_size_set(ui.edje_o, cur.part, cur.state, i);
   }
   /* Apply AlignX */
   else if (o == _alignx_entry)
   {
      if (sscanf(txt, "%lf", &f) != 1)
        dialog_alert_show(MSG_FLOAT);
      else
        edje_edit_state_text_align_x_set(ui.edje_o, cur.part, cur.state, f);
   }
   /* Apply AlignY */
   else if (o == _aligny_entry)
   {
      if (sscanf(txt, "%lf", &f) != 1)
        dialog_alert_show(MSG_FLOAT);
      else
        edje_edit_state_text_align_y_set(ui.edje_o, cur.part, cur.state, f);
   }
   /* Apply Colors*/
   else if (o == _color_entry)
   {
      if (sscanf(txt, "%d %d %d %d", &r, &g, &b, &a) != 4)
        dialog_alert_show(MSG_COLOR);
      else
        edje_edit_state_color_set(ui.edje_o, cur.part, cur.state, r, g, b, a);
   }
   else if (o == _color2_entry)
   {
      if (sscanf(txt, "%d %d %d %d", &r, &g, &b, &a) != 4)
        dialog_alert_show(MSG_COLOR);
      else
        edje_edit_state_color2_set(ui.edje_o, cur.part, cur.state, r, g, b, a);
   }
   else if (o == _color3_entry)
   {
      if (sscanf(txt, "%d %d %d %d", &r, &g, &b, &a) != 4)
        dialog_alert_show(MSG_COLOR);
      else
        edje_edit_state_color3_set(ui.edje_o, cur.part, cur.state, r, g, b, a);
   }
   /* Apply Elipsis*/
   else if (o == _elipsis_entry)
   {
      if (sscanf(txt, "%lf", &f) != 1)
        dialog_alert_show(MSG_FLOAT);
      else
        edje_edit_state_text_elipsis_set(ui.edje_o, cur.part, cur.state, f);
   }
   canvas_redraw();
   text_frame_update();
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


   NEW_ENTRY_TO_TABLE("elipsis:", 0, 5, 2, _elipsis_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("text:", 0, 6, 2, _text_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("text color:", 0, 7, 2, _color_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("shadow color:", 0, 8, 2, _color2_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("outline color:", 0, 9, 2, _color3_entry, EINA_TRUE)

   return tb;
}

void
program_frame_update(void)
{
   const char *s;

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
  

   //Set text
   /*t = edje_edit_state_text_get(ui.edje_o, cur.part, cur.state);
   if (t && strlen(t) > 0) //TODO t[0] != "/0" ???
   {
      elm_entry_entry_set(_text_entry, t);
      edje_edit_string_free(t);
   }
   else elm_entry_entry_set(_text_entry, "unset");

   //Set align
   elm_entry_printf(_alignx_entry, "%.2f",
            edje_edit_state_text_align_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_aligny_entry, "%.2f",
            edje_edit_state_text_align_y_get(ui.edje_o, cur.part, cur.state));*/

   //Set source
   /*fx = edje_edit_state_text_fit_x_get(ui.edje_o, cur.part, cur.state);
   fy = edje_edit_state_text_fit_y_get(ui.edje_o, cur.part, cur.state);

   if (fx && fy) elm_hoversel_label_set(_source_combo, "both");
   else if (fx)  elm_hoversel_label_set(_source_combo, "horizontal");
   else if (fy)  elm_hoversel_label_set(_source_combo, "vertical");
   else          elm_hoversel_label_set(_source_combo, "none");

   //Set Elipsis
   elm_entry_printf(_elipsis_entry, "%.3f",
               edje_edit_state_text_elipsis_get(ui.edje_o, cur.part, cur.state));*/

   //Set Effect
   /*eff = edje_edit_part_effect_get(ui.edje_o, cur.part);
   if (eff == EDJE_TEXT_EFFECT_OUTLINE)
      elm_hoversel_label_set(_effect_combo, "outline");
   else if (eff == EDJE_TEXT_EFFECT_SOFT_OUTLINE)
      elm_hoversel_label_set(_effect_combo, "outline (soft)");
   else if (eff == EDJE_TEXT_EFFECT_SHADOW)
      elm_hoversel_label_set(_effect_combo, "shadow");
   else if (eff == EDJE_TEXT_EFFECT_SOFT_SHADOW)
      elm_hoversel_label_set(_effect_combo, "shadow (soft)");
   else if (eff == EDJE_TEXT_EFFECT_OUTLINE_SHADOW)
      elm_hoversel_label_set(_effect_combo, "outline + shadow");
   else if (eff == EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW)
      elm_hoversel_label_set(_effect_combo, "outline + shadow (soft)");
   else if (eff == EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW)
      elm_hoversel_label_set(_effect_combo, "outline + shadow (soft)");
   else if (eff == EDJE_TEXT_EFFECT_FAR_SHADOW)
      elm_hoversel_label_set(_effect_combo, "far shadow");
   else if (eff == EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW)
      elm_hoversel_label_set(_effect_combo, "far shadow (soft)");
   else if (eff == EDJE_TEXT_EFFECT_GLOW)
      elm_hoversel_label_set(_effect_combo, "glow");
   else elm_hoversel_label_set(_effect_combo, "plain");*/

   //Set Colors
   /*edje_edit_state_color_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color_entry, "%d %d %d %d", r, g, b, a);

   edje_edit_state_color2_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color2_entry, "%d %d %d %d", r, g, b, a);

   edje_edit_state_color3_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color3_entry, "%d %d %d %d", r, g, b, a);*/
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

