#include "e.h"

static void _e_configure_gui_hide(E_Win *win);
static void _e_configure_apps_load(E_Configure *app);
static void _e_configure_apps_unload(E_Configure *app);
static void _e_configure_free(E_Configure *app);
static void _e_configure_apps_click(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_configure_cb_resize(E_Win *win);
    

E_Configure *
e_configure_show(E_Container *con)
{
   E_Configure *app;
   E_Manager *man;
   Evas_Coord w, h, ew, eh;   
   
   if (!con)
   {
      man = e_manager_current_get();
      if (!man) return NULL;
      con = e_container_current_get(man);
      if (!con) con = e_container_number_get(man, 0);
      if (!con) return NULL;
   }
   app = E_OBJECT_ALLOC(E_Configure, E_CONFIGURE_TYPE, _e_configure_free);
   if (!app) return NULL;
   app->win = e_win_new(con);   
   if (!app->win)
   {
      free(app);
      return NULL;
   }
            
   app->con = con;
   
   e_win_delete_callback_set(app->win, _e_configure_gui_hide);
   e_win_title_set(app->win, "Enlightenment Configuration");
   e_win_name_class_set(app->win, "EConfigure", "EConfigure");
   app->evas = e_win_evas_get(app->win);
   ecore_evas_data_set(app->win->ecore_evas, "App", app);		
   e_win_resize_callback_set(app->win, _e_configure_cb_resize);   
   e_win_centered_set(app->win, 1);
      
   app->edje = edje_object_add(app->evas);

   e_theme_edje_object_set(app->edje, "base/theme/configure", "widgets/configure/main");

   app->box = e_box_add(app->evas);
	
   e_box_homogenous_set (app->box, 0);
	
   app->apps = e_app_new(PACKAGE_DATA_DIR "/config-apps", 0);
   app->icons = NULL;
   app->app_ref = NULL;
   _e_configure_apps_load(app);	

   e_box_orientation_set(app->box, 1);
   e_box_align_set(app->box, 0.0, 0.0);	

   edje_object_size_min_calc (app->edje, &ew, &eh);
   e_box_min_size_get(app->box, &w, &h);
	
   if (ew > w )
      w = ew;
   if (eh > h)
      h = eh;
	
   edje_extern_object_min_size_set(app->box, w, h);
   e_win_size_base_set (app->win, w , h + eh);
   e_win_size_min_set (app->win, w, h + eh);
   e_win_resize(app->win, w, h + eh);
		
   edje_object_part_swallow(app->edje, "icon_swallow", app->box);
   evas_object_show(app->box);
   
   evas_object_show(app->edje);
		
   e_win_show(app->win);   
   
   
   return app;
}

static void
_e_configure_cb_resize(E_Win *win)
{
   Evas_Coord w, h;
   E_Configure *app;

   if (win)
     {
	ecore_evas_geometry_get(win->ecore_evas, NULL, NULL, &w, &h);

	app = (E_Configure *) ecore_evas_data_get(win->ecore_evas, "App");
	evas_object_resize(app->edje, w, h);
     }
}



static void
_e_configure_gui_hide(E_Win *win)
{
   E_Configure *app;

   app = (E_Configure *) ecore_evas_data_get(win->ecore_evas, "App");
   if (app)
     {
	_e_configure_apps_unload(app);

	edje_object_part_unswallow(app->edje, app->box);
	evas_object_del(app->box);
	evas_object_del(app->edje);
	e_object_del(E_OBJECT(app->win));
     }
}

static void
_e_configure_apps_load(E_Configure *app)
{
   E_App     *a;
   Evas_List *l;
   Evas_Object *o, *icon;
   Evas_Coord w, h;

   if (!app->apps) return;
   e_app_subdir_scan(app->apps, 0);
   for (l = app->apps->subapps; l; l = l->next)
     {
	a = l->data;
	e_object_ref(E_OBJECT(a));
	app->app_ref = evas_list_append(app->app_ref, a);
	
	o = edje_object_add(app->evas);
	e_theme_edje_object_set(o, "base/theme/configure", "widgets/configure/icon");
	icon = edje_object_add(app->evas);
	edje_object_file_set(icon, a->path, "icon");
	     	     	     
	edje_extern_object_min_size_set (icon, 64, 64);	     
	     
	edje_object_part_swallow(o, "icon_swallow", icon);	     
	edje_object_part_text_set(o, "title", a->name);
	evas_object_show(icon);

	edje_object_size_min_calc (o, &w, &h);	     
	edje_extern_object_min_size_set (o, w, h);
	e_box_pack_end(app->box, o);
	e_box_pack_options_set(o,
			       1, 0, /* fill */
			       1, 0, /* expand */
			       0.0, 0.0, /* align */
			       w, h, /* min */
			       999, 999 //172, 72 /* max */
			       );

	evas_object_show(o);
	app->icons = evas_list_append(app->icons, o);
	edje_object_signal_callback_add(o, "clicked", "",
					_e_configure_apps_click, a);
     }
}

static void
_e_configure_apps_unload(E_Configure *app)
{
   E_App       *a;

/*   while(app->icons)
     {
	icon = evas_list_data(app->icons);
// FIXME unswallow icon and free

	app->icons = evas_list_remove(app->icons, icon);
	evas_object_del(icon);
     }*/
   while (app->app_ref)
     {
	a = evas_list_data(app->app_ref);

	app->app_ref = evas_list_remove(app->app_ref, a);
	e_object_unref(E_OBJECT(a));
     }
}

static void
_e_configure_apps_click(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   E_App *a;

   a = data;
   if (a)
     {
	Ecore_Exe *exe;

	exe = ecore_exe_run(a->exe, NULL);
	if (exe) ecore_exe_free(exe);
     }
}

static void
_e_configure_free(E_Configure *app)
{
   _e_configure_apps_unload(app);
   e_object_del(E_OBJECT(app->win));
   free(app);
}
