/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

#define E_CONFIG_DD_NEW(str, typ) \
   eet_data_descriptor_new(str, sizeof(typ), \
			      (void *(*) (void *))evas_list_next, \
			      (void *(*) (void *, void *))evas_list_append, \
			      (void *(*) (void *))evas_list_data, \
			      (void *(*) (void *))evas_list_free, \
			      (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach, \
			      (void *(*) (void *, const char *, void *))evas_hash_add, \
			      (void  (*) (void *))evas_hash_free)
#define E_CONFIG_DD_FREE(eed) if (eed) { eet_data_descriptor_free((eed)); (eed) = NULL; }
#define E_CONFIG_VAL(edd, type, member, dtype) EET_DATA_DESCRIPTOR_ADD_BASIC(edd, type, #member, member, dtype)
#define E_CONFIG_SUB(edd, type, member, eddtype) EET_DATA_DESCRIPTOR_ADD_SUB(edd, type, #member, member, eddtype)
#define E_CONFIG_LIST(edd, type, member, eddtype) EET_DATA_DESCRIPTOR_ADD_LIST(edd, type, #member, member, eddtype)

#define CHAR   EET_T_CHAR
#define SHORT  EET_T_SHORT
#define INT    EET_T_INT
#define LL     EET_T_LONG_LONG
#define FLOAT  EET_T_FLOAT
#define DOUBLE EET_T_DOUBLE
#define UCHAR  EET_T_UCHAR
#define USHORT EET_T_USHORT
#define UINT   EET_T_UINT
#define ULL    EET_T_ULONG_LONG
#define STR    EET_T_STRING

#define E_CONFIG_LIMIT(v, min, max) {if (v > max) v = max; else if (v < min) v = min;}

typedef struct _E_Config        E_Config;
typedef struct _E_Config_Module E_Config_Module;
typedef struct _E_Config_Binding E_Config_Binding;
typedef Eet_Data_Descriptor     E_Config_DD;

typedef enum _E_Binding_Action
{
   E_BINDING_ACTION_MOVE,
   E_BINDING_ACTION_RESIZE,
   E_BINDING_ACTION_MENU
} E_Binding_Action;

#else
#ifndef E_CONFIG_H
#define E_CONFIG_H

struct _E_Config
{
   char       *desktop_default_background;
   double      menus_scroll_speed;
   double      menus_fast_mouse_move_thresthold;
   double      menus_click_drag_timeout;
   int         border_shade_animate;
   int         border_shade_transition;
   double      border_shade_speed;
   double      framerate;
   int         image_cache;
   int         font_cache;
   int         zone_desks_x_count;
   int         zone_desks_y_count;
   Evas_List  *modules;
   Evas_List  *bindings;
};

struct _E_Config_Module
{
   char          *name;
   unsigned char  enabled;
};

struct _E_Config_Binding
{
   int                button;
   Ecore_X_Event_Mask mask;
   int                modifiers;
   E_Binding_Action   action;

};

EAPI int e_config_init(void);
EAPI int e_config_shutdown(void);

EAPI void *e_config_domain_load(char *domain, E_Config_DD *edd);
EAPI int   e_config_domain_save(char *domain, E_Config_DD *edd, void *data);

EAPI int e_config_save(void);
EAPI void e_config_save_flush(void);
EAPI void e_config_save_queue(void);

extern EAPI E_Config *e_config;

#endif
#endif
