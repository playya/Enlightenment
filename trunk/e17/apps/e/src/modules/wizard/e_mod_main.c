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
     "Wizard"
};


EAPI void *
e_modapi_init(E_Module *m)
{
   Ecore_List *files;
   char buf[PATH_MAX];
   
   conf_module = m;
   e_wizard_init();
   
   snprintf(buf, sizeof(buf), "%s/%s", e_module_dir_get(m), MODULE_ARCH);
   files = ecore_file_ls(buf);
   if (files)
     {
	char *file;
	
	ecore_list_first_goto(files);
	while ((file = ecore_list_current(files)))
	  {
	     if (!strncmp(file, "page_", 5))
	       {
		  void *handle;
		  
		  snprintf(buf, sizeof(buf), "%s/%s/%s",
			   e_module_dir_get(m), MODULE_ARCH, file);
		  handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
		  if (handle)
		    {
		       e_wizard_page_add(handle,
					 dlsym(handle, "wizard_page_init"),
					 dlsym(handle, "wizard_page_shutdown"),
					 dlsym(handle, "wizard_page_show"),
					 dlsym(handle, "wizard_page_hide"),
					 dlsym(handle, "wizard_page_apply"));
		    }
	       }
	     ecore_list_next(files);
	  }
	ecore_list_destroy(files);
     }
   
   e_wizard_go();
   
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   e_wizard_shutdown();
   conf_module = NULL;
// FIXME: wrong place   
//   e_module_disable(m); /* disable - on restart this won't be loaded now */
//   e_sys_action_do(E_SYS_RESTART, NULL); /* restart e - cleanly try settings */
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
			_("Enlightenment First Run Wizard Module"),
			_("A module for setting up configuration for Enlightenment for the first time."));
   return 1;
}
