#include "erss.h"
#include "parse_config.h"



Evas        *evas=NULL;
Ecore_Evas  *ee=NULL;
int          world_x,
             world_y;
Evas_Object *bg=NULL,
            *cont=NULL,
            *tid=NULL;



int erss_set_time (void *data)
{
	erss_feed *f=(erss_feed *)data;
	char      *str;
	char       text[100];

	str = erss_time_format ();
	if (f->last_time)
		snprintf (text, sizeof (text), "Last update: %s", f->last_time);

	edje_object_part_text_set (tid, "clock", text);

	free (str);

	return TRUE;
}






static void erss_window_move (Ecore_Evas *ee)
{
	int x, y, w, h;
	Evas_Object *o = NULL;

	ecore_evas_geometry_get (ee, &x, &y, &w, &h);

	if((o = evas_object_name_find(ecore_evas_get(ee), "root_background")))
		esmart_trans_x11_freshen(o, x, y, w, h);

	world_x = x;
	world_y = y;

}
	
void erss_window_resize(Ecore_Evas *ee)
{
	int x, y, w, h;
	Evas_Object *o = NULL;

	ecore_evas_geometry_get(ee, &x, &y, &w, &h);

	if((o = evas_object_name_find(ecore_evas_get(ee), "root_background")))
	{
		evas_object_resize(o, w, h);
		esmart_trans_x11_freshen(o, x, y, w, h);
	}
	
	if((o = evas_object_name_find(ecore_evas_get(ee), "background")))
	{
		evas_object_resize(o, w, h);
	}

	if((o = evas_object_name_find(ecore_evas_get(ee), "container")))
		evas_object_resize(o, w, h);
}






void erss_mouse_click_item (void *data, Evas_Object *o, const char *sig, 
		const char *src)
{
	char *url = data;
	char  c[1024];

	if (!rc->browser) {
		fprintf (stderr, "%s error: you have not defined any browser in your config file setting /usr/bin/mozilla as default\n", PACKAGE);
		rc->browser = strdup ("mozilla");
	}
	
	snprintf (c, sizeof (c), "%s \"%s\"", rc->browser, url);
	ecore_exe_run (c, NULL);
}

void erss_mouse_in_cursor_change (void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	Ecore_X_Window win;

	win = ecore_evas_software_x11_window_get(ee);
	if (cfg->item_url)
		ecore_x_cursor_shape_set(win, ECORE_X_CURSOR_HAND2);
}

void erss_mouse_out_cursor_change (void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	Ecore_X_Window win;
  
	win = ecore_evas_software_x11_window_get(ee);
	ecore_x_cursor_set(win, 0);
}






int erss_gui_init (char *config) {
	Ecore_X_Window  win;
	Evas_Object    *header;
	char            title[PATH_MAX];
	int             x,y,w,h;
	int             height,width;

	ecore_x_init (NULL);
	if (!ecore_evas_init ()) return -1;

	width = 300;
	height = 16 * cfg->num_stories;

	if (cfg->header) height += 26;
	if (cfg->clock) height += 26;

	ee = ecore_evas_software_x11_new (NULL, 0, 0, 0, width, height);
	win = ecore_evas_software_x11_window_get(ee);
	
	if (!ee)
		return -1;

	ecore_evas_borderless_set (ee, cfg->borderless);
	snprintf(title, PATH_MAX, "erss - %s", config);
	ecore_evas_title_set (ee, title);
	ecore_x_window_prop_layer_set(win, ECORE_X_WINDOW_LAYER_BELOW);
	ecore_evas_show (ee);
	
	/* ecore_x_window_lower(win); */

	if (cfg->x != 0 && cfg->y != 0)
		ecore_evas_move (ee, cfg->x, cfg->y);

	evas = ecore_evas_get (ee);

	evas_font_path_append (evas, PACKAGE_DATA_DIR"/fonts/");

/*	erss_net_connect(&f); */

	ecore_evas_geometry_get (ee, &x, &y, &w, &h);
	
	bg = esmart_trans_x11_new (evas);
	evas_object_move (bg, 0, 0);
	evas_object_layer_set (bg, -5);
	evas_object_resize (bg, w, h);
	evas_object_name_set(bg, "root_background");
	evas_object_show (bg);

	bg = evas_object_rectangle_add(evas);
	evas_object_move (bg, 0, 0);
	evas_object_layer_set (bg, -6);
	evas_object_resize (bg, w, h);
	evas_object_color_set(bg, 255, 255, 255, 0);
	evas_object_name_set(bg, "background");
	evas_object_show (bg);

	ecore_evas_callback_move_set (ee, erss_window_move);
	ecore_evas_callback_resize_set(ee, erss_window_resize);

	cont = e_container_new(evas);
	evas_object_move(cont, 0, 0);
	evas_object_resize(cont, width, height);
	evas_object_layer_set(cont, 0);
	evas_object_name_set(cont, "container");
	evas_object_show(cont);
	e_container_padding_set(cont, 10, 10, 10, 10);
	e_container_spacing_set(cont, 5);
	e_container_direction_set(cont, 1);
	e_container_fill_policy_set(cont,
			CONTAINER_FILL_POLICY_FILL);

	edje_init();

	if (cfg->header) {
		header = edje_object_add (evas);
		edje_object_file_set (header, cfg->theme, "erss");
		edje_object_part_text_set (header, "header", cfg->header);
		evas_object_show (header);

		evas_object_event_callback_add (header,
						EVAS_CALLBACK_MOUSE_IN, erss_mouse_in_cursor_change, NULL);
		evas_object_event_callback_add (header,
						EVAS_CALLBACK_MOUSE_OUT, erss_mouse_out_cursor_change, NULL);

		edje_object_signal_callback_add (header, "exec*", "*",
						 erss_mouse_click_item, cfg->hostname);
		edje_object_signal_emit (header, "mouse,in", "article");
		edje_object_signal_emit (header, "mouse,out", "article");

		e_container_element_append(cont, header);
	}

	if ((rc->clock==1)||((cfg->clock==1)&&(rc->clock!=0))) {
		tid = edje_object_add (evas);
		edje_object_file_set (tid, cfg->theme, "erss_clock");
		edje_object_part_text_set (tid, "clock", "");
		evas_object_show (tid);

		e_container_element_append(cont, tid);
	}
	return 0;
}



int erss_gui_exit (void) {
	ecore_evas_shutdown ();
	ecore_x_shutdown ();
	return 0;
}
