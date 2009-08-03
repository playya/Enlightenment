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

static Evas_Object *_name_entry;
static Evas_Object *_min_entry;
static Evas_Object *_max_entry;
static Evas_Object *_current_entry;


static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   int w, h;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, maybe a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   //~ printf("Apply entry [%s]\n", txt);

   // Apply Group Name
   if (o == _name_entry)
   {
      if (!txt || !cur.group || ecore_str_equal(txt, cur.group))
        return;

      if (edje_edit_group_name_set(ui.edje_o, txt))
      {
         edje_object_part_text_set(EV_fakewin, "title", txt);
         set_current_group(txt);
      }
      else dialog_alert_show("Can't rename group.<br>Another group with this name exist?");
   }
   // Apply Group Min & Max
   else if (o == _min_entry || o == _max_entry)
   {
      if (sscanf(txt,"%dx%d", &w, &h) != 2)
      {
         dialog_alert_show("<b>Can't understand sizes.</b><br>The format need to be:<br> (for ex.) '100x120'");
         return;
      }
      if (o == _min_entry )
      {
         edje_edit_group_min_w_set(ui.edje_o, w);
         edje_edit_group_min_h_set(ui.edje_o, h);
      }
      else if (o == _max_entry )
      {
         edje_edit_group_max_w_set(ui.edje_o, w);
         edje_edit_group_max_h_set(ui.edje_o, h);
      }
   }
   group_frame_update();
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
      group_frame_update();
   }
}

/***   Implementation   ***/
Evas_Object*
group_frame_create(Evas_Object *parent)
{

   Evas_Object *tb, *_o;
   
   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("name:", 0, 0, _name_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("min:", 0, 1, _min_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("max:", 0, 2, _max_entry, EINA_TRUE)
   NEW_ENTRY_TO_TABLE("current:", 0, 3, _current_entry, EINA_FALSE)

   return tb;
}

void
group_frame_update(void)
{
   //~ printf("GROUP FRAME UPDATE\n");

   if (!cur.group) return;

   //Update name
   elm_entry_entry_set(_name_entry, cur.group);

   //Update min
   elm_entry_printf(_min_entry, "%dx%d",
                              edje_edit_group_min_w_get(ui.edje_o),
                              edje_edit_group_min_h_get(ui.edje_o));

   //Update max
   elm_entry_printf(_max_entry, "%dx%d",
                              edje_edit_group_max_w_get(ui.edje_o),
                              edje_edit_group_max_h_get(ui.edje_o));
}

void
group_size_update(int w, int h)
{
   //~ printf("GROUP SIZE UPDATE %d %d\n", w, h); //TODO this is called too often (from canvas.c)
   elm_entry_printf(_current_entry, "%dx%d", w, h);
}
