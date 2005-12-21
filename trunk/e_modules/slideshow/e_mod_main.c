#include <e.h>
#include <E_Lib.h>
#include <Ecore.h>
#ifdef WANT_OSIRIS
# include <Ecore_File.h>
#endif
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "config.h"

int idx, bg_id, bg_count;
static int slide_count;
Ecore_List *list;

static Slide *_slide_init(E_Module *m);
static void _slide_config_menu_new(Slide *e);
static void _slide_shutdown(Slide *e);

static int _slide_face_init(Slide_Face *sf);
static void _slide_face_free(Slide_Face *ef);
static void _slide_face_menu_new(Slide_Face *face);
static void _slide_face_enable(Slide_Face *face);
static void _slide_face_disable(Slide_Face *face);
static void _slide_face_cb_menu_enabled(void *data, E_Menu *m, E_Menu_Item *mi);
static void _slide_face_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void _slide_face_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi);

static void _slide_face_cb_mouse_down(void *data, Evas *e, Evas_Object *obj,void *event_info);
static void _slide_face_cb_gmc_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change);

static int _slide_cb_event_zone_desk_count_set(void *data, int type, void *event);
static int _slide_cb_event_desk_show(void *data, int type, void *event);
static int _slide_cb_check(void *data);

static void get_bg_count();
static void _set_bg(char *bg, Slide_Face *sf);

/* public module routines. all modules must have these */
E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Slideshow"
};

void 
*e_modapi_init(E_Module * m)
{
   Slide *e;

   /* actually init slide */
   e = _slide_init(m);
   m->config_menu = e->config_menu;

   if (!e->display)
     {
        char *tmp = getenv("DISPLAY");
        if (tmp) e->display = strdup(tmp);
     }

    /* make sure the display var is of the form name:0.0 or :0.0 */
   if (e->display)
     {
        char *p;
        char buf[1024];

        p = strrchr(e->display, ':');
        if (!p)
	  {
	     snprintf(buf, sizeof(buf), "%s:0.0", e->display);
	     free(e->display);
	     e->display = strdup(buf);
	  }
        else
	  {
	     p = strrchr(p, '.');
	     if (!p)
	       {
		  snprintf(buf, sizeof(buf), "%s.0", e->display);
		  free(e->display);
		  e->display = strdup(buf);
	       }
	  }
     }
   else
     e->display = strdup(":0.0");

   /* Init E Lib */
   if (e->display) e_lib_init(e->display);
   if (!e->display) e_lib_init(":0.0");

   return e;
}

int 
e_modapi_shutdown(E_Module *m)
{
   Slide *s;

   s = m->data;
   if (s)
     {
	if (m->config_menu)
	  {
	     e_menu_deactivate(m->config_menu);
	     e_object_del(E_OBJECT(m->config_menu));
	     m->config_menu = NULL;
	  }
	_slide_shutdown(s);
     }

   e_lib_shutdown();
   return 1;
}

int 
e_modapi_save(E_Module *m)
{
   Slide *e;

   e = m->data;
   if (e)
     e_config_domain_save("module.slideshow", e->conf_edd, e->conf);
   return 1;
}

int 
e_modapi_info(E_Module * m)
{
   m->icon_file = strdup(PACKAGE_DATA_DIR"/module_icon.png");
   return 1;
}

int 
e_modapi_about(E_Module * m)
{
   e_module_dialog_show(_("Enlightenment Slide Show Module"),
			("This module is VERY simple and is used to cycle desktop backgrounds"));
   return 1;
}

int 
e_modapi_config(E_Module *m) 
{
   Slide *s;
   E_Container *con;
   
   s = m->data;
   if (!s) return 0;
   con = e_container_current_get(e_manager_current_get());
   _config_slideshow_module(con, s);
   return 1;
}

/* Begin Private Routines */

static Slide 
*_slide_init(E_Module *m)
{
   Slide *e;
   E_Menu_Item *mi;
   Evas_List *managers, *l, *l2;

   e = E_NEW(Slide, 1);
   if (!e) return NULL;

   e->conf_edd = E_CONFIG_DD_NEW("Slide_Config", Config);
#undef T
#undef D
#define T Config
#define D e->conf_edd
#ifdef WANT_OSIRIS
   E_CONFIG_VAL(D, T, theme, STR);
#endif
   E_CONFIG_VAL(D, T, cycle_time, DOUBLE);

   e->conf = e_config_domain_load("module.slideshow", e->conf_edd);
   if (!e->conf)
     {
	e->conf = E_NEW(Config, 1);
	#ifdef WANT_OSIRIS
	e->conf->theme = (char *)evas_stringshare_add("");
	#endif
	e->conf->cycle_time = 600;
     }

   E_CONFIG_LIMIT(e->conf->cycle_time, 5.0, 600.0);

   _slide_config_menu_new(e);

   /* Setup Event Handlers */
   e->ev_handler_zone_desk_count_set = ecore_event_handler_add(E_EVENT_ZONE_DESK_COUNT_SET, _slide_cb_event_zone_desk_count_set, e);
   e->ev_handler_desk_show = ecore_event_handler_add(E_EVENT_DESK_SHOW, _slide_cb_event_desk_show, e);

   /* Managers */
   managers = e_manager_list ();
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;

	man = l->data;
	for (l2 = man->containers; l2; l2 = l2->next)
	  {
	     E_Container *con;
	     E_Zone *zone;
	     Slide_Face *ef;

	     con = l2->data;
	     zone = e_zone_current_get(con);
	     if (!zone) return NULL;

	     ef = E_NEW(Slide_Face, 1);
	     if (ef)
	       {
		  ef->conf_face_edd = E_CONFIG_DD_NEW("Slide_Config_Face", Config_Face);
#undef T
#undef D
#define T Config_Face
#define D ef->conf_face_edd
		  E_CONFIG_VAL(D, T, enabled, UCHAR);

		  e->face = ef;
		  ef->slide = e;
		  ef->con = con;
		  ef->zone = zone;
		  ef->evas = con->bg_evas;

		  ef->conf = E_NEW(Config_Face, 1);
		  ef->conf->enabled = 1;

		  if (!_slide_face_init(ef)) return NULL;

		  /* Menu */
		  /* This menu must be initialized after conf */
		  _slide_face_menu_new(ef);

		  /* Add main menu to face menu */
		  mi = e_menu_item_new(e->config_menu);
		  e_menu_item_label_set(mi, _("Configuration"));
		  e_menu_item_callback_set(mi, _slide_face_cb_menu_configure, ef);

		  mi = e_menu_item_new(e->config_menu);
		  e_menu_item_label_set(mi, con->name);
		  e_menu_item_submenu_set(mi, ef->menu);

		  /* Setup */
		  if (!ef->conf->enabled)
		    {
		       _slide_face_disable(ef);
		    }
		  else
		    {
		       _slide_face_enable(ef);
		    }
	       }
	  }
     }

   return e;
}

static void 
_slide_shutdown(Slide *e)
{
   if (list) ecore_list_destroy(list);

   _slide_face_free(e->face);

   if (e->cycle_timer)
     e->cycle_timer = ecore_timer_del(e->cycle_timer);

   if (e->ev_handler_zone_desk_count_set)
     ecore_event_handler_del(e->ev_handler_zone_desk_count_set);

   if (e->ev_handler_desk_show)
     ecore_event_handler_del(e->ev_handler_desk_show);
   #ifdef WANT_OSIRIS
   //evas_stringshare_del(e->conf->theme);
   #endif
   free(e->conf);
   E_CONFIG_DD_FREE(e->conf_edd);
   free(e);
}

static void 
_slide_config_menu_new(Slide * e)
{
   E_Menu *mn;

   mn = e_menu_new();
   e->config_menu = mn;
}

static int 
_slide_face_init(Slide_Face *sf)
{
   Evas_Object *o;

   sf->desk_x_current = sf->zone->desk_x_current;
   sf->desk_y_current = sf->zone->desk_y_current;

   e_object_ref(E_OBJECT(sf->zone));

   evas_event_freeze(sf->evas);
   o = edje_object_add(sf->evas);
   sf->slide_object = o;

   edje_object_file_set(o, PACKAGE_DATA_DIR"/slideshow.edj", "modules/slideshow/main");
   evas_object_show(o);

   o = evas_object_rectangle_add(sf->evas);
   sf->event_object = o;
   evas_object_layer_set(o, 2);
   evas_object_repeat_events_set(o, 1);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,_slide_face_cb_mouse_down, sf);
   evas_object_show(o);

   sf->gmc = e_gadman_client_new(sf->con->gadman);
   e_gadman_client_domain_set(sf->gmc, "module.slideshow", slide_count++);
   e_gadman_client_policy_set(sf->gmc,E_GADMAN_POLICY_ANYWHERE | E_GADMAN_POLICY_HMOVE |
			      E_GADMAN_POLICY_VMOVE | E_GADMAN_POLICY_HSIZE | E_GADMAN_POLICY_VSIZE);
   e_gadman_client_min_size_set(sf->gmc, 4, 4);
   e_gadman_client_max_size_set(sf->gmc, 128, 128);
   e_gadman_client_auto_size_set(sf->gmc, 40, 40);
   e_gadman_client_align_set(sf->gmc, 1.0, 1.0);
   e_gadman_client_resize(sf->gmc, 40, 40);
   e_gadman_client_change_func_set(sf->gmc, _slide_face_cb_gmc_change, sf);
   e_gadman_client_load(sf->gmc);
   evas_event_thaw(sf->evas);

   return 1;
}

static void 
_slide_face_free(Slide_Face * ef)
{
   if (ef->menu) e_object_del(E_OBJECT(ef->menu));
   if (ef->event_object) evas_object_del(ef->event_object);
   if (ef->slide_object) evas_object_del(ef->slide_object);
   if (ef->gmc) e_gadman_client_save(ef->gmc);
   if (ef->gmc) e_object_del(E_OBJECT(ef->gmc));
   if (ef->zone) e_object_unref(E_OBJECT(ef->zone));

   E_FREE(ef->conf);
   E_FREE(ef);
   slide_count--;
}

static void 
_slide_face_menu_new(Slide_Face * face)
{
   E_Menu *mn;
   E_Menu_Item *mi;

   mn = e_menu_new();
   face->menu = mn;

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Configuration"));
   e_menu_item_callback_set(mi, _slide_face_cb_menu_configure, face);
   /* Edit */
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Edit Mode"));
   e_menu_item_callback_set(mi, _slide_face_cb_menu_edit, face);
}

static void 
_slide_face_enable(Slide_Face * face)
{
   face->conf->enabled = 1;
   e_config_save_queue();
   evas_object_show(face->slide_object);
   evas_object_show(face->event_object);
   if (face->slide->cycle_timer)
     {
	if (face->slide->conf->cycle_time != 0)
	  {
	     ecore_timer_interval_set(face->slide->cycle_timer, face->slide->conf->cycle_time);
	  }
	else
	  {
	     face->slide->cycle_timer = ecore_timer_del(face->slide->cycle_timer);
	  }
     }
   else
     {
	face->slide->cycle_timer = ecore_timer_add(face->slide->conf->cycle_time, _slide_cb_check, face);
     }
}

static void 
_slide_face_disable(Slide_Face * face)
{
   face->conf->enabled = 0;
   e_config_save_queue();
   evas_object_hide(face->slide_object);
   evas_object_hide(face->event_object);
   if (face->slide->cycle_timer)
     face->slide->cycle_timer = ecore_timer_del(face->slide->cycle_timer);
}

static void
_slide_face_cb_gmc_change(void *data, E_Gadman_Client * gmc, E_Gadman_Change change)
{
   Slide_Face *ef;
   Evas_Coord x, y, w, h;

   ef = data;
   switch (change)
     {
      case E_GADMAN_CHANGE_MOVE_RESIZE:
	e_gadman_client_geometry_get(ef->gmc, &x, &y, &w, &h);
	evas_object_move(ef->slide_object, x, y);
	evas_object_move(ef->event_object, x, y);
	evas_object_resize(ef->slide_object, w, h);
	evas_object_resize(ef->event_object, w, h);
	break;
      case E_GADMAN_CHANGE_RAISE:
	evas_object_raise(ef->slide_object);
	evas_object_raise(ef->event_object);
	break;
     }
}

static void
_slide_face_cb_mouse_down(void *data, Evas * e, Evas_Object * obj,void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Slide_Face *ef;
   Slide *es;

   ev = event_info;
   ef = data;
   es = ef->slide;

   if (ev->button == 3)
     {
	e_menu_activate_mouse(ef->menu, ef->zone, ev->output.x, ev->output.y, 1, 1, E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	e_util_container_fake_mouse_up_all_later(ef->zone->container);
     }
   else if (ev->button == 2)
     {
	if (es->cycle_timer)
	  {
	     es->cycle_timer = ecore_timer_del(es->cycle_timer);
	     es->cycle_timer = NULL;
	  }
	else
	  {
	     es->cycle_timer = ecore_timer_add(es->conf->cycle_time, _slide_cb_check, ef);
	  }
     }
   else if (ev->button == 1)
     {
	_slide_cb_check(ef);
     }
}

static int 
_slide_cb_check(void *data)
{
   char *bg;
   E_Zone *zone;
   Evas_List *l;
   Slide_Face *ef = data;
   
#ifdef WANT_OSIRIS
   Slide *e;   
   e = ef->slide;
   get_bg_count(e->conf->theme);
#else
   get_bg_count(NULL);
#endif

   if (!ef) return 0;
   if (!ef->conf) return 0;
   if (!ef->con) return 0;
   if (!ef->zone) return 0;

   if (idx > bg_count) idx = 0;

   if (idx <= bg_count)
     {
	bg = ecore_list_goto_index(list, idx);
	if (bg == NULL)
	  {
	     idx = 0;
	     bg = ecore_list_goto_index(list, idx);
	  }
	if (bg != NULL)
	  {
	     _set_bg(bg, ef);
	     idx++;
	  }
     }
   if (ef->conf->enabled == 0) return 0;
   return 1;
}

/*
static void 
_slide_face_cb_menu_enabled(void *data, E_Menu * m, E_Menu_Item * mi)
{
   Slide_Face *face;
   unsigned char enabled;

   face = data;
   enabled = e_menu_item_toggle_get(mi);
   if ((face->conf->enabled) && (!enabled))
     {
	_slide_face_disable(face);
     }
   else if ((!face->conf->enabled) && (enabled))
     {
	_slide_face_enable(face);
     }
}
*/

static void 
_slide_face_cb_menu_edit(void *data, E_Menu * m, E_Menu_Item * mi)
{
   Slide_Face *face;

   face = data;
   e_gadman_mode_set(face->gmc->gadman, E_GADMAN_MODE_EDIT);
}

static void 
get_bg_count(char *name)
{
   char *list_item;
   char *home;
   char buffer[PATH_MAX];

   home = e_user_homedir_get();
#ifdef WANT_OSIRIS
   if (name == NULL)
     {
	snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds", home);
     }
   else
     {
	snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds/%s", home, name);
     }
#else
   snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds", home);
#endif
   bg_count = 0;
   list = ecore_file_ls(strdup(buffer));
   ecore_list_goto_first(list);
   while ((list_item = (char *) ecore_list_next(list)) != NULL)
     bg_count++;
}

int 
get_desk_x_count(E_Zone * zone)
{
   int desks_x, desks_y;

   e_zone_desk_count_get(zone, &desks_x, &desks_y);
   return desks_x;
}

int 
get_desk_y_count(E_Zone * zone)
{
   int desks_x, desks_y;

   e_zone_desk_count_get(zone, &desks_x, &desks_y);
   return desks_y;
}

static int 
_slide_cb_event_zone_desk_count_set(void *data, int type, void *event)
{
   E_Event_Zone_Desk_Count_Set *ev;
   Slide *slide;
   Evas_List *list;
   Slide_Face *face;
   int desks_x, desks_y;

   slide = data;
   ev = event;

   face = slide->face;
   if (face->zone != ev->zone) return 1;
   e_zone_desk_count_get(ev->zone, &desks_x, &desks_y);
   if ((face->numx == desks_x) && (face->numy == desks_y)) return 1;
   face->zone = ev->zone;
   face->numx = desks_x;
   face->numy = desks_y;

   return 1;
}

static int 
_slide_cb_event_desk_show(void *data, int type, void *event)
{
   E_Event_Desk_Show *ev;
   Slide *slide;
   Evas_List *list;
   Slide_Face *face;

   slide = data;
   ev = event;

   face = slide->face;
   if (face->zone != ev->desk->zone) return 1;
   face->zone = ev->desk->zone;
   face->desk_x_current = ev->desk->zone->desk_x_current;
   face->desk_y_current = ev->desk->zone->desk_y_current;

   return 1;
}

static void 
_set_bg(char *bg, Slide_Face *sf)
{
   char buffer[4096];
   char *home;
   E_Zone *zone;
   
   zone = sf->zone;
   home = e_user_homedir_get();

#ifdef WANT_OSIRIS
   Slide *e;
   e = sf->slide;
   if (e->conf->theme == NULL)
     {
	snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds/%s", home, bg);
     }
   else
     {
	snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds/%s/%s", home, e->conf->theme, bg);
     }
#else
   snprintf(buffer, sizeof(buffer), "%s/.e/e/backgrounds/%s", home, bg);
#endif

   if ((zone->container->num == 0) && (zone->num == 0) &&
       (zone->desk_x_current == 0) && (zone->desk_y_current == 0))
     {
	if (buffer) e_lib_background_set(strdup(buffer));
     }
   else
     {
	e_lib_desktop_background_del(zone->container->num, zone->num, zone->desk_x_current, zone->desk_y_current);
	e_lib_desktop_background_add(zone->container->num, zone->num, zone->desk_x_current, zone->desk_y_current, strdup(buffer));
     }
}

static void 
_slide_face_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Slide_Face *sf;
   E_Config_Dialog *cfg;
   
   sf = data;
   if (!sf) return;
   _config_slideshow_module(sf->con, sf->slide);
}

void 
_slide_cb_config_updated(void *data) 
{
   Slide *s;
   
   s = data;
   if (s->conf->cycle_time == 0) 
     {
	if (s->cycle_timer)
	  s->cycle_timer = ecore_timer_del(s->cycle_timer);   	
     }
   else 
     {
	if (s->cycle_timer) ecore_timer_interval_set(s->cycle_timer, s->conf->cycle_time);	
     }
}

