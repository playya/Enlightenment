/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

#define MODE_CUSTOM            0
#define MODE_BOTTOM_MIDDLE     1
#define MODE_BOTTOM_ALL        2
#define MODE_BOTTOM_DESKTOP    3
#define MODE_TOP_ALL           4
#define MODE_TOP_DESKTOP       5

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   E_Shelf *es;
   E_Config_Shelf *escfg;
   /* BASIC */
   int mode;
   /* ADVANCED */
   char *style;
   int orient;
   int fit_along;
   int fit_size;
   int size;
   int layering;
};

/* a nice easy setup function that does the dirty work */
EAPI void
e_int_shelf_config(E_Shelf *es)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   if (v)
     {
	/* methods */
	v->create_cfdata           = _create_data;
	v->free_cfdata             = _free_data;
	v->basic.apply_cfdata      = _basic_apply_data;
	v->basic.create_widgets    = _basic_create_widgets;
	v->advanced.apply_cfdata   = _advanced_apply_data;
	v->advanced.create_widgets = _advanced_create_widgets;
	
	v->override_auto_apply = 1;
	
	/* create config diaolg for bd object/data */
	cfd = e_config_dialog_new(es->zone->container, 
				  _("Shelf Configuration"),
				  "E", "_shelf_config_dialog",
				  NULL, 0, v, es);
	es->config_dialog = cfd;
     }
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->mode = MODE_CUSTOM;
   if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
       ((cfdata->escfg->style) && (!strcmp(cfdata->escfg->style, "default"))) &&
       (cfdata->escfg->fit_along == 1) &&
       (cfdata->escfg->popup) &&
       (cfdata->escfg->layer == 200))
     cfdata->mode = MODE_BOTTOM_MIDDLE;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
	 ((cfdata->escfg->style) && (!strcmp(cfdata->escfg->style, "default"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 200))
       cfdata->mode = MODE_BOTTOM_ALL;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
	 ((cfdata->escfg->style) && (!strcmp(cfdata->escfg->style, "invisible"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (!cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 1))
       cfdata->mode = MODE_BOTTOM_DESKTOP;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_TOP) &&
	 ((cfdata->escfg->style) && (!strcmp(cfdata->escfg->style, "default"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 200))
       cfdata->mode = MODE_TOP_ALL;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_TOP) &&
	 ((cfdata->escfg->style) && (!strcmp(cfdata->escfg->style, "invisible"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (!cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 1))
       cfdata->mode = MODE_TOP_DESKTOP;
   
   if (cfdata->escfg->style)
     cfdata->style = strdup(cfdata->escfg->style);
   else
     cfdata->style = strdup("");
   cfdata->orient = cfdata->escfg->orient;
   cfdata->fit_along = cfdata->escfg->fit_along;
   cfdata->fit_size = cfdata->escfg->fit_size;
   cfdata->size = cfdata->escfg->size;
   if ((!cfdata->escfg->popup) && 
       (cfdata->escfg->layer == 1))
     cfdata->layering = 0;
   else if ((cfdata->escfg->popup) && 
	    (cfdata->escfg->layer == 0))
     cfdata->layering = 1;
   else if ((cfdata->escfg->popup) && 
	    (cfdata->escfg->layer == 200))
     cfdata->layering = 2;
   else
     cfdata->layering = 2;
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
   cfdata->es = cfd->data;
   cfdata->escfg = cfdata->es->cfg;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Free the cfdata */
   cfdata->es->config_dialog = NULL;
   if (cfdata->style) free(cfdata->style);
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Zone *zone;
   int id;

   switch (cfdata->mode)
     {
      case MODE_BOTTOM_MIDDLE:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add("default");
	cfdata->escfg->fit_along = 1;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_BOTTOM_ALL:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add("default");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_BOTTOM_DESKTOP:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add("invisible");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 0;
	cfdata->escfg->layer = 1;
	break;
      case MODE_TOP_ALL:
	cfdata->escfg->orient = E_GADCON_ORIENT_TOP;
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add("default");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_TOP_DESKTOP:
	cfdata->escfg->orient = E_GADCON_ORIENT_TOP;
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add("invisible");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 0;
	cfdata->escfg->layer = 1;
	break;
      default:
	break;
     }
   
   cfdata->escfg->size = cfdata->size;

   zone = cfdata->es->zone;
   id = cfdata->es->id;
   cfdata->es->config_dialog = NULL;
   e_object_del(E_OBJECT(cfdata->es));
   cfdata->es = e_shelf_zone_new(zone, cfdata->escfg->name, 
				 cfdata->escfg->style,
				 cfdata->escfg->popup,
				 cfdata->escfg->layer, id);
   cfdata->es->cfg = cfdata->escfg;
   cfdata->es->fit_along = cfdata->escfg->fit_along;
   cfdata->es->fit_size = cfdata->escfg->fit_size;
   e_shelf_orient(cfdata->es, cfdata->escfg->orient);
   e_shelf_position_calc(cfdata->es);
   e_shelf_populate(cfdata->es);
   e_shelf_show(cfdata->es);
   e_config_save_queue();
   cfdata->es->config_dialog = cfd;
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   E_Zone *zone;
   int id;
   int restart = 0;

   /* Only change style is we need to */
   if (!cfdata->escfg->style) 
     {
	cfdata->escfg->style = evas_stringshare_add(cfdata->style);
	e_shelf_style_set(cfdata->es, cfdata->style);
     }
   else if ((cfdata->escfg->style) && 
	    (strcmp(cfdata->escfg->style, cfdata->style))) 
     {
	if (cfdata->escfg->style) evas_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = evas_stringshare_add(cfdata->style);
	e_shelf_style_set(cfdata->es, cfdata->style);
     }

   /* Only Change Orient if we need to */
   if (cfdata->escfg->orient != cfdata->orient) 
     {
	cfdata->escfg->orient = cfdata->orient;
	e_shelf_orient(cfdata->es, cfdata->orient);
	e_shelf_position_calc(cfdata->es);
	restart = 1;
     }

   /* Only Change fit along if we need to */
   if (cfdata->escfg->fit_along != cfdata->fit_along) 
     {
	cfdata->escfg->fit_along = cfdata->fit_along;
	cfdata->es->fit_along = cfdata->fit_along;
	restart = 1;
     }

   /* Only Change fit size if we need to */
   if (cfdata->escfg->fit_size != cfdata->fit_size) 
     {
	/* Not sure if this will need a restart or not */
	cfdata->escfg->fit_size = cfdata->fit_size;
	cfdata->es->fit_size = cfdata->fit_size;
	restart = 1;
     }
   
   /* Only Change size if we need to */
   if (cfdata->escfg->size != cfdata->size) 
     {	
	cfdata->escfg->size = cfdata->size;
	cfdata->es->size = cfdata->size;
	restart = 1;
     }
            
   if (cfdata->layering == 0)
     {
	if ((cfdata->escfg->popup != 0) || (cfdata->escfg->layer != 1)) 
	  {
	     restart = 1;	
	     cfdata->escfg->popup = 0;
	     cfdata->escfg->layer = 1;
	  }
     }
   else if (cfdata->layering == 1)
     {
	if ((cfdata->escfg->popup != 1) || (cfdata->escfg->layer != 0)) 
	  {
	     restart = 1;
	     cfdata->escfg->popup = 1;
	     cfdata->escfg->layer = 0;
	  }
     }
   else if (cfdata->layering == 2)
     {
	if ((cfdata->escfg->popup != 1) || (cfdata->escfg->layer != 200)) 
	  {
	     restart = 1;
	     cfdata->escfg->popup = 1;
	     cfdata->escfg->layer = 200;
	  }
     }

   if (restart) 
     {
	zone = cfdata->es->zone;
	id = cfdata->es->id;
	cfdata->es->config_dialog = NULL;
	e_object_del(E_OBJECT(cfdata->es));
	
	cfdata->es = e_shelf_zone_new(zone, cfdata->escfg->name, 
				      cfdata->escfg->style,
				      cfdata->escfg->popup,
				      cfdata->escfg->layer, id);
	cfdata->es->cfg = cfdata->escfg;
	cfdata->es->fit_along = cfdata->escfg->fit_along;
	cfdata->es->fit_size = cfdata->escfg->fit_size;
	e_shelf_orient(cfdata->es, cfdata->escfg->orient);
	e_shelf_position_calc(cfdata->es);
	e_shelf_populate(cfdata->es);
	e_shelf_show(cfdata->es);
     }

   e_config_save_queue();   
   cfdata->es->config_dialog = cfd;
   return 1; /* Apply was OK */
}

static void
_cb_configure(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata->es->gadcon->config_dialog)
     e_int_gadcon_config(cfdata->es->gadcon);
}
    
/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *of, *ob, *ol;
   E_Radio_Group *rg;

   o = e_widget_list_add(evas, 0, 0);

   ol = e_widget_list_add(evas, 0, 1);
   
   of = e_widget_framelist_add(evas, _("Layout"), 0);
   rg = e_widget_radio_group_new(&(cfdata->mode));
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_dock", 64, 24, MODE_BOTTOM_MIDDLE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_panel", 64, 24, MODE_BOTTOM_ALL, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_bottom_desk", 64, 24, MODE_BOTTOM_DESKTOP, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_menu_bar", 64, 24, MODE_TOP_ALL, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_top_desk", 64, 24, MODE_TOP_DESKTOP, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_custom", 64, 24, MODE_CUSTOM, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Size"), 0);
   rg = e_widget_radio_group_new(&(cfdata->size));
   ob = e_widget_radio_add(evas, _("Tiny"), 24, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Small"), 32, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Medium"), 40, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Large"), 48, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Huge"), 56, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   
   e_widget_list_object_append(o, ol, 0, 0, 0.5);
   
   ob = e_widget_button_add(evas, _("Configure Contents..."), "widget/config", _cb_configure, cfdata, NULL);
   e_widget_list_object_append(o, ob, 0, 0, 0.5);

   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *o2, *of, *ob, *oi, *oj;
   E_Radio_Group *rg;
   Evas_Coord wmw, wmh;
   Evas_List *styles, *l;
   int sel, n;
   
   /* FIXME: this is just raw config now - it needs UI improvments */
   o = e_widget_list_add(evas, 0, 1);
     
   o2 = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Stacking"), 0);
   rg = e_widget_radio_group_new(&(cfdata->layering));
   ob = e_widget_radio_add(evas, _("Above Everything"), 2, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Below Windows"), 1, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Below Everything"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);
   
   of = e_widget_frametable_add(evas, _("Layout"), 1);
   rg = e_widget_radio_group_new(&(cfdata->orient));
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_left", 24, 24, E_GADCON_ORIENT_LEFT, rg);
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_right", 24, 24, E_GADCON_ORIENT_RIGHT, rg);
   e_widget_frametable_object_append(of, ob, 4, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_top", 24, 24, E_GADCON_ORIENT_TOP, rg);
   e_widget_frametable_object_append(of, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_bottom", 24, 24, E_GADCON_ORIENT_BOTTOM, rg);
   e_widget_frametable_object_append(of, ob, 2, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_top_left", 24, 24, E_GADCON_ORIENT_CORNER_TL, rg);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_top_right", 24, 24, E_GADCON_ORIENT_CORNER_TR, rg);
   e_widget_frametable_object_append(of, ob, 3, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_bottom_left", 24, 24, E_GADCON_ORIENT_CORNER_BL, rg);
   e_widget_frametable_object_append(of, ob, 1, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_bottom_right", 24, 24, E_GADCON_ORIENT_CORNER_BR, rg);
   e_widget_frametable_object_append(of, ob, 3, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_left_top", 24, 24, E_GADCON_ORIENT_CORNER_LT, rg);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_right_top", 24, 24, E_GADCON_ORIENT_CORNER_RT, rg);
   e_widget_frametable_object_append(of, ob, 4, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_left_bottom", 24, 24, E_GADCON_ORIENT_CORNER_LB, rg);
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "enlightenment/shelf_position_right_bottom", 24, 24, E_GADCON_ORIENT_CORNER_RB, rg);
   e_widget_frametable_object_append(of, ob, 4, 3, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);
   
   e_widget_list_object_append(o, o2, 1, 1, 0.5);
   
   o2 = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Size"), 0);
   ob = e_widget_check_add(evas, _("Shrink to Content Size"), &(cfdata->fit_along));
   e_widget_framelist_object_append(of, ob);
//   ob = e_widget_check_add(evas, _("Expand width to fit contents"), &(cfdata->fit_size));
//   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Shelf Size"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%3.0f pixels"), 4, 120, 4, 0, NULL, &(cfdata->size), 100);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Styles"), 0);
   
   oi = e_widget_ilist_add(evas, 128, 20, &(cfdata->style));
   
   sel = 0;
   styles = e_theme_shelf_list();

   for (n = 0, l = styles; l; l = l->next, n++)
     {
	char buf[4096];
	
	ob = e_livethumb_add(evas);
	e_livethumb_vsize_set(ob, 256, 40);
	oj = edje_object_add(e_livethumb_evas_get(ob));
	snprintf(buf, sizeof(buf), "e/shelf/%s/base", (char *)l->data);
	e_theme_edje_object_set(oj, "base/theme/shelf", buf);
	e_livethumb_thumb_set(ob, oj);
	e_widget_ilist_append(oi, ob, (char *)l->data, NULL, NULL, l->data);
	if (!strcmp(cfdata->es->style, (char *)l->data))
	  sel = n;
     }
   e_widget_min_size_get(oi, &wmw, &wmh);
   e_widget_min_size_set(oi, wmw, 120);
   
   e_widget_ilist_go(oi);
   e_widget_ilist_selected_set(oi, sel);
   
   e_widget_framelist_object_append(of, oi);
   
   e_widget_list_object_append(o2, of, 0, 0, 0.5);
   
   ob = e_widget_button_add(evas, _("Configure Contents..."), "widget/config", _cb_configure, cfdata, NULL);
   e_widget_list_object_append(o2, ob, 0, 0, 0.5);
   
   e_widget_list_object_append(o, o2, 0, 0, 0.0);
   
   return o;   
}
