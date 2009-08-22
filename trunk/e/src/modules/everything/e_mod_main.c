/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* TODO
   - watch plugin directories
   - get plugins from ~/.e/e/everything_plugins
 */

#include "e_mod_main.h"

#define CONFIG_VERSION 2

/* actual module specifics */
static void _e_mod_action_exebuf_cb(E_Object *obj, const char *params);
static int  _e_mod_run_defer_cb(void *data);
static void _e_mod_run_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_mod_menu_add(void *data, E_Menu *m);

static E_Int_Menu_Augmentation *maug = NULL;
static E_Action *act = NULL; 

static Eina_Array  *plugins = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *evry_conf = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
  {
    E_MODULE_API_VERSION,
    "Everything"
  };


static Eina_Bool list_cb(Eina_Module *m, void *data)
{
   if (eina_module_load(m))
     return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void *
e_modapi_init(E_Module *m)
{
   Eina_List *files;
   char buf[4096], dir[4096];
   char *file;
   E_Action *act;

   snprintf(buf, sizeof(buf), "%s/.e/e/config/%s/module.everything",
	    e_user_homedir_get(), e_config_profile_get());
   ecore_file_mkdir(buf);

   conf_item_edd = E_CONFIG_DD_NEW("Plugin_Config", Plugin_Config);
#undef T
#undef D
#define T Plugin_Config
#define D conf_item_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, trigger, STR);
   E_CONFIG_VAL(D, T, min_query, INT);
   E_CONFIG_VAL(D, T, loaded, INT);
   E_CONFIG_VAL(D, T, enabled, INT);
   E_CONFIG_VAL(D, T, priority, INT);
   conf_edd = E_CONFIG_DD_NEW("Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
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
   E_CONFIG_LIST(D, T, conf_subjects, conf_item_edd);
   E_CONFIG_LIST(D, T, conf_actions, conf_item_edd);
   E_CONFIG_LIST(D, T, conf_objects, conf_item_edd);
#undef T
#undef D
   evry_conf = e_config_domain_load("module.everything", conf_edd);

   if (!evry_conf || (evry_conf->version != CONFIG_VERSION))
     {
	evry_conf = E_NEW(Config, 1);
	evry_conf->version = CONFIG_VERSION;
	evry_conf->rel_x = 50.0;
	evry_conf->rel_y = 50.0;
	evry_conf->width = 400;
	evry_conf->height = 350;
	evry_conf->scroll_animate = 0;
	evry_conf->scroll_speed = 0.08;
	evry_conf->hide_input = 0;
	evry_conf->hide_list = 1;
	evry_conf->quick_nav = 1;
	evry_conf->conf_subjects = NULL;
	evry_conf->conf_actions = NULL;
	evry_conf->conf_objects = NULL;	
     }

   /* search for plugins */
   eina_module_init();

   snprintf(dir, sizeof(dir), "%s/enlightenment/everything_plugins", e_prefix_lib_get());
   files = ecore_file_ls(dir);

   EINA_LIST_FREE(files, file)
     {
	snprintf(buf, sizeof(buf), "%s/%s/%s", dir, file, MODULE_ARCH);

	if (ecore_file_is_dir(buf))
	  plugins = eina_module_list_get(plugins, buf, 1, &list_cb, NULL);

	free(file);
     }

   /* add module supplied action */
   act = e_action_add("everything");
   if (act)
     {
	act->func.go = _e_mod_action_exebuf_cb;
     }

   e_action_predef_name_set(_("Everything"), _("Show Everything Dialog"),
			    "everything", "", NULL, 0);
   
   maug = e_int_menus_menu_augmentation_add("main/1", _e_mod_menu_add, NULL, NULL, NULL);

   e_configure_registry_category_add("extensions", 80, _("Extensions"),
				     NULL, "preferences-extensions");

   e_configure_registry_item_add("extensions/run_everything", 40, _("Run Everything"),
				 NULL, "system-run", evry_config_dialog);

   evry_init();

   e_module_delayed_set(m, 1);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;

   evry_shutdown();
   
   /* remove module-supplied menu additions */
   if (maug)
     {
   	e_int_menus_menu_augmentation_del("main/1", maug);
   	maug = NULL;
     }
   /* remove module-supplied action */
   if (act)
     {
	e_action_predef_name_del(_("Everything"), _("Show Everything Dialog"));
	e_action_del("everything");
     }

   if (plugins)
     {
	eina_module_list_unload(plugins);
	eina_module_list_flush(plugins);
	eina_array_free(plugins);
     }

   eina_module_shutdown();

   while ((cfd = e_config_dialog_get("E", "_config_everything_dialog")))
     e_object_del(E_OBJECT(cfd));

   e_configure_registry_item_del("extensions/run_everything");
   e_configure_registry_category_del("extensions");

   if (evry_conf->plugins)       eina_list_free(evry_conf->plugins);
   if (evry_conf->actions)       eina_list_free(evry_conf->actions);
   if (evry_conf->conf_subjects) eina_list_free(evry_conf->conf_subjects);
   if (evry_conf->conf_actions)  eina_list_free(evry_conf->conf_actions);
   if (evry_conf->conf_objects)  eina_list_free(evry_conf->conf_objects);
   
   E_FREE(evry_conf);
   
   /* Clean EET */
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.everything", conf_edd, evry_conf);
   return 1;
}

/* action callback */
static void
_e_mod_action_exebuf_cb(E_Object *obj, const char *params)
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
}

/* menu item callback(s) */
static int
_e_mod_run_defer_cb(void *data)
{
   E_Zone *zone;

   zone = data;
   if (zone) evry_show(zone, NULL);
   return 0;
}

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


static int
_evry_cb_plugin_sort(const void *data1, const void *data2)
{
   const Evry_Plugin *p1 = data1;
   const Evry_Plugin *p2 = data2;
   return p1->config->priority - p2->config->priority;
}

Evry_Plugin *
evry_plugin_new(Evry_Plugin *base, const char *name, int type,
		const char *type_in, const char *type_out,
		int async_fetch, const char *icon, const char *trigger,
		Evry_Plugin *(*begin) (Evry_Plugin *p, const Evry_Item *item),
		void (*cleanup) (Evry_Plugin *p),
		int  (*fetch) (Evry_Plugin *p, const char *input),
		int  (*action) (Evry_Plugin *p, const Evry_Item *item),
		Evas_Object *(*icon_get) (Evry_Plugin *p, const Evry_Item *it, Evas *e),
		Evas_Object *(*config_page) (Evry_Plugin *p),
		void (*config_apply) (Evry_Plugin *p))
{
   Evry_Plugin *p;

   if (base)
     p = base;
   else
     p = E_NEW(Evry_Plugin, 1);

   p->name = eina_stringshare_add(name);
   p->type = type;
   p->type_in  = (type_in  ? eina_stringshare_add(type_in)  : NULL);
   p->type_out = (type_out ? eina_stringshare_add(type_out) : NULL);
   p->trigger  = (trigger  ? eina_stringshare_add(trigger)  : NULL);
   p->icon     = (icon     ? eina_stringshare_add(icon)     : NULL);
   p->async_fetch = async_fetch;
   p->begin    = begin;
   p->cleanup  = cleanup;
   p->fetch    = fetch;
   p->icon_get = icon_get;
   p->action   = action;
   p->config_page  = config_page;
   p->config_apply = config_apply;
 
   return p;
}

void
evry_plugin_free(Evry_Plugin *p, int free_pointer)
{
   evry_plugin_unregister(p);

   if (p->name)     eina_stringshare_del(p->name);
   if (p->type_in)  eina_stringshare_del(p->type_in);
   if (p->type_out) eina_stringshare_del(p->type_out);
   if (p->trigger)  eina_stringshare_del(p->trigger);
   if (p->icon)     eina_stringshare_del(p->icon);

   if (p->config)
     {
	if (p->config->name)
	  eina_stringshare_del(p->config->name);
	if (p->config->trigger)
	  eina_stringshare_del(p->config->trigger);

	E_FREE(p->config);
     }

   if (free_pointer)
     E_FREE(p);
}


Evry_Action *
evry_action_new(const char *name, const char *type_in1, const char *type_in2,
		const char *type_out, const char *icon,
		int  (*action) (Evry_Action *act),
		int (*check_item) (Evry_Action *act, const Evry_Item *it),
		void (*cleanup)    (Evry_Action *act),
		int  (*intercept)  (Evry_Action *act),
		Evas_Object *(*icon_get) (Evry_Action *act, Evas *e))
{
   Evry_Action *act = E_NEW(Evry_Action, 1);
   act->name = eina_stringshare_add(name);
   act->type_in1 = (type_in1 ? eina_stringshare_add(type_in1) : NULL);
   act->type_in2 = (type_in2 ? eina_stringshare_add(type_in2) : NULL);
   act->type_out = (type_out ? eina_stringshare_add(type_out) : NULL);
   act->action = action;
   act->check_item = check_item;
   act->intercept = intercept;
   act->cleanup = cleanup;
   act->icon = (icon ? eina_stringshare_add(icon) : NULL);

   return act;
}


void
evry_action_free(Evry_Action *act)
{
   evry_action_unregister(act);

   if (act->name)     eina_stringshare_del(act->name);
   if (act->type_in1) eina_stringshare_del(act->type_in1);
   if (act->type_in2) eina_stringshare_del(act->type_in2);
   if (act->type_out) eina_stringshare_del(act->type_out);
   if (act->icon)     eina_stringshare_del(act->icon);

   E_FREE(act);
}

void
evry_plugin_register(Evry_Plugin *p, int priority)
{
   Eina_List *l, *confs = NULL;   
   Plugin_Config *pc;
   Eina_Bool found = 0;

   evry_conf->plugins = eina_list_append(evry_conf->plugins, p);

   if (p->type == type_subject)
     confs = evry_conf->conf_subjects;
   else if (p->type == type_action)
     confs = evry_conf->conf_actions;
   else if (p->type == type_object)
     confs = evry_conf->conf_objects;
   
   EINA_LIST_FOREACH(confs, l, pc)
     {
	if (pc->name && p->name && !strcmp(pc->name, p->name))
	  {
	     found = 1;
	     break;
	  }
     }

   if (!found)
     {
	pc = E_NEW(Plugin_Config, 1);
	pc->name = eina_stringshare_add(p->name);
	pc->enabled = 1;
	pc->priority = priority ? priority : 100;;

	confs = eina_list_append(confs, pc);
     }

   /* if (plugin->trigger && !pc->trigger)
    *   pc->trigger = eina_stringshare_add(plugin->trigger); */

   p->config = pc;

   evry_conf->plugins = eina_list_sort(evry_conf->plugins,
				       eina_list_count(evry_conf->plugins),
				       _evry_cb_plugin_sort);

   if (p->type == type_subject)
     evry_conf->conf_subjects = confs;
   else if (p->type == type_action)
     evry_conf->conf_actions = confs;
   else if (p->type == type_object)
     evry_conf->conf_objects = confs;
   
   if (p->type == type_subject)
     {
	char buf[256];
	snprintf(buf, sizeof(buf), "Show %s Plugin", p->name);
	
	e_action_predef_name_set(_("Everything"), buf,
				 "everything", p->name, NULL, 1);
     }
   /* TODO sorting, initialization, etc */
}

void
evry_plugin_unregister(Evry_Plugin *p)
{
   evry_conf->plugins = eina_list_remove(evry_conf->plugins, p);

   if (p->type == type_subject)
     {
	char buf[256];
	snprintf(buf, sizeof(buf), "Show %s Plugin", p->name);

	e_action_predef_name_del(_("Everything"), buf);
     }
   /* cleanup */
}

void
evry_action_register(Evry_Action *action)
{
   evry_conf->actions = eina_list_append(evry_conf->actions, action);
   /* TODO sorting, initialization, etc */
}

void
evry_action_unregister(Evry_Action *action)
{
   evry_conf->actions = eina_list_remove(evry_conf->actions, action);
   /* cleanup */
}


static int
_evry_cb_view_sort(const void *data1, const void *data2)
{
   const Evry_View *v1 = data1;
   const Evry_View *v2 = data2;
   return v1->priority - v2->priority;
}


void
evry_view_register(Evry_View *view, int priority)
{
   view->priority = priority;

   evry_conf->views = eina_list_append(evry_conf->views, view);

   evry_conf->views = eina_list_sort(evry_conf->views,
				     eina_list_count(evry_conf->views),
				     _evry_cb_view_sort);
}

void
evry_view_unregister(Evry_View *view)
{
   evry_conf->views = eina_list_remove(evry_conf->views, view);
}


/* taken from e_utils. just changed 48 to 72.. we need
   evry_icon_theme_set(Evas_Object *obj, const char *icon,
   size:small, mid, large) imo */
static int
_evry_icon_theme_set(Evas_Object *obj, const char *icon)
{
   const char *file;
   char buf[4096];

   if ((!icon) || (!icon[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", icon);
   file = e_theme_edje_file_get("base/theme/icons", buf);
   if (file[0])
     {
	e_icon_file_edje_set(obj, file, buf);
	return 1;
     }
   return 0;
}

static int
_evry_icon_fdo_set(Evas_Object *obj, const char *icon)
{
   char *path = NULL;
   unsigned int size;

   if ((!icon) || (!icon[0])) return 0;
   size = e_util_icon_size_normalize(72 * e_scale);
   path = efreet_icon_path_find(e_config->icon_theme, icon, size);

   if (!path) return 0;
   e_icon_file_set(obj, path);
   E_FREE(path);
   return 1;
}

Evas_Object *
evry_icon_theme_get(const char *icon, Evas *e)
{
   Evas_Object *o = e_icon_add(e);

   if (e_config->icon_theme_overrides)
     {
	if (_evry_icon_fdo_set(o, icon) ||
	    _evry_icon_theme_set(o, icon))
	  return o;
     }
   else
     {
	if (_evry_icon_theme_set(o, icon) ||
	    _evry_icon_fdo_set(o, icon))
	  return o;
     }

   evas_object_del(o);

   return NULL;
}

Evas_Object *
evry_icon_mime_get(const char *mime, Evas *e)
{
   char *icon;

   icon = efreet_mime_type_icon_get(mime, e_config->icon_theme, 64);

   if (icon)
     return e_util_icon_add(icon, e);
   else
     return evry_icon_theme_get("none", e);

   free(icon);

   return NULL;
}

