#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config Config;
typedef struct _Config_Face Config_Face;
typedef struct _Net Net;
typedef struct _Net_Face Net_Face;

struct _Config 
{
   char *device;
   int check_interval;
};

struct _Config_Face 
{
   unsigned char enabled;
};

struct _Net 
{
   E_Menu *config_menu;
   Net_Face *face;
   Config *conf;
   
   E_Config_DD *conf_edd;
   E_Config_Dialog *cfd;
};

struct _Net_Face 
{
   Evas *evas;
   E_Container *con;
   E_Menu *menu;
   Net *net;

   Config_Face *conf;
   E_Config_DD *conf_face_edd;
   
   Evas_Object *net_obj;
   Evas_Object *event_obj;
   Evas_Object *chart_obj;
   
   Ecore_Timer *monitor;
   
   E_Gadman_Client *gmc;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);
EAPI int   e_modapi_config   (E_Module *m);

#endif
