#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void
_evas_free_rectangle(Evas_Object o)
{
   Evas_Object_Rectangle oo;
   
   oo = o;
   free(o);
}

static void
_evas_free_rectangle_renderer_data(Evas e, Evas_Object o)
{
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	break;
     case RENDER_METHOD_3D_HARDWARE:
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     default:
	break;
     }
}

Evas_Object
evas_add_rectangle(Evas e)
{
   Evas_Object_Rectangle oo;
   Evas_Object_Any       o;
   Evas_List             l;
   Evas_Layer            layer;
   
   o = oo = malloc(sizeof(struct _Evas_Object_Rectangle));
   memset(o, 0, sizeof(struct _Evas_Object_Rectangle));
   o->type = OBJECT_RECTANGLE;
   o->object_free = _evas_free_rectangle;
   o->object_renderer_data_free = _evas_free_rectangle_renderer_data;

   o->current.x = 0;
   o->current.y = 0;
   o->current.w = 1;
   o->current.h = 1;

   for (l = e->layers; l; l = l->next)
     {
	layer = l->data;
	if (layer->layer == o->current.layer)
	  {
	     layer->objects = evas_list_append(layer->objects, o);
	     return o;
	  }
     }
   
   layer = malloc(sizeof(struct _Evas_Layer));
   memset(layer, 0, sizeof(struct _Evas_Layer));
   e->layers = evas_list_append(e->layers, layer);
   layer->objects = evas_list_append(layer->objects, o);

   return o;
}
