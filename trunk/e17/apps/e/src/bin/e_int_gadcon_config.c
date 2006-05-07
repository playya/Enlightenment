/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   E_Gadcon *gc;
   char *cname;
   int enabled;
   Evas_Object *o_enabled, *o_disabled;
};

/* a nice easy setup function that does the dirty work */
EAPI void
e_int_gadcon_config(E_Gadcon *gc)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   if (v)
     {
	/* methods */
	v->create_cfdata           = _create_data;
	v->free_cfdata             = _free_data;
	v->basic.apply_cfdata      = _basic_apply_data;
	v->basic.create_widgets    = _basic_create_widgets;
	v->override_auto_apply = 1;
	
	/* create config diaolg for bd object/data */
	cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
				  _("Contents Settings"), NULL, 0, v, gc);
	gc->config_dialog = cfd;
     }
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->cname = NULL;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->gc = cfd->data;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Free the cfdata */
   cfdata->gc->config_dialog = NULL;
   if (cfdata->cname) free(cfdata->cname);
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;
   E_Config_Gadcon *cf_gc;
   E_Config_Gadcon_Client *cf_gcc;
   int ok = 0;

   cfdata->gc->config_dialog = cfd;
   for (l = e_config->gadcons; l; l = l->next)
     {
	cf_gc = l->data;
	if ((!strcmp(cf_gc->name, cfdata->gc->name)) &&
	    (!strcmp(cf_gc->id, cfdata->gc->id)))
	  {
	     ok = 1;
	     break;
	  }
     }
   if (!ok) return;
   for (l = cf_gc->clients; l; l = l->next)
     {
	cf_gcc = l->data;
	if (!cf_gcc->name) continue;
	if (!strcmp(cf_gcc->name, cfdata->cname))
	  {
	     if (!cfdata->enabled)
	       {
		  /* remove from list */
		  cf_gc->clients = evas_list_remove_list(cf_gc->clients, l);
		  if (cf_gcc->name) evas_stringshare_del(cf_gcc->name);
		  if (cf_gcc->id) evas_stringshare_del(cf_gcc->id);
		  if (cf_gcc->style) evas_stringshare_del(cf_gcc->style);
		  free(cf_gcc);
		  goto savedone;
	       }
	     return 1; /* Apply was OK */
	  }
     }
   cf_gcc = E_NEW(E_Config_Gadcon_Client, 1);
   cf_gcc->name = evas_stringshare_add(cfdata->cname);
   cf_gcc->id = evas_stringshare_add("default");
   cf_gcc->geom.res = 800;
   cf_gcc->geom.size = 80;
   cf_gcc->geom.pos = cf_gcc->geom.res - cf_gcc->geom.size;
   cf_gcc->autoscroll = 0;
   cf_gcc->resizable = 0;
   cf_gc->clients = evas_list_append(cf_gc->clients, cf_gcc);
   savedone:
   e_gadcon_unpopulate(cfdata->gc);
   e_gadcon_populate(cfdata->gc);
   e_config_save_queue();
   return 1; /* Apply was OK */
}

static void
_cb_select(void *data)
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *l;
   E_Config_Gadcon *cf_gc;
   E_Config_Gadcon_Client *cf_gcc;
   int ok = 0, enabled = 0;
   
   cfdata = data;
   for (l = e_config->gadcons; l; l = l->next)
     {
	cf_gc = l->data;
	if ((!strcmp(cf_gc->name, cfdata->gc->name)) &&
	    (!strcmp(cf_gc->id, cfdata->gc->id)))
	  {
	     ok = 1;
	     break;
	  }
     }
   if (!ok) return;
   for (l = cf_gc->clients; l; l = l->next)
     {
	cf_gcc = l->data;
	if (!cf_gcc->name) continue;
	if (!strcmp(cf_gcc->name, cfdata->cname))
	  {
	     enabled = 1;
	     break;
	  }
     }
   e_widget_disabled_set(cfdata->o_enabled, 0);
   e_widget_disabled_set(cfdata->o_disabled, 0);
   e_widget_radio_toggle_set(cfdata->o_enabled, enabled);
   e_widget_radio_toggle_set(cfdata->o_disabled, 1 - enabled);
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *o2, *of, *ob, *oi, *oj;
   E_Radio_Group *rg;
   Evas_Coord wmw, wmh;
   Evas_List *styles, *l;
   int sel, n;
   
   /* FIXME: this is just raw config now - it needs UI improvments */
   o = e_widget_list_add(evas, 0, 1);

   of = e_widget_framelist_add(evas, _("Available Items"), 0);
   
   oi = e_widget_ilist_add(evas, 24, 24, &(cfdata->cname));

   for (l = e_gadcon_provider_list(); l; l = l->next)
     {
	E_Gadcon_Client_Class *cc;
	
	cc = l->data;
	/* FIXME: need icon */
	e_widget_ilist_append(oi, NULL, cc->name, _cb_select, cfdata, cc->name);
     }
   
   e_widget_ilist_go(oi);
   
   e_widget_min_size_get(oi, &wmw, &wmh);
   if (wmw < 200) wmw = 200;
   e_widget_min_size_set(oi, wmw, 250);
   
   e_widget_framelist_object_append(of, oi);
   
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Status"), 0);
   
   rg = e_widget_radio_group_new(&(cfdata->enabled));
   ob = e_widget_radio_add(evas, _("Enabled"), 1, rg);
   e_widget_disabled_set(ob, 1);
   e_widget_framelist_object_append(of, ob);
   cfdata->o_enabled = ob;
   ob = e_widget_radio_add(evas, _("Disabled"), 0, rg);
   e_widget_disabled_set(ob, 1);
   e_widget_framelist_object_append(of, ob);
   cfdata->o_disabled = ob;
   
   e_widget_list_object_append(o, of, 0, 0, 0.0);
   
   return o;
}
