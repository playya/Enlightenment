/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "e_mod_main.h"

/***************************************************************************/
/**/
/* actual module specifics */

static void  _e_mod_run_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void  _e_mod_menu_add(void *data, E_Menu *m);

static E_Module *conf_module = NULL;
static E_Int_Menu_Augmentation *maug = NULL;

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
   maug = e_int_menus_menu_augmentation_add("config/1", _e_mod_menu_add, NULL, NULL, NULL);

   conf_module = m;
   e_module_delayed_set(m, 1);
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

/* menu item callback(s) */
static void 
_e_mod_run_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   e_configure_registry_call("advanced/profiles", m->zone->container, NULL);
}

/* menu item add hook */
static void
_e_mod_menu_add(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Profiles"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/profiles");
   e_menu_item_callback_set(mi, _e_mod_run_cb, NULL);
}
