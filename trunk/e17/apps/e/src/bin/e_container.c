/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* TODO List:
 * 
 * * fix shape callbacks to be able to be safely deleted
 */

/* local subsystem functions */
static void _e_container_free(E_Container *con);

static void _e_container_cb_bg_ecore_evas_resize(Ecore_Evas *ee);

static void _e_container_shape_del(E_Container_Shape *es);
static void _e_container_shape_free(E_Container_Shape *es);
static void _e_container_shape_change_call(E_Container_Shape *es, E_Container_Shape_Change ch);
static void _e_container_resize_handle(E_Container *con);
static void _e_container_event_container_resize_free(void *data, void *ev);

int E_EVENT_CONTAINER_RESIZE = 0;
static int container_count;

/* externally accessible functions */
int
e_container_init(void)
{
   E_EVENT_CONTAINER_RESIZE = ecore_event_type_new();
   container_count = 0;
   return 1;
}

int
e_container_shutdown(void)
{
   return 1;
}

E_Container *
e_container_new(E_Manager *man)
{
   E_Container *con;
   E_Zone *zone;
   Evas_Object *o;
   char name[40];
   Evas_List *l, *screens;
   
   con = E_OBJECT_ALLOC(E_Container, E_CONTAINER_TYPE, _e_container_free);
   if (!con) return NULL;
   con->manager = man;
   con->manager->containers = evas_list_append(con->manager->containers, con);
   con->w = con->manager->w;
   con->h = con->manager->h;
   if (e_config->use_virtual_roots)
     {
        Ecore_X_Window mwin;
	
	con->win = ecore_x_window_override_new(con->manager->win, con->x, con->y, con->w, con->h);
	ecore_x_icccm_title_set(con->win, "Enlightenment Container");
	mwin = e_menu_grab_window_get();
	if (!mwin) mwin = e_init_window_get();
	if (!mwin)
	  ecore_x_window_raise(con->win);
	else
	  ecore_x_window_configure(con->win,
				   ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING |
				   ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
				   0, 0, 0, 0, 0,
				   mwin, ECORE_X_WINDOW_STACK_BELOW);
     }
   else
     {
	con->win = con->manager->win;
     }
   con->bg_ecore_evas = ecore_evas_software_x11_new(NULL, con->win, 0, 0, con->w, con->h);
   ecore_evas_override_set(con->bg_ecore_evas, 1);
   e_canvas_add(con->bg_ecore_evas);
   con->bg_evas = ecore_evas_get(con->bg_ecore_evas);
   con->bg_win = ecore_evas_software_x11_window_get(con->bg_ecore_evas);
   ecore_evas_name_class_set(con->bg_ecore_evas, "E", "Background_Window");
   ecore_evas_title_set(con->bg_ecore_evas, "Enlightenment Background");
   ecore_evas_avoid_damage_set(con->bg_ecore_evas, 1);
   ecore_x_window_lower(con->bg_win);

   ecore_evas_callback_resize_set(con->bg_ecore_evas, _e_container_cb_bg_ecore_evas_resize);
   
   o = evas_object_rectangle_add(con->bg_evas);
   con->bg_blank_object = o;
   evas_object_layer_set(o, -100);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, con->w, con->h);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_name_set(o, "desktop/background");
   evas_object_data_set(o, "e_container", con);
   evas_object_show(o);
   
   e_pointer_container_set(con);

   con->num = evas_list_count(con->manager->containers);
   snprintf(name, sizeof(name), "Container %d", con->num);
   con->name = strdup(name);

   screens = (Evas_List *)e_xinerama_screens_get();
   for (l = screens; l; l = l->next)
     {
	E_Screen *scr;
	
	scr = l->data;
	zone = e_zone_new(con, scr->screen, scr->x, scr->y, scr->w, scr->h);
     }
   con->gadman = e_gadman_new(con);
   
   return con;
}
        
void
e_container_show(E_Container *con)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if (con->visible) return;
   ecore_evas_show(con->bg_ecore_evas);
   ecore_x_window_lower(con->bg_win);
   if (con->win != con->manager->win)
     ecore_x_window_show(con->win);
   con->visible = 1;
}
        
void
e_container_hide(E_Container *con)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if (!con->visible) return;
   ecore_evas_hide(con->bg_ecore_evas);
   if (con->win != con->manager->win)
     ecore_x_window_hide(con->win);
   con->visible = 0;
}

E_Container *
e_container_current_get(E_Manager *man)
{
   Evas_List *l;
   E_OBJECT_CHECK_RETURN(man, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(man, E_MANAGER_TYPE, NULL);

   for (l = man->containers; l; l = l->next)
     {
	E_Container *con = l->data;
	if (con->visible)
	  return con;
     }
   return NULL;
}

void
e_container_move(E_Container *con, int x, int y)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if ((x == con->x) && (y == con->y)) return;
   con->x = x;
   con->y = y;
   if (con->win != con->manager->win)
     ecore_x_window_move(con->win, con->x, con->y);
   evas_object_move(con->bg_blank_object, con->x, con->y);
}
        
void
e_container_resize(E_Container *con, int w, int h)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if ((w == con->w) && (h == con->h)) return;
   con->w = w;
   con->h = h;
   if (con->win != con->manager->win)
     ecore_x_window_resize(con->win, con->w, con->h);
   ecore_evas_resize(con->bg_ecore_evas, con->w, con->h);
   evas_object_resize(con->bg_blank_object, con->w, con->h);
}

void
e_container_move_resize(E_Container *con, int x, int y, int w, int h)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if ((x == con->x) && (y == con->y) && (w == con->w) && (h == con->h)) return;
   con->x = x;
   con->y = y;
   con->w = w;
   con->h = h;
   if (con->win != con->manager->win)
     ecore_x_window_move_resize(con->win, con->x, con->y, con->w, con->h);
   ecore_evas_resize(con->bg_ecore_evas, con->w, con->h);
   evas_object_move(con->bg_blank_object, con->x, con->y);
   evas_object_resize(con->bg_blank_object, con->w, con->h);
}

void
e_container_raise(E_Container *con)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if (con->win != con->manager->win)
     {
	ecore_x_window_raise(con->win);
     }
   else
     {
	ecore_x_window_lower(con->bg_win);
     }
}

void
e_container_lower(E_Container *con)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   if (con->win != con->manager->win)
     ecore_x_window_lower(con->win);
   else
     {
	ecore_x_window_lower(con->bg_win);
     }
}

E_Zone *
e_container_zone_at_point_get(E_Container *con, int x, int y)
{
   Evas_List *l;
   
   E_OBJECT_CHECK_RETURN(con, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(con, E_CONTAINER_TYPE, NULL);
   for (l = con->zones; l; l = l->next)
     {
	E_Zone *zone;
	
	zone = l->data;
	if ((E_SPANS_COMMON(zone->x, zone->w, x, 1)) &&
	    (E_SPANS_COMMON(zone->y, zone->h, y, 1)))
	  return zone;
     }
   return NULL;
}

E_Zone *
e_container_zone_number_get(E_Container *con, int num)
{
   Evas_List *l;
   
   E_OBJECT_CHECK_RETURN(con, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(con, E_CONTAINER_TYPE, NULL);
   for (l = con->zones; l; l = l->next)
     {
	E_Zone *zone;
	
	zone = l->data;
	if (zone->num == num)
	  return zone;
     }
   return NULL;
}

E_Container_Shape *
e_container_shape_add(E_Container *con)
{
   E_Container_Shape *es;
   
   E_OBJECT_CHECK_RETURN(con, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(con, E_CONTAINER_TYPE, 0);
   
   es = E_OBJECT_ALLOC(E_Container_Shape, E_CONTAINER_SHAPE_TYPE, _e_container_shape_free);
   E_OBJECT_DEL_SET(es, _e_container_shape_del);
   es->con = con;
   con->shapes = evas_list_append(con->shapes, es);
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_ADD);
   return es;
}

void
e_container_shape_show(E_Container_Shape *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_CONTAINER_SHAPE_TYPE);
   if (es->visible) return;
   es->visible = 1;
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_SHOW);
}

void
e_container_shape_hide(E_Container_Shape *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_CONTAINER_SHAPE_TYPE);
   if (!es->visible) return;
   es->visible = 0;
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_HIDE);
}

void
e_container_shape_move(E_Container_Shape *es, int x, int y)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_CONTAINER_SHAPE_TYPE);
   if ((es->x == x) && (es->y == y)) return;
   es->x = x;
   es->y = y;
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_MOVE);
}

void
e_container_shape_resize(E_Container_Shape *es, int w, int h)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_CONTAINER_SHAPE_TYPE);
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((es->w == w) && (es->h == h)) return;
   es->w = w;
   es->h = h;
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_RESIZE);
}

Evas_List *
e_container_shape_list_get(E_Container *con)
{
   E_OBJECT_CHECK_RETURN(con, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(con, E_CONTAINER_TYPE, NULL);
   return con->shapes;
}

void
e_container_shape_geometry_get(E_Container_Shape *es, int *x, int *y, int *w, int *h)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_CONTAINER_SHAPE_TYPE);
   if (x) *x = es->x;
   if (y) *y = es->y;
   if (w) *w = es->w;
   if (h) *h = es->h;
}

E_Container *
e_container_shape_container_get(E_Container_Shape *es)
{
   E_OBJECT_CHECK_RETURN(es, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(es, E_CONTAINER_SHAPE_TYPE, NULL);
   return es->con;
}

void
e_container_shape_change_callback_add(E_Container *con, void (*func) (void *data, E_Container_Shape *es, E_Container_Shape_Change ch), void *data)
{
   E_Container_Shape_Callback *cb;
   
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   cb = calloc(1, sizeof(E_Container_Shape_Callback));
   if (!cb) return;
   cb->func = func;
   cb->data = data;
   con->shape_change_cb = evas_list_append(con->shape_change_cb, cb);
}

void
e_container_shape_change_callback_del(E_Container *con, void (*func) (void *data, E_Container_Shape *es, E_Container_Shape_Change ch), void *data)
{
   Evas_List *l;

   /* FIXME: if we call this from within a callback we are in trouble */
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   for (l = con->shape_change_cb; l; l = l->next)
     {
	E_Container_Shape_Callback *cb;
	
	cb = l->data;
	if ((cb->func == func) && (cb->data == data))
	  {
	     con->shape_change_cb = evas_list_remove_list(con->shape_change_cb, l);
	     free(cb);
	     return;
	  }
     }
}

Evas_List *
e_container_shape_rects_get(E_Container_Shape *es)
{
   E_OBJECT_CHECK_RETURN(es, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(es, E_CONTAINER_SHAPE_TYPE, NULL);
   return es->shape;
}




/* local subsystem functions */
static void
_e_container_free(E_Container *con)
{
   Evas_List *l, *tmp;
   
   if (con->gadman) e_object_del(E_OBJECT(con->gadman));
   /* We can't use e_object_del here, because border adds a ref to itself
    * when it is removed, and the ref is never unref'ed */
   for (l = con->clients; l;)
     {
	tmp = l;
	l = l->next;
	e_object_free(E_OBJECT(tmp->data));
     }
   for (l = con->zones; l;)
     {
	tmp = l;
	l = l->next;
	e_object_del(E_OBJECT(tmp->data));
     }
   con->manager->containers = evas_list_remove(con->manager->containers, con);
   e_canvas_del(con->bg_ecore_evas);
   ecore_evas_free(con->bg_ecore_evas);
   if (con->manager->win != con->win)
     {
	ecore_x_window_del(con->win);
     }
   free(con);
}
   
static void
_e_container_cb_bg_ecore_evas_resize(Ecore_Evas *ee)
{
   Evas *evas;
   Evas_Object *o;
   E_Container *con;
   Evas_Coord w, h;
   
   evas = ecore_evas_get(ee);
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   o = evas_object_name_find(evas, "desktop/background");
   con = evas_object_data_get(o, "e_container");
   _e_container_resize_handle(con);
}

static void
_e_container_shape_del(E_Container_Shape *es)
{
   _e_container_shape_change_call(es, E_CONTAINER_SHAPE_DEL);
}

static void
_e_container_shape_free(E_Container_Shape *es)
{
   Evas_List *l;

   es->con->shapes = evas_list_remove(es->con->shapes, es);
   for (l = es->shape; l; l = l->next)
     free(l->data);
   evas_list_free(es->shape);
   free(es);
}

static void
_e_container_shape_change_call(E_Container_Shape *es, E_Container_Shape_Change ch)
{
   Evas_List *l;
   
   for (l = es->con->shape_change_cb; l; l = l->next)
     {
	E_Container_Shape_Callback *cb;
	
	cb = l->data;
	cb->func(cb->data, es, ch);
     }
}

static void
_e_container_resize_handle(E_Container *con)
{
   E_Event_Container_Resize *ev;
   Evas_List *l, *screens;
   
   ev = calloc(1, sizeof(E_Event_Container_Resize));
   ev->container = con;

   e_xinerama_update();
   
   screens = (Evas_List *)e_xinerama_screens_get();
   for (l = screens; l; l = l->next)
     {
	E_Screen *scr;
	E_Zone *zone;
	
	scr = l->data;
	zone = e_container_zone_number_get(con, scr->screen);
	if (zone)
	  {
	     e_zone_move(zone, scr->x, scr->y);
	     e_zone_resize(zone, scr->w, scr->h);
	  }
	else
	  {
	     zone = e_zone_new(con, scr->screen, scr->x, scr->y, scr->w, scr->h);
	  }
	/* FIXME: what if a zone exists for a screen that doesn't exist?
	 *        not sure this will ever happen...
	 */
     }
   
   e_gadman_container_resize(con->gadman);
   e_object_ref(E_OBJECT(con));
   ecore_event_add(E_EVENT_CONTAINER_RESIZE, ev, _e_container_event_container_resize_free, NULL);
   for (l = con->clients; l; l = l->next)
     {
	E_Border *bd;
	
	bd = l->data;
	
	if (bd->w > bd->zone->w)
	  e_border_resize(bd, bd->zone->w, bd->h);
	if ((bd->x + bd->w) > (bd->zone->x + bd->zone->w))
	  e_border_move(bd, bd->zone->x + bd->zone->w - bd->w, bd->y);
	    
	if (bd->h > bd->zone->h)
	  e_border_resize(bd, bd->w, bd->zone->h);
	if ((bd->y + bd->h) > (bd->zone->y + bd->zone->h))
	  e_border_move(bd, bd->x, bd->zone->y + bd->zone->h - bd->h);
     }
}

static void
_e_container_event_container_resize_free(void *data, void *ev)
{
   E_Event_Container_Resize *e;
   
   e = ev;
   e_object_unref(E_OBJECT(e->container));
   free(e);
}
