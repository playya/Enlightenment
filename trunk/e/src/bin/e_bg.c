/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_bg_signal(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_bg_event_bg_update_free(void *data, void *event);
static int  _e_bg_slide_animator(void *data);

/* local subsystem globals */
EAPI int E_EVENT_BG_UPDATE = 0;
static E_Fm2_Mime_Handler *bg_hdl = NULL;

typedef struct _E_Bg_Anim_Params E_Bg_Anim_Params;
struct _E_Bg_Anim_Params
{
   E_Zone *zone;
   double start_time;
   int start_x;
   int start_y;
   int end_x;
   int end_y;

   struct {
      Eina_Bool x, y;
   } freedom;
};

/* externally accessible functions */
EAPI int
e_bg_init(void)
{
   Eina_List *l = NULL;
   E_Config_Desktop_Background *cfbg = NULL;

   /* Register mime handler */
   bg_hdl = e_fm2_mime_handler_new(_("Set As Background"),
				   "preferences-desktop-wallpaper",
				   e_bg_handler_set, NULL,
				   e_bg_handler_test, NULL);
   if (bg_hdl) e_fm2_mime_handler_glob_add(bg_hdl, "*.edj");

   /* Register files in use */
   if (e_config->desktop_default_background)
     e_filereg_register(e_config->desktop_default_background);

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
	if (!cfbg) continue;
	e_filereg_register(cfbg->file);
     }

   E_EVENT_BG_UPDATE = ecore_event_type_new();
   return 1;
}

EAPI int
e_bg_shutdown(void)
{
   Eina_List *l = NULL;
   E_Config_Desktop_Background *cfbg = NULL;

   /* Deregister mime handler */
   if (bg_hdl)
     {
	e_fm2_mime_handler_glob_del(bg_hdl, "*.edj");
	e_fm2_mime_handler_free(bg_hdl);
     }

   /* Deregister files in use */
   if (e_config->desktop_default_background)
     e_filereg_deregister(e_config->desktop_default_background);

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
	if (!cfbg) continue;
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
   Eina_List *l, *ll, *entries;
   E_Config_Desktop_Background *bg = NULL, *cfbg = NULL;
   const char *bgfile = "";
   char *entry;
   int current_spec = 0; /* how specific the setting is - we want the least general one that applies */

   /* look for desk specific background. */
   if (container_num >= 0 || zone_num >= 0 || desk_x >= 0 || desk_y >= 0)
     {
	EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
	  {
	     int spec;

	     if (!cfbg) continue;
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
	     bgfile = cfbg->file;
	     if (bgfile)
	       {
		  if (bgfile[0] != '/')
		    {
		       const char *bf;

		       bf = e_path_find(path_backgrounds, bgfile);
		       if (bf) bgfile = bf;
		    }
	       }
	     entries = edje_file_collection_list(bgfile);
	     if (entries)
	       {
		  EINA_LIST_FOREACH(entries, ll, entry)
		    {
		       if (!strcmp(entry, "e/desktop/background"))
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
   Eina_List *l, *entries;
   const char *bgfile = "";
   char *entry;
   int ok = 0;

   cfbg = e_bg_config_get(container_num, zone_num, desk_x, desk_y);

   /* fall back to default */
   if (cfbg)
     {
	bgfile = cfbg->file;
	if (bgfile)
	  {
	     if (bgfile[0] != '/')
	       {
		  const char *bf;

		  bf = e_path_find(path_backgrounds, bgfile);
		  if (bf) bgfile = bf;
	       }
	  }
     }
   else
     {
	bgfile = e_config->desktop_default_background;
	if (bgfile)
	  {
	     if (bgfile[0] != '/')
	       {
		  const char *bf;

		  bf = e_path_find(path_backgrounds, bgfile);
		  if (bf) bgfile = bf;
	       }
	  }
	entries = edje_file_collection_list(bgfile);
	if (entries)
	  {
	     EINA_LIST_FOREACH(entries, l, entry)
	       {
		  if (!strcmp(entry, "e/desktop/background"))
		    {
		       ok = 1;
		       break;
		    }
	       }
	     edje_file_collection_list_free(entries);
	  }
	if (!ok)
	  bgfile = e_theme_edje_file_get("base/theme/background",
					 "e/desktop/background");
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
   if ((!trans) || (!trans[0])) transition = E_BG_TRANSITION_NONE;
   if (e_config->desk_flip_pan_bg) transition = E_BG_TRANSITION_NONE;

   desk = e_desk_current_get(zone);
   if (desk)
     bgfile = e_bg_file_get(zone->container->num, zone->num, desk->x, desk->y);
   else
     bgfile = e_bg_file_get(zone->container->num, zone->num, -1, -1);

   if (zone->bg_object)
     {
	const char *pfile = "";

	edje_object_file_get(zone->bg_object, &pfile, NULL);
	if ((!e_util_strcmp(pfile, bgfile)) && !e_config->desk_flip_pan_bg) return;
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
	/* FIXME: segv if zone is deleted while up??? */
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
   edje_object_file_set(o, bgfile, "e/desktop/background");
   if (transition == E_BG_TRANSITION_NONE)
     {
	evas_object_move(o, zone->x, zone->y);
	evas_object_resize(o, zone->w, zone->h);
	evas_object_layer_set(o, -1);
     }
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_show(o);
   if (e_config->desk_flip_pan_bg)
     {
	int x = 0, y = 0;

	o = zone->bg_scrollframe;
	if (!o)
	  {
	     o = e_scrollframe_add(zone->container->bg_evas);
	     zone->bg_scrollframe = o;
	     e_scrollframe_custom_theme_set(o, "base/theme/background",
					    "e/desktop/background/scrollframe");
	     e_scrollframe_policy_set(o, E_SCROLLFRAME_POLICY_OFF, E_SCROLLFRAME_POLICY_OFF);
	     e_scrollframe_child_pos_set(o, 0, 0);
	     evas_object_show(o);
	  }
	e_scrollframe_child_set(o, zone->bg_object);
	if (desk)
	  {
	     x = desk->x;
	     y = desk->y;
	  }
	e_bg_zone_slide(zone, x, y);
	return;
     }

   if (transition != E_BG_TRANSITION_NONE)
     {
	edje_extern_object_max_size_set(zone->prev_bg_object, 65536, 65536);
	edje_extern_object_min_size_set(zone->prev_bg_object, 0, 0);
	edje_object_part_swallow(zone->transition_object, "e.swallow.bg.old",
				 zone->prev_bg_object);
	edje_extern_object_max_size_set(zone->bg_object, 65536, 65536);
	edje_extern_object_min_size_set(zone->bg_object, 0, 0);
	edje_object_part_swallow(zone->transition_object, "e.swallow.bg.new",
				 zone->bg_object);
	edje_object_signal_emit(zone->transition_object, "e,action,start", "e");
     }
}

EAPI void
e_bg_zone_slide(E_Zone *zone, int prev_x, int prev_y)
{
   Evas_Object *o;
   E_Desk *desk;
   Evas_Coord w, h, maxw, maxh, step_w, step_h;
   Ecore_Animator *anim;
   E_Bg_Anim_Params *params;
   Evas_Coord vw, vh, px, py;
   int fx, fy;
   const void *data;

   desk = e_desk_current_get(zone);
   edje_object_size_max_get(zone->bg_object, &w, &h);
   maxw = zone->w * zone->desk_x_count;
   maxh = zone->h * zone->desk_y_count;
   if (!w) w = maxw;
   if (!h) h = maxh;
   evas_object_resize(zone->bg_object, w, h);
   if (zone->desk_x_count > 1)
     step_w = ((double) (w - zone->w)) / (zone->desk_x_count - 1);
   else step_w = 0;
   if (zone->desk_y_count > 1)
     step_h = ((double) (h - zone->h)) / (zone->desk_y_count - 1);
   else step_h = 0;

   o = zone->bg_scrollframe;
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   evas_object_layer_set(o, -1);
   evas_object_clip_set(o, zone->bg_clip_object);

   data = edje_object_data_get(zone->bg_object, "directional_freedom");
   e_scrollframe_child_viewport_size_get(o, &vw, &vh);
   e_scrollframe_child_pos_get(o, &px, &py);
   params = evas_object_data_get(zone->bg_object, "switch_animator_params");
   if (!params)
     params = E_NEW(E_Bg_Anim_Params, 1);
   params->zone = zone;
   params->start_x = px;
   params->start_y = py;
   params->end_x = desk->x * step_w * e_config->desk_flip_pan_x_axis_factor;
   params->end_y = desk->y * step_h * e_config->desk_flip_pan_y_axis_factor;
   params->start_time = 0.0;
   if ((data) && (sscanf(data, "%d %d", &fx, &fy) == 2))
     {
	if (fx)
	  {
	     params->freedom.x = EINA_TRUE;
	     params->start_x = prev_x * step_w * e_config->desk_flip_pan_x_axis_factor;
	  }
	if (fy)
	  {
	     params->freedom.y = EINA_TRUE;
	     params->start_y = prev_y * step_h * e_config->desk_flip_pan_y_axis_factor;
	  }
     }

   anim = evas_object_data_get(zone->bg_object, "switch_animator");
   if (anim) ecore_animator_del(anim);
   anim = ecore_animator_add(_e_bg_slide_animator, params);
   evas_object_data_set(zone->bg_object, "switch_animator", anim);
   evas_object_data_set(zone->bg_object, "switch_animator_params", params);
}

EAPI void
e_bg_default_set(char *file)
{
   E_Event_Bg_Update *ev;

   if (e_config->desktop_default_background)
     {
	e_filereg_deregister(e_config->desktop_default_background);
	eina_stringshare_del(e_config->desktop_default_background);
     }

   if (file)
     {
	e_filereg_register(file);
	e_config->desktop_default_background = eina_stringshare_add(file);
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
   cfbg->file = eina_stringshare_add(file);
   e_config->desktop_backgrounds = eina_list_append(e_config->desktop_backgrounds, cfbg);

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
   Eina_List *l = NULL;
   E_Config_Desktop_Background *cfbg = NULL;
   E_Event_Bg_Update *ev;

   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cfbg)
     {
	if (!cfbg) continue;
	if ((cfbg->container == container) && (cfbg->zone == zone) &&
	    (cfbg->desk_x == desk_x) && (cfbg->desk_y == desk_y))
	  {
	     e_config->desktop_backgrounds = eina_list_remove_list(e_config->desktop_backgrounds, l);
	     e_filereg_deregister(cfbg->file);
	     if (cfbg->file) eina_stringshare_del(cfbg->file);
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
   Eina_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     {
	EINA_LIST_FOREACH(man->containers, ll, con)
	  {
	     EINA_LIST_FOREACH(con->zones, lll, zone)
	       {
		  e_zone_bg_reconfigure(zone);
	       }
	  }
     }
}

EAPI void
e_bg_handler_set(Evas_Object *obj, const char *path, void *data)
{
   E_Container *con;
   E_Zone *zone;
   E_Desk *desk;

   if (!path) return;
   con = e_container_current_get(e_manager_current_get());
   zone = e_zone_current_get(con);
   desk = e_desk_current_get(zone);
   e_bg_del(con->num, zone->num, desk->x, desk->y);
   e_bg_add(con->num, zone->num, desk->x, desk->y, (char *)path);
   e_bg_update();
   e_config_save_queue();
}

EAPI int
e_bg_handler_test(Evas_Object *obj, const char *path, void *data)
{
   if (!path) return 0;
   if (edje_file_group_exists(path, "e/desktop/background")) return 1;
   return 0;
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

static int
_e_bg_slide_animator(void *data)
{
   E_Bg_Anim_Params *params;
   E_Zone *zone;
   Evas_Object *o;
   E_Desk *desk;
   double st;
   double t, dt, spd;
   Evas_Coord px, py, rx, ry, bw, bh, panw, panh;
   Edje_Message_Int_Set *msg;

   params = data;
   zone = params->zone;
   desk = e_desk_current_get(zone);
   t = ecore_loop_time_get();
   dt = -1.0;
   spd = e_config->desk_flip_animate_time;

   o = zone->bg_scrollframe;
   if (!params->start_time)
     st = params->start_time = t;
   else
     st = params->start_time;

   dt = (t - st) / spd;
   if (dt > 1.0) dt = 1.0;
   dt = 1.0 - dt;
   dt *= dt; /* decelerate - could be a better hack */

   if (params->end_x > params->start_x)
     rx = params->start_x + (params->end_x - params->start_x) * (1.0 - dt);
   else
     rx = params->end_x + (params->start_x - params->end_x) * dt;
   if (params->freedom.x) px = zone->x;
   else px = rx;

   if (params->end_y > params->start_y)
     ry = params->start_y + (params->end_y - params->start_y) * (1.0 - dt);
   else
     ry = params->end_y + (params->start_y - params->end_y) * dt;
   if (params->freedom.y) py = zone->y;
   else py = ry;

   e_scrollframe_child_pos_set(o, px, py);

   evas_object_geometry_get(zone->bg_object, NULL, NULL, &bw, &bh);
   panw = bw - zone->w;
   if (panw < 0) panw = 0;
   panh = bh - zone->h;
   if (panh < 0) panh = 0;
   msg = alloca(sizeof(Edje_Message_Int_Set) + (5 * sizeof(int)));
   msg->count = 6;
   msg->val[0] = rx;
   msg->val[1] = ry;
   msg->val[2] = panw;
   msg->val[3] = panh;
   msg->val[4] = bw;
   msg->val[5] = bh;
   edje_object_message_send(zone->bg_object, EDJE_MESSAGE_INT_SET, 0, msg);

   if (dt <= 0.0)
     {
	evas_object_data_del(zone->bg_object, "switch_animator");
	evas_object_data_del(zone->bg_object, "switch_animator_params");
	E_FREE(params);
	return 0;
     }
   return 1;
}
