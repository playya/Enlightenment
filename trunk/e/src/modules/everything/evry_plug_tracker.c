#include "e.h"
#include "e_mod_main.h"

/* TODO check if trackerd is running */

typedef struct _Inst Inst;

struct _Inst
{
  E_DBus_Connection *conn;
};

static Evry_Plugin *p1;
static Evry_Plugin *p2;
static Inst *inst;
static Eina_Bool active = EINA_FALSE;

static void
_item_add(Evry_Plugin *p, char *file, char *mime, int prio)
{
   Evry_Item *it;   
   const char *filename;

   filename = ecore_file_file_get(file);

   if (!filename) return;
   
   it = evry_item_new(p, filename);
   it->priority = prio;
   it->uri = eina_stringshare_add(file);

   if (!strcmp(mime, "Folder"))
     {
	it->browseable = EINA_TRUE;
	it->mime = eina_stringshare_add("x-directory/normal");
     }
   else
     it->mime = eina_stringshare_add(mime);
   
   p->items = eina_list_append(p->items, it);
}

static void
_dbus_cb_reply(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter array, iter, item;
   Evry_Plugin *p;
   
   if (!active) return;

   p = data;
   
   if (dbus_error_is_set(error))
     {
	printf("Error: %s - %s\n", error->name, error->message);
	return;
     }

   dbus_message_iter_init(msg, &array);
   if(dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_ARRAY)
     {
	dbus_message_iter_recurse(&array, &item);
	while(dbus_message_iter_get_arg_type(&item) == DBUS_TYPE_ARRAY)
	  {
	     char *uri, *mime;
	     
	     dbus_message_iter_recurse(&item, &iter);
	     
	     if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
	       {
		  dbus_message_iter_get_basic(&iter, &uri);
		  dbus_message_iter_next(&iter);
		  /* dbus_message_iter_get_basic(&iter, &service); */
		  dbus_message_iter_next(&iter);
		  dbus_message_iter_get_basic(&iter, &mime);

		  if (uri && mime)
		    {
		       _item_add(p, uri, mime, 1); 
		    }
	       }
	     dbus_message_iter_next(&item);
	  }
     }
   
   evry_plugin_async_update(p, EVRY_ASYNC_UPDATE_ADD);
}

static void
_cleanup(Evry_Plugin *p)
{
   Evry_Item *it;
   
   EINA_LIST_FREE(p->items, it)
     {
	if (it->mime) eina_stringshare_del(it->mime);
	if (it->uri) eina_stringshare_del(it->uri);
	evry_item_free(it);
     }
   p->items = NULL;
   active = EINA_FALSE;
}

static int
_fetch(Evry_Plugin *p, const char *input)
{
   DBusMessage *msg;
   DBusMessageIter iter;
   int live_query_id = 0;
   int offset = 0;
   int max_hits = 50;
   char *service = "Files";
   char *match;
   
   _cleanup(p); 

   if (!input || (strlen(input) < 3)) return 0;
   
   active = EINA_TRUE;
   
   match = malloc(sizeof(char) * strlen(input) + 2);
   sprintf(match, "%s*", input);

   msg = dbus_message_new_method_call("org.freedesktop.Tracker",
				      "/org/freedesktop/Tracker/Search",
				      "org.freedesktop.Tracker.Search",
				      "TextDetailed");

   dbus_message_iter_init_append(msg, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32,  &live_query_id);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &service);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &match);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32,  &offset);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32,  &max_hits);
   e_dbus_message_send(inst->conn, msg, _dbus_cb_reply, -1, p);
   dbus_message_unref(msg);

   free(match);

   return 0;
}

static Evas_Object *
_item_icon_get(Evry_Plugin *p __UNUSED__, Evry_Item *it, Evas *e)
{
   char *icon_path;
   Evas_Object *o = NULL;
   
   if (it->browseable)
     {
	o = e_icon_add(e); 
	evry_icon_theme_set(o, "folder");
     }
   else
     {
	icon_path = efreet_mime_type_icon_get(it->mime, e_config->icon_theme, 64);

	if (icon_path)
	  {
	     o = e_util_icon_add(icon_path, e);
	     free(icon_path);
	  }
	else
	  {
	     o = e_icon_add(e); 
	     evry_icon_theme_set(o, "none");
	  }
     }
   return o;
}

static Eina_Bool
_init(void)
{
   E_DBus_Connection *conn = e_dbus_bus_get(DBUS_BUS_SESSION);

   if (!conn) return 0;

   p1 = E_NEW(Evry_Plugin, 1);
   p1->name = "Find Files";
   p1->type = type_subject;
   p1->type_in = "NONE";
   p1->type_out = "FILE";
   p1->async_query = 1;
   p1->fetch = &_fetch;
   p1->cleanup = &_cleanup;
   p1->icon_get = &_item_icon_get;
   evry_plugin_register(p1);
   
   p2 = E_NEW(Evry_Plugin, 1);
   p2->name = "Find Files";
   p2->type = type_object;
   p2->type_in = "NONE";
   p2->type_out = "FILE";
   p2->async_query = 1;
   p2->fetch = &_fetch;
   p2->cleanup = &_cleanup;
   p2->icon_get = &_item_icon_get;
   evry_plugin_register(p2);
   
   inst = E_NEW(Inst, 1);
   inst->conn = conn;   

   return EINA_TRUE;
}

static void
_shutdown(void)
{
   evry_plugin_unregister(p1);
   evry_plugin_unregister(p2);

   if (p1) E_FREE(p1);
   if (p2) E_FREE(p2);

   if (inst)
     {
	if (inst->conn)
	  e_dbus_connection_close(inst->conn);
	E_FREE(inst);
     }
}

EINA_MODULE_INIT(_init);
EINA_MODULE_SHUTDOWN(_shutdown);
