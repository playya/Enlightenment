#include "erss.h"
#include "parse.h"
#include "parse_config.h"

Evas *evas = NULL;
Ecore_Evas *ee = NULL;
Ecore_Con_Server *server = NULL;
Evas_Object *bg = NULL;
Evas_Object *cont = NULL;
Evas_Object *tid = NULL;
Ewd_List *config_files = NULL;

char *main_buffer = NULL;
char *last_time = NULL;

int main_bufsize = 0;
int waiting_for_reply = FALSE;
int total_connects = 0;

Erss_Tooltip *tt = NULL;


void erss_xml_error_handler (void *ctx, const char *msg, ...)
{
	fprintf (stderr, "%s xml report: %s\n", PACKAGE, msg);

	return;
}

int erss_connect (void *data)
{
	if (waiting_for_reply) {
		fprintf (stderr, "%s warning: client has not received all information ", 
				PACKAGE);
		fprintf (stderr, "from the last connection attempt yet! \n");
		return TRUE;
	}
	
	server = NULL;
	
	if (last_time)
		free (last_time);
	else
		printf ("%s info: connecting to '%s' ...\n", PACKAGE, cfg->url);

	if (rc->proxy) {
		if (!strcasecmp (rc->proxy, "")) {
			server = ecore_con_server_connect (ECORE_CON_REMOTE_SYSTEM,
												 cfg->hostname, 80, NULL);
		} else {
			if (!rc->proxy_port)
			{
				fprintf (stderr, 
						"%s error: You need to define a proxy port!\n", PACKAGE);
				exit (-1);
			}
			server = ecore_con_server_connect (ECORE_CON_REMOTE_SYSTEM,
												 rc->proxy, rc->proxy_port, NULL);
		}
	} else {
		server = ecore_con_server_connect (ECORE_CON_REMOTE_SYSTEM,
				cfg->hostname, 80, NULL);
	}

	if (!server) {
		fprintf (stderr, "%s error: Could not connect to server ..\n", PACKAGE);
		exit (-1);
	}

	total_connects++;
	last_time = strdup (erss_time_format ());
	erss_set_time (NULL);

	return TRUE;
}

int erss_alphasort (const void *a, const void *b)
{
	struct dirent **ad = (struct dirent **)a;
	struct dirent **bd = (struct dirent **)b;
	return (strcmp((*bd)->d_name, (*ad)->d_name));
}

char *erss_time_format () 
{
	char    *str;
	struct  tm  *ts;
	time_t  curtime;
	
	curtime = time(NULL);
	ts = localtime(&curtime);

	str = malloc (11);

	snprintf (str, 20, "%02d:%02d:%02d", 
			ts->tm_hour, ts->tm_min, ts->tm_sec);

	return str;
}

int erss_set_time (void *data) {
	char *str;
	char text[100];

	str = erss_time_format ();
	if (last_time)
		snprintf (text, sizeof (text), "Time now: %s  Last update: %s", 
				str, last_time);
	else
		snprintf (text, sizeof (text), "Time now: %s", str);
	
	edje_object_part_text_set (tid, "clock", text);

	free (str);

	return TRUE;
}

int handler_signal_exit (void *data, int ev_type, void *ev)
{
	Ecore_Event_Signal_Exit *e = ev;

	if (e->interrupt)
		printf ("%s exit: interrupt\n", PACKAGE);
	if (e->quit)
		printf ("%s exit: quit\n", PACKAGE);
	if (e->terminate)
		printf ("%s exit: terminate\n", PACKAGE);

	ecore_main_loop_quit ();

	return 1;
}

int handler_server_add (void *data, int type, void *event)
{
	Article *ptr;
	char c[1024];

	/*
	 * Clean out the evas objects from the container to
	 * make room for the new items.
	 */
	if (list) {
		ptr = ewd_list_goto_first (list);
		while ((ptr = ewd_list_next(list))) {

			/*
			 * Remove the evas object from the list 
			 * and destory it.
			 */

			if (ptr->obj)
				e_container_element_destroy (cont, ptr->obj);

			if (ptr->url)
				free (ptr->url);

			if (ptr->description)
				free (ptr->description);

			free (ptr);
		}
	}

	/* 
	 * Remove the list, we want to build a new one for 
	 * the next connection.
	 */
	if (list)
	{
		ewd_list_remove (list);
		list = NULL;
	}
	
	item = NULL;
	list = ewd_list_new ();

	/*
	 * We want to be connected before sending the request.
	 */
	if (total_connects == 1)
		printf ("%s info: sending HTTP request ...\n", PACKAGE);
	
	snprintf (c, sizeof (c), "GET %s HTTP/1.0\r\n", cfg->url);
	ecore_con_server_send (server, c, strlen (c));
	snprintf (c, sizeof (c), "Host: %s \r\n", cfg->hostname);
	ecore_con_server_send (server, c, strlen (c));
   snprintf (c, sizeof (c), "User-Agent: %s/%s\r\n\r\n", 
			 PACKAGE, VERSION);
   ecore_con_server_send (server, c, strlen (c));
	
	waiting_for_reply = TRUE;

	return 1;
}

int handler_server_data (void *data, int type, void *event)
{
	Ecore_Con_Event_Server_Data *e = event;

	if (total_connects == 1)
		printf ("%s info: receiving data ...\n", PACKAGE);

	/* 
	 * Read everything we receive into one big buffer, and handle
	 * that buffer when the server disconnects.
	 */
	main_buffer = realloc (main_buffer, main_bufsize + e->size);
	memcpy (main_buffer + main_bufsize, e->data, e->size);
	main_bufsize += e->size;

	return 1;
}

int handler_server_del (void *data, int type, void *event)
{
	Ecore_Con_Event_Server_Del *e = event;
	char *buf = main_buffer;
	xmlDocPtr doc;
	char *temp;

	if (total_connects == 1)
		printf ("%s info: disconnecting ...\n", PACKAGE);

	/*
	 * Now split our buffer in each newline and then parse the line.
	 */

	temp = strchr(buf, '<');
	doc = xmlParseMemory (temp, main_bufsize - (temp - main_buffer));

	erss_parse (doc);

	ecore_con_server_del (e->server);
	server = NULL;
	
	if (ewd_list_is_empty (list)) {
		if (buf) 
			printf ("%s\n", temp);
	 else 
			printf ("%s error: could not connect to '%s'\n", PACKAGE, cfg->url);

		fprintf (stderr, "\n%s error: parsing data\n", PACKAGE);
		fprintf (stderr, "%s error: are you sure you have to correct input in your config file?\n", PACKAGE);

		exit (-1);
	}

	if (main_buffer) {
		free (main_buffer);
		main_buffer = NULL;
	}

	main_bufsize = 0;
	waiting_for_reply = FALSE;

	if (total_connects == 1)
		printf ("%s info: connection information only displays on the first connect.\n", 
				PACKAGE);

	return 1;
}

void erss_window_move (Ecore_Evas * ee)
{
	int x, y, w, h;
	Evas_Object *o = NULL;
	
	ecore_evas_geometry_get (ee, &x, &y, &w, &h);

	if((o = evas_object_name_find(ecore_evas_get(ee), "root_background")))
			esmart_trans_x11_freshen(o, x, y, w, h);
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


void cb_mouse_out_item (void *data, Evas_Object *o, const char *sig, 
		const char *src)
{
	Article *item = data;
	char c[1024];

	if (!rc->browser) {
		fprintf (stderr, "%s error: you have not defined any browser in your config file setting /usr/bin/mozilla as default\n", PACKAGE);
		rc->browser = strdup ("mozilla");
	}
	
	snprintf (c, sizeof (c), "%s \"%s\"", rc->browser, item->url);
	ecore_exe_run (c, NULL);
}


int erss_tooltip_timer (void *data)
{
	Article *item = data;
	int x, y, w, h;
		
	tt->timer = NULL;
	
	if (!item->description) 
		return FALSE;
	
	printf ("Opening tooltip after %lf secs\n", 1.5);


	if (!tt->ee) {
		tt->ee = ecore_evas_software_x11_new (NULL, 0, 0, 0, 350, 200);
		ecore_evas_borderless_set (tt->ee, TRUE);
		ecore_evas_shaped_set (tt->ee, TRUE);
		tt->win = ecore_evas_software_x11_window_get(ee);
		ecore_x_window_prop_window_type_utility_set (tt->win);
		ecore_evas_geometry_get (tt->ee, &x, &y, &w, &h);

		tt->evas = ecore_evas_get (tt->ee);	  
		evas_font_path_append (tt->evas, PACKAGE_DATA_DIR"/fonts/");


		tt->bg = esmart_trans_x11_new (tt->evas);
		evas_object_move (tt->bg, 0, 0);
		evas_object_layer_set (tt->bg, -5);
		evas_object_resize (tt->bg, w, h);
		evas_object_name_set(tt->bg, "root_background");
		evas_object_show (tt->bg);
		
		tt->bg = evas_object_rectangle_add(tt->evas);
		evas_object_move (tt->bg, 0, 0);
		evas_object_layer_set (tt->bg, -6);
		evas_object_resize (tt->bg, w, h);
		evas_object_color_set(tt->bg, 255, 255, 255, 0);
		evas_object_name_set(tt->bg, "background");
		evas_object_show (tt->bg);
	
		tt->etox = etox_new_all(tt->evas, x, y, w, h , 255, ETOX_ALIGN_LEFT);
		etox_context_set_align(etox_get_context(tt->etox), ETOX_ALIGN_LEFT);
		etox_context_set_font(etox_get_context(tt->etox), "Vera", 10);
		etox_context_set_style(etox_get_context(tt->etox), "shadow");
		etox_context_set_color(etox_get_context(tt->etox), 225, 225, 225, 255);
		etox_set_soft_wrap(tt->etox, 1);
		etox_set_alpha(tt->etox, 255);
		evas_object_layer_set(tt->etox, 1000);
		etox_set_text (tt->etox, item->description);
		evas_object_show (tt->etox);


		ecore_evas_callback_move_set (tt->ee, erss_window_move);
		ecore_evas_callback_resize_set(tt->ee, erss_window_resize);
	}
	
	ecore_evas_show (tt->ee);
	
	return FALSE;
}

void cb_mouse_in (void *data, Evas *e, Evas_Object *obj, 
		void *event_info) 
{
	Evas_Event_Mouse_In *event = event_info;
	Article *item = data;
  Ecore_X_Window win;

	/*
	printf ("x: %d  y: %d\n", event->output.x, event->output.y);
	*/

	tt->timer = ecore_timer_add (1.5, erss_tooltip_timer, item);
  
	win = ecore_evas_software_x11_window_get(ee);
	if (cfg->item_url)
   	ecore_x_cursor_shape_set(win, ECORE_X_CURSOR_HAND2);
}

void cb_mouse_out (void *data, Evas *e, Evas_Object *obj, 
		void *event_info) 
{
   Ecore_X_Window win;
	 
	if (tt->ee) 
		ecore_evas_hide (tt->ee);

	if (tt->timer) {
		ecore_timer_del (tt->timer);
		tt->timer = NULL;
	}
	
  win = ecore_evas_software_x11_window_get(ee);
  ecore_x_cursor_set(win, 0);
} 


void erss_list_config_files (int output)
{
	char *str;
	char *ptr;
	Ewd_List *paths;
	struct dirent **dentries;
	struct stat statbuf;
	int num, i;
	char file[PATH_MAX];
	int found_files = FALSE;
	int no_dir = FALSE;

	paths = ewd_list_new ();
	config_files = ewd_list_new ();

	str = malloc (PATH_MAX);
	snprintf (str, PATH_MAX, "%s/.%s/config", getenv("HOME"), PACKAGE);
	ewd_list_append (paths, str);

	str = malloc (PATH_MAX);
	snprintf (str, PATH_MAX, "/etc/%s/config", PACKAGE);
	ewd_list_append (paths, str);

	str = malloc (PATH_MAX);
	snprintf (str, PATH_MAX, "%s/config", PACKAGE_DATA_DIR);
	ewd_list_append (paths, str);

	if (output)
		printf ("\n%s processing potential dirs ...\n", PACKAGE);

	ptr = ewd_list_goto_first (paths);
	while ((ptr = ewd_list_current (paths))) {
		if (output)
			printf ("\nprocessing '%s':\n", ptr);

		i = stat (ptr, &statbuf);

		if (i == -1) {
			no_dir = TRUE;
		} else {
			if (S_ISDIR(statbuf.st_mode)) {

				if ((num = scandir(ptr, &dentries, 0, erss_alphasort)) < 0) 
					perror("erss - scandir");

				while (num--) {
					snprintf(file, PATH_MAX, "%s/%s", ptr, dentries[num]->d_name);

					i = stat (file, &statbuf);
					if (i == -1) {
						perror("erss - stat 1");
						continue;
					}

					if (S_ISDIR(statbuf.st_mode))
						continue;

					if (strstr (dentries[num]->d_name, ".cfg")) {
						found_files = TRUE;
						ewd_list_append (config_files, strdup (file));
						if (output)
							printf ("\t%s\n", file);
					}
				}

				if (!found_files) {
					if (output)
						printf ("\tno config files in this dir ...\n");
					found_files = FALSE;
				}
			}
		}

		if (output) {
			if (no_dir)
				printf ("\tno such dir ...\n");
			no_dir = FALSE;
		}

		ewd_list_next (paths);
	}

	/*
	 * Finished reading and printing avaliable config files
	 * now remove the paths list since we don't need it anyumore.
	 */
	ptr = ewd_list_goto_first (paths);
	while ((ptr = ewd_list_current (paths))) {
		if (ptr)
			free (ptr);
		
		ewd_list_next (paths);
	}

	ewd_list_destroy (paths);
}

void erss_display_default_usage ()
{
	fprintf (stderr, "Usage: %s [OPTION] ...\n", PACKAGE);
	fprintf (stderr, "Try `%s -h` for more information\n", PACKAGE);
	exit (-1);
}

int main (int argc, char * const argv[])
{
	Ecore_X_Window win;
	Evas_Object *header;
	int x, y, w, h;
	int height;
	int width;
	int c = 0;

	int got_config_file = FALSE;
	int got_theme_file = FALSE;
	int got_rc_file = FALSE;
	char config_file[PATH_MAX];
	char theme_file[PATH_MAX];

	struct stat statbuf;

	cfg = NULL;

	/*
	 * Disable this for now..
	 * xmlSetGenericErrorFunc (NULL, erss_xml_error_handler);
	 */
	
	while ((c = getopt (argc, argv, "cvhlt")) != -1) 
	{
		switch (c) {
			case 'l':
				erss_list_config_files (TRUE);
				exit (-1);
			case 'c':
				
				if(optind >= argc) 
					erss_display_default_usage ();
				
				got_config_file = TRUE;
				snprintf (config_file, PATH_MAX, "%s", (char *) argv[optind]);
				break;
			case 't':
				got_theme_file = TRUE;
				snprintf (theme_file, PATH_MAX, "%s", (char *) argv[optind]);
				break;
			case 'v':
				printf ("%s %s\n", PACKAGE, VERSION);
				exit (-1);
			case 'h':
				printf ("Usage: %s [OPTION] ...\n\n", PACKAGE);
	
				printf ("  -c CONFIG \t specify a config file (required)\n");
				printf ("  -l        \t list avaliable config files\n\n");
				printf ("  -t THEME  \t specify an edje theme file (.eet)\n");
				printf ("            \t else the default will be used.\n\n");
				printf ("  -h        \t display this help and exit\n");
				printf ("  -v        \t display %s version\n\n", PACKAGE);
				exit (-1);
			default:
				break;
		}
				
	}

	if (erss_parse_rc_file ()) 
		got_rc_file = TRUE;
		
	if(!got_config_file) {
		
		if (!got_rc_file) {
			erss_display_default_usage ();
		}
		 else 
			erss_parse_config_file (rc->config);

	} else {
		erss_parse_config_file (config_file);
	}
	
	if (!got_theme_file) {
		if (!got_rc_file) 
			cfg->theme = strdup (PACKAGE_DATA_DIR"/default.eet");
		else
			cfg->theme = strdup (rc->theme);
	}
	else
		cfg->theme = strdup (theme_file);

	stat (cfg->theme, &statbuf);
	if (!S_ISREG(statbuf.st_mode)) {
		printf ("%s error: themefile '%s' does not exist!\n", 
				PACKAGE, cfg->theme);
		exit (-1);
	}

	ecore_init ();

	if (!ecore_con_init ())
		return -1;
	ecore_app_args_set (argc, (const char **) argv);

	if (!ecore_evas_init ())
		return -1;

	width = 300;
	height = 16 * cfg->num_stories;

	if (cfg->header)
		height += 26;
	
	if (cfg->clock)
		height += 26;

	ee = ecore_evas_software_x11_new (NULL, 0, 0, 0, width, height);
	win = ecore_evas_software_x11_window_get(ee);
  ecore_x_window_prop_window_type_desktop_set(win);

	tt = malloc (sizeof (Erss_Tooltip));
	memset(tt, 0, sizeof (Erss_Tooltip));
	
	if (!ee)
		return -1;

	ecore_evas_borderless_set (ee, cfg->borderless);
	snprintf(theme_file, PATH_MAX, "erss - %s", config_file);
	ecore_evas_title_set (ee, theme_file);
	ecore_evas_shaped_set (ee, 1);
	ecore_evas_show (ee);

	if (cfg->x != 0 && cfg->y != 0)
		ecore_evas_move (ee, cfg->x, cfg->y);

	evas = ecore_evas_get (ee);
	
	evas_font_path_append (evas, PACKAGE_DATA_DIR"/fonts/");

	ecore_event_handler_add (ECORE_CON_EVENT_SERVER_ADD,
							 handler_server_add, NULL);
	ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DEL,
							 handler_server_del, NULL);
	ecore_event_handler_add (ECORE_CON_EVENT_SERVER_DATA,
							 handler_server_data, NULL);

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

	ecore_event_handler_add (ECORE_EVENT_SIGNAL_EXIT,
							 handler_signal_exit, NULL);
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

		e_container_element_append(cont, header);
	}

	if (cfg->clock) {
		tid = edje_object_add (evas);
		edje_object_file_set (tid, cfg->theme, "erss_clock");
		edje_object_part_text_set (tid, "clock", "");
		evas_object_show (tid);

		e_container_element_append(cont, tid);
	}

	erss_connect (NULL);
	ecore_timer_add (cfg->update_rate, erss_connect, NULL); 
	ecore_timer_add (1, erss_set_time, NULL);
	
	ecore_main_loop_begin ();

	ecore_evas_shutdown ();
	ecore_shutdown ();

	return 0;
}
