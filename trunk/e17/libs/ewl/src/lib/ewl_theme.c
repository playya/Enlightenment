#include <Ewl.h>

#ifdef HAVE_CONFIG_H
#include "ewl-config.h"
#endif

static char     *theme_name = NULL;
static char     *theme_path = NULL;

static Ecore_List *font_paths = NULL;
static Ecore_Hash *cached_theme_data = NULL;
static Ecore_Hash *def_theme_data = NULL;

static void ewl_theme_font_path_init(void);
static char * ewl_theme_path_find(const char * name);

/**
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize the themeing  system
 *
 * Initializes the data structures involved with theme handling. Involves
 * finding the specified theme file. This is called by ewl_init, and is not
 * necessary for the end programmer to call.
 */
int ewl_theme_init(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	/*
	 * Alloacte and clear the default theme 
	 */
	def_theme_data = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	if (!def_theme_data)
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	/*
	 * Retrieve the current theme from the users config.
	 */
	if (!theme_name) {
		if(ewl_config.theme.name) {
			theme_name = strdup(ewl_config.theme.name);
			theme_path = ewl_theme_path_find(theme_name);
		}
	} else
		theme_path = ewl_theme_path_find(theme_name);

	/*
	 * Fall back to the default theme.
	 */	
	if(!theme_path) {
		theme_name = strdup("default");
		theme_path = ewl_theme_path_find(theme_name);
	}

	/*
	 * If we can't find a theme, no point in continuing further.
	 */
	if (!theme_path) {
		fprintf(stderr, "No usable theme found, exiting EWL\n");
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	ewl_theme_font_path_init();

	IF_FREE(theme_name);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/*
 * Private function for finding the theme path given a theme name, If no theme
 * of name is found we will return null.
 */
static char * ewl_theme_path_find(const char * name) 
{		
	struct stat     st;
	char 	       *theme_found_path = NULL;
	char 		theme_tmp_path[PATH_MAX];
	char 	       *home;

	/*
	 * Get the users home directory. This environment variable should
	 * always be set.
	 */
	home = getenv("HOME");
	if (!home) {
		DERROR("Environment variable HOME not defined\n"
		       "Try export HOME=/home/user in a bash like environemnt or\n"
		       "setenv HOME=/home/user in a csh like environment.\n");
	}

	/*
	 * Build a path to the theme if it is the users home dir and use it if
	 * available. 
	 */
	if (home) {
		snprintf(theme_tmp_path, PATH_MAX, "%s/.e/ewl/themes/%s.edj",
			home, name);
		if (((stat(theme_tmp_path, &st)) == 0) && S_ISREG(st.st_mode)) {
			theme_found_path = strdup(theme_tmp_path);
		}
	}

	/*
	 * No user theme, so we try the system-wide theme.
	 */
	if (!theme_found_path) {
		snprintf(theme_tmp_path, PATH_MAX, PACKAGE_DATA_DIR
				"/themes/%s.edj", name);
		if (((stat(theme_tmp_path, &st)) == 0) &&
				S_ISREG(st.st_mode)) {
			theme_found_path = strdup(theme_tmp_path);
		}
	}

	/*
	 * see if they gave a full path to the theme
	 */
	if (!theme_found_path) {
		if (name[0] != '/') {
			char   *cwd;

			cwd = getenv("PWD");
			if (cwd != NULL) 
				snprintf(theme_tmp_path, PATH_MAX, "%s/%s", cwd, name);
			else
				snprintf(theme_tmp_path, PATH_MAX, "%s", name);

		} else {
			snprintf(theme_tmp_path, PATH_MAX, "%s", name);
		}

		if (((stat(theme_tmp_path, &st)) == 0) &&
				S_ISREG(st.st_mode)) {
			theme_found_path = strdup(theme_tmp_path);
		}
	}
	return theme_found_path;
}

/**
 * @return Returns no value.
 * @brief Shutdown the EWL themeing subsystem
 */
void ewl_theme_shutdown()
{
	char *data;

	if (font_paths) {
		while ((data = ecore_list_remove_first(font_paths)))
			free(data);
		
		ecore_list_destroy(font_paths);
		font_paths = NULL;
	}

	if (def_theme_data) {
		ecore_hash_destroy(def_theme_data);
		def_theme_data = NULL;
	}
}

/*
 * Initializes the font path based on the theme. Also called by ewl_init, and
 * is not recommended to be called separately.
 */
static void ewl_theme_font_path_init()
{
	char           *font_path;
	char            key[PATH_MAX];

	/*
	 * Setup the default font paths
	 */
	font_paths = ecore_list_new();
	if (font_paths) {
		font_path = ewl_theme_data_str_get(NULL, "/theme/font_path");

		if (font_path) {
			if (*font_path == '/')
				ecore_list_append(font_paths, font_path);
			else {
				int len;
				char *tmp;

				len = strlen(theme_path);
				tmp = theme_path + len - 4;

				if (strcmp(tmp, ".edj"))
					snprintf(key, PATH_MAX, "%s/%s",
							theme_path, font_path);
				else
					snprintf(key, PATH_MAX, "%s",
							theme_path);
					
				ecore_list_append(font_paths, strdup(key));
				FREE(font_path);
			}
		}
	}
}

/**
 * @param w: the widget to initialize theme information
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize a widgets theme information to the default
 *
 * Sets the widget @a w's theme information to the default values.
 */
int ewl_theme_widget_init(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, FALSE);

	w->theme = def_theme_data;

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param w: the widget to remove theme information
 * @return Returns no value.
 * @brief remove the theme information from a widget
 *
 * Removes and frees the theme information from the widget @a w.
 */
void ewl_theme_widget_shutdown(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * We only want to destroy the hash if its not def_theme_data
	 * We destroy def_theme_data from else where.. 
	 */
	if (w->theme && w->theme != def_theme_data)
		ecore_hash_destroy(w->theme);

	else
		w->theme = NULL;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @brief Set the name of the theme to use.
 * @param name: the name of the theme to use.
 * @return Returns no value.
 */
void ewl_theme_name_set(char *name)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	IF_FREE(theme_name);
	theme_name = strdup(name);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @brief Return the name of the current theme
 * @return Returns a copy of the current theme name on success, NULL on failure
 */
char *ewl_theme_name_get()
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DRETURN_PTR((theme_name ? strdup(theme_name) : NULL), DLEVEL_STABLE);
}

/**
 * @brief Return the path of the current theme
 * @return Returns a copy of the current theme path on success, NULL on failure
 */
char *ewl_theme_path_get()
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DRETURN_PTR((theme_path ? strdup(theme_path) : NULL), DLEVEL_STABLE);
}

/**
 * @return Returns the font path of widget @a w on success, NULL on failure.
 * @brief retrieve the path of a widgets theme's fonts
 */
Ecore_List *ewl_theme_font_path_get()
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DRETURN_PTR(font_paths, DLEVEL_STABLE);
}

/**
 * @param path: the font to add to the search path
 * @return Returns no value.
 * @brief Add a specified path to the font search path
 *
 * Duplicates the string pointed to by @a path and adds it to
 * the list of paths that are searched for fonts.
 */
void ewl_theme_font_path_add(char *path)
{
	char *temp;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("path", path);

	temp = strdup(path);
	ecore_list_append(font_paths, temp);
	ewl_embed_font_path_add(temp);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to search
 * @param k: the image to search for
 * @return Returns the path of image key @a k on success, NULL on failure.
 * @brief retrieve the path to an image from a widgets theme
 */
char *ewl_theme_image_get(Ewl_Widget * w, char *k)
{
	char           *data;
	struct stat     st;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, NULL);
	DCHECK_PARAM_PTR_RET("k", k, NULL);

	data = ewl_theme_data_str_get(w, k);
	if (!data)
		data = strdup(theme_path);

	if (!data)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	/*
	 * Convert a relative path to an absolute path
	 */
	if (*data != '/') {
		char path[PATH_MAX];

		snprintf(path, PATH_MAX, "%s/%s", theme_path, data);

		FREE(data);
		data = strdup(path);
	}

	if (((stat(data, &st)) == -1) || !S_ISREG(st.st_mode))
		DWARNING("Couldn't stat image\n");

	DRETURN_PTR(data, DLEVEL_STABLE);
}


/**
 * @param w: the widget to search
 * @param k: the key to search for
 * @return Returns the string associated with @a k on success, NULL on failure.
 * @brief Retrieve an string value from a widgets theme
 */
char *ewl_theme_data_str_get(Ewl_Widget * w, char *k)
{
	char           *ret = NULL;
	char           *temp = NULL;
	char            key[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("k", k, NULL);

	/*
	 * Use the widget's appearance string to build a relative theme key.
	 */
	if (w) {
		char *tmp;

		tmp = ewl_widget_appearance_get(w);
		if (tmp) {
			snprintf(key, PATH_MAX, "%s/%s", tmp, k);
			FREE(tmp);
		} else
			snprintf(key, PATH_MAX, "%s", k);

	} else
		snprintf(key, PATH_MAX, "%s", k);

	if (ewl_config.theme.print_keys) 
		printf("%s\n", key);

	temp = key;
	while (temp && !ret) {
		if (w && w->theme)
			ret = ecore_hash_get(w->theme, temp);

		if (!ret && ewl_config.theme.cache && cached_theme_data)
			ret = ecore_hash_get(cached_theme_data, temp);

		if (!ret) {
			ret = edje_file_data_get(theme_path, temp);

		}
		temp++;
		temp = strchr(temp, '/');
		if (!temp && w && w->parent) {
			temp = key;
			w = w->parent;
		}
	}

	if (!ret && def_theme_data) {
		ret = ecore_hash_get(def_theme_data, temp);

		if (ret && ewl_config.theme.cache) {
			if (!cached_theme_data)
				cached_theme_data = ecore_hash_new(ecore_str_hash,
							   ecore_str_compare);
			ecore_hash_set(cached_theme_data, temp, strdup(ret));
		}
	}

	DRETURN_PTR(ret, DLEVEL_STABLE);
}

/**
 * @param w: the widget to search
 * @param k: the key to search for
 * @return Returns the integer associated with key @a k on success, 0 on failure.
 * @brief Retrieve an integer value from a widgets theme
 */
int ewl_theme_data_int_get(Ewl_Widget * w, char *k)
{
	int             ret = 0;
	char           *temp;
	char            key[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("k", k, FALSE);

	/*
	 * Use the widget's appearance string to build a relative theme key.
	 */
	if (w) {
		char *tmp;

		tmp = ewl_widget_appearance_get(w);
		if (tmp) {
			snprintf(key, PATH_MAX, "%s/%s", tmp, k);
			FREE(tmp);
		} else 
			snprintf(key, PATH_MAX, "%s", k);
	} else
		snprintf(key, PATH_MAX, "%s", k);

	for (temp = key; temp && !ret; temp = strchr(temp, '/')) {
		if (w && w->theme)
			ret = (int) (ecore_hash_get(w->theme, temp));
		else
			ret = (int) (ecore_hash_get(def_theme_data, temp));

		if (!ret) {
			char *val;

			val = edje_file_data_get(theme_path, temp);
			if (val) {
				ret = atoi(val);
				FREE(val);
			}
		}
		temp++;
	}
	DRETURN_INT(ret, DLEVEL_STABLE);
}

/**
 * @param w: the widget to change theme data
 * @param k: the key to change
 * @param v: the data to assign to the key
 * @return Returns no value.
 * @brief Store data into a widgets theme
 *
 * Changes the theme data in widget @a w so that key @a k now is
 * associated with value @a v.
 */
void ewl_theme_data_str_set(Ewl_Widget * w, char *k, char *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("k", k);

	if (!w->theme || w->theme == def_theme_data)
		w->theme = ecore_hash_new(ecore_str_hash, ecore_str_compare);

	if (v)
		ecore_hash_set(w->theme, k, strdup(v));
	else
		ecore_hash_set(w->theme, k, v);

	if (REALIZED(w)) {
		ewl_widget_unrealize(w);
		ewl_widget_realize(w);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to change theme data
 * @param k: the key to change
 * @param v: the data to assign to the key
 * @return Returns no value.
 * @brief Store data into a widgets theme
 *
 * Changes the theme data in widget @a w so that key @a k now is
 * associated with value @a v.
 */
void ewl_theme_data_int_set(Ewl_Widget * w, char *k, int v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("k", k);

	if (!w->theme || w->theme == def_theme_data)
		w->theme = ecore_hash_new(ecore_str_hash, ecore_str_compare);

	ecore_hash_set(w->theme, k, (void *) v);

	if (REALIZED(w)) {
		ewl_widget_unrealize(w);
		ewl_widget_realize(w);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param k: the key to be set
 * @param v: the value to set for the key
 * @return Returns no value.
 * @brief Set a theme key to a default value
 *
 * Sets the data associated with key @a k to value @a v in the default theme
 * data.
 */
void ewl_theme_data_default_str_set(char *k, char *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_hash_set(def_theme_data, k, strdup(v));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param k: the key to be set
 * @param v: the value to set for the key
 * @return Returns no value.
 * @brief Set a theme key to a default value
 *
 * Sets the data associated with key @a k to value @a v in the
 * default theme data.
 */
void ewl_theme_data_default_int_set(char *k, int v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ecore_hash_set(def_theme_data, k, (void *) v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
