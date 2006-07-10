#include <e.h>
#include <Ecore.h>
#include <Ecore_File.h>
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

typedef struct _Instance Instance;
typedef struct _Screenshot Screenshot;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object *ss_obj;
   Screenshot *ss;
   Ecore_Exe *exe;
   const char *filename;
};

struct _Screenshot
{
   Instance *inst;
   Evas_Object *ss_obj;
};

/* Function Protos for Gadcon Requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);

/* Function Protos for the Module */
static void _ss_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _ss_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _ss_menu_cb_post(void *data, E_Menu *m);
static Config_Item *_ss_config_item_get(const char *id);
static Screenshot *_ss_new(Evas *evas);
static void _ss_free(Screenshot *ss);
static void _ss_handle_mouse_down(Instance *inst);
static int _ss_exe_cb_exit(void *data, int type, void *event);
static void _ss_take_shot(void *data);
static void _ss_get_filename(void *data);
static void _cb_entry_ok(char *text, void *data);

char *_get_import_options(Config_Item *ci);
char *_get_scrot_options(Config_Item *ci);
char *_parse_options(char **opts);
char *_get_filename(Config_Item *ci);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *ss_config = NULL;

/* Define the gadcon class */
static const E_Gadcon_Client_Class _gc_class = {
   GADCON_CLIENT_CLASS_VERSION,
   "screenshot", {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon}
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Config_Item *ci;
   Screenshot *ss;
   char buf[4096];

   inst = E_NEW(Instance, 1);

   ci = _ss_config_item_get(id);
   if (!ci->id) ci->id = evas_stringshare_add(id);

   ss = _ss_new(gc->evas);
   ss->inst = inst;
   inst->ss = ss;

   o = ss->ss_obj;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->ss_obj = o;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _ss_cb_mouse_down, inst);

   ss_config->instances = evas_list_append(ss_config->instances, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   Screenshot *ss;
   
   inst = gcc->data;
   ss = inst->ss;
   
   if (inst->filename) evas_stringshare_del(inst->filename);
   ss_config->instances = evas_list_remove(ss_config->instances, inst);

   evas_object_event_callback_del(ss->ss_obj, EVAS_CALLBACK_MOUSE_DOWN, _ss_cb_mouse_down);
   
   _ss_free(ss);
   free(inst);
   inst = NULL;
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
   return D_("Screenshot");
}

static Evas_Object *
_gc_icon(Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/module.eap", e_module_dir_get(ss_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static void
_ss_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!ss_config->menu))
     {
        E_Menu *mn;
        E_Menu_Item *mi;
        int x, y, w, h;

        mn = e_menu_new();
        e_menu_post_deactivate_callback_set(mn, _ss_menu_cb_post, inst);
        ss_config->menu = mn;

        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, D_("Configuration"));
        e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
        e_menu_item_callback_set(mi, _ss_menu_cb_configure, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);
	
        e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
        e_menu_activate_mouse(mn,
                              e_util_zone_current_get(e_manager_current_get()),
                              x + ev->output.x, y + ev->output.y, 1, 1, E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
   else if ((ev->button == 1) && (inst))
      _ss_handle_mouse_down(inst);
}

static void
_ss_menu_cb_post(void *data, E_Menu *m)
{
   if (!ss_config->menu) return;
   e_object_del(E_OBJECT(ss_config->menu));
   ss_config->menu = NULL;
}

static void
_ss_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;

   inst = data;
   ci = _ss_config_item_get(inst->gcc->id);
   _config_screenshot_module(ci);
}

static Config_Item *
_ss_config_item_get(const char *id)
{
   Evas_List *l;
   Config_Item *ci;

   for (l = ss_config->items; l; l = l->next)
     {
        ci = l->data;
        if (!ci->id) continue;
        if (!strcmp(ci->id, id)) return ci;
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->delay_time = 60.0;
   if (ecore_file_app_installed("import"))
     {
        if (ecore_file_app_installed("scrot"))
          {
             ci->use_import = 0;
             ci->use_scrot = 1;
          }
        else
          {
             ci->use_import = 1;
             ci->use_scrot = 0;
          }
     }
   else if (ecore_file_app_installed("scrot"))
     {
        ci->use_import = 0;
        ci->use_scrot = 1;
     }
   ci->prompt = 0;
   ci->location = evas_stringshare_add(e_user_homedir_get());
   ci->filename = NULL;
   ci->import.use_img_border = 1;
   ci->import.use_dither = 1;
   ci->import.use_frame = 1;
   ci->import.use_mono = 0;
   ci->import.use_window = 0;
   ci->import.use_silent = 1;
   ci->import.use_trim = 1;
   ci->scrot.use_img_border = 1;
   ci->scrot.use_thumb = 0;
   ci->use_app = 0;
   ci->app = evas_stringshare_add("");

   ss_config->items = evas_list_append(ss_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi = {
   E_MODULE_API_VERSION,
   "Screenshot"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   conf_item_edd = E_CONFIG_DD_NEW("Screenshot_Config_Item", Config_Item);

#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, delay_time, DOUBLE);
   E_CONFIG_VAL(D, T, use_import, UCHAR);
   E_CONFIG_VAL(D, T, use_scrot, UCHAR);
   E_CONFIG_VAL(D, T, prompt, UCHAR);
   E_CONFIG_VAL(D, T, location, STR);
   E_CONFIG_VAL(D, T, filename, STR);
   E_CONFIG_VAL(D, T, import.use_img_border, UCHAR);
   E_CONFIG_VAL(D, T, import.use_dither, UCHAR);
   E_CONFIG_VAL(D, T, import.use_frame, UCHAR);
   E_CONFIG_VAL(D, T, import.use_mono, UCHAR);
   E_CONFIG_VAL(D, T, import.use_window, UCHAR);
   E_CONFIG_VAL(D, T, import.use_silent, UCHAR);
   E_CONFIG_VAL(D, T, import.use_trim, UCHAR);
   E_CONFIG_VAL(D, T, scrot.use_img_border, UCHAR);
   E_CONFIG_VAL(D, T, scrot.use_thumb, UCHAR);
   E_CONFIG_VAL(D, T, use_app, UCHAR);
   E_CONFIG_VAL(D, T, app, STR);

   conf_edd = E_CONFIG_DD_NEW("Screenshot_Config", Config);

#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   ss_config = e_config_domain_load("module.screenshot", conf_edd);
   if (!ss_config)
     {
        Config_Item *ci;

        ss_config = E_NEW(Config, 1);
        ci = E_NEW(Config_Item, 1);

        ci->id = evas_stringshare_add("0");
        ci->delay_time = 60.0;
        if (ecore_file_app_installed("import"))
          {
             if (ecore_file_app_installed("scrot"))
               {
                  ci->use_import = 0;
                  ci->use_scrot = 1;
               }
             else
               {
                  ci->use_import = 1;
                  ci->use_scrot = 0;
               }
          }
        else if (ecore_file_app_installed("scrot"))
          {
             ci->use_import = 0;
             ci->use_scrot = 1;
          }
	ci->prompt = 0;
        ci->location = evas_stringshare_add(e_user_homedir_get());
        ci->filename = NULL;
        ci->import.use_img_border = 1;
        ci->import.use_dither = 1;
        ci->import.use_frame = 1;
        ci->import.use_mono = 0;
        ci->import.use_window = 0;
        ci->import.use_silent = 1;
        ci->import.use_trim = 1;
        ci->scrot.use_img_border = 1;
        ci->scrot.use_thumb = 0;
        ci->use_app = 0;
        ci->app = evas_stringshare_add("");

        ss_config->items = evas_list_append(ss_config->items, ci);
     }

   ss_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return 1;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   ss_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   if (ss_config->config_dialog)
      e_object_del(E_OBJECT(ss_config->config_dialog));
   if (ss_config->menu)
     {
        e_menu_post_deactivate_callback_set(ss_config->menu, NULL, NULL);
        e_object_del(E_OBJECT(ss_config->menu));
        ss_config->menu = NULL;
     }

   if (ss_config->exe_exit_handler)
      ecore_event_handler_del(ss_config->exe_exit_handler);

   while (ss_config->items)
     {
        Config_Item *ci;

        ci = ss_config->items->data;
        if (ci->id) evas_stringshare_del(ci->id);
        if (ci->location) evas_stringshare_del(ci->location);
        if (ci->filename) evas_stringshare_del(ci->filename);
        if (ci->app) evas_stringshare_del(ci->app);
        ss_config->items = evas_list_remove_list(ss_config->items, ss_config->items);
        free(ci);
	ci = NULL;
     }
   free(ss_config);
   ss_config = NULL;
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   Evas_List *l;

   for (l = ss_config->instances; l; l = l->next)
     {
        Instance *inst;
        Config_Item *ci;

        inst = l->data;
        ci = _ss_config_item_get(inst->gcc->id);
        if (ci->id) evas_stringshare_del(ci->id);
        ci->id = evas_stringshare_add(inst->gcc->id);
     }
   e_config_domain_save("module.screenshot", conf_edd, ss_config);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m)
{
   e_module_dialog_show(m, D_("Enlightenment Screenshot Module"), 
			D_("This module is used to take screenshots"));
   return 1;
}

static Screenshot *
_ss_new(Evas *evas)
{
   Screenshot *ss;
   char buf[4096];

   ss = E_NEW(Screenshot, 1);
   ss->ss_obj = edje_object_add(evas);

   snprintf(buf, sizeof(buf), "%s/screenshot.edj", e_module_dir_get(ss_config->module));
   if (!e_theme_edje_object_set(ss->ss_obj, "base/theme/modules/screenshot", "modules/screenshot/main"))
      edje_object_file_set(ss->ss_obj, buf, "modules/screenshot/main");
   evas_object_show(ss->ss_obj);
   return ss;
}

static void
_ss_free(Screenshot *ss)
{
   evas_object_del(ss->ss_obj);
   free(ss);
   ss = NULL;
}

static void
_ss_handle_mouse_down(Instance *inst)
{
   Config_Item *ci;
   char buf[4096];
   
   if (inst->exe) return;
   ci = _ss_config_item_get(inst->gcc->id);

   if (!ci->prompt) 
     {
	char *f = _get_filename(ci);	
	inst->filename = evas_stringshare_add(f);
	_ss_take_shot(inst);	
     }
   else
     _ss_get_filename(inst);
}

char *
_get_import_options(Config_Item *ci)
{
   char *opt;
   char buf[1024];
   char *opts[8] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

   if (ci->import.use_img_border) opts[0] = strdup("-border");
   if (ci->import.use_dither) opts[1] = strdup("-dither");
   if (ci->import.use_frame) opts[2] = strdup("-frame");
   if (ci->import.use_mono) opts[3] = strdup("-mono");
   if (ci->import.use_silent) opts[4] = strdup("-silent");
   if (ci->import.use_trim) opts[5] = strdup("-trim");
   if (!ci->import.use_window) opts[6] = strdup("-window root");
   if (ci->delay_time > 0)
     {
        snprintf(buf, sizeof(buf), "-pause %.0f", ci->delay_time);
        opts[7] = strdup(buf);
     }
   opt = _parse_options(opts);
   return strdup(opt);
}

char *
_get_scrot_options(Config_Item *ci)
{
   char *opt;
   char buf[1024];
   char *opts[8] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

   if (ci->scrot.use_img_border) opts[0] = strdup("--border");
   if (ci->scrot.use_thumb) opts[1] = strdup("--thumb 25");
   if (ci->delay_time > 0)
     {
        snprintf(buf, sizeof(buf), "--delay %.0f", ci->delay_time);
        opts[2] = strdup(buf);
     }
   opt = _parse_options(opts);
   return strdup(opt);
}

char *
_parse_options(char **opts)
{
   int i, j;
   char buf[1024];

   j = 0;
   for (i = 0; i <= 7; i++)
     {
        if (opts[i] != '\0')
          {
             if (j == 0)
               {
                  snprintf(buf, sizeof(buf), "%s", opts[i]);
                  j++;
               }
             else
                snprintf(buf, sizeof(buf), "%s %s", strdup(buf), opts[i]);
          }
     }
   return strdup(buf);
}

char *
_get_filename(Config_Item *ci)
{
   char buff[256];
   time_t t;
   struct tm *loctime;
   Ecore_List *fl = NULL;
   int c = 0;
   char *file, *ext;

   if (!ci->location)
     {
        ci->location = evas_stringshare_add(e_user_homedir_get());
        e_config_save_queue();
     }

   if ((!ci->filename) || (ci->filename == NULL))
     {
        t = time(NULL);
        loctime = localtime(&t);
        strftime(buff, sizeof(buff), "%Y-%m-%d-%H%M%S", loctime);
        snprintf(buff, sizeof(buff), "%s.png", strdup(buff));
     }
   else
     {
        if (ecore_file_is_dir(ci->location))
          {
	     ext = ecore_file_strip_ext(ci->filename);
             fl = ecore_file_ls(ci->location);
             ecore_list_goto_first(fl);
             while ((file = ecore_list_next(fl)) != NULL)
		  if (strstr(file, ext)) c++;

             if (fl) ecore_list_destroy(fl);
             if (c == 0) 
	       c = 1;
             else 
	       c++;

	     if (!strrchr(ci->filename, '.')) 
	       snprintf(buff, sizeof(buff), "%s%i.png", ci->filename, c);
	     else
	       snprintf(buff, sizeof(buff), "%s%i.png", ext, c);;
          }
     }
   return strdup(buff);
}

static int
_ss_exe_cb_exit(void *data, int type, void *event)
{
   Instance *inst;
   Ecore_Exe_Event_Del *ev;
   Ecore_Exe *x;
   Config_Item *ci;
   char buf[4096];

   ev = event;
   if (!ev->exe) return 1;
   x = ev->exe;
   if (!x) return 1;

   inst = ecore_exe_data_get(x);
   x = NULL;
   inst->exe = NULL;
   if (inst->filename) evas_stringshare_del(inst->filename);
   if (ss_config->exe_exit_handler)
      ecore_event_handler_del(ss_config->exe_exit_handler);

   ci = _ss_config_item_get(inst->gcc->id);
   if ((ci->use_app) && (ci->app != NULL))
     {
        snprintf(buf, sizeof(buf), "%s %s", ci->app, inst->filename);
        x = ecore_exe_run(buf, NULL);
     }
   return 0;
}

static void 
_ss_take_shot(void *data) 
{
   Instance *inst;
   Config_Item *ci;
   Edje_Message_Int_Set *msg;
   char buf[1024];
   char *cmd, *opt, *p;
   
   inst = data;
   if (!inst) return;

   ci = _ss_config_item_get(inst->gcc->id);
   if (!ci) return;
   
   if (ci->use_import == 1)
     {
        cmd = strdup("import");
        opt = _get_import_options(ci);
     }
   else if (ci->use_scrot == 1)
     {
        cmd = strdup("scrot");
        opt = _get_scrot_options(ci);
     }
   else
     {
        e_module_dialog_show(ss_config->module, D_("Enlightenment Screenshot Module"),
                             D_("Please install either ImageMagick or Scrot for taking screenshots."));
        return;
     }

   p = strrchr(inst->filename, '.');
   if (!p) 
     snprintf(buf, sizeof(buf), "%s.png", inst->filename);
   
   snprintf(buf, sizeof(buf), "%s %s %s/%s", cmd, opt, ci->location, inst->filename);
   ss_config->exe_exit_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _ss_exe_cb_exit, NULL);
   if (ci->delay_time > 0)
     {
        msg = malloc(sizeof(Edje_Message_Int_Set) + 1 * sizeof(int));
        msg->count = 1;
        msg->val[0] = ci->delay_time - 1;
        edje_object_message_send(inst->ss->ss_obj, EDJE_MESSAGE_INT_SET, 1, msg);
        free(msg);
	msg = NULL;
     }
   inst->exe = ecore_exe_run(buf, inst);   
}

static void 
_ss_get_filename(void *data) 
{
   e_entry_dialog_show(_("Enlightenment Screenshot Module"), "enlightenment/e",
		       _("Enter a new filename to use for this screenshot"),
		       NULL, NULL, _cb_entry_ok, NULL, data);
}

static void
_cb_entry_ok(char *text, void *data) 
{
   Instance *inst;
   Config_Item *ci;   
   char buf[4096];
   char *t;
   
   inst = data;
   if (!inst) return;

   ci = _ss_config_item_get(inst->gcc->id);

   t = ecore_file_get_dir(text);
   if (!strcmp(t, text))
     {
        e_module_dialog_show(ss_config->module, D_("Enlightenment Screenshot Module"),
                             D_("You did not specify a path.<br>"
				"This shot will be saved in your home folder."));
	if (ci->location)
	  evas_stringshare_del(ci->location);
	ci->location = evas_stringshare_add(e_user_homedir_get());
	e_config_save_queue();
     }
   else
     {

	if (ci->location)
	  evas_stringshare_del(ci->location);
	ci->location = evas_stringshare_add(t);
	e_config_save_queue();	
     }

   inst->filename = evas_stringshare_add(text);
   _ss_take_shot(inst);
}

