#include "e.h"
#include "e_mod_main.h"

struct _E_Config_Dialog_Data 
{
   int menu_augmentation;
};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

E_Config_Dialog *
e_int_config_conf_module(E_Container *con, const char *params) 
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[PATH_MAX];

   /* is this config dialog already visible ? */
   if (e_config_dialog_find("Conf", "advanced/conf")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   snprintf(buf, sizeof(buf), "%s/e-module-conf.edj", conf->module->dir);
   cfd = e_config_dialog_new(con, _("Configuration Panel"), "Conf", 
                             "advanced/conf", buf, 0, v, NULL);

   conf->cfd = cfd;
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   conf->cfd = NULL;
   E_FREE(cfdata);
}

static void 
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   cfdata->menu_augmentation = conf->menu_augmentation;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("General"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   ow = e_widget_check_add(evas, _("Show configuration panel contents in Settings menu"), 
                           &(cfdata->menu_augmentation));
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 0, 0.5);

   return o;
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   conf->menu_augmentation = cfdata->menu_augmentation;
   if (conf->aug)
     {
        e_int_menus_menu_augmentation_del("config/0", conf->aug);
        conf->aug = NULL;
     }

   if (conf->menu_augmentation)
     {
       conf->aug = 
          e_int_menus_menu_augmentation_add("config/0", 
                                            e_mod_config_menu_add, 
                                            NULL, NULL, NULL);
       e_int_menus_menu_augmentation_point_disabled_set("config/1", 1);
     }
   else
     e_int_menus_menu_augmentation_point_disabled_set("config/1", 0);

   e_config_save_queue();
   return 1;
}
