#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Shpix      Shpix;
typedef struct _Shstore    Shstore;
typedef struct _Config     Config;
typedef struct _Dropshadow Dropshadow;
typedef struct _Shadow     Shadow;

struct _Shpix
{
   int            w, h;
   unsigned char *pix;
};

struct _Shstore
{
   int            w, h;
   unsigned int  *pix;
};

struct _Config
{
   int shadow_x, shadow_y;
   int blur_size;
   double shadow_darkness;
};

struct _Dropshadow
{
   E_Module       *module;
   Evas_List      *shadows;
   Evas_List      *cons;
   E_Before_Idler *idler_before;

   E_Config_DD    *conf_edd;
   Config         *conf;
   
   struct {
      unsigned char *gauss;
      int            gauss_size;
   } table;
   
   struct {
      Shstore *shadow[4];
      int      ref;
   } shared;
};

struct _Shadow
{
   Dropshadow *ds;
   int x, y, w, h;
   E_Container_Shape *shape;
   
   Evas_Object *object[4];
   
   unsigned char reshape : 1;
   unsigned char square : 1;
   unsigned char toosmall : 1;
   unsigned char use_shared : 1;
};

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);

#endif
