/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "config.h"

/* E_Zone is a child object of E_Container. There is one zone per screen
 * in a xinerama setup. Each zone has one or more desktops.
 */

static void _e_zone_free(E_Zone *zone);
static void _e_zone_cb_bg_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_cb_bg_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_cb_bg_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_zone_event_zone_desk_count_set_free(void *data, void *ev);
static int  _e_zone_cb_mouse_in(void *data, int type, void *event);
static int  _e_zone_cb_mouse_out(void *data, int type, void *event);
static int  _e_zone_cb_timer(void *data);
static int  _e_zone_cb_desk_show(void *data, int type, void *event);
static void _e_zone_update_flip(E_Zone *zone);

int E_EVENT_ZONE_DESK_COUNT_SET = 0;
int E_EVENT_POINTER_WARP = 0;

#define E_ZONE_FLIP_UP(zone) ((zone)->desk_y_current > 0)
#define E_ZONE_FLIP_RIGHT(zone) (((zone)->desk_x_current + 1) < (zone)->desk_x_count)
#define E_ZONE_FLIP_DOWN(zone) (((zone)->desk_y_current + 1) < (zone)->desk_y_count)
#define E_ZONE_FLIP_LEFT(zone) ((zone)->desk_x_current > 0)

int
e_zone_init(void)
{
   E_EVENT_ZONE_DESK_COUNT_SET = ecore_event_type_new();
   E_EVENT_POINTER_WARP = ecore_event_type_new();

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
   Evas_Object *o;

   zone = E_OBJECT_ALLOC(E_Zone, E_ZONE_TYPE, _e_zone_free);
   if (!zone) return NULL;

   zone->container = con;

   zone->x = x;
   zone->y = y;
   zone->w = w;
   zone->h = h;
   zone->num = num;

   zone->flip.left = ecore_x_window_input_new(con->win, zone->x, zone->y, 1, zone->h);
   zone->flip.right = ecore_x_window_input_new(con->win, zone->x + zone->w - 1, zone->y, 1, zone->h);
   zone->flip.top = ecore_x_window_input_new(con->win, zone->x + 1, zone->y, zone->w - 2, 1);
   zone->flip.bottom = ecore_x_window_input_new(con->win, zone->x + 1, zone->y + zone->h - 1, zone->w - 2, 1);

   zone->handlers = evas_list_append(zone->handlers,
				     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN,
							     _e_zone_cb_mouse_in, zone));
   zone->handlers = evas_list_append(zone->handlers,
				     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT,
							     _e_zone_cb_mouse_out, zone));
   zone->handlers = evas_list_append(zone->handlers,
				     ecore_event_handler_add(E_EVENT_DESK_SHOW,
							     _e_zone_cb_desk_show, zone));
   

   snprintf(name, sizeof(name), "Zone %d", zone->num);
   zone->name = strdup(name);

   con->zones = evas_list_append(con->zones, zone);
   
   o = evas_object_rectangle_add(con->bg_evas);
   zone->bg_clip_object = o;
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_show(o);

   o = evas_object_rectangle_add(con->bg_evas);
   zone->bg_event_object = o;
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_repeat_events_set(o, 1);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_zone_cb_bg_mouse_down, zone);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   _e_zone_cb_bg_mouse_up, zone);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _e_zone_cb_bg_mouse_move, zone);

   /* TODO: config the ecore_evas type. */
   zone->black_ecore_evas = ecore_evas_software_x11_new(NULL, zone->container->win,
							0, 0, zone->w, zone->h);
   ecore_evas_software_x11_direct_resize_set(zone->black_ecore_evas, 1);
   ecore_evas_override_set(zone->black_ecore_evas, 1);
   ecore_evas_layer_set(zone->black_ecore_evas, 6);

   zone->black_win = ecore_evas_software_x11_window_get(zone->black_ecore_evas);
   zone->black_evas = ecore_evas_get(zone->black_ecore_evas);

   o = evas_object_rectangle_add(zone->black_evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, zone->w, zone->h);
   evas_object_color_set(o, 0, 0, 0, 255);
   ecore_evas_name_class_set(zone->black_ecore_evas, "E", "Black_Window");
   snprintf(name, sizeof(name), "Enlightenment Black Zone (%d)", zone->num);
   ecore_evas_title_set(zone->black_ecore_evas, name);

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
   ecore_x_window_move_resize(zone->flip.left, zone->x, zone->y, 1, zone->h);
   ecore_x_window_move_resize(zone->flip.right, zone->x + zone->w - 1, zone->y, 1, zone->h);
   ecore_x_window_move_resize(zone->flip.top, zone->x + 1, zone->y, zone->w - 2, 1);
   ecore_x_window_move_resize(zone->flip.bottom, zone->x + 1, zone->y + zone->h - 1, zone->w - 2, 1);
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

   ecore_x_window_move_resize(zone->flip.left, zone->x, zone->y, 1, zone->h);
   ecore_x_window_move_resize(zone->flip.right, zone->x + zone->w - 1, zone->y, 1, zone->h);
   ecore_x_window_move_resize(zone->flip.top, zone->x + 1, zone->y, zone->w - 2, 1);
   ecore_x_window_move_resize(zone->flip.bottom, zone->x + 1, zone->y + zone->h - 1, zone->w - 2, 1);
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

void
e_zone_fullscreen_set(E_Zone *zone, int on)
{
   if ((!zone->fullscreen) && (on))
     {
	ecore_evas_show(zone->black_ecore_evas);
	e_container_window_raise(zone->container, zone->black_win, 150);
	zone->fullscreen = 1;
     }
   else if ((zone->fullscreen) && (!on))
     {
	ecore_evas_hide(zone->black_ecore_evas);
	zone->fullscreen = 0;
     }
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
   else if (!starting)
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
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   e_bg_zone_update(zone, E_BG_TRANSITION_CHANGE);
}

void
e_zone_flip_coords_handle(E_Zone *zone, int x, int y)
{
   if ((y == 0) && E_ZONE_FLIP_UP(zone))
     {
	/* top */
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_UP;
     }
   else if ((x == (zone->w - 1)) && E_ZONE_FLIP_RIGHT(zone))
     {
	/* right */
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_RIGHT;
     }
   else if ((y == (zone->h - 1)) && E_ZONE_FLIP_DOWN(zone))
     {
	/* bottom */
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_DOWN;
     }
   else if ((x == 0) && E_ZONE_FLIP_LEFT(zone))
     {
	/* left */
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
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

void
e_zone_desk_count_set(E_Zone *zone, int x_count, int y_count)
{
   E_Desk   **new_desks;
   E_Desk    *desk, *new_desk;
   int        x, y, xx, yy, moved;
   E_Border  *bd;
   E_Event_Zone_Desk_Count_Set *ev;
   E_Border_List *bl;
   
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
		  
		  bl = e_container_border_list_first(zone->container);
		  while ((bd = e_container_border_list_next(bl)))
		    {
		       if (bd->desk == desk)
			 e_border_desk_set(bd, new_desk);
		    }
		  e_container_border_list_free(bl);
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
		  
		  bl = e_container_border_list_first(zone->container);
		  while ((bd = e_container_border_list_next(bl)))
		    {
		       if (bd->desk == desk)
			 e_border_desk_set(bd, new_desk);
		    }
		  e_container_border_list_free(bl);
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
	moved = 1;
     }
   if (zone->desk_y_current >= yy)
     {
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
e_zone_update_flip_all(void)
{
   Evas_List *l, *ll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;
   
   for (l = e_manager_list(); l; l = l->next)
     {
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
	  {
	     con = ll->data;
	     zone = e_zone_current_get(con);
	     e_zone_update_flip(zone);
	  }
     }
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
   if (desk) e_desk_show(desk);
}

void
e_zone_desk_linear_flip_by(E_Zone *zone, int dx)
{
   dx = zone->desk_x_current + 
     (zone->desk_y_current * zone->desk_x_count) + dx;
   dx = dx % (zone->desk_x_count * zone->desk_y_count);
   while (dx < 0)
     dx += (zone->desk_x_count * zone->desk_y_count);
   e_zone_desk_linear_flip_to(zone, dx);
}

void
e_zone_desk_linear_flip_to(E_Zone *zone, int x)
{
   int y;

   if (x < 0) return;
   y = x / zone->desk_x_count;
   x = x - (y * zone->desk_x_count);
   if (y >= zone->desk_y_count) return;
   e_zone_desk_flip_to(zone, x, y);
}

void
e_zone_flip_win_disable(void)
{
   Evas_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   
   for (l = e_manager_list(); l; l = l->next)
     {
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
	  {
	     con = ll->data;
	     for (lll = con->zones; lll; lll = lll->next)
	       {
		  E_Zone *zone;
		  
		  zone = lll->data;
		  ecore_x_window_hide(zone->flip.left);
		  ecore_x_window_hide(zone->flip.right);
		  ecore_x_window_hide(zone->flip.top);
		  ecore_x_window_hide(zone->flip.bottom);
	       }
	  }
     }
}

void
e_zone_flip_win_restore(void)
{
   Evas_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   
   for (l = e_manager_list(); l; l = l->next)
     {
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
	  {
	     con = ll->data;
	     for (lll = con->zones; lll; lll = lll->next)
	       {
		  E_Zone *zone;
		  
		  zone = lll->data;
		  _e_zone_update_flip(zone);
	       }
	  }
     }
}

int
e_zone_app_exec(E_Zone *zone, E_App *a)
{
   static int launch_id = 1;
   char *p1, *p2;
   char *penv_display;
   char *penv_ld_preload;
   char *penv_ld_preload_path;
   char buf[4096], buf2[32];
   
   if (!a) return 0;
   /* save previous env vars we need to save */
   penv_display = getenv("DISPLAY");
   if (penv_display) penv_display = strdup(penv_display);
   penv_ld_preload = getenv("LD_PRELOAD");
   if (penv_ld_preload) penv_ld_preload = strdup(penv_ld_preload);
   penv_ld_preload_path = getenv("LD_PRELOAD_PATH");
   if (penv_ld_preload_path) penv_ld_preload_path = strdup(penv_ld_preload_path);
   
   /* set env vars */
   p1 = strrchr(penv_display, ':');
   p2 = strrchr(penv_display, '.');
   if ((p1) && (p2) && (p2 > p1)) /* "blah:x.y" */
     {
	/* yes it could overflow... but who will voerflow DISPLAY eh? why? to
	 * "exploit" your own applications running as you?
	 */
	strcpy(buf, penv_display);
	buf[p2 - penv_display + 1] = 0;
	snprintf(buf2, sizeof(buf2), "%i", zone->container->manager->num);
	strcat(buf, buf2);
     }
   else if (p1) /* "blah:x */
     {
	strcpy(buf, penv_display);
	snprintf(buf2, sizeof(buf2), ".%i", zone->container->manager->num);
	strcat(buf, buf2);
     }
   else
     strcpy(buf, penv_display);
   e_util_env_set("DISPLAY", buf);
/*   
   snprintf(buf, sizeof(buf), "%i %i", zone->desk_x_current, zone->desk_y_current);
   e_util_env_set("E_DESK", buf);
   snprintf(buf, sizeof(buf), "%i", zone->num);
   e_util_env_set("E_ZONE", buf);
   snprintf(buf, sizeof(buf), "%i", zone->container->num);
   e_util_env_set("E_CONTAINER", buf);
   snprintf(buf, sizeof(buf), "%i", zone->container->manager->num);
   e_util_env_set("E_MANAGER", buf);
   snprintf(buf, sizeof(buf), "%i", launch_id);
   e_util_env_set("E_LAUNCH_ID", buf);
   snprintf(buf, sizeof(buf), "%s/enlightenment/preload", e_prefix_lib_get());
   e_util_env_set("LD_PRELOAD_PATH", buf);
   snprintf(buf, sizeof(buf), "%s/enlightenment/preload/e_hack.so", e_prefix_lib_get());
 */
   snprintf(buf, sizeof(buf), "E_START|%i", launch_id);
   e_util_env_set("DESKTOP_STARTUP_ID", buf);
   if (launch_id == 0) launch_id = 1;
   /* execute */
   if (!e_app_exec(a, launch_id)) launch_id = 0;
   launch_id++;
   
   /* reset env vars */
   if (penv_display)
     {
	e_util_env_set("DISPLAY", penv_display);
	free(penv_display);
     }
/*   
   if (penv_ld_preload)
     {
	e_util_env_set("LD_PRELOAD", penv_ld_preload);
	free(penv_ld_preload);
     }
   if (penv_ld_preload_path)
     {
	e_util_env_set("LD_PRELOAD_PATH", penv_ld_preload_path);
	free(penv_ld_preload_path);
     }
 */
   return launch_id;
}

/* local subsystem functions */
static void
_e_zone_free(E_Zone *zone)
{
   E_Container *con;
   Evas_List *l;
   int x, y;

   if (zone->cur_mouse_action)
     {
	e_object_unref(E_OBJECT(zone->cur_mouse_action));
	zone->cur_mouse_action = NULL;
     }
   
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
   if (zone->prev_bg_object) evas_object_del(zone->prev_bg_object);
   if (zone->transition_object) evas_object_del(zone->transition_object);

   /* free desks */
   for (x = 0; x < zone->desk_x_count; x++)
     {
	for(y = 0; y < zone->desk_y_count; y++)
	  e_object_del(E_OBJECT(zone->desks[x + (y * zone->desk_x_count)]));
     }
   free(zone->desks);
   free(zone);
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
	     if (zone->cur_mouse_action)
	       {
		  if ((!zone->cur_mouse_action->func.end_mouse) &&
		      (!zone->cur_mouse_action->func.end))
		    zone->cur_mouse_action = NULL;
		  if (zone->cur_mouse_action)
		    e_object_ref(E_OBJECT(zone->cur_mouse_action));
	       }
	  }
     }
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
	e_object_unref(E_OBJECT(zone->cur_mouse_action));
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
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_UP;
     }
   else if (ev->win == zone->flip.right)
     {
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_RIGHT;
     }
   else if (ev->win == zone->flip.bottom)
     {
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
	zone->flip.direction = E_DIRECTION_DOWN;
     }
   else if (ev->win == zone->flip.left)
     {
	if (!zone->flip.timer)
	  zone->flip.timer = ecore_timer_add(e_config->edge_flip_timeout, _e_zone_cb_timer, zone);
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

   if ((ev->win == zone->flip.top) ||
       (ev->win == zone->flip.bottom) ||
       (ev->win == zone->flip.left) ||
       (ev->win == zone->flip.right))
     {
	if (zone->flip.timer)
	  ecore_timer_del(zone->flip.timer);
	zone->flip.timer = NULL;
     }
   return 1;
}

static int
_e_zone_cb_timer(void *data)
{
   E_Zone *zone;
   E_Desk *prev = NULL, *current = NULL;
   E_Event_Pointer_Warp *ev;
   int x, y;

   ev = E_NEW(E_Event_Pointer_Warp, 1);
   if (!ev) return 0;

   zone = data;

   ecore_x_pointer_xy_get(zone->container->win, &x, &y);
   ev->prev.x = x;
   ev->prev.y = y;
   prev = e_desk_at_xy_get(zone, zone->desk_x_current, zone->desk_y_current);

   switch (zone->flip.direction)
     {
      case E_DIRECTION_UP:
	if (E_ZONE_FLIP_UP(zone))
	   {
	      current = e_desk_at_xy_get(zone, zone->desk_x_current, zone->desk_y_current - 1);
	      if (current)
		{
		   e_desk_show(current);
		   ecore_x_pointer_warp(zone->container->win, x, zone->h - 2);
		   ev->curr.x = x;
		   ev->curr.y = zone->h - 2;
		}
	   }
	break;
      case E_DIRECTION_RIGHT:
	if (E_ZONE_FLIP_RIGHT(zone))
	  {
	     current = e_desk_at_xy_get(zone, zone->desk_x_current + 1, zone->desk_y_current);
	     if (current)
	       {
		  e_desk_show(current);
		  ecore_x_pointer_warp(zone->container->win, 2, y);
		  ev->curr.y = y;
		  ev->curr.x = 2;
	       }
	  }
	break;
      case E_DIRECTION_DOWN:
	if (E_ZONE_FLIP_DOWN(zone))
	  {
	     current = e_desk_at_xy_get(zone, zone->desk_x_current, zone->desk_y_current + 1);
	     if (current)
	       {
		  e_desk_show(current);
		  ecore_x_pointer_warp(zone->container->win, x, 2);
		  ev->curr.x = x;
		  ev->curr.y = 2;
	       }
	  }
	break;
      case E_DIRECTION_LEFT:
	if (E_ZONE_FLIP_LEFT(zone))
	  {
	     current = e_desk_at_xy_get(zone, zone->desk_x_current - 1, zone->desk_y_current);
	     if (current)
	       {
		  e_desk_show(current);
		  ecore_x_pointer_warp(zone->container->win, zone->w - 2, y);
		  ev->curr.y = y;
		  ev->curr.x = zone->w - 2;
	       }
	  }
	break;
     }
   
   zone->flip.timer = NULL;
   
   if (current)
     ecore_event_add(E_EVENT_POINTER_WARP, ev, NULL, NULL);
   else
     free(ev);
   
   return 0;
}

static int
_e_zone_cb_desk_show(void *data, int type, void *event)
{
   E_Event_Desk_Show *ev;
   E_Zone            *zone;

   ev = event;
   zone = data;
   if (ev->desk->zone != zone) return 1;

   _e_zone_update_flip(zone);
   return 1;
}

static void
_e_zone_update_flip(E_Zone *zone)
{
   
   if ((e_config->use_edge_flip) &&
       /* if we have more than 1 zone - disable */
       (evas_list_count(zone->container->zones) == 1))
     {
	if (E_ZONE_FLIP_LEFT(zone))
	  {
	     ecore_x_window_show(zone->flip.left);
	     e_container_window_raise(zone->container, zone->flip.left, 999);
	  }
	else
	  ecore_x_window_hide(zone->flip.left);
	
	if (E_ZONE_FLIP_RIGHT(zone))
	  {
	     ecore_x_window_show(zone->flip.right);
	     e_container_window_raise(zone->container, zone->flip.right, 999);
	  }
	else
	  ecore_x_window_hide(zone->flip.right);
	
	if (E_ZONE_FLIP_UP(zone))
	  {
	     ecore_x_window_show(zone->flip.top);
	     e_container_window_raise(zone->container, zone->flip.top, 999);
	  }
	else
	  ecore_x_window_hide(zone->flip.top);
	
	if (E_ZONE_FLIP_DOWN(zone))
	  {
	     ecore_x_window_show(zone->flip.bottom);
	     e_container_window_raise(zone->container, zone->flip.bottom, 999);
	  }
	else
	  ecore_x_window_hide(zone->flip.bottom);
	
     }
   else
     {
	ecore_x_window_hide(zone->flip.left);
	ecore_x_window_hide(zone->flip.right);
	ecore_x_window_hide(zone->flip.top);
	ecore_x_window_hide(zone->flip.bottom);
     }
}
