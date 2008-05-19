#ifndef E_MOD_GADMAN_H
#define E_MOD_GADMAN_H

#define DEFAULT_POS_X  0.1
#define DEFAULT_POS_Y  0.1
#define DEFAULT_SIZE_W 0.07
#define DEFAULT_SIZE_H 0.07

#define DRAG_START 0
#define DRAG_STOP  1
#define DRAG_MOVE  2

typedef struct _Manager Manager;

struct _Manager
{
   E_Gadcon    *gc;
   E_Gadcon    *gc_top;
   Evas_List   *gadgets;
   Evas_Object *mover;
   Evas_Object *mover_top;
   Evas_Object *full_bg;
   const char        *icon_name;
   
   int             visible;
   int             use_composite;
   Ecore_X_Window  top_win;
   Ecore_Evas     *top_ee;
   E_Container    *container;

   Evas_Coord  width, height;
   
   E_Module                *module;
   E_Config_Dialog         *config_dialog;
   E_Int_Menu_Augmentation *maug;
   E_Action                *action;
};

Manager *Man;

void             gadman_init(E_Module *m);
void             gadman_shutdown(void);
E_Gadcon_Client *gadman_gadget_add(E_Gadcon_Client_Class *cc, int ontop);
void             gadman_gadget_del(E_Gadcon_Client *gcc);
E_Gadcon_Client *gadman_gadget_place(E_Config_Gadcon_Client *cf, int ontop);
void             gadman_gadget_edit_start(E_Gadcon_Client *gcc);
void             gadman_gadget_edit_end(void);

#endif
