/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config      Config;
typedef struct _Config_Item Config_Item;
typedef struct _Itask      Itask;
typedef struct _Itask_Item Itask_Item;
typedef struct _Instance  Instance;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Config_Item	   *ci;
   Evas_Object     *o_itask;
   Itask            *itask;
};


struct _Config
{
   /* just config state */
   E_Module        *module;
   Eina_List       *instances;
   E_Menu          *menu;
   Eina_List       *handlers;
   Eina_List       *items;
   Eina_List	   *config_dialog;//E_Config_Dialog *;
};

struct _Config_Item 
{
   const char *id;
   int show_label;
   int show_zone;
   int show_desk;
   int icon_label;
   int skip_dialogs;
   int skip_always_below_windows;
   int swap_on_focus;
   int iconify_focused;
   int ibox_style;
   int max_items;
   int always_group;
   int menu_all_window;
   int hide_menu_button;
};

struct _Itask
{
   Instance       *inst; 
   Evas_Object    *o_box;
   Evas_Object    *o_button;
   Eina_List      *items;
   Eina_List      *items_menu;
   Eina_List      *items_bar;
   Eina_Hash      *item_groups;
   int             show_label;
   int		       show_zone;
   int		       show_desk;
   int             icon_label;
   int 			    skip_dialogs;
   int			    skip_always_below_windows;
   int             swap_on_focus;
   int             option_iconify_focused;
   int             option_ibox_style;
   int             max_items;
   int             always_group;
   int             menu_all_window;
   int 		   hide_menu_button;
   E_Zone         *zone;
   E_Popup        *item_label_popup;
   
   int item_width;
   int item_height;
   int itask_width;
   int itask_height;
   int module_width;
   int module_height;
   

   int num_items;
   /* should then better be an idle enterer*/
   Ecore_Timer *init_timer; //HACK
   //E_Border_List *border_list;
   //Eina_List *^borders;
   Eina_List *init_items;
  
   Itask_Item *menubutton;
};


struct _Itask_Item
{
   Itask        *itask;
   Evas_Object *o_holder;
   Evas_Object *o_icon;
   Evas_Object *o_holder2;
   Evas_Object *o_icon2;
   E_Border    *border;
   double 	    last_time; // last time this item was activated
   Eina_List   *items;  // better name  needed
   Eina_List   *on_list;  // the list, if the item is on the items list of another item
   int in_bar;
   char *label;
   struct {
      unsigned char  start : 1;
      unsigned char  dnd : 1;
      int            x, y;
      int            dx, dy;
   } drag;
};



EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);

// TODOthis shouldnt be here
EAPI void     itask_config_update(Config_Item *ci);
EAPI Config_Item *itask_config_item_get(const char *id); 
EAPI Eina_List *itask_zone_find(E_Zone *zone);
EAPI void itask_update_gc_orient(Itask *it);
EAPI void itask_resize_handle(Itask *it);

void _itask_config_update(void);
void _config_itask_module(Config_Item *ci);
extern Config *itask_config;
extern char *itask_theme_path;
#endif
