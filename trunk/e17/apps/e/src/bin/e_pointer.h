/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef struct _E_Pointer E_Pointer;

#else
#ifndef E_POINTER_H
#define E_POINTER_H

#define E_POINTER_TYPE 0xE0b01013

struct _E_Pointer
{
   E_Object e_obj_inherit;
   
   Evas             *evas;
   Evas_Object      *pointer_object;
   Evas_Object      *hot_object;
   int              *pixels;
   Ecore_X_Window    win;
   int               w, h;
   struct {
      int            x, y;
      unsigned char  update : 1;
   } hot;
};

EAPI E_Pointer *e_pointer_window_new(Ecore_X_Window win);
EAPI void       e_pointers_size_set(int size);
EAPI void       e_pointer_idler_before(void);

#endif
#endif
