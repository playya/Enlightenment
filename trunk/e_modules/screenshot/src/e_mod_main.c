#include <e.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_X.h>
#include "config.h"
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include "e_mod_main.h"

static E_Gadcon_Client *_gc_init        (E_Gadcon *gc, const char *name, 
					 const char *id, const char *style);
static void             _gc_shutdown    (E_Gadcon_Client *gcc);
static void             _gc_orient      (E_Gadcon_Client *gcc);
static char            *_gc_label       (void);
static Evas_Object     *_gc_icon        (Evas *evas);
static const char      *_gc_id_new      (void);
static void             _cfg_free       (void);
static int              _cfg_timer      (void *data);
static void             _cfg_new        (void);
static void             _cb_mouse_down  (void *data, Evas *evas, 
					 Evas_Object *obj, void *event_info);
static void             _cb_menu_post   (void *data, E_Menu *menu);
static void             _cb_menu_cfg    (void *data, E_Menu *menu, 
					 E_Menu_Item *mi);
static void             _cb_normal      (void *data, E_Menu *menu, 
					 E_Menu_Item *mi);
static void             _cb_window      (void *data, E_Menu *menu, 
					 E_Menu_Item *mi);
static void             _cb_region      (void *data, E_Menu *menu, 
					 E_Menu_Item *mi);
static void             _cb_start_shot  (void *data, Evas_Object *obj, 
					 const char *emission, 
					 const char *source);
static void             _cb_exec_shot   (void *data, Evas_Object *obj, 
					 const char *emission, 
					 const char *source);
static void             _cb_dialog_ok   (char *text, void *data);
static void             _cb_send_msg    (void *data);
static void             _cb_do_shot     (void);
static void             _cb_take_shot   (E_Object *obj, const char *params);

static Evas_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Action *act = NULL;
E_Module *ss_mod = NULL;
Config *cfg = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "screenshot", 
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL},
   E_GADCON_CLIENT_STYLE_PLAIN
};

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Screenshot"};

/* module functions */
EAPI void *
e_modapi_init(E_Module *m) 
{
   /* register config dialog for panel */
   e_configure_registry_category_add("screenshot", 110, "Screenshot", 
				     NULL, "enlightenment/appearance");
   e_configure_registry_item_add("screenshot/screenshot", 10, "Screenshot", 
				 NULL, "enlightenment/appearance", 
				 e_int_config_screenshot_module);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, mode, INT);
   E_CONFIG_VAL(D, T, quality, INT);
   E_CONFIG_VAL(D, T, thumb_size, INT);
   E_CONFIG_VAL(D, T, delay, DOUBLE);
   E_CONFIG_VAL(D, T, prompt, UCHAR);
   E_CONFIG_VAL(D, T, use_app, UCHAR);
   E_CONFIG_VAL(D, T, use_bell, UCHAR);
   E_CONFIG_VAL(D, T, use_thumb, UCHAR);
   E_CONFIG_VAL(D, T, location, STR);
   E_CONFIG_VAL(D, T, filename, STR);
   E_CONFIG_VAL(D, T, app, STR);

   cfg = e_config_domain_load("module.screenshot", conf_edd);
   if (cfg) 
     {
	if ((cfg->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
	  {
	     /* Config Too Old */
	     _cfg_free();
	     ecore_timer_add(1.0, _cfg_timer,
			     _("Screenshot Module Configuration data needed "
			       "upgrading. Your old configuration<br> has been"
			       " wiped and a new set of defaults initialized. "
			       "This<br>will happen regularly during "
			       "development, so don't report a<br>bug. "
			       "This simply means Screenshot module needs "
			       "new configuration<br>data by default for "
			       "usable functionality that your old<br>"
			       "configuration simply lacks. This new set of "
			       "defaults will fix<br>that by adding it in. "
			       "You can re-configure things now to your<br>"
			       "liking. Sorry for the inconvenience.<br>"));
	  }
	else if (cfg->version > MOD_CONFIG_FILE_VERSION) 
	  {
	     /* Config Too New */
	     _cfg_free();
	     ecore_timer_add(1.0, _cfg_timer, 
			     _("Your Screenshot Module configuration is NEWER "
			       "than the Screenshot Module version. This is "
			       "very<br>strange. This should not happen unless"
			       " you downgraded<br>the Screenshot Module or "
			       "copied the configuration from a place where"
			       "<br>a newer version of the Screenshot Module "
			       "was running. This is bad and<br>as a "
			       "precaution your configuration has been now "
			       "restored to<br>defaults. Sorry for the "
			       "inconvenience.<br>"));
	  }
     }

   if (!cfg) _cfg_new();

   /* register actions for keybindings */
   act = e_action_add("screenshot");
   if (act) 
     {
	act->func.go = _cb_take_shot;
	e_action_predef_name_set("Screenshot", "Take Screenshot", 
				 "screenshot", NULL, NULL, 0);	
     }

   ss_mod = m;
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   if (cfg->cfd) e_object_del(E_OBJECT(cfg->cfd));

   if (act) 
     {
	e_action_predef_name_del("Screenshot", "Take Screenshot");
	e_action_del("screenshot");
	act = NULL;
     }

   e_configure_registry_item_del("screenshot/screenshot");
   e_configure_registry_category_del("screenshot");

   e_gadcon_provider_unregister(&_gc_class);
   ss_mod = NULL;
   _cfg_free();
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.screenshot", conf_edd, cfg);
   return 1;
}

/* Gadcon Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst = NULL;
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", ss_mod->dir);

   inst = E_NEW(Instance, 1);

   inst->o_base = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(inst->o_base, "base/theme/modules/screenshot", 
				"modules/screenshot/main"))
     edje_object_file_set(inst->o_base, buf, "modules/screenshot/main");

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_base);
   inst->gcc->data = inst;

   edje_object_signal_callback_add(inst->o_base, "e,action,screenshot,start", 
				   "*", _cb_start_shot, inst);
   edje_object_signal_callback_add(inst->o_base, "e,action,screenshot,exec", 
				   "*", _cb_exec_shot, inst);
   evas_object_event_callback_add(inst->o_base, EVAS_CALLBACK_MOUSE_DOWN, 
				  _cb_mouse_down, inst);

   instances = evas_list_append(instances, inst);
   return inst->gcc;
}

static void 
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   instances = evas_list_remove(instances, inst);
   if (inst->menu) 
     {
	e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
	e_object_del(E_OBJECT(inst->menu));
	inst->menu = NULL;
     }
   if (inst->o_base) 
     {
	evas_object_event_callback_del(inst->o_base, EVAS_CALLBACK_MOUSE_DOWN,
				       _cb_mouse_down);
	evas_object_del(inst->o_base);
     }
   E_FREE(inst);
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
   return "Screenshot";
}

static Evas_Object *
_gc_icon(Evas *evas) 
{
   Evas_Object *o = NULL;
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s/e-module-screenshot.edj", ss_mod->dir);
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(void) 
{
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s.%d", _gc_class.name, 
	    evas_list_count(instances));
   return strdup(buf);
}

/* private module functions */
static void 
_cfg_free(void) 
{
   if (cfg->location) evas_stringshare_del(cfg->location);
   if (cfg->filename) evas_stringshare_del(cfg->filename);
   if (cfg->app) evas_stringshare_del(cfg->app);
   E_FREE(cfg);
}

static int 
_cfg_timer(void *data) 
{
   e_util_dialog_show("Screenshot Configuration Updated", data);
   return 0;
}

static void 
_cfg_new(void) 
{
   cfg = E_NEW(Config, 1);
   cfg->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) \
   if ((cfg->version & 0xffff) < v) {
#define IFMODCFGEND }

   IFMODCFG(0x008d);
   cfg->mode = 0;
   cfg->quality = 75;
   cfg->thumb_size = 50;
   cfg->delay = 60.0;
   cfg->prompt = 0;
   cfg->use_app = 0;
   cfg->use_bell = 1;
   cfg->use_thumb = 0;
   cfg->location = evas_stringshare_add(e_user_homedir_get());
   cfg->filename = NULL;
   cfg->app = NULL;
   IFMODCFGEND;

   cfg->version = MOD_CONFIG_FILE_VERSION;

   E_CONFIG_LIMIT(cfg->mode, 0, 2);
   E_CONFIG_LIMIT(cfg->quality, 1, 100);
   E_CONFIG_LIMIT(cfg->delay, 0.0, 60.0);
   E_CONFIG_LIMIT(cfg->thumb_size, 10, 100);
   e_config_save_queue();
}

static void 
_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info) 
{
   Instance *inst = NULL;
   Evas_Event_Mouse_Down *ev;
   E_Menu *mn = NULL;
   E_Menu_Item *mi = NULL;
   E_Zone *zone = NULL;
   int x, y;

   if (!(inst = data)) return;
   ev = event_info;
   if ((ev->button == 3) && (!inst->menu)) 
     {
	zone = e_util_zone_current_get(e_manager_current_get());

	mn = e_menu_new();
	inst->menu_mode = mn;

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, "Whole Screen");
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (cfg->mode == 0) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_normal, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, "Select Window");
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (cfg->mode == 1) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_window, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, "Select Region");
	e_menu_item_radio_group_set(mi, 1);
	e_menu_item_radio_set(mi, 1);
	if (cfg->mode == 2) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cb_region, inst);

	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _cb_menu_post, inst);
	inst->menu = mn;

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, "Capture Mode");
	e_menu_item_submenu_set(mi, inst->menu_mode);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, "Configuration");
	e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
	e_menu_item_callback_set(mi, _cb_menu_cfg, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);

	e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
	e_menu_activate_mouse(mn, zone, x + ev->output.x, y + ev->output.y, 
			      1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
	evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, 
				 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void 
_cb_menu_post(void *data, E_Menu *menu) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (!inst->menu) return;
   if (inst->menu_mode) e_object_del(E_OBJECT(inst->menu_mode));
   inst->menu_mode = NULL;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void 
_cb_menu_cfg(void *data, E_Menu *menu, E_Menu_Item *mi)
{
   E_Container *con;

   con = e_container_current_get(e_manager_current_get());
   e_int_config_screenshot_module(con, NULL);
}

static void 
_cb_normal(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   cfg->mode = 0;
   e_config_save_queue();
}

static void 
_cb_window(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   cfg->mode = 1;
   e_config_save_queue();
}

static void 
_cb_region(void *data, E_Menu *menu, E_Menu_Item *mi) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   cfg->mode = 2;
   e_config_save_queue();
}

static void 
_cb_start_shot(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   if (cfg->prompt)
     {
	e_entry_dialog_show("Screenshot Module", "enlightenment/e", 
			    "Enter a new filename for this screenshot",
			    NULL, NULL, NULL, _cb_dialog_ok, NULL, inst);
     }
   else 
     _cb_send_msg(inst);
}

static void 
_cb_exec_shot(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
   Instance *inst = NULL;
   Ecore_Exe *exe;
   char buf[4096];

   if (!(inst = data)) return;

   _cb_do_shot();
   edje_object_signal_emit(inst->o_base, "e,action,screenshot,stop", "");
   edje_object_message_signal_process(inst->o_base);
}

static void 
_cb_dialog_ok(char *text, void *data) 
{
   Instance *inst = NULL;
   char buf[4096];
   char *t = NULL;

   if (!(inst = data)) return;
   if (!text) return;
   t = ecore_file_dir_get(text);
   if (!strcmp(t, text)) 
     {
	snprintf(buf, sizeof(buf), "%s/%s", cfg->location, 
		 ecore_file_file_get(text));
     }
   else
     snprintf(buf, sizeof(buf), "%s", text);

   if (cfg->filename) evas_stringshare_del(cfg->filename);
   cfg->filename = evas_stringshare_add(buf);

   _cb_send_msg(inst);
}

static void 
_cb_send_msg(void *data) 
{
   Instance *inst = NULL;
   Edje_Message_Int_Set *msg = NULL;

   if (!(inst = data)) return;
   if (cfg->delay <= 0.0) return;
   msg = malloc(sizeof(Edje_Message_Int_Set) + 1 * sizeof(int));
   msg->count = 1;
   msg->val[0] = cfg->delay;
   edje_object_message_send(inst->o_base, EDJE_MESSAGE_INT_SET, 1, msg);
   free(msg);
   msg = NULL;
}

static void 
_cb_do_shot(void) 
{
   Ecore_Exe *exe;
   char *tmp;
   char buf[4096];

   tmp = strdup("");
   if (cfg->use_bell) 
     {
	snprintf(buf, sizeof(buf), "--beep ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if (cfg->quality > 0) 
     {
	snprintf(buf, sizeof(buf), "--quality %d ", cfg->quality);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   switch (cfg->mode) 
     {
      case 0:
	break;
      case 1:
	snprintf(buf, sizeof(buf), "--window ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
      case 2:
	snprintf(buf, sizeof(buf), "--region ");
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
	break;
     }

   if ((cfg->use_app) && (cfg->app)) 
     {
	snprintf(buf, sizeof(buf), "--app %s ", cfg->app);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((cfg->use_thumb) && (cfg->thumb_size > 0)) 
     {
	snprintf(buf, sizeof(buf), "--thumb-geom %d ", cfg->thumb_size);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }

   if ((cfg->prompt) && (cfg->filename))
     {
	snprintf(buf, sizeof(buf), "%s", cfg->filename);
	tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	strcat(tmp, buf);
     }
   else 
     {
	if ((cfg->location) && (cfg->filename)) 
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", cfg->location, 
		      cfg->filename);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);	     
	  }
	else if (cfg->location) 
	  {
	     snprintf(buf, sizeof(buf), "%s", cfg->location);
	     tmp = realloc(tmp, strlen(tmp) + strlen(buf) + 1);
	     strcat(tmp, buf);
	  }
     }

   snprintf(buf, sizeof(buf), "emprint %s", tmp);
   exe = ecore_exe_run(buf, NULL);
   if (exe) ecore_exe_free(exe);
}

static void 
_cb_take_shot(E_Object *obj, const char *params) 
{
   _cb_do_shot();
}
