#include <e.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Net Net;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *net_obj;
   Net *net;
   Ecore_Timer *check_timer;
};

struct _Net 
{
   Instance *inst;
   Evas_Object *net_obj;
};

/* Func Protos for Gadcon */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, char *name, char *id, char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);

/* Func Protos for Module */
static void _net_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _net_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _net_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _net_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _net_menu_cb_post(void *data, E_Menu *m);
static Config_Item *_net_config_item_get(const char *id);
static Net *_net_new(Evas *evas);
static void _net_free(Net *net);
static void _net_update_rx(Instance *inst, int value);
static void _net_update_tx(Instance *inst, int value);
static int _net_cb_check(void *data);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *net_config = NULL;

/* Define the gadcon class and functions provided by this module */
static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION,
     "net", { _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon }
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, char *name, char *id, char *style) 
{
   E_Gadcon_Client *gcc;
   Evas_Object *o;
   Instance *inst;
   Config_Item *ci;
   Net *net;
   char buf[4096];
   
   inst = E_NEW(Instance, 1);
   ci = _net_config_item_get(id);
   if (!ci->id) 
     ci->id = evas_stringshare_add(id);
   
   net = _net_new(gc->evas);
   net->inst = inst;
   inst->net = net;

   o = net->net_obj;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->net_obj = o;
   
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
				  _net_cb_mouse_down, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN,
				  _net_cb_mouse_in, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT,
				  _net_cb_mouse_out, inst);

   inst->check_timer = ecore_timer_add(ci->poll_time, _net_cb_check, inst);
   
   net_config->instances = evas_list_append(net_config->instances, inst);
   
   return gcc;
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
   return D_("Net");
}

static Evas_Object *
_gc_icon(Evas *evas) 
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/module.eap", e_module_dir_get(net_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   
   inst = gcc->data;
   if (inst->check_timer)
     ecore_timer_del(inst->check_timer);
   net_config->instances = evas_list_remove(net_config->instances, inst);
   _net_free(inst->net);
   free(inst);
}

static void
_net_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!net_config->menu))
     {
        E_Menu *mn;
        E_Menu_Item *mi;
        int x, y, w, h;

        mn = e_menu_new();
        e_menu_post_deactivate_callback_set(mn, _net_menu_cb_post, inst);
        net_config->menu = mn;

        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, D_("Configuration"));
        e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
        e_menu_item_callback_set(mi, _net_menu_cb_configure, inst);

        e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
        e_menu_activate_mouse(mn,
                              e_util_zone_current_get(e_manager_current_get()),
                              x + ev->output.x, y + ev->output.y, 1, 1, E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }   
}

static void
_net_menu_cb_post(void *data, E_Menu *m) 
{
   if (!net_config->menu)
     return;
   e_object_del(E_OBJECT(net_config->menu));
   net_config->menu = NULL;
}

static void
_net_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _net_config_item_get(inst->gcc->id);
   _config_net_module(ci);
}

void
_net_config_updated(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   
   if (!net_config)
     return;
   
   ci = _net_config_item_get(id);
   for (l = net_config->instances; l; l = l->next) 
     {
	Instance *inst;
	
	inst = l->data;
	if (!inst->gcc->id)
	  continue;
	
	if (!strcmp(inst->gcc->id, ci->id)) 
	  {
	     if (inst->check_timer)
	       ecore_timer_interval_set(inst->check_timer, (double)ci->poll_time);
	     else
	       inst->check_timer = ecore_timer_add((double)ci->poll_time, _net_cb_check, inst);
	     break;
	  }
     }
}

static Config_Item *
_net_config_item_get(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   
   for (l = net_config->items; l; l = l->next) 
     {
	ci = l->data;
	if (!ci->id)
	  continue;
	if (!strcmp(ci->id, id))
	  return ci;
     }
   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->device = evas_stringshare_add("eth0");
   ci->poll_time = 1.0;
   ci->max = 1500;
   net_config->items = evas_list_append(net_config->items, ci);
   return ci;
}

/* Module routines */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION,
     "Net"
};

EAPI void *
e_modapi_init(E_Module *m) 
{
   bindtextdomain(PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(PACKAGE, "UTF-8");
   
   conf_item_edd = E_CONFIG_DD_NEW("Net_Config_Item", Config_Item);
   
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, device, STR);
   E_CONFIG_VAL(D, T, max, DOUBLE);
   E_CONFIG_VAL(D, T, poll_time, DOUBLE);
   
   conf_edd = E_CONFIG_DD_NEW("Net_Config", Config);
   
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);
   
   net_config = e_config_domain_load("module.net", conf_edd);
   if (!net_config) 
     {
	Config_Item *ci;
	
	net_config = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);
	ci->id = evas_stringshare_add("0");
	ci->device = evas_stringshare_add("eth0");
	ci->poll_time = 1.0;
	ci->max = 1500;
	net_config->items = evas_list_append(net_config->items, ci);
     }
   net_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return 1;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
   net_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);
   
   if (net_config->config_dialog)
     e_object_del(E_OBJECT(net_config->config_dialog));
   if (net_config->menu) 
     {
	e_menu_post_deactivate_callback_set(net_config->menu, NULL, NULL);
	e_object_del(E_OBJECT(net_config->menu));
	net_config->menu = NULL;
     }
   while (net_config->items) 
     {
	Config_Item *ci;
	
	ci = net_config->items->data;
	if (ci->id)
	  evas_stringshare_del(ci->id);
	net_config->items = evas_list_remove_list(net_config->items, net_config->items);
	free(ci);
     }
   free(net_config);
   net_config = NULL;
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_info(E_Module *m) 
{
   char buf[4096];
   
   snprintf(buf, sizeof(buf), "%s/module_icon.png", e_module_dir_get(m));
   m->icon_file = strdup(buf);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   Evas_List *l;
   
   for (l = net_config->instances; l; l = l->next) 
     {
	Instance *inst;
	Config_Item *ci;
	
	inst = l->data;
	ci = _net_config_item_get(inst->gcc->id);
	if (ci->id)
	  evas_stringshare_del(ci->id);
	ci->id = evas_stringshare_add(inst->gcc->id);
     }
   e_config_domain_save("module.net", conf_edd, net_config);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m) 
{
   e_module_dialog_show(D_("Enlightenment Network Monitor Module"), 
			D_("This module is used to monitor a network device."));
   return 1;
}

static Net *
_net_new(Evas *evas) 
{
   Net *net;
   char buf[4096];
   
   net = E_NEW(Net, 1);
   net->net_obj = edje_object_add(evas);
   
   snprintf(buf, sizeof(buf), "%s/net.edj", e_module_dir_get(net_config->module));
   if (!e_theme_edje_object_set(net->net_obj, "base/theme/modules/net", "modules/net/main"))
     edje_object_file_set(net->net_obj, buf, "modules/net/main");
   evas_object_show(net->net_obj);

   return net;
}

static void
_net_free(Net *n) 
{
   evas_object_del(n->net_obj);
   free(n);
}

static void 
_net_update_rx(Instance *inst, int value) 
{
   Edje_Message_Int_Set *val;
   
   val = malloc(sizeof(Edje_Message_Int_Set) + (1 * sizeof(int)));
   val->count = 1;
   val->val[0] = value;
   edje_object_message_send(inst->net_obj, EDJE_MESSAGE_INT_SET, 1, val);
   free(val);   
}

static void 
_net_update_tx(Instance *inst, int value) 
{
   Edje_Message_Int_Set *val;

   val = malloc(sizeof(Edje_Message_Int_Set) + (1 * sizeof(int)));
   val->count = 1;
   val->val[0] = value;
   edje_object_message_send(inst->net_obj, EDJE_MESSAGE_INT_SET, 2, val);
   free(val);
}

static void 
_net_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst;
   
   inst = data;   
   edje_object_signal_emit(inst->net_obj, "label_active", "");
}

static void 
_net_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst;
   
   inst = data;
   edje_object_signal_emit(inst->net_obj, "label_passive", "");
}

static int 
_net_cb_check(void *data) 
{
   Instance *inst;
   Config_Item *ci;
   FILE *stat;
   char dev[64];
   char buf[256];
   static unsigned long old_in = 0;
   static unsigned long old_out = 0;
   unsigned long in = 0;
   unsigned long out = 0;
   unsigned long dummy = 0;
   int found;
   long max_in = 171008;
   long max_out = 28672;
   long bytes_in;
   long bytes_out;
   double in_use = 0.0;
   double out_use = 0.0;
   char in_str[100];
   char out_str[100];
   
   inst = data;
   ci = _net_config_item_get(inst->gcc->id);
   
   stat = fopen("/proc/net/dev", "r");
   if (!stat)
      return 1;

   found = 0;
   while (fgets(buf, 256, stat))
     {
        int i = 0;

        for (; buf[i] != 0; i++)
          {
             if (buf[i] == ':')
                buf[i] = ' ';
          }
        if (sscanf(buf, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
                   "%lu %lu %lu %lu\n", dev, &in, &dummy, &dummy,
                   &dummy, &dummy, &dummy, &dummy, &dummy, &out, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) < 17)
           continue;
	if (!ci->device)
	  continue;
        if (!strcmp(dev, ci->device))
          {
             found = 1;
             break;
          }
     }
   fclose(stat);

   if (!found)
      return 1;

   if (old_in && old_out)
     {
        bytes_in = in - old_in;
        bytes_out = out - old_out;

        if (bytes_in < 0)
           bytes_in = 0;
        if (bytes_out < 0)
           bytes_out = 0;

        in_use = ((bytes_in * 100L) / max_in);
        out_use = ((bytes_out * 100L) / max_out);
     }
   else
     {
        in_use = 0.0;
        out_use = 0.0;
     }

   old_in = in;
   old_out = out;
      
   if (bytes_in <= 0)
     edje_object_part_text_set(inst->net_obj, "rx_label", "Rx: 0 B");
   else 
     {
        if (bytes_in > 1048576)
          {
             bytes_in = bytes_in / 1048576;
             snprintf(in_str, sizeof(in_str), "Rx: %d Mb", bytes_in);
          }
        else if (bytes_in > 1024 && bytes_in < 1048576)
          {
             bytes_in = bytes_in / 1024;
             snprintf(in_str, sizeof(in_str), "Rx: %d Kb", bytes_in);
          }
        else
	  snprintf(in_str, sizeof(in_str), "Rx: %d B", bytes_in);
	
        edje_object_part_text_set(inst->net_obj, "rx_label", in_str);	
     }
   
   if (bytes_out <= 0) 
     edje_object_part_text_set(inst->net_obj, "tx_label", "Tx: 0 B");
   else 
     {
        if (bytes_out > 1048576)
          {
             bytes_out = bytes_out / 1048576;
             snprintf(out_str, sizeof(out_str), "Tx: %d Mb", bytes_out);
          }
        else if (bytes_out > 1024 && bytes_out < 1048576)
          {
             bytes_out = bytes_out / 1024;
             snprintf(out_str, sizeof(out_str), "Tx: %d Kb", bytes_out);
          }
        else
	  snprintf(out_str, sizeof(out_str), "Tx: %d B", bytes_out);
	
        edje_object_part_text_set(inst->net_obj, "tx_label", out_str);	
     }

   int x, y, w, h;
   double i, o;
   evas_object_geometry_get(inst->net_obj, &x, &y, &w, &h);
   i = ((double)in_use * ((double)w / (double)100));
   o = ((double)out_use * ((double)w / (double)100));
   
   if (i < 0)
     i = 0.0;
   if (o < 0)
     o = 0.0;
   
   if ((i > 0) && (i < 1))
     i = 10.0;
   if ((o > 0) && (o < 1))
     o = 10.0;
   
   _net_update_rx(inst, (i / 10));
   _net_update_tx(inst, (o / 10));
   
   return 1;
}
