#include <e.h>
#include "e_mod_main.h"

typedef struct _Instance Instance;
typedef struct _Cpu Cpu;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Cpu             *cpu;
   Ecore_Timer     *timer;
};

struct _Cpu 
{
   Instance *inst;
   Evas_Object *o_icon;
};

static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);
static Config_Item *_config_item_get(const char *id);
static int _set_cpu_load(void *data);
static int _get_cpu_count(void);
static int _get_cpu_load(void);
static void _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_cb_post(void *data, E_Menu *m);
static void _cpu_menu_fast(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cpu_menu_medium(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cpu_menu_normal(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cpu_menu_slow(void *data, E_Menu *m, E_Menu_Item *mi);
static void _cpu_menu_very_slow(void *data, E_Menu *m, E_Menu_Item *mi);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *cpu_conf = NULL;

static int cpu_count;
static int cpu_stats[4];

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "cpu", 
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon}
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Cpu             *cpu;
   Instance        *inst;
   Config_Item     *ci;
   E_Gadcon_Client *gcc;
   char             buf[4096];

   cpu_count = _get_cpu_count();
   
   inst = E_NEW(Instance, 1);   

   ci = _config_item_get(id);
   if (!ci->id)
     ci->id = evas_stringshare_add(id);

   cpu = E_NEW(Cpu, 1);
   cpu->inst = inst;

   snprintf(buf, sizeof(buf), "%s/cpu.edj", 
	    e_module_dir_get(cpu_conf->module));
   
   cpu->o_icon = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(cpu->o_icon, 
				"base/theme/modules/cpu", "modules/cpu/main"))
     edje_object_file_set(cpu->o_icon, buf, "modules/cpu/main");
   evas_object_show(cpu->o_icon);
   
   gcc = e_gadcon_client_new(gc, name, id, style, cpu->o_icon);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->cpu = cpu;

   cpu_conf->instances = evas_list_append(cpu_conf->instances, inst);

   evas_object_event_callback_add(cpu->o_icon, EVAS_CALLBACK_MOUSE_DOWN,
				  _button_cb_mouse_down, inst);

   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   Cpu      *cpu;
   
   inst = gcc->data;
   cpu = inst->cpu;

   if (inst->timer)
     ecore_timer_del(inst->timer);
   if (cpu->o_icon)
     evas_object_del(cpu->o_icon);
   
   cpu_conf->instances = evas_list_remove(cpu_conf->instances, inst);
   E_FREE(cpu);
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
   return _("Cpu");
}

static Evas_Object *
_gc_icon(Evas *evas) 
{
   Evas_Object *o;
   char         buf[4096];

   if (!cpu_conf->module) return NULL;
   
   snprintf(buf, sizeof(buf), "%s/module.eap", 
	    e_module_dir_get(cpu_conf->module));
   
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static Config_Item *
_config_item_get(const char *id) 
{
   Evas_List   *l;
   Config_Item *ci;

   for (l = cpu_conf->items; l; l = l->next) 
     {
	ci = l->data;
	if (!ci->id) continue;
	if (!strcmp(ci->id, id)) return ci;
     }

   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->interval = 1;
   
   cpu_conf->items = evas_list_append(cpu_conf->items, ci);
   return ci;
}

static int 
_set_cpu_load(void *data) 
{
   Instance *inst;
   Cpu      *cpu;
   int       load, i = 0;
   char      str[100], str_tmp[100];

   if (cpu_count == -1) return 0;

   inst = data;
   if (!inst) return 1;
   cpu = inst->cpu;
   if (!cpu) return 1;
   
   _get_cpu_load();

   snprintf(str, sizeof(str), "%d%%", cpu_stats[0]);
   i = 1;
   while (i < cpu_count)
     {
	snprintf(str_tmp, sizeof(str_tmp), " / %d%%", cpu_stats[i]);
	strncat(str, str_tmp, sizeof(str));
	i++;
     }
   edje_object_part_text_set(cpu->o_icon, "load", str);
   return 1;
}

static int
_get_cpu_count(void)
{
   FILE *f;
   char tmp[4];
   int cpu = -1;

   if (!(f = fopen("/proc/stat", "r"))) return cpu;

   while (fscanf(f, "cp%s %*u %*u %*u %*u %*u %*u %*u %*u\n", (char *) &tmp) == 1)
     cpu++;

   fclose(f);
   return cpu;
}

static int
_get_cpu_load(void) 
{
   FILE *stat;
   static unsigned long old_u[4], old_n[4], old_s[4], old_i[4], old_wa[4], old_hi[4], old_si[4];
   unsigned long new_u, new_n, new_s, new_i, new_wa = 0, new_hi = 0, new_si = 0, dummy2, ticks_past;
   int tmp_u = 0, tmp_n = 0, tmp_s = 0;
   char dummy[16];
   int i = 0;
   
   if (!(stat = fopen("/proc/stat", "r"))) return -1;

   /* since if there are more than 1 CPUs, the first entry is the summary:
	cpu  366384 274786 214744 7129029 1975609 12775 353729 0
	cpu0 167188 137966 127694 3664600 704402 12775 353588 0
	cpu1 199195 136820 87050 3464429 1271207 0 140 0
	
	In this case the first line is read and forgotten
   */
   if(cpu_count>1){
      /* I had to add another %lu (linux 2.6.17) */
      fscanf(stat, "%s %lu %lu %lu %lu %lu %lu %lu %lu", dummy, &new_u, &new_n, &new_s, &new_i, &new_wa, &new_hi, &new_si, &dummy2);
   }
   
   while (i < cpu_count)
     {

        /* I had to add another %lu (linux 2.6.17) */
	if (fscanf(stat, "%s %lu %lu %lu %lu %lu %lu %lu %lu", dummy, &new_u, &new_n,
	     &new_s, &new_i, &new_wa, &new_hi, &new_si, &dummy2) < 5)
	  {
	     fclose (stat);
	     return -1;
	  }

	ticks_past = ((new_u + new_n + new_s + new_i + new_wa + new_hi + new_si) -
		      (old_u[i] + old_n[i] + old_s[i] + old_i[i] + old_wa[i] + old_hi[i] + old_si[i]));

	if (ticks_past)
	  {
	     tmp_u = ((new_u - old_u[i]));
	     tmp_n = ((new_n - old_n[i]));
	     tmp_s = ((new_s - old_s[i]));
	  }
	
	cpu_stats[i] = (tmp_u + tmp_n + tmp_s);

	old_u[i] = new_u;
	old_n[i] = new_n;
	old_s[i] = new_s;
	old_wa[i] = new_wa;
	old_hi[i] = new_hi;
	old_si[i] = new_si;
	
	cpu_stats[i]=(cpu_stats[i]>100?100:cpu_stats[i]);

	i++;
     }
   fclose (stat);
   return 0;
}

static void
_button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   
   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!cpu_conf->menu))
     {
	E_Menu *mn;
	E_Menu_Item *mi;
	int cx, cy, cw, ch;
	Evas_List *l;
	char buf[256];
	Config_Item *ci;
	
	ci = _config_item_get(inst->gcc->id);

	mn = e_menu_new();
	cpu_conf->menu_interval = mn;
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Fast (0.5 sec)"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 1);
	if (ci->interval <= 0.5) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cpu_menu_fast, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Medium (1 sec)"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 1);
	if (ci->interval > 0.5) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cpu_menu_medium, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Normal (2 sec)"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 1);
	if (ci->interval >= 2.0) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cpu_menu_normal, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Slow (5 sec)"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 1);
	if (ci->interval >= 5.0) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cpu_menu_slow, inst);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Very Slow (30 sec)"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 1);
	if (ci->interval >= 30.0) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _cpu_menu_very_slow, inst);

	mn = e_menu_new();
	cpu_conf->menu = mn;
	e_menu_post_deactivate_callback_set(mn, _menu_cb_post, inst);
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Time Between Updates"));
	e_menu_item_submenu_set(mi, cpu_conf->menu_interval);

        e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
	
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
				 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_menu_cb_post(void *data, E_Menu *m)
{
   if (!cpu_conf->menu) return;
   e_object_del(E_OBJECT(cpu_conf->menu));
   cpu_conf->menu = NULL;
   if (cpu_conf->menu_interval)
     e_object_del(E_OBJECT(cpu_conf->menu_interval));
   cpu_conf->menu_interval = NULL;
}
   
static void
_cpu_menu_fast(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _config_item_get(inst->gcc->id);

   ci->interval = 0.5;
   ecore_timer_del(inst->timer);
   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   e_config_save_queue();
}

static void
_cpu_menu_medium(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _config_item_get(inst->gcc->id);

   ci->interval = 1.0;
   ecore_timer_del(inst->timer);
   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   e_config_save_queue();
}

static void
_cpu_menu_normal(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _config_item_get(inst->gcc->id);

   ci->interval = 2.0;
   ecore_timer_del(inst->timer);
   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   e_config_save_queue();
}

static void
_cpu_menu_slow(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _config_item_get(inst->gcc->id);

   ci->interval = 5.0;
   ecore_timer_del(inst->timer);
   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   e_config_save_queue();
}

static void
_cpu_menu_very_slow(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Instance *inst;
   Config_Item *ci;
   
   inst = data;
   ci = _config_item_get(inst->gcc->id);

   ci->interval = 30.0;
   ecore_timer_del(inst->timer);
   inst->timer = ecore_timer_add(ci->interval, _set_cpu_load, inst);
   e_config_save_queue();
}

EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION, "Cpu"
};

EAPI void *
e_modapi_init(E_Module *m) 
{
   conf_item_edd = E_CONFIG_DD_NEW("Cpu_Config_Item", Config_Item);
   conf_edd = E_CONFIG_DD_NEW("Cpu_Config", Config);
   
   #undef T
   #define T Config_Item
   #undef D
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, interval, DOUBLE);
   
   #undef T
   #define T Config
   #undef D
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);
   
   cpu_conf = e_config_domain_load("module.cpu", conf_edd);
   if (!cpu_conf) 
     {
	Config_Item *ci;
	
	cpu_conf = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);
	ci->id = evas_stringshare_add("0");
	ci->interval = 1;
	
	cpu_conf->items = evas_list_append(cpu_conf->items, ci);
     }
   
   cpu_conf->module = m;
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
   cpu_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);
   if (cpu_conf->config_dialog)
     e_object_del(E_OBJECT(cpu_conf->config_dialog));
   if (cpu_conf->menu) 
     {
	e_menu_post_deactivate_callback_set(cpu_conf->menu, NULL, NULL);
	e_object_del(E_OBJECT(cpu_conf->menu));
	cpu_conf->menu = NULL;
     }

   while(cpu_conf->items) 
     {
	Config_Item *ci;
	
	ci = cpu_conf->items->data;
	if (ci->id)
	  evas_stringshare_del(ci->id);
	cpu_conf->items = evas_list_remove_list(cpu_conf->items, cpu_conf->items);
	E_FREE(ci);
     }

   E_FREE(cpu_conf);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   Evas_List *l;
   
   for (l = cpu_conf->instances; l; l = l->next) 
     {
	Instance *inst;
	Config_Item *ci;
	
	inst = l->data;
	ci = _config_item_get(inst->gcc->id);
	if (ci->id)
	  evas_stringshare_del(ci->id);
	ci->id = evas_stringshare_add(inst->gcc->id);
     }
   e_config_domain_save("module.cpu", conf_edd, cpu_conf);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m)
{
  e_module_dialog_show (m, _("Enlightenment Cpu Monitor Module"),
			_("This module is used to monitor cpu load."));
  return 1;
}
