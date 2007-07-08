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
     "Configuration - Borders"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("appearance", 10, _("Appearance"), NULL, "enlightenment/appearance");
   e_configure_registry_item_add("appearance/borders", 50, _("Borders"), NULL, "enlightenment/windows", e_int_config_borders);
   e_configure_registry_category_add("internal", -1, _("Internal"), NULL, "enlightenment/internal");
   e_configure_registry_item_add("internal/borders_border", -1, _("Border"), NULL, "enlightenment/windows", e_int_config_borders_border);
   conf_module = m;
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   e_configure_registry_item_del("internal/borders_border");
   e_configure_registry_category_del("internal");
   e_configure_registry_item_del("appearance/borders");
   e_configure_registry_category_del("appearance");
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
			_("Enlightenment Configuration Module - Borders"),
			_("Configuration dialog for borders."));
   return 1;
}
