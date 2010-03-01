/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _cb_disable_check_list(void *data, Evas_Object *obj);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   int use_auto_raise;
   int allow_above_fullscreen;
   /*- ADVANCED -*/
   double auto_raise_delay;
   int border_raise_on_mouse_action;
   int border_raise_on_focus;
};

Eina_List *autoraise_list = NULL;

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_window_stacking(E_Container *con, const char *params __UNUSED__)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "_config_window_stacking_dialog")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Window Stacking"),
			     "E", "_config_window_stacking_dialog",
			     "preferences-window-stacking", 0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->use_auto_raise = e_config->use_auto_raise;
   cfdata->allow_above_fullscreen = e_config->allow_above_fullscreen;
   cfdata->auto_raise_delay = e_config->auto_raise_delay;
   cfdata->border_raise_on_mouse_action = e_config->border_raise_on_mouse_action;
   cfdata->border_raise_on_focus = e_config->border_raise_on_focus;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   autoraise_list = eina_list_free(autoraise_list);

   /* Free the cfdata */
   E_FREE(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_config->use_auto_raise = cfdata->use_auto_raise;
   e_config->allow_above_fullscreen = cfdata->allow_above_fullscreen;
   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_config->use_auto_raise = cfdata->use_auto_raise;
   e_config->allow_above_fullscreen = cfdata->allow_above_fullscreen;
   e_config->auto_raise_delay = cfdata->auto_raise_delay;
   e_config->border_raise_on_mouse_action = cfdata->border_raise_on_mouse_action;
   e_config->border_raise_on_focus = cfdata->border_raise_on_focus;
   e_config_save_queue();
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ob;

   o = e_widget_list_add(evas, 0, 0);

   ob = e_widget_check_add(evas, _("Automatically raise windows on mouse over"), 
                           &(cfdata->use_auto_raise));
   e_widget_list_object_append(o, ob, 1, 0, 0.5);

   ob = e_widget_check_add(evas, _("Allow windows to be above fullscreen window"), 
                           &(cfdata->allow_above_fullscreen));
   e_widget_list_object_append(o, ob, 1, 0, 0.5);

   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o, *ob, *of;
   Evas_Object *autoraise_check;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Autoraise"), 0);
   autoraise_check = e_widget_check_add(evas, _("Automatically raise windows on mouse over"), 
                                        &(cfdata->use_auto_raise));
   e_widget_framelist_object_append(of, autoraise_check);
   ob = e_widget_label_add(evas, _("Delay before raising:"));
   autoraise_list = eina_list_append (autoraise_list, ob);
   e_widget_disabled_set(ob, !cfdata->use_auto_raise); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f sec"), 0.0, 9.9, 0.1, 0, 
                            &(cfdata->auto_raise_delay), NULL, 200);
   autoraise_list = eina_list_append (autoraise_list, ob);
   e_widget_disabled_set(ob, !cfdata->use_auto_raise); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 0, 0.5);
   // handler for enable/disable widget array
   e_widget_on_change_hook_set(autoraise_check, 
                               _cb_disable_check_list, autoraise_list);

   of = e_widget_framelist_add(evas, _("Raise Window"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   ob = e_widget_check_add(evas, _("Raise when starting to move or resize"), 
                           &(cfdata->border_raise_on_mouse_action));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Raise when clicking to focus"), 
                           &(cfdata->border_raise_on_focus));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Allow windows to be above fullscreen window"), 
                           &(cfdata->allow_above_fullscreen));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 0, 0.5);

   return o;
}

/*!
 *  * If the check is disabled then disable the chained objects in the list.
 *
 * @param data A Eina_List of Evas_Object to chain widgets together with the checkbox
 * @param obj A Evas_Object checkbox created with e_widget_check_add()
 */
static void
_cb_disable_check_list(void *data, Evas_Object *obj)
{
   Eina_List *list = (Eina_List*) data;
   Eina_List *l;
   Evas_Object *o;

   EINA_LIST_FOREACH(list, l, o)
     e_widget_disabled_set(o, !e_widget_check_checked_get(obj));
}
