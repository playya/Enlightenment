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
static void _e_config_free(void);
static int  _e_config_cb_timer(void *data);

/* local subsystem globals */
static Ecore_Job *_e_config_save_job = NULL;

static E_Config_DD *_e_config_edd = NULL;
static E_Config_DD *_e_config_module_edd = NULL;
static E_Config_DD *_e_config_font_fallback_edd = NULL;
static E_Config_DD *_e_config_font_default_edd = NULL;
static E_Config_DD *_e_config_theme_edd = NULL;
static E_Config_DD *_e_config_bindings_mouse_edd = NULL;
static E_Config_DD *_e_config_bindings_key_edd = NULL;

/* externally accessible functions */
int
e_config_init(void)
{
   _e_config_theme_edd = E_CONFIG_DD_NEW("E_Config_Theme", E_Config_Theme);
#undef T
#undef D
#define T E_Config_Theme
#define D _e_config_theme_edd
   E_CONFIG_VAL(D, T, category, STR);
   E_CONFIG_VAL(D, T, file, STR);
   
   _e_config_module_edd = E_CONFIG_DD_NEW("E_Config_Module", E_Config_Module);
#undef T
#undef D
#define T E_Config_Module
#define D _e_config_module_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, enabled, UCHAR);

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

   _e_config_bindings_mouse_edd = E_CONFIG_DD_NEW("E_Config_Binding_Mouse", E_Config_Binding_Mouse);
#undef T
#undef D
#define T E_Config_Binding_Mouse
#define D _e_config_bindings_mouse_edd
   E_CONFIG_VAL(D, T, context, INT);
   E_CONFIG_VAL(D, T, modifiers, INT);
   E_CONFIG_VAL(D, T, action, STR);
   E_CONFIG_VAL(D, T, params, STR);
   E_CONFIG_VAL(D, T, button, UCHAR);
   E_CONFIG_VAL(D, T, any_mod, UCHAR);

   _e_config_bindings_key_edd = E_CONFIG_DD_NEW("E_Config_Binding_Key", E_Config_Binding_Key);
#undef T
#undef D
#define T E_Config_Binding_Key
#define D _e_config_bindings_key_edd
   E_CONFIG_VAL(D, T, context, INT);
   E_CONFIG_VAL(D, T, modifiers, INT);
   E_CONFIG_VAL(D, T, key, STR);
   E_CONFIG_VAL(D, T, action, STR);
   E_CONFIG_VAL(D, T, params, STR);
   E_CONFIG_VAL(D, T, any_mod, UCHAR);

   _e_config_edd = E_CONFIG_DD_NEW("E_Config", E_Config);
#undef T
#undef D
#define T E_Config
#define D _e_config_edd
   /**/ /* == already configurable via ipc */
   E_CONFIG_VAL(D, T, config_version, INT); /**/
   E_CONFIG_VAL(D, T, desktop_default_background, STR); /**/
   E_CONFIG_VAL(D, T, menus_scroll_speed, DOUBLE);
   E_CONFIG_VAL(D, T, menus_fast_mouse_move_threshhold, DOUBLE);
   E_CONFIG_VAL(D, T, menus_click_drag_timeout, DOUBLE);
   E_CONFIG_VAL(D, T, border_shade_animate, INT);
   E_CONFIG_VAL(D, T, border_shade_transition, INT);
   E_CONFIG_VAL(D, T, border_shade_speed, DOUBLE);
   E_CONFIG_VAL(D, T, framerate, DOUBLE);
   E_CONFIG_VAL(D, T, image_cache, INT);
   E_CONFIG_VAL(D, T, font_cache, INT);
   E_CONFIG_VAL(D, T, zone_desks_x_count, INT); /**/
   E_CONFIG_VAL(D, T, zone_desks_y_count, INT); /**/
   E_CONFIG_VAL(D, T, use_virtual_roots, INT); /* should not make this a config option (for now) */
   E_CONFIG_VAL(D, T, use_edge_flip, INT); 
   E_CONFIG_VAL(D, T, edge_flip_timeout, DOUBLE);
   E_CONFIG_VAL(D, T, language, STR); /**/
   E_CONFIG_LIST(D, T, modules, _e_config_module_edd); /**/
   E_CONFIG_LIST(D, T, font_fallbacks, _e_config_font_fallback_edd); /**/
   E_CONFIG_LIST(D, T, font_defaults, _e_config_font_default_edd); /**/
   E_CONFIG_LIST(D, T, themes, _e_config_theme_edd); /**/
   E_CONFIG_LIST(D, T, mouse_bindings, _e_config_bindings_mouse_edd); /**/
   E_CONFIG_LIST(D, T, key_bindings, _e_config_bindings_key_edd); /**/

   e_config = e_config_domain_load("e", _e_config_edd);
   if (e_config)
     {
	if (e_config->config_version < E_CONFIG_FILE_VERSION)
	  {
	     /* your config is too old - need new defaults */
	     _e_config_free();
	     ecore_timer_add(1.0, _e_config_cb_timer,
			     _("Configuration data needed upgrading. Your old configuration\n"
			       "has been wiped and a new set of defaults initialized. This\n"
			       "will happen regularly during development, so don't report a\n"
			       "bug. This simply means Enlightenment needs new confiugration\n"
			       "data by default for usable functionality that your old\n"
			       "configuration simply lacks. This new set of defaults will fix\n"
			       "that by adding it in. You can re-configure things now to your\n"
			       "liking. Sorry for the hiccup in your configuration.\n"));
	  }
	else if (e_config->config_version > E_CONFIG_FILE_VERSION)
	  {
	     /* your config is too new - what the fuck??? */
	     _e_config_free();
	     ecore_timer_add(1.0, _e_config_cb_timer,
			     _("Your configuration is NEWER than Enlightenment. This is very\n"
			       "strange. This should not happen unless you downgraded\n"
			       "Enlightenment or copied the configuration from a place where\n"
			       "a newer version of Enlightenment was running. This is bad and\n"
			       "as a precaution your confiugration has been now restored to\n"
			       "defaults. Sorry for the inconvenience.\n"));
	  }
     }
   
   if (!e_config)
     {
	/* DEFAULT CONFIG */
	e_config = E_NEW(E_Config, 1);
	e_config->config_version = E_CONFIG_FILE_VERSION;
	e_config->desktop_default_background = strdup(PACKAGE_DATA_DIR"/data/themes/default.edj");
	e_config->menus_scroll_speed = 1000.0;
	e_config->menus_fast_mouse_move_threshhold = 300.0;
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
	e_config->use_edge_flip = 1;
	e_config->edge_flip_timeout = 0.25;
	e_config->evas_engine_default = E_EVAS_ENGINE_SOFTWARE_X11;
	e_config->evas_engine_container = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_init = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_menus = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_borders = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_errors = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_popups = E_EVAS_ENGINE_DEFAULT;
	e_config->evas_engine_drag = E_EVAS_ENGINE_DEFAULT;
	e_config->language = strdup("");
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
	     E_Font_Fallback* eff;
	     
	     eff = E_NEW(E_Font_Fallback, 1);
	     eff->name = strdup("New-Sung");
	     e_config->font_fallbacks = evas_list_append(e_config->font_fallbacks, 
							 eff);

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
	     efd->text_class = strdup("default");
	     efd->font = strdup("Vera");
	     efd->size = 10;
             e_config->font_defaults = evas_list_append(e_config->font_defaults, efd); 
	
             efd = E_NEW(E_Font_Fallback, 1);
	     efd->text_class = strdup("title_bar");
	     efd->font = strdup("Vera");
	     efd->size = 10;
             e_config->font_defaults = evas_list_append(e_config->font_defaults, efd); 
	
	  }
	  {
	     E_Config_Theme *et;
	     
	     et = E_NEW(E_Config_Theme, 1);
	     et->category = strdup("theme");
	     et->file = strdup("default.edj");
	     e_config->themes = evas_list_append(e_config->themes, et);
	  }
	  {
	     E_Config_Binding_Mouse *eb;
	     
	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_BORDER;
	     eb->button = 1;
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_move");
	     eb->params = strdup("");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_BORDER;
	     eb->button = 2;
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_resize");
	     eb->params = strdup("");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_BORDER;
	     eb->button = 3;
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_menu");
	     eb->params = strdup("");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_ZONE;
	     eb->button = 1;
	     eb->modifiers = 0;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("main");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_ZONE;
	     eb->button = 2;
	     eb->modifiers = 0;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("clients");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Mouse, 1);
	     eb->context = E_BINDING_CONTEXT_ZONE;
	     eb->button = 3;
	     eb->modifiers = 0;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("favorites");
	     e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);
	  }
	  {
	     E_Config_Binding_Key *eb;
	     
	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Left");
	     eb->modifiers = E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_flip_by");
	     eb->params = strdup("-1 0");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);
	     
	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Right");
	     eb->modifiers = E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_flip_by");
	     eb->params = strdup("1 0");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Up");
	     eb->modifiers = E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_flip_by");
	     eb->params = strdup("0 -1");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Down");
	     eb->modifiers = E_BINDING_MODIFIER_SHIFT | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_flip_by");
	     eb->params = strdup("0 1");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Up");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_raise");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Down");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_lower");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("x");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_close");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("k");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_kill");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("w");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_menu");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("s");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_sticky_toggle");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("i");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_iconic_toggle");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("f");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_maximized_toggle");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("r");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("window_shaded_toggle");
	     eb->params = strdup("");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Left");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_by");
	     eb->params = strdup("-1");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Right");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_by");
	     eb->params = strdup("1");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F1");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("0");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F2");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("1");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F3");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("2");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F4");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("3");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F5");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("4");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F6");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("5");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F7");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("6");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F8");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("7");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F9");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("8");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F10");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("9");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F11");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("10");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("F12");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("desk_linear_flip_to");
	     eb->params = strdup("11");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("m");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("main");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("a");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("favorites");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Menu");
	     eb->modifiers = 0;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("main");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Menu");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("clients");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Menu");
	     eb->modifiers = E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("menu_show");
	     eb->params = strdup("favorites");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

	     eb = E_NEW(E_Config_Binding_Key, 1);
	     eb->context = E_BINDING_CONTEXT_ANY;
	     eb->key = strdup("Insert");
	     eb->modifiers = E_BINDING_MODIFIER_CTRL | E_BINDING_MODIFIER_ALT;
	     eb->any_mod = 0;
	     eb->action = strdup("exec");
	     eb->params = strdup("Eterm");
	     e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);

   /* should do these - can't yet due to other things missing... */
   
   /* need a yes/no dialog for these - to stop accidental logouts. for this
    * i'd make a special case that can ask u to log out, restart or shut down
    * or even reboot (if you have permissions to do so)
    */
   /* CTRL+ALT Delete - logout */
   /* CTRL+ALT End    - restart */
   
   /* need a way to display all focused windows nicely - subsystem for
    * this that also grabs the modifier on activate (if there are any) so
    * on release of modifier(s) OR on any new action this list aborts display
    */
   /* ALT Tab         - next window focus */
   /* ALT_SHIFT Tab   - prev window focus */
   
   /* need to support fullscreen anyway for this - ie netwm and the border
    * system need to handle this as well as possibly using xrandr/xvidmode
    */
   /* ALT Return      - fullscreen window */
	  }
	e_config_save_queue();
     }
//   e_config->evas_engine_container = E_EVAS_ENGINE_GL_X11;

   E_CONFIG_LIMIT(e_config->menus_scroll_speed, 1.0, 20000.0);
   E_CONFIG_LIMIT(e_config->menus_fast_mouse_move_threshhold, 1.0, 2000.0);
   E_CONFIG_LIMIT(e_config->menus_click_drag_timeout, 0.0, 10.0);
   E_CONFIG_LIMIT(e_config->border_shade_animate, 0, 1);
   E_CONFIG_LIMIT(e_config->border_shade_transition, 0, 3);
   E_CONFIG_LIMIT(e_config->border_shade_speed, 1.0, 20000.0);
   E_CONFIG_LIMIT(e_config->framerate, 1.0, 200.0);
   E_CONFIG_LIMIT(e_config->image_cache, 0, 256 * 1024);
   E_CONFIG_LIMIT(e_config->font_cache, 0, 32 * 1024);
   E_CONFIG_LIMIT(e_config->edge_flip_timeout, 0.0, 2.0);

   /* apply lang config - exception because config is loaded after intl setup */
   
   if ((e_config->language) && (strlen(e_config->language) > 0))
     {
	printf("SET LANG %s\n", e_config->language);
	e_intl_language_set(e_config->language);
     }
   
   return 1;
}

int
e_config_shutdown(void)
{
   E_CONFIG_DD_FREE(_e_config_edd);
   E_CONFIG_DD_FREE(_e_config_module_edd);
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

E_Config_Binding_Mouse *
e_config_binding_mouse_match(E_Config_Binding_Mouse *eb_in)
{
   Evas_List *l;
   
   for (l = e_config->mouse_bindings; l; l = l->next)
     {
	E_Config_Binding_Mouse *eb;
	
	eb = l->data;
	if ((eb->context == eb_in->context) &&
	    (eb->button == eb_in->button) &&
	    (eb->modifiers == eb_in->modifiers) &&
	    (eb->any_mod == eb_in->any_mod) &&
	    (!strcmp(eb->action, eb_in->action)) &&
	    (!strcmp(eb->action, eb_in->action)))
	  return eb;
     }
   return NULL;
}

E_Config_Binding_Key *
e_config_binding_key_match(E_Config_Binding_Key *eb_in)
{
   Evas_List *l;
   
   for (l = e_config->key_bindings; l; l = l->next)
     {
	E_Config_Binding_Key *eb;
	
	eb = l->data;
	if ((eb->context == eb_in->context) &&
	    (eb->modifiers == eb_in->modifiers) &&
	    (eb->any_mod == eb_in->any_mod) &&
	    (!strcmp(eb->key, eb_in->key)) &&
	    (!strcmp(eb->action, eb_in->action)) &&
	    (!strcmp(eb->action, eb_in->action)))
	  return eb;
     }
   return NULL;
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

static void
_e_config_free(void)
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
	while (e_config->themes)
	  {
	     E_Config_Theme *et;
	     
	     et = e_config->themes->data;
	     e_config->themes = evas_list_remove_list(e_config->themes, e_config->themes);
	     E_FREE(et->category);
	     E_FREE(et->file);
	     E_FREE(et);
	  }
	while (e_config->mouse_bindings)
	  {
	     E_Config_Binding_Mouse *eb;
	     
	     eb = e_config->mouse_bindings->data;
	     e_config->mouse_bindings  = evas_list_remove_list(e_config->mouse_bindings, e_config->mouse_bindings);
	     E_FREE(eb->action);
	     E_FREE(eb->params);
	     E_FREE(eb);
	  }
	while (e_config->key_bindings)
	  {
	     E_Config_Binding_Key *eb;
	     
	     eb = e_config->key_bindings->data;
	     e_config->key_bindings  = evas_list_remove_list(e_config->key_bindings, e_config->key_bindings);
	     E_FREE(eb->key);
	     E_FREE(eb->action);
	     E_FREE(eb->params);
	     E_FREE(eb);
	  }
	
	E_FREE(e_config->desktop_default_background);
	E_FREE(e_config->language);
	E_FREE(e_config);
     }
}

static int
_e_config_cb_timer(void *data)
{
   e_error_dialog_show(_("Configuration Upgraded"),
			 data);
   return 0;
}
