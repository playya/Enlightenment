#include "e_mod_main.h"


typedef struct _Tab Tab;

struct _Tab
{
  Evry_Plugin *plugin;
  Evas_Object *o_tab;

  int cw, mw;
};



static void
_tab_scroll_to(Tab_View *v, Evry_Plugin *p)
{
   int n, w, mw, i;
   double align;
   Eina_List *l;
   const Evry_State *s = v->state;

   for(i = 0, l = s->cur_plugins; l; l = l->next, i++)
     if (l->data == p) break;

   n = eina_list_count(s->cur_plugins);

   e_box_min_size_get(v->o_tabs, &mw, NULL);
   evas_object_geometry_get(v->o_tabs, NULL, NULL, &w, NULL);

   if (mw <= w + 5)
     {
	e_box_align_set(v->o_tabs, 0.0, 0.5);
	return;
     }

   if (n > 1)
     {
	align = (double)i / (double)(n - 1);
	/* if (evry_conf->scroll_animate)
	 *   {
	 *      if (!scroll_timer)
	 *        scroll_timer = ecore_timer_add(0.01, _evry_list_scroll_timer, NULL);
	 *      if (!scroll_animator)
	 *        scroll_animator = ecore_animator_add(_evry_list_animator, NULL);
	 *   }
	 * else */
	e_box_align_set(v->o_tabs, 1.0 - align, 0.5);
     }
   else
     e_box_align_set(v->o_tabs, 1.0, 0.5);
}

static void
_tabs_update(Tab_View *v)
{

   Eina_List *l, *ll;
   Evry_Plugin *p;
   const Evry_State *s = v->state;
   Tab *tab;
   Evas_Coord w;
   Evas_Object *o;

   evas_object_geometry_get(v->o_tabs, NULL, NULL, &w, NULL);

   /* remove tabs for not active plugins */
   e_box_freeze(v->o_tabs);

   EINA_LIST_FOREACH(v->tabs, l, tab)
     {
	e_box_unpack(tab->o_tab);
	evas_object_hide(tab->o_tab);
     }

   /* show/update tabs of active plugins */
   EINA_LIST_FOREACH(s->cur_plugins, l, p)
     {
	EINA_LIST_FOREACH(v->tabs, ll, tab)
	  if (tab->plugin == p) break;

	if (!tab)
	  {
	     tab = E_NEW(Tab, 1);
	     tab->plugin = p;

	     o = edje_object_add(v->evas);
	     e_theme_edje_object_set(o, "base/theme/everything",
				     "e/modules/everything/tab_item");
	     edje_object_part_text_set(o, "e.text.label", p->name);

	     tab->o_tab = o;

	     edje_object_size_min_calc(o, &tab->cw, NULL);
	     edje_object_size_min_get(o,  &tab->mw, NULL);

	     v->tabs = eina_list_append(v->tabs, tab);
	  }

	if (!tab) continue;

	o = tab->o_tab;
	evas_object_show(o);
	e_box_pack_end(v->o_tabs, o);

	e_box_pack_options_set(o, 1, 1, 1, 0, 0.0, 0.5,
			       (tab->mw < tab->cw ? tab->cw : tab->mw), 10,
			       (w ? w/3 : 150), 9999);
	if (s->plugin == p)
	  edje_object_signal_emit(o, "e,state,selected", "e");
	else
	  edje_object_signal_emit(o, "e,state,unselected", "e");
     }

   e_box_thaw(v->o_tabs);

   if (s->plugin)
     _tab_scroll_to(v, s->plugin);
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
   evry_plugin_select(v->state, p);

   _tabs_update(v);
   _tab_scroll_to(v, p);
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
	if (p->name && (!strncasecmp(p->name, key, 1)))
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
     else if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
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
	else if (ev->compose && isalpha(*ev->compose))
	  {
	     _plugin_next_by_name(v, key);
	     return 1;
	  }
     }

   return 0;
}


EAPI Tab_View *
evry_tab_view_new(const Evry_State *s, Evas *e)
{
   Tab_View *v;
   Evas_Object *o;

   v = E_NEW(Tab_View, 1);
   v->update   = &_tabs_update;
   v->clear    = &_tabs_clear;
   v->key_down = &_tabs_key_down;
   v->state = s;
   
   v->evas = e;
   o = e_box_add(e);
   e_box_orientation_set(o, 1);
   e_box_homogenous_set(o, 1);
   v->o_tabs = o;

   return v;
}

EAPI void
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

   E_FREE(v);
}
