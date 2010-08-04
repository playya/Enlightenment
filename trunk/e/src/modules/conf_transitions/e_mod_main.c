#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Transitions"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("appearance", 10, _("Look"), NULL, "preferences-appearance");
   e_configure_registry_item_add("appearance/transitions", 80, _("Transitions"), NULL, "preferences-transitions", e_int_config_transitions);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "appearance/transitions"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("appearance/transitions");
   e_configure_registry_category_del("appearance");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}
