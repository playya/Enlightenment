/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "e.h"
#include "e_mod_main.h"
#include "evry_api.h"
// TODO - show error when input not parseable

static int  _cb_data(void *data, int type, void *event);
static int  _cb_error(void *data, int type, void *event);
static int  _cb_del(void *data, int type, void *event);

static const Evry_API *evry = NULL;
static Evry_Module *evry_module = NULL;
static Eina_Bool active = EINA_FALSE;
static Evry_Plugin *p1;

static Ecore_Exe *exe = NULL;
static Eina_List *history = NULL;
static Eina_List *handlers = NULL;
static int error = 0;

static char _module_icon[] = "accessories-calculator";

static Evry_Plugin *
_begin(Evry_Plugin *p, const Evry_Item *item __UNUSED__)
{
   Evry_Item *it;

   if (history)
     {
	const char *result;

	EINA_LIST_FREE(history, result)
	  {
	     it = EVRY_ITEM_NEW(Evry_Item, p, result, NULL, NULL);
	     it->context = eina_stringshare_ref(p->name);
	     p->items = eina_list_prepend(p->items, it);
	     eina_stringshare_del(result);
	  }
     }

   it = EVRY_ITEM_NEW(Evry_Item, p, "0", NULL, NULL);
   it->context = eina_stringshare_ref(p->name);
   p->items = eina_list_prepend(p->items, it);

   return p;
}

static int
_run_bc(Evry_Plugin *p)
{
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (ECORE_EXE_EVENT_DATA, _cb_data, p));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (ECORE_EXE_EVENT_ERROR, _cb_error, p));
   handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (ECORE_EXE_EVENT_DEL, _cb_del, p));

   exe = ecore_exe_pipe_run("bc -l",
			    ECORE_EXE_PIPE_READ |
			    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
			    ECORE_EXE_PIPE_WRITE |
			    ECORE_EXE_PIPE_ERROR |
			    ECORE_EXE_PIPE_ERROR_LINE_BUFFERED,
			    NULL);
   return !!exe;
}


static void
_cleanup(Evry_Plugin *p)
{
   Ecore_Event_Handler *h;
   Evry_Item *it;
   int items = 10;

   if (p->items)
     {
	evry->item_free(p->items->data);
	p->items = eina_list_remove_list(p->items, p->items);
     }

   EINA_LIST_FREE(p->items, it)
     {
	if (items-- > 0)
	  history = eina_list_prepend(history, eina_stringshare_add(it->label));

	evry->item_free(it);
     }

   EINA_LIST_FREE(handlers, h)
     ecore_event_handler_del(h);

   if (exe)
     {
	ecore_exe_quit(exe);
	ecore_exe_free(exe);
	exe = NULL;
     }
}

/* static int
 * _action(Evry_Plugin *p, const Evry_Item *act __UNUSED__, const Evry_Item *it)
 * {
 *    Eina_List *l;
 *    Evry_Item *it2, *it_old;
 *
 *    /\* remove duplicates *\/
 *    if (p->items->next)
 *      {
 * 	it = p->items->data;
 *
 * 	EINA_LIST_FOREACH(p->items->next, l, it2)
 * 	  {
 * 	     if (!strcmp(it->label, it2->label))
 * 	       break;
 * 	     it2 = NULL;
 * 	  }
 *
 * 	if (it2)
 * 	  {
 * 	     p->items = eina_list_remove(p->items, it2);
 * 	     evry_item_free(it2);
 * 	  }
 *      }
 *
 *    it_old = p->items->data;
 *    it_old->selected = EINA_FALSE;
 *
 *    it2 = evry_item_new(NULL, p, it_old->label, NULL);
 *    it2->context = eina_stringshare_ref(p->name);
 *    p->items = eina_list_prepend(p->items, it2);
 *
 *    evry_plugin_async_update(p, EVRY_ASYNC_UPDATE_ADD);
 *
 *    return EVRY_ACTION_FINISHED;
 * } */

static int
_fetch(Evry_Plugin *p, const char *input)
{
   char buf[1024];

   if (!input) return 0;

   if (!exe && !_run_bc(p)) return 0;

   if (!strncmp(input, "scale=", 6))
     snprintf(buf, 1024, "%s\n", input);
   else
     snprintf(buf, 1024, "scale=3;%s\n", input);

   ecore_exe_send(exe, buf, strlen(buf));

   /* XXX after error we get no response for first input ?! - send a
      second time...*/
   if (error)
     {
	ecore_exe_send(exe, buf, strlen(buf));
	error = 0;
     }

   return 1;
}

static int
_cb_data(void *data, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Data *ev = event;
   Evry_Plugin *p = data;
   Evry_Item *it;

   if (ev->exe != exe) return 1;

   if (ev->lines)
     {
	it = p->items->data;
	eina_stringshare_del(it->label);
	it->label = eina_stringshare_add(ev->lines->line);

	if (it) evry->event_item_changed(it, 0, 0);
     }

   return 1;
}

static int
_cb_error(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Data *ev = event;

   if (ev->exe != exe)
     return 1;

   error = 1;

   return 1;
}

static int
_cb_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *e = event;

   if (e->exe != exe)
     return 1;

   exe = NULL;
   return 1;
}

static int
_plugins_init(const Evry_API *_api)
{   
   if (active) 
     return EINA_TRUE;

   evry = _api;

   if (!evry->api_version_check(EVRY_API_VERSION))
     return EINA_FALSE;

   EVRY_TYPE_TEXT = evry->type_register("TEXT");
   
   p1 = EVRY_PLUGIN_NEW(Evry_Plugin, N_("Calculator"),
			_module_icon,
			EVRY_TYPE_TEXT,
			_begin, _cleanup, _fetch, NULL);

   p1->history     = EINA_FALSE;
   p1->async_fetch = EINA_TRUE;

   if (evry->plugin_register(p1, EVRY_PLUGIN_SUBJECT, 0))
     {
	Plugin_Config *pc = p1->config;
	pc->view_mode = VIEW_MODE_LIST;
	pc->aggregate = EINA_FALSE;
	pc->trigger = eina_stringshare_add("=");
     }

   active = EINA_TRUE;
   
   return EINA_TRUE;
}

static void
_plugins_shutdown(void)
{
   if (!active) return;

   EVRY_PLUGIN_FREE(p1);

   active = EINA_FALSE;
}

/***************************************************************************/

EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "everything-calc"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   Eina_List *l;

   if ((evry = e_datastore_get("everything_loaded")))
     _plugins_init(evry);
   
   evry_module = E_NEW(Evry_Module, 1);
   evry_module->init     = &_plugins_init;
   evry_module->shutdown = &_plugins_shutdown;
   
   l = e_datastore_get("everything_modules");
   l = eina_list_append(l, evry_module);
   e_datastore_set("everything_modules", l);

   e_module_delayed_set(m, 1);
   
   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   Eina_List *l;
   
   if (e_datastore_get("everything_loaded"))
     _plugins_shutdown();

   l = e_datastore_get("everything_modules");
   l = eina_list_remove(l, evry_module);
   e_datastore_set("everything_modules", l);

   E_FREE(evry_module);
   
   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}

/***************************************************************************/
