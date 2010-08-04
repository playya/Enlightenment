#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Mouse Cursor"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("appearance", 10, _("Look"), NULL, "preferences-appearance");
   e_configure_registry_item_add("appearance/mouse_cursor", 70, _("Mouse Cursor"), NULL, "preferences-desktop-pointer", e_int_config_cursor);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "appearance/mouse_cursor"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("appearance/mouse_cursor");
   e_configure_registry_category_del("appearance");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   return 1;
}
