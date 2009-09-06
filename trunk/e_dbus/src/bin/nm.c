#include <E_Nm.h>
#include <Ecore_Data.h>

#include <unistd.h>
#include <string.h>

E_NM *nm = NULL;
E_NMS *nms = NULL;

static void *
memdup(const void *p, size_t n)
{
    void *q;

    q = malloc(n);
    memcpy(q, p, n);
    return q;
}

static void
dump_variant(E_NM_Variant *var)
{
    if (!var) return;
    switch (var->type)
    {
        case 'a': {
          E_NM_Variant *subvar;

          printf("a:");
          ecore_list_first_goto(var->a);
          while ((subvar = ecore_list_next(var->a)))
          {
              dump_variant(subvar);
              printf(";");
          }
          printf("\n");
          break;
        }
        case 's':
            printf("s:%s", var->s);
            break;
        case 'o':
            printf("o:%s", var->s);
            break;
        case 'u':
            printf("u:%d", var->u);
            break;
        case 'b':
            printf("b:%d", var->b);
            break;
        case 'y':
            printf("y:%d", var->y);
            break;
        case 't':
            printf("t:%lld", var->t);
            break;
    }
}

static Eina_Bool
dump_values(const Eina_Hash *hash, const void *key, void *value, void *data)
{
    printf(" - name: %s - ", (char *)key);
    dump_variant(value);
    printf("\n");
}
 
static Eina_Bool
dump_settings(const Eina_Hash *hash, const void *key, void *value, void *fdata)
{
    printf("name: %s\n", (char *)key);
    printf("values:\n");
    eina_hash_foreach(value, dump_values, NULL);
    printf("\n");
}

static int
cb_nms_connection_secrets(void *data, Ecore_Hash *secrets)
{
    printf("Secrets:\n");
    if (secrets)
        eina_hash_foreach(secrets, dump_settings, NULL);
    return 1;
}

static int
cb_nms_connection_settings(void *data, Ecore_Hash *settings)
{
    printf("Settings:\n");
    if (settings)
    {
        if (ecore_hash_get(settings, "802-11-wireless-security"))
            e_nms_connection_secrets_get_secrets(data, "802-11-wireless-security", NULL, 0, cb_nms_connection_secrets, NULL);
        eina_hash_foreach(settings, dump_settings, NULL);
    }
    return 1;
}

static int
cb_nms_connections(void *data, Ecore_List *list)
{
    E_NMS_Connection *conn;

    if (list)
    {
        ecore_list_first_goto(list);
        while ((conn = ecore_list_next(list)))
        {
            e_nms_connection_dump(conn);
            e_nms_connection_get_settings(conn, cb_nms_connection_settings, conn);
        }
        //ecore_list_destroy(list);
    }
    //ecore_main_loop_quit();
    //e_nms_list_connections(nms, cb_nms_connections, nms);
    return 1;
}

static int
cb_access_point(void *data, E_NM_Access_Point *ap)
{
    e_nm_access_point_dump(ap);
    e_nm_access_point_free(ap);
    return 1;
}

static int
cb_activate_connection(void *data, E_NM_Device *device)
{
    E_NM_Active_Connection *conn;

    conn = data;
    sleep(1);
    e_nm_active_connection_dump(conn);
    e_nm_activate_connection(nm, conn->service_name, conn->path, device, conn->specific_object);
    return 1;
}

static int
cb_active_connection(void *data, E_NM_Active_Connection *conn)
{
    const char *device;
    e_nm_deactivate_connection(nm, conn);
    ecore_list_first_goto(conn->devices);
    while ((device = ecore_list_next(conn->devices)))
         e_nm_device_get(nm, device, cb_activate_connection, conn);
    /*
    e_nm_active_connection_dump(conn);
    e_nm_active_connection_free(conn);
    */
    return 1;
}

static int
cb_ip4_config(void *data, E_NM_IP4_Config *config)
{
    e_nm_ip4_config_dump(config);
    e_nm_ip4_config_free(config);
    return 1;
}

static int
cb_access_points(void *data, Ecore_List *list)
{
    E_NM_Access_Point *ap;

    if (list)
    {
        ecore_list_first_goto(list);
        while ((ap = ecore_list_next(list)))
        {
            e_nm_access_point_dump(ap);
        }
        ecore_list_destroy(list);
    }
    return 1;
}

static int
cb_get_devices(void *data, Ecore_List *list)
{
    E_NM_Device *device;

    if (list)
    {
        ecore_list_first_goto(list);
        while ((device = ecore_list_next(list)))
        {
            e_nm_device_dump(device);
            if (device->device_type == E_NM_DEVICE_TYPE_WIRELESS)
            {
                /*
                e_nm_device_wireless_get_access_points(device, cb_access_points, NULL);
                e_nm_access_point_get(nm, device->wireless.active_access_point, cb_access_point, NULL);
                e_nm_ip4_config_get(nm, device->ip4_config, cb_ip4_config, NULL);
                */
            }
        }
        //ecore_list_destroy(list);
    }
    //ecore_main_loop_quit();
    return 1;
}

static int
cb_nms(void *data, E_NMS *reply)
{
    Ecore_Hash *settings, *values;
    E_NM_Variant variant;
    const char ssid[] = { };
    const char *bssids[] = { };

    settings = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(settings, free);
    ecore_hash_free_value_cb_set(settings, ECORE_FREE_CB(ecore_hash_destroy));
    /* connection */
    values = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(values, free);
    ecore_hash_free_value_cb_set(values, ECORE_FREE_CB(e_nm_variant_free));
    ecore_hash_set(settings, strdup("connection"), values);
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 't';
    variant.t = 1228201388;
    ecore_hash_set(values, strdup("timestamp"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("");
    ecore_hash_set(values, strdup("id"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("");
    ecore_hash_set(values, strdup("uuid"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("802-11-wireless");
    ecore_hash_set(values, strdup("type"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 'b';
    variant.b = 0;
    ecore_hash_set(values, strdup("autoconnect"), memdup(&variant, sizeof(E_NM_Variant)));
    /* 802-11-wireless */
    values = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(values, free);
    ecore_hash_free_value_cb_set(values, ECORE_FREE_CB(e_nm_variant_free));
    ecore_hash_set(settings, strdup("802-11-wireless"), values);
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("mode");
    ecore_hash_set(values, strdup("infrastructure"), memdup(&variant, sizeof(E_NM_Variant)));
    ecore_hash_set(values, strdup("ssid"), e_nm_variant_array_new('y', ssid, sizeof(ssid) / sizeof(ssid[0])));
    ecore_hash_set(values, strdup("seen-bssids"), e_nm_variant_array_new('s', bssids, sizeof(bssids) / sizeof(bssids[0])));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("802-11-wireless-security");
    ecore_hash_set(values, strdup("security"), memdup(&variant, sizeof(E_NM_Variant)));
    /* ipv4 */
    values = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(values, free);
    ecore_hash_free_value_cb_set(values, ECORE_FREE_CB(e_nm_variant_free));
    ecore_hash_set(settings, strdup("ipv4"), values);
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("auto");
    ecore_hash_set(values, strdup("method"), memdup(&variant, sizeof(E_NM_Variant)));
    /* 802-11-wireless-security */
    values = ecore_hash_new(ecore_str_hash, ecore_str_compare);
    ecore_hash_free_key_cb_set(values, free);
    ecore_hash_free_value_cb_set(values, ECORE_FREE_CB(e_nm_variant_free));
    ecore_hash_set(settings, strdup("802-11-wireless-security"), values);
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("none");
    ecore_hash_set(values, strdup("key-mgmt"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("open");
    ecore_hash_set(values, strdup("auth-alg"), memdup(&variant, sizeof(E_NM_Variant)));
    memset(&variant, 0, sizeof(E_NM_Variant));
    variant.type = 's';
    variant.s = strdup("");
    ecore_hash_set(values, strdup("wep-key0"), memdup(&variant, sizeof(E_NM_Variant)));

    nms = reply;
    e_nms_dump(nms);
    //ecore_hash_for_each_node(settings, dump_settings, NULL);
    //e_nms_system_add_connection(nms, settings);
    //sleep(1);
    e_nms_list_connections(nms, cb_nms_connections, nms);
    return 1;
}

static int
cb_nm(void *data, E_NM *reply)
{
    if (!reply)
    {
        ecore_main_loop_quit();
        return 1;
    }
    nm = reply;
    /*
    e_nm_wireless_enabled_set(nm, 1);
    if (nm->active_connections)
    {
        const char *conn;
        ecore_list_first_goto(nm->active_connections);
        while ((conn = ecore_list_next(nm->active_connections)))
            e_nm_active_connection_get(nm, conn, cb_active_connection, NULL);
    }
    */
    /*
    e_nm_get_devices(nm, cb_get_devices, nm);
    */
    e_nms_get(nm, E_NMS_CONTEXT_SYSTEM, cb_nms, NULL);
    return 1;
}
   

int 
main(int argc, char **argv)
{
    ecore_init();
    eina_init();
    e_dbus_init();
   
    if (!e_nm_get(cb_nm, (void *)0xdeadbeef))
    {
        printf("Error connecting to system bus. Is it running?\n");
        return 1;
    }
   
    ecore_main_loop_begin();
    e_nms_free(nms);
    e_nm_free(nm);
   
    e_dbus_shutdown();
    eina_shutdown();
    ecore_shutdown();
    return 0;
}
