#include "e.h"

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int  _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object  *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas,
					   E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   
   double numerator;
   double denominator;
   double threshold;
};

EAPI E_Config_Dialog *
e_int_config_mouse(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   if (e_config_dialog_find("E", "_config_mouse_dialog")) 
     return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->override_auto_apply = 1;
   
   cfd = e_config_dialog_new(con, _("Mouse Acceleration Settings"), "E", 
			     "_config_mouse_dialog", 
			     "enlightenment/mouse_clean", 0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->numerator = e_config->mouse_accel_numerator;
   cfdata->denominator = e_config->mouse_accel_denominator;
   cfdata->threshold = e_config->mouse_accel_threshold;
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
   if (!cfdata) return;

   E_FREE(cfdata);
}

/* advanced window */
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   e_config->mouse_accel_numerator = cfdata->numerator;
   // Force denominator to 1 for simplicity.
   e_config->mouse_accel_denominator = 1; //cfdata->denominator;
   e_config->mouse_accel_threshold = cfdata->threshold;

   // Apply the above settings
   e_mouse_init();

   e_config_save_queue();
   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Mouse Acceleration"), 0);

   ob = e_widget_label_add(evas, _("Acceleration"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 10.0, 1.0, 0, 
			    &(cfdata->numerator), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   
   /*ob = e_widget_label_add(evas, _("Denominator"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 10.0, 1.0, 0, 
			    &(cfdata->denominator), NULL, 200);
   e_widget_framelist_object_append(of, ob);*/

   ob = e_widget_label_add(evas, _("Threshold"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 10.0, 1.0, 0, 
			    &(cfdata->threshold), NULL, 200);
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   e_dialog_resizable_set(cfd->dia, 0);
   return o;   
}
