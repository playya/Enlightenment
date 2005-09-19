/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "config.h"

typedef struct _Main_Data Main_Data;

struct _Main_Data
{
   E_Menu *menu;
   E_Menu *apps;
   E_Menu *desktops;
   E_Menu *clients;
   E_Menu *modules;
   E_Menu *gadgets;
   E_Menu *themes;
   E_Menu *config;
   E_Menu *lost_clients;
};

/* local subsystem functions */
static void _e_int_menus_quit                (void);
static void _e_int_menus_quit_cb             (void *data);
static void _e_int_menus_main_del_hook       (void *obj);
static void _e_int_menus_main_about          (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_main_run            (void *data, E_Menu *m, E_Menu_Item*mi);
static void _e_int_menus_main_restart        (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_main_exit           (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_apps_scan           (E_Menu *m);
static void _e_int_menus_apps_start          (void *data, E_Menu *m);
static void _e_int_menus_apps_del_hook       (void *obj);
static void _e_int_menus_apps_free_hook      (void *obj);
static void _e_int_menus_apps_run            (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_config_pre_cb       (void *data, E_Menu *m);
static void _e_int_menus_config_free_hook    (void *obj);
static void _e_int_menus_clients_pre_cb      (void *data, E_Menu *m);
static void _e_int_menus_clients_free_hook   (void *obj);
static void _e_int_menus_clients_item_cb     (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_clients_cleanup_cb  (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_desktops_pre_cb     (void *data, E_Menu *m);
static void _e_int_menus_desktops_item_cb    (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_desktops_row_add_cb (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_desktops_row_del_cb (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_desktops_col_add_cb (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_desktops_col_del_cb (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_gadgets_pre_cb      (void *data, E_Menu *m);
static void _e_int_menus_gadgets_edit_mode_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_themes_pre_cb       (void *data, E_Menu *m);
static void _e_int_menus_themes_edit_mode_cb (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_lost_clients_pre_cb      (void *data, E_Menu *m);
static void _e_int_menus_lost_clients_free_hook   (void *obj);
static void _e_int_menus_lost_clients_item_cb     (void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_int_menus_augmentation_add    (E_Menu *m, Evas_List *augmentation);
static void _e_int_menus_augmentation_del    (E_Menu *m, Evas_List *augmentation);

/* local subsystem globals */
static Ecore_Job *_e_int_menus_quit_job = NULL;

static Evas_Hash *_e_int_menus_augmentation = NULL;

/* externally accessible functions */
E_Menu *
e_int_menus_main_new(void)
{
   E_Menu *m, *subm;
   E_Menu_Item *mi;
   Main_Data *dat;
   
   dat = calloc(1, sizeof(Main_Data));
   m = e_menu_new();
   dat->menu = m;
   e_object_data_set(E_OBJECT(m), dat);   
   e_object_del_attach_func_set(E_OBJECT(m), _e_int_menus_main_del_hook);
   
   subm = e_int_menus_favorite_apps_new();
   dat->apps = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Favorite Applications"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/favorites");
   e_menu_item_submenu_set(mi, subm);
  
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   
   subm = e_module_menu_new();
   dat->modules = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Modules"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/modules");
   e_menu_item_submenu_set(mi, subm);

   subm = e_int_menus_desktops_new();
   dat->desktops = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Desktops"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/desktops");
   e_menu_item_submenu_set(mi, subm);
  
   subm = e_int_menus_clients_new();
   dat->clients = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Windows"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/windows");
   e_menu_item_submenu_set(mi, subm);
  
   subm = e_int_menus_lost_clients_new();
   dat->lost_clients = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Lost Windows"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/lost_windows");
   e_menu_item_submenu_set(mi, subm);
  
   subm = e_int_menus_gadgets_new();
   dat->gadgets = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Gadgets"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/gadgets");
   e_menu_item_submenu_set(mi, subm);
   
   subm = e_int_menus_themes_new();
   dat->themes = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Themes"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/themes");
   e_menu_item_submenu_set(mi, subm);   
  
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("About Enlightenment"));   
   e_util_menu_item_edje_icon_set(mi, "enlightenment/e");
   e_menu_item_callback_set(mi, _e_int_menus_main_about, NULL);

   if (ecore_file_app_installed("exige"))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Run Command"));
	e_util_menu_item_edje_icon_set(mi, "enlightenment/run");
	e_menu_item_callback_set(mi, _e_int_menus_main_run, NULL);
     }

   subm = e_int_menus_config_new();
   dat->config = subm;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Configuration"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
   e_menu_item_submenu_set(mi, subm);

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Restart Enlightenment"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/reset");
   e_menu_item_callback_set(mi, _e_int_menus_main_restart, NULL);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Exit Enlightenment"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/exit");
   e_menu_item_callback_set(mi, _e_int_menus_main_exit, NULL);
   return m;
}

E_Menu *
e_int_menus_apps_new(char *dir)
{
   E_Menu *m;
   E_App *a;
   
   m = e_menu_new();
   a = e_app_new(dir, 0);
   e_object_data_set(E_OBJECT(m), a);
   e_menu_pre_activate_callback_set(m, _e_int_menus_apps_start, NULL);
   e_object_del_attach_func_set(E_OBJECT(m), _e_int_menus_apps_del_hook);
   e_object_free_attach_func_set(E_OBJECT(m), _e_int_menus_apps_free_hook);
   return m;
}

E_Menu *
e_int_menus_desktops_new(void)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_desktops_pre_cb, NULL);
   return m;
}

E_Menu *
e_int_menus_favorite_apps_new(void)
{
   E_Menu *m;
   char buf[4096];
   char *homedir;
   
   homedir = e_user_homedir_get();
   if (homedir)
     {
	snprintf(buf, sizeof(buf), "%s/.e/e/applications/favorite", homedir);
	m = e_int_menus_apps_new(buf);
	free(homedir);
	return m;
     }
   return NULL;
}

E_Menu *
e_int_menus_config_new(void)
{
   E_Menu *m;
   
   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_config_pre_cb, NULL);

   return m;
}

E_Menu *
e_int_menus_clients_new(void)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_clients_pre_cb, NULL);
   return m;
}

E_Menu *
e_int_menus_gadgets_new(void)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_gadgets_pre_cb, NULL);
   return m;
}

E_Menu *
e_int_menus_themes_new(void)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_themes_pre_cb, NULL);
   return m;
}

E_Menu *
e_int_menus_lost_clients_new(void)
{
   E_Menu *m;

   m = e_menu_new();
   e_menu_pre_activate_callback_set(m, _e_int_menus_lost_clients_pre_cb, NULL);
   return m;
}

E_Int_Menu_Augmentation *
e_int_menus_menu_augmentation_add(const char *menu,
				  void (*func_add) (void *data, E_Menu *m),
				  void *data_add,
				  void (*func_del) (void *data, E_Menu *m),
				  void *data_del)
{
   E_Int_Menu_Augmentation *maug;
   Evas_List *l;

   maug = E_NEW(E_Int_Menu_Augmentation, 1);
   if (!maug) return NULL;

   maug->add.func = func_add;
   maug->add.data = data_add;

   maug->del.func = func_del;
   maug->del.data = data_del;

   l = evas_hash_find(_e_int_menus_augmentation, menu);
   l = evas_list_append(l, maug);
   _e_int_menus_augmentation = evas_hash_add(_e_int_menus_augmentation, menu, l);

   return maug;
}

void
e_int_menus_menu_augmentation_del(const char *menu, E_Int_Menu_Augmentation *maug)
{
   Evas_List *l;

   l = evas_hash_find(_e_int_menus_augmentation, menu);
   if (l)
     {
	/*
	 * We should always add the list to the hash, in case the list
	 * becomes empty, or the first element is removed.
	 */
	_e_int_menus_augmentation = evas_hash_del(_e_int_menus_augmentation, menu, l);

	l = evas_list_remove(l, maug);
	if (l)
	  _e_int_menus_augmentation = evas_hash_add(_e_int_menus_augmentation, menu, l);
     }
   free(maug);
}

/* local subsystem functions */
static void
_e_int_menus_quit(void)
{
   if (_e_int_menus_quit_job)
     {
	ecore_job_del(_e_int_menus_quit_job);
	_e_int_menus_quit_job = NULL;
     }
   _e_int_menus_quit_job = ecore_job_add(_e_int_menus_quit_cb, NULL);
}

static void
_e_int_menus_quit_cb(void *data)
{
   E_Action *a;
   
   a = e_action_find("exit");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
   _e_int_menus_quit_job = NULL;
}

static void
_e_int_menus_main_del_hook(void *obj)
{
   Main_Data *dat;
   E_Menu *m;
   
   m = obj;
   dat = e_object_data_get(E_OBJECT(obj));
   if (dat)
     {
	e_object_del(E_OBJECT(dat->apps));
	e_object_del(E_OBJECT(dat->modules));
	e_object_del(E_OBJECT(dat->desktops));
	e_object_del(E_OBJECT(dat->clients));
	e_object_del(E_OBJECT(dat->gadgets));
	e_object_del(E_OBJECT(dat->themes));	
	e_object_del(E_OBJECT(dat->config));
	e_object_del(E_OBJECT(dat->lost_clients));
	free(dat);
     }
}

static void
_e_int_menus_main_about(void *data, E_Menu *m, E_Menu_Item *mi)
{
   e_error_dialog_show(_("About Enlightenment"),
		       _("This is Enlightenment %s.\n"
			 "Copyright © 1999-2005, by the Enlightenment Dev Team.\n"
			 "\n"
			 "We hope you enjoy using this software as much as we enjoyed writing it.\n\n"
			 "Please think of the aardvarks. They need some love too."),
		       VERSION
		       );
}

static void
_e_int_menus_main_run(void *data, E_Menu *m, E_Menu_Item *mi)
{
   Ecore_Exe *exe;

   exe = ecore_exe_run("exige", NULL);
   if (exe) ecore_exe_free(exe);
}

static void
_e_int_menus_main_restart(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Action *a;
   
   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
}

static void
_e_int_menus_main_exit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (!e_util_immortal_check()) _e_int_menus_quit();
}

static void
_e_int_menus_apps_scan(E_Menu *m)
{
   E_Menu_Item *mi;
   E_App *a;
   Evas_List *l;
   int app_count = 0;
   
   a = e_object_data_get(E_OBJECT(m));
   if (a)
     {
	e_app_subdir_scan(a, 0);
	for (l = a->subapps; l; l = l->next)
	  {
	     a = l->data;
	     
	     mi = e_menu_item_new(m);
	     e_menu_item_label_set(mi, a->name);
	     if (a->exe)
	       {
		  e_menu_item_icon_edje_set(mi, a->path, "icon");
		  e_menu_item_callback_set(mi, _e_int_menus_apps_run, a);
		  app_count++;
	       }
	     else
	       {
		  char buf[4096];
		  
		  snprintf(buf, sizeof(buf), "%s/.directory.eap", a->path);
		  e_menu_item_icon_edje_set(mi, buf, "icon");
		  e_menu_item_submenu_set(mi, e_int_menus_apps_new(a->path));
		  app_count++;
	       }
	  }
     }
   if (app_count == 0)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("(No Applications)"));
     }
}

static void
_e_int_menus_apps_start(void *data, E_Menu *m)
{
   _e_int_menus_apps_scan(m);
   e_menu_pre_activate_callback_set(m, NULL, NULL);
}

static void
_e_int_menus_apps_del_hook(void *obj)
{
   E_Menu *m;
   Evas_List *l;
   
   m = obj;
   for (l = m->items; l; l = l->next)
     {
	E_Menu_Item *mi;
	
	mi = l->data;
	if (mi->submenu) e_object_del(E_OBJECT(mi->submenu));
     }
}

static void
_e_int_menus_apps_free_hook(void *obj)
{
   E_Menu *m;
   E_App *a;
   
   m = obj;
   a = e_object_data_get(E_OBJECT(m));
   /* note: app objects are shared so we ALWAYS unref not del! */
   if (a) e_object_unref(E_OBJECT(a));
}

static void
_e_int_menus_apps_run(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_App *a;
   
   a = data;
   e_zone_app_exec(m->zone, a);
}

static void
_e_int_menus_desktops_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   E_Menu *root;

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   root = e_menu_root_get(m);
   /* Get the desktop list for this zone */
   /* FIXME: Menu code needs to determine what zone menu was clicked in */
   if ((root) && (root->zone))
     {
	int i;
	E_Zone *zone;
	
	zone = root->zone;
	for (i = 0; i < zone->desk_x_count * zone->desk_y_count; i++)
	  {
	     E_Desk *desk = zone->desks[i];
	
	     mi = e_menu_item_new(m);
	     e_menu_item_radio_group_set(mi, 1);
	     e_menu_item_radio_set(mi, 1);
	     e_menu_item_label_set(mi, desk->name);
	     if (desk == e_desk_current_get(zone))
	       e_menu_item_toggle_set(mi, 1);
	     e_menu_item_callback_set(mi, _e_int_menus_desktops_item_cb, desk);
	  }
     }

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("New Row of Desktops"));
   e_menu_item_callback_set(mi, _e_int_menus_desktops_row_add_cb, NULL);
	       
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Remove Row of Desktops"));
   e_menu_item_callback_set(mi, _e_int_menus_desktops_row_del_cb, NULL);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("New Column of Desktops"));
   e_menu_item_callback_set(mi, _e_int_menus_desktops_col_add_cb, NULL);
	       
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Remove Column of Desktops"));
   e_menu_item_callback_set(mi, _e_int_menus_desktops_col_del_cb, NULL);
}

/* FIXME: Use the zone the menu was clicked in */
static void
_e_int_menus_desktops_row_add_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Menu *root;
     
   root = e_menu_root_get(m);
   if (root)
     e_desk_row_add(root->zone);
}

static void
_e_int_menus_desktops_row_del_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Menu *root;
     
   root = e_menu_root_get(m);
   if (root)
     e_desk_row_remove(root->zone);
}

static void
_e_int_menus_desktops_col_add_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Menu *root;
     
   root = e_menu_root_get(m);
   if (root)
     e_desk_col_add(root->zone);
}

static void
_e_int_menus_desktops_col_del_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Menu *root;
     
   root = e_menu_root_get(m);
   if (root)
     e_desk_col_remove(root->zone);
}

static void
_e_int_menus_desktops_item_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Desk *desk = data;

   e_desk_show(desk);
}

static void
_e_int_menus_config_pre_cb(void *data, E_Menu *m)
{
   E_Menu *subm;
   E_Menu_Item *mi;
   Evas_List *l;
   char buf[4096];

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   
   snprintf(buf, sizeof(buf), "%s/config-apps", e_prefix_data_get());
   subm = e_int_menus_apps_new(buf);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Applications"));
   e_menu_item_submenu_set(mi, subm);

   l = evas_hash_find(_e_int_menus_augmentation, "config");
   if (l)
     {
	mi = e_menu_item_new(m);
	e_menu_item_separator_set(mi, 1);

	_e_int_menus_augmentation_add(m, l);
     }

   e_object_free_attach_func_set(E_OBJECT(m), _e_int_menus_config_free_hook);
   e_object_data_set(E_OBJECT(m), subm);
}

static void
_e_int_menus_config_free_hook(void *obj)
{
   E_Menu *m, *subm;

   m = obj;
   subm = e_object_data_get(E_OBJECT(obj));
   if (subm)
     e_object_del(E_OBJECT(subm));

   _e_int_menus_augmentation_del(m, evas_hash_find(_e_int_menus_augmentation, "config"));
}

static void
_e_int_menus_clients_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   Evas_List *l, *borders = NULL;
   E_Menu *root;
   E_Zone *zone = NULL;
   char *s;

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   root = e_menu_root_get(m);
   /* get the current clients */
   if (root)
     zone = root->zone;
   for (l = e_border_clients_get(); l; l = l->next)
     {
	E_Border *border;

	border = l->data;
	if ((border->zone == zone) || (border->iconic))
	  borders = evas_list_append(borders, border);
     }

   if (!borders)
     { 
	/* FIXME here we want nothing, but that crashes!!! */
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("(No Windows)"));
	return;
     }
   for (l = borders; l; l = l->next)
     {
	E_Border *bd = l->data;
	E_App *a;
	const char *title;
	
	title = e_border_name_get(bd);
	mi = e_menu_item_new(m);
	e_menu_item_check_set(mi, 1);
	if ((title) && (title[0]))
	  e_menu_item_label_set(mi, title);
	else
	  e_menu_item_label_set(mi, _("No name!!"));
	/* ref the border as we implicitly unref it in the callback */
	e_object_ref(E_OBJECT(bd));
//	e_object_breadcrumb_add(E_OBJECT(bd), "clients_menu");
	e_menu_item_callback_set(mi, _e_int_menus_clients_item_cb, bd);
	if (!bd->iconic) e_menu_item_toggle_set(mi, 1);
	a = e_app_window_name_class_title_role_find(bd->client.icccm.name,
						    bd->client.icccm.class,
						    title,
						    bd->client.icccm.window_role);
	if (a) e_menu_item_icon_edje_set(mi, a->path, "icon");
     }
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Cleanup Windows"));
   s = e_path_find(path_icons, "default.edj");
   e_menu_item_icon_edje_set(mi, s, "windows");
   E_FREE(s);
   e_menu_item_callback_set(mi, _e_int_menus_clients_cleanup_cb, zone);
   
   e_object_free_attach_func_set(E_OBJECT(m), _e_int_menus_clients_free_hook);
   e_object_data_set(E_OBJECT(m), borders);
}

static void
_e_int_menus_clients_free_hook(void *obj)
{
   E_Menu *m;
   Evas_List *borders;
   
   m = obj;
   borders = e_object_data_get(E_OBJECT(m));
   while (borders)
     {
	E_Border *bd;
	
	bd = borders->data;
	borders = evas_list_remove_list(borders, borders);
//	e_object_breadcrumb_del(E_OBJECT(bd), "clients_menu");
	e_object_unref(E_OBJECT(bd));
     }
}

static void 
_e_int_menus_clients_item_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   
   bd = data;
   E_OBJECT_CHECK(bd);
   if (bd->iconic)
     {
	if (!bd->lock_user_iconify)
	  e_border_uniconify(bd);
     }
   e_desk_show(bd->desk);
   if (!bd->lock_user_stacking)
     e_border_raise(bd);
   if (!bd->lock_focus_out)
     e_border_focus_set(bd, 1, 1);
}

static void 
_e_int_menus_clients_cleanup_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Zone *zone;

   zone = data;
   e_place_zone_region_smart_cleanup(zone);
}

static void
_e_int_menus_gadgets_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   E_Menu *root;

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   root = e_menu_root_get(m);
   if ((root) && (root->zone))
     {
	mi = e_menu_item_new(m);
	  e_menu_item_check_set(mi, 1);
	if (e_gadman_mode_get(root->zone->container->gadman) == E_GADMAN_MODE_EDIT)
	  e_menu_item_toggle_set(mi, 1);
	else
	  e_menu_item_toggle_set(mi, 0);
	e_menu_item_label_set(mi, _("Edit Mode"));
	e_menu_item_callback_set(mi, _e_int_menus_gadgets_edit_mode_cb, root->zone->container->gadman);
     }
   else
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("(Unused)"));
	e_menu_item_callback_set(mi, NULL, NULL);
     }
}

static void
_e_int_menus_gadgets_edit_mode_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadman *gm;
   
   gm = data;
   if (e_menu_item_toggle_get(mi))
     e_gadman_mode_set(gm, E_GADMAN_MODE_EDIT);
   else
     e_gadman_mode_set(gm, E_GADMAN_MODE_NORMAL);
}

/* FIXME:
 * 
 * Remove this later, keep for fast theme switching now.
 */
static void
_e_int_menus_themes_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   E_Menu *root;
   int num = 0;

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   root = e_menu_root_get(m);
   if ((root) && (root->zone))
     {
	char buf[4096];
	char *homedir;
	
	homedir = e_user_homedir_get();
	if (homedir)
	  {
	     snprintf(buf, sizeof(buf), "%s/.e/e/themes", homedir);
	     free(homedir);
	  }
	
	if ((ecore_file_exists(buf)) && (ecore_file_is_dir(buf)))
	  {
	     Ecore_List *themes;
	     
	     themes = ecore_file_ls(buf);
	     if (themes)
	       {
		  char *theme, *deftheme = NULL;
		  Evas_List *l;
		  
		  for (l = e_config->themes; l; l = l->next)
		    {
		       E_Config_Theme *et;
		       
		       et = l->data;
		       if (!strcmp(et->category, "theme"))
			 deftheme = et->file;
		    }		  		  
		  
		  mi = e_menu_item_new(m);
		  e_menu_item_radio_set(mi, 1);
		  e_menu_item_radio_group_set(mi, 1);
		  if (((deftheme) && (!strcmp("default.edj", deftheme))) ||
		      (!deftheme))
		    e_menu_item_toggle_set(mi, 1);
		  e_menu_item_label_set(mi, "default");
		  e_menu_item_callback_set(mi, _e_int_menus_themes_edit_mode_cb, NULL);
		  num++;
		  
		  while ((theme = ecore_list_next(themes)))
		    {
		       char *ext;
		       
		       ext = strrchr(theme, '.');
		       if (ecore_file_is_dir(theme) || (!ext))
			 continue;
		       if (!strcasecmp(ext, ".edj"))
			 {
			    mi = e_menu_item_new(m);
			    e_menu_item_radio_set(mi, 1);
			    e_menu_item_radio_group_set(mi, 1);
			    if (deftheme)
			      {			  
				 if (!strcmp(theme, deftheme))
				   e_menu_item_toggle_set(mi, 1);
			      }
			    *ext = 0;
			    e_menu_item_label_set(mi, theme);
			    e_menu_item_callback_set(mi, _e_int_menus_themes_edit_mode_cb, NULL);
			    num++;
			 }
		    }
		  ecore_list_destroy(themes);
	       }
	  }
     }
   if (num == 0)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("(Empty)"));
     }
}

static void
_e_int_menus_themes_edit_mode_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Config_Theme *et;
   Evas_List *l;

   for (l = e_config->themes; l; l = l->next)
     {
	et = l->data;
	if (!strcmp(et->category, "theme"))
	  {
	     e_config->themes = evas_list_remove_list(e_config->themes, l);
	     E_FREE(et->category);
	     E_FREE(et->file);
	     E_FREE(et);
	     break;
	  }
     }

   /* FIXME: EVIL... relies on label + .edj being the filename! */
   et = E_NEW(E_Config_Theme, 1);
   et->category = strdup("theme");
   et->file = E_NEW(char, strlen(mi->label) + 4 + 1);
   strcpy(et->file, mi->label);
   strcat(et->file, ".edj");
   e_config->themes = evas_list_append(e_config->themes, et);
   
   e_config_save_queue();

   restart = 1;
   _e_int_menus_quit();   
}

static void
_e_int_menus_lost_clients_pre_cb(void *data, E_Menu *m)
{
   E_Menu_Item *mi;
   Evas_List *l, *borders = NULL;
   E_Menu *root;
   E_Zone *zone = NULL;

   e_menu_pre_activate_callback_set(m, NULL, NULL);
   root = e_menu_root_get(m);
   /* get the current clients */
   if (root)
     zone = root->zone;
   borders = e_border_lost_windows_get(zone);

   if (!borders)
     { 
	/* FIXME here we want nothing, but that crashes!!! */
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("(No Windows)"));
	return;
     }
   for (l = borders; l; l = l->next)
     {
	E_Border *bd = l->data;
	E_App *a;
	const char *title = "";
	
	title = e_border_name_get(bd);
	mi = e_menu_item_new(m);
	if ((title) && (title[0]))
	  e_menu_item_label_set(mi, title);
	else
	  e_menu_item_label_set(mi, _("No name!!"));
	/* ref the border as we implicitly unref it in the callback */
	e_object_ref(E_OBJECT(bd));
//	e_object_breadcrumb_add(E_OBJECT(bd), "lost_clients_menu");
	e_menu_item_callback_set(mi, _e_int_menus_lost_clients_item_cb, bd);
	a = e_app_window_name_class_title_role_find(bd->client.icccm.name,
						    bd->client.icccm.class,
						    title,
						    bd->client.icccm.window_role);
	if (a) e_menu_item_icon_edje_set(mi, a->path, "icon");
     }
   e_object_free_attach_func_set(E_OBJECT(m), _e_int_menus_lost_clients_free_hook);
   e_object_data_set(E_OBJECT(m), borders);
}

static void
_e_int_menus_lost_clients_free_hook(void *obj)
{
   E_Menu *m;
   Evas_List *borders;
   
   m = obj;
   borders = e_object_data_get(E_OBJECT(m));
   while (borders)
     {
	E_Border *bd;
	
	bd = borders->data;
	borders = evas_list_remove_list(borders, borders);
//	e_object_breadcrumb_del(E_OBJECT(bd), "lost_clients_menu");
	e_object_unref(E_OBJECT(bd));
     }
}


static void 
_e_int_menus_lost_clients_item_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd = data;

   E_OBJECT_CHECK(bd);
   if (bd->iconic) e_border_uniconify(bd);
   if (bd->desk) e_desk_show(bd->desk);
   e_border_move(bd, bd->zone->x + ((bd->zone->w - bd->w) / 2), bd->zone->y + ((bd->zone->h - bd->h) / 2));
   e_border_raise(bd);
   e_border_focus_set(bd, 1, 1);
}

static void
_e_int_menus_augmentation_add(E_Menu *m, Evas_List *augmentation)
{
   Evas_List *l;
   E_Menu_Item *mi;
   int i = 0;

   for (l = augmentation; l; l = l->next)
     {
	E_Int_Menu_Augmentation *aug;

	aug = l->data;

	if (i)
	  {
	     mi = e_menu_item_new(m);
	     e_menu_item_separator_set(mi, 1);
	  }
	else
	  i++;

	if (aug->add.func)
	  aug->add.func(aug->add.data, m);
     }
}

static void
_e_int_menus_augmentation_del(E_Menu *m, Evas_List *augmentation)
{
   Evas_List *l;

   for (l = augmentation; l; l = l->next)
     {
	E_Int_Menu_Augmentation *aug;

	aug = l->data;

	if (aug->del.func)
	  aug->del.func(aug->del.data, m);
     }
}
