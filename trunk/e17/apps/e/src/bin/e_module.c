/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "config.h"

/* TODO List:
 * 
 * * add module types/classes
 * * add list of exclusions that a module cant work with
 * 
 */

typedef struct _Module_Menu_Data Module_Menu_Data;

struct _Module_Menu_Data
{
   Evas_List *submenus;
};

/* local subsystem functions */
static void _e_module_free(E_Module *m);
static E_Menu *_e_module_control_menu_new(E_Module *mod);
static void _e_module_menu_free(void *obj);
static void _e_module_control_menu_about(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_module_control_menu_enabled(void *data, E_Menu *m, E_Menu_Item *mi);

/* local subsystem globals */
static Evas_List *_e_modules = NULL;
static E_Path    *_e_path_modules  = NULL;

static E_Module_Api _e_module_api =
{
   E_MODULE_API_VERSION
};

/* externally accessible functions */
int
e_module_init(void)
{
   Evas_List *l;
   
   _e_path_modules = e_path_new();
   if (!_e_path_modules) return 0;
   e_path_path_append(_e_path_modules, "~/.e/e/modules");
   e_path_path_append(_e_path_modules, PACKAGE_LIB_DIR"/enlightenment/modules");
   e_path_path_append(_e_path_modules, PACKAGE_LIB_DIR"/enlightenment/modules_extra");
   
   for (l = e_config->modules; l; l = l->next)
     {
	E_Config_Module *em;
	E_Module *m;
	
	em = l->data;
	m = e_module_new(em->name);
	if ((em->enabled) && (m)) e_module_enable(m);
     }
   
   return 1;
}

int
e_module_shutdown(void)
{
   Evas_List *l, *tmp;
   for (l = _e_modules; l;)
     {
	tmp = l;
	l = l->next;
	e_object_del(E_OBJECT(tmp->data));
     }
   e_object_del(E_OBJECT(_e_path_modules));
   _e_path_modules = NULL;
   return 1;
}

E_Module *
e_module_new(char *name)
{
   E_Module *m;
   char buf[4096];
   char *modpath, *s;
   Evas_List *l;
   int in_list = 0;

   if (!name) return NULL;
   m = E_OBJECT_ALLOC(E_Module, E_MODULE_TYPE, _e_module_free);
   m->api = &_e_module_api;
   if (name[0] != '/')
     {
	snprintf(buf, sizeof(buf), "%s/%s/module.so", name, MODULE_ARCH);
	modpath = e_path_find(_e_path_modules, buf);
     }
   else
     modpath = name;
   if (!modpath)
     {
	e_error_dialog_show("Error loading Module",
			    "There was an error loading module named: %s\n"
			    "No module named %s could be found in the\n"
			    "module search directories\n",
			    name, buf);
	free(m);
	return NULL;
     }
   m->handle = dlopen(modpath, RTLD_NOW | RTLD_LOCAL);
   if (!m->handle)
     {
	e_error_dialog_show("Error loading Module",
			    "There was an error loading module named: %s\n"
			    "The full path to this module is:\n"
			    "%s\n"
			    "The error reported was:\n"
			    "%s",
			    name, buf, dlerror());
	free(m);
	return NULL;
     }
   m->func.init = dlsym(m->handle, "init");
   m->func.shutdown = dlsym(m->handle, "shutdown");
   m->func.save = dlsym(m->handle, "save");
   m->func.info = dlsym(m->handle, "info");
   m->func.about = dlsym(m->handle, "about");
   if ((!m->func.init) ||
       (!m->func.shutdown) ||
       (!m->func.save) ||
       (!m->func.info) ||
       (!m->func.about)
       )
     {
	e_error_dialog_show("Error loading Module",
			    "There was an error loading module named: %s\n"
			    "The full path to this module is:\n"
			    "%s\n"
			    "The error reported was:\n"
			    "%s",
			    name, buf, dlerror());
	dlclose(m->handle);
	free(m);
	return NULL;
     }
   _e_modules = evas_list_append(_e_modules, m);
   m->name = strdup(name);
   s = ecore_file_get_dir(modpath);
   if (s)
     {
	m->dir = ecore_file_get_dir(s);
	free(s);
     }
   m->func.info(m);
   for (l = e_config->modules; l; l = l->next)
     {
	E_Config_Module *em;
	
	em = l->data;
	if (!strcmp(em->name, m->name))
	  {
	     in_list = 1;
	     break;
	  }
     }
   if (!in_list)
     {
	E_Config_Module *em;
	
	em = E_NEW(E_Config_Module, 1);
	em->name = strdup(m->name);
	em->enabled = 0;
	e_config->modules = evas_list_append(e_config->modules, em);
	e_config_save_queue();
     }
   return m;
}

int
e_module_save(E_Module *m)
{
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MODULE_TYPE, 0);
   return m->func.save(m);
}

const char *
e_module_dir_get(E_Module *m)
{
   E_OBJECT_CHECK_RETURN(m, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MODULE_TYPE, 0);
   return m->dir;
}

int
e_module_enable(E_Module *m)
{
   Evas_List *l;
   
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MODULE_TYPE, 0);
   if (m->enabled) return 0;
   m->data = m->func.init(m);
   if (m->data) m->enabled = 1;
   for (l = e_config->modules; l; l = l->next)
     {
	E_Config_Module *em;
	
	em = l->data;
	if (!strcmp(em->name, m->name))
	  {
	     em->enabled = 1;
	     e_config_save_queue();
	     break;
	  }
     }
   return 1;
}

int
e_module_disable(E_Module *m)
{
   Evas_List *l;
   int ret;
   
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MODULE_TYPE, 0);
   if (!m->enabled) return 0;
   ret = m->func.shutdown(m);
   m->data = NULL;
   m->enabled = 0;
   for (l = e_config->modules; l; l = l->next)
     {
	E_Config_Module *em;
	
	em = l->data;
	if (!strcmp(em->name, m->name))
	  {
	     em->enabled = 0;
	     e_config_save_queue();
	     break;
	  }
     }
   return ret;
}

int
e_module_enabled_get(E_Module *m)
{
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MODULE_TYPE, 0);
   return m->enabled;
}

int
e_module_save_all(void)
{
   Evas_List *l;
   int ret = 1;
   
   for (l = _e_modules; l; l = l->next) e_object_ref(E_OBJECT(l->data));
   for (l = _e_modules; l; l = l->next)
     {
	E_Module *m;
	
	m = l->data;
	if (m->enabled)
	  {
	     if (!m->func.save(m)) ret = 0;
	  }
     }
   for (l = _e_modules; l; l = l->next) e_object_unref(E_OBJECT(l->data));
   return ret;
}

E_Module *
e_module_find(char *name)
{
   Evas_List *l;
   
   if (!name) return NULL;
   for (l = _e_modules; l; l = l->next)
     {
	E_Module *m;
	
	m = l->data;
	if (!strcmp(name, m->name)) return m;
     }
   return NULL;
}

Evas_List *
e_module_list(void)
{
   return _e_modules;
}

E_Menu *
e_module_menu_new(void)
{
   E_Menu *m, *subm;
   E_Menu_Item *mi;
   Evas_List *l;
   Module_Menu_Data *dat;
   
   dat = calloc(1, sizeof(Module_Menu_Data));
   m = e_menu_new();
   e_object_data_set(E_OBJECT(m), dat);
   e_object_free_attach_func_set(E_OBJECT(m), _e_module_menu_free);
   for (l = _e_modules; l; l = l->next)
     {
	E_Module *mod;
	
	mod = l->data;
	mi = e_menu_item_new(m);
	if (mod->label) e_menu_item_label_set(mi, mod->label);
	else e_menu_item_label_set(mi, mod->name);
	if (mod->edje_icon_file)
	  {
	     if (mod->edje_icon_key)
	       e_menu_item_icon_edje_set(mi, mod->edje_icon_file, mod->edje_icon_key);
	     else
	       e_menu_item_icon_edje_set(mi, mod->edje_icon_file, "icon");
	  }
	else if (mod->icon_file)
	  e_menu_item_icon_file_set(mi, mod->icon_file);
	subm = _e_module_control_menu_new(mod);
	e_menu_item_submenu_set(mi, subm);
	dat->submenus = evas_list_append(dat->submenus, subm);
     }
   return m;
}

/* local subsystem functions */

static void
_e_module_free(E_Module *m)
{
   Evas_List *l;
   
   for (l = e_config->modules; l; l = l->next)
     {
	E_Config_Module *em;
	
	em = l->data;
	if (!strcmp(em->name, m->name))
	  {
	     e_config->modules = evas_list_remove(e_config->modules, em);
	     E_FREE(em->name);
	     E_FREE(em);
	     /* FIXME
	      * This is crap, a job is added, but doesn't run because
	      * main loop has quit!
	     e_config_save_queue();
	     */
	     break;
	  }
     }
   
   if (m->enabled)
     {
	m->func.save(m);
	m->func.shutdown(m);
     }
   if (m->name) free(m->name);
   if (m->dir) free(m->dir);
   dlclose(m->handle);
   _e_modules = evas_list_remove(_e_modules, m);
   if (m->label) free(m->label);
   if (m->icon_file) free(m->icon_file);
   if (m->edje_icon_file) free(m->edje_icon_file);
   if (m->edje_icon_key) free(m->edje_icon_key);
   free(m);
}

static E_Menu *
_e_module_control_menu_new(E_Module *mod)
{
   E_Menu *m;
   E_Menu_Item *mi;
   
   m = e_menu_new();
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, "About...");
   e_menu_item_callback_set(mi, _e_module_control_menu_about, mod);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, "Enabled");
   e_menu_item_check_set(mi, 1);
   if (mod->enabled) e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, _e_module_control_menu_enabled, mod);

   if (mod->config_menu)
     {
	mi = e_menu_item_new(m);
	e_menu_item_separator_set(mi, 1);
	
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, "Configuration");
	e_menu_item_submenu_set(mi, mod->config_menu);
     }
   return m;
}

static void
_e_module_menu_free(void *obj)
{
   Module_Menu_Data *dat;
   
   dat = e_object_data_get(E_OBJECT(obj));
   while (dat->submenus)
     {
	E_Menu *subm;
	
	subm = dat->submenus->data;
	dat->submenus = evas_list_remove_list(dat->submenus, dat->submenus);
	e_object_del(E_OBJECT(subm));
     }
   free(dat);
}

static void
_e_module_control_menu_about(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Module *mod;
   
   mod = data;
   mod->func.about(mod);
}

static void
_e_module_control_menu_enabled(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Module *mod;
   int enabled;
   
   mod = data;
   enabled = e_menu_item_toggle_get(mi);
   if ((mod->enabled) && (!enabled))
     {
	e_module_save(mod);
	e_module_disable(mod);
     }
   else if ((!mod->enabled) && (enabled))
     {
	e_module_enable(mod);
     }
   e_menu_item_toggle_set(mi, e_module_enabled_get(mod));
}
