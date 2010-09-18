#include "evas_common.h"
#include "evas_private.h"

static void
_evas_event_havemap_adjust(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Object *pmap;
   pmap = obj->smart.parent;

   while (pmap)
     {
        if ((pmap->cur.map) && (pmap->cur.map->count == 4) && (pmap->cur.usemap))
          break;
        pmap = pmap->smart.parent;
     }
   if (!pmap) return;

   evas_map_coords_get(pmap->cur.map, *x, *y, x, y, obj->mouse_grabbed);
   if (pmap->cur.map)
     {
        *x += pmap->cur.map->normal_geometry.x;
        *y += pmap->cur.map->normal_geometry.y;
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

/**
 * @addtogroup Evas_Event_Freezing_Group
 * @{
 */

/**
 * Freeze all event processing.
 * @param e The canvas to freeze event processing on.
 *
 * This function will indicate to evas that the canvas @p e is to have
 * all event processing frozen until a matching evas_event_thaw()
 * function is called on the same canvas. Every freeze call must be
 * matched by a thaw call in order to completely thaw out a canvas.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern Evas_Object *object;
 *
 * evas_event_freeze(evas);
 * evas_object_move(object, 50, 100);
 * evas_object_resize(object, 200, 200);
 * evas_event_thaw(evas);
 * @endcode
 */
EAPI void
evas_event_freeze(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->events_frozen++;
}

/**
 * Thaw a canvas out after freezing.
 *
 * @param e The canvas to thaw out.
 *
 * This will thaw out a canvas after a matching evas_event_freeze()
 * call. If this call completely thaws out a canvas, events will start
 * being processed again after this call, but this call will not
 * invole any "missed" events to be evaluated.
 *
 * See evas_event_freeze() for an example.
 */
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

/**
 * @}
 */

/**
 * @addtogroup Evas_Event_Feeding_Group
 * @{
 */

/**
 * Return the freeze count of a given canvas.
 * @param e The canvas to fetch the freeze count from.
 *
 * This returns the number of times the canvas has been told to freeze
 * events.  It is possible to call evas_event_freeze() multiple times,
 * and these must be matched by evas_event_thaw() calls. This call
 * allows the program to discover just how many times things have been
 * frozen in case it may want to break out of a deep freeze state
 * where the count is high.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * while (evas_event_freeze_get(evas) > 0) evas_event_thaw(evas);
 * @endcode
 *
 */
EAPI int
evas_event_freeze_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return e->events_frozen;
}


/**
 * Mouse down event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags The evas button flags.
 * @param timestamp The timestamp of the mouse down event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is pressed. It prepares information to be treated
 * by the callback function.
 *
 */
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
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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

/**
 * Mouse up event feed.
 *
 * @param e The given canvas pointer.
 * @param b The button number.
 * @param flags evas button flags.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse button is released. It prepares information to be treated
 * by the callback function.
 *
 */
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
             _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
                  _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
                  _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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


/**
 * Mouse cancel event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will call evas_event_feed_mouse_up() when a
 * mouse cancel event happens.
 *
 */
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

/**
 * Mouse wheel event feed.
 *
 * @param e The given canvas pointer.
 * @param direction The wheel mouse direction.
 * @param z How much mouse wheel was scrolled up or down.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse wheel is scrolled up or down. It prepares information to
 * be treated by the callback function.
 *
 */
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
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
	if (e->events_frozen <= 0)
	  evas_object_event_callback_call(obj, EVAS_CALLBACK_MOUSE_WHEEL, &ev);
	if (e->delete_me) break;
     }
   if (copy) copy = eina_list_free(copy);
   _evas_post_event_callback_call(e);

   _evas_unwalk(e);
}

/**
 * Mouse move event feed.
 *
 * @param e The given canvas pointer.
 * @param x The horizontal position of the mouse pointer.
 * @param y The vertical position of the mouse pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse is moved from its last position. It prepares information
 * to be treated by the callback function.
 *
 */
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
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y);
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

	     if (copy) copy = eina_list_free(copy);
	     while (outs)
	       {
		  Evas_Object *obj;

		  obj = outs->data;
		  outs = eina_list_remove(outs, obj);
		  if ((obj->mouse_grabbed == 0) && (!e->delete_me))
		    {
                       ev.canvas.x = e->pointer.x;
                       ev.canvas.y = e->pointer.y;
                       _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
                       _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y);
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
                       _evas_event_havemap_adjust(obj, &ev2.canvas.x, &ev2.canvas.y);
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
             _evas_event_havemap_adjust(obj, &ev3.canvas.x, &ev3.canvas.y);
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

/**
 * Mouse in event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse in event happens. It prepares information to be treated
 * by the callback function.
 *
 */
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
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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

/**
 * Mouse out event feed.
 *
 * @param e The given canvas pointer.
 * @param timestamp Timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * the mouse out event happens. It prepares information to be treated
 * by the callback function.
 *
 */
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
             _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
        _evas_event_havemap_adjust(obj, &ev.canvas.x, &ev.canvas.y);
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
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y);
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
                  _evas_event_havemap_adjust(obj, &ev.cur.canvas.x, &ev.cur.canvas.y);
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

/**
 * Key down event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key pressed.
 * @param string A String
 * @param compose The compose string
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is pressed. It prepares information to be treated by the
 * callback function.
 *
 */
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

/**
 * Key up event feed
 *
 * @param e The canvas to thaw out
 * @param keyname  Name of the key
 * @param key The key released.
 * @param string string
 * @param compose compose
 * @param timestamp Timestamp of the mouse up event
 * @param data Data for canvas.
 *
 * This function will set some evas properties that is necessary when
 * a key is released. It prepares information to be treated by the
 * callback function.
 *
 */
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

/**
 * Hold event feed
 *
 * @param e The given canvas pointer.
 * @param hold The hold.
 * @param timestamp The timestamp of the mouse up event.
 * @param data The data for canvas.
 *
 * This function makes the object to stop sending events.
 *
 */
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

/**
 * @}
 */

/**
 * @addtogroup Evas_Object_Group_Events
 * @{
 */

/**
 * Set an object's pass events state.
 * @param obj the evas object
 * @param pass whether to pass events or not
 *
 * If @p pass is true, this will cause events on @p obj to be ignored.
 * They will be triggered on the next lower object (that is not set to
 * pass events) instead.
 *
 * If @p pass is false, events will be processed as normal.
 */
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

/**
 * Determine whether an object is set to pass events.
 * @param obj
 * @return pass events state
 */
EAPI Eina_Bool
evas_object_pass_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pass_events;
}

/**
 * Set an object's repeat events state.
 * @param obj the object
 * @param repeat wheter to repeat events or not
 *
 * If @p repeat is true, this will cause events on @p obj to trigger
 * callbacks, but also to be repeated on the next lower object in the
 * stack.
 *
 * If @p repeat is false, events occurring on @p obj will be processed
 * normally.
 */
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

/**
 * Determine whether an object is set to repeat events.
 * @param obj
 * @return repeat events state
 */
EAPI Eina_Bool
evas_object_repeat_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->repeat_events;
}

/**
 * Set whether events on a smart member object should propagate to its
 * parent.
 *
 * @param obj the smart member object
 * @param prop wheter to propagate events or not
 *
 * This function has no effect if @p obj is not a member of a smart
 * object.
 *
 * If @p prop is true, events occurring on this object will propagate on
 * to the smart object of which @p obj is a member.
 *
 * If @p prop is false, events for which callbacks are set on the member
 * object, @p obj, will not be passed on to the parent smart object.
 *
 * The default value is true.
 */
EAPI void
evas_object_propagate_events_set(Evas_Object *obj, Eina_Bool prop)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->no_propagate = !prop;
}

/**
 * Determine whether an object is set to propagate events.
 * @param obj
 * @return propagate events state
 */
EAPI Eina_Bool
evas_object_propagate_events_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return !(obj->no_propagate);
}

/**
 * @}
 */

/**
 * Set pointer behavior.
 *
 * @param obj
 * @param setting desired behavior.
 *
 * This function has direct effect on event callbacks related to
 * mouse.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_AUTOGRAB, then when mouse
 * is down at this object, events will be restricted to it as source,
 * mouse moves, for example, will be emitted even if outside this
 * object area.
 *
 * If @p setting is EVAS_OBJECT_POINTER_MODE_NOGRAB, then events will
 * be emitted just when inside this object area.
 *
 * The default value is EVAS_OBJECT_POINTER_MODE_AUTOGRAB.
 *
 * @ingroup Evas_Object_Group_Extras
 */
EAPI void
evas_object_pointer_mode_set(Evas_Object *obj, Evas_Object_Pointer_Mode setting)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   obj->pointer_mode = setting;
}

/**
 * Determine how pointer will behave.
 * @param obj
 * @return pointer behavior.
 * @ingroup Evas_Object_Group_Extras
 */
EAPI Evas_Object_Pointer_Mode
evas_object_pointer_mode_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->pointer_mode;
}
