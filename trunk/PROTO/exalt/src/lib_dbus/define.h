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
 * @defgroup Some defines
 * @brief Some defines used by the library
 * @{
 */

/** The exalt-daemon service */
#define EXALTD_SERVICE "org.e.Exalt"
/** the read interface of exalt-daemon, used to get an ip address ...
 * This interface can't modify the configuration */
#define EXALTD_INTERFACE_READ "org.e.Exalt.Read"
/** The write interface of exalt-daemon, used to modify the configuration */
#define EXALTD_INTERFACE_WRITE "org.e.Exalt.Write"
/** The exalt-daemon path */
#define EXALTD_PATH "/org/e/Exalt"

/** Create a method for the read interface */
#define exalt_dbus_read_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE,EXALTD_PATH, EXALTD_INTERFACE_READ, member)
/** Create a method for the write interface */
#define exalt_dbus_write_call_new(member) dbus_message_new_method_call(EXALTD_SERVICE,EXALTD_PATH, EXALTD_INTERFACE_WRITE, member)

#endif   /* ----- #ifndef DEFINE_INC  ----- */

/** @} */


