/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* FIXME: corner case if module is sized to full screen... cant stop edit or */
/*        re-enter edit mode (cant access root menu) */

/* FIXME: handle edge move/changes */
/* FIXME: handle drag from zone to zone */
/* FIXME: handle save */
/* FIXME: handle load */
/* FIXME: handle move resist */
/* FIXME: handle resize resist */

/* local subsystem functions */
static void _e_gadman_free(E_Gadman *gm);
static void _e_gadman_client_free(E_Gadman_Client *gmc);
static void _e_gadman_client_edit_begin(E_Gadman_Client *gmc);
static void _e_gadman_client_edit_end(E_Gadman_Client *gmc);
static void _e_gadman_client_down_store(E_Gadman_Client *gmc);
static int  _e_gadman_client_is_being_modified(E_Gadman_Client *gmc);
static void _e_gadman_client_geometry_to_align(E_Gadman_Client *gmc);
static void _e_gadman_client_aspect_enforce(E_Gadman_Client *gmc, double cx, double cy, int use_horiz);
static void _e_gadman_client_geometry_apply(E_Gadman_Client *gmc);

static void _e_gadman_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_gadman_cb_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_gadman_cb_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_gadman_cb_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);

static void _e_gadman_cb_signal_move_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_move_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_move_go(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_left_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_left_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_left_go(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_right_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_right_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_right_go(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_up_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_up_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_up_go(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_down_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_down_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_gadman_cb_signal_resize_down_go(void *data, Evas_Object *obj, const char *emission, const char *source);

static void _e_gadman_cb_menu_end(void *data, E_Menu *m);

static void _e_gadman_cb_half_width(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_full_width(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_auto_width(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_center_horiz(void *data, E_Menu *m, E_Menu_Item *mi);

static void _e_gadman_cb_half_height(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_full_height(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_auto_height(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_gadman_cb_center_vert(void *data, E_Menu *m, E_Menu_Item *mi);

/* externally accessible functions */
int
e_gadman_init(void)
{
   return 1;
}

int
e_gadman_shutdown(void)
{
   return 1;
}

E_Gadman *
e_gadman_new(E_Container *con)
{
   E_Gadman    *gm;

   gm = E_OBJECT_ALLOC(E_Gadman, _e_gadman_free);
   if (!gm) return NULL;
   gm->container = con;
   return gm;
}

void
e_gadman_mode_set(E_Gadman *gm, E_Gadman_Mode mode)
{
   Evas_List *l;
   
   E_OBJECT_CHECK(gm);
   if (gm->mode == mode) return;
   gm->mode = mode;
   if (gm->mode == E_GADMAN_MODE_EDIT)
     {
	for (l = gm->clients; l; l = l->next)
	  _e_gadman_client_edit_begin(l->data);
     }
   else if (gm->mode == E_GADMAN_MODE_NORMAL)
     {
	for (l = gm->clients; l; l = l->next)
	  _e_gadman_client_edit_end(l->data);
     }
}

E_Gadman_Mode
e_gadman_mode_get(E_Gadman *gm)
{
   E_OBJECT_CHECK_RETURN(gm, E_GADMAN_MODE_NORMAL);
   return gm->mode;
}

E_Gadman_Client *
e_gadman_client_new(E_Gadman *gm)
{
   E_Gadman_Client *gmc;
   E_OBJECT_CHECK_RETURN(gm, NULL);
   
   gmc = E_OBJECT_ALLOC(E_Gadman_Client, _e_gadman_client_free);
   if (!gmc) return NULL;
   gmc->gadman = gm;
   gmc->policy = E_GADMAN_POLICY_ANYWHERE | E_GADMAN_POLICY_HSIZE | E_GADMAN_POLICY_VSIZE | E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE;
   gmc->zone = e_zone_current_get(gm->container);
   gmc->edge = E_GADMAN_EDGE_BOTTOM;
   gmc->minw = 1;
   gmc->minh = 1;
   gmc->maxw = 0;
   gmc->maxh = 0;
   gmc->ax = 0.0;
   gmc->ay = 1.0;
   gmc->mina = 0.0;
   gmc->maxa = 9999999.0;
   gm->clients = evas_list_append(gm->clients, gmc);
   return gmc;
}

void
e_gadman_client_save(E_Gadman_Client *gmc)
{
   E_OBJECT_CHECK(gmc);
   /* save all values */
}

void
e_client_gadman_edge_set(E_Gadman_Client *gmc, E_Gadman_Edge edge)
{
   E_OBJECT_CHECK(gmc);
   gmc->edge = edge;
}

void
e_gadman_client_load(E_Gadman_Client *gmc)
{
   E_OBJECT_CHECK(gmc);
   /* load all the vales */
   /* implement all the values */
}

void
e_gadman_client_domain_set(E_Gadman_Client *gmc, char *domain, int instance)
{
   E_OBJECT_CHECK(gmc);
   if (gmc->domain) free(gmc->domain);
   gmc->domain = strdup(domain);
   gmc->instance = instance;
}

void
e_gadman_client_zone_set(E_Gadman_Client *gmc, E_Zone *zone)
{
   E_OBJECT_CHECK(gmc);
   gmc->zone = zone;
}

void
e_gadman_client_policy_set(E_Gadman_Client *gmc, E_Gadman_Policy pol)
{
   E_OBJECT_CHECK(gmc);
   gmc->policy = pol;
}

void
e_gadman_client_min_size_set(E_Gadman_Client *gmc, Evas_Coord minw, Evas_Coord minh)
{
   E_OBJECT_CHECK(gmc);
   gmc->minw = minw;
   gmc->minh = minh;
}

void
e_gadman_client_max_size_set(E_Gadman_Client *gmc, Evas_Coord maxw, Evas_Coord maxh)
{
   E_OBJECT_CHECK(gmc);
   gmc->maxw = maxw;
   gmc->maxh = maxh;
}

void
e_gadman_client_align_set(E_Gadman_Client *gmc, double xalign, double yalign)
{
   E_OBJECT_CHECK(gmc);
   gmc->ax = xalign;
   gmc->ay = yalign;
}

void
e_gadman_client_aspect_set(E_Gadman_Client *gmc, double mina, double maxa)
{
   E_OBJECT_CHECK(gmc);
   gmc->mina = mina;
   gmc->maxa = maxa;
}

void
e_gadman_client_auto_size_set(E_Gadman_Client *gmc, Evas_Coord autow, Evas_Coord autoh)
{
   E_OBJECT_CHECK(gmc);
   gmc->autow = autow;
   gmc->autoh = autoh;
}

void
e_gadman_client_geometry_get(E_Gadman_Client *gmc, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   E_OBJECT_CHECK(gmc);
   if (x) *x = gmc->x;
   if (y) *y = gmc->y;
   if (w) *w = gmc->w;
   if (h) *h = gmc->h;
}

void
e_gadman_client_change_func_set(E_Gadman_Client *gmc, void (*func) (void *data, E_Gadman_Client *gmc, E_Gadman_Change change), void *data)
{
   E_OBJECT_CHECK(gmc);
   gmc->func = func;
   gmc->data = data;
}

E_Menu *
e_gadman_client_menu_new(E_Gadman_Client *gmc)
{
   E_Menu *m;
   E_Menu_Item *mi;
   
   E_OBJECT_CHECK_RETURN(gmc, NULL);
   m = e_menu_new();
   
   gmc->menu = m;

   if (gmc->policy & E_GADMAN_POLICY_HSIZE)
     {
	if (gmc->autow > 0)
	  {
	     mi = e_menu_item_new(m);
	     e_menu_item_label_set(mi, "Automatic Width");
	     e_menu_item_check_set(mi, 1);
	     e_menu_item_toggle_set(mi, gmc->use_autow);
	     e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				       "auto_width");
	     e_menu_item_callback_set(mi, _e_gadman_cb_auto_width, gmc);
	     mi = e_menu_item_new(m);
	     e_menu_item_separator_set(mi, 1);
	  }
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Half Screen Width");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "half_width");
	e_menu_item_callback_set(mi, _e_gadman_cb_half_width, gmc);
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Full Screen Width");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "full_width");
	e_menu_item_callback_set(mi, _e_gadman_cb_full_width, gmc);
     }
   if (gmc->policy & E_GADMAN_POLICY_HMOVE)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Center Horizontally");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "center_horiz");
	e_menu_item_callback_set(mi, _e_gadman_cb_center_horiz, gmc);
     }
   if (((gmc->policy & E_GADMAN_POLICY_HSIZE) ||
	(gmc->policy & E_GADMAN_POLICY_HMOVE)) &&
       ((gmc->policy & E_GADMAN_POLICY_VSIZE) ||
	(gmc->policy & E_GADMAN_POLICY_VMOVE)))
     {
	mi = e_menu_item_new(m);
	e_menu_item_separator_set(mi, 1);
     }
   if (gmc->policy & E_GADMAN_POLICY_VSIZE)
     {
	if (gmc->autoh > 0)
	  {
	     mi = e_menu_item_new(m);
	     e_menu_item_label_set(mi, "Automatic Height");
	     e_menu_item_check_set(mi, 1);
	     e_menu_item_toggle_set(mi, gmc->use_autoh);
	     e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				       "auto_eight");
	     e_menu_item_callback_set(mi, _e_gadman_cb_auto_height, gmc);
	     mi = e_menu_item_new(m);
	     e_menu_item_separator_set(mi, 1);
	  }
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Half Screen Height");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "half_height");
	e_menu_item_callback_set(mi, _e_gadman_cb_half_height, gmc);
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Full Screen Height");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "full_height");
	e_menu_item_callback_set(mi, _e_gadman_cb_full_height, gmc);
     }
   if (gmc->policy & E_GADMAN_POLICY_VMOVE)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Center Vertically");
	e_menu_item_icon_edje_set(mi, e_path_find(path_icons, "default.eet"),
				  "center_vert");
	e_menu_item_callback_set(mi, _e_gadman_cb_center_vert, gmc);
     }
   
   return m;
}

/* local subsystem functions */
static void
_e_gadman_free(E_Gadman *gm)
{
   free(gm);
}

static void
_e_gadman_client_free(E_Gadman_Client *gmc)
{
   if (gmc->menu) e_object_del(E_OBJECT(gmc->menu));
   if (gmc->control_object) evas_object_del(gmc->control_object);
   if (gmc->event_object) evas_object_del(gmc->event_object);
   gmc->gadman->clients = evas_list_remove(gmc->gadman->clients, gmc);
   if (gmc->domain) free(gmc->domain);
   free(gmc);
}

static void
_e_gadman_client_edit_begin(E_Gadman_Client *gmc)
{
   gmc->control_object = edje_object_add(gmc->gadman->container->bg_evas);
   evas_object_layer_set(gmc->control_object, 100);
   evas_object_move(gmc->control_object, gmc->x, gmc->y);
   evas_object_resize(gmc->control_object, gmc->w, gmc->h);
   edje_object_file_set(gmc->control_object,
			/* FIXME: "default.eet" needs to come from conf */
			e_path_find(path_themes, "default.eet"),
			"gadman/control");
   edje_object_signal_callback_add(gmc->control_object, "move_start", "",
				   _e_gadman_cb_signal_move_start, gmc);
   edje_object_signal_callback_add(gmc->control_object, "move_stop", "",
				   _e_gadman_cb_signal_move_stop, gmc);
   edje_object_signal_callback_add(gmc->control_object, "move_go", "",
				   _e_gadman_cb_signal_move_go, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_start", "left",
				   _e_gadman_cb_signal_resize_left_start, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_stop", "left",
				   _e_gadman_cb_signal_resize_left_stop, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_go", "left",
				   _e_gadman_cb_signal_resize_left_go, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_start", "right",
				   _e_gadman_cb_signal_resize_right_start, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_stop", "right",
				   _e_gadman_cb_signal_resize_right_stop, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_go", "right",
				   _e_gadman_cb_signal_resize_right_go, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_start", "up",
				   _e_gadman_cb_signal_resize_up_start, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_stop", "up",
				   _e_gadman_cb_signal_resize_up_stop, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_go", "up",
				   _e_gadman_cb_signal_resize_up_go, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_start", "down",
				   _e_gadman_cb_signal_resize_down_start, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_stop", "down",
				   _e_gadman_cb_signal_resize_down_stop, gmc);
   edje_object_signal_callback_add(gmc->control_object, "resize_go", "down",
				   _e_gadman_cb_signal_resize_down_go, gmc);
   gmc->event_object = evas_object_rectangle_add(gmc->gadman->container->bg_evas);
   evas_object_color_set(gmc->event_object, 0, 0, 0, 0);
   evas_object_repeat_events_set(gmc->event_object, 1);
   evas_object_layer_set(gmc->event_object, 100);
   evas_object_move(gmc->event_object, gmc->x, gmc->y);
   evas_object_resize(gmc->event_object, gmc->w, gmc->h);
   evas_object_event_callback_add(gmc->event_object, EVAS_CALLBACK_MOUSE_DOWN, _e_gadman_cb_mouse_down, gmc);
   evas_object_event_callback_add(gmc->event_object, EVAS_CALLBACK_MOUSE_UP, _e_gadman_cb_mouse_up, gmc);
   evas_object_event_callback_add(gmc->event_object, EVAS_CALLBACK_MOUSE_IN, _e_gadman_cb_mouse_in, gmc);
   evas_object_event_callback_add(gmc->event_object, EVAS_CALLBACK_MOUSE_OUT, _e_gadman_cb_mouse_out, gmc);
   
   if (gmc->policy & E_GADMAN_POLICY_HSIZE)
     edje_object_signal_emit(gmc->control_object, "hsize", "on");
   else
     edje_object_signal_emit(gmc->control_object, "hsize", "off");
   if (gmc->policy & E_GADMAN_POLICY_VSIZE)
     edje_object_signal_emit(gmc->control_object, "vsize", "on");
   else
     edje_object_signal_emit(gmc->control_object, "vsize", "off");
   if (gmc->policy & (E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE))
     edje_object_signal_emit(gmc->control_object, "move", "on");
   else
     edje_object_signal_emit(gmc->control_object, "move", "off");
   
   evas_object_show(gmc->event_object);
   evas_object_show(gmc->control_object);
}

static void
_e_gadman_client_edit_end(E_Gadman_Client *gmc)
{
   evas_object_del(gmc->control_object);
   gmc->control_object = NULL;
   evas_object_del(gmc->event_object);
   gmc->event_object = NULL;
}

static void
_e_gadman_client_down_store(E_Gadman_Client *gmc)
{
   gmc->down_store_x = gmc->x;
   gmc->down_store_y = gmc->y;
   gmc->down_store_w = gmc->w;
   gmc->down_store_h = gmc->h;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &gmc->down_x, &gmc->down_y);
}

static int
_e_gadman_client_is_being_modified(E_Gadman_Client *gmc)
{
   if ((gmc->moving) || 
       (gmc->resizing_l) || (gmc->resizing_r) || 
       (gmc->resizing_u) || (gmc->resizing_d))
     return 1;
   return 0;
}

static void
_e_gadman_client_geometry_to_align(E_Gadman_Client *gmc)
{
   if (gmc->w != gmc->zone->w)
     gmc->ax = (double)gmc->x / (double)(gmc->zone->w - gmc->w);
   else
     gmc->ax = 0.0;
   if (gmc->h != gmc->zone->h)
     gmc->ay = (double)gmc->y / (double)(gmc->zone->h - gmc->h);
   else
     gmc->ay = 0.0;
}

static void
_e_gadman_client_aspect_enforce(E_Gadman_Client *gmc, double cx, double cy, int use_horiz)
{
   Evas_Coord neww, newh;
   double aspect;
   int change = 0;
   
   if (gmc->h > 0)
     aspect = (double)gmc->w / (double) gmc->h;
   else
     aspect = 0.0;
   neww = gmc->w;
   newh = gmc->h;
   if (aspect > gmc->maxa)
     {
	if (use_horiz)
	  newh = gmc->w / gmc->maxa;
	else
	  neww = gmc->h * gmc->mina;
	change = 1;
     }
   else if (aspect < gmc->mina)
     {
	if (use_horiz)
	  newh = gmc->w / gmc->maxa;
	else
	  neww = gmc->h * gmc->mina;
	change = 1;
     }
   if (change)
     {
	gmc->x = gmc->x + ((gmc->w - neww) * cx);
	gmc->y = gmc->y + ((gmc->h - newh) * cy);
	gmc->w = neww;
	gmc->h = newh;
     }
}

static void
_e_gadman_client_geometry_apply(E_Gadman_Client *gmc)
{
   evas_object_move(gmc->event_object, gmc->x, gmc->y);
   evas_object_resize(gmc->event_object, gmc->w, gmc->h);
   evas_object_move(gmc->control_object, gmc->x, gmc->y);
   evas_object_resize(gmc->control_object, gmc->w, gmc->h);
}

static void
_e_gadman_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Gadman_Client *gmc;
   
   gmc = data;
   ev = event_info;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   /* FIXME: how do we prevent this if you don't have a right mouse button */
   /*        maybe make this menu available in he modules menu? */
   if (ev->button == 3)
     {
	E_Menu *m;
	
	m = e_gadman_client_menu_new(gmc);
	if (m)
	  {
	     e_menu_post_deactivate_callback_set(m, _e_gadman_cb_menu_end, gmc);
	     e_menu_activate_mouse(m, gmc->zone, ev->output.x, ev->output.y, 1, 1,
				   E_MENU_POP_DIRECTION_DOWN);
	     e_util_container_fake_mouse_up_all_later(gmc->zone->container);
	  }
     }
}

static void
_e_gadman_cb_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   E_Gadman_Client *gmc;
   
   gmc = data;
   ev = event_info;
   if (_e_gadman_client_is_being_modified(gmc)) return;
}

static void
_e_gadman_cb_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   E_Gadman_Client *gmc;
   
   gmc = data;
   ev = event_info;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   edje_object_signal_emit(gmc->control_object, "active", "");
}

static void
_e_gadman_cb_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   E_Gadman_Client *gmc;
   
   gmc = data;
   ev = event_info;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   edje_object_signal_emit(gmc->control_object, "inactive", "");
}

static void
_e_gadman_cb_signal_move_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   _e_gadman_client_down_store(gmc);
   gmc->moving = 1;
}

static void
_e_gadman_cb_signal_move_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->moving = 0;
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_signal_move_go(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   Evas_Coord x, y;
   int new_edge = 0;
   int new_zone = 0;
   
   gmc = data;
   if (!gmc->moving) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   if ((gmc->policy & 0xff) == E_GADMAN_POLICY_EDGES)
     {
	double xr, yr;
	E_Gadman_Edge ne;
	
	ne = gmc->edge;
	xr = (double)(x - gmc->zone->x) / (double)gmc->zone->w;
	yr = (double)(y - gmc->zone->y) / (double)gmc->zone->h;
	
	if ((xr + yr) <= 1.0) /* top or left */
	  {
	     if (((1.0 - yr) + xr) <= 1.0) ne = E_GADMAN_EDGE_LEFT;
	     else ne = E_GADMAN_EDGE_TOP;
	  }
	else /* bottom or right */
	  {
	     if (((1.0 - yr) + xr) <= 1.0) ne = E_GADMAN_EDGE_BOTTOM;
	     else ne = E_GADMAN_EDGE_RIGHT;
	  }
	
	if (ne != gmc->edge)
	  {
	     gmc->edge = ne;
	     new_edge = 1;
	  }
	if (gmc->edge == E_GADMAN_EDGE_LEFT)
	  {
	     gmc->x = gmc->zone->x;
	     gmc->y = gmc->down_store_y + (y - gmc->down_y);
	     if (gmc->y < gmc->zone->y)
	       gmc->y = gmc->zone->y;
	     else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	       gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_RIGHT)
	  {
	     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	     gmc->y = gmc->down_store_y + (y - gmc->down_y);
	     if (gmc->y < gmc->zone->y)
	       gmc->y = gmc->zone->y;
	     else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	       gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_TOP)
	  {
	     gmc->x = gmc->down_store_x + (x - gmc->down_x);
	     gmc->y = gmc->zone->y;
	     if (gmc->x < gmc->zone->x)
	       gmc->x = gmc->zone->x;
	     else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	       gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_BOTTOM)
	  {
	     gmc->x = gmc->down_store_x + (x - gmc->down_x);
	     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	     if (gmc->x < gmc->zone->x)
	       gmc->x = gmc->zone->x;
	     else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	       gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	  }
     }
   else
     {
	if (gmc->policy & E_GADMAN_POLICY_HMOVE)
	  gmc->x = gmc->down_store_x + (x - gmc->down_x);
	else
	  gmc->x = gmc->down_store_x;
	if (gmc->policy & E_GADMAN_POLICY_VMOVE)
	  gmc->y = gmc->down_store_y + (y - gmc->down_y);
	else
	  gmc->y = gmc->down_store_y;
	gmc->w = gmc->down_store_w;
	gmc->h = gmc->down_store_h;
	if (gmc->x < gmc->zone->x)
	  gmc->x = gmc->zone->x;
	else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	  gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	if (gmc->y < gmc->zone->y)
	  gmc->y = gmc->zone->y;
	else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	  gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
     }
   _e_gadman_client_geometry_to_align(gmc);
   if (new_zone)
     {
	/* FIXME: callback for edge change */
     }
   if (new_edge)
     {
	/* FIXME: callback for edge change */
     }
   _e_gadman_client_geometry_apply(gmc);
}

static void
_e_gadman_cb_signal_resize_left_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   _e_gadman_client_down_store(gmc);
   gmc->use_autow = 0;
   gmc->resizing_l = 1;
}

static void
_e_gadman_cb_signal_resize_left_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->resizing_l = 0;
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_signal_resize_left_go(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   Evas_Coord x, y;
   
   gmc = data;
   if (!gmc->resizing_l) return;
   if (!(gmc->policy & E_GADMAN_POLICY_HSIZE)) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   gmc->x = gmc->down_store_x + (x - gmc->down_x);
   gmc->y = gmc->down_store_y;
   gmc->w = gmc->down_store_w - (x - gmc->down_x);
   gmc->h = gmc->down_store_h;
   /* limit to zone left edge */
   if (gmc->x < gmc->zone->x)
     {
	gmc->w = (gmc->down_store_x + gmc->down_store_w) - gmc->zone->x;
	gmc->x = gmc->zone->x;
     }
   /* limit to min size */
   if (gmc->w < gmc->minw)
     {
	gmc->x = (gmc->down_store_x + gmc->down_store_w) - gmc->minw;
	gmc->w = gmc->minw;
     }
   /* if we are atthe edge or beyond. assyme we want to be all the way there */
   if (x <= gmc->zone->x)
     {
	gmc->w = (gmc->x + gmc->w) - gmc->zone->x;
	gmc->x = gmc->zone->x;
     }
   /* limit to max size */
   if (gmc->maxw > 0)
     {
	if (gmc->w > gmc->minw)
	  {
	     gmc->x -= (gmc->maxw - gmc->w);
	     gmc->w = gmc->maxw;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 1.0, 0.5, 1);
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
}

static void
_e_gadman_cb_signal_resize_right_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   _e_gadman_client_down_store(gmc);
   gmc->use_autow = 0;
   gmc->resizing_r = 1;
}

static void
_e_gadman_cb_signal_resize_right_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->resizing_r = 0;
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_signal_resize_right_go(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   Evas_Coord x, y;
   
   gmc = data;
   if (!gmc->resizing_r) return;
   if (!(gmc->policy & E_GADMAN_POLICY_HSIZE)) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   gmc->x = gmc->down_store_x;
   gmc->y = gmc->down_store_y;
   gmc->w = gmc->down_store_w + (x - gmc->down_x);
   gmc->h = gmc->down_store_h;
   /* limit to zone right edge */
   if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     {
	gmc->w = (gmc->zone->x + gmc->zone->w) - gmc->x;
     }
   /* limit to min size */
   if (gmc->w < gmc->minw)
     {
	gmc->w = gmc->minw;
     }
   /* if we are atthe edge or beyond. assyme we want to be all the way there */
   if (x >= (gmc->zone->x + gmc->zone->w - 1))
     {
	gmc->w = gmc->zone->x + gmc->zone->w - gmc->x;
     }
   /* limit to max size */
   if (gmc->maxw > 0)
     {
	if (gmc->w > gmc->maxw)
	  {
	     gmc->w = gmc->maxw;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 0.0, 0.5, 1);
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
}

static void
_e_gadman_cb_signal_resize_up_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   _e_gadman_client_down_store(gmc);
   gmc->use_autoh = 0;
   gmc->resizing_u = 1;
}

static void
_e_gadman_cb_signal_resize_up_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->resizing_u = 0;
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_signal_resize_up_go(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   Evas_Coord x, y;
   
   gmc = data;
   if (!gmc->resizing_u) return;
   if (!(gmc->policy & E_GADMAN_POLICY_VSIZE)) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   gmc->x = gmc->down_store_x;
   gmc->y = gmc->down_store_y + (y - gmc->down_y);
   gmc->w = gmc->down_store_w;
   gmc->h = gmc->down_store_h - (y - gmc->down_y);
   /* limit to zone top edge */
   if (gmc->y < gmc->zone->y)
     {
	gmc->h = (gmc->down_store_y + gmc->down_store_h) - gmc->zone->y;
	gmc->y = gmc->zone->y;
     }
   /* limit to min size */
   if (gmc->h < gmc->minh)
     {
	gmc->y = (gmc->down_store_y + gmc->down_store_h) - gmc->minh;
	gmc->h = gmc->minh;
     }
   /* if we are atthe edge or beyond. assyme we want to be all the way there */
   if (y <= gmc->zone->y)
     {
	gmc->h = (gmc->y + gmc->h) - gmc->zone->y;
	gmc->y = gmc->zone->y;
     }
   /* limit to max size */
   if (gmc->maxh > 0)
     {
	if (gmc->h > gmc->minh)
	  {
	     gmc->y -= (gmc->maxh - gmc->h);
	     gmc->h = gmc->maxh;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 0.5, 1.0, 0);
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
}

static void
_e_gadman_cb_signal_resize_down_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (_e_gadman_client_is_being_modified(gmc)) return;
   _e_gadman_client_down_store(gmc);
   gmc->use_autoh = 0;
   gmc->resizing_d = 1;
}

static void
_e_gadman_cb_signal_resize_down_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->resizing_d = 0;
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_signal_resize_down_go(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Gadman_Client *gmc;
   Evas_Coord x, y;
   
   gmc = data;
   if (!gmc->resizing_d) return;
   if (!(gmc->policy & E_GADMAN_POLICY_VSIZE)) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   gmc->x = gmc->down_store_x;
   gmc->y = gmc->down_store_y;
   gmc->w = gmc->down_store_w;
   gmc->h = gmc->down_store_h + (y - gmc->down_y);
   /* limit to zone right bottom */
   if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     {
	gmc->h = (gmc->zone->y + gmc->zone->h) - gmc->y;
     }
   /* limit to min size */
   if (gmc->h < gmc->minh)
     {
	gmc->h = gmc->minh;
     }
   /* if we are atthe edge or beyond. assyme we want to be all the way there */
   if (y >= (gmc->zone->y + gmc->zone->h - 1))
     {
	gmc->h = gmc->zone->y + gmc->zone->h - gmc->y;
     }
   /* limit to max size */
   if (gmc->maxh > 0)
     {
	if (gmc->h > gmc->maxh)
	  {
	     gmc->h = gmc->maxh;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 0.5, 0.0, 0);
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
}

static void
_e_gadman_cb_menu_end(void *data, E_Menu *m)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   e_object_del(E_OBJECT(m));
   gmc->menu = NULL;
}

static void
_e_gadman_cb_half_width(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->w = gmc->zone->w / 2;
   if (gmc->w < gmc->minw) gmc->w = gmc->minw;
   if (gmc->maxw > 0)
     {
	if (gmc->w > gmc->maxw) gmc->w = gmc->maxw;
     }
   gmc->use_autow = 0;
   _e_gadman_client_aspect_enforce(gmc, 0.0, 0.5, 1);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_full_width(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->w = gmc->zone->w;
   if (gmc->w < gmc->minw) gmc->w = gmc->minw;
   if (gmc->maxw > 0)
     {
	if (gmc->w > gmc->maxw) gmc->w = gmc->maxw;
     }
   gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) / 2);
   gmc->use_autow = 0;
   _e_gadman_client_aspect_enforce(gmc, 0.0, 0.5, 1);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_auto_width(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (e_menu_item_toggle_get(mi))
     {
	gmc->use_autow = 1;
	gmc->w = gmc->autow;
	if (gmc->w > gmc->zone->w)
	  gmc->w = gmc->zone->w;
	if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	  gmc->x = (gmc->zone->x + gmc->zone->w) - gmc->w;
     }
   else
     gmc->use_autow = 0;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_center_horiz(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) / 2);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_half_height(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->h = gmc->zone->h / 2;
   if (gmc->h < gmc->minh) gmc->h = gmc->minh;
   if (gmc->maxh > 0)
     {
	if (gmc->h > gmc->maxh) gmc->h = gmc->maxh;
     }
   gmc->use_autoh = 0;
   _e_gadman_client_aspect_enforce(gmc, 0.5, 0.0, 0);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_full_height(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->h = gmc->zone->h;
   if (gmc->h < gmc->minh) gmc->h = gmc->minh;
   if (gmc->maxh > 0)
     {
	if (gmc->h > gmc->maxh) gmc->h = gmc->maxh;
     }
   gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) / 2);
   gmc->use_autoh = 0;
   _e_gadman_client_aspect_enforce(gmc, 0.5, 0.0, 0);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

static void
_e_gadman_cb_auto_height(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   if (e_menu_item_toggle_get(mi))
     {
	gmc->use_autoh = 1;
	gmc->h = gmc->autoh;
	if (gmc->h > gmc->zone->h)
	  gmc->h = gmc->zone->h;
	if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	  gmc->y = (gmc->zone->y + gmc->zone->h) - gmc->h;
     }
   else
     gmc->use_autoh = 0;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}


static void
_e_gadman_cb_center_vert(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;
   
   gmc = data;
   gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) / 2);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
}

