#include <Ewl.h>
#include <Ecore_Config.h>

#ifdef HAVE_CONFIG_H
#include "ewl-config.h"
#endif

enum Ewl_Config_Types {
	EWL_CONFIG_DEBUG_ENABLE,
	EWL_CONFIG_DEBUG_LEVEL,
	EWL_CONFIG_EVAS_RENDER_METHOD,
	EWL_CONFIG_EVAS_FONT_CACHE,
	EWL_CONFIG_EVAS_IMAGE_CACHE,
	EWL_CONFIG_THEME_NAME,
	EWL_CONFIG_THEME_CACHE,
	EWL_CONFIG_THEME_COLOR_CLASSES_OVERRIDE
};

extern Ecore_List *ewl_embed_list;

static void ewl_config_set_defaults(void);
static void ewl_config_read_config(void);

static int ewl_config_listener(const char *key, const Ecore_Config_Type type, 
						    const int tag, void *data);

Ewl_Config ewl_config;

/**
 * @return Returns true on success, false on failure.
 * @brief Initialize the configuration system
 *
 * This sets up the necessary configuration variables.
 */
int ewl_config_init(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_config_system_init();
	memset(&ewl_config, 0, sizeof(Ewl_Config));
	ewl_config_read_config();

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

void ewl_config_shutdown(void)
{
	IF_FREE(ewl_config.evas.render_method);
	IF_FREE(ewl_config.theme.name);

	ecore_config_system_shutdown();
}

/**
 * @param k: the key to set in the configuration database
 * @param v: the string value that will be associated with the key
 * @return Returns TRUE on success, FALSE on failure.
 * @brief set the value of key to the specified string
 *
 * Sets the string value associated with the key @a k to @a v in the
 * configuration database.
 */
int ewl_config_set_str(const char *k, char *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_config_string_set(k, v);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param k: the key to set in the configuration database
 * @param v: the integer value that will be associated with the key
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Set the value of key to the specified integer
 *
 * Sets the integer value associated with the key @a k to @a v in the
 * configuration database.
 */
int ewl_config_set_int(const char *k, int v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_config_int_set(k, v);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}


/**
 * @param k: the key to set in the configuration database
 * @param v: the float value that will be associated with the key
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Set the value of key to the specified float
 *
 * Sets the float value associated with the key @a k to @a v in the
 * configuration database.
 */
int ewl_config_set_float(const char *k, float v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_config_float_set(k, v);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}


/**
 * @param k: the key to search
 * @return Returns the found string value on success, NULL on failure.
 * @brief Retrieve string value associated with a key
 */
char *ewl_config_get_str(const char *k)
{
	char *ret = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	ret = ecore_config_string_get(k);

	DRETURN_PTR(ret, DLEVEL_STABLE);
}


/**
 * @param k: the key to search
 * @return Returns the found integer value on success, 0 on failure.
 * @brief Retrieve integer value associated with a key
 */
int ewl_config_get_int(const char *k)
{
	int v = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	v = ecore_config_int_get(k);

	DRETURN_INT(v, DLEVEL_STABLE);
}

/**
 * @param k: the key to search
 * @return Returns the found float value on success, 0.0 on failure.
 * @brief Retrieve floating point value associated with a key
 */
float ewl_config_get_float(const char *k)
{
	float v = 0.0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	v = ecore_config_float_get(k);

	DRETURN_FLOAT(v, DLEVEL_STABLE);
}

/**
 * @return Returns the found render method, default software render.
 * @brief Retrieve the render method of the evas
 */
char *ewl_config_get_render_method()
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DRETURN_INT((ewl_config.evas.render_method ?
				strdup(ewl_config.evas.render_method) : NULL),
			DLEVEL_STABLE);
}

static void ewl_config_read_config(void)
{
	int             cc;
	Ewl_Config      nc;
	Ecore_Config_Prop *prop;

	DENTER_FUNCTION(DLEVEL_STABLE);

	/*
	 * Clean out some memory first, this is likely to get re-used if the
	 * values have not changed.
	 */
	IF_FREE(ewl_config.evas.render_method);
	IF_FREE(ewl_config.theme.name);

	ewl_config_set_defaults();

	nc.debug.enable = ewl_config_get_int("/ewl/debug/enable");
	nc.debug.level = ewl_config_get_int("/ewl/debug/level");
	nc.evas.font_cache = ewl_config_get_int("/ewl/evas/font_cache");
	nc.evas.image_cache = ewl_config_get_int("/ewl/evas/image_cache");
	nc.evas.render_method = ewl_config_get_str("/ewl/evas/render_method");
	nc.theme.name = ewl_config_get_str("/ewl/theme/name");
	nc.theme.cache = ewl_config_get_int("/ewl/theme/cache");
	nc.theme.cclass_override = 
			ewl_config_get_int("/ewl/theme/color_classes/override");

	if (nc.theme.cclass_override) {
		int i;

		cc = ewl_config_get_int("/ewl/theme/color_classes/count");
		prop = ecore_config_get("/ewl/theme/color_classes/count");
		prop->flags &= ~PF_MODIFIED;
		prop->flags |= PF_SYSTEM;

		for (i = 0; i < cc; i++) {
			char *name;
			char key[PATH_MAX];

			snprintf(key, PATH_MAX,
					"/ewl/theme/color_classes/%d/name", i);
			name = ewl_config_get_str(key);
			prop = ecore_config_get(key);
			prop->flags &= ~PF_MODIFIED;
			prop->flags |= PF_SYSTEM;

			if (name) {
				int r, g, b, a;
				int r2, g2, b2, a2;
				int r3, g3, b3, a3;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/r", i);
				r = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/g", i);
				g = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/b", i);
				b = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/a", i);
				a = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/r2", i);
				r2 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/g2", i);
				g2 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/b2", i);
				b2 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/a2", i);
				a2 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/r3", i);
				r3 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/g3", i);
				g3 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/b3", i);
				b3 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				snprintf(key, PATH_MAX,
						"/ewl/theme/color_classes/%d/a3", i);
				a3 = ewl_config_get_int(key);
				prop = ecore_config_get(key);
				prop->flags &= ~PF_MODIFIED;
				prop->flags |= PF_SYSTEM;

				edje_color_class_set(name, r, g, b, a,
						r2, g2, b2, a2,
						r3, g3, b3, a3);
				FREE(name);
			}
		}
	}

	if (ewl_embed_list && !ecore_list_is_empty(ewl_embed_list)) {
		Ewl_Embed      *e;

		ecore_list_goto_first(ewl_embed_list);

		while ((e = ecore_list_next(ewl_embed_list)) != NULL) {
			if (!e->evas)
				continue;

			if (nc.evas.font_cache) {
				evas_font_cache_flush(e->evas);
				evas_font_cache_set(e->evas, 0);
			}

			if (nc.evas.image_cache) {
				evas_image_cache_flush(e->evas);
				evas_image_cache_set(e->evas,
						     nc.evas.image_cache);
			}
		}
	}

	ewl_config.debug.enable = nc.debug.enable;
	ewl_config.debug.level = nc.debug.level;
	ewl_config.evas.font_cache = nc.evas.font_cache;
	ewl_config.evas.image_cache = nc.evas.image_cache;
	ewl_config.evas.render_method = nc.evas.render_method;
	ewl_config.theme.name = nc.theme.name;
	ewl_config.theme.cache = nc.theme.cache;
	ewl_config.theme.cclass_override = nc.theme.cclass_override;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void ewl_config_set_defaults(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_config_int_default("/ewl/debug/enable", 0);
	ecore_config_int_default("/ewl/debug/level", 0);
	ecore_config_string_default("/ewl/evas/render_method", "software_x11");
	ecore_config_int_default("/ewl/evas/font_cache", 2097152);
	ecore_config_int_default("/ewl/evas/image_cache", 8388608);
	ecore_config_theme_default("/ewl/theme/name", "default");
	ecore_config_int_default("/ewl/theme/cache", 0);
	ecore_config_int_default("/ewl/theme/color_classes/override", 0);

	/* need to set each of these keys into the system section */
	{
		Ecore_Config_Prop *prop = NULL;
		int i = 0;
		char *keys [] = {
		    "/ewl/debug/enable",
		    "/ewl/debug/level",
		    "/ewl/evas/render_method",
		    "/ewl/evas/font_cache",
		    "/ewl/evas/image_cache",
		    "/ewl/theme/name",
		    "/ewl/theme/cache",
		    "/ewl/theme/color_classes/override",
		    NULL
		};

		for(i = 0; keys[i] != NULL; i++) {
			prop = ecore_config_get(keys[i]);
			prop->flags &= ~PF_MODIFIED;
			prop->flags |= PF_SYSTEM;
		}

		ecore_config_listen("ewl_debug_enable", "/ewl/debug/enable",
		    ewl_config_listener, EWL_CONFIG_DEBUG_ENABLE, NULL);
		ecore_config_listen("ewl_debug_level", "/ewl/debug/level",
		    ewl_config_listener, EWL_CONFIG_DEBUG_LEVEL, NULL);
		ecore_config_listen("ewl_render_method", "/ewl/evas/render_method",
		    ewl_config_listener, EWL_CONFIG_EVAS_RENDER_METHOD, NULL);
		ecore_config_listen("ewl_font_cache", "/ewl/evas/font_cache",
		    ewl_config_listener, EWL_CONFIG_EVAS_FONT_CACHE, NULL);
		ecore_config_listen("ewl_image_cache", "/ewl/evas/image_cache",
		    ewl_config_listener, EWL_CONFIG_EVAS_IMAGE_CACHE, NULL);
		ecore_config_listen("ewl_theme_name", "/ewl/theme/name",
		    ewl_config_listener, EWL_CONFIG_THEME_NAME, NULL);
		ecore_config_listen("ewl_theme_cache", "/ewl/theme/cache",
		    ewl_config_listener, EWL_CONFIG_THEME_CACHE, NULL);
		ecore_config_listen("ewl_theme_cclases_override", "/ewl/theme/color_classes/override",
		    ewl_config_listener, EWL_CONFIG_THEME_COLOR_CLASSES_OVERRIDE, NULL);
	}

	DRETURN(DLEVEL_STABLE);
}

static int ewl_config_listener(const char *key, const Ecore_Config_Type type, 
						const int tag, void *data)
{
	switch(tag) {
		case EWL_CONFIG_DEBUG_ENABLE:
			ewl_config.debug.enable = ewl_config_get_int(key);
			break;

		case EWL_CONFIG_DEBUG_LEVEL:
			ewl_config.debug.level = ewl_config_get_int(key);
			break;

		case EWL_CONFIG_EVAS_RENDER_METHOD:
			IF_FREE(ewl_config.evas.render_method);
			ewl_config.evas.render_method = ewl_config_get_str(key);
			break;

		case EWL_CONFIG_EVAS_FONT_CACHE:
			ewl_config.evas.font_cache = ewl_config_get_int(key);
			break;

		case EWL_CONFIG_EVAS_IMAGE_CACHE:
			ewl_config.evas.image_cache = ewl_config_get_int(key);
			break;

		case EWL_CONFIG_THEME_NAME:
			IF_FREE(ewl_config.theme.name);
			ewl_config.theme.name = ewl_config_get_str(key);
			break;

		case EWL_CONFIG_THEME_CACHE:
			ewl_config.theme.cache = ewl_config_get_int(key);
			break;
			
		case EWL_CONFIG_THEME_COLOR_CLASSES_OVERRIDE:
			ewl_config.theme.cclass_override = ewl_config_get_int(key);
			break;
	}
	return 0;
}


