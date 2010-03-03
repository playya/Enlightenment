/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_order_free       (E_Order *eo);
static void _e_order_cb_monitor (void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
static void _e_order_read       (E_Order *eo);
static void _e_order_save       (E_Order *eo);

#if 0
static int  _e_order_cb_efreet_desktop_change(void *data, int ev_type, void *ev);
#endif

static Eina_List *orders = NULL;
static Eina_List *handlers = NULL;

/* externally accessible functions */
EAPI int
e_order_init(void)
{
#if 0
   handlers = eina_list_append(handlers, ecore_event_handler_add(EFREET_EVENT_DESKTOP_CHANGE, _e_order_cb_efreet_desktop_change, NULL));
#endif
   efreet_menu_file_set(e_config->default_system_menu);
   return 1;
}

EAPI int
e_order_shutdown(void)
{
   orders = eina_list_free(orders);

   E_FREE_LIST(handlers, ecore_event_handler_del);
   return 1;
}

EAPI E_Order *
e_order_new(const char *path)
{
   E_Order *eo;
 
   eo = E_OBJECT_ALLOC(E_Order, E_ORDER_TYPE, _e_order_free);
   if (!eo) return NULL;

   if (path) eo->path = eina_stringshare_add(path);
   _e_order_read(eo);
   eo->monitor = ecore_file_monitor_add(path, _e_order_cb_monitor, eo);

   orders = eina_list_append(orders, eo);

   return eo;
}

EAPI void
e_order_update_callback_set(E_Order *eo, void (*cb)(void *data, E_Order *eo), void *data)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   eo->cb.update = cb;
   eo->cb.data = data;
}

EAPI void
e_order_remove(E_Order *eo, Efreet_Desktop *desktop)
{
   Eina_List *tmp;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   tmp = eina_list_data_find_list(eo->desktops, desktop);
   if (!tmp) return;
   efreet_desktop_free(eina_list_data_get(tmp));
   eo->desktops = eina_list_remove_list(eo->desktops, tmp);
   _e_order_save(eo);
}

EAPI void
e_order_append(E_Order *eo, Efreet_Desktop *desktop)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   efreet_desktop_ref(desktop);
   eo->desktops = eina_list_append(eo->desktops, desktop);
   _e_order_save(eo);
}

EAPI void
e_order_prepend_relative(E_Order *eo, Efreet_Desktop *desktop, Efreet_Desktop *before)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   efreet_desktop_ref(desktop);
   eo->desktops = eina_list_prepend_relative(eo->desktops, desktop, before);
   _e_order_save(eo);
}

EAPI void
e_order_files_append(E_Order *eo, Eina_List *files)
{
   Eina_List *l;
   const char *file;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   EINA_LIST_FOREACH(files, l, file)
     {
	Efreet_Desktop *desktop;

	desktop = efreet_desktop_get(file);
	if (!desktop) continue;
	eo->desktops = eina_list_append(eo->desktops, desktop);
     }
   _e_order_save(eo);
}

EAPI void
e_order_files_prepend_relative(E_Order *eo, Eina_List *files, Efreet_Desktop *before)
{
   Eina_List *l;
   const char *file;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   EINA_LIST_FOREACH(files, l, file)
     {
	Efreet_Desktop *desktop;

	desktop = efreet_desktop_get(file);
	if (!desktop) continue;
	eo->desktops = eina_list_prepend_relative(eo->desktops, desktop, before);
     }
   _e_order_save(eo);
}

EAPI void
e_order_clear(E_Order *eo)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   _e_order_save(eo);
}

/* local subsystem functions */
static void
_e_order_free(E_Order *eo)
{
   if (eo->delay) ecore_timer_del(eo->delay);
   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   if (eo->path) eina_stringshare_del(eo->path);
   if (eo->monitor) ecore_file_monitor_del(eo->monitor);
   orders = eina_list_remove(orders, eo);
   free(eo);
}

static int
_e_order_cb_monitor_delay(void *data)
{
   E_Order *eo = data;
   
   /* It doesn't really matter what the change is, just re-read the file */
   _e_order_read(eo);
   if (eo->cb.update) eo->cb.update(eo->cb.data, eo);
   eo->delay = NULL;
   return 0;
}

static void
_e_order_cb_monitor(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
   E_Order *eo = data;

   if (eo->delay) ecore_timer_del(eo->delay);
   eo->delay = ecore_timer_add(0.2, _e_order_cb_monitor_delay, eo);
}

static void
_e_order_read(E_Order *eo)
{
   FILE *f;
   char *dir;

   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   if (!eo->path) return;

   dir = ecore_file_dir_get(eo->path);
   f = fopen(eo->path, "rb");
   if (f)
     {
	char buf[4096];

	while (fgets(buf, sizeof(buf), f))
	  {
	     int len;

	     len = strlen(buf);
	     if (len > 0)
	       {
		  if (buf[len - 1] == '\n')
		    {
		       buf[len - 1] = 0;
		       len--;
		    }
		  if (len > 0)
		    {
		       Efreet_Desktop *desktop;
		       
		       desktop = NULL;
		       if (buf[0] == '/')
		         desktop = efreet_desktop_get(buf);
		       if (!desktop)
		         desktop = efreet_desktop_get(ecore_file_file_get(buf));
		       if (!desktop)
			 {
			    desktop = efreet_util_desktop_file_id_find(ecore_file_file_get(buf));
			    /* Need to ref this as we only get a cache pointer from efreet_util */
			    if (desktop) efreet_desktop_ref(desktop);
			 }
		       if (desktop) eo->desktops = eina_list_append(eo->desktops, desktop);
		    }
	       }
	  }
	fclose(f);
     }
   if (dir) free(dir);
}

static void
_e_order_save(E_Order *eo)
{
   FILE *f;
   Eina_List *l;
   Efreet_Desktop *desktop;

   if (!eo->path) return;
   f = fopen(eo->path, "wb");
   if (!f) return;

   EINA_LIST_FOREACH(eo->desktops, l, desktop)
     {
	const char *id;

	id = efreet_util_path_to_file_id(desktop->orig_path);
	if (id)
	  {
	     fprintf(f, "%s\n", id);
	  }
	else
	  {
	     fprintf(f, "%s\n", desktop->orig_path);
	  }
     }

   fclose(f);
}

#if 0
static int
_e_order_cb_efreet_desktop_change(void *data, int ev_type, void *ev)
{
   Efreet_Event_Desktop_Change *event;
   Eina_List *l;
   E_Order *eo;

   event = ev;
   switch (event->change)
     {
      case EFREET_DESKTOP_CHANGE_ADD:
	 /* If a desktop is added, reread all .order files */
	 EINA_LIST_FOREACH(orders, l, eo)
	   {
	      _e_order_read(eo);
	      if (eo->cb.update) eo->cb.update(eo->cb.data, eo);
	   }
	 break;
      case EFREET_DESKTOP_CHANGE_REMOVE:
	 /* If a desktop is removed, drop the .desktop pointer */
	 EINA_LIST_FOREACH(orders, l, eo)
	   {
	      Eina_List *l2;
	      Efreet_Desktop *desktop;
	      int changed = 0;

	      EINA_LIST_FOREACH(eo->desktops, l2, desktop)
		{
		   if (desktop == event->current)
		     {
			efreet_desktop_free(desktop);
			eo->desktops = eina_list_remove_list(eo->desktops, l2);
			changed = 1;
		     }
		}
	      if ((changed) && (eo->cb.update)) eo->cb.update(eo->cb.data, eo);
	   }
	 break;
      case EFREET_DESKTOP_CHANGE_UPDATE:
	 /* If a desktop is updated, point to the new desktop and update */
	 EINA_LIST_FOREACH(orders, l, eo)
	   {
	      Eina_List *l2;
	      Efreet_Desktop *desktop;
	      int changed = 0;

	      EINA_LIST_FOREACH(eo->desktops, l2, desktop)
		{
		   if (desktop == event->previous)
		     {
			efreet_desktop_free(desktop);
			efreet_desktop_ref(event->current);
			l2->data = event->current;
			changed = 1;
		     }
		}
	      if ((changed) && (eo->cb.update)) eo->cb.update(eo->cb.data, eo);
	   }
	 break;
     }
   return 1;
}
#endif
