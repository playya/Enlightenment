#ifndef GEIST_OBJECT_H
#define GEIST_OBJECT_H

typedef enum __geist_object_state
{ SELECTED = 0x00000001, HILITED = 0x00000002, DRAG = 0x00000004 }
geist_object_state;

struct __geist_object
{
   char *name;
   int width;
   int height;
   int x;
   int y;
   int visible;
   long int state;
   enum
   { SIZEMODE_ZOOM, SIZEMODE_STRETCH, SIZEMODE_CENTER, SIZEMODE_LEFT,
      SIZEMODE_RIGHT
   }
   sizemode;
   void (*free) (geist_object * obj);
   void (*render) (geist_object * obj, Imlib_Image im);
};

/* allocation functions */
geist_object *geist_object_new(void);
void geist_object_init(geist_object * obj);
void geist_object_free(geist_object * obj);

void geist_object_render(geist_object * obj, Imlib_Image dest);
void geist_object_show(geist_object * obj);
void geist_object_int_free(geist_object * obj);
#define geist_object_set_state(obj, state) (obj->state |=  state)
#define geist_object_unset_state(obj, state) (obj->state &= ^state)
#define geist_object_get_state(obj, state) (obj->state & state)

#endif
