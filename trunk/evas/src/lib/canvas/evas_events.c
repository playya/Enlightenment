#include "evas_common.h"
#include "evas_private.h"

static void
_evas_event_havemap_adjust(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Eina_Bool mouse_grabbed)
{
   if (obj->smart.parent)
      _evas_event_havemap_adjust(obj->smart.parent, x, y, mouse_grabbed);

   if ((!obj->cur.map) || (!obj->cur.map->count == 4) || (!obj->cur.usemap))
      return;

   if (obj->cur.map)
     {
        evas_map_coords_get(obj->cur.map, *x, *y, x, y, mouse_grabbed);
        *x += obj->cur.geometry.x;
        *y += obj->cur.geometry.y;
     }
}

static Eina_List *
_evas_event_object_list_in_get(Evas *e, Eina_List *in,
                               const Eina_Inlist *list, Evas_Object *stop,
                               int x, int y, int *no_rep)
{
   Evas_Object *obj;
   if (!list) return in;
   EINA_INLIST_REVERSE_FOREACH(list, obj)
     {
	if (obj == stop)
	  {
	     *no_rep = 1;
	     return in;
	  }
	if (evas_event_passes_through(obj)) continue;
        if ((obj->cur.visible) && (obj->delete_me == 0) &&
            (!obj->clip.clipees) &&
            (evas_object_clippers_is_visible(obj)))
          {
             if (obj->smart.smart)
               {
                  int norep;
                  int inside;

                  norep = 0;
                  if (((obj->cur.map) && (obj->cur.map->count == 4) && (obj->cur.usemap)))
                    {
                       inside = evas_object_is_in_output_rect(obj, x, y, 1, 1);
                       if (inside)
                         {
                            if (!evas_map_coords_get(obj->cur.map, x, y,
                                                     &(obj->cur.map->mx),
                                                     &(obj->cur.map->my), 0))
                              {
                                 inside = 0;
                              }
                            else
                              {
                                 in = _evas_event_object_list_in_get
                                    (e, in,
                                     evas_object_smart_members_get_direct(obj),
                                     stop,
                                     obj->cur.geometry.x + obj->cur.map->mx,
                                     obj->cur.geometry.y + obj->cur.map->my, &norep);
                              }
                         }
                    }
                  else
                    {
                       in = _evas_event_object_list_in_get
                          (e, in, evas_object_smart_members_get_direct(obj),
                           stop, x, y, &norep);
                    }
                  if (norep)
                    {
                       *no_rep = 1;
                       return in;
                    }
               }
             else
               {
                  int inside = 1;
                  
                  if (((obj->cur.map) && (obj->cur.map->count == 4) && (obj->cur.usemap)))
                    {
                       inside = evas_object_is_in_output_rect(obj, x, y, 1, 1);
                       if ((inside) && (!evas_map_coords_get(obj->cur.map, x, y,
                                                             &(obj->cur.map->mx),
                                                             &(obj->cur.map->my), 0)))
                         {
                            inside = 0;
                         }
                    }
                  else
                    {
                       inside = evas_object_is_in_output_rect(obj, x, y, 1, 1);
                    }

                  if (inside && ((!obj->precise_is_inside) ||
                                 (evas_object_is_inside(obj, x, y))))
                    {
                       in = eina_list_append(in, obj);
                       if (!obj->repeat_events)
                         {
                            *no_rep = 1;
                            return in;
                         }
                    }
               }
	  }
     }
   *no_rep = 0;
   return in;
}

Eina_List *
evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y)
{
   Evas_Layer *lay;
   Eina_List *in = NULL;

   if (!e->layers) return NULL;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
	int norep;

	norep = 0;
	in = _evas_event_object_list_in_get(e, in, 
                                            EINA_INLIST_GET(lay->objects), 
                                            stop, x, y, &norep);
	if (norep) return in;
     }
   return in;
}

static Eina_List *evas_event_list_copy(Eina_List *list);
static Eina_List *
evas_event_list_copy(Eina_List *list)
{
   Eina_List *l, *new_l = NULL;
   const void *data;

   EINA_LIST_FOREACH(list, l, data)
     new_l = eina_list_append(new_l, data);
   return new_l;
}
/* public functions */

EAPI void
evas_event_freeze(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen++;
}

EAPI void
evas_event_thaw(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen--;
   if (e->events_frozen == 0)
     {
	Evas_Layer *lay;

	EINA_INLIST_FOREACH((EINA_INLIST_GET(e->layers)), lay)
	  {
	     Evas_Object *obj;

	     EINA_INLIST_FOREACH(lay->objects, obj)
	       {
		  evas_object_clip_recalc(obj);
		  evas_object_recalc_clippees(obj);
	       }
	  }
     }
   if (e->events_frozen < 0)
     evas_debug_generic("  Thaw of events when already thawed!!!\n");
}

EAPI int
evas_event_freeze_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return e->events_frozen;
}


EAPI void
evas_event_feed_mouse_down(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;
   Eina_List *ins;
   Evas_Event_Mouse_Down ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((b < 1) || (b > 32)) return;

   e->pointer.button |= (1 << (b - 1));

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.button = b;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
   /* free our old list of ins */
   e->pointer.object.in = eina_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, obj)
     {
        if (obj->delete_me) continue;
        
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
	if (obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB)
	  {
	     obj->mouse_grabbed++;
	     e->pointer.mouse_grabbed++;
	  }

	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_DOWN, &ev);
	if (e->delete_me) break;
     }
   if (copy) eina_list_free(copy);
   e->last_mouse_down_counter++;
   _evas_post_event_callback_call(e);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_up(Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((b < 1) || (b > 32)) return;

   e->pointer.button &= ~(1 << (b - 1));

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

     {
	Evas_Event_Mouse_Up ev;
	Evas_Object *obj;

	_evas_object_event_new();

	ev.button = b;
	ev.output.x = e->pointer.x;
	ev.output.y = e->pointer.y;
	ev.canvas.x = e->pointer.x;
	ev.canvas.y = e->pointer.y;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.flags = flags;
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;

	_evas_walk(e);
	copy = evas_event_list_copy(e->pointer.object.in);
	EINA_LIST_FOREACH(copy, l, obj)
	  {
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
	     if ((obj->pointer_mode != EVAS_OBJECT_POINTER_MODE_NOGRAB) &&
		 (obj->mouse_in) && (obj->mouse_grabbed > 0))
	       {
		  obj->mouse_grabbed--;
		  e->pointer.mouse_grabbed--;
	       }
             if (!obj->delete_me)
               {
                  if (e->events_frozen <= 0)
                     evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_UP, &ev);
               }
	     if (e->delete_me) break;
	  }
	if (copy) copy = eina_list_free(copy);
	e->last_mouse_up_counter++;
        _evas_post_event_callback_call(e);
     }

   if (!e->pointer.button)
     {
	Eina_List *ins;
	Eina_List *l;
	  {
	     Evas_Event_Mouse_Out ev;
	     Evas_Object *obj;

	     _evas_object_event_new();

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

	     /* get new list of ins */
	     ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
	     /* go thru old list of in objects */
	     copy = evas_event_list_copy(e->pointer.object.in);
	     EINA_LIST_FOREACH(copy, l, obj)
	       {
                  ev.canvas.x = e->pointer.x;
                  ev.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
		  if ((!eina_list_data_find(ins, obj)) ||
		      (!e->pointer.inside))
		    {
                       if (obj->mouse_in)
                         {
                            obj->mouse_in = 0;
                            if (e->events_frozen <= 0)
                               evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                         }
		    }
		  if (e->delete_me) break;
	       }
             _evas_post_event_callback_call(e);
	  }
	if (copy) copy = eina_list_free(copy);
	if (e->pointer.inside)
	  {
	     Evas_Event_Mouse_In ev;
	     Evas_Object *obj;

	     _evas_object_event_new();

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

	     EINA_LIST_FOREACH(ins, l, obj)
	       {
                  ev.canvas.x = e->pointer.x;
                  ev.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
		  if (!eina_list_data_find(e->pointer.object.in, obj))
		    {
                       if (!obj->mouse_in)
                         {
                            obj->mouse_in = 1;
                            if (e->events_frozen <= 0)
                               evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
                         }
		    }
		  if (e->delete_me) break;
	       }
             _evas_post_event_callback_call(e);
	  }
	else
	  {
	     ins = eina_list_free(ins);
	  }
	/* free our old list of ins */
	e->pointer.object.in = eina_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
	if (e->pointer.inside)
	  evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, timestamp, data);
     }

   if (e->pointer.mouse_grabbed < 0)
     {
        ERR("BUG? e->pointer.mouse_grabbed (=%d) < 0!",
	      e->pointer.mouse_grabbed);
     }

   if ((e->pointer.button == 0) && (e->pointer.mouse_grabbed != 0))
     {
        INF("restore to 0 grabs (from %i)", e->pointer.mouse_grabbed);
	e->pointer.mouse_grabbed = 0;
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_cancel(Evas *e, unsigned int timestamp, const void *data)
{
   int i;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;

   _evas_walk(e);
   for (i = 0; i < 32; i++)
     {
	if ((e->pointer.button & (1 << i)))
	  evas_event_feed_mouse_up(e, i + 1, 0, timestamp, data);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_wheel(Evas *e, int direction, int z, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Mouse_Wheel ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.direction = direction;
   ev.z = z;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *) data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);

   EINA_LIST_FOREACH(copy, l, obj)
     {
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_WHEEL, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = eina_list_free(copy);
   _evas_post_event_callback_call(e);

   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_move(Evas *e, int x, int y, unsigned int timestamp, const void *data)
{
   int px, py;
////   Evas_Coord pcx, pcy;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   px = e->pointer.x;
   py = e->pointer.y;
////   pcx = e->pointer.canvas_x;
////   pcy = e->pointer.canvas_y;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   e->pointer.x = x;
   e->pointer.y = y;
////   e->pointer.canvas_x = x;
////   e->pointer.canvas_y = y;
////   e->pointer.canvas_x = evas_coord_screen_x_to_world(e, x);
////   e->pointer.canvas_y = evas_coord_screen_y_to_world(e, y);
   if ((!e->pointer.inside) && (e->pointer.mouse_grabbed == 0)) return;
   _evas_walk(e);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
	/* go thru old list of in objects */
	Eina_List *outs = NULL;
	Eina_List *l, *copy;

	  {
	     Evas_Event_Mouse_Move ev;
	     Evas_Object *obj;

	     _evas_object_event_new();

	     ev.buttons = e->pointer.button;
	     ev.cur.output.x = e->pointer.x;
	     ev.cur.output.y = e->pointer.y;
	     ev.cur.canvas.x = e->pointer.x;
	     ev.cur.canvas.y = e->pointer.y;
	     ev.prev.output.x = px;
	     ev.prev.output.y = py;
	     ev.prev.canvas.x = px;
	     ev.prev.canvas.y = py;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;
	     copy = evas_event_list_copy(e->pointer.object.in);
	     EINA_LIST_FOREACH(copy, l, obj)
	       {
                  ev.cur.canvas.x = e->pointer.x;
                  ev.cur.canvas.y = e->pointer.y;
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
		  if ((obj->cur.visible) &&
		      (evas_object_clippers_is_visible(obj)) &&
		      (!evas_event_passes_through(obj)) &&
		      (!obj->clip.clipees))
		    {
		       if ((px != x) || (py != y))
			 {
			    if (e->events_frozen <= 0)
			      evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
			 }
		    }
		  else
		    outs = eina_list_append(outs, obj);
		  if (e->delete_me) break;
	       }
             _evas_post_event_callback_call(e);
	  }
	  {
	     Evas_Event_Mouse_Out ev;

	     _evas_object_event_new();

	     ev.buttons = e->pointer.button;
	     ev.output.x = e->pointer.x;
	     ev.output.y = e->pointer.y;
	     ev.canvas.x = e->pointer.x;
	     ev.canvas.y = e->pointer.y;
	     ev.data = (void *)data;
	     ev.modifiers = &(e->modifiers);
	     ev.locks = &(e->locks);
	     ev.timestamp = timestamp;
	     ev.event_flags = EVAS_EVENT_FLAG_NONE;

	     if (copy) eina_list_free(copy);
	     while (outs)
	       {
		  Evas_Object *obj;

		  obj = outs->data;
		  outs = eina_list_remove(outs, obj);
		  if ((obj->mouse_grabbed == 0) && (!e->delete_me))
		    {
                       ev.canvas.x = e->pointer.x;
                       ev.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
		       e->pointer.object.in = eina_list_remove(e->pointer.object.in, obj);
                       if (obj->mouse_in)
                         {
                            obj->mouse_in = 0;
                            if (!obj->delete_me)
                              {
                                 if (e->events_frozen <= 0)
                                    evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                              }
                         }
		    }
	       }
             _evas_post_event_callback_call(e);
	  }
     }
   else
     {
	Eina_List *ins;
	Eina_List *l, *copy;
	Evas_Event_Mouse_Move ev;
	Evas_Event_Mouse_Out ev2;
	Evas_Event_Mouse_In ev3;
	Evas_Object *obj;

	_evas_object_event_new();

	ev.buttons = e->pointer.button;
	ev.cur.output.x = e->pointer.x;
	ev.cur.output.y = e->pointer.y;
	ev.cur.canvas.x = e->pointer.x;
	ev.cur.canvas.y = e->pointer.y;
	ev.prev.output.x = px;
	ev.prev.output.y = py;
	ev.prev.canvas.x = px;
	ev.prev.canvas.y = py;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;

	ev2.buttons = e->pointer.button;
	ev2.output.x = e->pointer.x;
	ev2.output.y = e->pointer.y;
	ev2.canvas.x = e->pointer.x;
	ev2.canvas.y = e->pointer.y;
	ev2.data = (void *)data;
	ev2.modifiers = &(e->modifiers);
	ev2.locks = &(e->locks);
	ev2.timestamp = timestamp;
	ev2.event_flags = EVAS_EVENT_FLAG_NONE;

	ev3.buttons = e->pointer.button;
	ev3.output.x = e->pointer.x;
	ev3.output.y = e->pointer.y;
	ev3.canvas.x = e->pointer.x;
	ev3.canvas.y = e->pointer.y;
	ev3.data = (void *)data;
	ev3.modifiers = &(e->modifiers);
	ev3.locks = &(e->locks);
	ev3.timestamp = timestamp;
	ev3.event_flags = EVAS_EVENT_FLAG_NONE;

	/* get all new in objects */
	ins = evas_event_objects_event_list(e, NULL, x, y);
	/* go thru old list of in objects */
	copy = evas_event_list_copy(e->pointer.object.in);
	EINA_LIST_FOREACH(copy, l, obj)
	  {
	     /* if its under the pointer and its visible and its in the new */
	     /* in list */
// FIXME: i don't think we need this
//	     evas_object_clip_recalc(obj);
	     if (evas_object_is_in_output_rect(obj, x, y, 1, 1) &&
		 (obj->cur.visible) &&
		 (evas_object_clippers_is_visible(obj)) &&
		 (eina_list_data_find(ins, obj)) &&
		 (!evas_event_passes_through(obj)) &&
		 (!obj->clip.clipees) &&
		 ((!obj->precise_is_inside) ||
		  (evas_object_is_inside(obj, x, y))))
	       {
		  if ((px != x) || (py != y))
		    {
                       ev.cur.canvas.x = e->pointer.x;
                       ev.cur.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_MOVE, &ev);
		    }
	       }
	     /* otherwise it has left the object */
	     else
	       {
                  if (obj->mouse_in)
                    {
                       obj->mouse_in = 0;
                       ev2.canvas.x = e->pointer.x;
                       ev2.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(obj, &ev2.canvas.x, &ev2.canvas.y, obj->mouse_grabbed);
                       if (e->events_frozen <= 0)
                          evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev2);
                    }
	       }
	     if (e->delete_me) break;
	  }
        _evas_post_event_callback_call(e);

	_evas_object_event_new();

	if (copy) copy = eina_list_free(copy);
	/* go thru our current list of ins */
	EINA_LIST_FOREACH(ins, l, obj)
	  {
             ev3.canvas.x = e->pointer.x;
             ev3.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(obj, &ev3.canvas.x, &ev3.canvas.y, obj->mouse_grabbed);
	     /* if its not in the old list of ins send an enter event */
	     if (!eina_list_data_find(e->pointer.object.in, obj))
	       {
                  if (!obj->mouse_in)
                    {
                       obj->mouse_in = 1;
                       if (e->events_frozen <= 0)
                          evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev3);
                    }
	       }
	     if (e->delete_me) break;
	  }
	/* free our old list of ins */
	eina_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
        _evas_post_event_callback_call(e);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_in(Evas *e, unsigned int timestamp, const void *data)
{
   Eina_List *ins;
   Eina_List *l;
   Evas_Event_Mouse_In ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 1;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   if (e->pointer.mouse_grabbed != 0) return;

   _evas_object_event_new();

   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   /* get new list of ins */
   ins = evas_event_objects_event_list(e, NULL, e->pointer.x, e->pointer.y);
   EINA_LIST_FOREACH(ins, l, obj)
     {
        ev.canvas.x = e->pointer.x;
        ev.canvas.y = e->pointer.y;
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
	if (!eina_list_data_find(e->pointer.object.in, obj))
	  {
             if (!obj->mouse_in)
               {
                  obj->mouse_in = 1;
                  if (e->events_frozen <= 0)
                     evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_IN, &ev);
               }
	  }
	if (e->delete_me) break;
     }
   /* free our old list of ins */
   e->pointer.object.in = eina_list_free(e->pointer.object.in);
   /* and set up the new one */
   e->pointer.object.in = ins;
   _evas_post_event_callback_call(e);
   evas_event_feed_mouse_move(e, e->pointer.x, e->pointer.y, timestamp, data);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_mouse_out(Evas *e, unsigned int timestamp, const void *data)
{
   Evas_Event_Mouse_Out ev;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->pointer.inside = 0;

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.buttons = e->pointer.button;
   ev.output.x = e->pointer.x;
   ev.output.y = e->pointer.y;
   ev.canvas.x = e->pointer.x;
   ev.canvas.y = e->pointer.y;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed == 0)
     {
	/* go thru old list of in objects */
	Eina_List *l, *copy;
	Evas_Object *obj;

	copy = evas_event_list_copy(e->pointer.object.in);
	EINA_LIST_FOREACH(copy, l, obj)
	  {
             ev.canvas.x = e->pointer.x;
             ev.canvas.y = e->pointer.y;
             _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
             if (obj->mouse_in)
               {
                  obj->mouse_in = 0;
                  if (!obj->delete_me)
                    {
                       if (e->events_frozen <= 0)
                          evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                    }
               }
	    if (e->delete_me) break;
	  }
	if (copy) copy = eina_list_free(copy);
	/* free our old list of ins */
	e->pointer.object.in =  eina_list_free(e->pointer.object.in);
        _evas_post_event_callback_call(e);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_multi_down(Evas *e, 
                           int d, int x, int y, 
                           double rad, double radx, double rady,
                           double pres, double ang,
                           double fx, double fy,
                           Evas_Button_Flags flags, unsigned int timestamp, 
                           const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Multi_Down ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.device = d;
   ev.output.x = x;
   ev.output.y = y;
   ev.canvas.x = x;
   ev.canvas.y = y;
   ev.radius = rad;
   ev.radius_x = radx;
   ev.radius_y = rady;
   ev.pressure = pres;
   ev.angle = ang;
   ev.canvas.xsub = fx;
   ev.canvas.ysub = fy;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, obj)
     {
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x) 
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MULTI_DOWN, &ev);
	if (e->delete_me) break;
     }
   if (copy) eina_list_free(copy);
   _evas_post_event_callback_call(e);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_multi_up(Evas *e, 
                         int d, int x, int y, 
                         double rad, double radx, double rady,
                         double pres, double ang,
                         double fx, double fy,
                         Evas_Button_Flags flags, unsigned int timestamp, 
                         const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Multi_Up ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.device = d;
   ev.output.x = x;
   ev.output.y = y;
   ev.canvas.x = x;
   ev.canvas.y = y;
   ev.radius = rad;
   ev.radius_x = radx;
   ev.radius_y = rady;
   ev.pressure = pres;
   ev.angle = ang;
   ev.canvas.xsub = fx;
   ev.canvas.ysub = fy;
   ev.data = (void *)data;
   ev.modifiers = &(e->modifiers);
   ev.locks = &(e->locks);
   ev.flags = flags;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;
   
   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, obj)
     {
        ev.canvas.x = x;
        ev.canvas.y = y;
        ev.canvas.xsub = fx;
        ev.canvas.ysub = fy;
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y, obj->mouse_grabbed);
        if (x != ev.canvas.x) 
          ev.canvas.xsub = ev.canvas.x; // fixme - lost precision
        if (y != ev.canvas.y)
          ev.canvas.ysub = ev.canvas.y; // fixme - lost precision
        if (e->events_frozen <= 0)
          evas_object_event_callback_call(obj, EVAS_CALLBACK_MULTI_UP, &ev);
        if (e->delete_me) break;
     }
   if (copy) copy = eina_list_free(copy);
   _evas_post_event_callback_call(e);
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_multi_move(Evas *e, 
                           int d, int x, int y, 
                           double rad, double radx, double rady,
                           double pres, double ang,
                           double fx, double fy,
                           unsigned int timestamp, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   if (!e->pointer.inside) return;
   
   _evas_walk(e);
   /* if our mouse button is grabbed to any objects */
   if (e->pointer.mouse_grabbed > 0)
     {
	/* go thru old list of in objects */
	Eina_List *l, *copy;
        Evas_Event_Multi_Move ev;
        Evas_Object *obj;

	_evas_object_event_new();

        ev.device = d;
        ev.cur.output.x = x;
        ev.cur.output.y = y;
        ev.cur.canvas.x = x;
        ev.cur.canvas.y = y;
        ev.radius = rad;
        ev.radius_x = radx;
        ev.radius_y = rady;
        ev.pressure = pres;
        ev.angle = ang;
        ev.cur.canvas.xsub = fx;
        ev.cur.canvas.ysub = fy;
        ev.data = (void *)data;
        ev.modifiers = &(e->modifiers);
        ev.locks = &(e->locks);
        ev.timestamp = timestamp;
        ev.event_flags = EVAS_EVENT_FLAG_NONE;

        copy = evas_event_list_copy(e->pointer.object.in);
        EINA_LIST_FOREACH(copy, l, obj)
          {
             if ((obj->cur.visible) &&
                 (evas_object_clippers_is_visible(obj)) &&
                 (!evas_event_passes_through(obj)) &&
                 (!obj->clip.clipees))
               {
                  ev.cur.canvas.x = x;
                  ev.cur.canvas.y = y;
                  ev.cur.canvas.xsub = fx;
                  ev.cur.canvas.ysub = fy;
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                  if (x != ev.cur.canvas.x) 
                    ev.cur.canvas.xsub = ev.cur.canvas.x; // fixme - lost precision
                  if (y != ev.cur.canvas.y)
                    ev.cur.canvas.ysub = ev.cur.canvas.y; // fixme - lost precision
                  if (e->events_frozen <= 0)
                    evas_object_event_callback_call(obj, EVAS_CALLBACK_MULTI_MOVE, &ev);
	       }
             if (e->delete_me) break;
	  }
        _evas_post_event_callback_call(e);
     }
   else
     {
	Eina_List *ins;
	Eina_List *l, *copy;
	Evas_Event_Multi_Move ev;
	Evas_Object *obj;

	_evas_object_event_new();

	ev.device = d;
	ev.cur.output.x = x;
	ev.cur.output.y = y;
	ev.cur.canvas.x = x;
	ev.cur.canvas.y = y;
        ev.radius = rad;
        ev.radius_x = radx;
        ev.radius_y = rady;
        ev.pressure = pres;
        ev.angle = ang;
        ev.cur.canvas.xsub = fx;
        ev.cur.canvas.ysub = fy;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;
        
	/* get all new in objects */
	ins = evas_event_objects_event_list(e, NULL, x, y);
	/* go thru old list of in objects */
	copy = evas_event_list_copy(e->pointer.object.in);
	EINA_LIST_FOREACH(copy, l, obj)
	  {
	     /* if its under the pointer and its visible and its in the new */
	     /* in list */
// FIXME: i don't think we need this
//	     evas_object_clip_recalc(obj);
	     if (evas_object_is_in_output_rect(obj, x, y, 1, 1) &&
		 (obj->cur.visible) &&
		 (evas_object_clippers_is_visible(obj)) &&
		 (eina_list_data_find(ins, obj)) &&
		 (!evas_event_passes_through(obj)) &&
		 (!obj->clip.clipees) &&
		 ((!obj->precise_is_inside) ||
		  (evas_object_is_inside(obj, x, y))))
	       {
                  ev.cur.canvas.x = x;
                  ev.cur.canvas.y = y;
                  ev.cur.canvas.xsub = fx;
                  ev.cur.canvas.ysub = fy;
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y, obj->mouse_grabbed);
                  if (x != ev.cur.canvas.x) 
                    ev.cur.canvas.xsub = ev.cur.canvas.x; // fixme - lost precision
                  if (y != ev.cur.canvas.y)
                    ev.cur.canvas.ysub = ev.cur.canvas.y; // fixme - lost precision
                  if (e->events_frozen <= 0)
                    evas_object_event_callback_call(obj, EVAS_CALLBACK_MULTI_MOVE, &ev);
	       }
	     if (e->delete_me) break;
	  }
	if (copy) copy = eina_list_free(copy);
	/* free our old list of ins */
	eina_list_free(e->pointer.object.in);
	/* and set up the new one */
	e->pointer.object.in = ins;
        _evas_post_event_callback_call(e);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_key_down(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);
     {
	Evas_Event_Key_Down ev;
	int exclusive;

	_evas_object_event_new();

	exclusive = 0;
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.key = key;
	ev.string = string;
	ev.compose = compose;
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;
	if (e->grabs)
	  {
	     Eina_List *l;
	     Evas_Key_Grab *g;

	     e->walking_grabs++;
	     EINA_LIST_FOREACH(e->grabs, l, g)
	       {
		  if (g->just_added)
		    {
		       g->just_added = 0;
		       continue;
		    }
		  if (g->delete_me) continue;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (!(e->modifiers.mask & g->not_modifiers))
			 {
			    if (e->events_frozen <= 0)
			      evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_DOWN, &ev);
			    if (g->exclusive) exclusive = 1;
			 }
		    }
		  if (e->delete_me) break;
	       }
	     e->walking_grabs--;
	     if (e->walking_grabs <= 0)
	       {
		  while (e->delete_grabs > 0)
		    {
		       e->delete_grabs--;
		       for (l = e->grabs; l;)
			 {
			    g = eina_list_data_get(l);
			    l = eina_list_next(l);
			    if (g->delete_me)
			      evas_key_grab_free(g->object, g->keyname, g->modifiers, g->not_modifiers);
			 }
		    }
	       }
	  }
	if ((e->focused) && (!exclusive))
	  {
	     if (e->events_frozen <= 0)
               evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_DOWN, &ev);
	  }
        _evas_post_event_callback_call(e);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_key_up(Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;
   _evas_walk(e);
     {
	Evas_Event_Key_Up ev;
	int exclusive;

	_evas_object_event_new();

	exclusive = 0;
	ev.keyname = (char *)keyname;
	ev.data = (void *)data;
	ev.modifiers = &(e->modifiers);
	ev.locks = &(e->locks);
	ev.key = key;
	ev.string = string;
	ev.compose = compose;
	ev.timestamp = timestamp;
	ev.event_flags = EVAS_EVENT_FLAG_NONE;
	if (e->grabs)
	  {
	     Eina_List *l;
	     Evas_Key_Grab *g;

	     e->walking_grabs++;
	     EINA_LIST_FOREACH(e->grabs, l, g)
	       {
		  if (g->just_added)
		    {
		       g->just_added = 0;
		       continue;
		    }
		  if (g->delete_me) continue;
		  if (((e->modifiers.mask & g->modifiers) ||
		       (g->modifiers == e->modifiers.mask)) &&
		      (!((e->modifiers.mask & g->not_modifiers) ||
			 (g->not_modifiers == ~e->modifiers.mask))) &&
		      (!strcmp(keyname, g->keyname)))
		    {
		       if (e->events_frozen <= 0)
			 evas_object_event_callback_call(g->object, EVAS_CALLBACK_KEY_UP, &ev);
		       if (g->exclusive) exclusive = 1;
		    }
		  if (e->delete_me) break;
	       }
	     e->walking_grabs--;
	     if (e->walking_grabs <= 0)
	       {
		  while (e->delete_grabs > 0)
		    {
		       Eina_List *l, *l_next;
		       Evas_Key_Grab *g;

		       e->delete_grabs--;
		       EINA_LIST_FOREACH_SAFE(e->grabs, l, l_next, g)
			 {
			    if (g->delete_me)
			      evas_key_grab_free(g->object, g->keyname, g->modifiers, g->not_modifiers);
			 }
		    }
	       }
	  }
	if ((e->focused) && (!exclusive))
	  {
	     if (e->events_frozen <= 0)
               evas_object_event_callback_call(e->focused, EVAS_CALLBACK_KEY_UP, &ev);
	  }
        _evas_post_event_callback_call(e);
     }
   _evas_unwalk(e);
}

EAPI void
evas_event_feed_hold(Evas *e, int hold, unsigned int timestamp, const void *data)
{
   Eina_List *l, *copy;
   Evas_Event_Hold ev;
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->events_frozen > 0) return;
   e->last_timestamp = timestamp;

   _evas_object_event_new();

   ev.hold = hold;
   ev.data = (void *)data;
   ev.timestamp = timestamp;
   ev.event_flags = EVAS_EVENT_FLAG_NONE;

   _evas_walk(e);
   copy = evas_event_list_copy(e->pointer.object.in);
   EINA_LIST_FOREACH(copy, l, obj)
     {
	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_HOLD, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = eina_list_free(copy);
   _evas_post_event_callback_call(e);
   _evas_unwalk(e);
   _evas_object_event_new();
}

EAPI void
evas_object_pass_events_set(Evas_Object *obj, Eina_Bool pass)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   pass = !!pass;
   if (obj->pass_events == pass) return;
   obj->pass_events = pass;
   evas_object_smart_member_cache_invalidate(obj);
   if (evas_object_is_in_output_rect(obj,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y, 1, 1) &&
       ((!obj->precise_is_inside) ||
	(evas_object_is_inside(obj,
                               obj->layer->evas->pointer.x,
                               obj->layer->evas->pointer.y))))
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
}

EAPI Eina_Bool
evas_object_pass_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pass_events;
}

EAPI void
evas_object_repeat_events_set(Evas_Object *obj, Eina_Bool repeat)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   repeat = !!repeat;
   if (obj->repeat_events == repeat) return;
   obj->repeat_events = repeat;
   if (evas_object_is_in_output_rect(obj,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y, 1, 1) &&
       ((!obj->precise_is_inside) ||
	(evas_object_is_inside(obj,
                               obj->layer->evas->pointer.x,
                               obj->layer->evas->pointer.y))))
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
}

EAPI Eina_Bool
evas_object_repeat_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->repeat_events;
}

EAPI void
evas_object_propagate_events_set(Evas_Object *obj, Eina_Bool prop)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->no_propagate = !prop;
}

EAPI Eina_Bool
evas_object_propagate_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return !(obj->no_propagate);
}

EAPI void
evas_object_pointer_mode_set(Evas_Object *obj, Evas_Object_Pointer_Mode setting)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->pointer_mode = setting;
}

EAPI Evas_Object_Pointer_Mode
evas_object_pointer_mode_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pointer_mode;
}
