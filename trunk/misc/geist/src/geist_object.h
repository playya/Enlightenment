#ifndef GEIST_OBJECT_H
#define GEIST_OBJECT_H

#include "geist.h"
#include "geist_document.h"

#define GEIST_OBJECT(O) ((geist_object *) O)

typedef enum __geist_object_state
{ SELECTED = 1UL << 0, HILITED = 1UL << 1, DRAG = 1UL << 2 }
geist_object_state;

struct __geist_object
{
   char *name;
   int w;
   int h;
   int x;
   int y;
   int clicked_x;
   int clicked_y;
   int visible;
   unsigned long int state;
   enum
   { SIZEMODE_ZOOM, SIZEMODE_STRETCH, SIZEMODE_CENTER, SIZEMODE_LEFT,
      SIZEMODE_RIGHT
   }
   sizemode;
   void (*free) (geist_object * obj);
   void (*render) (geist_object * obj, Imlib_Image im);
   void (*render_selected) (geist_object * obj, Imlib_Image im, unsigned char multiple);
   void (*render_partial) (geist_object * obj, Imlib_Image im, int x, int y, int w, int h);
};

/* allocation functions */
geist_object *geist_object_new(void);
void geist_object_init(geist_object * obj);
void geist_object_free(geist_object * obj);

void geist_object_render(geist_object * obj, Imlib_Image dest);
void geist_object_render_selected(geist_object * obj, Imlib_Image dest, unsigned char multiple);
void geist_object_render_partial(geist_object *obj, Imlib_Image dest, int x, int y, int w, int h);
void geist_object_show(geist_object * obj);
void geist_object_raise(geist_document *doc, geist_object * obj);
void geist_object_int_free(geist_object * obj);
void geist_object_int_render(geist_object *obj, Imlib_Image dest);
void geist_object_int_render_selected(geist_object * obj, Imlib_Image dest, unsigned char multiple);
void geist_object_int_render_partial(geist_object *obj, Imlib_Image dest, int x, int y, int w, int h);
#define geist_object_set_state(o, s) (o->state |=  s)
#define geist_object_unset_state(o, s) (o->state &= ~(s))
#define geist_object_get_state(o, s) (o->state & s)
#define geist_object_toggle_state(o, s) ((o->state & s) ? (o->state &= ~(s)) : (o->state |=  s))

#endif
