/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

static void _e_shelf_free(E_Shelf *es);
static const char *_e_shelf_orient_string_get(E_Shelf *es);
static void _e_shelf_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static void _e_shelf_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static Evas_Object *_e_shelf_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style);
static void _e_shelf_toggle_border_fix(E_Shelf *es);
static void _e_shelf_cb_menu_config(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_shelf_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_shelf_cb_menu_contents(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_shelf_cb_confirm_dialog_yes(void *data);
static void _e_shelf_cb_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_shelf_menu_append(E_Shelf *es, E_Menu *mn);
static void _e_shelf_cb_menu_items_append(void *data, E_Menu *mn);
static void _e_shelf_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_shelf_cb_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _e_shelf_cb_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static int  _e_shelf_cb_id_sort(void *data1, void *data2);
static int  _e_shelf_cb_hide_animator(void *data);
static int  _e_shelf_cb_hide_animator_timer(void *data);
static int  _e_shelf_cb_instant_hide_timer(void *data);
static void _e_shelf_menu_del_hook(void *data);
static void _e_shelf_menu_pre_cb(void *data, E_Menu *m);

static Evas_List *shelves = NULL;
static Evas_Hash *winid_shelves = NULL;

/* externally accessible functions */
EAPI int
e_shelf_init(void)
{
   return 1;
}

EAPI int
e_shelf_shutdown(void)
{
   while (shelves)
     {
	E_Shelf *es;

	es = shelves->data;
	e_object_del(E_OBJECT(es));
     }

   return 1;
}

EAPI void
e_shelf_config_init(void)
{
   Evas_List *l;

   while (shelves)
     {
	E_Shelf *es;

	es = shelves->data;
	e_object_del(E_OBJECT(es));
     }

   for (l = e_config->shelves; l; l = l->next)
     {
	E_Config_Shelf *cf_es;
	E_Zone *zone;
	
	cf_es = l->data;
	zone = e_util_container_zone_number_get(cf_es->container, cf_es->zone);
	if (zone)
	  {
	     E_Shelf *es;
	     
	     es = e_shelf_zone_new(zone, cf_es->name, cf_es->style,
				   cf_es->popup, cf_es->layer, -1);
	     if (es)
	       {
		  if (!cf_es->hide_timeout) cf_es->hide_timeout = 1.0; 
		  if (!cf_es->hide_duration) cf_es->hide_duration = 1.0; 
		  es->cfg = cf_es;
		  es->fit_along = cf_es->fit_along;
		  es->fit_size = cf_es->fit_size;

		  e_shelf_orient(es, cf_es->orient);
		  e_shelf_position_calc(es);
		  e_shelf_populate(es);

		  if (cf_es->desk_show_mode)
		    {
		       E_Desk *desk;
		       Evas_List *ll;

		       desk = e_desk_current_get(zone);
		       for (ll = cf_es->desk_list; ll; ll = ll->next)
			 {
			    E_Config_Shelf_Desk *sd;

			    sd = ll->data;
			    if ((desk->x == sd->x) && (desk->y == sd->y))
			      {
				 e_shelf_show(es);
				 break;
			      }
			 }
		    }
		  else
		    e_shelf_show(es);

		  e_shelf_toggle(es, 0);
	       }
	  }
     }
}

EAPI Evas_List *
e_shelf_list(void)
{
   shelves = evas_list_sort(shelves, evas_list_count(shelves), 
			    _e_shelf_cb_id_sort);
   return shelves;
}

EAPI E_Shelf *
e_shelf_zone_new(E_Zone *zone, const char *name, const char *style, int popup, int layer, int id)
{
   E_Shelf *es;
   const char *option;
   char buf[1024];
   
   es = E_OBJECT_ALLOC(E_Shelf, E_SHELF_TYPE, _e_shelf_free);
   if (!es) return NULL;

   es->x = 0;
   es->y = 0;
   es->w = 32;
   es->h = 32;
   if (popup)
     {
	es->popup = e_popup_new(zone, es->x, es->y, es->w, es->h);
	e_drop_xdnd_register_set(es->popup->evas_win, 1);
	e_popup_layer_set(es->popup, layer);
	es->ee = es->popup->ecore_evas;
	es->evas = es->popup->evas;
     }
   else
     {
	e_drop_xdnd_register_set(zone->container->event_win, 1);
	es->ee = zone->container->bg_ecore_evas;
	es->evas = zone->container->bg_evas;
     }
   es->fit_along = 1;
   es->layer = layer;
   es->zone = zone;
   es->style = evas_stringshare_add(style);

   es->o_event = evas_object_rectangle_add(es->evas);
   evas_object_color_set(es->o_event, 0, 0, 0, 0);
   evas_object_resize(es->o_event, es->w, es->h);
   evas_object_event_callback_add(es->o_event, EVAS_CALLBACK_MOUSE_DOWN, _e_shelf_cb_mouse_down, es);

   es->o_hide = evas_object_rectangle_add(es->evas);
   evas_object_color_set(es->o_hide, 0, 0, 0, 0);
   evas_object_resize(es->o_hide, es->w, es->h);
   evas_object_event_callback_add(es->o_hide, EVAS_CALLBACK_MOUSE_IN, _e_shelf_cb_mouse_in, es);
   evas_object_event_callback_add(es->o_hide, EVAS_CALLBACK_MOUSE_OUT, _e_shelf_cb_mouse_out, es);
   evas_object_repeat_events_set(es->o_hide, 1);
 
   es->o_base = edje_object_add(es->evas);
   es->name = evas_stringshare_add(name);
   snprintf(buf, sizeof(buf), "e/shelf/%s/base", es->style);
   evas_object_resize(es->o_base, es->w, es->h);
   if (!e_theme_edje_object_set(es->o_base, "base/theme/shelf", buf))
     e_theme_edje_object_set(es->o_base, "base/theme/shelf",
			     "e/shelf/default/base");
   if (es->popup)
     {
	evas_object_layer_set(es->o_hide, 1);
	evas_object_show(es->o_hide);
	evas_object_show(es->o_event);
	evas_object_show(es->o_base);
	e_popup_edje_bg_object_set(es->popup, es->o_base);
     }
   else
     {
	evas_object_move(es->o_hide, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_event, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_base, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_layer_set(es->o_hide, layer + 1);
	evas_object_layer_set(es->o_event, layer);
	evas_object_layer_set(es->o_base, layer);
     }
   if (id < 0)
     es->id = evas_list_count(shelves);
   else
     es->id = id;

   snprintf(buf, sizeof(buf), "%i", es->id);
   es->gadcon = e_gadcon_swallowed_new(es->name, buf, es->o_base, "e.swallow.content");
   e_gadcon_min_size_request_callback_set(es->gadcon,
					  _e_shelf_gadcon_min_size_request, es);

   e_gadcon_size_request_callback_set(es->gadcon,
				      _e_shelf_gadcon_size_request, es);
   e_gadcon_frame_request_callback_set(es->gadcon,
				       _e_shelf_gadcon_frame_request, es);
   e_gadcon_orient(es->gadcon, E_GADCON_ORIENT_TOP);
   snprintf(buf, sizeof(buf), "e,state,orientation,%s", 
	    _e_shelf_orient_string_get(es));
   edje_object_signal_emit(es->o_base, buf, "e");
   edje_object_message_signal_process(es->o_base);
   e_gadcon_zone_set(es->gadcon, zone);
   e_gadcon_ecore_evas_set(es->gadcon, es->ee);
   e_gadcon_shelf_set(es->gadcon, es);
   if (popup)
     {
	e_gadcon_dnd_window_set(es->gadcon, es->popup->evas_win);
	winid_shelves = evas_hash_add(winid_shelves, e_util_winid_str_get(es->popup->evas_win), es);
     }
   else
     e_gadcon_dnd_window_set(es->gadcon, zone->container->event_win);
   e_gadcon_util_menu_attach_func_set(es->gadcon,
				      _e_shelf_cb_menu_items_append, es);
   
   shelves = evas_list_append(shelves, es);
   shelves = evas_list_sort(shelves, -1, _e_shelf_cb_id_sort);
   
   es->hidden = 0;
   es->hide_step = 0;

   option =  edje_object_data_get(es->o_base, "hidden_state_size");
   if (option)
     es->hidden_state_size = atoi(option);
   else
     es->hidden_state_size = 4;
   option =  edje_object_data_get(es->o_base, "instant_delay");
   if (option)
     es->instant_delay = atof(option);
   else
     es->instant_delay = -1.0;

   es->hide_origin = -1;
   
   return es;
}

EAPI void
e_shelf_zone_move_resize_handle(E_Zone *zone)
{
   Evas_List *l;
   E_Shelf *es;
   
   for (l = shelves; l; l = l->next)
     {
	es = l->data;
	if (es->zone == zone) e_shelf_position_calc(es);
     }
}

EAPI void
e_shelf_populate(E_Shelf *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   e_gadcon_populate(es->gadcon);
}

EAPI void
e_shelf_show(E_Shelf *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   if (es->popup)
     e_popup_show(es->popup);
   else
     {
	evas_object_show(es->o_hide);
	evas_object_show(es->o_event);
	evas_object_show(es->o_base);
     }
}

EAPI void
e_shelf_hide(E_Shelf *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   if (es->popup)
     e_popup_hide(es->popup);
   else
     {
	evas_object_hide(es->o_hide);
	evas_object_hide(es->o_event);
	evas_object_hide(es->o_base);
     }
}

EAPI void
e_shelf_toggle(E_Shelf *es, int show)
{
  E_OBJECT_CHECK(es);
  E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);

  es->interrupted = -1;
  if ((show) && (es->hidden))
    {  
       es->hidden = 0;
       edje_object_signal_emit(es->o_base, "e,state,visible", "e");
       if (es->instant_delay >= 0.0)
	 {
	    _e_shelf_cb_instant_hide_timer(es);
	 }
       else
	 {
	    if (es->hide_timer)
	      {
		 ecore_timer_del(es->hide_timer);
		 es->hide_timer = NULL;
	      }
	    if (!es->hide_animator)
	      es->hide_animator = ecore_animator_add(_e_shelf_cb_hide_animator, es);
	 }
    }
  else if ((!show) && (!es->hidden) && (!es->gadcon->editing) && (es->cfg->autohide))
    {
       edje_object_signal_emit(es->o_base, "e,state,hidden", "e");
       if (es->instant_delay >= 0.0)
	 {
	    es->hidden = 1; 
	    if (!es->instant_timer)
	      es->instant_timer = ecore_timer_add(es->instant_delay, _e_shelf_cb_instant_hide_timer, es);
	 }
       else
	 {
	    if (es->hide_animator)
	      {
		 es->interrupted = show;
		 return;
	      }
	    es->hidden = 1; 
	    if (es->hide_timer) ecore_timer_del(es->hide_timer);
	    es->hide_timer = ecore_timer_add(es->cfg->hide_timeout, _e_shelf_cb_hide_animator_timer, es);
	 }
    }
}

EAPI void
e_shelf_move(E_Shelf *es, int x, int y)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   es->x = x;
   es->y = y;
   if (es->popup)
     e_popup_move(es->popup, es->x, es->y);
   else
     {
	evas_object_move(es->o_hide, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_event, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_base, es->zone->x + es->x, es->zone->y + es->y);
     }
}

EAPI void
e_shelf_resize(E_Shelf *es, int w, int h)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   es->w = w;
   es->h = h;
   if (es->popup)
     e_popup_resize(es->popup, es->w, es->h);
   evas_object_resize(es->o_hide, es->w, es->h);
   evas_object_resize(es->o_event, es->w, es->h);
   evas_object_resize(es->o_base, es->w, es->h);
}

EAPI void
e_shelf_move_resize(E_Shelf *es, int x, int y, int w, int h)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   es->x = x;
   es->y = y;
   es->w = w;
   es->h = h;
   if (es->popup) 
     e_popup_move_resize(es->popup, es->x, es->y, es->w, es->h); 
   else
     {
	evas_object_move(es->o_hide, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_event, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_base, es->zone->x + es->x, es->zone->y + es->y);
     }
   evas_object_resize(es->o_hide, es->w, es->h);
   evas_object_resize(es->o_event, es->w, es->h);
   evas_object_resize(es->o_base, es->w, es->h);
}

EAPI void
e_shelf_layer_set(E_Shelf *es, int layer)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);

   es->layer = layer;
   if (es->popup)
     e_popup_layer_set(es->popup, es->layer);
   else
     {
	evas_object_layer_set(es->o_hide, es->layer + 1);
	evas_object_layer_set(es->o_event, es->layer);
	evas_object_layer_set(es->o_base, es->layer);
     }
}

EAPI void
e_shelf_save(E_Shelf *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   if (es->cfg)
     {
	es->cfg->orient = es->gadcon->orient;
	if (es->cfg->style) evas_stringshare_del(es->cfg->style);
	es->cfg->style = evas_stringshare_add(es->style);
     }
   else
     {
	E_Config_Shelf *cf_es;
	
	cf_es = E_NEW(E_Config_Shelf, 1);
	cf_es->name = evas_stringshare_add(es->name);
	cf_es->container = es->zone->container->num;
	cf_es->zone = es->zone->num;
	if (es->popup) cf_es->popup = 1;
	cf_es->layer = es->layer;
	e_config->shelves = evas_list_append(e_config->shelves, cf_es);
	cf_es->orient = es->gadcon->orient;
	cf_es->style = evas_stringshare_add(es->style);
	cf_es->fit_along = es->fit_along;
	cf_es->fit_size = es->fit_size;
	cf_es->overlap = 0;
	cf_es->autohide = 0; 
	cf_es->hide_timeout = 1.0; 
	cf_es->hide_duration = 1.0; 
	es->cfg = cf_es;
     }
   e_config_save_queue();
}

EAPI void
e_shelf_unsave(E_Shelf *es)
{
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   if (es->cfg)
     {
	e_config->shelves = evas_list_remove(e_config->shelves, es->cfg);
	evas_stringshare_del(es->cfg->name);
	if (es->cfg->style) evas_stringshare_del(es->cfg->style);
	free(es->cfg);
     }
}

EAPI void
e_shelf_orient(E_Shelf *es, E_Gadcon_Orient orient)
{
   char buf[PATH_MAX];
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);
   
   e_gadcon_orient(es->gadcon, orient);
   snprintf(buf, sizeof(buf), "e,state,orientation,%s", _e_shelf_orient_string_get(es));
   edje_object_signal_emit(es->o_base, buf, "e");
   edje_object_message_signal_process(es->o_base);
}

EAPI void
e_shelf_position_calc(E_Shelf *es)
{
   E_Gadcon_Orient orient = E_GADCON_ORIENT_FLOAT;
   int size = 40;
   
   if (es->cfg)
     {
	orient = es->cfg->orient;
	size = es->cfg->size;
     }
   else
     orient = es->gadcon->orient;
   switch (orient)
     {
      case E_GADCON_ORIENT_FLOAT:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	break;
      case E_GADCON_ORIENT_HORIZ:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = (es->zone->w - es->w) / 2;
	break;
      case E_GADCON_ORIENT_VERT:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->y = (es->zone->h - es->h) / 2;
	break;
      case E_GADCON_ORIENT_LEFT:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = 0;
	es->y = (es->zone->h - es->h) / 2;
	break;
      case E_GADCON_ORIENT_RIGHT:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = es->zone->w - es->w;
	es->y = (es->zone->h - es->h) / 2;
	break;
      case E_GADCON_ORIENT_TOP:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = (es->zone->w - es->w) / 2;
	es->y = 0;
	break;
      case E_GADCON_ORIENT_BOTTOM:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = (es->zone->w - es->w) / 2;
	es->y = es->zone->h - es->h;
	break;
      case E_GADCON_ORIENT_CORNER_TL:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = 0;
	es->y = 0;
	break;
      case E_GADCON_ORIENT_CORNER_TR:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = es->zone->w - es->w;
	es->y = 0;
	break;
      case E_GADCON_ORIENT_CORNER_BL:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = 0;
	es->y = es->zone->h - es->h;
	break;
      case E_GADCON_ORIENT_CORNER_BR:
	if (!es->fit_along) es->w = es->zone->w;
	if (!es->fit_size) es->h = size;
	es->x = es->zone->w - es->w;
	es->y = es->zone->h - es->h;
	break;
      case E_GADCON_ORIENT_CORNER_LT:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = 0;
	es->y = 0;
	break;
      case E_GADCON_ORIENT_CORNER_RT:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = es->zone->w - es->w;
	es->y = 0;
	break;
      case E_GADCON_ORIENT_CORNER_LB:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = 0;
	es->y = es->zone->h - es->h;
	break;
      case E_GADCON_ORIENT_CORNER_RB:
	if (!es->fit_along) es->h = es->zone->h;
	if (!es->fit_size) es->w = size;
	es->x = es->zone->w - es->w;
	es->y = es->zone->h - es->h;
	break;
      default:
	break;
     }
   es->hide_step = 0;
   
   e_shelf_move_resize(es, es->x, es->y, es->w, es->h);
}

EAPI void 
e_shelf_style_set(E_Shelf *es, const char *style) 
{
   const char *option;
   char buf[1024];
   
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);

   if (!es->o_base) return;

   if (es->style)
     evas_stringshare_del(es->style);
   es->style = evas_stringshare_add(style);
   
   if (style)
     snprintf(buf, sizeof(buf), "e/shelf/%s/base", style);
   else
     snprintf(buf, sizeof(buf), "e/shelf/%s/base", "default");
     
   if (!e_theme_edje_object_set(es->o_base, "base/theme/shelf", buf))
     e_theme_edje_object_set(es->o_base, "base/theme/shelf", 
			     "e/shelf/default/base");
   
   option =  edje_object_data_get(es->o_base, "hidden_state_size");
   if (option)
     es->hidden_state_size = atoi(option);
   else
     es->hidden_state_size = 4;
   option =  edje_object_data_get(es->o_base, "instant_delay");
   if (option)
     es->instant_delay = atof(option);
   else
     es->instant_delay = -1.0;
   
   e_gadcon_unpopulate(es->gadcon);
   e_gadcon_populate(es->gadcon);
}

EAPI void 
e_shelf_popup_set(E_Shelf *es, int popup) 
{
   /* FIXME: Needs to recreate the evas objects. */
   E_OBJECT_CHECK(es);
   E_OBJECT_TYPE_CHECK(es, E_SHELF_TYPE);

   if (!es->cfg) return;
   if (es->popup) e_object_del(E_OBJECT(es->popup));
   if (popup) 
     {
	es->popup = e_popup_new(es->zone, es->x, es->y, es->w, es->h);
	e_drop_xdnd_register_set(es->popup->evas_win, 1);
	e_popup_layer_set(es->popup, es->cfg->layer);
	es->ee = es->popup->ecore_evas;
	es->evas = es->popup->evas;
	evas_object_show(es->o_hide);
	evas_object_show(es->o_event);
	evas_object_show(es->o_base);
	e_popup_edje_bg_object_set(es->popup, es->o_base);
     }
   else 
     {
	evas_object_move(es->o_hide, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_event, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_move(es->o_base, es->zone->x + es->x, es->zone->y + es->y);
	evas_object_layer_set(es->o_hide, es->cfg->layer);
	evas_object_layer_set(es->o_event, es->cfg->layer);
	evas_object_layer_set(es->o_base, es->cfg->layer);
     }
}

/* local subsystem functions */
static void
_e_shelf_free(E_Shelf *es)
{
   if (es->hide_animator)
     {
	ecore_animator_del(es->hide_animator);
	es->hide_animator = NULL;
     }
   if (es->instant_timer)
     {
	ecore_timer_del(es->instant_timer);
	es->instant_timer = NULL;
     }

   if (es->menu)
     {
	e_menu_post_deactivate_callback_set(es->menu, NULL, NULL);
	e_object_del(E_OBJECT(es->menu));
	es->menu = NULL;
     }
   if (es->config_dialog) e_object_del(E_OBJECT(es->config_dialog));
   shelves = evas_list_remove(shelves, es);
   e_object_del(E_OBJECT(es->gadcon));
   evas_stringshare_del(es->name);
   evas_stringshare_del(es->style);
   evas_object_del(es->o_hide);
   evas_object_del(es->o_event);
   evas_object_del(es->o_base);
   if (es->popup)
     {
	e_drop_xdnd_register_set(es->popup->evas_win, 0);
	winid_shelves = evas_hash_del(winid_shelves, e_util_winid_str_get(es->popup->evas_win), es);
	e_object_del(E_OBJECT(es->popup));
     }
   free(es);
}

static const char *
_e_shelf_orient_string_get(E_Shelf *es)
{
   const char *sig = "";
   
   switch (es->gadcon->orient)
     {
      case E_GADCON_ORIENT_FLOAT:
	sig = "float";
	break;
      case E_GADCON_ORIENT_HORIZ:
	sig = "horizontal";
	break;
      case E_GADCON_ORIENT_VERT:
	sig = "vertical";
	break;
      case E_GADCON_ORIENT_LEFT:
	sig = "left";
	break;
      case E_GADCON_ORIENT_RIGHT:
	sig = "right";
	break;
      case E_GADCON_ORIENT_TOP:
	sig = "top";
	break;
      case E_GADCON_ORIENT_BOTTOM:
	sig = "bottom";
	break;
      case E_GADCON_ORIENT_CORNER_TL:
	sig = "top_left";
	break;
      case E_GADCON_ORIENT_CORNER_TR:
	sig = "top_right";
	break;
      case E_GADCON_ORIENT_CORNER_BL:
	sig = "bottom_left";
	break;
      case E_GADCON_ORIENT_CORNER_BR:
	sig = "bottom_right";
	break;
      case E_GADCON_ORIENT_CORNER_LT:
	sig = "left_top";
	break;
      case E_GADCON_ORIENT_CORNER_RT:
	sig = "right_top";
	break;
      case E_GADCON_ORIENT_CORNER_LB:
	sig = "left_bottom";
	break;
      case E_GADCON_ORIENT_CORNER_RB:
	sig = "right_bottom";
	break;
      default:
	break;
     }
   return sig;
}

static void
_e_shelf_gadcon_min_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h)
{
   return;
}

static void
_e_shelf_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h)
{
   E_Shelf *es;
   Evas_Coord nx, ny, nw, nh, ww, hh;
   
   es = data;
   nx = es->x;
   ny = es->y;
   nw = es->w;
   nh = es->h;
   ww = hh = 0;
   evas_object_geometry_get(gc->o_container, NULL, NULL, &ww, &hh);
   switch (gc->orient)
     {
      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
	if (!es->fit_along) w = ww;
	if (!es->fit_size) h = hh;
	break;
      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
	if (!es->fit_along) h = hh;
	if (!es->fit_size) w = ww;
	break;
      default:
	break;
     }
   e_gadcon_swallowed_min_size_set(gc, w, h);
   edje_object_size_min_calc(es->o_base, &nw, &nh);
   switch (gc->orient)
     {
      case E_GADCON_ORIENT_FLOAT:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = es->x + ((es->w - nw) / 2);
	break;
      case E_GADCON_ORIENT_HORIZ:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = es->x + ((es->w - nw) / 2);
	break;
      case E_GADCON_ORIENT_VERT:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = es->y + ((es->h - nh) / 2);
	break;
      case E_GADCON_ORIENT_LEFT:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = (es->zone->h - nh) / 2;
	// nx = 0;
	break;
      case E_GADCON_ORIENT_RIGHT:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = (es->zone->h - nh) / 2;
	// nx = es->zone->w - nw;
	break;
      case E_GADCON_ORIENT_TOP:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = (es->zone->w - nw) / 2;
	// ny = 0;
	break;
      case E_GADCON_ORIENT_BOTTOM:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = (es->zone->w - nw) / 2;
	//ny = es->zone->h - nh;
	break;
      case E_GADCON_ORIENT_CORNER_TL:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = 0;
	// ny = 0;
	break;
      case E_GADCON_ORIENT_CORNER_TR:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	nx = es->zone->w - nw;
	// ny = 0;
	break;
      case E_GADCON_ORIENT_CORNER_BL:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nw != es->w) nx = 0;
	// ny = es->zone->h - nh;
	break;
      case E_GADCON_ORIENT_CORNER_BR:
	if (!es->fit_along) nw = es->w;
	if (!es->fit_size) nh = es->h;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	nx = es->zone->w - nw;
	//ny = es->zone->h - nh;
	break;
      case E_GADCON_ORIENT_CORNER_LT:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = 0;
	// nx = 0;
	break;
      case E_GADCON_ORIENT_CORNER_RT:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = 0;
	// nx = es->zone->w - nw;
	break;
      case E_GADCON_ORIENT_CORNER_LB:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = es->zone->h - nh;
	// nx = 0;
	break;
      case E_GADCON_ORIENT_CORNER_RB:
	if (!es->fit_along) nh = es->h;
	if (!es->fit_size) nw = es->w;
	if (nw > es->zone->w) nw = es->zone->w;
	if (nh > es->zone->h) nh = es->zone->h;
	if (nh != es->h) ny = es->zone->h - nh;
	// nx = es->zone->w - nw;
	break;
      default:
	break;
     }
   e_shelf_move_resize(es, nx, ny, nw, nh);
}

static Evas_Object *
_e_shelf_gadcon_frame_request(void *data, E_Gadcon_Client *gcc, const char *style)
{
   E_Shelf *es;
   Evas_Object *o;
   char buf[4096];
   
   es = data;
   o = edje_object_add(gcc->gadcon->evas);

   snprintf(buf, sizeof(buf), "e/shelf/%s/%s", es->style, style);
   if (!e_theme_edje_object_set(o, "base/theme/shelf", buf))
     {
	/* if an inset style (e.g. plain) isn't implemented for a given
	 * shelf style, fall back to the default one. no need for every
	 * theme to implement the plain style */
	snprintf(buf, sizeof(buf), "e/shelf/default/%s", style);
	if (!e_theme_edje_object_set(o, "base/theme/shelf", buf))
	  {
	     evas_object_del(o);
	     return NULL;
	  }
     }
   snprintf(buf, sizeof(buf), "e,state,orientation,%s", _e_shelf_orient_string_get(es));
   edje_object_signal_emit(es->o_base, buf, "e");
   edje_object_message_signal_process(o);
   return o;
}

static void
_e_shelf_toggle_border_fix(E_Shelf *es)
{
   Evas_List *l;

   if (es->cfg->overlap || !e_config->border_fix_on_shelf_toggle)
     return;

   for (l = e_border_client_list(); l; l = l->next)
     {
	E_Border *bd;

	bd = l->data;
	if ((bd->maximized & E_MAXIMIZE_TYPE) == E_MAXIMIZE_NONE)
	  {
	     if (bd->lock_client_location)
	       continue;
	     if (es->hidden)
	       {
		  if (!bd->shelf_fix.modified)
		    continue;
		  if (!--bd->shelf_fix.modified)
		    {
		       e_border_move(bd, bd->shelf_fix.x, bd->shelf_fix.y);
		       continue;
		    }
	       }

	     if (!E_INTERSECTS(bd->x, bd->y, bd->w, bd->h,
		      es->x, es->y, es->w, es->h))
	       continue;

	     if (!es->hidden)
	       {
		  if (!bd->shelf_fix.modified++)
		    bd->shelf_fix.x = bd->x;
		    bd->shelf_fix.y = bd->y;
	       }

	     switch (es->gadcon->orient)
	       {
		case E_GADCON_ORIENT_TOP:
		case E_GADCON_ORIENT_CORNER_TL:
		case E_GADCON_ORIENT_CORNER_TR:
		   if (!es->hidden)
		     e_border_move(bd, bd->x, bd->y + es->h);
		   break;
		case E_GADCON_ORIENT_BOTTOM:
		case E_GADCON_ORIENT_CORNER_BL:
		case E_GADCON_ORIENT_CORNER_BR:
		   if (!es->hidden)
		     e_border_move(bd, bd->x, bd->y - es->h);
		   break;
		case E_GADCON_ORIENT_LEFT:
		case E_GADCON_ORIENT_CORNER_LB:
		case E_GADCON_ORIENT_CORNER_LT:
		   if (!es->hidden)
		     e_border_move(bd, bd->x + es->w, bd->y);
		   break; 
		case E_GADCON_ORIENT_RIGHT:
		case E_GADCON_ORIENT_CORNER_RB:
		case E_GADCON_ORIENT_CORNER_RT:
		   if (!es->hidden)
		     e_border_move(bd, bd->x - es->w, bd->y);
		   break; 
		default:
		   break;
	       }
	  }
	else
	  {
	     E_Maximize max;
	     max = bd->maximized;
	     e_border_unmaximize(bd, E_MAXIMIZE_BOTH);
	     e_border_maximize(bd, max);
	  }
     }
}

static void
_e_shelf_menu_append(E_Shelf *es, E_Menu *mn)
{
   E_Menu_Item *mi;
   E_Menu *subm;
   const char *name;
   char buf[256];
   
   name = es->name;
   if (!name) name = _("Shelf #");
   snprintf(buf, sizeof(buf), "%s %i", name, es->id);
   
   subm = e_menu_new();
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, buf);
   e_util_menu_item_edje_icon_set(mi, "enlightenment/shelf");
   e_menu_pre_activate_callback_set(subm, _e_shelf_menu_pre_cb, es);
   e_object_free_attach_func_set(E_OBJECT(subm), _e_shelf_menu_del_hook);
   e_menu_item_submenu_set(mi, subm);
}

static void
_e_shelf_cb_menu_items_append(void *data, E_Menu *mn)
{
   E_Shelf *es;
   
   es = data;
   _e_shelf_menu_append(es, mn);
}

static void
_e_shelf_cb_menu_config(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Shelf *es;
   
   es = data;
   if (!es->config_dialog) e_int_shelf_config(es);
}

static void
_e_shelf_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Shelf *es;
   
   es = data;
   if (es->gadcon->editing)
     {
	e_gadcon_edit_end(es->gadcon);
	e_shelf_toggle(es, 0);
     }
   else
     {
	e_shelf_toggle(es, 1);
	e_gadcon_edit_begin(es->gadcon);
     }
}

static void
_e_shelf_cb_menu_contents(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Shelf *es;
   
   es = data;
   if (!es->gadcon->config_dialog)
     e_int_gadcon_config(es->gadcon);
}

static void
_e_shelf_cb_confirm_dialog_destroy(void *data)
{
   E_Shelf *es;

   es = data;
   e_object_unref(E_OBJECT(es));
}

static void
_e_shelf_cb_confirm_dialog_yes(void *data)
{
   E_Config_Shelf *cfg;
   E_Shelf *es;

   es = data;
   cfg = es->cfg;
   if (e_object_is_del(E_OBJECT(es))) return;
   e_object_del(E_OBJECT(es));
   e_config->shelves = evas_list_remove(e_config->shelves, cfg);
   if (cfg->name) evas_stringshare_del(cfg->name);
   if (cfg->style) evas_stringshare_del(cfg->style);
   E_FREE(cfg);

   e_config_save_queue();
}


static void
_e_shelf_cb_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Shelf *es;
   E_Config_Shelf *cfg;
  
   es = data;

   if (e_config->cnfmdlg_disabled) 
     {
	cfg = es->cfg;
	if (e_object_is_del(E_OBJECT(es))) return;
	e_object_del(E_OBJECT(es));
	e_config->shelves = evas_list_remove(e_config->shelves, cfg);
	if (cfg->name) evas_stringshare_del(cfg->name);
	if (cfg->style) evas_stringshare_del(cfg->style);
	E_FREE(cfg);
	
	e_config_save_queue();
	return;
     }
   
   e_object_ref(E_OBJECT(es));
   e_confirm_dialog_show(_("Are you sure you want to delete this shelf?"), "enlightenment/e",
			 _("You requested to delete this shelf.<br>"
			      "<br>"
			      "Are you sure you want to delete it?"), NULL, NULL,
			 _e_shelf_cb_confirm_dialog_yes, NULL, data, NULL, 
			 _e_shelf_cb_confirm_dialog_destroy, data);
}

static void
_e_shelf_cb_menu_post(void *data, E_Menu *m)
{
   E_Shelf *es;
   
   es = data;
   if (!es->menu) return;
   e_object_del(E_OBJECT(es->menu));
   es->menu = NULL;
}

static void
_e_shelf_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Shelf *es;
   E_Menu *mn;
   int cx, cy, cw, ch;
   
   es = data;
   ev = event_info;
   switch (ev->button)
     {
      case 1:
	if (es->cfg->autohide_show_action) e_shelf_toggle (es, 1);
	break;
      case 3:
	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _e_shelf_cb_menu_post, es);
	es->menu = mn;
	
	_e_shelf_menu_append(es, mn);
	
	e_gadcon_canvas_zone_geometry_get(es->gadcon, &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	e_util_evas_fake_mouse_up_later(es->gadcon->evas, ev->button);
	break;
     }
}

static void
_e_shelf_cb_mouse_in(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Shelf *es;

   es = data;
   edje_object_signal_emit(es->o_base, "e,state,focused", "e");
   if (!es->cfg->autohide_show_action) e_shelf_toggle (es, 1);
}

static void 
_e_shelf_cb_mouse_out(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   E_Shelf *es;

   es = data;
   e_shelf_toggle(es, 0);
}

static int
_e_shelf_cb_id_sort(void *data1, void *data2)
{
   E_Shelf *es1, *es2;

   es1 = data1;
   es2 = data2;
   if ((es1->id) < (es2->id)) return -1;
   else if (es1->id > es2->id) return 1;
   return 0;
}

static int
_e_shelf_cb_hide_animator(void *data)
{
   E_Shelf *es;
   int step = 2, move = 0;

   es = data;

   switch (es->gadcon->orient)
     {
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
	 step = ((es->h - es->hidden_state_size) / e_config->framerate) / es->cfg->hide_duration;
	 if (!step) step = 1;
	 if (es->hidden)
	   {
	      if (es->hide_origin == -1) es->hide_origin = es->y;
	      if (es->hide_step < es->h - es->hidden_state_size)
		{
		   if (es->hide_step + step > es->h - es->hidden_state_size)
		     {
			move = es->hide_origin - es->h + es->hidden_state_size;
			es->hide_step = es->h - es->hidden_state_size;
		     }
		   else
		     {
			move = es->y - step;
			es->hide_step += step;
		     }
		   e_shelf_move(es, es->x, move);
		}
	      else goto end;
	   }
	 else
	   {
	      if (es->hide_step > 0)
		{
		   if (es->hide_step < step)
		     {
			move = es->hide_origin;
			es->hide_step = 0;
		     }
		   else
		     {
			move = es->y + step;
			es->hide_step -= step;	       
		     }
		   e_shelf_move(es, es->x, move);
		}
	      else goto end;
	   }
	 break;
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
	 step = ((es->h - es->hidden_state_size) / e_config->framerate) / es->cfg->hide_duration;
	 if (!step) step = 1;
	 if (es->hidden)
	   {
	      if (es->hide_origin == -1) es->hide_origin = es->y;
	      if (es->hide_step < es->h - es->hidden_state_size)
		{
		   if (es->hide_step + step > es->h - es->hidden_state_size)
		     {
			move = es->hide_origin + es->h - es->hidden_state_size;
			es->hide_step = es->h - es->hidden_state_size;
		     }
		   else
		     {
			move = es->y + step;
			es->hide_step += step;
		     }
		   e_shelf_move(es, es->x, move);
		}
	      else goto end;
	   }
	 else
	   {
	      if (es->hide_step > 0)
		{
		   if (es->hide_step < step)
		     {
			move = es->hide_origin;
			es->hide_step = 0;
		     }
		   else
		     {
			move = es->y - step;
			es->hide_step -= step;	       
		     }
		   e_shelf_move(es, es->x, move);
		}
	      else goto end;
	   }

	 break;
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_LT:
	 step = ((es->w - es->hidden_state_size) / e_config->framerate) / es->cfg->hide_duration;
	 if (!step) step = 1;
	 if (es->hidden)
	   {
	      if (es->hide_origin == -1) es->hide_origin = es->x;
	      if (es->hide_step < es->w - es->hidden_state_size)
		{
		   if (es->hide_step + step > es->w - es->hidden_state_size)
		     {
			move = es->hide_origin - es->w + es->hidden_state_size;
			es->hide_step = es->w - es->hidden_state_size;
		     }
		   else
		     {
			move = es->x - step;
			es->hide_step += step;
		     }
		   e_shelf_move(es, move, es->y);
		}
	      else goto end;
	   }
	 else
	   {
	      if (es->hide_step > 0)
		{
		   if (es->hide_step < step)
		     {
			move = es->hide_origin;
			es->hide_step = 0;
		     }
		   else
		     {
			move = es->x + step;
			es->hide_step -= step;	       
		     }
		   e_shelf_move(es, move, es->y);
		}
	      else goto end;
	   }

	 break; 
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_RB:
      case E_GADCON_ORIENT_CORNER_RT:
	 step = ((es->w - es->hidden_state_size) / e_config->framerate) / es->cfg->hide_duration;
	 if (!step) step = 1;
	 if (es->hidden)
	   {
	      if (es->hide_origin == -1) es->hide_origin = es->x;
	      if (es->hide_step < es->w - es->hidden_state_size)
		{
		   if (es->hide_step + step > es->w - es->hidden_state_size)
		     {
			move = es->hide_origin + es->w - es->hidden_state_size;
			es->hide_step = es->w - es->hidden_state_size;
		     }
		   else
		     {
			move = es->x + step;
			es->hide_step += step;
		     }
		   e_shelf_move(es, move, es->y);
		}
	      else goto end;
	   }
	 else
	   {
	      if (es->hide_step > 0)
		{
		   if (es->hide_step < step)
		     {
			move = es->hide_origin;
			es->hide_step = 0;
		     }
		   else
		     {
			move = es->x - step;
			es->hide_step -= step;	       
		     }
		   e_shelf_move(es, move, es->y);
		}
	      else goto end;
	   }
	 break; 
      default:
	 break;
     }
   return 1;

end:
   es->hide_animator = NULL;
   if (es->interrupted > -1)
     e_shelf_toggle(es, es->interrupted);
   else
     _e_shelf_toggle_border_fix(es);
   return 0;
}

static int
_e_shelf_cb_hide_animator_timer(void *data)
{
   E_Shelf *es;

   es = data;
   if (!es->hide_animator)
     es->hide_animator = ecore_animator_add(_e_shelf_cb_hide_animator, es);
   es->hide_timer = NULL;
   return 0;
}

static int
_e_shelf_cb_instant_hide_timer(void *data)
{
   E_Shelf *es;

   es = data;

   switch (es->gadcon->orient)
     {
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
	 if (es->hidden)
	   e_shelf_move(es, es->x, es->y - es->h + es->hidden_state_size);
	 else
	   e_shelf_move(es, es->x, es->y + es->h - es->hidden_state_size);
	 break;
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
	 if (es->hidden)
	   e_shelf_move(es, es->x, es->y + es->h - es->hidden_state_size);
	 else
	   e_shelf_move(es, es->x, es->y - es->h + es->hidden_state_size);
	 break;
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_LT:
	 if (es->hidden)
	   e_shelf_move(es, es->x - es->w + es->hidden_state_size, es->y);
	 else
	   e_shelf_move(es, es->x + es->w - es->hidden_state_size, es->y);
	 break; 
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_RB:
      case E_GADCON_ORIENT_CORNER_RT:
	 if (es->hidden)
	   e_shelf_move(es, es->x + es->w - es->hidden_state_size, es->y);
	 else
	   e_shelf_move(es, es->x - es->w + es->hidden_state_size, es->y);
	 break; 
      default:
	 break;
     }
   es->instant_timer = NULL;
   _e_shelf_toggle_border_fix(es);
   return 0;
}

static void 
_e_shelf_menu_del_hook(void *data) 
{
   E_Menu *m;
   Evas_List *l;
   
   m = data;
   for (l = m->items; l; l = l->next) 
     {
	E_Menu_Item *mi;
	
	mi = l->data;
	if (mi->submenu) e_object_del(E_OBJECT(mi->submenu));
     }
}

static void 
_e_shelf_menu_pre_cb(void *data, E_Menu *m) 
{
   E_Shelf *es;
   E_Menu_Item *mi;

   es = data;
   e_menu_pre_activate_callback_set(m, NULL, NULL);
   
   mi = e_menu_item_new(m);
   if (es->gadcon->editing)
     e_menu_item_label_set(mi, _("Stop Moving/Resizing Items"));
   else
     e_menu_item_label_set(mi, _("Begin Moving/Resizing Items"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/edit");
   e_menu_item_callback_set(mi, _e_shelf_cb_menu_edit, es);

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Shelf Configuration"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/shelf");
   e_menu_item_callback_set(mi, _e_shelf_cb_menu_config, es);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Configure Shelf Contents"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/shelf");
   e_menu_item_callback_set(mi, _e_shelf_cb_menu_contents, es);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Delete this Shelf"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/delete");
   e_menu_item_callback_set(mi, _e_shelf_cb_menu_delete, es);   
}
