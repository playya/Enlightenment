/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <e.h>
#include "e_mod_main.h"

#include "config.h"

/* TODO List:
 *
 * immediate fixes needed:
 * * hook up event callbacks for Engage_App_Icon s
 * * render the list of subapps better (semicircle)
 * * fix mouse overs etc to reach all the sub icons
 * * add emblems to show what is iconified and what is running
 *
 * * pick up iconified apps and running apps on startup
 * * zoom and unzoom (eb->zoom from 1.0 to conf->zoom_factor) on timer
 * * bounce icons on click ( following e_app exec hints? )
 *
 * * maybe add system tray
 * 
 * * Fix menu
 *
 * * icon labels & label tooltips supported for the name of the app
 * * use part list to know how many icons & where to put in the overlay of an icon
 * * description bubbles/tooltips for icons
 * * support dynamic iconsize change on the fly
 * * app subdirs - need to somehow handle these...
 * * use overlay object and repeat events for doing auto hide/show
 * * emit signals on hide/show due to autohide/show
 * * virtualise autoshow/hide to later allow for key bindings, mouse events elsewhere, ipc and other singals to show/hide
 *
 * BONUS Features (maybe do this later):
 *
 * * allow engage icons to be dragged around to re-order/delete
 *
 */

static int bar_count;
static E_Config_DD *conf_edd;
static E_Config_DD *conf_bar_edd;

/* const strings */
static const char *_engage_main_orientation[] =
{"left", "right", "top", "bottom"};

/* module private routines */
static Engage   *_engage_new();
static void    _engage_free(Engage *e);
static void    _engage_app_change(void *data, E_App *a, E_App_Change ch);
static void    _engage_config_menu_new(Engage *e);

static int     _engage_cb_event_border_add(void *data, int type, void *event);
static int     _engage_cb_event_border_remove(void *data, int type, void *event);
static int     _engage_cb_event_border_iconify(void *data, int type, void *event);
static int     _engage_cb_event_border_uniconify(void *data, int type, void *event);

static Engage_Bar *_engage_bar_new(Engage *e, E_Container *con);
static void    _engage_bar_free(Engage_Bar *eb);
static void    _engage_bar_menu_new(Engage_Bar *eb);
static void    _engage_bar_enable(Engage_Bar *eb);
static void    _engage_bar_disable(Engage_Bar *eb);
static void    _engage_bar_frame_resize(Engage_Bar *eb);
static void    _engage_bar_edge_change(Engage_Bar *eb, int edge);
static void    _engage_bar_update_policy(Engage_Bar *eb);
static void    _engage_bar_motion_handle(Engage_Bar *eb, Evas_Coord mx, Evas_Coord my);

static Engage_Icon *_engage_icon_new(Engage_Bar *eb, E_App *a);
static void    _engage_icon_free(Engage_Icon *ic);
static Engage_Icon *_engage_icon_find(Engage_Bar *eb, E_App *a);
static void    _engage_icon_reorder_after(Engage_Icon *ic, Engage_Icon *after);

static Engage_App_Icon *_engage_app_icon_new(Engage_Icon *ic, E_Border *bd, int min);
static void    _engage_app_icon_free(Engage_App_Icon *ai);
static Engage_App_Icon *_engage_app_icon_find(Engage_Icon *ic, E_Border *bd);

static void    _engage_bar_cb_gmc_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change);
static void    _engage_bar_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void    _engage_bar_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void    _engage_bar_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_bar_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_bar_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_bar_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_bar_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void    _engage_icon_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void    _engage_icon_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void    _engage_icon_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_icon_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_icon_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void    _engage_icon_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

#if 0
static void    _engage_icon_reorder_before(Engage_Icon *ic, Engage_Icon *before);
#endif
static void    _engage_app_icon_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void    _engage_app_icon_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void    _engage_app_icon_cb_intercept_show(void *data, Evas_Object *o);
static void    _engage_app_icon_cb_intercept_hide(void *data, Evas_Object *o);

static void    _engage_app_icon_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void    _engage_bar_iconsize_change(Engage_Bar *eb);

static void    _engage_bar_cb_menu_enabled(void *data, E_Menu *m, E_Menu_Item *mi);
static void    _engage_bar_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);

static void    _engage_bar_cb_menu_zoom(void *data, E_Menu *m, E_Menu_Item *mi);

static int     zoom_function(double d, double *zoom, double *disp, Engage_Bar *eb);

E_App         *_engage_unmatched_app;

/* public module routines. all modules must have these */
void *
init(E_Module *m)
{
   Engage *e;

   /* check module api version */
   if (m->api->version < E_MODULE_API_VERSION)
     {
	e_error_dialog_show("Module API Error",
			    "Error initializing Module: Engage\n"
			    "It requires a minimum module API version of: %i.\n"
			    "The module API advertized by Enlightenment is: %i.\n"
			    "Aborting module.",
			    E_MODULE_API_VERSION,
			    m->api->version);
	return NULL;
     }
   /* actually init engage */
   e = _engage_new();
   m->config_menu = e->config_menu;
   return e;
}

int
shutdown(E_Module *m)
{
   Engage *e;

   if (m->config_menu)
     m->config_menu = NULL;

   e = m->data;
   if (e)
     _engage_free(e);
   return 1;
}

int
save(E_Module *m)
{
   Engage *e;

   e = m->data;
   e_config_domain_save("module.engage", conf_edd, e->conf);
   return 1;
}

int
info(E_Module *m)
{
   m->label = strdup("Engage");
   m->icon_file = strdup(PACKAGE_LIB_DIR "/engage/module/module_icon.png");
   return 1;
}

int
about(E_Module *m)
{
   e_error_dialog_show("Enlightenment Engage Module",
		       "This is the Engage module for Enlightenment.\n"
		       "It is the native E17 version of engage.\n\n"
		       "This version offers far greater features\n"
		       "and will be the main focus of development from now on.");
   return 1;
}

/* module private routines */
static Engage *
_engage_new()
{
   Engage *e;
   char buf[4096];
   Evas_List *managers, *l, *l2, *cl;

   bar_count = 0;
   e = E_NEW(Engage, 1);
   if (!e) return NULL;
   
   _engage_unmatched_app = e_app_new(PACKAGE_DATA_DIR "/icons/xapp.eapp", 0);
   if (!_engage_unmatched_app)
     printf("Error, engage cannot find default icon - you need to make isntall\n");

   conf_bar_edd = E_CONFIG_DD_NEW("Engage_Config_Bar", Config_Bar);
#undef T
#undef D
#define T Config_Bar
#define D conf_bar_edd
   E_CONFIG_VAL(D, T, enabled, INT);
   E_CONFIG_VAL(D, T, zoom, INT);
   E_CONFIG_VAL(D, T, zoom_factor, DOUBLE);

   conf_edd = E_CONFIG_DD_NEW("Engage_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, appdir, STR);
   E_CONFIG_VAL(D, T, iconsize, INT);
   E_CONFIG_LIST(D, T, bars, conf_bar_edd);
   /*
   E_CONFIG_VAL(D, T, handle, DOUBLE);
   E_CONFIG_VAL(D, T, autohide, UCHAR);
   */

   e->conf = e_config_domain_load("module.engage", conf_edd);
   if (!e->conf)
     {
	e->conf = E_NEW(Config, 1);
	e->conf->appdir = strdup("engage");
	e->conf->iconsize = 32;
	/*
	e->conf->handle = 0.5;
	e->conf->autohide = 0;
	*/
     }
   E_CONFIG_LIMIT(e->conf->iconsize, 2, 400);
   /*
   E_CONFIG_LIMIT(e->conf->handle, 0.0, 1.0);
   E_CONFIG_LIMIT(e->conf->autohide, 0, 1);
   */

   _engage_config_menu_new(e);

   if (e->conf->appdir[0] != '/')
     {
	char *homedir;

	homedir = e_user_homedir_get();
	if (homedir)
	  {
	     snprintf(buf, sizeof(buf), "%s/.e/e/applications/%s", homedir, e->conf->appdir);
	     free(homedir);
	  }
     }
   else
     strcpy(buf, e->conf->appdir);

   e->apps = e_app_new(buf, 0);
   if (e->apps) e_app_subdir_scan(e->apps, 0);
   e_app_change_callback_add(_engage_app_change, e);

   managers = e_manager_list();
   cl = e->conf->bars;
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;

	man = l->data;
	for (l2 = man->containers; l2; l2 = l2->next)
	  {
	     E_Container *con;
	     Engage_Bar *eb;
	     /* Config */
	     con = l2->data;
	     eb = _engage_bar_new(e, con);
	     if (eb)
	       {
		  E_Menu_Item *mi;

		  if (!cl)
		    {
		       eb->conf = E_NEW(Config_Bar, 1);
		       eb->conf->enabled = 1;
		       eb->conf->zoom = 1;
		       eb->conf->zoom_factor = 2.0;
		       e->conf->bars = evas_list_append(e->conf->bars, eb->conf);
		       E_CONFIG_LIMIT(eb->conf->zoom, 0, 1);
		       E_CONFIG_LIMIT(eb->conf->zoom_factor, 1.0, 4.0);
		    }
		  else
		    {
		       eb->conf = cl->data;
		       cl = cl->next;
		    }
		  /* Menu */
		  _engage_bar_menu_new(eb);

		  /* Add main menu to bar menu */

		  mi = e_menu_item_new(e->config_menu);
		  e_menu_item_label_set(mi, con->name);
		  e_menu_item_submenu_set(mi, eb->menu);

		  /* Setup */
		  if (!eb->conf->enabled)
		    _engage_bar_disable(eb);

	       }
	  }
     }
   return e;
}

static void
_engage_free(Engage *e)
{
   E_CONFIG_DD_FREE(conf_edd);
   E_CONFIG_DD_FREE(conf_bar_edd);

   while (e->bars)
     _engage_bar_free(e->bars->data);
   if (e->apps)
     e_object_unref(E_OBJECT(e->apps));

   E_FREE(e->conf->appdir);
   e_app_change_callback_del(_engage_app_change, e);
   e_object_del(E_OBJECT(e->config_menu));
   evas_list_free(e->conf->bars);
   free(e->conf);
   free(e);
}

static void
_engage_app_change(void *data, E_App *a, E_App_Change ch)
{
   Engage *e;
   Evas_List *l, *ll;

   e = data;
   for (l = e->bars; l; l = l->next)
     {
	Engage_Bar *eb;

	eb = l->data;
	switch (ch)
	  {
	   case E_APP_ADD:
	     if (e_app_is_parent(e->apps, a))
	       {
		  Engage_Icon *ic;

		  e_box_freeze(eb->box_object);
		  ic = _engage_icon_new(eb, a);
		  if (ic)
		    {
		       for (ll = e->apps->subapps; ll; ll = ll->next)
			 {
			    E_App *a2;

			    a2 = ll->data;
			    ic = _engage_icon_find(eb, a2);
			    if (ic) _engage_icon_reorder_after(ic, NULL);
			 }
		       _engage_bar_frame_resize(eb);
		    }
		  e_box_thaw(eb->box_object);
	       }
	     break;
	   case E_APP_DEL:
	     if (e_app_is_parent(e->apps, a))
	       {
		  Engage_Icon *ic;

		  ic = _engage_icon_find(eb, a);
		  if (ic) _engage_icon_free(ic);
		  _engage_bar_frame_resize(eb);
	       }
	     break;
	   case E_APP_CHANGE:
	     if (e_app_is_parent(e->apps, a))
	       {
		  Engage_Icon *ic;

		  e_box_freeze(eb->box_object);
		  ic = _engage_icon_find(eb, a);
		  if (ic) _engage_icon_free(ic);
		  evas_image_cache_flush(eb->evas);
		  evas_image_cache_reload(eb->evas);
		  ic = _engage_icon_new(eb, a);
		  if (ic)
		    {
		       for (ll = e->apps->subapps; ll; ll = ll->next)
			 {
			    E_App *a2;

			    a2 = ll->data;
			    ic = _engage_icon_find(eb, a2);
			    if (ic) _engage_icon_reorder_after(ic, NULL);
			 }
		       _engage_bar_frame_resize(eb);
		    }
		  e_box_thaw(eb->box_object);
	       }
	     break;
	   case E_APP_ORDER:
	     if (a == e->apps)
	       {
		  e_box_freeze(eb->box_object);
		  for (ll = e->apps->subapps; ll; ll = ll->next)
		    {
		       Engage_Icon *ic;
		       E_App *a2;

		       a2 = ll->data;
		       ic = _engage_icon_find(eb, a2);
		       if (ic) _engage_icon_reorder_after(ic, NULL);
		    }
		  e_box_thaw(eb->box_object);
	       }
	     break;
	   case E_APP_EXEC:
	     break;
	   case E_APP_READY:
	     break;
	   case E_APP_EXIT:
	     break;
	   default:
	     break;
	  }
     }
}

static Engage_Bar *
_engage_bar_new(Engage *e, E_Container *con)
{
   Engage_Bar *eb;
   Evas_List *l;
   Evas_Object *o;
   E_Gadman_Policy policy;

   eb = E_NEW(Engage_Bar, 1);
   if (!eb) return NULL;
   eb->engage = e;
   e->bars = evas_list_append(e->bars, eb);

   eb->con = con;
   e_object_ref(E_OBJECT(con));
   eb->evas = con->bg_evas;

   eb->x = eb->y = eb->w = eb->h = -1;
   eb->zoom = 1.0;
   eb->mouse_out = -1;

   evas_event_freeze(eb->evas);
   o = edje_object_add(eb->evas);
   eb->bar_object = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR "/themes/module.eet", "main");
   evas_object_show(o);

   o = evas_object_rectangle_add(eb->evas);
   eb->event_object = o;
   evas_object_layer_set(o, 2);
   evas_object_repeat_events_set(o, 1);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,  _engage_bar_cb_mouse_in,  eb);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _engage_bar_cb_mouse_out, eb);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _engage_bar_cb_mouse_down, eb);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _engage_bar_cb_mouse_up, eb);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _engage_bar_cb_mouse_move, eb);
   evas_object_show(o);

   o = e_box_add(eb->evas);
   eb->box_object = o;
   evas_object_intercept_move_callback_add(o, _engage_bar_cb_intercept_move, eb);
   evas_object_intercept_resize_callback_add(o, _engage_bar_cb_intercept_resize, eb);
   e_box_freeze(o);
   edje_object_part_swallow(eb->bar_object, "items", o);
   evas_object_show(o);

   if (eb->engage->apps)
     {
	for (l = eb->engage->apps->subapps; l; l = l->next)
	  {
	     E_App *a;
	     Engage_Icon *ic;

	     a = l->data;
	     ic = _engage_icon_new(eb, a);
	  }
     }

   eb->align_req = 0.5;
   eb->align = 0.5;
   e_box_align_set(eb->box_object, 0.5, 0.5);

   e_box_thaw(eb->box_object);

   eb->gmc = e_gadman_client_new(eb->con->gadman);
   e_gadman_client_domain_set(eb->gmc, "module.engage", bar_count++);
   policy = E_GADMAN_POLICY_EDGES | E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE;
   e_gadman_client_policy_set(eb->gmc, policy);
   e_gadman_client_min_size_set(eb->gmc, 16, 16);
   e_gadman_client_max_size_set(eb->gmc, 800, 136);
   e_gadman_client_auto_size_set(eb->gmc, -1, -1);
   e_gadman_client_align_set(eb->gmc, 0.5, 1.0);
   e_gadman_client_resize(eb->gmc, 400, 40);
   e_gadman_client_change_func_set(eb->gmc, _engage_bar_cb_gmc_change, eb);
   e_gadman_client_load(eb->gmc);
   /* update for appropriate bar we loaded on */
   _engage_bar_update_policy(eb);

   evas_event_thaw(eb->evas);

   /* We need to resize, if the width is auto and the number
    * of apps has changed since last startup */
   _engage_bar_frame_resize(eb);

   /*
   edje_object_signal_emit(eb->bar_object, "passive", "");
   */


   eb->add_handler = ecore_event_handler_add(E_EVENT_BORDER_ADD,
	 _engage_cb_event_border_add, eb);
   eb->remove_handler = ecore_event_handler_add(E_EVENT_BORDER_REMOVE,
	 _engage_cb_event_border_remove, eb);
   /* FIXME - these are not really iconify events, we need them to be
    * added to E before we can hook in "properly" */
   eb->iconify_handler = ecore_event_handler_add(E_EVENT_BORDER_HIDE,
	 _engage_cb_event_border_iconify, eb);
   eb->uniconify_handler = ecore_event_handler_add(E_EVENT_BORDER_SHOW,
	 _engage_cb_event_border_uniconify, eb);
   return eb;
}

static void
_engage_bar_free(Engage_Bar *eb)
{
   e_object_unref(E_OBJECT(eb->con));

   e_object_del(E_OBJECT(eb->menu));

   while (eb->icons)
     _engage_icon_free(eb->icons->data);

   evas_object_del(eb->bar_object);
   evas_object_del(eb->box_object);
   evas_object_del(eb->event_object);

   e_gadman_client_save(eb->gmc);
   e_object_del(E_OBJECT(eb->gmc));

   eb->engage->bars = evas_list_remove(eb->engage->bars, eb);

   ecore_event_handler_del(eb->add_handler);
   ecore_event_handler_del(eb->remove_handler);
   ecore_event_handler_del(eb->iconify_handler);
   ecore_event_handler_del(eb->uniconify_handler);

   free(eb->conf);
   free(eb);
   bar_count--;

}

static void
_engage_bar_menu_new(Engage_Bar *eb)
{
   E_Menu *mn;
   E_Menu_Item *mi;

   mn = e_menu_new();
   eb->menu = mn;

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "Zoom Icons");
   e_menu_item_check_set(mi, 1);
   if (eb->conf->zoom == 1) e_menu_item_toggle_set(mi, 1);
     e_menu_item_callback_set(mi, _engage_bar_cb_menu_zoom, eb);

   mi = e_menu_item_new(mn); 
   e_menu_item_separator_set(mi, 1);
	 
   /* Enabled */
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "Enabled");
   e_menu_item_check_set(mi, 1);
   if (eb->conf->enabled) e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, _engage_bar_cb_menu_enabled, eb);

   /* Edit */
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "Edit Mode");
   e_menu_item_callback_set(mi, _engage_bar_cb_menu_edit, eb);
}

static void
_engage_bar_enable(Engage_Bar *eb)
{
   eb->conf->enabled = 1;
   evas_object_show(eb->bar_object);
   evas_object_show(eb->box_object);
   evas_object_show(eb->event_object);
   e_config_save_queue();
}

static void
_engage_bar_disable(Engage_Bar *eb)
{
   eb->conf->enabled = 0;
   evas_object_hide(eb->bar_object);
   evas_object_hide(eb->box_object);
   evas_object_hide(eb->event_object);
   e_config_save_queue();
}

static Engage_Icon *
_engage_icon_new(Engage_Bar *eb, E_App *a)
{
   Engage_Icon *ic;
   Evas_Object *o;
   Evas_Coord bw, bh;

   ic = E_NEW(Engage_Icon, 1);
   if (!ic) return NULL;
   ic->eb = eb;
   ic->app = a;
   ic->scale = 1.0;
   ic->temp = 0;
   e_object_ref(E_OBJECT(a));
   eb->icons = evas_list_append(eb->icons, ic);

   o = evas_object_rectangle_add(eb->evas);
   ic->event_object = o;
   evas_object_layer_set(o, 1);
//   evas_object_clip_set(o, evas_object_clip_get(eb->box_object));
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_repeat_events_set(o, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,  _engage_icon_cb_mouse_in,  ic);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _engage_icon_cb_mouse_out, ic);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _engage_icon_cb_mouse_down, ic);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _engage_icon_cb_mouse_up, ic);
   evas_object_show(o);

   o = edje_object_add(eb->evas);
   ic->bg_object = o;
   evas_object_intercept_move_callback_add(o, _engage_icon_cb_intercept_move, ic);
   evas_object_intercept_resize_callback_add(o, _engage_icon_cb_intercept_resize, ic);
   edje_object_file_set(o, PACKAGE_DATA_DIR "/themes/module.eet", "icon");
   evas_object_show(o);

   o = edje_object_add(eb->evas);
   ic->icon_object = o;
   edje_object_file_set(o, ic->app->path, "icon");
   edje_extern_object_min_size_set(o, eb->engage->conf->iconsize, eb->engage->conf->iconsize);
   edje_object_part_swallow(ic->bg_object, "item", o);
   edje_object_size_min_calc(ic->bg_object, &bw, &bh);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);

   o = edje_object_add(eb->evas);
   ic->overlay_object = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR "/themes/module.eet", 
	 "icon_overlay");
   evas_object_show(o);

   evas_object_raise(ic->event_object);

   e_box_pack_end(eb->box_object, ic->bg_object);
   e_box_pack_options_set(ic->bg_object,
			  1, 1, /* fill */
			  0, 0, /* expand */
			  0.5, 0.5, /* align */
			  bw, bh, /* min */
			  bw, bh /* max */
			  );

   edje_object_signal_emit(ic->bg_object, "passive", "");
   edje_object_signal_emit(ic->overlay_object, "passive", "");
   return ic;
}

static void
_engage_icon_free(Engage_Icon *ic)
{
   ic->eb->icons = evas_list_remove(ic->eb->icons, ic);
   if (ic->bg_object) evas_object_del(ic->bg_object);
   if (ic->overlay_object) evas_object_del(ic->overlay_object);
   if (ic->icon_object) evas_object_del(ic->icon_object);
   if (ic->event_object) evas_object_del(ic->event_object);
   while (ic->extra_icons)
     {
	Engage_App_Icon *ai;

	ai = ic->extra_icons->data;
	ic->extra_icons = evas_list_remove_list(ic->extra_icons, ic->extra_icons);
	_engage_app_icon_free(ai);
     }
   e_object_unref(E_OBJECT(ic->app));
   free(ic);
}

static Engage_Icon *
_engage_icon_find(Engage_Bar *eb, E_App *a)
{
   Evas_List *l;

   for (l = eb->icons; l; l = l->next)
     {
	Engage_Icon *ic;

	ic = l->data;
	if (ic->app == a) return ic;
     }
   return NULL;
}

static Engage_App_Icon *
_engage_app_icon_new(Engage_Icon *ic, E_Border *bd, int min)
{
   Engage_App_Icon *ai;
   Evas_Object *o;

   ai = E_NEW(Engage_App_Icon, 1);
   if (!ai) return NULL;
   
   ai->ic = ic;
   ai->border = bd;
   e_object_ref(E_OBJECT(bd));
   ai->min = min?1:0;
   ic->extra_icons = evas_list_append(ic->extra_icons, ai);

   o = evas_object_rectangle_add(ic->eb->evas);
   ai->event_object = o;
   evas_object_layer_set(o, 1);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_repeat_events_set(o, 0);
//   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,  _engage_app_icon_cb_mouse_in,  ai);
//   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _engage_app_icon_cb_mouse_out, ai);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _engage_app_icon_cb_mouse_down, ai);
//   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _engage_app_icon_cb_mouse_up, ai);

   o = edje_object_add(ic->eb->evas);
   ai->bg_object = o;
   evas_object_intercept_move_callback_add(o, _engage_app_icon_cb_intercept_move, ai);
   evas_object_intercept_resize_callback_add(o, _engage_app_icon_cb_intercept_resize, ai);
   evas_object_intercept_show_callback_add(o, _engage_app_icon_cb_intercept_show, ai);
   evas_object_intercept_hide_callback_add(o, _engage_app_icon_cb_intercept_hide, ai);
   edje_object_file_set(o, PACKAGE_DATA_DIR "/themes/module.eet", "icon");

   o = edje_object_add(ic->eb->evas);
   ai->icon_object = o;
   edje_object_file_set(o, ic->app->path, "icon");
   edje_object_part_swallow(ai->bg_object, "item", o);
   evas_object_pass_events_set(o, 1);

   o = edje_object_add(ic->eb->evas);
   ai->overlay_object = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR "/themes/module.eet", 
	 "icon_overlay");

   evas_object_raise(ai->event_object);
   evas_object_resize(ai->bg_object, ic->eb->engage->iconbordersize / 2, ic->eb->engage->iconbordersize / 2);

   edje_object_signal_emit(ai->bg_object, "passive", "");
   edje_object_signal_emit(ai->overlay_object, "passive", "");
   return ai;
}

static void
_engage_app_icon_free(Engage_App_Icon *ai)
{
   ai->ic->extra_icons = evas_list_remove(ai->ic->extra_icons, ai);
   if (ai->bg_object) evas_object_del(ai->bg_object);
   if (ai->overlay_object) evas_object_del(ai->overlay_object);
   if (ai->icon_object) evas_object_del(ai->icon_object);
   if (ai->event_object) evas_object_del(ai->event_object);
   e_object_unref(E_OBJECT(ai->border));
   free(ai);
}

static Engage_App_Icon *
_engage_app_icon_find(Engage_Icon *ic, E_Border *bd)
{
   Evas_List *l;

   for (l = ic->extra_icons; l; l = l->next)
     {
	Engage_App_Icon *ai;

	ai = l->data;
	if (ai->border == bd) return ai;
     }
   return NULL;
}

void
_engage_config_menu_new(Engage *e)
{
   E_Menu *mn;
   E_Menu_Item *mi;

   mn = e_menu_new();

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "(Unused)");

/*
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "Auto hide");
   e_menu_item_check_set(mi, 1);
   if (e->conf->autohide == 0) e_menu_item_toggle_set(mi, 1);

   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, "More Options...");
*/

   e->config_menu = mn;
}

static int
_engage_cb_event_border_add(void *data, int type, void *event)
{
   Engage_Bar *eb;
   Engage_Icon *ic;
   Engage_App_Icon *ai;
   E_Event_Border_Add *e;
   E_App *app;

   e = event;
   eb = data;
   if (e->border->container != eb->con)
     return;

   app = e_app_window_name_class_find(e->border->client.icccm.name,
				      e->border->client.icccm.class);
   if (!app)
     app = _engage_unmatched_app;
   ic = _engage_icon_find(eb, app);
   if (!ic)
     {
	ic = _engage_icon_new(eb, app);
	ic->temp = 1;
	_engage_bar_frame_resize(eb);
     }
   if (ic)
     {
	ai = _engage_app_icon_new(ic, e->border, 0);
     }
}

static int
_engage_cb_event_border_remove(void *data, int type, void *event)
{
   Engage_Bar *eb;
   Engage_Icon *ic;
   Engage_App_Icon *ai;
   E_Event_Border_Remove *e;
   E_App *app;
   Evas_List *icons;

   e = event;
   eb = data;

   if (e->border->container != eb->con)
     return;

   app = e_app_window_name_class_find(e->border->client.icccm.name,
				      e->border->client.icccm.class);
   if (!app)
     app = _engage_unmatched_app;
   ic = _engage_icon_find(eb, app);
   if (!ic)
     return;

   icons = ic->extra_icons;
   while (icons)
     {
	ai = icons->data;
	if (ai->border == e->border)
	  {
	      _engage_app_icon_free(ai);
	      if (!ic->extra_icons && ic->temp == 1)
		{
		   _engage_icon_free(ic);
		   _engage_bar_frame_resize(eb);
		}				      
	      break;
	  }
	icons = icons->next;
     }
}

static int
_engage_cb_event_border_iconify(void *data, int type, void *event)
{
   Engage_Bar *eb;
   Engage_Icon *ic;
   Engage_App_Icon *ai;
   E_Event_Border_Hide *e;
   E_App *app;
   Evas_List *icons;

   e = event;
   eb = data;
   if (e->border->container != eb->con)
     return;

   /* FIXME we can remove this when this is a real iconify event */
   if (!e->border->iconic)
     return;
   app = e_app_window_name_class_find(e->border->client.icccm.name,
				      e->border->client.icccm.class);
   if (!app)
     app = _engage_unmatched_app;
   ic = _engage_icon_find(eb, app);
   if (!ic)
     {
	ic = _engage_icon_new(eb, app);
	ic->temp = 1;
	_engage_bar_frame_resize(eb);
     }
   if (!ic)
     return 0;

   icons = ic->extra_icons;
   while (icons)
     {
	ai = icons->data;
	if (ai->border == e->border)
	  {
	     ai->min = 1;
	     return 0;
	  }
	icons = icons->next;
     }
   /* fallback, is this needed? */
   ai = _engage_app_icon_new(ic, e->border, 1);
}

static int
_engage_cb_event_border_uniconify(void *data, int type, void *event)
{
   Engage_Bar *eb;
   Engage_Icon *ic;
   Engage_App_Icon *ai;
   E_Event_Border_Show *e;
   E_App *app;
   Evas_List *icons;

   e = event;
   eb = data;

   if (e->border->container != eb->con)
     return;

   /* FIXME we can remove this when this is a real iconify event */
   if (!e->border->iconic)
     return;
   app = e_app_window_name_class_find(e->border->client.icccm.name,
				      e->border->client.icccm.class);
   if (!app)
     app = _engage_unmatched_app;
   ic = _engage_icon_find(eb, app);
   if (!ic)
     return 0;

   icons = ic->extra_icons;
   while (icons)
     {
	ai = icons->data;
	if (ai->min && ai->border == e->border)
	  {
	      ai->min = 0;
	      return;
	  }
	icons = icons->next;
     }
   /* fallback, is this needed? */
   ai = _engage_app_icon_new(ic, e->border, 0);
}


#if 0
static void
_engage_icon_reorder_before(Engage_Icon *ic, Engage_Icon *before)
{
   Evas_Coord bw, bh;

   e_box_freeze(ic->eb->box_object);
   e_box_unpack(ic->bg_object);
   ic->eb->icons = evas_list_remove(ic->eb->icons, ic);
   if (before)
     {
	ic->eb->icons = evas_list_prepend_relative(ic->eb->icons, ic, before);
	e_box_pack_before(ic->eb->box_object, ic->bg_object, before->bg_object);
     }
   else
     {
	ic->eb->icons = evas_list_prepend(ic->eb->icons, ic);
	e_box_pack_start(ic->eb->box_object, ic->bg_object);
     }
   edje_object_size_min_calc(ic->bg_object, &bw, &bh);
   e_box_pack_options_set(ic->bg_object,
			  1, 1, /* fill */
			  0, 0, /* expand */
			  0.5, 0.5, /* align */
			  bw, bh, /* min */
			  bw, bh /* max */
			  );
   e_box_thaw(ic->eb->box_object);
}
#endif

static void
_engage_icon_reorder_after(Engage_Icon *ic, Engage_Icon *after)
{
   Evas_Coord bw, bh;

   e_box_freeze(ic->eb->box_object);
   e_box_unpack(ic->bg_object);
   ic->eb->icons = evas_list_remove(ic->eb->icons, ic);
   if (after)
     {
	ic->eb->icons = evas_list_append_relative(ic->eb->icons, ic, after);
	e_box_pack_after(ic->eb->box_object, ic->bg_object, after->bg_object);
     }
   else
     {
	ic->eb->icons = evas_list_append(ic->eb->icons, ic);
	e_box_pack_end(ic->eb->box_object, ic->bg_object);
     }
   edje_object_size_min_calc(ic->bg_object, &bw, &bh);
   e_box_pack_options_set(ic->bg_object,
			  1, 1, /* fill */
			  0, 0, /* expand */
			  0.5, 0.5, /* align */
			  bw, bh, /* min */
			  bw, bh /* max */
			  );
   e_box_thaw(ic->eb->box_object);
}

static void
_engage_bar_frame_resize(Engage_Bar *eb)
{
   Evas_Coord w, h;
   /* Not finished loading config yet! */
   if ((eb->x == -1)
       || (eb->y == -1)
       || (eb->w == -1)
       || (eb->h == -1))
     return;

   evas_event_freeze(eb->evas);
   e_box_freeze(eb->box_object);

   e_box_min_size_get(eb->box_object, &w, &h);

   e_gadman_client_resize(eb->gmc, w, h);
   evas_object_resize(eb->event_object, w, h);
   e_box_thaw(eb->box_object);
   evas_event_thaw(eb->evas);
}

static void
_engage_bar_edge_change(Engage_Bar *eb, int edge)
{
   Evas_List *l;
   Evas_Coord bw, bh, tmp;
   Evas_Object *o;
   E_Gadman_Policy policy;
   int changed;

   evas_event_freeze(eb->evas);
   o = eb->bar_object;
   edje_object_signal_emit(o, "set_orientation", _engage_main_orientation[edge]);
   edje_object_message_signal_process(o);

   e_box_freeze(eb->box_object);
   l = eb->icons;
   for (l = eb->icons; l; l = l->next)
     {
	Engage_Icon *ic;

	ic = l->data;
	o = ic->bg_object;
	edje_object_signal_emit(o, "set_orientation", _engage_main_orientation[edge]);
	edje_object_message_signal_process(o);
	edje_object_size_min_calc(ic->bg_object, &bw, &bh);

	o = ic->overlay_object;
	edje_object_signal_emit(o, "set_orientation", _engage_main_orientation[edge]);
	edje_object_message_signal_process(o);

	e_box_pack_options_set(ic->bg_object,
			       1, 1, /* fill */
			       0, 0, /* expand */
			       0.5, 0.5, /* align */
			       bw, bh, /* min */
			       bw, bh /* max */
			       );
     }

   eb->align_req = 0.5;
   eb->align = 0.5;
   e_box_align_set(eb->box_object, 0.5, 0.5);

   policy = E_GADMAN_POLICY_EDGES | E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE;
   if ((edge == E_GADMAN_EDGE_BOTTOM) ||
       (edge == E_GADMAN_EDGE_TOP))
     {
	changed = (e_box_orientation_get(eb->box_object) != 1);
	if (changed)
	  {
	     e_box_orientation_set(eb->box_object, 1);
	     policy |= E_GADMAN_POLICY_VSIZE;
	     e_gadman_client_policy_set(eb->gmc, policy);
	     tmp = eb->w;
	     eb->w = eb->h;
	     eb->h = tmp;
	  }
     }
   else if ((edge == E_GADMAN_EDGE_LEFT) ||
	    (edge == E_GADMAN_EDGE_RIGHT))
     {
	changed = (e_box_orientation_get(eb->box_object) != 0);
	if (changed)
	  {
	     e_box_orientation_set(eb->box_object, 0);
	     policy |= E_GADMAN_POLICY_HSIZE;
	     e_gadman_client_policy_set(eb->gmc, policy);
	     tmp = eb->w;
	     eb->w = eb->h;
	     eb->h = tmp;
	  }
     }

   e_box_thaw(eb->box_object);
   evas_event_thaw(eb->evas);

   _engage_bar_frame_resize(eb);
}

static void
_engage_bar_update_policy(Engage_Bar *eb)
{
   E_Gadman_Policy policy;

   policy = E_GADMAN_POLICY_EDGES | E_GADMAN_POLICY_HMOVE | E_GADMAN_POLICY_VMOVE;
   if ((e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_BOTTOM) ||
       (e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_TOP))
     {
	policy |= E_GADMAN_POLICY_VSIZE;
	e_gadman_client_policy_set(eb->gmc, policy);
     }
   else if ((e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_LEFT) ||
	    (e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_RIGHT))
     {
	policy |= E_GADMAN_POLICY_HSIZE;
	e_gadman_client_policy_set(eb->gmc, policy);
     }
}

static void
_engage_bar_motion_handle(Engage_Bar *eb, Evas_Coord mx, Evas_Coord my)
{
   Evas_Coord x, y, w, h, md, md2, xx, yy, app_size;
   double relx, rely, left, right, dummy;
   Evas_List *items, *extras;
   int bordersize, counter;
   Engage_Icon *prev;
   E_Gadman_Edge edge;
              
   evas_object_geometry_get(eb->box_object, &x, &y, &w, &h);
   if (w > 0) relx = (double)(mx - x) / (double)w;
   else relx = 0.0;
   if (h > 0) rely = (double)(my - y) / (double)h;
   else rely = 0.0;
   if ((e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_BOTTOM) ||
       (e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_TOP))
     {
	eb->align_req = 1.0 - relx;
     }
   else if ((e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_LEFT) ||
	    (e_gadman_client_edge_get(eb->gmc) == E_GADMAN_EDGE_RIGHT))
     {
	eb->align_req = 1.0 - rely;
     }

   e_gadman_client_geometry_get(eb->gmc, &x, &y, &w, &h);
   edge = e_gadman_client_edge_get(eb->gmc);
   
   e_box_freeze(eb->box_object);
   items = eb->icons;
   
   bordersize = eb->engage->iconbordersize - eb->engage->conf->iconsize;
   if (edge == E_GADMAN_EDGE_LEFT || edge == E_GADMAN_EDGE_RIGHT)
     {
	md = my;
	md2 = mx;
	counter = y;
     }
   else
     {
	md = mx;
	md2 = my;
	counter = x;
     }
   app_size = eb->engage->iconbordersize / 1.5;
   counter += (eb->engage->iconbordersize / 2) + 1;
   while (items)
     {
	Engage_Icon *icon;
	double       distance, new_zoom, relative, size;
	int          do_zoom;

	icon = (Engage_Icon *) items->data;
	if (eb->mouse_out != -1)
	  distance = (double) (counter - eb->mouse_out) / (eb->engage->iconbordersize);
	else
	  distance = (double) (counter - md) / (eb->engage->iconbordersize);

	do_zoom = zoom_function(distance, &new_zoom, &relative, eb);
	size = icon->scale * new_zoom * eb->engage->iconbordersize;
	if (md2 > size)
	  {
	     if (eb->mouse_out == -1)
	       eb->mouse_out = md;
	  }
	else
	  eb->mouse_out = -1;
			       
	evas_object_image_fill_set(icon->icon_object, 0.0, 0.0, size, size);
	evas_object_resize(icon->bg_object, size, size);
	xx = x;
	yy = y;
	if (edge == E_GADMAN_EDGE_LEFT)
	  yy = counter - 0.5 * size;
	else if (edge == E_GADMAN_EDGE_RIGHT)
	  {
	     xx = x + w - size;
	     yy = counter - 0.5 * size;
	  }
	else if (edge == E_GADMAN_EDGE_TOP)
	  xx = counter - 0.5 * size;
	else
	  {
	     xx = counter - 0.5 * size;
	     yy = y + h - size;
	  }
	evas_object_move(icon->bg_object, xx, yy);

	if (edge == E_GADMAN_EDGE_LEFT)
	  xx += size;
	else if (edge == E_GADMAN_EDGE_RIGHT)
	  xx -= app_size;
	else if (edge == E_GADMAN_EDGE_TOP)
	  yy += size;
	else
	  yy -= app_size;
	if (do_zoom && -0.5 < distance && distance < 0.5)
	  {
	     evas_object_raise(icon->icon_object);
	     evas_object_show(icon->event_object);
 
	     for (extras = icon->extra_icons; extras; extras = extras->next)
	       {
		  Engage_App_Icon *ai;

		  ai = extras->data;
		  evas_object_resize(ai->bg_object, app_size, app_size);
		  evas_object_move(ai->bg_object, xx, yy);
		  evas_object_show(ai->bg_object);

		  if (edge == E_GADMAN_EDGE_LEFT || edge == E_GADMAN_EDGE_RIGHT)
		    yy += app_size;
		  else
		    xx += app_size;
	       }
	  }
	else
	  {
	     evas_object_hide(icon->event_object);
	     for (extras = icon->extra_icons; extras; extras = extras->next)
	       {
		   Engage_App_Icon *ai;

		   ai = extras->data;
		   evas_object_hide(ai->bg_object);
	       }
	  }

	prev = icon;
	items = items->next;
	counter += eb->engage->iconbordersize;
     }

   e_box_thaw(eb->box_object);
}

static void
_engage_icon_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
   Engage_Icon *ic;

   ic = data;
   evas_object_move(o, x, y);
   evas_object_move(ic->event_object, x, y);
   evas_object_move(ic->overlay_object, x, y);
}

static void
_engage_icon_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   Engage_Icon *ic;

   ic = data;
   evas_object_resize(o, w, h);
   evas_object_resize(ic->event_object, w, h);
   evas_object_resize(ic->overlay_object, w, h);
}

static void
_engage_app_icon_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
   Engage_App_Icon *ai;

   ai = data;
   evas_object_move(o, x, y);
   evas_object_move(ai->event_object, x, y);
   evas_object_move(ai->overlay_object, x, y);
}

static void
_engage_app_icon_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   Engage_App_Icon *ai;

   ai = data;
   evas_object_resize(o, w, h);
   evas_object_resize(ai->event_object, w, h);
   evas_object_resize(ai->overlay_object, w, h);
}

static void
_engage_app_icon_cb_intercept_show(void *data, Evas_Object *o)
{
   Engage_App_Icon *ai;

   ai = data;
   evas_object_show(o);
   evas_object_show(ai->event_object);
   evas_object_show(ai->overlay_object);
}

static void
_engage_app_icon_cb_intercept_hide(void *data, Evas_Object *o)
{
   Engage_App_Icon *ai;

   ai = data;
   evas_object_hide(o);
   evas_object_hide(ai->event_object);
   evas_object_hide(ai->overlay_object);
}

static void
_engage_bar_cb_intercept_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
   Engage_Bar *eb;

   eb = data;
   evas_object_move(o, x, y);
   evas_object_move(eb->event_object, x, y);
}

static void
_engage_bar_cb_intercept_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   Engage_Bar *eb;
   E_Gadman_Edge edge;
   Evas_Coord border;

   eb = data;

   evas_object_resize(o, w, h);
   evas_object_resize(eb->event_object, w, h);
   
   if (eb->gmc)
     edge = e_gadman_client_edge_get(eb->gmc);
   else
     edge = E_GADMAN_EDGE_BOTTOM;

   /* FIXME "4" should not be hardcoded, difference between engage->conf->icon
    * and engage->iconbordersize */
   border = 4;
   if (edge == E_GADMAN_EDGE_TOP || edge == E_GADMAN_EDGE_BOTTOM)
     eb->engage->conf->iconsize = h - border;
   else
     eb->engage->conf->iconsize = w - border;

   _engage_bar_iconsize_change(eb);
}

static void
_engage_app_icon_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Engage_App_Icon *ai;

   ev = event_info;
   ai = data;
   if (ev->button == 1)
     {
	edje_object_signal_emit(ai->bg_object, "start", "");
	edje_object_signal_emit(ai->overlay_object, "start", "");
	if (ai->min)
	  e_border_uniconify(ai->border);
	e_border_raise(ai->border);
	e_desk_show(ai->border->desk);
     }
}

static void
_engage_icon_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Engage_Icon *ic;

   ev = event_info;
   ic = data;
   evas_event_freeze(ic->eb->evas);
   evas_object_raise(ic->event_object);
   evas_object_stack_below(ic->overlay_object, ic->event_object);
   evas_event_thaw(ic->eb->evas);
//   edje_object_signal_emit(ic->bg_object, "active", "");
//   edje_object_signal_emit(ic->overlay_object, "active", "");
}

static void
_engage_icon_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Engage_Icon *ic;

   ev = event_info;
   ic = data;
//   edje_object_signal_emit(ic->bg_object, "passive", "");
//   edje_object_signal_emit(ic->overlay_object, "passive", "");
}

static void
_engage_icon_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Engage_Icon *ic;

   ev = event_info;
   ic = data;
   if (ev->button == 1)
     {
	edje_object_signal_emit(ic->bg_object, "start", "");
	edje_object_signal_emit(ic->overlay_object, "start", "");
	e_app_exec(ic->app);
     }
}

static void
_engage_icon_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Engage_Icon *ic;

   ev = event_info;
   ic = data;
   if (ev->button == 1)
     {
	edje_object_signal_emit(ic->bg_object, "start_end", "");
	edje_object_signal_emit(ic->overlay_object, "start_end", "");
     }
}

static void
_engage_bar_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Engage_Bar *eb;
   Evas_Coord x, y, w, h;
   E_Gadman_Edge edge;

   ev = event_info;
   eb = data;

   eb->zoom = 2.0;
   evas_object_geometry_get(eb->box_object, &x, &y, &w, &h);
   edge = e_gadman_client_edge_get(eb->gmc);
   if (edge == E_GADMAN_EDGE_LEFT)
     {
	evas_object_resize(eb->event_object, w * (eb->conf->zoom_factor + 1), h);
     }
   else if (edge == E_GADMAN_EDGE_RIGHT)
     {
	evas_object_resize(eb->event_object, w * (eb->conf->zoom_factor + 1), h);
	evas_object_move(eb->event_object, x - w * eb->conf->zoom_factor, y);
     }
   else if (edge == E_GADMAN_EDGE_TOP)
     {
	evas_object_resize(eb->event_object, w , h * (eb->conf->zoom_factor + 1));
     }
   else
     {
	evas_object_resize(eb->event_object, w , h * (eb->conf->zoom_factor + 1));
	evas_object_move(eb->event_object, x, y - h * eb->conf->zoom_factor);
     }
//   _engage_bar_motion_handle(eb, ev->canvas.x, ev->canvas.y);
}

static void
_engage_bar_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Engage_Bar *eb;
   Evas_Coord x, y, w, h;

   ev = event_info;
   eb = data;

   eb->zoom = 1.0;
   evas_object_geometry_get(eb->box_object, &x, &y, &w, &h);
   evas_object_move(eb->event_object, x, y);
   evas_object_resize(eb->event_object, w, h);
   _engage_bar_motion_handle(eb, ev->canvas.x, ev->canvas.y);
}

static void
_engage_bar_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Engage_Bar *eb;

   ev = event_info;
   eb = data;
   if (ev->button == 3)
     {
	e_menu_activate_mouse(eb->menu, e_zone_current_get(eb->con),
			      ev->output.x, ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN);
	e_util_container_fake_mouse_up_all_later(eb->con);
     }
}

static void
_engage_bar_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Engage_Bar *eb;

   ev = event_info;
   eb = data;
}

static void
_engage_bar_cb_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Engage_Bar *eb;

   ev = event_info;
   eb = data;
   _engage_bar_motion_handle(eb, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_engage_bar_cb_gmc_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change)
{
   Engage_Bar *eb;

   eb = data;
   switch (change)
     {
      case E_GADMAN_CHANGE_MOVE_RESIZE:
	 e_gadman_client_geometry_get(eb->gmc, &eb->x, &eb->y, &eb->w, &eb->h);

	 edje_extern_object_min_size_set(eb->box_object, eb->w, eb->h);
	 edje_object_part_swallow(eb->bar_object, "items", eb->box_object);

	 evas_object_move(eb->bar_object, eb->x, eb->y);
	 evas_object_resize(eb->bar_object, eb->w, eb->h);

	 break;
      case E_GADMAN_CHANGE_EDGE:
	 _engage_bar_edge_change(eb, e_gadman_client_edge_get(eb->gmc));
	 break;
      case E_GADMAN_CHANGE_RAISE:
      case E_GADMAN_CHANGE_ZONE:
	 /* FIXME
	  * Must we do something here?
	  */
	 break;
     }
}

static void
_engage_bar_iconsize_change(Engage_Bar *eb)
{
   Evas_List *l;
   Evas_Coord border;
   int done_mins;

   e_box_freeze(eb->box_object);
   for (l = eb->icons; l; l = l->next)
     {
	Engage_Icon *ic;
	Evas_Object *o;
	Evas_Coord bw, bh;

	ic = l->data;
	o = ic->icon_object;
	edje_extern_object_min_size_set(o, eb->engage->conf->iconsize, eb->engage->conf->iconsize);
	evas_object_resize(o, eb->engage->conf->iconsize, eb->engage->conf->iconsize);
	
	edje_object_size_min_calc(ic->bg_object, &border, NULL);

	edje_object_part_swallow(ic->bg_object, "item", o);
	edje_object_size_min_calc(ic->bg_object, &bw, &bh);

	e_box_pack_options_set(ic->bg_object,
	      1, 1, /* fill */
	      0, 0, /* expand */
	      0.5, 0.5, /* align */
	      bw, bh, /* min */
	      bw, bh /* max */
	      );
     }
   eb->engage->iconbordersize = border;
   
   e_box_thaw(eb->box_object);
   _engage_bar_frame_resize(eb);
}

static void
_engage_bar_cb_menu_enabled(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Engage_Bar *eb;
   unsigned char enabled;

   eb = data;
   enabled = e_menu_item_toggle_get(mi);
   if ((eb->conf->enabled) && (!enabled))
     {  
	_engage_bar_disable(eb);
     }
   else if ((!eb->conf->enabled) && (enabled))
     { 
	_engage_bar_enable(eb);
     }
}

static void
_engage_bar_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Engage_Bar *eb;

   eb = data;
   e_gadman_mode_set(eb->gmc->gadman, E_GADMAN_MODE_EDIT);
}

static void
_engage_bar_cb_menu_zoom(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Engage_Bar *eb;

   eb = data;
   eb->conf->zoom = e_menu_item_toggle_get(mi);
   e_config_save_queue();
}


/* engage ported functions */


static int
zoom_function(double d, double *zoom, double *disp, Engage_Bar *eb)
{
   double          range, f, x;
   double          ff, sqrt_ffxx, sqrt_ff_1;

   range = 1.0;
   f = 1.5;
   x = d / range;

   /* some more vars to save computing things over and over */
   ff = f * f;
   sqrt_ffxx = sqrt(ff - x * x);
   sqrt_ff_1 = sqrt(ff - 1.0);

   if (eb->zoom == 1.0)
     {
	*disp = d * eb->engage->iconbordersize;
	*zoom = 1.0;
	return 0;
     }

   if (d > -range && d < range)
     {
	*zoom = (eb->zoom - 1.0) * (eb->conf->zoom_factor - 1.0) *
	    ((sqrt_ff_1 - sqrt_ffxx) / (sqrt_ff_1 - f)) + 1.0;
/* disp is not currently used, so free up some cycles */
#if 0
	*disp = (eb->engage->iconbordersize) *
	    ((eb->zoom - 1.0) * (eb->conf->zoom_factor - 1.0) *
	    (range * (x * (2 * sqrt_ff_1 - sqrt_ffxx) -
		ff * atan(x / sqrt_ffxx)) / (2.0 * (sqrt_ff_1 - f))) + d);
#endif
      } else {
	*zoom = 1.0;
#if 0
	*disp = (eb->engage->iconbordersize) *
	    ((eb->zoom - 1.0) * (eb->conf->zoom_factor - 1.0) *
	        (range * (sqrt_ff_1 - ff * atan(1.0 / sqrt_ff_1)) /
	        (2.0 * (sqrt_ff_1 - f))) + range + fabs(d) - range);
	if (d < 0.0)
	   *disp = -(*disp);
#endif
      }
   return 1;
}

