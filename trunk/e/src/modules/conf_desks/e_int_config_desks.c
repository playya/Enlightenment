/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _cb_slider_change(void *data, Evas_Object *obj);
static void _cb_disable_flip_anim(void *data, Evas_Object *obj);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   int x;
   int y;
   int flip_animate;
   
   /*- ADVANCED -*/
   int edge_flip_dragging;
   int flip_wrap;
   int flip_mode;
   int flip_interp;
   int flip_pan_bg;
   double flip_speed;
   double x_axis_pan;
   double y_axis_pan;

   /*- GUI -*/
   Evas_Object *preview;
   Eina_List *flip_anim_list;
};

/* a nice easy setup function that does the dirty work */
EAPI E_Config_Dialog *
e_int_config_desks(E_Container *con, const char *params __UNUSED__)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   if (e_config_dialog_find("E", "screen/virtual_desktops")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   
   /* methods */
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.apply_cfdata      = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->basic.check_changed     = _basic_check_changed;
   v->advanced.apply_cfdata   = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   v->advanced.check_changed  = _advanced_check_changed;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con,
			     _("Virtual Desktops Settings"),
			     "E", "_config_desks_dialog",
			     "preferences-desktop", 0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->x = e_config->zone_desks_x_count;
   cfdata->y = e_config->zone_desks_y_count;
   cfdata->flip_animate = e_config->desk_flip_animate_mode > 0;
   cfdata->edge_flip_dragging = e_config->edge_flip_dragging;
   cfdata->flip_wrap = e_config->desk_flip_wrap;
   cfdata->flip_mode = e_config->desk_flip_animate_mode;
   cfdata->flip_interp = e_config->desk_flip_animate_interpolation;
   cfdata->flip_pan_bg = e_config->desk_flip_pan_bg;
   cfdata->flip_speed = e_config->desk_flip_animate_time;
   cfdata->x_axis_pan = e_config->desk_flip_pan_x_axis_factor;
   cfdata->y_axis_pan = e_config->desk_flip_pan_y_axis_factor;
}

static void *
_create_data(E_Config_Dialog *cdd)
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
_free_data(E_Config_Dialog *cdd, E_Config_Dialog_Data *cfdata)
{
   eina_list_free(cfdata->flip_anim_list);
   E_FREE(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cdd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   const Eina_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     EINA_LIST_FOREACH(man->containers, ll, con)
       EINA_LIST_FOREACH(con->zones, lll, zone)
         e_zone_desk_count_set(zone, cfdata->x, cfdata->y);

   if (cfdata->flip_animate)
     {
	cfdata->flip_mode = 1;
	e_config->desk_flip_animate_mode = 1;
	e_config->desk_flip_animate_interpolation = 0;
	e_config->desk_flip_animate_time  = 0.5;
     }
   else
     {
	cfdata->flip_mode = 0;
	e_config->desk_flip_animate_mode = 0;
     }

   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   const Eina_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     EINA_LIST_FOREACH(man->containers, ll, con)
       EINA_LIST_FOREACH(con->zones, lll, zone)
         {
	    int x, y;
	    e_zone_desk_count_get(zone, &x, &y);
	    if ((x != cfdata->x) || (y != cfdata->y))
	      return 1;
	 }

   if (cfdata->flip_animate)
     {
	if ((cfdata->flip_mode != 1) ||
	    (e_config->desk_flip_animate_mode != 1) ||
	    (e_config->desk_flip_animate_interpolation != 0) ||
	    (e_config->desk_flip_animate_time != 0.5))
	  return 1;
     }
   else
     {
	if ((cfdata->flip_mode != 0) ||
	    (e_config->desk_flip_animate_mode != 0))
	  return 1;
     }

   return 0;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   const Eina_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     EINA_LIST_FOREACH(man->containers, ll, con)
       EINA_LIST_FOREACH(con->zones, lll, zone)
         e_zone_desk_count_set(zone, cfdata->x, cfdata->y);

   e_config->desk_flip_animate_mode = cfdata->flip_mode;
   e_config->desk_flip_animate_interpolation = cfdata->flip_interp;
   e_config->desk_flip_pan_bg = cfdata->flip_pan_bg;
   e_config->desk_flip_animate_time = cfdata->flip_speed;
   e_config->desk_flip_pan_x_axis_factor = cfdata->x_axis_pan;
   e_config->desk_flip_pan_y_axis_factor = cfdata->y_axis_pan;
   
   e_config->edge_flip_dragging = cfdata->edge_flip_dragging;
   e_config->desk_flip_wrap = cfdata->flip_wrap;

   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_advanced_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   const Eina_List *l, *ll, *lll;
   E_Manager *man;
   E_Container *con;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     EINA_LIST_FOREACH(man->containers, ll, con)
       EINA_LIST_FOREACH(con->zones, lll, zone)
         {
	    int x, y;
	    e_zone_desk_count_get(zone, &x, &y);
	    if ((x != cfdata->x) || (y != cfdata->y))
	      return 1;
	 }

   return ((e_config->desk_flip_animate_mode != cfdata->flip_mode) ||
	   (e_config->desk_flip_animate_interpolation != cfdata->flip_interp) ||
	   (e_config->desk_flip_pan_bg != cfdata->flip_pan_bg) ||
	   (e_config->desk_flip_animate_time != cfdata->flip_speed) ||
	   (e_config->desk_flip_pan_x_axis_factor != cfdata->x_axis_pan) ||
	   (e_config->desk_flip_pan_y_axis_factor != cfdata->y_axis_pan) ||
	   (e_config->edge_flip_dragging != cfdata->edge_flip_dragging) ||
	   (e_config->desk_flip_wrap != cfdata->flip_wrap));
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cdd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ob, *of, *ot;  
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Number of Desktops"), 0);
   
   ot = e_widget_table_add(evas, 0);

   ob = e_widget_desk_preview_add(evas, cfdata->x, cfdata->y);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   cfdata->preview = ob;

   ob = e_widget_slider_add(evas, 0, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->y), 150);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 0, 1, 0, 1);

   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->x), 200);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   e_widget_framelist_object_append(of, ot);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
    
   of = e_widget_framelist_add(evas, _("Desktop Mouse Flip"), 0);
   ob = e_widget_check_add(evas, _("Animated flip"), &(cfdata->flip_animate));
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o, *ob, *of, *ot, *ott;
   E_Radio_Group *rg;
   
   o = e_widget_list_add(evas, 0, 0);
   ott = e_widget_table_add(evas, 0);
   
   of = e_widget_framelist_add(evas, _("Desktops"), 0);
   ot = e_widget_table_add(evas, 0);

   ob = e_widget_desk_preview_add(evas, cfdata->x, cfdata->y);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   cfdata->preview = ob;

   ob = e_widget_slider_add(evas, 0, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->y), 150);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 0, 1, 0, 1);

   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f"), 1.0, 12.0, 1.0, 0, NULL, &(cfdata->x), 200);
   e_widget_on_change_hook_set(ob, _cb_slider_change, cfdata);
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   e_widget_framelist_object_append(of, ot);
   e_widget_table_object_append(ott, of, 0, 0, 1, 2, 1, 1, 1, 1);
   
   of = e_widget_framelist_add(evas, _("Desktop Mouse Flip"), 0);
   ob = e_widget_check_add(evas, _("Flip when dragging objects to the screen edge"), &(cfdata->edge_flip_dragging));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Wrap desktops around when flipping"), &(cfdata->flip_wrap));
   e_widget_framelist_object_append(of, ob);
   e_widget_table_object_append(ott, of, 1, 0, 1, 1, 1, 0, 1, 0);
   
   of = e_widget_framelist_add(evas, _("Flip Animation"), 0);
   rg = e_widget_radio_group_new(&(cfdata->flip_mode));
   ob = e_widget_radio_add(evas, _("Off"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_on_change_hook_set(ob, _cb_disable_flip_anim, cfdata);
   ob = e_widget_radio_add(evas, _("Pane"), 1, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_on_change_hook_set(ob, _cb_disable_flip_anim, cfdata);
   ob = e_widget_radio_add(evas, _("Zoom"), 2, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_on_change_hook_set(ob, _cb_disable_flip_anim, cfdata);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f sec"), 0.0, 5.0, 0.05, 0, &(cfdata->flip_speed), NULL, 200);
   e_widget_disabled_set(ob, !cfdata->flip_mode);
   cfdata->flip_anim_list = eina_list_append(cfdata->flip_anim_list, ob);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Background panning"), &(cfdata->flip_pan_bg));
   e_widget_disabled_set(ob, !cfdata->flip_mode);
   cfdata->flip_anim_list = eina_list_append(cfdata->flip_anim_list, ob);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%.2f X-axis pan factor"), 0.0, 1.0, 0.01, 0, &(cfdata->x_axis_pan), NULL, 200);
   e_widget_disabled_set(ob, !cfdata->flip_mode);
   cfdata->flip_anim_list = eina_list_append(cfdata->flip_anim_list, ob);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%.2f Y-axis pan factor"), 0.0, 1.0, 0.01, 0, &(cfdata->y_axis_pan), NULL, 200);
   e_widget_disabled_set(ob, !cfdata->flip_mode);
   cfdata->flip_anim_list = eina_list_append(cfdata->flip_anim_list, ob);
   e_widget_framelist_object_append(of, ob);

   e_widget_table_object_append(ott, of, 1, 1, 1, 1, 1, 1, 1, 1);
  
   e_widget_list_object_append(o, ott, 1, 1, 0.5);
   
   return o;
}

static void
_cb_slider_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_desk_preview_num_desks_set(cfdata->preview, cfdata->x, cfdata->y);
}

static void
_cb_disable_flip_anim(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata = (E_Config_Dialog_Data*) data;
   Eina_List *list = cfdata->flip_anim_list;
   Eina_List *l;
   Evas_Object *o;

   EINA_LIST_FOREACH(list, l, o)
      e_widget_disabled_set(o, !cfdata->flip_mode);
}
