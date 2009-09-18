#include "e_mod_main.h"

#define MAX_ITEMS 100
#define TERM_ACTION_DIR "%s"

typedef struct _Plugin Plugin;
typedef struct _Data Data;

struct _Plugin
{
  Evry_Plugin base;

  const char *directory;
  /* all files of directory */
  Eina_List  *files;
  /* current list of files */
  Eina_List  *cur;
  Eina_Bool command;

  const char *input;
};

struct _Data
{
  Plugin *plugin;
  long id;
  int level;
  int cnt;
  Eina_List *files;
};

static Evry_Plugin *p1 = NULL;
static Evry_Plugin *p2 = NULL;
static Evry_Action *act1 = NULL;
static Evry_Action *act2 = NULL;

static long thread_cnt = 0;
static long thread_last = 0;

static const char *mime_folder;


static void
_item_fill(Evry_Item_File *file)
{
   const char *mime;

   if (file->mime) return;

   if ((mime = efreet_mime_type_get(file->uri)))
     {
	file->mime = eina_stringshare_add(mime);
	EVRY_ITEM(file)->context = eina_stringshare_ref(file->mime);
	  
	if ((!strcmp(mime, "inode/directory")) ||
	    (!strcmp(mime, "inode/mount-point")))
	  EVRY_ITEM(file)->browseable = EINA_TRUE;

	return;
     }

   file->mime = eina_stringshare_add("None");
}

static int
_cb_sort(const void *data1, const void *data2)
{
   const Evry_Item *it1 = data1;
   const Evry_Item *it2 = data2;

   if (it1->browseable && !it2->browseable)
     return -1;

   if (!it1->browseable && it2->browseable)
     return 1;

   if (it1->fuzzy_match && it2->fuzzy_match)
     if (it1->fuzzy_match - it2->fuzzy_match)
       return (it1->fuzzy_match - it2->fuzzy_match);

   return strcasecmp(it1->label, it2->label);
}

static void
_item_free(Evry_Item *it)
{
   ITEM_FILE(file, it);
   if (file->uri) eina_stringshare_del(file->uri);
   if (file->mime) eina_stringshare_del(file->mime);

   E_FREE(file);
}

static void
_scan_func(void *data)
{
   Data *d = data;
   Plugin *p = d->plugin;
   Eina_List *files;
   char *filename;
   Evry_Item_File *file;
   char buf[4096];

   files = ecore_file_ls(p->directory);

   EINA_LIST_FREE(files, filename)
     {
	if (filename[0] == '.')
	  {
	     free(filename);
	     continue;
	  }

	file = E_NEW(Evry_Item_File, 1);
	if (!file)
	  {
	     free(filename);
	     continue;
	  }

	evry_item_new(EVRY_ITEM(file), EVRY_PLUGIN(p), NULL, _item_free);

	EVRY_ITEM(file)->data = filename;

	snprintf(buf, sizeof(buf), "%s/%s", p->directory, filename);
	file->uri = strdup(buf);
	
	if (ecore_file_is_dir(file->uri))
	  EVRY_ITEM(file)->browseable = EINA_TRUE;

	d->files = eina_list_append(d->files, file);
     }
}

static int
_append_file(Plugin *p, Evry_Item_File *file)
{
   int match;

   if (p->input && (match = evry_fuzzy_match(EVRY_ITEM(file)->label, p->input)))
     {
	EVRY_ITEM(file)->fuzzy_match = match;
	EVRY_PLUGIN_ITEM_APPEND(p, file);
	return 1;
     }
   else if (!p->input)
     {
	EVRY_PLUGIN_ITEM_APPEND(p, file);
	return 1;
     }

   return 0;
}

static const char *
_item_id(const char *uri)
{
   const char *s1, *s2, *s3;
   s1 = s2 = s3 = uri;
   
   while (s1 && ++s1 && (s1 = strchr(s1, '/')))
     {
	s3 = s2;
	s2 = s1;
     }
   
   return s3;
}

static void
_scan_end_func(void *data)
{
   Data *d = data;
   Plugin *p = d->plugin;
   int cnt = 0;
   Evry_Item *item;
   char *filename, *uri;

   if (d->id != thread_last)
     {
	EINA_LIST_FREE(d->files, item)
	  {
	     filename = item->data;
	     free(filename);
	     evry_item_free(item);
	  }
	E_FREE(d);
	return;
     }

   EINA_LIST_FREE(d->files, item)
     {
	ITEM_FILE(file, item);

	filename = item->data;
	uri = (char *) file->uri;
	file->uri = eina_stringshare_add(uri);
	item->id = eina_stringshare_add(_item_id(uri));
	item->label = eina_stringshare_add(filename);
	free(filename);
	free(uri);

	p->files = eina_list_append(p->files, file);

	if (item->browseable)
	  {
	     file->mime = eina_stringshare_ref(mime_folder);
	     EVRY_ITEM(file)->context = eina_stringshare_ref(file->mime);
	  }
	
	if (p->command || cnt >= MAX_ITEMS) continue;
	cnt += _append_file(p, file);
     }

   if (!p->command)
     {
	EVRY_PLUGIN_ITEMS_SORT(p, _cb_sort);
	evry_plugin_async_update(EVRY_PLUGIN(p), EVRY_ASYNC_UPDATE_ADD);
     }

   E_FREE(d);
}

static void
_read_directory(Plugin *p)
{
   thread_last = ++thread_cnt;

   Data *d = E_NEW(Data, 1);
   d->plugin = p;
   d->id = thread_cnt;
   ecore_thread_run(_scan_func, _scan_end_func, d);
}

static Evry_Plugin *
_begin(Evry_Plugin *plugin, const Evry_Item *it)
{
   Plugin *p = NULL;

   /* is FILE ? */
   if (it && (it->plugin->type_out == plugin->type_in))
     {
	ITEM_FILE(file, it);

	if (!file->uri || !ecore_file_is_dir(file->uri))
	  return NULL;

	p = E_NEW(Plugin, 1);
	p->base = *plugin;
	p->base.items = NULL;

	p->directory = eina_stringshare_add(file->uri);
     }
   else
     {
	p = E_NEW(Plugin, 1);
	p->base = *plugin;
	p->base.items = NULL;
	p->directory = eina_stringshare_add(e_user_homedir_get());
     }

   _read_directory(p);

   return EVRY_PLUGIN(p);
}

static void
_cleanup(Evry_Plugin *plugin)
{
   PLUGIN(p, plugin);

   Evry_Item_File *file;

   /* if a thread for this plugin returns
      it will free its data if its id is smaller
      than thread_last */
   thread_last = ++thread_cnt;

   if (p->directory)
     eina_stringshare_del(p->directory);

   EINA_LIST_FREE(p->files, file)
     evry_item_free(EVRY_ITEM(file));

   EVRY_PLUGIN_ITEMS_CLEAR(p);

   E_FREE(p);
}

static void
_folder_item_add(Plugin *p, const char *path)
{
   Evry_Item_File *file = E_NEW(Evry_Item_File, 1);

   if (!file) return;

   evry_item_new(EVRY_ITEM(file), EVRY_PLUGIN(p), path, _item_free);
   file->uri = eina_stringshare_add(path);
   file->mime = eina_stringshare_ref(mime_folder);
   EVRY_ITEM(file)->browseable = EINA_TRUE;
   EVRY_PLUGIN_ITEM_APPEND(p, file);
}

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
   PLUGIN(p, plugin);
   Evry_Item_File *file;
   Eina_List *l;
   int cnt = 0;

   if (!p->command)
     EVRY_PLUGIN_ITEMS_CLEAR(p);

   p->input = input;

   /* input is command ? */
   if (input)
     {
   	if (!strncmp(input, "/", 1))
   	  {
	     if (p->command) return 1;

	     _folder_item_add(p, "/");
	     p->command = EINA_TRUE;
	     return 1;
   	  }
   	else if (!strncmp(input, "..", 2))
   	  {
   	     char *end;
   	     char dir[4096];
   	     char *tmp;
   	     int prio = 0;

   	     if (p->command) return 1;
	     if (strncmp(p->directory, "/", 1)) return 0;
   	     if (!strcmp(p->directory, "/")) return 0;
	     
   	     snprintf(dir, 4096, "%s", p->directory);
   	     end = strrchr(dir, '/');

   	     while (end != dir)
   	       {
   		  tmp = strdup(dir);
   		  snprintf(dir, (end - dir) + 1, "%s", tmp);

		  _folder_item_add(p, dir);

   		  end = strrchr(dir, '/');
   		  free(tmp);
   		  prio--;
   	       }

	     _folder_item_add(p, "/");
	     file = E_NEW(Evry_Item_File, 1);

   	     p->command = EINA_TRUE;
   	     return 1;
   	  }
     }

   if (p->command)
     {
   	p->command = EINA_FALSE;
	EVRY_PLUGIN_ITEMS_FREE(p);
     }

   EINA_LIST_FOREACH(p->files, l, file)
     {
	if (cnt >= MAX_ITEMS);
	cnt += _append_file(p, file);
     }

   if (!EVRY_PLUGIN(p)->items)
     return 0;

   EVRY_PLUGIN_ITEMS_SORT(p, _cb_sort);
   
   return 1;
}

static Evas_Object *
_icon_get(Evry_Plugin *p __UNUSED__, const Evry_Item *it, Evas *e)
{
   Evas_Object *o = NULL;
   ITEM_FILE(file, it);

   if (!file->mime)
     _item_fill(file);

   if (!file->mime) return NULL;

   if (it->browseable)
     o = evry_icon_theme_get("folder", e);
   else
     o = evry_icon_mime_get(file->mime, e);

   return o;
}


static int
_open_folder_check(Evry_Action *act __UNUSED__, const Evry_Item *it)
{
   return (it->browseable && e_action_find("fileman"));
}

static int
_open_folder_action(Evry_Action *act)
{
   E_Action *action = e_action_find("fileman");
   char *path;
   Eina_List *m;

   if (!action) return 0;

   m = e_manager_list();

   ITEM_FILE(file, act->item1);

   if (!act->item1->browseable)
     {
	path = ecore_file_dir_get(file->uri);
	if (!path) return 0;
	action->func.go(E_OBJECT(m->data), path);
	free(path);
     }
   else
     {
	action->func.go(E_OBJECT(m->data), file->uri);
     }

   return 1;
}

static int
_open_term_action(Evry_Action *act)
{
   ITEM_FILE(file, act->item1);
   Evry_Item_App *tmp;
   char cwd[4096];
   char *dir;
   int ret = 0;

   if (act->item1->browseable)
     dir = strdup(file->uri);
   else
     dir = ecore_file_dir_get(file->uri);

   if (dir)
     {
	getcwd(cwd, sizeof(cwd));
	chdir(dir);
	
	tmp = E_NEW(Evry_Item_App, 1);
	tmp->file = evry_conf->cmd_terminal;

	ret = evry_util_exec_app(EVRY_ITEM(tmp), NULL);
	E_FREE(tmp);
	E_FREE(dir);
	chdir(cwd);
     }
   
   return ret;
}

static Eina_Bool
_init(void)
{
   p1 = evry_plugin_new(NULL, "Files", type_subject, "FILE", "FILE", 0, NULL, NULL,
			_begin, _cleanup, _fetch, NULL, _icon_get,
			NULL, NULL);

   p2 = evry_plugin_new(NULL, "Files", type_object, "FILE", "FILE", 0, NULL, NULL,
			_begin, _cleanup, _fetch, NULL, _icon_get,
			NULL, NULL);
   
   evry_plugin_register(p1, 3);
   evry_plugin_register(p2, 1);

   act1 = evry_action_new("Open Folder (EFM)", "FILE", NULL, NULL, "folder-open",
			 _open_folder_action, _open_folder_check, NULL, NULL, NULL);   
   evry_action_register(act1, 0);

   act2 = evry_action_new("Open Terminal here", "FILE", NULL, NULL,
			  "system-run",
			  _open_term_action, NULL, NULL, NULL, NULL);
   evry_action_register(act2, 2);


   
   
   mime_folder = eina_stringshare_add("inode/directory");

   return EINA_TRUE;
}

static void
_shutdown(void)
{
   EVRY_PLUGIN_FREE(p1);
   EVRY_PLUGIN_FREE(p2);

   eina_stringshare_del(mime_folder);

   evry_action_free(act1);
   evry_action_free(act2);
}

EINA_MODULE_INIT(_init);
EINA_MODULE_SHUTDOWN(_shutdown);
