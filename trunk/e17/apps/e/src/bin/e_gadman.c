/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* FIXME: corner case if module is sized to full screen... cant stop edit or */
/*        re-enter edit mode (cant access root menu) */
/* FIXME: resist can still jump on top of other gads... */

/* local subsystem functions */

typedef struct _Gadman_Client_Config Gadman_Client_Config;

struct _Gadman_Client_Config
{
   double ax, ay;
   int w, h;
   int edge;
   int zone;
   int use_autow, use_autoh;
};

static void _e_gadman_free(E_Gadman *gm);
static void _e_gadman_client_free(E_Gadman_Client *gmc);
static void _e_gadman_client_edit_begin(E_Gadman_Client *gmc);
static void _e_gadman_client_edit_end(E_Gadman_Client *gmc);
static void _e_gadman_client_overlap_deny(E_Gadman_Client *gmc);
static void _e_gadman_client_down_store(E_Gadman_Client *gmc);
static int  _e_gadman_client_is_being_modified(E_Gadman_Client *gmc);
static void _e_gadman_client_geometry_to_align(E_Gadman_Client *gmc);
static void _e_gadman_client_aspect_enforce(E_Gadman_Client *gmc, double cx, double cy, int use_horiz);
static void _e_gadman_client_geometry_apply(E_Gadman_Client *gmc);
static void _e_gadman_client_callback_call(E_Gadman_Client *gmc, E_Gadman_Change change);

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

static void _e_gadman_cb_end_edit_mode(void *data, E_Menu *m, E_Menu_Item *mi);

static E_Config_DD *gadman_config_edd = NULL;

/* externally accessible functions */
int
e_gadman_init(void)
{
   gadman_config_edd = E_CONFIG_DD_NEW("Gadman_Client_Config", Gadman_Client_Config);
#undef T
#undef D
#define T Gadman_Client_Config
#define D gadman_config_edd
   E_CONFIG_VAL(D, T, ax, DOUBLE);
   E_CONFIG_VAL(D, T, ay, DOUBLE);
   E_CONFIG_VAL(D, T, w, INT);
   E_CONFIG_VAL(D, T, h, INT);
   E_CONFIG_VAL(D, T, edge, INT);
   E_CONFIG_VAL(D, T, zone, INT);
   E_CONFIG_VAL(D, T, use_autow, INT);
   E_CONFIG_VAL(D, T, use_autoh, INT);
   return 1;
}

int
e_gadman_shutdown(void)
{
   E_CONFIG_DD_FREE(gadman_config_edd);
   gadman_config_edd = NULL;
   return 1;
}

E_Gadman *
e_gadman_new(E_Container *con)
{
   E_Gadman    *gm;

   gm = E_OBJECT_ALLOC(E_Gadman, E_GADMAN_TYPE, _e_gadman_free);
   if (!gm) return NULL;
   gm->container = con;
   return gm;
}

void
e_gadman_mode_set(E_Gadman *gm, E_Gadman_Mode mode)
{
   Evas_List *l;
   
   E_OBJECT_CHECK(gm);
   E_OBJECT_TYPE_CHECK(gm, E_GADMAN_TYPE);
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
   E_OBJECT_TYPE_CHECK_RETURN(gm, E_GADMAN_TYPE, E_GADMAN_MODE_NORMAL);
   return gm->mode;
}

void
e_gadman_container_resize(E_Gadman *gm)
{
   Evas_List *l;
   
   E_OBJECT_CHECK(gm);
   for (l = gm->clients; l; l = l->next)
     {
	E_Gadman_Client *gmc;
	
	gmc = l->data;
	if (gmc->use_autow)
	  {
	     gmc->w = gmc->autow;
	     gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
	  }
	if (gmc->use_autoh)
	  {
	     gmc->h = gmc->autoh;
	     gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
	  }
	if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
	if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
	gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
	gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
	_e_gadman_client_overlap_deny(gmc);
	_e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
     }
}

E_Gadman_Client *
e_gadman_client_new(E_Gadman *gm)
{
   E_Gadman_Client *gmc;
   E_OBJECT_CHECK_RETURN(gm, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(gm, E_GADMAN_TYPE, NULL);
   
   gmc = E_OBJECT_ALLOC(E_Gadman_Client, E_GADMAN_CLIENT_TYPE, _e_gadman_client_free);
   if (!gmc) return NULL;
   gmc->gadman = gm;
   gmc->policy = E_GADMAN_POLICY_ANYWHERE | E_GADMAN_POLICY_HSIZE | E_GADMAN_POLICY_VSIZE | E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE;
   gmc->zone = e_zone_current_get(gm->container);
   gmc->edge = E_GADMAN_EDGE_TOP;
   gmc->minw = 1;
   gmc->minh = 1;
   gmc->maxw = 0;
   gmc->maxh = 0;
   gmc->ax = 0.0;
   gmc->ay = 0.0;
   gmc->mina = 0.0;
   gmc->maxa = 9999999.0;
   gm->clients = evas_list_append(gm->clients, gmc);
   return gmc;
}

void
e_gadman_client_save(E_Gadman_Client *gmc)
{
   Gadman_Client_Config cf;
   char buf[1024];
   
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   /* save all values */
   cf.ax = gmc->ax;
   cf.ay = gmc->ay;
   cf.w = gmc->w;
   cf.h = gmc->h;
   cf.edge = gmc->edge;
   cf.zone = gmc->zone->num;
   cf.use_autow = gmc->use_autow;
   cf.use_autoh = gmc->use_autoh;
   snprintf(buf, sizeof(buf), "gadman.%s.%i", gmc->domain, gmc->instance);
   e_config_domain_save(buf, gadman_config_edd, &cf);
}

void
e_client_gadman_edge_set(E_Gadman_Client *gmc, E_Gadman_Edge edge)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->edge = edge;
}

void
e_gadman_client_load(E_Gadman_Client *gmc)
{
   Gadman_Client_Config *cf;
   char buf[1024];
   
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   snprintf(buf, sizeof(buf), "gadman.%s.%i", gmc->domain, gmc->instance);
   cf = e_config_domain_load(buf, gadman_config_edd);
   if (cf)
     {
	E_Zone *zone;
	
	E_CONFIG_LIMIT(cf->ax, 0.0, 1.0);
	E_CONFIG_LIMIT(cf->ay, 0.0, 1.0);
	E_CONFIG_LIMIT(cf->w, 0, 10000);
	E_CONFIG_LIMIT(cf->h, 0, 10000);
	E_CONFIG_LIMIT(cf->edge, E_GADMAN_EDGE_LEFT, E_GADMAN_EDGE_BOTTOM);
	gmc->ax = cf->ax;
	gmc->ay = cf->ay;
	gmc->w = cf->w;
	gmc->h = cf->h;
	gmc->edge = cf->edge;
	gmc->use_autow = cf->use_autow;
	gmc->use_autoh = cf->use_autoh;
	zone = e_container_zone_number_get(gmc->zone->container, cf->zone);
	if (zone) gmc->zone = zone;
	if (gmc->use_autow)
	  {
	     gmc->w = gmc->autow;
	     gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
	  }
	if (gmc->use_autoh)
	  {
	     gmc->h = gmc->autoh;
	     gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
	  }
	if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
	if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
	gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
	gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
	free(cf);
     }
   _e_gadman_client_overlap_deny(gmc);
   e_object_ref(E_OBJECT(gmc));
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_ZONE);
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_EDGE);
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
   e_object_unref(E_OBJECT(gmc));
}

void
e_gadman_client_domain_set(E_Gadman_Client *gmc, char *domain, int instance)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if (gmc->domain) free(gmc->domain);
   gmc->domain = strdup(domain);
   gmc->instance = instance;
}

void
e_gadman_client_zone_set(E_Gadman_Client *gmc, E_Zone *zone)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if (zone == gmc->zone) return;
   gmc->zone = zone;
   gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
   gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
   e_object_ref(E_OBJECT(gmc));
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_ZONE);
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
   e_object_unref(E_OBJECT(gmc));
}

void
e_gadman_client_policy_set(E_Gadman_Client *gmc, E_Gadman_Policy pol)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->policy = pol;
}

void
e_gadman_client_min_size_set(E_Gadman_Client *gmc, Evas_Coord minw, Evas_Coord minh)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if ((gmc->minw == minw) && (gmc->minh == minh)) return;
   gmc->minw = minw;
   gmc->minh = minh;
   if (gmc->minw > gmc->w)
     {
	gmc->w = gmc->minw;
	gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
     }
   if (gmc->minh > gmc->h)
     {
	gmc->h = gmc->minh;
	gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
     }
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

void
e_gadman_client_max_size_set(E_Gadman_Client *gmc, Evas_Coord maxw, Evas_Coord maxh)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if ((gmc->maxw == maxw) && (gmc->maxh == maxh)) return;
   gmc->maxw = maxw;
   gmc->maxh = maxh;
   if (gmc->maxw < gmc->w)
     {
	gmc->w = gmc->maxw;
	gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
     }
   if (gmc->maxh < gmc->h)
     {
	gmc->h = gmc->maxh;
	gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
     }
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

void
e_gadman_client_align_set(E_Gadman_Client *gmc, double xalign, double yalign)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if ((gmc->ax == xalign) && (gmc->ay == yalign)) return;
   gmc->ax = xalign;
   gmc->ay = yalign;
   gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
   gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

void
e_gadman_client_aspect_set(E_Gadman_Client *gmc, double mina, double maxa)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->mina = mina;
   gmc->maxa = maxa;
}

void
e_gadman_client_auto_size_set(E_Gadman_Client *gmc, Evas_Coord autow, Evas_Coord autoh)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->autow = autow;
   gmc->autoh = autoh;
   if (gmc->use_autow)
     {
	gmc->w = gmc->autow;
	gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
     }
   if (gmc->use_autoh)
     {
	gmc->h = gmc->autoh;
	gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
     }
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

void
e_gadman_client_edge_set(E_Gadman_Client *gmc, E_Gadman_Edge edge)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->edge = edge;
}

E_Gadman_Edge
e_gadman_client_edge_get(E_Gadman_Client *gmc)
{
   E_OBJECT_CHECK_RETURN(gmc, E_GADMAN_EDGE_TOP);
   E_OBJECT_TYPE_CHECK_RETURN(gmc, E_GADMAN_CLIENT_TYPE, E_GADMAN_EDGE_TOP);
   return gmc->edge;
}

void
e_gadman_client_geometry_get(E_Gadman_Client *gmc, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if (x) *x = gmc->x;
   if (y) *y = gmc->y;
   if (w) *w = gmc->w;
   if (h) *h = gmc->h;
}

void
e_gadman_client_resize(E_Gadman_Client *gmc, Evas_Coord w, Evas_Coord h)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   if ((gmc->w == w) && (gmc->h == h)) return;
   gmc->w = w;
   if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
   gmc->x = gmc->zone->x + ((gmc->zone->w - gmc->w) * gmc->ax);
   gmc->h = h;
   if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
   gmc->y = gmc->zone->y + ((gmc->zone->h - gmc->h) * gmc->ay);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

void
e_gadman_client_change_func_set(E_Gadman_Client *gmc, void (*func) (void *data, E_Gadman_Client *gmc, E_Gadman_Change change), void *data)
{
   E_OBJECT_CHECK(gmc);
   E_OBJECT_TYPE_CHECK(gmc, E_GADMAN_CLIENT_TYPE);
   gmc->func = func;
   gmc->data = data;
}

E_Menu *
e_gadman_client_menu_new(E_Gadman_Client *gmc)
{
   E_Menu *m;
   E_Menu_Item *mi;
   
   E_OBJECT_CHECK_RETURN(gmc, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(gmc, E_GADMAN_CLIENT_TYPE, NULL);
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
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, "End Edit Mode");
   e_menu_item_callback_set(mi, _e_gadman_cb_end_edit_mode, gmc);
   
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
   
   evas_object_clip_set(gmc->event_object, gmc->zone->bg_clip_object);
   evas_object_clip_set(gmc->control_object, gmc->zone->bg_clip_object);
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
_e_gadman_client_overlap_deny(E_Gadman_Client *gmc)
{
   Evas_List *l;
   Evas_Coord ox, oy;
   int ok = 0;
   int iterate = 0;
   
   ox = gmc->x;
   oy = gmc->y;
   ok = 0;
   if ((gmc->policy & 0xff) == E_GADMAN_POLICY_EDGES)
     {
	if ((gmc->edge == E_GADMAN_EDGE_LEFT) ||
	    (gmc->edge == E_GADMAN_EDGE_RIGHT))
	  {
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  gmc2 = l->data;
		  if (gmc != gmc2)
		    {
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    ok = 0;
			    gmc->y = gmc2->y + gmc2->h;
			 }
		    }
	       }
	     if (ok) return;
	     if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	       gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	     ok = 1;
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  gmc2 = l->data;
		  if (gmc != gmc2)
		    {
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    ok = 0;
			    break;
			 }
		    }
	       }
	     if (ok)
	       {
		  _e_gadman_client_geometry_to_align(gmc);
		  return;
	       }
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  if (gmc != gmc2)
		    {
		       gmc2 = l->data;
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    gmc->y = gmc2->y - gmc->h;
			 }
		    }
	       }
	     if (gmc->y < gmc->zone->y)
	       gmc->y = gmc->zone->y;
	  }
	else if ((gmc->edge == E_GADMAN_EDGE_TOP) ||
		 (gmc->edge == E_GADMAN_EDGE_BOTTOM))
	  {
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  gmc2 = l->data;
		  if (gmc != gmc2)
		    {
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    ok = 0;
			    gmc->x = gmc2->x + gmc2->w;
			 }
		    }
	       }
	     if (ok) return;
	     if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	       gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	     ok = 1;
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  gmc2 = l->data;
		  if (gmc != gmc2)
		    {
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    ok = 0;
			    break;
			 }
		    }
	       }
	     if (ok)
	       {
		  _e_gadman_client_geometry_to_align(gmc);
		  return;
	       }
	     for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	       {
		  E_Gadman_Client *gmc2;
		  
		  if (gmc != gmc2)
		    {
		       gmc2 = l->data;
		       if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
			   (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
			 {
			    gmc->x = gmc2->x - gmc->w;
			 }
		    }
	       }
	     if (gmc->x < gmc->zone->x)
	       gmc->x = gmc->zone->x;
	  }
	_e_gadman_client_geometry_to_align(gmc);
	return;
     }
   while ((!ok) && (iterate < 1000))
     {
	ok = 1;
	for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	  {
	     E_Gadman_Client *gmc2;
	     
	     gmc2 = l->data;
	     if (gmc != gmc2)
	       {
		  if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
		      (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
		    {
		       ok = 0;
		       gmc->x = gmc2->x + gmc2->w;
		    }
	       }
	  }
	if (ok) break;
	if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	  gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	ok = 1;
	for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	  {
	     E_Gadman_Client *gmc2;
	     
	     gmc2 = l->data;
	     if (gmc != gmc2)
	       {
		  if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
		      (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
		    {
		       ok = 0;
		       break;
		    }
	       }
	  }
	if (ok) break;
	for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	  {
	     E_Gadman_Client *gmc2;
	     
	     if (gmc != gmc2)
	       {
		  gmc2 = l->data;
		  if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
		      (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
		    {
		       gmc->x = gmc2->x - gmc->w;
		    }
	       }
	  }
	if (gmc->x < gmc->zone->x)
	  gmc->x = gmc->zone->x;
	ok = 1;
	for (l = gmc->zone->container->gadman->clients; l; l = l->next)
	  {
	     E_Gadman_Client *gmc2;
	     
	     gmc2 = l->data;
	     if (gmc != gmc2)
	       {
		  if ((E_SPANS_COMMON(gmc->x, gmc->w, gmc2->x, gmc2->w)) &&
		      (E_SPANS_COMMON(gmc->y, gmc->h, gmc2->y, gmc2->h)))
		    {
		       ok = 0;
		       break;
		    }
	       }
	  }
	if (ok) break;
	if (gmc->y > (gmc->zone->y + (gmc->zone->h / 2)))
	  gmc->y -= 8;
	else
	  gmc->y += 8;
	if (oy < (gmc->zone->y + (gmc->zone->h / 2)))
	  {
	     if ((gmc->y - oy) > (gmc->zone->h / 3))
	       {
		  gmc->x = ox;
		  gmc->y = oy;
		  break;
	       }
	  }
	else
	  {
	     if ((oy - gmc->y) > (gmc->zone->h / 3))
	       {
		  gmc->x = ox;
		  gmc->y = oy;
		  break;
	       }
	  }
	iterate++;
     }
   _e_gadman_client_geometry_to_align(gmc);
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
_e_gadman_client_callback_call(E_Gadman_Client *gmc, E_Gadman_Change change)
{
   if (gmc->func) gmc->func(gmc->data, gmc, change);
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
   evas_object_raise(gmc->control_object);
   evas_object_raise(gmc->event_object);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_RAISE);
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
   int nx, ny, nxx, nyy;
   int new_zone = 0;
   Evas_List *skiplist = NULL;
   
   gmc = data;
   if (!gmc->moving) return;
   evas_pointer_canvas_xy_get(gmc->gadman->container->bg_evas, &x, &y);
   nxx = nx = gmc->down_store_x + (x - gmc->down_x);
   nyy = ny = gmc->down_store_y + (y - gmc->down_y);
   skiplist = evas_list_append(skiplist, gmc);
   e_resist_container_gadman_position(gmc->zone->container, skiplist, 
				      gmc->x, gmc->y, gmc->w, gmc->h,
				      nx, ny, gmc->w, gmc->h,
				      &nxx, &nyy);
   evas_list_free(skiplist);
   x += (nxx - nx);
   y += (nyy - ny);
   if ((gmc->policy & 0xff) == E_GADMAN_POLICY_EDGES)
     {
	double xr, yr;
	E_Gadman_Edge ne;
	
	ne = gmc->edge;
	
	if (!(gmc->policy & E_GADMAN_POLICY_FIXED_ZONE))
	  {
	     E_Zone *nz;
	     
	     nz = e_container_zone_at_point_get(gmc->zone->container, x, y);
	     if ((nz) && (nz != gmc->zone))
	       {
		  gmc->zone = nz;
		  new_zone = 1;
		  evas_object_clip_set(gmc->event_object, gmc->zone->bg_clip_object);
		  evas_object_clip_set(gmc->control_object, gmc->zone->bg_clip_object);
	       }
	  }
	
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
	     if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
	     if (gmc->y < gmc->zone->y)
	       gmc->y = gmc->zone->y;
	     else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	       gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_RIGHT)
	  {
	     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	     gmc->y = gmc->down_store_y + (y - gmc->down_y);
	     if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
	     if (gmc->y < gmc->zone->y)
	       gmc->y = gmc->zone->y;
	     else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
	       gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_TOP)
	  {
	     gmc->x = gmc->down_store_x + (x - gmc->down_x);
	     gmc->y = gmc->zone->y;
	     if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
	     if (gmc->x < gmc->zone->x)
	       gmc->x = gmc->zone->x;
	     else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
	       gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
	  }
	else if (gmc->edge == E_GADMAN_EDGE_BOTTOM)
	  {
	     gmc->x = gmc->down_store_x + (x - gmc->down_x);
	     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
	     if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
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
	if (!(gmc->policy & E_GADMAN_POLICY_FIXED_ZONE))
	  {
	     E_Zone *nz;
	     
	     nz = e_container_zone_at_point_get(gmc->zone->container, x, y);
	     if ((nz) && (nz != gmc->zone))
	       {
		  gmc->zone = nz;
		  new_zone = 1;
		  evas_object_clip_set(gmc->event_object, gmc->zone->bg_clip_object);
		  evas_object_clip_set(gmc->control_object, gmc->zone->bg_clip_object);
	       }
	  }
	if (gmc->h > gmc->zone->h) gmc->h = gmc->zone->h;
	if (gmc->w > gmc->zone->w) gmc->w = gmc->zone->w;
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
   _e_gadman_client_geometry_apply(gmc);
   e_object_ref(E_OBJECT(gmc));
   if (!e_object_del_get(E_OBJECT(gmc)))
     {
	if (new_zone)
	  _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_ZONE);
     }
   if (!e_object_del_get(E_OBJECT(gmc)))
     {
	if (new_edge)
	  _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_EDGE);
     }
   if (!e_object_del_get(E_OBJECT(gmc)))
     _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
   e_object_unref(E_OBJECT(gmc));
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
	if (gmc->w > gmc->maxw)
	  {
	     gmc->x -= (gmc->maxw - gmc->w);
	     gmc->w = gmc->maxw;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 1.0, 0.5, 1);
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
	if (gmc->h > gmc->maxh)
	  {
	     gmc->y -= (gmc->maxh - gmc->h);
	     gmc->h = gmc->maxh;
	  }
     }
   _e_gadman_client_aspect_enforce(gmc, 0.5, 1.0, 0);
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_to_align(gmc);
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   if (gmc->x < gmc->zone->x)
     gmc->x = gmc->zone->x;
   else if ((gmc->x + gmc->w) > (gmc->zone->x + gmc->zone->w))
     gmc->x = gmc->zone->x + gmc->zone->w - gmc->w;
   if (gmc->y < gmc->zone->y)
     gmc->y = gmc->zone->y;
   else if ((gmc->y + gmc->h) > (gmc->zone->y + gmc->zone->h))
     gmc->y = gmc->zone->y + gmc->zone->h - gmc->h;
   _e_gadman_client_geometry_apply(gmc);
   _e_gadman_client_geometry_to_align(gmc);
   e_gadman_client_save(gmc);
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
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
   _e_gadman_client_callback_call(gmc, E_GADMAN_CHANGE_MOVE_RESIZE);
}

static void
_e_gadman_cb_end_edit_mode(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman_Client *gmc;

   gmc = data;
   e_gadman_mode_set(gmc->gadman, E_GADMAN_MODE_NORMAL);
}
