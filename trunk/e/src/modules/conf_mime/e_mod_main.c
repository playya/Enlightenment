#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - File Icons"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("fileman", 100, _("Files"), NULL, "system-file-manager");
   e_configure_registry_item_add("fileman/file_icons", 20, _("File Icons"), NULL, "preferences-file-icons", e_int_config_mime);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "_config_mime_edit_dialog"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "fileman/file_icons"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("fileman/file_icons");
   e_configure_registry_category_del("fileman");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}
