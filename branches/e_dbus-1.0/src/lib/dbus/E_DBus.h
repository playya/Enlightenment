#ifndef E_DBUS_H
#define E_DBUS_H

#define DBUS_API_SUBJECT_TO_CHANGE

#ifdef _WIN32
# ifdef interface
#  undef interface
# endif
#endif

#ifdef _WIN32
# ifdef interface
#  undef interface
# endif
# define DBUS_API_SUBJECT_TO_CHANGE
#endif

#include <dbus/dbus.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EDBUS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDBUS_BUILD */
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

/**
 * @mainpage EDbus
 *
 * @section edbus_intro_sec Introduction
 *
 * EDbus is a wrapper around the
 * <a href="http://www.freedesktop.org/wiki/Software/dbus">dbus</a>
 * library, which is a message bus system. It also implement a set of
 * specifications using dbus as interprocess comunication.
 *
 * @section edbus_modules_sec Modules
 *
 * @li @ref EDbus_Group Wrapper around the dbus library, which
 * implementent an inter-process communication (IPC) system for
 * software applications to communicate with one another.
 * @li @ref EBluez_Group Implementation of the <a
 * href="http://www.bluez.org/">BlueZ</a> specifications, for wireless
 * communications with Bleutooth devices.
 * @li @ref EConnman_Group Implementation of the <a
 * href="http://connman.net/">connman</a> specifications, which
 * manages internet connections within embedded devices running the
 * Linux operating system.
 * @li @ref EHal_Group Implementation of the <a
 * href="http://www.freedesktop.org/wiki/Software/hal">HAL</a>
 * specifications, which is a (software) layer between the hardware
 * devices of a computer and the softwares that run on that
 * computer (Hardware Abstraction Layer). HAL is deprecated, in favor
 * of DeviceKit.
 * @li @ref ENotify_Group To de described. 
 * @li @ref EOfono_Group Implementation of the <a
 * href="http://ofono.org/">ofono</a> specifications, which is an
 * interface for mobile telephony applications.
 * @li @ref EUkit_Group Implementation of the <a
 * href="http://freedesktop.org/wiki/Software/DeviceKit">DeviceKit</a>
 * specifications, which is, like HAL, an Hardware Abstraction
 * Layer. DeviceKit is a replacement of the deprecated HAL system. It
 * has two submodules: UDisks, which manipulate storage devices, and
 * UPower, which manage power devices.
 */

/**
 * @defgroup EDbus_Group EDbus
 *
 * @{
 */

#define E_DBUS_FDO_BUS "org.freedesktop.DBus"
#define E_DBUS_FDO_PATH "/org/freedesktop/DBus"
#define E_DBUS_FDO_INTERFACE E_DBUS_FDO_BUS
#define E_DBUS_FDO_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

#ifdef __cplusplus
extern "C" {
#endif
   
#define E_DBUS_VERSION_MAJOR 1
#define E_DBUS_VERSION_MINOR 0
   
   typedef struct _E_DBus_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } E_DBus_Version;
   
   EAPI extern E_DBus_Version *e_dbus_version;
   
   EAPI extern int E_DBUS_DOMAIN_GLOBAL;
   EAPI extern int E_DBUS_EVENT_SIGNAL;

   typedef struct E_DBus_Connection E_DBus_Connection;
   typedef struct E_DBus_Object E_DBus_Object;
   typedef struct E_DBus_Interface E_DBus_Interface;
   typedef struct E_DBus_Signal_Handler E_DBus_Signal_Handler;

   typedef DBusMessage *(* E_DBus_Method_Cb)(E_DBus_Object *obj, DBusMessage *message);
   typedef void (*E_DBus_Method_Return_Cb) (void *data, DBusMessage *msg, DBusError *error);
   typedef void (*E_DBus_Signal_Cb) (void *data, DBusMessage *msg);

   typedef void (*E_DBus_Object_Property_Get_Cb) (E_DBus_Object *obj, const char *property, int *type, void **value);
   typedef int  (*E_DBus_Object_Property_Set_Cb) (E_DBus_Object *obj, const char *property, int type, void *value);

/**
 * A callback function for a DBus call
 * @param user_data the data passed in to the method call
 * @param event_data a struct containing the return data.
 */
   typedef void (*E_DBus_Callback_Func) (void *user_data, void *method_return, DBusError *error);
   typedef void *(*E_DBus_Unmarshal_Func) (DBusMessage *msg, DBusError *err);
   typedef void (*E_DBus_Free_Func) (void *data);

   typedef struct E_DBus_Callback E_DBus_Callback;


   EAPI int e_dbus_init(void);
   EAPI int e_dbus_shutdown(void);

/* setting up the connection */

   EAPI E_DBus_Connection *e_dbus_bus_get(DBusBusType type);

   EAPI void e_dbus_connection_ref(E_DBus_Connection *conn);

   EAPI E_DBus_Connection *e_dbus_connection_setup(DBusConnection *conn);
   EAPI void e_dbus_connection_close(E_DBus_Connection *conn);

/* receiving method calls */
   EAPI E_DBus_Interface *e_dbus_interface_new(const char *interface);
   EAPI void e_dbus_interface_ref(E_DBus_Interface *iface);
   EAPI void e_dbus_interface_unref(E_DBus_Interface *iface);
   EAPI void e_dbus_object_interface_attach(E_DBus_Object *obj, E_DBus_Interface *iface);
   EAPI void e_dbus_object_interface_detach(E_DBus_Object *obj, E_DBus_Interface *iface);
   EAPI int e_dbus_interface_method_add(E_DBus_Interface *iface, const char *member, const char *signature, const char *reply_signature, E_DBus_Method_Cb func);

   EAPI int e_dbus_interface_signal_add(E_DBus_Interface *iface, const char *name, const char *signature);

   EAPI E_DBus_Object *e_dbus_object_add(E_DBus_Connection *conn, const char *object_path, void *data);
   EAPI void e_dbus_object_free(E_DBus_Object *obj);
   EAPI void *e_dbus_object_data_get(E_DBus_Object *obj);
   EAPI E_DBus_Connection *e_dbus_object_conn_get(E_DBus_Object *obj);
   EAPI const char *e_dbus_object_path_get(E_DBus_Object *obj);
   EAPI const Eina_List *e_dbus_object_interfaces_get(E_DBus_Object *obj);

   EAPI void e_dbus_object_property_get_cb_set(E_DBus_Object *obj, E_DBus_Object_Property_Get_Cb func);
   EAPI void e_dbus_object_property_set_cb_set(E_DBus_Object *obj, E_DBus_Object_Property_Set_Cb func);


/* sending method calls */


   EAPI DBusPendingCall *e_dbus_message_send(E_DBus_Connection *conn, DBusMessage *msg, E_DBus_Method_Return_Cb cb_return, int timeout, void *data);

   EAPI DBusPendingCall *e_dbus_method_call_send(E_DBus_Connection *conn, DBusMessage *msg, E_DBus_Unmarshal_Func unmarshal_func, E_DBus_Callback_Func cb_func, E_DBus_Free_Func free_func, int timeout, void *data);


/* signal receiving */

   EAPI E_DBus_Signal_Handler *e_dbus_signal_handler_add(E_DBus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, E_DBus_Signal_Cb cb_signal, void *data);
   EAPI void e_dbus_signal_handler_del(E_DBus_Connection *conn, E_DBus_Signal_Handler *sh);

/* standard dbus method calls */

   EAPI DBusPendingCall *e_dbus_request_name(E_DBus_Connection *conn, const char *name,
					     unsigned int flags,
					     E_DBus_Method_Return_Cb cb_return,
					     const void *data);
   EAPI DBusPendingCall *e_dbus_release_name(E_DBus_Connection *conn, const char *name,
					     E_DBus_Method_Return_Cb cb_return,
					     const void *data);

   EAPI DBusPendingCall *e_dbus_get_name_owner(E_DBus_Connection *conn, const char *name,
					       E_DBus_Method_Return_Cb cb_return,
					       const void *data);
   EAPI DBusPendingCall *e_dbus_list_names(E_DBus_Connection *conn,
					   E_DBus_Method_Return_Cb cb_return,
					   const void *data);
   EAPI DBusPendingCall *e_dbus_list_activatable_names(E_DBus_Connection *conn,
						       E_DBus_Method_Return_Cb cb_return,
						       const void *data);
   EAPI DBusPendingCall *e_dbus_name_has_owner(E_DBus_Connection *conn, const char *name,
					       E_DBus_Method_Return_Cb cb_return,
					       const void *data);
   EAPI DBusPendingCall *e_dbus_start_service_by_name(E_DBus_Connection *conn, const char *name, unsigned int flags,
						      E_DBus_Method_Return_Cb cb_return,
						      const void *data);

/* standard methods calls on objects */
   EAPI DBusPendingCall *e_dbus_introspect(E_DBus_Connection *conn, const char *bus,
       const char *object_path, E_DBus_Method_Return_Cb cb_return, const void *data);
   EAPI DBusPendingCall *e_dbus_peer_ping(E_DBus_Connection *conn, const char *destination,
					  const char *path, E_DBus_Method_Return_Cb cb_return,
					  const void *data);
   EAPI DBusPendingCall *e_dbus_peer_get_machine_id(E_DBus_Connection *conn,
						    const char *destination, const char *path,
						    E_DBus_Method_Return_Cb cb_return,
						    const void *data);
   EAPI DBusPendingCall *e_dbus_properties_get_all(E_DBus_Connection *conn, const char *destination,
						   const char *path, const char *interface,
						   E_DBus_Method_Return_Cb cb_return,
						   const void *data);
   EAPI DBusPendingCall *e_dbus_properties_get(E_DBus_Connection *conn, const char *destination,
					       const char *path, const char *interface,
					       const char *property,
					       E_DBus_Method_Return_Cb cb_return,
					       const void *data);
   EAPI DBusPendingCall *e_dbus_properties_set(E_DBus_Connection *conn, const char *destination,
					       const char *path, const char *interface,
					       const char *property, int value_type,
					       const void *value, E_DBus_Method_Return_Cb cb_return,
					       const void *data);


   EAPI E_DBus_Callback *e_dbus_callback_new(E_DBus_Callback_Func cb_func, E_DBus_Unmarshal_Func unmarshal_func, E_DBus_Free_Func free_func, void *user_data);

   EAPI void e_dbus_callback_free(E_DBus_Callback *callback);
   EAPI void e_dbus_callback_call(E_DBus_Callback *cb, void *data, DBusError *error);
   EAPI void *e_dbus_callback_unmarshal(E_DBus_Callback *cb, DBusMessage *msg, DBusError *err);
   EAPI void e_dbus_callback_return_free(E_DBus_Callback *callback, void *data);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
