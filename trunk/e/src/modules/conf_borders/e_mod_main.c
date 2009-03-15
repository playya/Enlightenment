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
     "Settings - Borders"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("appearance", 10, _("Look"), NULL, "preferences-appearance");
   e_configure_registry_item_add("appearance/borders", 50, _("Borders"), NULL, "preferences-system-windows", e_int_config_borders);
   e_configure_registry_category_add("internal", -1, _("Internal"), NULL, "enlightenment/internal");
   e_configure_registry_item_add("internal/borders_border", -1, _("Border"), NULL, "preferences-system-windows", e_int_config_borders_border);
   conf_module = m;
   e_module_delayed_set(m, 1);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "_config_border_style_dialog"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "_config_border_border_style_dialog"))) e_object_del(E_OBJECT(cfd));
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
