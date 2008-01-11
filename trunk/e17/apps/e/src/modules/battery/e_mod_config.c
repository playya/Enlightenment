#include "e.h"
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int show_alert;   
   int poll_interval;
   int alarm_time;
   int alarm_percent;
};

/* Protos */
static void          *_create_data(E_Config_Dialog *cfd);
static void          _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object   *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int           _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object   *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int           _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

EAPI E_Config_Dialog *
e_int_config_battery_module(E_Container *con, const char *params __UNUSED__) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   snprintf(buf, sizeof(buf), "%s/e-module-battery.edj", e_module_dir_get(battery_config->module));
   cfd = e_config_dialog_new(con,
			     _("Battery Monitor Configuration"), 
			     "E", "_e_mod_battery_config_dialog",
			     buf, 0, v, NULL);
   battery_config->config_dialog = cfd;
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return;
   cfdata->alarm_time = battery_config->alarm;
   cfdata->alarm_percent = battery_config->alarm_p;
   cfdata->poll_interval = battery_config->poll_interval;
   if (cfdata->alarm_time > 0 || cfdata->alarm_percent > 0) 
     cfdata->show_alert = 1;
   else 
     cfdata->show_alert = 0;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return;
   battery_config->config_dialog = NULL;
   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
      
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Basic Settings"), 0);
   ob = e_widget_check_add(evas, _("Show alert when battery is low"), &(cfdata->show_alert));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return 0;
   if (cfdata->show_alert)
   { 
     battery_config->alarm = cfdata->alarm_time;
     battery_config->alarm_p = cfdata->alarm_percent;
   }
   else
   { 
     battery_config->alarm = 0;
     battery_config->alarm_p = 0;
   }
   _battery_config_updated();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
   
   /* Use Sliders for both cfg options */
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_frametable_add(evas, _("Advanced Settings"), 1);
   
   ob = e_widget_label_add(evas, _("Check battery every:"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f ticks"), 1, 1024, 4, 0, NULL, &(cfdata->poll_interval), 200);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_check_add(evas, _("Show alert when battery is low"), &(cfdata->show_alert));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 0);   
   
   ob = e_widget_label_add(evas, _("Alert when battery is down to:"));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 0, 1, 1);
   
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f minutes"), 1, 60, 1, 0, NULL, &(cfdata->alarm_time), 200);
   e_widget_frametable_object_append(of, ob, 0, 4, 1, 1, 1, 0, 1, 0);

   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f percent"), 1, 100, 1, 0, NULL, &(cfdata->alarm_percent), 200);
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return 0;
   battery_config->poll_interval = cfdata->poll_interval;
   if (cfdata->show_alert)
   { 
     battery_config->alarm = cfdata->alarm_time;
     battery_config->alarm_p = cfdata->alarm_percent;
   }
   else 
   {
     battery_config->alarm = 0;
     battery_config->alarm_p = 0;
   }
   _battery_config_updated();
   e_config_save_queue();
   return 1;
}

