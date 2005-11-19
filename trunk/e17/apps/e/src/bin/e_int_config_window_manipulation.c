/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
typedef struct _CFData CFData;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, CFData *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _CFData
{
   /*- BASIC -*/
   int auto_raise;
   int resist;
   int maximize;
   /*- ADVANCED -*/
   int use_auto_raise;
   double auto_raise_delay;
   int use_resist;
   int desk_resist;
   int window_resist;
   int gadget_resist;
   int maximize_policy;
};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_window_manipulation(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   
   /* methods */
   v.create_cfdata           = _create_data;
   v.free_cfdata             = _free_data;
   v.basic.apply_cfdata      = _basic_apply_data;
   v.basic.create_widgets    = _basic_create_widgets;
   v.advanced.apply_cfdata   = _advanced_apply_data;
   v.advanced.create_widgets = _advanced_create_widgets;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Window Manipulation"), NULL, 0, &v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(CFData *cfdata)
{
   cfdata->use_auto_raise = e_config->use_auto_raise;
   cfdata->auto_raise_delay = e_config->auto_raise_delay;
   cfdata->use_resist = e_config->use_resist;
   cfdata->desk_resist = e_config->desk_resist;
   cfdata->window_resist = e_config->window_resist;
   cfdata->gadget_resist = e_config->gadget_resist;
   cfdata->maximize_policy = e_config->maximize_policy;
   if (cfdata->maximize_policy == E_MAXIMIZE_NONE)
     cfdata->maximize_policy = E_MAXIMIZE_FULLSCREEN;
   if (cfdata->use_auto_raise) cfdata->auto_raise = 1;
   if (cfdata->use_resist) cfdata->resist = 1;
   cfdata->maximize = cfdata->maximize_policy;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   CFData *cfdata;
   
   cfdata = E_NEW(CFData, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, CFData *cfdata)
{
   /* Free the cfdata */
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_config->use_auto_raise = cfdata->auto_raise;
   cfdata->use_resist = cfdata->resist;
   e_config->maximize_policy = cfdata->maximize;
   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_config->use_auto_raise = cfdata->use_auto_raise;
   e_config->auto_raise_delay = cfdata->auto_raise_delay;
   e_config->use_resist = cfdata->use_resist;
   e_config->desk_resist = cfdata->desk_resist;
   e_config->window_resist = cfdata->window_resist;
   e_config->gadget_resist = cfdata->gadget_resist;
   e_config->maximize_policy = cfdata->maximize_policy;
   e_config_save_queue();
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *of, *ob;
   E_Radio_Group *rg;
   
   _fill_data(cfdata);
   
   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Miscellaneous Options"), 0);
   ob = e_widget_check_add(evas, _("Automatically raise windows on mouse over"), &(cfdata->auto_raise));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("When moving or resizing windows, resist at the boundaries"), &(cfdata->resist));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Maximize Policy"), 0);
   rg = e_widget_radio_group_new(&(cfdata->maximize));
   ob = e_widget_radio_add(evas, _("Fullscreen"), E_MAXIMIZE_FULLSCREEN, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Smart expansion"), E_MAXIMIZE_SMART, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Expand the window"), E_MAXIMIZE_EXPAND, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Fill available space"), E_MAXIMIZE_FILL, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o, *ob, *of;
   E_Radio_Group *rg;
   
   _fill_data(cfdata);
   
   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Autoraise"), 0);
   ob = e_widget_check_add(evas, _("Automatically raise windows on mouse over"), &(cfdata->use_auto_raise));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Delay before raising:"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f sec"), 0.0, 9.9, 0.1, 0, &(cfdata->auto_raise_delay), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Resistance"), 0);
   ob = e_widget_check_add(evas, _("Resist moving or resizing a window over an obstacle"), &(cfdata->use_resist));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Resistance between windows:"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0, 64.0, 1.0, 0, NULL, &(cfdata->window_resist), 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Resistance at the edge of the screen:"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0, 64.0, 1.0, 0, NULL, &(cfdata->desk_resist), 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Resistance to desktop gadgets:"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0, 64.0, 1.0, 0, NULL, &(cfdata->gadget_resist), 200);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Maximize Policy"), 0);
   rg = e_widget_radio_group_new(&(cfdata->maximize_policy));
   ob = e_widget_radio_add(evas, _("Fullscreen"), E_MAXIMIZE_FULLSCREEN, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Smart expansion"), E_MAXIMIZE_SMART, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Expand the window"), E_MAXIMIZE_EXPAND, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Fill available space"), E_MAXIMIZE_FILL, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}
