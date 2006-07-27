/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_DBUS_H
#define _ECORE_DBUS_H
#endif

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#define ECORE_DBUS_MAJOR_PROTOCOL_VERSION 0x1

#ifdef __cplusplus
extern "C" {
#endif

   typedef struct _Ecore_DBus_Server            Ecore_DBus_Server;
   typedef struct _Ecore_DBus_Event_Server_Add  Ecore_DBus_Event_Server_Add;
   typedef struct _Ecore_DBus_Event_Server_Del  Ecore_DBus_Event_Server_Del;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Server_Data;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Server_Signal;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Method_Return;
   typedef struct _Ecore_DBus_Message           Ecore_DBus_Message;
   typedef struct _Ecore_DBus_Message_Arg       Ecore_DBus_Message_Arg;
   typedef struct _Ecore_DBus_Message_Field     Ecore_DBus_Message_Field;

   typedef enum _Ecore_DBus_Type
     {
	ECORE_DBUS_BUS_SESSION,
	ECORE_DBUS_BUS_SYSTEM,
	ECORE_DBUS_BUS_ACTIVATION
     } Ecore_DBus_Type;

   typedef enum _Ecore_DBus_Message_Type
     {
	ECORE_DBUS_MESSAGE_TYPE_INVALID,
	ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL,
	ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN,
	ECORE_DBUS_MESSAGE_TYPE_ERROR,
	ECORE_DBUS_MESSAGE_TYPE_SIGNAL
     } Ecore_DBus_Message_Type;

   typedef enum _Ecore_DBus_Data_Type
     {
	ECORE_DBUS_DATA_TYPE_INVALID          = ((int) '\0'),
	ECORE_DBUS_DATA_TYPE_BYTE             = ((int) 'y'),
	ECORE_DBUS_DATA_TYPE_BOOLEAN          = ((int) 'b'), /* 0,1 */
	ECORE_DBUS_DATA_TYPE_INT16            = ((int) 'n'),
	ECORE_DBUS_DATA_TYPE_UINT16           = ((int) 'q'),
	ECORE_DBUS_DATA_TYPE_INT32            = ((int) 'i'),
	ECORE_DBUS_DATA_TYPE_UINT32           = ((int) 'u'),
	ECORE_DBUS_DATA_TYPE_INT64            = ((int) 'x'),
	ECORE_DBUS_DATA_TYPE_UINT64           = ((int) 't'),
	ECORE_DBUS_DATA_TYPE_DOUBLE           = ((int) 'd'),
	ECORE_DBUS_DATA_TYPE_STRING           = ((int) 's'),
	ECORE_DBUS_DATA_TYPE_OBJECT_PATH      = ((int) 'o'),
	ECORE_DBUS_DATA_TYPE_SIGNATURE        = ((int) 'g'),
	ECORE_DBUS_DATA_TYPE_ARRAY            = ((int) 'a'),
	ECORE_DBUS_DATA_TYPE_VARIANT          = ((int) 'v'),
	ECORE_DBUS_DATA_TYPE_STRUCT           = ((int) 'r'),
	ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN     = ((int) '('),
	ECORE_DBUS_DATA_TYPE_STRUCT_END       = ((int) ')'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY       = ((int) 'e'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN = ((int) '{'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END   = ((int) '}'),
     } Ecore_DBus_Data_Type;

   struct _Ecore_DBus_Event_Server_Add
     {
	Ecore_DBus_Server *server;
     };

   struct _Ecore_DBus_Event_Server_Del
     {
	Ecore_DBus_Server *server;
     };

   struct _Ecore_DBus_Event_Server_Data
     {
	Ecore_DBus_Server       *server;
	Ecore_DBus_Message_Type  type;
	Ecore_DBus_Message      *message;
	struct {
	     const char   *path;
	     const char   *interface;
	     const char   *member;
	     const char   *error_name;
	     unsigned int  reply_serial;
	     const char   *destination;
	     const char   *sender;
	     const char   *signature;
	} header;
	Ecore_DBus_Message_Arg *args;
     };

   struct _Ecore_DBus_Message_Arg
     {
	Ecore_DBus_Data_Type  type;
	void                 *value;
     };

   typedef enum _Ecore_DBus_Message_Header_Field
     {
	ECORE_DBUS_HEADER_FIELD_INVALID,
	ECORE_DBUS_HEADER_FIELD_PATH,
	ECORE_DBUS_HEADER_FIELD_INTERFACE,
	ECORE_DBUS_HEADER_FIELD_MEMBER,
	ECORE_DBUS_HEADER_FIELD_ERROR_NAME,
	ECORE_DBUS_HEADER_FIELD_REPLY_SERIAL,
	ECORE_DBUS_HEADER_FIELD_DESTINATION,
	ECORE_DBUS_HEADER_FIELD_SENDER,
	ECORE_DBUS_HEADER_FIELD_SIGNATURE
     } Ecore_DBus_Message_Header_Field;

   EAPI extern int ECORE_DBUS_EVENT_SERVER_ADD;
   EAPI extern int ECORE_DBUS_EVENT_SERVER_DEL;
   EAPI extern int ECORE_DBUS_EVENT_SERVER_SIGNAL;

   /* callback */
   typedef void (*Ecore_DBus_Method_Cb)(void *data, Ecore_DBus_Message_Type type, Ecore_DBus_Method_Return *reply);

   /* init */
   EAPI int ecore_dbus_init(void);
   EAPI int ecore_dbus_shutdown(void);

   /* connection */
   EAPI Ecore_DBus_Server *ecore_dbus_server_connect(Ecore_DBus_Type type, char *name, int port, const void *data);
   EAPI void               ecore_dbus_server_del(Ecore_DBus_Server *svr);


   /* message */
   EAPI int           ecore_dbus_server_send(Ecore_DBus_Server *svr, char *command, int length);
   EAPI unsigned int  ecore_dbus_message_new_method_call(Ecore_DBus_Server *svr,
							 char *destination, char *path,
							 char *interface, char *method,
							 void (*method_cb)(void *udata,
									   Ecore_DBus_Message_Type type,
									   Ecore_DBus_Event_Server_Data *data),
							 void *data,
							 char *fmt, ...);
   EAPI void          ecore_dbus_message_del(Ecore_DBus_Message *msg);
   EAPI void          ecore_dbus_message_print(Ecore_DBus_Message *msg);
   EAPI void         *ecore_dbus_message_header_field_get(Ecore_DBus_Message *msg, Ecore_DBus_Message_Header_Field field);
   EAPI void         *ecore_dbus_message_body_field_get(Ecore_DBus_Message *msg, unsigned int pos);

   /* methods */
   EAPI int ecore_dbus_method_hello(Ecore_DBus_Server *svr, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_list_names(Ecore_DBus_Server *svr, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_name_has_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_start_service_by_name(Ecore_DBus_Server *svr, char *name, unsigned int flags, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_get_name_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_get_connection_unix_user(Ecore_DBus_Server *svr, char *connection, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_add_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Cb method_cb, void *data);
   EAPI int ecore_dbus_method_remove_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Cb method_cb, void *data);

#ifdef __cplusplus
}
#endif
