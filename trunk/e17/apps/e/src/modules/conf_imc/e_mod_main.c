/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "e_mod_main.h"

/***************************************************************************/
/**/
/* actual module specifics */

static E_Module *conf_module = NULL;

/**/
/***************************************************************************/

/***************************************************************************/
/**/

/**/
/***************************************************************************/

/***************************************************************************/
/**/
/* module setup */
EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Configuration - Input Methods"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("language", 70, _("Language"), NULL, "enlightenment/intl");
   e_configure_registry_item_add("language/input_method_settings", 20, _("Input Method Settings"), NULL, "enlightenment/imc", e_int_config_imc);
   conf_module = m;
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "_config_imc_dialog"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("language/input_method_settings");
   e_configure_registry_category_del("language");
   conf_module = NULL;
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}

EAPI int
e_modapi_about(E_Module *m)
{
   e_module_dialog_show(m,
			_("Enlightenment Configuration Module - Input Methods"),
			_("Configuration dialog for input methods."));
   return 1;
}
