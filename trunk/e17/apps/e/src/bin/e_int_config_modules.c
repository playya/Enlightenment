/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#define MOD_ENABLED 0
#define MOD_LOADED 1
#define MOD_UNLOADED 2

/* PROTOTYPES - same all the time */
typedef struct _CFData CFData;
typedef struct _CFModule CFModule;
typedef struct _E_Widget_Data E_Widget_Data;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, CFData *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);

static void _ilist_cb_change(void *data, Evas_Object *obj);
static int _sort_modules(void *data1, void *data2);

static void _module_configure(void *data, void *data2);
static void _module_about(void *data, void *data2);

struct _E_Widget_Data
{
   E_Radio_Group *group;
   Evas_Object *o_radio;
   int valnum;
};

struct _CFModule
{
   char *name;
   char *path;
   int state;
};

/* Actual config data we will be playing with whil the dialog is active */
struct _CFData
{
   E_Config_Dialog *cfd;
   Evas_List *modules;
   int state;
   struct
     {
	Evas_Object *configure, *enabled, *about;
	Evas_Object *loaded, *unloaded, *list;
     }
   gui;
};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_modules(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;

   /* methods */
   v.create_cfdata           = _create_data;
   v.free_cfdata             = _free_data;
   v.basic.apply_cfdata      = _basic_apply_data;
   v.basic.create_widgets    = _basic_create_widgets;
   /* Disable For Now */
   v.advanced.apply_cfdata   = NULL;
   v.advanced.create_widgets = NULL;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Module Settings"), NULL, 0, &v, NULL);
   return cfd;
}

static void
_module_configure(void *data, void *data2)
{
   Evas_Object *obj;
   E_Module *m;
   char *v;

   obj = data;
   v = strdup(e_widget_ilist_selected_label_get(obj));
   m = e_module_find(v);
   if (m)
     {
	if (m->func.config)
	  m->func.config(m);
     }
}

static void
_module_about(void *data, void *data2)
{
   Evas_Object *obj;
   E_Module *m;
   char *v;

   obj = data;
   v = strdup(e_widget_ilist_selected_label_get(obj));
   m = e_module_find(v);
   if (m)
     {
	if (m->func.about)
	  m->func.about(m);
     }
}

/**--CREATE--**/
static void
_fill_data(CFData *cfdata)
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
	       }
	  }
     }

   if (cfdata->modules)
     cfdata->modules = evas_list_sort(cfdata->modules, evas_list_count(cfdata->modules), _sort_modules);

   /* Free Lists */
   if (l) evas_list_free(l);
   if (dirs != NULL) ecore_list_destroy(dirs);
   return;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   CFData *cfdata;

   cfdata = E_NEW(CFData, 1);
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, CFData *cfdata)
{
   /* Free the cfdata */
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata)
{
   E_Module *m;
   char *v;
   int i;

   v = strdup(e_widget_ilist_selected_label_get(cfdata->gui.list));
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
		     case MOD_LOADED:
		       if (m->enabled) 
			 {
			    e_module_save(m);
			    e_module_disable(m);
			 }
		       if (m->func.about)
			 e_widget_disabled_set(cfdata->gui.about, 0);
		       
		       cm->state = MOD_LOADED;
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
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *of, *ob, *oc, *ot, *ilist;
   E_Radio_Group *rg;
   E_Module *m;
   char buf[4096];
   int i;

   o = e_widget_list_add(evas, 1, 0);

   of = e_widget_framelist_add(evas, _("Modules"), 1);
   ot = e_widget_table_add(evas, 1);

   ilist = e_widget_ilist_add(evas, 16, 16, NULL);
   cfdata->gui.list = ilist;
   e_widget_on_change_hook_set(ilist, _ilist_cb_change, cfdata);

   cfdata->state = -1;
   for (i = 0; i < evas_list_count(cfdata->modules); i++)
     {
	CFModule *cm;

	cm = evas_list_nth(cfdata->modules, i);
	if (cm)
	  {
	     cm->state = MOD_UNLOADED;
	     m = e_module_find(cm->name);
	     if (m)
	       {
		  cm->state = MOD_LOADED;
		  if (m->enabled) cm->state = MOD_ENABLED;
	       }
	     /* This Fails if module author names icon something else */
	     snprintf(buf, sizeof(buf), "%s/module_icon.png", cm->path);
	     oc = e_icon_add(evas);
	     e_icon_file_set(oc, buf);
	     e_widget_ilist_append(ilist, oc, cm->name, NULL, NULL, NULL);
	  }
     }

   e_widget_ilist_go(ilist);
   e_widget_table_object_append(ot, ilist, 0, 0, 1, 7, 1, 1, 1, 1);

   rg = e_widget_radio_group_new(&(cfdata->state));

   ob = e_widget_radio_add(evas, _("Enabled"), MOD_ENABLED, rg);
   cfdata->gui.enabled = ob;
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 1, 0, 1, 0);

   ob = e_widget_radio_add(evas, _("Loaded"), MOD_LOADED, rg);
   cfdata->gui.loaded = ob;
   e_widget_table_object_append(ot, ob, 1, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_radio_add(evas, _("Unloaded"), MOD_UNLOADED, rg);
   cfdata->gui.unloaded = ob;
   e_widget_table_object_append(ot, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   ob = e_widget_button_add(evas, _("Configure"), NULL, _module_configure, ilist, NULL);
   cfdata->gui.configure = ob;
   e_widget_table_object_append(ot, ob, 1, 3, 1, 1, 1, 0, 1, 0);

   ob = e_widget_button_add(evas, _("About"), NULL, _module_about, ilist, NULL);
   cfdata->gui.about = ob;
   e_widget_table_object_append(ot, ob, 1, 4, 1, 1, 1, 0, 1, 0);

   e_widget_disabled_set(cfdata->gui.configure, 1);
   e_widget_disabled_set(cfdata->gui.about, 1);
   
   e_widget_framelist_object_append(of, ot);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   e_dialog_resizable_set(cfd->dia, 1);

   return o;
}

static void
_ilist_cb_change(void *data, Evas_Object *obj)
{
   E_Module *m;
   E_Widget_Data *wd;
   CFData *cfdata;
   char *v;
   int i;

   cfdata = data;

   v = strdup(e_widget_ilist_selected_label_get(obj));
   for (i = 0; i < evas_list_count(cfdata->modules); i++)
     {
	CFModule *cm;

	cm = evas_list_nth(cfdata->modules, i);
	if ((cm) && (!strcmp(cm->name, v)))
	  {
	     cfdata->state = cm->state;
	     switch (cm->state)
	       {
		case MOD_ENABLED:
		  wd = e_widget_data_get(cfdata->gui.enabled);
		  edje_object_signal_emit(wd->o_radio, "toggle_on", "");
		  wd = e_widget_data_get(cfdata->gui.loaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  wd = e_widget_data_get(cfdata->gui.unloaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  break;
		case MOD_LOADED:
		  wd = e_widget_data_get(cfdata->gui.enabled);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  wd = e_widget_data_get(cfdata->gui.loaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_on", "");
		  wd = e_widget_data_get(cfdata->gui.unloaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  break;
		case MOD_UNLOADED:
		  wd = e_widget_data_get(cfdata->gui.enabled);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  wd = e_widget_data_get(cfdata->gui.loaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		  wd = e_widget_data_get(cfdata->gui.unloaded);
		  edje_object_signal_emit(wd->o_radio, "toggle_on", "");
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
