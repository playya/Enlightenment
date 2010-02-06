#include "e.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "e_mod_comp.h"
#include "config.h"

struct _E_Config_Dialog_Data
{
   int use_shadow;
   int engine;
   int texture_from_pixmap;
   int lock_fps;
   int efl_sync;
   int loose_sync;
   int grab;
   
   int keep_unmapped;
   int max_unmapped_pixels;
   int max_unmapped_time;
   int min_unmapped_time;
   int send_flush;
   int send_dump;
   
   int effect_fade;
   int effect_zoom;
   int effect_wobble;
   int effect_rotate;
};

/* Protos */
static void        *_create_data          (E_Config_Dialog *cfd);
static void         _free_data            (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data     (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

E_Config_Dialog *
e_int_config_comp_module(E_Container *con, const char *params __UNUSED__)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];
   Mod *mod = _comp_mod;

   if (e_config_dialog_find("E", "appearance/comp")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;

   snprintf(buf, sizeof(buf), "%s/e-module-comp.edj", 
            e_module_dir_get(mod->module));
   cfd = e_config_dialog_new(con,
			     _("Composite Settings"),
			     "E", "appearance/comp",
			     buf, 0, v, mod);
   mod->config_dialog = cfd;
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);

   cfdata->use_shadow = _comp_mod->conf->use_shadow;
   cfdata->engine = _comp_mod->conf->engine;
   if ((cfdata->engine != E_EVAS_ENGINE_SOFTWARE_X11) &&
       (cfdata->engine != E_EVAS_ENGINE_GL_X11))
     cfdata->engine = E_EVAS_ENGINE_SOFTWARE_X11;
   cfdata->texture_from_pixmap = _comp_mod->conf->texture_from_pixmap;
   cfdata->lock_fps = _comp_mod->conf->lock_fps;
   cfdata->efl_sync = _comp_mod->conf->efl_sync;
   cfdata->loose_sync = _comp_mod->conf->loose_sync;
   cfdata->grab = _comp_mod->conf->grab;

   cfdata->keep_unmapped = _comp_mod->conf->keep_unmapped;
   cfdata->max_unmapped_pixels = _comp_mod->conf->max_unmapped_pixels;
   cfdata->max_unmapped_time = _comp_mod->conf->max_unmapped_time;
   cfdata->min_unmapped_time = _comp_mod->conf->min_unmapped_time;
   cfdata->send_flush = _comp_mod->conf->send_flush;
   cfdata->send_dump = _comp_mod->conf->send_dump;
   
   cfdata->effect_fade = _comp_mod->conf->effect_fade;
   cfdata->effect_zoom = _comp_mod->conf->effect_zoom;
   cfdata->effect_wobble = _comp_mod->conf->effect_wobble;
   cfdata->effect_rotate = _comp_mod->conf->effect_rotate;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   _comp_mod->config_dialog = NULL;
   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *ob, *ol, *ol2, *of, *otb;
   E_Radio_Group *rg;

   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);
   
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   ob = e_widget_check_add(evas, _("Shadows"), &(cfdata->use_shadow));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_check_add(evas, _("Limit framerate"), &(cfdata->lock_fps));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   of = e_widget_framelist_add(evas, _("Show/hide"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   ob = e_widget_check_add(evas, _("Fade"), &(cfdata->effect_fade));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Zoom"), &(cfdata->effect_zoom));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Wobble"), &(cfdata->effect_wobble));
   e_widget_framelist_object_append(of, ob);
// implement much later   
//   ob = e_widget_check_add(evas, _("Rotate"), &(cfdata->effect_rotate));
//   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Effects"), ol, 0, 0, 0, 0, 0.5, 0.0);
   
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   ob = e_widget_check_add(evas, _("Sync windows"), &(cfdata->efl_sync));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_check_add(evas, _("Loose sync"), &(cfdata->loose_sync));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_check_add(evas, _("Grab X11 during draw"), &(cfdata->grab));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Sync"), ol, 0, 0, 0, 0, 0.5, 0.0);
   
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   rg = e_widget_radio_group_new(&(cfdata->engine));
   ob = e_widget_radio_add(evas, _("Software"), E_EVAS_ENGINE_SOFTWARE_X11, rg);
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_X11))
     {
        ob = e_widget_radio_add(evas, _("OpenGL"), E_EVAS_ENGINE_GL_X11, rg);
        e_widget_list_object_append(ol, ob, 1, 1, 0.5);
     }
   of = e_widget_framelist_add(evas, _("OpenGL options"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   ob = e_widget_check_add(evas, _("Texture from pixmap"), &(cfdata->texture_from_pixmap));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Engine"), ol, 0, 0, 0, 0, 0.5, 0.0);
   
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   ob = e_widget_check_add(evas, _("Send flush"), &(cfdata->send_flush));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_check_add(evas, _("Send dump"), &(cfdata->send_dump));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_check_add(evas, _("Keep hidden windows"), &(cfdata->keep_unmapped));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   of = e_widget_framelist_add(evas, _("Maximum hidden pixels"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->max_unmapped_pixels));
   ob = e_widget_radio_add(evas, _("1M"), 1 * 1024, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("2M"), 2 * 1024, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("8M"), 8 * 1024, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("32M"), 32 * 1024, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("256M"), 256 * 1024, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Memory"), ol, 0, 0, 0, 0, 0.5, 0.0);
   
   ///////////////////////////////////////////
   ol = e_widget_list_add(evas, 0, 0);
   ol2 = e_widget_list_add(evas, 1, 1);
   of = e_widget_framelist_add(evas, _("Min hidden"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->min_unmapped_time));
   ob = e_widget_radio_add(evas, _("30 Seconds"), 30, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("1 Minute"), 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("5 Minutes"), 5 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("30 Minutes"), 30 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("2 Hours"), 2 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("10 Hours"), 10 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Forever"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol2, of, 1, 1, 0.5);
   of = e_widget_framelist_add(evas, _("Max hidden"), 0);
   e_widget_framelist_content_align_set(of, 0.5, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->max_unmapped_time));
   ob = e_widget_radio_add(evas, _("30 Seconds"), 30, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("1 Minute"), 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("5 Minutes"), 5 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("30 Minutes"), 30 * 60, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("2 Hours"), 2 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("10 Hours"), 10 * 3600, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Forever"), 0, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol2, of, 1, 1, 0.5);
   e_widget_list_object_append(ol, ol2, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Timeouts"), ol, 0, 0, 0, 0, 0.5, 0.0);
   
   e_widget_toolbook_page_show(otb, 0);
   
   return otb;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   // FIXME: save new config options as they are implemented.
   if ((_comp_mod->conf->use_shadow != cfdata->use_shadow) ||
       (cfdata->lock_fps != _comp_mod->conf->lock_fps) ||
       (cfdata->grab != _comp_mod->conf->grab) ||
       (cfdata->effect_fade != _comp_mod->conf->effect_fade))
     {
        _comp_mod->conf->use_shadow = cfdata->use_shadow;
        _comp_mod->conf->lock_fps = cfdata->lock_fps;
        _comp_mod->conf->grab = cfdata->grab;
//        _comp_mod->conf->effect_fade = cfdata->effect_fade;
        e_mod_comp_shadow_set();
     }
   if ((_comp_mod->conf->engine != cfdata->engine) ||
       (cfdata->texture_from_pixmap != _comp_mod->conf->texture_from_pixmap) ||
       (cfdata->efl_sync != _comp_mod->conf->efl_sync) ||
       (cfdata->loose_sync != _comp_mod->conf->loose_sync))
     {
        _comp_mod->conf->engine = cfdata->engine;
        _comp_mod->conf->texture_from_pixmap = cfdata->texture_from_pixmap;
        _comp_mod->conf->efl_sync = cfdata->efl_sync;
        _comp_mod->conf->loose_sync = cfdata->loose_sync;
        e_mod_comp_shutdown();
        e_mod_comp_init();
     }
   e_config_save_queue();
   return 1;
}
