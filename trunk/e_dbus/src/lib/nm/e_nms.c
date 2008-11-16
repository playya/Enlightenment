#include "E_Nm.h"
#include "e_nm_private.h"

#include <string.h>

static void
cb_nms_user_connections(void *data, void *reply, DBusError *err)
{
  Reply_Data  *d;
  E_NMS *nms;
  Ecore_List *connections;
  Ecore_List *list, *list2;
  char *path;

  d = data;
  nms = d->object;
  list = d->reply;
  if (dbus_error_is_set(err))
  {
    printf("Error: %s - %s\n", err->name, err->message);
    d->cb_func(d->data, NULL);
    ecore_list_destroy(list);
    free(d);
    return;
  }
  list2 = ecore_list_new();
  ecore_list_free_cb_set(list2, ECORE_FREE_CB(e_nms_connection_free));
  while ((path = ecore_list_first_remove(list)))
  {
    ecore_list_append(list2, e_nms_connection_get(nms, E_NMS_SERVICE_SYSTEM, path));
    free(path);
  }
  ecore_list_destroy(list);
 
  connections = reply;
  ecore_list_first_goto(connections);
  while ((path = ecore_list_next(connections)))
    ecore_list_append(list2, e_nms_connection_get(nms, E_NMS_SERVICE_USER, path));
  d->cb_func(d->data, list2);
  free(d);
}

static void
cb_nms_system_connections(void *data, void *reply, DBusError *err)
{
  Reply_Data  *d;
  E_NMS_Internal *nmsi;
  Ecore_List *connections;
  Ecore_List *list;
  const char *conn;
  DBusMessage *msg;

  d = data;
  nmsi = d->object;
  if (dbus_error_is_set(err))
  {
    printf("Error: %s - %s\n", err->name, err->message);
    d->cb_func(d->data, NULL);
    free(d);
    return;
  }
  list = ecore_list_new();
  ecore_list_free_cb_set(list, free);
  d->reply = list;

  connections = reply;
  ecore_list_first_goto(connections);
  while ((conn = ecore_list_next(connections)))
    ecore_list_append(list, strdup(conn));

  msg = e_nms_call_new(E_NMS_SERVICE_USER, "ListConnections");

  e_dbus_method_call_send(nmsi->nmi->conn, msg, cb_nm_object_path_list, cb_nms_user_connections, free_nm_object_path_list, -1, d);
  dbus_message_unref(msg);
}

static void
new_connection(const char *service_name, void *data, DBusMessage *msg)
{
  E_NMS_Internal *nmsi;
  const char *conn;
  DBusError err;
  if (!msg || !data) return;

  nmsi = data;
  dbus_error_init(&err);
  dbus_message_get_args(msg, &err, DBUS_TYPE_OBJECT_PATH, &conn, DBUS_TYPE_INVALID);
  if (dbus_error_is_set(&err))
  {
    printf("Error: %s - %s\n", err.name, err.message);
    return;
  }

  if (nmsi->new_connection)
    nmsi->new_connection((E_NMS *)nmsi, service_name, conn);
}

static void
cb_new_system_connection(void *data, DBusMessage *msg)
{
  new_connection(E_NMS_SERVICE_SYSTEM, data, msg);
}

static void
cb_new_user_connection(void *data, DBusMessage *msg)
{
  new_connection(E_NMS_SERVICE_USER, data, msg);
}

EAPI E_NMS *
e_nms_get(E_NM *nm)
{
  E_NMS_Internal *nmsi;

  nmsi = calloc(1, sizeof(E_NMS_Internal));
  nmsi->nmi = (E_NM_Internal *)nm;
  nmsi->handlers = ecore_list_new();
  ecore_list_append(nmsi->handlers, e_nms_signal_handler_add(nmsi->nmi->conn, E_NMS_SERVICE_SYSTEM, "NewConnection", cb_new_system_connection, nmsi));
  ecore_list_append(nmsi->handlers, e_nms_signal_handler_add(nmsi->nmi->conn, E_NMS_SERVICE_USER, "NewConnection", cb_new_user_connection, nmsi));

  return (E_NMS *)nmsi;
}

EAPI void
e_nms_free(E_NMS *nms)
{
  E_NMS_Internal *nmsi;
  if (!nms) return;
  nmsi = (E_NMS_Internal *)nms;

  if (nmsi->handlers)
  {
    E_DBus_Signal_Handler *sh;

    while ((sh = ecore_list_first_remove(nmsi->handlers)))
      e_dbus_signal_handler_del(nmsi->nmi->conn, sh);
    ecore_list_destroy(nmsi->handlers);
  }
  free(nmsi);
}

EAPI void
e_nms_dump(E_NMS *nms)
{
  if (!nms) return;
  printf("E_NMS:\n");
  printf("\n");
}

EAPI int
e_nms_list_connections(E_NMS *nms, int (*cb_func)(void *data, Ecore_List *list), void *data)
{
  DBusMessage *msg;
  Reply_Data   *d;
  E_NMS_Internal *nmsi;
  int ret;

  nmsi = (E_NMS_Internal *)nms;
  d = calloc(1, sizeof(Reply_Data));
  d->cb_func = OBJECT_CB(cb_func);
  d->data = data;
  d->object = nmsi;

  msg = e_nms_call_new(E_NMS_SERVICE_SYSTEM, "ListConnections");

  ret = e_dbus_method_call_send(nmsi->nmi->conn, msg, cb_nm_object_path_list, cb_nms_system_connections, free_nm_object_path_list, -1, d) ? 1 : 0;
  dbus_message_unref(msg);
  return ret;
}

EAPI void
e_nms_data_set(E_NMS *nms, void *data)
{
  E_NMS_Internal *nmsi;

  nmsi = (E_NMS_Internal *)nms;
  nmsi->data = data;
}

EAPI void *
e_nms_data_get(E_NMS *nms)
{
  E_NMS_Internal *nmsi;

  nmsi = (E_NMS_Internal *)nms;
  return nmsi->data;
}

EAPI void
e_nms_callback_new_connection_set(E_NMS *nms, int (*cb_func)(E_NMS *nms, const char *service_name, const char *connection))
{
  E_NMS_Internal *nmsi;

  nmsi = (E_NMS_Internal *)nms;
  nmsi->new_connection = cb_func;
}
