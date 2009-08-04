/*
 * =====================================================================================
 *
 *       Filename:  cb_wireless.h
 *
 *    Description:  All functions about a wireless interface
 *
 *        Version:  1.0
 *        Created:  09/01/2007 12:06:19 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

#ifndef  CB_WIRELESS_INC
#define  CB_WIRELESS_INC

#include "daemon.h"

DBusMessage * dbus_cb_wireless_list_get(E_DBus_Object *obj, DBusMessage *msg);

DBusMessage * dbus_cb_wireless_essid_get(E_DBus_Object *obj, DBusMessage *msg);
DBusMessage * dbus_cb_wireless_disconnect(E_DBus_Object *obj, DBusMessage *msg);

DBusMessage * dbus_cb_wireless_wpasupplicant_driver_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg);
DBusMessage * dbus_cb_wireless_wpasupplicant_driver_set(E_DBus_Object *obj __UNUSED__, DBusMessage *msg);

DBusMessage * dbus_cb_wireless_scan(E_DBus_Object *obj __UNUSED__, DBusMessage *msg);

#endif   /* ----- #ifndef CB_WIRELESS_INC  ----- */

