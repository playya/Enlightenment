
#include <Ewl.h>

#ifdef HAVE_CONFIG_H
#include "ewl-config.h"
#endif


static E_DB_File *config_db = NULL;

static void __create_user_config(void);
static int __open_user_config(void);
static void __close_config(void);

extern Ewd_List *ewl_window_list;

/**
 * ewl_config_init - initialize the configuration system
 *
 * Returns true on success, false on failure. This sets up the necessary
 * configuration variables.
 */
int
ewl_config_init(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	memset(&ewl_config, 0, sizeof(Ewl_Config));

	if (!__open_user_config())
	  {
		  __close_config();
		  DRETURN_INT(FALSE, DLEVEL_STABLE);
	  }
	else
		__create_user_config();

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * ewl_config_set_str - set the value of key to the specified string
 * @k: the key to set in the configuration database
 * @v: the string value that will be associated with the key
 *
 * Returns TRUE on success, FALSE on failure. Sets the string value associated
 * with the key @k to @v in the configuration database.
 */
int
ewl_config_set_str(char *k, char *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	e_db_str_set(config_db, k, v);

	__close_config();

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}



/**
 * ewl_config_set_int - set the value of key to the specified integer
 * @k: the key to set in the configuration database
 * @v: the integer value that will be associated with the key
 *
 * Returns TRUE on success, FALSE on failure. Sets the integer value associated
 * with the key @k to @v in the configuration database.
 */
int
ewl_config_set_int(char *k, int v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	e_db_int_set(config_db, k, v);

	__close_config();

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}


/**
 * ewl_config_set_float - set the value of key to the specified float
 * @k: the key to set in the configuration database
 * @v: the float value that will be associated with the key
 *
 * Returns TRUE on success, FALSE on failure. Sets the float value associated
 * with the key @k to @v in the configuration database.
 */
int
ewl_config_set_float(char *k, float v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	e_db_float_set(config_db, k, v);

	__close_config();

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}


/**
 * ewl_config_get_str - retrieve string value associated with a key
 * @k: the key to search
 *
 * Returns the string value associated with key @k in the configuration
 * database on success, NULL on failure.
 */
char *
ewl_config_get_str(char *k)
{
	char *ret = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
	  {
		  ret = e_db_str_get(config_db, k);

		  __close_config();
	  }

	DRETURN_PTR(ret, DLEVEL_STABLE);
}


/**
 * ewl_config_get_int - retrieve integer value associated with a key
 * @k: the key to search
 *
 * Returns the integer value associated with key @k in the configuration
 * database on success, 0 on failure.
 */
int
ewl_config_get_int(char *k)
{
	int ret = -1;
	int v;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
	  {
		  ret = e_db_int_get(config_db, k, &v);

		  __close_config();
	  }

	if (ret < 0)
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	DRETURN_INT(ret, DLEVEL_STABLE);
}

/**
 * ewl_config_get_float - retrieve floating point value associated with a key
 * @k: the key to search
 *
 * Returns the float value associated with key @k in the configuration
 * database on success, 0.0 on failure.
 */
float
ewl_config_get_float(char *k)
{
	int ret = -1;
	float v = 0.0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!__open_user_config())
	  {
		  ret = e_db_float_get(config_db, k, &v);

		  __close_config();
	  }

	if (ret < 0)
		DRETURN_FLOAT(0.0, DLEVEL_STABLE);

	DRETURN_FLOAT(ret, DLEVEL_STABLE);
}

/**
 * ewl_config_get_render_method - retrieve the render method of the evas
 *
 * Returns the found render method on success, software rendering on failure.
 */
Evas_Render_Method
ewl_config_get_render_method()
{
	Evas_Render_Method method = RENDER_METHOD_ALPHA_SOFTWARE;
	char *str = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	str = ewl_config_get_str("/evas/render_method");

	if (str)
	  {
		  if (!strncasecmp(str, "software", 8))
			  method = RENDER_METHOD_ALPHA_SOFTWARE;
		  else if (!strncasecmp(str, "hardware", 8))
			  method = RENDER_METHOD_3D_HARDWARE;
		  else if (!strncasecmp(str, "x11", 3))
			  method = RENDER_METHOD_BASIC_HARDWARE;

		  FREE(str);
	  }

	DRETURN_INT(method, DLEVEL_STABLE);
}

/**
 * ewl_config_reread_and_apply - reread the values of the configuration database
 *
 * Returns no value. Reads in the values of the configuration database and
 * applies them to the running ewl program.
 */
void
ewl_config_reread_and_apply(void)
{
	Ewl_Config nc;

	DENTER_FUNCTION(DLEVEL_STABLE);

	nc.debug.enable = ewl_config_get_int("/debug/enable");
	nc.debug.level = ewl_config_get_int("/debug/level");
	nc.evas.font_cache = ewl_config_get_int("/evas/font_cache");
	nc.evas.image_cache = ewl_config_get_int("/evas/image_cache");
	nc.evas.render_method = ewl_config_get_str("/evas/render_method");
	nc.fx.max_fps = ewl_config_get_float("/fx/max_fps");
	nc.fx.timeout = ewl_config_get_float("/fx/timeout");
	nc.theme.name = ewl_config_get_str("/theme/name");
	nc.theme.cache = ewl_config_get_int("/theme/cache");

	if (ewl_window_list && !ewd_list_is_empty(ewl_window_list))
	  {
		  Ewl_Widget *w;

		  ewd_list_goto_first(ewl_window_list);

		  while ((w = ewd_list_next(ewl_window_list)) != NULL)
		    {
			    if (!w->evas)
				    continue;

			    if (nc.evas.font_cache)
			      {
				      evas_flush_font_cache(w->evas);
				      evas_set_font_cache(w->evas,
							  nc.evas.font_cache);
			      }

			    if (nc.evas.image_cache)
			      {
				      evas_flush_image_cache(w->evas);
				      evas_set_image_cache(w->evas,
							   nc.evas.
							   image_cache);
			      }

			    evas_set_output_method(w->evas,
						   ewl_config_get_render_method
						   ());
		    }
	  }

	IF_FREE(ewl_config.evas.render_method);
	IF_FREE(ewl_config.theme.name);

	ewl_config.debug.enable = nc.debug.enable;
	ewl_config.debug.level = nc.debug.level;
	ewl_config.evas.font_cache = nc.evas.font_cache;
	ewl_config.evas.image_cache = nc.evas.image_cache;
	ewl_config.evas.render_method = nc.evas.render_method;
	ewl_config.fx.max_fps = nc.fx.max_fps;
	ewl_config.fx.timeout = nc.fx.timeout;
	ewl_config.theme.name = nc.theme.name;
	ewl_config.theme.cache = nc.theme.cache;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__create_user_config(void)
{
	char *home;
	char pe[256];

	DENTER_FUNCTION(DLEVEL_STABLE);

	home = getenv("HOME");

	if (!home)
	  {
		  DWARNING("Failed to fetch environment variable HOME\n");
		  DRETURN(DLEVEL_STABLE);
	  }

	snprintf(pe, 1024, "%s/.e", home);
	mkdir(pe, 0755);
	snprintf(pe, 1024, "%s/.e/ewl", home);
	mkdir(pe, 0755);
	snprintf(pe, 1024, "%s/.e/ewl/config", home);
	mkdir(pe, 0755);

	ewl_config_set_int("/debug/enable", 0);
	ewl_config_set_int("/debug/level", 0);
	ewl_config_set_str("/evas/render_method", "software");
	ewl_config_set_int("/evas/font_cache", 2097152);
	ewl_config_set_int("/evas/image_cache", 8388608);
	ewl_config_set_float("/fx/max_fps", 25.0);
	ewl_config_set_float("/fx/timeout", 2.0);
	ewl_config_set_str("/theme/name", "default");
	ewl_config_set_int("/theme/cache", 1);

	DRETURN(DLEVEL_STABLE);
}

static int
__open_user_config(void)
{
	char *home;
	char path[256];

	DENTER_FUNCTION(DLEVEL_STABLE);

	home = getenv("HOME");

	if (!home)
	  {
		  DWARNING("Failed to fetch environment variable HOME\n");
		  DRETURN_INT(FALSE, DLEVEL_STABLE);
	  }

	snprintf(path, 256, "%s/.e/ewl/config/system.db", home);

	config_db = e_db_open(path);

	if (config_db)
		DRETURN_INT(TRUE, DLEVEL_STABLE);

	DRETURN_INT(FALSE, DLEVEL_STABLE);
}

static void
__close_config(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (config_db)
		e_db_close(config_db);

	e_db_flush();

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
