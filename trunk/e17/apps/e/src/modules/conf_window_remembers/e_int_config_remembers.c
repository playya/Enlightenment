#include "e.h"

/* function protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _fill_remembers(E_Config_Dialog_Data *cfdata);
static void _cb_delete(void *data, void *data2);
static void _cb_list_change(void *data, Evas_Object *obj);

struct _E_Config_Dialog_Data 
{
   Evas_Object *list, *btn;
};

EAPI E_Config_Dialog *
e_int_config_remembers(E_Container *con, const char *params __UNUSED__) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "_config_remembers_dialog")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;

   cfd = e_config_dialog_new(con, _("Window Remembers"), "E", 
                             "_config_remembers_dialog", 
                             "enlightenment/window_remembers", 0, v, NULL);
   e_dialog_resizable_set(cfd->dia, 1);
   return cfd;
}

/* private functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ow;

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_frametable_add(evas, _("Window Remembers"), 0);

   ow = e_widget_button_add(evas, _("Delete Remember(s)"), NULL, _cb_delete, 
                            cfdata, NULL);
   cfdata->btn = ow;

   ow = e_widget_ilist_add(evas, 24, 24, NULL);
   cfdata->list = ow;
   e_widget_ilist_multi_select_set(ow, 1);
   e_widget_on_change_hook_set(ow, _cb_list_change, cfdata);
   _fill_remembers(cfdata);

   e_widget_frametable_object_append(of, cfdata->list, 0, 0, 1, 1, 1, 1, 1, 1);
   e_widget_frametable_object_append(of, cfdata->btn, 0, 1, 1, 1, 1, 1, 1, 0);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   e_widget_disabled_set(cfdata->btn, 1);
   return o;
}

static void 
_fill_remembers(E_Config_Dialog_Data *cfdata) 
{
   Evas *evas;
   Evas_List *l = NULL;
   int w = 0;

   evas = evas_object_evas_get(cfdata->list);
   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(cfdata->list);
   e_widget_ilist_clear(cfdata->list);

   for (l = e_config->remembers; l; l = l->next) 
     {
        E_Remember *rem = NULL;

        if (!(rem = l->data)) continue;

        /* Filter out E's own remember */
        if ((rem->name) && (!strcmp(rem->name, "E"))) continue;

        if (rem->name) 
          e_widget_ilist_append(cfdata->list, NULL, rem->name, NULL, rem, NULL);
        else if (rem->class) 
          e_widget_ilist_append(cfdata->list, NULL, rem->class, NULL, rem, NULL);
        else if (rem->title) 
          e_widget_ilist_append(cfdata->list, NULL, rem->title, NULL, rem, NULL);
        else if (rem->role) 
          e_widget_ilist_append(cfdata->list, NULL, rem->role, NULL, rem, NULL);             
     }

   e_widget_ilist_go(cfdata->list);
   e_widget_min_size_get(cfdata->list, &w, NULL);
   e_widget_min_size_set(cfdata->list, w, 200);
   e_widget_ilist_thaw(cfdata->list);
   edje_thaw();
   evas_event_thaw(evas);

   e_widget_disabled_set(cfdata->btn, 1);
}

static void 
_cb_delete(void *data, void *data2) 
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *l = NULL;
   int i = 0, changed = 0;

   if (!(cfdata = data)) return;
   for (i = 0, l = e_widget_ilist_items_get(cfdata->list); l; l = l->next, i++) 
     {
        E_Ilist_Item *item = NULL;
        E_Remember *rem = NULL;

        item = l->data;
        if ((!item) || (!item->selected)) continue;
        if (!(rem = e_widget_ilist_nth_data_get(cfdata->list, i))) continue;
        e_remember_unuse(rem);
        e_remember_del(rem);
        changed = 1;
     }

   if (changed) e_config_save_queue();
   if (1) evas_list_free(l);

   _fill_remembers(cfdata);
}

static void 
_cb_list_change(void *data, Evas_Object *obj) 
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;
   if (e_widget_ilist_selected_count_get(cfdata->list) < 1) 
     {
        e_widget_disabled_set(cfdata->btn, 1);
        return;
     }
   e_widget_disabled_set(cfdata->btn, 0);
}
