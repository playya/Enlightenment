#include "e.h"
#include "e_mod_main.h"
#include "e_mod_ind_win.h"

/* local variables */
static Eina_List *iwins = NULL;

/* external variables */
const char *_ind_mod_dir = NULL;

EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Illume-Indicator" };

EAPI void *
e_modapi_init(E_Module *m) 
{
   E_Manager *man;
   Eina_List *l;

   /* setup variable to hold module directory */
   _ind_mod_dir = eina_stringshare_add(m->dir);

   /* loop through the managers (root windows) */
   EINA_LIST_FOREACH(e_manager_list(), l, man) 
     {
        E_Container *con;
        Eina_List *cl;

        /* loop through the containers */
        EINA_LIST_FOREACH(man->containers, cl, con) 
          {
             E_Zone *zone;
             Eina_List *zl;

             /* for each zone in this container, create a indicator window */
             EINA_LIST_FOREACH(con->zones, zl, zone) 
               {
                  Il_Ind_Win *iwin = NULL;

                  if (!(iwin = e_mod_ind_win_new(zone))) continue;
                  iwins = eina_list_append(iwins, iwin);
               }
          }
     }

   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   Il_Ind_Win *iwin;

   /* destroy the indicator windows */
   EINA_LIST_FREE(iwins, iwin) 
     {
        e_object_del(E_OBJECT(iwin));
        iwin = NULL;
     }

   /* clear the module directory variable */
   if (_ind_mod_dir) eina_stringshare_del(_ind_mod_dir);
   _ind_mod_dir = NULL;

   /* reset indicator geometry for conformant apps to zero */
   ecore_x_e_illume_top_shelf_geometry_set(ecore_x_window_root_first_get(), 
                                           0, 0, 0, 0);

   return 1;
}

EAPI int 
e_modapi_save(E_Module *m) 
{
   return 1;
}
