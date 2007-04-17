/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#define MOD_UNLOADED 0
#define MOD_ENABLED 1

typedef struct _CFModule CFModule;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

static void _ilist_cb_change(void *data, Evas_Object *obj);
static int _sort_modules(void *data1, void *data2);
static void _module_configure(void *data, void *data2);
static void _module_about(void *data, void *data2);

static void _module_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path);
static void _mod_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path);
static void _dir_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path);
static void _load_modules(E_Config_Dialog_Data *cfdata);
static void _fill_list(E_Config_Dialog_Data *cfdata);

static Evas_List *monitors;
Ecore_File_Monitor *mod_mon, *dir_mon;

struct _CFModule
{
   char           *name;
   Efreet_Desktop *desktop;
   int             state;
};

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas_List *modules;
   Evas *evas;
   int state;
   char *modname;
   struct {
      Evas_Object *configure, *about;
      Evas_Object *enabled, *unloaded, *list;
   } gui;
};

EAPI E_Config_Dialog *
e_int_config_modules(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "_config_modules_dialog")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->basic.apply_cfdata      = _basic_apply_data;
   
   cfd = e_config_dialog_new(con,
			     _("Module Settings"),
			    "E", "_config_modules_dialog",
			     "enlightenment/modules", 0, v, NULL);
   return cfd;
}

static void
_module_configure(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Module *m;

   cfdata = data;
   if (!cfdata->modname) return;
   m = e_module_find(cfdata->modname);
   if (m)
     {
	if (m->func.config) m->func.config(m);
     }
}

static void
_module_about(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Module *m;

   cfdata = data;
   if (!cfdata->modname) return;
   m = e_module_find(cfdata->modname);
   if (m)
     {
	if (m->func.about) m->func.about(m);
     }
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   for (l = e_path_dir_list_get(path_modules); l; l = l->next)
     {
	E_Path_Dir *epd;

	epd = l->data;
	if (ecore_file_is_dir(epd->dir)) 
	  {
	     Ecore_File_Monitor *monitor;
	     monitor = ecore_file_monitor_add(epd->dir, _module_cb_monitor, 
					      cfdata);
	     monitors = evas_list_append(monitors, monitor);
	  }
     }
      
   _load_modules(cfdata);
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   if (mod_mon)
     ecore_file_monitor_del(mod_mon);
   if (dir_mon)
     ecore_file_monitor_del(dir_mon);
   
   while (monitors) 
     {
	Ecore_File_Monitor *mon;
	
	mon = monitors->data;
	ecore_file_monitor_del(mon);
	monitors = evas_list_remove_list(monitors, monitors);
     }
   
   while ((l = cfdata->modules))
     {
        CFModule *m;

        m = l->data;
        cfdata->modules = evas_list_remove_list(cfdata->modules, l);
        E_FREE(m->name);
        E_FREE(m);
     }
   E_FREE(cfdata->modname);
   E_FREE(cfdata);
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Module *m;
   const char *v;
   int i;

   v = cfdata->modname;
   if (!v) return 0;
   for (i = 0; i < evas_list_count(cfdata->modules); i++)
     {
	CFModule *cm;

	cm = evas_list_nth(cfdata->modules, i);
	if ((cm) && (!strcmp(cm->name, v)))
	  {
	     if (cm->state != cfdata->state)
	       {
		  e_widget_disabled_set(cfdata->gui.configure, 1);
		  e_widget_disabled_set(cfdata->gui.about, 1);
		  
		  m = e_module_find(v);
		  if (!m) 
		    { 
		       m = e_module_new(v);
		       if (!m) break;
		    }		  
		  switch (cfdata->state)
		    {
		     case MOD_ENABLED:
		       if (!m->enabled) 
			 {
			    if (!e_module_enable(m)) 
			      {
				 cm->state = MOD_UNLOADED;
				 break;
			      }
			 }
		       if (m->enabled) 
			 {	 
			    if (m->func.config)
			      e_widget_disabled_set(cfdata->gui.configure, 0);
			    if (m->func.about)
			      e_widget_disabled_set(cfdata->gui.about, 0);
			    cm->state = MOD_ENABLED;
			 }
		       break;
		     case MOD_UNLOADED:
		       if (m)
			 {
			    if (m->func.config)
			      e_widget_disabled_set(cfdata->gui.configure, 1);
			    if (m->func.about)
			      e_widget_disabled_set(cfdata->gui.about, 1);
			    e_module_disable(m);
			    e_object_del(E_OBJECT(m));
			    cm->state = MOD_UNLOADED;			    
			 }
		       break;
		    }
	       }
	     break;
	  }
     }
   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob, *ot, *ilist, *mt;
   E_Radio_Group *rg;

   cfdata->evas = evas;
   
   o = e_widget_list_add(evas, 1, 0);
   ot = e_widget_table_add(evas, 1);
   
   of = e_widget_framelist_add(evas, _("Modules"), 1);
   ilist = e_widget_ilist_add(evas, 24, 24, &(cfdata->modname));
   cfdata->gui.list = ilist;
   e_widget_on_change_hook_set(ilist, _ilist_cb_change, cfdata);

   cfdata->state = -1;

   _fill_list(cfdata);
   
   e_widget_framelist_object_append(of, ilist);
   e_widget_table_object_append(ot, of, 0, 0, 1, 2, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, _("Module State"), 0);
   mt = e_widget_table_add(evas, 0);
   
   rg = e_widget_radio_group_new(&(cfdata->state));

   ob = e_widget_radio_add(evas, _("Enabled"), MOD_ENABLED, rg);
   cfdata->gui.enabled = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_table_object_append(mt, ob, 0, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_radio_add(evas, _("Disabled"), MOD_UNLOADED, rg);
   cfdata->gui.unloaded = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_table_object_append(mt, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   
   e_widget_framelist_object_append(of, mt);
   e_widget_table_object_append(ot, of, 1, 0, 1, 1, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, _("Module Actions"), 0);
   mt = e_widget_table_add(evas, 0); 
   
   ob = e_widget_button_add(evas, _("Configure"), NULL, _module_configure, cfdata, NULL);
   cfdata->gui.configure = ob;
   e_widget_table_object_append(mt, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_button_add(evas, _("About"), NULL, _module_about, cfdata, NULL);
   cfdata->gui.about = ob;
   e_widget_table_object_append(mt, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   e_widget_disabled_set(cfdata->gui.configure, 1);
   e_widget_disabled_set(cfdata->gui.about, 1);

   e_widget_framelist_object_append(of, mt);
   e_widget_table_object_append(ot, of, 1, 1, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(o, ot, 1, 1, 0.5);

   return o;
}

static void
_ilist_cb_change(void *data, Evas_Object *obj)
{
   E_Module *m;
   E_Config_Dialog_Data *cfdata;
   Evas_List *l;
   const char *v;

   cfdata = data;
   v = cfdata->modname;
   if (!v) return;
   for (l = cfdata->modules; l; l = l->next)
     {
	CFModule *cm;

	cm = l->data;
	if ((cm) && (!strcmp(cm->name, v)))
	  {
	     cfdata->state = cm->state;
	     e_widget_disabled_set(cfdata->gui.enabled, 0);
	     e_widget_disabled_set(cfdata->gui.unloaded, 0);
	     switch (cm->state)
	       {
		case MOD_ENABLED:
		  e_widget_radio_toggle_set(cfdata->gui.enabled, 1);
		  e_widget_radio_toggle_set(cfdata->gui.unloaded, 0);
		  break;
		case MOD_UNLOADED:
		  e_widget_radio_toggle_set(cfdata->gui.unloaded, 1);
		  e_widget_radio_toggle_set(cfdata->gui.enabled, 0);
		  break;
	       }
	     e_widget_disabled_set(cfdata->gui.about, 1);
	     e_widget_disabled_set(cfdata->gui.configure, 1);
	     m = e_module_find(v);
	     if (m)
	       {
		  if (m->func.about)
		    e_widget_disabled_set(cfdata->gui.about, 0);
		  if (m->enabled && m->func.config) 
		    e_widget_disabled_set(cfdata->gui.configure, 0);
	       }
	     break;
	  }
     }
}

static int
_sort_modules(void *data1, void *data2)
{
   CFModule *m1, *m2;

   if (!data1) return 1;
   if (!data2) return -1;

   m1 = data1;
   m2 = data2;
   return (strcmp(m1->name, m2->name));
}

static void 
_module_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path) 
{
   E_Config_Dialog_Data *cfdata;
   const char *file;
   
   cfdata = data;
   if (!cfdata) return;
   
   file = ecore_file_get_file(path);
   switch (event) 
     {
      case ECORE_FILE_EVENT_CREATED_DIRECTORY:
	if (mod_mon) ecore_file_monitor_del(mod_mon);
	mod_mon = ecore_file_monitor_add(path, _mod_cb_monitor, cfdata);
	break;
      case ECORE_FILE_EVENT_DELETED_DIRECTORY:
	_load_modules(cfdata);
	_fill_list(cfdata);	
	break;
      default:
	break;
     }
}

static void 
_mod_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path) 
{
   E_Config_Dialog_Data *cfdata;
   const char *f;
   
   cfdata = data;
   if (!cfdata) return;

   switch (event) 
     {
      case ECORE_FILE_EVENT_CREATED_DIRECTORY:
	f = ecore_file_get_file(path);
	if (!e_util_glob_case_match(f, MODULE_ARCH)) break;
	if (!dir_mon)
	  dir_mon = ecore_file_monitor_add(path, _dir_cb_monitor, cfdata);
	break;
      default:
	break;
     }
}

static void 
_dir_cb_monitor(void *data, Ecore_File_Monitor *mon, Ecore_File_Event event, const char *path) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if (!cfdata) return;
   
   switch (event) 
     {
      case ECORE_FILE_EVENT_CREATED_FILE:
	if (e_util_glob_case_match(path, "*.so")) 
	  {
	     ecore_file_monitor_del(dir_mon);
	     dir_mon = NULL;
	     if (mod_mon) ecore_file_monitor_del(mod_mon);
	     mod_mon = NULL;

	     _load_modules(cfdata);
	     _fill_list(cfdata);
	  }
	break;
      default:
	break;
     }
}

static void 
_load_modules(E_Config_Dialog_Data *cfdata) 
{
   Evas_List *l;
   Ecore_List *dirs = NULL;

   while ((l = cfdata->modules))
     {
	CFModule *m;

	m = l->data;
	cfdata->modules = evas_list_remove_list(cfdata->modules, l);
	E_FREE(m->name);
	E_FREE(m);
     }

   for (l = e_path_dir_list_get(path_modules); l; l = l->next)
     {
	E_Path_Dir *epd;

	epd = l->data;
	if (ecore_file_is_dir(epd->dir))
	  {
	     dirs = ecore_file_ls(epd->dir);
	     if (dirs)
	       {
		  char *mod;

		  ecore_list_goto_first(dirs);
		  while ((mod = ecore_list_next(dirs)))
		    {
		       Efreet_Desktop *ef;
		       CFModule *m;
		       char buf[4096];

		       snprintf(buf, sizeof(buf), "%s/%s/module.desktop", epd->dir, mod);
		       if (!ecore_file_exists(buf)) continue;
		       ef = efreet_desktop_get(buf);
		       if (!ef) continue;

		       m = E_NEW(CFModule, 1);
		       m->desktop = ef;
		       m->name = strdup(mod);
		       cfdata->modules = 
			  evas_list_append(cfdata->modules, m);
		    }
		  ecore_list_destroy(dirs);
	       }
	  }
     }

   if (cfdata->modules)
     cfdata->modules = evas_list_sort(cfdata->modules, 
				      evas_list_count(cfdata->modules), 
				      _sort_modules);
   
   /* Free Lists */
   if (l) evas_list_free(l);
}

static void
_fill_list(E_Config_Dialog_Data *cfdata) 
{
   E_Module *m;
   Evas_List *l;
   char buf[4096];

   if (!cfdata->gui.list) return;

   evas_event_freeze(evas_object_evas_get(cfdata->gui.list));
   edje_freeze();
   e_widget_ilist_freeze(cfdata->gui.list);
   
   e_widget_ilist_clear(cfdata->gui.list);
   e_widget_ilist_go(cfdata->gui.list);

   for (l = cfdata->modules; l; l = l->next)
     {
	CFModule *cm;

	cm = l->data;
	if (cm)
	  {
	     Evas_Object *oc = NULL;

	     cm->state = MOD_UNLOADED;
	     m = e_module_find(cm->name);
	     if ((m) && (m->enabled)) cm->state = MOD_ENABLED;

	     if (cm->desktop->icon)
	       {
		  const char *icon;

		  icon = efreet_icon_path_find(e_config->icon_theme, cm->desktop->icon, "64x64");
		  if (!icon)
		    {
		       char *path;
		       path = ecore_file_get_dir(cm->desktop->orig_path);
		       snprintf(buf, sizeof(buf), "%s/%s.edj",
				path, cm->desktop->icon);
		       icon = buf;
		       free(path);
		    }
		  oc = e_util_icon_add(icon, cfdata->evas);
	       }
	     e_widget_ilist_append(cfdata->gui.list, oc, cm->desktop->name, 
				   NULL, NULL, cm->name);
	  }
     }

   e_widget_ilist_go(cfdata->gui.list);
   e_widget_ilist_thaw(cfdata->gui.list);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(cfdata->gui.list));
}
