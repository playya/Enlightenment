/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_manager_free(E_Manager *man);

static int _e_manager_cb_window_show_request(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_configure(void *data, int ev_type, void *ev);
#if 0 /* use later - maybe */
static int _e_manager_cb_window_destroy(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_hide(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_reparent(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_create(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_configure_request(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_gravity(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_stack(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_stack_request(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_property(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_colormap(void *data, int ev_type, void *ev);
static int _e_manager_cb_window_shape(void *data, int ev_type, void *ev);
static int _e_manager_cb_client_message(void *data, int ev_type, void *ev);
#endif

/* local subsystem globals */
static Evas_List *managers = NULL;
    
/* externally accessible functions */
int
e_manager_init(void)
{
   return 1;
}

int
e_manager_shutdown(void)
{
   while (managers)
     _e_manager_free((E_Manager *)(managers->data));
   return 1;
}

Evas_List *
e_manager_list(void)
{
   return managers;
}

E_Manager *
e_manager_new(Ecore_X_Window root)
{
   E_Manager *man;
   Ecore_Event_Handler *h;

   if (!ecore_x_window_manage(root)) return NULL;
   man = E_OBJECT_ALLOC(E_Manager, E_MANAGER_TYPE, _e_manager_free);
   if (!man) return NULL;
   managers = evas_list_append(managers, man);
   man->root = root;
   ecore_x_window_size_get(man->root, &(man->w), &(man->h));
   man->win = ecore_x_window_override_new(man->root, man->x, man->y, man->w, man->h);
   ecore_x_icccm_title_set(man->win, "Enlightenment Manager");
   h = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, _e_manager_cb_window_show_request, man);
   if (h) man->handlers = evas_list_append(man->handlers, h);
   h = ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE, _e_manager_cb_window_configure, man);
   if (h) man->handlers = evas_list_append(man->handlers, h);
   return man;
}

void
e_manager_show(E_Manager *man)
{
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   if (man->visible) return;
   ecore_x_window_show(man->win);
   ecore_x_window_focus(man->win);
   e_init_show();
   man->visible = 1;
}

void
e_manager_hide(E_Manager *man)
{
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   if (!man->visible) return;
   ecore_x_window_hide(man->win);
   man->visible = 0; 
}

void
e_manager_move(E_Manager *man, int x, int y)
{
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   if ((x == man->x) && (y == man->y)) return;
   man->x = x;
   man->y = y;
   ecore_x_window_move(man->win, man->x, man->y);
}

void
e_manager_resize(E_Manager *man, int w, int h)
{
   Evas_List *l;
   
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   if ((w == man->w) && (h == man->h)) return;
   man->w = w;
   man->h = h;
   ecore_x_window_resize(man->win, man->w, man->h);
	
   for (l = man->containers; l; l = l->next)
     {
	E_Container *con;
	
	con = l->data;
	e_container_resize(con, man->w, man->h);
     }
}

void
e_manager_move_resize(E_Manager *man, int x, int y, int w, int h)
{
   Evas_List *l;
   
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   if ((x == man->x) && (y == man->y) && (w == man->w) && (h == man->h)) return;
   man->x = x;
   man->y = y;
   man->w = w;
   man->h = h;
   ecore_x_window_move_resize(man->win, man->x, man->y, man->w, man->h);

   for (l = man->containers; l; l = l->next)
     {
	E_Container *con;
	
	con = l->data;
	e_container_resize(con, man->w, man->h);
     }
}

void
e_manager_raise(E_Manager *man)
{
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   ecore_x_window_raise(man->win);
   e_init_show();
}

void
e_manager_lower(E_Manager *man)
{
   E_OBJECT_CHECK(man);
   E_OBJECT_TYPE_CHECK(man, E_MANAGER_TYPE);
   ecore_x_window_lower(man->win);
}

/* local subsystem functions */
static void
_e_manager_free(E_Manager *man)
{
   while (man->handlers)
     {
	Ecore_Event_Handler *h;
   
	h = man->handlers->data;
	man->handlers = evas_list_remove(man->handlers, h);
	ecore_event_handler_del(h);
     }
   while (man->containers)
     e_object_free(E_OBJECT(man->containers->data));
   ecore_x_window_del(man->win);
   managers = evas_list_remove(managers, man);   
   free(man);
}

static int
_e_manager_cb_window_show_request(void *data, int ev_type, void *ev)
{
   E_Manager *man;
   Ecore_X_Event_Window_Show_Request *e;
   
   man = data;
   e = ev;
   if (e->parent != man->root) return 1; /* try other handlers for this */
   
     {
	E_Container *con;
	E_Border *bd;
	
	con = e_container_current_get(man);
	if (!e_border_find_by_client_window(e->win))
	  {
	     bd = e_border_new(con, e->win, 0);
	     if (bd)
	       {
		  e_border_raise(bd);
		  e_border_show(bd);
	       }
	     else ecore_x_window_show(e->win);
	  }
     }
   return 1;
}

static int
_e_manager_cb_window_configure(void *data, int ev_type, void *ev)
{
   E_Manager *man;
   Ecore_X_Event_Window_Configure *e;
   
   man = data;
   e = ev;
   if (e->win != man->root) return 1;
   e_manager_resize(man, e->w, e->h);
   return 1;
}

#if 0 /* use later - maybe */
static int _e_manager_cb_window_destroy(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_hide(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_reparent(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_create(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_configure_request(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_configure(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_gravity(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_stack(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_stack_request(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_property(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_colormap(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_window_shape(void *data, int ev_type, void *ev){return 1;}
static int _e_manager_cb_client_message(void *data, int ev_type, void *ev){return 1;}
#endif
