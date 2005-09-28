/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef struct _E_Action E_Action;

#else
#ifndef E_ACTIONS_H
#define E_ACTIONS_H

#define E_ACTION_TYPE 0xE0b01010

struct _E_Action
{
   E_Object             e_obj_inherit;
   
   char *name;
   struct {
      void (*go)        (E_Object *obj, char *params);
      void (*go_mouse)  (E_Object *obj, char *params, Ecore_X_Event_Mouse_Button_Down *ev);
      void (*go_wheel)  (E_Object *obj, char *params, Ecore_X_Event_Mouse_Wheel *ev);
      void (*go_key)    (E_Object *obj, char *params, Ecore_X_Event_Key_Down *ev);
      void (*go_signal) (E_Object *obj, char *params, char *sig, char *src);
      void (*end)       (E_Object *obj, char *params);
      void (*end_mouse) (E_Object *obj, char *params, Ecore_X_Event_Mouse_Button_Up *ev);
      void (*end_key)   (E_Object *obj, char *params, Ecore_X_Event_Key_Up *ev);
   } func;
};

EAPI int         e_actions_init(void);
EAPI int         e_actions_shutdown(void);

EAPI Evas_List  *e_action_name_list(void);
EAPI E_Action   *e_action_add(char *name);
EAPI E_Action   *e_action_find(char *name);
    
#endif
#endif
