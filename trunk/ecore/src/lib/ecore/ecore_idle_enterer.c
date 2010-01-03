/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"


struct _Ecore_Idle_Enterer
{
   EINA_INLIST;
   ECORE_MAGIC;
   int          delete_me : 1;
   int        (*func) (void *data);
   void        *data;
};


static Ecore_Idle_Enterer *idle_enterers = NULL;
static int                 idle_enterers_delete_me = 0;

/**
 * Add an idle enterer handler.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 * @ingroup Idle_Group
 */
EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Enterer *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Enterer));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_ENTERER);
   ie->func = func;
   ie->data = (void *)data;
   idle_enterers = (Ecore_Idle_Enterer *) eina_inlist_append(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));
   return ie;
}

/**
 * Add an idle enterer handler at the start of the list so it gets called earlier than others.
 * @param   func The function to call when entering an idle state.
 * @param   data The data to be passed to the @p func call
 * @return  A handle to the idle enterer callback if successful.  Otherwise,
 *          NULL is returned.
 * @ingroup Idle_Group
 */
EAPI Ecore_Idle_Enterer *
ecore_idle_enterer_before_add(int (*func) (void *data), const void *data)
{
   Ecore_Idle_Enterer *ie;

   if (!func) return NULL;
   ie = calloc(1, sizeof(Ecore_Idle_Enterer));
   if (!ie) return NULL;
   ECORE_MAGIC_SET(ie, ECORE_MAGIC_IDLE_ENTERER);
   ie->func = func;
   ie->data = (void *)data;
   idle_enterers = (Ecore_Idle_Enterer *) eina_inlist_prepend(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));
   return ie;
}

/**
 * Delete an idle enterer callback.
 * @param   idle_enterer The idle enterer to delete
 * @return  The data pointer passed to the idler enterer callback on success.
 *          NULL otherwise.
 * @ingroup Idle_Group
 */
EAPI void *
ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer)
{
   if (!ECORE_MAGIC_CHECK(idle_enterer, ECORE_MAGIC_IDLE_ENTERER))
     {
	ECORE_MAGIC_FAIL(idle_enterer, ECORE_MAGIC_IDLE_ENTERER,
			 "ecore_idle_enterer_del");
	return NULL;
     }
   idle_enterer->delete_me = 1;
   idle_enterers_delete_me = 1;
   return idle_enterer->data;
}

void
_ecore_idle_enterer_shutdown(void)
{
   Ecore_Idle_Enterer *ie;
   while ((ie = idle_enterers))
     {
	idle_enterers = (Ecore_Idle_Enterer *) eina_inlist_remove(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(idle_enterers));
	ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
	free(ie);
     }
   idle_enterers_delete_me = 0;
}

void
_ecore_idle_enterer_call(void)
{
   Ecore_Idle_Enterer *ie;

   EINA_INLIST_FOREACH(idle_enterers, ie)
     {
	if (!ie->delete_me)
	  {
	     if (!ie->func(ie->data)) ecore_idle_enterer_del(ie);
	  }
     }
   if (idle_enterers_delete_me)
     {
        Ecore_Idle_Enterer *l;
	for(l = idle_enterers; l;)
	  {
	     ie = l;
	     l = (Ecore_Idle_Enterer *) EINA_INLIST_GET(l)->next;
	     if (ie->delete_me)
	       {
		  idle_enterers = (Ecore_Idle_Enterer *) eina_inlist_remove(EINA_INLIST_GET(idle_enterers), EINA_INLIST_GET(ie));
		  ECORE_MAGIC_SET(ie, ECORE_MAGIC_NONE);
		  free(ie);
	       }
	  }
	idle_enterers_delete_me = 0;
     }
}

int
_ecore_idle_enterer_exist(void)
{
   if (idle_enterers) return 1;
   return 0;
}
