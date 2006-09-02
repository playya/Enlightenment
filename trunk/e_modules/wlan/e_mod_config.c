#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   char *device;
   Ecore_List *devs;
   int dev_num;
   
  double poll_time;
  int always_text;
  int show_percent;
};

/* Protos */
static void *_create_data (E_Config_Dialog * cfd);
static void _free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static Evas_Object *_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
					   E_Config_Dialog_Data * cfdata);
static int _basic_apply_data (E_Config_Dialog * cfd,
			      E_Config_Dialog_Data * cfdata);
static void _fill_data (Config_Item * ci, E_Config_Dialog_Data * cfdata);
static void _wlan_config_get_devices(Ecore_List *devs);

/* Config Calls */
void
_config_wlan_module (Config_Item * ci)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_View *v;
  E_Container *con;
  char buf[4096];

  v = E_NEW (E_Config_Dialog_View, 1);

  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.apply_cfdata = _basic_apply_data;
  v->basic.create_widgets = _basic_create_widgets;

  snprintf (buf, sizeof (buf), "%s/module.eap",
	    e_module_dir_get (wlan_config->module));
  con = e_container_current_get (e_manager_current_get ());
  cfd = e_config_dialog_new (con, D_ ("Wlan Configuration"), "Wlan", "_e_modules_wlan_config_dialog", buf, 0, v, ci);
  wlan_config->config_dialog = cfd;
}

static void
_fill_data (Config_Item * ci, E_Config_Dialog_Data * cfdata)
{
   char *tmp;
   int i = 0;
   
  cfdata->poll_time = ci->poll_time;
  cfdata->always_text = ci->always_text;
  cfdata->show_percent = ci->show_percent;
   
   cfdata->device = NULL;
   if (ci->device != NULL) 
     cfdata->device = strdup(ci->device);
   
   if (!cfdata->device) return;
   
   cfdata->devs = ecore_list_new();
   _wlan_config_get_devices(cfdata->devs);
   if (!cfdata->devs) return;

   ecore_list_goto_first(cfdata->devs);
   while ((tmp = ecore_list_next(cfdata->devs)) != NULL) 
     {
	if (!strcmp(cfdata->device, tmp)) 
	  {
	     cfdata->dev_num = i;
	     break;
	  }
	i++;
     }
}

static void *
_create_data (E_Config_Dialog * cfd)
{
  E_Config_Dialog_Data *cfdata;
  Config_Item *ci;

  ci = cfd->data;
  cfdata = E_NEW (E_Config_Dialog_Data, 1);
  
  _fill_data (ci, cfdata);
  return cfdata;
}

static void
_free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  if (!wlan_config)
    return;
  wlan_config->config_dialog = NULL;
   E_FREE(cfdata->device);
   if (cfdata->devs)
     ecore_list_destroy(cfdata->devs);
   
  free (cfdata);
  cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
		       E_Config_Dialog_Data * cfdata)
{
  Evas_Object *o, *of, *ob;
  E_Radio_Group *rg;
   char *tmp;
   int i = 0;
   
  o = e_widget_list_add (evas, 0, 0);
  of = e_widget_framelist_add (evas, D_ ("General Settings"), 0);
  rg = e_widget_radio_group_new (&(cfdata->always_text));
  ob = e_widget_radio_add (evas, D_ ("Always Show Text"), 1, rg);
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_radio_add (evas, D_ ("Show Text On Mouse Over"), 0, rg);
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_check_add (evas, D_ ("Show Text As Percent"),
			&(cfdata->show_percent));
  e_widget_framelist_object_append (of, ob);

  ob = e_widget_label_add (evas, D_ ("Check Interval:"));
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_slider_add (evas, 1, 0, _("%1.0f seconds"), 1.0, 60.0, 1.0, 0,
			 &(cfdata->poll_time), NULL, 150);
  e_widget_framelist_object_append (of, ob);
  e_widget_list_object_append (o, of, 1, 1, 0.5);

   if (cfdata->devs) 
     {
	of = e_widget_framelist_add (evas, D_ ("Device Settings"), 0);
	rg = e_widget_radio_group_new(&(cfdata->dev_num));
	ecore_list_goto_first(cfdata->devs);
	while ((tmp = ecore_list_next(cfdata->devs)) != NULL) 
	  {
	     ob = e_widget_radio_add(evas, tmp, i, rg);
	     e_widget_framelist_object_append (of, ob);
	     i++;

	  }
	e_widget_list_object_append (o, of, 1, 1, 0.5);
     }
   
  return o;
}

static int
_basic_apply_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  Config_Item *ci;
   char *tmp;
   
  ci = cfd->data;
  ci->poll_time = cfdata->poll_time;

  ci->always_text = cfdata->always_text;
  ci->show_percent = cfdata->show_percent;
   
   if (cfdata->devs) 
     {
	tmp = ecore_list_goto_index(cfdata->devs, cfdata->dev_num);
	if (tmp != NULL) 
	  {
	     if (ci->device) evas_stringshare_del(ci->device);

	     ci->device = evas_stringshare_add(tmp);
	     E_FREE(tmp);
	  }
     }
   
  e_config_save_queue ();
  _wlan_config_updated (ci->id);

  return 1;
}

static void 
_wlan_config_get_devices(Ecore_List *devs) 
{
  FILE *stat;
  char dev[64];
  char buf[256];
  unsigned long dummy;

  stat = fopen ("/proc/net/wireless", "r");
  if (!stat)
    return;

  while (fgets (buf, 256, stat))
    {
      int i = 0;

      for (; buf[i] != 0; i++)
	{
	  if (buf[i] == ':')
	    buf[i] = ' ';
	}	
      if (sscanf (buf, "%s %lu %f %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
		  "%lu %lu %lu %lu\n", dev, &dummy, &dummy, &dummy,
		  &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
		  &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) < 4)
	continue;
      ecore_list_append (devs, strdup (dev));
    }
  fclose (stat);   
}
