/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "e_mod_main.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static char *_gc_label(E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(E_Gadcon_Client_Class *client_class);
/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
     "evry-starter",
     {
	_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
	e_gadcon_site_is_not_toolbar
     },
   E_GADCON_CLIENT_STYLE_PLAIN
};

typedef struct _Instance Instance;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_button;

   E_Object_Delfn *del_fn;
   Evry_Window *win;
};

static void _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;

   inst = E_NEW(Instance, 1);

   o = edje_object_add(gc->evas);
   e_theme_edje_object_set(o, "base/theme/modules/start", "e/modules/start/main");
   edje_object_signal_emit(o, "e,state,unfocused", "e");

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->o_button = o;

   e_gadcon_client_util_menu_attach(gcc);

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
				  _button_cb_mouse_down, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   if (inst->del_fn && inst->win)
     e_object_delfn_del(E_OBJECT(inst->win->popup), inst->del_fn);

   evas_object_del(inst->o_button);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst;
   Evas_Coord mw, mh;

   inst = gcc->data;
   mw = 0, mh = 0;
   edje_object_size_min_get(inst->o_button, &mw, &mh);
   if ((mw < 1) || (mh < 1))
     edje_object_size_min_calc(inst->o_button, &mw, &mh);
   if (mw < 4) mw = 4;
   if (mh < 4) mh = 4;
   e_gadcon_client_aspect_set(gcc, mw, mh);
   e_gadcon_client_min_size_set(gcc, mw, mh);
}

static char *
_gc_label(E_Gadcon_Client_Class *client_class)
{
   return _("Everything Starter");
}

static Evas_Object *
_gc_icon(E_Gadcon_Client_Class *client_class, Evas *evas)
{
   Evas_Object *o;
   /* char buf[4096];
    *
    * o = edje_object_add(evas);
    * snprintf(buf, sizeof(buf), "%s/e-module-start.edj",
    * 	    e_module_dir_get(start_module));
    * edje_object_file_set(o, buf, "icon"); */
   return NULL;
}

static const char *
_gc_id_new(E_Gadcon_Client_Class *client_class)
{
   return _gadcon_class.name;
}

/***************************************************************************/


static void _del_func(void *data, void *obj)
{
   Instance *inst = data;

   e_gadcon_locked_set(inst->gcc->gadcon, 0);
   e_object_delfn_del(E_OBJECT(inst->win->popup), inst->del_fn);
   inst->del_fn = NULL;
   inst->win = NULL;
   edje_object_signal_emit(inst->o_button, "e,state,unfocused", "e");
}

static void
_button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if (ev->button == 1)
     {
	Evas_Coord x, y, w, h;
	int cx, cy, px, py, pw, ph;
	Evry_Window *win;
	int dir;

	win = evry_show(e_util_zone_current_get(e_manager_current_get()), 0, "Start");
	if (!win) return;

	evas_object_geometry_get(inst->o_button, &x, &y, &w, &h);
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon,
					  &cx, &cy, NULL, NULL);
	x += cx;
	y += cy;
	pw = win->popup->w;
	ph = win->popup->h;

	switch (inst->gcc->gadcon->orient)
	  {

	   case E_GADCON_ORIENT_TOP:
	   case E_GADCON_ORIENT_CORNER_TL:
	   case E_GADCON_ORIENT_CORNER_TR:
	      e_popup_move(win->popup, x, y + h);
	      break;
	   case E_GADCON_ORIENT_BOTTOM:
	   case E_GADCON_ORIENT_CORNER_BR:
	   case E_GADCON_ORIENT_CORNER_BL:
	      e_popup_move(win->popup, x, y - ph);
	      break;
	   case E_GADCON_ORIENT_LEFT:
	   case E_GADCON_ORIENT_CORNER_LT:
	   case E_GADCON_ORIENT_CORNER_LB:
	      e_popup_move(win->popup, x + w, y);
	      break;
	   case E_GADCON_ORIENT_RIGHT:
	   case E_GADCON_ORIENT_CORNER_RT:
	   case E_GADCON_ORIENT_CORNER_RB:
	      e_popup_move(win->popup, x - pw, y);
	      break;
	   case E_GADCON_ORIENT_FLOAT:
	   case E_GADCON_ORIENT_HORIZ:
	   case E_GADCON_ORIENT_VERT:
	   default:
	      break;
	  }

	inst->win = win;

	if (win->popup->x + pw > win->popup->zone->w)
	  e_popup_move(win->popup, win->popup->zone->w - pw, win->popup->y);

	if (win->popup->y + ph > win->popup->zone->h)
	  e_popup_move(win->popup, win->popup->x, win->popup->zone->h - ph);

	e_gadcon_locked_set(inst->gcc->gadcon, 1);

	inst->del_fn = e_object_delfn_add(E_OBJECT(win->popup), _del_func, inst);

	edje_object_signal_emit(inst->o_button, "e,state,focused", "e");
     }
}


int
evry_gadget_init(void)
{
   e_gadcon_provider_register(&_gadcon_class);
   return 1;
}

void
evry_gadget_shutdown(void)
{
   e_gadcon_provider_unregister(&_gadcon_class);
}
