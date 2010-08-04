#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Window Manipulation"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("windows", 50, _("Windows"), NULL, "preferences-system-windows");
   e_configure_registry_item_add("windows/window_geometry", 30, _("Window Geometry"), NULL, "preferences-window-geometry", e_int_config_window_geometry);
   e_configure_registry_item_add("windows/window_stacking", 30, _("Window Stacking"), NULL, "preferences-window-stacking", e_int_config_window_stacking);
   e_configure_registry_item_add("windows/window_maxpolicy", 30, _("Window Maximize Policy"), NULL, "preferences-window-maximize", e_int_config_window_maxpolicy);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "windows/window_geometry"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "_config_window_stacking_dialog"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "_config_window_maxpolicy_dialog"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("windows/window_geometry");
   e_configure_registry_item_del("windows/window_stacking");
   e_configure_registry_item_del("windows/window_maxpolicy");
   e_configure_registry_category_del("windows");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   return 1;
}
