#include "geist.h"
#include "geist_object.h"

geist_object *geist_object_new(void)
{
   geist_object *obj = NULL;
   D_ENTER(5);

   obj = emalloc(sizeof(geist_object));
      
   D_RETURN(5,obj);
}

void geist_object_init(geist_object *obj)
{  
   D_ENTER(5);

   if(!obj)
      eprintf("bug. no object to init.");

   memset(obj, 0, sizeof(geist_object));
   obj->free = geist_object_int_free;
   obj->render = geist_object_render;
   obj->type = TYPE_OBJECT;
   obj->sizemode = SIZEMODE_ZOOM; 
   
   D_RETURN_(5);
}

void geist_object_int_free(geist_object *obj)
{
   D_ENTER(5);

   if(!obj)
      D_RETURN_(5);

   if(obj->name)
      free(obj->name);

   free(obj);
   
   D_RETURN_(5);
}

void geist_object_free(geist_object *obj)
{
   D_ENTER(5);

   if(!obj)
      D_RETURN_(5);

   obj->free(obj);

   free(obj);
   
   D_RETURN_(5);
}

void geist_object_render(geist_object *obj, Imlib_Image dest)
{  
   D_ENTER(5);

   obj->render(obj, dest);
   
   D_RETURN_(5);
}

void geist_object_show(geist_object *obj)
{
   D_ENTER(3);

   obj->visible = TRUE;
   
   D_RETURN_(3);
}
