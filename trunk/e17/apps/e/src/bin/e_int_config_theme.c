/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* PROTOTYPES - same all the time */
typedef struct _E_Cfg_Theme_Data E_Cfg_Theme_Data;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   char *theme;
   char *current_theme;
   /*- ADVANCED -*/

};

struct _E_Cfg_Theme_Data
{
   E_Config_Dialog *cfd;
   char *file;
   char *theme;
};

/* a nice easy setup function that does the dirty work */
EAPI E_Config_Dialog *
e_int_config_theme(E_Container *con)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   
   /* methods */
   v.create_cfdata           = _create_data;
   v.free_cfdata             = _free_data;
   v.basic.apply_cfdata      = _basic_apply_data;
   v.basic.create_widgets    = _basic_create_widgets;
   v.advanced.apply_cfdata   = NULL;
   v.advanced.create_widgets = NULL; 
   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Theme Selector"), NULL, 0, &v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   /* get current theme */
   E_Config_Theme * c;
   c = e_theme_config_get("theme");
   cfdata->current_theme = strdup(c->file);
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
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   /* Free the cfdata */  
   free(cfdata->current_theme);
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Action *a;
   
   /* Actually take our cfdata settings and apply them in real life */
   e_theme_config_set("theme", cfdata->theme);
   e_config_save_queue();

   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);   
   return 1; /* Apply was OK */
}

void
_e_config_theme_cb_standard(void *data)
{
   E_Cfg_Theme_Data *d;
   E_Config_Dialog_Data *cfdata;
   const char *tmp;
   
   d = data;
   e_widget_image_object_set(d->cfd->data, e_thumb_evas_object_get(d->file, d->cfd->dia->win->evas, 160, 120, 1));
   
   cfdata = d->cfd->cfdata;
   if (cfdata->current_theme) 
     {
	tmp = ecore_file_get_file(d->file);
	tmp = ecore_file_strip_ext(tmp);
	if (!strcmp(tmp, cfdata->current_theme)) 
	  {
	     e_dialog_button_disable_num_set(d->cfd->dia, 0, 1);
	     e_dialog_button_disable_num_set(d->cfd->dia, 1, 1);	
	  }
     }
}

/**--GUI--**/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *fr, *im = NULL;
   Evas_Object *il;
   char buf[4096];
   char *homedir;
   Evas_Object *theme;
   char fulltheme[PATH_MAX];
   
   theme = edje_object_add(evas);
   
   _fill_data(cfdata);
   cfdata->theme = NULL;
   
   o = e_widget_table_add(evas, 0);
   
   il = e_widget_ilist_add(evas, 48, 48, &(cfdata->theme));
   homedir = e_user_homedir_get();
   if (homedir)
     {
	snprintf(buf, sizeof(buf), "%s/.e/e/themes", homedir);
	free(homedir);
     }

   if (ecore_file_is_dir(buf))
     {
	Ecore_List *themes;
	
	themes = ecore_file_ls(buf);
	if (themes)
	  {
	     /* add default theme */
	     ecore_list_prepend(themes, strdup("default.edj"));
	     ecore_list_goto_first(themes);
	     
	     Evas_Object *o;
	     Ecore_Evas *eebuf;
	     Evas *evasbuf;
	     int i = 0;
	     char *themefile;
	     
	     eebuf = ecore_evas_buffer_new(1, 1);
	     evasbuf = ecore_evas_get(eebuf);
	     o = edje_object_add(evasbuf);
		  
	     while ((themefile = ecore_list_next(themes)))
	       {
		  if (!strcmp(themefile, "default.edj"))
		    snprintf(fulltheme, sizeof(fulltheme), PACKAGE_DATA_DIR"/data/themes/default.edj");
		  else
		    snprintf(fulltheme, sizeof(fulltheme), "%s/%s", buf, themefile);
		  if (ecore_file_is_dir(fulltheme))
		    continue;
		  
		  /* minimum theme requirements */
		  if (edje_object_file_set(o, fulltheme, "desktop/background"))
		    {
		       Evas_Object *o = NULL;
		       char *noext;
		       E_Cfg_Theme_Data *cb_data;
		       
		       o = e_thumb_generate_begin(fulltheme, 48, 48, cfd->dia->win->evas, &o, NULL, NULL);
		       noext = ecore_file_strip_ext(themefile);
		       
		       cb_data = E_NEW(E_Cfg_Theme_Data, 1);
		       cb_data->cfd = cfd;
		       cb_data->file = strdup(fulltheme);
		       cb_data->theme = strdup(themefile);
		       
		       e_widget_ilist_append(il, o, noext, _e_config_theme_cb_standard, cb_data, themefile);
		       if (!(strcmp(themefile, cfdata->current_theme)))
			 {
			    e_widget_ilist_selected_set(il, i);
			    im = e_widget_image_add_from_object(evas, theme, 320, 240);
			    e_widget_image_object_set(im, e_thumb_evas_object_get(fulltheme, evas, 320, 240, 1));
			 }
		       free(noext);
		       i++;
		    }
	       }
	     free(themefile);
	     evas_object_del(o);
	     ecore_evas_free(eebuf);
	     ecore_list_destroy(themes);
	  }
     }

   e_widget_ilist_go(il);   
   e_widget_min_size_set(il, 180, 40);
   e_widget_table_object_append(o, il, 0, 0, 1, 2, 1, 1, 1, 1);
   fr = e_widget_framelist_add(evas, "Preview", 0);
   if (im == NULL)
     {
	snprintf(fulltheme, sizeof(fulltheme), PACKAGE_DATA_DIR"/data/themes/default.edj");
	theme = e_thumb_generate_begin(fulltheme, 320, 240, evas, &theme, NULL, NULL);
	im = e_widget_image_add_from_object(evas, theme, 320, 240);
	e_widget_image_object_set(im, e_thumb_evas_object_get(fulltheme, evas, 320, 240, 1));
     }
   cfd->data = im;
   e_widget_min_size_set(fr, 320, 240);
   e_widget_table_object_append(o, fr, 1, 0, 1, 2, 1, 1, 1, 1);   
   e_widget_framelist_object_append(fr, im);   
   
   return o;
}

