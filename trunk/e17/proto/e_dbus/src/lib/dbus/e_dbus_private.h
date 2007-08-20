#ifndef E_DBUS_PRIVATE_H
#define E_DBUS_PRIVATE_H

#include "dbus/dbus.h"
#include "Ecore_Data.h"

struct E_DBus_Connection
{
  DBusBusType shared_type;
  DBusConnection *conn;
  char *conn_name;

  Ecore_List *fd_handlers;
  Ecore_List *timeouts;

  Ecore_Idler *idler;

  int refcount;
};

struct E_DBus_Callback
{
  E_DBus_Callback_Func cb_func;
  E_DBus_Unmarshal_Func unmarshal_func;
  E_DBus_Free_Func free_func;
  void *user_data;
};

int e_dbus_object_init(void);
void e_dbus_object_shutdown(void);

#endif
