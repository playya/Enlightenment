/*
 * =====================================================================================
 *
 *       Filename:  cb_wireless.c
 *
 *    Description:  All CB about a wireless interface
 *
 *        Version:  1.0
 *        Created:  09/01/2007 12:05:24 PM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

#include "cb_wireless.h"

DBusMessage * dbus_cb_wireless_list_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    return dbus_cb_eth_wireless_list_get(obj,msg,1);
}

DBusMessage * dbus_cb_wireless_essid_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter args;
    Exalt_Ethernet* eth;
    char* essid;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));

    eth= dbus_get_eth(msg);
    EXALT_ASSERT_CUSTOM_RET(eth!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_ERROR_ID,
                EXALT_DBUS_INTERFACE_ERROR);
            return reply);

    EXALT_ASSERT_CUSTOM_RET(exalt_eth_wireless_is(eth),
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR_ID,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR);
            return reply);

    essid = exalt_wireless_essid_get(exalt_eth_wireless_get(eth));
    EXALT_ASSERT_CUSTOM_RET(essid!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_ESSID_ERROR_ID,
                EXALT_DBUS_ESSID_ERROR);
            return reply);

    dbus_args_valid_append(reply);
    dbus_message_iter_init_append(reply, &args);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &essid),
            EXALT_FREE(essid);return reply);

    EXALT_FREE(essid);
    return reply;
}


DBusMessage * dbus_cb_wireless_disconnect(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    Exalt_Ethernet* eth;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));

    eth= dbus_get_eth(msg);
    EXALT_ASSERT_CUSTOM_RET(eth!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_ERROR_ID,
                EXALT_DBUS_INTERFACE_ERROR);
            return reply);

    EXALT_ASSERT_CUSTOM_RET(exalt_eth_wireless_is(eth),
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR_ID,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR);
            return reply);

    exalt_wireless_disconnect(exalt_eth_wireless_get(eth));

    dbus_args_valid_append(reply);

    return reply;
}


DBusMessage * dbus_cb_wireless_wpasupplicant_driver_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter args;
    Exalt_Ethernet* eth;
    const char* essid;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));

    eth= dbus_get_eth(msg);
    EXALT_ASSERT_CUSTOM_RET(eth!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_ERROR_ID,
                EXALT_DBUS_INTERFACE_ERROR);
            return reply);

    EXALT_ASSERT_CUSTOM_RET(exalt_eth_wireless_is(eth),
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR_ID,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR);
            return reply);

    essid = exalt_wireless_wpasupplicant_driver_get(exalt_eth_wireless_get(eth));
    EXALT_ASSERT_CUSTOM_RET(essid!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_ESSID_ERROR_ID,
                EXALT_DBUS_ESSID_ERROR);
            return reply);

    dbus_args_valid_append(reply);
    dbus_message_iter_init_append(reply, &args);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &essid),
            ;return reply);

    return reply;
}

DBusMessage * dbus_cb_wireless_wpasupplicant_driver_set(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter args;
    Exalt_Ethernet* eth;
    const char* driver;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));
    eth= dbus_get_eth(msg);
    EXALT_ASSERT_CUSTOM_RET(eth!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_ERROR_ID,
                EXALT_DBUS_INTERFACE_ERROR);
            return reply);

    EXALT_ASSERT_CUSTOM_RET(exalt_eth_wireless_is(eth),
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR_ID,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR);
            return reply);


    //retrieve the driver
    dbus_message_iter_init(msg, &args);

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&args, &driver);

    EXALT_ASSERT_CUSTOM_RET(driver!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
            return reply;
            );

    exalt_wireless_wpasupplicant_driver_set(exalt_eth_wireless_get(eth),driver);
    exalt_eth_save(CONF_FILE, eth);

    dbus_args_valid_append(reply);
    return reply;
}



DBusMessage * dbus_cb_wireless_scan(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    Exalt_Ethernet* eth;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));
    eth= dbus_get_eth(msg);
    EXALT_ASSERT_CUSTOM_RET(eth!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_ERROR_ID,
                EXALT_DBUS_INTERFACE_ERROR);
            return reply);

    EXALT_ASSERT_CUSTOM_RET(exalt_eth_wireless_is(eth),
            dbus_args_error_append(reply,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR_ID,
                EXALT_DBUS_INTERFACE_NOT_WIRELESS_ERROR);
            return reply);

    exalt_wireless_scan_start(eth);

    dbus_args_valid_append(reply);
    return reply;
}

DBusMessage * dbus_cb_wireless_network_configuration_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter args;
    Exalt_Connection *c;
    Exalt_Connection_Network *cn;
    char* essid;
    const char *string;
    int integer;

    reply = dbus_message_new_method_return(msg);
    dbus_message_set_path(reply,dbus_message_get_path(msg));

    dbus_message_iter_init(msg, &args);

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&args, &essid);

    EXALT_ASSERT_CUSTOM_RET(essid!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
            return reply;
            );

    c = exalt_conn_network_load(CONF_FILE, essid);
    EXALT_ASSERT_CUSTOM_RET(c!=NULL,
            dbus_args_error_append(reply,
                EXALT_DBUS_NETWORK_UNKNOWN_ID,
                EXALT_DBUS_NETWORK_UNKNOWN);
            return reply;
            );
    cn = exalt_conn_network_get(c);

    dbus_args_valid_append(reply);
    dbus_message_iter_init_append(reply, &args);


    integer = exalt_conn_mode_get(c);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    string = exalt_conn_ip_get(c);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    string = exalt_conn_netmask_get(c);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    string = exalt_conn_gateway_get(c);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    string = exalt_conn_cmd_after_apply_get(c);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    string = exalt_conn_network_essid_get(cn);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    integer = exalt_conn_network_encryption_is(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_mode_get(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    string = exalt_conn_network_key_get(cn);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    string = exalt_conn_network_login_get(cn);
    if(!string) string = "";
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string),
            return reply);

    integer = exalt_conn_network_wep_is(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_wep_hexa_is(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_wpa_is(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_wpa_type_get(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_group_cypher_get(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_pairwise_cypher_get(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_auth_suites_get(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    integer = exalt_conn_network_save_when_apply_is(cn);
    EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &integer),
            return reply);

    return reply;
}


