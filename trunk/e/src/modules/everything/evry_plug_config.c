#include "e.h"
#include "e_mod_main.h"

static Evry_Plugin_Class class;
static Evry_Plugin *_plug_new();
static void  _plug_free(Evry_Plugin *p);
static int  _fetch(Evry_Plugin *p, const char *input);
static int  _action(Evry_Plugin *p, Evry_Item *item, const char *input);
static void _cleanup(Evry_Plugin *p);
static void _item_add(Evry_Plugin *p, E_Configure_It *eci, int prio);
static void _item_icon_get(Evry_Plugin *p, Evry_Item *it, Evas *e);
static int  _cb_sort(const void *data1, const void *data2);

EAPI int
evry_plug_config_init(void)
{
   class.name = "Settings";
   class.type_in  = "NONE";
   class.type_out = "NONE";
   class.instances = NULL;
   class.new = &_plug_new;
   class.free = &_plug_free;
   evry_plugin_register(&class);
   return 1;
}

EAPI int
evry_plug_config_shutdown(void)
{
   evry_plugin_unregister(&class);
   
   return 1;
}

static Evry_Plugin *
_plug_new()
{
   Evry_Plugin *p = E_NEW(Evry_Plugin, 1);
   p->class = &class;
   p->fetch = &_fetch;
   p->action = &_action;
   p->cleanup = &_cleanup;
   p->icon_get = &_item_icon_get;
   p->items = NULL;   
}


static void
_plug_free(Evry_Plugin *p)
{
   _cleanup(p);
   E_FREE(p);
}

static int
_action(Evry_Plugin *p, Evry_Item *it, const char *input)
{
   E_Configure_It *eci, *eci2;
   E_Container *con;
   E_Configure_Cat *ecat;
   Eina_List *l, *ll;
   char buf[1024];
   int found = 0;

   if (!it) return 0;
   
   eci = it->data[0];
   con = e_container_current_get(e_manager_current_get());

   for (l = e_configure_registry; l && !found; l = l->next)
     {
	ecat = l->data;
	for (ll = ecat->items; ll && !found; ll = ll->next)
	  {
	     eci2 = ll->data;
	     if (eci == eci2)
	       {
		  found = 1;
		  snprintf(buf, sizeof(buf), "%s/%s",
			   ecat->cat, 
			   eci->item);
	       }
	  }
     }

   if (found)
     e_configure_registry_call(buf, con, NULL);   
   
   return 1;
}

static void
_cleanup(Evry_Plugin *p)
{
   Evry_Item *it;

   EINA_LIST_FREE(p->items, it)
     {
	if (it->label) eina_stringshare_del(it->label);
	if (it->o_icon) evas_object_del(it->o_icon);
	free(it);
     }
}

static int
_fetch(Evry_Plugin *p, const char *input)
{
   E_Manager *man;
   E_Zone *zone;
   
   char match1[4096];
   char match2[4096];
   Eina_List *l, *ll;
   E_Configure_Cat *ecat;
   E_Configure_It *eci;
   
   _cleanup(p); 

   snprintf(match1, sizeof(match1), "%s*", input);
   snprintf(match2, sizeof(match2), "*%s*", input);

   for (l = e_configure_registry; l; l = l->next)
     {
	ecat = l->data;
	if ((ecat->pri >= 0) && (ecat->items))
	  {
	     for (ll = ecat->items; ll; ll = ll->next)
	       {
		  eci = ll->data;
		  if (eci->pri >= 0)
		    {
		       if (e_util_glob_case_match(eci->label, match1))
			 _item_add(p, eci, 1);
		       else if (e_util_glob_case_match(eci->label, match2))
			 _item_add(p, eci, 2);
		       else if (e_util_glob_case_match(ecat->label, match1))
			 _item_add(p, eci, 3);
		       else if (e_util_glob_case_match(ecat->label, match2))
			 _item_add(p, eci, 4);
		    }
	       }
	  }
     }
   
   if (eina_list_count(p->items) > 0)
     {
	p->items = eina_list_sort(p->items, 
				       eina_list_count(p->items),
				       _cb_sort);
	return 1;
     }

   return 0;
}

static void
_item_icon_get(Evry_Plugin *p, Evry_Item *it, Evas *e)
{
   E_Configure_It *eci = it->data[0];
   Evas_Object *o = NULL;
   
   if (eci->icon) 
     {
	o = e_icon_add(e);
	if (!e_util_icon_theme_set(o, eci->icon))
	  {
	     evas_object_del(o);
	     o = e_util_icon_add(eci->icon, e);
	  }
     }

   it->o_icon = o;
}

static void
_item_add(Evry_Plugin *p, E_Configure_It *eci, int prio)
{
   Evry_Item *it;   

   it = calloc(1, sizeof(Evry_Item));
   it->data[0] = eci;
   it->priority = prio;
   it->label = eina_stringshare_add(eci->label);
   it->o_icon = NULL; 
	     
   p->items = eina_list_append(p->items, it);
}


// TODO sort name?
static int
_cb_sort(const void *data1, const void *data2)
{
   const Evry_Item *it1, *it2;
   
   it1 = data1;
   it2 = data2;

   return (it1->priority - it2->priority);
}
