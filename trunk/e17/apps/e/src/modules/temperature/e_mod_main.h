/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config           Config;
typedef struct _Config_Face      Config_Face;
typedef struct _Temperature      Temperature;
typedef struct _Temperature_Face Temperature_Face;

struct _Config
{
   double poll_time;
   int low, high;
   Evas_List *faces;
};

struct _Config_Face
{
   unsigned char enabled;
};

struct _Temperature
{
   E_Menu           *config_menu;
   E_Menu           *config_menu_low;
   E_Menu           *config_menu_high;
   E_Menu           *config_menu_poll;
   Evas_List        *faces;

   Config           *conf;
   Ecore_Timer      *temperature_check_timer;

   unsigned char     have_temp;
};

struct _Temperature_Face
{
   E_Container *con;
   E_Menu      *menu;
   Config_Face *conf;

   Evas_Object *temp_object;
   Evas_Object *event_object;

   E_Gadman_Client *gmc;
};

EAPI void *init     (E_Module *m);
EAPI int   shutdown (E_Module *m);
EAPI int   save     (E_Module *m);
EAPI int   info     (E_Module *m);
EAPI int   about    (E_Module *m);

#endif
