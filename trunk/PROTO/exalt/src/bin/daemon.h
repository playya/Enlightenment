/*
 * =====================================================================================
 *
 *       Filename:  daemon.h
 *
 *    Description:  define globals functions
 *
 *        Version:  1.0
 *        Created:  08/28/2007 04:26:56 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

#ifndef  DAEMON_INC
#define  DAEMON_INC


#include <stdio.h>
#include <libexalt.h>

#include "define.h"
#include "cb_ethernet.h"
#include "cb_wireless.h"
#include "cb_wirelessnetwork.h"
#include "cb_dns.h"
#include "boot_process.h"
#include "cb_bootprocess.h"
#include "cb_network.h"
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
extern int exaltd_log_domain;


typedef struct _dbus_object_item DBus_Object_Item;

struct _dbus_object_item
{
    E_DBus_Object* o;
    char* iface;
};


/*
 * @brief The dbus connection of the daemon
 */
E_DBus_Connection* exaltd_conn;


int main(int argc, char** argv);
int setup(E_DBus_Connection *conn);
Exalt_Ethernet* dbus_get_eth(DBusMessage* msg);
void eth_cb(Exalt_Ethernet* eth, Exalt_Enum_Action action, void* data);
void wireless_scan_cb(Exalt_Ethernet* eth, Eina_List* networks, void* data);

DBusMessage* conf_from_dbusmessage(Exalt_Configuration *c, DBusMessage *msg,DBusMessage *reply);


int dbus_args_error_append(DBusMessage *msg, int id_error, const char* error);
int dbus_args_valid_append(DBusMessage *msg);

#endif   /* ----- #ifndef DAEMON_INC  ----- */


