#include "e.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "config.h"

typedef struct _Cfg_File_Data Cfg_File_Data;

struct _E_Config_Dialog_Data
{
   int quality;
   int blur_size;
   int shadow_x;
   int darkness;
   double shadow_darkness;
};

struct _Cfg_File_Data 
{
   E_Config_Dialog *cfd;
   char *file;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

void
_config_dropshadow_module(E_Container *con, Dropshadow *ds) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   
   cfd = e_config_dialog_new(con, _("Dropshadow Configuration"), NULL, 0, v, ds);   
   ds->config_dialog = cfd;
}

static void 
_fill_data(Dropshadow *ds, E_Config_Dialog_Data *cfdata) 
{
   cfdata->quality = ds->conf->quality;
   cfdata->blur_size = ds->conf->blur_size;
   cfdata->shadow_x = ds->conf->shadow_x;
   if (cfdata->shadow_x >= 32) 
     {
	cfdata->shadow_x = 32;
     }
   else if ((cfdata->shadow_x < 32) && (cfdata->shadow_x >= 16)) 
     {
	cfdata->shadow_x = 16;
     }
   else if ((cfdata->shadow_x < 16) && (cfdata->shadow_x >= 8)) 
     {
	cfdata->shadow_x = 8;
     }
   else if ((cfdata->shadow_x < 8) && (cfdata->shadow_x >= 4)) 
     {
	cfdata->shadow_x = 4;
     }
   
   cfdata->shadow_darkness = ds->conf->shadow_darkness;
   if (cfdata->shadow_darkness == 1.0) 
     {
	cfdata->darkness = 0;
     }
   else if (cfdata->shadow_darkness == 0.75) 
     {
	cfdata->darkness = 1;
     }
   else if (cfdata->shadow_darkness == 0.5) 
     {
	cfdata->darkness = 2;
     }
   else if (cfdata->shadow_darkness == 0.25) 
     {
	cfdata->darkness = 3;
     }   
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   Dropshadow *ds;
   
   ds = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ds, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Dropshadow *ds;
   
   ds = cfd->data;
   ds->config_dialog = NULL;
   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *ob, *of;
   E_Radio_Group *rg;
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Quality"), 0);
   rg = e_widget_radio_group_new(&(cfdata->quality));   
   ob = e_widget_radio_add(evas, _("High Quality"), 1, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Medium Quality"), 2, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Low Quality"), 4, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Blur Type"), 0);
   rg = e_widget_radio_group_new(&(cfdata->blur_size));   
   ob = e_widget_radio_add(evas, _("Very Fuzzy"), 80, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Fuzzy"), 40, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Medium"), 20, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Sharp"), 10, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Very Sharp"), 5, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Shadow Distance"), 0);
   rg = e_widget_radio_group_new(&(cfdata->shadow_x));   
   ob = e_widget_radio_add(evas, _("Very Far"), 32, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Far"), 16, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Near"), 8, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Very Near"), 4, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Shadow Darkness"), 0);
   rg = e_widget_radio_group_new(&(cfdata->darkness));   
   ob = e_widget_radio_add(evas, _("Very Dark"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Dark"), 1, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Light"), 2, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Very Light"), 3, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);   
   
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Dropshadow *ds;
   
   ds = cfd->data;
   e_border_button_bindings_ungrab_all();

   ds->conf->quality = cfdata->quality;
   ds->conf->blur_size = cfdata->blur_size;
   ds->conf->shadow_x = cfdata->shadow_x;
   ds->conf->shadow_y = cfdata->shadow_x;
   switch (cfdata->darkness) 
     {
      case 0:
	ds->conf->shadow_darkness = 1.0;
	break;
      case 1:
	ds->conf->shadow_darkness = 0.75;
	break;
      case 2:
	ds->conf->shadow_darkness = 0.5;	
	break;
      case 3:
	ds->conf->shadow_darkness = 0.25;	
	break;
     }
   
   e_config_save_queue();
   e_border_button_bindings_grab_all();
   
   /* Update Config */
   _dropshadow_cb_config_updated(ds);
   return 1;
}
