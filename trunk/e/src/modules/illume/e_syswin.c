#include "e.h"
#include "e_syswin.h"

typedef struct _Win_Entry Win_Entry;

struct _Win_Entry
{
   E_Syswin   *syswin;
   E_Border    *border;
   Evas_Object *icon;
};

EAPI int E_EVENT_SYSWIN_DEL = 0;

/* internal calls */

E_Syswin *_e_syswin_new(E_Zone *zone, const char *themedir);
static void _e_syswin_free(E_Syswin *ess);
static int _e_syswin_cb_animate(void *data);
static void _e_syswin_slide(E_Syswin *ess, int out, double len);
static int _e_syswin_cb_mouse_up(void *data, int type, void *event);
static int _e_syswin_cb_zone_move_resize(void *data, int type, void *event);
static int _e_syswin_cb_zone_del(void *data, int type, void *event);
static void _e_syswin_event_simple_free(void *data, void *ev);
static void _e_syswin_object_del_attach(void *o);
static void _e_syswin_cb_item_sel(void *data);

static Evas_Object *_theme_obj_new(Evas *e, const char *custom_dir, const char *group);

/* state */
static Eina_List *syswins = NULL;

/* called from the module core */
EAPI int
e_syswin_init(void)
{
   E_EVENT_SYSWIN_DEL = ecore_event_type_new();
   return 1;
}

EAPI int
e_syswin_shutdown(void)
{
   return 1;
}

EAPI E_Syswin *
e_syswin_new(E_Zone *zone, const char *themedir)
{
   E_Syswin *esw;
   Evas_Coord mw, mh;
   int x, y;
   Evas_Object *o;
     
   esw = E_OBJECT_ALLOC(E_Syswin, E_SYSWIN_TYPE, _e_syswin_free);
   if (!esw) return NULL;
   
   esw->zone = zone;
   if (themedir) esw->themedir = eina_stringshare_add(themedir);
   
   esw->clickwin = ecore_x_window_input_new(zone->container->win,
					    zone->x, zone->y, zone->w, zone->h);
   esw->popup = e_popup_new(esw->zone, -1, -1, 1, 1);
   ecore_x_window_configure(esw->clickwin, 
			    ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING|ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
			    0, 0, 0, 0, 0,
			    esw->popup->evas_win,
			    ECORE_X_WINDOW_STACK_BELOW);
   e_popup_layer_set(esw->popup, 220);
   
   esw->base_obj = _theme_obj_new(esw->popup->evas,
				  esw->themedir,
				  "e/modules/syswin/base/default");

   esw->focused_border = e_border_focused_get();

   edje_object_size_min_calc(esw->base_obj, &mw, &mh);

   o = e_widget_ilist_add(esw->popup->evas, 32 * e_scale, 32 * e_scale, NULL);
   esw->ilist_obj = o;
   e_widget_ilist_selector_set(o, 1);
   edje_object_part_swallow(esw->base_obj, "e.swallow.content", o);
   evas_object_show(o);
   
   x = zone->x;
   y = zone->y + zone->h;
   mw = zone->w;
   
   mh = 300;
   
   e_popup_move_resize(esw->popup, x, y, mw, mh);

   evas_object_resize(esw->base_obj, esw->popup->w, esw->popup->h);
   e_popup_edje_bg_object_set(esw->popup, esw->base_obj);
   evas_object_show(esw->base_obj);
   
   e_popup_show(esw->popup);

   syswins = eina_list_append(syswins, esw);

   esw->handlers = eina_list_append
     (esw->handlers,
      ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
			      _e_syswin_cb_mouse_up, esw));
   
   e_object_del_attach_func_set(E_OBJECT(esw), _e_syswin_object_del_attach);
   
   return esw;
}

EAPI void
e_syswin_show(E_Syswin *esw)
{
   Evas_Object *o;
   Evas_Coord mw, mh, vw, vh, w, h;
   Eina_List *borders, *l;
   Win_Entry *ent;
   int i, selnum;
   
   E_OBJECT_CHECK(esw);
   E_OBJECT_TYPE_CHECK(esw, E_SYSWIN_TYPE);

   while (esw->borders)
     {
	ent = esw->borders->data;
	evas_object_del(ent->icon);
	e_object_unref(E_OBJECT(ent->border));
	esw->borders = eina_list_remove_list(esw->borders, esw->borders);
	free(ent);
     }
   e_widget_ilist_freeze(esw->ilist_obj);
   e_widget_ilist_clear(esw->ilist_obj);
   e_widget_ilist_thaw(esw->ilist_obj);
   
   borders = e_border_client_list();
   selnum = -1;
   e_widget_ilist_freeze(esw->ilist_obj);
   for (i = 0, l = borders; l; l = l->next)
     {
	E_Border *bd;
	Evas_Object *ic;
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
	ic = e_border_icon_add(bd, esw->popup->evas);
	ent = calloc(1, sizeof(Win_Entry));
	ent->syswin = esw;
	ent->border = bd;
	ent->icon = ic;
	esw->borders = eina_list_append(esw->borders, ent);
	e_widget_ilist_append(esw->ilist_obj, ic, title, _e_syswin_cb_item_sel, ent, NULL);
	if (bd == e_border_focused_get()) selnum = i;
	i++;
     }
   e_widget_ilist_thaw(esw->ilist_obj);
   
   if (selnum >= 0) e_widget_ilist_selected_set(esw->ilist_obj, selnum);

   e_widget_ilist_go(esw->ilist_obj);

   e_widget_ilist_preferred_size_get(esw->ilist_obj, &mw, &mh);
   if (mh < (120 *e_scale)) mh = 120 * e_scale;
   edje_extern_object_min_size_set(esw->ilist_obj, mw, mh);
   edje_object_part_swallow(esw->base_obj, "e.swallow.content", esw->ilist_obj);

   edje_object_size_min_calc(esw->base_obj, &mw, &mh);

   edje_extern_object_min_size_set(esw->ilist_obj, 0, 0);

   
   edje_object_part_swallow(esw->base_obj, "e.swallow.content", esw->ilist_obj);
   
   mw = esw->zone->w;
   if (mh > esw->zone->h) mh = esw->zone->h;
   e_popup_resize(esw->popup, mw, mh);

   evas_object_resize(esw->base_obj, esw->popup->w, esw->popup->h);
   
   _e_syswin_slide(esw, 1, 1.0);
}

EAPI void
e_syswin_hide(E_Syswin *esw)
{
   E_OBJECT_CHECK(esw);
   E_OBJECT_TYPE_CHECK(esw, E_SYSWIN_TYPE);

   _e_syswin_slide(esw, 0, 1.0);
}

EAPI void
e_syswin_border_select_callback_set(E_Syswin *esw, void (*func) (void *data, E_Syswin *ess, E_Border *bd), const void *data)
{
   E_OBJECT_CHECK(esw);
   E_OBJECT_TYPE_CHECK(esw, E_SYSWIN_TYPE);
   esw->callback.func = func;
   esw->callback.data = data;
}

/* internal calls */
static void
_e_syswin_free(E_Syswin *esw)
{
   Ecore_Event_Handler *handle;

   syswins = eina_list_remove(syswins, esw);
   EINA_LIST_FREE(esw->handlers, handle)
     ecore_event_handler_del(handle);
   if (esw->animator) ecore_animator_del(esw->animator);
   if (esw->themedir) eina_stringshare_del(esw->themedir);
   ecore_x_window_free(esw->clickwin);
   e_object_del(E_OBJECT(esw->popup));
   free(esw);
}

static int
_e_syswin_cb_animate(void *data)
{
   E_Syswin *esw;
   double t, v;
   
   esw = data;
   t = ecore_loop_time_get() - esw->start;
   if (t > esw->len) t = esw->len;
   v = t / esw->len;
   v = 1.0 - v;
   v = v * v * v * v;
   v = 1.0 - v;
   esw->adjust = (esw->adjust_target * v) + (esw->adjust_start  * (1.0 - v));
   e_popup_move(esw->popup, 
		esw->zone->x, 
		esw->zone->y + esw->zone->h - esw->adjust);
   if (t == esw->len)
     {
	esw->animator = NULL;
	if (esw->out)
	  edje_object_signal_emit(esw->base_obj, "e,state,out,end", "e");
	else
	  {
	     edje_object_signal_emit(esw->base_obj, "e,state,in,end", "e");
	     while (esw->borders)
	       {
		  Win_Entry *ent;
		  
		  ent = esw->borders->data;
		  evas_object_del(ent->icon);
		  e_object_unref(E_OBJECT(ent->border));
		  esw->borders = eina_list_remove_list(esw->borders, esw->borders);
		  free(ent);
	       }
	     e_widget_ilist_freeze(esw->ilist_obj);
	     e_widget_ilist_clear(esw->ilist_obj);
	     e_widget_ilist_thaw(esw->ilist_obj);
	  }
	return 0;
     }
   return 1;
}

static void
_e_syswin_slide(E_Syswin *esw, int out, double len)
{
   if (out == esw->out) return;
   if (len == 0.0) len = 0.000001; /* FIXME: handle len = 0.0 case */
   
   if (!esw->animator)
     esw->animator = ecore_animator_add(_e_syswin_cb_animate, esw);
   esw->start = ecore_loop_time_get();
   esw->len = len;
   esw->out = out;
   esw->adjust_start = esw->adjust;
   if (esw->out) esw->adjust_target = esw->popup->h;
   else esw->adjust_target = 0;
   if (esw->out)
     {
	edje_object_signal_emit(esw->base_obj, "e,state,out,begin", "e");
	ecore_x_window_configure(esw->clickwin, 
				 ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING|ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
				 0, 0, 0, 0, 0,
				 esw->popup->evas_win,
				 ECORE_X_WINDOW_STACK_BELOW);
	ecore_x_window_show(esw->clickwin);
     }
   else
     {
	edje_object_signal_emit(esw->base_obj, "e,state,in,begin", "e");
	ecore_x_window_hide(esw->clickwin);
     }
}

static int
_e_syswin_cb_mouse_up(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Button *ev;
   E_Syswin *esw;
   
   ev = event;
   esw = data;
   if (ev->window == esw->clickwin)
     {
	if (esw->out) _e_syswin_slide(esw, 0, 1.0);
	else _e_syswin_slide(esw, 1, 1.0);
     }
   return 1;
}

static int
_e_syswin_cb_zone_move_resize(void *data, int type, void *event)
{
   E_Event_Zone_Move_Resize *ev;
   E_Syswin *esw;
   
   ev = event;
   esw = data;
   if (esw->zone == ev->zone)
     {
	/* FIXME: handle new size pants */
     }
   return 1;
}

static int
_e_syswin_cb_zone_del(void *data, int type, void *event)
{
   E_Event_Zone_Del *ev;
   E_Syswin *esw;
   
   ev = event;
   esw = data;
   if (esw->zone == ev->zone)
     {
	e_object_del(E_OBJECT(esw));
     }
   return 1;
}
				       
static void
_e_syswin_event_simple_free(void *data, void *ev)
{
   struct _E_Event_Syswin_Simple *e;
   
   e = ev;
   e_object_unref(E_OBJECT(e->syswin));
   free(e);
}

static void
_e_syswin_object_del_attach(void *o)
{
   E_Syswin *esw;
   E_Event_Syswin_Del *ev;

   if (e_object_is_del(E_OBJECT(o))) return;
   esw = o;
   while (esw->borders)
     {
	Win_Entry *ent;
	
	ent = esw->borders->data;
	evas_object_del(ent->icon);
	e_object_unref(E_OBJECT(ent->border));
	esw->borders = eina_list_remove_list(esw->borders, esw->borders);
	free(ent);
     }
/*    ev = calloc(1, sizeof(E_Event_Syswin_Del)); */
/*    ev->syswin = esw; */
/*    e_object_ref(E_OBJECT(esw)); */
/*    fprintf(stderr, "event add E_EVENT_SYSWIN_DEL\n"); */
/*    ecore_event_add(E_EVENT_SYSWIN_DEL, ev,  */
/* 		   _e_syswin_event_simple_free, NULL); */
}

static void
_e_syswin_cb_item_sel(void *data)
{
   Win_Entry *ent;

   ent = data;
   if (ent->syswin->callback.func)
     ent->syswin->callback.func(ent->syswin->callback.data, ent->syswin, ent->border);
   e_syswin_hide(ent->syswin);
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

