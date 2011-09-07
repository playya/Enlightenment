#ifndef E_MOD_TILING_H
#define E_MOD_TILING_H
#include <libintl.h>
#define D_(str) dgettext(PACKAGE, str)

#include <e.h>
#include <e_border.h>
#include <e_shelf.h>

#include <stdbool.h>

#include "config.h"

typedef struct _Config      Config;
typedef struct _Tiling_Info Tiling_Info;

struct tiling_g
{
   E_Module *module;
   Config   *config;
   int       log_domain;
};
extern struct tiling_g tiling_g;

#define ERR(...) EINA_LOG_DOM_ERR(tiling_g.log_domain, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(tiling_g.log_domain, __VA_ARGS__)

#define TILING_MAX_COLUMNS 8

struct _Config_vdesk
{
   int           x, y;
   unsigned int  zone_num;
   int           nb_cols;
};

struct _Config
{
    int            tile_dialogs;
    int            show_titles;
    Eina_List     *vdesks;
};

struct _Tiling_Info
{
    /* The desk for which this _Tiling_Info is used. Needed because
     * (for example) on e restart all desks are shown on all zones but no
     * change events are triggered */
    const E_Desk    *desk;

    struct _Config_vdesk *conf;

    /* List of windows which were toggled floating */
    Eina_List *floating_windows;

    Eina_List *columns[TILING_MAX_COLUMNS];
    int              x[TILING_MAX_COLUMNS];
    int              w[TILING_MAX_COLUMNS];

    int borders;
};

struct _E_Config_Dialog_Data
{
   struct _Config config;
   Evas_Object *o_zonelist;
   Evas_Object *o_desklist;
   Evas_Object *o_deskscroll;
   Evas_Object *o_space_between;
   Evas        *evas;
};

E_Config_Dialog *e_int_config_tiling_module(E_Container *con,
                                            const char  *params);

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init(E_Module *m);
EAPI int   e_modapi_shutdown(E_Module *m);
EAPI int   e_modapi_save(E_Module *m);

void change_column_number(struct _Config_vdesk *newconf);

void e_tiling_update_conf(void);

struct _Config_vdesk *
get_vdesk(Eina_List *vdesks,
          int x,
          int y,
          unsigned int zone_num);

#define EINA_LIST_IS_IN(_list, _el) \
    (eina_list_data_find(_list, _el) == _el)
#define EINA_LIST_APPEND(_list, _el) \
    _list = eina_list_append(_list, _el)
#define EINA_LIST_REMOVE(_list, _el) \
    _list = eina_list_remove(_list, _el)

#endif
