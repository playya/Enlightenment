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

typedef struct _E_Config                E_Config;
typedef struct _E_Config_Module         E_Config_Module;
typedef struct _E_Config_Theme          E_Config_Theme;
typedef struct _E_Config_Binding_Mouse  E_Config_Binding_Mouse;
typedef struct _E_Config_Binding_Key    E_Config_Binding_Key;
typedef Eet_Data_Descriptor             E_Config_DD;

#else
#ifndef E_CONFIG_H
#define E_CONFIG_H

/* increment this whenever we change config enough that you need new 
 * defaults for e to work - started at 100 when we introduced this config
 * versioning feature
 */
#define E_CONFIG_FILE_VERSION 106

#define E_EVAS_ENGINE_DEFAULT      0
#define E_EVAS_ENGINE_SOFTWARE_X11 1
#define E_EVAS_ENGINE_GL_X11       2

struct _E_Config
{
   int         config_version;
   char       *desktop_default_background;
   double      menus_scroll_speed;
   double      menus_fast_mouse_move_threshhold;
   double      menus_click_drag_timeout;
   int         border_shade_animate;
   int         border_shade_transition;
   double      border_shade_speed;
   double      framerate;
   int         image_cache;
   int         font_cache;
   int         zone_desks_x_count;
   int         zone_desks_y_count;
   int         use_virtual_roots;
   int         use_edge_flip;
   double      edge_flip_timeout;
   int         evas_engine_default;
   int         evas_engine_container;
   int         evas_engine_init;
   int         evas_engine_menus;
   int         evas_engine_borders;
   int         evas_engine_errors;
   int         evas_engine_popups;
   int         evas_engine_drag;
   char       *language;
   Evas_List  *modules;
   Evas_List  *font_fallbacks;
   Evas_List  *font_defaults;
   Evas_List  *themes;
   Evas_List  *mouse_bindings;
   Evas_List  *key_bindings;
   Evas_List  *path_append_data;
   Evas_List  *path_append_images;
   Evas_List  *path_append_fonts;
   Evas_List  *path_append_themes;
   Evas_List  *path_append_init;
   Evas_List  *path_append_icons;
   Evas_List  *path_append_modules;
   Evas_List  *path_append_backgrounds;
   int         focus_policy;
   int         pass_click_on;
   int         always_click_to_raise;
   int         use_auto_raise;
   double      auto_raise_delay;
   int         drag_resist;
};

/* FIXME: all of thsie needs to become eet lumps for enmcode/decode */
struct _E_Config_Module
{
   char          *name;
   unsigned char  enabled;
};

struct _E_Config_Theme
{
   char          *category;
   char          *file;
};

struct _E_Config_Binding_Mouse
{
   int            context;
   int            modifiers;
   char          *action;
   char          *params;
   unsigned char  button;
   unsigned char  any_mod;
};

struct _E_Config_Binding_Key
{
   int            context;
   int            modifiers;
   char          *key;
   char          *action;
   char          *params;
   unsigned char  any_mod;
};

EAPI int e_config_init(void);
EAPI int e_config_shutdown(void);

EAPI void *e_config_domain_load(char *domain, E_Config_DD *edd);
EAPI int   e_config_domain_save(char *domain, E_Config_DD *edd, void *data);

EAPI int e_config_save(void);
EAPI void e_config_save_flush(void);
EAPI void e_config_save_queue(void);

EAPI E_Config_Binding_Mouse *e_config_binding_mouse_match(E_Config_Binding_Mouse *eb_in);
EAPI E_Config_Binding_Key *e_config_binding_key_match(E_Config_Binding_Key *eb_in);
    
extern EAPI E_Config *e_config;

#endif
#endif
