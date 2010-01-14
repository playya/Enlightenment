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
static void _cb_disable_check_list(void *data, Evas_Object *obj);
static void _calibrate_bindings(void);

#define MODE_CUSTOM 0
#define MODE_BOTTOM_MIDDLE 1
#define MODE_BOTTOM_ALL 2
#define MODE_BOTTOM_DESKTOP 3
#define MODE_TOP_ALL 4
#define MODE_TOP_DESKTOP 5

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   E_Shelf *es;
   E_Config_Shelf *escfg;

   /* BASIC */
   int mode;
   int basic_size;

   /* ADVANCED */
   const char *style;
   int orient;
   int fit_along;
   int fit_size;
   int size;
   int layering;
   int overlapping;
   int autohiding;
   int autohiding_show_action;
   double hide_timeout;
   double hide_duration;

   int desk_show_mode;
   Eina_List *desk_list;
   Eina_List *autohide_list;

   Evas_Object *desk_sel_list;
};


/* a nice easy setup function that does the dirty work */
EAPI void
e_int_shelf_config(E_Shelf *es)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return;

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   v->override_auto_apply = 1;
	
   /* create config diaolg for bd object/data */
   cfd = e_config_dialog_new(es->zone->container, 
                             _("Shelf Settings"),
                             "E", "_shelf_config_dialog",
                             "preferences-desktop-shelf", 0, v, es);
   es->config_dialog = cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->mode = MODE_CUSTOM;
   if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
       ((cfdata->escfg->style) && 
        (!strcmp(cfdata->escfg->style, "default"))) &&
       (cfdata->escfg->fit_along == 1) &&
       (cfdata->escfg->popup) &&
       (cfdata->escfg->layer == 200))
     cfdata->mode = MODE_BOTTOM_MIDDLE;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
	 ((cfdata->escfg->style) && 
          (!strcmp(cfdata->escfg->style, "default"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 200))
       cfdata->mode = MODE_BOTTOM_ALL;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_BOTTOM) &&
	 ((cfdata->escfg->style) && 
          (!strcmp(cfdata->escfg->style, "invisible"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (!cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 1))
       cfdata->mode = MODE_BOTTOM_DESKTOP;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_TOP) &&
	 ((cfdata->escfg->style) && 
          (!strcmp(cfdata->escfg->style, "default"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 200))
       cfdata->mode = MODE_TOP_ALL;
   else
     if ((cfdata->escfg->orient == E_GADCON_ORIENT_TOP) &&
	 ((cfdata->escfg->style) && 
          (!strcmp(cfdata->escfg->style, "invisible"))) &&
	 (cfdata->escfg->fit_along == 0) &&
	 (!cfdata->escfg->popup) &&
	 (cfdata->escfg->layer == 1))
       cfdata->mode = MODE_TOP_DESKTOP;
   
   if (cfdata->escfg->style)
     cfdata->style = eina_stringshare_ref(cfdata->escfg->style);
   else
     cfdata->style = eina_stringshare_add("");
   cfdata->orient = cfdata->escfg->orient;
   cfdata->fit_along = cfdata->escfg->fit_along;
   cfdata->fit_size = cfdata->escfg->fit_size;
   cfdata->size = cfdata->escfg->size;
   cfdata->overlapping = cfdata->escfg->overlap;
   cfdata->autohiding = cfdata->escfg->autohide;
   cfdata->autohiding_show_action = cfdata->escfg->autohide_show_action;
   cfdata->hide_timeout = cfdata->escfg->hide_timeout;
   cfdata->hide_duration = cfdata->escfg->hide_duration;
   cfdata->desk_show_mode = cfdata->escfg->desk_show_mode;
   cfdata->desk_list = cfdata->escfg->desk_list;
   if (cfdata->size <= 24)
     cfdata->basic_size = 24;
   else if (cfdata->size <= 32)
     cfdata->basic_size = 32;
   else if (cfdata->size <= 40)
     cfdata->basic_size = 40;
   else if (cfdata->size <= 48)
     cfdata->basic_size = 48;
   else if (cfdata->size >= 48)
     cfdata->basic_size = 56;

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

static void 
_desk_sel_list_load(E_Config_Dialog_Data *cfdata) 
{
   Evas *evas;
   int x, y;

   if (!cfdata->desk_sel_list) return;
   evas = evas_object_evas_get(cfdata->desk_sel_list);
   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(cfdata->desk_sel_list);
   e_widget_ilist_clear(cfdata->desk_sel_list);

   for (y = 0; y < e_config->zone_desks_y_count; y++)
     for (x = 0; x < e_config->zone_desks_x_count; x++)
       {
	  E_Desk *desk;
	  Eina_List *l = NULL;
	  E_Config_Shelf_Desk *sd;

	  desk = e_desk_at_xy_get(cfdata->es->zone, x, y);
	  e_widget_ilist_append(cfdata->desk_sel_list, NULL, desk->name, 
		NULL, NULL, NULL);

	  EINA_LIST_FOREACH(cfdata->desk_list, l, sd)
	    {
	       if (!sd) continue;
	       if ((sd->x != x) || (sd->y != y)) continue;

	       e_widget_ilist_multi_select(cfdata->desk_sel_list, 
		     e_widget_ilist_count(cfdata->desk_sel_list));
	       break;
	    }
       }
   e_widget_ilist_go(cfdata->desk_sel_list);
   e_widget_ilist_thaw(cfdata->desk_sel_list);
   edje_thaw();
   evas_event_thaw(evas);
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
   eina_list_free(cfdata->autohide_list);

   /* Free the cfdata */
   cfdata->es->config_dialog = NULL;
   eina_stringshare_del(cfdata->style);
   E_FREE(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Zone *zone;

   switch (cfdata->mode)
     {
      case MODE_BOTTOM_MIDDLE:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_add("default");
	cfdata->escfg->fit_along = 1;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_BOTTOM_ALL:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_add("default");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_BOTTOM_DESKTOP:
	cfdata->escfg->orient = E_GADCON_ORIENT_BOTTOM;
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_add("invisible");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 0;
	cfdata->escfg->layer = 1;
	break;
      case MODE_TOP_ALL:
	cfdata->escfg->orient = E_GADCON_ORIENT_TOP;
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_add("default");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 1;
	cfdata->escfg->layer = 200;
	break;
      case MODE_TOP_DESKTOP:
	cfdata->escfg->orient = E_GADCON_ORIENT_TOP;
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_add("invisible");
	cfdata->escfg->fit_along = 0;
	cfdata->escfg->popup = 0;
	cfdata->escfg->layer = 1;
	break;
      default:
	break;
     }
   
   cfdata->escfg->size = cfdata->basic_size;
   cfdata->size = cfdata->basic_size;
   
   zone = cfdata->es->zone;
   cfdata->es->config_dialog = NULL;
   e_object_del(E_OBJECT(cfdata->es));
   cfdata->es = e_shelf_zone_new(zone, cfdata->escfg->name, 
				 cfdata->escfg->style,
				 cfdata->escfg->popup,
				 cfdata->escfg->layer,
				 cfdata->escfg->id);
   cfdata->es->cfg = cfdata->escfg;
   cfdata->es->fit_along = cfdata->escfg->fit_along;
   cfdata->es->fit_size = cfdata->escfg->fit_size;
   e_shelf_orient(cfdata->es, cfdata->escfg->orient);
   e_shelf_position_calc(cfdata->es);
   e_shelf_populate(cfdata->es);
   e_shelf_toggle(cfdata->es, 1);
   e_shelf_show(cfdata->es);
   _calibrate_bindings();
   e_config_save_queue();
   cfdata->es->config_dialog = cfd;
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   E_Zone *zone;
   int idx;
   int restart = 0;

   /* Only change style is we need to */
   if (!cfdata->escfg->style) 
     {
	cfdata->escfg->style = eina_stringshare_ref(cfdata->style);
	e_shelf_style_set(cfdata->es, cfdata->style);
     }
   else if ((cfdata->escfg->style) && 
	    (strcmp(cfdata->escfg->style, cfdata->style))) 
     {
	if (cfdata->escfg->style) eina_stringshare_del(cfdata->escfg->style);
	cfdata->escfg->style = eina_stringshare_ref(cfdata->style);
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
	if (cfdata->size <= 24)
	  cfdata->basic_size = 24;
	else if (cfdata->size <= 32)
	  cfdata->basic_size = 32;
	else if (cfdata->size <= 40)
	  cfdata->basic_size = 40;
	else if (cfdata->size <= 48)
	  cfdata->basic_size = 48;
	else if (cfdata->size >= 48)
	  cfdata->basic_size = 56;
	  
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
	if ((cfdata->escfg->popup != 0) || (cfdata->escfg->layer != 1)) 
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

   cfdata->escfg->overlap = cfdata->overlapping;

   cfdata->escfg->autohide = cfdata->autohiding;
   cfdata->escfg->autohide_show_action = cfdata->autohiding_show_action;
   cfdata->escfg->hide_timeout = cfdata->hide_timeout;
   cfdata->escfg->hide_duration = cfdata->hide_duration;

   cfdata->escfg->desk_show_mode = cfdata->desk_show_mode;
   cfdata->escfg->desk_list = NULL;
   if (cfdata->desk_show_mode)
     {
	Eina_List *l = NULL;
	Eina_List *desk_list = NULL;
	E_Ilist_Item *item;

	idx = -1;
	EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->desk_sel_list), l, item)
	  {
	     E_Desk *desk;
	     E_Config_Shelf_Desk *sd;

	     idx++;
	     if ((!item) || (!item->selected)) continue;

	     desk = e_desk_at_pos_get(cfdata->es->zone, idx);
	     if (!desk) continue;

	     sd = E_NEW(E_Config_Shelf_Desk, 1);
	     sd->x = desk->x;
	     sd->y = desk->y;
	     desk_list = eina_list_append(desk_list, sd);
	  }
	cfdata->escfg->desk_list = desk_list;
     }

   if (restart) 
     {
	zone = cfdata->es->zone;
	cfdata->es->config_dialog = NULL;
	e_object_del(E_OBJECT(cfdata->es));
	
	cfdata->es = e_shelf_zone_new(zone, cfdata->escfg->name, 
				      cfdata->escfg->style,
				      cfdata->escfg->popup,
				      cfdata->escfg->layer,
				      cfdata->escfg->id);
	cfdata->es->cfg = cfdata->escfg;
	cfdata->es->fit_along = cfdata->escfg->fit_along;
	cfdata->es->fit_size = cfdata->escfg->fit_size;
	e_shelf_orient(cfdata->es, cfdata->escfg->orient);
	e_shelf_position_calc(cfdata->es);
	e_shelf_populate(cfdata->es);
     }
   if (cfdata->escfg->desk_show_mode)
     {
	E_Desk *desk;
	Eina_List *l;
	E_Config_Shelf_Desk *sd;
	int show_shelf=0;

	desk = e_desk_current_get(cfdata->es->zone);
	EINA_LIST_FOREACH(cfdata->escfg->desk_list, l, sd)
          {
             if ((desk->x == sd->x) && (desk->y == sd->y))
               {
	          show_shelf = 1;
		  break;
               }
          }
	if (show_shelf)
	  e_shelf_show(cfdata->es);
	else  
	  e_shelf_hide(cfdata->es);
     }
   else
     e_shelf_show(cfdata->es);

   if (cfdata->escfg->autohide && !cfdata->es->hidden)
     e_shelf_toggle(cfdata->es, 0);
   else if (!cfdata->escfg->autohide && cfdata->es->hidden)
     e_shelf_toggle(cfdata->es, 1);

   e_zone_useful_geometry_dirty(cfdata->es->zone);
   _calibrate_bindings();
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
     e_int_gadcon_config_shelf(cfdata->es->gadcon);
}

static void
_calibrate_bindings(void)
{
   E_Binding_Edge *bind;
   Eina_List *l;
   E_Shelf *es;

#define EDGE_BINDING_REMOVE(type, click, delay) \
   bind = e_bindings_edge_get("shelf_show", type, click); \
   if (bind) \
     e_bindings_edge_del(E_BINDING_CONTEXT_ZONE, type, \
	   0, 1, "shelf_show", NULL, delay);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_LEFT, 0, 0.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_RIGHT, 0, 0.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_TOP, 0, 0.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_BOTTOM, 0, 0.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_LEFT, 1, -1.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_RIGHT, 1, -1.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_TOP, 1, -1.0);
   EDGE_BINDING_REMOVE(E_ZONE_EDGE_BOTTOM, 1, -1.0);
#undef EDGE_BINDING_REMOVE

#define EDGE_BINDING_ADD(es, type) \
   if (es->cfg->autohide) \
     { \
	if (es->cfg->autohide_show_action) \
	  e_bindings_edge_add(E_BINDING_CONTEXT_ZONE, type, \
		0, 1, "shelf_show", NULL, -1.0); \
	else \
	  e_bindings_edge_add(E_BINDING_CONTEXT_ZONE, type, \
		0, 1, "shelf_show", NULL, 0.0); \
     }

   EINA_LIST_FOREACH(e_shelf_list(), l, es)
     {
	switch(es->gadcon->orient)
	  {
	   case E_GADCON_ORIENT_LEFT:
	   case E_GADCON_ORIENT_CORNER_LT:
	   case E_GADCON_ORIENT_CORNER_LB:
	      EDGE_BINDING_ADD(es, E_ZONE_EDGE_LEFT)
	      break;
	   case E_GADCON_ORIENT_RIGHT:
	   case E_GADCON_ORIENT_CORNER_RT:
	   case E_GADCON_ORIENT_CORNER_RB:
	      EDGE_BINDING_ADD(es, E_ZONE_EDGE_RIGHT)
	      break;
	   case E_GADCON_ORIENT_TOP:
	   case E_GADCON_ORIENT_CORNER_TL:
	   case E_GADCON_ORIENT_CORNER_TR:
	      EDGE_BINDING_ADD(es, E_ZONE_EDGE_TOP)
	      break;
	   case E_GADCON_ORIENT_BOTTOM:
	   case E_GADCON_ORIENT_CORNER_BL:
	   case E_GADCON_ORIENT_CORNER_BR:
	      EDGE_BINDING_ADD(es, E_ZONE_EDGE_BOTTOM)
	      break;
	  }
     }
#undef EDGE_BINDING_ADD
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
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-dock", 
                                64, 24, MODE_BOTTOM_MIDDLE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-panel", 
                                64, 24, MODE_BOTTOM_ALL, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-bottom-desk", 
                                64, 24, MODE_BOTTOM_DESKTOP, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-menu-bar", 
                                64, 24, MODE_TOP_ALL, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-top-desk", 
                                64, 24, MODE_TOP_DESKTOP, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-desktop-shelf-custom", 
                                64, 24, MODE_CUSTOM, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Size"), 0);
   rg = e_widget_radio_group_new(&(cfdata->basic_size));
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

   ob = e_widget_button_add(evas, _("Set Contents..."), "configure", 
                            _cb_configure, cfdata, NULL);
   e_widget_list_object_append(o, ob, 0, 0, 0.5);

   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *o2, *of, *ob, *oi, *oj;
   Evas_Object *autohide_check;
   E_Radio_Group *rg;
   Evas_Coord wmw, wmh;
   Eina_List *styles, *l;
   char *style;
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
   ob = e_widget_check_add(evas, _("Allow windows to overlap the shelf"), 
                           &(cfdata->overlapping));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Layout"), 0);
   oi = e_widget_table_add(evas, 1);
   rg = e_widget_radio_group_new(&(cfdata->orient));
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-left", 
                                24, 24, E_GADCON_ORIENT_LEFT, rg);
   e_widget_table_object_append(oi, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-right", 
                                24, 24, E_GADCON_ORIENT_RIGHT, rg);
   e_widget_table_object_append(oi, ob, 2, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-top", 
                                24, 24, E_GADCON_ORIENT_TOP, rg);
   e_widget_table_object_append(oi, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom", 
                                24, 24, E_GADCON_ORIENT_BOTTOM, rg);
   e_widget_table_object_append(oi, ob, 1, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-top-left", 
                                24, 24, E_GADCON_ORIENT_CORNER_TL, rg);
   e_widget_table_object_append(oi, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-top-right", 
                                24, 24, E_GADCON_ORIENT_CORNER_TR, rg);
   e_widget_table_object_append(oi, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom-left", 
                                24, 24, E_GADCON_ORIENT_CORNER_BL, rg);
   e_widget_table_object_append(oi, ob, 0, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-bottom-right", 
                                24, 24, E_GADCON_ORIENT_CORNER_BR, rg);
   e_widget_table_object_append(oi, ob, 2, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-left-top", 
                                24, 24, E_GADCON_ORIENT_CORNER_LT, rg);
   e_widget_table_object_append(oi, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-right-top", 
                                24, 24, E_GADCON_ORIENT_CORNER_RT, rg);
   e_widget_table_object_append(oi, ob, 2, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-left-bottom", 
                                24, 24, E_GADCON_ORIENT_CORNER_LB, rg);
   e_widget_table_object_append(oi, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_icon_add(evas, NULL, "preferences-position-right-bottom", 
                                24, 24, E_GADCON_ORIENT_CORNER_RB, rg);
   e_widget_table_object_append(oi, ob, 2, 3, 1, 1, 1, 1, 1, 1);

   e_widget_framelist_object_append(of, oi);

   ob = e_widget_check_add(evas, _("Shrink to Content Size"), 
                           &(cfdata->fit_along));
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   e_widget_list_object_append(o, o2, 1, 1, 0.5);
   
   o2 = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Size"), 0);
   ob = e_widget_slider_add(evas, 1, 0, _("%3.0f pixels"), 4, 120, 4, 0, 
                            NULL, &(cfdata->size), 100);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Styles"), 0);
   
   oi = e_widget_ilist_add(evas, 60, 20, &(cfdata->style));
   
   sel = 0;
   styles = e_theme_shelf_list();

   n = 0;
   EINA_LIST_FOREACH(styles, l, style)
     {
	char buf[PATH_MAX];

	ob = e_livethumb_add(evas);
	e_livethumb_vsize_set(ob, 120, 40);
	oj = edje_object_add(e_livethumb_evas_get(ob));
	snprintf(buf, sizeof(buf), "e/shelf/%s/base", style);
	e_theme_edje_object_set(oj, "base/theme/shelf", buf);
	e_livethumb_thumb_set(ob, oj);
	e_widget_ilist_append(oi, ob, style, NULL, NULL, style);
	if (!strcmp(cfdata->es->style, style)) sel = n;
	n++;
     }
   e_widget_size_min_get(oi, &wmw, &wmh);
   e_widget_size_min_set(oi, wmw, 160);
   
   e_widget_ilist_go(oi);
   e_widget_ilist_selected_set(oi, sel);
   
   e_widget_framelist_object_append(of, oi);
   
   e_widget_list_object_append(o2, of, 1, 1, 0.5);
   
   ob = e_widget_button_add(evas, _("Set Contents..."), "configure", 
                            _cb_configure, cfdata, NULL);
   e_widget_list_object_append(o2, ob, 0, 0, 0.5);

   e_widget_list_object_append(o, o2, 0, 0, 0.0);

   o2 = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Auto Hide"), 0);
   autohide_check = e_widget_check_add(evas, _("Auto-hide the shelf"), 
                                       &(cfdata->autohiding));
   e_widget_framelist_object_append(of, autohide_check);
   rg = e_widget_radio_group_new(&(cfdata->autohiding_show_action));
   ob = e_widget_radio_add(evas, _("Show on mouse in"), 0, rg);
   cfdata->autohide_list = eina_list_append(cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Show on mouse click"), 1, rg);
   cfdata->autohide_list = eina_list_append (cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Hide timeout"));
   cfdata->autohide_list = eina_list_append (cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%.1f seconds"), 0.2, 6.0, 0.2, 0, 
                            &(cfdata->hide_timeout), NULL, 60);
   cfdata->autohide_list = eina_list_append (cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Hide duration"));
   cfdata->autohide_list = eina_list_append (cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%.1f seconds"), 0.1, 2.0, 0.1, 0, 
                            &(cfdata->hide_duration), NULL, 60);
   cfdata->autohide_list = eina_list_append (cfdata->autohide_list, ob);
   e_widget_disabled_set(ob, !cfdata->autohiding); // set state from saved config
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o2, of, 1, 1, 0.5);
   // handler for enable/disable widget array
   e_widget_on_change_hook_set(autohide_check, _cb_disable_check_list, 
                               cfdata->autohide_list);

   of = e_widget_framelist_add(evas, _("Desktop"), 0);
   rg = e_widget_radio_group_new(&(cfdata->desk_show_mode));
   ob = e_widget_radio_add(evas, _("Show on all Desktops"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Show on specified Desktops"), 1, rg);
   e_widget_framelist_object_append(of, ob);

   ob = e_widget_ilist_add(evas, 16, 16, NULL);
   e_widget_disabled_set(ob, 1); // set state from saved config
   cfdata->desk_sel_list = ob;
   e_widget_ilist_multi_select_set(ob, 1);
   _desk_sel_list_load(cfdata);
   e_widget_size_min_get(ob, &wmw, &wmh);
   e_widget_size_min_set(ob, wmw, 64);
   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o2, of, 1, 1, 0.5);
   e_widget_list_object_append(o, o2, 0, 0, 0.0);

   return o;   
}

/*!
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
