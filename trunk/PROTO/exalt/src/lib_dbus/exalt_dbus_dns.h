/*
 * =====================================================================================
 *
 *       Filename:  exalt_dbus_dns.h
 *
 *    Description:  manage the dns list
 *
 *        Version:  1.0
 *        Created:  09/21/2007 09:02:16 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

/** @file exalt_dbus_dns.h */

#ifndef  EXALT_DBUS_DNS_INC
#define  EXALT_DBUS_DNS_INC

#include "libexalt_dbus.h"

/**
 * @defgroup DNS
 * @brief configure your DNS
 * @ingroup Exalt_DBus
 * @{
 */

int exalt_dbus_dns_list_get(Exalt_DBus_Conn* conn);
int exalt_dbus_dns_add(Exalt_DBus_Conn* conn, const char* dns);
int exalt_dbus_dns_delete(Exalt_DBus_Conn* conn, const char* dns);
int exalt_dbus_dns_replace(Exalt_DBus_Conn* conn, const char* old_dns, const char* new_dns);

#endif   /* ----- #ifndef EXALT_DBUS_DNS_INC  ----- */

/** @} */
