/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

/********************************************************************************/
/* About									*/
/********************************************************************************/
/* Author: Jorge Luis Zapata							*/
/* Author: Brian Mattern (rephorm)						*/
/* Author: Sebastian Dransfeld							*/
/* Version: 0.3.0								*/
/********************************************************************************/
/* Todo										*/
/********************************************************************************/
/* free allocated data								*/
/* make other authentication mechanisms besides external			*/
/********************************************************************************/
/* Changelog									*/
/********************************************************************************/
/* 0.0 	usable interface							*/
/* 0.1 	change dbus spec version (0.11):					*/
/* 	different header format							*/
/* 	new type signature							*/
/*	header length = 8 byte multiple 					*/
/*	paddings value must be null						*/
/*	body need not to end in a 8 byte boundary				*/
/*	new data type: variant,signature,dict					*/
/*	ecore_oldlist cant join two lists so is difficult to handle compound	*/
/*	data types (variant,struct,dict,array) in a stack way			*/
/* 0.2 	change again the spec version (0.8)					*/
/*	i realized that the first version was correct, when i read the spec	*/
/*	for ecore_dbus 0.1 i was reading a previous version :(			*/
/*	put again the data type byte in each marshaled data			*/
/*										*/
/* 29-03-05									*/
/* 0.2.1 some segfault fixes, new tests						*/
/* 0.3.0 add ability to send signals, receive method class and respond to them  */
/*       add address parsing and functions to connect to standard busses        */
/*       change API of ecore_dbus_message_new_method_call()			*/

/* global variables  */

EAPI int                 ECORE_DBUS_EVENT_SERVER_ADD = 0;
EAPI int                 ECORE_DBUS_EVENT_SERVER_DEL = 0;
EAPI int                 ECORE_DBUS_EVENT_SIGNAL = 0;
EAPI int                 ECORE_DBUS_EVENT_METHOD_CALL = 0;

/* private function declaration */

/* helper functions */
static char        *_ecore_dbus_getuid(void);
static char        *_ecore_dbus_hex_encode(char *src_str);
/* auth functions */
unsigned char      *_ecore_dbus_auth_external(void *data);
/* con functions */
static int          _ecore_dbus_event_server_add(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_del(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_data(void *data, int ev_type, void *ev);
static void         _ecore_dbus_event_server_del_free(void *data, void *ev);
static void         _ecore_dbus_event_server_data_free(void *data, void *ev);

static Ecore_DBus_Event_Server_Data *_ecore_dbus_event_create(Ecore_DBus_Server *svr, Ecore_DBus_Message *msg);

static void         _ecore_dbus_method_hello_cb(void *data, Ecore_DBus_Method_Return *reply);
static void         _ecore_dbus_method_error_cb(void *data, const char *error);

/* local variables  */

static const Ecore_DBus_Auth auths[] = {
   {"EXTERNAL", 1, {_ecore_dbus_auth_external, NULL, NULL, NULL, NULL}},
   {"MAGIC_COOKIE", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"DBUS_COOKIE_SHA1", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"KERBEROS_V4", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"SKEY", 0, {NULL, NULL, NULL, NULL, NULL}},
};

static int                  init_count = 0;
static Ecore_List2         *servers = NULL;
static Ecore_Event_Handler *handler[3];

int words_bigendian = -1;

/* public functions */
EAPI int
ecore_dbus_init(void)
{
   int i = 0;

   if (++init_count != 1) return init_count;

   if (words_bigendian == -1)
     {
	unsigned long int v;

	v = htonl(0x12345678);
	if (v == 0x12345678) words_bigendian = 1;
	else words_bigendian = 0;
     }

   ecore_con_init();

   ECORE_DBUS_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_DBUS_EVENT_SIGNAL = ecore_event_type_new();
   ECORE_DBUS_EVENT_METHOD_CALL = ecore_event_type_new();

   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
					  _ecore_dbus_event_server_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
					  _ecore_dbus_event_server_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
					  _ecore_dbus_event_server_data, NULL);

   ecore_dbus_objects_init();
   return init_count;
}

EAPI int
ecore_dbus_shutdown(void)
{
   int i = 0;

   if (--init_count != 0) return init_count;

   ecore_dbus_objects_shutdown();
   /* FIXME: Delete servers */

   for (i = 0; i < 3; i++)
     ecore_event_handler_del(handler[i]);

   ecore_con_shutdown();

   return init_count;
}

/**
 * Connect to the system bus.
 */
EAPI Ecore_DBus_Server *
ecore_dbus_server_system_connect(const void *data)
{
  Ecore_List *addrs;
  Ecore_DBus_Server *svr;
  char *bus_env;

  /* get the system bus address from the environment */
  bus_env = getenv("DBUS_SYSTEM_BUS_ADDRESS");
  if (bus_env)
  {
    addrs = ecore_dbus_address_parse(bus_env);
    if (addrs)
      {
	 svr = ecore_dbus_address_list_connect(addrs, data);
	 ecore_list_destroy(addrs);
	 if (svr) return svr;
      }
  }

  /* if we haven't returned already, try the default location */
  return ecore_dbus_server_connect(ECORE_CON_LOCAL_SYSTEM, "/var/run/dbus/system_bus_socket", -1, data);
}

/**
 * Connect to the session bus.
 */
EAPI Ecore_DBus_Server *
ecore_dbus_server_session_connect(const void *data)
{
  Ecore_List *addrs;
  Ecore_DBus_Server *svr;
  char *bus_env;

  /* get the session bus address from the environment */
  bus_env = getenv("DBUS_SESSION_BUS_ADDRESS");
  if (bus_env)
  {
    addrs = ecore_dbus_address_parse(bus_env);
    if (addrs)
      {
	 svr = ecore_dbus_address_list_connect(addrs, data);
	 ecore_list_destroy(addrs);
	 if (svr) return svr;
      }
  }

  /*
   * XXX try getting address from _DBUS_SESSION_BUS_ADDRESS property (STRING) set
   * on the root window 
   */

  return NULL;
}

EAPI Ecore_DBus_Server *
ecore_dbus_server_starter_connect(const void *data)
{
  Ecore_List *addrs;
  Ecore_DBus_Server *svr;
  char *bus_env;

  /* get the session bus address from the environment */
  bus_env = getenv("DBUS_STARTER_ADDRESS");
  if (bus_env)
  {
    addrs = ecore_dbus_address_parse(bus_env);
    if (addrs)
      {
	 svr = ecore_dbus_address_list_connect(addrs, data);
	 ecore_list_destroy(addrs);
	 if (svr) return svr;
      }
  }
  return NULL;
}


EAPI Ecore_DBus_Server *
ecore_dbus_server_connect(Ecore_Con_Type con_type, const char *name, int port,
			  const void *data __UNUSED__)
{
   /* XXX data isn't used! */
   Ecore_DBus_Server  *svr;
 
   svr = calloc(1, sizeof(Ecore_DBus_Server));
   if (!svr) return NULL;

   svr->server =
     ecore_con_server_connect(con_type, name, port, svr);

   if (!svr->server)
     {
	fprintf(stderr, "Ecore_DBus Error: Couldn't connect to server\n");
	free(svr);
	return NULL;
     }
   svr->authenticated = 0;
   svr->cnt_msg = 0;
   svr->auth_type = -1;
   svr->auth_type_transaction = 0;
   svr->messages = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
   ecore_hash_free_value_cb_set(svr->messages, ECORE_FREE_CB(_ecore_dbus_message_free));
   svr->objects = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   ecore_hash_free_key_cb_set(svr->objects, free);
   ecore_hash_free_value_cb_set(svr->objects, ECORE_FREE_CB(ecore_dbus_object_free));
   servers = _ecore_list2_append(servers, svr);

   return svr;
}

EAPI void
ecore_dbus_server_del(Ecore_DBus_Server *svr)
{
   if (svr->server) ecore_con_server_del(svr->server);
   servers = _ecore_list2_remove(servers, svr);
   if (svr->unique_name) free(svr->unique_name);
   ecore_hash_destroy(svr->messages);
   ecore_hash_destroy(svr->objects);
   free(svr);
}

EAPI int
ecore_dbus_server_send(Ecore_DBus_Server *svr, const char *command, int length)
{
   int                 ret;

   ret = ecore_con_server_send(svr->server, command, length);
   printf("[ecore_dbus] ecore_dbus_server: %p ecore_con_server: %p sent %d of %d bytes\n",
	  svr, svr->server, ret, length);
   return ret;
}

/* helper functions */

static char *
_ecore_dbus_getuid(void)
{
   /* this calculation is from comp.lang.c faq */
#define MAX_LONG_LEN ((sizeof (long) * 8 + 2) / 3 + 1)	/* +1 for '-' */
   int                 len;
   char               *uid;
   char               *tmp;

   tmp = (char *)malloc(MAX_LONG_LEN);
   len = snprintf(tmp, MAX_LONG_LEN, "%ld", (long) getuid());
   uid = (char *)malloc(len + 1);
   uid = memcpy(uid, tmp, len);
   uid[len] = '\0';

   free(tmp);
   return uid;
}

/* encodes a string into a string of hex values	*/
/* each byte is two hex digits			*/
static char *
_ecore_dbus_hex_encode(char *src_str)
{
   const char          hexdigits[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
   };
   char               *enc_str;
   char               *p;
   char               *end;
   int                 len;
   int                 i = 0;

   len = strlen(src_str);
   p = src_str;
   end = p + len;

   enc_str = malloc(2 * len + 1);
   while (p != end)
     {
	enc_str[i] = hexdigits[(*p >> 4)];
	i++;
	enc_str[i] = hexdigits[(*p & 0x0f)];
	i++;
	p++;
     }
   enc_str[i] = '\0';
   return enc_str;
}

/* auth functions */

unsigned char *
_ecore_dbus_auth_external(void *data __UNUSED__)
{
   char          *uid, *enc_uid, *msg;

   uid = _ecore_dbus_getuid();
   enc_uid = _ecore_dbus_hex_encode(uid);
   free(uid);
   msg = malloc(strlen(enc_uid) + 17);
   sprintf(msg, "AUTH EXTERNAL %s\r\n", enc_uid);
   free(enc_uid);
   return (unsigned char *)msg;
}

/* con functions */

static int
_ecore_dbus_event_server_add(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Add *e;
   Ecore_DBus_Server          *svr;

   e = ev;
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

   ecore_dbus_server_send(svr, "\0", 1);
   ecore_dbus_server_send(svr, "AUTH\r\n", 6);
   printf("[ecore_dbus] begining auth process\n");
   return 0;
}

static int
_ecore_dbus_event_server_del(void *udata __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Del  *e;
   Ecore_DBus_Server           *svr;
   Ecore_DBus_Event_Server_Del *e2;

   e = ev;
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

   e2 = calloc(1, sizeof(Ecore_DBus_Event_Server_Del));
   if (e2)
     {
	svr->server = NULL;
	e2->server = svr;
	ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, e2, _ecore_dbus_event_server_del_free, NULL);
     }
   return 0;
}

static int
_ecore_dbus_event_server_data(void *udata __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_Con_Event_Server_Data *e;
   Ecore_DBus_Server           *svr;

   e = ev;
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

   if (!svr->authenticated)
     {
	/* authentication protocol */
	const Ecore_DBus_Auth *auth;
	Ecore_DBus_Auth_Transaction trans;

	if (!strncmp(e->data, "OK", 2))
	  {
	     printf("[ecore_dbus] auth type %s successful\n", auths[svr->auth_type].name);
	     ecore_dbus_server_send(svr, "BEGIN\r\n", 7);
	     svr->authenticated = 1;
	     /* Register on the bus */
	     ecore_dbus_method_hello(svr, _ecore_dbus_method_hello_cb, _ecore_dbus_method_error_cb, svr);
	  }
	else if (!strncmp(e->data, "DATA", 4))
	  {
	     printf("[ecore_dbus] requiring data (unavailable)\n");
	  }
	else if (!strncmp(e->data, "ERROR", 5))
	  {
	     printf("[ecore_dbus] auth process error\n");
	  }
	else if (!strncmp(e->data, "REJECTED", 8))
	  {
	     unsigned char      *msg;

	     if (svr->auth_type >= 0)
	       printf("[ecore_dbus] auth type %s rejected\n", auths[svr->auth_type].name);
	     svr->auth_type++;
	     auth = &auths[svr->auth_type];
	     trans = auth->transactions[0];
	     printf("[ecore_dbus] auth type %s started\n", auth->name);
	     msg = trans(NULL);
	     ecore_dbus_server_send(svr, (char *)msg, strlen((char *)msg));
	     free(msg);
	  }
     }
   else
     {
	/* message protocol */
	Ecore_DBus_Message           *msg;
	unsigned int                  offset = 0;

	printf("[ecore_dbus] received server data, %d bytes\n", e->size);
	while (e->size)
	  {
	     Ecore_DBus_Event_Server_Data *ev2;

	     msg = _ecore_dbus_message_unmarshal(svr, (unsigned char *)(e->data) + offset, e->size);
	     if (!msg) break;
	     offset += msg->length;
	     e->size -= msg->length;
	     printf("[ecore_dbus] dbus message length %u bytes, still %d\n",
		    msg->length, e->size);
	     //ecore_dbus_message_print(msg);
	     /* Trap known messages */
	     ev2 = _ecore_dbus_event_create(svr, msg);
	     if (!ev2) break;
	     if (msg->type == ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN)
	       {
		  Ecore_DBus_Message *sent;
		  sent = ecore_hash_remove(svr->messages, (void *)(ev2->header.reply_serial));
		  if ((sent) && (sent->cb.method_return))
		    {
		       sent->cb.method_return(sent->cb.data, ev2);
		    }
		  else
		    {
		       printf("[ecore_dbus] Reply without reply serial!\n");
		       printf("REPLY SERIAL: %d\n", ev2->header.reply_serial);
		    }
		  if (sent) _ecore_dbus_message_free(sent);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	     else if (msg->type == ECORE_DBUS_MESSAGE_TYPE_ERROR)
	       {
		  Ecore_DBus_Message *sent;
		  sent = ecore_hash_remove(svr->messages, (void *)(ev2->header.reply_serial));
		  if ((sent) && (sent->cb.error))
		    {
		       char *error = NULL;
		       if ((ev2->args) && (ev2->args[0].type == ECORE_DBUS_DATA_TYPE_STRING))
			 error = ev2->args[0].value;
		       sent->cb.error(sent->cb.data, error);
		    }
		  else
		    {
		       printf("[ecore_dbus] Error without reply serial!\n");
		       ecore_dbus_message_print(ev2->message);
		    }
		  if (sent) _ecore_dbus_message_free(sent);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	     else if (msg->type == ECORE_DBUS_MESSAGE_TYPE_SIGNAL)
	       {
		  ecore_event_add(ECORE_DBUS_EVENT_SIGNAL, ev2,
				  _ecore_dbus_event_server_data_free, NULL);
	       }
	     else if (msg->type == ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL)
	       {
		  ecore_event_add(ECORE_DBUS_EVENT_METHOD_CALL, ev2,
				  _ecore_dbus_event_server_data_free, NULL);
	       }
	     else
	       {
		  printf("Ecore_DBus: Unknown return type %d\n", msg->type);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	  }
     }
   return 0;
}

static void
_ecore_dbus_event_server_del_free(void *data __UNUSED__, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   ecore_dbus_server_del(event->server);
   free(ev);
}

static void
_ecore_dbus_event_server_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;

   event = ev;
   _ecore_dbus_message_free(event->message);
   if (event->args) free(event->args);
   free(ev);
}

static Ecore_DBus_Event_Server_Data *
_ecore_dbus_event_create(Ecore_DBus_Server *svr, Ecore_DBus_Message *msg)
{
   Ecore_DBus_Event_Server_Data *ev;
   unsigned int                 *serial;

   ev = calloc(1, sizeof(Ecore_DBus_Event_Server_Data));
   if (!ev) return NULL;
   ev->server = svr;
   ev->type = msg->type;
   ev->message = msg;
   ev->header.path = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_PATH);
   ev->header.interface = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_INTERFACE);
   ev->header.member = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_MEMBER);
   ev->header.error_name = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_ERROR_NAME);
   serial = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_REPLY_SERIAL);
   if (serial)
     ev->header.reply_serial = *serial;
   ev->header.destination = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_DESTINATION);
   ev->header.sender = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_SENDER);
   ev->header.signature = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_SIGNATURE);
   if (!ecore_list_empty_is(msg->fields))
     {
	Ecore_DBus_Message_Field *f;
	int i = 0;

	ev->args = malloc(ecore_list_count(msg->fields) * sizeof(Ecore_DBus_Message_Arg));
	ecore_list_first_goto(msg->fields);
	while ((f = ecore_list_next(msg->fields)))
	  {
	     ev->args[i].type = f->type;
	     ev->args[i].value = _ecore_dbus_message_field_value_get(f);
	     i++;
	  }
     }
   return ev;
}

static void
_ecore_dbus_method_hello_cb(void *data, Ecore_DBus_Method_Return *reply)
{
   Ecore_DBus_Event_Server_Add *svr_add;
   Ecore_DBus_Server           *svr;
   char                        *name;

   svr = data;
   name = reply->args[0].value;
   printf("Got unique name: %s\n", name);
   if (svr->unique_name)
     {
	printf("Ecore_DBus: Already said hello %s - %s\n",
	      svr->unique_name, name);
	free(svr->unique_name);
     }
   svr->unique_name = strdup(name);

   svr_add = malloc(sizeof(Ecore_DBus_Event_Server_Add));
   svr_add->server = svr;
   ecore_event_add(ECORE_DBUS_EVENT_SERVER_ADD, svr_add, NULL, NULL);
}

static void
_ecore_dbus_method_error_cb(void *data, const char *error)
{
   Ecore_DBus_Event_Server_Del *ev;
   Ecore_DBus_Server           *svr;

   svr = data;
   printf("Ecore_DBus: error %s\n", error);

   ev = malloc(sizeof(Ecore_DBus_Event_Server_Del));
   if (!ev) return;
   ev->server = svr;
   ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, ev, _ecore_dbus_event_server_del_free, NULL);
}
