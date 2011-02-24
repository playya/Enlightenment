#include <e.h>
#include <Eina.h>

#include "main.h"
#include "elfe_config.h"

/* local function prototypes */
static void *_elfe_home_config_create(E_Config_Dialog *cfd);
static void _elfe_home_config_free(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_elfe_home_config_ui(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _elfe_home_config_changed(void *data, Evas_Object *obj, void *event);
static void _elfe_home_config_slider_changed(void *data, Evas_Object *obj);
static void _elfe_home_config_click_changed(void *data, Evas_Object *obj, void *event);
static Eina_Bool _elfe_home_config_change_timeout(void *data);

/* local variables */
EAPI Elfe_Home_Config *elfe_home_cfg = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_desktop_edd = NULL;
static E_Config_DD *conf_desktop_item_edd = NULL;
Ecore_Timer *_elfe_home_config_change_timer = NULL;
Evas_Object *delay_label, *delay_slider;

/* public functions */
int 
elfe_home_config_init(E_Module *m) 
{
   Eina_List *l;

   conf_desktop_item_edd = E_CONFIG_DD_NEW("Elfe_Desktop_Item_Cfg", Elfe_Desktop_Item_Config);

#undef T
#undef D
#define T Elfe_Desktop_Item_Config
#define D conf_desktop_item_edd
   E_CONFIG_VAL(D, T, type, INT);
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, pos_x, INT);
   E_CONFIG_VAL(D, T, pos_y, INT);
   E_CONFIG_VAL(D, T, size_w, INT);
   E_CONFIG_VAL(D, T, size_h, INT);
   E_CONFIG_VAL(D, T, col, INT);
   E_CONFIG_VAL(D, T, row, INT);

   conf_desktop_edd = E_CONFIG_DD_NEW("Elfe_Desktop_Cfg", Elfe_Desktop_Config);
#undef T
#undef D
#define T Elfe_Desktop_Config
#define D conf_desktop_edd
   E_CONFIG_LIST(D, T, items, conf_desktop_item_edd);


   conf_edd = E_CONFIG_DD_NEW("Elfe_Cfg", Elfe_Home_Config);
#undef T
#undef D
#define T Elfe_Home_Config
#define D conf_edd
   E_CONFIG_LIST(D, T, desktops, conf_desktop_edd);
 
   elfe_home_cfg = e_config_domain_load("module.elfe", conf_edd);


   if (!elfe_home_cfg)
     {
         int i;
         elfe_home_cfg = E_NEW(Elfe_Home_Config, 1);

         for (i = 0; i < 5; i++)
	   {
	      Elfe_Desktop_Config *dc;
	      dc = E_NEW(Elfe_Desktop_Config, 1);
	      elfe_home_cfg->desktops = eina_list_append(elfe_home_cfg->desktops, dc);
	   }

     }


   elfe_home_cfg->mod_dir = eina_stringshare_add(m->dir);
   elfe_home_cfg->theme = eina_stringshare_printf("%s/default.edj", elfe_home_cfg->mod_dir);

   return 1;
}

int 
elfe_home_config_shutdown(void) 
{
   e_configure_registry_item_del("illume/elfe");
   e_configure_registry_category_del("illume");

   if (elfe_home_cfg->mod_dir) eina_stringshare_del(elfe_home_cfg->mod_dir);

   E_FREE(elfe_home_cfg);
   elfe_home_cfg = NULL;

   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

int 
elfe_home_config_save(void) 
{
   printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<< CONFIG SAVE\n");
   e_config_domain_save("module.elfe", conf_edd, elfe_home_cfg);
   return 1;
}

/* local functions */
static void *
_elfe_home_config_create(E_Config_Dialog *cfd __UNUSED__) 
{
   return NULL;
}

static void 
_elfe_home_config_free(E_Config_Dialog *cfd __UNUSED__, E_Config_Dialog_Data *cfdata __UNUSED__) 
{
   elfe_home_win_cfg_update();
}

static Evas_Object *
_elfe_home_config_ui(E_Config_Dialog *cfd __UNUSED__, Evas *evas, E_Config_Dialog_Data *cfdata __UNUSED__) 
{
 
}

static void 
_elfe_home_config_changed(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__) 
{
   if (_elfe_home_config_change_timer) 
     ecore_timer_del(_elfe_home_config_change_timer);
   _elfe_home_config_change_timer = 
     ecore_timer_add(0.5, _elfe_home_config_change_timeout, data);
}

static void 
_elfe_home_config_slider_changed(void *data, Evas_Object *obj __UNUSED__) 
{
   if (_elfe_home_config_change_timer) 
     ecore_timer_del(_elfe_home_config_change_timer);
   _elfe_home_config_change_timer = 
     ecore_timer_add(0.5, _elfe_home_config_change_timeout, data);
}

static void 
_elfe_home_config_click_changed(void *data, Evas_Object *obj, void *event) 
{
   _elfe_home_config_changed(data, obj, event);
}

static Eina_Bool
_elfe_home_config_change_timeout(void *data __UNUSED__) 
{
   elfe_home_win_cfg_update();
   e_config_save_queue();
   _elfe_home_config_change_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

void elfe_home_config_desktop_item_add(int desktop,
                                       Elfe_Desktop_Item_Type type,
                                       int col, int row,
                                       Evas_Coord x, Evas_Coord y,
                                       Evas_Coord w, Evas_Coord h,
                                       const char *name)
{
   Elfe_Desktop_Config *dc;
   Elfe_Desktop_Item_Config *dic;

   dic = calloc(1, sizeof(Elfe_Desktop_Item_Config));
   dic->type = type;
   dic->name = eina_stringshare_add(name);

   dic->size_w = w;
   dic->size_h = h;
   dic->pos_x = x;
   dic->pos_y = y;
   dic->col = col;
   dic->row = row;

   dc = eina_list_nth(elfe_home_cfg->desktops, desktop);
   if (dc)
     dc->items = eina_list_append(dc->items, dic);

   elfe_home_config_save();
}
