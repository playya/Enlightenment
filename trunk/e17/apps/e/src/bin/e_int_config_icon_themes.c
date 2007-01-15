/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#define MOD_UNLOADED 0
#define MOD_ENABLED 1

typedef struct _CFIconTheme CFIconTheme;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

static void _ilist_cb_change(void *data, Evas_Object *obj);
static void _add_theme(void *value, void *user_data);
static int _sort_icon_themes(void *data1, void *data2);

struct _CFIconTheme
{
   char *name;
   Ecore_Desktop_Icon_Theme *theme;
};

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas_List *icon_themes;
   int state;
   char *themename;
   struct {
      Evas_Object *comment;
      Evas_Object *list;
      Evas_Object *o_fm;
      Evas_Object *o_frame;
      Evas_Object *o_up_button;
   } gui;
};

EAPI E_Config_Dialog *
e_int_config_icon_themes(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "_config_icon_theme_dialog")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   v->advanced.apply_cfdata   = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->basic.apply_cfdata      = _basic_apply_data;
   
   cfd = e_config_dialog_new(con,
			     _("Icon Theme Settings"),
			     "E", "_config_icon_theme_dialog",
			     "enlightenment/icon_theme", 0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Ecore_Hash *icon_themes = NULL;

   if ((icon_themes = ecore_desktop_icon_theme_list()))
      ecore_hash_for_each_node(icon_themes, _add_theme, cfdata);

   if (cfdata->icon_themes)
     cfdata->icon_themes = evas_list_sort(cfdata->icon_themes, evas_list_count(cfdata->icon_themes), _sort_icon_themes);

   if (e_config->icon_theme)
      cfdata->themename = strdup(e_config->icon_theme);
   else
      cfdata->themename = strdup("hicolor");

   return;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   while ((l = cfdata->icon_themes))
     {
        CFIconTheme *m;

        m = l->data;
        cfdata->icon_themes = evas_list_remove_list(cfdata->icon_themes, l);
        free(m->name);
        E_FREE(m);
     }
   E_FREE(cfdata->themename);
   E_FREE(cfdata);
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Action *a;
   
   /* Actually take our cfdata settings and apply them in real life */
   e_config->icon_theme = evas_stringshare_add(cfdata->themename);
   e_config_save_queue();

   /* If it's good enough for themes, it's good enough for icon themes, but ICK!. */
   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
   return 1; /* Apply was OK */
}

static void
_cb_button_up(void *data1, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data1;
   if (cfdata->gui.o_fm)
     e_fm2_parent_go(cfdata->gui.o_fm);
   if (cfdata->gui.o_frame)
     e_widget_scrollframe_child_pos_set(cfdata->gui.o_frame, 0, 0);
}

static void
_cb_files_changed(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if (!cfdata->gui.o_fm) return;
   if (!e_fm2_has_parent_get(cfdata->gui.o_fm))
     {
	if (cfdata->gui.o_up_button)
	  e_widget_disabled_set(cfdata->gui.o_up_button, 1);
     }
   else
     {
	if (cfdata->gui.o_up_button)
	  e_widget_disabled_set(cfdata->gui.o_up_button, 0);
     }
   if (cfdata->gui.o_frame)
     e_widget_scrollframe_child_pos_set(cfdata->gui.o_frame, 0, 0);
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob, *ot, *ilist, *mt;
   Evas_List *l;
   E_Fm2_Config fmc;
   int i;

   o = e_widget_list_add(evas, 1, 0);
   ot = e_widget_table_add(evas, 1);

   of = e_widget_framelist_add(evas, _("Icon Themes"), 1);
   ilist = e_widget_ilist_add(evas, 24, 24, &(cfdata->themename));
   cfdata->gui.list = ilist;
   e_widget_on_change_hook_set(ilist, _ilist_cb_change, cfdata);

   evas_event_freeze(evas_object_evas_get(ilist));
   edje_freeze();
   e_widget_ilist_freeze(ilist);
   
   cfdata->state = -1;
   i = 0;
   for (l = cfdata->icon_themes; l; l = l->next)
     {
	CFIconTheme *cm;
	Evas_Object *oc = NULL;
	
	cm = l->data;
	if (cm)
	  {
	     if (cm->theme)
	        {
	           if (!cm->theme->example_path)
                      cm->theme->example_path = (char *) ecore_desktop_icon_find(cm->theme->example, "24x24", cm->name);
	           if (cm->theme->example_path)
		      {
                         oc = e_icon_add(evas);
	                 e_icon_file_set(oc, cm->theme->example_path);
	                 e_icon_fill_inside_set(oc, 1);
		      }
		}
             e_widget_ilist_append(ilist, oc, cm->theme->name, NULL, NULL, cm->name);
	     if (!strcmp(cfdata->themename, cm->name))
	       e_widget_ilist_selected_set(ilist, i);
	     i++;
	  }
     }

   e_widget_ilist_go(ilist);
   e_widget_min_size_set(of, 160, 160);
   e_widget_ilist_thaw(ilist);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(ilist));
   
   e_widget_framelist_object_append(of, ilist);
   e_widget_table_object_append(ot, of, 0, 0, 2, 4, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, _("Icon Theme"), 0);

   mt = e_widget_textblock_add(evas);
   e_widget_textblock_plain_set(mt, "");
   e_widget_min_size_set(mt, 200, 75);
   cfdata->gui.comment = mt;
   e_widget_framelist_object_append(of, mt);

   mt = e_widget_button_add(evas, _("Go up a Directory"), "widget/up_dir",
			   _cb_button_up, cfdata, NULL);
   cfdata->gui.o_up_button = mt;
   e_widget_framelist_object_append(of, mt);

   mt = e_fm2_add(evas);
   cfdata->gui.o_fm = mt;
   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_LIST;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 1;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 0;
   fmc.view.extra_file_source = NULL;
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
   e_fm2_icon_menu_flags_set(o, E_FM2_MENU_NO_SHOW_HIDDEN);
   evas_object_smart_callback_add(mt, "dir_changed",
				  _cb_files_changed, cfdata);
//   e_fm2_path_set(cfdata->gui.o_fm, "/opt/kde3/share/icons/crystalsvg", "/");

   ob = e_widget_scrollframe_pan_add(evas, mt,
				     e_fm2_pan_set,
				     e_fm2_pan_get,
				     e_fm2_pan_max_get,
				     e_fm2_pan_child_size_get);
   cfdata->gui.o_frame = ob;
   e_widget_min_size_set(ob, 200, 120);
   e_widget_framelist_object_append(of, ob);
   e_box_pack_options_set(ob,
			  1, 1, /* fill */
			  1, 1, /* expand */
			  1, 1, /* align */
			  200, 120, /* min */
			  99999, 99999 /* max */
			  );

   e_widget_table_object_append(ot, of, 2, 0, 2, 4, 1, 1, 1, 1);
   e_widget_list_object_append(o, ot, 1, 1, 0.5);
   e_dialog_resizable_set(cfd->dia, 1);
   _ilist_cb_change(cfdata, ilist);
   return o;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ilist, *of;
   Evas_List *l;
   int i;

   o = e_widget_list_add(evas, 1, 0);
   of = e_widget_framelist_add(evas, _("Icon Themes"), 0);
   ilist = e_widget_ilist_add(evas, 24, 24, &(cfdata->themename));
   cfdata->gui.list = ilist;
   e_widget_on_change_hook_set(ilist, _ilist_cb_change, cfdata);

   evas_event_freeze(evas_object_evas_get(ilist));
   edje_freeze();
   e_widget_ilist_freeze(ilist);
   
   cfdata->state = -1;
   i = 0;
   for (l = cfdata->icon_themes; l; l = l->next)
     {
	CFIconTheme *cm;
	Evas_Object *oc = NULL;
	
	cm = l->data;
	if (cm)
	  {
	     if (cm->theme)
	        {
	           if (!cm->theme->example_path)
                      cm->theme->example_path = (char *) ecore_desktop_icon_find(cm->theme->example, "24x24", cm->name);
	           if (cm->theme->example_path)
		      {
                         oc = e_icon_add(evas);
	                 e_icon_file_set(oc, cm->theme->example_path);
	                 e_icon_fill_inside_set(oc, 1);
		      }
		}
             e_widget_ilist_append(ilist, oc, cm->theme->name, NULL, NULL, cm->name);
	     if (strcmp(cfdata->themename, cm->name) == 0)
		e_widget_ilist_selected_set(ilist, i);
	     i++;
	  }
     }

   e_widget_ilist_go(ilist);
   e_widget_min_size_set(ilist, 200, 240);
   e_widget_ilist_thaw(ilist);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(ilist));

   e_widget_framelist_object_append(of, ilist);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   e_dialog_resizable_set(cfd->dia, 1);
   _ilist_cb_change(cfdata, ilist);
   return o;
}

static void
_ilist_cb_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;
   const char *v;
   Ecore_Desktop_Icon_Theme *theme;
   char * dir;

   cfdata = data;
   v = cfdata->themename;
   if (!v) return;
   if (!cfdata->gui.comment) return;

   if ((theme = ecore_desktop_icon_theme_get(v, NULL)))
      {
         char *text;
	 size_t length;

         length = strlen(theme->comment) + strlen(theme->path) + 16;
	 if (theme->inherits)
	   length += strlen(theme->inherits) + 32;
	 text = alloca(length);
	 if (text)
	    {
	       if (theme->inherits)
		 sprintf(text, "%s\npath = %s\ninherits from %s", theme->comment, theme->path, theme->inherits);
	       else
		 sprintf(text, "%s\npath = %s", theme->comment, theme->path);
               e_widget_textblock_plain_set(cfdata->gui.comment, text);
	    }
	 dir = ecore_file_get_dir(theme->path);
         e_fm2_path_set(cfdata->gui.o_fm, dir, "/");
	 E_FREE(dir);
      }
}

static void
_add_theme(void *value, void *user_data)
{
   Ecore_Hash_Node    *node;
   E_Config_Dialog_Data *cfdata;
   const char           *key;
   Ecore_Desktop_Icon_Theme *theme;
   CFIconTheme *m;

   cfdata = user_data;
   node = (Ecore_Hash_Node *) value;
   key = node->key;
   theme = (Ecore_Desktop_Icon_Theme *)node->value;

   m = E_NEW(CFIconTheme, 1);
   if (m)
     {
	m->name = strdup(key);
	m->theme = theme;
	cfdata->icon_themes = evas_list_append(cfdata->icon_themes, m);
     }
}

static int
_sort_icon_themes(void *data1, void *data2)
{
   CFIconTheme *m1, *m2;

   if (!data1) return 1;
   if (!data2) return -1;

   m1 = data1;
   m2 = data2;

   /* These are supposed to be required strings.  Be paranoid pending further investigation. */
   if (!m1->theme) return 1;
   if (!m2->theme) return -1;

   if (!m1->theme->name) return 1;
   if (!m2->theme->name) return -1;

   return (strcmp((const char*)m1->theme->name, (const char*)m2->theme->name));
}
