/*
 * =====================================================================================
 *
 *       Filename:  define.h
 *
 *    Description:  define some variables
 *
 *        Version:  1.0
 *        Created:  08/29/2007 01:40:41 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

/** @file define.h */

#ifndef  DEFINE_INC
#define  DEFINE_INC

/**
 * @defgroup Defines
 * @brief Some defines used by the library
 * @ingroup Exalt_DBus
 * @{
 */

#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif

#define EXALT_DBUS_ERROR_PRINT(error) if(error && dbus_error_is_set(error)) { EXALT_LOG_ERR("%s\n",error->name);return; }

#define EXALTD_PATH_DNS "/org/exalt/dns"
#define EXALTD_INTERFACE_DNS "org.exalt.dns"

#define EXALTD_PATH_IFACE "/org/exalt/interface"
#define EXALTD_INTERFACE_IFACE "org.exalt.interface"

#define EXALTD_PATH_IFACES_WIRED "/org/exalt/interfaces/wired"
#define EXALTD_INTERFACE_IFACES_WIRED "org.exalt.interfaces.wired"

#define EXALTD_PATH_IFACES_WIRELESS "/org/exalt/interfaces/wireless"
#define EXALTD_INTERFACE_IFACES_WIRELESS "org.exalt.interfaces.wireless"

#define EXALTD_PATH_NETWORK "/org/exalt/network"
#define EXALTD_INTERFACE_NETWORK "org.exalt.network"


#define EXALTD_PATH_NOTIFY "/org/exalt/notify"
#define EXALTD_INTERFACE_NOTIFY "org.exalt.notify"



/** The exalt-daemon service */
#define EXALTD_SERVICE "org.e.Exalt"
/** the read interface of exalt-daemon, used to get an ip address ...
 * This interface can't modify the configuration */
#define EXALTD_INTERFACE_READ "org.e.Exalt.Read"
/** The write interface of exalt-daemon, used to modify the configuration */
#define EXALTD_INTERFACE_WRITE "org.e.Exalt.Write"
/** The exalt-daemon path */
#define EXALTD_PATH "/org/e/Exalt"

#define exalt_dbus_dns_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE, \
        EXALTD_PATH_DNS, EXALTD_INTERFACE_DNS, member)

#define exalt_dbus_iface_call_new(member,path,interface) dbus_message_new_method_call(EXALTD_SERVICE, \
        path, interface, member)

#define exalt_dbus_ifaces_wired_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE, \
        EXALTD_PATH_IFACES_WIRED, EXALTD_INTERFACE_IFACES_WIRED, member)

#define exalt_dbus_ifaces_wireless_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE, \
        EXALTD_PATH_IFACES_WIRELESS, EXALTD_INTERFACE_IFACES_WIRELESS, member)

#define exalt_dbus_ifaces_network_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE, \
        EXALTD_PATH_NETWORK, EXALTD_INTERFACE_NETWORK, member)


/** Create a method for the read interface */
#define exalt_dbus_read_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE,EXALTD_PATH, EXALTD_INTERFACE_READ, member)
/** Create a method for the write interface */
#define exalt_dbus_write_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE,EXALTD_PATH, EXALTD_INTERFACE_WRITE, member)

#endif   /* ----- #ifndef DEFINE_INC  ----- */

/** @} */


