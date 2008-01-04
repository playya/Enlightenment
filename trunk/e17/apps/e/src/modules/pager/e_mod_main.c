/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "e.h"
#include "e_mod_main.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);
static const char *_gc_id_new(void);
/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
     "pager",
     {
	_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL
     },
   E_GADCON_CLIENT_STYLE_INSET
};

/* actual module specifics */
typedef struct _Instance Instance;
typedef struct _Pager       Pager;
typedef struct _Pager_Desk  Pager_Desk;
typedef struct _Pager_Win   Pager_Win;
typedef struct _Pager_Popup Pager_Popup;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_pager; /* table */
   Pager           *pager;
   E_Drop_Handler  *drop_handler;
};

struct _Pager
{
   Instance     *inst;
   unsigned char is_popup : 1;
   Evas_Object  *o_table;
   E_Zone       *zone;
   int           xnum, ynum;
   Evas_List    *desks;
   unsigned char dragging : 1;
   unsigned char just_dragged : 1;
   Evas_Coord    dnd_x, dnd_y;
   Pager_Desk   *active_drop_pd;
};

struct _Pager_Desk
{
   Pager           *pager;
   E_Desk          *desk;
   Evas_List       *wins;
   Evas_Object     *o_desk;
   Evas_Object     *o_layout;
   int              xpos, ypos;
   int              current : 1;
   int              urgent;
   struct {
      Pager         *from_pager;
      unsigned char  in_pager : 1;
      unsigned char  start : 1;
      int            x, y;
      int            dx, dy;
      int            button;
   } drag;
};

struct _Pager_Win
{
   E_Border     *border;
   Pager_Desk   *desk;
   Evas_Object  *o_window;
   Evas_Object  *o_icon;
   unsigned char skip_winlist : 1;
   struct {
      Pager         *from_pager;
      unsigned char  start : 1;
      unsigned char  in_pager : 1;
      unsigned char  no_place : 1;
      unsigned char  desktop  : 1;
      int            x, y;
      int            dx, dy;
      int            button;
   } drag;
};

struct _Pager_Popup
{
   E_Popup      *popup;
   Pager        *pager;
   Evas_Object  *o_bg;
   Ecore_Timer  *timer;
   unsigned char urgent : 1;
};

static void _pager_cb_obj_moveresize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_cb_post(void *data, E_Menu *m);
static void _pager_inst_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _pager_inst_cb_menu_virtual_desktops_dialog(void *data, E_Menu *m, E_Menu_Item *mi);
static void _pager_instance_drop_zone_recalc(Instance *inst);
static int _pager_cb_event_border_resize(void *data, int type, void *event);
static int _pager_cb_event_border_move(void *data, int type, void *event);
static int _pager_cb_event_border_add(void *data, int type, void *event);
static int _pager_cb_event_border_remove(void *data, int type, void *event);
static int _pager_cb_event_border_iconify(void *data, int type, void *event);
static int _pager_cb_event_border_uniconify(void *data, int type, void *event);
static int _pager_cb_event_border_stick(void *data, int type, void *event);
static int _pager_cb_event_border_unstick(void *data, int type, void *event);
static int _pager_cb_event_border_desk_set(void *data, int type, void *event);
static int _pager_cb_event_border_stack(void *data, int type, void *event);
static int _pager_cb_event_border_icon_change(void *data, int type, void *event);
static int _pager_cb_event_border_urgent_change(void *data, int type, void *event);
static int _pager_cb_event_border_property(void *data, int type, void *event);
static int _pager_cb_event_zone_desk_count_set(void *data, int type, void *event);
static int _pager_cb_event_desk_show(void *data, int type, void *event);
static int _pager_cb_event_desk_name_change(void *data, int type, void *event);
static int _pager_cb_event_container_resize(void *data, int type, void *event);
static void _pager_window_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_window_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_window_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_window_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_window_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void *_pager_window_cb_drag_convert(E_Drag *drag, const char *type);
static void _pager_window_cb_drag_finished(E_Drag *drag, int dropped);
static void _pager_inst_cb_enter(void *data, const char *type, void *event_info);
static void _pager_inst_cb_move(void *data, const char *type, void *event_info);
static void _pager_inst_cb_leave(void *data, const char *type, void *event_info);
static void _pager_inst_cb_drop(void *data, const char *type, void *event_info);
static void _pager_inst_cb_scroll(void *data);
static void _pager_update_drop_position(Instance *inst, Evas_Coord x, Evas_Coord y);
static void _pager_desk_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_desk_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_desk_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _pager_desk_cb_drag_finished(E_Drag *drag, int dropped);
static void _pager_desk_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int _pager_popup_cb_timeout(void *data);
static Pager *_pager_new(Evas *evas, E_Zone *zone);
static void _pager_free(Pager *p);
static void _pager_fill(Pager *p);
static void _pager_empty(Pager *p);
static Pager_Desk *_pager_desk_new(Pager *p, E_Desk *desk, int xpos, int ypos);
static void _pager_desk_free(Pager_Desk *pd);
static Pager_Desk *_pager_desk_at_coord(Pager *p, Evas_Coord x, Evas_Coord y);
static void _pager_desk_select(Pager_Desk *pd);
static Pager_Desk *_pager_desk_find(Pager *p, E_Desk *desk);
static void _pager_desk_switch(Pager_Desk *pd1, Pager_Desk *pd2);
static Pager_Win *_pager_window_new(Pager_Desk *pd, E_Border *border);
static void _pager_window_free(Pager_Win *pw);
static void _pager_window_move(Pager_Win *pw);
static Pager_Win *_pager_window_find(Pager *p, E_Border *border);
static Pager_Win *_pager_desk_window_find(Pager_Desk *pd, E_Border *border);
static Pager_Popup *_pager_popup_new(E_Zone *zone, int urgent);
static void _pager_popup_free(Pager_Popup *pp);
static Pager_Popup *_pager_popup_find(E_Zone *zone);
static E_Config_Dialog *_pager_config_dialog(E_Container *con, const char *params);

static E_Config_DD *conf_edd = NULL;
static Evas_List *pager_popups = NULL;

Config *pager_config = NULL;

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Pager *p;
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Evas_Coord x, y, w, h;
   const char *drop[] = { "enlightenment/pager_win", "enlightenment/border", "enlightenment/vdesktop"};

   inst = E_NEW(Instance, 1);

   p = _pager_new(gc->evas, gc->zone);
   p->inst = inst;
   inst->pager = p;
   o = p->o_table;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->o_pager = o;

   evas_object_geometry_get(o, &x, &y, &w, &h);
   inst->drop_handler =
     e_drop_handler_add(E_OBJECT(inst->gcc), inst,
			_pager_inst_cb_enter, _pager_inst_cb_move,
			_pager_inst_cb_leave, _pager_inst_cb_drop,
			drop, 3, x, y, w, h);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOVE,
				  _pager_cb_obj_moveresize, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
				  _pager_cb_obj_moveresize, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
				  _button_cb_mouse_down, inst);
   pager_config->instances = evas_list_append(pager_config->instances, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   pager_config->instances = evas_list_remove(pager_config->instances, inst);
   e_drop_handler_del(inst->drop_handler);
   _pager_free(inst->pager);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   e_gadcon_client_aspect_set(gcc,
			      inst->pager->xnum * inst->pager->zone->w,
			      inst->pager->ynum * inst->pager->zone->h);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static char *
_gc_label(void)
{
   return _("Pager");
}

static Evas_Object *
_gc_icon(Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-pager.edj",
	    e_module_dir_get(pager_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(void)
{
   return _gadcon_class.name;
}

static Pager *
_pager_new(Evas *evas, E_Zone *zone)
{
   Pager *p;

   p = E_NEW(Pager, 1);
   p->o_table = e_table_add(evas);
   e_table_homogenous_set(p->o_table, 1);
   p->zone = zone;
   _pager_fill(p);
   return p;
}

static void
_pager_free(Pager *p)
{
   _pager_empty(p);
   evas_object_del(p->o_table);
   free(p);
}

static void
_pager_fill(Pager *p)
{
   int x, y;

   e_zone_desk_count_get(p->zone, &(p->xnum), &(p->ynum));
   for (x = 0; x < p->xnum; x++)
     {
	for (y = 0; y < p->ynum; y++)
	  {
	     Pager_Desk *pd;
	     E_Desk     *desk;

	     desk = e_desk_at_xy_get(p->zone, x, y);
	     pd = _pager_desk_new(p, desk, x, y);
	     if (pd)
	       {
		  p->desks = evas_list_append(p->desks, pd);
		  if (desk == e_desk_current_get(desk->zone))
		    _pager_desk_select(pd);
	       }
	  }
     }
}

static void
_pager_empty(Pager *p)
{
   // if (p->popup) _pager_popup_free(p->popup);

   while (p->desks)
     {
	_pager_desk_free(p->desks->data);
	p->desks = evas_list_remove_list(p->desks, p->desks);
     }
}

static Pager_Desk *
_pager_desk_new(Pager *p, E_Desk *desk, int xpos, int ypos)
{
   Pager_Desk    *pd;
   Evas_Object   *o;
   E_Border_List *bl;
   E_Border      *bd;

   pd = E_NEW(Pager_Desk, 1);
   if (!pd) return NULL;

   pd->xpos = xpos;
   pd->ypos = ypos;

   pd->urgent = 0;

   pd->desk = desk;
   e_object_ref(E_OBJECT(desk));
   pd->pager = p;

   o = edje_object_add(evas_object_evas_get(p->o_table));
   pd->o_desk = o;
   e_theme_edje_object_set(o, "base/theme/modules/pager",
			   "e/modules/pager/desk");
   edje_object_part_text_set(o, "label", desk->name);
   e_table_pack(p->o_table, o, xpos, ypos, 1, 1);
   e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _pager_desk_cb_mouse_down, pd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _pager_desk_cb_mouse_up, pd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _pager_desk_cb_mouse_move, pd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _pager_desk_cb_mouse_wheel, pd);
   evas_object_show(o);

   o = e_layout_add(evas_object_evas_get(p->o_table));
   pd->o_layout = o;

   e_layout_virtual_size_set(o, desk->zone->w, desk->zone->h);
   edje_object_part_swallow(pd->o_desk, "items", pd->o_layout);
   evas_object_show(o);

   bl = e_container_border_list_first(desk->zone->container);
   while ((bd = e_container_border_list_next(bl)))
     {
	Pager_Win   *pw;

	if ((bd->new_client) || ((bd->desk != desk) && (!bd->sticky))) continue;
	pw = _pager_window_new(pd, bd);
	if (pw) pd->wins = evas_list_append(pd->wins, pw);
     }
   e_container_border_list_free(bl);
   return pd;
}

static void
_pager_desk_free(Pager_Desk *pd)
{
   Evas_List *l;

   evas_object_del(pd->o_desk);
   evas_object_del(pd->o_layout);
   for (l = pd->wins; l; l = l->next)
     _pager_window_free(l->data);
   evas_list_free(pd->wins);
   e_object_unref(E_OBJECT(pd->desk));
   free(pd);
}

static Pager_Desk *
_pager_desk_at_coord(Pager *p, Evas_Coord x, Evas_Coord y)
{
   Evas_List *l;

   for (l = p->desks; l; l = l->next)
     {
	Pager_Desk *pd;
	Evas_Coord dx, dy, dw, dh;

	pd = l->data;
	evas_object_geometry_get(pd->o_desk, &dx, &dy, &dw, &dh);
	if (E_INSIDE(x, y, dx, dy, dw, dh)) return pd;
     }
   return NULL;
}

static void
_pager_desk_select(Pager_Desk *pd)
{
   Evas_List *l;

   if (pd->current) return;
   for (l = pd->pager->desks; l; l = l->next)
     {
	Pager_Desk *pd2;

	pd2 = l->data;
	if (pd == pd2)
	  {
	     pd2->current = 1;
	     edje_object_signal_emit(pd2->o_desk, "e,state,selected", "e");
	  }
	else
	  {
	     if (pd2->current)
	       {
		  pd2->current = 0;
		  edje_object_signal_emit(pd2->o_desk, "e,state,unselected", "e");
	       }
	  }
     }
}

static Pager_Desk *
_pager_desk_find(Pager *p, E_Desk *desk)
{
   Evas_List *l;

   for (l = p->desks; l; l = l->next)
     {
	Pager_Desk *pd;

	pd = l->data;
	if (pd->desk == desk) return pd;
     }
   return NULL;
}

static void
_pager_desk_switch(Pager_Desk *pd1, Pager_Desk *pd2)
{
   int c;
   E_Zone     *zone1, *zone2;
   E_Desk     *desk1, *desk2;
   Pager_Win  *pw;
   Evas_List  *l;

   if ((!pd1) || (!pd2) || (!pd1->desk) || (!pd2->desk)) return;
   if (pd1 == pd2) return;

   desk1 = pd1->desk;
   desk2 = pd2->desk;
   zone1 = pd1->desk->zone;
   zone2 = pd2->desk->zone;

   /* Move opened windows from on desk to the other */
   for (l = pd1->wins; l; l = l->next)
     {
	pw = l->data;
	if (!pw || !pw->border || pw->border->iconic) continue;
	e_border_desk_set(pw->border, desk2);
     }
   for (l = pd2->wins; l; l = l->next)
     {
	pw = l->data;
	if (!pw || !pw->border || pw->border->iconic) continue;
	e_border_desk_set(pw->border, desk1);
     }

   /* Modify desktop names in the config */
   for (l = e_config->desktop_names, c = 0; l && c < 2; l = l->next)
     {
	E_Config_Desktop_Name *tmp_dn;

	tmp_dn = l->data;
	if (!tmp_dn) continue;
	if (tmp_dn->desk_x == desk1->x &&
	    tmp_dn->desk_y == desk1->y &&
	    tmp_dn->zone   == desk1->zone->num)
	  {
	     tmp_dn->desk_x = desk2->x;
	     tmp_dn->desk_y = desk2->y;
	     tmp_dn->zone   = desk2->zone->num;
	     c++;
	  }
	else if (tmp_dn->desk_x == desk2->x &&
		 tmp_dn->desk_y == desk2->y &&
		 tmp_dn->zone   == desk2->zone->num)
	  {
	     tmp_dn->desk_x = desk1->x;
	     tmp_dn->desk_y = desk1->y;
	     tmp_dn->zone   = desk1->zone->num;
	     c++;
	  }
     }
   if (c > 0) e_config_save();
   e_desk_name_update();

   /* Modify desktop backgrounds in the config */
   for (l = e_config->desktop_backgrounds, c = 0; l && c < 2; l = l->next)
     {
	E_Config_Desktop_Background *tmp_db;

	tmp_db = l->data;
	if (!tmp_db) continue;
	if (tmp_db->desk_x == desk1->x &&
	    tmp_db->desk_y == desk1->y &&
	    tmp_db->zone   == desk1->zone->num)
	  {
	     tmp_db->desk_x = desk2->x;
	     tmp_db->desk_y = desk2->y;
	     tmp_db->zone   = desk2->zone->num;
	     c++;
	  }
	else if (tmp_db->desk_x == desk2->x &&
		 tmp_db->desk_y == desk2->y &&
		 tmp_db->zone   == desk2->zone->num)
	  {
	     tmp_db->desk_x = desk1->x;
	     tmp_db->desk_y = desk1->y;
	     tmp_db->zone   = desk1->zone->num;
	     c++;
	  }
     }
   if (c > 0) e_config_save();

   /* If the current desktop has been switched, force to update of the screen */
   if (desk2 == e_desk_current_get(zone2))
     {
	desk2->visible = 0;
	e_desk_show(desk2);
     }
   if (desk1 == e_desk_current_get(zone1))
     {
	desk1->visible = 0;
	e_desk_show(desk1);
     }
}

static Pager_Win  *
_pager_window_new(Pager_Desk *pd, E_Border *border)
{
   Pager_Win   *pw;
   Evas_Object *o;
   int          visible;

   if (!border) return NULL;
   pw = E_NEW(Pager_Win, 1);
   if (!pw) return NULL;

   pw->border = border;
   e_object_ref(E_OBJECT(border));

   visible = !border->iconic && !border->client.netwm.state.skip_pager;
   pw->skip_winlist = border->client.netwm.state.skip_pager;
   pw->desk = pd;

   o = edje_object_add(evas_object_evas_get(pd->pager->o_table));
   pw->o_window = o;
   e_theme_edje_object_set(o, "base/theme/modules/pager",
			   "e/modules/pager/window");
   if (visible) evas_object_show(o);

   e_layout_pack(pd->o_layout, pw->o_window);
   e_layout_child_raise(pw->o_window);

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,  _pager_window_cb_mouse_in,  pw);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _pager_window_cb_mouse_out, pw);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _pager_window_cb_mouse_down, pw);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _pager_window_cb_mouse_up, pw);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _pager_window_cb_mouse_move, pw);

   o = e_border_icon_add(border, evas_object_evas_get(pd->pager->o_table));
   if (o)
     {
	pw->o_icon = o;
	evas_object_show(o);
	edje_object_part_swallow(pw->o_window, "icon", o);
     }

   if (border->client.icccm.urgent)
     {
	if (!(border->iconic))
	  edje_object_signal_emit(pd->o_desk,
				  "e,state,urgent", "e");
	edje_object_signal_emit(pw->o_window,
				"e,state,urgent", "e");
     }

   evas_object_show(o);

   _pager_window_move(pw);
   return pw;
}

static void
_pager_window_free(Pager_Win *pw)
{
   if ((pw->drag.from_pager) && (pw->desk->pager->dragging))
     pw->desk->pager->dragging = 0;
   if (pw->o_window) evas_object_del(pw->o_window);
   if (pw->o_icon) evas_object_del(pw->o_icon);
   e_object_unref(E_OBJECT(pw->border));
   free(pw);
}

static void
_pager_window_move(Pager_Win *pw)
{
   e_layout_child_move(pw->o_window,
		       pw->border->x - pw->desk->desk->zone->x,
		       pw->border->y - pw->desk->desk->zone->y);
   e_layout_child_resize(pw->o_window, pw->border->w, pw->border->h);
}

static Pager_Win *
_pager_window_find(Pager *p, E_Border *border)
{
   Evas_List  *l;

   for (l = p->desks; l; l = l->next)
     {
	Pager_Desk *pd;
	Pager_Win *pw;

	pd = l->data;
	pw = _pager_desk_window_find(pd, border);
	if (pw) return pw;
     }
   return NULL;
}

static Pager_Win *
_pager_desk_window_find(Pager_Desk *pd, E_Border *border)
{
   Evas_List  *l;

   for (l = pd->wins; l; l = l->next)
     {
	Pager_Win  *pw;

	pw = l->data;
	if (pw->border == border) return pw;
     }
   return NULL;
}

static Pager_Popup *
_pager_popup_new(E_Zone *zone, int urgent)
{
   Pager_Popup *pp;
   Evas_Coord w, h;
   int x, y, height, width;
   E_Desk *desk;

   pp = E_NEW(Pager_Popup, 1);
   if (!pp) return NULL;

   /* Show popup */
   pp->popup = e_popup_new(zone, 0, 0, 1, 1);
   if (!pp->popup)
     {
	free(pp);
	return NULL;
     }
   e_popup_layer_set(pp->popup, 255);

   pp->pager = _pager_new(pp->popup->evas, zone);

   pp->pager->is_popup = 1;

   e_zone_desk_count_get(zone, &x, &y);
   height = pager_config->popup_pager_height * y;
   width = ((zone->w * x)/(zone->h * y)) * height;

   evas_object_move(pp->pager->o_table, 0, 0);
   evas_object_resize(pp->pager->o_table, width, height);

   pp->o_bg = edje_object_add(pp->popup->evas);
   e_theme_edje_object_set(pp->o_bg,
			   "base/theme/modules/pager",
			   "e/widgets/pager/popup");
   desk = e_desk_current_get(zone);
   if (desk)
     edje_object_part_text_set(pp->o_bg, "text", desk->name);
   evas_object_show(pp->o_bg);

   edje_extern_object_min_size_set(pp->pager->o_table, width, height);
   edje_object_part_swallow(pp->o_bg, "pager", pp->pager->o_table);
   edje_object_size_min_calc(pp->o_bg, &w, &h);

   evas_object_move(pp->o_bg, 0, 0);
   evas_object_resize(pp->o_bg, w, h);
   e_popup_edje_bg_object_set(pp->popup, pp->o_bg);
   //e_popup_ignore_events_set(pp->popup, 1);
   e_popup_move_resize(pp->popup, ((zone->w - w) / 2),
		       ((zone->h - h) / 2), w, h);
   e_bindings_mouse_grab(E_BINDING_CONTEXT_POPUP, pp->popup->evas_win);
   e_bindings_wheel_grab(E_BINDING_CONTEXT_POPUP, pp->popup->evas_win);
   e_popup_show(pp->popup);

   if (!urgent)
     {
	pp->timer = ecore_timer_add(pager_config->popup_speed,
				    _pager_popup_cb_timeout, pp);
     }
   else
     {
	if (!pager_config->popup_urgent_stick)
	  pp->timer = ecore_timer_add(pager_config->popup_urgent_speed,
				      _pager_popup_cb_timeout, pp);
     }
   pp->urgent = urgent;

   pager_popups = evas_list_append(pager_popups, pp);

   return pp;
}

static void
_pager_popup_free(Pager_Popup *pp)
{
   if (pp->timer) ecore_timer_del(pp->timer);
   evas_object_del(pp->o_bg);
   _pager_free(pp->pager);
   e_bindings_mouse_ungrab(E_BINDING_CONTEXT_POPUP, pp->popup->evas_win);
   e_bindings_wheel_ungrab(E_BINDING_CONTEXT_POPUP, pp->popup->evas_win);
   e_object_del(E_OBJECT(pp->popup));
   pager_popups = evas_list_remove(pager_popups, pp);
   free(pp);
}

static Pager_Popup *
_pager_popup_find(E_Zone *zone)
{
   Pager_Popup *pp;
   Evas_List *l;

   for (l = pager_popups; l; l = l->next)
     {
	pp = l->data;
	if (pp->pager->zone == zone)
	  return pp;
     }
   return NULL;
}

static void
_pager_cb_obj_moveresize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;

   inst = data;
   _pager_instance_drop_zone_recalc(inst);
}

static void
_button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!pager_config->menu))
     {
	E_Menu *mn;
	E_Menu_Item *mi;
	int cx, cy, cw, ch;

	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _menu_cb_post, inst);
	pager_config->menu = mn;

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Configuration"));
	e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
	e_menu_item_callback_set(mi, _pager_inst_cb_menu_configure, NULL);

	if (e_configure_registry_exists("screen/virtual_desktops"))
	  {
	     mi = e_menu_item_new(mn);
	     e_menu_item_label_set(mi, _("Virtual Desktops Configuration"));
	     e_util_menu_item_edje_icon_set(mi, "enlightenment/vdesktops");
	     e_menu_item_callback_set(mi, _pager_inst_cb_menu_virtual_desktops_dialog, inst);
	  }

	e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);

	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	e_util_evas_fake_mouse_up_later(inst->gcc->gadcon->evas, ev->button);
     }
}

static void
_menu_cb_post(void *data, E_Menu *m)
{
   if (!pager_config->menu) return;
   e_object_del(E_OBJECT(pager_config->menu));
   pager_config->menu = NULL;
}

static void
_pager_inst_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!pager_config) return;
   if (pager_config->config_dialog) return;
   /* FIXME: pass zone config item */
   _config_pager_module(NULL);
}

static E_Config_Dialog *
_pager_config_dialog(E_Container *con, const char *params)
{
   if (!pager_config) return NULL;
   if (pager_config->config_dialog) return NULL;
   /* FIXME: pass zone config item */
   _config_pager_module(NULL);  
   return pager_config->config_dialog;
}

static void
_pager_inst_cb_menu_virtual_desktops_dialog(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;

   inst = data;
   e_configure_registry_call("screen/virtual_desktops", inst->gcc->gadcon->zone->container, NULL);
}

static void
_pager_instance_drop_zone_recalc(Instance *inst)
{
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(inst->o_pager, &x, &y, &w, &h);
   e_drop_handler_geometry_set(inst->drop_handler, x, y, w, h);
}

void
_pager_cb_config_updated(void)
{
   if (!pager_config) return;
}

static int
_pager_cb_event_border_resize(void *data, int type, void *event)
{
   E_Event_Border_Resize *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw) _pager_window_move(pw);
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_move(void *data, int type, void *event)
{
   E_Event_Border_Move *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw) _pager_window_move(pw);
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_add(void *data, int type, void *event)
{
   E_Event_Border_Add *ev;
   Evas_List *l;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;
	Pager_Desk *pd;

	inst = l->data;
	if ((inst->pager->zone != ev->border->zone) ||
	    (_pager_window_find(inst->pager, ev->border)))
	  continue;
	pd = _pager_desk_find(inst->pager, ev->border->desk);
	if (pd)
	  {
	     Pager_Win *pw;

	     pw = _pager_window_new(pd, ev->border);
	     if (pw) pd->wins = evas_list_append(pd->wins, pw);
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_remove(void *data, int type, void *event)
{
   E_Event_Border_Remove *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  pd->wins = evas_list_remove(pd->wins, pw);
		  _pager_window_free(pw);
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_iconify(void *data, int type, void *event)
{
   E_Event_Border_Iconify *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  if ((pw->drag.from_pager) && (pw->desk->pager->dragging))
		    pw->desk->pager->dragging = 0;
		  evas_object_hide(pw->o_window);
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_uniconify(void *data, int type, void *event)
{
   E_Event_Border_Uniconify *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw && !pw->skip_winlist) evas_object_show(pw->o_window);
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_stick(void *data, int type, void *event)
{
   E_Event_Border_Stick *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;
	Pager_Win *pw;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	pw = _pager_window_find(inst->pager, ev->border);
	if (!pw) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;

	     pd = l2->data;
	     if (ev->border->desk != pd->desk)
	       {
		  pw = _pager_window_new(pd, ev->border);
		  if (pw) pd->wins = evas_list_append(pd->wins, pw);
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_unstick(void *data, int type, void *event)
{
   E_Event_Border_Unstick *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;

	     pd = l2->data;
	     if (ev->border->desk != pd->desk)
	       {
		  Pager_Win *pw;

		  pw = _pager_desk_window_find(pd, ev->border);
		  if (pw)
		    {
		       pd->wins = evas_list_remove(pd->wins, pw);
		       _pager_window_free(pw);
		    }
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_desk_set(void *data, int type, void *event)
{
   E_Event_Border_Desk_Set *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance   *inst;
	Pager_Win  *pw;
	Pager_Desk *pd;

	inst = l->data;
	/* if this pager is not for the zone of the border */
	if (inst->pager->zone != ev->border->zone)
	  {
	     /* look at all desks in the pager */
	     for (l2 = inst->pager->desks; l2; l2 = l2->next)
	       {
		  pd = l2->data;
		  /* find this border in this desk */
		  pw = _pager_desk_window_find(pd, ev->border);
		  if (pw)
		    {
		       /* if it is found - remove it. it does not belong in this
			* pager as it probably moves zones */
		       pd->wins = evas_list_remove(pd->wins, pw);
		       _pager_window_free(pw);
		    }
	       }
	     continue;
	  }
	/* and this pager zone is for this border */
	/* see if the window is in this pager at all */
	pw = _pager_window_find(inst->pager, ev->border);
	if (pw)
	  {
	     /* is it sticky */
	     if (ev->border->sticky)
	       {
		  /* if its sticky and in this pager - its already everywhere, so abort
		   * doing anything else */
		  continue;
	       }
	     /* move it to the right desk */
	     /* find the pager desk of the target desk */
	     pd = _pager_desk_find(inst->pager, ev->border->desk);
	     if (pd)
	       {
		  Pager_Win *pw2 = NULL;
		  E_Border *bd;

		  /* remove it from whatever desk it was on */
		  pw->desk->wins = evas_list_remove(pw->desk->wins, pw);
		  e_layout_unpack(pw->o_window);

		  /* add it to the one its MEANT to be on */
		  pw->desk = pd;
		  pd->wins = evas_list_append(pd->wins, pw);
		  e_layout_pack(pd->o_layout, pw->o_window);

		  bd = e_util_desk_border_above(pw->border);
		  if (bd)
		    pw2 = _pager_desk_window_find(pd, bd);
		  if (pw2)
		    e_layout_child_lower_below(pw->o_window, pw2->o_window);
		  else
		    e_layout_child_raise(pw->o_window);

		  _pager_window_move(pw);
	       }
	  }
	/* the border isnt in this pager at all - it must have moved zones */
	else
	  {
	     if (!ev->border->sticky)
	       {
		  /* find the pager desk it needs to go to */
		  pd = _pager_desk_find(inst->pager, ev->border->desk);
		  if (pd)
		    {
		       /* create it and add it */
		       pw = _pager_window_new(pd, ev->border);
		       if (pw)
			 {
			    Pager_Win *pw2 = NULL;
			    E_Border *bd;

			    pd->wins = evas_list_append(pd->wins, pw);
			    bd = e_util_desk_border_above(pw->border);
			    if (bd)
			      pw2 = _pager_desk_window_find(pd, bd);
			    if (pw2)
			      e_layout_child_lower_below(pw->o_window, pw2->o_window);
			    else
			      e_layout_child_raise(pw->o_window);

			    _pager_window_move(pw);
			 }
		    }
	       }
	     else
	       {
		  /* go through all desks */
		  for (l2 = inst->pager->desks; l2; l2 = l2->next)
		    {
		       pd = l2->data;
		       /* create it and add it */
		       pw = _pager_window_new(pd, ev->border);
		       if (pw)
			 {
			    Pager_Win *pw2 = NULL;
			    E_Border *bd;

			    pd->wins = evas_list_append(pd->wins, pw);
			    bd = e_util_desk_border_above(pw->border);
			    if (bd)
			      pw2 = _pager_desk_window_find(pd, bd);
			    if (pw2)
			      e_layout_child_lower_below(pw->o_window, pw2->o_window);
			    else
			      e_layout_child_raise(pw->o_window);

			    _pager_window_move(pw);
			 }
		    }
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_stack(void *data, int type, void *event)
{
   E_Event_Border_Stack *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw, *pw2 = NULL;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  if (ev->stack)
		    {
		       pw2 = _pager_desk_window_find(pd, ev->stack);
		       if (!pw2)
			 {
			    /* This border is on another desk... */
			    E_Border *bd = NULL;

			    if (ev->type == E_STACKING_ABOVE)
			      bd = e_util_desk_border_below(ev->border);
			    else if (ev->type == E_STACKING_BELOW)
			      bd = e_util_desk_border_above(ev->border);
			    if (bd) pw2 = _pager_desk_window_find(pd, bd);
			 }
		    }
		  if (ev->type == E_STACKING_ABOVE)
		    {
		       if (pw2)
			 e_layout_child_raise_above(pw->o_window, pw2->o_window);
		       else
			 /* If we aren't above any window, we are at the bottom */
			 e_layout_child_lower(pw->o_window);
		    }
		  else if (ev->type == E_STACKING_BELOW)
		    {
		       if (pw2)
			 e_layout_child_lower_below(pw->o_window, pw2->o_window);
		       else
			 /* If we aren't below any window, we are at the top */
			 e_layout_child_raise(pw->o_window);
		    }
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_icon_change(void *data, int type, void *event)
{
   E_Event_Border_Icon_Change *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  Evas_Object *o;

		  if (pw->o_icon)
		    {
		       evas_object_del(pw->o_icon);
		       pw->o_icon = NULL;
		    }
		  o = e_border_icon_add
		    (ev->border, evas_object_evas_get(inst->pager->o_table));
		  if (o)
		    {
		       pw->o_icon = o;
		       evas_object_show(o);
		       edje_object_part_swallow(pw->o_window, "icon", o);
		    }
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_border_urgent_change(void *data, int type, void *event)
{
   E_Event_Border_Urgent_Change *ev;
   Evas_List *l, *l2;
   Pager_Popup *pp;
   E_Zone *zone;
   int urgent;
   int urgent_cnt = 0;

   ev = event;
   zone = ev->border->zone;
   urgent = ev->border->client.icccm.urgent;

   /* this didnt handle the case when two windows became urgent at
      the same time */

   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  if (urgent)
		    {
		       if (!(ev->border->iconic))
			 edje_object_signal_emit(pd->o_desk,
						 "e,state,urgent", "e");
		       edje_object_signal_emit(pw->o_window,
					       "e,state,urgent", "e");
		    }
		  else
		    {
		       /* FIXME this seems to be wrong - why set the
			  desk to not urgent when iconic, while
			  another window on that desk could still be urgent? */
		       if (!(ev->border->iconic))
			 edje_object_signal_emit(pd->o_desk,
						 "e,state,not_urgent", "e");
		       edje_object_signal_emit(pw->o_window,
					       "e,state,not_urgent", "e");
		    }
	       }
	  }
     }

   if (pager_config->popup_urgent)
     {
	pp = _pager_popup_find(zone);

	if ((!pp) && (urgent))
	  _pager_popup_new(zone, 1);
	else if (pp)
	  {
	     for (l = pp->pager->desks; l; l = l->next)
	       {
		  Pager_Desk *pd;
		  Pager_Win *pw;

		  pd = l->data;
		  pw = _pager_desk_window_find(pd, ev->border);
		  if (!pw) continue;

		  if (urgent)
		    {
		       edje_object_signal_emit(pd->o_desk,
					       "e,state,urgent", "e");
		       edje_object_signal_emit(pw->o_window,
					       "e,state,urgent", "e");
		       pd->urgent++;
		    }
		  else
		    {
		       pd->urgent--;

		       if (!(pd->urgent))
			 edje_object_signal_emit(pd->o_desk,
						 "e,state,not_urgent", "e");
		       edje_object_signal_emit(pw->o_window,
					       "e,state,not_urgent", "e");
		    }
		  urgent_cnt += pd->urgent;
	       }
	     if (urgent_cnt == 0) _pager_popup_free(pp);
	  }
     }

   return 1;
}

static int
_pager_cb_event_border_property(void *data, int type, void *event)
{
   E_Event_Border_Property *ev;
   Evas_List *l, *l2;
   int found = 0;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->border->zone) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;
	     Pager_Win *pw;

	     pd = l2->data;
	     pw = _pager_desk_window_find(pd, ev->border);
	     if (pw)
	       {
		  found = 1;
		  if (ev->border->client.netwm.state.skip_pager)
		    {
		       pd->wins = evas_list_remove(pd->wins, pw);
		       _pager_window_free(pw);
		    }
	       }
	  }
     }
   if (found) return 1;

   /* If we did not find this window in the pager, then add it because
    * the skip_pager state may have changed to 1 */
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;
	Pager_Desk *pd;

	inst = l->data;
	if ((inst->pager->zone != ev->border->zone) ||
	    (_pager_window_find(inst->pager, ev->border)))
	  continue;
	if (!ev->border->sticky)
	  {
	     pd = _pager_desk_find(inst->pager, ev->border->desk);
	     if (pd)
	       {
		  Pager_Win *pw;

		  pw = _pager_window_new(pd, ev->border);
		  if (pw)
		    {
		       Pager_Win *pw2 = NULL;
		       E_Border *bd;

		       pd->wins = evas_list_append(pd->wins, pw);
		       bd = e_util_desk_border_above(pw->border);
		       if (bd)
			 pw2 = _pager_desk_window_find(pd, bd);
		       if (pw2)
			 e_layout_child_lower_below(pw->o_window, pw2->o_window);
		       else
			 e_layout_child_raise(pw->o_window);
		       _pager_window_move(pw);
		    }
	       }
	  }
	else
	  {
	     for (l2 = inst->pager->desks; l2; l2 = l2->next)
	       {
		  Pager_Win *pw;

		  pd = l2->data;
		  pw = _pager_window_new(pd, ev->border);
		  if (pw)
		    {
		       Pager_Win *pw2 = NULL;
		       E_Border *bd;

		       pd->wins = evas_list_append(pd->wins, pw);
		       bd = e_util_desk_border_above(pw->border);
		       if (bd)
			 pw2 = _pager_desk_window_find(pd, bd);
		       if (pw2)
			 e_layout_child_lower_below(pw->o_window, pw2->o_window);
		       else
			 e_layout_child_raise(pw->o_window);
		       _pager_window_move(pw);
		    }
	       }
	  }
     }
   return 1;
}

static int
_pager_cb_event_zone_desk_count_set(void *data, int type, void *event)
{
   Evas_List *l;

   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	_pager_empty(inst->pager);
	_pager_fill(inst->pager);
	_gc_orient(inst->gcc);
     }
   return 1;
}

static int
_pager_cb_event_desk_show(void *data, int type, void *event)
{
   E_Event_Desk_Show *ev;
   Evas_List *l;
   Pager_Desk *pd;
   Pager_Popup *pp;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone != ev->desk->zone) continue;
	pd = _pager_desk_find(inst->pager, ev->desk);
	if (pd)  _pager_desk_select(pd);
     }

   if (pager_config->popup)
     {
	if (!(pp = _pager_popup_find(ev->desk->zone)))
	  _pager_popup_new(ev->desk->zone, 0);
	else
	  {
	     pd = _pager_desk_find(pp->pager, ev->desk);
	     if (pd) _pager_desk_select(pd);
	     ecore_timer_del(pp->timer);
	     pp->timer = ecore_timer_add(pager_config->popup_speed,
					 _pager_popup_cb_timeout, pp);
	  }
     }

   return 1;
}

static int
_pager_cb_event_desk_name_change(void *data, int type, void *event)
{
   E_Event_Desk_Name_Change *ev;
   Evas_List *l;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;
	Pager_Desk *pd;

	inst = l->data;
	if (inst->pager->zone != ev->desk->zone) continue;
	pd = _pager_desk_find(inst->pager, ev->desk);
	if (pd)
	  edje_object_part_text_set(pd->o_desk, "label", ev->desk->name);
     }
   return 1;
}

static int
_pager_cb_event_container_resize(void *data, int type, void *event)
{
   E_Event_Container_Resize *ev;
   Evas_List *l, *l2;

   ev = event;
   for (l = pager_config->instances; l; l = l->next)
     {
	Instance *inst;

	inst = l->data;
	if (inst->pager->zone->container != ev->container) continue;
	for (l2 = inst->pager->desks; l2; l2 = l2->next)
	  {
	     Pager_Desk *pd;

	     pd = l2->data;
	     e_layout_virtual_size_set(pd->o_layout, pd->desk->zone->w,
				       pd->desk->zone->h);
	  }
	_gc_orient(inst->gcc);
     }
   return 1;
}

static void
_pager_window_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Pager_Win *pw;

   ev = event_info;
   pw = data;
   /* FIXME: display window title in some tooltip thing */
}

static void
_pager_window_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Pager_Win *pw;

   ev = event_info;
   pw = data;
   /* FIXME: close tooltip */
}

static void
_pager_window_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Pager_Win *pw;

   ev = event_info;
   pw = data;


   if (!pw) return;
   if (pw->desk->pager->is_popup) return;
   if (pw->border->lock_user_location) return;
   if (ev->button == pager_config->btn_desk) return;
   if ((ev->button == pager_config->btn_drag) ||
       (ev->button == pager_config->btn_noplace))
     {
	Evas_Coord ox, oy;

	evas_object_geometry_get(pw->o_window, &ox, &oy, NULL, NULL);
	pw->drag.in_pager = 1;
	pw->drag.x = ev->canvas.x;
	pw->drag.y = ev->canvas.y;
	pw->drag.dx = ox - ev->canvas.x;
	pw->drag.dy = oy - ev->canvas.y;
	pw->drag.start = 1;
	pw->drag.no_place = 0;
	pw->drag.button = ev->button;
	if (ev->button == pager_config->btn_noplace)
	  pw->drag.no_place = 1;
     }
}

static void
_pager_window_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Pager_Win *pw;

   ev = event_info;
   pw = data;
   if (!pw) return;
   if (pw->desk->pager->is_popup) return;
   if (ev->button == pager_config->btn_desk) return;
   if ((ev->button == pager_config->btn_drag) ||
       (ev->button == pager_config->btn_noplace))
     {
	if (!pw->drag.from_pager)
	  {
	     if (!pw->drag.start) pw->desk->pager->just_dragged = 1;
	     pw->drag.in_pager = 0;
	     pw->drag.start = 0;
	     pw->desk->pager->dragging = 0;
	  }
     }
}

static void
_pager_window_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Pager_Win *pw;
   E_Drag *drag;
   Evas_Object *o, *oo;
   Evas_Coord x, y, w, h;
   const char *file = NULL, *part = NULL;
   const char *drag_types[] = { "enlightenment/pager_win", "enlightenment/border" };
   Evas_Coord dx, dy;
   unsigned int resist = 0;
   Evas_Coord mx, my, vx, vy;
   Pager_Desk *pd;

   ev = event_info;
   pw = data;

   if (!pw) return;
   if (pw->desk->pager->is_popup) return;
   /* prevent drag for a few pixels */
   if (pw->drag.start)
     {
	dx = pw->drag.x - ev->cur.output.x;
	dy = pw->drag.y - ev->cur.output.y;
	if (pw->desk && pw->desk->pager && pw->desk->pager->inst)
	  resist = pager_config->drag_resist;

	if (((dx * dx) + (dy * dy)) <= (resist * resist)) return;

	pw->desk->pager->dragging = 1;
	pw->drag.start = 0;
     }

   /* dragging this win around inside the pager */
   if (pw->drag.in_pager)
     {
	/* m for mouse */
	mx = ev->cur.canvas.x;
	my = ev->cur.canvas.y;

	/* find desk at pointer */
	pd = _pager_desk_at_coord(pw->desk->pager, mx, my);
	if ((pd) && (!pw->drag.no_place))
	  {
	     e_layout_coord_canvas_to_virtual(pd->o_layout,
					      mx + pw->drag.dx,
					      my + pw->drag.dy, &vx, &vy);
	     if (pd != pw->desk)
	       e_border_desk_set(pw->border, pd->desk);
	     e_border_move(pw->border,
			   vx + pd->desk->zone->x,
			   vy + pd->desk->zone->y);
	  }
	else
	  {
	     evas_object_geometry_get(pw->o_window, &x, &y, &w, &h);
	     evas_object_hide(pw->o_window);

	     drag = e_drag_new(pw->desk->pager->inst->gcc->gadcon->zone->container,
			       x, y, drag_types, 2, pw, -1,
			       _pager_window_cb_drag_convert,
			       _pager_window_cb_drag_finished);

	     o = edje_object_add(drag->evas);
	     edje_object_file_get(pw->o_window, &file, &part);
	     edje_object_file_set(o, file, part);

	     oo = o;
	     o = edje_object_add(drag->evas);
	     edje_object_file_get(pw->o_icon, &file, &part);
	     edje_object_file_set(o, file, part);
	     edje_object_part_swallow(oo, "icon", o);
	     e_drag_object_set(drag, oo);
	     e_drag_resize(drag, w, h);
	     e_drag_start(drag, x - pw->drag.dx, y - pw->drag.dy);

	     /* this prevents the desk from switching on drags */
	     pw->drag.from_pager = pw->desk->pager;
	     pw->drag.from_pager->dragging = 1;
	     pw->drag.in_pager = 0;
	     e_util_evas_fake_mouse_up_later(evas_object_evas_get(pw->desk->pager->o_table),
					     pw->drag.button);
	  }
     }
}

static void *
_pager_window_cb_drag_convert(E_Drag *drag, const char *type)
{
   Pager_Win *pw;

   pw = drag->data;
   if (!strcmp(type, "enlightenment/pager_win")) return pw;
   if (!strcmp(type, "enlightenment/border")) return pw->border;
   return NULL;
}

static void
_pager_window_cb_drag_finished(E_Drag *drag, int dropped)
{
   Pager_Win *pw;
   E_Container *cont;
   E_Zone *zone;
   E_Desk *desk;
   int x, y, dx, dy;

   pw = drag->data;
   if (!pw) return;
   evas_object_show(pw->o_window);
   if (!dropped)
     {
	/* wasn't dropped (on pager). move it to position of mouse on screen */
	cont = e_container_current_get(e_manager_current_get());
	zone = e_zone_current_get(cont);
	desk = e_desk_current_get(zone);

	e_border_zone_set(pw->border, zone);
	e_border_desk_set(pw->border, desk);

	ecore_x_pointer_last_xy_get(&x, &y);

	dx = (pw->border->w / 2);
	dy = (pw->border->h / 2);

	/* offset so that center of window is on mouse, but keep within desk bounds */
	if (dx < x)
	  {
	     x -= dx;
	     if ((pw->border->w < zone->w) &&
		 (x + pw->border->w > zone->x + zone->w))
	       x -= x + pw->border->w - (zone->x + zone->w);
	  }
	else x = 0;

	if (dy < y)
	  {
	     y -= dy;
	     if ((pw->border->h < zone->h) &&
		 (y + pw->border->h > zone->y + zone->h))
	       y -= y + pw->border->h - (zone->y + zone->h);
	  }
	else y = 0;
	e_border_move(pw->border, x, y);
     }
   if (pw && pw->drag.from_pager) pw->drag.from_pager->dragging = 0;
   pw->drag.from_pager = NULL;
}

static void
_pager_inst_cb_scroll(void *data)
{
   Instance * inst;

   inst = data;
   _pager_update_drop_position(inst, inst->pager->dnd_x, inst->pager->dnd_y);
}

static void
_pager_update_drop_position(Instance *inst, Evas_Coord x, Evas_Coord y)
{
   Pager_Desk *pd, *pd2;
   Evas_Coord xx, yy;
   int ox, oy;
   Evas_List *l;

   inst->pager->dnd_x = x;
   inst->pager->dnd_y = y;
   evas_object_geometry_get(inst->pager->o_table, &xx, &yy, NULL, NULL);
   e_box_align_pixel_offset_get(inst->gcc->o_box, &ox, &oy);
   pd = _pager_desk_at_coord(inst->pager, x + xx + ox, y + yy + oy);
   for (l = inst->pager->desks; l; l = l->next)
     {
	pd2 = l->data;
	if (pd == pd2)
	  edje_object_signal_emit(pd2->o_desk, "e,action,drag,in", "e");
	else if (pd2 == inst->pager->active_drop_pd)
	  edje_object_signal_emit(pd2->o_desk, "e,action,drag,out", "e");
     }
   inst->pager->active_drop_pd = pd;
}

static void
_pager_inst_cb_enter(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Enter *ev;
   Instance *inst;

   ev = event_info;
   inst = data;
   _pager_update_drop_position(inst, ev->x, ev->y);
   e_gadcon_client_autoscroll_cb_set(inst->gcc, _pager_inst_cb_scroll, inst);
   e_gadcon_client_autoscroll_update(inst->gcc, ev->x, ev->y);
}

static void
_pager_inst_cb_move(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Move *ev;
   Instance *inst;

   ev = event_info;
   inst = data;
   _pager_update_drop_position(inst, ev->x, ev->y);
   e_gadcon_client_autoscroll_update(inst->gcc, ev->x, ev->y);
}

static void
_pager_inst_cb_leave(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Leave *ev;
   Instance *inst;
   Evas_List *l;

   ev = event_info;
   inst = data;

   for (l = inst->pager->desks; l && inst->pager->active_drop_pd; l = l->next)
     {
	Pager_Desk *pd;

	pd = l->data;
	if (pd == inst->pager->active_drop_pd)
	  {
	     edje_object_signal_emit(pd->o_desk, "e,action,drag,out", "e");
	     inst->pager->active_drop_pd = NULL;
	  }
     }

   e_gadcon_client_autoscroll_cb_set(inst->gcc, NULL, NULL);
}

static void
_pager_inst_cb_drop(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Drop *ev;
   Instance *inst;
   Pager_Desk *pd;
   Pager_Desk *pd2 = NULL;
   E_Border *bd = NULL;
   Evas_List *l;
   int dx = 0, dy = 0;
   Pager_Win *pw = NULL;
   Evas_Coord xx, yy;
   int x, y;
   Evas_Coord wx, wy, wx2, wy2;
   Evas_Coord nx, ny;

   ev = event_info;
   inst = data;

   evas_object_geometry_get(inst->pager->o_table, &xx, &yy, NULL, NULL);
   e_box_align_pixel_offset_get(inst->gcc->o_box, &x, &y);
   pd = _pager_desk_at_coord(inst->pager, ev->x + xx + x, ev->y + yy + y);
   if (pd)
     {
	if (!strcmp(type, "enlightenment/pager_win"))
	  {
	     pw = (Pager_Win *)(ev->data);
	     if (pw)
	       {
		  bd = pw->border;
		  dx = pw->drag.dx;
		  dy = pw->drag.dy;
	       }
	  }
	else if (!strcmp(type, "enlightenment/border"))
	  {
	     bd = ev->data;
	     e_layout_coord_virtual_to_canvas(pd->o_layout, bd->x, bd->y, &wx, &wy);
	     e_layout_coord_virtual_to_canvas(pd->o_layout, bd->x + bd->w, bd->y + bd->h, &wx2, &wy2);
	     dx = (wx - wx2) / 2;
	     dy = (wy - wy2) / 2;
	  }
	else if (!strcmp(type, "enlightenment/vdesktop"))
	  {
	     pd2 = ev->data;
	     if (!pd2) return;
	     _pager_desk_switch(pd, pd2);
	  }
	else
	  return;

	if (bd)
	  {
	     if (bd->iconic) e_border_uniconify(bd);
	     e_border_desk_set(bd, pd->desk);
	     if ((!pw) || ((pw) && (!pw->drag.no_place)))
	       {
		  e_layout_coord_canvas_to_virtual(pd->o_layout,
						   ev->x + xx + x + dx,
						   ev->y + yy + y + dy,
						   &nx, &ny);
		  e_border_move(bd, nx + pd->desk->zone->x, ny + pd->desk->zone->y);
	       }
	  }
     }

   for (l = inst->pager->desks; l && inst->pager->active_drop_pd; l = l->next)
     {
	pd = l->data;
	if (pd == inst->pager->active_drop_pd)
	  {
	     edje_object_signal_emit(pd->o_desk, "e,action,drag,out", "e");
	     inst->pager->active_drop_pd = NULL;
	  }
     }

   e_gadcon_client_autoscroll_cb_set(inst->gcc, NULL, NULL);
}

static void
_pager_desk_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Pager_Desk *pd;
   Evas_Coord ox, oy;

   ev = event_info;
   pd = data;
   if (!pd) return;
   if (ev->button == pager_config->btn_desk)
     {
	evas_object_geometry_get(pd->o_desk, &ox, &oy, NULL, NULL);
	pd->drag.start = 1;
	pd->drag.in_pager = 1;
	pd->drag.dx = ox - ev->canvas.x;
	pd->drag.dy = oy - ev->canvas.y;
	pd->drag.x = ev->canvas.x;
	pd->drag.y = ev->canvas.y;
	pd->drag.button = ev->button;
     }
}

static void
_pager_desk_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Pager_Desk *pd;
   Pager *p;

   ev = event_info;
   pd = data;

   if (!pd) return;
   p = pd->pager;

   /* if ( p->is_popup &&
	(pd->desk == e_desk_current_get(pd->pager->zone)) )
     {
	if (p->inst->pager->popup)
	  _pager_popup_free(p->inst->pager->popup);
     }
   */
   /* FIXME: pd->pager->dragging is 0 when finishing a drag from desk to desk */
   if ((ev->button == 1) && (!pd->pager->dragging) &&
       (!pd->pager->just_dragged))
     {
	e_desk_show(pd->desk);
	pd->drag.start = 0;
	pd->drag.in_pager = 0;
     }
   pd->pager->just_dragged = 0;
}

static void
_pager_desk_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Pager_Desk *pd;
   Evas_Coord dx, dy;
   unsigned int resist = 0;
   E_Drag *drag;
   Evas_Object *o, *oo, *o_icon;
   Evas_Coord x, y, w, h;
   const char *drag_types[] = { "enlightenment/vdesktop" };
   Pager_Win *pw;
   Evas_List *l;

   ev = event_info;
   pd = data;
   if (!pd) return;
   /* prevent drag for a few pixels */
   if (pd->drag.start)
     {
	dx = pd->drag.x - ev->cur.output.x;
	dy = pd->drag.y - ev->cur.output.y;
	if (pd->pager && pd->pager->inst)
	  resist = pager_config->drag_resist;

	if (((dx * dx) + (dy * dy)) <= (resist * resist)) return;

	pd->pager->dragging = 1;
	pd->drag.start = 0;
     }

   if (pd->drag.in_pager)
     {
	evas_object_geometry_get(pd->o_desk, &x, &y, &w, &h);
	drag = e_drag_new(pd->pager->inst->gcc->gadcon->zone->container,
			  x, y, drag_types, 1, pd, -1,
			  NULL, _pager_desk_cb_drag_finished);

	/* set a background to the drag icon */
	o = evas_object_rectangle_add(drag->evas);
	evas_object_color_set(o, 255, 255, 255, 255);
	evas_object_resize(o, w, h);
	evas_object_show(o);

	/* redraw the desktop theme above */
	o = edje_object_add(drag->evas);
	e_theme_edje_object_set(o, "base/theme/modules/pager",
				"e/modules/pager/desk");
	evas_object_show(o);
	e_drag_object_set(drag, o);

	/* and redraw is content */
	oo = e_layout_add(drag->evas);
	e_layout_virtual_size_set(oo, pd->pager->zone->w, pd->pager->zone->h);
	edje_object_part_swallow(o, "items", oo);
	evas_object_show(oo);

	for (l = pd->wins; l; l = l->next)
	  {
	     pw = l->data;
	     if (!pw || pw->border->iconic
		 || pw->border->client.netwm.state.skip_pager)
	       continue;

	     o = edje_object_add(drag->evas);
	     e_theme_edje_object_set(o, "base/theme/modules/pager",
				     "e/modules/pager/window");
	     e_layout_pack(oo, o);
	     e_layout_child_raise(o);
	     e_layout_child_move(o,
				 pw->border->x - pw->desk->desk->zone->x,
				 pw->border->y - pw->desk->desk->zone->y);
	     e_layout_child_resize(o, pw->border->w, pw->border->h);
	     evas_object_show(o);

	     if ((o_icon = e_border_icon_add(pw->border, drag->evas)))
	       {
		  evas_object_show(o_icon);
		  edje_object_part_swallow(o, "icon", o_icon);
	       }
	  }
	e_drag_resize(drag, w, h);
	e_drag_start(drag, x - pd->drag.dx, y - pd->drag.dy);

	pd->drag.from_pager = pd->pager;
	pd->drag.from_pager->dragging = 1;
	pd->drag.in_pager = 0;
	e_util_evas_fake_mouse_up_later(evas_object_evas_get(pd->pager->o_table),
					pd->drag.button);
     }
}

static void
_pager_desk_cb_drag_finished(E_Drag *drag, int dropped)
{
   Pager_Desk *pd;
   Pager_Desk *pd2 = NULL;
   Evas_List *l;
   E_Desk *desk;
   E_Zone *zone;

   pd = drag->data;
   if (!pd) return;
   if (!dropped)
     {
	/* wasn't dropped on pager, switch with current desktop */
	if (!pd->desk) return;
	zone = e_util_zone_current_get(e_manager_current_get());
	desk = e_desk_current_get(zone);
	for (l = pager_config->instances; l && !pd2; l = l->next)
	  {
	     Instance *inst;
	     Pager *pager = NULL;

	     inst = l->data;
	     if (!(pager = inst->pager)) continue;
	     pd2 = _pager_desk_find(pager, desk);
	  }
	_pager_desk_switch(pd, pd2);
     }
   if (pd->drag.from_pager)
     {
	pd->drag.from_pager->dragging = 0;
	pd->drag.from_pager->just_dragged = 0;
     }
   pd->drag.from_pager = NULL;
}

static void
_pager_desk_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Pager_Desk *pd;

   ev = event_info;
   pd = data;

   if (pd->pager->is_popup) return;

   if (pager_config->flip_desk)
     e_zone_desk_linear_flip_by(pd->desk->zone, ev->z);
}

static int
_pager_popup_cb_timeout(void *data)
{
   Pager_Popup *pp;

   pp = data;
   _pager_popup_free(pp);
   return 0;
}

/***************************************************************************/
/**/
/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Pager"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   conf_edd = E_CONFIG_DD_NEW("Pager_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, popup, UINT);
   E_CONFIG_VAL(D, T, popup_speed, DOUBLE);
   E_CONFIG_VAL(D, T, popup_urgent, UINT);
   E_CONFIG_VAL(D, T, popup_urgent_stick, UINT);
   E_CONFIG_VAL(D, T, popup_urgent_speed, DOUBLE);
   E_CONFIG_VAL(D, T, popup_pager_height, INT);
   E_CONFIG_VAL(D, T, drag_resist, UINT);
   E_CONFIG_VAL(D, T, scale, UCHAR);
   E_CONFIG_VAL(D, T, resize, UCHAR);
   E_CONFIG_VAL(D, T, btn_drag, UCHAR);
   E_CONFIG_VAL(D, T, btn_noplace, UCHAR);
   E_CONFIG_VAL(D, T, btn_desk, UCHAR);
   E_CONFIG_VAL(D, T, flip_desk, UCHAR);

   pager_config = e_config_domain_load("module.pager", conf_edd);

   if (!pager_config)
     {
	pager_config = E_NEW(Config, 1);
	pager_config->popup = 1;
	pager_config->popup_speed = 1.0;
	pager_config->popup_urgent = 0;
	pager_config->popup_urgent_stick = 0;
	pager_config->popup_urgent_speed = 1.5;
	pager_config->popup_pager_height = 60;
	pager_config->drag_resist = 3;
	pager_config->scale = 1;
	pager_config->resize = PAGER_RESIZE_BOTH;
	pager_config->btn_drag = 1;
	pager_config->btn_noplace = 2;
	pager_config->btn_desk = 0;
	pager_config->flip_desk = 0;
     }
   E_CONFIG_LIMIT(pager_config->popup, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_speed, 0.1, 10.0);
   E_CONFIG_LIMIT(pager_config->popup_urgent, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_urgent_stick, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_urgent_speed, 0.1, 10.0);
   E_CONFIG_LIMIT(pager_config->popup_pager_height, 20, 200);
   E_CONFIG_LIMIT(pager_config->drag_resist, 0, 50);
   E_CONFIG_LIMIT(pager_config->resize, PAGER_RESIZE_HORZ, PAGER_RESIZE_BOTH);
   E_CONFIG_LIMIT(pager_config->flip_desk, 0, 1);
   E_CONFIG_LIMIT(pager_config->scale, 0, 1);
   E_CONFIG_LIMIT(pager_config->btn_drag, 0, 32);
   E_CONFIG_LIMIT(pager_config->btn_noplace, 0, 32);
   E_CONFIG_LIMIT(pager_config->btn_desk, 0, 32);

   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_RESIZE, _pager_cb_event_border_resize, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_MOVE, _pager_cb_event_border_move, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_ADD, _pager_cb_event_border_add, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_REMOVE, _pager_cb_event_border_remove, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_ICONIFY, _pager_cb_event_border_iconify, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_UNICONIFY, _pager_cb_event_border_uniconify, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_STICK, _pager_cb_event_border_stick, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_UNSTICK, _pager_cb_event_border_unstick, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_DESK_SET, _pager_cb_event_border_desk_set, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_STACK, _pager_cb_event_border_stack, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_ICON_CHANGE, _pager_cb_event_border_icon_change, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_URGENT_CHANGE,
       _pager_cb_event_border_urgent_change, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_BORDER_PROPERTY, _pager_cb_event_border_property, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_ZONE_DESK_COUNT_SET, _pager_cb_event_zone_desk_count_set, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_DESK_SHOW, _pager_cb_event_desk_show, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_DESK_NAME_CHANGE, _pager_cb_event_desk_name_change, NULL));
   pager_config->handlers = evas_list_append
     (pager_config->handlers, ecore_event_handler_add
      (E_EVENT_CONTAINER_RESIZE, _pager_cb_event_container_resize, NULL));

   pager_config->module = m;

   e_gadcon_provider_register(&_gadcon_class);

   e_configure_registry_item_add("extensions/pager", 40, N_("Pager"), NULL, 
				 "enlightenment/pager", _pager_config_dialog);

  return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   e_gadcon_provider_unregister(&_gadcon_class);

   if (pager_config->config_dialog)
     e_object_del(E_OBJECT(pager_config->config_dialog));
   while (pager_config->handlers)
     {
	ecore_event_handler_del(pager_config->handlers->data);
	pager_config->handlers = evas_list_remove_list(pager_config->handlers, pager_config->handlers);
     }

   if (pager_config->menu)
     {
	e_menu_post_deactivate_callback_set(pager_config->menu, NULL, NULL);
	e_object_del(E_OBJECT(pager_config->menu));
	pager_config->menu = NULL;
     }

   e_configure_registry_item_del("extensions/pager");   

   E_FREE(pager_config);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.pager", conf_edd, pager_config);
   return 1;
}
