#include "e_mod_main.h"

/* module private routines */

static Monitor  *_monitor_new();
static void    _monitor_shutdown(Monitor *monitor);
static void    _monitor_config_menu_new(Monitor *monitor);

static Monitor_Face *_monitor_face_new(E_Container *con);
static void    _monitor_face_free(Monitor_Face *face);
static void    _monitor_face_menu_new(Monitor_Face *face);
static void    _monitor_face_cb_gmc_change(void *data, E_Gadman_Client *gmc, 
					   E_Gadman_Change change);
Config_Face *  _monitor_face_config_init(Config_Face *conf);
static int     _monitor_face_config_cb_timer(void *data);

static void _monitor_face_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _monitor_face_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void _monitor_cpu_text_update_callcack(Flow_Chart *chart, void *data);
static void _monitor_mem_real_text_update_callback(Flow_Chart *chart, void *data);
static void _monitor_mem_swap_text_update_callback(Flow_Chart *chart, void *data);
static void _monitor_net_in_text_update_callcack(Flow_Chart *chart, void *data);
static void _monitor_net_out_text_update_callcack(Flow_Chart *chart, void *data);
static void _monitor_wlan_link_text_update_callcack(Flow_Chart *chart, void *data);
static void _monitor_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);

static int _monitor_count;

static E_Config_DD *conf_edd;
static E_Config_DD *conf_face_edd;

static Flow_Chart *flow_chart_cpu;
static Flow_Chart *flow_chart_net_in;
static Flow_Chart *flow_chart_net_out;
static Flow_Chart *flow_chart_mem_real;
static Flow_Chart *flow_chart_mem_swap;
static Flow_Chart *flow_chart_wlan_link;

/* public module routines. all modules must have these */
E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Monitor"
};

void *
e_modapi_init(E_Module *module)
{
   Monitor *monitor;
   
   /* actually init buttons */
   monitor = _monitor_new();
   module->config_menu = monitor->config_menu;
   return monitor;
}

int
e_modapi_shutdown(E_Module *module)
{
   Monitor *monitor;

   if (module->config_menu)
     module->config_menu = NULL;

   monitor = module->data;
   if (monitor)
     _monitor_shutdown(monitor);

   return 1;
}

int
e_modapi_save(E_Module *module)
{
   Monitor *monitor;

   monitor = module->data;
   e_config_domain_save("module.monitor", conf_edd, monitor->conf);
   return 1;
}

int
e_modapi_info(E_Module *module)
{
   module->icon_file = strdup(PACKAGE_DATA_DIR "/module_icon.png");
   return 1;
}

int
e_modapi_about(E_Module *module)
{
   e_module_dialog_show(_("Enlightenment Monitor Module"),
		       _("A simple module to give E17 a usage monitor for some resources."));
   return 1;
}

int
e_modapi_config(E_Module *module) 
{
   Monitor *mon;
   Evas_List *l;
   
   mon = module->data;
   for (l = mon->faces; l; l = l->next) 
     {
	Monitor_Face *f;
	
	f = l->data;
	if (f->mon == mon) 
	  {
	     _config_monitor_module(f->con, f);
	     break;
	  }
     }
   return 1;
}

static int     
_monitor_face_config_cb_timer(void *data)
{
   e_error_dialog_show(_("Configuration Upgraded"), data);
   return 0;
}

Config_Face *
_monitor_face_config_init(Config_Face *conf)
{
   if (!conf) return NULL;
   conf->config_version = MONITOR_CONFIG_VERSION;
   conf->enabled = 1;
   conf->cpu_interval = 1.0;
   conf->mem_interval = 1.0;
   conf->net_interval = 1.0;
   conf->net_interface = (char *)evas_stringshare_add("eth0");
   conf->wlan_interval = 1.0;
   conf->wlan_interface = (char *)evas_stringshare_add("wlan0");
   conf->mem_real_ignore_cached = 0;
   conf->mem_real_ignore_buffers = 0;

   return conf;
}

/* module private routines */
static Monitor *
_monitor_new()
{
   Monitor *monitor;
   Evas_List *managers, *l, *l2, *cl;
   E_Menu_Item *mi;
  
   _monitor_count = 0;
   monitor = E_NEW(Monitor, 1);
   if (!monitor) return NULL;

   conf_face_edd = E_CONFIG_DD_NEW("Monitor_Config_Face", Config_Face);
#undef T
#undef D
#define T Config_Face
#define D conf_face_edd
   E_CONFIG_VAL(D, T, config_version, INT);
   E_CONFIG_VAL(D, T, enabled, INT);
   E_CONFIG_VAL(D, T, cpu_interval, DOUBLE);
   E_CONFIG_VAL(D, T, mem_interval, DOUBLE);
   E_CONFIG_VAL(D, T, net_interval, DOUBLE);
   E_CONFIG_VAL(D, T, net_interface, STR);
   E_CONFIG_VAL(D, T, wlan_interval, DOUBLE);
   E_CONFIG_VAL(D, T, wlan_interface, STR);
   E_CONFIG_VAL(D, T, mem_real_ignore_cached, INT);
   E_CONFIG_VAL(D, T, mem_real_ignore_buffers, INT);

   conf_edd = E_CONFIG_DD_NEW("Monitor_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, faces, conf_face_edd);

   monitor->conf = e_config_domain_load("module.monitor", conf_edd);
   if (!monitor->conf)
     {
	monitor->conf = E_NEW(Config, 1);
     }

   _monitor_config_menu_new(monitor);

   managers = e_manager_list();
   cl = monitor->conf->faces;
   for (l = managers; l; l = l->next)
     {
	E_Manager *man;
	
	man = l->data;
	for (l2 = man->containers; l2; l2 = l2->next)
	  {
	     E_Container *con;
	     Monitor_Face *face;
	     
	     con = l2->data;
	     face = _monitor_face_new(con);
	     if (face)
	       {
	       face->mon = monitor;
		  monitor->faces = evas_list_append(monitor->faces, face);
		  /* Config */
		  if (!cl)
		    {
		       face->conf = E_NEW(Config_Face, 1);
		       face->conf = _monitor_face_config_init(face->conf);
		       monitor->conf->faces = evas_list_append(monitor->conf->faces, face->conf);
		    }
		  else
		    {
		       face->conf = cl->data;
		       if (face->conf->config_version != MONITOR_CONFIG_VERSION)
		          {
			     face->conf = E_NEW(Config_Face, 1);
		             face->conf = _monitor_face_config_init(face->conf);
			     ecore_timer_add(1.0, _monitor_face_config_cb_timer,
					     _("Configuration data needed upgrading. Your old configuration\n"
					       "has been removed. Please reset your setting for the monitor\n"
					       "module. Sorry for the inconvenience.\n"));
			  }
		       cl = cl->next;
		    }

		  /* Menu */
		  /* This menu must be initialized after conf */
		  _monitor_face_menu_new(face);

		  mi = e_menu_item_new(monitor->config_menu);
		  e_menu_item_label_set(mi, _("Configuration"));
		  e_menu_item_callback_set(mi, _monitor_menu_cb_configure, face);

		  mi = e_menu_item_new(monitor->config_menu);
		  e_menu_item_label_set(mi, con->name);
		  e_menu_item_submenu_set(mi, face->menu);

		  /* 
		   * Now that init is done, set up intervals,
		   * interfaces, and memory monitor parameters
		   * according to the configuration.
		   */
		  mem_real_ignore_buffers_set(face->conf->mem_real_ignore_buffers);
		  mem_real_ignore_cached_set(face->conf->mem_real_ignore_cached);
		  net_interface_set(face->conf->net_interface);
		  wlan_interface_set(face->conf->wlan_interface);

		  flow_chart_update_rate_set(flow_chart_cpu, face->conf->cpu_interval);
		  flow_chart_update_rate_set(flow_chart_mem_real, face->conf->mem_interval);
		  flow_chart_update_rate_set(flow_chart_mem_swap, face->conf->mem_interval);
		  flow_chart_update_rate_set(flow_chart_net_in, face->conf->net_interval);
		  flow_chart_update_rate_set(flow_chart_net_out, face->conf->net_interval);
		  flow_chart_update_rate_set(flow_chart_wlan_link, face->conf->wlan_interval);
	       }
	  }
     }
   return monitor;
}

static void
_monitor_shutdown(Monitor *monitor)
{
   Evas_List *list;

   E_CONFIG_DD_FREE(conf_edd);
   E_CONFIG_DD_FREE(conf_face_edd);

   for (list = monitor->faces; list; list = list->next)
     _monitor_face_free(list->data);
   evas_list_free(monitor->faces);

   e_object_del(E_OBJECT(monitor->config_menu));

   evas_list_free(monitor->conf->faces);
   free(monitor->conf);
   free(monitor);
}

static void
_monitor_config_menu_new(Monitor *monitor)
{
   monitor->config_menu = e_menu_new();
}

static Monitor_Face *
_monitor_face_new(E_Container *con)
{
   Monitor_Face *face;
   Evas_Object *o;

   /* 
    * Configuration cannot be used yet as the face config 
    * is not initialized. Everything will be updated after 
    * the init is complete.
    */
   double tmp_cpu_interval = 1.0;
   double tmp_mem_interval = 1.0;
   double tmp_net_interval = 1.0;
   double tmp_wlan_interval = 1.0;

   Chart_Container *chart_con;

   face = E_NEW(Monitor_Face, 1);
   if (!face) return NULL;
   
   face->con = con;
   e_object_ref(E_OBJECT(con));
   
   evas_event_freeze(con->bg_evas);
   
   /* setup monitor object */
   o = edje_object_add(con->bg_evas);
   face->monitor_object = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR"/monitor.edj", "monitor/main");
   evas_object_show(o);
   /* setup res table */
   o = e_table_add(con->bg_evas);
   face->table_object = o;
   e_table_homogenous_set(o, 1);   
   edje_object_part_swallow(face->monitor_object, "items", face->table_object);
   evas_object_show(o);

   /* setup cpu */
   o = edje_object_add(con->bg_evas);
   face->cpu = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR"/monitor.edj", "monitor/cpu");
   e_table_pack(face->table_object, o, 0, 0, 1, 1);
   e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);   
   evas_object_layer_set(o, evas_object_layer_get(face->monitor_object)+1);
   evas_object_show(o);
   /* add cpu chart */
   chart_con = chart_container_new(con->bg_evas,0,0,0,0);
   flow_chart_cpu = flow_chart_new();
   flow_chart_color_set(flow_chart_cpu, 33, 100, 220, 255);
   flow_chart_get_value_function_set(flow_chart_cpu, cpu_usage_get);
   flow_chart_update_rate_set(flow_chart_cpu, tmp_cpu_interval);
   chart_container_chart_add(chart_con, flow_chart_cpu);
   face->chart_cpu = chart_con;   
   flow_chart_callback_set(flow_chart_cpu, _monitor_cpu_text_update_callcack, face);
   
   o = evas_object_rectangle_add(con->bg_evas);
   face->cpu_ev_obj = o;
   evas_object_color_set(o, 255,255,255,0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _monitor_face_cb_mouse_down, face);
   evas_object_show(o);

   /* setup mem */
   o = edje_object_add(con->bg_evas);
   face->mem = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR"/monitor.edj", "monitor/mem");
   e_table_pack(face->table_object, o, 1, 0, 1, 1);
   e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);   
   evas_object_layer_set(o, evas_object_layer_get(face->monitor_object)+1);
   evas_object_show(o);
   /* add mem charts */
   chart_con = chart_container_new(con->bg_evas,0,0,0,0);
   flow_chart_mem_real = flow_chart_new();
   flow_chart_color_set(flow_chart_mem_real, 213, 91, 91, 255);
   flow_chart_get_value_function_set(flow_chart_mem_real, mem_real_usage_get);
   flow_chart_update_rate_set(flow_chart_mem_real, tmp_mem_interval);
   chart_container_chart_add(chart_con, flow_chart_mem_real);
   face->chart_mem = chart_con;   
   flow_chart_callback_set(flow_chart_mem_real, _monitor_mem_real_text_update_callback, face);

   flow_chart_mem_swap = flow_chart_new();
   flow_chart_color_set(flow_chart_mem_swap, 51, 181, 69, 255);
   flow_chart_get_value_function_set(flow_chart_mem_swap, mem_swap_usage_get);
   flow_chart_update_rate_set(flow_chart_mem_swap, tmp_mem_interval);
   flow_chart_alignment_set(flow_chart_mem_swap, 0);
   chart_container_chart_add(chart_con, flow_chart_mem_swap);
   flow_chart_callback_set(flow_chart_mem_swap, _monitor_mem_swap_text_update_callback, face);

   o = evas_object_rectangle_add(con->bg_evas);
   face->mem_ev_obj = o;
   evas_object_color_set(o, 255,255,255,0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _monitor_face_cb_mouse_down, face);
   evas_object_show(o);

   /* setup net */
   o = edje_object_add(con->bg_evas);
   face->net = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR"/monitor.edj", "monitor/net");
   e_table_pack(face->table_object, o, 2, 0, 1, 1);
   e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);   
   evas_object_layer_set(o, evas_object_layer_get(face->monitor_object)+1);
   evas_object_show(o);
   /* add net charts */
   chart_con = chart_container_new(con->bg_evas,0,0,0,0);
   flow_chart_net_in = flow_chart_new();
   flow_chart_color_set(flow_chart_net_in, 213, 91, 91, 255);
   flow_chart_get_value_function_set(flow_chart_net_in, net_in_usage_get);
   flow_chart_update_rate_set(flow_chart_net_in, tmp_net_interval);
   chart_container_chart_add(chart_con, flow_chart_net_in);
   face->chart_net = chart_con;   
   flow_chart_callback_set(flow_chart_net_in, _monitor_net_in_text_update_callcack, face);
   
   flow_chart_net_out = flow_chart_new();
   flow_chart_color_set(flow_chart_net_out, 51, 181, 69, 255);
   flow_chart_get_value_function_set(flow_chart_net_out, net_out_usage_get);
   flow_chart_update_rate_set(flow_chart_net_out, tmp_net_interval);
   flow_chart_alignment_set(flow_chart_net_out, 0);
   chart_container_chart_add(chart_con, flow_chart_net_out);
   flow_chart_callback_set(flow_chart_net_out, _monitor_net_out_text_update_callcack, face);

   o = evas_object_rectangle_add(con->bg_evas);
   face->net_ev_obj = o;
   evas_object_color_set(o, 255,255,255,0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _monitor_face_cb_mouse_down, face);
   evas_object_show(o);

   /* setup wlan */
   o = edje_object_add(con->bg_evas);
   face->wlan = o;
   edje_object_file_set(o, PACKAGE_DATA_DIR"/monitor.edj", "monitor/wlan");
   e_table_pack(face->table_object, o, 3, 0, 1, 1);
   e_table_pack_options_set(o, 1, 1, 1, 1, 0.5, 0.5, 0, 0, -1, -1);   
   evas_object_layer_set(o, evas_object_layer_get(face->monitor_object)+1);
   evas_object_show(o);
   /* add wlan charts */
   chart_con = chart_container_new(con->bg_evas,0,0,0,0);
   flow_chart_wlan_link = flow_chart_new();
   flow_chart_color_set(flow_chart_wlan_link, 33, 100, 220, 255);
   flow_chart_get_value_function_set(flow_chart_wlan_link, wlan_link_get);
   flow_chart_update_rate_set(flow_chart_wlan_link, tmp_wlan_interval);
   chart_container_chart_add(chart_con, flow_chart_wlan_link);
   face->chart_wlan = chart_con;
   flow_chart_callback_set(flow_chart_wlan_link, _monitor_wlan_link_text_update_callcack, face);

   o = evas_object_rectangle_add(con->bg_evas);
   face->wlan_ev_obj = o;
   evas_object_color_set(o, 255,255,255,0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _monitor_face_cb_mouse_down, face);
   evas_object_show(o);

   /* setup gadman */
   face->gmc = e_gadman_client_new(con->gadman);
   e_gadman_client_domain_set(face->gmc, "module.monitor", _monitor_count++);
   e_gadman_client_policy_set(face->gmc,
			      E_GADMAN_POLICY_ANYWHERE |
			      E_GADMAN_POLICY_HMOVE |
			      E_GADMAN_POLICY_VMOVE |
			      E_GADMAN_POLICY_HSIZE |
			      E_GADMAN_POLICY_VSIZE);
   e_gadman_client_min_size_set(face->gmc, 14, 7);
   e_gadman_client_align_set(face->gmc, 1.0, 1.0);
   e_gadman_client_resize(face->gmc, 160, 40);
   e_gadman_client_change_func_set(face->gmc, _monitor_face_cb_gmc_change, face);
   e_gadman_client_load(face->gmc);

   evas_event_thaw(con->bg_evas);
   return face;
}

static void 
_monitor_cpu_text_update_callcack(Flow_Chart *chart, void *data)
{
  Monitor_Face *face;
  char buf[64];

  face = data;

  snprintf(buf, 64, "%i%%", chart->current_value);
  edje_object_part_text_set(face->cpu, "cpu-text", buf);
}

static void 
_monitor_mem_real_text_update_callback(Flow_Chart *chart, void *data)
{
  Monitor_Face *face;
  char buf[64];

  long kbytes = mem_real_get();

  face = data;
  
  if (kbytes > 1048576 )
    snprintf(buf, 64, "%ldGB", kbytes/1048576);
  else if (kbytes > 1024 && kbytes < 1048576 )
    snprintf(buf, 64, "%ldMB", kbytes/1024);
  else
    snprintf(buf, 64, "%ldKB", kbytes);
  
  edje_object_part_text_set(face->mem, "mem-real-text", buf);
}

static void 
_monitor_mem_swap_text_update_callback(Flow_Chart *chart, void *data)
{
  Monitor_Face *face;
  char buf[64];

  long kbytes = mem_swap_get();

  face = data;
  
  if (kbytes > 1048576 )
    snprintf(buf, 64, "%ldGB", kbytes/1048576);
  else if (kbytes > 1024 && kbytes < 1048576 )
    snprintf(buf, 64, "%ldMB", kbytes/1024);
  else
    snprintf(buf, 64, "%ldKB", kbytes);
  
  edje_object_part_text_set(face->mem, "mem-swap-text", buf);
}

static void 
_monitor_net_in_text_update_callcack(Flow_Chart *chart, void *data)
{
  Monitor_Face *face;
  char buf[64];

  long bytes = net_bytes_in_get();

  face = data;
  
  if (bytes > 1048576 )
    snprintf(buf, 64, "%ldMB", bytes/1048576);
  else if (bytes > 1024 && bytes < 1048576 )
    snprintf(buf, 64, "%ldKB", bytes/1024);
  else
    snprintf(buf, 64, "%ldB", bytes);
  
  edje_object_part_text_set(face->net, "net-in-text", buf);
}

static void 
_monitor_net_out_text_update_callcack(Flow_Chart *chart, void *data)
{
  Monitor_Face *face;
  char buf[64];

  long bytes = net_bytes_out_get();

  face = data;

  if (bytes > 1048576 )
    snprintf(buf, 64, "%ldMB", bytes/1048576);
  else if (bytes > 1024 && bytes < 1048576 )
    snprintf(buf, 64, "%ldKB", bytes/1024);
  else
    snprintf(buf, 64, "%ldB", bytes);
  
  edje_object_part_text_set(face->net, "net-out-text", buf);
}

static void 
_monitor_wlan_link_text_update_callcack(Flow_Chart *chart, void *data)
{
   Monitor_Face *face;
   char buf[64];
   long link = wlan_link_get();
   face = data;
   snprintf(buf, 64, "LNK: %ld", link);
   edje_object_part_text_set(face->wlan, "wlan-link-text", buf);
}

static void
_monitor_face_free(Monitor_Face *face)
{
   e_object_unref(E_OBJECT(face->con));
   e_object_del(E_OBJECT(face->gmc));
 
   evas_object_del(face->cpu);
   evas_object_del(face->cpu_ev_obj);
   evas_object_del(face->mem);
   evas_object_del(face->mem_ev_obj);
   evas_object_del(face->net);
   evas_object_del(face->net_ev_obj);
   evas_object_del(face->wlan);
   evas_object_del(face->wlan_ev_obj);

   chart_container_del(face->chart_cpu);
   chart_container_del(face->chart_mem);
   chart_container_del(face->chart_net);
   chart_container_del(face->chart_wlan);
   
   if (face->monitor_object) evas_object_del(face->monitor_object);
   if (face->table_object) evas_object_del(face->table_object);

   e_object_del(E_OBJECT(face->menu));

   if (face->conf->wlan_interface) evas_stringshare_del(face->conf->wlan_interface);
   if (face->conf->net_interface) evas_stringshare_del(face->conf->net_interface);
   free(face->conf);
   free(face);
   _monitor_count--;
}

static void
_monitor_face_cb_gmc_change(void *data, E_Gadman_Client *gmc, E_Gadman_Change change)
{
   Monitor_Face *face;
   Evas_Coord x, y, w, h;
   
   face = data;
   switch (change)
     {
      case E_GADMAN_CHANGE_MOVE_RESIZE:
	 e_gadman_client_geometry_get(face->gmc, &x, &y, &w, &h);

	 evas_object_move(face->monitor_object, x, y);
	 evas_object_resize(face->monitor_object, w, h);

	 evas_object_geometry_get(face->cpu,  &x, &y, &w, &h);
	 evas_object_move(face->cpu_ev_obj, x, y);
	 evas_object_resize(face->cpu_ev_obj, w, h);
	 chart_container_move(face->chart_cpu, x+2,y+2);
	 chart_container_resize(face->chart_cpu, w-4,h-4);

	 evas_object_geometry_get(face->mem,  &x, &y, &w, &h);
	 evas_object_move(face->mem_ev_obj, x, y);
	 evas_object_resize(face->mem_ev_obj, w, h);
	 chart_container_move(face->chart_mem, x+2,y+2);
	 chart_container_resize(face->chart_mem, w-4,h-4);
	 
	 evas_object_geometry_get(face->net,  &x, &y, &w, &h);
	 evas_object_move(face->net_ev_obj, x, y);
	 evas_object_resize(face->net_ev_obj, w, h);
	 chart_container_move(face->chart_net, x+2,y+2);
	 chart_container_resize(face->chart_net, w-4,h-4);

	 evas_object_geometry_get(face->wlan,  &x, &y, &w, &h);
	 evas_object_move(face->wlan_ev_obj, x, y);
	 evas_object_resize(face->wlan_ev_obj, w, h);
	 chart_container_move(face->chart_wlan, x+2,y+2);
	 chart_container_resize(face->chart_wlan, w-4,h-4);

	 break;
      case E_GADMAN_CHANGE_RAISE:
	//evas_object_raise(face->exit_event_object);
	 break;
      case E_GADMAN_CHANGE_EDGE:
      case E_GADMAN_CHANGE_ZONE:
	 /* FIXME
	  * Must we do something here?
	  */
	 break;
     }
}

static void
_monitor_face_cb_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Monitor_Face *face;

   face = data;
   e_gadman_mode_set(face->gmc->gadman, E_GADMAN_MODE_EDIT);
}

static void
_monitor_face_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Monitor_Face *face;
   Evas_Event_Mouse_Down *ev;
   
   face = data;
   ev = event_info;
   
   if (ev->button == 3)
     {
	e_menu_activate_mouse(face->menu, e_zone_current_get(face->con),
			      ev->output.x, ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN,ev->timestamp);
	e_util_container_fake_mouse_up_all_later(face->con);
     }
   /*
   else if(ev->button == 1)
     {
       Evas_Coord x, y, w, h;
       e_gadman_client_geometry_get(face->gmc, &x, &y, &w, &h);

       if(ev->output.x < x + w / 2)
	 restart = 1;
       ecore_main_loop_quit();	 
     }
   */
}
/*
static void
_monitor_face_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, 
			  void *event_info)
{
}

static void
_monitor_face_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, 
			   void *event_info)
{
}
*/

static void
_monitor_face_menu_new(Monitor_Face *face)
{
   E_Menu *mn;
   E_Menu_Item *mi;
   
   mn = e_menu_new();
   face->menu = mn;
	
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Configuration"));
   e_menu_item_callback_set(mi, _monitor_menu_cb_configure, face);
   
   mi = e_menu_item_new(face->menu);
   e_menu_item_label_set(mi, _("Edit Mode"));
   e_menu_item_callback_set(mi, _monitor_face_cb_menu_edit, face);	
}

void _monitor_cb_config_updated(void *data)
{
   Monitor_Face *face;
   
   face = data;
   flow_chart_update_rate_set(flow_chart_cpu, face->conf->cpu_interval);

   mem_real_ignore_cached_set(face->conf->mem_real_ignore_cached);
   mem_real_ignore_buffers_set(face->conf->mem_real_ignore_buffers);
   flow_chart_update_rate_set(flow_chart_mem_real, face->conf->mem_interval);
   flow_chart_update_rate_set(flow_chart_mem_swap, face->conf->mem_interval);

   wlan_interface_set(face->conf->wlan_interface);
   flow_chart_update_rate_set(flow_chart_wlan_link, face->conf->wlan_interval);

   net_interface_set(face->conf->net_interface);
   flow_chart_update_rate_set(flow_chart_net_in, face->conf->net_interval);
   flow_chart_update_rate_set(flow_chart_net_out, face->conf->net_interval);   
}

static void 
_monitor_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Monitor_Face *f;
   
   f = data;
   if (!f) return;
   _config_monitor_module(f->con, f);	
}
