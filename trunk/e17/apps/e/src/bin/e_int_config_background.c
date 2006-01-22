/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "e.h"

#define BG_SET_DEFAULT_DESK 0
#define BG_SET_THIS_DESK 1
#define BG_SET_ALL_DESK 2

static void        *_create_data                (E_Config_Dialog *cfd);
static void         _free_data                  (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data                  (E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data           (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets       (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply_data        (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets    (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void         _load_bgs                   (E_Config_Dialog *cfd, Evas_Object *il);
void                _ilist_cb_bg_selected       (void *data);
static void         _bg_config_dialog_cb_import (void *data, void *data2);
static int          _bg_dialog_close            (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _bg_file_added              (void *data, Ecore_File_Monitor *monitor, Ecore_File_Event event, const char *path);

static Ecore_File_Monitor *_bg_file_monitor;

struct _E_Config_Dialog_Data
{
   char *bg, *current_bg;
   int bg_method;
   E_Config_Dialog *cfd;
   E_Config_Dialog *import;
   Evas_Object *il;
};

EAPI E_Config_Dialog *
e_int_config_background(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.apply_cfdata      = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->advanced.apply_cfdata   = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   v->close_cfdata            = _bg_dialog_close;

   cfd = e_config_dialog_new(con, _("Background Settings"), NULL, 0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->bg_method = BG_SET_DEFAULT_DESK;
   if (e_config->desktop_default_background)
     cfdata->current_bg = strdup(e_config->desktop_default_background);
   else
     cfdata->current_bg = NULL;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   cfd->cfdata = cfdata;
   cfdata->cfd = cfd;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (_bg_file_monitor) 
     { 
	ecore_file_monitor_del(_bg_file_monitor);
	_bg_file_monitor = NULL;
     }   
   if (cfdata->current_bg) free(cfdata->current_bg);
   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ot, *of, *il, *im;
   char path[4096];
   
   _fill_data(cfdata);

   ot = e_widget_table_add(evas, 0);
   il = e_widget_ilist_add(evas, 48, 48, &(cfdata->bg));

   cfdata->il = il;
   
   e_widget_ilist_selector_set(il, 1);
   e_widget_min_size_set(il, 180, 40);

   /* Load Bgs */
   _load_bgs(cfd, il);
   im = cfd->data;

   /* e_widget_focus_set(il, 1); */
   e_widget_ilist_go(il);
   e_widget_table_object_append(ot, il, 0, 0, 1, 2, 1, 1, 1, 1);

   /* Add import Button */
   o = e_widget_button_add(evas, _("Import An Image"), NULL, _bg_config_dialog_cb_import, cfd, NULL);
   e_widget_table_object_append(ot, o, 0, 2, 1, 1, 1, 0, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Background Preview"), 0);
   e_widget_min_size_set(of, 320, 240);
   e_widget_table_object_append(ot, of, 1, 0, 1, 2, 1, 1, 1, 1);
   e_widget_framelist_object_append(of, im);

   if (_bg_file_monitor) 
     {
	ecore_file_monitor_del(_bg_file_monitor);
	_bg_file_monitor = NULL;
     }
      
   snprintf(path, sizeof(path), "%s/.e/e/backgrounds", e_user_homedir_get());
   _bg_file_monitor = ecore_file_monitor_add(path, _bg_file_added, cfdata);
   return ot;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (!cfdata->bg) return 1;

   while (e_config->desktop_backgrounds)
     {
	E_Config_Desktop_Background *cfbg;
	cfbg = e_config->desktop_backgrounds->data;
	e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
     }
   if (e_config->desktop_default_background)
     evas_stringshare_del(e_config->desktop_default_background);
   
   if (!cfdata->bg[0]) e_config->desktop_default_background = NULL;
   else
     e_config->desktop_default_background = evas_stringshare_add(cfdata->bg);

   e_bg_update();
   e_config_save_queue();
   if (cfdata->current_bg) free(cfdata->current_bg);
   cfdata->current_bg = strdup(cfdata->bg);
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ot, *of, *il, *im, *oc;
   E_Radio_Group *rg;
   char path[4096];
   
   _fill_data(cfdata);

   ot = e_widget_table_add(evas, 0);
   il = e_widget_ilist_add(evas, 48, 48, &(cfdata->bg));
   cfdata->il = il;
   e_widget_ilist_selector_set(il, 1);
   e_widget_min_size_set(il, 180, 40);

   /* Load Bgs */
   _load_bgs(cfd, il);
   im = cfd->data;

   e_widget_focus_set(il, 1);
   e_widget_ilist_go(il);
   e_widget_table_object_append(ot, il, 0, 0, 1, 3, 1, 1, 1, 1);

   /* Add import Button */
   o = e_widget_button_add(evas, _("Import An Image"), NULL, _bg_config_dialog_cb_import, cfd, NULL);
   e_widget_table_object_append(ot, o, 0, 3, 1, 1, 1, 0, 0, 0);
   
   of = e_widget_framelist_add(evas, _("Background Preview"), 0);
   e_widget_min_size_set(of, 320, 240);
   e_widget_table_object_append(ot, of, 1, 0, 1, 2, 1, 1, 1, 1);
   e_widget_framelist_object_append(of, im);

   rg = e_widget_radio_group_new(&(cfdata->bg_method));
   of = e_widget_framelist_add(evas, _("Set Background For"), 0);
   e_widget_min_size_set(of, 200, 160);

   oc = e_widget_radio_add(evas, _("Default Desktop"), BG_SET_DEFAULT_DESK, rg);
   e_widget_framelist_object_append(of, oc);
   oc = e_widget_radio_add(evas, _("This Desktop"), BG_SET_THIS_DESK, rg);
   e_widget_framelist_object_append(of, oc);
   oc = e_widget_radio_add(evas, _("All Desktops"), BG_SET_ALL_DESK, rg);
   e_widget_framelist_object_append(of, oc);

   e_widget_table_object_append(ot, of, 1, 2, 1, 1, 1, 1, 1, 1);

   if (_bg_file_monitor) 
     { 
	ecore_file_monitor_del(_bg_file_monitor);
	_bg_file_monitor = NULL;
     }

   snprintf(path, sizeof(path), "%s/.e/e/backgrounds", e_user_homedir_get());   
   _bg_file_monitor = ecore_file_monitor_add(path, _bg_file_added, cfdata);   
   return ot;
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Zone *z;
   E_Desk *d;
   int x, y;

   if (!cfdata->bg) return 0;
   z = e_zone_current_get(cfd->con);
   d = e_desk_current_get(z);
   e_desk_xy_get(d, &x, &y);

   switch (cfdata->bg_method)
     {
      case BG_SET_DEFAULT_DESK:
	e_bg_del(-1, -1, -1, -1);
	e_bg_del(-1, z->num, x, y);
	e_bg_del(z->container->num, -1, x, y);
	e_bg_del(z->container->num, z->num, x, y);
	e_bg_del(-1, z->num, -1, -1);
	e_bg_del(z->container->num, -1, -1, -1);
	e_bg_del(z->container->num, z->num, -1, -1);

	if (e_config->desktop_default_background) 
	  evas_stringshare_del(e_config->desktop_default_background);
	
	if (!cfdata->bg[0]) e_config->desktop_default_background = NULL;
	else
	  e_config->desktop_default_background = evas_stringshare_add(cfdata->bg);

	e_bg_update();
	e_config_save_queue();
	break;
      case BG_SET_THIS_DESK:
	e_bg_del(-1, -1, -1, -1);
	e_bg_del(-1, z->num, x, y);
	e_bg_del(z->container->num, -1, x, y);
	e_bg_del(z->container->num, z->num, x, y);
	if (cfdata->bg[0]) 
	  e_bg_add(z->container->num, z->num, x, y, cfdata->bg);	
	
	e_bg_update();
        e_config_save_queue();
	break;
      case BG_SET_ALL_DESK:
	while (e_config->desktop_backgrounds)
	  {
	     E_Config_Desktop_Background *cfbg;

	     cfbg = e_config->desktop_backgrounds->data;
	     e_bg_del(cfbg->container, cfbg->zone, cfbg->desk_x, cfbg->desk_y);
	  }
	if (cfdata->bg[0]) 
	  e_bg_add(-1, -1, -1, -1, cfdata->bg);

	e_bg_update();
        e_config_save_queue();
	break;
     }
   if (cfdata->current_bg) free(cfdata->current_bg);
   cfdata->current_bg = strdup(cfdata->bg);
   return 1; /* Apply was OK */
}

static void
_load_bgs(E_Config_Dialog *cfd, Evas_Object *il)
{
   Evas *evas;
   Evas_Object *ic, *im, *o, *bg_obj;
   Evas_List *bg_dirs, *bg;
   Ecore_Evas *eebuf;
   Evas *evasbuf;
   const char *f;
   char *c;

   if (!il) return;
   
   evas = evas_object_evas_get(il);
   bg_obj = edje_object_add(cfd->dia->win->evas);

   /* Load The Theme's Background */
   eebuf = ecore_evas_buffer_new(1, 1);
   evasbuf = ecore_evas_get(eebuf);
   o = edje_object_add(evasbuf);
   f = e_theme_edje_file_get("base/theme/backgrounds", "desktop/background");
   c = strdup(f);
   if (edje_object_file_set(o, f, "desktop/background"))
     {
	Evas_Object *o = NULL;

	if (!e_thumb_exists(c))
	  o = e_thumb_generate_begin(c, 48, 48, cfd->dia->win->evas, &o, NULL, NULL);
	else
	  o = e_thumb_evas_object_get(c, cfd->dia->win->evas, 48, 48, 1);

	e_widget_ilist_append(il, o, "Theme Background", _ilist_cb_bg_selected, cfd, "");
     }
   if (!e_config->desktop_default_background)
     e_widget_ilist_selected_set(il, 0);
   
   im = e_widget_image_add_from_object(cfd->dia->win->evas, bg_obj, 320, 240);
   e_widget_image_object_set(im, e_thumb_evas_object_get(c, cfd->dia->win->evas, 320, 240, 1));

   evas_object_del(o);
   ecore_evas_free(eebuf);

   /* Load other backgrounds */
   bg_dirs = e_path_dir_list_get(path_backgrounds);
   for (bg = bg_dirs; bg; bg = bg->next)
     {
	E_Path_Dir *d;

	d = bg->data;
	if (ecore_file_is_dir(d->dir))
	  {
	     char *bg_file;
	     Ecore_List *bgs;
	     int i = 1;

	     bgs = ecore_file_ls(d->dir);
	     if (!bgs) continue;
	     while ((bg_file = ecore_list_next(bgs)))
	       {
		  char full_path[4096];

		  snprintf(full_path, sizeof(full_path), "%s/%s", d->dir, bg_file);
		  if (ecore_file_is_dir(full_path)) continue;
		  if (!e_util_edje_collection_exists(full_path, "desktop/background")) continue;

		  if (!e_thumb_exists(full_path))
		    ic = e_thumb_generate_begin(full_path, 48, 48, evas, &ic, NULL, NULL);
		  else
		    ic = e_thumb_evas_object_get(full_path, evas, 48, 48, 1);

		  e_widget_ilist_append(il, ic, ecore_file_strip_ext(bg_file), _ilist_cb_bg_selected, cfd, full_path);
		  if ((e_config->desktop_default_background) &&
		      (!strcmp(e_config->desktop_default_background, full_path)))
		    {
		       Evas_Object *o = NULL;

		       e_widget_ilist_selected_set(il, i);
		       o = edje_object_add(cfd->dia->win->evas);
		       edje_object_file_set(o, e_config->desktop_default_background, "desktop/background");
		       im = e_widget_image_add_from_object(cfd->dia->win->evas, o, 320, 240);
		       e_widget_image_object_set(im, e_thumb_evas_object_get(full_path, cfd->dia->win->evas, 320, 240, 1));
		    }
		  i++;
	       }
	     free(bg_file);
	     ecore_list_destroy(bgs);
	  }
	free(d);
     }
   evas_list_free(bg);
   evas_list_free(bg_dirs);
   free(c);
   cfd->data = im;
}

void
_ilist_cb_bg_selected(void *data)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_Data *cfdata;
   Evas *evas;

   cfd = data;
   cfdata = cfd->cfdata;
   evas = cfd->dia->win->evas;

   if (!(cfdata->bg[0]))
     {
	const char *theme;
	theme = e_theme_edje_file_get("base/theme/backgrounds", "desktop/background");
	e_widget_image_object_set(cfd->data, e_thumb_evas_object_get(strdup(theme), evas, 320, 240, 1));
     }
   else
     {
	e_widget_image_object_set(cfd->data, e_thumb_evas_object_get(cfdata->bg, evas, 320, 240, 1));
     }

   if (cfdata->current_bg)
     {
	if (!strcmp(cfdata->bg, cfdata->current_bg))
	  {
	     e_dialog_button_disable_num_set(cfd->dia, 0, 1);
	     e_dialog_button_disable_num_set(cfd->dia, 1, 1);
	  }
     }
}

static void
_bg_config_dialog_cb_import(void *data, void *data2) 
{
   E_Config_Dialog *parent;
   E_Config_Dialog *import;
   
   parent = data;
   if (!parent) return;

   import = e_int_config_background_import(parent);
   parent->cfdata->import = import;
}

static void 
_bg_file_added(void *data, Ecore_File_Monitor *monitor, Ecore_File_Event event, const char *path) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_Data *cfdata;
   Evas *evas;
   Evas_Object *il, *ic;
   char *file;
   
   cfdata = data;
   if (!cfdata) return;
   
   il = cfdata->il;
   if (!il) return;
   
   cfd = cfdata->cfd;
   if (!cfd) return;
   
   evas = e_win_evas_get(cfd->dia->win);
   
   file = (char *)ecore_file_get_file((char *)path);
   if (event == ECORE_FILE_EVENT_CREATED_FILE) 
     {
	if (e_util_edje_collection_exists((char *)path, "desktop/background")) 
	  {
	     if (!e_thumb_exists((char *)path))
	       ic = e_thumb_generate_begin((char *)path, 48, 48, evas, &ic, NULL, NULL);
	     else
	       ic = e_thumb_evas_object_get((char *)path, evas, 48, 48, 1);
	     e_widget_ilist_append(il, ic, ecore_file_strip_ext(file), _ilist_cb_bg_selected, cfd, (char *)path);	     
	  }
     }
   free(file);
}

static int
_bg_dialog_close(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{   
   if (!cfd) return 0;
   if (!cfdata) return 0;
   if (!cfdata->import) return 1;
   if (!cfdata->import->dia) return 0;
   
   e_object_del_attach_func_set(E_OBJECT(cfd->dia), NULL);
   e_object_del(E_OBJECT(cfdata->import->dia));
   
   return 1;
}
