#include "ephoto.h"

static void
_ephoto_flow_browser_delete(void *data, Evas_Object *obj, void *event_info)
{
   Ephoto *ephoto = data;
   ephoto->flow_browser = NULL;
   elm_layout_content_set
     (ephoto->layout, "ephoto.swallow.content", ephoto->thumb_browser);
   ephoto->state = EPHOTO_STATE_THUMB;
}

static void
_ephoto_flow_browser_show(Ephoto *ephoto, Ephoto_Entry *entry)
{
   /* remove thumb browser without deleting it */
   elm_layout_content_unset(ephoto->layout, "ephoto.swallow.content");

   ephoto->flow_browser = ephoto_flow_browser_add(ephoto, ephoto->layout);
   ephoto_flow_browser_image_set(ephoto->flow_browser, entry->path);
   elm_layout_content_set
     (ephoto->layout, "ephoto.swallow.content", ephoto->flow_browser);

   evas_object_smart_callback_add
     (ephoto->flow_browser, "back", _ephoto_flow_browser_delete, ephoto);
   ephoto->state = EPHOTO_STATE_FLOW;
}

static void
_ephoto_thumb_browser_view(void *data, Evas_Object *obj, void *event_info)
{
   Ephoto *ephoto = data;
   Ephoto_Entry *entry = event_info;
   _ephoto_flow_browser_show(ephoto, entry);
}

static void
_win_del(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Ephoto *ephoto = data;
   if (ephoto->timer.thumb_regen) ecore_timer_del(ephoto->timer.thumb_regen);
   free(ephoto);
}

Evas_Object *
ephoto_window_add(const char *path)
{
   Ephoto *ephoto = calloc(1, sizeof(Ephoto));
   Ethumb_Client *client = elm_thumb_ethumb_client_get();
   Evas_Object *ed;
   Evas_Coord mw, mh, iw, ih;
   const char *s;
   EINA_SAFETY_ON_NULL_RETURN_VAL(ephoto, NULL);

   ephoto->win = elm_win_add(NULL, "ephoto", ELM_WIN_BASIC);
   if (!ephoto->win)
     {
        free(ephoto);
        return NULL;
     }

   evas_object_event_callback_add
     (ephoto->win, EVAS_CALLBACK_DEL, _win_del, ephoto);

   elm_win_autodel_set(ephoto->win, EINA_TRUE);
   evas_object_show(ephoto->win);

   if (!ephoto_config_init(ephoto))
     {
        evas_object_del(ephoto->win);
        return NULL;
     }

   if ((ephoto->config->thumb_gen_size != 128) &&
       (ephoto->config->thumb_gen_size != 256) &&
       (ephoto->config->thumb_gen_size != 512))
     ephoto_thumb_size_set(ephoto, ephoto->config->thumb_size);
   else if (client)
     ethumb_client_size_set
       (client, ephoto->config->thumb_gen_size, ephoto->config->thumb_gen_size);

   ephoto->bg = elm_bg_add(ephoto->win);
   evas_object_size_hint_weight_set
     (ephoto->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ephoto->bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(ephoto->win, ephoto->bg);
   evas_object_show(ephoto->bg);

   ephoto->layout = elm_layout_add(ephoto->win);
   if (!elm_layout_file_set(ephoto->layout, THEME_FILE, "ephoto/main/layout"))
     {
        ERR("could not load group 'ephoto/main/layout' from file %s",
            THEME_FILE);
        evas_object_del(ephoto->win);
        return NULL;
     }
   evas_object_size_hint_weight_set
     (ephoto->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set
     (ephoto->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(ephoto->win, ephoto->layout);
   evas_object_show(ephoto->layout);

   ephoto->thumb_browser = ephoto_thumb_browser_add(ephoto, ephoto->layout);
   if (!ephoto->thumb_browser)
     {
        ERR("could not add thumb browser");
        evas_object_del(ephoto->win);
        return NULL;
     }
   elm_layout_content_set
     (ephoto->layout, "ephoto.swallow.content", ephoto->thumb_browser);

   ed = elm_layout_edje_get(ephoto->layout);
   edje_object_size_min_get(ed, &mw, &mh);
   edje_object_size_min_restricted_calc(ed, &mw, &mh, mw, mh);
   if (mw < 1) mw = 320;
   if (mh < 1) mh = 240;

   s = edje_object_data_get(ed, "initial_size");
   if ((!s) || (sscanf(s, "%d %d", &iw, &ih) != 2)) iw = ih = 0;
   if (iw < mw) iw = mw;
   if (ih < mh) ih = mh;
   evas_object_resize(ephoto->win, iw, ih);

   if (!path)
     ephoto_thumb_browser_directory_set
       (ephoto->thumb_browser, ephoto->config->directory);
   else if (ecore_file_is_dir(path))
     ephoto_thumb_browser_directory_set(ephoto->thumb_browser, path);
   else
     ERR("loading file to be done");

   return ephoto->win;
}

#if 0
        char current_directory[PATH_MAX];
        Ethumb_Client *client = elm_thumb_ethumb_client_get();

	em = calloc(1, sizeof(Ephoto));
        if (!ephoto_config_init(em))
          {
             _ephoto_delete_main_window(NULL, NULL, NULL);
             return;
          }

        if ((em->config->thumb_gen_size != 128) &&
            (em->config->thumb_gen_size != 256) &&
            (em->config->thumb_gen_size != 512))
          ephoto_thumb_size_set(em->config->thumb_size);
        else
          ethumb_client_size_set
            (client, em->config->thumb_gen_size, em->config->thumb_gen_size);

	/*Setup the main window*/
	em->win = elm_win_add(NULL, "ephoto", ELM_WIN_BASIC);
	elm_win_title_set(em->win, "Ephoto");

	evas_object_show(em->win);

	/*Callback to close the main window*/
	evas_object_smart_callback_add(em->win, "delete,request", 
					_ephoto_delete_main_window, NULL);

	/*Grab the main evas canvas we are working with*/
	em->e = evas_object_evas_get(em->win);

	/*Set a custom background for the window*/
	em->bg = elm_bg_add(em->win);
	elm_win_resize_object_add(em->win, em->bg);
	evas_object_size_hint_weight_set(em->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(em->bg);

	/* Add the main layout to the window */
	em->layout = elm_layout_add(em->win);


        /* Prepare the slideshow beforehand, in order
         * to obtain the list of transitions */
	ephoto_create_slideshow();

        if (!directory)
        {
                getcwd(current_directory, PATH_MAX);
        }

        if (!em->config->directory)
        {
                if (directory)
                        em->config->directory = eina_stringshare_add(directory);
                else
                        em->config->directory = eina_stringshare_add(current_directory);
        }
        else if (directory || !em->config->remember_directory)
        {
                if (directory)
                        eina_stringshare_replace(&em->config->directory,
                                                 directory);
                else
                        eina_stringshare_replace(&em->config->directory,
                                                 current_directory);
        }

	if (image)
	{
                em->flow_browser = ephoto_create_flow_browser(em->layout);
		ephoto_flow_browser_image_set(em->flow_browser, image);
		eina_stringshare_del(image);	
		elm_layout_content_set(em->layout, "ephoto.content.swallow", em->flow_browser);
		evas_object_smart_callback_add(em->flow_browser, 
					       "delete,request",
					       _ephoto_flow_browser_delete_cb,
					       NULL);
		em->state = EPHOTO_STATE_FLOW;
	}
	else
        {
                em->thumb_browser =  ephoto_thumb_browser_add(em->layout);
		elm_layout_content_set(em->layout, "ephoto.content.swallow", em->thumb_browser);
		evas_object_smart_callback_add(em->thumb_browser, 
					       "selected",
					       _ephoto_thumb_browser_selected_cb,
					       NULL);
		em->state = EPHOTO_STATE_THUMB;
	}

	evas_object_resize(em->win, 1024, 534);
}

/*Delete the main ephoto window*/
static void
_ephoto_delete_main_window(void *data, Evas_Object *obj, void *event_info)
{
        ephoto_config_save(em, EINA_TRUE);

        if (em->thumb_browser)
                evas_object_del(em->thumb_browser);
        if (em->flow_browser)
	        evas_object_del(em->flow_browser);

	evas_object_del(em->layout);
	evas_object_del(em->bg);
        ephoto_entries_free(em);
        ephoto_config_free(em);
	free(em);
	elm_exit();
}
#endif

static Eina_Bool
_thumb_gen_size_changed_timer_cb(void *data)
{
   Ephoto *ephoto = data;
   Ethumb_Client *client;
   const Eina_List *l;
   Evas_Object *o;

   if (ephoto->config->thumb_gen_size == ephoto->thumb_gen_size) goto end;

   INF("thumbnail generation size changed from %d to %d",
       ephoto->config->thumb_gen_size, ephoto->thumb_gen_size);

   client = elm_thumb_ethumb_client_get();
   if (!client)
     {
        DBG("no client yet, try again later");
        return EINA_TRUE;
     }

   ephoto->config->thumb_gen_size = ephoto->thumb_gen_size;
   ethumb_client_size_set
     (client, ephoto->thumb_gen_size, ephoto->thumb_gen_size);

   EINA_LIST_FOREACH(ephoto->thumbs, l, o)
     {
        Ethumb_Thumb_Format format;
        format = (long)evas_object_data_get(o, "ephoto_format");
        ethumb_client_format_set(client, format);
        elm_thumb_reload(o);
     }

 end:
   ephoto->timer.thumb_regen = NULL;
   return EINA_FALSE;
}

void
ephoto_thumb_size_set(Ephoto *ephoto, int size)
{
   if (ephoto->config->thumb_size != size)
     {
        INF("thumbnail display size changed from %d to %d",
            ephoto->config->thumb_size, size);
        ephoto->config->thumb_size = size;
        ephoto_config_save(ephoto, EINA_FALSE);
     }

   if (size <= 128)      ephoto->thumb_gen_size = 128;
   else if (size <= 256) ephoto->thumb_gen_size = 256;
   else                  ephoto->thumb_gen_size = 512;

   if (ephoto->timer.thumb_regen) ecore_timer_del(ephoto->timer.thumb_regen);
   ephoto->timer.thumb_regen = ecore_timer_add
     (0.1, _thumb_gen_size_changed_timer_cb, ephoto);
}

static void
_thumb_del(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Ephoto *ephoto = data;
   ephoto->thumbs = eina_list_remove(ephoto->thumbs, o);
}

Evas_Object *
ephoto_thumb_add(Ephoto *ephoto, Evas_Object *parent, const char *path)
{
   Evas_Object *o;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   o = elm_thumb_add(parent);
   if (!o) return NULL;

   if (path) ephoto_thumb_path_set(o, path);
   elm_object_style_set(o, "noframe");
   ephoto->thumbs = eina_list_append(ephoto->thumbs, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _thumb_del, ephoto);
   return o;
}

void
ephoto_thumb_path_set(Evas_Object *o, const char *path)
{
   Ethumb_Thumb_Format format = ETHUMB_THUMB_FDO;
   const char *ext = strrchr(path, '.');
   if (ext)
     {
        ext++;
        if ((strcasecmp(ext, "jpg") == 0) ||
            (strcasecmp(ext, "jpeg") == 0))
          format = ETHUMB_THUMB_JPEG; /* faster! */
     }

   ethumb_client_format_set(elm_thumb_ethumb_client_get(), format);
   evas_object_data_set(o, "ephoto_format", (void*)(long)format);
   elm_thumb_file_set(o, path, NULL);
}

Ephoto_Entry *
ephoto_entry_new(Ephoto *ephoto, const char *path, const char *label)
{
   Ephoto_Entry *entry;
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   entry = calloc(1, sizeof(Ephoto_Entry));
   EINA_SAFETY_ON_NULL_RETURN_VAL(entry, NULL);
   entry->ephoto = ephoto;
   entry->path = eina_stringshare_add(path);
   entry->basename = ecore_file_file_get(entry->path);
   entry->label = eina_stringshare_add(label);
   return entry;
}

void
ephoto_entry_free(Ephoto_Entry *entry)
{
   const char *s;
   EINA_SAFETY_ON_NULL_RETURN(entry);
   eina_stringshare_del(entry->path);
   eina_stringshare_del(entry->label);
   EINA_LIST_FREE(entry->dir_files, s) eina_stringshare_del(s);
   free(entry);
}

void
ephoto_entries_free(Ephoto *ephoto)
{
   Ephoto_Entry *entry;
   EINA_LIST_FREE(ephoto->entries, entry) ephoto_entry_free(entry);
}
