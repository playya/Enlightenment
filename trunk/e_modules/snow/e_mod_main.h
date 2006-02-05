#define D_(str) dgettext(PACKAGE, str)

#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config Config;
typedef struct _Snow Snow;
typedef struct _Snow_Flake Snow_Flake;

struct _Config
{
   int tree_count;
   int flake_count;
   int show_trees;
};

struct _Snow
{
   E_Module *module;
   Evas_List *cons;
   Evas *canvas;
   Ecore_Animator *animator;
   Evas_List *trees;
   Evas_List *flakes;

   E_Config_DD *conf_edd;
   Config *conf;
   Evas_Coord width, height;
   E_Config_Dialog *config_dialog;
};

struct _Snow_Flake
{
   Evas_Object *flake;
   double start_time;
   int speed;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);
EAPI int e_modapi_info(E_Module *m);
EAPI int e_modapi_about(E_Module *m);
EAPI int e_modapi_config(E_Module *m);

void _snow_cb_config_updated(void *data);
#endif
