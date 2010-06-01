#include "e_mod_main.h"


typedef struct _Tab Tab;

struct _Tab
{
  Tab_View *tab_view;
  Evry_Plugin *plugin;
  Evas_Object *o_tab;

  int cw, mw;
};


static void _tabs_update(Tab_View *v);
static void _plugin_select(Tab_View *v, Evry_Plugin *p);
static void _plugin_next(Tab_View *v);
static void _plugin_prev(Tab_View *v);


#if 0
static int
_animator(void *data)
{
   Tab_View *v = data;

   double da;
   double spd = (15.0 / (double)e_config->framerate);
   if (spd > 0.9) spd = 0.9;
   int wait = 0;

   if (v->align != v->align_to)
     {
	v->align = (v->align * (1.0 - spd)) + (v->align_to * spd);

	da = v->align - v->align_to;
	if (da < 0.0) da = -da;
	if (da < 0.01)
	  v->align = v->align_to;
	else
	  wait++;

	e_box_align_set(v->o_tabs, 1.0 - v->align, 0.5);
     }

   if (wait) return 1;

   v->animator = NULL;
   return 0;

}

static void
_tab_scroll_to(Tab_View *v, Evry_Plugin *p, int animate)
{
   int n, w, mw, i;
   double align;
   Eina_List *l;
   const Evry_State *s = v->state;

   for(i = 0, l = s->cur_plugins; l; l = l->next, i++)
     if (l->data == p) break;

   n = eina_list_count(s->cur_plugins);

   e_box_size_min_get(v->o_tabs, &mw, NULL);
   evas_object_geometry_get(v->o_tabs, NULL, NULL, &w, NULL);

   if (mw < w)
     {
   	e_box_align_set(v->o_tabs, 0.0, 0.5);
   	return;
     }

   if (n > 1)
     {
	align = (double)i / (double)(n - 1);
	if (animate && evry_conf->scroll_animate)
	  {
	     v->align_to = align;

	     if (!v->animator)
	       v->animator = ecore_animator_add(_animator, v);
	  }
	else
	  e_box_align_set(v->o_tabs, 1.0 - align, 0.5);
     }
   else
     e_box_align_set(v->o_tabs, 0.0, 0.5);
}
#endif

static int
_timer_cb(void *data)
{
   Tab_View *v = data;

   _tabs_update(v);

   v->timer = NULL;
   return 0;

}

static void
_tab_cb_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   /* Evas_Event_Mouse_Down *ev = event_info; */
}

static void
_tab_cb_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Tab *tab = data;
   Tab_View *v = tab->tab_view;

   if (ev->button == 1)
     {
	_plugin_select(v, tab->plugin);
	v->view->update(v->view);
     }
}
static void
_tabs_cb_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = event_info;
   Tab_View *v = data;

   if (ev->z > 0)
     {
	_plugin_next(v);
	v->view->update(v->view);
     }
   else if (ev->z < 0)
     {
	_plugin_prev(v);
	v->view->update(v->view);
     }
}

static void
_tabs_update(Tab_View *v)
{
   Eina_List *l, *ll, *plugins;
   Evry_Plugin *p;
   const Evry_State *s = v->state;
   Tab *tab;
   Evas_Coord w, x;
   Evas_Object *o;
   int cur, i = 0;

   edje_object_calc_force(v->o_tabs);
   evas_object_geometry_get(v->o_tabs, &x, NULL, &w, NULL);

   if (!w && !v->timer)
     {
	v->timer = ecore_timer_add(0.1, _timer_cb, v);
	return;
     }

   /* remove tabs for not active plugins */
   e_box_freeze(v->o_tabs);

   EINA_LIST_FOREACH(v->tabs, l, tab)
     {
	e_box_unpack(tab->o_tab);
	evas_object_hide(tab->o_tab);
     }

   for(cur = 0, l = s->cur_plugins; l; l = l->next, cur++)
     if (l->data == s->plugin) break;

   if (cur > 2)
     {
	if ((cur + 1) == eina_list_count(s->cur_plugins))
	  plugins = eina_list_nth_list(s->cur_plugins, cur - 3);
	else
	  plugins = eina_list_nth_list(s->cur_plugins, cur - 2);
     }
   else
     {
	plugins = s->cur_plugins;
     }

   /* show/update tabs of active plugins */
   EINA_LIST_FOREACH(plugins, l, p)
     {
	EINA_LIST_FOREACH(v->tabs, ll, tab)
	  if (tab->plugin == p) break;

	if (!tab)
	  {
	     tab = E_NEW(Tab, 1);
	     tab->plugin = p;
	     tab->tab_view = v;
	     o = edje_object_add(v->evas);
	     e_theme_edje_object_set(o, "base/theme/modules/everything",
				     "e/modules/everything/tab_item");
	     edje_object_part_text_set(o, "e.text.label", EVRY_ITEM(p)->label);

	     evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
					    _tab_cb_down, tab);
	     evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,
					    _tab_cb_up, tab);
	     tab->o_tab = o;

	     edje_object_size_min_calc(o, &tab->cw, NULL);
	     edje_object_size_min_get(o,  &tab->mw, NULL);

	     v->tabs = eina_list_append(v->tabs, tab);
	  }

	if (!tab) continue;

	o = tab->o_tab;
	evas_object_show(o);
	e_box_pack_end(v->o_tabs, o);
	w++;
	if (eina_list_count(s->cur_plugins) == 2)
	  e_box_pack_options_set(o, 1, 1, 0, 0, 0.0, 0.5,
				 w/4, 10, w/3, 9999);
	else
	  e_box_pack_options_set(o, 1, 1, 0, 0, 0.0, 0.5,
				 w/4, 10,
				 w/3, 9999);
	if (s->plugin == p)
	  edje_object_signal_emit(o, "e,state,selected", "e");
	else
	  edje_object_signal_emit(o, "e,state,unselected", "e");

	if (++i > 3) break;
     }

   /* if (s->plugin)
    *   _tab_scroll_to(v, s->plugin, 0); */
   e_box_align_set(v->o_tabs, 0.0, 0.5);

   e_box_thaw(v->o_tabs);
}


static void
_tabs_clear(Tab_View *v)
{
   Eina_List *l;
   Tab *tab;

   e_box_freeze(v->o_tabs);
   EINA_LIST_FOREACH(v->tabs, l, tab)
     {
	e_box_unpack(tab->o_tab);
	evas_object_hide(tab->o_tab);
     }
   e_box_thaw(v->o_tabs);
}

static void
_plugin_select(Tab_View *v, Evry_Plugin *p)
{
   evry_plugin_select(p);

   _tabs_update(v);
   /* _tab_scroll_to(v, p, 1); */
   /* _tabs_update(v); */
}

static void
_plugin_next(Tab_View *v)
{
   Eina_List *l;
   Evry_Plugin *p = NULL;
   const Evry_State *s = v->state;

   if (!s->plugin) return;

   l = eina_list_data_find_list(s->cur_plugins, s->plugin);

   if (l && l->next)
     p = l->next->data;
   else if (s->plugin != s->cur_plugins->data)
     p = s->cur_plugins->data;

   if (p) _plugin_select(v, p);
}

static void
_plugin_next_by_name(Tab_View *v, const char *key)
{
   Eina_List *l;
   Evry_Plugin *p, *first = NULL, *next = NULL;
   int found = 0;
   const Evry_State *s = v->state;

   if (!s->plugin) return;

   EINA_LIST_FOREACH(s->cur_plugins, l, p)
     {
	/* FIXME how can this happen ? */
	if (!p) continue;

	if (EVRY_ITEM(p)->label && (!strncasecmp(EVRY_ITEM(p)->label, key, 1)))
	  {
	     if (!first) first = p;

	     if (found && !next)
	       next = p;
	  }
	if (p == s->plugin) found = 1;
     }

   if (next)
     p = next;
   else if (first != s->plugin)
     p = first;
   else
     p = NULL;

   if (p) _plugin_select(v, p);
}

static void
_plugin_prev(Tab_View *v)
{
   Eina_List *l;
   Evry_Plugin *p = NULL;
   const Evry_State *s = v->state;

   if (!s->plugin) return;

   l = eina_list_data_find_list(s->cur_plugins, s->plugin);

   if (l && l->prev)
     p = l->prev->data;
   else
     {
	l = eina_list_last(s->cur_plugins);
	if (s->plugin != l->data)
	  p = l->data;
     }

   if (p) _plugin_select(v, p);
}


static int
_tabs_key_down(Tab_View *v, const Ecore_Event_Key *ev)
{
   const char *key = ev->key;

   if (!v->state || !v->state->cur_plugins) return 0;

   if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     {
	if (!strcmp(key, "Next"))
	  {
	     _plugin_next(v);
	     return 1;
	  }
	else if (!strcmp(key, "Prior"))
	  {
	     _plugin_prev(v);
	     return 1;
	  }
     }
   else if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
	if (!strcmp(key, "Left"))
	  {
	     _plugin_prev(v);
	     return 1;
	  }
	else if (!strcmp(key, "Right"))
	  {
	     _plugin_next(v);
	     return 1;
	  }
	else if (ev->compose)
	  {
	     _plugin_next_by_name(v, key);
	     return 1;
	  }
     }

   return 0;
}


Tab_View *
evry_tab_view_new(Evry_View *view, const Evry_State *s, Evas *e)
{
   Tab_View *v;
   Evas_Object *o;

   v = E_NEW(Tab_View, 1);
   v->update   = &_tabs_update;
   v->clear    = &_tabs_clear;
   v->key_down = &_tabs_key_down;
   v->view = view;
   v->state = s;

   v->evas = e;
   o = e_box_add(e);
   e_box_orientation_set(o, 1);
   e_box_homogenous_set(o, 1);
   /* evas_object_propagate_events_set(o, 1);  */
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL,
				  _tabs_cb_wheel, v);
   v->o_tabs = o;

   return v;
}

void
evry_tab_view_free(Tab_View *v)
{
   Tab *tab;

   EINA_LIST_FREE(v->tabs, tab)
     {
	e_box_unpack(tab->o_tab);
	evas_object_del(tab->o_tab);
	E_FREE(tab);
     }

   evas_object_del(v->o_tabs);

   if (v->animator)
     ecore_animator_del(v->animator);

   if (v->timer)
     ecore_timer_del(v->timer);

   E_FREE(v);
}
