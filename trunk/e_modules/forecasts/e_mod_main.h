#define D_(str) dgettext(PACKAGE, str)

#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#define DEGREES_F 0
#define DEGREES_C 1

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;
typedef struct _Popup Popup;

struct _Config
{
   E_Module *module;
   E_Config_Dialog *config_dialog;
   Evas_List *instances;
   Evas_List *items;
   E_Menu *menu;
};

struct _Config_Item
{
   const char *id;

   double poll_time;
   int degrees;
   const char *host, *code;
   int show_text;
};

struct _Popup
{
   E_Popup *win;

   int w, h;
   int pinned : 1;

   Evas_Object *o_bg;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);
EAPI int   e_modapi_about(E_Module *m);

void _config_forecasts_module(Config_Item *ci);
void _forecasts_config_updated(const char *id);

extern Config *forecasts_config;

#endif
