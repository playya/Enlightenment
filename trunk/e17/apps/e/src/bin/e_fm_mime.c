/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local types */
typedef struct _E_Fm2_Mime_Handler_Tuple E_Fm2_Mime_Handler_Tuple;
struct _E_Fm2_Mime_Handler_Tuple
{
   Evas_List *list;
   const char *str;
};

/* local subsystem functions */
static Evas_Bool _e_fm2_mime_handler_glob_match_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Bool _e_fm_mime_icon_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata);

static Evas_Hash *icon_map = NULL;
static Evas_Hash *_mime_handlers = NULL;
static Evas_Hash *_glob_handlers = NULL;

/* externally accessible functions */
EAPI const char *
e_fm_mime_filename_get(const char *fname)
{
   return efreet_mime_globs_type_get(fname);
}

/* returns:
 * NULL == don't know
 * "THUMB" == generate a thumb
 * "e/icons/fileman/mime/..." == theme icon
 * "/path/to/file....edj" = explicit icon edje file
 * "/path/to/file..." = explicit image file to use
 */
EAPI const char *
e_fm_mime_icon_get(const char *mime)
{
   char buf[4096], buf2[4096], *val;
   const char *homedir = NULL;
   Evas_List *l = NULL;
   E_Config_Mime_Icon *mi;
   
   /* 0.0 clean out hash cache once it has mroe than 512 entries in it */
   if (evas_hash_size(icon_map) > 512) e_fm_mime_icon_cache_flush();
   
   /* 0. look in mapping cache */
   val = evas_hash_find(icon_map, mime);
   if (val) return val;
   
   strncpy(buf2, mime, sizeof(buf2) - 1);
   buf2[sizeof(buf2) - 1] = 0;
   val = strchr(buf2, '/');
   if (val) *val = 0;
   
   /* 1. look up in mapping to file or thumb (thumb has flag)*/
   for (l = e_config->mime_icons; l; l = l->next)
     {
	mi = l->data;
	if (e_util_glob_match(mi->mime, mime))
	  {
	     strncpy(buf, mi->icon, sizeof(buf) - 1);
	     buf[sizeof(buf) - 1] = 0;
	     goto ok;
	  }
     }
   
   /* 2. look up in ~/.e/e/icons */
   homedir = e_user_homedir_get();

   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.edj", homedir, mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.svg", homedir, mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.png", homedir, mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.edj", homedir, buf2);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.svg", homedir, buf2);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/.e/e/icons/%s.png", homedir, buf2);
   if (ecore_file_exists(buf)) goto ok;
   
   /* 3. look up icon in theme */
   snprintf(buf, sizeof(buf), "e/icons/fileman/mime/%s", mime);
   val = (char *)e_theme_edje_file_get("base/theme/fileman", buf);
   if ((val) && (e_util_edje_collection_exists(val, buf))) goto ok;
   snprintf(buf, sizeof(buf), "e/icons/fileman/mime/%s", buf2);
   val = (char *)e_theme_edje_file_get("base/theme/fileman", buf);
   if ((val) && (e_util_edje_collection_exists(val, buf))) goto ok;
   
   /* 4. look up icon in PREFIX/share/enlightent/data/icons */
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.edj", e_prefix_data_get(), mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.svg", e_prefix_data_get(), mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.png", e_prefix_data_get(), mime);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.edj", e_prefix_data_get(), buf2);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.svg", e_prefix_data_get(), buf2);
   if (ecore_file_exists(buf)) goto ok;
   snprintf(buf, sizeof(buf), "%s/data/icons/%s.png", e_prefix_data_get(), buf2);
   if (ecore_file_exists(buf)) goto ok;
   
   return NULL;
   
   ok:
   val = (char *)evas_stringshare_add(buf);
   icon_map = evas_hash_add(icon_map, mime, val);
   return val;
}

EAPI void
e_fm_mime_icon_cache_flush(void)
{
   Evas_List *freelist = NULL;
   
   evas_hash_foreach(icon_map, _e_fm_mime_icon_foreach, &freelist);
   while (freelist)
     {
	evas_stringshare_del(freelist->data);
	freelist = evas_list_remove_list(freelist, freelist);
     }
   evas_hash_free(icon_map);
   icon_map = NULL;
}

/* create (allocate), set properties, and return a new mime handler */
EAPI E_Fm2_Mime_Handler *
e_fm2_mime_handler_new(const char *label, const char *icon_group, 
                      void (*action_func) (Evas_Object *obj, const char *path, void *data),
		      void *action_data,
		      int (test_func) (Evas_Object *obj, const char *path, void *data),
		      void *test_data)
{
   E_Fm2_Mime_Handler *handler;

   if ((!label) || (!action_func)) return NULL;

   handler = E_NEW(E_Fm2_Mime_Handler, 1);
   if (!handler) return NULL;

   handler->label = evas_stringshare_add(label);
   handler->icon_group = icon_group ? evas_stringshare_add(icon_group) : NULL;
   handler->action_func = action_func;
   handler->action_data = action_data;
   handler->test_func = test_func;
   handler->test_data = test_data;
   
   return handler;
}

EAPI void
e_fm2_mime_handler_free(E_Fm2_Mime_Handler *handler) 
{
   if (!handler) return;
   evas_stringshare_del(handler->label);
   if (handler->icon_group) evas_stringshare_del(handler->icon_group);
   E_FREE(handler);
}

/* associate a certain mime type with a handler */
EAPI Evas_Bool
e_fm2_mime_handler_mime_add(E_Fm2_Mime_Handler *handler, const char *mime)
{
   Evas_List *handlers = NULL;

   if ((!handler) || (!mime)) return 0;

   /* if there's an entry for this mime already, then append to its list */
   if ((handlers = evas_hash_find(_mime_handlers, mime)))
     {
	handlers = evas_list_append(handlers, handler);
	evas_hash_modify(_mime_handlers, mime, handlers);
     }
   else
     {
	/* no previous entry for this mime, lets add one */
	handlers = evas_list_append(handlers, handler);
	_mime_handlers = evas_hash_add(_mime_handlers, mime, handlers);
     }

   return 1;
}

/* associate a certain glob with a handler */
EAPI Evas_Bool
e_fm2_mime_handler_glob_add(E_Fm2_Mime_Handler *handler, const char *glob)
{
   Evas_List *handlers = NULL;

   if ((!handler) || (!glob)) return 0;

   /* if there's an entry for this glob already, then append to its list */
   if ((handlers = evas_hash_find(_glob_handlers, glob)))
     {
	handlers = evas_list_append(handlers, handler);
	evas_hash_modify(_glob_handlers, glob, handlers);
     }
   else
     {
	/* no previous entry for this mime, lets add one */
	handlers = evas_list_append(handlers, handler);
	_glob_handlers = evas_hash_add(_mime_handlers, glob, handlers);
     }

   return 1;
}

/* delete a certain handler for a certian mime */
EAPI void
e_fm2_mime_handler_mime_del(E_Fm2_Mime_Handler *handler, const char *mime) 
{
   Evas_List *handlers = NULL;

   if ((!handler) || (!mime)) return;

   /* if there's an entry for this mime already, then append to its list */
   if ((handlers = evas_hash_find(_mime_handlers, mime)))
     {
	handlers = evas_list_remove(handlers, handler);
	if (handlers)
	  _mime_handlers = evas_hash_modify(_mime_handlers, mime, handlers);
	else
	  _mime_handlers = evas_hash_del(_mime_handlers, mime, handlers);
     }
}

/* delete a certain handler for a certain glob */
EAPI void
e_fm2_mime_handler_glob_del(E_Fm2_Mime_Handler *handler, const char *glob) 
{
   Evas_List *handlers = NULL;

   if ((!handler) || (!glob)) return;

   /* if there's an entry for this glob already, then append to its list */
   if ((handlers = evas_hash_find(_glob_handlers, glob)))
     {
	handlers = evas_list_remove(handlers, handler);
	if (handlers)
	  _glob_handlers = evas_hash_modify(_glob_handlers, glob, handlers);
	else
	  _glob_handlers = evas_hash_del(_glob_handlers, glob, handlers);
     }
}

/* get the list of mime handlers for a mime */
EAPI Evas_List *
e_fm2_mime_handler_mime_handlers_get(const char *mime)
{
   if ((!mime) || (!_mime_handlers))
     return NULL;
   
   return evas_hash_find(_mime_handlers, mime);
}

/* get the list of glob handlers for a glob. NOTE: the list should be free()'ed */
EAPI Evas_List *
e_fm2_mime_handler_glob_handlers_get(const char *glob)
{
   E_Fm2_Mime_Handler_Tuple *tuple;
   Evas_List *handlers;

   if ((!glob) || (!_glob_handlers))
     return NULL;

   tuple = E_NEW(E_Fm2_Mime_Handler_Tuple, 1);
   tuple->list = NULL;
   tuple->str = glob;
   evas_hash_foreach(_glob_handlers, _e_fm2_mime_handler_glob_match_foreach, tuple);
   handlers = tuple->list;
   E_FREE(tuple);
   return handlers;
}

/* call a certain handler */
EAPI Evas_Bool
e_fm2_mime_handler_call(E_Fm2_Mime_Handler *handler, Evas_Object *obj, const char *path)
{
   if (!handler || !obj || !path || !handler->action_func)
     return 0;

   if (handler->test_func)
     {
	if (handler->test_func(obj, path, handler->test_data))
	  {
	     handler->action_func(obj, path, handler->action_data);
	     return 1;
	  }
	else
	  return 0;
     }

   handler->action_func(obj, path, handler->action_data);
   return 1;
}

/* call all handlers related to a certain mime */
EAPI void
e_fm2_mime_handler_mime_handlers_call_all(Evas_Object *obj, const char *path, const char *mime)
{
   Evas_List *handlers;
   Evas_List *l;

   if ((!obj) || (!path) || (!mime))
     return;

   handlers = e_fm2_mime_handler_mime_handlers_get(mime);
   if (!handlers)
     return;

   for (l = handlers; l; l = l->next)
     {
	E_Fm2_Mime_Handler *handler;

	handler = l->data;
	if (!handler) continue;
	
	e_fm2_mime_handler_call(handler, obj, path);
     }
}

/* call all handlers related to a certain glob */
EAPI void
e_fm2_mime_handler_glob_handlers_call_all(Evas_Object *obj, const char *path, const char *glob)
{
   Evas_List *handlers;
   Evas_List *l;

   if ((!obj) || (!path) || (!glob))
     return;

   handlers = e_fm2_mime_handler_glob_handlers_get(glob);
   if (!handlers)
     return;

   for (l = handlers; l; l = l->next)
     {
	E_Fm2_Mime_Handler *handler;

	handler = l->data;
	if (!handler) continue;
	
	e_fm2_mime_handler_call(handler, obj, path);
     }
}

/* local subsystem functions */
/* used to loop a glob hash and determine if the glob handler matches the filename */
static Evas_Bool _e_fm2_mime_handler_glob_match_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   E_Fm2_Mime_Handler_Tuple *tuple;

   tuple = fdata;
   if (e_util_glob_match(tuple->str, key))
     {
	Evas_List *handlers;
	Evas_List *l;

	handlers = data;
	for (l = handlers; l; l = l->next)
	  {
	     if (l->data)
	       tuple->list = evas_list_append(tuple->list, l->data);
	  }
     }
   
   return 1;
}

static Evas_Bool
_e_fm_mime_icon_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Evas_List **freelist;
   
   freelist = fdata;
   *freelist = evas_list_append(*freelist, data);
   return 1;
}
