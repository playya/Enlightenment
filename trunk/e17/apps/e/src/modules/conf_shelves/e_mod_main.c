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
     "Configuration - Shelves"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("extensions", 90, _("Extensions"), NULL, "enlightenment/extensions");
   e_configure_registry_item_add("extensions/shelves", 20, _("Shelves"), NULL, "enlightenment/shelf", e_int_config_shelf);
   conf_module = m;
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "_config_shelf_dialog"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("extensions/shelves");
   e_configure_registry_category_del("extensions");
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
			_("Enlightenment Configuration Module - Shelves"),
			_("Configuration dialog for shelves."));
   return 1;
}
