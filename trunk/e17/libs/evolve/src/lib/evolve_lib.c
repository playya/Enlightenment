#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "evolve_private.h"

#include <Evas_Engine_Buffer.h>  

#define NEWD(str, typ) \
     eet_data_descriptor_new(str, sizeof(typ), \
				(void *(*) (void *))evas_list_next, \
				(void *(*) (void *, void *))evas_list_append, \
				(void *(*) (void *))evas_list_data, \
				(void *(*) (void *))evas_list_free, \
				(void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach, \
				(void *(*) (void *, const char *, void *))evas_hash_add, \
				(void  (*) (void *))evas_hash_free)

#define FREED(eed) \
	 if (eed) \
	     { \
		eet_data_descriptor_free((eed)); \
		(eed) = NULL; \
	     }

#define EVOLVE_WIDGET_PROP_NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evolve_widget_prop_edd, Etk_Property, str, it, type)

#define EVOLVE_WIDGET_PROP_NEWS(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_SUB(_evolve_widget_prop_edd, Etk_Property, str, it, type)

#define PROP_DEF_VAL_NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_prop_def_val_edd, Etk_Property_Value, str, it, type)
  
#define PROP_DEF_VAL_NEWS(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_SUB(_prop_def_val_edd, Etk_Property_Value, str, it, type)

#define PROP_DEF_VAL_VALUE_NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_prop_def_val_value_edd, Etk_Property_Value, str, it, type)

#define EVOLVE_WIDGET_PACKING_PROP_NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evolve_widget_packing_prop_edd, Evolve_Widget_Property, str, it, type)

#define EVOLVE_WIDGET_SIGNAL_NEWI(str, it, type) \
     EET_DATA_DESCRIPTOR_ADD_BASIC(_evolve_widget_signal_edd, Evolve_Widget_Signal, str, it, type)

#define EVOLVE_WIDGET_NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evolve_widget_edd, Evolve_Widget, str, it, type)

#define EVOLVE_WIDGET_PROPS_NEWH(str, it, type) \
     EET_DATA_DESCRIPTOR_ADD_HASH(_evolve_widget_edd, Evolve_Widget, str, it, type)

#define EVOLVE_WIDGET_PACKING_PROPS_NEWH(str, it, type) \
     EET_DATA_DESCRIPTOR_ADD_HASH(_evolve_widget_edd, Evolve_Widget, str, it, type)

#define EVOLVE_WIDGET_SIGNALS_NEWL(str, it, type) \
     EET_DATA_DESCRIPTOR_ADD_LIST(_evolve_widget_edd, Evolve_Widget, str, it, type)

#define EVOLVE_WIDGETS_NEWL(str, it, type) \
     EET_DATA_DESCRIPTOR_ADD_LIST(_evolve_widgets_edd, Evolve, str, it, type)

Evas_List *_evolve_defines = NULL;
extern Etk_String *_evolve_edje_code;
extern FILE *yyin;
int yyparse(void);
Evas_List *_evolve_widgets_show_all = NULL;
Evolve *_evolve_ev = NULL;
Evas_List *widgets = NULL;
char *evolve_filename = NULL;

static Eet_Data_Descriptor *_evolve_widget_prop_edd = NULL;
static Eet_Data_Descriptor *_prop_def_val_edd = NULL;
static Eet_Data_Descriptor *_prop_def_val_value_edd = NULL;
static Eet_Data_Descriptor *_evolve_widget_packing_prop_edd = NULL;
static Eet_Data_Descriptor *_evolve_widget_signal_edd = NULL;
static Eet_Data_Descriptor *_evolve_widget_edd = NULL;
static Eet_Data_Descriptor *_evolve_widgets_edd = NULL;

static Evas_Bool _evolve_print_props_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata);


/* initialize evolve and return 1 on success, 0 otherwise */
int evolve_init()
{
   eet_init();   
   evas_init();
   ecore_file_init();
   
   _prop_def_val_value_edd = NEWD("Etk_Property_Value", Etk_Property_Value);
   PROP_DEF_VAL_VALUE_NEWI("iv", value.int_value, EET_T_INT);
   PROP_DEF_VAL_VALUE_NEWI("ib", value.bool_value, EET_T_INT);
   PROP_DEF_VAL_VALUE_NEWI("cv", value.char_value, EET_T_CHAR);
   PROP_DEF_VAL_VALUE_NEWI("fv", value.float_value, EET_T_FLOAT);
   PROP_DEF_VAL_VALUE_NEWI("dv", value.double_value, EET_T_DOUBLE);
   PROP_DEF_VAL_VALUE_NEWI("sv", value.short_value, EET_T_SHORT);
   PROP_DEF_VAL_VALUE_NEWI("lv", value.long_value, EET_T_LONG_LONG);
   PROP_DEF_VAL_VALUE_NEWI("tv", value.string_value, EET_T_STRING);
   
   _prop_def_val_edd = NEWD("Etk_Property_Value", Etk_Property_Value);
   PROP_DEF_VAL_NEWS("va", value, _prop_def_val_value_edd);
   PROP_DEF_VAL_NEWI("ty", type, EET_T_INT);   
   
   _evolve_widget_prop_edd = NEWD("Etk_Property", Etk_Property);
   EVOLVE_WIDGET_PROP_NEWI("nm", name, EET_T_STRING);
   EVOLVE_WIDGET_PROP_NEWI("id", id, EET_T_INT);
   EVOLVE_WIDGET_PROP_NEWI("tp", type, EET_T_INT);
   EVOLVE_WIDGET_PROP_NEWI("fl", flags, EET_T_INT);
   EVOLVE_WIDGET_PROP_NEWS("dv", default_value, _prop_def_val_edd);
   
   _evolve_widget_packing_prop_edd = NEWD("Evolve_Widget_Property", Evolve_Widget_Property);
   EVOLVE_WIDGET_PACKING_PROP_NEWI("nm", name, EET_T_STRING);   
   EVOLVE_WIDGET_PACKING_PROP_NEWI("vl", value, EET_T_STRING);
   EVOLVE_WIDGET_PACKING_PROP_NEWI("tp", type, EET_T_INT);   
      
   _evolve_widget_signal_edd = NEWD("Evolve_Widget_Signal", Evolve_Widget_Signal);
   EVOLVE_WIDGET_SIGNAL_NEWI("nm", name, EET_T_STRING);
   EVOLVE_WIDGET_SIGNAL_NEWI("cb", callback, EET_T_STRING);
   EVOLVE_WIDGET_SIGNAL_NEWI("em", emit, EET_T_STRING);   
   EVOLVE_WIDGET_SIGNAL_NEWI("sw", swapped, EET_T_INT);
   EVOLVE_WIDGET_SIGNAL_NEWI("dt", data_type, EET_T_STRING);
   EVOLVE_WIDGET_SIGNAL_NEWI("dn", data_name, EET_T_STRING);   
   
   _evolve_widget_edd = NEWD("Evolve_Widget", Evolve_Widget);
   EVOLVE_WIDGET_NEWI("tp", type, EET_T_STRING);
   EVOLVE_WIDGET_NEWI("nm", name, EET_T_STRING);
   EVOLVE_WIDGET_NEWI("pt", parent, EET_T_STRING);   
   EVOLVE_WIDGET_PROPS_NEWH("pr", props, _evolve_widget_prop_edd);
   EVOLVE_WIDGET_PACKING_PROPS_NEWH("pp", packing_props, _evolve_widget_packing_prop_edd);
   EVOLVE_WIDGET_SIGNALS_NEWL("sg", signals, _evolve_widget_signal_edd);

   _evolve_widgets_edd = NEWD("Evolve", Evolve);
   EVOLVE_WIDGETS_NEWL("ws", widgets, _evolve_widget_edd);   

   evolve_widget_types_populate();   
   evolve_widget_packing_info_populate();
   
   return 1;
}

/* shutdown evolve and return 1 on success, 0 otherwise */
int evolve_shutdown()
{
   FREED(_evolve_widget_prop_edd);
   FREED(_prop_def_val_edd);
   FREED(_prop_def_val_value_edd);
   FREED(_evolve_widget_packing_prop_edd);   
   FREED(_evolve_widget_signal_edd);   
   FREED(_evolve_widget_edd);
   FREED(_evolve_widgets_edd);
   evas_shutdown();
   eet_shutdown();
   return 1;
}

/* print out an evolve struct, useful for debugging */
void evolve_print(Evolve *evolve)
{
   Evas_List *l;
   
   if (!evolve || !evolve->widgets)
     return;
   
   printf("Found %d widgets:\n", evas_list_count(evolve->widgets));
   
   for(l = evolve->widgets; l; l = l->next)
     {
	Evolve_Widget *widget;
	
	widget = l->data;
	printf("widget:\n"
	       "type: %s\n"
	       "name: %s\n"
	       "parent: %s\n",
	       widget->type, widget->name, widget->parent);

	if (widget->props)
	  {
	     printf("properties:\n");
	     evas_hash_foreach(widget->props, _evolve_print_props_foreach, NULL);
	  }

	if (widget->packing_props)
	  {
	     printf("packing properties:\n");
	     evas_hash_foreach(widget->packing_props, _evolve_print_props_foreach, NULL);
	  }	
	
	if (widget->signals)
	  {
	     Evas_List *l;
	     
	     printf("signals:\n");
	     
	     for (l = widget->signals; l; l = l->next)
	       {
		  Evolve_Widget_Signal *sig;
		  
		  sig = l->data;
		  printf("%s: %s\n", sig->name, sig->callback);
	       }
	  }
	
	printf("\n");
     }
}

/* render an evolve struct and display it */
void evolve_render(Evolve *evolve)
{
   Evas_List *l;  
   
   if (!evolve || !evolve->widgets)
     return;

   _evolve_ev = evolve;
         
   /* render widgets and pack them into their parents */
   for (l = evolve->widgets; l; l = l->next)
     {
	Evolve_Widget *widget;
	
	widget = l->data;
	evolve->parents = evas_hash_add(evolve->parents, widget->name, widget);
	evolve_widget_render(widget);
	evolve_widget_properties_apply(widget);
	if (widget->parent)
	  evolve_widget_reparent(widget, evas_hash_find(evolve->parents, widget->parent));
     }
   
   /* connect signals and do post render operations */
   for (l = evolve->widgets; l; l = l->next)
     {
	Evolve_Widget *widget;
	
	widget = l->data;
	evolve_widget_signals_connect(widget, evolve);
	evolve_widget_post_render(widget);
     }
   
   /* show top level widgets that want to be shown */
   for (l = _evolve_widgets_show_all; l; l = l->next)
     {
	Evolve_Widget *widget;
	
	widget = l->data;
	etk_widget_show_all(widget->widget);
     }
   
   _evolve_ev = NULL;
}

/* return the evolve code for an evolve struc */
char *evolve_code_get(Evolve *evolve)
{
   Evas_List *l;
   Etk_String *code;
   char *ret;
   
   if (!evolve || !evolve->widgets)
     return NULL;
   
   code = etk_string_new("/* This file has been generated by Evolve */\n");
   for (l = evolve->widgets; l; l = l->next)
     {
	Evolve_Widget *widget;
	char *c;
	
	widget = l->data;
	if (!(c = evolve_widget_code_get(widget)))
	  continue;
	
	code = etk_string_append(code, c);
     }
   
   ret = strdup(etk_string_get(code));
   etk_object_destroy(ETK_OBJECT(code));
   return ret;
}

/* save an evovle struct to an eet file */
int evolve_eet_save(Evolve *evolve, char *file)
{
   Eet_File  *ef;
   Evas *evas = NULL;
   Evas_List *l;
   int ret;

   if (!evolve || !evolve->widgets)
     return 0;
   
   if (_evolve_edje_code)
     {
	char edje_cc[PATH_MAX];
	char tmpf[4096];	
	int fd;
	
	strcpy(tmpf, "/tmp/evolve_edc.etk-tmp-XXXXXX");
	fd = mkstemp(tmpf);
	if (fd >= 0)
	  {
	     FILE *fp;
	     
	     if ((fp = fopen(tmpf, "w")))
	       {
		  fputs(etk_string_get(_evolve_edje_code), fp);
		  fclose(fp);
		  snprintf(edje_cc, sizeof(edje_cc), "edje_cc %s -o %s\n", tmpf, file);
		  system(edje_cc);
	       }
	     close(fd);
	  }
	else
	  fprintf(stderr, "EVOLVE WARNING: Could not create temporary file %s!\n"
		  "Evolve could not encode Edje data into compiled file.\n"
		  , tmpf);
	etk_object_destroy(ETK_OBJECT(_evolve_edje_code));
	unlink(tmpf);
	ef = eet_open(file, EET_FILE_MODE_READ_WRITE);	
     }
   else
     ef = eet_open(file, EET_FILE_MODE_WRITE);
   if (!ef)
     return 0;

   /* Add any images used by image objects into the eet */
   for (l = evolve->widgets ;l; l = l->next)
     {
	Evolve_Widget *widget;
	Etk_Property *prop;
	Evas_Object *image;
	void *image_data = NULL;
	int image_w;
	int image_h;
	char key[PATH_MAX];
	
	widget = l->data;
	if (!widget || !widget->type || !widget->props)
	  continue;
	
	if (!strcmp(widget->type, "image"))
	  {
	     if ( !(prop = evas_hash_find(widget->props, "file")) ||
		  !prop->default_value || !ecore_file_exists(etk_property_value_string_get(prop->default_value)))
	       continue;
	  }
	else if (!strcmp(widget->type, "button"))
	  {
	     if ( !(prop = evas_hash_find(widget->props, "image")) ||
		  !prop->default_value || !ecore_file_exists(etk_property_value_string_get(prop->default_value)))
	       continue;	     
	  }
	else
	  continue;
	
	if (!evas)
	  {
	     Evas_Engine_Info_Buffer*        einfo;
	     
	     evas = evas_new();
	     evas_output_method_set(evas, evas_render_method_lookup("buffer"));
	     einfo = (Evas_Engine_Info_Buffer *) evas_engine_info_get(evas);
	     einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
	     einfo->info.dest_buffer = NULL;
	     einfo->info.dest_buffer_row_bytes = 0;
	     einfo->info.use_color_key = 0;
	     einfo->info.alpha_threshold = 128;
	     einfo->info.color_key_r = 0xff;
	     einfo->info.color_key_g = 0x00;
	     /* FIXME: Strange. */
	     einfo->info.color_key_b = 0xff;
	     evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);	     
	  }
	
	image = evas_object_image_add(evas);
	evas_object_image_file_set(image, etk_property_value_string_get(prop->default_value), NULL);
	image_data = evas_object_image_data_get(image, 1);
	
	if (!image_data)
	  {
	     evas_object_del(image);
	     continue;	     
	  }
	
	snprintf(key, sizeof(key), "/etk/images/%s", etk_property_value_string_get(prop->default_value));
	evas_object_image_size_get(image, &image_w, &image_h);

	ret = eet_data_image_write(ef, key, image_data, image_w, image_h, 1, 1, 100, 0);
	if (!ret)
	  fprintf(stderr, "Error writing image to eet: %s\n", etk_property_value_string_get(prop->default_value));
	evas_object_del(image);	
     }
      
   ret = eet_data_write(ef, _evolve_widgets_edd, "/etk/widgets", evolve, 1);
   eet_close(ef);   
   if (!ret)
     {
	printf("Problem saving data!\n");
	return 0;
     }
   
   return 1;
}

/* load an evolve struct from an evolve eet file */
Evolve *evolve_eet_load(char *file)
{
   Eet_File *ef;
   Evolve *evolve;
   
   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     return 0;
   
   evolve = eet_data_read(ef, _evolve_widgets_edd, "/etk/widgets");
   eet_close(ef);
   
   if(!evolve->widgets)
     {
	printf("Problem loading data!\n");
	return NULL;
     }
   
   evolve->eet_filename = strdup(file);
   return evolve;
}

/* load an evolve struct from an etk file */
Evolve *evolve_etk_load(char *file)
{
   Evolve *evolve;
   
   int fd;
   char buf[4096];
   char tmpf[4096];
   
   if (!file) return NULL;
   strcpy(tmpf, "/tmp/evolve_parse.etk-tmp-XXXXXX");
   fd = mkstemp(tmpf);
   if (fd >= 0)
     {
	int ret;
	char *path, *t;
	char *def;
	
	path = strdup(file);
	t = strrchr(path, '/');
	if (t) {
	   *t = '\0';
	} else {
	   free(path);
	   path = strdup(".");
	}

	if (!_evolve_defines)
	  def = strdup("");
	else
	  {
	     Evas_List *l;
	     int len;

	     len = 0;
	     for (l = _evolve_defines; l; l = l->next)
	       {
		  len += strlen(l->data) + 1;
	       }
	     def = malloc(len + 1);
	     def[0] = 0;
	     for (l = _evolve_defines; l; l = l->next)
	       {
		  strcat(def, l->data);
		  strcat(def, " ");
	       }
	  }

	snprintf(buf, sizeof(buf), "cat %s | cpp -E -I%s %s -o %s", file, path, def, tmpf);
	ret = system(buf);
	if (ret < 0)
	  {
	     snprintf(buf, sizeof(buf), "gcc -E -I%s %s -o %s %s", path, def, tmpf, file);
	     ret = system(buf);
	  }
	free(path);
	free(def);
	if (ret >= 0) file = tmpf;
	close(fd);
     }
   
   yyin = fopen(tmpf, "r");
   if (!yyin)
     {
	fprintf(stderr, "Problem loading etk file: %s\n", file);
	return NULL;
     }
   free(evolve_filename);
   evolve_filename = strdup(file);
   yyparse();
   fclose(yyin);
   unlink(tmpf);
   
   evolve = calloc(1, sizeof(Evolve));
   evolve->widgets = evolve_widget_list_sort(widgets);
   return evolve;
}

/* find and return the etk  widget given an evolve widget name */
Etk_Widget *evolve_etk_widget_find(Evolve *evolve, char *name)
{
   Evas_List *l;
   
   if (!evolve || !evolve->widgets)
     return NULL;
   
   for (l = evolve->widgets; l; l = l->next)
     {
	Evolve_Widget *widget;
	
	widget = l->data;
	
	if (widget->name && !strcmp(widget->name, name))
	  return widget->widget;
     }
   
   return NULL;
}

/* set the list of defines for use with evolve */
void evolve_defines_set(Evas_List *defines)
{
   _evolve_defines = defines;
}

static Evas_Bool _evolve_print_props_foreach(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Evolve_Widget_Property *prop;
   
   prop = data;
   printf("  %s: %s\n", key, prop->value);
   return 1;
}
