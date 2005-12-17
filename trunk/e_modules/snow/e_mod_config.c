#include "e.h"
#include "e_mod_main.h"
#include "config.h"

typedef struct _cfdata CFData;
typedef struct _Cfg_File_Data Cfg_File_Data;

#define DENSITY_SPARSE 0
#define DENSITY_MEDIUM 1
#define DENSITY_DENSE 2

struct _cfdata 
{   
   int show_trees;
   int density;
};

struct _Cfg_File_Data 
{
   E_Config_Dialog *cfd;
   char *file;
};

/* Protos */
static Evas_Object *_create_widgets(E_Config_Dialog *cfd, Evas *evas, Config *cfdata);
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata);

void
e_int_config_snow(E_Container *con, Snow *s) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   
   v.create_cfdata = _create_data;
   v.free_cfdata = _free_data;
   v.basic.apply_cfdata = _basic_apply_data;
   v.basic.create_widgets = _basic_create_widgets;
   v.advanced.apply_cfdata = NULL;
   v.advanced.create_widgets = NULL;

   cfd = e_config_dialog_new(con, _("Snow Module"), NULL, 0, &v, s);
}

static void 
_fill_data(Snow *sn, CFData *cfdata) 
{
   cfdata->show_trees = sn->conf->show_trees;
   switch (sn->conf->flake_count) 
     {
      case 5:
	cfdata->density = DENSITY_SPARSE;
	break;
      case 10:
	cfdata->density = DENSITY_MEDIUM;
	break;
      case 20:
	cfdata->density = DENSITY_DENSE;
	break;
      default:
	break;
     }
}

static void
*_create_data(E_Config_Dialog *cfd) 
{
   CFData *cfdata;
   Snow *s;
   
   s = cfd->data;
   cfdata = E_NEW(CFData, 1);
   _fill_data(s, cfdata);
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
   E_Radio_Group *rg;
   
   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("General Settings"), 0);
   ob = e_widget_check_add(evas, _("Show Trees"), &(cfdata->show_trees));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Snow Density"), 0);
   rg = e_widget_radio_group_new(&(cfdata->density));
   ob = e_widget_radio_add(evas, _("Sparse"), DENSITY_SPARSE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Medium"), DENSITY_MEDIUM, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Dense"), DENSITY_DENSE, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   Snow *sn;
   
   sn = cfd->data;
   e_border_button_bindings_ungrab_all();
   switch (cfdata->density) 
     {
      case 0:
	sn->conf->tree_count = 5;
	sn->conf->flake_count = 20;
	break;
      case 1:
	sn->conf->tree_count = 10;
	sn->conf->flake_count = 60;
	break;
      case 2:
	sn->conf->tree_count = 20;
	sn->conf->flake_count = 150;	
	break;
      default:
	break;
     }
   sn->conf->show_trees = cfdata->show_trees;
   
   e_config_save_queue();
   e_border_button_bindings_grab_all();
   
   _snow_cb_config_updated(sn);
   return 1;
}
