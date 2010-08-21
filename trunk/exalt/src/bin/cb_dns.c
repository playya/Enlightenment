#include "cb_dns.h"

#define EXALT_LOG_DOMAIN exaltd_log_domain

DBusMessage * dbus_cb_dns_list_get(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter iter;
    DBusMessageIter	iter_array;
    char* dns;
    Eina_List *dnss;
    Eina_List *l;

    reply = dbus_message_new_method_return(msg);

    dnss = exalt_dns_get_list();
    EXALT_ASSERT_ADV(!!dnss,
            dbus_args_error_append(reply,
                EXALT_DBUS_DNS_ERROR_ID,
                EXALT_DBUS_DNS_ERROR);
            return reply,
            "dnss!=NULL failed\n");

    dbus_args_valid_append(reply);

    dbus_message_iter_init_append(reply, &iter);

    EXALT_ASSERT_CUSTOM_RET(
                dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
                    DBUS_TYPE_STRING_AS_STRING, &iter_array),
                eina_list_free(dnss);return reply);

    EINA_LIST_FOREACH(dnss,l,dns)
    {
        EXALT_ASSERT_CUSTOM_RET(!!dns,
                dbus_args_error_append(reply,
                    EXALT_DBUS_DNS_ERROR_ID,
                    EXALT_DBUS_DNS_ERROR);
                eina_list_free(dnss);return reply);

        EXALT_ASSERT_CUSTOM_RET(dbus_message_iter_append_basic(&iter_array, DBUS_TYPE_STRING, &dns),
                dbus_args_error_append(reply,
                    EXALT_DBUS_DNS_ERROR_ID,
                    EXALT_DBUS_DNS_ERROR);
                eina_list_free(dnss);return reply);
    }

    dbus_message_iter_close_container (&iter, &iter_array);

    eina_list_free(dnss);
    return reply;
}

DBusMessage * dbus_cb_dns_add(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter iter;
    char* dns;

    reply = dbus_message_new_method_return(msg);

    if(!dbus_message_iter_init(msg, &iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_NO_ARGUMENT_ID,
                EXALT_DBUS_NO_ARGUMENT);
        return reply;
    }

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&iter, &dns);

    exalt_dns_add(dns);

    dbus_args_valid_append(reply);

    return reply;
}


DBusMessage * dbus_cb_dns_delete(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter iter;
    char* dns;

    reply = dbus_message_new_method_return(msg);

    if(!dbus_message_iter_init(msg, &iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_NO_ARGUMENT_ID,
                EXALT_DBUS_NO_ARGUMENT);
        return reply;
    }

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&iter, &dns);

    exalt_dns_delete(dns);

    dbus_args_valid_append(reply);

    return reply;
}


DBusMessage * dbus_cb_dns_replace(E_DBus_Object *obj __UNUSED__, DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter iter;
    char* old_dns, *new_dns;

    reply = dbus_message_new_method_return(msg);

    if(!dbus_message_iter_init(msg, &iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_NO_ARGUMENT_ID,
                EXALT_DBUS_NO_ARGUMENT);
        return reply;
    }

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&iter, &old_dns);

    dbus_message_iter_next(&iter);

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&iter))
    {
        dbus_args_error_append(reply,
                EXALT_DBUS_ARGUMENT_NOT_STRING_ID,
                EXALT_DBUS_ARGUMENT_NOT_STRING);
        return reply;
    }
    else
        dbus_message_iter_get_basic(&iter, &new_dns);

    exalt_dns_replace(old_dns, new_dns);

    dbus_args_valid_append(reply);
    return reply;
}
