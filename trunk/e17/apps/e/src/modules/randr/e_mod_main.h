/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config           Config;
typedef struct _Config_Manager   Config_Manager;
typedef struct _Randr            Randr;
typedef struct _Randr_Resolution Randr_Resolution;

struct _Config
{
   int store;
   Evas_List *managers;
};

struct _Config_Manager
{
   int manager;
   int width;
   int height;
};

struct _Randr
{
   E_Menu      *config_menu;
   E_Menu      *resolution_menu;

   E_Int_Menu_Augmentation *augmentation;

   Ecore_Timer *timer;
   E_Dialog    *dialog;

   Config      *conf;
};

struct _Randr_Resolution
{
   E_Manager *manager;
   Randr     *randr;
   Ecore_X_Screen_Size prev, next;
};

extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);

#endif
