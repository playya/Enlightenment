/*
  * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
  */
#ifdef E_TYPEDEFS

typedef struct _E_App_Edit E_App_Edit;

#else
#ifndef E_EAP_EDIT_H
#define E_EAP_EDIT_H

#define E_EAP_EDIT_TYPE 0xE0b01019

struct _E_App_Edit
{
   E_Object                     e_obj_inherit;
   
   E_App       *eap;
   Evas        *evas;
   
   Evas_Object *img;
   Evas_Object *img_widget;
   Evas_Object *fsel;
   E_Dialog    *fsel_dia;
   int          img_set;
   
   E_Config_Dialog *cfd;
};

EAPI E_App_Edit *e_eap_edit_show(E_Container *con, E_App *a);

#endif
#endif
