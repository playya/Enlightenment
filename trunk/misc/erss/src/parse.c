#include "erss.h"
#include "parse.h"

Ewd_List *list = NULL;
Article *item = NULL;
Config *cfg = NULL;


char *get_element (char **buffer, char *type)
{
	char tmp_char;
	char *c;
	char *start_tmp;
	char *end_tmp;
	char *ret_val = NULL;
	int size;

	if (!buffer || !*buffer || !type)
		goto err_clean_none;

	size = strlen (type) + 4;
	c = malloc (size);
	if (!c)
		goto err_clean_none;

	snprintf (c, size, "<%s>", type);
	start_tmp = strstr (*buffer, c);
	if (!start_tmp)
		goto err_clean_c;

	start_tmp += size - 2;

	snprintf (c, size, "</%s>", type);
	end_tmp = strstr (start_tmp, c);
	if (!end_tmp)
		goto err_clean_c;

	tmp_char = *end_tmp;
	*end_tmp = '\0';
	ret_val = strdup (start_tmp);
	*end_tmp = tmp_char;
	if (!ret_val)
		goto err_clean_c;

	*buffer = end_tmp + size - 1;

  err_clean_c:
	free (c);
  err_clean_none:
	return ret_val;
}

int get_new_story (char *buffer)
{
	char c[1024];
	char *ptr;

	/*
	 * First check for <item> cases
	 */
	snprintf (c, sizeof (c), "<%s>", cfg->item_start);

	ptr = strstr (buffer, c);

	if (ptr)
		return TRUE;

	/* 
	 * Second check for <item ....> cases.
	 */
	snprintf (c, sizeof (c), "<%s ", cfg->item_start);
	ptr = strstr (buffer, c);

	if (ptr)
		return TRUE;

	/* 
	 * The item didn't match.
	 */
	return FALSE;
}

int get_end_story (char *buffer)
{
	char c[1024];
	char *ptr;

	snprintf (c, sizeof (c), "</%s>", cfg->item_start);

	ptr = strstr (buffer, c);

	if (ptr)
		return TRUE;
	else
		return FALSE;
}

char *remove_garbage (char *c, char *garbage)
{
	char *str;

	if (!garbage)
		return c;

	str = strstr (c, garbage);

	if (str)
		strcpy (str, str + strlen (garbage));
	
	return c;
}

void parse_data (char *buf)
{
	char *c;
	char *text;
	int i;

	/* 
	 * Some of the feeds may have "garbage" in their titles
	 * we want to remove it.
	 */
	char *garbage [2] = {
		"<![CDATA[",
		"]]>"
	};

	/*
	printf ("data: %s\n", buf);
	*/

	if (ewd_list_nodes (list) >= cfg->num_stories)
		return;

	if (get_new_story (buf))
	{
		/* 
		 * We have a new story, allocate an item for it
		 */
		item = malloc (sizeof (Article *));
		item->description = NULL;
		item->url = NULL;

		return;
	}

	if (get_end_story (buf))
	{
		/*
		if (item->description)
			printf ("[%s]\n", item->description);
		*/
		
		ewd_list_append (list, item);

		return;
	}

	/* If the item is not allocated then we dont have a 
	 * real story. So return with an error
	 */
	if (!item)
		return;

	if ((c = get_element (&buf, cfg->item_title)) != NULL)
	{

		for (i = 0; i < 2; i++)
			c = remove_garbage (c, garbage[i]);

		c = ecore_txt_convert ("iso-8859-1", "utf8", c);

		text = malloc (1024);

		if (cfg->prefix)
			snprintf (text, strlen (c) + strlen (cfg->prefix), " %s %s", 
					cfg->prefix, c);
		else
			snprintf (text, strlen (c) + 5, " . %s", c);

		item->obj = edje_object_add (evas);
		edje_object_file_set (item->obj, 
				PACKAGE_DATA_DIR"/default.eet", "erss_item");

		if (text)
			edje_object_part_text_set (item->obj, "article", text);

		evas_object_show (item->obj);

		evas_object_event_callback_add (item->obj,
				EVAS_CALLBACK_MOUSE_IN, cb_mouse_in, NULL);
		evas_object_event_callback_add (item->obj,
				EVAS_CALLBACK_MOUSE_OUT, cb_mouse_out, NULL);

		e_container_element_append(cont, item->obj);

		free (c);
		free (text);

		return; 
	}

	if ((c = get_element (&buf, cfg->item_url)) != NULL)
	{
		item->url = strdup (c);

		edje_object_signal_callback_add (item->obj, "exec*", "*", 
				cb_mouse_out_item, item);
		edje_object_signal_emit (item->obj, "mouse,in", "article");
		edje_object_signal_emit (item->obj, "mouse,out", "article");

		free (c);
		return;
	}

	if ((c = get_element (&buf, cfg->item_description)) != NULL)
	{
		item->description = strdup (c);
		
		free (c);
		return;
	}

	
}

char *get_next_line (FILE * fp)
{
	int index = 0;
	int bufsize = 512;
	signed char temp;
	char *buf = NULL;

	buf = malloc (bufsize);

	while ((temp = fgetc (fp)) != '\n')
	{
		if (feof (fp))
			return NULL;

		if (index == bufsize)
		{
			bufsize += 512;
			buf = realloc (buf, bufsize);
		}

		buf[index++] = temp;
	}

	if (index == bufsize)
		buf = realloc (buf, bufsize);

	buf[index] = '\0';
	buf = realloc (buf, strlen (buf) ? strlen (buf) : 1);

	return buf;
}

void parse_config_file (char *file)
{
	FILE *fp;
	char *line;
	char *c;

	if ((fp = fopen (file, "r")) == NULL)
	{
		fprintf (stderr, "Erss error: Can't open config file\n");
		exit (-1);
	}

	cfg = malloc (sizeof (Config));

	while ((line = get_next_line (fp)) != NULL)
	{
		if ((c = get_element (&line, "header")) != NULL)
		{
			cfg->header = strdup (c);
			continue;
		}
		
		if ((c = get_element (&line, "hostname")) != NULL)
		{
			cfg->hostname = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "url")) != NULL)
		{
			cfg->url = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "item_start")) != NULL)
		{
			cfg->item_start = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "item_title")) != NULL) 
		{
			cfg->item_title = strdup (c);
			continue;
		}	

		if ((c = get_element (&line, "item_url")) != NULL)
		{
			cfg->item_url = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "item_description")) != NULL)
		{
			cfg->item_description = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "prefix")) != NULL)
		{
			cfg->prefix = strdup (c);
			continue;
		}


		if ((c = get_element (&line, "update_rate")) != NULL)
		{
			cfg->update_rate = atoi (c);
			continue;
		}

		if ((c = get_element (&line, "clock")) != NULL)
		{
			cfg->clock = atoi (c);

			if (cfg->clock != 1 && cfg->clock != 0)
			{
				fprintf (stderr,
						"ERROR: Clock option has wrong value - check your config file!\n");
				exit (-1);
			}
			continue;
		}

		if ((c = get_element (&line, "stories")) != NULL)
		{
			cfg->num_stories = atoi (c);

			if (cfg->num_stories > 10)
			{
				fprintf (stderr,
						 "ERROR: Max stories to show is 10 - check your config file!\n");
				exit (-1);
			}
			continue;
		}

		if ((c = get_element (&line, "browser")) != NULL)
		{
			cfg->browser = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "borderless")) != NULL)
		{
			cfg->borderless = atoi (c);

			if (cfg->borderless != 1 && cfg->borderless != 0)
			{
				fprintf (stderr,
						 "ERROR: Borderless option has wrong value - check your config file!\n");
				exit (-1);
			}
			continue;
		}

		if ((c = get_element (&line, "x")) != NULL)
		{
			cfg->x = atoi (c);
			continue;
		}

		if ((c = get_element (&line, "y")) != NULL)
		{
			cfg->y = atoi (c);
			continue;
		}

		if ((c = get_element (&line, "proxy")) != NULL)
		{
			cfg->proxy = strdup (c);
			continue;
		}

		if ((c = get_element (&line, "proxy_port")) != NULL)
		{
			cfg->proxy_port = atoi (c);
		}
	}

	free (line);

	fclose (fp);

}
