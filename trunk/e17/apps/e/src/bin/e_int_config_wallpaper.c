/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "e.h"

static void        *_create_data             (E_Config_Dialog *cfd);
static void         _free_data               (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data               (E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data        (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets    (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data     (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

#define E_CONFIG_WALLPAPER_ALL 0
#define E_CONFIG_WALLPAPER_DESK 1
#define E_CONFIG_WALLPAPER_SCREEN 2

struct _E_Config_Wallpaper
{
   int specific_config;
   int con_num, zone_num;
   int desk_x, desk_y;
};

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas_Object *o_frame;
   Evas_Object *o_fm;
   Evas_Object *o_up_button;
   Evas_Object *o_preview;
   Evas_Object *o_theme_bg;
   Evas_Object *o_personal;
   Evas_Object *o_system;
   int fmdir;

   int use_theme_bg;
   char *bg;

   /* advanced */
   int all_this_desk_screen;
   /* dialogs */
   E_Win *win_import;
   E_Dialog *dia_gradient;
};

EAPI E_Config_Dialog *
e_int_config_wallpaper(E_Container *con)
{
   return e_int_config_wallpaper_desk(-1, -1, -1, -1);
}

EAPI E_Config_Dialog *
e_int_config_wallpaper_desk(int con_num, int zone_num, int desk_x, int desk_y)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Config_Wallpaper *cw;
   E_Container *con;

   if (e_config_dialog_find("E", "_config_wallpaper_dialog")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   cw = E_NEW(E_Config_Wallpaper, 1);
  
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.apply_cfdata      = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;

   if (!(con_num == -1 && zone_num == -1 && desk_x == -1 && desk_y == -1))
     cw->specific_config = 1;
   else
     {
	v->advanced.apply_cfdata   = _advanced_apply_data;
	v->advanced.create_widgets = _advanced_create_widgets;
     }

   v->override_auto_apply = 1;

   cw->con_num = con_num;
   cw->zone_num = zone_num;
   cw->desk_x = desk_x;
   cw->desk_y = desk_y;

   con = e_container_current_get(e_manager_current_get());

   cfd = e_config_dialog_new(con,
			     _("Wallpaper Settings"),
			     "E", "_config_wallpaper_dialog",
			     "enlightenment/background", 0, v, cw);
   return cfd;
}

EAPI void
e_int_config_wallpaper_update(E_Config_Dialog *dia, char *file)
{
   E_Config_Dialog_Data *cfdata;
   char path[4096];
   const char *homedir;
   
   cfdata = dia->cfdata;
   homedir = e_user_homedir_get();
   cfdata->fmdir = 1;
   e_widget_radio_toggle_set(cfdata->o_personal, 1);
   snprintf(path, sizeof(path), "%s/.e/e/backgrounds", homedir);
   E_FREE(cfdata->bg);
   cfdata->bg = strdup(file);
   cfdata->use_theme_bg = 0;
   if (cfdata->o_theme_bg)
     e_widget_check_checked_set(cfdata->o_theme_bg, cfdata->use_theme_bg);
   if (cfdata->o_fm)
     e_fm2_path_set(cfdata->o_fm, path, "/");
   if (cfdata->o_preview)
     e_widget_preview_edje_set(cfdata->o_preview, cfdata->bg, "e/desktop/background");
   if (cfdata->o_frame)
     e_widget_change(cfdata->o_frame);
}

EAPI void
e_int_config_wallpaper_import_done(E_Config_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = dia->cfdata;
   cfdata->win_import = NULL;
}

EAPI void
e_int_config_wallpaper_gradient_done(E_Config_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = dia->cfdata;
   cfdata->dia_gradient = NULL;
}

static void
_cb_button_up(void *data1, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data1;
   if (cfdata->o_fm)
     e_fm2_parent_go(cfdata->o_fm);
   if (cfdata->o_frame)
     e_widget_scrollframe_child_pos_set(cfdata->o_frame, 0, 0);
}

static void
_cb_files_changed(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if (!cfdata->o_fm) return;
   if (!e_fm2_has_parent_get(cfdata->o_fm))
     {
	if (cfdata->o_up_button)
	  e_widget_disabled_set(cfdata->o_up_button, 1);
     }
   else
     {
	if (cfdata->o_up_button)
	  e_widget_disabled_set(cfdata->o_up_button, 0);
     }
   if (cfdata->o_frame)
     e_widget_scrollframe_child_pos_set(cfdata->o_frame, 0, 0);
}

static void
_cb_files_selection_change(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *selected;
   E_Fm2_Icon_Info *ici;
   const char *realpath;
   char buf[4096];
   
   cfdata = data;
   if (!cfdata->o_fm) return;
   selected = e_fm2_selected_list_get(cfdata->o_fm);
   if (!selected) return;
   ici = selected->data;
   realpath = e_fm2_real_path_get(cfdata->o_fm);
   if (!strcmp(realpath, "/"))
     snprintf(buf, sizeof(buf), "/%s", ici->file);
   else
     snprintf(buf, sizeof(buf), "%s/%s", realpath, ici->file);
   evas_list_free(selected);
   if (ecore_file_is_dir(buf)) return;
   E_FREE(cfdata->bg);
   cfdata->bg = strdup(buf);
   if (cfdata->o_preview)
     e_widget_preview_edje_set(cfdata->o_preview, buf, "e/desktop/background");
   if (cfdata->o_theme_bg)
     e_widget_check_checked_set(cfdata->o_theme_bg, 0);
   cfdata->use_theme_bg = 0;
   if (cfdata->o_frame)
     e_widget_change(cfdata->o_frame);
}

static void
_cb_files_selected(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
}

static void
_cb_files_files_changed(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   const char *p, *homedir;
   char buf[4096];
   
   cfdata = data;
   if (!cfdata->bg) return;
   if (!cfdata->o_fm) return;
   p = e_fm2_real_path_get(cfdata->o_fm);
   if (p)
     {
	if (strncmp(p, cfdata->bg, strlen(p))) return;
     }
   homedir = e_user_homedir_get();
   snprintf(buf, sizeof(buf), "%s/.e/e/backgrounds", homedir);
   if (!p) return;
   if (!strncmp(cfdata->bg, buf, strlen(buf)))
     p = cfdata->bg + strlen(buf) + 1;
   else
     {
	snprintf(buf, sizeof(buf), "%s/data/backgrounds", e_prefix_data_get());
	if (!strncmp(cfdata->bg, buf, strlen(buf)))
	  p = cfdata->bg + strlen(buf) + 1;
	else
	  p = cfdata->bg;
     }
   
   e_fm2_select_set(cfdata->o_fm, p, 1);
   e_fm2_file_show(cfdata->o_fm, p);
}

static void
_cb_files_files_deleted(void *data, Evas_Object *obj, void *event_info) 
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *sel, *all, *n;
   E_Fm2_Icon_Info *ici, *ic;
   
   cfdata = data;
   if (!cfdata->bg) return;
   if (!cfdata->o_fm) return;

   all = e_fm2_all_list_get(cfdata->o_fm);
   if (!all) return;
   sel = e_fm2_selected_list_get(cfdata->o_fm);
   if (!sel) return;

   ici = sel->data;
   
   all = evas_list_find_list(all, ici);
   n = evas_list_next(all);
   if (!n) 
     {
	n = evas_list_prev(all);
	if (!n) return;
     }
   
   ic = n->data;
   if (!ic) return;
   
   e_fm2_select_set(cfdata->o_fm, ic->file, 1);
   e_fm2_file_show(cfdata->o_fm, ic->file);
   
   evas_list_free(n);
   
   evas_object_smart_callback_call(cfdata->o_fm, "selection_change", cfdata);
}

static void
_cb_theme_wallpaper(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   const char *f;
   
   cfdata = data;
   if (cfdata->use_theme_bg)
     {
	f = e_theme_edje_file_get("base/theme/backgrounds", "e/desktop/background");
	E_FREE(cfdata->bg);
	cfdata->bg = strdup(f);
	if (cfdata->o_preview)
	  e_widget_preview_edje_set(cfdata->o_preview, f, "e/desktop/background");
     }
   else
     {
	if (cfdata->bg)
	  {
	     if (cfdata->o_preview)
	       e_widget_preview_edje_set(cfdata->o_preview, cfdata->bg, "e/desktop/background");
	  }
     }
}

static void
_cb_dir(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   char path[4096];
   const char *homedir;
   
   cfdata = data;
   if (cfdata->fmdir == 1)
     {
	snprintf(path, sizeof(path), "%s/data/backgrounds", e_prefix_data_get());
     }
   else
     {
	homedir = e_user_homedir_get();
	snprintf(path, sizeof(path), "%s/.e/e/backgrounds", homedir);
     }
   e_fm2_path_set(cfdata->o_fm, path, "/");
}

static void
_cb_import(void *data1, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data1;
   if (cfdata->win_import)
     e_win_raise(cfdata->win_import);
   else 
     cfdata->win_import = e_int_config_wallpaper_import(cfdata->cfd);
}

static void
_cb_gradient(void *data1, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data1;
   if (cfdata->dia_gradient)
     e_win_raise(cfdata->dia_gradient->win);
   else 
     cfdata->dia_gradient = e_int_config_wallpaper_gradient(cfdata->cfd);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   char path[4096];
   E_Config_Wallpaper *cw;
   const E_Config_Desktop_Background *cfbg;
  
   cw = cfdata->cfd->data;

   if (cw->specific_config)
     {
	const char *bg;
	/* specific config passed in. set for that only */
	bg = e_bg_file_get(cw->con_num, cw->zone_num, cw->desk_x, cw->desk_y);
	if (bg) cfdata->bg = strdup(bg);
     }
   else
     {
	/* get current desk. advanced mode allows selecting all, screen or desk */
	E_Container *c;
	E_Zone *z;
	E_Desk *d;

	c = e_container_current_get(e_manager_current_get());
	z = e_zone_current_get(c);
	d = e_desk_current_get(z);

	cfbg = e_bg_config_get(c->num, z->num, d->x, d->y);
	/* if we have a config for this bg, use it. */
	if (cfbg)
	  {
	     if (cfbg->container >= 0 && cfbg->zone >= 0)
	       {
		  if (cfbg->desk_x >= 0 && cfbg->desk_y >= 0)
		    cfdata->all_this_desk_screen = E_CONFIG_WALLPAPER_DESK;
		  else
		    cfdata->all_this_desk_screen = E_CONFIG_WALLPAPER_SCREEN;
	       }
	     E_FREE(cfdata->bg);
	     cfdata->bg = strdup(cfbg->file);
	  }
     }
   
   if ((!cfdata->bg) && e_config->desktop_default_background) 
     cfdata->bg = strdup(e_config->desktop_default_background);
   
   if (cfdata->bg)
     {
	const char *f;

	f = e_theme_edje_file_get("base/theme/backgrounds", "e/desktop/background");
	if (!strcmp(cfdata->bg, f))
	  cfdata->use_theme_bg = 1;
	snprintf(path, sizeof(path), "%s/data/backgrounds", e_prefix_data_get());
	if (!strncmp(cfdata->bg, path, strlen(path)))
	  cfdata->fmdir = 1;
     }
   else
     cfdata->use_theme_bg = 1;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfd->cfdata = cfdata;
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->win_import) 
     e_int_config_wallpaper_del(cfdata->win_import);
   if (cfdata->dia_gradient) 
     e_int_config_wallpaper_gradient_del(cfdata->dia_gradient);
   E_FREE(cfdata->bg);
   E_FREE(cfd->data);
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ot, *of, *il, *ol;
   char path[4096];
   const char *f, *homedir;
   E_Fm2_Config fmc;
   E_Zone *z;
   E_Radio_Group *rg;
   
   homedir = e_user_homedir_get();

   z = e_zone_current_get(cfd->con);
   
   ot = e_widget_table_add(evas, 0);
   ol = e_widget_table_add(evas, 0);
   il = e_widget_table_add(evas, 1);
   
   rg = e_widget_radio_group_new(&(cfdata->fmdir));
   o = e_widget_radio_add(evas, _("Personal"), 0, rg);
   cfdata->o_personal = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 0, 0, 1, 1, 1, 1, 0, 0);
   o = e_widget_radio_add(evas, _("System"), 1, rg);
   cfdata->o_system = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 1, 0, 1, 1, 1, 1, 0, 0);
   
   e_widget_table_object_append(ol, il, 0, 0, 1, 1, 0, 0, 0, 0);
   
   o = e_widget_button_add(evas, _("Go up a Directory"), "widget/up_dir",
			   _cb_button_up, cfdata, NULL);
   cfdata->o_up_button = o;
   e_widget_table_object_append(ol, o, 0, 1, 1, 1, 0, 0, 0, 0);
   
   if (cfdata->fmdir == 1)
     snprintf(path, sizeof(path), "%s/data/backgrounds", e_prefix_data_get());
   else
     snprintf(path, sizeof(path), "%s/.e/e/backgrounds", homedir);
   
   o = e_fm2_add(evas);
   cfdata->o_fm = o;
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_LIST;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 1;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 0;
   fmc.icon.list.w = 48;
   fmc.icon.list.h = 48;
   fmc.icon.fixed.w = 1;
   fmc.icon.fixed.h = 1;
   fmc.icon.extension.show = 0;
   fmc.icon.key_hint = NULL;
   fmc.list.sort.no_case = 1;
   fmc.list.sort.dirs.first = 0;
   fmc.list.sort.dirs.last = 1;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set(o, &fmc);
   e_fm2_icon_menu_flags_set(o, E_FM2_MENU_NO_SHOW_HIDDEN);
   evas_object_smart_callback_add(o, "dir_changed",
				  _cb_files_changed, cfdata);
   evas_object_smart_callback_add(o, "selection_change",
				  _cb_files_selection_change, cfdata);
   evas_object_smart_callback_add(o, "selected",
				  _cb_files_selected, cfdata);
   evas_object_smart_callback_add(o, "changed",
				  _cb_files_files_changed, cfdata);
   evas_object_smart_callback_add(o, "files_deleted",
				  _cb_files_files_deleted, cfdata);
   
   e_fm2_path_set(o, path, "/");

   of = e_widget_scrollframe_pan_add(evas, o,
				     e_fm2_pan_set,
				     e_fm2_pan_get,
				     e_fm2_pan_max_get,
				     e_fm2_pan_child_size_get);
   cfdata->o_frame = of;
   e_widget_min_size_set(of, 160, 160);
   e_widget_table_object_append(ol, of, 0, 2, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(ot, ol, 0, 0, 1, 1, 1, 1, 1, 1);
   
   of = e_widget_list_add(evas, 0, 0);
   
   il = e_widget_list_add(evas, 0, 1);
   o = e_widget_check_add(evas, _("Use Theme Wallpaper"), &cfdata->use_theme_bg);
   cfdata->o_theme_bg = o;
   evas_object_smart_callback_add(o, "changed", _cb_theme_wallpaper, cfdata);
   e_widget_list_object_append(il, o, 1, 0, 0.5);

   ol = e_widget_list_add(evas, 1, 1);
   o = e_widget_button_add(evas, _("Picture..."), "enlightenment/picture",
			   _cb_import, cfdata, NULL);
   e_widget_list_object_append(ol, o, 1, 0, 0.5);
   o = e_widget_button_add(evas, _("Gradient..."), "enlightenment/gradient",
			   _cb_gradient, cfdata, NULL);
   e_widget_list_object_append(ol, o, 1, 0, 0.5);
   e_widget_list_object_append(il, ol, 1, 0, 0.5);
   e_widget_list_object_append(of, il, 1, 0, 0.0);

   o = e_widget_preview_add(evas, 320, (320 * z->h) / z->w);
   cfdata->o_preview = o;
   if (cfdata->bg)
     f = cfdata->bg;
   else
     f = e_theme_edje_file_get("base/theme/backgrounds", "e/desktop/background");
   e_widget_preview_edje_set(o, f, "e/desktop/background");
   e_widget_list_object_append(of, o, 0, 0, 0.5);
   
   e_widget_table_object_append(ot, of, 1, 0, 1, 1, 1, 1, 1, 1);
   
   e_dialog_resizable_set(cfd->dia, 1);
   return ot;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Config_Wallpaper *cw;

   cw = cfd->data;
   if (cw->specific_config)
     {
	/* update a specific config */
	e_bg_del(cw->con_num, cw->zone_num, cw->desk_x, cw->desk_y);
	e_bg_add(cw->con_num, cw->zone_num, cw->desk_x, cw->desk_y, cfdata->bg);
     }
   else
     {
	/* set the default and nuke individual configs */
	while (e_config->desktop_backgrounds)
	  {
	     E_Config_Desktop_Background *cfbg;
	     cfbg = e_config->desktop_backgrounds->data;
	     e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
	  }
	if ((cfdata->use_theme_bg) || (!cfdata->bg))
	  e_bg_default_set(NULL);
	else
	  e_bg_default_set(cfdata->bg);

	cfdata->all_this_desk_screen = 0;
     }

   e_bg_update();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ot, *of, *il, *ol;
   char path[4096];
   const char *f, *homedir;
   E_Fm2_Config fmc;
   E_Zone *z;
   E_Radio_Group *rg;
   
   homedir = e_user_homedir_get();

   z = e_zone_current_get(cfd->con);
   
   ot = e_widget_table_add(evas, 0);
   ol = e_widget_table_add(evas, 0);
   il = e_widget_table_add(evas, 1);
   
   rg = e_widget_radio_group_new(&(cfdata->fmdir));
   o = e_widget_radio_add(evas, _("Personal"), 0, rg);
   cfdata->o_personal = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 0, 0, 1, 1, 1, 1, 0, 0);
   o = e_widget_radio_add(evas, _("System"), 1, rg);
   cfdata->o_system = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 1, 0, 1, 1, 1, 1, 0, 0);
   
   e_widget_table_object_append(ol, il, 0, 0, 1, 1, 0, 0, 0, 0);
   
   o = e_widget_button_add(evas, _("Go up a Directory"), "widget/up_dir",
			   _cb_button_up, cfdata, NULL);
   cfdata->o_up_button = o;
   e_widget_table_object_append(ol, o, 0, 1, 1, 1, 0, 0, 0, 0);
   
   if (cfdata->fmdir == 1)
     snprintf(path, sizeof(path), "%s/data/backgrounds", e_prefix_data_get());
   else
     snprintf(path, sizeof(path), "%s/.e/e/backgrounds", homedir);
   
   o = e_fm2_add(evas);
   cfdata->o_fm = o;
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_LIST;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 1;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 0;
   fmc.icon.list.w = 48;
   fmc.icon.list.h = 48;
   fmc.icon.fixed.w = 1;
   fmc.icon.fixed.h = 1;
   fmc.icon.extension.show = 0;
   fmc.icon.key_hint = NULL;
   fmc.list.sort.no_case = 1;
   fmc.list.sort.dirs.first = 0;
   fmc.list.sort.dirs.last = 1;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set(o, &fmc);
   e_fm2_icon_menu_flags_set(o, E_FM2_MENU_NO_SHOW_HIDDEN);
   evas_object_smart_callback_add(o, "dir_changed",
				  _cb_files_changed, cfdata);
   evas_object_smart_callback_add(o, "selection_change",
				  _cb_files_selection_change, cfdata);
   evas_object_smart_callback_add(o, "selected",
				  _cb_files_selected, cfdata);
   evas_object_smart_callback_add(o, "changed",
				  _cb_files_files_changed, cfdata);
   e_fm2_path_set(o, path, "/");

   of = e_widget_scrollframe_pan_add(evas, o,
				     e_fm2_pan_set,
				     e_fm2_pan_get,
				     e_fm2_pan_max_get,
				     e_fm2_pan_child_size_get);
   cfdata->o_frame = of;
   e_widget_min_size_set(of, 160, 160);
   e_widget_table_object_append(ol, of, 0, 2, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(ot, ol, 0, 0, 1, 1, 1, 1, 1, 1);
   
   of = e_widget_list_add(evas, 0, 0);
   
   il = e_widget_list_add(evas, 0, 1);
   o = e_widget_check_add(evas, _("Use Theme Wallpaper"), &cfdata->use_theme_bg);
   cfdata->o_theme_bg = o;
   evas_object_smart_callback_add(o, "changed", _cb_theme_wallpaper, cfdata);
   e_widget_list_object_append(il, o, 1, 0, 0.5);

   ol = e_widget_list_add(evas, 1, 1);
   o = e_widget_button_add(evas, _("Picture..."), "enlightenment/picture",
			   _cb_import, cfdata, NULL);
   e_widget_list_object_append(ol, o, 1, 0, 0.5);
   o = e_widget_button_add(evas, _("Gradient..."), "enlightenment/gradient",
			   _cb_gradient, cfdata, NULL);
   e_widget_list_object_append(ol, o, 1, 0, 0.5);
   e_widget_list_object_append(il, ol, 1, 0, 0.5);
   e_widget_list_object_append(of, il, 1, 0, 0.0);
   
   o = e_widget_preview_add(evas, 320, (320 * z->h) / z->w);
   cfdata->o_preview = o;
   if (cfdata->bg)
     f = cfdata->bg;
   else
     f = e_theme_edje_file_get("base/theme/backgrounds", "e/desktop/background");
   e_widget_preview_edje_set(o, f, "e/desktop/background");
   e_widget_list_object_append(of, o, 0, 0, 0.5);
   
   ol = e_widget_framelist_add(evas, _("Where to place the Wallpaper"), 0);
   e_widget_framelist_content_align_set(ol, 0.0, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->all_this_desk_screen));

   o = e_widget_radio_add(evas, _("All Desktops"), E_CONFIG_WALLPAPER_ALL, rg);
   e_widget_framelist_object_append(ol, o);
   o = e_widget_radio_add(evas, _("This Desktop"), E_CONFIG_WALLPAPER_DESK, rg);
   e_widget_framelist_object_append(ol, o);
   o = e_widget_radio_add(evas, _("This Screen"), E_CONFIG_WALLPAPER_SCREEN, rg);
   if (!((e_util_container_zone_number_get(0, 1)) ||
	 (e_util_container_zone_number_get(1, 0))))
     e_widget_disabled_set(o, 1);
   e_widget_framelist_object_append(ol, o);
   
   e_widget_list_object_append(of, ol, 1, 0, 0.5);
   
   e_widget_table_object_append(ot, of, 1, 0, 1, 1, 1, 1, 1, 1);
   
   e_dialog_resizable_set(cfd->dia, 1);
   return ot;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *fl = NULL, *l;
   E_Zone *z;
   E_Desk *d;
   
   z = e_zone_current_get(cfdata->cfd->con);
   if (!z) return 0;
   d = e_desk_current_get(z);
   if (!d) return 0;
   if (cfdata->use_theme_bg)
     {
	while (e_config->desktop_backgrounds)
	  {
	     E_Config_Desktop_Background *cfbg;
	     cfbg = e_config->desktop_backgrounds->data;
	     e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
	  }
	  e_bg_default_set(NULL);
     }
   else
     {
	if (cfdata->all_this_desk_screen == E_CONFIG_WALLPAPER_ALL)
	  {
	     while (e_config->desktop_backgrounds)
	       {
		  E_Config_Desktop_Background *cfbg;
		  cfbg = e_config->desktop_backgrounds->data;
		  e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
	       }
	     e_bg_default_set(cfdata->bg);
	  }
	else if (cfdata->all_this_desk_screen == E_CONFIG_WALLPAPER_DESK)
	  {
	     e_bg_del(z->container->num, z->num, d->x, d->y);
	     e_bg_del(z->container->num, -1, d->x, d->y);
	     e_bg_del(-1, z->num, d->x, d->y);
	     e_bg_del(-1, -1, d->x, d->y);
	     e_bg_add(z->container->num, z->num, d->x, d->y, cfdata->bg);
	     
	  }
	else if (cfdata->all_this_desk_screen == E_CONFIG_WALLPAPER_SCREEN)
	  {
	     for (l = e_config->desktop_backgrounds; l; l = l->next)
	       {
		  E_Config_Desktop_Background *cfbg;
		  
		  cfbg = l->data;
		  if (
		      (cfbg->container == z->container->num) &&
		      (cfbg->zone == z->num) 
		     )
		    fl = evas_list_append(fl, cfbg);
	       }
	     while (fl)
	       {
		  E_Config_Desktop_Background *cfbg;
		  cfbg = fl->data;
		  e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
		  fl = evas_list_remove_list(fl, fl);
	       }
	     e_bg_add(z->container->num, z->num, -1, -1, cfdata->bg);
	  }
     }
   e_bg_update();
   e_config_save_queue();
   return 1;
}
