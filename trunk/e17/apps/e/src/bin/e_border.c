#include "e.h"

#define RESIZE_NONE 0
#define RESIZE_TL   1
#define RESIZE_T    2
#define RESIZE_TR   3
#define RESIZE_R    4
#define RESIZE_BR   5
#define RESIZE_B    6
#define RESIZE_BL   7
#define RESIZE_L    8

/* local subsystem functions */
static void _e_border_free(E_Border *bd);

/* FIXME: these likely belong in a separate icccm/client handler */
/* and the border needs to be come a dumb object that just does what its */
/* told to do */
static int _e_border_cb_window_show_request(void *data, int ev_type, void *ev);
static int _e_border_cb_window_destroy(void *data, int ev_type, void *ev);
static int _e_border_cb_window_hide(void *data, int ev_type, void *ev);
static int _e_border_cb_window_reparent(void *data, int ev_type, void *ev);
static int _e_border_cb_window_configure_request(void *data, int ev_type, void *ev);
static int _e_border_cb_window_gravity(void *data, int ev_type, void *ev);
static int _e_border_cb_window_stack_request(void *data, int ev_type, void *ev);
static int _e_border_cb_window_property(void *data, int ev_type, void *ev);
static int _e_border_cb_window_colormap(void *data, int ev_type, void *ev);
static int _e_border_cb_window_shape(void *data, int ev_type, void *ev);
static int _e_border_cb_window_focus_in(void *data, int ev_type, void *ev);
static int _e_border_cb_window_focus_out(void *data, int ev_type, void *ev);
static int _e_border_cb_client_message(void *data, int ev_type, void *ev);

static void _e_border_cb_signal_move_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_move_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_tl_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_t_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_tr_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_r_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_br_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_b_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_bl_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_l_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_resize_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_border_cb_signal_action(void *data, Evas_Object *obj, const char *emission, const char *source);
static int  _e_border_cb_mouse_in(void *data, int type, void *event);
static int  _e_border_cb_mouse_out(void *data, int type, void *event);
static int  _e_border_cb_mouse_down(void *data, int type, void *event);
static int  _e_border_cb_mouse_up(void *data, int type, void *event);
static int  _e_border_cb_mouse_move(void *data, int type, void *event);
static int  _e_border_cb_mouse_wheel(void *data, int type, void *event);

static void _e_border_eval(E_Border *bd);
static void _e_border_resize_limit(E_Border *bd, int *w, int *h);
static void _e_border_moveinfo_gather(E_Border *bd, const char *source);
static void _e_border_resize_handle(E_Border *bd);
    
/* local subsystem globals */
static Evas_List *handlers = NULL;
static Evas_List *borders = NULL;
static E_Border  *focused = NULL;

/* externally accessible functions */
int
e_border_init(void)
{
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, _e_border_cb_window_show_request, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, _e_border_cb_window_destroy, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_HIDE, _e_border_cb_window_hide, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_REPARENT, _e_border_cb_window_reparent, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, _e_border_cb_window_configure_request, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_GRAVITY, _e_border_cb_window_gravity, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_STACK_REQUEST, _e_border_cb_window_stack_request, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY, _e_border_cb_window_property, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_COLORMAP, _e_border_cb_window_colormap, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHAPE, _e_border_cb_window_shape, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, _e_border_cb_window_focus_in, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, _e_border_cb_window_focus_out, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, _e_border_cb_client_message, NULL));
   return 1;
}

int
e_border_shutdown(void)
{
   while (handlers)
     {
	Ecore_Event_Handler *h;
   
	h = handlers->data;
	handlers = evas_list_remove(handlers, h);
	ecore_event_handler_del(h);
     }
   return 1;
}

E_Border *
e_border_new(E_Container *con, Ecore_X_Window win, int first_map)
{
   E_Border *bd;
   Ecore_Event_Handler *h;
   Ecore_X_Window_Attributes *att;
   
   bd = E_OBJECT_ALLOC(E_Border, _e_border_free);
   if (!bd) return NULL;
   bd->container = con;
   bd->w = 1;
   bd->h = 1;
   bd->win = ecore_x_window_override_new(bd->container->win, 0, 0, bd->w, bd->h);
   bd->bg_ecore_evas = ecore_evas_software_x11_new(NULL, bd->win, 0, 0, bd->w, bd->h);
   ecore_evas_software_x11_direct_resize_set(bd->bg_ecore_evas, 1);
   e_canvas_add(bd->bg_ecore_evas);
   bd->event_win = ecore_x_window_input_new(bd->win, 0, 0, bd->w, bd->h);
   bd->bg_evas = ecore_evas_get(bd->bg_ecore_evas);
   bd->bg_win = ecore_evas_software_x11_window_get(bd->bg_ecore_evas);
   ecore_evas_name_class_set(bd->bg_ecore_evas, "E", "Frame_Window");
   ecore_evas_title_set(bd->bg_ecore_evas, "Enlightenment Frame");
   /* ecore_evas_avoid_damage_set(bd->bg_ecore_evas, 1); */
   ecore_evas_show(bd->bg_ecore_evas);
   evas_font_path_append(bd->bg_evas, e_path_find(path_data, "fonts"));
   bd->client.shell_win = ecore_x_window_override_new(bd->win, 0, 0, 1, 1);
   ecore_x_window_container_manage(bd->client.shell_win);
   ecore_x_window_client_manage(win);
   /* FIXME: Round trip. XCB */
   /* 2nd fetch needed to avoid grabbing the server as window may vanish */
   att = &bd->client.initial_attributes;
   if ((!ecore_x_window_attributes_get(win, att)) || (att->input_only))
     {
	e_canvas_del(bd->bg_ecore_evas);
	ecore_evas_free(bd->bg_ecore_evas);
	ecore_x_window_del(bd->client.shell_win);
	ecore_x_window_del(bd->win);
	free(bd);
	return NULL;
     }
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN, _e_border_cb_mouse_in, bd));
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT, _e_border_cb_mouse_out, bd));
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, _e_border_cb_mouse_down, bd));
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, _e_border_cb_mouse_up, bd));
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE, _e_border_cb_mouse_move, bd));
   bd->handlers = evas_list_append(bd->handlers, ecore_event_handler_add(ECORE_X_EVENT_MOUSE_WHEEL, _e_border_cb_mouse_wheel, bd));
   
   bd->client.win = win;

   bd->client.icccm.title = strdup("");
   bd->client.icccm.name = strdup("");
   bd->client.icccm.class = strdup("");
   bd->client.icccm.icon_name = strdup("");
   bd->client.icccm.machine = strdup("");
   bd->client.icccm.min_w = 1;
   bd->client.icccm.min_h = 1;
   bd->client.icccm.max_w = 32767;
   bd->client.icccm.max_h = 32767;
   bd->client.icccm.base_w = 0;
   bd->client.icccm.base_h = 0;
   bd->client.icccm.step_w = 1;
   bd->client.icccm.step_h = 1;
   bd->client.icccm.min_aspect = 0.0;
   bd->client.icccm.max_aspect = 0.0;
   
   bd->client.icccm.fetch.title = 1;
   bd->client.icccm.fetch.name_class = 1;
   bd->client.icccm.fetch.icon_name = 1;
   bd->client.icccm.fetch.machine = 1;
   bd->client.icccm.fetch.hints = 1;
   bd->client.icccm.fetch.size_pos_hints = 1;
   bd->client.icccm.fetch.protocol = 1;
   bd->client.mwm.fetch.hints = 1;
   bd->client.netwm.fetch.pid = 1;
   bd->client.netwm.fetch.desktop = 1;
   bd->client.border.changed = 1;
   
   bd->client.w = att->w;
   bd->client.h = att->h;
   
   bd->w = bd->client.w;
   bd->h = bd->client.h;
   bd->changes.size = 1;
   
   /* FIXME: if first_map is 1 then we should ignore the first hide event 
    * or ensure the window is alreayd hidden and events flushed before we
    * create a border for it
    */
   if (first_map)
     {
	bd->x = att->x;
	bd->y = att->y;
	bd->changes.pos = 1;
	bd->re_manage = 1;
	bd->ignore_first_unmap = 2;
     }
   
   ecore_x_window_save_set_add(win);
   ecore_x_window_reparent(win, bd->client.shell_win, 0, 0);
   ecore_x_window_border_width_set(win, 0);
   ecore_x_window_show(bd->event_win);
   ecore_x_window_show(bd->client.shell_win);
   bd->shape = e_container_shape_add(con);

   bd->new_client = 1;
   bd->changed = 1;
   
   con->clients = evas_list_append(con->clients, bd);
   borders = evas_list_append(borders, bd);
   return bd;
}

void
e_border_show(E_Border *bd)
{
   E_OBJECT_CHECK(bd);
   if (bd->visible) return;
   e_container_shape_show(bd->shape);
   ecore_x_window_show(bd->client.win);
   ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_NORMAL);
   bd->visible = 1;
   bd->changed = 1;
   bd->changes.visible = 1;
}

void
e_border_hide(E_Border *bd)
{
   E_OBJECT_CHECK(bd);
   if (!bd->visible) return;
   ecore_x_window_hide(bd->client.win);
   e_container_shape_hide(bd->shape);
   /* FIXME: might be iconic too - need to do this elsewhere */
   ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_WITHDRAWN);
   /* ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_ICONIC); */
   bd->visible = 0;
   bd->changed = 1;
   bd->changes.visible = 1;
}

void
e_border_move(E_Border *bd, int x, int y)
{
   E_OBJECT_CHECK(bd);
   if ((x == bd->x) && (y == bd->y)) return;
   bd->x = x;
   bd->y = y;
   bd->changed = 1;
   bd->changes.pos = 1;
   ecore_x_icccm_move_resize_send(bd->client.win, 
				  bd->x + bd->client_inset.l, 
				  bd->y + bd->client_inset.t, 
				  bd->client.w,
				  bd->client.h);
}

void
e_border_resize(E_Border *bd, int w, int h)
{
   E_OBJECT_CHECK(bd);
   if ((w == bd->w) && (h == bd->h)) return;
   bd->w = w;
   bd->h = h;
   bd->client.w = bd->w - (bd->client_inset.l + bd->client_inset.r);
   bd->client.h = bd->h - (bd->client_inset.t + bd->client_inset.b);
   bd->changed = 1;
   bd->changes.size = 1;
   ecore_x_icccm_move_resize_send(bd->client.win, 
				  bd->x + bd->client_inset.l, 
				  bd->y + bd->client_inset.t, 
				  bd->client.w,
				  bd->client.h);
}

void
e_border_move_resize(E_Border *bd, int x, int y, int w, int h)
{
   E_OBJECT_CHECK(bd);
   if ((x == bd->x) && (y == bd->y) && (w == bd->w) && (h == bd->h)) return;
   bd->x = x;
   bd->y = y;
   bd->w = w;
   bd->h = h;
   bd->client.w = bd->w - (bd->client_inset.l + bd->client_inset.r);
   bd->client.h = bd->h - (bd->client_inset.t + bd->client_inset.b);
   bd->changed = 1;
   bd->changes.pos = 1;
   bd->changes.size = 1;
   ecore_x_icccm_move_resize_send(bd->client.win, 
				  bd->x + bd->client_inset.l, 
				  bd->y + bd->client_inset.t, 
				  bd->client.w,
				  bd->client.h);
}

void
e_border_raise(E_Border *bd)
{
   E_OBJECT_CHECK(bd);
   bd->container->clients = evas_list_remove(bd->container->clients, bd);
   bd->container->clients = evas_list_append(bd->container->clients, bd);
   ecore_x_window_raise(bd->win);
}

void
e_border_lower(E_Border *bd)
{
   E_OBJECT_CHECK(bd);
   bd->container->clients = evas_list_remove(bd->container->clients, bd);
   bd->container->clients = evas_list_prepend(bd->container->clients, bd);
   ecore_x_window_lower(bd->win);
}

void
e_border_stack_above(E_Border *bd, E_Border *above)
{
   E_OBJECT_CHECK(bd);
   bd->container->clients = evas_list_remove(bd->container->clients, bd);
   bd->container->clients = evas_list_append_relative(bd->container->clients, bd, above);
   ecore_x_window_configure(bd->win,
			    ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING |
			    ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
			    0, 0, 0, 0, 0, 
			    above->win, ECORE_X_WINDOW_STACK_ABOVE);
}

void
e_border_stack_below(E_Border *bd, E_Border *below)
{
   E_OBJECT_CHECK(bd);
   bd->container->clients = evas_list_remove(bd->container->clients, bd);
   bd->container->clients = evas_list_prepend_relative(bd->container->clients, bd, below);
   ecore_x_window_configure(bd->win,
			    ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING |
			    ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
			    0, 0, 0, 0, 0, 
			    below->win, ECORE_X_WINDOW_STACK_BELOW);
}

void
e_border_focus_set(E_Border *bd, int focus, int set)
{
   E_OBJECT_CHECK(bd);
   if (!bd->client.icccm.accepts_focus) return;
   if ((focus) && (!bd->focused))
     edje_object_signal_emit(bd->bg_object, "active", "");
   else if ((!focus) && (bd->focused))
     edje_object_signal_emit(bd->bg_object, "passive", "");
   bd->focused = focus;
   if (set)
     {
	if (bd->focused)
	  {
	     if ((focused != bd) && (focused))
	       e_border_focus_set(focused, 0, 0);
	     if (bd->client.icccm.take_focus)
	       ecore_x_icccm_take_focus_send(bd->client.win, ECORE_X_CURRENT_TIME);
	     else
	       ecore_x_window_focus(bd->client.win);
	  }
	else
	  {
	     ecore_x_window_focus(bd->container->manager->win);
	  }
     }
   if ((bd->focused) && (focused != bd))
     focused = bd;
   else if ((!bd->focused) && (focused == bd))
     focused = NULL;

   printf("F %x %i\n", bd->client.win, bd->focused);
}

E_Border *
e_border_find_by_client_window(Ecore_X_Window win)
{
   Evas_List *l;
   
   for (l = borders; l; l = l->next)
     {
	E_Border *bd;
	
	bd = l->data;
	if (bd->client.win == win) return bd;
     }
   return NULL;
}

void
e_border_idler_before(void)
{
   Evas_List *l;

   for (l = borders; l; l = l->next)
     {
	E_Border *bd;
	
	bd = l->data;
	if (bd->changed) _e_border_eval(bd);
     }
}


/* local subsystem functions */
static void
_e_border_free(E_Border *bd)
{
   if (focused == bd) focused = NULL;
   while (bd->handlers)
     {
	Ecore_Event_Handler *h;
   
	h = bd->handlers->data;
	bd->handlers = evas_list_remove(bd->handlers, h);
	ecore_event_handler_del(h);
     }
   ecore_x_window_reparent(bd->client.win, bd->container->manager->root, bd->x + bd->client_inset.l, bd->y + bd->client_inset.t);
   ecore_x_window_save_set_del(bd->client.win);
   if (bd->client.border.name) free(bd->client.border.name);
   if (bd->client.icccm.title) free(bd->client.icccm.title);
   if (bd->client.icccm.name) free(bd->client.icccm.name);
   if (bd->client.icccm.class) free(bd->client.icccm.class);
   if (bd->client.icccm.icon_name) free(bd->client.icccm.icon_name);
   if (bd->client.icccm.machine) free(bd->client.icccm.machine);
   e_object_del(E_OBJECT(bd->shape));
   evas_object_del(bd->bg_object);
   e_canvas_del(bd->bg_ecore_evas);
   ecore_evas_free(bd->bg_ecore_evas);
   ecore_x_window_del(bd->client.shell_win);
   ecore_x_window_del(bd->win);
   bd->container->clients = evas_list_remove(bd->container->clients, bd);
   borders = evas_list_remove(borders, bd);
   free(bd);
}

static int
_e_border_cb_window_show_request(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Show_Request *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   return 1;
}

static int _e_border_cb_window_destroy(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Destroy *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   e_object_del(E_OBJECT(bd));
   return 1;
}

static int
_e_border_cb_window_hide(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Hide *e;
   
   bd = data;
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   if (bd->ignore_first_unmap > 0)
     {
	bd->ignore_first_unmap--;
	return 1;
     }
   e_object_del(E_OBJECT(bd));
   return 1;
}

static int
_e_border_cb_window_reparent(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Reparent *e;
   
   bd = data;
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   if (e->parent == bd->client.shell_win) return 1;
   e_object_del(E_OBJECT(bd));
   return 1;
}

static int
_e_border_cb_window_configure_request(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Configure_Request *e;
   
   bd = data;
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd)
     {
	printf("generic config request %x %i %i %ix%i ...\n",
	       e->win, e->x, e->y, e->w, e->h);
	ecore_x_window_configure(e->win, e->value_mask,
				 e->x, e->y, e->w, e->h, e->border,
				 e->abovewin, e->detail);
	return 1;
     }
   printf("config req %0x\n", e->win);
     {
	if ((e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_X) ||
	    (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_Y))
	  {
	     int x, y;
	     
	     y = bd->y;
	     x = bd->x;
	     if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_X)
	       x = e->x - bd->client_inset.l;
	     if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_Y)
	       y = e->y - bd->client_inset.t;
	     if ((e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_W) ||
		 (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_H))
	       {
		  int w, h;
		  
		  h = bd->h;
		  w = bd->w;
		  if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_W)
		    w = e->w + bd->client_inset.l + bd->client_inset.r;
		  if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_H)
		    h = e->h + bd->client_inset.t + bd->client_inset.b;
		  e_border_move_resize(bd, x, y, w, h);
	       }
	     else
	       e_border_move(bd, x, y);
	  }
	else if ((e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_W) ||
		 (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_H))
	  {
	     int w, h;
	     
	     h = bd->h;
	     w = bd->w;
	     if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_W)
	       w = e->w + bd->client_inset.l + bd->client_inset.r;
	     if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_H)
	       h = e->h + bd->client_inset.t + bd->client_inset.b;
	     e_border_resize(bd, w, h);
	  }
	if ((e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE) &&
	    (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING))
	  {
	     E_Border *obd;
	     
	     if (e->detail == ECORE_X_WINDOW_STACK_ABOVE)
	       {
		  obd = e_border_find_by_client_window(e->abovewin);
		  if (obd)
		    e_border_stack_above(bd, obd);
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_BELOW)
	       {
		  obd = e_border_find_by_client_window(e->abovewin);
		  if (obd)
		    e_border_stack_below(bd, obd);
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_TOP_IF)
	       {
		  /* FIXME: do */
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_BOTTOM_IF)
	       {
		  /* FIXME: do */
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_OPPOSITE)
	       {
		  /* FIXME: do */
	       }
	  }
	else if (e->value_mask & ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE)
	  {
	     if (e->detail == ECORE_X_WINDOW_STACK_ABOVE)
	       {
		  e_border_raise(bd);
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_BELOW)
	       {
		  e_border_lower(bd);
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_TOP_IF)
	       {
		  /* FIXME: do */
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_BOTTOM_IF)
	       {
		  /* FIXME: do */
	       }
	     else if (e->detail == ECORE_X_WINDOW_STACK_OPPOSITE)
	       {
		  /* FIXME: do */
	       }
	  }
     }
   return 1;
}

static int
_e_border_cb_window_gravity(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Gravity *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   printf("gravity for %0x\n", e->win);
   return 1;
}

static int
_e_border_cb_window_stack_request(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Stack_Request *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   printf("stack req for %0x bd %p\n", e->win, bd);
   if (!bd) return 1;
   return 1;
}

static int
_e_border_cb_window_property(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Property *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
     {
	char *name;
	
	name = XGetAtomName(ecore_x_display_get(), e->atom);
	printf("property for %0x [%s]\n", e->win, name);
	XFree(name);
     }
   /* FIXME: only flag the property to fetch based on the atom of the change */
   bd->client.icccm.fetch.title = 1;
   bd->client.icccm.fetch.name_class = 1;
   bd->client.icccm.fetch.icon_name = 1;
   bd->client.icccm.fetch.machine = 1;
   bd->client.icccm.fetch.hints = 1;
   bd->client.icccm.fetch.size_pos_hints = 1;
   bd->client.icccm.fetch.protocol = 1;
   bd->client.mwm.fetch.hints = 1;
   bd->client.netwm.fetch.pid = 1;
   bd->client.netwm.fetch.desktop = 1;
//   bd->client.border.changed = 1;
   bd->changed = 1;
   return 1;
}

static int
_e_border_cb_window_colormap(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Colormap *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   return 1;
}

static int
_e_border_cb_window_shape(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Shape *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   return 1;
}

static int
_e_border_cb_window_focus_in(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Focus_In *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   printf("f IN  %i | %i\n", e->mode, e->detail);
   e_border_focus_set(bd, 1, 0);
   return 1;
}

static int
_e_border_cb_window_focus_out(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Window_Focus_Out *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   printf("f OUT %i | %i\n", e->mode, e->detail);
   e_border_focus_set(bd, 0, 0);
   return 1;
}

static int
_e_border_cb_client_message(void *data, int ev_type, void *ev)
{
   E_Border *bd;
   Ecore_X_Event_Client_Message *e;
   
   e = ev;
   bd = e_border_find_by_client_window(e->win);
   if (!bd) return 1;
   printf("client message for %0x\n", e->win);
   return 1;
}

static void
_e_border_cb_signal_move_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->moving = 1;
   _e_border_moveinfo_gather(bd, source);
   e_border_raise(bd);
}

static void
_e_border_cb_signal_move_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   int x, y;
   
   bd = data;
   bd->moving = 0;
}

#define GRAV_SET(bd, grav) \
printf("GRAV TO %i\n", grav); \
ecore_x_window_gravity_set(bd->bg_win, grav); \
ecore_x_window_pixel_gravity_set(bd->bg_win, grav); \
ecore_x_window_gravity_set(bd->client.shell_win, grav); \
ecore_x_window_pixel_gravity_set(bd->client.shell_win, grav); \
ecore_x_window_gravity_set(bd->client.win, grav); \
ecore_x_window_gravity_set(ecore_evas_software_x11_subwindow_get(bd->bg_ecore_evas), grav); \
ecore_x_window_pixel_gravity_set(ecore_evas_software_x11_subwindow_get(bd->bg_ecore_evas), grav);

static void
_e_border_cb_signal_resize_tl_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_TL;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_SE);
}

static void
_e_border_cb_signal_resize_t_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_T;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_S);
   ecore_x_window_gravity_set(bd->bg_win, ECORE_X_GRAVITY_S);
}

static void
_e_border_cb_signal_resize_tr_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_TR;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_SW);
}

static void
_e_border_cb_signal_resize_r_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_R;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_W);
}

static void
_e_border_cb_signal_resize_br_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_BR;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_NW);
}

static void
_e_border_cb_signal_resize_b_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_B;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_N);
}

static void
_e_border_cb_signal_resize_bl_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_BL;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_NE);
}

static void
_e_border_cb_signal_resize_l_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   bd->resize_mode = RESIZE_L;
   _e_border_moveinfo_gather(bd, source);
   GRAV_SET(bd, ECORE_X_GRAVITY_E);
}

static void
_e_border_cb_signal_resize_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   int x, y;
   
   bd = data;
   _e_border_resize_handle(bd);
   bd->resize_mode = RESIZE_NONE;
   bd->changes.reset_gravity = 1;
   bd->changed = 1;
}

static void
_e_border_cb_signal_action(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Border *bd;
   
   bd = data;
   printf("action %s\n", source);
   if (!strcmp(source, "close"))
     {
	if (bd->client.icccm.delete_request)
	  ecore_x_window_delete_request_send(bd->client.win);
	else
	  {
	     ecore_x_killall(bd->client.win);
//	     ecore_x_window_del(bd->client.win);
	     e_object_del(E_OBJECT(bd));
	  }
     }
}

static int
_e_border_cb_mouse_in(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_In *ev;
   E_Border *bd;
   
   ev = event;
   bd = data;
//   if (ev->mode == ECORE_X_EVENT_MODE_GRAB) return 1;
//   if (ev->mode == ECORE_X_EVENT_MODE_UNGRAB) return 1;
//   if (ev->mode == ECORE_X_EVENT_MODE_WHILE_GRABBED) return 1;
   if (ev->event_win == bd->win)
     {
	/* FIXME: this would normally put focus on the client on pointer */
	/* focus - but click to focus it wouldnt */
	e_border_focus_set(bd, 1, 1);
     }
   if (ev->win != bd->event_win) return 1;
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
   evas_event_feed_mouse_in(bd->bg_evas);
   return 1;
}

static int
_e_border_cb_mouse_out(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Out *ev;
   E_Border *bd;

   ev = event;
   bd = data;
   /* FIXME: this would normally take focus away in pointer focus mode */
//   if (ev->mode == ECORE_X_EVENT_MODE_UNGRAB) return 1;
//   if (ev->mode == ECORE_X_EVENT_MODE_WHILE_GRABBED) return 1;
   if (ev->event_win == bd->win)
     {
	const char *modes[] = {   
	   "ECORE_X_EVENT_MODE_NORMAL",
	     "ECORE_X_EVENT_MODE_WHILE_GRABBED",
	     "ECORE_X_EVENT_MODE_GRAB",
	     "ECORE_X_EVENT_MODE_UNGRAB"
	};
	const char *details[] = {
	   "ECORE_X_EVENT_DETAIL_ANCESTOR",
	     "ECORE_X_EVENT_DETAIL_VIRTUAL",
	     "ECORE_X_EVENT_DETAIL_INFERIOR",
	     "ECORE_X_EVENT_DETAIL_NON_LINEAR",
	     "ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL",
	     "ECORE_X_EVENT_DETAIL_POINTER",
	     "ECORE_X_EVENT_DETAIL_POINTER_ROOT",
	     "ECORE_X_EVENT_DETAIL_DETAIL_NONE"
	};
	
	printf("OUT 0x%x [%s] md=%s dt=%s\n",
	       ev->win,
	       bd->client.icccm.title,
	       modes[ev->mode],
	       details[ev->detail]);
	
	if (ev->mode != ECORE_X_EVENT_MODE_GRAB)
	  e_border_focus_set(bd, 0, 1);
	else
	  {
	     printf("OUT GRAB!\n");
	  }
     }
   if (ev->win != bd->event_win) return 1;
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
   evas_event_feed_mouse_out(bd->bg_evas);
   return 1;
}

static int
_e_border_cb_mouse_down(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Button_Down *ev;
   E_Border *bd;
   
   ev = event;
   bd = data;
   if (ev->win != bd->event_win) return 1;
   if ((ev->button >= 1) && (ev->button <= 3))
     {
	bd->mouse.last_down[ev->button - 1].mx = ev->root.x;
	bd->mouse.last_down[ev->button - 1].my = ev->root.y;
	bd->mouse.last_down[ev->button - 1].x = bd->x;
	bd->mouse.last_down[ev->button - 1].y = bd->y;
	bd->mouse.last_down[ev->button - 1].w = bd->w;
	bd->mouse.last_down[ev->button - 1].h = bd->h;
     }
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   if (bd->moving)
     {
     }
   else
     {
	evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
	evas_event_feed_mouse_down(bd->bg_evas, ev->button);
     }
   return 1;
}

static int
_e_border_cb_mouse_up(void *data, int type, void *event)
{  
   Ecore_X_Event_Mouse_Button_Up *ev;
   E_Border *bd;
   
   ev = event;
   bd = data;
   if (ev->win != bd->event_win) return 1;
   if ((ev->button >= 1) && (ev->button <= 3))
     {
	bd->mouse.last_up[ev->button - 1].mx = ev->root.x;
	bd->mouse.last_up[ev->button - 1].my = ev->root.y;
	bd->mouse.last_up[ev->button - 1].x = bd->x;
	bd->mouse.last_up[ev->button - 1].y = bd->y;
     }
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
   evas_event_feed_mouse_up(bd->bg_evas, ev->button);
   return 1;
}

static int
_e_border_cb_mouse_move(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Move *ev;
   E_Border *bd;
   
   ev = event;
   bd = data;
   if (ev->win != bd->event_win) return 1;
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   if (bd->moving)
     {
	int x, y, new_x, new_y;
	Evas_List *skiplist = NULL;
	
	if ((bd->moveinfo.down.button >= 1) && (bd->moveinfo.down.button <= 3))
	  {
	     x = bd->mouse.last_down[bd->moveinfo.down.button - 1].x +
	       (bd->mouse.current.mx - bd->moveinfo.down.mx);
	     y = bd->mouse.last_down[bd->moveinfo.down.button - 1].y +
	       (bd->mouse.current.my - bd->moveinfo.down.my);
	  }
	else
	  {
	     x = bd->x +
	       (bd->mouse.current.mx - bd->moveinfo.down.mx);
	     y = bd->y +
	       (bd->mouse.current.my - bd->moveinfo.down.my);
	  }
	new_x = x;
	new_y = y;
	skiplist = evas_list_append(skiplist, bd);
	e_resist_container_position(bd->container, skiplist,
				    bd->x, bd->y, bd->w, bd->h,
				    x, y, bd->w, bd->h,
				    &new_x, &new_y);
	evas_list_free(skiplist);
	e_border_move(bd, new_x, new_y);
     }
   else if (bd->resize_mode != RESIZE_NONE)
     {
	_e_border_resize_handle(bd);
     }
   else
     {
	evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
     }
   return 1;
}

static int
_e_border_cb_mouse_wheel(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Wheel *ev;
   E_Border *bd;
   
   ev = event;
   bd = data;
   if (ev->win != bd->event_win) return 1;
   bd->mouse.current.mx = ev->root.x;
   bd->mouse.current.my = ev->root.y;
   evas_event_feed_mouse_move(bd->bg_evas, ev->x, ev->y);
   evas_event_feed_mouse_wheel(bd->bg_evas, ev->direction, ev->z);
   return 1;
}

static void
_e_border_eval(E_Border *bd)
{
   /* FIXME: get min/max/start size etc. gravity etc. */
   
   /* fetch any info queued to be fetched */
   if (bd->client.icccm.fetch.title)
     {
	if (bd->client.icccm.title) free(bd->client.icccm.title);
	bd->client.icccm.title = ecore_x_icccm_title_get(bd->client.win);
	bd->client.icccm.fetch.title = 0;
	if (bd->bg_object)
	  {
	     edje_object_part_text_set(bd->bg_object, "title_text", 
				       bd->client.icccm.title);
	     printf("SET TITLE %s\n", bd->client.icccm.title);
	  }
     }
   if (bd->client.icccm.fetch.name_class)
     {
	if (bd->client.icccm.name) free(bd->client.icccm.name);
	bd->client.icccm.name = NULL;
	if (bd->client.icccm.class) free(bd->client.icccm.class);
	bd->client.icccm.class = NULL;
	ecore_x_window_prop_name_class_get(bd->client.win, &bd->client.icccm.name, &bd->client.icccm.class);
	bd->client.icccm.fetch.name_class = 0;
     }
   if (bd->client.icccm.fetch.icon_name)
     {
	if (bd->client.icccm.icon_name) free(bd->client.icccm.icon_name);
	bd->client.icccm.icon_name = ecore_x_window_prop_icon_name_get(bd->client.win);
	bd->client.icccm.fetch.icon_name = 0;
     }
   if (bd->client.icccm.fetch.machine)
     {
	if (bd->client.icccm.machine) free(bd->client.icccm.machine);
	bd->client.icccm.machine = ecore_x_window_prop_client_machine_get(bd->client.win);
	bd->client.icccm.fetch.machine = 0;
     }
   if (bd->client.icccm.fetch.hints)
     {
	int accepts_focus = 1;
	int is_urgent = 0;
	
	bd->client.icccm.initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
	ecore_x_icccm_hints_get(bd->client.win,
				&accepts_focus,
				&bd->client.icccm.initial_state,
				&bd->client.icccm.icon_pixmap,
				&bd->client.icccm.icon_mask,
				&bd->client.icccm.icon_window,
				&bd->client.icccm.window_group,
				&is_urgent);
	bd->client.icccm.accepts_focus = accepts_focus;
	bd->client.icccm.urgent = is_urgent;
	bd->client.icccm.fetch.hints = 0;
     }
   if (bd->client.icccm.fetch.size_pos_hints)
     {
	int request_pos = 0;
	
	if (!ecore_x_icccm_size_pos_hints_get(bd->client.win,
					      &request_pos,
					      &bd->client.icccm.gravity,
					      &bd->client.icccm.min_w,
					      &bd->client.icccm.min_h,
					      &bd->client.icccm.max_w,
					      &bd->client.icccm.max_h,
					      &bd->client.icccm.base_w,
					      &bd->client.icccm.base_h,
					      &bd->client.icccm.step_w,
					      &bd->client.icccm.step_h,
					      &bd->client.icccm.min_aspect,
					      &bd->client.icccm.max_aspect))
	  {
	     printf("NO SIZE HINTS!\n");
	  }
	if (bd->client.icccm.min_w > 32767) bd->client.icccm.min_w = 32767;
	if (bd->client.icccm.min_h > 32767) bd->client.icccm.min_h = 32767;
	if (bd->client.icccm.max_w > 32767) bd->client.icccm.max_w = 32767;
	if (bd->client.icccm.max_h > 32767) bd->client.icccm.max_h = 32767;
	if (bd->client.icccm.base_w > 32767) bd->client.icccm.base_w = 32767;
	if (bd->client.icccm.base_h > 32767) bd->client.icccm.base_h = 32767;
	bd->client.icccm.request_pos = request_pos;
	bd->client.icccm.fetch.size_pos_hints = 0;
     }
   if (bd->client.icccm.fetch.protocol)
     {
	int i, num;
	Ecore_X_WM_Protocol *proto;
	
	proto = ecore_x_window_prop_protocol_list_get(bd->client.win, &num);
	if (proto)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (proto[i] == ECORE_X_WM_PROTOCOL_DELETE_REQUEST)
		    bd->client.icccm.delete_request = 1;
		  if (proto[i] == ECORE_X_WM_PROTOCOL_TAKE_FOCUS)
		    bd->client.icccm.take_focus = 1;
	       }
	     free(proto);
	  }
	bd->client.icccm.fetch.protocol = 0;
     }
   if (bd->client.mwm.fetch.hints)
     {
	int pb;
	
	bd->client.mwm.exists = 
	  ecore_x_mwm_hints_get(bd->client.win,
				&bd->client.mwm.func,
				&bd->client.mwm.decor,
				&bd->client.mwm.input);
	pb = bd->client.mwm.borderless;
	bd->client.mwm.borderless = 0;
	if (bd->client.mwm.exists)
	  {
	     printf("MWM hints!\n");
	     if ((!(bd->client.mwm.decor & ECORE_X_MWM_HINT_DECOR_ALL)) && 
		 (!(bd->client.mwm.decor & ECORE_X_MWM_HINT_DECOR_TITLE)) &&
		 (!(bd->client.mwm.decor & ECORE_X_MWM_HINT_DECOR_BORDER)))
	       bd->client.mwm.borderless = 1;
	  }
	if (bd->client.mwm.borderless != pb)
	  {
	     if (bd->client.border.name) free(bd->client.border.name);
	     if (bd->client.mwm.borderless)
	       bd->client.border.name = strdup("borderless");
	     else
	       bd->client.border.name = strdup("default");
	     if (bd->client.mwm.borderless)
	       printf("client %s borderless\n", bd->client.icccm.title);
	     bd->client.border.changed = 1;
	  }
	bd->client.mwm.fetch.hints = 0;
     }
   if (bd->client.netwm.fetch.pid)
     {
	bd->client.netwm.pid = ecore_x_window_prop_pid_get(bd->client.win);
	bd->client.netwm.fetch.pid = 0;
     }
   if (bd->client.netwm.fetch.desktop)
     {
	bd->client.netwm.desktop = ecore_x_window_prop_desktop_get(bd->client.win);
	bd->client.netwm.fetch.desktop = 0;
     }
   
   if (bd->client.border.changed)
     {
	Evas_Object *o;
	int iw, ih;
	const char *path, *str;
	char buf[4096];

	if (!bd->client.border.name)
	  {
	     bd->client.border.name = strdup("default");
	  }
	if (bd->bg_object)
	  {
	     bd->w -= (bd->client_inset.l + bd->client_inset.r);
	     bd->h -= (bd->client_inset.t + bd->client_inset.b);
	     bd->client_inset.l = 0;
	     bd->client_inset.r = 0;
	     bd->client_inset.t = 0;
	     bd->client_inset.b = 0;
	     bd->changes.size = 1;
	     evas_object_del(bd->bg_object);
	  }
        o = edje_object_add(bd->bg_evas);
	bd->bg_object = o;
	/* FIXME: "default.eet" needs to come from conf */
	path = e_path_find(path_themes, "default.eet");
	snprintf(buf, sizeof(buf), "widgets/border/%s/border", 
		 bd->client.border.name);
        edje_object_file_set(o, path, buf);
	edje_object_part_text_set(o, "title_text", 
				  bd->client.icccm.title);
	printf("SET TITLE2 %s\n", bd->client.icccm.title);
	str = edje_object_data_get(o, "client_inset");
	if (str)
	  {
	     int l, r, t, b;
	     
	     if (sscanf(str, "%i %i %i %i", &l, &r, &t, &b) == 4)
	       {
		  bd->client_inset.l = l;
		  bd->client_inset.r = r;
		  bd->client_inset.t = t;
		  bd->client_inset.b = b;
		  bd->w += (bd->client_inset.l + bd->client_inset.r);
		  bd->h += (bd->client_inset.t + bd->client_inset.b);
		  bd->changes.size = 1;
		  ecore_x_window_move(bd->client.shell_win, l, t);
	       }
	  }
	edje_object_signal_callback_add(o, "move_start", "*",
					_e_border_cb_signal_move_start, bd);
	edje_object_signal_callback_add(o, "move_stop", "*",
					_e_border_cb_signal_move_stop, bd);
	edje_object_signal_callback_add(o, "resize_tl_start", "*",
					_e_border_cb_signal_resize_tl_start, bd);
	edje_object_signal_callback_add(o, "resize_t_start", "*",
					_e_border_cb_signal_resize_t_start, bd);
	edje_object_signal_callback_add(o, "resize_tr_start", "*",
					_e_border_cb_signal_resize_tr_start, bd);
	edje_object_signal_callback_add(o, "resize_r_start", "*",
					_e_border_cb_signal_resize_r_start, bd);
	edje_object_signal_callback_add(o, "resize_br_start", "*",
					_e_border_cb_signal_resize_br_start, bd);
	edje_object_signal_callback_add(o, "resize_b_start", "*",
					_e_border_cb_signal_resize_b_start, bd);
	edje_object_signal_callback_add(o, "resize_bl_start", "*",
					_e_border_cb_signal_resize_bl_start, bd);
	edje_object_signal_callback_add(o, "resize_l_start", "*",
					_e_border_cb_signal_resize_l_start, bd);
	edje_object_signal_callback_add(o, "resize_stop", "*",
					_e_border_cb_signal_resize_stop, bd);
	edje_object_signal_callback_add(o, "action", "*",
					_e_border_cb_signal_action, bd);
	if (bd->focused)
	  edje_object_signal_emit(bd->bg_object, "active", "");
	evas_object_move(o, 0, 0);
	evas_object_resize(o, bd->w, bd->h);
	evas_object_show(o);
	bd->client.border.changed = 0;
     }

   if (bd->new_client)
     {
	printf("NEW CLIENT SETUP\n");
	if (bd->re_manage)
	  {
	     printf("REMANAGE!\n");
	     bd->x -= bd->client_inset.l;
	     bd->y -= bd->client_inset.t;
	     bd->changes.pos = 1;
	  }
	else
	  {
	     if (bd->client.icccm.request_pos)
	       {
		  Ecore_X_Window_Attributes *att;
		  int bw;
		  
		  printf("REQUEST POS!\n");
		  att = &bd->client.initial_attributes;
		  bw = att->border * 2;
		  switch (bd->client.icccm.gravity)
		    {
		     case ECORE_X_GRAVITY_N:
		       bd->x = (att->x - (bw / 2));
		       bd->y = att->y;
		       break;
		     case ECORE_X_GRAVITY_NE:
		       bd->x = (att->x - (bw)) - (bd->client_inset.l);
		       bd->y = att->y;
		       break;
		     case ECORE_X_GRAVITY_E:
		       bd->x = (att->x - (bw)) - (bd->client_inset.l);
		       bd->y = (att->y - (bw / 2));
		       break;
		     case ECORE_X_GRAVITY_SE:
		       bd->x = (att->x - (bw)) - (bd->client_inset.l);
		       bd->y = (att->y - (bw)) - (bd->client_inset.t);
		       break;
		     case ECORE_X_GRAVITY_S:
		       bd->x = (att->x - (bw / 2));
		       bd->y = (att->y - (bw)) - (bd->client_inset.t);
		       break;
		     case ECORE_X_GRAVITY_SW:
		       bd->x = att->x;
		       bd->y = (att->y - (bw)) - (bd->client_inset.t);
		       break;
		     case ECORE_X_GRAVITY_W:
		       bd->x = att->x;
		       bd->y = (att->y - (bw)) - (bd->client_inset.t);
		       break;
		     case ECORE_X_GRAVITY_NW:
		     default:
		       bd->x = att->x;
		       bd->y = att->y;
		    }
		  bd->changes.pos = 1;
	       }
	     else
	       {
		  Evas_List *skiplist = NULL;
		  int new_x, new_y;
		  
		  printf("AUTO POS!\n");
		  new_x = rand() % (bd->container->w - bd->w);
		  new_y = rand() % (bd->container->h - bd->h);
		  skiplist = evas_list_append(skiplist, bd);
		  e_place_container_region_smart(bd->container, skiplist,
						 bd->x, bd->y, bd->w, bd->h,
						 &new_x, &new_y);
		  evas_list_free(skiplist);
		  bd->x = new_x;
		  bd->y = new_y;
		  bd->changes.pos = 1;
	       }
	  }
     }
   
   /* effect changes to the window border itself */
   if (bd->changes.visible)
     {
	if (bd->visible) ecore_x_window_show(bd->win);
	else ecore_x_window_hide(bd->win);
	bd->changes.visible = 0;
     }
   if ((bd->changes.pos) && (bd->changes.size))
     {
	printf("border move resize\n");
	evas_obscured_clear(bd->bg_evas);
	evas_obscured_rectangle_add(bd->bg_evas,
				    bd->client_inset.l, bd->client_inset.t,
				    bd->w - (bd->client_inset.l + bd->client_inset.r),
				    bd->h - (bd->client_inset.t + bd->client_inset.b));
	ecore_x_window_move_resize(bd->win, bd->x, bd->y, bd->w, bd->h);
	ecore_x_window_move_resize(bd->event_win, 0, 0, bd->w, bd->h);
	ecore_x_window_move_resize(bd->client.shell_win, 
				   bd->client_inset.l, bd->client_inset.t,
				   bd->client.w, bd->client.h);
	ecore_x_window_move_resize(bd->client.win, 0, 0,
				   bd->client.w, bd->client.h);
	ecore_evas_move_resize(bd->bg_ecore_evas, 0, 0, bd->w, bd->h);
	evas_object_resize(bd->bg_object, bd->w, bd->h);
	e_container_shape_resize(bd->shape, bd->w, bd->h);
	e_container_shape_move(bd->shape, bd->x, bd->y);
	bd->changes.pos = 0;
	bd->changes.size = 0;
	printf("border move resize done\n");
    }
   else if (bd->changes.pos)
     {
	ecore_x_window_move(bd->win, bd->x, bd->y);
	e_container_shape_move(bd->shape, bd->x, bd->y);
	bd->changes.pos = 0;
     }
   else if (bd->changes.size)
     {
	printf("border move resize\n");
	evas_obscured_clear(bd->bg_evas);
	evas_obscured_rectangle_add(bd->bg_evas,
				    bd->client_inset.l, bd->client_inset.t,
				    bd->w - (bd->client_inset.l + bd->client_inset.r), bd->h - (bd->client_inset.t + bd->client_inset.b));
	ecore_x_window_move_resize(bd->event_win, 0, 0, bd->w, bd->h);
	ecore_x_window_resize(bd->win, bd->w, bd->h);
	ecore_x_window_move_resize(bd->client.shell_win, 
				   bd->client_inset.l, bd->client_inset.t,
				   bd->client.w, bd->client.h);
	ecore_x_window_move_resize(bd->client.win, 0, 0,
				   bd->client.w, bd->client.h);
	ecore_evas_move_resize(bd->bg_ecore_evas, 0, 0, bd->w, bd->h);
	evas_object_resize(bd->bg_object, bd->w, bd->h);
	e_container_shape_resize(bd->shape, bd->w, bd->h);
	printf("border move resize done\n");
	bd->changes.size = 0;
     }
   if (bd->changes.reset_gravity)
     {
	GRAV_SET(bd, ECORE_X_GRAVITY_NW);
	bd->changes.reset_gravity = 0;
     }
   
   bd->new_client = 0;
   bd->changed = 0;
   
   bd->changes.stack = 0;
   bd->changes.prop = 0;
   bd->changes.border = 0;
}

static void
_e_border_resize_limit(E_Border *bd, int *w, int *h)
{
   double a;
   
   *w -= bd->client_inset.l + bd->client_inset.r;
   *h -= bd->client_inset.t + bd->client_inset.b;
   if (*h < 1) *h = 1;
   if (*w < 1) *w = 1;
   a = (double)*w / (double)*h;
   if ((bd->client.icccm.min_aspect != 0.0) && 
       (a < bd->client.icccm.min_aspect))
     *w = *h * bd->client.icccm.min_aspect;
   else if
     ((bd->client.icccm.max_aspect != 0.0) &&
      (a > bd->client.icccm.max_aspect))
     *h = *w / bd->client.icccm.max_aspect;
   *w = bd->client.icccm.base_w + 
     (((*w - bd->client.icccm.base_w) / bd->client.icccm.step_w) *
      bd->client.icccm.step_w);
   *h = bd->client.icccm.base_h + 
     (((*h - bd->client.icccm.base_h) / bd->client.icccm.step_h) *
      bd->client.icccm.step_h);
   
   if (*h < 1) *h = 1;
   if (*w < 1) *w = 1;
   
   if      (*w > bd->client.icccm.max_w) *w = bd->client.icccm.max_w;
   else if (*w < bd->client.icccm.min_w) *w = bd->client.icccm.min_w;
   if      (*h > bd->client.icccm.max_h) *h = bd->client.icccm.max_h;
   else if (*h < bd->client.icccm.min_h) *h = bd->client.icccm.min_h;
   
   *w += bd->client_inset.l + bd->client_inset.r;
   *h += bd->client_inset.t + bd->client_inset.b;
}

static void
_e_border_moveinfo_gather(E_Border *bd, const char *source)
{
   if (!strcmp(source, "mouse,1")) bd->moveinfo.down.button = 1;
   else if (!strcmp(source, "mouse,2")) bd->moveinfo.down.button = 2;
   else if (!strcmp(source, "mouse,3")) bd->moveinfo.down.button = 3;
   else bd->moveinfo.down.button = 0;
   if ((bd->moveinfo.down.button >= 1) && (bd->moveinfo.down.button <= 3))
     {
	bd->moveinfo.down.mx = bd->mouse.last_down[bd->moveinfo.down.button - 1].mx;
	bd->moveinfo.down.my = bd->mouse.last_down[bd->moveinfo.down.button - 1].my;
     }
   else
     {
	bd->moveinfo.down.mx = bd->mouse.current.mx;
	bd->moveinfo.down.my = bd->mouse.current.my;
     }
}

static void
_e_border_resize_handle(E_Border *bd)
{
   int x, y, w, h;
   int tw, th;
   
   x = bd->x;
   y = bd->y;
   w = bd->w;
   h = bd->h;
   
   if ((bd->resize_mode == RESIZE_TR) ||
       (bd->resize_mode == RESIZE_R) ||
       (bd->resize_mode == RESIZE_BR))
     {
	if ((bd->moveinfo.down.button >= 1) &&
	    (bd->moveinfo.down.button <= 3))
	  w = bd->mouse.last_down[bd->moveinfo.down.button - 1].w +
	  (bd->mouse.current.mx - bd->moveinfo.down.mx);
	else
	  w = bd->w + (bd->mouse.current.mx - bd->moveinfo.down.mx);
     }
   else if ((bd->resize_mode == RESIZE_TL) ||
	    (bd->resize_mode == RESIZE_L) ||
	    (bd->resize_mode == RESIZE_BL))
     {
	if ((bd->moveinfo.down.button >= 1) &&
	    (bd->moveinfo.down.button <= 3))
	  w = bd->mouse.last_down[bd->moveinfo.down.button - 1].w -
	  (bd->mouse.current.mx - bd->moveinfo.down.mx);
	else
	  w = bd->w - (bd->mouse.current.mx - bd->moveinfo.down.mx);
     }
   
   if ((bd->resize_mode == RESIZE_TL) ||
       (bd->resize_mode == RESIZE_T) ||
       (bd->resize_mode == RESIZE_TR))
     {
	if ((bd->moveinfo.down.button >= 1) &&
	    (bd->moveinfo.down.button <= 3))
	  h = bd->mouse.last_down[bd->moveinfo.down.button - 1].h -
	  (bd->mouse.current.my - bd->moveinfo.down.my);
	else
	  h = bd->h - (bd->mouse.current.my - bd->moveinfo.down.my);
     }
   else if ((bd->resize_mode == RESIZE_BL) ||
	    (bd->resize_mode == RESIZE_B) ||
	    (bd->resize_mode == RESIZE_BR))
     {
	if ((bd->moveinfo.down.button >= 1) &&
	    (bd->moveinfo.down.button <= 3))
	  h = bd->mouse.last_down[bd->moveinfo.down.button - 1].h +
	  (bd->mouse.current.my - bd->moveinfo.down.my);
	else
	  h = bd->h + (bd->mouse.current.my - bd->moveinfo.down.my);
     }

   tw = bd->w;
   th = bd->h;
   _e_border_resize_limit(bd, &w, &h);
   
   if ((bd->resize_mode == RESIZE_TL) ||
       (bd->resize_mode == RESIZE_L) ||
       (bd->resize_mode == RESIZE_BL))
     x += (tw - w);
   if ((bd->resize_mode == RESIZE_TL) ||
       (bd->resize_mode == RESIZE_T) ||
       (bd->resize_mode == RESIZE_TR))
     y += (th - h);
	  
   e_border_move_resize(bd, x, y, w, h);
}
