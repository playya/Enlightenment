/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* TODO
   - watch plugin directories
   - get plugins from ~/.e/e/everything_plugins
 */

#include "e_mod_main.h"
#include "evry_api.h"

/* #undef DBG
 * #define DBG(...) ERR(__VA_ARGS__) */

#define CONFIG_VERSION 15

/* actual module specifics */
static void _e_mod_action_cb(E_Object *obj, const char *params);
static int  _e_mod_run_defer_cb(void *data);
static void _e_mod_run_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_mod_menu_add(void *data, E_Menu *m);
static void _config_init(void);
static void _config_free(void);
static void _plugin_config_free(void);
static int _cleanup_history(void *data);

static Evry_API *_api = NULL;

static E_Int_Menu_Augmentation *maug = NULL;
static E_Action *act = NULL;

/* static Eina_Array  *plugins = NULL; */
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *plugin_conf_edd = NULL;
static E_Config_DD *plugin_setting_edd = NULL;

static Ecore_Timer *cleanup_timer;
static int _update = 0;


EAPI int _e_module_evry_log_dom = -1;


EAPI Evry_Config *evry_conf = NULL;

EAPI int EVRY_EVENT_ITEM_SELECT;
EAPI int EVRY_EVENT_ITEM_CHANGED;
EAPI int EVRY_EVENT_ITEMS_UPDATE;

EAPI Evry_Type EVRY_TYPE_FILE;
EAPI Evry_Type EVRY_TYPE_DIR;
EAPI Evry_Type EVRY_TYPE_APP;
EAPI Evry_Type EVRY_TYPE_ACTION;
EAPI Evry_Type EVRY_TYPE_PLUGIN;
EAPI Evry_Type EVRY_TYPE_NONE;
EAPI Evry_Type EVRY_TYPE_BORDER;
EAPI Evry_Type EVRY_TYPE_TEXT;

/* module setup */
EAPI E_Module_Api e_modapi =
  {
    E_MODULE_API_VERSION,
    "Everything"
  };

static Eina_List *_evry_types = NULL;


EAPI Evry_Type
evry_type_register(const char *type)
{
   const char *t = eina_stringshare_add(type);
   Evry_Type ret = 0;
   const char *i;
   Eina_List *l;

   EINA_LIST_FOREACH(_evry_types, l, i)
     {
	if (i == t) break;
	ret++;
     }

   if(!l)
     {
	_evry_types = eina_list_append(_evry_types, t);
	return ret;
     }
   eina_stringshare_del(t);

   return ret;
}

EAPI const char *
evry_type_get(Evry_Type type)
{
   const char *ret = eina_list_nth(_evry_types, type);
   if (!ret)
     return eina_stringshare_add("");

   return ret;
}

EAPI void *
e_modapi_init(E_Module *m)
{
   Eina_List *l;
   Evry_Module *em;
   
   _e_module_evry_log_dom = eina_log_domain_register
     ("e_module_everything", EINA_LOG_DEFAULT_COLOR);

   if(_e_module_evry_log_dom < 0)
     {
	EINA_LOG_ERR
	  ("impossible to create a log domain for everything module");
	return NULL;
     }

   EVRY_TYPE_NONE   = evry_type_register("NONE");
   EVRY_TYPE_FILE   = evry_type_register("FILE");
   EVRY_TYPE_DIR    = evry_type_register("DIRECTORY");
   EVRY_TYPE_APP    = evry_type_register("APPLICATION");
   EVRY_TYPE_ACTION = evry_type_register("ACTION");
   EVRY_TYPE_PLUGIN = evry_type_register("PLUGIN");
   EVRY_TYPE_BORDER = evry_type_register("BORDER");
   EVRY_TYPE_TEXT   = evry_type_register("TEXT");

   _config_init();
   evry_history_init();

   evry_plug_actions_init();
   view_thumb_init();
   view_help_init();
   evry_plug_clipboard_init();
   evry_plug_text_init();

   /* add module supplied action */
   act = e_action_add("everything");
   if (act)
     {
	act->func.go = _e_mod_action_cb;
	e_action_predef_name_set
	  (_("Everything Launcher"),
	   _("Show Everything Dialog"),
	   "everything", "", NULL, 0);
     }

   maug = e_int_menus_menu_augmentation_add
     ("main/1", _e_mod_menu_add, NULL, NULL, NULL);

   e_configure_registry_category_add
     ("extensions", 80, _("Extensions"), NULL, "preferences-extensions");

   e_configure_registry_item_add
     ("extensions/run_everything", 40, _("Everything Configuration"),
      NULL, "system-run", evry_config_dialog);
   evry_init();

   if (!EVRY_EVENT_ITEMS_UPDATE)
     EVRY_EVENT_ITEMS_UPDATE = ecore_event_type_new();
   if (!EVRY_EVENT_ITEM_SELECT)
     EVRY_EVENT_ITEM_SELECT = ecore_event_type_new();
   if (!EVRY_EVENT_ITEM_CHANGED)
     EVRY_EVENT_ITEM_CHANGED = ecore_event_type_new();

   e_module_delayed_set(m, 0);

   /* make sure module is loaded before others */
   e_module_priority_set(m, -1000);

   _api = E_NEW(Evry_API, 1);
   _api->log_dom = _e_module_evry_log_dom;
#define SET(func) (_api->func = &evry_##func);
   SET(api_version_check);
   SET(item_new);
   SET(item_free);
   SET(item_ref);
   SET(plugin_new);
   SET(plugin_free);
   SET(plugin_register);
   SET(plugin_unregister);
   SET(plugin_update);
   SET(action_new);
   SET(action_free);
   SET(action_register);
   SET(action_unregister);
   SET(action_find);
   SET(api_version_check);
   SET(type_register);
   SET(icon_mime_get);
   SET(icon_theme_get);
   SET(fuzzy_match);
   SET(util_exec_app);
   SET(util_url_escape);
   SET(util_url_unescape);
   SET(util_file_detail_set);
   SET(util_plugin_items_add);
   SET(util_md5_sum);
   SET(util_icon_get);
   SET(items_sort_func);
   SET(event_item_changed);
   SET(file_path_get);
   SET(file_url_get);
#undef SET
   
   e_datastore_set("everything_loaded", _api);

   EINA_LIST_FOREACH(e_datastore_get("everything_modules"), l, em)
     em->init(_api);
   
   /* cleanup every hour :) */
   cleanup_timer = ecore_timer_add(3600, _cleanup_history, NULL);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;
   const char *t;
   Eina_List *l;
   Evry_Module *em;
   
   EINA_LIST_FOREACH(e_datastore_get("everything_modules"), l, em)
     {
	printf("call shutdown\n");
	em->shutdown();
     }
   
   e_datastore_del("everything_loaded");
   E_FREE(_api);
   
   evry_shutdown();

   view_thumb_shutdown();
   view_help_shutdown();
   evry_plug_clipboard_shutdown();
   evry_plug_text_shutdown();
   evry_plug_actions_shutdown();

   _config_free();
   evry_history_free();

   EINA_LIST_FREE(_evry_types, t)
     eina_stringshare_del(t);

   e_configure_registry_item_del("extensions/run_everything");
   e_configure_registry_category_del("extensions");

   while ((cfd = e_config_dialog_get("E", "_config_everything_dialog")))
     e_object_del(E_OBJECT(cfd));

   if (act)
     {
	e_action_predef_name_del(_("Everything Launcher"),
				 _("Show Everything Dialog"));
	e_action_del("everything");
     }

   if (maug)
     {
   	e_int_menus_menu_augmentation_del("main/1", maug);
   	maug = NULL;
     }

   /* Clean EET */
   E_CONFIG_DD_FREE(conf_edd);
   E_CONFIG_DD_FREE(plugin_conf_edd);
   E_CONFIG_DD_FREE(plugin_setting_edd);

   if (cleanup_timer)
     ecore_timer_del(cleanup_timer);

   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.everything", conf_edd, evry_conf);
   return 1;
}

static int
_cleanup_history(void *data)
{
   /* cleanup old entries */
   evry_history_free();
   evry_history_init();

   return 1;
}

static void
_config_init()
{
#undef T
#undef D
#define T Plugin_Config
#define D plugin_conf_edd
   plugin_conf_edd = E_CONFIG_DD_NEW("Plugin_Config", Plugin_Config);
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, enabled, INT);
   E_CONFIG_VAL(D, T, priority, INT);
   E_CONFIG_VAL(D, T, trigger, STR);
   E_CONFIG_VAL(D, T, trigger_only, INT);
   E_CONFIG_VAL(D, T, view_mode, INT);
   E_CONFIG_VAL(D, T, aggregate, INT);
   E_CONFIG_VAL(D, T, top_level, INT);
   E_CONFIG_VAL(D, T, min_query, INT);
#undef T
#undef D
#define T Evry_Config
#define D conf_edd
   conf_edd = E_CONFIG_DD_NEW("Config", Evry_Config);
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, width, INT);
   E_CONFIG_VAL(D, T, height, INT);
   E_CONFIG_VAL(D, T, rel_x, DOUBLE);
   E_CONFIG_VAL(D, T, rel_y, DOUBLE);
   E_CONFIG_VAL(D, T, scroll_animate, INT);
   E_CONFIG_VAL(D, T, scroll_speed, DOUBLE);
   E_CONFIG_VAL(D, T, hide_input, INT);
   E_CONFIG_VAL(D, T, hide_list, INT);
   E_CONFIG_VAL(D, T, quick_nav, INT);
   E_CONFIG_VAL(D, T, cmd_terminal, STR);
   E_CONFIG_VAL(D, T, cmd_sudo, STR);
   E_CONFIG_VAL(D, T, view_mode, INT);
   E_CONFIG_VAL(D, T, view_zoom, INT);
   E_CONFIG_VAL(D, T, cycle_mode, INT);
   E_CONFIG_VAL(D, T, history_sort_mode, INT);
   E_CONFIG_LIST(D, T, conf_subjects, plugin_conf_edd);
   E_CONFIG_LIST(D, T, conf_actions, plugin_conf_edd);
   E_CONFIG_LIST(D, T, conf_objects, plugin_conf_edd);
   E_CONFIG_LIST(D, T, conf_views,   plugin_conf_edd);
   E_CONFIG_VAL(D, T, first_run, UCHAR);
#undef T
#undef D
   evry_conf = e_config_domain_load("module.everything", conf_edd);

   if (evry_conf)
     {
	if (evry_conf->version <= 7)
	  {
	     evry_conf->scroll_speed = 10.0;
	     evry_conf->version = 8;
	  }

	if (evry_conf->version <= 8)
	  {
	     evry_conf->width = 445;
	     evry_conf->height = 310;
	     evry_conf->rel_y = 0.25;
	     evry_conf->scroll_animate = 1;
	     evry_conf->version = 9;
	  }

	if (evry_conf->version <= 9)
	  {
	     evry_conf->first_run = EINA_TRUE;
	     evry_conf->version = 13;
	  }

	if (evry_conf->version <= 13)
	  {
	     evry_conf->hide_list = 0;
	     evry_conf->version = 14;
	  }

	if (evry_conf->version <= 14)
	  {
	     _plugin_config_free();
	     evry_conf->version = CONFIG_VERSION;
	  }

	if (evry_conf->version != CONFIG_VERSION)
	  {
	     _config_free();
	     evry_conf = NULL;
	  }
     }
   
   if (!evry_conf)
     {
	evry_conf = E_NEW(Evry_Config, 1);
	evry_conf->version = CONFIG_VERSION;
	evry_conf->rel_x = 0.5;
	evry_conf->rel_y = 0.25;
	evry_conf->width = 445;
	evry_conf->height = 310;
	evry_conf->scroll_animate = 1;
	evry_conf->scroll_speed = 10.0;
	evry_conf->hide_input = 0;
	evry_conf->hide_list = 0;
	evry_conf->quick_nav = 1;
	evry_conf->cmd_terminal = eina_stringshare_add("/usr/bin/xterm");
	evry_conf->cmd_sudo = eina_stringshare_add("/usr/bin/gksudo --preserve-env");
	evry_conf->view_mode = VIEW_MODE_DETAIL;
	evry_conf->view_zoom = 0;
	evry_conf->cycle_mode = 0;
	evry_conf->history_sort_mode = 0;
	evry_conf->first_run = EINA_TRUE;
     }
}

static void
_plugin_config_free(void)
{
   Plugin_Config *pc;
   
   EINA_LIST_FREE(evry_conf->conf_subjects, pc)
     {
	if (pc->name) eina_stringshare_del(pc->name);
	if (pc->trigger) eina_stringshare_del(pc->trigger);
	if (pc->plugin) evry_plugin_free(pc->plugin);
	E_FREE(pc);
     }
   EINA_LIST_FREE(evry_conf->conf_actions, pc)
     {
	if (pc->name) eina_stringshare_del(pc->name);
	if (pc->trigger) eina_stringshare_del(pc->trigger);
	if (pc->plugin) evry_plugin_free(pc->plugin);
	E_FREE(pc);
     }
   EINA_LIST_FREE(evry_conf->conf_objects, pc)
     {
	if (pc->name) eina_stringshare_del(pc->name);
	if (pc->trigger) eina_stringshare_del(pc->trigger);
	if (pc->plugin) evry_plugin_free(pc->plugin);
	E_FREE(pc);
     }
}

static void
_config_free(void)
{
   _plugin_config_free();

   if (evry_conf->cmd_terminal)
     eina_stringshare_del(evry_conf->cmd_terminal);
   if (evry_conf->cmd_sudo)
     eina_stringshare_del(evry_conf->cmd_sudo);

   E_FREE(evry_conf);
}


/* action callback */

static int
_e_mod_run_defer_cb(void *data)
{
   E_Zone *zone;

   zone = data;
   if (zone) evry_show(zone, NULL);
   return 0;
}

static void
_e_mod_action_cb(E_Object *obj, const char *params)
{
   E_Zone *zone = NULL;

   if (obj)
     {
	if (obj->type == E_MANAGER_TYPE)
	  zone = e_util_zone_current_get((E_Manager *)obj);
	else if (obj->type == E_CONTAINER_TYPE)
	  zone = e_util_zone_current_get(((E_Container *)obj)->manager);
	else if (obj->type == E_ZONE_TYPE)
	  zone = e_util_zone_current_get(((E_Zone *)obj)->container->manager);
	else
	  zone = e_util_zone_current_get(e_manager_current_get());
     }
   if (!zone) zone = e_util_zone_current_get(e_manager_current_get());

   if (!zone) return;

   if (params && params[0])
     evry_show(zone, params);
   else
     evry_show(zone, NULL);

   /* FIXME popup flickers sometimes when deferes*/
   /* if (params && params[0])
    *   evry_show(zone, params);
    * else
    *   ecore_idle_enterer_add(_e_mod_run_defer_cb, zone); */
}

/* menu item callback(s) */
static void
_e_mod_run_cb(void *data __UNUSED__, E_Menu *m, E_Menu_Item *mi __UNUSED__)
{
   ecore_idle_enterer_add(_e_mod_run_defer_cb, m->zone);
}

/* menu item add hook */
static void
_e_mod_menu_add(void *data __UNUSED__, E_Menu *m)
{
   E_Menu_Item *mi;

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Run Everything"));
   e_util_menu_item_theme_icon_set(mi, "system-run");
   e_menu_item_callback_set(mi, _e_mod_run_cb, NULL);
}


EAPI int evry_api_version_check(int version)
{
   if (EVRY_API_VERSION == version)
     return 1;

   ERR("module API is %d, required is %d", version, EVRY_API_VERSION);

   return 0;
}

static int
_evry_cb_plugin_sort(const void *data1, const void *data2)
{
   const Plugin_Config *pc1 = data1;
   const Plugin_Config *pc2 = data2;

   return pc1->priority - pc2->priority;
}

void
_evry_plugin_free(Evry_Item *it)
{
   GET_EVRY_PLUGIN(p, it);

   evry_plugin_unregister(p);

   DBG("%s", p->name);
   if (p->config) p->config->plugin = NULL;
   if (p->name) eina_stringshare_del(p->name);

   if (p->free)
     p->free(p);
   else
     E_FREE(p);
}

EAPI Evry_Plugin *
evry_plugin_new(Evry_Plugin *base, const char *name, const char *label,
		const char *icon, Evry_Type item_type,
		Evry_Plugin *(*begin) (Evry_Plugin *p, const Evry_Item *item),
		void (*finish) (Evry_Plugin *p),
		int  (*fetch) (Evry_Plugin *p, const char *input),
		void (*cb_free) (Evry_Plugin *p))
{
   Evry_Plugin *p;
   Evry_Item *it;

   if (base)
     p = base;
   else
     p = E_NEW(Evry_Plugin, 1);

   it = evry_item_new(EVRY_ITEM(p), NULL, label, NULL, _evry_plugin_free);
   it->plugin = p;
   it->browseable = EINA_TRUE;

   p->base.icon  = icon;
   p->base.type  = EVRY_TYPE_PLUGIN;

   if (item_type)
     p->base.subtype = item_type;

   p->name = eina_stringshare_add(name);
   p->begin  = begin;
   p->finish = finish;
   p->fetch  = fetch;

   p->async_fetch = EINA_FALSE;
   p->history     = EINA_TRUE;

   p->free = cb_free;

   return p;
}

EAPI void
evry_plugin_free(Evry_Plugin *p)
{
   evry_item_free(EVRY_ITEM(p));
}

/* TODO make int return */
EAPI int
evry_plugin_register(Evry_Plugin *p, int type, int priority)
{
   Eina_List *l;
   Plugin_Config *pc;
   Eina_List *conf[3];
   int i = 0;
   int new_conf = 0;

   if (type < 0 || type > 2)
     return 0;

   conf[0] = evry_conf->conf_subjects;
   conf[1] = evry_conf->conf_actions;
   conf[2] = evry_conf->conf_objects;

   EINA_LIST_FOREACH(conf[type], l, pc)
     if (pc->name && p->name && !strcmp(pc->name, p->name))
       break;

   if (!pc)
     {
	new_conf = 1;
	pc = E_NEW(Plugin_Config, 1);
	pc->name = eina_stringshare_add(p->name);
	pc->enabled = 1;
	pc->priority = priority ? priority : 100;
	pc->view_mode = VIEW_MODE_NONE;
	pc->aggregate = EINA_TRUE;
	pc->top_level = EINA_TRUE;

	conf[type] = eina_list_append(conf[type], pc);
     }
   if (pc->trigger && strlen(pc->trigger) == 0)
     {
	eina_stringshare_del(pc->trigger);
	pc->trigger = NULL;
     }

   p->config = pc;
   pc->plugin = p;

   conf[type] = eina_list_sort(conf[type], -1, _evry_cb_plugin_sort);

   EINA_LIST_FOREACH(conf[type], l, pc)
     pc->priority = i++;

   evry_conf->conf_subjects = conf[0];
   evry_conf->conf_actions = conf[1];
   evry_conf->conf_objects = conf[2];

   if (type == EVRY_PLUGIN_SUBJECT)
     {
	char buf[256];
	snprintf(buf, sizeof(buf), _("Show %s Plugin"), p->name);

	e_action_predef_name_set(_("Everything Launcher"), buf,
				 "everything", p->name, NULL, 1);
     }

   return new_conf;
}

EAPI void
evry_plugin_unregister(Evry_Plugin *p)
{
   DBG("%s", p->name);
   Eina_List *l = evry_conf->conf_subjects;

   if (l && eina_list_data_find_list(l, p->config))
     {
	char buf[256];
   	snprintf(buf, sizeof(buf), _("Show %s Plugin"), p->name);

   	e_action_predef_name_del(_("Everything"), buf);
     }
}



static int
_evry_cb_view_sort(const void *data1, const void *data2)
{
   const Evry_View *v1 = data1;
   const Evry_View *v2 = data2;
   return v1->priority - v2->priority;
}


EAPI void
evry_view_register(Evry_View *view, int priority)
{
   view->priority = priority;

   evry_conf->views = eina_list_append(evry_conf->views, view);

   evry_conf->views = eina_list_sort(evry_conf->views,
				     eina_list_count(evry_conf->views),
				     _evry_cb_view_sort);
}

EAPI void
evry_view_unregister(Evry_View *view)
{
   evry_conf->views = eina_list_remove(evry_conf->views, view);
}
