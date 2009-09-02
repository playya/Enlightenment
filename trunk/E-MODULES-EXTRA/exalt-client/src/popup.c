// Author:  (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>

#include "e_mod_main.h"

void popup_init(Instance* inst)
{
    inst->popup = NULL;
}

void popup_create(Instance* inst)
{
    Evas_Object *base, *ilist, *button, *o;
    Evas *evas;
    Evas_Coord mw, mh;

    inst->popup = e_gadcon_popup_new(inst->gcc);/*, NULL popup_cb_resize );*/
    evas = inst->popup->win->evas;

    edje_freeze();

    base = e_widget_table_add(evas, 0);

    o = edje_object_add(evas);
    e_theme_edje_object_set(o, "base/theme/modules/exalt",
            "e/modules/exalt/network");
    edje_object_size_min_get(o, &mw, &mh);
    if ((mw < 1) || (mh < 1)) edje_object_size_min_calc(o, &mw, &mh);
    if (mw < 20) mw = 20;
    if (mh < 20) mh = 20;
    evas_object_del(o);
    ilist = e_widget_ilist_add(evas, mw, mh, NULL);
    inst->popup_ilist_obj = ilist;

    e_widget_ilist_freeze(ilist);

    e_widget_ilist_go(ilist);
    e_widget_ilist_thaw(ilist);

    e_widget_size_min_set(ilist, 240, 250);
    e_widget_table_object_append(base, ilist,
            0, 0, 1, 1, 1, 1, 1, 1);

    button = e_widget_button_add(evas, D_("DNS configuration"), NULL, popup_cb_dns,inst, NULL);
    e_widget_table_object_append(base, button,
            0, 1, 1, 1, 1, 1, 1, 1);

    button = e_widget_button_add(evas, D_("Network configuration"), NULL, popup_cb_network,inst, NULL);
    e_widget_table_object_append(base, button,
            0, 2, 1, 1, 1, 1, 1, 1);

    //Some elive extra buttons
#ifdef ELIVE
    button = e_widget_button_add(evas, D_("Modem dialer"), NULL, popup_cb_elive_modem,inst, NULL);
    e_widget_table_object_append(base, button,
            0, 3, 1, 1, 1, 1, 1, 1);

    button = e_widget_button_add(evas, D_("Mobile Phone (3G)"), NULL, popup_cb_elive_mobile_phone,inst, NULL);
    e_widget_table_object_append(base, button,
            0, 4, 1, 1, 1, 1, 1, 1);
#endif

    edje_thaw();

    e_gadcon_popup_content_set(inst->popup, base);

    exalt_dbus_eth_list_get(inst->conn);
    exalt_dbus_wireless_list_get(inst->conn);
}

void popup_cb_dns(void *data, void *data2)
{
    Instance *inst = data;

    popup_hide(inst);
    dns_dialog_show(inst);
}

void popup_cb_network(void *data, void *data2)
{
    Instance *inst = data;

    popup_hide(inst);
    network_conf_dialog_show(inst);
}

#ifdef ELIVE
void popup_cb_elive_modem(void *data, void *data2)
{
    Instance *inst = data;

    Ecore_Exe *exe = ecore_exe_run ("gnome-ppp &", NULL);
    popup_hide(inst);
}


void popup_cb_elive_mobile_phone(void *data, void *data2)
{
    Instance *inst = data;

    Ecore_Exe *exe = ecore_exe_run ("3g-dialer & ", NULL);
    popup_hide(inst);
}
#endif

void popup_iface_remove(Instance *inst, const char*  iface)
{
    Eina_List *l;
    int iface_pos;
    Popup_Elt *elt;

    if(!inst->popup_ilist_obj)
        return;

    iface_pos = 0;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt->type == POPUP_IFACE
                && strcmp(elt->iface,iface) == 0)
            break;
        else
            iface_pos++;
    }

    if(!elt)
        return;

    if(elt->iface_type==IFACE_WIRELESS)
    {
        int id_first,id_last;
        Eina_List* first,*last;
        Eina_List*l;
        Popup_Elt *elt_n;

        if(elt->scan_timer)
        {
            ecore_timer_del(elt->scan_timer);
            elt->scan_timer = NULL;
        }

        popup_network_interval_get(inst,iface,&id_first,&id_last,
                &first,&last);

        l=eina_list_next(first);
        while(l)
        {
            elt_n = eina_list_data_get(l);
            if(elt_n->type==POPUP_NETWORK)
            {
                l=eina_list_next(l);

                inst->l = eina_list_remove(inst->l,elt_n);
                elt_n->nb_use--;
                if(elt_n->nb_use==0)
                    popup_elt_free(elt_n);
                e_widget_ilist_remove_num(inst->popup_ilist_obj,
                        iface_pos+1);
            }
            else
                break;
        }
    }

    inst->l = eina_list_remove(inst->l,elt);
    elt->nb_use--;
    if(elt->nb_use==0)
        popup_elt_free(elt);

    e_widget_ilist_remove_num(inst->popup_ilist_obj,iface_pos);
    e_widget_ilist_go(inst->popup_ilist_obj);
    e_widget_ilist_thaw(inst->popup_ilist_obj);
}

    void
popup_iface_add(Instance* inst, const char* iface, Iface_Type iface_type)
{
    Evas_Object *icon;
    Popup_Elt* elt;
    char buf[1024];

    if(!inst->popup_ilist_obj)
        return;

    icon = edje_object_add(evas_object_evas_get(inst->popup_ilist_obj));
    snprintf(buf,1024,"%s/e-module-exalt.edj",exalt_conf->module->dir);
    switch(iface_type)
    {
        case IFACE_WIRED:
            edje_object_file_set(icon, buf,"modules/exalt/icons/wired");
            break;
        case IFACE_WIRELESS:
            edje_object_file_set(icon, buf,"modules/exalt/icons/wireless");
            break;
    }
    evas_object_show(icon);

    elt = calloc(1,sizeof(Popup_Elt));
    elt->inst = inst;
    elt->iface = strdup(iface);
    elt->type = POPUP_IFACE;
    elt->iface_type = iface_type;
    elt->icon = icon;
    elt->nb_use++;

    inst->l = eina_list_append(inst->l,elt);
    popup_iface_label_create(elt,buf,1024,NULL);
    e_widget_ilist_append(inst->popup_ilist_obj, icon, buf,
            popup_cb_ifnet_sel , elt, NULL);

    e_widget_ilist_go(inst->popup_ilist_obj);
    e_widget_ilist_thaw(inst->popup_ilist_obj);

    popup_icon_update(inst,iface);

    exalt_dbus_eth_ip_get(inst->conn,iface);
    exalt_dbus_eth_up_is(inst->conn,iface);
    exalt_dbus_eth_link_is(inst->conn,iface);
    exalt_dbus_eth_connected_is(inst->conn, iface);

    if(iface_type == IFACE_WIRELESS)
    {
        exalt_dbus_wireless_essid_get(inst->conn, elt->iface);
        exalt_dbus_wireless_scan(inst->conn,elt->iface);
    }
}

void popup_iface_label_create(Popup_Elt *elt, char *buf, int buf_size, char* ip)
{
    if(!ip)
        ip = D_("No IP Address");
    switch(elt->iface_type)
    {
        case IFACE_WIRED:
            snprintf(buf,buf_size,"%s (%s)",D_("Wired interface"),ip);
            break;
        case IFACE_WIRELESS:
            snprintf(buf,buf_size,"%s (%s)",D_("Wireless interface"),ip);
            break;
    }
}

void popup_cb_ifnet_sel(void *data)
{
    Popup_Elt *elt = data;
    Instance* inst = elt->inst;

    switch(elt->type)
    {
        case POPUP_IFACE:
            switch(elt->iface_type)
            {
                case IFACE_WIRED:
                    if(!exalt_conf->mode)
                    {
                        if_wired_dialog_basic_show(inst);
                        if_wired_dialog_basic_set(inst,elt);
                        if_wired_dialog_hide(inst);
                    }
                    else
                    {
                        if_wired_dialog_show(inst);
                        if_wired_dialog_set(inst,elt);
                        if_wired_dialog_basic_hide(inst);
                    }
                    if_network_dialog_hide(inst);
                    if_wireless_dialog_hide(inst);
                    if_network_dialog_basic_hide(inst);
                    break;
                case IFACE_WIRELESS:
                    if_wireless_dialog_show(inst);
                    if_wireless_dialog_set(inst,elt);
                    if_network_dialog_hide(inst);
                    if_wired_dialog_hide(inst);
                    if_network_dialog_basic_hide(inst);
                    if_wired_dialog_basic_hide(inst);
                    break;
            }
            break;
        case POPUP_NETWORK:
            if(!exalt_conf->mode)
            {
                if_network_dialog_basic_show(inst);
                if_network_dialog_basic_set(inst,elt);
                if_network_dialog_hide(inst);
            }
            else
            {
                if_network_dialog_show(inst);
                if_network_dialog_set(inst,elt);
                if_network_dialog_basic_hide(inst);
            }
            if_wired_dialog_hide(inst);
            if_wired_dialog_basic_hide(inst);
            if_wireless_dialog_hide(inst);
            break;
    }
    network_conf_dialog_hide(inst);
    if_network_dialog_new_hide(inst);
    popup_hide(inst);
}

    void
popup_ip_update(Instance* inst, char* iface, char* ip)
{
    int i = 0;
    char buf[1024];
    Popup_Elt* elt;
    Eina_List *l;

    if(!inst->popup_ilist_obj || !iface)
        return;

    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            if(elt->iface_type == IFACE_WIRED)
            {
                popup_iface_label_create(elt,buf,1024,ip);
                e_widget_ilist_nth_label_set(inst->popup_ilist_obj,i,buf);
            }
            break;
        }
        i++;
    }
}

void popup_up_update(Instance* inst, char* iface, int is_up)
{
    int i;
    Popup_Elt* elt;
    Eina_List *l;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            elt->is_up = is_up;
            break;
        }
    }
    popup_icon_update(inst,iface);
}

void popup_link_update(Instance* inst, char* iface, int is_link)
{
    Popup_Elt* elt;
    Eina_List *l;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            elt->is_link = is_link;
            break;
        }
    }
    popup_icon_update(inst,iface);
}

void popup_connected_update(Instance* inst, char* iface, int is_connected)
{
    Popup_Elt* elt;
    Eina_List *l;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && elt->iface_type == IFACE_WIRELESS
                && strcmp(elt->iface,iface)==0)
        {
            elt->is_connected = is_connected;
            break;
        }
    }
    popup_icon_update(inst,iface);
}

void popup_essid_update(Instance* inst, char* iface, char* essid)
{
    int i = 0;
    char buf[1024];
    Popup_Elt* elt;
    Eina_List *l;

    if(!inst->popup_ilist_obj || !iface)
        return;


    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            popup_iface_label_create(elt,buf,1024,essid);
            e_widget_ilist_nth_label_set(inst->popup_ilist_obj,i,buf);
            break;
        }
        i++;
    }
}

void popup_icon_update(Instance* inst, const char* iface)
{
    char* group;
    char buf[1024];
    Popup_Elt* elt;
    Eina_List *l;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            if(!elt->is_link)
                edje_object_signal_emit(elt->icon,"notLink","exalt");
            else if(!elt->is_up)
                edje_object_signal_emit(elt->icon,"notActivate","exalt");
            else if(!elt->is_connected && elt->iface_type == IFACE_WIRELESS)
                edje_object_signal_emit(elt->icon,"notConnected","exalt");
            else
                edje_object_signal_emit(elt->icon,"default","exalt");
            break;
        }
    }
}

void popup_update(Instance* inst, Exalt_DBus_Response* response)
{
    const char* iface;

    if(!inst->popup_ilist_obj)
        return;

    switch(exalt_dbus_response_type_get(response))
    {
        case EXALT_DBUS_RESPONSE_IFACE_WIRED_LIST:
            {
                Eina_List *l2;
                Eina_List* l = exalt_dbus_response_list_get(response);
                EINA_LIST_FOREACH(l,l2,iface)
                    popup_iface_add(inst,iface,IFACE_WIRED);
            }
            break;
        case EXALT_DBUS_RESPONSE_IFACE_WIRELESS_LIST:
            {
                Eina_List *l2;
                Eina_List* l = exalt_dbus_response_list_get(response);
                EINA_LIST_FOREACH(l,l2,iface)
                    popup_iface_add(inst,iface,IFACE_WIRELESS);
            }
            break;
        case EXALT_DBUS_RESPONSE_IFACE_WIRELESS_IS:
            if(exalt_dbus_response_is_get(response))
            {
                Eina_List *l;
                Popup_Elt *elt;
                iface = exalt_dbus_response_iface_get(response);
                EINA_LIST_FOREACH(inst->l,l,elt)
                    if( elt->type==POPUP_IFACE
                             && strcmp(elt->iface,iface)==0)
                        break;

                if(elt)
                {
                    char buf[1024];
                    elt->iface_type = IFACE_WIRELESS;
                    snprintf(buf,1024,"%s/e-module-exalt.edj",
                            exalt_conf->module->dir);
                    edje_object_file_set(elt->icon, buf
                            ,"modules/exalt/icons/wireless");
                    exalt_dbus_eth_ip_get(inst->conn,iface);
                    exalt_dbus_eth_up_is(inst->conn,iface);
                    exalt_dbus_wireless_essid_get(inst->conn, iface);
                    exalt_dbus_wireless_scan(inst->conn,iface);
                }
            }
            break;
        case EXALT_DBUS_RESPONSE_IFACE_IP_GET:
            popup_ip_update(inst,exalt_dbus_response_iface_get(response),
                    exalt_dbus_response_address_get(response));
            break;
        case EXALT_DBUS_RESPONSE_IFACE_UP_IS:
            popup_up_update(inst,exalt_dbus_response_iface_get(response),
                    exalt_dbus_response_is_get(response));
            break;
        case EXALT_DBUS_RESPONSE_IFACE_LINK_IS:
            popup_link_update(inst,exalt_dbus_response_iface_get(response),
                    exalt_dbus_response_is_get(response));
            break;
        case EXALT_DBUS_RESPONSE_IFACE_CONNECTED_IS:
            popup_connected_update(inst,exalt_dbus_response_iface_get(response),
                    exalt_dbus_response_is_get(response));
            break;
        case EXALT_DBUS_RESPONSE_WIRELESS_ESSID_GET:
            popup_essid_update(inst, exalt_dbus_response_iface_get(response),
                    exalt_dbus_response_string_get(response));
            break;
        default: ;
    }

    e_widget_ilist_go(inst->popup_ilist_obj);
    e_widget_ilist_thaw(inst->popup_ilist_obj);
}


void popup_network_interval_get(Instance* inst, const char* iface, int *id_first, int* id_last, Eina_List** first, Eina_List** last)
{
    int i=0;
    char buf[1024];

    *id_first = -1;
    *id_last = -1;

    *first = NULL;
    *last = NULL;

    Popup_Elt* elt;
    Eina_List *l;
    EINA_LIST_FOREACH(inst->l,l,elt)
    {
        if(elt && elt->type == POPUP_IFACE && elt->iface
                && strcmp(elt->iface,iface)==0)
        {
            *id_first = i;
            *first = l;
            break;
        }
        i++;
    }

    i=*id_first+1;
    *id_last = *id_first;
    *last = *first;

    EINA_LIST_FOREACH(eina_list_next(l),l,elt)
    {
        if(elt && elt->type == POPUP_IFACE)
        {
            break;
        }
        else
        {
            *id_last = i;
            *last = l;
        }
        i++;
    }
}

void popup_iface_essid_create(Popup_Elt *elt, char *buf, int buf_size, int quality)
{
    snprintf(buf,buf_size,"(%d %%)    %s",quality,elt->essid);
}



void popup_notify_scan(char* iface, Eina_List* networks, void* user_data )
{
    Instance* inst = user_data;
    Exalt_Wireless_Network* w;
    Eina_List *l;
    int i;
    int id_first, id_last;
    Eina_List* first, *last;
    Popup_Elt* elt;

    if(!inst->popup_ilist_obj)
        return;

    popup_network_interval_get(inst,iface,&id_first,&id_last,&first,&last);
    l=first;
    do
    {
        elt = eina_list_data_get(l);
        elt->is_find--;
    }while(l!=last && (l = eina_list_next(l)) );

    EINA_LIST_FOREACH(networks,l,w)
    {
        Eina_List* l2;
        int find = -1;
        Popup_Elt* elt_find=NULL;
        int i =0;

        const char* essid = exalt_wireless_network_essid_get(w);
        l2=first;
        i=0;
        do
        {
            Popup_Elt* elt;
            elt = eina_list_data_get(l2);
            if(elt && elt->essid && essid && strcmp(elt->essid,essid)==0)
            {
                find = i;
                elt_find = elt;
            }
            i++;
        }while(l2!=last && !elt_find && (l2 = eina_list_next(l2)) );

        if(!elt_find)
        {
            //add a new network in the list
            Popup_Elt* elt;
            Evas_Object* icon;
            char buf[1024];


            icon = edje_object_add(evas_object_evas_get(inst->popup_ilist_obj));
            snprintf(buf,1024,"%s/e-module-exalt.edj",exalt_conf->module->dir);
            edje_object_file_set(icon, buf,"modules/exalt/icons/encryption");

            if(exalt_wireless_network_encryption_is(w))
                edje_object_signal_emit(icon,"visible,essid,new","exalt");
            else
                edje_object_signal_emit(icon,"invisible","exalt");

            evas_object_show(icon);

            elt = calloc(1,sizeof(Popup_Elt));
            elt->inst = inst;
            elt->iface = strdup(iface);
            elt->type = POPUP_NETWORK;
            elt->essid = strdup(essid);
            elt->icon = icon;
            elt->n = w;
            elt->is_find = 2;
            elt->nb_use++;

            popup_iface_essid_create(elt,buf,1024,exalt_wireless_network_quality_get(w));

            inst->l = eina_list_append_relative(inst->l,elt,eina_list_data_get(last));
            last = eina_list_next(last);


            e_widget_ilist_append_relative(inst->popup_ilist_obj,
                    icon, buf,popup_cb_ifnet_sel , elt, NULL,id_last);

            id_last++;
        }
        else
        {
            //update the network
            Popup_Elt* elt = elt_find;
            exalt_wireless_network_free(&(elt->n));
            elt->n = w;
            char buf[1024];

            if(exalt_wireless_network_encryption_is(w))
                edje_object_signal_emit(elt->icon,"visible","exalt");
            else
                edje_object_signal_emit(elt->icon,"invisible","exalt");


            elt->is_find = 2;
            popup_iface_essid_create(elt,buf,1024,exalt_wireless_network_quality_get(w));
            e_widget_ilist_nth_label_set(inst->popup_ilist_obj,find+id_first,buf);
        }
    }

    //remove old networks
    l=first;
    Eina_List* l_prev=NULL;
    int jump = 0;
    i=0;
    do
    {
        if(jump)
            jump = 0;

        Popup_Elt* elt;
        elt = eina_list_data_get(l);
        if(elt && elt->type == POPUP_NETWORK && !elt->is_find)
        {
            e_widget_ilist_remove_num(inst->popup_ilist_obj,i+id_first);
            l = l_prev;
            inst->l = eina_list_remove(inst->l,elt);
            elt->nb_use--;
            if(elt->nb_use==0)
                popup_elt_free(elt);
        }
        else
            i++;
        l_prev = l;
    }while(l!=last && (l = eina_list_next(l)) );

    /*Eina_List* l2;
    EINA_LIST_FOREACH(inst->l,l2,elt)
    {
        if(elt->essid)
            printf("%s\n",elt->essid);
    }*/



    elt = eina_list_data_get(first);
    if(elt->scan_timer) ecore_timer_del(elt->scan_timer);
    elt->scan_timer  = ecore_timer_add(2,popup_scan_timer_cb,elt);
}

int popup_scan_timer_cb(void *data)
{
    Popup_Elt* elt = data;

    if(!elt) return;

    if(elt->scan_timer)
        ecore_timer_del(elt->scan_timer);
    elt->scan_timer = NULL;

    exalt_dbus_wireless_scan(elt->inst->conn,elt->iface);
}

void popup_show(Instance* inst)
{
    int i;

    if(!inst->popup)
        popup_create(inst);

    e_gadcon_popup_show(inst->popup);
}

void popup_hide(Instance *inst)
{
    if (inst->popup)
    {
        Eina_List* l;
        Popup_Elt* elt;

        e_object_del(E_OBJECT(inst->popup));
        inst->popup = NULL;
        inst->popup_ilist_obj = NULL;

        EINA_LIST_FOREACH(inst->l,l,elt)
        {
            elt->nb_use--;
            if(elt->scan_timer)
            {
                ecore_timer_del(elt->scan_timer);
                elt->scan_timer = NULL;
            }
            popup_elt_free(elt);
        }
        eina_list_free(inst->l);
        inst->l = NULL;
    }
}

void popup_elt_free(Popup_Elt* elt)
{
    if(elt->nb_use>0)
        return ;

    EXALT_FREE(elt->iface);
    EXALT_FREE(elt->essid);
    if(elt->icon)
        evas_object_del(elt->icon);
    if(elt->n)
        exalt_wireless_network_free(&(elt->n));
    if(elt->scan_timer)
    {
        ecore_timer_del(elt->scan_timer);
        elt->scan_timer = NULL;
    }
    EXALT_FREE(elt);
}
