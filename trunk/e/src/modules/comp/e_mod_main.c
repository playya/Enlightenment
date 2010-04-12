#include "e.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "e_mod_comp.h"

static Ecore_Event_Handler *init_done_handler = NULL;

//static int
//_e_init_done(void *data, int type, void *event)
//{
//   ecore_event_handler_del(init_done_handler);
//   init_done_handler = NULL;
//   if (!e_mod_comp_init())
//     {
//        // FIXME: handle if comp init fails
//     }
//   return 1;
//}

/* module private routines */
Mod *_comp_mod = NULL;

/* public module routines. all modules must have these */
EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION,
   "Composite"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   Mod *mod;
   char buf[4096];
   
   mod = calloc(1, sizeof(Mod));
   mod->module = m;
   snprintf(buf, sizeof(buf), "%s/e-module-comp.edj", e_module_dir_get(m));
   e_configure_registry_category_add("appearance", 10, _("Look"), NULL, "preferences-appearance");
   e_configure_registry_item_add("appearance/comp", 120, _("Composite"), NULL, buf, e_int_config_comp_module);
   
   mod->conf_edd = E_CONFIG_DD_NEW("Comp_Config", Config);
#undef T
#undef D
#define T Config
#define D mod->conf_edd
   E_CONFIG_VAL(D, T, use_shadow, UCHAR);
   E_CONFIG_VAL(D, T, shadow_file, STR);
   E_CONFIG_VAL(D, T, shadow_style, STR);
   E_CONFIG_VAL(D, T, engine, INT);
   E_CONFIG_VAL(D, T, indirect, UCHAR);
   E_CONFIG_VAL(D, T, texture_from_pixmap, UCHAR);
   E_CONFIG_VAL(D, T, lock_fps, UCHAR);
   E_CONFIG_VAL(D, T, efl_sync, UCHAR);
   E_CONFIG_VAL(D, T, loose_sync, UCHAR);
   E_CONFIG_VAL(D, T, grab, UCHAR);
   E_CONFIG_VAL(D, T, vsync, UCHAR);
   E_CONFIG_VAL(D, T, keep_unmapped, UCHAR);
   E_CONFIG_VAL(D, T, send_flush, UCHAR);
   E_CONFIG_VAL(D, T, send_dump, UCHAR);
   E_CONFIG_VAL(D, T, nocomp_fs, UCHAR);
   E_CONFIG_VAL(D, T, smooth_windows, UCHAR);
   E_CONFIG_VAL(D, T, max_unmapped_pixels, INT);
   E_CONFIG_VAL(D, T, max_unmapped_time, INT);
   E_CONFIG_VAL(D, T, min_unmapped_time, INT);
   
   mod->conf = e_config_domain_load("module.comp", mod->conf_edd);
   if (!mod->conf)
     {
	mod->conf = E_NEW(Config, 1);
        mod->conf->use_shadow = 1;
        mod->conf->shadow_file = NULL;
        mod->conf->shadow_style = eina_stringshare_add("default");
        mod->conf->engine = E_EVAS_ENGINE_SOFTWARE_X11;
        mod->conf->indirect = 0;
        mod->conf->texture_from_pixmap = 0; 
        mod->conf->lock_fps = 0;
        mod->conf->efl_sync = 1;
        mod->conf->loose_sync = 1;
        mod->conf->grab = 0;
        mod->conf->vsync = 1;
        mod->conf->keep_unmapped = 1;
        mod->conf->send_flush = 1; // implement
        mod->conf->send_dump = 0; // implement
        mod->conf->nocomp_fs = 0; // buggy
        mod->conf->smooth_windows = 0;
        mod->conf->max_unmapped_pixels =  32 * 1024; // implement
        mod->conf->max_unmapped_time = 10 * 3600; // implement
        mod->conf->min_unmapped_time = 5 * 60; // implement
     }
   
   _comp_mod = mod;

//   if (e_init_count_get() <= 0)
//     {
        if (!e_mod_comp_init())
          {
             // FIXME: handle if comp init fails
          }
//     }
//   else
//     {
//        init_done_handler = ecore_event_handler_add(E_EVENT_INIT_DONE,
//                                                    _e_init_done, NULL);
//     }

   e_module_priority_set(m, -1000);
   return mod;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   Mod *mod = m->data;
   
   e_mod_comp_shutdown();
   
   e_configure_registry_item_del("appearance/comp");
   e_configure_registry_category_del("appearance");
   
   if (mod->config_dialog) 
     {
        e_object_del(E_OBJECT(mod->config_dialog));
        mod->config_dialog = NULL;
     }
   if (mod->conf->shadow_file) eina_stringshare_del(mod->conf->shadow_file);
   if (mod->conf->shadow_style) eina_stringshare_del(mod->conf->shadow_style);
   free(mod->conf);
   E_CONFIG_DD_FREE(mod->conf_edd);
   free(mod);
   
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   Mod *mod = m->data;
   e_config_domain_save("module.comp", mod->conf_edd, mod->conf);
   return 1;
}
