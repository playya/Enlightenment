/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   int x;
   int y;
   int flip;
   /*- ADVANCED -*/
   int zone_desks_x_count;
   int zone_desks_y_count;
   int use_edge_flip;
   double edge_flip_timeout;
};

/* a nice easy setup function that does the dirty work */
EAPI E_Config_Dialog *
e_int_config_desks(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   /* methods */
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.apply_cfdata      = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->advanced.apply_cfdata   = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Desktop Settings"), NULL, 0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->zone_desks_x_count = e_config->zone_desks_x_count;
   cfdata->zone_desks_y_count = e_config->zone_desks_y_count;
   cfdata->use_edge_flip = e_config->use_edge_flip;
   cfdata->edge_flip_timeout = e_config->edge_flip_timeout;
   
   cfdata->x = cfdata->zone_desks_x_count;
   cfdata->y = cfdata->zone_desks_y_count;
   cfdata->flip = cfdata->use_edge_flip;
}

static void *
_create_data(E_Config_Dialog *cdd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cdd, E_Config_Dialog_Data *cfdata)
{
   /* Free the cfdata */
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cdd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
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
	     for (lll = con ->zones; lll; lll = lll->next)
	       {
		  zone = lll->data;
		  e_zone_desk_count_set(zone, cfdata->x, cfdata->y);
	       }
	  }
     }
   
   e_config->use_edge_flip = cfdata->flip;

   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
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
	     for (lll = con ->zones; lll; lll = lll->next)
	       {
		  zone = lll->data;
		  e_zone_desk_count_set(zone, cfdata->zone_desks_x_count, cfdata->zone_desks_y_count);
	       }
	  }
     }

   e_config->use_edge_flip = cfdata->use_edge_flip;
   e_config->edge_flip_timeout = cfdata->edge_flip_timeout;

   e_zone_update_flip_all();
   e_config_save_queue();
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cdd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ob, *of, *ot;
      
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Number of Desktops"), 0);
   
   ot = e_widget_table_add(evas, 0);
   ob = e_widget_slider_add(evas, 0, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->y), 150);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 0, 1, 0, 1);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->x), 200);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   e_widget_framelist_object_append(of, ot);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Desktop Mouse Flip"), 0);
   ob = e_widget_check_add(evas, _("Flip desktops when mouse at screen edge"), &(cfdata->flip));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o, *ob, *of, *ot;
   
   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Number of Desktops"), 0);
   
   ot = e_widget_table_add(evas, 0);
   ob = e_widget_slider_add(evas, 0, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->y), 150);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 0, 1, 0, 1);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->x), 200);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   e_widget_framelist_object_append(of, ot);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Desktop Mouse Flip"), 0);
   ob = e_widget_check_add(evas, _("Flip desktops when mouse at screen edge"), &(cfdata->use_edge_flip));
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_label_add(evas, _("Time the mouse is at the edge before flipping:"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f sec"), 0.0, 2.0, 0.05, 0, &(cfdata->edge_flip_timeout), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;
}
