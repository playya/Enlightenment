#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Dropshadow Dropshadow;
typedef struct _Shadow Shadow;

struct _Dropshadow
{
   E_Module       *module;
   Evas_List      *shadows;
   Evas_List      *cons;
   E_Before_Idler *idler_before;
   
   struct {
      int shadow_x, shadow_y;
      int blur_size;
      double shadow_darkness;
   } conf;
   
   struct {
      unsigned char *gauss;
      int            gauss_size;
   } table;
};

struct _Shadow
{
   Dropshadow *ds;
   int x, y, w, h;
   E_Container_Shape *shape;
   
   Evas_Object *object[4];
   
   unsigned char reshape : 1;
   unsigned char square : 1;
};

#endif
