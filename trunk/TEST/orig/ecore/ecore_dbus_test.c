/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_dbus_test.h"

static int ecore_dbus_event_server_add(void *udata, int ev_type, void *ev);
static int ecore_dbus_event_server_del(void *udata, int ev_type, void *ev);

static void ecore_dbus_method_list_names_cb(void *data, Ecore_DBus_Method_Return *reply);
static void ecore_dbus_method_test_cb(void *data, Ecore_DBus_Method_Return *reply);
static void ecore_dbus_method_error_cb(void *data, const char *error);

static const char *event_type_get(Ecore_DBus_Message_Type type);
static void _test_type_length();

static Ecore_DBus_Server *svr = NULL;

int
main(int argc, char **argv)
{
   ecore_dbus_init();

   _test_type_length();
   svr = ecore_dbus_server_session_connect(NULL);
   if (!svr)
     {
	printf("Couldn't connect to dbus session server!\n");
     }
   else
     {
	int i = 0;
	Ecore_Event_Handler *handler[3];

	printf("Connected!\n");

	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_ADD,
					       ecore_dbus_event_server_add, NULL);
	handler[i++] = ecore_event_handler_add(ECORE_DBUS_EVENT_SERVER_DEL,
					       ecore_dbus_event_server_del, NULL);

	ecore_main_loop_begin();

	for (i = 0; i < 2; i++)
	  ecore_event_handler_del(handler[i]);

	if (svr) ecore_dbus_server_del(svr);
     }
   ecore_dbus_shutdown();
   return 0;
}

static int
ecore_dbus_event_server_add(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Add *event;
   Ecore_List *ids;
   int i;

   event = ev;
   printf("ecore_dbus_event_server_add\n");
   ecore_dbus_method_list_names(event->server,
				ecore_dbus_method_list_names_cb,
				ecore_dbus_method_error_cb, NULL);

   ids = ecore_list_new();
   ecore_list_free_cb_set(ids, free);
   for(i = 0; i < 5; i++)
     {
 	unsigned int *id;
	id = malloc(sizeof(int));
	*id = i * 2;
        ecore_list_append(ids, id); 
     }
   ecore_dbus_message_new_method_call(event->server,
				      "/org/enlightenment/test" /*path*/,
				      "org.enlightenment.Test" /*interface*/,
				      "Test" /*method*/,
				      "org.enlightenment.Test" /*destination*/,
				      ecore_dbus_method_test_cb,
				      ecore_dbus_method_error_cb, NULL,
				      "usaus" /*fmt*/,
				      5, "hello", ids, "goodbye");

   ecore_list_destroy(ids);
   return 0;
}

static int
ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   printf("ecore_dbus_event_server_del\n");
   svr = NULL;
   ecore_main_loop_quit();
   return 0;
}

static void
ecore_dbus_method_test_cb(void *data,
			  Ecore_DBus_Method_Return *reply)
{
   printf("test reply cb\n");
}

static void
ecore_dbus_method_list_names_cb(void *data,
				Ecore_DBus_Method_Return *reply)
{
   Ecore_List *names;

   printf("ecore_dbus_event_server_data %s %s.%s\n", event_type_get(reply->type),
						     reply->header.interface,
						     reply->header.member);

   names = reply->args[0].value;
   printf("Got names %c\n", reply->args[0].type);
   if (names)
     {
	char *name;
	ecore_list_first_goto(names);
	while ((name = ecore_list_next(names)))
	  {
	     printf("Name: %s\n", name);
	  }
	ecore_list_destroy(names);
     }
   ecore_main_loop_quit();
}

static void
ecore_dbus_method_error_cb(void *data, const char *error)
{
   printf("Error: %s\n", error);
   ecore_main_loop_quit();
}

static const char *
event_type_get(Ecore_DBus_Message_Type type)
{
   switch (type)
     {
      case ECORE_DBUS_MESSAGE_TYPE_INVALID:
	 return "ECORE_DBUS_MESSAGE_TYPE_INVALID";
      case ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL:
	 return "ECORE_DBUS_MESSAGE_TYPE_CALL";
      case ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN:
	 return "ECORE_DBUS_MESSAGE_TYPE_RETURN";
      case ECORE_DBUS_MESSAGE_TYPE_ERROR:
	 return "ECORE_DBUS_MESSAGE_TYPE_ERROR";
      case ECORE_DBUS_MESSAGE_TYPE_SIGNAL:
	 return "ECORE_DBUS_MESSAGE_TYPE_SIGNAL";
     }
   return "UNKNOWN";
}

//int _ecore_dbus_complete_type_length(const char *);
static void
_test_type_length()
{
#define _NUM_TYPES 4
   struct { char *type; int len; } types[_NUM_TYPES] = {
	  { "us", 1 },
	  { "ads", 2 },
	  { "a(a(ai))su", 8 },
	  { "a{ss}u", 5 }
   };

   int i;

   printf("Test type length\n---------------\n");
   for (i = 0; i < _NUM_TYPES; i++)
     {
	int len = _ecore_dbus_complete_type_length_get(types[i].type);
	printf("\"%s\" => %d (expected %d) %s\n", types[i].type, len, types[i].len, len == types[i].len ? "PASS" : "FAIL");
     }
   printf("---------------\n");
}
