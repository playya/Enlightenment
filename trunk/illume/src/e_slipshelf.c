#include <e.h>
#include "e_slipshelf.h"
#include "e_winilist.h"
#include "e_cfg.h"

EAPI int E_EVENT_SLIPSHELF_ADD = 0;
EAPI int E_EVENT_SLIPSHELF_DEL = 0;
EAPI int E_EVENT_SLIPSHELF_CHANGE = 0;

/* internal calls */

E_Slipshelf *_e_slipshelf_new(E_Zone *zone, const char *themedir);
static void _e_slipshelf_free(E_Slipshelf *ess);
static void _e_slipshelf_cb_toggle(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_home(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_close(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_apps(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_applist_add(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_applist_del(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_keyboard(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_slipshelf_cb_item_sel(void *data, void *data2);
static int _e_slipshelf_cb_animate(void *data);
static void _e_slipshelf_slide(E_Slipshelf *ess, int out, double len);
static int _e_slipshelf_cb_mouse_up(void *data, int type, void *event);
static int _e_slipshelf_cb_zone_move_resize(void *data, int type, void *event);
static int _e_slipshelf_cb_zone_del(void *data, int type, void *event);
static void _e_slipshelf_event_simple_free(void *data, void *ev);
static void _e_slipshelf_object_del_attach(void *o);
static int _e_slipshelf_cb_border_focus_in(void *data, int type, void *event);
static int _e_slipshelf_cb_border_focus_out(void *data, int type, void *event);
static int _e_slipshelf_cb_border_property(void *data, int type, void *event);
static void _e_slipshelf_title_update(E_Slipshelf *ess);
static void _e_slipshelf_cb_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static void _e_slipshelf_cb_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static Evas_Object *_e_slipshelf_cb_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style);

static Evas_Object *_theme_obj_new(Evas *e, const char *custom_dir, const char *group);

/* state */
static Evas_List *slipshelves = NULL;

/* called from the module core */
EAPI int
e_slipshelf_init(void)
{
   e_winilist_init();
   E_EVENT_SLIPSHELF_ADD = ecore_event_type_new();
   E_EVENT_SLIPSHELF_DEL = ecore_event_type_new();
   E_EVENT_SLIPSHELF_CHANGE = ecore_event_type_new();
   return 1;
}

EAPI int
e_slipshelf_shutdown(void)
{
   E_Config_Dialog *cfd;
   
   e_winilist_shutdown();
   return 1;
}

EAPI E_Slipshelf *
e_slipshelf_new(E_Zone *zone, const char *themedir)
{
   E_Slipshelf *ess;
   Evas_Coord mw, mh, vx, vy, vw, vh, w, h;
   int x, y;
   Evas_Object *o;

   ess = E_OBJECT_ALLOC(E_Slipshelf, E_SLIPSHELF_TYPE, _e_slipshelf_free);
   if (!ess) return NULL;
   
   ess->zone = zone;
   if (themedir) ess->themedir = evas_stringshare_add(themedir);
   
   ess->clickwin = ecore_x_window_input_new(zone->container->win,
					    zone->x, zone->y, zone->w, zone->h);
   ess->popup = e_popup_new(ess->zone, -1, -1, 1, 1);
   ecore_x_window_configure(ess->clickwin, 
			    ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING|ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
			    0, 0, 0, 0, 0,
			    ess->popup->evas_win,
			    ECORE_X_WINDOW_STACK_BELOW);
   e_popup_layer_set(ess->popup, 200);

   ess->main_size = illume_cfg->slipshelf.main_gadget_size * e_scale / 2.0;
   ess->extra_size = illume_cfg->slipshelf.extra_gagdet_size * e_scale / 2.0;
   
   ess->base_obj = _theme_obj_new(ess->popup->evas,
				  ess->themedir,
				  "e/modules/slipshelf/base/default");
   if (0)
     {
	ess->control_obj = _theme_obj_new(ess->popup->evas,
					  ess->themedir,
					  "e/modules/slipshelf/controls/default");
     }
   else
     {
	ess->control_obj = _theme_obj_new(ess->popup->evas,
					  ess->themedir,
					  "e/modules/slipshelf/controls/applist");
	edje_object_part_text_set(ess->control_obj, "e.add.label",
				  "ADD");
	edje_object_part_text_set(ess->control_obj, "e.del.label",
				  "REMOVE");
	edje_object_part_text_set(ess->base_obj, "e.add.label",
				  "ADD");
	edje_object_part_text_set(ess->base_obj, "e.del.label",
				  "REMOVE");
     }
   edje_object_part_swallow(ess->base_obj, "e.swallow.controls",
			    ess->control_obj);
   evas_object_show(ess->control_obj);

   ess->focused_border = e_border_focused_get();
   _e_slipshelf_title_update(ess);
   
   o = evas_object_rectangle_add(ess->popup->evas);
   evas_object_color_set(o, 0, 0, 0, 0);
   edje_object_part_swallow(ess->base_obj, "e.swallow.visible", o);
   ess->vis_obj = o;
   
   o = evas_object_rectangle_add(ess->popup->evas);
   evas_object_color_set(o, 0, 0, 0, 0);
   edje_extern_object_min_size_set(o, ess->extra_size, ess->extra_size);
   edje_object_part_swallow(ess->base_obj, "e.swallow.extra", o);
   ess->swallow1_obj = o;
   
   o = evas_object_rectangle_add(ess->popup->evas);
   evas_object_color_set(o, 0, 0, 0, 0);
   edje_extern_object_min_size_set(o, ess->main_size, ess->main_size);
   edje_object_part_swallow(ess->base_obj, "e.swallow.content", o);
   ess->swallow2_obj = o;
   
   edje_object_size_min_calc(ess->base_obj, &mw, &mh);
   
   evas_object_resize(ess->base_obj, mw, mh);
   edje_object_part_geometry_get(ess->base_obj, "e.swallow.visible", &vx, &vy, &vw, &vh);
//   evas_object_geometry_get(ess->vis_obj, &vx, &vy, &vw, &vh);

   evas_object_del(ess->swallow1_obj);
   ess->gadcon_extra = e_gadcon_swallowed_new("slipshelf_extra", 0, ess->base_obj, "e.swallow.extra");
   ess->gadcon_extra->instant_edit = 1;
   edje_extern_object_min_size_set(ess->gadcon_extra->o_container, ess->extra_size, ess->extra_size);
   edje_object_part_swallow(ess->base_obj, "e.swallow.extra", ess->gadcon_extra->o_container);
   
   e_gadcon_min_size_request_callback_set(ess->gadcon_extra, _e_slipshelf_cb_gadcon_min_size_request, ess);
   e_gadcon_size_request_callback_set(ess->gadcon_extra, _e_slipshelf_cb_gadcon_size_request, ess);
   e_gadcon_frame_request_callback_set(ess->gadcon_extra, _e_slipshelf_cb_gadcon_frame_request, ess);
   e_gadcon_orient(ess->gadcon_extra, E_GADCON_ORIENT_TOP);
   e_gadcon_zone_set(ess->gadcon_extra, ess->zone);
   e_gadcon_ecore_evas_set(ess->gadcon_extra, ess->popup->ecore_evas);

   evas_object_del(ess->swallow2_obj);
   ess->gadcon = e_gadcon_swallowed_new("slipshelf", 0, ess->base_obj, "e.swallow.content");
   ess->gadcon->instant_edit = 1;
   edje_extern_object_min_size_set(ess->gadcon->o_container, ess->main_size, ess->main_size);
   edje_object_part_swallow(ess->base_obj, "e.swallow.content", ess->gadcon->o_container);
   
   e_gadcon_min_size_request_callback_set(ess->gadcon, _e_slipshelf_cb_gadcon_min_size_request, ess);
   e_gadcon_size_request_callback_set(ess->gadcon, _e_slipshelf_cb_gadcon_size_request, ess);
   e_gadcon_frame_request_callback_set(ess->gadcon, _e_slipshelf_cb_gadcon_frame_request, ess);
   e_gadcon_orient(ess->gadcon, E_GADCON_ORIENT_TOP);
   e_gadcon_zone_set(ess->gadcon, ess->zone);
   e_gadcon_ecore_evas_set(ess->gadcon, ess->popup->ecore_evas);
   
   printf("### v... = %i %i %ix%i\n", vx, vy, vw, vh);
   
   ess->hidden = vy;
   x = zone->x;
   y = zone->y - ess->hidden;
   mw = zone->w;
   e_popup_move_resize(ess->popup, x, y, mw, mh);

   evas_object_resize(ess->base_obj, ess->popup->w, ess->popup->h);
   e_popup_edje_bg_object_set(ess->popup, ess->base_obj);

   if (0)
     {
     }
   else
     {
	o = e_winilist_add(ess->popup->evas);
	edje_object_part_swallow(ess->control_obj, "e.swallow.content", o);
	ess->scrollframe_obj = o;
	e_winilist_border_select_callback_set(o, 
					      _e_slipshelf_cb_item_sel,
					      ess);
	e_winilist_special_append(o, NULL, "Home", 
				  _e_slipshelf_cb_item_sel,
				  ess, NULL);
	evas_object_show(o);
/*	
	o = e_scrollframe_add(ess->popup->evas);
	edje_object_part_swallow(ess->control_obj, "e.swallow.content", o);
	evas_object_show(o);
	ess->scrollframe_obj = o;
	
	o = e_ilist_add(ess->popup->evas);
	e_ilist_selector_set(o, 1);
	
	e_ilist_min_size_get(o, &mw, &mh);
	evas_object_resize(o, mw, mh);
	
	e_scrollframe_child_set(ess->scrollframe_obj, o);
	e_scrollframe_child_viewport_size_get(ess->scrollframe_obj, &vw, &vh);
	evas_object_geometry_get(ess->scrollframe_obj, NULL, NULL, &w, &h);
	if (mw > vw) mw = mw + (w - vw);
	else if (mw < vw) evas_object_resize(o, vw, mh);
        evas_object_show(o);
	ess->ilist_obj = o;
 */
     }
   
   evas_object_show(ess->base_obj);

   edje_object_signal_callback_add(ess->base_obj, "e,action,toggle", "", _e_slipshelf_cb_toggle, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,keyboard", "", _e_slipshelf_cb_keyboard, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,home", "", _e_slipshelf_cb_home, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,close", "", _e_slipshelf_cb_close, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,apps", "", _e_slipshelf_cb_apps, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,applist,add", "", _e_slipshelf_cb_applist_add, ess);
   edje_object_signal_callback_add(ess->base_obj, "e,action,do,applist,del", "", _e_slipshelf_cb_applist_del, ess);
   
   edje_object_signal_callback_add(ess->control_obj, "e,action,toggle", "", _e_slipshelf_cb_toggle, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,keyboard", "", _e_slipshelf_cb_keyboard, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,home", "", _e_slipshelf_cb_home, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,close", "", _e_slipshelf_cb_close, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,apps", "", _e_slipshelf_cb_apps, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,applist,add", "", _e_slipshelf_cb_applist_add, ess);
   edje_object_signal_callback_add(ess->control_obj, "e,action,do,applist,del", "", _e_slipshelf_cb_applist_del, ess);
   
   /* FIXME: add callbacks for app remove/add */
   
   e_popup_show(ess->popup);

   slipshelves = evas_list_append(slipshelves, ess);

   ess->handlers = evas_list_append
     (ess->handlers,
      ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP,
			      _e_slipshelf_cb_mouse_up, ess));
   ess->handlers = evas_list_append
     (ess->handlers,
      ecore_event_handler_add(E_EVENT_BORDER_FOCUS_IN,
			      _e_slipshelf_cb_border_focus_in, ess));
   ess->handlers = evas_list_append
     (ess->handlers,
      ecore_event_handler_add(E_EVENT_BORDER_FOCUS_OUT,
			      _e_slipshelf_cb_border_focus_out, ess));
   ess->handlers = evas_list_append
     (ess->handlers,
      ecore_event_handler_add(E_EVENT_BORDER_PROPERTY,
			      _e_slipshelf_cb_border_property, ess));
   ess->handlers = evas_list_append
     (ess->handlers,
      ecore_event_handler_add(E_EVENT_ZONE_MOVE_RESIZE,
			      _e_slipshelf_cb_zone_move_resize, ess));
   /* FIXME: handle:
    * 
    * E_EVENT_BORDER_URGENT_CHANGE
    * 
    */
							   
   ess->action.home.enabled = 1;
   ess->action.close.enabled = 1;
   ess->action.apps.enabled = 1;
   ess->action.keyboard.enabled = 1;
   
     {
	E_Event_Slipshelf_Add *ev;
	
	ev = calloc(1, sizeof(E_Event_Slipshelf_Add));
	ev->slipshelf = ess;
	e_object_ref(E_OBJECT(ess));
	ecore_event_add(E_EVENT_SLIPSHELF_ADD, ev,
			_e_slipshelf_event_simple_free, NULL);
     }
   
   e_object_del_attach_func_set(E_OBJECT(ess), _e_slipshelf_object_del_attach);

   e_gadcon_populate(ess->gadcon_extra);
   e_gadcon_populate(ess->gadcon);
   return ess;
}

EAPI void
e_slipshelf_action_enabled_set(E_Slipshelf *ess, E_Slipshelf_Action action, Evas_Bool enabled)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   switch (action)
     {
      case E_SLIPSHELF_ACTION_HOME:
 	if (ess->action.home.enabled != enabled)
	  {
	     ess->action.home.enabled = enabled;
	     if (enabled)
	       edje_object_signal_emit(ess->control_obj, "e,state,action,home,enabled", "e");
	     else
	       edje_object_signal_emit(ess->control_obj, "e,state,action,home,disabled", "e");
	  }
	break;
      case E_SLIPSHELF_ACTION_CLOSE:
 	if (ess->action.close.enabled != enabled)
	  {
	     ess->action.close.enabled = enabled;
	     if (enabled)
	       edje_object_signal_emit(ess->control_obj, "e,state,action,close,enabled", "e");
	     else
	       edje_object_signal_emit(ess->control_obj, "e,state,action,close,disabled", "e");
	  }
	break;
      case E_SLIPSHELF_ACTION_APPS:
 	if (ess->action.apps.enabled != enabled)
	  {
	     ess->action.apps.enabled = enabled;
	     if (enabled)
	       edje_object_signal_emit(ess->control_obj, "e,state,action,apps,enabled", "e");
	     else
	       edje_object_signal_emit(ess->control_obj, "e,state,action,apps,disabled", "e");
	  }
	break;
      case E_SLIPSHELF_ACTION_KEYBOARD:
 	if (ess->action.keyboard.enabled != enabled)
	  {
	     ess->action.keyboard.enabled = enabled;
	     if (enabled)
	       edje_object_signal_emit(ess->control_obj, "e,state,action,keyboard,enabled", "e");
	     else
	       edje_object_signal_emit(ess->control_obj, "e,state,action,keyboard,disabled", "e");
	  }
	break;
      default:
	break;
     }
}

EAPI Evas_Bool
e_slipshelf_action_enabled_get(E_Slipshelf *ess, E_Slipshelf_Action action)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK_RETURN(ess, E_SLIPSHELF_TYPE, 0);
   switch (action)
     {
      case E_SLIPSHELF_ACTION_HOME:
	return ess->action.home.enabled;
	break;
      case E_SLIPSHELF_ACTION_CLOSE:
	return ess->action.home.enabled;
	break;
      case E_SLIPSHELF_ACTION_APPS:
	return ess->action.home.enabled;
	break;
      case E_SLIPSHELF_ACTION_KEYBOARD:
	return ess->action.keyboard.enabled;
	break;
      default:
	break;
     }
   return 0;
}

EAPI void
e_slipshelf_action_callback_set(E_Slipshelf *ess, E_Slipshelf_Action action, void (*func) (const void *data, E_Slipshelf *ess, E_Slipshelf_Action action), const void *data)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   switch (action)
     {
      case E_SLIPSHELF_ACTION_HOME:
	ess->action.home.func = func;
	ess->action.home.data = data;
	break;
      case E_SLIPSHELF_ACTION_CLOSE:
	ess->action.close.func = func;
	ess->action.close.data = data;
	break;
      case E_SLIPSHELF_ACTION_APPS:
	ess->action.apps.func = func;
	ess->action.apps.data = data;
	break;
      case E_SLIPSHELF_ACTION_KEYBOARD:
	ess->action.keyboard.func = func;
	ess->action.keyboard.data = data;
	break;
      default:
	break;
     }
}

EAPI void
e_slipshelf_safe_app_region_get(E_Zone *zone, int *x, int *y, int *w, int *h)
{
   Evas_List *l;   
   int sx, sy, sw, sh;
   
   sx = zone->x;
   sy = zone->y;
   sw = zone->w;
   sh = zone->h;
   for (l = slipshelves; l; l = l->next)
     {
	E_Slipshelf *ess;
	
	ess = l->data;
	if (e_object_is_del(E_OBJECT(ess))) continue;
	if (ess->zone == zone)
	  {
	     sh -= (ess->popup->h - ess->hidden);
	     sy += (ess->popup->h - ess->hidden);
	     break;
	  }
     }
   if (x) *x = sx;
   if (y) *y = sy;
   if (w) *w = sw;
   if (h) *h = sh;
}

EAPI void
e_slipshelf_default_title_set(E_Slipshelf *ess, const char *title)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   if (ess->default_title) evas_stringshare_del(ess->default_title);
   if (title)
     ess->default_title = evas_stringshare_add(title);
   else
     ess->default_title = NULL;
   if (!ess->focused_border)
     edje_object_part_text_set(ess->base_obj, "e.text.label",
			       ess->default_title);
}

EAPI void
e_slipshelf_border_select_callback_set(E_Slipshelf *ess, void (*func) (void *data, E_Slipshelf *ess, E_Border *bd), const void *data)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   ess->callback_border_select.func = func;
   ess->callback_border_select.data = data;
}

EAPI void
e_slipshelf_border_add_callback_set(E_Slipshelf *ess, void (*func) (void *data, E_Slipshelf *ess, E_Border *bd), const void *data)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   ess->callback_border_add.func = func;
   ess->callback_border_add.data = data;
}

EAPI void
e_slipshelf_border_del_callback_set(E_Slipshelf *ess, void (*func) (void *data, E_Slipshelf *ess, E_Border *bd), const void *data)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   ess->callback_border_del.func = func;
   ess->callback_border_del.data = data;
}

EAPI void
e_slipshelf_border_home_callback_set(E_Slipshelf *ess, void (*func) (void *data, E_Slipshelf *ess, E_Border *bd), const void *data)
{
   E_OBJECT_CHECK(ess);
   E_OBJECT_TYPE_CHECK(ess, E_SLIPSHELF_TYPE);
   ess->callback_border_home.func = func;
   ess->callback_border_home.data = data;
}

/* internal calls */
static void
_e_slipshelf_free(E_Slipshelf *ess)
{
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   slipshelves = evas_list_remove(slipshelves, ess);
   e_object_del(E_OBJECT(ess->gadcon));
   e_object_del(E_OBJECT(ess->gadcon_extra));
   while (ess->handlers)
     {
	if (ess->handlers->data)
	  ecore_event_handler_del(ess->handlers->data);
	ess->handlers = evas_list_remove_list(ess->handlers, ess->handlers);
     }
   if (ess->animator) ecore_animator_del(ess->animator);
   if (ess->themedir) evas_stringshare_del(ess->themedir);
   if (ess->default_title) evas_stringshare_del(ess->default_title);
   if (ess->clickwin) ecore_x_window_del(ess->clickwin);
   e_object_del(E_OBJECT(ess->popup));
   free(ess);
}

static void
_e_slipshelf_cb_toggle(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   if (ess->out) _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
   else _e_slipshelf_slide(ess, 1, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
}

static void
_e_slipshelf_cb_home(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
   if ((ess->action.home.func) && (ess->action.home.enabled))
     ess->action.home.func(ess->action.home.data, ess, E_SLIPSHELF_ACTION_HOME);
}

static void
_e_slipshelf_cb_close(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
   if ((ess->action.close.func) && (ess->action.close.enabled))
     ess->action.close.func(ess->action.close.data, ess, E_SLIPSHELF_ACTION_CLOSE);
}

static void
_e_slipshelf_cb_apps(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
   if ((ess->action.apps.func) && (ess->action.apps.enabled))
     ess->action.apps.func(ess->action.apps.data, ess, E_SLIPSHELF_ACTION_APPS);
}

static void
_e_slipshelf_cb_applist_add(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->callback_border_add.func)
     {
	if (ess->bsel)
	  ess->callback_border_add.func(ess->callback_border_add.data, ess, ess->bsel);
	else
	  ess->callback_border_add.func(ess->callback_border_add.data, ess, e_border_focused_get());
     }
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
}

static void
_e_slipshelf_cb_applist_del(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if (ess->callback_border_del.func)
     {
	if (ess->bsel)
	  ess->callback_border_del.func(ess->callback_border_del.data, ess, ess->bsel);
	else
	  ess->callback_border_del.func(ess->callback_border_del.data, ess, e_border_focused_get());
     }
   if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
   ess->slide_down_timer = NULL;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
}

static void
_e_slipshelf_cb_keyboard(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Slipshelf *ess;
   
   ess = data;
   if ((ess->action.keyboard.func) && (ess->action.keyboard.enabled))
     ess->action.keyboard.func(ess->action.keyboard.data, ess, E_SLIPSHELF_ACTION_KEYBOARD);
}

static int
_e_slipshelf_cb_slide_down_delay(void *data)
{
   E_Slipshelf *ess;
   
   ess = data;
   _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
   ess->slide_down_timer = NULL;
   return 0;
}

static void
_e_slipshelf_cb_item_sel(void *data, void *data2)
{
   E_Slipshelf *ess;
   E_Border *bd;
   
   ess = data;
   bd = data2;
   ess->bsel = bd;
   if (bd)
     {
	if (e_border_focused_get() == bd)
	  {
	     if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
	     ess->slide_down_timer = ecore_timer_add(0.5, _e_slipshelf_cb_slide_down_delay, ess);
//	     _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
	     return;
	  }
	if (ess->callback_border_select.func)
	  ess->callback_border_select.func(ess->callback_border_select.data, ess, bd);
	if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
	ess->slide_down_timer = ecore_timer_add(0.5, _e_slipshelf_cb_slide_down_delay, ess);
//	_e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
     }
   else
     {
	if (ess->callback_border_home.func)
	  ess->callback_border_home.func(ess->callback_border_home.data, ess, bd);
	if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
	ess->slide_down_timer = ecore_timer_add(0.5, _e_slipshelf_cb_slide_down_delay, ess);
//	_e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
     }
}

static void
_e_slipshelf_applist_update(E_Slipshelf *ess)
{
   Evas_Coord mw, mh, vx, vy, vw, vh, w, h, sfmw, sfmh, cmw, cmh, smw, smh;
//   Evas_List *borders, *l;
   int i, selnum, x, y;
   int pw, ph;
   
   i = 0;

   pw = ess->popup->w;
   ph = ess->popup->h;
   /*
   borders = e_border_client_list();
   e_ilist_freeze(ess->ilist_obj);
   for (l = borders; l; l = l->next)
     {
	E_Border *bd;
	const char *title;
	
	bd = l->data;
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	
	e_object_ref(E_OBJECT(bd));
	title = "???";
	if (bd->client.netwm.name) title = bd->client.netwm.name;
	else if (bd->client.icccm.title) title = bd->client.icccm.title;
	e_ilist_append(ess->ilist_obj, NULL, title, 0, 
		       _e_slipshelf_cb_item_sel,
		       NULL, ess, bd);
	ess->borders = evas_list_append(ess->borders, bd);
	if (bd == e_border_focused_get()) selnum = i;
	i++;
     }
   e_ilist_append(ess->ilist_obj, NULL, "Home", 0, 
		  _e_slipshelf_cb_item_sel,
		  NULL, ess, NULL);
//   selnum = 0;
   i++;
   e_ilist_thaw(ess->ilist_obj);
*/
   ess->bsel = e_border_focused_get();
   
//   if (selnum >= 0) e_ilist_selected_set(ess->ilist_obj, selnum);

   printf("-------------------\n");
   e_winilist_optimial_size_get(ess->scrollframe_obj, &sfmw, &sfmh);
/*   
   e_scrollframe_child_viewport_size_get(ess->scrollframe_obj, &vw, &vh);
   e_ilist_min_size_get(ess->ilist_obj, &mw, &mh);
   
   evas_object_geometry_get(ess->scrollframe_obj, NULL, NULL, &w, &h);
   
   if (mw > vw) mw = mw + (w - vw);
   else if (mw < vw) mw = vw;
   
   evas_object_resize(ess->ilist_obj, mw, mh);
   evas_object_resize(ess->scrollframe_obj, mw, mh);
   printf("resize ilist + frame to %i %i\n", mw, mh);
   
   evas_object_geometry_get(ess->scrollframe_obj, NULL, NULL, &w, &h);
   sfmw = mw + (mw - w);
   sfmh = mh + (mh - h);
   printf("adjust to %i %i\n", sfmw, sfmh);
 */   

   sfmw = 0;
   
   edje_extern_object_min_size_set(ess->scrollframe_obj, sfmw, sfmh);
   edje_object_part_swallow(ess->control_obj, "e.swallow.content",
			    ess->scrollframe_obj);   
   edje_object_size_min_calc(ess->control_obj, &cmw, &cmh);
   printf("control min %i %i\n", cmw, cmh);

   edje_extern_object_min_size_set(ess->control_obj, cmw, cmh);
   edje_object_part_swallow(ess->base_obj, "e.swallow.controls",
			    ess->control_obj);
   edje_object_size_min_calc(ess->base_obj, &smw, &smh);
   
   edje_extern_object_min_size_set(ess->scrollframe_obj, 0, 0);
   edje_object_part_swallow(ess->control_obj, "e.swallow.content",
			    ess->scrollframe_obj);   
   
   edje_extern_object_min_size_set(ess->control_obj, 0, 0);
   edje_object_part_swallow(ess->base_obj, "e.swallow.controls",
			    ess->control_obj);
   
   smw = ess->zone->w;
   if (smh > ess->zone->h) smh = ess->zone->h;
   
   printf("min size: %i %i\n", smw, smh);
//   smh = 400;
   
   evas_object_resize(ess->base_obj, smw, smh);
   edje_object_calc_force(ess->base_obj);
   edje_object_calc_force(ess->control_obj);
   edje_object_part_geometry_get(ess->base_obj, "e.swallow.controls", &vx, &vy, &vw, &vh);
//   evas_object_geometry_get(ess->control_obj, &vx, &vy, &vw, &vh);
   ess->control.w = vw;
   ess->control.h = vh;
   printf("control size: %i %i\n", vw, vh);
   edje_extern_object_min_size_set(ess->control_obj, ess->control.w, ess->control.h);
   edje_object_part_swallow(ess->base_obj, "e.swallow.controls",
			    ess->control_obj);
   edje_object_calc_force(ess->base_obj);
   edje_object_calc_force(ess->control_obj);
   edje_object_part_geometry_get(ess->base_obj, "e.swallow.visible", &vx, &vy, &vw, &vh);
//   evas_object_geometry_get(ess->vis_obj, &vx, &vy, &vw, &vh);
   
   printf("v... = %i %i %ix%i\n", vx, vy, vw, vh);
   ess->hidden = vy;
   x = ess->zone->x;
   y = ess->zone->y - ess->hidden + ess->adjust;
   printf("hidden: %i\n", ess->hidden);
   e_popup_move_resize(ess->popup, x, y, smw, smh);
   evas_object_resize(ess->base_obj, ess->popup->w, ess->popup->h);
}

/*
static void
_e_slipshelf_applist_clear(E_Slipshelf *ess)
{
   Evas_Coord mw, mh, vw, vh, w, h;

   e_ilist_freeze(ess->ilist_obj);
   e_ilist_clear(ess->ilist_obj);
   e_ilist_thaw(ess->ilist_obj);
   
   ess->bsel = NULL;
   while (ess->borders)
     {
	e_object_unref(E_OBJECT(ess->borders->data));
	ess->borders = evas_list_remove_list(ess->borders, ess->borders);
     }
   
   e_scrollframe_child_viewport_size_get(ess->scrollframe_obj, &vw, &vh);
   e_ilist_min_size_get(ess->ilist_obj, &mw, &mh);
   
   evas_object_geometry_get(ess->scrollframe_obj, NULL, NULL, &w, &h);
   
   if (mw > vw) mw = mw + (w - vw);
   else if (mw < vw) mw = vw;
   evas_object_resize(ess->ilist_obj, mw, mh);
}
*/

static int
_e_slipshelf_cb_animate(void *data)
{
   E_Slipshelf *ess;
   double t, v;
   
   ess = data;
   t = ecore_time_get() - ess->start;
   if (t > ess->len) t = ess->len;
   if (ess->len > 0.0)
     {
	v = t / ess->len;
	v = 1.0 - v;
	v = v * v * v * v;
	v = 1.0 - v;
     }
   else
     {
	t = ess->len;
	v = 1.0;
     }
   ess->adjust = (ess->adjust_target * v) + (ess->adjust_start  * (1.0 - v));
//   printf("%i | %i\n", ess->hidden, ess->adjust);
   e_popup_move(ess->popup, 
		ess->zone->x, 
		ess->zone->y - ess->hidden + ess->adjust);
   if (t >= ess->len)
     {
	ess->animator = NULL;
	if (ess->out)
	  {
	     edje_object_signal_emit(ess->control_obj, "e,state,out,end", "e");
	     edje_object_signal_emit(ess->base_obj, "e,state,out,end", "e");
	  }
	else
	  {
	     edje_object_signal_emit(ess->control_obj, "e,state,in,end", "e");
	     edje_object_signal_emit(ess->base_obj, "e,state,in,end", "e");
//	     _e_slipshelf_applist_clear(ess);
	  }
	return 0;
     }
   return 1;
}

static void
_e_slipshelf_slide(E_Slipshelf *ess, int out, double len)
{
   if (out == ess->out) return;
   ess->start = ecore_time_get();
   ess->len = len;
   ess->out = out;
   ess->adjust_start = ess->adjust;
   if (ess->out)
     {
//	_e_slipshelf_applist_clear(ess);
	_e_slipshelf_applist_update(ess);
	
	edje_object_signal_emit(ess->control_obj, "e,state,out,begin", "e");
	edje_object_signal_emit(ess->base_obj, "e,state,out,begin", "e");
	ecore_x_window_configure(ess->clickwin, 
				 ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING|ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
				 0, 0, 0, 0, 0,
				 ess->popup->evas_win,
				 ECORE_X_WINDOW_STACK_BELOW);
	ecore_x_window_show(ess->clickwin);
     }
   else
     {
	edje_object_signal_emit(ess->control_obj, "e,state,in,begin", "e");
	edje_object_signal_emit(ess->base_obj, "e,state,in,begin", "e");
	ecore_x_window_hide(ess->clickwin);
     }
   if (ess->out) ess->adjust_target = ess->hidden;
   else ess->adjust_target = 0;
   printf("adjust_target = %i | asjust_start =  %i\n", ess->adjust_target, ess->adjust_start);
   if (len <= 0.0)
     {
	_e_slipshelf_cb_animate(ess);
	return;
     }
   if (!ess->animator)
     ess->animator = ecore_animator_add(_e_slipshelf_cb_animate, ess);
}

static int
_e_slipshelf_cb_mouse_up(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Button_Up *ev;
   E_Slipshelf *ess;
   
   ev = event;
   ess = data;
   if (ev->win == ess->clickwin)
     {
	if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
	ess->slide_down_timer = NULL;
	if (ess->out) _e_slipshelf_slide(ess, 0, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
	else _e_slipshelf_slide(ess, 1, (double)illume_cfg->sliding.slipshelf.duration / 1000.0);
     }
   return 1;
}

static int
_e_slipshelf_cb_zone_move_resize(void *data, int type, void *event)
{
   E_Event_Zone_Move_Resize *ev;
   E_Slipshelf *ess;
   
   ev = event;
   ess = data;
   if (ess->zone == ev->zone)
     {
	if (ess->slide_down_timer) ecore_timer_del(ess->slide_down_timer);
	ess->slide_down_timer = NULL;
	_e_slipshelf_slide(ess, 0, 0.0);
	e_popup_move_resize(ess->popup,
			    ess->zone->x,
			    ess->zone->y - ess->hidden + ess->adjust,
			    ess->zone->w, ess->popup->h);
	evas_object_resize(ess->base_obj, ess->popup->w, ess->popup->h);
     }
   return 1;
}

static void
_e_slipshelf_event_simple_free(void *data, void *ev)
{
   struct _E_Event_Slipshelf_Simple *e;
   
   e = ev;
   e_object_unref(E_OBJECT(e->slipshelf));
   free(e);
}

static void
_e_slipshelf_object_del_attach(void *o)
{
   E_Slipshelf *ess;
   E_Event_Slipshelf_Del *ev;

   if (e_object_is_del(E_OBJECT(o))) return;
   ess = o;
   ev = calloc(1, sizeof(E_Event_Slipshelf_Del));
   ev->slipshelf = ess;
   e_object_ref(E_OBJECT(ess));
   ecore_event_add(E_EVENT_SLIPSHELF_DEL, ev, 
		   _e_slipshelf_event_simple_free, NULL);
}
    
static int
_e_slipshelf_cb_border_focus_in(void *data, int type, void *event)
{
   E_Event_Border_Focus_In *ev;
   E_Slipshelf *ess;
   
   ev = event;
   ess = data;
   ess->focused_border = ev->border;
     _e_slipshelf_title_update(ess);
}

static int
_e_slipshelf_cb_border_focus_out(void *data, int type, void *event)
{
   E_Event_Border_Focus_Out *ev;
   E_Slipshelf *ess;
   
   ev = event;
   ess = data;
   if (ess->focused_border == ev->border)
     ess->focused_border = NULL;
     _e_slipshelf_title_update(ess);
}

static int
_e_slipshelf_cb_border_property(void *data, int type, void *event)
{
   E_Event_Border_Property *ev;
   E_Slipshelf *ess;
   
   ev = event;
   ess = data;
   if (ess->focused_border == ev->border)
     _e_slipshelf_title_update(ess);
}

static void
_e_slipshelf_title_update(E_Slipshelf *ess)
{
   if (ess->focused_border)
     {
	if (ess->focused_border->client.netwm.name)
	  edje_object_part_text_set(ess->base_obj, "e.text.label",
				    ess->focused_border->client.netwm.name);
	else if (ess->focused_border->client.icccm.title)
	  edje_object_part_text_set(ess->base_obj, "e.text.label",
				    ess->focused_border->client.icccm.title);
	else
	  edje_object_part_text_set(ess->base_obj, "e.text.label",
				    ess->default_title);
     }
   else
     edje_object_part_text_set(ess->base_obj, "e.text.label",
			       ess->default_title);
}

static void
_e_slipshelf_cb_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h)
{
   E_Slipshelf *ess;
   Evas_Coord x, y, mw, mh, vx, vy, vw, vh;
   
   ess = data;
   printf("REQ MIN %ix%i\n", w, h);
   if (ess->animator) ecore_animator_del(ess->animator);
   ess->animator = NULL;
   ess->out = 0;
   
   if (gc == ess->gadcon)
     {
	if (h < ess->main_size) h = ess->main_size;
	edje_extern_object_min_size_set(ess->gadcon->o_container, w, h);
	edje_object_part_swallow(ess->base_obj, "e.swallow.content", ess->gadcon->o_container);
     }
   else if (gc == ess->gadcon_extra)
     {
	if (h < ess->extra_size) h = ess->extra_size;
	edje_extern_object_min_size_set(ess->gadcon_extra->o_container, w, h);
	edje_object_part_swallow(ess->base_obj, "e.swallow.extra", ess->gadcon_extra->o_container);
     }
   edje_extern_object_min_size_set(ess->control_obj, ess->control.w, ess->control.h);
   edje_object_part_swallow(ess->base_obj, "e.swallow.controls",
			    ess->control_obj);
   edje_object_size_min_calc(ess->base_obj, &mw, &mh);
   
   evas_object_resize(ess->base_obj, mw, mh);
   edje_object_part_geometry_get(ess->base_obj, "e.swallow.visible", &vx, &vy, &vw, &vh);
//   evas_object_geometry_get(ess->vis_obj, &vx, &vy, &vw, &vh);
   ess->hidden = vy;
   x = ess->zone->x;
   y = ess->zone->y - ess->hidden;
   mw = ess->zone->w;
   e_popup_move_resize(ess->popup, x, y, mw, mh);
   evas_object_resize(ess->base_obj, ess->popup->w, ess->popup->h);
   return;
}

static void
_e_slipshelf_cb_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h)
{
   E_Slipshelf *ess;
   
   ess = data;
   printf("REQ %ix%i\n", w, h);
   return;
}

static Evas_Object *
_e_slipshelf_cb_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style)
{
   /* FIXME: provide an inset look edje thing */
   return NULL;
}


static Evas_Object *
_theme_obj_new(Evas *e, const char *custom_dir, const char *group)
{
   Evas_Object *o;
   
   o = edje_object_add(e);
   if (!e_theme_edje_object_set(o, "base/theme/modules/illume", group))
     {
	if (custom_dir)
	  {
	     char buf[PATH_MAX];
	     
	     snprintf(buf, sizeof(buf), "%s/illume.edj", custom_dir);
	     if (edje_object_file_set(o, buf, group))
	       {
		  printf("OK FALLBACK %s\n", buf);
	       }
	  }
     }
   return o;
}
