/*
 * =====================================================================================
 *
 *       Filename:  iface_list.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  24/03/09 21:38:24 CET
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  (Watchwolf), Atton Jonathan <watchwolf@watchwolf.fr>
 *        Company:
 *
 * =====================================================================================
 */

#ifndef  IFACE_LIST_INC
#define  IFACE_LIST_INC

#include "main.h"

typedef enum _Iface_List_Enum Iface_List_Enum;
typedef struct _Iface_List_Elt Iface_List_Elt;

enum _Iface_List_Enum
{
    ITEM_IFACE,
    ITEM_NETWORK
};

struct _Iface_List_Elt
{
    Iface_List_Enum type;

    char* iface;
    Iface_Type iface_type;

    Elm_Genlist_Item * item;

    char *ip;
    int is_link;
    int is_up;

    Evas_Object *box;
    Evas_Object *icon;
    Evas_Object *lbl_name;
    Evas_Object *lbl_ip;
};

Evas_Object* iface_list_new();
void iface_list_add(Evas_Object *l, const char* iface, Iface_Type type);
void iface_list_response(Evas_Object *l, Exalt_DBus_Response* response);
Elm_Genlist_Item* iface_list_get_elt_from_name(Evas_Object *list,char* iface);
void network_list_interval_get(Elm_Genlist_Item* list, const char* iface, int *id_first, int* id_last, Elm_Genlist_Item** first, Elm_Genlist_Item** last);

#endif   /* ----- #ifndef IFACE_LIST_INC  ----- */

