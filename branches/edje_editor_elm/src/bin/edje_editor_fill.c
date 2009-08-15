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

static Evas_Object *_origin_relx_entry;
static Evas_Object *_origin_rely_entry;
static Evas_Object *_origin_offx_entry;
static Evas_Object *_origin_offy_entry;
static Evas_Object *_size_relx_entry;
static Evas_Object *_size_rely_entry;
static Evas_Object *_size_offx_entry;
static Evas_Object *_size_offy_entry;

/***  Callbacks  ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   double f;
   int i;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, probably a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   printf("Apply entry [%s]\n", txt);

   // apply origin
   if (o == _origin_relx_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
         edje_edit_state_fill_origin_relative_x_set(ui.edje_o, cur.part, cur.state, f);
      else
         dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _origin_rely_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
         edje_edit_state_fill_origin_relative_y_set(ui.edje_o, cur.part, cur.state, f);
      else
         dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _origin_offx_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
        edje_edit_state_fill_origin_offset_x_set(ui.edje_o, cur.part, cur.state, i);
      else
        dialog_alert_show(MSG_INT);
   }
   else if (o == _origin_offy_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
        edje_edit_state_fill_origin_offset_y_set(ui.edje_o, cur.part, cur.state, i);
      else
        dialog_alert_show(MSG_INT);
   }
   
   // apply size
   else if (o == _size_relx_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
         edje_edit_state_fill_size_relative_x_set(ui.edje_o, cur.part, cur.state, f);
      else
         dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _size_rely_entry)
   {
      if (sscanf(txt,"%lf", &f) == 1)
         edje_edit_state_fill_size_relative_y_set(ui.edje_o, cur.part, cur.state, f);
      else
         dialog_alert_show(MSG_FLOAT);
   }
   else if (o == _size_offx_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
        edje_edit_state_fill_size_offset_x_set(ui.edje_o, cur.part, cur.state, i);
      else
        dialog_alert_show(MSG_INT);
   }
   else if (o == _size_offy_entry)
   {
      if (sscanf(txt,"%d", &i) == 1)
        edje_edit_state_fill_size_offset_y_set(ui.edje_o, cur.part, cur.state, i);
      else
        dialog_alert_show(MSG_INT);
   }

   fill_frame_update();
   canvas_redraw();
}

static void
_entry_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   
   //~ printf("KEY DOWN %s\n", ev->key);
   if (ecore_str_equal(ev->key, "Return"))
      _entry_apply(obj);
   else if(ecore_str_equal(ev->key, "Escape"))
      fill_frame_update();
}

/***  Implementation  ***/
Evas_Object*
fill_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *_o;
   
   tb = elm_table_add(parent);

   NEW_TITLE_TO_TABLE("origin", 0, 0, 3)
   NEW_DOUBLE_ENTRY_TO_TABLE("relative:", 0, 1, _origin_relx_entry, _origin_rely_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("offset:", 0, 2, _origin_offx_entry, _origin_offy_entry, EINA_TRUE)

   NEW_TITLE_TO_TABLE("size", 0, 3, 3)
   NEW_DOUBLE_ENTRY_TO_TABLE("relative:", 0, 4, _size_relx_entry, _size_rely_entry, EINA_TRUE)
   NEW_DOUBLE_ENTRY_TO_TABLE("offset:", 0, 5, _size_offx_entry, _size_offy_entry, EINA_TRUE)
   
   return tb;
}

void
fill_frame_update(void)
{
   if (!cur.part || !cur.state) return;

   //Update origin
   elm_entry_printf(_origin_relx_entry, "%.2f",
      edje_edit_state_fill_origin_relative_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_origin_rely_entry, "%.2f",
      edje_edit_state_fill_origin_relative_y_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_origin_offx_entry, "%d",
      edje_edit_state_fill_origin_offset_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_origin_offy_entry, "%d",
      edje_edit_state_fill_origin_offset_y_get(ui.edje_o, cur.part, cur.state));

   //Update size
   elm_entry_printf(_size_relx_entry, "%.2f",
      edje_edit_state_fill_size_relative_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_size_rely_entry, "%.2f",
      edje_edit_state_fill_size_relative_y_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_size_offx_entry, "%d",
      edje_edit_state_fill_size_offset_x_get(ui.edje_o, cur.part, cur.state));
   elm_entry_printf(_size_offy_entry, "%d",
      edje_edit_state_fill_size_offset_y_get(ui.edje_o, cur.part, cur.state));

}
