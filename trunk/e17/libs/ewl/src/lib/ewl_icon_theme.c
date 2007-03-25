/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "ewl_base.h"
#include "ewl_icon_theme.h"
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

static int ewl_icon_theme_is_edje = 0;

static Ecore_Hash *ewl_icon_theme_cache = NULL;
static Ecore_Hash *ewl_icon_fallback_theme_cache = NULL;
static void ewl_icon_theme_cb_free(void *data);
static char *ewl_icon_theme_icon_path_get_helper(const char *icon, 
					const char *size, const char *theme, 
					const char *key, Ecore_Hash *cache);

/**
 * @return Returns TRUE on success or FALSE on failure
 * @brief Initializes the icon theme system
 */
int
ewl_icon_theme_init(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!ewl_icon_theme_cache)
	{
		ewl_icon_theme_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
		ecore_hash_set_free_key(ewl_icon_theme_cache, ewl_icon_theme_cb_free);
		ecore_hash_set_free_value(ewl_icon_theme_cache, ewl_icon_theme_cb_free);

		ewl_icon_fallback_theme_cache = ecore_hash_new(
						ecore_str_hash, ecore_str_compare);
		ecore_hash_set_free_key(ewl_icon_fallback_theme_cache, 
						ewl_icon_theme_cb_free);
		ecore_hash_set_free_value(ewl_icon_fallback_theme_cache, 
						ewl_icon_theme_cb_free);
	}

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @return Returns no value.
 * @brief Shuts down the icon theme system
 */
void
ewl_icon_theme_shutdown(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	IF_FREE_HASH(ewl_icon_theme_cache);
	IF_FREE_HASH(ewl_icon_fallback_theme_cache);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @return Returns no value
 * @brief Called when the icon theme is changed so we can clean up any
 * caching we have in place
 */
void
ewl_icon_theme_theme_change(void)
{
	const char *icon_theme;
	
	DENTER_FUNCTION(DLEVEL_STABLE);

	icon_theme = ewl_config_string_get(ewl_config, EWL_CONFIG_THEME_ICON_THEME);

	/* check if this is an edje theme */
	if (icon_theme && (!strncasecmp(icon_theme + (strlen(icon_theme) - 4),
					      ".edj", 4)))
		ewl_icon_theme_is_edje = 1;
	else
		ewl_icon_theme_is_edje = 0;

	/* destroy the cache and re-create it */
	IF_FREE_HASH(ewl_icon_theme_cache);

	ewl_icon_theme_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	ecore_hash_set_free_key(ewl_icon_theme_cache, ewl_icon_theme_cb_free);
	ecore_hash_set_free_value(ewl_icon_theme_cache, ewl_icon_theme_cb_free);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param icon: The Icon Spec icon name to lookup
 * @param size: The size of the icon to retrieve. A 0 value will cause
 * the default size to be used.
 * @return Returns the path to the icon we are looking for or NULL if none found
 * @brief Retrives the full path to the specified icon, or NULL if none found
 */
const char *
ewl_icon_theme_icon_path_get(const char *icon, int size)
{
	char *ret;
	const char *icon_theme;
	char icon_size[16];
	char key[256];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, NULL);

	icon_theme = ewl_config_string_get(ewl_config, 
				EWL_CONFIG_THEME_ICON_THEME);

	/* make sure we have an icon theme */
	if (!icon_theme)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	/* if our theme is an edje just return the .edj file */
	if (ewl_icon_theme_is_edje)
		DRETURN_PTR(icon_theme, DLEVEL_STABLE);

	if (size == 0)
		size = ewl_config_int_get(ewl_config, 
					EWL_CONFIG_THEME_ICON_SIZE);

	snprintf(icon_size, sizeof(icon_size), "%dx%d", size, size);
	snprintf(key, sizeof(key), "%s@%s", icon, icon_size);
	ret = ewl_icon_theme_icon_path_get_helper(icon, icon_size, icon_theme, 
						key, ewl_icon_theme_cache);

	if (ret == EWL_THEME_KEY_NOMATCH)
		ret = ewl_icon_theme_icon_path_get_helper(icon, icon_size, "EWL",
					key, ewl_icon_fallback_theme_cache);

	if (ret == EWL_THEME_KEY_NOMATCH)
		ret = NULL;

	DRETURN_PTR(ret, DLEVEL_STABLE);
}

static char *
ewl_icon_theme_icon_path_get_helper(const char *icon, const char *size, 
					const char *theme, const char *key,
					Ecore_Hash *cache)
{
	char *ret;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("icon", icon, EWL_THEME_KEY_NOMATCH);

	ret = ecore_hash_get(cache, key);
	if (!ret)
	{
		ret = ecore_desktop_icon_find(icon, size, theme);
		if (!ret) ret = EWL_THEME_KEY_NOMATCH;

		ecore_hash_set(cache, strdup(key), ret);
	}

	DRETURN_PTR(ret, DLEVEL_STABLE);;
}

static void
ewl_icon_theme_cb_free(void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (data == EWL_THEME_KEY_NOMATCH)
		DRETURN(DLEVEL_STABLE);

	IF_FREE(data);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


