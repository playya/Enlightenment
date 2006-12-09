/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* FIXME:
 * 
 * basic -
 * + show file 
 * + show size
 * + show modified date
 * + show mimetype
 * + show permissions (others read, others write)
 * + show preview
 * + show owner
 * * show icon
 * * show symlink/fifo/socket/etc. status
 * * show broken link status
 * * change icon for mime type
 * * change icon for just this file
 * * change permissions (others read, others write)
 * 
 * advanced (extra) -
 * * show access date
 * * show change date
 * * show pseudolink status
 * * show pseudolink src
 * * show comment
 * * show generic
 * * show mount status
 * * show setuid bit
 * * show link destination (if symlink or link)
 * * show group
 * * change link destination
 * * change app to open THIS file with (or dir)
 * 
 */

/* PROTOTYPES - same all the time */

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

static void _cb_icon_sel(void *data, void *data2);
static void _cb_type(void *data, Evas_Object *obj, void *event_info);
static void _cb_preview_update(void *data, Evas_Object *obj, void *event_info);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   E_Fm2_Icon_Info *fi;
   struct {
      Evas_Object *icon_wid;
      Evas_Object *preview;
      Evas_Object *preview_table;
   } gui;
   /*- BASIC -*/
   char *file;
   char *size;
   char *mod_date;
   char *mime;
   char *owner;
   int owner_read;
   int owner_write;
   int others_read;
   int others_write;
   int icon_type;
   int icon_mime;
   /*- ADVANCED -*/
};

/* a nice easy setup function that does the dirty work */
EAPI E_Config_Dialog *
e_fm_prop_file(E_Container *con, E_Fm2_Icon_Info *fi)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   /* methods */
   v->create_cfdata           = _create_data;
   v->free_cfdata             = _free_data;
   v->basic.apply_cfdata      = _basic_apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->advanced.apply_cfdata   = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con,
			     _("File Properties"),
			     "E", "_fm_prop",
			     "enlightenment/file_properties", 0, v, fi);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata, E_Fm2_Icon_Info *fi)
{
   char buf[4096];
   struct passwd *pw;
   
   cfdata->fi = fi;
   if (fi->file) cfdata->file = strdup(fi->file);
   cfdata->size = e_util_size_string_get(fi->statinfo.st_size);
   cfdata->mod_date = e_util_file_time_get(fi->statinfo.st_mtime);
   if (fi->mime) cfdata->mime = strdup(fi->mime);
   pw = getpwuid(fi->statinfo.st_uid);
   if (pw) cfdata->owner = strdup(pw->pw_name);
   if (fi->statinfo.st_mode & S_IRUSR) cfdata->owner_read = 1;
   if (fi->statinfo.st_mode & S_IWUSR) cfdata->owner_write = 1;
   if (fi->statinfo.st_mode & S_IROTH) cfdata->others_read = 1;
   if (fi->statinfo.st_mode & S_IWOTH) cfdata->others_write = 1;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata, cfd->data);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Free the cfdata */
   E_FREE(cfdata->file);
   E_FREE(cfdata->size);
   E_FREE(cfdata->mod_date);
   E_FREE(cfdata->mime);
   E_FREE(cfdata->owner);
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   return 1; /* Apply was OK */
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   return 1; /* Apply was OK */
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ot, *ob, *of, *oi;
   E_Radio_Group *rg;
   char buf[4096];
   
   snprintf(buf, sizeof(buf), "%s/%s", 
	    e_fm2_real_path_get(cfdata->fi->fm), cfdata->fi->file);
   o = e_widget_table_add(evas, 0);

   ot = e_widget_table_add(evas, 0);
   
   ob = e_widget_label_add(evas, _("File:"));
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->file));
   e_widget_min_size_set(ob, 140, -1);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_table_object_append(ot, ob, 1, 0, 1, 1, 1, 0, 1, 0);
   
   ob = e_widget_label_add(evas, _("Size:"));
   e_widget_table_object_append(ot, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->size));
   e_widget_min_size_set(ob, 140, -1);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_table_object_append(ot, ob, 1, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, _("Last Modified:"));
   e_widget_table_object_append(ot, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->mod_date));
   e_widget_min_size_set(ob, 140, -1);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_table_object_append(ot, ob, 1, 2, 1, 1, 1, 0, 1, 0);

   ob = e_widget_label_add(evas, _("File Type:"));
   e_widget_table_object_append(ot, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_entry_add(evas, &(cfdata->mime));
   e_widget_min_size_set(ob, 140, -1);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_table_object_append(ot, ob, 1, 3, 1, 1, 1, 0, 1, 0);

   of = e_widget_frametable_add(evas, _("Permissions"), 0);
   ob = e_widget_label_add(evas, _("Owner:"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_entry_add(evas, &(cfdata->owner));
   e_widget_min_size_set(ob, 60, -1);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("Others can read"), &(cfdata->others_read));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("Others can write"), &(cfdata->others_write));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("Owner can read"), &(cfdata->owner_read));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("Owner can write"), &(cfdata->owner_write));
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(ot, of, 0, 4, 2, 1, 1, 0, 1, 0);
   
   e_widget_table_object_append(o, ot, 0, 0, 1, 1, 1, 1, 1, 1);
   
   of = e_widget_frametable_add(evas, _("Preview"), 0);
   
   ot = e_widget_table_add(evas, 0);
   ob = e_widget_preview_add(evas, 128, 128);
   cfdata->gui.preview = ob;
   cfdata->gui.preview_table = ot;
   evas_object_smart_callback_add(ob, "preview_update",
				  _cb_preview_update, cfdata);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 0, 0, 1, 1);
   e_widget_preview_thumb_set(ob, buf,
			      "e/desktop/background", 128, 128);
   e_widget_frametable_object_append(of, ot, 0, 0, 1, 1, 1, 1, 1, 1);
   
   e_widget_table_object_append(o, of, 1, 0, 1, 1, 1, 1, 1, 1);
   
   ot = e_widget_frametable_add(evas, _("Icon"), 0);
   
   ob = e_widget_button_add(evas, "", NULL, _cb_icon_sel, cfdata, cfd);   
   cfdata->gui.icon_wid = ob;
   if (0) /* FIXME: find icon */
     {
	oi = NULL;
	e_widget_button_icon_set(ob, oi);
     }
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(ot, ob, 0, 0, 1, 3, 1, 1, 1, 1);
   
   rg = e_widget_radio_group_new(&cfdata->icon_type);
   ob = e_widget_radio_add(evas, _("Thumbnail"), 0, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Theme Icon"), 1, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Edje File"), 2, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Image"), 3, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 2, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Default"), 4, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(ot, ob, 2, 1, 1, 1, 1, 1, 1, 1);
   
   ob = e_widget_check_add(evas, _("Use this icon for files of this type"), &(cfdata->icon_mime));
   e_widget_frametable_object_append(ot, ob, 0, 3, 3, 1, 1, 1, 1, 1);
   
   e_widget_table_object_append(o, ot, 0, 1, 2, 1, 1, 1, 1, 1);
   
   return o;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *o;
      
   o = e_widget_table_add(evas, 0);
   return o;
}





static void
_cb_icon_sel(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   /* FIXME: select an icon */
}

static void
_cb_type(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   if ((cfdata->icon_type == 2) || (cfdata->icon_type == 3))
     e_widget_disabled_set(cfdata->gui.icon_wid, 0);
   else
     e_widget_disabled_set(cfdata->gui.icon_wid, 1);
}

static void
_cb_preview_update(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;
   e_widget_table_object_repack(cfdata->gui.preview_table,
				cfdata->gui.preview,
				0, 0, 1, 1, 0, 0, 1, 1);
}
