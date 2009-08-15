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

static Evas_Object *_color_entry;


/***   Callbacks   ***/
static void
_entry_apply(Evas_Object *o)
{
   char *txt;
   int r, g, b, a;

   /* TODO FIX THIS IN ELM */
   /* I get a <br> at the end of the line */
   /* Need to fix elm for this, maybe a single_line entry must take care of this*/
   const char *to_fix;
   to_fix = elm_entry_entry_get(o);
   txt = strdup(to_fix);
   if (ecore_str_has_suffix(txt, "<br>"))
      txt[strlen(txt) - 4] = '\0';
   //~ printf("Apply entry [%s]\n", txt);

   if (!txt || !cur.group || !cur.part || !cur.state)
     return;

   // apply color
   if (o == _color_entry)
   {
      if (sscanf(txt,"%d %d %d %d", &r, &g, &b, &a) != 4)
      {
         dialog_alert_show(MSG_COLOR);
         return;
      }
      edje_edit_state_color_set(ui.edje_o, cur.part, cur.state, r, g, b, a);
   }

   rectangle_frame_update();
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
rectangle_frame_create(Evas_Object *parent)
{
   Evas_Object *tb, *_o;

   tb = elm_table_add(parent);
   evas_object_show(tb);

   NEW_ENTRY_TO_TABLE("color:", 0, 0, 1, _color_entry, EINA_TRUE)

   return tb;
}

void
rectangle_frame_update(void)
{
   int r, g, b, a;
   if (!cur.part || !cur.state) return;
   
   edje_edit_state_color_get(ui.edje_o, cur.part, cur.state, &r, &g, &b, &a);
   elm_entry_printf(_color_entry, "%d %d %d %d", r, g, b, a);
}
