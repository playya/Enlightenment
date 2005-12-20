#include "e.h"
#include "e_mod_main.h"

typedef struct _calendar_cfdata CFData;

struct _calendar_cfdata
{
   char *size;
   char *size1;
   char *size2;
   int temp_ImageYes;
   int temp_UserCS;

   Calendar *calendar;
};

static Evas_Object 	  *_create_widgets(E_Config_Dialog *cfd, Evas *evas, Config *cfdata);
static void 	    	 *_create_data(E_Config_Dialog *cfd);
static void 	    	  _free_data(E_Config_Dialog *cfd, void *data);
static Evas_Object  	 *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data);
static int 	     	_basic_apply_data(E_Config_Dialog *cfd, void *data);
static Evas_Object   *_color_edit_advanced_create_widgets(E_Config_Dialog *cfd,Evas *evas,void *data);
static int            _color_edit_advanced_apply_data(E_Config_Dialog *cfd, void *data);

/***************************************************
/ Function: 
/ Purpose:  
/ Returns: nothing
/ Takes: 
/
/
/
/*****************************************************/
void
e_int_config_calendar_weekend(void *con, void *calendar)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   /* methods */
   v.create_cfdata           = _create_data;
   v.free_cfdata             = _free_data;
   v.basic.apply_cfdata      = _basic_apply_data;
   v.basic.create_widgets    = _basic_create_widgets;
   v.advanced.apply_cfdata   = _color_edit_advanced_apply_data;
   v.advanced.create_widgets = _color_edit_advanced_create_widgets;

   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(con, _("Calendar Settings"), NULL, 0, &v, calendar);
}
/***************************************************
/ Function: 
/ Purpose:  
/ Returns: nothing
/ Takes: 
/
/
/
/*****************************************************/
static void *
_create_data(E_Config_Dialog *cfd)
{
   CFData *cfdata;
   cfdata = E_NEW(CFData, 1);
   cfdata->calendar = cfd->data;
   cfdata->temp_ImageYes = cfdata->calendar->conf->ImageYes;
   cfdata->temp_UserCS = cfdata->calendar->conf->UserCS;

   return cfdata;
}
/***************************************************
/ Function: 
/ Purpose:  
/ Returns: nothing
/ Takes: 
/
/
/
/*****************************************************/
static void
_free_data(E_Config_Dialog *cfd, void *data)
{
   /* Free the cfdata */
   free(data);
}
/***************************************************
/ Function: 
/ Purpose:  
/ Returns: nothing
/ Takes: 
/
/
/
/*****************************************************/
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *of, *ob, *of2, *of3, *fs, *of1,*check, *today, *weekend, *weekday, *of_y, *o2;
   Evas_Object *slider_red, *slider_green, *slider_alpha, *slider_blue;
   Evas_Object *s_r_tf, *s_g_tf, *s_a_tf, *s_b_tf, *s_r_tf_o, *s_b_tf_o, *s_g_tf_o, *s_a_tf_o, *s_r_tf_s, *s_b_tf_s, *s_g_tf_s, *s_a_tf_s, *today_f, *weekend_f, *weekday_f ;

   CFData *cfdata;
   cfdata = data;

   c_array *WeTC_Ptr = cfdata->calendar->conf->Weekend_text_colors->data; 
   c_array *WeBC_Ptr = cfdata->calendar->conf->Weekend_back_colors->data; 

   o = e_widget_table_add(evas, 0);

     //weekend colors box
    Evas_Object *we_today;
    weekend = e_widget_framelist_add(evas, _("Weekend"),1);
    we_today = e_widget_table_add(evas, 0);
    e_widget_table_object_append(we_today, e_widget_label_add(evas, _("Red:")),0,0,1,1,0,0,0,0);
    slider_red = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeBC_Ptr->red), 60);
    e_widget_table_object_append(we_today, slider_red,1,0,1,1,0,0,0,0);
    e_widget_table_object_append(we_today, e_widget_label_add(evas, _("Blue:")),2,0,1,1,0,0,0,0);
    slider_blue = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeBC_Ptr->blue), 60);
    e_widget_table_object_append(we_today, slider_blue,3,0,1,1,0,0,0,0);
    e_widget_table_object_append(we_today,e_widget_label_add(evas, _("Green:")),0,1,1,1,0,0,0,0);
    slider_green = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeBC_Ptr->green), 60);
    e_widget_table_object_append(we_today, slider_green,1,1,1,1,0,0,0,0);
    e_widget_table_object_append(we_today,e_widget_label_add(evas, _("Alpha:")),2,1,1,1,0,0,0,0);
    slider_alpha = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeBC_Ptr->alpha), 60);
    e_widget_table_object_append(we_today, slider_alpha,3,1,1,1,0,0,0,0);
    e_widget_framelist_object_append(weekend, we_today);
    e_widget_table_object_append(o, weekend, 1,1,1,1,1,1,1,1);

// weekdend Font colors
    Evas_Object *tablef_we;
    tablef_we = e_widget_table_add(evas, 0);
    weekend_f = e_widget_framelist_add(evas, _("Weekend Font"),1);
    e_widget_table_object_append(tablef_we, e_widget_label_add(evas,_("Red:")),0,0,1,1,0,0,0,0);
    s_r_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeTC_Ptr->red), 60);
    e_widget_table_object_append(tablef_we, s_r_tf,1,0,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas, _("Blue:")),2,0,1,1,0,0,0,0);
    s_b_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeTC_Ptr->blue), 60);
    e_widget_table_object_append(tablef_we, s_b_tf,3,0,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas,_("Green:")),0,1,1,1,0,0,0,0);
    s_g_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL,&(WeTC_Ptr->green), 60);
    e_widget_table_object_append(tablef_we, s_g_tf,1,1,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas,_("Alpha:")),2,1,1,1,0,0,0,0);
    s_a_tf = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeTC_Ptr->alpha), 60);
    e_widget_table_object_append(tablef_we, s_a_tf,3,1,1,1,0,0,0,0);
    e_widget_framelist_object_append(weekend_f,tablef_we);
    e_widget_table_object_append(o, weekend_f, 2,1,1,1,1,1,1,1);

   return o;
}
/***************************************************
/ Function: 
/ Purpose:  
/ Returns: nothing
/ Takes: 
/
/
/
/*****************************************************/
/**--APPLY--**/
static int
_basic_apply_data(E_Config_Dialog *cfd, void *data)
{
   CFData *cfdata;
   cfdata = data;
   /* Actually take our cfdata settings and apply them in real life */
   e_border_button_bindings_ungrab_all();
   Calendar *calendar;
   calendar = cfd->data;
   int SwitchImage = 0;

   if ((calendar->conf->ImageYes != cfdata->temp_ImageYes) || (calendar->conf->UserCS !=
							                   cfdata->temp_UserCS))
   {
	SwitchImage = 1;
	calendar->conf->ImageYes = cfdata->calendar->conf->ImageYes;
	if (calendar->conf->ImageYes != cfdata->temp_ImageYes)
	{
		if (cfdata->temp_ImageYes == 0)
	   		cfdata->temp_ImageYes = 1;
		else 
	   		cfdata->temp_ImageYes = 0;
	}
	if (calendar->conf->UserCS != cfdata->temp_UserCS)
	{
		if (cfdata->temp_UserCS)
			cfdata->temp_UserCS = FALSE;
		else
			cfdata->temp_UserCS = TRUE;
	}
   }
 
   e_config_save_queue();
   redraw_calendar(calendar,SwitchImage);
   e_border_button_bindings_grab_all();

   return 1; /* Apply was OK */
}
/***************************************************
/ Function: 
/ Purpose:  Move data from dialog to config object 
/           after "advanced" apply button presesed.
/
/*****************************************************/
static int
_color_edit_advanced_apply_data(E_Config_Dialog *cfd, void *data)
{
   CFData *cfdata;
   cfdata = data;
   /* Actually take our cfdata settings and apply them in real life */
   e_border_button_bindings_ungrab_all();
   Calendar *calendar;
   calendar = cfd->data;
   int SwitchImage = 0;

   if ((calendar->conf->ImageYes != cfdata->temp_ImageYes) || (calendar->conf->UserCS !=
							                   cfdata->temp_UserCS))
   {
	SwitchImage = 1;
	calendar->conf->ImageYes = cfdata->calendar->conf->ImageYes;
	if (calendar->conf->ImageYes != cfdata->temp_ImageYes)
	{
		if (cfdata->temp_ImageYes == 0)
	   		cfdata->temp_ImageYes = 1;
		else 
	   		cfdata->temp_ImageYes = 0;
	}
	if (calendar->conf->UserCS != cfdata->temp_UserCS)
	{
		if (cfdata->temp_UserCS)
			cfdata->temp_UserCS = FALSE;
		else
			cfdata->temp_UserCS = TRUE;
	}
   }
 
   e_config_save_queue();
   redraw_calendar(calendar,SwitchImage);
   e_border_button_bindings_grab_all();

   return 1; /* Apply was OK */
}
/***************************************************
/ Function: 
/ Purpose:  create widgets for advanced dialog
/
/
/*****************************************************/
static Evas_Object *
_color_edit_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *of, *ob, *of2, *of3, *fs, *of1,*check, *today, *weekend, *weekday, *of_y, *o2;
   Evas_Object *slider_red, *slider_green, *slider_alpha, *slider_blue;
   Evas_Object *s_r_tf, *s_g_tf, *s_a_tf, *s_b_tf, *s_r_tf_o, *s_b_tf_o, *s_g_tf_o, *s_a_tf_o, *s_r_tf_s, *s_b_tf_s, *s_g_tf_s, *s_a_tf_s, *today_f, *weekend_f, *weekday_f ;
   Evas_Object *ym, *day_text, *start_table;

   CFData *cfdata;
   cfdata = data;

   c_array *WeTC_Ptr = cfdata->calendar->conf->Weekend_text_colors->data; 
   c_array *WeBC_Ptr = cfdata->calendar->conf->Weekend_back_colors->data; 

   o = e_widget_table_add(evas, 0);

     //weekend colors box
    Evas_Object *we_today;
    weekend = e_widget_framelist_add(evas, _("Weekend"),1);
    we_today = e_widget_table_add(evas, 0);
    e_widget_table_object_append(we_today, e_widget_label_add(evas, _("Red:")),0,0,1,1,0,0,0,0);
    slider_red = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeBC_Ptr->red), 60);
    e_widget_table_object_append(we_today, slider_red,1,0,1,1,0,0,0,0);
    e_widget_table_object_append(we_today, e_widget_label_add(evas, _("Blue:")),2,0,1,1,0,0,0,0);
    slider_blue = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeBC_Ptr->blue), 60);
    e_widget_table_object_append(we_today, slider_blue,3,0,1,1,0,0,0,0);
    e_widget_table_object_append(we_today,e_widget_label_add(evas, _("Green:")),0,1,1,1,0,0,0,0);
    slider_green = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeBC_Ptr->green), 60);
    e_widget_table_object_append(we_today, slider_green,1,1,1,1,0,0,0,0);
    e_widget_table_object_append(we_today,e_widget_label_add(evas, _("Alpha:")),2,1,1,1,0,0,0,0);
    slider_alpha = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeBC_Ptr->alpha), 60);
    e_widget_table_object_append(we_today, slider_alpha,3,1,1,1,0,0,0,0);
    e_widget_framelist_object_append(weekend, we_today);
    e_widget_table_object_append(o, weekend, 1,1,1,1,1,1,1,1);

// weekdend Font colors
    Evas_Object *tablef_we;
    tablef_we = e_widget_table_add(evas, 0);
    weekend_f = e_widget_framelist_add(evas, _("Weekend Font"),1);
    e_widget_table_object_append(tablef_we, e_widget_label_add(evas,_("Red:")),0,0,1,1,0,0,0,0);
    s_r_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeTC_Ptr->red), 60);
    e_widget_table_object_append(tablef_we, s_r_tf,1,0,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas, _("Blue:")),2,0,1,1,0,0,0,0);
    s_b_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL, &(WeTC_Ptr->blue), 60);
    e_widget_table_object_append(tablef_we, s_b_tf,3,0,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas,_("Green:")),0,1,1,1,0,0,0,0);
    s_g_tf = e_widget_slider_add(evas, 1, 0, _("%1.0f"),0,254,1,0,NULL,&(WeTC_Ptr->green), 60);
    e_widget_table_object_append(tablef_we, s_g_tf,1,1,1,1,0,0,0,0);
    e_widget_table_object_append(tablef_we,e_widget_label_add(evas,_("Alpha:")),2,1,1,1,0,0,0,0);
    s_a_tf = e_widget_slider_add(evas,1,0,_("%1.0f"),0,254,1,0,NULL,&(WeTC_Ptr->alpha), 60);
    e_widget_table_object_append(tablef_we, s_a_tf,3,1,1,1,0,0,0,0);

  e_widget_table_object_append(tablef_we, e_widget_label_add(evas,
						_("R Outline:")),0,2,1,1,0,0,0,0);
  s_r_tf_o = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL, &(WeTC_Ptr->red), 60);
  e_widget_table_object_append(tablef_we, s_r_tf_o,1,2,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas,
 						_("B Outline:")),2,2,1,1,0,0,0,0);
  s_b_tf_o = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL, &(WeTC_Ptr->blue), 60);
  e_widget_table_object_append(tablef_we, s_b_tf_o,3,2,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("G Outline:")),0,3,1,1,0,0,0,0);
  s_g_tf_o = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL,&(WeTC_Ptr->green), 60);
  e_widget_table_object_append(tablef_we, s_g_tf_o,1,3,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("A Outline:")),2,3,1,1,0,0,0,0);
  s_a_tf_o = e_widget_slider_add(evas,1,0,_("%1.2f"),0,254,1,0,NULL,&(WeTC_Ptr->alpha), 60);
  e_widget_table_object_append(tablef_we, s_a_tf_o,3,3,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("R Shadow:")),0,4,1,1,0,0,0,0);
  s_r_tf_s = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL, &(WeTC_Ptr->red), 60);
  e_widget_table_object_append(tablef_we, s_r_tf_s,1,4,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("B Shadow:")),2,4,1,1,0,0,0,0);
  s_b_tf_s = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL, &(WeTC_Ptr->blue), 60);
  e_widget_table_object_append(tablef_we, s_b_tf_s,3,4,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("G Shadow:")),0,5,1,1,0,0,0,0);
  s_g_tf_s = e_widget_slider_add(evas, 1, 0, _("%1.2f"),0,254,1,0,NULL,&(WeTC_Ptr->green), 60);
  e_widget_table_object_append(tablef_we, s_g_tf_s,1,5,1,1,0,0,0,0);
  e_widget_table_object_append(tablef_we, e_widget_label_add(evas, 
						_("A Shadow:")),2,5,1,1,0,0,0,0);
  s_a_tf_s = e_widget_slider_add(evas,1,0,_("%1.2f"),0,254,1,0,NULL,&(WeTC_Ptr->alpha), 60);
  e_widget_table_object_append(tablef_we, s_a_tf_s,3,5,1,1,0,0,0,0);

    e_widget_framelist_object_append(weekend_f,tablef_we);
    e_widget_table_object_append(o, weekend_f, 2,1,1,1,1,1,1,1);


   return o;
}