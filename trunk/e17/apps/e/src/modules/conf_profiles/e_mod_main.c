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
     "Configuration - Profiles"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("advanced", 80, _("Advanced"), NULL, "enlightenment/advanced");
   e_configure_registry_item_add("advanced/profiles", 50, _("Profiles"), NULL, "enlightenment/profiles", e_int_config_profiles);
   conf_module = m;
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "_config_profiles_dialog"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("advanced/profiles");
   e_configure_registry_category_del("advanced");
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
			_("Enlightenment Configuration Module - Profiles"),
			_("Configuration dialog for config profiles."));
   return 1;
}
