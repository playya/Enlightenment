/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* E_Zone is a child object of E_Container. There is one zone per screen
 * in a xinerama setup. Each zone has one or more desktops.
 */

static void _e_zone_free(E_Zone *zone);
static void _e_zone_cb_menu_end(void *data, E_Menu *m);
static void _e_zone_cb_bg_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_cb_bg_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_cb_bg_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_event_zone_desk_count_set_free(void *data, void *ev);
static int  _e_zone_cb_mouse_in(void *data, int type, void *event);
static int  _e_zone_cb_mouse_out(void *data, int type, void *event);
static int  _e_zone_cb_timer(void *data);
static void _e_zone_update_flip(E_Zone *zone);

int E_EVENT_ZONE_DESK_COUNT_SET = 0;

#define E_ZONE_FLIP_UP(zone) ((zone)->desk_y_current > 0)
#define E_ZONE_FLIP_RIGHT(zone) (((zone)->desk_x_current + 1) < (zone)->desk_x_count)
#define E_ZONE_FLIP_DOWN(zone) (((zone)->desk_y_current + 1) < (zone)->desk_y_count)
#define E_ZONE_FLIP_LEFT(zone) ((zone)->desk_x_current > 0)

int
e_zone_init(void)
{
   E_EVENT_ZONE_DESK_COUNT_SET = ecore_event_type_new();
   
   return 1;
}

int
e_zone_shutdown(void)
{
   return 1;
}

E_Zone *
e_zone_new(E_Container *con, int num, int x, int y, int w, int h)
{
   E_Zone *zone;
   char    name[40];

   zone = E_OBJECT_ALLOC(E_Zone, E_ZONE_TYPE, _e_zone_free);
   if (!zone) return NULL;

   zone->container = con;

   zone->x = x;
   zone->y = y;
   zone->w = w;
   zone->h = h;
   zone->num = num;

   zone->flip.top = ecore_x_window_input_new(con->win, 1, 0, w - 2, 1);
   zone->flip.right = ecore_x_window_input_new(con->win, w - 1, 1, 1, h - 2);
   zone->flip.bottom = ecore_x_window_input_new(con->win, 1, h - 1, w - 2, 1);
   zone->flip.left = ecore_x_window_input_new(con->win, 0, 1, 1, h - 2);

   zone->handlers = evas_list_append(zone->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN, _e_zone_cb_mouse_in, zone));
   zone->handlers = evas_list_append(zone->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT, _e_zone_cb_mouse_out, zone));

   snprintf(name, sizeof(name), "Zone %d", zone->num);
   zone->name = strdup(name);

   con->zones = evas_list_append(con->zones, zone);
   
   if (1)
     {
	char name[40];
	Evas_Object *o;

	o = evas_object_rectangle_add(con->bg_evas);
	zone->bg_clip_object = o;
	evas_object_move(o, x, y);
	evas_object_resize(o, w, h);
	evas_object_color_set(o, 255, 255, 255, 255);
	evas_object_show(o);
	
	o = edje_object_add(con->bg_evas);
	zone->bg_object = o;
	evas_object_layer_set(o, -1);
	snprintf(name, sizeof(name), "desktop/background/%d", zone->num);
	evas_object_name_set(o, name);
	evas_object_data_set(o, "e_zone", zone);
	evas_object_move(o, x, y);
	evas_object_resize(o, w, h);
	edje_object_file_set(o,
			     e_config->desktop_default_background,
			     "desktop/background");
	evas_object_clip_set(o, zone->bg_clip_object);
	evas_object_show(o);

	o = evas_object_rectangle_add(con->bg_evas);
	zone->bg_event_object = o;
	evas_object_clip_set(o, zone->bg_clip_object);
	evas_object_move(o, x, y);
	evas_object_resize(o, w, h);
	evas_object_color_set(o, 255, 255, 255, 0);
	evas_object_show(o);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_zone_cb_bg_mouse_down, zone);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   _e_zone_cb_bg_mouse_up, zone);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _e_zone_cb_bg_mouse_move, zone);
     }

   zone->desk_x_count = 0;
   zone->desk_y_count = 0;
   zone->desk_x_current = 0;
   zone->desk_y_current = 0;
   e_zone_desk_count_set(zone,
			 e_config->zone_desks_x_count,
			 e_config->zone_desks_y_count);

   _e_zone_update_flip(zone);

   return zone;
}

void
e_zone_name_set(E_Zone *zone, const char *name)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   if (zone->name)
      free(zone->name);
   zone->name = strdup(name);
}

void
e_zone_move(E_Zone *zone, int x, int y)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   
   if ((x == zone->x) && (y == zone->y)) return;
   zone->x = x;
   zone->y = y;
   evas_object_move(zone->bg_object, x, y);
   evas_object_move(zone->bg_event_object, x, y);
   evas_object_move(zone->bg_clip_object, x, y);
}

void
e_zone_resize(E_Zone *zone, int w, int h)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   
   if ((w == zone->w) && (h == zone->h)) return;
   zone->w = w;
   zone->h = h;
   evas_object_resize(zone->bg_object, w, h);
   evas_object_resize(zone->bg_event_object, w, h);
   evas_object_resize(zone->bg_clip_object, w, h);

   ecore_x_window_move_resize(zone->flip.top, 1, 0, w - 2, 1);
   ecore_x_window_move_resize(zone->flip.right, w - 1, 1, 1, h - 2);
   ecore_x_window_move_resize(zone->flip.bottom, 1, h - 1, w - 2, 1);
   ecore_x_window_move_resize(zone->flip.left, 0, 1, 1, h - 2);
}

void
e_zone_move_resize(E_Zone *zone, int x, int y, int w, int h)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   if ((x == zone->x) && (y == zone->y) && (w == zone->w) && (h == zone->h))
     return;

   zone->x = x;
   zone->y = y;
   zone->w = w;
   zone->h = h;
   
   evas_object_move(zone->bg_object, x, y);
   evas_object_move(zone->bg_event_object, x, y);
   evas_object_move(zone->bg_clip_object, x, y);
   evas_object_resize(zone->bg_object, w, h);
   evas_object_resize(zone->bg_event_object, w, h);
   evas_object_resize(zone->bg_clip_object, w, h);
} 

E_Zone *
e_zone_current_get(E_Container *con)
{
   Evas_List *l;
   E_Border *bd;
   
   E_OBJECT_CHECK_RETURN(con, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(con, E_CONTAINER_TYPE, NULL);
   bd = e_border_focused_get();
   if (bd)
     {
	/* the current zone is whatever zone has the focused window */
	return bd->zone;
     }
   else
     {
	int x, y;

	ecore_x_pointer_xy_get(con->win, &x, &y);
	for (l = con->zones; l; l = l->next)
	  {
	     E_Zone *zone;
	     
	     zone = l->data;
	     if (E_INTERSECTS(x, y, 1, 1,
			      zone->x, zone->y, zone->w, zone->h))
	       return zone;
	  }
     }
   if (!con->zones)
     return NULL;
   l = con->zones;
   return (E_Zone *)l->data;
}

void
e_zone_bg_reconfigure(E_Zone *zone)
{
   Evas_Object *o;
   
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   o = zone->bg_object;
   evas_object_hide(o);
   edje_object_file_set(o,
			e_config->desktop_default_background,
			"desktop/background");
   evas_object_layer_set(o, -1);
   evas_object_show(o);
}

void
e_zone_flip_coords_handle(E_Zone *zone, int x, int y)
{
   if ((y == 0) && E_ZONE_FLIP_UP(zone))
     {
	/* top */
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_UP;
     }
   else if ((x == (zone->w - 1)) && E_ZONE_FLIP_RIGHT(zone))
     {
	/* right */
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_RIGHT;
     }
   else if ((y == (zone->h - 1)) && E_ZONE_FLIP_DOWN(zone))
     {
	/* bottom */
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_DOWN;
     }
   else if ((x == 0) && E_ZONE_FLIP_LEFT(zone))
     {
	/* left */
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_LEFT;
     }
   else
     {
	/* in zone */
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = NULL;
     }
}

static void
_e_zone_free(E_Zone *zone)
{
   E_Container *con;
   Evas_List *l;
   int x, y;

   /* remove handlers */
   for (l = zone->handlers; l; l = l->next)
     {
	Ecore_Event_Handler *h;

	h = l->data;
	ecore_event_handler_del(h);
     }
   evas_list_free(zone->handlers);
   zone->handlers = NULL;

   con = zone->container;
   if (zone->name) free(zone->name);
   con->zones = evas_list_remove(con->zones, zone);
   evas_object_del(zone->bg_event_object);
   evas_object_del(zone->bg_clip_object);
   evas_object_del(zone->bg_object);
   /* free desks */
   for (x = 0; x < zone->desk_x_count; x++)
     for(y = 0; y < zone->desk_y_count; y++)
       e_object_del(E_OBJECT(zone->desks[x + (y * zone->desk_x_count)]));
   free(zone->desks);

   free(zone);
}

static void
_e_zone_cb_menu_end(void *data, E_Menu *m)
{
   e_object_del(E_OBJECT(m));
}

static void
_e_zone_cb_bg_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Zone *zone;
   Evas_Event_Mouse_Down *ev;
   
   ev = (Evas_Event_Mouse_Down *)event_info;
   zone = data;
   if (e_menu_grab_window_get()) return;

   if (!zone->cur_mouse_action)
     {
	if (ecore_event_current_type_get() == ECORE_X_EVENT_MOUSE_BUTTON_DOWN)
	  {
	     Ecore_X_Event_Mouse_Button_Down *ev2;
	     
	     ev2 = ecore_event_current_event_get();
	     zone->cur_mouse_action =
	       e_bindings_mouse_down_event_handle(E_BINDING_CONTEXT_ZONE,
						  E_OBJECT(zone), ev2);
	  }
     }
#if 0  /* FIXME: nuke this later once the new configurable bindings settle */
   if (ev->button == 1)
     {
	E_Menu *m;

	m = e_int_menus_main_new();
	e_menu_post_deactivate_callback_set(m, _e_zone_cb_menu_end, NULL);
	e_menu_activate_mouse(m, zone, ev->output.x, ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN);
	e_util_container_fake_mouse_up_all_later(zone->container);
     }
   else if (ev->button == 2)
     {
	E_Menu *m;

	m = e_int_menus_clients_new();
	/* FIXME: this is a bit of a hack... setting m->con - bad hack */
	m->zone = zone;
	e_menu_post_deactivate_callback_set(m, _e_zone_cb_menu_end, NULL);
	e_menu_activate_mouse(m, zone, ev->output.x, ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN);
	e_util_container_fake_mouse_up_all_later(zone->container);
     }
   else if (ev->button == 3)
     {
	E_Menu *m;

	m = e_int_menus_favorite_apps_new();
	e_menu_post_deactivate_callback_set(m, _e_zone_cb_menu_end, NULL);
	e_menu_activate_mouse(m, zone, ev->output.x, ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN);
	e_util_container_fake_mouse_up_all_later(zone->container);
     }
#endif   
}

static void
_e_zone_cb_bg_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Zone *zone;
   Evas_Event_Mouse_Up *ev;
   
   ev = (Evas_Event_Mouse_Up *)event_info;      
   zone = data;
   if (zone->cur_mouse_action)
     {
	if (ecore_event_current_type_get() == ECORE_X_EVENT_MOUSE_BUTTON_UP)
	  {
	     Ecore_X_Event_Mouse_Button_Up *ev2;
	     
	     ev2 = ecore_event_current_event_get();
	     if (zone->cur_mouse_action->func.end_mouse)
	       zone->cur_mouse_action->func.end_mouse(E_OBJECT(zone), "", ev2);
	     else if (zone->cur_mouse_action->func.end)
	       zone->cur_mouse_action->func.end(E_OBJECT(zone), "");
	  }
	zone->cur_mouse_action = NULL;
     }
   else
     {
	if (ecore_event_current_type_get() == ECORE_X_EVENT_MOUSE_BUTTON_UP)
	  {
	     Ecore_X_Event_Mouse_Button_Up *ev2;
	     
	     ev2 = ecore_event_current_event_get();
	     e_bindings_mouse_up_event_handle(E_BINDING_CONTEXT_ZONE,
					      E_OBJECT(zone), ev2);
	  }
     }
}

static void
_e_zone_cb_bg_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Zone *zone;
   Evas_Event_Mouse_Move *ev;
   
   ev = (Evas_Event_Mouse_Move *)event_info;   
   zone = data;
}

void
e_zone_desk_count_set(E_Zone *zone, int x_count, int y_count)
{
   E_Desk   **new_desks;
   E_Desk    *desk, *new_desk;
   int        x, y, xx, yy, moved;
   Evas_List *l;
   E_Border  *bd;
   E_Event_Zone_Desk_Count_Set *ev;
   
   xx = x_count;
   if (xx < 1) xx = 1;
   yy = y_count;
   if (yy < 1) yy = 1;

   new_desks = malloc(xx * yy * sizeof(E_Desk *));
   for (x = 0; x < xx; x++)
     {
	for (y = 0; y < yy; y++)
	  {
	     if ((x < zone->desk_x_count) && (y < zone->desk_y_count))
	       desk = zone->desks[x + (y * zone->desk_x_count)];
	     else
	       desk = e_desk_new(zone, x, y);
	     new_desks[x + (y * xx)] = desk;
	  }
     }

   /* catch windoes that have fallen off the end if we got smaller */
   if (xx < zone->desk_x_count)
     {
	for (y = 0; y < zone->desk_y_count; y++)
	  {
	     new_desk = zone->desks[xx - 1 + (y * zone->desk_x_count)];
	     for (x = xx; x < zone->desk_x_count; x++)
	       {
		  desk = zone->desks[x + (y * zone->desk_x_count)];
		  
		  for (l = zone->container->clients; l; l = l->next)
		    {
		       bd = l->data;
		       if (bd->desk == desk)
			 e_border_desk_set(bd, new_desk);
		    }
		  e_object_del(E_OBJECT(desk));
	       }
	  }
     }
   if (yy < zone->desk_y_count)
     {
	for (x = 0; x < zone->desk_x_count; x++)
	  {
	     new_desk = zone->desks[x + ((yy - 1) * zone->desk_x_count)];
	     for (y = yy; y < zone->desk_y_count; y++)
	       {
		  desk = zone->desks[x + (y * zone->desk_x_count)];
		  
		  for (l = zone->container->clients; l; l = l->next)
		    {
		       bd = l->data;
		       if (bd->desk == desk)
			 e_border_desk_set(bd, new_desk);
		    }
		  e_object_del(E_OBJECT(desk));
	       }
	  }	
     }
   if (zone->desks) free(zone->desks);
   zone->desks = new_desks;
   
   zone->desk_x_count = xx;
   zone->desk_y_count = yy;

   moved = 0;
   if (zone->desk_x_current >= xx)
     {
	zone->desk_x_current = xx - 1;
	moved = 1;
     }
   if (zone->desk_y_current >= yy)
     {
	zone->desk_y_current = yy - 1;
	moved = 1;
     }
   if (moved)
     e_desk_show(e_desk_at_xy_get(zone, xx - 1, yy - 1));
   else
     {
	desk = e_desk_current_get(zone);
	desk->visible = 0;
	e_desk_show(desk);
     }
   e_config->zone_desks_x_count = xx;
   e_config->zone_desks_y_count = yy;
   e_config_save_queue();

   ev = E_NEW(E_Event_Zone_Desk_Count_Set, 1);
   if (!ev) return;
   ev->zone = zone;
   e_object_ref(E_OBJECT(zone));
   ecore_event_add(E_EVENT_ZONE_DESK_COUNT_SET, ev, _e_zone_event_zone_desk_count_set_free, NULL);
}

void
e_zone_desk_count_get(E_Zone *zone, int *x_count, int *y_count)
{
   *x_count = zone->desk_x_count;
   *y_count = zone->desk_y_count;
}

void
e_zone_update_flip(E_Zone *zone)
{
   _e_zone_update_flip(zone);
}

void
e_zone_desk_flip_by(E_Zone *zone, int dx, int dy)
{
   dx = zone->desk_x_current + dx;
   dy = zone->desk_y_current + dy;
   e_zone_desk_flip_to(zone, dx, dy);
}

void
e_zone_desk_flip_to(E_Zone *zone, int x, int y)
{
   E_Desk *desk;
   
   if (x < 0) x = 0;
   else if (x >= zone->desk_x_count) x = zone->desk_x_count - 1;
   if (y < 0) y = 0;
   else if (y >= zone->desk_y_count) y = zone->desk_y_count - 1;
   desk = e_desk_at_xy_get(zone, x, y);
   if (desk)
     {
	e_desk_show(desk);
	_e_zone_update_flip(zone);
     }
}

void
e_zone_desk_linear_flip_by(E_Zone *zone, int dx)
{
   dx = zone->desk_x_current + 
     (zone->desk_y_current * zone->desk_x_count) + dx;
   e_zone_desk_linear_flip_to(zone, dx);
}

void
e_zone_desk_linear_flip_to(E_Zone *zone, int x)
{
   int y;
   
   y = x / zone->desk_x_count;
   x = x - (y * zone->desk_x_count);
   y = y % zone->desk_y_count;
   e_zone_desk_flip_to(zone, x, y);
}

static void
_e_zone_event_zone_desk_count_set_free(void *data, void *ev)
{
   E_Event_Zone_Desk_Count_Set *e;

   e = ev;
   e_object_unref(E_OBJECT(e->zone));
   free(e);
}

static int
_e_zone_cb_mouse_in(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_In *ev;
   E_Zone *zone;

   ev = event;
   zone = data;

   if (ev->win == zone->flip.top)
     {
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_UP;
     }
   else if (ev->win == zone->flip.right)
     {
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_RIGHT;
     }
   else if (ev->win == zone->flip.bottom)
     {
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_DOWN;
     }
   else if (ev->win == zone->flip.left)
     {
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = ecore_timer_add(0.5, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_LEFT;
     }
   return 1;
}

static int
_e_zone_cb_mouse_out(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Out *ev;
   E_Zone *zone;

   ev = event;
   zone = data;

   if (zone->flip.timer)
     ecore_timer_del(zone->flip.timer);
   zone->flip.timer = NULL;

   return 1;
}

static int
_e_zone_cb_timer(void *data)
{
   E_Zone *zone;
   E_Desk *desk;
   int x, y;

   zone = data;

   ecore_x_pointer_xy_get(zone->container->win, &x, &y);

   switch (zone->flip.direction)
     {
      case E_DIRECTION_UP:
	 if (E_ZONE_FLIP_UP(zone))
	   {
	      desk = e_desk_at_xy_get(zone, zone->desk_x_current, zone->desk_y_current - 1);
	      if (desk)
		{
		   e_desk_show(desk);
		   ecore_x_pointer_warp(zone->container->win, x, zone->h - 2);
		   _e_zone_update_flip(zone);
		}
	   }
	 break;
      case E_DIRECTION_RIGHT:
	 if (E_ZONE_FLIP_RIGHT(zone))
	   {
	      desk = e_desk_at_xy_get(zone, zone->desk_x_current + 1, zone->desk_y_current);
	      if (desk)
		{
		   e_desk_show(desk);
		   ecore_x_pointer_warp(zone->container->win, 2, y);
		   _e_zone_update_flip(zone);
		}
	   }
	 break;
      case E_DIRECTION_DOWN:
	 if (E_ZONE_FLIP_DOWN(zone))
	   {
	      desk = e_desk_at_xy_get(zone, zone->desk_x_current, zone->desk_y_current + 1);
	      if (desk)
		{
		   e_desk_show(desk);
		   ecore_x_pointer_warp(zone->container->win, x, 2);
		   _e_zone_update_flip(zone);
		}
	   }
	 break;
      case E_DIRECTION_LEFT:
	 if (E_ZONE_FLIP_LEFT(zone))
	   {
	      desk = e_desk_at_xy_get(zone, zone->desk_x_current - 1, zone->desk_y_current);
	      if (desk)
		{
		   e_desk_show(desk);
		   ecore_x_pointer_warp(zone->container->win, zone->w - 2, y);
		   _e_zone_update_flip(zone);
		}
	   }
	 break;
     }

   zone->flip.timer = NULL;

   return 0;
}

static void
_e_zone_update_flip(E_Zone *zone)
{

   if (E_ZONE_FLIP_UP(zone))
     {
	ecore_x_window_show(zone->flip.top);
	e_container_window_raise(zone->container, zone->flip.top, 999);
     }
   else
     ecore_x_window_hide(zone->flip.top);

   if (E_ZONE_FLIP_RIGHT(zone))
     {
	ecore_x_window_show(zone->flip.right);
	e_container_window_raise(zone->container, zone->flip.right, 999);
     }
   else
     ecore_x_window_hide(zone->flip.right);

   if (E_ZONE_FLIP_DOWN(zone))
     {
	ecore_x_window_show(zone->flip.bottom);
	e_container_window_raise(zone->container, zone->flip.bottom, 999);
     }
   else
     ecore_x_window_hide(zone->flip.bottom);

   if (E_ZONE_FLIP_LEFT(zone))
     {
	ecore_x_window_show(zone->flip.left);
	e_container_window_raise(zone->container, zone->flip.left, 999);
     }
   else
     ecore_x_window_hide(zone->flip.left);
}
