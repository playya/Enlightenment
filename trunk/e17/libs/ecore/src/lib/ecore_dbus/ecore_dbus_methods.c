/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* Standard dbus methods */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

EAPI int
ecore_dbus_method_hello(Ecore_DBus_Server *svr, Ecore_DBus_Method_Cb method_cb, void *data)
{
   if (svr->unique_name)
     {
	printf("Ecore_DBus: Already registered on the message bus.\n");
	return 0;
     }
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "Hello" /*method*/,
					     method_cb, data,
					     NULL /*fmt*/);
}

EAPI int
ecore_dbus_method_list_names(Ecore_DBus_Server *svr, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "ListNames" /*method*/,
					     method_cb, data,
					     NULL /*fmt*/);
}

EAPI int
ecore_dbus_method_name_has_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "NameHasOwner" /*method*/,
					     method_cb, data,
					     "s" /*fmt*/, name);
}

EAPI int
ecore_dbus_method_start_service_by_name(Ecore_DBus_Server *svr, char *name, unsigned int flags, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "StartServiceByName" /*method*/,
					     method_cb, data,
					     "su" /*fmt*/, name, flags);
}

EAPI int
ecore_dbus_method_get_name_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "GetNameOwner" /*method*/,
					     method_cb, data,
					     "s" /*fmt*/, name);
}

EAPI int
ecore_dbus_method_get_connection_unix_user(Ecore_DBus_Server *svr, char *connection, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "GetConnectionUnixUser" /*method*/,
					     method_cb, data,
					     "s" /*fmt*/, connection);
}

EAPI int
ecore_dbus_method_add_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "AddMatch" /*method*/,
					     method_cb, data,
					     "s" /*fmt*/, match);
}

EAPI int
ecore_dbus_method_remove_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Cb method_cb, void *data)
{
   return ecore_dbus_message_new_method_call(svr,
					     "org.freedesktop.DBus" /*destination*/,
					     "/org/freedesktop/DBus" /*path*/,
					     "org.freedesktop.DBus" /*interface*/,
					     "RemoveMatch" /*method*/,
					     method_cb, data,
					     "s" /*fmt*/, match);
}
