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

struct _CFModule
{
   char *name;
   char *path;
   int state;
};

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas_List *modules;
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

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->basic.apply_cfdata      = _basic_apply_data;
   
   cfd = e_config_dialog_new(con, _("Module Settings"), NULL, 0, v, NULL);
   return cfd;
}

static void
_module_configure(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Module *m;
   const char *v;

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
   const char *v;

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
   Ecore_List *dirs = NULL;

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
		       CFModule *m;
		       char buff[4096];

		       m = E_NEW(CFModule, 1);
		       if (m)
			 {
			    snprintf(buff, sizeof(buff), "%s/%s", epd->dir, mod);
			    m->name = strdup(mod);
			    m->path = strdup(buff);
			    cfdata->modules = evas_list_append(cfdata->modules, m);
			 }
		    }
		  ecore_list_destroy(dirs);
	       }
	  }
     }

   if (cfdata->modules)
     cfdata->modules = evas_list_sort(cfdata->modules, evas_list_count(cfdata->modules), _sort_modules);

   /* Free Lists */
   if (l) evas_list_free(l);
   return;
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

   while ((l = cfdata->modules))
     {
        CFModule *m;

        m = l->data;
        cfdata->modules = evas_list_remove_list(cfdata->modules, l);
        free(m->name);
        free(m->path);
        E_FREE(m);
     }
   E_FREE(cfdata->modname);
   free(cfdata);
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
		       if (!m->enabled) e_module_enable(m);
		       if (m->enabled) 
			 {	 
			    if (m->func.config)
			      e_widget_disabled_set(cfdata->gui.configure, 0);
			    if (m->func.about)
			      e_widget_disabled_set(cfdata->gui.about, 0);
			 }
		       cm->state = MOD_ENABLED;
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
			    e_config_save_queue();
			 }
		       cm->state = MOD_UNLOADED;
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
   E_Module *m;
   Evas_List *l;
   char buf[4096];

   o = e_widget_list_add(evas, 1, 0);
   ot = e_widget_table_add(evas, 1);
   
   of = e_widget_framelist_add(evas, _("Modules"), 1);
   ilist = e_widget_ilist_add(evas, 24, 24, &(cfdata->modname));
   cfdata->gui.list = ilist;
   e_widget_on_change_hook_set(ilist, _ilist_cb_change, cfdata);

   cfdata->state = -1;
   for (l = cfdata->modules; l; l = l->next)
     {
	CFModule *cm;
	Evas_Object *oc;
	
	cm = l->data;
	if (cm)
	  {
	     E_App *a;
	     
	     cm->state = MOD_UNLOADED;
	     m = e_module_find(cm->name);
	     if (m)
	       {
		  if (m->enabled) cm->state = MOD_ENABLED;
	       }
	     snprintf(buf, sizeof(buf), "%s/module.eap", cm->path);
	     
	     a = e_app_new(buf, 0);
	     if (a)
	       {
		  oc = edje_object_add(evas);
		  edje_object_file_set(oc, buf, "icon");
		  e_widget_ilist_append(ilist, oc, a->name, NULL, NULL, cm->name);
		  e_object_unref(E_OBJECT(a));
	       }
	  }
     }

   e_widget_ilist_go(ilist);
   e_widget_framelist_object_append(of, ilist);
   e_widget_table_object_append(ot, of, 0, 0, 2, 5, 1, 1, 1, 1);

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
   e_widget_table_object_append(mt, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   
   e_widget_framelist_object_append(of, mt);
   e_widget_table_object_append(ot, of, 2, 0, 2, 2, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, _("Module Actions"), 0);   
   ob = e_widget_button_add(evas, _("Configure"), NULL, _module_configure, cfdata, NULL);
   cfdata->gui.configure = ob;
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_button_add(evas, _("About"), NULL, _module_about, cfdata, NULL);
   cfdata->gui.about = ob;
   e_widget_framelist_object_append(of, ob);

   e_widget_disabled_set(cfdata->gui.configure, 1);
   e_widget_disabled_set(cfdata->gui.about, 1);

   e_widget_table_object_append(ot, of, 2, 3, 2, 2, 1, 1, 1, 1);
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
   return (strcmp((const char*)m1->name, (const char*)m2->name));
}
