/*
    * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
*/
#include "e.h"

typedef struct _CFModule CFModule;
typedef struct _CFType CFType;
typedef struct _CFTypes CFTypes;

struct _CFModule 
{
   const char *short_name, *name, *comment;
   const char *icon, *orig_path;
   int enabled, selected;
};

struct _CFType 
{
   const char *key, *name, *icon;
   Eina_Hash *modules;
};

struct _CFTypes 
{
   const char *key, *name, *icon;
};

struct _E_Config_Dialog_Data 
{
   Evas_Object *l_avail, *l_loaded;
   Evas_Object *b_load, *b_unload;
   Evas_Object *o_desc;
};

/* Key pairs for module types 
 * 
 * Should be in alphabetic order 
 */
const CFTypes _types[] = 
{
     {"appearance", N_("Appearance"), "preferences-appearance"},
     {"config", N_("Settings"), "preferences-system"},
     {"fileman", N_("File Manager"), "system-file-manager"},
     {"shelf", N_("Shelf"), "preferences-desktop-shelf"}, //FIXME use gadget icon
     {"system", N_("System"), "system"},
     {NULL, NULL, NULL}
};

/* local function protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void _fill_type_hash(void);
static void _load_modules(const char *dir);
static void _fill_list(Evas_Object *obj, int enabled);
static Eina_Bool _fill_list_types_avail(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);
static Eina_Bool _fill_list_types_load(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);
static Eina_Bool _fill_list_types(Evas_Object *obj, CFType *cft, int enabled);
static Eina_Bool _types_hash_cb_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__);
static Eina_Bool _mod_hash_cb_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__);
static Eina_Bool _mod_hash_avail_list(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);
static Eina_Bool _mod_hash_load_list(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);
static int _mod_list_sort(const void *data1, const void *data2);
static void _list_widget_load(Evas_Object *obj, Eina_List *list);
static void _avail_list_cb_change(void *data, Evas_Object *obj);
static void _load_list_cb_change(void *data, Evas_Object *obj);
static void _unselect_all_modules(void);
static Eina_Bool _mod_hash_unselect(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__);
static void _select_all_modules(Evas_Object *obj, void *data);
static void _btn_cb_unload(void *data, void *data2);
static void _btn_cb_load(void *data, void *data2);
static Eina_Bool _mod_hash_load(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__);
static Eina_Bool _mod_hash_unload(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__);
static void _enable_modules(int enable);
static Eina_Bool _enable_modules_types_enable(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);
static Eina_Bool _enable_modules_types_disable(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata);

/* local variables */
static Eina_Hash *types_hash = NULL;

EAPI E_Config_Dialog *
e_int_config_modules(E_Container *con, const char *params __UNUSED__)
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;

   if (e_config_dialog_find("E", "_config_modules_dialog")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;

   cfd = e_config_dialog_new(con, _("Module Settings"), 
			     "E", "_config_modules_dialog", 
			     "preferences-plugin", 0, v, NULL);
   e_dialog_resizable_set(cfd->dia, 1);
   return cfd;
}

/* local functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;

   _fill_type_hash();

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   if (!cfdata) return NULL;

   _fill_data(cfdata);
   return cfdata;
}

static void 
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   Eina_List *mdirs = NULL, *l = NULL;
   E_Path_Dir *epd = NULL;

   if (!cfdata) return;

   /* loop each path_modules dir and load modules for that path */
   mdirs = e_path_dir_list_get(path_modules);
   EINA_LIST_FOREACH(mdirs, l, epd)
     {
	if (!epd) continue;
	if (!ecore_file_is_dir(epd->dir)) continue;
	_load_modules(epd->dir);
     }
   if (mdirs) e_path_dir_list_free(mdirs);
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (types_hash)
     {
	eina_hash_foreach(types_hash, _types_hash_cb_free, NULL);
	eina_hash_free(types_hash);
	types_hash = NULL;
     }
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ol;

   o = e_widget_table_add(evas, 0);

   of = e_widget_frametable_add(evas, _("Available Modules"), 0);
   ol = e_widget_ilist_add(evas, 24, 24, NULL);
   cfdata->l_avail = ol;
   e_widget_ilist_multi_select_set(ol, 1);
   e_widget_on_change_hook_set(ol, _avail_list_cb_change, cfdata);
   _fill_list(ol, 0);
   e_widget_frametable_object_append(of, ol, 0, 0, 1, 1, 1, 1, 1, 1);
   ol = e_widget_button_add(evas, _("Load Module"), "list-add", 
			    _btn_cb_load, cfdata, NULL);
   cfdata->b_load = ol;
   e_widget_disabled_set(ol, 1);
   e_widget_frametable_object_append(of, ol, 0, 1, 1, 1, 1, 1, 1, 0);
   e_widget_table_object_append(o, of, 0, 0, 1, 1, 1, 1, 1, 1);

   of = e_widget_frametable_add(evas, _("Loaded Modules"), 0);
   ol = e_widget_ilist_add(evas, 24, 24, NULL);
   cfdata->l_loaded = ol;
   e_widget_ilist_multi_select_set(ol, 1);
   e_widget_on_change_hook_set(ol, _load_list_cb_change, cfdata);
   _fill_list(ol, 1);
   e_widget_frametable_object_append(of, ol, 0, 0, 1, 1, 1, 1, 1, 1);
   ol = e_widget_button_add(evas, _("Unload Module"), "list-remove", 
			    _btn_cb_unload, cfdata, NULL);
   cfdata->b_unload = ol;
   e_widget_disabled_set(ol, 1);
   e_widget_frametable_object_append(of, ol, 0, 1, 1, 1, 1, 1, 1, 0);
   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 1, 1, 1);

   ol = e_widget_textblock_add(evas);
   e_widget_size_min_set(ol, (200 * e_scale), 70);
   cfdata->o_desc = ol;
   e_widget_textblock_markup_set(ol, _("Description: Unavailable"));
   e_widget_table_object_append(o, ol, 0, 1, 2, 1, 1, 0, 1, 0);

   return o;
}

static void
_fill_type_hash(void)
{
   int i = 0;

   /* create the inital hash based on predefined list of types */
   for (i = 0; _types[i].name; i++)
     {
	CFType *cft = NULL;

	if (!_types[i].key) continue;
	if (eina_hash_find(types_hash, _types[i].key)) continue;

	cft = E_NEW(CFType, 1);
	if (!cft) continue;
	cft->key = eina_stringshare_add(_types[i].key);
	cft->name = eina_stringshare_add(_(_types[i].name));
	cft->icon = eina_stringshare_add(_types[i].icon);
	if (!types_hash) types_hash = eina_hash_string_superfast_new(NULL);
	eina_hash_direct_add(types_hash, cft->key, cft);
     }
}

static void
_load_modules(const char *dir)
{
   Eina_List *files = NULL, *l;
   char *mod = NULL, *file;

   if (!dir) return;
   if (!(files = ecore_file_ls(dir))) return;

   /* get all modules in this path_dir */
   EINA_LIST_FOREACH(files, l, mod)
     {
	Efreet_Desktop *desk = NULL;
	CFType *cft = NULL;
	CFModule *cfm = NULL;
	const char *type = NULL;
	char buf[PATH_MAX];

	/* check that we have a desktop file for this module */
	snprintf(buf, sizeof(buf), "%s/%s/module.desktop", dir, mod);
	if (!ecore_file_exists(buf)) continue;
	if (!(desk = efreet_desktop_get(buf))) continue;

	/* does the ModuleType exist in desktop? */
	if (desk->x)
	  type = eina_hash_find(desk->x, "X-Enlightenment-ModuleType");
	if (!type) type = eina_stringshare_add("shelf");

	/* do we have this module already in it's type hash ? */
	cft = eina_hash_find(types_hash, type);
	if (cft)
	  {
	     if ((cft->modules) && (eina_hash_find(cft->modules, mod))) 
	       {
		  if ((!desk->x) && (type)) eina_stringshare_del(type);
		  if (desk) efreet_desktop_free(desk);
		  continue;
	       }
	  }
	else
	  {
	     char buf[1024];

	     cft = E_NEW(CFType, 1);
	     if (!cft) continue;
	     cft->key = eina_stringshare_add(type);
	     snprintf(buf, sizeof(buf), "%s", type);
	     *buf = toupper(*buf);
	     cft->name = eina_stringshare_add(buf);
	     snprintf(buf, sizeof(buf), "enlightenment/%s", type);
	     if (e_util_edje_icon_check(buf))
	       cft->icon = eina_stringshare_add(buf);
	     if (!types_hash) types_hash = eina_hash_string_superfast_new(NULL);
	     eina_hash_direct_add(types_hash, cft->key, cft);
	  }

	/* module not in it's type hash, add */
	cfm = E_NEW(CFModule, 1);
	if (!cfm) continue;
	cfm->short_name = eina_stringshare_add(mod);
	if (desk->name) cfm->name = eina_stringshare_add(desk->name);
	if (desk->icon) cfm->icon = eina_stringshare_add(desk->icon);
	if (desk->comment) cfm->comment = eina_stringshare_add(desk->comment);
	if (desk->orig_path)
	  cfm->orig_path = eina_stringshare_add(desk->orig_path);
	if ((!desk->x) && (type)) eina_stringshare_del(type);
	efreet_desktop_free(desk);

	if (e_module_find(mod)) cfm->enabled = 1;
	if (!cft->modules) cft->modules = eina_hash_string_superfast_new(NULL);
	eina_hash_direct_add(cft->modules, cfm->short_name, cfm);
     }
   free(mod);
   EINA_LIST_FREE(files, file)
     free(file);
}

static void
_fill_list(Evas_Object *obj, int enabled)
{
   Evas *evas;
   Evas_Coord w;

   /* freeze evas, edje, and list widget */
   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(obj);
   e_widget_ilist_clear(obj);

   if (types_hash)
     {
	if (!enabled)
	  eina_hash_foreach(types_hash, _fill_list_types_avail, obj);
	else
	  eina_hash_foreach(types_hash, _fill_list_types_load, obj);
     }

   e_widget_ilist_go(obj);
   e_widget_size_min_get(obj, &w, NULL);
   e_widget_size_min_set(obj, (w * e_scale), (200 * e_scale));
//   e_widget_size_min_set(obj, w > (180 * e_scale) ? (w * e_scale) : (180 * e_scale), 
//                         (200 * e_scale));
   e_widget_ilist_thaw(obj);
   edje_thaw();
   evas_event_thaw(evas);
}

static Eina_Bool
_fill_list_types_avail(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   CFType *cft;
   Evas_Object *obj;

   cft = data;
   obj = fdata;

   return _fill_list_types(obj, cft, 0);
}

static Eina_Bool
_fill_list_types_load(const Eina_Hash *hash __UNUSED__,
		      const void *key __UNUSED__, void *data, void *fdata)
{
   CFType *cft;
   Evas_Object *obj;

   cft = data;
   obj = fdata;

   return _fill_list_types(obj, cft, 1);
}

static Eina_Bool
_fill_list_types(Evas_Object *obj, CFType *cft, int enabled)
{
   Evas *evas;
   Eina_List *l = NULL;
   Evas_Object *ic = NULL;
   int count;

   evas = evas_object_evas_get(obj);

   if (cft->modules)
     {
	if (!enabled)
	  eina_hash_foreach(cft->modules, _mod_hash_avail_list, &l);
	else
	  eina_hash_foreach(cft->modules, _mod_hash_load_list, &l);
     }

   if (l) count = eina_list_count(l);
   else return EINA_TRUE;

   /* We have at least one, append header */
   if (cft->icon)
     {
	ic = e_icon_add(evas);
	e_util_icon_theme_set(ic, cft->icon);
     }
   e_widget_ilist_header_append(obj, ic, cft->name);

   /* sort the list if we have more than one */
   if (count > 1)
     l = eina_list_sort(l, -1, _mod_list_sort);

   _list_widget_load(obj, l);

   if (l)
     {
	eina_list_free(l);
	l = NULL;
     }

   return EINA_TRUE;
}

static Eina_Bool
_types_hash_cb_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   CFType *type = NULL;

   if (!(type = data)) return 1;
   if (type->key) eina_stringshare_del(type->key);
   if (type->name) eina_stringshare_del(type->name);
   if (type->icon) eina_stringshare_del(type->icon);
   if (type->modules)
     {
	eina_hash_foreach(type->modules, _mod_hash_cb_free, NULL);
	eina_hash_free(type->modules);
	type->modules = NULL;
     }
   E_FREE(type);
   return EINA_TRUE;
}

static Eina_Bool
_mod_hash_cb_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   CFModule *mod = NULL;

   if (!(mod = data)) return 1;
   if (mod->short_name) eina_stringshare_del(mod->short_name);
   if (mod->name) eina_stringshare_del(mod->name);
   if (mod->icon) eina_stringshare_del(mod->icon);
   if (mod->comment) eina_stringshare_del(mod->comment);
   if (mod->orig_path) eina_stringshare_del(mod->orig_path);
   E_FREE(mod);
   return EINA_TRUE;
}

static Eina_Bool
_mod_hash_avail_list(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   Eina_List **l;
   CFModule *mod = NULL;

   mod = data;
   if ((!mod) || (mod->enabled)) return 1;
   l = fdata;
   *l = eina_list_append(*l, mod);
   return EINA_TRUE;
}

static Eina_Bool
_mod_hash_load_list(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   Eina_List **l;
   CFModule *mod = NULL;

   mod = data;
   if ((!mod) || (!mod->enabled)) return 1;
   l = fdata;
   *l = eina_list_append(*l, mod);
   return EINA_TRUE;
}

static int
_mod_list_sort(const void *data1, const void *data2)
{
   const CFModule *m1, *m2;

   if (!(m1 = data1)) return 1;
   if (!(m2 = data2)) return -1;
   return (strcmp(m1->name, m2->name));
}

/* nice generic function to load an ilist with items */
static void
_list_widget_load(Evas_Object *obj, Eina_List *list)
{
   Evas *evas;
   Eina_List *ml = NULL;
   CFModule *mod;

   if ((!obj) || (!list)) return;
   evas = evas_object_evas_get(obj);
   EINA_LIST_FOREACH(list, ml, mod)
     {
	Evas_Object *ic = NULL;
	char *path;
	char buf[PATH_MAX];

	if (!mod) continue;
	if (mod->icon)
	  {
	     ic = e_icon_add(evas);
	     if (!e_util_icon_theme_set(ic, mod->icon))
	       {
		  if (mod->orig_path)
		    {
		       path = ecore_file_dir_get(mod->orig_path);
		       snprintf(buf, sizeof(buf), "%s/%s.edj", path, mod->icon);
		       e_icon_file_edje_set(ic, buf, "icon");
		       free(path);
		    }
	       }
	  }
	if (mod->name)
	  e_widget_ilist_append(obj, ic, mod->name, NULL, mod, NULL);
	else if (mod->short_name)
	  e_widget_ilist_append(obj, ic, mod->short_name, NULL, mod, NULL);
     }
}

static void
_avail_list_cb_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata = NULL;

   if (!(cfdata = data)) return;

   /* Unselect all in loaded list & disable buttons */
   e_widget_ilist_unselect(cfdata->l_loaded);
   e_widget_disabled_set(cfdata->b_unload, 1);
   e_widget_disabled_set(cfdata->b_load, 1);

   /* Unselect all modules */
   _unselect_all_modules();

   /* Make sure something is selected */
   if (e_widget_ilist_selected_count_get(cfdata->l_avail) < 1) return;

   /* Select all modules in avail list that user wants */
   _select_all_modules(cfdata->l_avail, cfdata);

   /* Enable load button */
   e_widget_disabled_set(cfdata->b_load, 0);
}

static void
_load_list_cb_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata = NULL;

   if (!(cfdata = data)) return;

   /* Unselect all in avail list & disable button */
   e_widget_ilist_unselect(cfdata->l_avail);
   e_widget_disabled_set(cfdata->b_unload, 1);
   e_widget_disabled_set(cfdata->b_load, 1);

   /* Unselect all modules */
   _unselect_all_modules();

   /* Make sure something is selected */
   if (e_widget_ilist_selected_count_get(cfdata->l_loaded) < 1) return;

   /* Select all modules in loaded list that user wants */
   _select_all_modules(cfdata->l_loaded, cfdata);

   /* Enable unload button */
   e_widget_disabled_set(cfdata->b_unload, 0);
}

static void
_unselect_all_modules(void)
{
   int i = 0;

   if (!types_hash) return;

   /* loop types, getting all modules */
   for (i = 0; _types[i].name; i++)
     {
	CFType *cft = NULL;

	if (!_types[i].key) continue;
	cft = eina_hash_find(types_hash, _types[i].key);
	if ((!cft) || (!cft->modules)) continue;
	eina_hash_foreach(cft->modules, _mod_hash_unselect, NULL);
     }
}

static Eina_Bool
_mod_hash_unselect(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   CFModule *mod = NULL;

   if (!(mod = data)) return 1;
   mod->selected = 0;
   return 1;
}

static void
_select_all_modules(Evas_Object *obj, void *data)
{
   Eina_List *l = NULL;
   E_Ilist_Item *item = NULL;
   E_Config_Dialog_Data *cfdata = NULL;
   int i = -1;

   if (!(cfdata = data)) return;
   EINA_LIST_FOREACH(e_widget_ilist_items_get(obj), l, item)
     {
	CFModule *mod = NULL;

	i++;
	if ((!item) || (!item->selected)) continue;
	if (!(mod = e_widget_ilist_nth_data_get(obj, i))) continue;
	mod->selected = 1;
	if (mod->comment)
	  e_widget_textblock_markup_set(cfdata->o_desc, mod->comment);
	else
	  e_widget_textblock_markup_set(cfdata->o_desc,
					_("Description: Unavailable"));
     }
}

static void
_btn_cb_unload(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = NULL;
   int sel = -1;

   if (!(cfdata = data)) return;

   /* get what is currently selected in the list */
   sel = e_widget_ilist_selected_get(cfdata->l_loaded);

   _enable_modules(0);
   e_widget_disabled_set(cfdata->b_unload, 1);
   e_widget_textblock_markup_set(cfdata->o_desc, _("Description: Unavailable"));

   /* using a total reload here as it's simpler than parsing the list(s),
    * finding what was selected, removing it, checking for headers, etc */
   _fill_list(cfdata->l_avail, 0);
   _fill_list(cfdata->l_loaded, 1);

   /* move the selection down one if possible. Ilist itself will check
    * for headers, etc, etc */
   e_widget_ilist_selected_set(cfdata->l_loaded, sel);
}

static void
_btn_cb_load(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata = NULL;
   int sel = -1;

   if (!(cfdata = data)) return;

   /* get what is currently selected in the list */
   sel = e_widget_ilist_selected_get(cfdata->l_avail);

   _enable_modules(1);
   e_widget_disabled_set(cfdata->b_load, 1);
   e_widget_textblock_markup_set(cfdata->o_desc, _("Description: Unavailable"));

   /* using a total reload here as it's simpler than parsing the list(s),
    * finding what was selected, removing it, checking for headers, etc */
   _fill_list(cfdata->l_avail, 0);
   _fill_list(cfdata->l_loaded, 1);

   /* move the selection down one if possible. Ilist itself will check
    * for headers, etc, etc */
   e_widget_ilist_selected_set(cfdata->l_avail, sel);
}

static void
_enable_modules(int enable)
{
   if (!types_hash) return;

   if (enable)
     eina_hash_foreach(types_hash, _enable_modules_types_enable, NULL);
   else
     eina_hash_foreach(types_hash, _enable_modules_types_disable, NULL);
}

static Eina_Bool
_enable_modules_types_enable(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   CFType *cft;

   cft = data;
   if ((cft) && (cft->modules))
     eina_hash_foreach(cft->modules, _mod_hash_load, NULL);
   return 1;
}

static Eina_Bool
_enable_modules_types_disable(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   CFType *cft;

   cft = data;
   if ((cft) && (cft->modules))
     eina_hash_foreach(cft->modules, _mod_hash_unload, NULL);
   return 1;
}

static Eina_Bool
_mod_hash_load(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   CFModule *mod = NULL;
   E_Module *module = NULL;

   mod = data;
   if ((!mod) || (!mod->selected)) return 1;
   module = e_module_find(mod->short_name);
   if (!module) module = e_module_new(mod->short_name);
   if (!module) return 1;
   mod->enabled = e_module_enable(module);
   mod->selected = 0;
   return 1;
}

static Eina_Bool
_mod_hash_unload(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   CFModule *mod = NULL;
   E_Module *module = NULL;

   mod = data;
   if ((!mod) || (!mod->selected)) return 1;
   module = e_module_find(mod->short_name);
   if (module) 
     {
	e_module_disable(module);
	e_object_del(E_OBJECT(module));
     }
   mod->enabled = 0;
   mod->selected = 0;
   return 1;
}
