#include <string.h>
#include <dlfcn.h>

#include "evolve_private.h"

extern void *handle;

/* create a new internal evovle signal */
Evolve_Signal *evolve_signal_new(char *name, char *emission, Evolve *evolve)
{
   Evolve_Signal *esig;
   
   esig = calloc(1, sizeof(Evolve_Signal));
   esig->name = strdup(name);
   esig->emit = strdup(emission);
   esig->evolve = evolve;
   evolve->emissions = evas_hash_add(evolve->emissions, emission, esig);
   return esig;
}

/* callback that runs for internal signals and in turn calls wigget specific callbacks for that signal */
void evolve_signal_emit_cb(void *data, Etk_Object *obj)
{
   Evolve_Signal *esig;
   Evas_List *cbs;
   Evas_List *l;
   
   esig = data;
   
   if (!(cbs = evas_hash_find(esig->evolve->callbacks, esig->emit)))
     return;
   
   for (l = cbs; l; l = l->next)
     {
	Evolve_Signal_Callback *cb;
	
	cb = l->data;
	if(cb->func)
	  cb->func(esig->emit, cb->data);
     }
}

/* connect a callback to a custom evolve signal */
void evolve_signal_connect(Evolve *evolve, char *emission, void (*callback)(char *emission, void *data), void *data)
{
   Evas_List *l = NULL;
   Evolve_Signal_Callback *sig_cb;
   
   if (!evolve || !emission || !callback)
     return;
      
   sig_cb = calloc(1, sizeof(Evolve_Signal_Callback));   
   sig_cb->func = callback;
   sig_cb->data = data;   
   
   if ((l = evas_hash_find(evolve->callbacks, emission)))
     l = evas_list_append(l, sig_cb);     
   else
     {
	l = evas_list_append(l, sig_cb);
	evolve->callbacks = evas_hash_add(evolve->callbacks, emission, l);
     }
}
