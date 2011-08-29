#include <Eina.h>
#include <E_DBus.h>
#include "plugin.h"
#include "log.h"

#define FSO_OUSAGED_SERVICE "org.freesmartphone.ousaged"
#define FSO_OUSAGED_OBJECT_PATH "/org/freesmartphone/Usage"
#define FSO_OUSAGED_INTERFACE "org.freesmartphone.Usage"

static E_DBus_Connection *sysconn = NULL;

/* callbacks */

static void
fso_request_reource_cb(void *data, DBusMessage *replymsg, DBusError *error)
{
   DBG("Request sent to fsousaged to enable resource.");
	
   if (error && dbus_error_is_set(error)) 
      ERR("Error requesting FSO resource: %s - %s\n", error->name, error->message);
}

static void
fso_release_reource_cb(void *data, DBusMessage *replymsg, DBusError *error)
{
   DBG("Request sent to fsousaged to disable resource.");
	
   if (error && dbus_error_is_set(error)) 
      ERR("Error releasing FSO resource: %s - %s", error->name, error->message);
}


/* methods */

static void
fso_request_resource(const char *resource)
{
   DBusMessage *msg;
   msg = dbus_message_new_method_call(
               FSO_OUSAGED_SERVICE,
               FSO_OUSAGED_OBJECT_PATH,
               FSO_OUSAGED_INTERFACE,
               "RequestResource");
   dbus_message_append_args(msg, DBUS_TYPE_STRING, &resource, DBUS_TYPE_INVALID);
   e_dbus_message_send(sysconn, msg, fso_request_reource_cb, -1, NULL);
   dbus_message_unref(msg);	
}


static void
fso_release_resource(const char *resource)
{
   DBusMessage *msg;
   msg = dbus_message_new_method_call(
               FSO_OUSAGED_SERVICE,
               FSO_OUSAGED_OBJECT_PATH,
               FSO_OUSAGED_INTERFACE,
               "ReleaseResource");
   dbus_message_append_args(msg, DBUS_TYPE_STRING, &resource, DBUS_TYPE_INVALID);
   e_dbus_message_send(sysconn, msg, fso_release_reource_cb, -1, NULL);
   dbus_message_unref(msg);	
}

Eina_Bool
fso_init(void)
{
   if (sysconn) return EINA_FALSE;
   e_dbus_init();
   sysconn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
   fso_request_resource("CPU");
   return EINA_TRUE;
}

void
fso_shutdown(void)
{
   if (!sysconn) return;
   fso_release_resource("CPU");
   e_dbus_shutdown();
   sysconn = NULL;
}

EINA_MODULE_INIT(fso_init);
EINA_MODULE_SHUTDOWN(fso_shutdown);
