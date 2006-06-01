#include <e.h>
#include "e_mod_main.h"
#include "e_mod_lang.h"
#include "e_mod_config.h"
#include "config.h"

#define ILIST_ICON_LANG_WIDTH   35
#define ILIST_ICON_LANG_HEIGHT  35

#define ILIST_ICON_WIDTH   32
#define ILIST_ICON_HEIGHT  32

#define ILIST_LANGUAGE_LABEL_FORMAT "%s (%s)"

struct _E_Config_Dialog_Data
{
   lang_switch_policy_t	lang_policy;
   int			lang_show_indicator;

   /* for internal use only */
   Config   *conf;
   Evas	    *evas;

   Evas_List *s_langs;

   char *plang;
   char *slang;
   char *kb_model;
   char *kb_variant;
   /*************************/

   struct 
     {
	Evas_Object  *o_plang;
	Evas_Object  *o_slang;

	Evas_Object  *o_add;
	Evas_Object  *o_del;
	Evas_Object  *o_up;
	Evas_Object  *o_down;

	Evas_Object  *o_kbd_model;
	Evas_Object  *o_kbd_variant;
     } gui;
};

/**************** Private funcs **********************************/

/******************* Config Dialog related funcs *****************/
static void	    *_create_data(E_Config_Dialog *cfd);
static void	    _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static Evas_Object  *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas,
					   E_Config_Dialog_Data *cfdata);
static int	    _apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static Evas_Object  *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas,
					      E_Config_Dialog_Data *cfdata);

static void	    _fill_data(E_Config_Dialog_Data *cfdata);

/****************** ilist callbacks **********************************/
static void _conf_cb_planguage_select  (void *data);
static void _conf_cb_slanguage_select  (void *data);
static void _conf_cb_kbd_model_select  (void *data);
static void _conf_cb_kbd_variant_select(void *data);

/****************** button callbacks *********************************/
static void _conf_cb_language_add   (void *data, void *data2);
static void _conf_cb_language_del   (void *data, void *data2);
static void _conf_cb_language_up    (void *data, void *data2);
static void _conf_cb_language_down  (void *data, void *data2);

/***************** button state set functions ************************/
static void _conf_add_button_availability_set	(E_Config_Dialog_Data *cfdata);
static void _conf_del_button_availability_set	(E_Config_Dialog_Data *cfdata);
static void _conf_up_button_availability_set	(E_Config_Dialog_Data *cfdata);
static void _conf_down_button_availability_set	(E_Config_Dialog_Data *cfdata);

/***************** list update functions ****************************/
static void _conf_fill_planguages(E_Config_Dialog_Data *cfdata);
static void _conf_fill_slanguages(E_Config_Dialog_Data *cfdata);
static void _conf_fill_kbd_model (E_Config_Dialog_Data *cfdata);

/*************************************************************************/
void
_lang_configure_language_module(Config *conf)
{
   E_Config_Dialog	*cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata	      = _create_data;
   v->free_cfdata	      = _free_data;
   v->basic.apply_cfdata      = _apply_data;
   v->basic.create_widgets    = _basic_create_widgets;
   v->advanced.apply_cfdata   = _apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     _("Language Module Settings"), NULL, 0, v, conf);

   conf->config_dialog = cfd;
}

/*************** Config Dialog Functions ********************/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;

   cfdata->lang_policy		 = cfdata->conf->lang_policy;
   cfdata->lang_show_indicator	 = cfdata->conf->lang_show_indicator;

   for (l = cfdata->conf->languages; l; l = l->next)
     {
	Language *lang;
	
	lang = lang_language_copy(l->data);
	if (lang)
	  cfdata->s_langs = evas_list_append(cfdata->s_langs, lang);
     }

   cfdata->plang	= NULL;
   cfdata->slang	= NULL;
   cfdata->kb_model	= NULL;
   cfdata->kb_variant	= NULL;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config		*conf;

   conf = cfd->data;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->conf = conf;

   _fill_data(cfdata);
   return cfdata;
}
static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   while (cfdata->s_langs)
     {
	lang_language_free(cfdata->s_langs->data);
	cfdata->s_langs = evas_list_remove_list(cfdata->s_langs, cfdata->s_langs);
     }

   if (cfdata->plang) E_FREE(cfdata->plang);
   if (cfdata->slang) E_FREE(cfdata->slang);
   if (cfdata->kb_model) E_FREE(cfdata->kb_model);
   if (cfdata->kb_variant) E_FREE(cfdata->kb_variant);

   cfdata->conf->config_dialog = NULL;
   free(cfdata);
}
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   E_Radio_Group *rg;
   Evas_Object *o, *ol, *ol2, *of, *ob;

   cfdata->evas = evas;

   o = e_widget_list_add(evas, 0, 0);

   ol = e_widget_list_add(evas, 0, 1);
      of = e_widget_framelist_add(evas, _("Available Languages"), 1);
	 ob = e_widget_ilist_add(evas, ILIST_ICON_WIDTH, ILIST_ICON_HEIGHT, &(cfdata->plang));
	 e_widget_min_size_set(ob, 220, 160);
	 e_widget_ilist_go(ob);
	 e_widget_framelist_object_append(of, ob);
	 cfdata->gui.o_plang = ob;
      e_widget_list_object_append(ol, of, 1, 1, 0.5);

      of = e_widget_framelist_add(evas, _("Selected Languages"), 0);
	 ob = e_widget_ilist_add(evas, ILIST_ICON_WIDTH, ILIST_ICON_HEIGHT, &(cfdata->slang));
	 e_widget_min_size_set(ob, 220, 160);
	 e_widget_ilist_go(ob);
	 e_widget_framelist_object_append(of, ob);
	 cfdata->gui.o_slang = ob;

	 ol2 = e_widget_list_add(evas, 0, 1);
	    ob = e_widget_button_add(evas, ">>", NULL, _conf_cb_language_add, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_add = ob;

	    ob = e_widget_button_add(evas, "<<", NULL, _conf_cb_language_del, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_del = ob;

	    ob = e_widget_button_add(evas, _("Up"), "widget/up_arrow", 
				     _conf_cb_language_up, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_up = ob;

	    ob = e_widget_button_add(evas, _("Down"), "widget/down_arrow", 
				     _conf_cb_language_down, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_down = ob;
	 e_widget_framelist_object_append(of, ol2);
      e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_list_object_append(o, ol, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Language Switch Policy"), 1);
      rg = e_widget_radio_group_new((int *)&(cfdata->lang_policy));

      ob = e_widget_radio_add(evas, _("Global"), LS_GLOBAL_POLICY, rg);
      e_widget_framelist_object_append(of, ob);

      ob = e_widget_radio_add(evas, _("Window"), LS_WINDOW_POLICY, rg);
      e_widget_framelist_object_append(of, ob);

      ob = e_widget_radio_add(evas, _("Application"), LS_APPLICATION_POLICY, rg);
      e_widget_framelist_object_append(of, ob);
      e_widget_disabled_set(ob, 1); // this is temporary
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   cfdata->gui.o_kbd_model    = NULL;
   cfdata->gui.o_kbd_variant  = NULL;

   _conf_fill_planguages(cfdata); 
   _conf_fill_slanguages(cfdata);

   _conf_add_button_availability_set(cfdata);
   _conf_del_button_availability_set(cfdata);
   _conf_up_button_availability_set(cfdata);
   _conf_down_button_availability_set(cfdata);

   return o;
}
static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   E_Radio_Group  *rg;
   Evas_Object *o, *ol, *ol2, *of, *ob;

   cfdata->evas = evas;

   o = e_widget_list_add(evas, 0, 0);

   ol = e_widget_list_add(evas, 0, 1);
      of = e_widget_framelist_add(evas, _("Available Languages"), 1);
	 ob = e_widget_ilist_add(evas, ILIST_ICON_WIDTH, ILIST_ICON_HEIGHT, &(cfdata->plang));
	 e_widget_min_size_set(ob, 220, 160);
	 e_widget_ilist_go(ob);
	 e_widget_framelist_object_append(of, ob);
	 cfdata->gui.o_plang = ob;
      e_widget_list_object_append(ol, of, 1, 1, 0.5);

      of = e_widget_framelist_add(evas, _("Selected Languages"), 0);
	 ob = e_widget_ilist_add(evas, ILIST_ICON_WIDTH, ILIST_ICON_HEIGHT, &(cfdata->slang));
	 e_widget_min_size_set(ob, 220, 160);
	 e_widget_ilist_go(ob);
	 e_widget_framelist_object_append(of, ob);
	 cfdata->gui.o_slang = ob;

	 ol2 = e_widget_list_add(evas, 0, 1);
	    ob = e_widget_button_add(evas, ">>", NULL, _conf_cb_language_add, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_add = ob;

	    ob = e_widget_button_add(evas, "<<", NULL, _conf_cb_language_del, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_del = ob;

	    ob = e_widget_button_add(evas, _("Up"), "widget/up_arrow",
				     _conf_cb_language_up, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_up = ob;

	    ob = e_widget_button_add(evas, _("Down"), "widget/down_arrow",
				     _conf_cb_language_down, cfdata, NULL);
	    e_widget_list_object_append(ol2, ob, 1, 1, 0.5);
	    cfdata->gui.o_down = ob;
	 e_widget_framelist_object_append(of, ol2);
      e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_list_object_append(o, ol, 1, 1, 0.5);

   ol = e_widget_list_add(evas, 0, 1);
      of = e_widget_framelist_add(evas, _("Keyboard Model"), 1);
	 ob = e_widget_ilist_add(evas, 0, 0, &(cfdata->kb_model));
	 e_widget_min_size_set(ob, 390, 80);
	 e_widget_ilist_go(ob);
	 cfdata->gui.o_kbd_model = ob;
	 e_widget_framelist_object_append(of, ob);
      e_widget_list_object_append(ol, of, 1, 1, 0.5);

      of = e_widget_framelist_add(evas, _("Layout Variant"), 0);
	 ob = e_widget_ilist_add(evas, 0, 0, &(cfdata->kb_variant));
	 e_widget_min_size_set(ob, 120, 80);
	 e_widget_ilist_go(ob);
	 cfdata->gui.o_kbd_variant = ob;
	 e_widget_framelist_object_append(of, ob);
      e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_list_object_append(o, ol, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Language Switch Policy"), 1);
      rg = e_widget_radio_group_new((int *)&(cfdata->lang_policy));

      ob = e_widget_radio_add(evas, _("Global"), LS_GLOBAL_POLICY, rg);
      e_widget_framelist_object_append(of, ob);

      ob = e_widget_radio_add(evas, _("Window"), LS_WINDOW_POLICY, rg);
      e_widget_framelist_object_append(of, ob);

      ob = e_widget_radio_add(evas, _("Application"), LS_APPLICATION_POLICY, rg);
      e_widget_framelist_object_append(of, ob);
      e_widget_disabled_set(ob, 1); // this is temporary
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   _conf_fill_planguages(cfdata);
   _conf_fill_slanguages(cfdata);

   _conf_add_button_availability_set(cfdata);
   _conf_del_button_availability_set(cfdata);
   _conf_up_button_availability_set(cfdata);
   _conf_down_button_availability_set(cfdata);

   return o;
}
static int
_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;
   Language    *lang, *lang2;

   cfdata->conf->lang_policy = cfdata->lang_policy;

   language_clear_border_language_setup_list();
   language_unregister_callback_handlers();
   if (cfdata->conf->lang_policy == LS_WINDOW_POLICY ||
       cfdata->conf->lang_policy == LS_APPLICATION_POLICY)
     {
	language_register_callback_handlers();
     }

   cfdata->conf->lang_show_indicator = cfdata->lang_show_indicator;

   while (cfdata->conf->languages)
     {
	lang_language_free(cfdata->conf->languages->data);
	cfdata->conf->languages = evas_list_remove_list(cfdata->conf->languages,
						        cfdata->conf->languages);
     }

   for (l = cfdata->s_langs; l; l = l->next)
     {
	lang = lang_language_copy(l->data);
	if (!lang) continue;

	lang_language_xorg_values_get(lang);
	cfdata->conf->languages = evas_list_append(cfdata->conf->languages, lang);
     }
   cfdata->conf->language_selector = 0;
   e_config_save_queue();

   lang_language_switch_to(cfdata->conf, 0);
   return 1;
}

/***************** list update functions ****************************/
static void
_conf_fill_planguages(E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;
   char	     buf[128];
   char	     lflag[2048];

   e_widget_ilist_clear(cfdata->gui.o_plang);
   for (l = cfdata->conf->language_predef_list; l; l = l->next)
     {
	Evas_Object	*ic = NULL;
	Language_Predef *lp;
	Evas_List	*l2;
	int		found = 0;

	lp = l->data;

	for (l2 = cfdata->s_langs; l2; l2 = l2->next)
	  {
	     Language *lang = l2->data;
	     if (!strcmp(lang->lang_name, lp->lang_name))
	       found = 1;
	  }

	if (found) continue;
	
	snprintf(buf, sizeof(buf), ILIST_LANGUAGE_LABEL_FORMAT, lp->lang_name, lp->lang_shortcut);

	ic = e_icon_add(cfdata->evas);
	snprintf(lflag, sizeof(lflag), "%s/images/%s.png",
	         e_module_dir_get(cfdata->conf->module), lp->lang_flag);
	e_icon_file_set(ic, lflag);
	e_widget_ilist_append(cfdata->gui.o_plang, ic, buf, _conf_cb_planguage_select,
			      cfdata, (char *)lp->lang_name);
     }
   e_widget_ilist_go(cfdata->gui.o_plang);
}

static void
_conf_fill_slanguages(E_Config_Dialog_Data *cfdata)
{
   Evas_List *l;
   char	     buf[128];
   char	     lflag[2048];

   e_widget_ilist_clear(cfdata->gui.o_slang);
   for (l = cfdata->s_langs; l; l = l->next)
     {
	Evas_Object *ic = NULL;
	Language    *lang;

	lang = l->data;

	snprintf(buf, sizeof(buf), ILIST_LANGUAGE_LABEL_FORMAT,
	         lang->lang_name, lang->lang_shortcut);

	ic = e_icon_add(cfdata->evas);
	snprintf(lflag, sizeof(lflag), "%s/images/%s.png",
	         e_module_dir_get(cfdata->conf->module), lang->lang_flag);
	e_icon_file_set(ic, lflag);
	e_widget_ilist_append(cfdata->gui.o_slang, ic, buf, _conf_cb_slanguage_select,
			      cfdata, (char *)lang->lang_name);
     }
   e_widget_ilist_go(cfdata->gui.o_slang);
}
static void
_conf_fill_kbd_model(E_Config_Dialog_Data *cfdata)
{
   int kbdm_i = -1;
   Language *slang;

   if (!cfdata->gui.o_kbd_model) return;

   if (!cfdata->slang)
     { 
	e_widget_ilist_clear(cfdata->gui.o_kbd_model);
	return;
     }

   slang = evas_list_nth(cfdata->s_langs, e_widget_ilist_selected_get(cfdata->gui.o_slang));

   if (!e_widget_ilist_count(cfdata->gui.o_kbd_model))
     {
	Evas_List *l;
	int i;
	char buf[256];

	for (l = cfdata->conf->language_kbd_model_list, i = 0; l; l = l->next, i++)
	  {
	     Language_Kbd_Model *lkm;

	     lkm = l->data;

	     if (!strcmp(lkm->kbd_model, slang->rdefs.model))
	       kbdm_i = i;

	     snprintf(buf, sizeof(buf), "%s", lkm->kbd_model_description);
	     e_widget_ilist_append(cfdata->gui.o_kbd_model, NULL, buf,
				   _conf_cb_kbd_model_select, cfdata, (char *)lkm->kbd_model);
	  }
     }
   else
     {
	Evas_List *l;

	for (l = cfdata->conf->language_kbd_model_list, kbdm_i = 0; l; l = l->next, kbdm_i++)
	  {
	     Language_Kbd_Model *lkm;

	     lkm = l->data;

	     if (!strcmp(lkm->kbd_model, slang->rdefs.model))
	       break;
	  }
	if (!l)
	  kbdm_i = 0;
     }

   e_widget_ilist_go(cfdata->gui.o_kbd_model);
   e_widget_ilist_selected_set(cfdata->gui.o_kbd_model, kbdm_i);
}
static void
_conf_fill_kbd_variant(E_Config_Dialog_Data *cfdata)
{
   Language  *slang;
   Evas_List *l;
   int kbdv_i = 0;

   if (!cfdata->gui.o_kbd_variant) return;

   e_widget_ilist_clear(cfdata->gui.o_kbd_variant);

   if (!cfdata->slang) 
     return;

   e_widget_ilist_clear(cfdata->gui.o_kbd_variant);

   for (l = cfdata->conf->language_predef_list; l; l = l->next)
     {
	Language_Predef *lp;

	lp = l->data;

	if (!strcmp(lp->lang_name, cfdata->slang))
	  {
	     Language *lang;
	     int i;

	     lang = evas_list_nth(cfdata->s_langs,
				  e_widget_ilist_selected_get(cfdata->gui.o_slang));

	     e_widget_ilist_append(cfdata->gui.o_kbd_variant, NULL, "basic",
				   _conf_cb_kbd_variant_select, cfdata, "basic");

	     for (l = lp->kbd_variant, i = 1; l; l = l->next, i++)
	       {
		  char *var;

		  var = l->data;

		  if (!strcmp(var, "basic")) continue;

		  if (lang->rdefs.variant && !strcmp(lang->rdefs.variant, var))
		    kbdv_i = i;

		  e_widget_ilist_append(cfdata->gui.o_kbd_variant, NULL, var,
				        _conf_cb_kbd_variant_select, cfdata, var);
	       }
	     break;
	  }
     }

   e_widget_ilist_go(cfdata->gui.o_kbd_variant);
   e_widget_ilist_selected_set(cfdata->gui.o_kbd_variant, kbdv_i);
}
/***************** button state set functions ************************/
static void
_conf_add_button_availability_set(E_Config_Dialog_Data *cfdata)
{
   if (!e_widget_ilist_selected_label_get(cfdata->gui.o_plang))
     e_widget_disabled_set(cfdata->gui.o_add, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_add, 0);
}
static void
_conf_del_button_availability_set(E_Config_Dialog_Data *cfdata)
{
   if (!e_widget_ilist_selected_label_get(cfdata->gui.o_slang))
     e_widget_disabled_set(cfdata->gui.o_del, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_del, 0);
}
static void
_conf_up_button_availability_set(E_Config_Dialog_Data *cfdata)
{
   if (!e_widget_ilist_selected_label_get(cfdata->gui.o_slang) ||
       !e_widget_ilist_selected_get(cfdata->gui.o_slang) ||
       evas_list_count(cfdata->s_langs) < 1)
     e_widget_disabled_set(cfdata->gui.o_up, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_up, 0);
}
static void
_conf_down_button_availability_set(E_Config_Dialog_Data *cfdata)
{
   if (!e_widget_ilist_selected_label_get(cfdata->gui.o_slang) ||
       (e_widget_ilist_selected_get(cfdata->gui.o_slang) ==
	   e_widget_ilist_count(cfdata->gui.o_slang) - 1) ||
       evas_list_count(cfdata->s_langs) < 1)
     e_widget_disabled_set(cfdata->gui.o_down, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_down, 0);
}

/***************** ilist callbacks ***********************************/
static void 
_conf_cb_planguage_select(void *data)
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = data;

   _conf_add_button_availability_set(data);
}
static void 
_conf_cb_slanguage_select(void *data)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   _conf_fill_kbd_model(cfdata);
   _conf_fill_kbd_variant(cfdata);

   _conf_del_button_availability_set(cfdata);
   _conf_up_button_availability_set(cfdata);
   _conf_down_button_availability_set(cfdata);
}
static void
_conf_cb_kbd_model_select(void *data)
{
   Language *l;
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   l = evas_list_nth(cfdata->s_langs, e_widget_ilist_selected_get(cfdata->gui.o_slang));
   if (!l) return;

   if (l->rdefs.model) evas_stringshare_del(l->rdefs.model);
   l->rdefs.model = (char *)evas_stringshare_add(cfdata->kb_model);
}
static void
_conf_cb_kbd_variant_select(void *data)
{
   Language *l;
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   l = evas_list_nth(cfdata->s_langs, e_widget_ilist_selected_get(cfdata->gui.o_slang));
   if (!l) return;

   if (l->rdefs.variant) evas_stringshare_del(l->rdefs.variant);
   l->rdefs.variant = (char *)evas_stringshare_add(cfdata->kb_variant);
}
/****************** button callbacks *********************************/
static void 
_conf_cb_language_add(void *data, void *data2)
{
   Language		*lang;
   E_Config_Dialog_Data *cfdata;
   Language_Predef	*lp = NULL;
   Evas_List		*l;

   cfdata = data;

   for (l = cfdata->conf->language_predef_list; l; l = l->next)
     {
	lp = l->data;
	if (!strcmp(lp->lang_name, cfdata->plang))
	  break;
	lp = NULL;
     }

   if (!lp) return;

   lang = E_NEW(Language, 1);
   if (!lang) return;

   lang->id		= evas_list_count(cfdata->s_langs);
   lang->lang_name	= evas_stringshare_add(lp->lang_name);
   lang->lang_shortcut	= evas_stringshare_add(lp->lang_shortcut);
   lang->lang_flag	= !(lp->lang_flag) ? NULL : evas_stringshare_add(lp->lang_flag);
   lang->rdefs.model	= (char *) lang_language_current_kbd_model_get();
   lang->rdefs.layout	= (char *) evas_stringshare_add(lp->kbd_layout);
   lang->rdefs.variant	= (char *) evas_stringshare_add("basic");

   cfdata->s_langs = evas_list_append(cfdata->s_langs, lang);

   _conf_fill_planguages(cfdata);
   _conf_fill_slanguages(cfdata);

   E_FREE(cfdata->plang);

   e_widget_ilist_selected_set(cfdata->gui.o_slang, evas_list_count(cfdata->s_langs) - 1);
   _conf_add_button_availability_set(cfdata);
}
static void
_conf_cb_language_del(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Language *l;
   int n;

   cfdata = data;

   n = e_widget_ilist_selected_get(cfdata->gui.o_slang);

   l = evas_list_nth(cfdata->s_langs, n);
   cfdata->s_langs = evas_list_remove(cfdata->s_langs, l);

   lang_language_free(l);

   _conf_fill_planguages(cfdata);
   _conf_fill_slanguages(cfdata);

   E_FREE(cfdata->slang);

   if (n >= evas_list_count(cfdata->s_langs)) 
     n = evas_list_count(cfdata->s_langs) - 1;

   if (n >= 0) 
     e_widget_ilist_selected_set(cfdata->gui.o_slang, n);
   else
     {
	E_FREE(cfdata->kb_model);
	E_FREE(cfdata->kb_variant);

	_conf_fill_kbd_model(cfdata);
	_conf_fill_kbd_variant(cfdata);

	_conf_del_button_availability_set(cfdata);
     }
}
static void
_conf_cb_language_up(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *l, *l2;
   void *tmp;
   int n;

   cfdata = data;

   n = e_widget_ilist_selected_get(cfdata->gui.o_slang);
   if (n <= 0) return;

   l  = evas_list_nth_list(cfdata->s_langs, n);
   l2 = evas_list_nth_list(cfdata->s_langs, n - 1); 

   ((Language *)(l->data))->id --;
   ((Language *)(l2->data))->id ++;

   tmp = l->data;
   l->data = l2->data;
   l2->data = tmp;

   _conf_fill_slanguages(cfdata);
   e_widget_ilist_selected_set(cfdata->gui.o_slang, n - 1);
}
static void 
_conf_cb_language_down(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   Evas_List *l, *l2;
   void *tmp;
   int n;

   cfdata = data;

   n = e_widget_ilist_selected_get(cfdata->gui.o_slang);
   if ((n < 0) || (n >= evas_list_count(cfdata->s_langs))) return;

   l  = evas_list_nth_list(cfdata->s_langs, n);
   l2 = evas_list_nth_list(cfdata->s_langs, n + 1);

   ((Language *)(l->data))->id ++;
   ((Language *)(l2->data))->id --;

   tmp = l->data;
   l->data = l2->data;
   l2->data = tmp;

   _conf_fill_slanguages(cfdata);
   e_widget_ilist_selected_set(cfdata->gui.o_slang, n + 1);
}
