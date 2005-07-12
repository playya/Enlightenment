/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config      Config;
typedef struct _Config_Face Config_Face;
typedef struct _Clock       Clock;
typedef struct _Clock_Face  Clock_Face;

struct _Config
{
   Evas_List *faces;
};

struct _Config_Face
{
   unsigned char enabled;
   int
      digitalStyle
   ;
};

struct _Clock
{
   Evas_List   *faces;
   E_Menu      *config_menu;
   
   Config      *conf;
};

struct _Clock_Face
{
   E_Container *con;
   E_Menu      *menu;
   E_Menu      *digital_menu;
   Config_Face *conf;
   
   Evas_Object *clock_object;
   Evas_Object *event_object;
   
   E_Gadman_Client *gmc;
};

EAPI void *e_modapi_init     (E_Module *module);
EAPI int   e_modapi_shutdown (E_Module *module);
EAPI int   e_modapi_save     (E_Module *module);
EAPI int   e_modapi_info     (E_Module *module);
EAPI int   e_modapi_about    (E_Module *module);

#endif
