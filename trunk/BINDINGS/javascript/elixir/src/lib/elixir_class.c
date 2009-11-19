#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include <eina_hash.h>

#include "Elixir.h"

static Eina_Hash*   class = NULL;

typedef struct elixir_class_s   elixir_class_t;
struct elixir_class_s
{
   elixir_class_t        *parent;
   JSClass                class;
   char                   name[1];
};

void
elixir_class_init(void)
{
   eina_init();

   class = eina_hash_string_superfast_new(free);

   color_parameter.class = elixir_class_request("color", NULL);
   size_parameter.class = elixir_class_request("size", NULL);
   geometry_parameter.class = elixir_class_request("geometry", NULL);
   position_parameter.class = elixir_class_request("position", NULL);
   eet_parameter.class = elixir_class_request("eet_data", NULL);
}

void
elixir_class_shutdown(void)
{
   eina_hash_free(class);
   class = NULL;

   size_parameter.class = NULL;
   color_parameter.class = NULL;
   geometry_parameter.class = NULL;
   position_parameter.class = NULL;
   eet_parameter.class = NULL;

   eina_shutdown();
}

JSClass*
elixir_class_request(const char *class_name, const char *class_parent)
{
   elixir_class_t       *parent = NULL;
   elixir_class_t       *news;

   news = eina_hash_find(class, class_name);
   if (news)
     return &news->class;

   if (class_parent)
     {
        parent = eina_hash_find(class, class_parent);
        if (!parent)
          return NULL;
     }

   news = calloc(1, sizeof (elixir_class_t) + strlen(class_name));
   if (!news)
     return NULL;

   memcpy(news->name, class_name, strlen(class_name) + 1);
   news->parent = parent;

   news->class.name = news->name;

   news->class.flags = JSCLASS_HAS_PRIVATE;

   news->class.addProperty = JS_PropertyStub;
   news->class.delProperty = JS_PropertyStub;
   news->class.getProperty = JS_PropertyStub;
   news->class.setProperty = JS_PropertyStub;
   news->class.enumerate = JS_EnumerateStub;
   news->class.resolve = JS_ResolveStub;
   news->class.convert = JS_ConvertStub;
   news->class.finalize = JS_FinalizeStub;

   eina_hash_direct_add(class, news->name, news);

   return &news->class;
}

static elixir_class_t*
elixir_class_request_lookup(const char *class_name)
{
   elixir_class_t       *lookup;

   lookup = eina_hash_find(class, class_name);
   if (!lookup)
     return NULL;
   return lookup->parent;
}

const char*
elixir_class_request_parent_name(const char *class_name)
{
   elixir_class_t       *parent;

   parent = elixir_class_request_lookup(class_name);
   if (!parent)
     return NULL;
   return parent->name;
}

JSClass*
elixir_class_request_parent(const char *class_name)
{
   elixir_class_t       *parent;

   parent = elixir_class_request_lookup(class_name);
   if (!parent)
     return NULL;
   return &parent->class;
}
