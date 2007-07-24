#ifdef E_TYPEDEFS
#else
#ifndef E_INT_CONFIG_CLIENTLIST_H
#define E_INT_CONFIG_CLIENTLIST_H

#define E_CLIENTLIST_GROUP_NONE 0
#define E_CLIENTLIST_GROUP_DESK 1
#define E_CLIENTLIST_GROUP_CLASS 2

#define E_CLIENTLIST_GROUP_SEP_NONE 0
#define E_CLIENTLIST_GROUP_SEP_BAR 1
#define E_CLIENTLIST_GROUP_SEP_MENU 2

#define E_CLIENTLIST_SORT_NONE 0  
#define E_CLIENTLIST_SORT_ALPHA 1
#define E_CLIENTLIST_SORT_ZORDER 2
#define E_CLIENTLIST_SORT_MOST_RECENT 3

#define E_CLIENTLIST_GROUPICONS_OWNER 0
#define E_CLIENTLIST_GROUPICONS_CURRENT 1
#define E_CLIENTLIST_GROUPICONS_SEP 2

#define E_CLIENTLIST_MAX_CAPTION_LEN 256

EAPI E_Config_Dialog *e_int_config_clientlist(E_Container *con, const char *params __UNUSED__);

#endif
#endif
