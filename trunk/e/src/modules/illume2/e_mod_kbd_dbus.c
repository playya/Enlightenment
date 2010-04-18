#include "e_illume_private.h"
#include "e_mod_kbd_dbus.h"

/* local function prototypes */
static void _e_mod_kbd_dbus_ignore_load(void);
static void _e_mod_kbd_dbus_ignore_load_file(const char *file);
static void _e_mod_kbd_dbus_cb_input_kbd(void *data __UNUSED__, void *reply, DBusError *err);
static void _e_mod_kbd_dbus_cb_input_kbd_is(void *data, void *reply, DBusError *err);
static void _e_mod_kbd_dbus_kbd_add(const char *udi);
static void _e_mod_kbd_dbus_kbd_del(const char *udi);
static void _e_mod_kbd_dbus_kbd_eval(void);
static void _e_mod_kbd_dbus_dev_add(void *data __UNUSED__, DBusMessage *msg);
static void _e_mod_kbd_dbus_dev_del(void *data __UNUSED__, DBusMessage *msg);
static void _e_mod_kbd_dbus_dev_chg(void *data __UNUSED__, DBusMessage *msg);

/* local variables */
static int have_real_kbd = 0;
static E_DBus_Connection *_dbus_conn = NULL;
static E_DBus_Signal_Handler *_dev_add = NULL;
static E_DBus_Signal_Handler *_dev_del = NULL;
static E_DBus_Signal_Handler *_dev_chg = NULL;
static Eina_List *_dbus_kbds = NULL, *_ignore_kbds = NULL;

void 
e_mod_kbd_dbus_init(void) 
{
   /* load the 'ignored' keyboard file */
   _e_mod_kbd_dbus_ignore_load();

   /* try to attach to the system dbus */
   if (!(_dbus_conn = e_dbus_bus_get(DBUS_BUS_SYSTEM))) return;

   /* ask HAL for any input keyboards */
   e_hal_manager_find_device_by_capability(_dbus_conn, "input.keyboard", 
                                           _e_mod_kbd_dbus_cb_input_kbd, NULL);

   /* setup dbus signal handlers for when a device gets added/removed/changed */
   _dev_add = 
     e_dbus_signal_handler_add(_dbus_conn, "org.freedesktop.Hal", 
                               "/org/freedesktop/Hal/Manager", 
                               "org.freedesktop.Hal.Manager", 
                               "DeviceAdded", _e_mod_kbd_dbus_dev_add, NULL);
   _dev_del = 
     e_dbus_signal_handler_add(_dbus_conn, "org.freedesktop.Hal", 
                               "/org/freedesktop/Hal/Manager", 
                               "org.freedesktop.Hal.Manager", 
                               "DeviceRemoved", _e_mod_kbd_dbus_dev_del, NULL);
   _dev_chg = 
     e_dbus_signal_handler_add(_dbus_conn, "org.freedesktop.Hal", 
                               "/org/freedesktop/Hal/Manager", 
                               "org.freedesktop.Hal.Manager", 
                               "NewCapability", _e_mod_kbd_dbus_dev_chg, NULL);
}

void 
e_mod_kbd_dbus_shutdown(void) 
{
   char *str;

   /* remove the dbus signal handlers if we can */
   if (_dev_add) e_dbus_signal_handler_del(_dbus_conn, _dev_add);
   if (_dev_del) e_dbus_signal_handler_del(_dbus_conn, _dev_del);
   if (_dev_chg) e_dbus_signal_handler_del(_dbus_conn, _dev_chg);

   /* free the list of ignored keyboards */
   EINA_LIST_FREE(_ignore_kbds, str)
     eina_stringshare_del(str);

   /* free the list of keyboards */
   EINA_LIST_FREE(_dbus_kbds, str)
     eina_stringshare_del(str);
}

/* local functions */
static void 
_e_mod_kbd_dbus_ignore_load(void) 
{
   char buff[PATH_MAX];

   /* load the 'ignore' file from the user's home dir */
   e_user_dir_concat_static(buff, "keyboards/ignore_built_in_keyboards");
   _e_mod_kbd_dbus_ignore_load_file(buff);

   /* load the 'ignore' file from the system/module dir */
   snprintf(buff, sizeof(buff), 
            "%s/ignore_built_in_keyboards", _e_illume_mod_dir);
   _e_mod_kbd_dbus_ignore_load_file(buff);
}

static void 
_e_mod_kbd_dbus_ignore_load_file(const char *file) 
{
   char buff[PATH_MAX];
   FILE *f;

   /* can this file be opened */
   if (!(f = fopen(file, "r"))) return;

   /* parse out the info in the ignore file */
   while (fgets(buff, sizeof(buff), f))
     {
	char *p;
	int len;

	if (buff[0] == '#') continue;
        len = strlen(buff);
	if (len > 0)
	  {
	     if (buff[len - 1] == '\n') buff[len - 1] = 0;
	  }
	p = buff;
	while (isspace(*p)) p++;

        /* append this kbd to the ignore list */
	if (*p) 
          {
             _ignore_kbds = 
               eina_list_append(_ignore_kbds, eina_stringshare_add(p));
          }
     }
   fclose(f);
}

static void 
_e_mod_kbd_dbus_cb_input_kbd(void *data __UNUSED__, void *reply, DBusError *err) 
{
   E_Hal_Manager_Find_Device_By_Capability_Return *ret = reply;
   Eina_List *l;
   char *dev;

   if ((!ret) || (!ret->strings)) return;

   /* if dbus errored then cleanup and get out */
   if (dbus_error_is_set(err)) 
     {
        dbus_error_free(err);
        return;
     }

   /* for each returned keyboard, add it and evaluate it */
   EINA_LIST_FOREACH(ret->strings, l, dev) 
     {
        _e_mod_kbd_dbus_kbd_add(dev);
        _e_mod_kbd_dbus_kbd_eval();
     }
}

static void 
_e_mod_kbd_dbus_cb_input_kbd_is(void *data, void *reply, DBusError *err) 
{
   E_Hal_Device_Query_Capability_Return *ret = reply;
   char *udi = data;

   /* if dbus errored then cleanup and get out */
   if (dbus_error_is_set(err)) 
     {
        dbus_error_free(err);
        return;
     }

   /* if it's an input keyboard, than add it and eval */
   if ((ret) && (ret->boolean)) 
     {
        if (udi) 
          {
             _e_mod_kbd_dbus_kbd_add(udi);
             _e_mod_kbd_dbus_kbd_eval();
          }
     }
}

static void 
_e_mod_kbd_dbus_kbd_add(const char *udi) 
{
   const char *str;
   Eina_List *l;

   if (!udi) return;
   EINA_LIST_FOREACH(_dbus_kbds, l, str)
     if (!strcmp(str, udi)) return;
   _dbus_kbds = eina_list_append(_dbus_kbds, eina_stringshare_add(udi));
}

static void 
_e_mod_kbd_dbus_kbd_del(const char *udi) 
{
   const char *str;
   Eina_List *l;

   if (!udi) return;
   EINA_LIST_FOREACH(_dbus_kbds, l, str)
     if (!strcmp(str, udi)) 
       {
          eina_stringshare_del(str);
          _dbus_kbds = eina_list_remove_list(_dbus_kbds, l);
          return;
       }
}

static void 
_e_mod_kbd_dbus_kbd_eval(void) 
{
   Eina_List *l, *ll;
   const char *g, *gg;
   int have_real = 0;

   have_real = eina_list_count(_dbus_kbds);
   EINA_LIST_FOREACH(_dbus_kbds, l, g)
     EINA_LIST_FOREACH(_ignore_kbds, ll, gg)
       if (e_util_glob_match(g, gg)) 
         {
            have_real--;
            break;
         }

   if (have_real != have_real_kbd) 
     {
        have_real_kbd = have_real;
#if 0
//        if (have_real_kbd) e_kbd_all_disable();
        else
#endif
//          e_kbd_all_enable();
     }
}

static void 
_e_mod_kbd_dbus_dev_add(void *data __UNUSED__, DBusMessage *msg) 
{
   DBusError err;
   char *udi;

   dbus_error_init(&err);
   dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &udi, DBUS_TYPE_INVALID);
   e_hal_device_query_capability(_dbus_conn, udi, "input.keyboard", 
                                 _e_mod_kbd_dbus_cb_input_kbd_is, udi);
}

static void 
_e_mod_kbd_dbus_dev_del(void *data __UNUSED__, DBusMessage *msg) 
{
   DBusError err;
   char *udi;

   dbus_error_init(&err);
   dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &udi, DBUS_TYPE_INVALID);
   if (udi) 
     {
        _e_mod_kbd_dbus_kbd_del(udi);
        _e_mod_kbd_dbus_kbd_eval();
     }
}

static void 
_e_mod_kbd_dbus_dev_chg(void *data __UNUSED__, DBusMessage *msg) 
{
   DBusError err;
   char *udi, *cap;

   dbus_error_init(&err);
   dbus_message_get_args(msg, &err, DBUS_TYPE_STRING, &udi, 
                         DBUS_TYPE_STRING, &cap, DBUS_TYPE_INVALID);
   if (cap) 
     {
        if (!strcmp(cap, "input.keyboard")) 
          {
             if (udi) 
               {
                  _e_mod_kbd_dbus_kbd_add(udi);
                  _e_mod_kbd_dbus_kbd_eval();
               }
          }
        free(cap);
     }
}
