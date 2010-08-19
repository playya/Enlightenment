#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Desk"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("internal", -1, _("Internal"), NULL, 
                                     "enlightenment/internal");
   e_configure_registry_item_add("internal/desk", -1, _("Desk"), NULL, 
                                 "preferences-system-windows", 
                                 e_int_config_desk);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "internal/desk"))) 
     e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("internal/desk");
   e_configure_registry_category_del("internal");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   return 1;
}
