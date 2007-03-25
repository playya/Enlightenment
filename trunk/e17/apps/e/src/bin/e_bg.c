/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_bg_signal(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_bg_event_bg_update_free(void *data, void *event);

/* local subsystem globals */
EAPI int E_EVENT_BG_UPDATE = 0;

/* externally accessible functions */
EAPI int 
e_bg_init(void)
{
   Evas_List *l;

   /* Register files in use */
   if (e_config->desktop_default_background)
      e_filereg_register(e_config->desktop_default_background);

   for (l = e_config->desktop_backgrounds; l; l = l->next)
     {
	E_Config_Desktop_Background *cfbg;
	cfbg = l->data;
	e_filereg_register(cfbg->file);
     }
   
   E_EVENT_BG_UPDATE = ecore_event_type_new();
   return 1;
}

EAPI int 
e_bg_shutdown(void)
{
   Evas_List *l;

   /* Register files in use */
   if (e_config->desktop_default_background)
      e_filereg_deregister(e_config->desktop_default_background);

   for (l = e_config->desktop_backgrounds; l; l = l->next)
     {
       E_Config_Desktop_Background *cfbg;
       cfbg = l->data;
       e_filereg_deregister(cfbg->file);
     }

   return 1;
}

/**
 * Find the configuration for a given desktop background
 * Use -1 as a wild card for each parameter.
 * The most specific match will be returned
 */
EAPI const E_Config_Desktop_Background *
e_bg_config_get(int container_num, int zone_num, int desk_x, int desk_y)
{
   Evas_List *l, *ll, *entries;

   E_Config_Desktop_Background *bg = NULL;
   int current_spec = 0; /* how specific the setting is - we want the least general one that applies */

   /* look for desk specific background. */
   if (container_num >= 0 || zone_num >= 0 || desk_x >= 0 || desk_y >= 0)
     {
	for (l = e_config->desktop_backgrounds; l; l = l->next)
	  {
	     E_Config_Desktop_Background *cfbg;
	     int spec;

	     cfbg = l->data;

	     spec = 0;
	     if (cfbg->container == container_num) spec++;
	     else if (cfbg->container >= 0) continue;
	     if (cfbg->zone == zone_num) spec++;
	     else if (cfbg->zone >= 0) continue;
	     if (cfbg->desk_x == desk_x) spec++;
	     else if (cfbg->desk_x >= 0) continue;
	     if (cfbg->desk_y == desk_y) spec++;
	     else if (cfbg->desk_y >= 0) continue;

	     if (spec <= current_spec) continue;

	     entries = edje_file_collection_list(cfbg->file);
	     if (entries)
	       {
		  for (ll = entries; ll; ll = ll->next)
		    {
		       if (!strcmp(ll->data, "e/desktop/background"))
			 {
			    bg = cfbg;
			    current_spec = spec;
			 }
		    }
		  edje_file_collection_list_free(entries);
	       }
	  }
     }
   return bg;
}

EAPI const char *
e_bg_file_get(int container_num, int zone_num, int desk_x, int desk_y)
{
   const E_Config_Desktop_Background *cfbg;
   Evas_List *l, *entries;
   const char *bgfile = "";
   int ok = 0;

   cfbg = e_bg_config_get(container_num, zone_num, desk_x, desk_y);

   /* fall back to default */
   if (cfbg)
     bgfile = cfbg->file;
   else
     {
	entries = edje_file_collection_list(e_config->desktop_default_background);
	if (entries)
	  {
	     for (l = entries; l; l = l->next)
	       {
		  if (!strcmp(l->data, "e/desktop/background"))
		    {
		       bgfile = e_config->desktop_default_background;
		       ok = 1;
		       break;
		    }
	       }
	     edje_file_collection_list_free(entries);
	  }
	if (!ok)
	  {
	     bgfile = e_theme_edje_file_get("base/theme/background", "e/desktop/background");
	  }
     }

   return bgfile;
}

EAPI void
e_bg_zone_update(E_Zone *zone, E_Bg_Transition transition)
{
   Evas_Object *o;
   const char *bgfile = "";
   const char *trans = "";
   E_Desk *desk;
   
   if (transition == E_BG_TRANSITION_START) trans = e_config->transition_start;
   else if (transition == E_BG_TRANSITION_DESK) trans = e_config->transition_desk;
   else if (transition == E_BG_TRANSITION_CHANGE) trans = e_config->transition_change;
   if ((!trans) || (strlen(trans) < 1)) transition = E_BG_TRANSITION_NONE;

   desk = e_desk_current_get(zone);
   if (desk)
     bgfile = e_bg_file_get(zone->container->num, zone->num, desk->x, desk->y);
   else
     bgfile = e_bg_file_get(zone->container->num, zone->num, -1, -1);

   if (zone->bg_object)
     {
	const char *pfile = "";
	
	edje_object_file_get(zone->bg_object, &pfile, NULL);
	if (!e_util_strcmp(pfile, bgfile))
	  return;
     }
   
   if (transition == E_BG_TRANSITION_NONE)
     {
	if (zone->bg_object)
	  {
	     evas_object_del(zone->bg_object);
	     zone->bg_object = NULL;
	  }
     }
   else
     {
	char buf[4096];
	
	if (zone->bg_object)
	  {
	     if (zone->prev_bg_object)
	       evas_object_del(zone->prev_bg_object);
	     zone->prev_bg_object = zone->bg_object;
	     if (zone->transition_object)
	       evas_object_del(zone->transition_object);
	     zone->transition_object = NULL;
	     zone->bg_object = NULL;
	  }
	o = edje_object_add(zone->container->bg_evas);
	zone->transition_object = o;
	evas_object_data_set(o, "e_zone", zone);
	snprintf(buf, sizeof(buf), "e/transitions/%s", trans);
	e_theme_edje_object_set(o, "base/theme/transitions", buf);
	edje_object_signal_callback_add(o, "e,state,done", "*", _e_bg_signal, zone);
	evas_object_move(o, zone->x, zone->y);
	evas_object_resize(o, zone->w, zone->h);
	evas_object_layer_set(o, -1);
	evas_object_clip_set(o, zone->bg_clip_object);
	evas_object_show(o);
     }
   o = edje_object_add(zone->container->bg_evas);
   zone->bg_object = o;
   evas_object_data_set(o, "e_zone", zone);
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   edje_object_file_set(o, bgfile, "e/desktop/background");
   evas_object_layer_set(o, -1);
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_show(o);
   
   if (transition != E_BG_TRANSITION_NONE)
     {
	edje_object_part_swallow(zone->transition_object, "e.swallow.bg.old",
				 zone->prev_bg_object);
	edje_object_part_swallow(zone->transition_object, "e.swallow.bg.new",
				 zone->bg_object);
	edje_object_signal_emit(zone->transition_object, "e,action,start", "e");
     }
}

EAPI void
e_bg_default_set(char * file)
{
   E_Event_Bg_Update *ev;

   if (e_config->desktop_default_background)
     {
	e_filereg_deregister(e_config->desktop_default_background);
	evas_stringshare_del(e_config->desktop_default_background);
     }

   if (file)
     {
	e_filereg_register(file);
	e_config->desktop_default_background = evas_stringshare_add(file);
     }
   else
     e_config->desktop_default_background = NULL;

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->container = -1;
   ev->zone = -1;
   ev->desk_x = -1;
   ev->desk_y = -1;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

EAPI void
e_bg_add(int container, int zone, int desk_x, int desk_y, char *file)
{
   E_Config_Desktop_Background *cfbg;
   E_Event_Bg_Update *ev;
   
   e_bg_del(container, zone, desk_x, desk_y);
   cfbg = E_NEW(E_Config_Desktop_Background, 1);
   cfbg->container = container;
   cfbg->zone = zone;
   cfbg->desk_x = desk_x;
   cfbg->desk_y = desk_y;
   cfbg->file = evas_stringshare_add(file);
   e_config->desktop_backgrounds = evas_list_append(e_config->desktop_backgrounds, cfbg);
   
   e_filereg_register(cfbg->file);

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->container = container;
   ev->zone = zone;
   ev->desk_x = desk_x;
   ev->desk_y = desk_y;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

EAPI void
e_bg_del(int container, int zone, int desk_x, int desk_y)
{
   Evas_List *l;
   E_Event_Bg_Update *ev;
   
   for (l = e_config->desktop_backgrounds; l; l = l->next)
     {
	E_Config_Desktop_Background *cfbg;
	
	cfbg = l->data;
	if ((cfbg->container == container) && (cfbg->zone == zone) &&
	    (cfbg->desk_x == desk_x) && (cfbg->desk_y == desk_y))
	  {
	     e_config->desktop_backgrounds = evas_list_remove_list(e_config->desktop_backgrounds, l);
	     e_filereg_deregister(cfbg->file);
	     if (cfbg->file) evas_stringshare_del(cfbg->file);
	     free(cfbg);
	     break;
	  }
     }

   ev = E_NEW(E_Event_Bg_Update, 1);
   ev->container = container;
   ev->zone = zone;
   ev->desk_x = desk_x;
   ev->desk_y = desk_y;
   ecore_event_add(E_EVENT_BG_UPDATE, ev, _e_bg_event_bg_update_free, NULL);
}

EAPI void
e_bg_update(void)
{
   Evas_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   for (l = e_manager_list(); l; l = l->next)
     {
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
	  {
	     con = ll->data;
	     for (lll = con->zones; lll; lll = lll->next)
	       {
		  zone = lll->data;
		  e_zone_bg_reconfigure(zone);
	       }
	  }
     }
}

/* local subsystem functions */

static void
_e_bg_signal(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_Zone *zone;
   
   zone = data;

   if (zone->prev_bg_object)
     {
	evas_object_del(zone->prev_bg_object);
	zone->prev_bg_object = NULL;
     }
   if (zone->transition_object)
     {
	evas_object_del(zone->transition_object);
	zone->transition_object = NULL;
     }
   evas_object_move(zone->bg_object, zone->x, zone->y);
   evas_object_resize(zone->bg_object, zone->w, zone->h);
   evas_object_layer_set(zone->bg_object, -1);
   evas_object_clip_set(zone->bg_object, zone->bg_clip_object);
   evas_object_show(zone->bg_object);
}

static void
_e_bg_event_bg_update_free(void *data, void *event)
{
   free(event);
}
