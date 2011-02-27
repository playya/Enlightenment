/*
 * Configuration dialog handling for XKB Switcher module, sync'd with Eet storage.
 */

#include <e.h>
#include "e_mod_main.h"
#include "e_mod_parse.h"

/* This is automatically typedef'd by E */

struct _E_Config_Dialog_Data 
{
    Evas *evas;
    Evas_Object *layout_list, *used_list, *model_list, *variant_list;
    Evas_Object *btn_add, *btn_del, *btn_up, *btn_down;
    Ecore_Timer *fill_delay;
    Eina_List *layouts_used;
};

/* Local prototypes */

static void        *_create_data (E_Config_Dialog *cfd);
static void         _free_data   (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data   (E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void _cb_add (void *data, void *data2 __UNUSED__);
static void _cb_del (void *data, void *data2 __UNUSED__);
static void _cb_up  (void *data, void *data2 __UNUSED__);
static void _cb_down(void *data, void *data2 __UNUSED__);

static Eina_Bool _cb_fill_delay   (void *data);
static void      _cb_layout_select(void *data);

static void _fill_used_list(E_Config_Dialog_Data *cfdata);

static int _cb_layouts_bylabel(const void *data1, const void *data2);
static int _cb_layouts_sort   (const void *data1, const void *data2);

/* External Functions */

E_Config_Dialog *e_int_config_xkbswitch_module(E_Container *con, const char *params) 
{
    E_Config_Dialog    *cfd = NULL;
    E_Config_Dialog_View *v = NULL;
    char buf[4096];

    if (e_config_dialog_find("XKB Switcher", "keyboard_and_mouse/xkbswitch")) return NULL;

    v = E_NEW(E_Config_Dialog_View, 1);
    if (!v) return NULL;

    v->create_cfdata        = _create_data;
    v->free_cfdata          = _free_data;
    v->basic.create_widgets = _basic_create;
    v->basic.apply_cfdata   = _basic_apply;

    /* Icon in the theme */
    snprintf(buf, sizeof(buf), "%s/e-module-xkbswitch.edj", xkbswitch_conf->module->dir);

    /* create our config dialog */
    cfd = e_config_dialog_new(
        con,
        D_("XKB Switcher Module"),
        "XKB Switcher", 
        "advanced/xkbswitch",
        buf, 0, v, NULL
    );

    e_dialog_resizable_set(cfd->dia, 1);
    xkbswitch_conf->cfd = cfd;
    return cfd;
}

/* Locals */

static void *_create_data(E_Config_Dialog *cfd)
{
    E_Config_Dialog_Data *cfdata = NULL;

    cfdata = E_NEW(E_Config_Dialog_Data, 1);
    _fill_data(cfdata);
    return cfdata;
}

static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
    xkbswitch_conf->cfd = NULL;
    E_FREE(cfdata);
}

static void _fill_data(E_Config_Dialog_Data *cfdata)
{
    /* Here, fill some stuff from config into cfdata */
}

static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
    /* Main toolbook that holds all the pages */
    Evas_Object *main_tbox = NULL;

    /* Available layouts page */
    Evas_Object *avb_box = NULL;
    Evas_Object *avb_lst = NULL;
    Evas_Object *avb_act_hbox = NULL;
    Evas_Object *avb_add = NULL;
    Evas_Object *avb_del = NULL;

    /* Used layouts page */
    Evas_Object *add_box = NULL;
    Evas_Object *add_lst = NULL;
    Evas_Object *add_act_hbox = NULL;
    Evas_Object *add_up = NULL;
    Evas_Object *add_down = NULL;

    /* Available models page */
    Evas_Object *mdl_lst = NULL;

    /* Available variants page */
    Evas_Object *vrn_lst = NULL;

    /* Create widgets */

    /* The toolbook */
    main_tbox = e_widget_toolbook_add(evas, 24, 24);

    /* Available page */
    avb_box = e_widget_list_add(evas, 0, 0);

    avb_lst = e_widget_ilist_add(evas, 32, 32, NULL);
    e_widget_ilist_multi_select_set(avb_lst, EINA_TRUE);
    e_widget_size_min_set(avb_lst, 220, 160);
    e_widget_ilist_go(avb_lst);
    e_widget_list_object_append(avb_box, avb_lst, 1, 1, 0.5);

    avb_act_hbox = e_widget_list_add(evas, 0, 1);

    avb_add = e_widget_button_add(
        evas, D_("Add"), "list-add", 
        _cb_add, cfdata, NULL
    );
    e_widget_disabled_set(avb_add, EINA_TRUE);
    e_widget_list_object_append(avb_act_hbox, avb_add, 1, 1, 0.5);

    avb_del = e_widget_button_add(
        evas, D_("Remove"), "list-remove", 
        _cb_del, cfdata, NULL
    );
    e_widget_disabled_set(avb_del, EINA_TRUE);
    e_widget_list_object_append(avb_act_hbox, avb_del, 1, 1, 0.5);

    e_widget_list_object_append(avb_box, avb_act_hbox, 1, 0, -1);

    e_widget_toolbook_page_append(
        main_tbox,
        NULL,
        D_("Available"),
        avb_box,
        1, 1, 1, 1,
        0.5, 0.0
    );

    /* Used page */
    add_box = e_widget_list_add(evas, 0, 0);

    add_lst = e_widget_ilist_add(evas, 32, 32, NULL);
    e_widget_size_min_set(add_lst, 220, 160);
    e_widget_ilist_go(add_lst);
    e_widget_list_object_append(add_box, add_lst, 1, 1, 0.5);

    add_act_hbox = e_widget_list_add(evas, 0, 1);

    add_up = e_widget_button_add(
        evas, D_("Up"), "go-up", 
        _cb_up, cfdata, NULL
    );
    e_widget_disabled_set(add_up, EINA_TRUE);
    e_widget_list_object_append(add_act_hbox, add_up, 1, 1, 0.5);

    add_down = e_widget_button_add(
        evas, D_("Down"), "go-down", 
        _cb_down, cfdata, NULL
    );
    e_widget_disabled_set(add_down, EINA_TRUE);
    e_widget_list_object_append(add_act_hbox, add_down, 1, 1, 0.5);

    e_widget_list_object_append(add_box, add_act_hbox, 1, 0, -1);
    
    e_widget_toolbook_page_append(
        main_tbox,
        NULL,
        D_("Used"),
        add_box,
        1, 1, 1, 1,
        0.5, 0.0
    );

    /* Models page */
    mdl_lst = e_widget_ilist_add(evas, 32, 32, NULL);
    e_widget_toolbook_page_append(
        main_tbox,
        NULL,
        D_("Model"),
        mdl_lst,
        1, 1, 1, 1,
        0.5, 0.0
    );

    /* Variants page */
    vrn_lst = e_widget_ilist_add(evas, 32, 32, NULL);
    e_widget_toolbook_page_append(
        main_tbox,
        NULL,
        D_("Variant"),
        vrn_lst,
        1, 1, 1, 1,
        0.5, 0.0
    );

    /* Set default page */
    e_widget_toolbook_page_show(main_tbox, 0);

    /* Expose ilists outside */
    cfdata->layout_list  = avb_lst;
    cfdata->used_list    = add_lst;
    cfdata->model_list   = mdl_lst;
    cfdata->variant_list = vrn_lst;
    /* Expose buttons outside */
    cfdata->btn_add  = avb_add;
    cfdata->btn_del  = avb_del;
    cfdata->btn_up   = add_up;
    cfdata->btn_down = add_down;
    /* Expose evas */
    cfdata->evas = evas;

    if (cfdata->fill_delay) ecore_timer_del(cfdata->fill_delay);
    cfdata->fill_delay = ecore_timer_add(0.2, _cb_fill_delay, cfdata);

    return main_tbox;
}

static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
    /* Here, set stuff from cfdata back into config */
    e_config_save_queue();
    return 1;
}

static void _cb_add(void *data, void *data2 __UNUSED__)
{
    E_Config_Dialog_Data *cfdata = NULL;
    const E_Ilist_Item *it = NULL;
    e_xkb_layout *layout = NULL;
    Evas_Object *end = NULL;
    Eina_List *l = NULL;

    if (!(cfdata = data)) return;
    EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->layout_list), l, it)
    {
        if (!it->selected || it->header) continue;
        if (!(layout = eina_list_search_unsorted(layouts, _cb_layouts_bylabel, it->label))) continue;
        if (!eina_list_search_unsorted(cfdata->layouts_used, _cb_layouts_sort, layout))
        {
            end = e_widget_ilist_item_end_get(it);
            if (end) edje_object_signal_emit(end, "e,state,checked", "e");
            cfdata->layouts_used = eina_list_append(cfdata->layouts_used, layout);
        }
    }

    e_widget_ilist_unselect(cfdata->layout_list);
    e_widget_disabled_set(cfdata->btn_add, EINA_TRUE);
    e_widget_disabled_set(cfdata->btn_del, EINA_TRUE);

    _fill_used_list(cfdata);
}

static void _cb_del(void *data, void *data2 __UNUSED__)
{
    E_Config_Dialog_Data *cfdata = NULL;
    const E_Ilist_Item *it = NULL;
    e_xkb_layout *layout = NULL;
    Evas_Object *end = NULL;
    Eina_List *l = NULL;

    if (!(cfdata = data)) return;
    EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->layout_list), l, it)
    {
        if (!it->selected || it->header) continue;
        if ((layout = eina_list_search_unsorted(cfdata->layouts_used, _cb_layouts_bylabel, it->label)))
        {
            end = e_widget_ilist_item_end_get(it);
            if (end) edje_object_signal_emit(end, "e,state,unchecked", "e");
            cfdata->layouts_used = eina_list_remove(cfdata->layouts_used, layout);
        }
    }

    e_widget_ilist_unselect(cfdata->layout_list);
    e_widget_disabled_set(cfdata->btn_add, EINA_TRUE);
    e_widget_disabled_set(cfdata->btn_del, EINA_TRUE);

    _fill_used_list(cfdata);
}

static void _cb_up(void *data, void *data2 __UNUSED__)
{
    E_Config_Dialog_Data *cfdata = NULL;
    if (!(cfdata = data)) return;
    return;
}

static void _cb_down(void *data, void *data2 __UNUSED__)
{
    E_Config_Dialog_Data *cfdata = NULL;
    if (!(cfdata = data)) return;
    return;
}

static Eina_Bool _cb_fill_delay(void *data)
{
    E_Config_Dialog_Data *cfdata = NULL;
    Eina_List *l = NULL;
    Evas_Object *ic = NULL;
    Evas_Object *end = NULL;
    e_xkb_layout *layout = NULL;
    char buf[4096];

    if (!(cfdata = data)) return ECORE_CALLBACK_RENEW;

    EINA_LIST_FOREACH(layouts, l, layout)
    {
        ic = e_icon_add(cfdata->evas);
        snprintf(
            buf, sizeof(buf),
            "%s/flags/%s_flag.png",
            e_module_dir_get(xkbswitch_conf->module),
            layout->short_descr
        );
        if (!ecore_file_exists(buf))
            snprintf(
                buf, sizeof(buf),
                "%s/flags/unknown_flag.png", 
                e_module_dir_get(xkbswitch_conf->module)
            );
        e_icon_file_set(ic, buf);

        end = edje_object_add(cfdata->evas);
        if (!e_theme_edje_object_set(
            end, "base/theme/widgets",
            "e/widgets/ilist/toggle_end"
        ))
        {
            evas_object_del(end);
            end = NULL;
        }

        if (eina_list_search_unsorted(
            cfdata->layouts_used,
            _cb_layouts_sort,
            layout
        )) if (end)
            edje_object_signal_emit(end, "e,state,checked", "e");
        else if (end)
            edje_object_signal_emit(end, "e,state,unchecked", "e");

        snprintf(buf, sizeof(buf), "%s (%s)", layout->description, layout->short_descr);
        e_widget_ilist_append_full(
            cfdata->layout_list,
            ic, end, buf,
            _cb_layout_select,
            cfdata,
            layout->name
        );
        
    }

    e_widget_ilist_go(cfdata->layout_list);

    const E_Ilist_Item *it;
    EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->layout_list), l, it)
    {
        Evas_Object *end = e_widget_ilist_item_end_get(it);
        if (end) edje_object_signal_emit(end, "e,state,checked", "e");
    }

    cfdata->fill_delay = NULL;
    return ECORE_CALLBACK_CANCEL;
}

static void _fill_used_list(E_Config_Dialog_Data *cfdata)
{
}

static void _cb_layout_select(void *data)
{
    E_Config_Dialog_Data *cfdata = NULL;
    const E_Ilist_Item *it = NULL;
    Eina_List *l = NULL;
    unsigned int enabled = 0, disabled = 0;

    if (!(cfdata = data)) return;
    EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->layout_list), l, it)
    {
        if (!it->selected || it->header) continue;
        if (eina_list_search_unsorted(cfdata->layouts_used, _cb_layouts_bylabel, it->label))
            enabled++;
        else
            disabled++;
    }

    e_widget_disabled_set(cfdata->btn_add, !disabled);
    e_widget_disabled_set(cfdata->btn_del, !enabled);
}

static int _cb_layouts_bylabel(const void *data1, const void *data2)
{
    const e_xkb_layout *l1 = NULL;
    const char *l2 = NULL;
    char buf[128];

    if (!(l1 = data1)) return 1;
    if (!l1->name) return 1;
    if (!(l2 = data2)) return -1;

    /* XXX This is nasty, but it's definitely better than iterating
     * over the whole list. User-defined property for ilist item would
     * solve, but E widget system lacks this currently.
     */
    snprintf(buf, sizeof(buf), "%s (%s)", l1->description, l1->short_descr);
    return strcmp(buf, l2);
}

static int _cb_layouts_sort(const void *data1, const void *data2)
{
    const e_xkb_layout *l1 = NULL, *l2 = NULL;

    if (!(l1 = data1)) return 1;
    if (!l1->name) return 1;
    if (!(l2 = data2)) return -1;
    if (!l2->name) return -1;
    return strcmp(l1->name, l2->name);
}
