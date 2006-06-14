#include <e.h>
#include <linux/kernel.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Uptime Uptime;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *ut_obj;
   Uptime *ut;
   Ecore_Timer *monitor;
   int uptime;
};

struct _Uptime 
{
   Instance *inst;
   Evas_Object *ut_obj;
};

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);

static void _ut_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ut_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _ut_menu_cb_post(void *data, E_Menu *m);
static Config_Item *_ut_config_item_get(const char *id);
static Uptime *_ut_new(Evas *evas);
static void _ut_free(Uptime *ut);
static int _ut_cb_check(void *data);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *ut_config = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION,
     "uptime", {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon}
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Config_Item *ci;
   Uptime *ut;
   char buf[4096];
   struct sysinfo s_info;
   
   inst = E_NEW(Instance, 1);
   ci = _ut_config_item_get(id);
   if (!ci->id) ci->id = evas_stringshare_add(id);
   
   ut = _ut_new(gc->evas);
   ut->inst = inst;
   inst->ut = ut;
   
   o = ut->ut_obj;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->ut_obj = o;
   
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _ut_cb_mouse_down, inst);
   ut_config->instances = evas_list_append(ut_config->instances, inst);

   sysinfo(&s_info);
   inst->uptime = s_info.uptime;
   
   if (!inst->monitor)
     inst->monitor = ecore_timer_add(ci->check_interval, _ut_cb_check, inst);
   
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   Uptime *ut;
   
   inst = gcc->data;
   ut = inst->ut;

   if (inst->monitor) ecore_timer_del(inst->monitor);
   
   ut_config->instances = evas_list_remove(ut_config->instances, inst);
   evas_object_event_callback_del(ut->ut_obj, EVAS_CALLBACK_MOUSE_DOWN, _ut_cb_mouse_down);

   _ut_free(ut);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc) 
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static char *
_gc_label(void) 
{
   return D_("Uptime");
}

static Evas_Object *
_gc_icon(Evas *evas) 
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/module.eap", e_module_dir_get(ut_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static void 
_ut_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   
   if (ut_config->menu) return;
   
   inst = data;
   ev = event_info;
   if (ev->button == 3)
     {
	E_Menu *mn;
	E_Menu_Item *mi;
	int x, y, w, h;
	
	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _ut_menu_cb_post, inst);
	ut_config->menu = mn;
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, D_("Configuration"));
	e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
	e_menu_item_callback_set(mi, _ut_menu_cb_configure, inst);
	
	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);
	
        e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
        e_menu_activate_mouse(mn, e_util_zone_current_get(e_manager_current_get()),
                              x + ev->output.x, y + ev->output.y, 1, 1, E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_ut_menu_cb_post(void *data, E_Menu *m) 
{
   if (!ut_config->menu) return;
   e_object_del(E_OBJECT(ut_config->menu));
   ut_config->menu = NULL;
}

static void
_ut_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _ut_config_item_get(inst->gcc->id);
   _config_ut_module(ci);
}

static Config_Item *
_ut_config_item_get(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   char buf[4096];
   
   for (l = ut_config->items; l; l = l->next) 
     {
	ci = l->data;
	if (!ci->id) continue;
	if (!strcmp(ci->id, id)) return ci;
     }
   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->check_interval = 60.0;
   
   ut_config->items = evas_list_append(ut_config->items, ci);
   return ci;
}

void
_ut_config_updated(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   
   if (!ut_config) return;
   ci = _ut_config_item_get(id);
   for (l = ut_config->instances; l; l = l->next) 
     {
	Instance *inst;
	
	inst = l->data;
	if (!inst->gcc->id) continue;
	if (!strcmp(inst->gcc->id, ci->id)) 
	  {
	     if (inst->monitor) ecore_timer_del(inst->monitor);
	     inst->monitor = ecore_timer_add(ci->check_interval, _ut_cb_check, inst);
	     break;
	  }
     }
   return;
}

EAPI E_Module_Api e_modapi = {
   E_MODULE_API_VERSION,
   "Uptime"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Uptime_Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, check_interval, DOUBLE);
   
   conf_edd = E_CONFIG_DD_NEW("Uptime_Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);
   
   ut_config = e_config_domain_load("module.uptime", conf_edd);
   if (!ut_config) 
     {
	Config_Item *ci;
	char buf[4096];
	
	ut_config = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);
	ci->id = evas_stringshare_add("0");
	ci->check_interval = 60.0;
	ut_config->items = evas_list_append(ut_config->items, ci);
     }
   ut_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return 1;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   ut_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);
   
   if (ut_config->config_dialog) 
     e_object_del(E_OBJECT(ut_config->config_dialog));
   
   if (ut_config->menu) 
     {
	e_menu_post_deactivate_callback_set(ut_config->menu, NULL, NULL);
	e_object_del(E_OBJECT(ut_config->menu));
	ut_config->menu = NULL;
     }
   while (ut_config->items) 
     {
	Config_Item *ci;
	
	ci = ut_config->items->data;
	ut_config->items = evas_list_remove_list(ut_config->items, ut_config->items);
	if (ci->id) evas_stringshare_del(ci->id);
	free(ci);
     }
   free(ut_config);
   ut_config = NULL;
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   Evas_List *l;
   
   for (l = ut_config->instances; l; l = l->next) 
     {
	Instance *inst;
	Config_Item *ci;
	
	inst = l->data;
	ci = _ut_config_item_get(inst->gcc->id);
	if (ci->id) evas_stringshare_del(ci->id);
	ci->id = evas_stringshare_add(inst->gcc->id);
     }
   e_config_domain_save("module.uptime", conf_edd, ut_config);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m)
{
   e_module_dialog_show(D_("Enlightenment Uptime Monitor Module"), 
			D_("This module is used to monitor uptime."));
   return 1;
}

static Uptime *
_ut_new(Evas *evas) 
{
   Uptime *ut;
   char buf[4096];
   
   ut = E_NEW(Uptime, 1);
   snprintf(buf, sizeof(buf), "%s/uptime.edj", e_module_dir_get(ut_config->module));

   ut->ut_obj = edje_object_add(evas);
   if (!e_theme_edje_object_set(ut->ut_obj, "base/theme/modules/uptime", "modules/uptime/main"))
     edje_object_file_set(ut->ut_obj, buf, "modules/uptime/main");

   evas_object_show(ut->ut_obj);   
   return ut;
}

static void
_ut_free(Uptime *ut) 
{
   evas_object_del(ut->ut_obj);
   free(ut);
}

static int
_ut_cb_check(void *data)
{
   Instance *inst;
   Config_Item *ci;
   long minute = 60;
   long hour = minute * 60;
   long day = hour * 24;
   char u_date_time[256];

   inst = data;
   if (!inst) return 0;

   ci = _ut_config_item_get(inst->gcc->id);
   if (!ci) return 0;
   
   inst->uptime += (1 * ci->check_interval);
   
   sprintf(u_date_time, D_("uptime: %ld days, %ld:%02ld:%02ld"),
           inst->uptime / day, (inst->uptime % day) / hour, (inst->uptime % hour) / minute, inst->uptime % minute);
   edje_object_part_text_set(inst->ut->ut_obj, "uptime", u_date_time);
   
   return 1;
}
