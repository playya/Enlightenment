#include "e_illume_private.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "e_mod_policy.h"
#include "e_mod_kbd.h"
#include "e_mod_quickpanel.h"

/* NB: Initially I had done this rewrite with eina_logging enabled, but it 
 * degraded performance so much that it was just not worth it. So now this 
 * module just uses printfs on the console to report things */

/* external variables */
const char *_e_illume_mod_dir = NULL;
E_Illume_Keyboard *_e_illume_kbd = NULL;
Eina_List *_e_illume_qps = NULL;

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Illume2" };

EAPI void *
e_modapi_init(E_Module *m) 
{
   Eina_List *ml;
   E_Manager *man;

   /* set module priority so we load first */
   e_module_priority_set(m, 100);

   /* set module directory variable */
   _e_illume_mod_dir = eina_stringshare_add(m->dir);

   /* try to initialize the config subsystem */
   if (!e_mod_config_init()) 
     {
        /* clear module directory variable */
        if (_e_illume_mod_dir) eina_stringshare_del(_e_illume_mod_dir);
        _e_illume_mod_dir = NULL;

        return NULL;
     }

   /* try to initialize the policy subsystem */
   if (!e_mod_policy_init()) 
     {
        /* shutdown the config subsystem */
        e_mod_config_shutdown();

        /* clear module directory variable */
        if (_e_illume_mod_dir) eina_stringshare_del(_e_illume_mod_dir);
        _e_illume_mod_dir = NULL;

        return NULL;
     }

   /* initialize the keyboard subsystem */
   e_mod_kbd_init();

   /* initialize the quickpanel subsystem */
   e_mod_quickpanel_init();

   /* create a new vkbd & hide it initially */
   _e_illume_kbd = e_mod_kbd_new();
   e_mod_kbd_hide();

   /* loop the zones and create quickpanels for each one */
   EINA_LIST_FOREACH(e_manager_list(), ml, man) 
     {
        Eina_List *cl;
        E_Container *con;

        EINA_LIST_FOREACH(man->containers, cl, con) 
          {
             Eina_List *zl;
             E_Zone *zone;

             EINA_LIST_FOREACH(con->zones, zl, zone) 
               {
                  E_Illume_Quickpanel *qp;

                  /* try to create a new quickpanel for this zone */
                  if (!(qp = e_mod_quickpanel_new(zone))) continue;

                  /* append new qp to list */
                  _e_illume_qps = eina_list_append(_e_illume_qps, qp);
               }
          }
     }

   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   E_Illume_Quickpanel *qp;

   /* delete the quickpanels */
   EINA_LIST_FREE(_e_illume_qps, qp)
     e_object_del(E_OBJECT(qp));

   /* shutdown the quickpanel subsystem */
   e_mod_quickpanel_shutdown();

   /* delete the keyboard object */
   if (_e_illume_kbd) e_object_del(E_OBJECT(_e_illume_kbd));
   _e_illume_kbd = NULL;

   /* shutdown the keyboard subsystem */
   e_mod_kbd_shutdown();

   /* shutdown the policy subsystem */
   e_mod_policy_shutdown();

   /* shutdown the config subsystem */
   e_mod_config_shutdown();

   /* clear module directory variable */
   if (_e_illume_mod_dir) eina_stringshare_del(_e_illume_mod_dir);
   _e_illume_mod_dir = NULL;

   return 1;
}

EAPI int 
e_modapi_save(E_Module *m) 
{
   return e_mod_config_save();
}
