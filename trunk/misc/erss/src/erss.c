#include "erss.h"
#include "parse.h"

int erss_connect (void *data);
char *time_format ();
int set_time (void *data);

Evas *evas;
Ecore_Evas *ee;
Ecore_Con_Server *server;
Evas_Object *bg;
Evas_Object *cont;
Evas_Object *tid;

char *main_buffer;
char *last_time;

int bufsize = 0;


int erss_connect (void *data)
{
	
	if (!strcasecmp (cfg->proxy, "")) {
		server = ecore_con_server_connect (ECORE_CON_REMOTE_SYSTEM,
										   cfg->hostname, 80, NULL);
	} else {
		if (!cfg->proxy_port)
		{
			fprintf (stderr, "ERROR: You need to define a proxy port!\n");
			exit (-1);
		}
		server = ecore_con_server_connect (ECORE_CON_REMOTE_SYSTEM,
										   cfg->proxy, cfg->proxy_port, NULL);
	}
	
	if (!server) {
		fprintf (stderr, "ERROR: Could not connect to server ..\n");
		exit (-1);
	}

	if (last_time)
		free (last_time);
	
	last_time = strdup (time_format ());
	set_time (NULL);
	

	return TRUE;
}

char *time_format () 
{
	char    *str;
	struct  tm  *ts;
	time_t  curtime;
	
	curtime = time(NULL);
	ts = localtime(&curtime);

	str = malloc (50);
	
	if (ts->tm_hour < 10 && ts->tm_min < 10 && ts->tm_sec < 10)
		snprintf (str, 20, "0%d:0%d:0%d", 
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_hour < 10 && ts->tm_min < 10)
		snprintf (str, 20, "0%d:0%d:%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_hour < 10 && ts->tm_sec < 10)
		snprintf (str, 20, "0%d:%d:0%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_min < 10 && ts->tm_sec < 10)
		snprintf (str, 20, "%d:0%d:0%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_hour < 10)
		snprintf (str, 20, "0%d:%d:%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_min < 10)
		snprintf (str, 20, "%d:0%d:%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);
	else if (ts->tm_sec < 10)
		 snprintf (str, 20, "%d:%d:0%d",
				 ts->tm_hour, ts->tm_min, ts->tm_sec);
	else
		snprintf (str, 20, "%d:%d:%d",
				ts->tm_hour, ts->tm_min, ts->tm_sec);

	return str;
}

int set_time (void *data) {
	char *str;
	char text[100];

	str = time_format ();
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
		printf ("exit: interrupt\n");
	if (e->quit)
		printf ("exit: quit\n");
	if (e->terminate)
		printf ("exit: terminate\n");

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
			e_container_element_destroy (cont, ptr->obj);

			if (ptr->url)
				free (ptr->url);

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
	snprintf (c, sizeof (c), "GET %s HTTP/1.0\r\n", cfg->url);
	ecore_con_server_send (server, c, strlen (c));
	snprintf (c, sizeof (c), "Host: %s \r\n\r\n", cfg->hostname);
	ecore_con_server_send (server, c, strlen (c));

	return 1;
}

int handler_server_data (void *data, int type, void *event)
{
	Ecore_Con_Event_Server_Data *e = event;

	/* 
	 * Read everything we recive into one big buffer, and handle
	 * that buffer when the server disconnects.
	 */
	main_buffer = realloc (main_buffer, bufsize + e->size);
	memcpy (main_buffer + bufsize, e->data, e->size);
	bufsize += e->size;

	return 1;
}

int handler_server_del (void *data, int type, void *event)
{
	Ecore_Con_Event_Server_Del *e = event;
	char *leader;

	/*
	 * Now split our main buffer in each newline and then parse 
	 * the line.
	 */

	while (main_buffer != NULL)
	{
		char temp;

		leader = strchr (main_buffer, '\n');
		if (leader)
		{
			temp = *leader;
			*leader = '\0';
			parse_data (main_buffer);
			*leader = temp;
			main_buffer = leader + 1;
		} else
		{
			main_buffer = leader;
		}
	}

	ecore_con_server_del (e->server);
	server = NULL;
	
	if (!list) {
		printf ("Erss: error parsing data\n");
		printf ("------------------------\n");
		printf ("%s\n", main_buffer);
	}

	if (main_buffer)
		free (main_buffer);

	bufsize = 0;

	return 1;
}

void window_move_cb (Ecore_Evas * ee)
{
	int x, y, w, h;
	Evas_Object *o = NULL;
	
	ecore_evas_geometry_get (ee, &x, &y, &w, &h);

	if((o = evas_object_name_find(ecore_evas_get(ee), "root_background")))
			esmart_trans_x11_freshen(o, x, y, w, h);
}

void window_resize_cb(Ecore_Evas *ee)
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

	snprintf (c, sizeof (c), "%s %s", cfg->browser, item->url);
	ecore_exe_run (c, NULL);
}

void cb_mouse_in (void *data, Evas *e, Evas_Object *obj, 
		void *event_info) 
{
	printf ("In\n");
	edje_object_signal_emit (obj, "over", "header");
}

void cb_mouse_out (void *data, Evas *e, Evas_Object *obj, 
		void *event_info) 
{
	printf ("out\n");
	edje_object_signal_emit (obj, "default", "header");
} 




int main (int argc, const char **argv)
{
	Evas_Object *header;
	int x, y, w, h;
	int height;
	int width;


	if (argc < 2) {
		fprintf (stderr, "Usage: erss feedconfig.cfg\n");
		fprintf (stderr, "Example config files in the config/ folder.\n");
		exit (-1);
	}

	parse_config_file ((char *) argv[1]);

	ecore_init ();

	if (!ecore_con_init ())
		return -1;
	ecore_app_args_set (argc, argv);

	if (!ecore_evas_init ())
		return -1;


	width = 300;
	height = 15 * cfg->num_stories;

	if (cfg->header)
		height += 25;
	
	if (cfg->clock)
		height += 25;

	ee = ecore_evas_software_x11_new (NULL, 0, 0, 0, width, height);

	if (!ee)
		return -1;

	ecore_evas_borderless_set (ee, cfg->borderless);
	ecore_evas_title_set (ee, "erss");
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
	ecore_evas_callback_move_set (ee, window_move_cb);
	ecore_evas_callback_resize_set(ee, window_resize_cb);

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
		edje_object_file_set (header, PACKAGE_DATA_DIR"/default.eet", "erss");
		edje_object_part_text_set (header, "header", cfg->header);
		evas_object_show (header);

		e_container_element_append(cont, header);
	}

	if (cfg->clock) {
		tid = edje_object_add (evas);
		edje_object_file_set (tid, PACKAGE_DATA_DIR"/default.eet", "erss_clock");
		edje_object_part_text_set (tid, "clock", "");
		evas_object_show (tid);

		e_container_element_append(cont, tid);
	}

	if (!cfg->hostname) 
	{		
		fprintf (stderr, "ERROR: No hostname defined!\n");
		exit (-1);
	}

	if (!cfg->url)
	{
		fprintf (stderr, "ERROR: No url defined!\n");
		exit (-1);
	}
	
	erss_connect (NULL);
	ecore_timer_add (cfg->update_rate, erss_connect, NULL); 
	ecore_timer_add (1, set_time, NULL);
	
	ecore_main_loop_begin ();

	ecore_evas_shutdown ();
	ecore_shutdown ();

	return 0;
}
