#include "layers.h"

geist_layer *
geist_layer_new(void)
{
   geist_layer *l = NULL;

   D_ENTER(3);

   l = emalloc(sizeof(geist_layer));

   memset(l, 0, sizeof(geist_layer));

   l->visible = TRUE;

   D_RETURN(3, l);
}

void
geist_layer_free(geist_layer * layer)
{
   geist_list *l;

   D_ENTER(3);

   for (l = layer->objects; l; l = l->next)
      geist_object_free(((geist_object *) l->data));

   geist_list_free(layer->objects);

   free(layer);

   D_RETURN_(3);
}

void
geist_layer_render(geist_layer * layer, Imlib_Image dest)
{
   geist_list *l;

   D_ENTER(3);


   if (layer->visible)
   {
      D(3, ("rendering layer %p\n", layer));
      for (l = layer->objects; l; l = l->next)
         geist_object_render(((geist_object *) l->data), dest);
   }

   D_RETURN_(3);
}

void
geist_layer_add_object(geist_layer * layer, geist_object * obj)
{
   D_ENTER(3);

   if (!obj)
      D_RETURN_(3);

   layer->objects = geist_list_add_end(layer->objects, obj);

   D_RETURN_(3);
}
