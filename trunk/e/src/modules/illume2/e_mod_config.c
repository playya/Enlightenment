#include "e.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "e_mod_animation.h"
#include "e_mod_windows.h"

/* local variables */
EAPI Il_Config *il_cfg = NULL;
static E_Config_DD *conf_edd = NULL;

/* public functions */
EAPI int 
il_config_init(E_Module *m) 
{
   conf_edd = E_CONFIG_DD_NEW("Illume_Cfg", Il_Config);
   #undef T
   #undef D
   #define T Il_Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, sliding.kbd.duration, INT);
   E_CONFIG_VAL(D, T, sliding.softkey.duration, INT);
   E_CONFIG_VAL(D, T, policy.name, STR);
   E_CONFIG_VAL(D, T, policy.vkbd.class, STR);
   E_CONFIG_VAL(D, T, policy.vkbd.name, STR);
   E_CONFIG_VAL(D, T, policy.vkbd.title, STR);
   E_CONFIG_VAL(D, T, policy.vkbd.win_type, STR);
   E_CONFIG_VAL(D, T, policy.softkey.class, STR);
   E_CONFIG_VAL(D, T, policy.softkey.name, STR);
   E_CONFIG_VAL(D, T, policy.softkey.title, STR);
   E_CONFIG_VAL(D, T, policy.softkey.win_type, STR);
   E_CONFIG_VAL(D, T, policy.home.class, STR);
   E_CONFIG_VAL(D, T, policy.home.name, STR);
   E_CONFIG_VAL(D, T, policy.home.title, STR);
   E_CONFIG_VAL(D, T, policy.home.win_type, STR);

   il_cfg = e_config_domain_load("module.illume2", conf_edd);
   if ((il_cfg) && 
       ((il_cfg->version >> 16) < IL_CONFIG_MAJ)) 
     {
        E_FREE(il_cfg);
        il_cfg = NULL;
     }
   if (!il_cfg) 
     {
        il_cfg = E_NEW(Il_Config, 1);
        il_cfg->version = 0;
        il_cfg->sliding.kbd.duration = 1000;
        il_cfg->sliding.softkey.duration = 1000;
     }
   if (il_cfg) 
     {
        /* Add new config variables here */
        if ((il_cfg->version & 0xffff) < 1) 
          {
             il_cfg->policy.name = NULL;
             il_cfg->policy.vkbd.class = 
               eina_stringshare_add("Virtual-Keyboard");
             il_cfg->policy.vkbd.name = 
               eina_stringshare_add("E");
             il_cfg->policy.vkbd.title = 
               eina_stringshare_add("Virtual Keyboard");
             il_cfg->policy.vkbd.win_type = NULL;
             il_cfg->policy.softkey.class = 
               eina_stringshare_add("Illume-Softkey");
             il_cfg->policy.softkey.name = 
               eina_stringshare_add("Illume-Softkey");
             il_cfg->policy.softkey.title = 
               eina_stringshare_add("Illume Softkey");
             il_cfg->policy.softkey.win_type = NULL;
             il_cfg->policy.home.class = 
               eina_stringshare_add("Illume-Home");
             il_cfg->policy.home.name = 
               eina_stringshare_add("Illume-Home");
             il_cfg->policy.home.title = 
               eina_stringshare_add("Illume Home");
             il_cfg->policy.home.win_type = NULL;
          }
        il_cfg->version = (IL_CONFIG_MAJ << 16) | IL_CONFIG_MIN;
     }
   il_cfg->mod_dir = eina_stringshare_add(m->dir);

   e_configure_registry_category_add("illume", 0, _("Illume"), NULL, 
                                     "enlightenment/display");
   e_configure_registry_generic_item_add("illume/animation", 0, _("Animation"), 
                                         NULL, "enlightenment/animation", 
                                         il_config_animation_show);
   e_configure_registry_generic_item_add("illume/windows", 0, _("Windows"), 
                                         NULL, "enlightenment/windows", 
                                         il_config_windows_show);
   return 1;
}

EAPI int 
il_config_shutdown(void) 
{
   e_configure_registry_item_del("illume/windows");
   e_configure_registry_item_del("illume/animation");
   e_configure_registry_category_del("illume");

   if (il_cfg->policy.name) eina_stringshare_del(il_cfg->policy.name);

   if (il_cfg->policy.vkbd.class) 
     eina_stringshare_del(il_cfg->policy.vkbd.class);
   if (il_cfg->policy.vkbd.name) 
     eina_stringshare_del(il_cfg->policy.vkbd.name);
   if (il_cfg->policy.vkbd.title) 
     eina_stringshare_del(il_cfg->policy.vkbd.title);
   if (il_cfg->policy.vkbd.win_type) 
     eina_stringshare_del(il_cfg->policy.vkbd.win_type);

   if (il_cfg->policy.softkey.class) 
     eina_stringshare_del(il_cfg->policy.softkey.class);
   if (il_cfg->policy.softkey.name) 
     eina_stringshare_del(il_cfg->policy.softkey.name);
   if (il_cfg->policy.softkey.title) 
     eina_stringshare_del(il_cfg->policy.softkey.title);
   if (il_cfg->policy.softkey.win_type) 
     eina_stringshare_del(il_cfg->policy.softkey.win_type);

   if (il_cfg->policy.home.class) 
     eina_stringshare_del(il_cfg->policy.home.class);
   if (il_cfg->policy.home.name) 
     eina_stringshare_del(il_cfg->policy.home.name);
   if (il_cfg->policy.home.title) 
     eina_stringshare_del(il_cfg->policy.home.title);
   if (il_cfg->policy.home.win_type) 
     eina_stringshare_del(il_cfg->policy.home.win_type);

   if (il_cfg->mod_dir) eina_stringshare_del(il_cfg->mod_dir);

   E_FREE(il_cfg);
   il_cfg = NULL;

   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int 
il_config_save(void) 
{
   e_config_domain_save("module.illume2", conf_edd, il_cfg);
   return 1;
}
