/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* FIXME: fwin - he fm2 filemanager wrapped with a window and scrollframe.
 * primitive BUT enough to test generic dnd and fm stuff more easily. don't
 * play with this unless u want to help with it. NOT COMPLETE! BEWARE!
 */

typedef enum
{
   E_FWIN_EXEC_NONE,
   E_FWIN_EXEC_DIRECT,
   E_FWIN_EXEC_SH,
   E_FWIN_EXEC_TERMINAL_DIRECT,
   E_FWIN_EXEC_TERMINAL_SH,
   E_FWIN_EXEC_DESKTOP
} E_Fwin_Exec_Type;

/* local subsystem functions */
static void _e_fwin_free(E_Fwin *fwin);
static void _e_fwin_cb_delete(E_Win *win);
static void _e_fwin_cb_resize(E_Win *win);
static void _e_fwin_deleted(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_changed(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_selected(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_menu_extend(void *data, Evas_Object *obj, E_Menu *m, E_Fm2_Icon_Info *info);
static void _e_fwin_parent(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_fwin_cb_menu_extend_start(void *data, Evas_Object *obj, E_Menu *m, E_Fm2_Icon_Info *info);
static void _e_fwin_cb_menu_open(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_fwin_cb_menu_open_with(void *data, E_Menu *m, E_Menu_Item *mi);

static void _e_fwin_cb_ilist_change(void *data);
static void _e_fwin_cb_ilist_selected(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_cb_fm_selection_change(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_cb_fm_selected(void *data, Evas_Object *obj, void *event_info);
static void _e_fwin_cb_open(void *data, E_Dialog *dia);
static void _e_fwin_cb_close(void *data, E_Dialog *dia);
static void _e_fwin_cb_dialog_free(void *obj);
static Evas_Bool _e_fwin_cb_hash_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata);
static E_Fwin_Exec_Type _e_fwin_file_is_exec(E_Fm2_Icon_Info *ici);
static void _e_fwin_file_exec(E_Fwin *fwin, E_Fm2_Icon_Info *ici, E_Fwin_Exec_Type ext);
static void _e_fwin_file_open_dialog(E_Fwin *fwin, Evas_List *files, int always);

/* local subsystem globals */
static Evas_List *fwins = NULL;

/* externally accessible functions */
EAPI int
e_fwin_init(void)
{
   return 1;
}

EAPI int
e_fwin_shutdown(void)
{
   Evas_List *l, *tmp;
   
   for (l = fwins; l;)
     {
	tmp = l;
	l = l->next;
	e_object_del(E_OBJECT(tmp->data));
     }
   return 1;
}

/* FIXME: this opens a new window - we need a way to inherit a zone as the
 * "fwin" window
 */
EAPI E_Fwin *
e_fwin_new(E_Container *con, const char *dev, const char *path)
{
   E_Fwin *fwin;
   char buf[4096];
   const char *file;
   Evas_Object *o;
   E_Fm2_Config fmc;
   
   fwin = E_OBJECT_ALLOC(E_Fwin, E_FWIN_TYPE, _e_fwin_free);
   if (!fwin) return NULL;
   fwin->win = e_win_new(con);
   if (!fwin->win)
     {
	free(fwin);
	return NULL;
     }
   fwins = evas_list_append(fwins, fwin);
   e_win_delete_callback_set(fwin->win, _e_fwin_cb_delete);
   e_win_resize_callback_set(fwin->win, _e_fwin_cb_resize);
   fwin->win->data = fwin;

   /* fm issues: */
   /* FIXME: if file executable - run it */
   
   o = edje_object_add(e_win_evas_get(fwin->win));
   e_theme_edje_object_set(o, "base/theme/fileman",
			   "e/fileman/window/main");
   evas_object_show(o);
   fwin->bg_obj = o;
   
   o = e_fm2_add(e_win_evas_get(fwin->win));
   fwin->fm_obj = o;
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_LIST;
   fmc.view.open_dirs_in_place = 0;
   fmc.view.selector = 0;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 0;
   fmc.icon.list.w = 24;
   fmc.icon.list.h = 24;
   fmc.icon.fixed.w = 1;
   fmc.icon.fixed.h = 1;
   fmc.icon.extension.show = 1;
   fmc.list.sort.no_case = 1;
   fmc.list.sort.dirs.first = 1;
   fmc.list.sort.dirs.last = 0;
   fmc.selection.single = 0;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set(o, &fmc);
   evas_object_smart_callback_add(o, "dir_changed",
				  _e_fwin_changed, fwin);
   evas_object_smart_callback_add(o, "dir_deleted",
				  _e_fwin_deleted, fwin);
   evas_object_smart_callback_add(o, "selected",
				  _e_fwin_selected, fwin);
   e_fm2_icon_menu_start_extend_callback_set(o, _e_fwin_cb_menu_extend_start, fwin);
   e_fm2_path_set(o, dev, path);
   e_fm2_icon_menu_end_extend_callback_set(o, _e_fwin_menu_extend, fwin);
   evas_object_show(o);
   
   o = e_scrollframe_add(e_win_evas_get(fwin->win));
   /* FIXME: this theme object will have more versions and options later
    * for things like swallowing widgets/buttons ot providing them - a
    * gadcon for starters for fm widgets. need to register the owning 
    * e_object of the gadcon so gadcon clients can get it and thus do
    * things like find out what dirs/path the fwin is for etc. this will
    * probably be how you add optional gadgets to fwin views like empty/full
    * meters for disk usage, and other dir info/stats or controls. also it
    * might be possible that we can have custom frames per dir later so need
    * a way to set an edje file directly
    */
   e_scrollframe_custom_theme_set(o, "base/theme/fileman",
				  "e/fileman/scrollframe/default");
   e_scrollframe_extern_pan_set(o, fwin->fm_obj,
				e_fm2_pan_set,
				e_fm2_pan_get,
				e_fm2_pan_max_get,
				e_fm2_pan_child_size_get);
   evas_object_propagate_events_set(fwin->fm_obj, 0);
   fwin->scrollframe_obj = o;
   evas_object_move(o, 0, 0);
   evas_object_show(o);
   
   e_fm2_window_object_set(fwin->fm_obj, E_OBJECT(fwin->win));
   
   evas_object_focus_set(fwin->fm_obj, 1);
   
   snprintf(buf, sizeof(buf), "_fwin::/%s", e_fm2_real_path_get(fwin->fm_obj));
   e_win_name_class_set(fwin->win, "E", buf);
   file = ecore_file_get_file(e_fm2_real_path_get(fwin->fm_obj));
   if (file)
     snprintf(buf, sizeof(buf), "%s", file);
   else
     snprintf(buf, sizeof(buf), "%s", e_fm2_real_path_get(fwin->fm_obj));
   e_win_title_set(fwin->win, buf);
   e_win_size_min_set(fwin->win, 24, 24);
   e_win_resize(fwin->win, 280, 200);
   e_win_show(fwin->win);
   
   return fwin;
}

/* local subsystem functions */
static void
_e_fwin_free(E_Fwin *fwin)
{
   if (fwin->fad)
     {
	e_object_del(E_OBJECT(fwin->fad->dia));
	fwin->fad = NULL;
     }
   e_object_del(E_OBJECT(fwin->win));
   fwins = evas_list_remove(fwins, fwin);
   free(fwin);
}

static void
_e_fwin_cb_delete(E_Win *win)
{
   E_Fwin *fwin;
   
   fwin = win->data;
   e_object_del(E_OBJECT(fwin));
}

static void
_e_fwin_cb_resize(E_Win *win)
{
   E_Fwin *fwin;
   
   fwin = win->data;
   evas_object_resize(fwin->bg_obj, fwin->win->w, fwin->win->h);
   evas_object_resize(fwin->scrollframe_obj, fwin->win->w, fwin->win->h);
}

static void
_e_fwin_deleted(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin *fwin;
   
   fwin = data;
   e_object_del(E_OBJECT(fwin));
}

static void
_e_fwin_changed(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin *fwin;
   
   fwin = data;
   if (fwin->scrollframe_obj)
     e_scrollframe_child_pos_set(fwin->scrollframe_obj, 0, 0);
}

static void
_e_fwin_selected(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin *fwin;
   Evas_List *selected;
   
   fwin = data;
   selected = e_fm2_selected_list_get(fwin->fm_obj);
   if (!selected) return;
   _e_fwin_file_open_dialog(fwin, selected, 0);
   evas_list_free(selected);
}

static void
_e_fwin_menu_extend(void *data, Evas_Object *obj, E_Menu *m, E_Fm2_Icon_Info *info)
{
   E_Fwin *fwin;
   E_Menu_Item *mi;
   
   fwin = data;
   if (e_fm2_has_parent_get(obj))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Go to Parent Directory"));
	e_menu_item_icon_edje_set(mi,
				  e_theme_edje_file_get("base/theme/fileman",
							"e/fileman/button/parent"),
				  "e/fileman/button/parent");
	e_menu_item_callback_set(mi, _e_fwin_parent, obj);
     }
   /* FIXME: if info != null then check mime type and offer options based
    * on that
    */
}

static void
_e_fwin_parent(void *data, E_Menu *m, E_Menu_Item *mi)
{
   e_fm2_parent_go(data);
}

static void
_e_fwin_cb_menu_extend_start(void *data, Evas_Object *obj, E_Menu *m, E_Fm2_Icon_Info *info)
{
   E_Menu_Item *mi;
   E_Fwin *fwin;
   
   fwin = data;
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Open"));
   e_menu_item_icon_edje_set(mi,
			     e_theme_edje_file_get("base/theme/fileman",
						   "e/fileman/button/open"),
			     "e/fileman/button/open");
   e_menu_item_callback_set(mi, _e_fwin_cb_menu_open, fwin);
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Open with..."));
   e_menu_item_icon_edje_set(mi,
			     e_theme_edje_file_get("base/theme/fileman",
						   "e/fileman/button/open"),
			     "e/fileman/button/open");
   e_menu_item_callback_set(mi, _e_fwin_cb_menu_open_with, fwin);
}

static void
_e_fwin_cb_menu_open(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fwin *fwin;
   Evas_List *selected;
   
   fwin = data;
   selected = e_fm2_selected_list_get(fwin->fm_obj);
   if (!selected) return;
   _e_fwin_file_open_dialog(fwin, selected, 0);
   evas_list_free(selected);
}

static void
_e_fwin_cb_menu_open_with(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fwin *fwin;
   Evas_List *selected;
   
   fwin = data;
   selected = e_fm2_selected_list_get(fwin->fm_obj);
   if (!selected) return;
   _e_fwin_file_open_dialog(fwin, selected, 1);
   evas_list_free(selected);
}


static void
_e_fwin_cb_ilist_change(void *data)
{
   E_Fwin_Apps_Dialog *fad;
   
   fad = data;
   E_FREE(fad->app2);
   if (fad->o_fm) e_fm2_select_set(fad->o_fm, NULL, 0);
}

static void
_e_fwin_cb_ilist_selected(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin_Apps_Dialog *fad;
   
   fad = data;
   _e_fwin_cb_open(fad, fad->dia);
}

static void
_e_fwin_cb_fm_selection_change(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin_Apps_Dialog *fad;
   Evas_List *sel;
   E_Fm2_Icon_Info *ici;
   
   fad = data;
   E_FREE(fad->app1);
   if (fad->o_ilist) e_widget_ilist_unselect(fad->o_ilist);
   E_FREE(fad->app2);
   sel = e_fm2_selected_list_get(obj);
   if (sel)
     {
	ici = sel->data;
	fad->app2 = strdup(ici->file);
	evas_list_free(sel);
     }
}

static void
_e_fwin_cb_fm_selected(void *data, Evas_Object *obj, void *event_info)
{
   E_Fwin_Apps_Dialog *fad;
   
   fad = data;
   _e_fwin_cb_open(fad, fad->dia);
}

static void
_e_fwin_cb_open(void *data, E_Dialog *dia)
{
   E_Fwin_Apps_Dialog *fad;
   Efreet_Desktop *desktop = NULL;
   char pcwd[4096], buf[4096];
   Evas_List *selected, *l;
   E_Fm2_Icon_Info *ici;
   Ecore_List *files = NULL;
   
   fad = data;
   if (fad->app1) desktop = efreet_util_desktop_file_id_find(fad->app1);
   else if (fad->app2) desktop = efreet_util_desktop_file_id_find(fad->app2);
   if (desktop)
     {
	getcwd(pcwd, sizeof(pcwd));
	chdir(e_fm2_real_path_get(fad->fwin->fm_obj));

	selected = e_fm2_selected_list_get(fad->fwin->fm_obj);
	if (selected)
	  {
	     files = ecore_list_new();
	     ecore_list_set_free_cb(files, free);
	     for (l = selected; l; l = l->next)
	       {
		  E_Fwin_Exec_Type ext;
		  
		  ici = l->data;
		  /* this snprintf is silly - but it's here in case i really do
		   * need to provide full paths (seems silly since we chdir
		   * into the dir)
		   */
		  buf[0] = 0;
		  ext = _e_fwin_file_is_exec(ici);
		  if (ext != E_FWIN_EXEC_NONE)
		    _e_fwin_file_exec(fad->fwin, ici, ext);
		  else
		    {
		       if (!((ici->link) && (ici->mount)))
			 {
			    if (ici->link)
			      {
				 if (!S_ISDIR(ici->statinfo.st_mode))
				   snprintf(buf, sizeof(buf), "%s", ici->file);
			      }
			    else
			      {
				 if (!S_ISDIR(ici->statinfo.st_mode))
				   snprintf(buf, sizeof(buf), "%s", ici->file);
			      }
			 }
		    }
		  if (buf[0] != 0)
		    {
		       if (ici->mime)
			 e_exehist_mime_desktop_add(ici->mime, desktop);
		       ecore_list_append(files, strdup(ici->file));
		    }
	       }
	     evas_list_free(selected);
	     e_exec(fad->fwin->win->border->zone, desktop, NULL, files, "fwin");
	     ecore_list_destroy(files);
	  }
	chdir(pcwd);
     }
   e_object_del(E_OBJECT(fad->dia));
}
    
static void
_e_fwin_cb_close(void *data, E_Dialog *dia)
{
   E_Fwin_Apps_Dialog *fad;
   
   fad = data;
   e_object_del(E_OBJECT(fad->dia));
}

static void
_e_fwin_cb_dialog_free(void *obj)
{
   E_Dialog *dia;
   E_Fwin_Apps_Dialog *fad;

   dia = (E_Dialog *)obj;
   fad = dia->data;
   E_FREE(fad->app1);
   E_FREE(fad->app2);
   fad->fwin->fad = NULL;
   free(fad);
}

static Evas_Bool
_e_fwin_cb_hash_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Evas_List **mlist;
   
   mlist = fdata;
   *mlist = evas_list_append(*mlist, key);
   return 1;
}

static E_Fwin_Exec_Type
_e_fwin_file_is_exec(E_Fm2_Icon_Info *ici)
{
   /* special file or dir - can't exec anyway */
   if ((S_ISDIR(ici->statinfo.st_mode)) ||
       (S_ISCHR(ici->statinfo.st_mode)) ||
       (S_ISBLK(ici->statinfo.st_mode)) ||
       (S_ISFIFO(ici->statinfo.st_mode)) ||
       (S_ISSOCK(ici->statinfo.st_mode)))
     return E_FWIN_EXEC_NONE;
   /* it is executable */
   if ((ici->statinfo.st_mode & S_IXOTH) ||
       ((getgid() == ici->statinfo.st_gid) &&
	(ici->statinfo.st_mode & S_IXGRP)) ||
       ((getuid() == ici->statinfo.st_uid) &&
	(ici->statinfo.st_mode & S_IXUSR)))
     {
	/* no mimetype */
	if (!ici->mime)
	  {
	     return E_FWIN_EXEC_DIRECT;
	  }
	/* mimetype */
	else
	  {
	     /* FIXME: - this could be config */
	     if (!strcmp(ici->mime, "application/x-desktop"))
	       {
		  return E_FWIN_EXEC_DESKTOP;
	       }
	     else if ((!strcmp(ici->mime, "application/x-sh")) ||
		      (!strcmp(ici->mime, "application/x-shellscript")) ||
		      (!strcmp(ici->mime, "application/x-csh")) ||
		      (!strcmp(ici->mime, "application/x-perl")) ||
		      (!strcmp(ici->mime, "application/x-shar")) ||
		      (!strcmp(ici->mime, "text/x-csh")) ||
		      (!strcmp(ici->mime, "text/x-python")) ||
		      (!strcmp(ici->mime, "text/x-sh"))
		      )
	       {
		  return E_FWIN_EXEC_DIRECT;
	       }
	  }
     }
   else
     {
	/* mimetype */
	if (ici->mime)
	  {
	     /* FIXME: - this could be config */
	     if (!strcmp(ici->mime, "application/x-desktop"))
	       {
		  return E_FWIN_EXEC_DESKTOP;
	       }
	     else if ((!strcmp(ici->mime, "application/x-sh")) ||
		      (!strcmp(ici->mime, "application/x-shellscript")) ||
		      (!strcmp(ici->mime, "text/x-sh"))
		      )
	       {
		  return E_FWIN_EXEC_TERMINAL_SH;
	       }
	  }
	else if ((e_util_glob_match(ici->file, "*.desktop")) ||
		 (e_util_glob_match(ici->file, "*.kdelink"))
		 )
	  {
	     return E_FWIN_EXEC_DESKTOP;
	  }
	else if (e_util_glob_match(ici->file, "*.run"))
	  {
	     return E_FWIN_EXEC_TERMINAL_SH;
	  }
     }
   return E_FWIN_EXEC_NONE;
}

static void
_e_fwin_file_exec(E_Fwin *fwin, E_Fm2_Icon_Info *ici, E_Fwin_Exec_Type ext)
{
   char buf[4096];
   Efreet_Desktop *desktop;
   
   /* FIXME: execute file ici with either a terminal, the shell, or directly
    * or open the .desktop and exec it */
   switch (ext)
     {
      case E_FWIN_EXEC_NONE:
	break;
      case E_FWIN_EXEC_DIRECT:
	e_exec(fwin->win->border->zone, NULL, ici->file, NULL, "fwin");
	break;
      case E_FWIN_EXEC_SH:
	snprintf(buf, sizeof(buf), "/bin/sh %s", e_util_filename_escape(ici->file));
	e_exec(fwin->win->border->zone, NULL, buf, NULL, NULL);
	break;
      case E_FWIN_EXEC_TERMINAL_DIRECT:
	snprintf(buf, sizeof(buf), "%s %s", e_config->exebuf_term_cmd, e_util_filename_escape(ici->file));
	e_exec(fwin->win->border->zone, NULL, buf, NULL, NULL);
	break;
      case E_FWIN_EXEC_TERMINAL_SH:
	snprintf(buf, sizeof(buf), "%s /bin/sh %s", e_config->exebuf_term_cmd, e_util_filename_escape(ici->file));
	e_exec(fwin->win->border->zone, NULL, buf, NULL, NULL);
	break;
      case E_FWIN_EXEC_DESKTOP:
	snprintf(buf, sizeof(buf), "%s/%s", e_fm2_real_path_get(fwin->fm_obj), ici->file);
	desktop = efreet_desktop_get(buf);
	if (desktop) e_exec(fwin->win->border->zone, desktop, NULL, NULL, NULL);
	break;
      default:
	break;
     }
}

static void
_e_fwin_file_open_dialog(E_Fwin *fwin, Evas_List *files, int always)
{
   E_Dialog *dia;
   Evas_Coord mw, mh;
   Evas_Object *o, *ocon, *of, *oi, *mt;
   Evas *evas;
   Evas_List *l;
   Evas_List *apps;
   E_Fwin_Apps_Dialog *fad;
   E_Fm2_Config fmc;
   E_Fm2_Icon_Info *ici;
   char buf[4096];
   const char *f;
   int need_dia = 0;
   Evas_Hash *mimes = NULL;
   Evas_List *mlist = NULL;

   if (fwin->fad)
     {
	e_object_del(E_OBJECT(fwin->fad->dia));
	fwin->fad = NULL;
     }
   if (!always)
     {
	for (l = files; l; l = l->next)
	  {
	     ici = l->data;
	     if ((ici->link) && (ici->mount))
	       e_fwin_new(fwin->win->container, ici->link, "/");
	     else if ((ici->link) && (ici->removable))
	       e_fwin_new(fwin->win->container, ici->link, "/");
	     else if (ici->link)
	       {
		  if (S_ISDIR(ici->statinfo.st_mode))
		    e_fwin_new(fwin->win->container, NULL, ici->link);
		  else
		    need_dia = 1;
	       }
	     else
	       {
		  snprintf(buf, sizeof(buf), "%s/%s", 
			   e_fm2_real_path_get(fwin->fm_obj), ici->file);
		  if (S_ISDIR(ici->statinfo.st_mode))
		    e_fwin_new(fwin->win->container, NULL, buf);
		  else
		    need_dia = 1;
	       }
	  }
	if (!need_dia) return;
	need_dia = 0;
     }
   
   /* 1. build hash of mimetypes */
   for (l = files; l; l = l->next)
     {
	ici = l->data;
        if (!((ici->link) && (ici->mount)))
	  {
	     if (_e_fwin_file_is_exec(ici) == E_FWIN_EXEC_NONE)
	       {
		  if (ici->link)
		    {
		       if (!S_ISDIR(ici->statinfo.st_mode))
			 {
			    f = e_fm_mime_filename_get(ici->link);
			    mimes = evas_hash_del(mimes, f, (void *)1);
			    mimes = evas_hash_direct_add(mimes, f, (void *)1);
			 }
		    }
		  else
		    {
		       snprintf(buf, sizeof(buf), "%s/%s",
				e_fm2_real_path_get(fwin->fm_obj), ici->file);
		       if (!S_ISDIR(ici->statinfo.st_mode))
			 {
			    mimes = evas_hash_del(mimes, ici->mime, (void *)1);
			    mimes = evas_hash_direct_add(mimes, ici->mime, (void *)1);
			 }
		    }
	       }
	  }
     }
   /* 2. for each mimetype list apps that handle it */
   if (mimes)
     {
	evas_hash_foreach(mimes, _e_fwin_cb_hash_foreach, &mlist);
	evas_hash_free(mimes);
     }
   /* 3. add apps to a list so its a unique app list */
   apps = NULL;
   if (mlist)
     {
	Ecore_List *ml;
	Efreet_Desktop *desktop;

	for (l = mlist; l; l = l->next)
	  {
	     ml = efreet_util_desktop_mime_list(l->data);
	     if (ml)
	       {
		  ecore_list_goto_first(ml);
		  while ((desktop = ecore_list_next(ml)))
		    apps = evas_list_append(apps, desktop);
		  ecore_list_destroy(ml);
	       }
	  }
     }

   if (!always)
     {
	/* FIXME: well this is simplisitic - if only 1 mime type is being
	 * opened then look for the previously used app for that mimetype and
	 * if found, use that.
	 * 
	 * we could get more sophisitcated.
	 * 1. find apps for each mimetype in mlist. if all prev used apps are
	 * the same, then use previously used app.
	 * OR if this fails
	 * 2. find all apps for each mimetype. find the one used the most.
	 * if that app can handle all mimetypes in the list - use that. if not
	 * find the next most often listed app - if that can handle all apps,
	 * use it, if not fall back again - and so on - if all apps listed do
	 * not contain 1 that handles all the mime types - fall back to dialog
	 */
	if (evas_list_count(mlist) <= 1)
	  {
	     Efreet_Desktop *desktop = NULL;
	     char pcwd[4096];
	     Ecore_List *files_list = NULL;
	   
	     need_dia = 1;
	     if (mlist) desktop = e_exehist_mime_desktop_get(mlist->data);
	     getcwd(pcwd, sizeof(pcwd));
	     chdir(e_fm2_real_path_get(fwin->fm_obj));
	     
	     files_list = ecore_list_new();
	     ecore_list_set_free_cb(files_list, free);
	     for (l = files; l; l = l->next)
	       {
		  ici = l->data;
		  if (_e_fwin_file_is_exec(ici) == E_FWIN_EXEC_NONE)
		    ecore_list_append(files_list, strdup(ici->file));
	       }
	     for (l = files; l; l = l->next)
	       {
		  E_Fwin_Exec_Type ext;
		  
		  ici = l->data;
		  ext = _e_fwin_file_is_exec(ici);
		  if (ext != E_FWIN_EXEC_NONE)
		    {
		       _e_fwin_file_exec(fwin, ici, ext);
		       need_dia = 0;
		    }
	       }
	     if (desktop)
	       {
		  if (e_exec(fwin->win->border->zone, desktop, NULL, files_list, "fwin"))
		    need_dia = 0;
	       }
	     ecore_list_destroy(files_list);
	     
	     chdir(pcwd);
	     if (!need_dia)
	       {
		  if (apps) evas_list_free(apps);
		  evas_list_free(mlist);
		  return;
	       }
	  }
     }
   evas_list_free(mlist);
   
   fad = E_NEW(E_Fwin_Apps_Dialog, 1);
   dia = e_dialog_new(fwin->win->border->zone->container, 
		      "E", "_fwin_open_apps");
   e_dialog_title_set(dia, _("Open with..."));
   e_dialog_border_icon_set(dia, "enlightenment/applications");
   e_dialog_button_add(dia, _("Open"), "enlightenment/open",
		       _e_fwin_cb_open, fad);
   e_dialog_button_add(dia, _("Close"), "enlightenment/close",
		       _e_fwin_cb_close, fad);

   fad->dia = dia;
   fad->fwin = fwin;
   fwin->fad = fad;
   dia->data = fad;
   e_object_free_attach_func_set(E_OBJECT(dia), _e_fwin_cb_dialog_free);
   
   evas = e_win_evas_get(dia->win);
   
   o = e_widget_list_add(evas, 1, 1);
   ocon = o;

   if (apps)
     {
	of = e_widget_framelist_add(evas, _("Specific Applications"), 0);
	o = e_widget_ilist_add(evas, 24, 24, &(fad->app1));
	fad->o_ilist = o;
	for (l = apps; l; l = l->next)
	  {
	     Efreet_Desktop *desktop;

	     desktop = l->data;
	     oi = e_util_desktop_icon_add(desktop, "24x24", evas);
	     e_widget_ilist_append(o, oi, desktop->name,
				   _e_fwin_cb_ilist_change, fad, 
				   efreet_util_path_to_file_id(desktop->orig_path));
	  }
	evas_list_free(apps);
	e_widget_ilist_go(o);
	e_widget_min_size_set(o, 160, 240);
	e_widget_framelist_object_append(of, o);
	e_widget_list_object_append(ocon, of, 1, 1, 0.5);
	evas_object_smart_callback_add(o, "selected",
				       _e_fwin_cb_ilist_selected, fad);
     }
   
   of = e_widget_framelist_add(evas, _("All Applications"), 0);
   mt = e_fm2_add(evas);
   fad->o_fm = mt;
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_LIST;
   fmc.view.open_dirs_in_place = 0;
   fmc.view.selector = 1;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 1;
   fmc.icon.list.w = 24;
   fmc.icon.list.h = 24;
   fmc.icon.fixed.w = 1;
   fmc.icon.fixed.h = 1;
   fmc.icon.extension.show = 1;
   fmc.icon.key_hint = NULL;
   fmc.list.sort.no_case = 1;
   fmc.list.sort.dirs.first = 1;
   fmc.list.sort.dirs.last = 0;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set(mt, &fmc);
   e_fm2_icon_menu_flags_set(mt, E_FM2_MENU_NO_SHOW_HIDDEN | E_FM2_MENU_NO_REMEMBER_ORDERING | E_FM2_MENU_NO_NEW_DIRECTORY | E_FM2_MENU_NO_RENAME | E_FM2_MENU_NO_DELETE);
   evas_object_smart_callback_add(mt, "selection_change",
				  _e_fwin_cb_fm_selection_change, fad);
   evas_object_smart_callback_add(mt, "selected",
				  _e_fwin_cb_fm_selected, fad);
   snprintf(buf, sizeof(buf), "%s/.e/e/applications/all", e_user_homedir_get());
   e_fm2_path_set(mt, buf, "/");
   o = e_widget_scrollframe_pan_add(evas, mt,
				    e_fm2_pan_set,
				    e_fm2_pan_get,
				    e_fm2_pan_max_get,
				    e_fm2_pan_child_size_get);
   e_widget_min_size_set(o, 160, 240);
   e_widget_framelist_object_append(of, o);
   e_widget_list_object_append(ocon, of, 1, 1, 0.5);
   
   e_widget_min_size_get(ocon, &mw, &mh);
   e_dialog_content_set(dia, ocon, mw, mh);
   
   e_dialog_show(dia);
}
