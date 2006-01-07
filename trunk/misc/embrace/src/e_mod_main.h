#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct {
	Embrace *embrace;

	E_Gadman_Client *gmc;
	Evas_Object     *event_object;
} EmbraceModule;

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *module);
EAPI int   e_modapi_shutdown (E_Module *module);
EAPI int   e_modapi_save     (E_Module *module);
EAPI int   e_modapi_info     (E_Module *module);
EAPI int   e_modapi_about    (E_Module *module);

#endif
