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

static Evas_Object *_text_entry;
static Evas_Object *_font_entry;
static Evas_Object *_size_entry;
static Evas_Object *_alignx_entry;
static Evas_Object *_aligny_entry;
static Evas_Object *_fit_combo;
static Evas_Object *_effect_combo;
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
   else if (o  == _font_entry)
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
_fit_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   switch ((int)(long)data)
   {
      case FIT_X:
         edje_edit_state_text_fit_x_set(ui.edje_o, cur.part, cur.state, 1);
         edje_edit_state_text_fit_y_set(ui.edje_o, cur.part, cur.state, 0);
         break;
      case FIT_Y:
         edje_edit_state_text_fit_x_set(ui.edje_o, cur.part, cur.state, 0);
         edje_edit_state_text_fit_y_set(ui.edje_o, cur.part, cur.state, 1);
         break;
      case FIT_BOTH:
         edje_edit_state_text_fit_x_set(ui.edje_o, cur.part, cur.state, 1);
         edje_edit_state_text_fit_y_set(ui.edje_o, cur.part, cur.state, 1);
         break;
      case FIT_NONE: default:
         edje_edit_state_text_fit_x_set(ui.edje_o, cur.part, cur.state, 0);
         edje_edit_state_text_fit_y_set(ui.edje_o, cur.part, cur.state, 0);
         break;
   }
   canvas_redraw();
   text_frame_update();
}

static void
_effect_combo_sel(void *data, Evas_Object *obj, void *event_info)
{
   if (!cur.part) return;
   edje_edit_part_effect_set(ui.edje_o, cur.part,(int)(long)data);
   canvas_redraw();
   text_frame_update();
}

static void
_fonts_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
   fonts_browser_show(ui.win);
}

/***   Implementation   ***/
Evas_Object*
text_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *bt, *_o;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("font:", 0, 0, 1, _font_entry, EINA_TRUE)

   bt = elm_button_add(parent);
   elm_button_label_set(bt, "browse");
   evas_object_size_hint_align_set(bt, 1.0, 0.5);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_smart_callback_add(bt, "clicked", _fonts_button_clicked, NULL);
   evas_object_show(bt);
   
   NEW_ENTRY_TO_TABLE("size:", 0, 1, 2, _size_entry, EINA_TRUE)
    
   NEW_DOUBLE_ENTRY_TO_TABLE("align:", 0, 2, _alignx_entry, _aligny_entry, EINA_TRUE)
    
   NEW_COMBO_TO_TABLE( _fit_combo, "fit:", 0, 3, 2, NULL, NULL)
   elm_hoversel_item_add(_fit_combo, "none", NULL, ELM_ICON_NONE,
                         _fit_combo_sel, (void*)FIT_NONE);
   elm_hoversel_item_add(_fit_combo, "horizontal", NULL, ELM_ICON_NONE,
                         _fit_combo_sel, (void*)FIT_X);
   elm_hoversel_item_add(_fit_combo, "vertical", NULL, ELM_ICON_NONE,
                         _fit_combo_sel, (void*)FIT_Y);
   elm_hoversel_item_add(_fit_combo, "both", NULL, ELM_ICON_NONE,
                         _fit_combo_sel, (void*)FIT_BOTH);
   
   NEW_COMBO_TO_TABLE( _effect_combo, "effect:", 0, 4, 2, NULL, NULL)
   elm_hoversel_item_add(_effect_combo, "plain", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_PLAIN);
   elm_hoversel_item_add(_effect_combo, "outline", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_OUTLINE);
   elm_hoversel_item_add(_effect_combo, "outline (soft)", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_SOFT_OUTLINE);
   elm_hoversel_item_add(_effect_combo, "shadow", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_SHADOW);
   elm_hoversel_item_add(_effect_combo, "shadow (soft)", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_SOFT_SHADOW);
   elm_hoversel_item_add(_effect_combo, "outline + shadow", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_OUTLINE_SHADOW);
   elm_hoversel_item_add(_effect_combo, "outline + shadow (soft)", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW);
   elm_hoversel_item_add(_effect_combo, "far shadow", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_FAR_SHADOW);
   elm_hoversel_item_add(_effect_combo, "far shadow (soft)", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW);
   elm_hoversel_item_add(_effect_combo, "glow", NULL, ELM_ICON_NONE,
                         _effect_combo_sel, (void*)EDJE_TEXT_EFFECT_GLOW);

   NEW_ENTRY_TO_TABLE("elipsis:", 0, 5, 2, _elipsis_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("text:", 0, 6, 2, _text_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("text color:", 0, 7, 2, _color_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("shadow color:", 0, 8, 2, _color2_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("outline color:", 0, 9, 2, _color3_entry, EINA_TRUE)

   return tb;
}

void
text_frame_update(void)
{
   int eff = 0;
   int r, g, b, a;
   const char *t;
   Eina_Bool fx, fy;

   if (!cur.part || !cur.state) return;
   
   //Set font
   t = edje_edit_state_font_get(ui.edje_o, cur.part, cur.state);
   elm_entry_entry_set(_font_entry, t);
   edje_edit_string_free(t);

   //Set font size 
   elm_entry_printf(_size_entry, "%d",
               edje_edit_state_text_size_get(ui.edje_o, cur.part, cur.state));

   //Set text
   t = edje_edit_state_text_get(ui.edje_o, cur.part, cur.state);
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
            edje_edit_state_text_align_y_get(ui.edje_o, cur.part, cur.state));

   //Set fit
   fx = edje_edit_state_text_fit_x_get(ui.edje_o, cur.part, cur.state);
   fy = edje_edit_state_text_fit_y_get(ui.edje_o, cur.part, cur.state);

   if (fx && fy) elm_hoversel_label_set(_fit_combo, "both");
   else if (fx)  elm_hoversel_label_set(_fit_combo, "horizontal");
   else if (fy)  elm_hoversel_label_set(_fit_combo, "vertical");
   else          elm_hoversel_label_set(_fit_combo, "none");

   //Set Elipsis
   elm_entry_printf(_elipsis_entry, "%.3f",
               edje_edit_state_text_elipsis_get(ui.edje_o, cur.part, cur.state));

   //Set Effect
   eff = edje_edit_part_effect_get(ui.edje_o, cur.part);
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
   else elm_hoversel_label_set(_effect_combo, "plain");

   //Set Colors
   edje_edit_state_color_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color_entry, "%d %d %d %d", r, g, b, a);

   edje_edit_state_color2_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color2_entry, "%d %d %d %d", r, g, b, a);

   edje_edit_state_color3_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color3_entry, "%d %d %d %d", r, g, b, a);
}
