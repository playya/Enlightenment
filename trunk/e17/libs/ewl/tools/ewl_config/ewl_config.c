#include <Ewl.h>
#include <ewl-config.h>

struct _ewl_config_main
{
	Ewl_Widget *main_win;
	Ewl_Widget *main_vbox;
	Ewl_Widget *button_hbox;
	Ewl_Widget *button_save;
	Ewl_Widget *button_restore;
	Ewl_Widget *button_defaults;
	Ewl_Widget *button_exit;
	Ewl_Widget *notebook;

	Ewl_Widget *page_evas_label;
	Ewl_Widget *page_evas;
	Ewl_Widget *render_method_label;
	Ewl_Widget *render_method_software;
	Ewl_Widget *render_method_hardware;
	Ewl_Widget *render_method_x11;
	Ewl_Widget *font_cache_label;
	Ewl_Widget *font_cache;
	Ewl_Widget *image_cache_label;
	Ewl_Widget *image_cache;

	Ewl_Widget *page_debug_label;
	Ewl_Widget *page_debug;
	Ewl_Widget *enable_debug;
	Ewl_Widget *debug_level_label;
	Ewl_Widget *debug_level;

	Ewl_Widget *page_fx_label;
	Ewl_Widget *page_fx;
	Ewl_Widget *max_fps_label;
	Ewl_Widget *max_fps;
	Ewl_Widget *timeout_label;
	Ewl_Widget *timeout;

	Ewl_Widget *page_theme_label;
	Ewl_Widget *page_theme;
	Ewl_Widget *theme_name_label;
	Ewl_Widget *theme_name;
}
e_conf;

Ewl_Config user_settings;
Ewl_Config init_settings;
Ewl_Config default_settings;

void ewl_config_read_configs(void);
int ewl_config_read_config(Ewl_Config * conf);
void ewl_set_settings(Ewl_Config * c);
Ewl_Config *ewl_get_settings(void);
void ewl_save_user_config(Ewl_Config * c);

void ewl_config_save_cb(Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_config_restore_cb(Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_config_defaults_cb(Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_config_exit_cb(Ewl_Widget * w, void *ev_data, void *user_data);

int
main(int argc, char **argv)
{
	memset(&e_conf, 0, sizeof(struct _ewl_config_main));

	ewl_init(argc, argv);

	ewl_config_read_configs();

	ewl_theme_data_set_default("/appearance/box/horizontal/base/visible",
				   "no");
	ewl_theme_data_set_default("/appearance/box/vertical/base/visible",
				   "no");

	e_conf.main_win = ewl_window_new();
	ewl_window_resize(e_conf.main_win, 446, 300);
	ewl_window_set_min_size(e_conf.main_win, 446, 300);
	ewl_window_set_title(e_conf.main_win, "EWL Configuration Program");
	ewl_callback_append(e_conf.main_win, EWL_CALLBACK_DELETE_WINDOW,
			    ewl_config_exit_cb, NULL);
	ewl_widget_show(e_conf.main_win);

	e_conf.main_vbox = ewl_vbox_new();
	ewl_box_set_spacing(e_conf.main_vbox, 10);
	ewl_container_append_child(EWL_CONTAINER(e_conf.main_win),
				   e_conf.main_vbox);
	ewl_widget_show(e_conf.main_vbox);

	e_conf.notebook = ewl_notebook_new();
	ewl_container_append_child(EWL_CONTAINER(e_conf.main_vbox),
				   e_conf.notebook);
	ewl_widget_show(e_conf.notebook);

	e_conf.button_hbox = ewl_hbox_new();
	ewl_box_set_spacing(e_conf.button_hbox, 5);
	ewl_object_set_padding(EWL_OBJECT(e_conf.button_hbox), 0, 0, 0, 10);
	ewl_object_set_custom_size(EWL_OBJECT(e_conf.button_hbox), 415, 17);
	ewl_object_set_alignment(EWL_OBJECT(e_conf.button_hbox),
				 EWL_ALIGNMENT_CENTER);
	ewl_object_set_fill_policy(EWL_OBJECT(e_conf.button_hbox),
				   EWL_FILL_POLICY_NORMAL);
	ewl_container_append_child(EWL_CONTAINER(e_conf.main_vbox),
				   e_conf.button_hbox);
	ewl_widget_show(e_conf.button_hbox);

	e_conf.button_save = ewl_button_new("Save");
	ewl_object_set_custom_size(EWL_OBJECT(e_conf.button_save), 100, 17);
	ewl_container_append_child(EWL_CONTAINER(e_conf.button_hbox),
				   e_conf.button_save);
	ewl_callback_append(e_conf.button_save, EWL_CALLBACK_CLICKED,
			    ewl_config_save_cb, NULL);
	ewl_widget_show(e_conf.button_save);

	e_conf.button_restore = ewl_button_new("Restore");
	ewl_object_set_custom_size(EWL_OBJECT(e_conf.button_restore), 100,
				   17);
	ewl_container_append_child(EWL_CONTAINER(e_conf.button_hbox),
				   e_conf.button_restore);
	ewl_callback_append(e_conf.button_restore, EWL_CALLBACK_CLICKED,
			    ewl_config_restore_cb, NULL);
	ewl_widget_show(e_conf.button_restore);

	e_conf.button_defaults = ewl_button_new("Defaults");
	ewl_object_set_custom_size(EWL_OBJECT(e_conf.button_defaults), 100,
				   17);
	ewl_container_append_child(EWL_CONTAINER(e_conf.button_hbox),
				   e_conf.button_defaults);
	ewl_callback_append(e_conf.button_defaults, EWL_CALLBACK_CLICKED,
			    ewl_config_defaults_cb, NULL);
	ewl_widget_show(e_conf.button_defaults);

	e_conf.button_exit = ewl_button_new("Exit");
	ewl_object_set_custom_size(EWL_OBJECT(e_conf.button_exit), 100, 17);
	ewl_container_append_child(EWL_CONTAINER(e_conf.button_hbox),
				   e_conf.button_exit);
	ewl_callback_append(e_conf.button_exit, EWL_CALLBACK_CLICKED,
			    ewl_config_exit_cb, NULL);
	ewl_widget_show(e_conf.button_exit);

	/* Evas Page */

	e_conf.page_evas_label = ewl_text_new();
	ewl_text_set_text(e_conf.page_evas_label, "Evas Settings");
	ewl_text_set_font_size(e_conf.page_evas_label, 8);
	ewl_widget_show(e_conf.page_evas_label);

	e_conf.page_evas = ewl_vbox_new();
	ewl_object_set_padding(EWL_OBJECT(e_conf.page_evas), 10, 5, 5, 0);
	ewl_box_set_spacing(e_conf.page_evas, 5);
	ewl_widget_show(e_conf.page_evas);

	e_conf.render_method_label = ewl_text_new();
	ewl_text_set_text(e_conf.render_method_label, "Render Method");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.render_method_label);
	ewl_widget_show(e_conf.render_method_label);

	e_conf.render_method_software =
		ewl_radiobutton_new("Software Engine");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.render_method_software);
	ewl_widget_show(e_conf.render_method_software);

	e_conf.render_method_hardware =
		ewl_radiobutton_new("Hardware Engine");
	ewl_radiobutton_set_chain(e_conf.render_method_hardware,
				  e_conf.render_method_software);
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.render_method_hardware);
	ewl_widget_show(e_conf.render_method_hardware);

	e_conf.render_method_x11 = ewl_radiobutton_new("X11 Engine");
	ewl_radiobutton_set_chain(e_conf.render_method_x11,
				  e_conf.render_method_hardware);
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.render_method_x11);
	ewl_widget_show(e_conf.render_method_x11);

	e_conf.font_cache_label = ewl_text_new();
	ewl_text_set_text(e_conf.font_cache_label, "Font Cache (kB)");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.font_cache_label);
	ewl_widget_show(e_conf.font_cache_label);

	e_conf.font_cache = ewl_spinner_new();
	ewl_spinner_set_min_val(e_conf.font_cache, (1024 * 1024 * 0.5));
	ewl_spinner_set_max_val(e_conf.font_cache, (1024 * 1024 * 250));
	ewl_spinner_set_digits(e_conf.font_cache, 0);
	ewl_spinner_set_step(e_conf.font_cache, 1);
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.font_cache);
	ewl_widget_show(e_conf.font_cache);

	e_conf.image_cache_label = ewl_text_new();
	ewl_text_set_text(e_conf.image_cache_label, "Image Cache (kB)");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.image_cache_label);
	ewl_widget_show(e_conf.image_cache_label);

	e_conf.image_cache = ewl_spinner_new();
	ewl_spinner_set_min_val(e_conf.image_cache, (1024 * 1024 * 0.5));
	ewl_spinner_set_max_val(e_conf.image_cache, (1024 * 1024 * 250));
	ewl_spinner_set_digits(e_conf.image_cache, 0);
	ewl_spinner_set_step(e_conf.image_cache, 1);
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_evas),
				   e_conf.image_cache);
	ewl_widget_show(e_conf.image_cache);

	ewl_notebook_append_page(e_conf.notebook, e_conf.page_evas,
				 e_conf.page_evas_label);

	/* Debug Page */

	e_conf.page_debug_label = ewl_text_new();
	ewl_text_set_text(e_conf.page_debug_label, "Debug Settings");
	ewl_text_set_font_size(e_conf.page_debug_label, 8);
	ewl_widget_show(e_conf.page_debug_label);

	e_conf.page_debug = ewl_vbox_new();
	ewl_object_set_padding(EWL_OBJECT(e_conf.page_debug), 10, 5, 5, 0);
	ewl_box_set_spacing(e_conf.page_debug, 5);
	ewl_widget_show(e_conf.page_debug);

	e_conf.enable_debug = ewl_checkbutton_new("Enable Debug ?");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_debug),
				   e_conf.enable_debug);
	ewl_widget_show(e_conf.enable_debug);

	e_conf.debug_level_label = ewl_text_new();
	ewl_text_set_text(e_conf.debug_level_label, "Debug Level");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_debug),
				   e_conf.debug_level_label);
	ewl_widget_show(e_conf.debug_level_label);

	e_conf.debug_level = ewl_spinner_new();
	ewl_spinner_set_min_val(e_conf.debug_level, 0.0);
	ewl_spinner_set_max_val(e_conf.debug_level, 20.0);
	ewl_spinner_set_digits(e_conf.debug_level, 0);
	ewl_spinner_set_step(e_conf.debug_level, 1);
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_debug),
				   e_conf.debug_level);
	ewl_widget_show(e_conf.debug_level);

	ewl_notebook_append_page(e_conf.notebook, e_conf.page_debug,
				 e_conf.page_debug_label);

	/* FX Page */

	e_conf.page_fx_label = ewl_text_new();
	ewl_text_set_text(e_conf.page_fx_label, "FX Settings");
	ewl_text_set_font_size(e_conf.page_fx_label, 8);
	ewl_widget_show(e_conf.page_fx_label);

	e_conf.page_fx = ewl_vbox_new();
	ewl_object_set_padding(EWL_OBJECT(e_conf.page_fx), 10, 5, 5, 0);
	ewl_box_set_spacing(e_conf.page_fx, 5);
	ewl_widget_show(e_conf.page_fx);


	e_conf.max_fps_label = ewl_text_new();
	ewl_text_set_text(e_conf.max_fps_label, "Max FPS");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_fx),
				   e_conf.max_fps_label);
	ewl_widget_show(e_conf.max_fps_label);

	e_conf.max_fps = ewl_spinner_new();
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_fx),
				   e_conf.max_fps);
	ewl_widget_show(e_conf.max_fps);

	e_conf.timeout_label = ewl_text_new();
	ewl_text_set_text(e_conf.timeout_label, "FX Timeout");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_fx),
				   e_conf.timeout_label);
	ewl_widget_show(e_conf.timeout_label);

	e_conf.timeout = ewl_spinner_new();
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_fx),
				   e_conf.timeout);
	ewl_widget_show(e_conf.timeout);

	ewl_notebook_append_page(e_conf.notebook, e_conf.page_fx,
				 e_conf.page_fx_label);

	/* Theme Page */
	e_conf.page_theme_label = ewl_text_new();
	ewl_text_set_text(e_conf.page_theme_label, "Theme Settings");
	ewl_text_set_font_size(e_conf.page_theme_label, 8);
	ewl_widget_show(e_conf.page_theme_label);

	e_conf.page_theme = ewl_vbox_new();
	ewl_object_set_padding(EWL_OBJECT(e_conf.page_theme), 10, 5, 5, 0);
	ewl_box_set_spacing(e_conf.page_theme, 5);
	ewl_widget_show(e_conf.page_theme);


	e_conf.theme_name_label = ewl_text_new();
	ewl_text_set_text(e_conf.theme_name_label, "Theme Name");
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_theme),
				   e_conf.theme_name_label);
	ewl_widget_show(e_conf.theme_name_label);

	e_conf.theme_name = ewl_entry_new();
	ewl_container_append_child(EWL_CONTAINER(e_conf.page_theme),
				   e_conf.theme_name);
	ewl_widget_show(e_conf.theme_name);

	ewl_notebook_append_page(e_conf.notebook, e_conf.page_theme,
				 e_conf.page_theme_label);

	ewl_set_settings(&init_settings);

	ewl_main();

	exit(1);
}

void
ewl_config_read_configs(void)
{
	int user_read;

	user_read = ewl_config_read_config(&user_settings);

	if (user_read != -1)
		ewl_config_read_config(&init_settings);
	else if (user_read == -1)
	  {
		  printf("Couldnt open user config, please check permissions\n");
		  exit(-1);
	  }
}

int
ewl_config_read_config(Ewl_Config * conf)
{
	if (!conf)
		return -1;

	/* Evas stuff */
	conf->evas.render_method = ewl_config_get_str("/evas/render_method");

	if (!conf->evas.render_method)
		conf->evas.render_method = strdup("default");

	if (!ewl_config_get_int("/evas/font_cache", &conf->evas.font_cache))
		conf->evas.font_cache = 1024 * 1024 * 2;

	if (!ewl_config_get_int("/evas/image_cache", &conf->evas.image_cache))
		conf->evas.image_cache = 1024 * 1024 * 8;

	/* Debug stuff */
	if (!ewl_config_get_int("/debug/enable", &conf->debug.enable))
		conf->debug.enable = 0;

	if (!ewl_config_get_int("/debug/level", &conf->debug.level))
		conf->debug.level = 0;

	/* FX stuff */
	if (!ewl_config_get_float("/fx/max_fps", &conf->fx.max_fps))
		conf->fx.max_fps = 25.0;

	if (!ewl_config_get_float("/fx/timeout", &conf->fx.timeout))
		conf->fx.timeout = 2.0;

	/* Theme stuff */
	conf->theme.name = ewl_config_get_str("/theme/name");

	if (!conf->theme.name)
		conf->theme.name = strdup("default");

	return 1;
}

void
ewl_set_settings(Ewl_Config * c)
{
	if (c->evas.render_method
	    && !strncasecmp(c->evas.render_method, "software", 8))
		ewl_radiobutton_set_checked(e_conf.render_method_software, 1);
	else if (c->evas.render_method
		 && !strncasecmp(c->evas.render_method, "hardware", 8))
		ewl_radiobutton_set_checked(e_conf.render_method_hardware, 1);
	else if (c->evas.render_method
		 && !strncasecmp(c->evas.render_method, "x11", 3))
		ewl_radiobutton_set_checked(e_conf.render_method_x11, 1);
	else
		ewl_radiobutton_set_checked(e_conf.render_method_software, 1);

	ewl_spinner_set_value(e_conf.font_cache,
			      (double) (c->evas.font_cache));
	ewl_spinner_set_value(e_conf.image_cache,
			      (double) (c->evas.image_cache));

	ewl_checkbutton_set_checked(e_conf.enable_debug, c->debug.enable);
	ewl_spinner_set_value(e_conf.debug_level, (double) (c->debug.level));

	ewl_spinner_set_value(e_conf.max_fps, (double) (c->fx.max_fps));
	ewl_spinner_set_value(e_conf.timeout, (double) (c->fx.timeout));

	ewl_entry_set_text(e_conf.theme_name, c->theme.name);
}

Ewl_Config *
ewl_get_settings(void)
{
	Ewl_Config *c;

	c = NEW(Ewl_Config, 1);
	memset(c, 0, sizeof(Ewl_Config));

	if (ewl_radiobutton_is_checked(e_conf.render_method_software))
		c->evas.render_method = strdup("software");
	else if (ewl_radiobutton_is_checked(e_conf.render_method_hardware))
		c->evas.render_method = strdup("hardware");
	else if (ewl_radiobutton_is_checked(e_conf.render_method_x11))
		c->evas.render_method = strdup("x11");

	c->evas.font_cache = (int) (ewl_spinner_get_value(e_conf.font_cache));
	c->evas.image_cache =
		(int) (ewl_spinner_get_value(e_conf.image_cache));

	if (ewl_checkbutton_is_checked(e_conf.enable_debug))
		c->debug.enable = 1;
	else
	  {
		  c->debug.enable = 0;
		  c->debug.level = 0;
	  }

	if (c->debug.enable)
		c->debug.level =
			(int) (ewl_spinner_get_value(e_conf.debug_level));

	c->fx.max_fps = (float) (ewl_spinner_get_value(e_conf.max_fps));
	c->fx.timeout = (float) (ewl_spinner_get_value(e_conf.timeout));

	c->theme.name = ewl_entry_get_text(e_conf.theme_name);

	if (!c->theme.name)
		c->theme.name = strdup("default");

	return c;
}

void
ewl_save_config(Ewl_Config * c)
{
	if (!c)
		return;

	ewl_config_set_int("/evas/font_cache", c->evas.font_cache);
	ewl_config_set_int("/evas/image_cache", c->evas.image_cache);
	ewl_config_set_str("/evas/render_method", c->evas.render_method);
	ewl_config_set_int("/debug/enable", c->debug.enable);
	ewl_config_set_int("/debug/level", c->debug.level);
	ewl_config_set_float("/fx/max_fps", c->fx.max_fps);
	ewl_config_set_float("/fx/timeout", c->fx.timeout);
	ewl_config_set_str("/theme/name", c->theme.name);
}

void
ewl_config_save_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Config *c;
	char *home;
	char pe[1024];

	home = getenv("HOME");

	if (!home)
	  {
		  printf("ERROR: Environment variable $HOME was not found.\n"
			 "Try export HOME in a bash-like environment\n"
			 "or setenv HOME in a sh like environment.\n");
		  exit(-1);
	  }

	snprintf(pe, 1024, "%s/.e", home);
	mkdir(pe, 0755);
	snprintf(pe, 1024, "%s/.e/ewl", home);
	mkdir(pe, 0755);
	snprintf(pe, 1024, "%s/.e/ewl/config", home);
	mkdir(pe, 0755);

	c = ewl_get_settings();

	ewl_save_config(c);

	FREE(c->evas.render_method);
	FREE(c->theme.name);
	FREE(c);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void
ewl_config_restore_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_set_settings(&init_settings);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void
ewl_config_defaults_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void
ewl_config_exit_cb(Ewl_Widget * w, void *user_data, void *ev_data)
{
	ewl_widget_destroy_recursive(e_conf.main_win);

	ewl_main_quit();

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
