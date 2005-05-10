/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_popup_free(E_Popup *pop);
static int  _e_popup_cb_window_shape(void *data, int ev_type, void *ev);

/* local subsystem globals */
static Ecore_Event_Handler *_e_popup_window_shape_handler = NULL;
static Evas_List *_e_popup_list = NULL;

/* externally accessible functions */

int
e_popup_init(void)
{
   _e_popup_window_shape_handler = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHAPE,
							   _e_popup_cb_window_shape, NULL);
   return 1;
}

int
e_popup_shutdown(void)
{
   E_FN_DEL(ecore_event_handler_del, _e_popup_window_shape_handler);
   return 1;
}

E_Popup *
e_popup_new(E_Zone *zone, int x, int y, int w, int h)
{
   E_Popup *pop;
   
   pop = E_OBJECT_ALLOC(E_Popup, E_POPUP_TYPE, _e_popup_free);
   if (!pop) return NULL;
   pop->zone = zone;
   pop->x = x;
   pop->y = y;
   pop->w = w;
   pop->h = h;
   pop->layer = 250;
   if (e_canvas_engine_decide(e_config->evas_engine_popups) ==
       E_EVAS_ENGINE_GL_X11)
     {
	pop->ecore_evas = ecore_evas_gl_x11_new(NULL,
						pop->zone->container->win,
						pop->zone->x + pop->x,
						pop->zone->y + pop->y,
						pop->w, pop->h);
	ecore_evas_gl_x11_direct_resize_set(pop->ecore_evas, 1);
	pop->evas_win = ecore_evas_gl_x11_window_get(pop->ecore_evas);
     }
   else
     {
	pop->ecore_evas = ecore_evas_software_x11_new(NULL,
						      pop->zone->container->win,
						      pop->zone->x + pop->x,
						      pop->zone->y + pop->y,
						      pop->w, pop->h);
	ecore_evas_software_x11_direct_resize_set(pop->ecore_evas, 1);
	pop->evas_win = ecore_evas_software_x11_window_get(pop->ecore_evas);
     }
   e_canvas_add(pop->ecore_evas);
   pop->shape = e_container_shape_add(pop->zone->container);
   e_container_shape_move(pop->shape, pop->zone->x + pop->x, pop->zone->y + pop->y);
   e_container_shape_resize(pop->shape, pop->w, pop->h);
   pop->evas = ecore_evas_get(pop->ecore_evas);
   e_container_window_raise(pop->zone->container, pop->evas_win, pop->layer);
   ecore_x_window_shape_events_select(pop->evas_win, 1);
   ecore_evas_name_class_set(pop->ecore_evas, "E", "_e_popup_window");
   ecore_evas_title_set(pop->ecore_evas, "E Popup");
   e_object_ref(E_OBJECT(pop->zone));
   pop->zone->popups = evas_list_append(pop->zone->popups, pop);
   _e_popup_list = evas_list_append(_e_popup_list, pop);
   return pop;
}

void
e_popup_show(E_Popup *pop)
{
   if (pop->visible) return;
   pop->visible = 1;
   ecore_evas_show(pop->ecore_evas);
   e_container_shape_show(pop->shape);
}

void
e_popup_hide(E_Popup *pop)
{
   if (!pop->visible) return;
   pop->visible = 0;
   ecore_evas_hide(pop->ecore_evas);
   e_container_shape_hide(pop->shape);
}

void
e_popup_move(E_Popup *pop, int x, int y)
{
   if ((pop->x == x) && (pop->y == y)) return;
   pop->x = x;
   pop->y = y;
   ecore_evas_move(pop->ecore_evas,
		   pop->zone->x + pop->x, 
		   pop->zone->y + pop->y);
   e_container_shape_move(pop->shape,
			  pop->zone->x + pop->x, 
			  pop->zone->y + pop->y);
}

void
e_popup_resize(E_Popup *pop, int w, int h)
{
   if ((pop->w == w) && (pop->h == h)) return;
   pop->w = w;
   pop->h = h;
   ecore_evas_resize(pop->ecore_evas, pop->w, pop->h);
   e_container_shape_resize(pop->shape, pop->w, pop->h);
}
  
void
e_popup_move_resize(E_Popup *pop, int x, int y, int w, int h)
{
   if ((pop->x == x) && (pop->y == y) &&
       (pop->w == w) && (pop->h == h)) return;
   pop->x = x;
   pop->y = y;
   pop->w = w;
   pop->h = h;
   ecore_evas_move_resize(pop->ecore_evas,
			  pop->zone->x + pop->x, 
			  pop->zone->y + pop->y,
			  pop->w, pop->h);
   e_container_shape_move(pop->shape,
			  pop->zone->x + pop->x, 
			  pop->zone->y + pop->y);
   e_container_shape_resize(pop->shape, pop->w, pop->h);
}

void
e_popup_edje_bg_object_set(E_Popup *pop, Evas_Object *o)
{
   const char *shape_option;
   
   shape_option = edje_object_data_get(o, "shaped");
   if (shape_option)
     {
	if (!strcmp(shape_option, "1"))
	  pop->shaped = 1;
	else
	  pop->shaped = 0;
	ecore_evas_shaped_set(pop->ecore_evas, pop->shaped);
     }
}

void
e_popup_layer_set(E_Popup *pop, int layer)
{
   pop->layer = layer;
   e_container_window_raise(pop->zone->container, pop->evas_win, pop->layer);
}

void
e_popup_idler_before(void)
{
   Evas_List *l;
   
   for (l = _e_popup_list; l; l = l->next)
     {
	E_Popup *pop;
	
	pop = l->data;
	if (pop->need_shape_export)
	  {
	     Ecore_X_Rectangle *rects;
	     int num;
	     
	     rects = ecore_x_window_shape_rectangles_get(pop->evas_win, &num);
	     if (rects)
	       {
		  e_container_shape_rects_set(pop->shape, rects, num);
		  free(rects);
	       }
	     pop->need_shape_export = 0;
	     if (pop->visible)
	       e_container_shape_show(pop->shape);
	  }
     }
}

/* local subsystem functions */

static void
_e_popup_free(E_Popup *pop)
{
   e_container_shape_hide(pop->shape);
   e_object_del(E_OBJECT(pop->shape));
   e_canvas_del(pop->ecore_evas);
   ecore_evas_free(pop->ecore_evas);
   e_object_unref(E_OBJECT(pop->zone));
   pop->zone->popups = evas_list_remove(pop->zone->popups, pop);
   _e_popup_list = evas_list_remove(_e_popup_list, pop);
   free(pop);
}

static int
_e_popup_cb_window_shape(void *data, int ev_type, void *ev)
{
   Evas_List *l;
   Ecore_X_Event_Window_Shape *e;
   
   e = ev;
   for (l = _e_popup_list; l; l = l->next)
     {
	E_Popup *pop;
	
	pop = l->data;
	if (pop->evas_win == e->win)
	  pop->need_shape_export = 1;
     }
   return 1;
}
