#include "E_Notify.h"
#include "e_notify_private.h"
static E_DBus_Connection *client_conn;
static int init_count = 0;
  
int
e_notification_init(void)
{
  if (init_count) return ++init_count;

  if (!e_dbus_init()) return 0;
  client_conn = e_dbus_bus_get(DBUS_BUS_SESSION);
  if (!client_conn)
  {
    e_dbus_shutdown();
    return 0;
  }

  return ++init_count;
}

int
e_notification_shutdown(void)
{
  if (--init_count) return init_count;
  e_dbus_connection_unref(client_conn);
  client_conn = NULL;
  e_dbus_shutdown();
  return 0;
}

/**** client api ****/
void
e_notification_send(E_Notification *n, E_DBus_Callback_Func func, void *data)
{
  DBusMessage *msg;

  msg = e_notify_marshal_notify(n);
  e_dbus_method_call_send(client_conn, msg, e_notify_unmarshal_notify_return, func, -1, data);
}

void
e_notification_get_capabilities(E_DBus_Callback_Func func, void *data)
{
  DBusMessage *msg;

  msg = e_notify_marshal_get_capabilities();
  e_dbus_method_call_send(client_conn, msg, e_notify_unmarshal_get_capabilities_return, func, -1, data);
}

void
e_notification_get_server_information(E_DBus_Callback_Func func, void *data)
{
  DBusMessage *msg;

  msg = e_notify_marshal_get_server_information();
  e_dbus_method_call_send(client_conn, msg, e_notify_unmarshal_get_server_information_return, func, -1, data);
}
