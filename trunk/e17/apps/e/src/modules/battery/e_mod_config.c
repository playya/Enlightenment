#include "e.h"
#include "e_mod_main.h"
#include "config.h"

typedef struct _cfdata CFData;
typedef struct _Cfg_File_Data Cfg_File_Data;

struct _cfdata 
{
   int show_alert;   
   double poll_time;   
   int alarm_time;
};

struct _Cfg_File_Data 
{
   E_Config_Dialog *cfd;
   char *file;
};

/* Protos */
static Evas_Object   *_create_widgets(E_Config_Dialog *cfd, Evas *evas, Config *cfdata);
static void          *_create_data(E_Config_Dialog *cfd);
static void          _free_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object   *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);
static int           _basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object   *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);
static int           _advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata);

Battery *b = NULL;

void
e_int_config_battery(E_Container *con, Battery *bat) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   
   b = bat;
   
   v.create_cfdata = _create_data;
   v.free_cfdata = _free_data;
   v.basic.apply_cfdata = _basic_apply_data;
   v.basic.create_widgets = _basic_create_widgets;
   v.advanced.apply_cfdata = _advanced_apply_data;
   v.advanced.create_widgets = _advanced_create_widgets;
   
   cfd = e_config_dialog_new(con, _("Battery Module"), NULL, 0, &v, bat);
}

static void
_fill_data(CFData *cfdata) 
{
   cfdata->alarm_time = b->conf->alarm;
   cfdata->poll_time = b->conf->poll_time;
   if (cfdata->alarm_time > 0) 
     {
	cfdata->show_alert = 1;
     }
   else 
     {
	cfdata->show_alert = 0;
     }
}

static void
*_create_data(E_Config_Dialog *cfd) 
{
   CFData *cfdata;
   cfdata = E_NEW(CFData, 1);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   free(cfdata);
}

static Evas_Object
*_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata) 
{
   Evas_Object *o, *of, *ob;
   
   _fill_data(cfdata);
   
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Basic Settings"), 0);
   ob = e_widget_check_add(evas, _("Show alert when battery is low"), &(cfdata->show_alert));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   e_border_button_bindings_ungrab_all();
   b->conf->poll_time = 10.0;   
   e_border_button_bindings_grab_all();
   e_config_save_queue();
   
   _battery_face_cb_config_updated(b);
   return 1;
}

static Evas_Object
*_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata) 
{
   Evas_Object *o, *of, *ob, *ot;
   
   /* Use Sliders for both cfg options */
   _fill_data(cfdata);

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_frametable_add(evas, _("Advanced Settings"), 1);
   
   ob = e_widget_label_add(evas, _("Check battery every:"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f seconds"), 0.5, 1000.0, 0.5, 0, &(cfdata->poll_time), NULL, 200);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_check_add(evas, _("Show alert when battery is low"), &(cfdata->show_alert));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 0);   
   
   ob = e_widget_label_add(evas, _("Alert when battery is down to:"));
   e_widget_frametable_object_append(of, ob, 0, 4, 1, 1, 1, 0, 1, 1);
   
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f minutes"), 1, 60, 1, 0, NULL, &(cfdata->alarm_time), 200);
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 1, 1, 0, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   e_border_button_bindings_ungrab_all();
   
   b->conf->poll_time = cfdata->poll_time;
   if (cfdata->show_alert) 
     {
	b->conf->alarm = cfdata->alarm_time;
     }
   else 
     {
	b->conf->alarm = 0;
     }
   
   e_border_button_bindings_grab_all();
   e_config_save_queue();
   
   _battery_face_cb_config_updated(b);
   return 1;
}
