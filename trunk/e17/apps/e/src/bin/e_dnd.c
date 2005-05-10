/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

static Evas_List *event_handlers = NULL;
static Evas_List *drop_handlers = NULL;

static Ecore_X_Window drag_win = 0;

static Evas_List *draggies = NULL;
static E_Drag *drag_current = NULL;

static int  _e_dnd_cb_mouse_up(void *data, int type, void *event);
static int  _e_dnd_cb_mouse_move(void *data, int type, void *event);
static int  _e_dnd_cb_event_dnd_enter(void *data, int type, void *event);
static int  _e_dnd_cb_event_dnd_leave(void *data, int type, void *event);
static int  _e_dnd_cb_event_dnd_position(void *data, int type, void *event);
static int  _e_dnd_cb_event_dnd_drop(void *data, int type, void *event);
static int  _e_dnd_cb_event_dnd_selection(void *data, int type, void *event);

int
e_dnd_init(void)
{
   Evas_List *l, *l2;
   E_Manager *man;
   E_Container *con;

   event_handlers = evas_list_append(event_handlers,
				     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP,
							     _e_dnd_cb_mouse_up, NULL));
   event_handlers = evas_list_append(event_handlers,
				     ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE,
							     _e_dnd_cb_mouse_move, NULL));

   for (l = e_manager_list(); l; l = l->next)
     {
	man = l->data;

	for (l2 = man->containers; l2; l2 = l2->next)
	  {
	     con = l2->data;

	     ecore_x_dnd_aware_set(con->bg_win, 1);
	     event_handlers = evas_list_append(event_handlers,
					       ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
								       _e_dnd_cb_event_dnd_enter,
								       con));
	     event_handlers = evas_list_append(event_handlers,
					       ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE,
								       _e_dnd_cb_event_dnd_leave,
								       con));
	     event_handlers = evas_list_append(event_handlers,
					       ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
								       _e_dnd_cb_event_dnd_position,
								       con));
	     event_handlers = evas_list_append(event_handlers,
					       ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
								       _e_dnd_cb_event_dnd_drop,
								       con));
	     event_handlers = evas_list_append(event_handlers,
					       ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
								       _e_dnd_cb_event_dnd_selection,
								       con));
	  }
     }
   return 1;
}

int
e_dnd_shutdown(void)
{
   Evas_List *l;

   for (l = draggies; l; l = l->next)
     {
	E_Drag *drag;

	drag = l->data;
	e_drag_del(drag);
     }
   evas_list_free(draggies);
   draggies = NULL;

   for (l = event_handlers; l; l = l->next)
     {
	Ecore_Event_Handler *h;

	h = l->data;
	ecore_event_handler_del(h);
     }
   evas_list_free(event_handlers);
   event_handlers = NULL;

   for (l = drop_handlers; l; l = l->next)
     {
	E_Drop_Handler *h;

	h = l->data;
	e_drop_handler_del(h);
     }
   evas_list_free(drop_handlers);
   drop_handlers = NULL;

   return 1;
}

E_Drag*
e_drag_new(E_Container *con,
	   const char *type, void *data,
	   void (*finished_cb)(E_Drag *drag, int dropped),
	   const char *icon_path, const char *icon)
{
   E_Drag *drag;
   Evas_Coord w, h;

   drag = E_NEW(E_Drag, 1);
   if (!drag) return NULL;

   drag->ee = ecore_evas_software_x11_new(NULL, con->win,
					  0, 0, 10, 10);
   ecore_evas_override_set(drag->ee, 1);
   ecore_evas_software_x11_direct_resize_set(drag->ee, 1);
   ecore_evas_shaped_set(drag->ee, 1);
   e_canvas_add(drag->ee);
   ecore_evas_borderless_set(drag->ee, 1);

   drag->object = edje_object_add(ecore_evas_get(drag->ee));
   edje_object_file_set(drag->object, icon_path, icon);

   w = h = 24;
   evas_object_move(drag->object, 0, 0);
   evas_object_resize(drag->object, w, h);
   ecore_evas_resize(drag->ee, w, h);
   
   drag->type = strdup(type);
   drag->data = data;
   drag->cb.finished = finished_cb;
   drag->container = con;

   return drag;
}

void
e_drag_del(E_Drag *drag)
{
   draggies = evas_list_remove(draggies, drag);

   evas_object_del(drag->object);
   e_canvas_del(drag->ee);
   ecore_evas_free(drag->ee);
   free(drag->type);
   free(drag);
}

void
e_drag_resize(E_Drag *drag, Evas_Coord w, Evas_Coord h)
{
   evas_object_resize(drag->object, w, h);
   ecore_evas_resize(drag->ee, w, h);
}

int
e_dnd_active(void)
{
   return (drag_win != 0);
}

void
e_drag_start(E_Drag *drag)
{
   Evas_List *l;

   drag_win = ecore_x_window_input_new(drag->container->win, 
				       drag->container->x, drag->container->y,
				       drag->container->w, drag->container->h);
   ecore_x_window_show(drag_win);
   ecore_x_pointer_confine_grab(drag_win);
   ecore_x_keyboard_grab(drag_win);

   for (l = drop_handlers; l; l = l->next)
     {
	E_Drop_Handler *h;

	h = l->data;
	
	h->active = !strcmp(h->type, drag->type);
	h->entered = 0;
     }

   drag_current = drag;
}

void
e_drag_update(int x, int y)
{
   Evas_List *l;
   E_Event_Dnd_Enter *enter_ev;
   E_Event_Dnd_Move *move_ev;
   E_Event_Dnd_Leave *leave_ev;
   int w, h;

   if (!drag_current->visible)
     {
	evas_object_show(drag_current->object);
	ecore_evas_show(drag_current->ee);
	ecore_evas_raise(drag_current->ee);
	drag_current->visible = 1;
     }

   evas_object_geometry_get(drag_current->object, NULL, NULL, &w, &h);
   ecore_evas_move(drag_current->ee, x - (w / 2), y - (h / 2));

   enter_ev = E_NEW(E_Event_Dnd_Enter, 1);
   enter_ev->x = x;
   enter_ev->y = y;
   
   move_ev = E_NEW(E_Event_Dnd_Move, 1);
   move_ev->x = x;
   move_ev->y = y;

   leave_ev = E_NEW(E_Event_Dnd_Leave, 1);
   leave_ev->x = x;
   leave_ev->y = y;

   for (l = drop_handlers; l; l = l->next)
     {
	E_Drop_Handler *h;

	h = l->data;

	if (!h->active)
	  continue;
	
	if (E_INSIDE(x, y, h->x, h->y, h->w, h->h))
	  {
	     if (!h->entered)
	       {
		  if (h->cb.enter)
		    h->cb.enter(h->data, drag_current->type, enter_ev);
		  h->entered = 1;
	       }
	     if (h->cb.move)
	       h->cb.move(h->data, drag_current->type, move_ev);
	  }
	else
	  {
	     if (h->entered)
	       {
		  if (h->cb.leave)
		    h->cb.leave(h->data, drag_current->type, leave_ev);
		  h->entered = 0;
	       }
	  }
     }

   free(enter_ev);
   free(move_ev);
   free(leave_ev);
}

void
e_drag_end(int x, int y)
{
   Evas_List *l;
   E_Event_Dnd_Drop *ev;
   int dropped;

   if (drag_current->ee)
     ecore_evas_hide(drag_current->ee);
   drag_current->visible = 0;

   ecore_x_pointer_ungrab();
   ecore_x_keyboard_ungrab();
   ecore_x_window_del(drag_win);
   drag_win = 0;

   ev = E_NEW(E_Event_Dnd_Drop, 1);
   ev->data = drag_current->data;
   ev->x = x;
   ev->y = y;

   dropped = 0;
   for (l = drop_handlers; l; l = l->next)
     {
	E_Drop_Handler *h;

	h = l->data;

	if (!h->active)
	  continue;
	
	if ((h->cb.drop)
	    && E_INSIDE(x, y, h->x, h->y, h->w, h->h))
	  {
	     h->cb.drop(h->data, drag_current->type, ev);
	     dropped = 1;
	  }
     }
   if (drag_current->cb.finished)
     drag_current->cb.finished(drag_current, dropped);

   free(ev);
   drag_current = NULL;
}

E_Drop_Handler *
e_drop_handler_add(void *data,
		   void (*enter_cb)(void *data, const char *type, void *event),
		   void (*move_cb)(void *data, const char *type, void *event),
		   void (*leave_cb)(void *data, const char *type, void *event),
		   void (*drop_cb)(void *data, const char *type, void *event),
		   const char *type, int x, int y, int w, int h)
{
   E_Drop_Handler *handler;

   handler = E_NEW(E_Drop_Handler, 1);
   if (!handler) return NULL;

   handler->data = data;
   handler->cb.enter = enter_cb;
   handler->cb.move = move_cb;
   handler->cb.leave = leave_cb;
   handler->cb.drop = drop_cb;
   handler->type = strdup(type);
   handler->x = x;
   handler->y = y;
   handler->w = w;
   handler->h = h;

   drop_handlers = evas_list_append(drop_handlers, handler);

   return handler;
}

void
e_drop_handler_del(E_Drop_Handler *handler)
{
   free(handler->type);
   free(handler);
}

static int
_e_dnd_cb_mouse_up(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Button_Up *ev;

   ev = event;
   if (ev->win != drag_win) return 1;

   e_drag_end(ev->x, ev->y);

   return 1;
}

static int
_e_dnd_cb_mouse_move(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Move *ev;

   ev = event;
   if (ev->win != drag_win) return 1;

   e_drag_update(ev->x, ev->y);
   return 1;
}

static int
_e_dnd_cb_event_dnd_enter(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Enter *ev;
   E_Container *con;

   ev = event;
   con = data;
   if (con->bg_win != ev->win) return 1;
   printf("Xdnd enter\n");
   return 1;
}

static int
_e_dnd_cb_event_dnd_leave(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Leave *ev;
   E_Container *con;

   ev = event;
   con = data;
   if (con->bg_win != ev->win) return 1;
   printf("Xdnd leave\n");
   return 1;
}

static int
_e_dnd_cb_event_dnd_position(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Position *ev;
   E_Container *con;

   ev = event;
   con = data;
   if (con->bg_win != ev->win) return 1;
   printf("Xdnd pos\n");

#if 0
   for (l = drop_handlers; l; l = l->next)
     {
	E_Drop_Handler *h;

	h = l->data;
	
	if ((x >= h->x) && (x < h->x + h->w) && (y >= h->y) && (y < h->y + h->h)
	    && (!strcmp(h->type, drag_type)))
	  {
	     h->func(h->data, drag_type, ev);
	  }
     }
#endif

   return 1;
}

static int
_e_dnd_cb_event_dnd_drop(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Drop *ev;
   E_Container *con;

   ev = event;
   con = data;
   if (con->bg_win != ev->win) return 1;
   printf("Xdnd drop\n");
   return 1;
}

static int
_e_dnd_cb_event_dnd_selection(void *data, int type, void *event)
{
   Ecore_X_Event_Selection_Notify *ev;
   E_Container *con;

   ev = event;
   con = data;
   if (con->bg_win != ev->win) return 1;
   printf("Xdnd selection\n");
   return 1;
}
