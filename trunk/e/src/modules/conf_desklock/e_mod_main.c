#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Screen Lock"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("screen", 30, _("Screen"), NULL, 
                                     "preferences-desktop-display");
   e_configure_registry_item_add("screen/screen_lock", 30, _("Screen Lock"), 
                                 NULL, "preferences-desklock", 
                                 e_int_config_desklock);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m __UNUSED__)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "screen/screen_lock"))) 
     e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("screen/screen_lock");
   e_configure_registry_category_del("screen");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m __UNUSED__)
{
   return 1;
}
