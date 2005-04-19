/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* TODO List
 *
 * * setting up a new config value and a listener callback is too long winded - need to have helper funcs and macros do this so it's more like 1 line per new config value or 2
 */

#if ((E17_PROFILE >= LOWRES_PDA) && (E17_PROFILE <= HIRES_PDA))
#define DEF_MENUCLICK 1.25
#else
#define DEF_MENUCLICK 0.25
#endif

E_Config *e_config = NULL;

/* local subsystem functions */
static void _e_config_save_cb(void *data);

/* local subsystem globals */
static Ecore_Job *_e_config_save_job = NULL;

static E_Config_DD *_e_config_edd = NULL;
static E_Config_DD *_e_config_module_edd = NULL;
static E_Config_DD *_e_config_binding_edd = NULL;
static E_Config_DD *_e_config_font_fallback_edd = NULL;
static E_Config_DD *_e_config_font_default_edd = NULL;

/* externally accessible functions */
int
e_config_init(void)
{
   _e_config_module_edd = E_CONFIG_DD_NEW("E_Config_Module", E_Config_Module);
#undef T
#undef D
#define T E_Config_Module
#define D _e_config_module_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, enabled, UCHAR);

   _e_config_binding_edd = E_CONFIG_DD_NEW("E_Config_Binding", E_Config_Binding);
#undef T
#undef D
#define T E_Config_Binding
#define D _e_config_binding_edd
   E_CONFIG_VAL(D, T, button, INT);
   E_CONFIG_VAL(D, T, mask, INT);
   E_CONFIG_VAL(D, T, modifiers, INT);
   E_CONFIG_VAL(D, T, action, INT);

   _e_config_font_default_edd = E_CONFIG_DD_NEW("E_Font_Default", 
						 E_Font_Default);   
#undef T
#undef D
#define T E_Font_Default
#define D _e_config_font_default_edd
   E_CONFIG_VAL(D, T, text_class, STR);
   E_CONFIG_VAL(D, T, font, STR);
   E_CONFIG_VAL(D, T, size, INT);

   _e_config_font_fallback_edd = E_CONFIG_DD_NEW("E_Font_Fallback", 
						  E_Font_Fallback);   
#undef T
#undef D
#define T E_Font_Fallback
#define D _e_config_font_fallback_edd
   E_CONFIG_VAL(D, T, name, STR);

   _e_config_edd = E_CONFIG_DD_NEW("E_Config", E_Config);
#undef T
#undef D
#define T E_Config
#define D _e_config_edd
   E_CONFIG_VAL(D, T, desktop_default_background, STR);
   E_CONFIG_VAL(D, T, menus_scroll_speed, DOUBLE);
   E_CONFIG_VAL(D, T, menus_fast_mouse_move_thresthold, DOUBLE);
   E_CONFIG_VAL(D, T, menus_click_drag_timeout, DOUBLE);
   E_CONFIG_VAL(D, T, border_shade_animate, INT);
   E_CONFIG_VAL(D, T, border_shade_transition, INT);
   E_CONFIG_VAL(D, T, border_shade_speed, DOUBLE);
   E_CONFIG_VAL(D, T, framerate, DOUBLE);
   E_CONFIG_VAL(D, T, image_cache, INT);
   E_CONFIG_VAL(D, T, font_cache, INT);
   E_CONFIG_VAL(D, T, zone_desks_x_count, INT);
   E_CONFIG_VAL(D, T, zone_desks_y_count, INT);
   E_CONFIG_LIST(D, T, modules, _e_config_module_edd);
   E_CONFIG_LIST(D, T, bindings, _e_config_binding_edd);
   E_CONFIG_LIST(D, T, font_fallbacks, _e_config_font_fallback_edd);
   E_CONFIG_LIST(D, T, font_defaults, _e_config_font_default_edd);

   e_config = e_config_domain_load("e", _e_config_edd);
   if (!e_config)
     {
	/* DEFAULT CONFIG */
	e_config = E_NEW(E_Config, 1);
	e_config->desktop_default_background = strdup(PACKAGE_DATA_DIR"/data/themes/default.edj");
	e_config->menus_scroll_speed = 1000.0;
	e_config->menus_fast_mouse_move_thresthold = 300.0;
	e_config->menus_click_drag_timeout = DEF_MENUCLICK;
	e_config->border_shade_animate = 1;
	e_config->border_shade_transition = E_TRANSITION_DECELERATE;
	e_config->border_shade_speed = 3000.0;
	e_config->framerate = 30.0;
	e_config->image_cache = 4096;
	e_config->font_cache = 512;
	e_config->zone_desks_x_count = 4;
	e_config->zone_desks_y_count = 1;
	e_config->use_virtual_roots = 0;
	  {
	     E_Config_Module *em;

	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("ibar");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("dropshadow");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("clock");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("battery");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("cpufreq");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("temperature");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	     em = E_NEW(E_Config_Module, 1);
	     em->name = strdup("pager");
	     em->enabled = 1;
	     e_config->modules = evas_list_append(e_config->modules, em);
	  }
	  {
	     E_Config_Binding *eb;

	     eb = E_NEW(E_Config_Binding, 1);
	     eb->button = 1;
	     eb->mask = ECORE_X_EVENT_MASK_MOUSE_DOWN |
			ECORE_X_EVENT_MASK_MOUSE_UP |
			ECORE_X_EVENT_MASK_MOUSE_MOVE;
	     eb->modifiers = ECORE_X_MODIFIER_ALT;
	     eb->action = E_BINDING_ACTION_MOVE;
	     e_config->bindings = evas_list_append(e_config->bindings, eb);

	     eb = E_NEW(E_Config_Binding, 1);
	     eb->button = 2;
	     eb->mask = ECORE_X_EVENT_MASK_MOUSE_DOWN |
			ECORE_X_EVENT_MASK_MOUSE_UP |
			ECORE_X_EVENT_MASK_MOUSE_MOVE;
	     eb->modifiers = ECORE_X_MODIFIER_ALT;
	     eb->action = E_BINDING_ACTION_RESIZE;
	     e_config->bindings = evas_list_append(e_config->bindings, eb);

	     eb = E_NEW(E_Config_Binding, 1);
	     eb->button = 3;
	     eb->mask = ECORE_X_EVENT_MASK_MOUSE_DOWN;
	     eb->modifiers = ECORE_X_MODIFIER_ALT;
	     eb->action = E_BINDING_ACTION_MENU;
	     e_config->bindings = evas_list_append(e_config->bindings, eb);
	  }
	  {
	     E_Font_Fallback* eff;
	     
	     eff = E_NEW(E_Font_Fallback, 1);
	     eff->name = strdup("Kochi-Gothic");
	     e_config->font_fallbacks = evas_list_append(e_config->font_fallbacks, 
							 eff);
	     
	     eff = E_NEW(E_Font_Fallback, 1);
	     eff->name = strdup("Baekmuk-Dotum");
	     e_config->font_fallbacks = evas_list_append(e_config->font_fallbacks, 
							 eff);

	  }
	  { 
	     E_Font_Default* efd;
	     
             efd = E_NEW(E_Font_Fallback, 1);
	     efd->text_class = strdup("title_bar");
	     efd->font = strdup("Vera");
	     efd->size = 10;
             e_config->font_defaults = evas_list_append(e_config->font_defaults, efd); 
	
	  }
	e_config_save_queue();
     }

   E_CONFIG_LIMIT(e_config->menus_scroll_speed, 1.0, 20000.0);
   E_CONFIG_LIMIT(e_config->menus_fast_mouse_move_thresthold, 1.0, 2000.0);
   E_CONFIG_LIMIT(e_config->menus_click_drag_timeout, 0.0, 10.0);
   E_CONFIG_LIMIT(e_config->border_shade_animate, 0, 1);
   E_CONFIG_LIMIT(e_config->border_shade_transition, 0, 3);
   E_CONFIG_LIMIT(e_config->border_shade_speed, 1.0, 20000.0);
   E_CONFIG_LIMIT(e_config->framerate, 1.0, 200.0);
   E_CONFIG_LIMIT(e_config->image_cache, 0, 256 * 1024);
   E_CONFIG_LIMIT(e_config->font_cache, 0, 32 * 1024);
   return 1;
}

int
e_config_shutdown(void)
{
   if (e_config)
     {
	while (e_config->modules)
	  {
	     E_Config_Module *em;

	     em = e_config->modules->data;
	     e_config->modules = evas_list_remove_list(e_config->modules, e_config->modules);
	     E_FREE(em->name);
	     E_FREE(em);
	  }
	while (e_config->bindings)
	  {
	     E_Config_Binding *eb;

	     eb = e_config->bindings->data;
	     e_config->bindings = evas_list_remove_list(e_config->bindings, e_config->bindings);
	     E_FREE(eb);
	  }
	while (e_config->font_fallbacks)
	  {
	     E_Font_Fallback *eff;
	     
	     eff = e_config->font_fallbacks->data;
	     e_config->font_fallbacks = evas_list_remove_list(e_config->font_fallbacks, e_config->font_fallbacks);
	     E_FREE(eff->name);
	     E_FREE(eff);
	  }
	while (e_config->font_defaults)
	  {
	     E_Font_Default *efd;
	     
	     efd = e_config->font_defaults->data;
	     e_config->font_defaults = evas_list_remove_list(e_config->font_defaults, e_config->font_defaults);
	     E_FREE(efd->text_class);
	     E_FREE(efd->font);
	     E_FREE(efd);
	  }

	E_FREE(e_config->desktop_default_background);
	E_FREE(e_config);
     }
   E_CONFIG_DD_FREE(_e_config_edd);
   E_CONFIG_DD_FREE(_e_config_module_edd);
   E_CONFIG_DD_FREE(_e_config_binding_edd);
   E_CONFIG_DD_FREE(_e_config_font_default_edd);
   E_CONFIG_DD_FREE(_e_config_font_fallback_edd);
   return 1;
}

int
e_config_save(void)
{
   if (_e_config_save_job)
     {
	ecore_job_del(_e_config_save_job);
	_e_config_save_job = NULL;
     }
   return e_config_domain_save("e", _e_config_edd, e_config);
}

void
e_config_save_flush(void)
{
   if (_e_config_save_job)
     {
	ecore_job_del(_e_config_save_job);
	_e_config_save_job = NULL;
	e_config_domain_save("e", _e_config_edd, e_config);
     }
}

void
e_config_save_queue(void)
{
   if (_e_config_save_job) ecore_job_del(_e_config_save_job);
   _e_config_save_job = ecore_job_add(_e_config_save_cb, NULL);
}

void *
e_config_domain_load(char *domain, E_Config_DD *edd)
{
   Eet_File *ef;
   char buf[4096];
   char *homedir;
   void *data = NULL;

   homedir = e_user_homedir_get();
   snprintf(buf, sizeof(buf), "%s/.e/e/config/%s.cfg", homedir, domain);
   E_FREE(homedir);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
	data = eet_data_read(ef, edd, "config");
	eet_close(ef);
     }
   return data;
}

int
e_config_domain_save(char *domain, E_Config_DD *edd, void *data)
{
   Eet_File *ef;
   char buf[4096];
   char *homedir;
   int ok = 0;

   /* FIXME: check for other sessions fo E runing */
   homedir = e_user_homedir_get();
   snprintf(buf, sizeof(buf), "%s/.e/e/config/%s.cfg", homedir, domain);
   E_FREE(homedir);
   ef = eet_open(buf, EET_FILE_MODE_WRITE);
   if (ef)
     {
	ok = eet_data_write(ef, edd, "config", data, 0);
	eet_close(ef);
     }
   return ok;
}

/* local subsystem functions */
static void
_e_config_save_cb(void *data)
{
   printf("SAVE!!!!\n");
   e_module_save_all();
   e_config_domain_save("e", _e_config_edd, e_config);
   _e_config_save_job = NULL;
}
