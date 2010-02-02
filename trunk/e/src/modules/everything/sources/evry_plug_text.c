#include "Evry.h"

static Evry_Plugin *p1;
static Evry_Plugin *p2;



static void
_cleanup(Evry_Plugin *p)
{
   EVRY_PLUGIN_ITEMS_FREE(p);
}

static int
_fetch(Evry_Plugin *p, const char *input)
{
   Evry_Item *it;

   EVRY_PLUGIN_ITEMS_FREE(p);
   
   it = evry_item_new(NULL, p, input, NULL);

   EVRY_PLUGIN_ITEM_APPEND(p, it);

   return 1;
}

static Eina_Bool
_init(void)
{
   p1 = evry_plugin_new(NULL, "Text", type_subject, NULL, "TEXT", 1, "accessories-editor", NULL,
			NULL, _cleanup, _fetch, NULL, NULL, NULL, NULL);

   p2 = evry_plugin_new(NULL, "Text", type_object, NULL, "TEXT", 1, "accessories-editor", NULL,
			NULL, _cleanup, _fetch, NULL, NULL, NULL, NULL);

   evry_plugin_register(p1, 10);
   evry_plugin_register(p2, 10);

   return EINA_TRUE;
}

static void
_shutdown(void)
{
   EVRY_PLUGIN_FREE(p1);
   EVRY_PLUGIN_FREE(p2);
}


EINA_MODULE_INIT(_init);
EINA_MODULE_SHUTDOWN(_shutdown);
