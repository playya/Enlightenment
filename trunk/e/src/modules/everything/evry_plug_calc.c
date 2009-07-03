/* TODO
 * - dc support?
 */

#include "e.h"
#include "e_mod_main.h"

static int  _begin(Evry_Item *it);
static int  _fetch(const char *input);
static int  _action(Evry_Item *item, const char *input);
static void _cleanup(void);
static void _item_add(char *output, int prio);
static int  _cb_sort(const void *data1, const void *data2);
static void _item_icon_get(Evry_Item *it, Evas *e);

static int  _cb_data(void *data, int type, void *event);


static Evry_Plugin *p;

static Ecore_Exe *exe = NULL;
static Eina_List *history = NULL;
static Ecore_Event_Handler *data_handler = NULL;
static Ecore_Event_Handler *error_handler = NULL;


EAPI int
evry_plug_calc_init(void)
{
   p = E_NEW(Evry_Plugin, 1);
   p->name = "Calculator";
   p->type_in  = "NONE";
   p->type_out = "NONE";
   p->need_query = 0;
   p->prio = 6;
   p->async_query = 1;
   p->begin = &_begin;
   p->fetch = &_fetch;
   p->action = &_action;
   p->cleanup = &_cleanup;
   p->icon_get = &_item_icon_get;
   evry_plugin_register(p);
   
   return 1;
}

EAPI int
evry_plug_calc_shutdown(void)
{
   Evry_Item *it;
   
   EINA_LIST_FREE(p->items, it)
     {
	if (it->label) eina_stringshare_del(it->label);
	free(it);
     }
   
   evry_plugin_unregister(p);
   E_FREE(p);   

   return 1;
}

static int
_begin(Evry_Item *it)
{

   data_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DATA, _cb_data, p);
   
   exe = ecore_exe_pipe_run("bc",
			    ECORE_EXE_PIPE_READ |
			    ECORE_EXE_PIPE_READ_LINE_BUFFERED |
			    ECORE_EXE_PIPE_WRITE,
			    NULL);
}

static void
_cleanup()
{
   Evry_Item *it;
   int i = 0;
   
   EINA_LIST_FREE(p->items, it)
     {
	if (i < 10)
	  {
	     history = eina_list_append(history, it);
	  }
	else
	  {
	     if (it->label) eina_stringshare_del(it->label);
	     free(it);
	  }
     }

   ecore_event_handler_del(data_handler);
   data_handler = NULL;
   
   ecore_exe_quit(exe);
   exe = NULL;
}

static int
_send_input(const char *input)
{
   char buf[1024];
   snprintf(buf, 1024, "%s\n", input);

   return ecore_exe_send(exe, buf, strlen(buf));
}

static int
_action(Evry_Item *it, const char *input)
{
   if (!it)
     {
	if (p->items)
	  {
	     Evry_Item *it2 = p->items->data;
	     
	     _item_add((char *) it2->label, 1);
	     evry_plugin_async_update(p, 1);
	  }
	
	return EVRY_ACTION_CONTINUE;
     }
   else
     {
	/* XXX on which windows must the selection be set? */
	ecore_x_selection_primary_set(e_manager_current_get()->win,
					it->label, strlen(it->label));
	if (p->items->data == it)
	  {
	     Evry_Item *it2 = p->items->data;
	     _item_add((char *) it2->label, 1);
	  }
     }

   return EVRY_ACTION_FINISHED;
}

static int
_fetch(const char *input)
{
   if (history)
     {
	p->items = history;
	history = NULL;
     }
   
   _send_input(input);
   
   return 1;
}

static void
_item_icon_get(Evry_Item *it, Evas *e)
{ 
   it->o_icon = NULL;
}

static void
_item_add(char *output, int prio)
{
   Evry_Item *it;   

   it = E_NEW(Evry_Item, 1);

   it->priority = prio;
   it->label = eina_stringshare_add(output);
	     
   p->items = eina_list_prepend(p->items, it);
}

static int
_cb_data(void *data, int type, void *event)
{
   Ecore_Exe_Event_Data *ev = event;
   Ecore_Exe_Event_Data_Line *l;
   
   if (data != p) return 1;

   evry_plugin_async_update(p, 0);

   for (l = ev->lines; l && l->line; l++)
     {
	if (p->items)
	  {
	     Evry_Item *it = p->items->data;
	     if (it->label)
	       eina_stringshare_del(it->label);
	     it->label = eina_stringshare_add(l->line);
	  }
	else
	  _item_add(l->line, 1);
     }

   evry_plugin_async_update(p, 1);
   
   return 1;
}
