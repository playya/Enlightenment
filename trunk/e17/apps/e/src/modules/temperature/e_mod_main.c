/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "e_mod_main.h"

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

/***************************************************************************/
/**/
/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);
static const char *_gc_id_new(void);
static void _gc_id_del(const char *id);
/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
     "temperature",
     {
        _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, _gc_id_del
     },
   E_GADCON_CLIENT_STYLE_PLAIN
};
/**/
/***************************************************************************/

/***************************************************************************/
/**/
/* actual module specifics */

static int _temperature_cb_exe_data(void *data, int type, void *event);
static int _temperature_cb_exe_del(void *data, int type, void *event);
static void _temperature_face_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _temperature_face_cb_post_menu(void *data, E_Menu *m);
static void _temperature_face_level_set(Config_Face *inst, double level);
static void _temperature_face_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi);

static Evas_Bool _temperature_face_shutdown(const Evas_Hash *hash __UNUSED__, const char *key __UNUSED__, void *hdata, void *fdata __UNUSED__);
static Evas_Bool _temperature_face_id_max(const Evas_Hash *hash __UNUSED__, const char *key, void *hdata __UNUSED__, void *fdata);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_face_edd = NULL;

static int uuid = 0;

static Config *temperature_config = NULL;

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Config_Face *inst;

   inst = evas_hash_find(temperature_config->faces, id);
   if (!inst)
     {
	inst = E_NEW(Config_Face, 1);
	inst->id = evas_stringshare_add(id);
	inst->poll_interval = 128;
	inst->low = 30;
	inst->high = 80;
	inst->sensor_type = SENSOR_TYPE_NONE;
	inst->sensor_name = NULL;
	inst->units = CELCIUS;
	temperature_config->faces = evas_hash_direct_add(temperature_config->faces, inst->id, inst);
     }
   if (!inst->id) evas_stringshare_add(id);
   E_CONFIG_LIMIT(inst->poll_interval, 1, 1024);
   E_CONFIG_LIMIT(inst->low, 0, 100);
   E_CONFIG_LIMIT(inst->high, 0, 220);
   E_CONFIG_LIMIT(inst->units, CELCIUS, FAHRENHEIT);

   o = edje_object_add(gc->evas);
   e_theme_edje_object_set(o, "base/theme/modules/temperature",
			   "e/modules/temperature/main");
   
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   
   inst->gcc = gcc;
   inst->o_temp = o;
   inst->module = temperature_config->module;
   inst->have_temp = -1;

   inst->tempget_data_handler = 
     ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
			     _temperature_cb_exe_data,
			     inst);
   inst->tempget_del_handler = 
     ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
			     _temperature_cb_exe_del,
			     inst);
   
   temperature_face_update_config(inst);
   
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
				  _temperature_face_cb_mouse_down, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Config_Face *inst;
   
   inst = gcc->data;
   if (inst->tempget_exe)
     {
	ecore_exe_terminate(inst->tempget_exe);
	ecore_exe_free(inst->tempget_exe);
	inst->tempget_exe = NULL;
     }
   if (inst->tempget_data_handler)
     {
	ecore_event_handler_del(inst->tempget_data_handler);
	inst->tempget_data_handler = NULL;
     }
   if (inst->tempget_del_handler)
     {
	ecore_event_handler_del(inst->tempget_del_handler);
	inst->tempget_del_handler = NULL;
     }
   if (inst->o_temp) evas_object_del(inst->o_temp);
   inst->o_temp = NULL;
   if (inst->config_dialog) e_object_del(E_OBJECT(inst->config_dialog));
   inst->config_dialog = NULL;
   if (inst->menu) e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void
_gc_orient(E_Gadcon_Client *gcc)
{
   Config_Face *inst;
   
   inst = gcc->data;
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}
   
static char *
_gc_label(void)
{
   return _("Temperature");
}

static Evas_Object *
_gc_icon(Evas *evas)
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-temperature.edj",
	    e_module_dir_get(temperature_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(void)
{
   Config_Face *inst;
   char         id[128];

   snprintf(id, sizeof(id), "%s.%d", _gadcon_class.name, ++uuid);

   inst = E_NEW(Config_Face, 1);
   inst->id = evas_stringshare_add(id);
   inst->poll_interval = 128;
   inst->low = 30;
   inst->high = 80;
   inst->sensor_type = SENSOR_TYPE_NONE;
   inst->sensor_name = NULL;
   inst->units = CELCIUS;
   temperature_config->faces = evas_hash_direct_add(temperature_config->faces, inst->id, inst);
   return inst->id;
}

static void
_gc_id_del(const char *id)
{
   Config_Face *inst;

   inst = evas_hash_find(temperature_config->faces, id);
   if (inst)
     {
	temperature_config->faces = evas_hash_del(temperature_config->faces, id, inst);
	if (inst->sensor_name) evas_stringshare_del(inst->sensor_name);
	free(inst);
     }
}

/**/
/***************************************************************************/

/***************************************************************************/
/**/
static int
_temperature_cb_exe_data(void *data, int type, void *event)
{    
   Ecore_Exe_Event_Data *ev;
   Config_Face *inst;
   int temp;
   
   ev = event;
   inst = data;
   if (ev->exe != inst->tempget_exe) return 1;
   temp = -999;
   if ((ev->lines) && (ev->lines[0].line))
     {
	int i;
	
	for (i = 0; ev->lines[i].line; i++)
	  {
	     if (!strcmp(ev->lines[i].line, "ERROR"))
	       temp = -999;
	     else
	       temp = atoi(ev->lines[i].line);
	  }
     }
   if (temp != -999)
     {
	char *utf8;
	char buf[256];

	if (inst->units == FAHRENHEIT)
	  temp = (temp * 9.0 / 5.0) + 32;
	
	if (inst->have_temp != 1)
	  {
	     /* enable therm object */
	     edje_object_signal_emit(inst->o_temp, "e,state,known", "");
	     inst->have_temp = 1;
	  }

	if (inst->units == FAHRENHEIT) 
	  snprintf(buf, sizeof(buf), "%i�F", temp);
	else
	  snprintf(buf, sizeof(buf), "%i�C", temp);  
	utf8 = ecore_txt_convert("iso-8859-1", "utf-8", buf);
	
        _temperature_face_level_set(inst,
				    (double)(temp - inst->low) /
				    (double)(inst->high - inst->low));
	edje_object_part_text_set(inst->o_temp, "e.text.reading", utf8);
	free(utf8);
     }
   else
     {
	if (inst->have_temp != 0)
	  {
	     /* disable therm object */
	     edje_object_signal_emit(inst->o_temp, "e,state,unknown", "");
	     edje_object_part_text_set(inst->o_temp, "e.text.reading", "N/A");
	     _temperature_face_level_set(inst, 0.5);
	     inst->have_temp = 0;
	  }
     }
   return 0;
}

static int
_temperature_cb_exe_del(void *data, int type, void *event)
{
   Ecore_Exe_Event_Del *ev;
   Config_Face *inst;
   
   ev = event;
   inst = data;
   if (ev->exe != inst->tempget_exe) return 1;
   inst->tempget_exe = NULL;
   return 0;
}

static void
_temperature_face_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Config_Face *inst;
   Evas_Event_Mouse_Down *ev;
   
   inst = data;
   ev = event_info;
   if ((ev->button == 3) && (!inst->menu))
     {
	E_Menu *mn;
	E_Menu_Item *mi;
	int cx, cy, cw, ch;
	
	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _temperature_face_cb_post_menu, inst);
	inst->menu = mn;
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Configuration"));
	e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
	e_menu_item_callback_set(mi, _temperature_face_cb_menu_configure, inst);
	
	e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
	
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon,
					  &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	e_util_evas_fake_mouse_up_later(inst->gcc->gadcon->evas,
					ev->button);
     }
}

static void
_temperature_face_cb_post_menu(void *data, E_Menu *m)
{
   Config_Face *inst;

   inst = data;

   if (!inst->menu) return;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void
_temperature_face_level_set(Config_Face *inst, double level)
{
   Edje_Message_Float msg;

   if (level < 0.0) level = 0.0;
   else if (level > 1.0) level = 1.0;
   msg.val = level;
   edje_object_message_send(inst->o_temp, EDJE_MESSAGE_FLOAT, 1, &msg);
}

static void
_temperature_face_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Config_Face *inst;

   inst = data;
   if (inst->config_dialog) return;
   config_temperature_module(inst);
}

static Evas_Bool
_temperature_face_shutdown(const Evas_Hash *hash __UNUSED__, const char *key __UNUSED__, void *hdata, void *fdata __UNUSED__)
{
   Config_Face *inst;

   inst = hdata;

   if (inst->sensor_name) evas_stringshare_del(inst->sensor_name);
   if (inst->id) evas_stringshare_del(inst->id);
   free(inst);
   return 1;
}

static Evas_Bool
_temperature_face_id_max(const Evas_Hash *hash __UNUSED__, const char *key, void *hdata __UNUSED__, void *fdata)
{
   const char *p;
   int *max;
   int num = -1;

   max = fdata;
   p = strrchr(key, '.');
   if (p) num = atoi(p + 1);
   if (num > *max) *max = num;
   return 1;
}

void 
temperature_face_update_config(Config_Face *inst)
{
   char buf[PATH_MAX];

   if (inst->tempget_exe)
     {
	ecore_exe_terminate(inst->tempget_exe);
	ecore_exe_free(inst->tempget_exe);
	inst->tempget_exe = NULL;
     }
   snprintf(buf, sizeof(buf),
	    "%s/%s/tempget %i \"%s\" %i", 
	    e_module_dir_get(temperature_config->module), MODULE_ARCH, 
	    inst->sensor_type,
	    (inst->sensor_name != NULL ? inst->sensor_name : "(null)"),
	    inst->poll_interval);
   inst->tempget_exe = ecore_exe_pipe_run(buf, 
					  ECORE_EXE_PIPE_READ | 
					  ECORE_EXE_PIPE_READ_LINE_BUFFERED |
					  ECORE_EXE_NOT_LEADER,
					  inst);
}

Ecore_List *
temperature_get_bus_files(const char* bus)
{
   Ecore_List *result;
   Ecore_List *therms;
   char        path[PATH_MAX];
   char		busdir[PATH_MAX];
   
   result = ecore_list_new();
   if (result)
     {
	ecore_list_free_cb_set(result, free);
	snprintf(busdir, sizeof(busdir), "/sys/bus/%s/devices", bus);
	/* Look through all the devices for the given bus. */
	therms = ecore_file_ls(busdir);
	if (therms)
	  {
	     char *name;
	     
	     while ((name = ecore_list_next(therms)))
	       {
		  Ecore_List *files;
		  
		  /* Search each device for temp*_input, these should be 
		   * temperature devices. */
		  snprintf(path, sizeof(path),
			   "%s/%s", busdir, name);
		  files = ecore_file_ls(path);
		  if (files)
		    {
		       char *file;
		       
		       while ((file = ecore_list_next(files)))
			 {
			    if ((!strncmp("temp", file, 4)) && 
				(!strcmp("_input", &file[strlen(file) - 6])))
			      {
				 char *f;
				 
				 snprintf(path, sizeof(path),
					  "%s/%s/%s", busdir, name, file);
				 f = strdup(path);
				 if (f) ecore_list_append(result, f);
			      }
			 }
		       ecore_list_destroy(files);
		    }
	       }
	     ecore_list_destroy(therms);
	  }
	ecore_list_first_goto(result);
     }
   return result;
}


/***************************************************************************/
/**/
/* module setup */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION,
     "Temperature"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   conf_face_edd = E_CONFIG_DD_NEW("Temperature_Config_Face", Config_Face);
#undef T
#undef D
#define T Config_Face
#define D conf_face_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, poll_interval, INT);
   E_CONFIG_VAL(D, T, low, INT);
   E_CONFIG_VAL(D, T, high, INT);
   E_CONFIG_VAL(D, T, sensor_type, INT);
   E_CONFIG_VAL(D, T, sensor_name, STR);
   E_CONFIG_VAL(D, T, units, INT);

   conf_edd = E_CONFIG_DD_NEW("Temperature_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_HASH(D, T, faces, conf_face_edd);

   temperature_config = e_config_domain_load("module.temperature", conf_edd);
   if (!temperature_config)
     {
	temperature_config = E_NEW(Config, 1);
     }
   else
     {
	evas_hash_foreach(temperature_config->faces, _temperature_face_id_max, &uuid);
     }
   temperature_config->module = m;
   
   e_gadcon_provider_register(&_gadcon_class);
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   e_gadcon_provider_unregister(&_gadcon_class);
   
   evas_hash_foreach(temperature_config->faces, _temperature_face_shutdown, NULL);
   evas_hash_free(temperature_config->faces);
   free(temperature_config);
   temperature_config = NULL;
   E_CONFIG_DD_FREE(conf_face_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   e_config_domain_save("module.temperature", conf_edd, temperature_config);
   return 1;
}
/**/
/***************************************************************************/
