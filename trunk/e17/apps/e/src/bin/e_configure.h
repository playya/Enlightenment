#ifdef E_TYPEDEFS

typedef struct _E_Configure_Cat E_Configure_Cat;
typedef struct _E_Configure_It E_Configure_It;

#else
#ifndef E_CONFIGURE_H
#define E_CONFIGURE_H

struct _E_Configure_Cat
{
   const char *cat;
   int         pri;
   const char *label;
   const char *icon_file;
   const char *icon;
   Evas_List  *items;
};

struct _E_Configure_It
{
   const char        *item;
   int                pri;
   const char        *label;
   const char        *icon_file;
   const char        *icon;
   E_Config_Dialog *(*func) (E_Container *con, const char *params);
};

EAPI void e_configure_registry_item_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon, E_Config_Dialog *(*func) (E_Container *con, const char *params));
EAPI void e_configure_registry_item_del(const char *path);
EAPI void e_configure_registry_category_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon);
EAPI void e_configure_registry_category_del(const char *path);
EAPI void e_configure_registry_call(const char *path, E_Container *con, const char *params);
EAPI int  e_configure_registry_exists(const char *path);

EAPI void e_configure_init(void);

EAPI Evas_List *e_configure_registry;

#endif
#endif
