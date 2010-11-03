#include "e.h"
#include "e_mod_win.h"
#include "e_slipshelf.h"
#include "e_slipwin.h"
#include "e_mod_layout.h"
#include "e_kbd.h"
#include "e_kbd_int.h"
#include "e_busywin.h"
#include "e_busycover.h"
#include "e_cfg.h"
#include "e_flaunch.h"
#include "e_pwr.h"
#include "e_appwin.h"
#include "e_syswin.h"

// FIXME:
// 
// fix next/prev to keep a window list in order of most recently focused.

/* internal calls */
static void _app_fill(void);
static void _app_clear(void);
static void _app_add(E_Border *bd);
static void _app_del(E_Border *bd);
static void _app_promote_allow(E_Border *bd);
static void _app_promote(E_Border *bd);
static void _app_prev(void);
static void _app_next(void);
static void _app_home(void);
static void _app_close(E_Border *bd);

static void _cb_cfg_exec(const void *data, E_Container *con, const char *params, Efreet_Desktop *desktop);
static void _desktop_run(Efreet_Desktop *desktop);
static Eina_Bool _cb_zone_move_resize(void *data, int type, void *event);
static void _cb_resize(void);
static void _cb_run(void *data);
static Eina_Bool _cb_event_border_add(void *data, int type, void *event);
static Eina_Bool _cb_event_border_remove(void *data, int type, void *event);
static Eina_Bool _cb_event_border_focus_in(void *data, int type, void *event);
static Eina_Bool _cb_event_border_focus_out(void *data, int type, void *event);
static Eina_Bool _cb_event_exe_del(void *data, int type, void *event);
static Eina_Bool _cb_run_timeout(void *data);
static int _have_borders(void);
static void _cb_slipshelf_home(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipshelf_close(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipshelf_apps(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipshelf_keyboard(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipshelf_app_next(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipshelf_app_prev(const void *data, E_Slipshelf *ess, E_Slipshelf_Action action);
static void _cb_slipwin_border_select(void *data, E_Slipwin *esw, E_Border *bd);
static void _cb_slipshelf_border_select(void *data, E_Slipshelf *ess, E_Border *bd);
static void _cb_slipshelf_border_home2(void *data, E_Slipshelf *ess, E_Border *pbd);
static void _cb_selected(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _cb_efreet_cache_update(void *data, int type, void *event);
static void _apps_unpopulate(void);
static void _apps_populate(void);
static Eina_Bool _cb_update_deferred(void *data);
static void _cb_sys_con_close(void *data);
static void _cb_sys_con_home(void *data);

static void _e_illume_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _e_illume_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
static void _e_illume_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
static void _e_illume_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
//static void _e_illume_pan_scroll_update(void);

/* state */
/* public for cfg */
E_Slipshelf *slipshelf = NULL;

static E_Module *mod = NULL;
static E_Zone *zone = NULL;
static Evas *evas = NULL;
static Evas_Object *sf = NULL;
static Evas_Object *bx = NULL;
static Evas_Object *fm = NULL;
static Eina_List *desks = NULL;
static Eina_List *handlers = NULL;
static Eina_List *sels = NULL;
static E_Slipwin *slipwin = NULL;
static Ecore_Timer *defer = NULL;
static E_Kbd *vkbd = NULL;
static E_Kbd_Int *vkbd_int = NULL;
static E_Busywin *busywin = NULL;
static E_Busycover *busycover = NULL;
static E_Flaunch *flaunch = NULL;
static E_Appwin *appwin = NULL;
static E_Syswin *syswin = NULL;

static E_Sys_Con_Action *sys_con_act_close = NULL;
static E_Sys_Con_Action *sys_con_act_home = NULL;

/* called from the module core */
void
_e_mod_win_init(E_Module *m)
{
   mod = m;
   
   zone = e_util_container_zone_number_get(0, 0);
   
   ecore_x_window_background_color_set(zone->container->manager->root, 0, 0, 0);
   
   slipshelf = e_slipshelf_new(zone, e_module_dir_get(m));
   e_slipshelf_default_title_set(slipshelf, "ILLUME");
   if (!_have_borders())
     {
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APPS, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV, 0);
     }
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_HOME,
				   _cb_slipshelf_home, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_CLOSE,
				   _cb_slipshelf_close, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APPS,
				   _cb_slipshelf_apps, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_KEYBOARD,
				   _cb_slipshelf_keyboard, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT,
				   _cb_slipshelf_app_next, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV,
				   _cb_slipshelf_app_prev, NULL);
   e_slipshelf_border_select_callback_set(slipshelf, _cb_slipshelf_border_select, NULL);
   e_slipshelf_border_home_callback_set(slipshelf, _cb_slipshelf_border_home2, NULL);
   
   slipwin = e_slipwin_new(zone, e_module_dir_get(m));
   e_slipwin_border_select_callback_set(slipwin, _cb_slipwin_border_select, NULL);

   appwin = e_appwin_new(zone, e_module_dir_get(m));
   syswin = e_syswin_new(zone, e_module_dir_get(m));
   
   vkbd = e_kbd_new(zone, 
		    e_module_dir_get(m), 
		    e_module_dir_get(m),
		    e_module_dir_get(m));
   e_mod_win_cfg_kbd_start();

//   busywin = e_busywin_new(zone, e_module_dir_get(m));
   busycover = e_busycover_new(zone, e_module_dir_get(m));

   flaunch = e_flaunch_new(zone, e_module_dir_get(m));
   e_flaunch_desktop_exec_callback_set(flaunch, _desktop_run);
   
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (E_EVENT_BORDER_ADD, _cb_event_border_add, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (E_EVENT_BORDER_REMOVE, _cb_event_border_remove, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (E_EVENT_BORDER_FOCUS_IN, _cb_event_border_focus_in, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (E_EVENT_BORDER_FOCUS_OUT, _cb_event_border_focus_out, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (ECORE_EXE_EVENT_DEL, _cb_event_exe_del, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (EFREET_EVENT_DESKTOP_CACHE_UPDATE, _cb_efreet_cache_update, NULL));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (E_EVENT_ZONE_MOVE_RESIZE, _cb_zone_move_resize, NULL));
   
   evas = zone->container->bg_evas;

   _apps_unpopulate();
   _apps_populate();
   e_configure_registry_custom_desktop_exec_callback_set(_cb_cfg_exec, NULL);

   sys_con_act_close = e_sys_con_extra_action_register
     (_("Close"), "enlightenment/close", "button", _cb_sys_con_close, NULL);
   if (sys_con_act_close) sys_con_act_close->disabled = 1;
   sys_con_act_home = e_sys_con_extra_action_register
     (_("Home"), "enlightenment/home", "button", _cb_sys_con_home, NULL);
   if (sys_con_act_home) sys_con_act_home->disabled = 1;
   _app_fill();
}

void
_e_mod_win_shutdown(void)
{
   Ecore_Event_Handler *handle;

   _app_clear();
   if (sys_con_act_close)
     {
        e_sys_con_extra_action_unregister(sys_con_act_close);
        sys_con_act_close = NULL;
     }
   if (sys_con_act_home)
     {
        e_sys_con_extra_action_unregister(sys_con_act_home);
        sys_con_act_home = NULL;
     }
   e_object_del(E_OBJECT(flaunch));
   flaunch = NULL;
   if (busywin)
     {
	e_object_del(E_OBJECT(busywin));
	busywin = NULL;
     }
   if (busycover)
     {
	e_object_del(E_OBJECT(busycover));
	busycover = NULL;
     }
   e_mod_win_cfg_kbd_stop();
   
   e_object_del(E_OBJECT(vkbd));
   vkbd = NULL;
   e_configure_registry_custom_desktop_exec_callback_set(NULL, NULL);
   _apps_unpopulate();
   if (sf) evas_object_del(sf);
   if (bx) evas_object_del(bx);
   if (fm) evas_object_del(fm);
   e_object_del(E_OBJECT(slipshelf));
   slipshelf = NULL;
   e_object_del(E_OBJECT(slipwin));
   slipwin = NULL;
   e_object_del(E_OBJECT(appwin));
   appwin = NULL;
   e_object_del(E_OBJECT(syswin));
   syswin = NULL;

   EINA_LIST_FREE(handlers, handle)
     ecore_event_handler_del(handle);
}

static Ecore_Exe *_kbd_exe = NULL;
static Ecore_Event_Handler *_kbd_exe_exit_handler = NULL;

static Eina_Bool
_e_mod_win_win_cfg_kbd_cb_exit(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe ==_kbd_exe) _kbd_exe = NULL;
   return ECORE_CALLBACK_PASS_ON;
}
  
void
e_mod_win_cfg_kbd_start(void)
{
   if (illume_cfg->kbd.use_internal)
     {
	vkbd_int = e_kbd_int_new(e_module_dir_get(mod),
				 e_module_dir_get(mod),
				 e_module_dir_get(mod));
     }
   else if (illume_cfg->kbd.run_keyboard)
     {
	E_Exec_Instance *exeinst;
	Efreet_Desktop *desktop;
	
	desktop = efreet_util_desktop_file_id_find(illume_cfg->kbd.run_keyboard);
	if (!desktop)
	  {
	     Eina_List *kbds;
	     Efreet_Desktop *d;
	     
	     kbds = efreet_util_desktop_category_list("Keyboard");
	     if (kbds)
	       {
		  EINA_LIST_FREE(kbds, d)
		    {
		       const char *dname;
		       
		       dname = ecore_file_file_get(desktop->orig_path);
		       if (dname)
			 {
			    if (!strcmp(dname, illume_cfg->kbd.run_keyboard))
			      {
				 desktop = d;
				 efreet_desktop_ref(desktop);
			      }
			 }
		       efreet_desktop_free(d);
		    }
	       }
	  }
	if (desktop)
	  {
	     exeinst = e_exec(zone, desktop, NULL, NULL, "illume-kbd");
	     if (exeinst)
	       {
		  _kbd_exe = exeinst->exe;
		  _kbd_exe_exit_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _e_mod_win_win_cfg_kbd_cb_exit, NULL);
	       }
	     efreet_desktop_free(desktop);
	  }
     }
}

void
e_mod_win_cfg_kbd_stop(void)
{
   if (vkbd_int)
     {
	e_kbd_int_free(vkbd_int);
	vkbd_int = NULL;
     }
   if (_kbd_exe)
     {
	ecore_exe_interrupt(_kbd_exe);
	_kbd_exe = NULL;
     }
   if (_kbd_exe_exit_handler)
     ecore_event_handler_del(_kbd_exe_exit_handler);
   _kbd_exe_exit_handler = NULL;
}

void
e_mod_win_cfg_kbd_update(void)
{
   e_mod_win_cfg_kbd_stop();
   e_mod_win_cfg_kbd_start();
}

void
_e_mod_win_cfg_update(void)
{
   _apps_unpopulate();
   _apps_populate();
}

void
_e_mod_win_slipshelf_cfg_update(void)
{
   if (slipshelf) e_object_del(E_OBJECT(slipshelf));
   
   slipshelf = e_slipshelf_new(zone, e_module_dir_get(mod));
   e_slipshelf_default_title_set(slipshelf, "ILLUME");
   if (!_have_borders())
     {
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APPS, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV, 0);
     }
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_HOME,
				   _cb_slipshelf_home, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_CLOSE,
				   _cb_slipshelf_close, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APPS,
				   _cb_slipshelf_apps, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_KEYBOARD,
				   _cb_slipshelf_keyboard, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT,
				   _cb_slipshelf_app_next, NULL);
   e_slipshelf_action_callback_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV,
				   _cb_slipshelf_app_prev, NULL);
   e_slipshelf_border_select_callback_set(slipshelf, _cb_slipshelf_border_select, NULL);
   e_slipshelf_border_home_callback_set(slipshelf, _cb_slipshelf_border_home2, NULL);

   _cb_resize();
   _e_mod_layout_apply_all();
}

/* internal calls */
/////////

static Eina_List *applist = NULL;
static E_Border *nopromote = NULL;

static void
_app_fill(void)
{
   Eina_List *l;

   _app_clear();
   for (l = e_border_client_list(); l; l = l->next)
     applist = eina_list_append(applist, l->data);
}

static void
_app_clear(void)
{
   nopromote = NULL;
   if (applist)
     {
        eina_list_free(applist);
        applist = NULL;
     }
}

static void
_app_add(E_Border *bd)
{
   if (eina_list_data_find(applist, bd)) return;
   applist = eina_list_prepend(applist, bd);
}

static void
_app_del(E_Border *bd)
{
   if (bd == nopromote) nopromote = NULL;
   applist = eina_list_remove(applist, bd);
}

static void
_app_promote_allow(E_Border *bd)
{
   if (bd == nopromote) nopromote = NULL;
}

static void
_app_promote(E_Border *bd)
{
   if (bd == nopromote) return;
   applist = eina_list_remove(applist, bd);
   applist = eina_list_prepend(applist, bd);
}

static Eina_List *
__app_list(void)
{
   Eina_List *tlist = NULL, *l;
        E_Border *bd;
        
   EINA_LIST_FOREACH(applist, l, bd)
     {
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	tlist = eina_list_append(tlist, bd);
     }
   return tlist;
}

static Eina_List *
__app_find(Eina_List *list, E_Border *bd)
{
   Eina_List *l;
   E_Border *over;

   EINA_LIST_FOREACH(list, l, over)
     if (over == bd) return l;
   
   return NULL;
}

static void
_app_prev(void)
{
   E_Border *bd, *bd2;
   Eina_List *apps, *bl;
   
   // go to the next bd in the list and focus it
   bd2 = e_border_focused_get();
   apps = __app_list();
   if (!apps) return;
   if (!bd2)
     {
        bd = apps->data;
        nopromote = bd;
        _e_mod_layout_border_show(bd);
        eina_list_free(apps);
        return;
     }
   bl = __app_find(apps, bd2);
   if (!bl)
     {
        eina_list_free(apps);
        return;
     }
   if (!bl->next) _app_home();
   else
     {
        bd = bl->next->data;
        nopromote = bd;
        _e_mod_layout_border_show(bd);
     }
   eina_list_free(apps);
}

static void
_app_next(void)
{
   E_Border *bd, *bd2;
   Eina_List *apps, *bl;
   
   // go to the prev bd in the list and focus it
   bd2 = e_border_focused_get();
   apps = __app_list();
   if (!apps) return;
   if (!bd2)
     {
        bd = eina_list_last(apps)->data;
        nopromote = bd;
        _e_mod_layout_border_show(bd);
        eina_list_free(apps);
        return;
     }
   bl = __app_find(apps, bd2);
   if (!bl)
     {
        eina_list_free(apps);
        return;
     }
   if (!bl->prev) _app_home();
   else
     {
        bd = bl->prev->data;
        nopromote = bd;
        _e_mod_layout_border_show(bd);
     }
   eina_list_free(apps);
}

static void
_app_home(void)
{
   Eina_List *borders;
   Eina_List *l;
   E_Border *bd;
   
   borders = e_border_client_list();
   EINA_LIST_FOREACH(borders, l, bd)
     {
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	_e_mod_layout_border_hide(bd);
     }
}

static void
_app_close(E_Border *bd)
{
   if (e_object_is_del(E_OBJECT(bd))) return;
   if ((!bd->client.icccm.accepts_focus) &&
       (!bd->client.icccm.take_focus)) return;
   if (bd->client.netwm.state.skip_taskbar) return;
   if (bd->user_skip_winlist) return;
   _e_mod_layout_border_close(bd);
}

/////////

static Eina_Bool
_cb_zone_move_resize(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   _cb_resize();
   return ECORE_CALLBACK_PASS_ON;
}

static void
_cb_resize(void)
{
   Evas_Coord mw, mh;
   int x, y, w, h;

   e_slipshelf_safe_app_region_get(zone, &x, &y, &w, &h);
   w = zone->w;
   h = zone->y + zone->h - y;
   h -= flaunch->height;
   if (bx)
     {
	e_box_size_min_get(bx, &mw, &mh);
	if (mw < w) mw = w;
	evas_object_move(sf, x, y);
	evas_object_resize(bx, mw, mh);
	evas_object_resize(sf, w, h);
     }
   else
     {
	evas_object_move(sf, x, y);
	evas_object_resize(sf, w, h);
     }
}

typedef struct _Instance Instance;

struct _Instance
{
   Efreet_Desktop *desktop;
   E_Border       *border;
   Ecore_Timer    *timeout;
   int             startup_id;
   pid_t           pid;
   void           *handle;
};

static Eina_List *instances = NULL;

static void
_cb_cfg_exec(const void *data __UNUSED__, E_Container *con __UNUSED__, const char *params __UNUSED__, Efreet_Desktop *desktop)
{
   _desktop_run(desktop);
}

static void
_desktop_run(Efreet_Desktop *desktop)
{
   E_Exec_Instance *eins;
   Instance *ins;
   E_Border *bd;
   Eina_List *l;
   char *exename, *p;

   if (!desktop) return;
   if (!desktop->exec) return;
   EINA_LIST_FOREACH(instances, l, ins)
	if (ins->desktop == desktop)
	  {
	     if (ins->border)
	       _e_mod_layout_border_show(ins->border);
	     return;
	  }
   exename = NULL;
   p = strchr(desktop->exec, ' ');
   if (!p)
     exename = strdup(desktop->exec);
   else
     {
	exename = malloc(p - desktop->exec + 1);
	if (exename)
	  {
	     eina_strlcpy(exename, desktop->exec, p - desktop->exec + 1);
	  }
     }
   if (exename)
     {
	p = strrchr(exename, '/');
	if (p) strcpy(exename, p + 1);
     }
   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     {
	if (e_exec_startup_id_pid_find(bd->client.netwm.pid,
				       bd->client.netwm.startup_id) == desktop)
	  {
	     _e_mod_layout_border_show(bd);
	     if (exename) free(exename);
	     return;
	  }
	if (exename)
	  {
	     if (bd->client.icccm.command.argv)
	       {
		  char *pp;
		  
		  pp = strrchr(bd->client.icccm.command.argv[0], '/');
		  if (!pp) pp = bd->client.icccm.command.argv[0];
		  if (!strcmp(exename, pp))
		    {
		       _e_mod_layout_border_show(bd);
		       if (exename) free(exename);
		       return;
		    }
	       }
	     if ((bd->client.icccm.name) &&
		 (!strcasecmp(bd->client.icccm.name, exename)))
	       {
		  _e_mod_layout_border_show(bd);
		  if (exename) free(exename);
		  return;
	       }
	  }
     }
   if (exename) free(exename);

   ins = calloc(1, sizeof(Instance));
   if (!ins) return;
   eins = e_exec(zone, desktop, NULL, NULL, "illume-launcher");
   efreet_desktop_ref(desktop);
   ins->desktop = desktop;
   if (eins)
     {
	ins->startup_id = eins->startup_id;
	ins->pid = ecore_exe_pid_get(eins->exe);
     }
   ins->timeout = ecore_timer_add(20.0, _cb_run_timeout, ins);
   
     {
	char buf[256];
	
	snprintf(buf, sizeof(buf), "Starting %s", desktop->name);
//	ins->handle = e_busywin_push(busywin, buf, NULL);
	ins->handle = e_busycover_push(busycover, buf, NULL);
     }
   instances = eina_list_append(instances, ins);
}

static void
_cb_run(void *data)
{
   Efreet_Desktop *desktop;
   
   desktop = data;
   _desktop_run(desktop);
}

static Eina_Bool
_cb_event_border_add(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   E_Event_Border_Add *ev;
   Instance *ins;
   Eina_List *l;
   
   ev = event;
   _app_add(ev->border);
   if (_have_borders())
     {
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APPS, 1);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT, 1);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV, 1);
     }
   EINA_LIST_FOREACH(instances, l, ins)
	if (!ins->border)
	  {
	     if ((ins->startup_id == ev->border->client.netwm.startup_id) ||
		 (ins->pid == ev->border->client.netwm.pid)) 
	       {
		  ins->border = ev->border;
		  if (ins->handle)
		    {
//		       e_busywin_pop(busywin, ins->handle);
		       e_busycover_pop(busycover, ins->handle);
		       ins->handle = NULL;
		    }
		  if (ins->timeout) ecore_timer_del(ins->timeout);
		  ins->timeout = NULL;
		  // FIXME; if gadget is disabled, enable it
		  return ECORE_CALLBACK_PASS_ON;
	       }
	  }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_event_border_remove(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   E_Event_Border_Remove *ev;
   Instance *ins;
   Eina_List *l;
   
   ev = event;
   _app_del(ev->border);
   if (!_have_borders())
     {
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APPS, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_NEXT, 0);
	e_slipshelf_action_enabled_set(slipshelf, E_SLIPSHELF_ACTION_APP_PREV, 0);
     }
   EINA_LIST_FOREACH(instances, l, ins)
	if (ins->border == ev->border)
	  {
	     if (ins->handle)
	       {
//		  e_busywin_pop(busywin, ins->handle);
		  e_busycover_pop(busycover, ins->handle);
		  ins->handle = NULL;
	       }
	     ins->border = NULL;
	     return ECORE_CALLBACK_PASS_ON;
	  }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_event_border_focus_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   E_Event_Border_Focus_In *ev;
   
   ev = event;
   _app_promote(ev->border);
   if (sys_con_act_close)
     sys_con_act_close->disabled = 0;
   if (sys_con_act_home)
     sys_con_act_home->disabled = 0;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_event_border_focus_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   E_Event_Border_Focus_Out *ev;
   
   ev = event;
   _app_promote_allow(ev->border);
   if (sys_con_act_close)
     sys_con_act_close->disabled = 1;
   if (sys_con_act_home)
     sys_con_act_home->disabled = 1;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_event_exe_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *ev;
   Instance *ins;
   Eina_List *l;
   
   ev = event;
   EINA_LIST_FOREACH(instances, l, ins)
     {
	if (ins->pid == ev->pid)
	  {
	     if (ins->handle)
	       {
//		  e_busywin_pop(busywin, ins->handle);
		  e_busycover_pop(busycover, ins->handle);
		  ins->handle = NULL;
	       }
	     instances = eina_list_remove_list(instances, l);
	     if (ins->timeout) ecore_timer_del(ins->timeout);
	     free(ins);
	     return ECORE_CALLBACK_PASS_ON;
	  }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_run_timeout(void *data)
{
   Instance *ins;
   
   ins = data;
   if (ins->handle)
     {
//	e_busywin_pop(busywin, ins->handle);
	e_busycover_pop(busycover, ins->handle);
	ins->handle = NULL;
     }
   if (!ins->border)
     {
	instances = eina_list_remove(instances, ins);
	free(ins);
	return ECORE_CALLBACK_CANCEL;
     }
   ins->timeout = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static int
_have_borders(void)
{
   Eina_List *l;
   E_Border *bd;
   int num = 0;
   
   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     {
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	num++;
     }
   return num;
}

static void
_cb_slipshelf_home(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   _app_home();
}

static void
_cb_slipshelf_close(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   E_Border *bd;
   
   bd = e_border_focused_get();
   if (bd)
     _app_close(bd);
   else
     {
        E_Action *a;
        
        a = e_action_find("syscon");
        if ((a) && (a->func.go)) a->func.go(NULL, NULL);
//	e_syswin_show(slipwin);
     }
}

static void
_cb_slipshelf_apps(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   if (!_have_borders()) return;
   e_slipwin_show(slipwin);
}

static void
_cb_slipshelf_keyboard(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   if (vkbd->visible) e_kbd_hide(vkbd);
   else e_kbd_show(vkbd);
}

static void
_cb_slipshelf_app_next(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   _app_next();
}

static void
_cb_slipshelf_app_prev(const void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Slipshelf_Action action __UNUSED__)
{
   _app_prev();
}

static void 
_cb_slipwin_border_select(void *data __UNUSED__, E_Slipwin *esw __UNUSED__, E_Border *bd) 
{
   if (bd) _e_mod_layout_border_show(bd);
}

static void
_cb_slipshelf_border_select(void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Border *bd)
{
   if (bd) _e_mod_layout_border_show(bd);
}

static void
_cb_slipshelf_border_home2(void *data __UNUSED__, E_Slipshelf *ess __UNUSED__, E_Border *pbd __UNUSED__)
{
   Eina_List *l;
   E_Border *bd;

   EINA_LIST_FOREACH(e_border_client_list(), l, bd)
     {
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	_e_mod_layout_border_hide(bd);
     }
}

static void
_apps_unpopulate(void)
{
   char buf[PATH_MAX];
   Evas_Object *obj;
   Eina_List *files;
   char *file;
   size_t len;

   EINA_LIST_FREE(sels, obj)
     evas_object_del(obj);

   if (bx) evas_object_del(bx);
   bx = NULL;

   if (fm) evas_object_del(fm);
   fm = NULL;

   if (sf) evas_object_del(sf);
   sf = NULL;

   len = e_user_dir_concat_static(buf, "appshadow");
   if (len + 2 >= sizeof(buf)) return;

   files = ecore_file_ls(buf);

   buf[len] = '/';
   len++;

   EINA_LIST_FREE(files, file)
     {
	if (eina_strlcpy(buf + len, file, sizeof(buf) - len) >= sizeof(buf) - len)
	  continue;
	ecore_file_unlink(buf);
	free(file);
     }
}

static void
_apps_fm_config(Evas_Object *o)
{
   E_Fm2_Config fmc;
                                  
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_GRID_ICONS;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 0;
   fmc.view.single_click = illume_cfg->launcher.single_click;
   fmc.view.single_click_delay = illume_cfg->launcher.single_click_delay;
   fmc.view.no_subdir_jump = 1;
   fmc.icon.extension.show = 0;
   fmc.icon.icon.w = illume_cfg->launcher.icon_size * e_scale / 2.0;
   fmc.icon.icon.h = illume_cfg->launcher.icon_size * e_scale / 2.0;
   fmc.icon.fixed.w = illume_cfg->launcher.icon_size * e_scale / 2.0;
   fmc.icon.fixed.h = illume_cfg->launcher.icon_size * e_scale / 2.0;
   fmc.list.sort.no_case = 0;
   fmc.list.sort.dirs.first = 1;
   fmc.list.sort.dirs.last = 0;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set(o, &fmc);
   e_fm2_custom_theme_content_set(o, "desktop");
}

static void
_apps_populate(void)
{
   Evas_Coord sfw, sfh;
   Evas_Object *o = NULL;
   char buf[PATH_MAX];
   int num = 0;
   
   sf = e_scrollframe_add(evas);
   e_scrollframe_single_dir_set(sf, 1);
   evas_object_move(sf, zone->x, zone->y);
   evas_object_resize(sf, zone->w, zone->h);
   evas_object_show(sf);

   e_scrollframe_custom_theme_set(sf, "base/theme/fileman",
				  "e/modules/illume/launcher/scrollview");
   if (illume_cfg->launcher.mode == 0)
     {
	bx = e_box_add(evas);
	e_box_orientation_set(bx, 0);
	e_box_homogenous_set(bx, 1);
	e_box_freeze(bx);
	e_scrollframe_child_set(sf, bx);
     }
   else
     {
	e_user_dir_concat_static(buf, "appshadow");
	ecore_file_mkpath(buf);
	fm = e_fm2_add(evas);
	_apps_fm_config(fm);
	e_scrollframe_extern_pan_set(sf, fm,
				     _e_illume_pan_set,
				     _e_illume_pan_get,
				     _e_illume_pan_max_get,
				     _e_illume_pan_child_size_get);
     }
   
   e_scrollframe_child_viewport_size_get(sf, &sfw, &sfh);
   
     {
	// TODO: Needs some efreet love
	Efreet_Menu *menu, *entry, *subentry;
	char *label, *icon, *plabel;
	Eina_List *settings_desktops, *system_desktops, *keyboard_desktops;
	Eina_List *l, *ll;
	
	settings_desktops = efreet_util_desktop_category_list("Settings");
	system_desktops = efreet_util_desktop_category_list("System");
	keyboard_desktops = efreet_util_desktop_category_list("Keyboard");
	menu = efreet_menu_get();
	if (menu)
	  {
	     EINA_LIST_FOREACH(menu->entries, l, entry)
	       {
		  if (entry->type != EFREET_MENU_ENTRY_MENU) continue;
		  
		  plabel = NULL;
		  
		  if (entry->name) plabel = strdup(entry->name);
		  if (!plabel) plabel = strdup("???");
		  
		  if (illume_cfg->launcher.mode == 0)
		    {
		       o = e_slidesel_add(evas);
		       e_slidesel_item_distance_set(o, 128);
		    }
		  
		  EINA_LIST_FOREACH(entry->entries, ll, subentry)
		    {
		       Efreet_Desktop *desktop;
		       if (subentry->type != EFREET_MENU_ENTRY_DESKTOP) continue;

		       label = icon = NULL;
		       desktop = subentry->desktop;
		       
		       if (!desktop) continue;
		       
		       if ((settings_desktops) && (system_desktops) &&
			   (eina_list_data_find(settings_desktops, desktop)) &&
			   (eina_list_data_find(system_desktops, desktop))) continue;
		       if ((keyboard_desktops) &&
			   (eina_list_data_find(keyboard_desktops, desktop))) continue;
		       
		       if ((desktop) && (desktop->x))
			 {
			    icon = eina_hash_find(desktop->x, "X-Application-Screenshot");
			    if (icon) icon = strdup(icon);
			 }
		       if ((!icon) && (subentry->icon))
			 {
			    if (subentry->icon[0] == '/')
			      icon = strdup(subentry->icon);
			    else
			      icon = efreet_icon_path_find(e_config->icon_theme,
							   subentry->icon, 512);
			 }
		       if (subentry->name) label = strdup(subentry->name);
		       if (desktop)
			 {
			    if (!label)
			      label = strdup(desktop->generic_name);
			    
			    if ((!icon) && (desktop->icon))
			      icon = efreet_icon_path_find(e_config->icon_theme,
							   desktop->icon, 512);
			 }
		  
		       if (!icon) icon = efreet_icon_path_find(e_config->icon_theme,
							  "hires.jpg", 512);
		       if (!icon) icon = strdup("DEFAULT");
		       if (!label) label = strdup("???");
		       
		       snprintf(buf, sizeof(buf), "%s / %s", plabel, label);
		       desks = eina_list_append(desks, desktop);
		       efreet_desktop_ref(desktop);
		       if (illume_cfg->launcher.mode == 0)
			 {
			    e_slidesel_item_add(o, buf, icon, _cb_run, desktop);
			 }
		       else
			 {
			    if (desktop)
			      {
				 e_user_dir_snprintf(buf, sizeof(buf), "appshadow/%04x.desktop", num);
				 ecore_file_symlink(desktop->orig_path, buf);
			      }
			    num++;
			 }
		       if (label) free(label);
		       if (icon) free(icon);
		       
		    }
		  if (plabel) free(plabel);
		  if (illume_cfg->launcher.mode == 0)
		    {
		       e_box_pack_end(bx, o);
//		       e_box_pack_options_set(o, 1, 1, 0, 0, 0.5, 0.5, 
//					      420, 215, 420, 215);
		       e_box_pack_options_set(o, 1, 1, 0, 0, 0.5, 0.5, 
					      sfw, illume_cfg->launcher.icon_size * e_scale, 
					      sfw, illume_cfg->launcher.icon_size * e_scale);
		       evas_object_show(o);
		       sels = eina_list_append(sels, o);
		    }
	       }
	  }
     }
   
   if (illume_cfg->launcher.mode == 0)
     {
	e_box_thaw(bx);
     }
   
   _cb_resize();
   
   if (illume_cfg->launcher.mode == 0)
     {
//	e_scrollframe_child_set(sf, bx);
	evas_object_show(bx);
     }
   else
     {
	/* FIXME: hacked to copy fileman - make illume themed */
////	e_scrollframe_custom_theme_set(sf, "base/theme/fileman",
////				       "e/fileman/desktop/scrollframe");
//	e_scrollframe_extern_pan_set(sf, fm,
//				     _e_illume_pan_set,
//				     _e_illume_pan_get,
//				     _e_illume_pan_max_get,
//				     _e_illume_pan_child_size_get);
	e_user_dir_concat_static(buf, "appshadow");
	e_fm2_path_set(fm, NULL, buf);
	evas_object_show(fm);
	evas_object_smart_callback_add(fm, "selected",
				       _cb_selected, NULL);
     }
}

static void
_cb_selected(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Eina_List *selected;
   E_Fm2_Icon_Info *ici;
   
   selected = e_fm2_selected_list_get(obj);
   if (!selected) return;
   EINA_LIST_FREE(selected, ici)
     {
	Efreet_Desktop *desktop;
	
	desktop = efreet_desktop_get(ici->real_link);
	if (desktop)
	  {
	     _desktop_run(desktop);
	     efreet_desktop_free(desktop);
	  }
     }
}

static Eina_Bool
_cb_efreet_cache_update(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Efreet_Desktop *desktop;

   EINA_LIST_FREE(desks, desktop)
     efreet_desktop_free(desktop);

   if (defer) ecore_timer_del(defer);
   defer = ecore_timer_add(1.0, _cb_update_deferred, NULL);
   return 1;
}

static Eina_Bool
_cb_update_deferred(void *data __UNUSED__)
{
   _apps_unpopulate();
   _apps_populate();
   e_mod_win_cfg_kbd_update();
   e_pwr_init_done();
   defer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_cb_sys_con_close(void *data __UNUSED__)
{
   E_Border *bd;
   
   bd = e_border_focused_get();
   if (bd) _app_close(bd);
}

static void
_cb_sys_con_home(void *data __UNUSED__)
{
   _app_home();
}


static void
_e_illume_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
//   E_Fwin *fwin;
//   
//   fwin = evas_object_data_get(obj, "fwin");
   e_fm2_pan_set(obj, x, y);
//   if (x > fwin->fm_pan.max_x) x = fwin->fm_pan.max_x;
//   if (y > fwin->fm_pan.max_y) y = fwin->fm_pan.max_y;
//   if (x < 0) x = 0;
//   if (y < 0) y = 0;
//   fwin->fm_pan.x = x;
//   fwin->fm_pan.y = y;
//   _e_illume_pan_scroll_update(fwin);
}
     
static void
_e_illume_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
//   E_Fwin *fwin;
//   
//   fwin = evas_object_data_get(obj, "fwin");
   e_fm2_pan_get(obj, x, y);
//   fwin->fm_pan.x = *x;
//   fwin->fm_pan.y = *y;
}
   
static void
_e_illume_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
//   E_Fwin *fwin;
//   
//   fwin = evas_object_data_get(obj, "fwin");
   e_fm2_pan_max_get(obj, x, y);
//   fwin->fm_pan.max_x = *x;
//   fwin->fm_pan.max_y = *y;
//   _e_illume_pan_scroll_update(fwin);
}

static void
_e_illume_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
//   E_Fwin *fwin;
   
//   fwin = evas_object_data_get(obj, "fwin");
   e_fm2_pan_child_size_get(obj, w, h);
//   fwin->fm_pan.w = *w;
//   fwin->fm_pan.h = *h;
//   _e_illume_pan_scroll_update(fwin);
}

/*
static void
_e_illume_pan_scroll_update(void)
{
   Edje_Message_Int_Set *msg;
   
   if ((fwin->fm_pan.x == fwin->fm_pan_last.x) &&
       (fwin->fm_pan.y == fwin->fm_pan_last.y) &&
       (fwin->fm_pan.max_x == fwin->fm_pan_last.max_x) &&
       (fwin->fm_pan.max_y == fwin->fm_pan_last.max_y) &&
       (fwin->fm_pan.w == fwin->fm_pan_last.w) &&
       (fwin->fm_pan.h == fwin->fm_pan_last.h)) return;
   msg = alloca(sizeof(Edje_Message_Int_Set) -
		sizeof(int) + (6 * sizeof(int)));
   msg->count = 6;
   msg->val[0] = fwin->fm_pan.x;
   msg->val[1] = fwin->fm_pan.y;
   msg->val[2] = fwin->fm_pan.max_x;
   msg->val[3] = fwin->fm_pan.max_y;
   msg->val[4] = fwin->fm_pan.w;
   msg->val[5] = fwin->fm_pan.h;
   if (fwin->under_obj)
     edje_object_message_send(fwin->under_obj, EDJE_MESSAGE_INT_SET, 1, msg);
   if (fwin->over_obj)
     edje_object_message_send(fwin->over_obj, EDJE_MESSAGE_INT_SET, 1, msg);
   if (fwin->scrollframe_obj)
     edje_object_message_send(e_scrollframe_edje_object_get(fwin->scrollframe_obj), EDJE_MESSAGE_INT_SET, 1, msg);
   fwin->fm_pan_last.x = fwin->fm_pan.x;
   fwin->fm_pan_last.y = fwin->fm_pan.y;
   fwin->fm_pan_last.max_x = fwin->fm_pan.max_x;
   fwin->fm_pan_last.max_y = fwin->fm_pan.max_y;
   fwin->fm_pan_last.w = fwin->fm_pan.w;
   fwin->fm_pan_last.h = fwin->fm_pan.h;
}
*/
