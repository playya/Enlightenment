#include "e.h"

static void        *_create_data  (E_Config_Dialog *cfd);
static void         _free_data    (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply  (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void         _fill_list    (E_Config_Dialog_Data *cfdata);
static void         _cb_add       (void *data, void *data2);
static void         _cb_del       (void *data, void *data2);
static void         _cb_config    (void *data, void *data2);
static void         _list_cb_sel  (void *data);

static void         _cb_confirm_yes     (void *data);
static void         _cb_confirm_destroy (void *data);

struct _E_Config_Dialog_Data 
{
   Evas_List *mimes;
   const char *sel_mt;
   struct 
     {
	Evas_Object *list;
	Evas_Object *add, *del, *config;
     } gui;
};

EAPI E_Config_Dialog *
e_int_config_mime(E_Container *con) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   if (e_config_dialog_find("E", "_config_mime_dialog")) return NULL;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   
   cfd = e_config_dialog_new(con, _("Mime Types"), "E", "_config_mime_dialog",
			     "enlightenment/e", 0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   Evas_List *l;
   
   for (l = e_config->mime_icons; l; l = l->next) 
     {
	E_Config_Mime_Icon *mi, *mi2;
	
	mi = l->data;
	if (!mi) continue;
	mi2 = E_NEW(E_Config_Mime_Icon, 1);
	mi2->mime = evas_stringshare_add(mi->mime);
	mi2->icon = evas_stringshare_add(mi->icon);
	cfdata->mimes = evas_list_append(cfdata->mimes, mi2);
     }
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   while (cfdata->mimes) 
     {
	E_Config_Mime_Icon *mi;
	
	mi = cfdata->mimes->data;
	if (!mi) continue;
	if (mi->mime)
	  evas_stringshare_del(mi->mime);
	if (mi->icon)
	  evas_stringshare_del(mi->icon);
	E_FREE(mi);
	
	cfdata->mimes = evas_list_remove_list(cfdata->mimes, cfdata->mimes);
     }
   
   E_FREE(cfdata);
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   return 1;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ol;
   Evas_Object *ot, *ob;
   
   o = e_widget_list_add(evas, 0, 1);
   of = e_widget_framelist_add(evas, _("Mime Types"), 0);
   ol = e_widget_ilist_add(evas, 16, 16, NULL);
   cfdata->gui.list = ol;
   _fill_list(cfdata);
   e_widget_framelist_object_append(of, ol);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   ot = e_widget_table_add(evas, 0);
   ob = e_widget_button_add(evas, _("Add"), "widget/add", _cb_add, cfdata, NULL);
   cfdata->gui.add = ob;
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, _("Delete"), "widget/del", _cb_del, cfdata, NULL);
   cfdata->gui.del = ob;
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 1, 1, 0);
   ob = e_widget_button_add(evas, _("Configure"), "widget/config", _cb_config, cfdata, NULL);
   cfdata->gui.config = ob;
   e_widget_table_object_append(ot, ob, 0, 2, 1, 1, 1, 1, 1, 0);
   
   e_widget_disabled_set(cfdata->gui.del, 1);
   e_widget_disabled_set(cfdata->gui.config, 1);
   
   e_widget_list_object_append(o, ot, 1, 1, 0.0);
   return o;
}

static void 
_fill_list(E_Config_Dialog_Data *cfdata) 
{
   Evas_List *l;
   Evas_Coord w, h;
   
   e_widget_ilist_clear(cfdata->gui.list);
   for (l = cfdata->mimes; l; l = l->next) 
     {
	E_Config_Mime_Icon *mi;
	
	mi = l->data;
	if (!mi) continue;
	e_widget_ilist_append(cfdata->gui.list, NULL, mi->mime, _list_cb_sel, cfdata, NULL);
     }
   e_widget_ilist_go(cfdata->gui.list);
   e_widget_min_size_get(cfdata->gui.list, &w, &h);
   e_widget_min_size_set(cfdata->gui.list, w, 250);
}

static void
_cb_add(void *data, void *data2) 
{
   
}

static void
_cb_del(void *data, void *data2) 
{
   E_Config_Dialog_Data *cfdata;
   char buf[4096];
   
   cfdata = data;
   if (!cfdata) return;

   snprintf(buf, sizeof(buf), _("You requested to delete \"%s\".<br><br>"
				"Are you sure you want to delete this mime type?"), cfdata->sel_mt);
   e_confirm_dialog_show(_("Are you sure you want to delete this mime type?"),
			 "enlightenment/exit", buf, NULL, NULL, _cb_confirm_yes,
			 NULL, cfdata, NULL, _cb_confirm_destroy, NULL);
}

static void
_cb_config(void *data, void *data2) 
{
   
}

static void 
_list_cb_sel(void *data) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if (!cfdata) return;

   cfdata->sel_mt = e_widget_ilist_selected_label_get(cfdata->gui.list);
   if (!cfdata->sel_mt) return;
   
   e_widget_disabled_set(cfdata->gui.del, 0);
   e_widget_disabled_set(cfdata->gui.config, 0);
}

static void
_cb_confirm_yes(void *data) 
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *l;
   
   cfdata = data;
   if (!cfdata) return;
   if (!cfdata->sel_mt) return;
   
   for (l = e_config->mime_icons; l; l = l->next) 
     {
	E_Config_Mime_Icon *mi;
	
	mi = l->data;
	if (!mi) continue;
	if (strcmp(mi->mime, cfdata->sel_mt)) continue;
	cfdata->mimes = evas_list_remove(cfdata->mimes, mi);
	e_config->mime_icons = evas_list_remove(e_config->mime_icons, mi);
     }
}

static void
_cb_confirm_destroy(void *data) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if (!cfdata) return;
   _fill_list(cfdata);
}
