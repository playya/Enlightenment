#ifndef GEIST_OBJECT_H
#define GEIST_OBJECT_H

#include "geist.h"
#include "geist_document.h"

extern GtkWidget *obj_list;

#define GEIST_OBJECT(O) ((geist_object *) O)
#define GEIST_OBJECT_DOC(o) (((geist_object *)o)->layer->doc)

enum __sizemode
{
   SIZEMODE_NONE,
   SIZEMODE_ZOOM,
   SIZEMODE_STRETCH
};

enum __alignment
{
   ALIGN_NONE,
   ALIGN_HCENTER,
   ALIGN_VCENTER,
   ALIGN_CENTER,
   ALIGN_LEFT,
   ALIGN_RIGHT,
   ALIGN_TOP,
   ALIGN_BOTTOM
};

enum __resize_type
{
   RESIZE_NONE,
   RESIZE_LEFT,
   RESIZE_TOPLEFT,
   RESIZE_TOP,
   RESIZE_TOPRIGHT,
   RESIZE_RIGHT,
   RESIZE_BOTTOMRIGHT,
   RESIZE_BOTTOM,
   RESIZE_BOTTOMLEFT
};

typedef enum __geist_object_state
{ SELECTED = 1UL << 0, HILITED = 1UL << 1, DRAG = 1UL << 2, RESIZE =
      1UL << 3, VISIBLE = 1UL << 4
}
geist_object_state;

struct __geist_object
{
   geist_object_type type;
   char *name;
   geist_layer *layer;
   /* Object position/size */
   int x;
   int y;
   int w;
   int h;
   /* x,y offset of rendered image within object */
   int rendered_x;
   int rendered_y;
   /* Actual size of rendered image/shape */
   int rendered_w;
   int rendered_h;
   /* where was the object clicked? (for dragging etc) */
   int clicked_x;
   int clicked_y;
   /* type of resize being performed */
   int resize;
   /* sizing mode */
   int sizemode;
   /* alignment */
   int alignment;
   /* object state */
   unsigned long int state;
   void (*free) (geist_object * obj);
   void (*render) (geist_object * obj, Imlib_Image im);
   void (*render_selected) (geist_object * obj, Imlib_Image im,
                            unsigned char multiple);
   void (*render_partial) (geist_object * obj, Imlib_Image im, int x, int y,
                           int w, int h);
     Imlib_Image(*get_rendered_image) (geist_object * obj);
     Imlib_Updates(*get_selection_updates) (geist_object * obj);
   geist_object *(*duplicate) (geist_object * obj);
   unsigned char (*part_is_transparent) (geist_object * obj, int x, int y);
   void (*resize_event) (geist_object * obj, int x, int y);
   void (*display_props) (geist_object * obj);
};

/* allocation functions */
geist_object *geist_object_new(void);
void geist_object_init(geist_object * obj);
void geist_object_free(geist_object * obj);

geist_object_type geist_object_get_type(geist_object * obj);
void geist_object_set_type(geist_object * obj, geist_object_type type);
void geist_object_render(geist_object * obj, Imlib_Image dest);
void geist_object_render_selected(geist_object * obj, Imlib_Image dest,

                                  unsigned char multiple);
void geist_object_render_partial(geist_object * obj, Imlib_Image dest, int x,
                                 int y, int w, int h);
void geist_object_show(geist_object * obj);
void geist_object_hide(geist_object * obj);
void geist_object_raise(geist_object * obj);
void geist_object_int_free(geist_object * obj);
void geist_object_int_render(geist_object * obj, Imlib_Image dest);
void geist_object_int_render_selected(geist_object * obj, Imlib_Image dest,

                                      unsigned char multiple);
void geist_object_int_render_partial(geist_object * obj, Imlib_Image dest,
                                     int x, int y, int w, int h);
void geist_object_add_to_object_list(geist_object * obj);
Imlib_Image geist_object_get_rendered_image(geist_object * obj);
Imlib_Image geist_object_int_get_rendered_image(geist_object * obj);
void geist_object_select(geist_object * obj);
Imlib_Updates geist_object_int_get_selection_updates(geist_object * obj);
Imlib_Updates geist_object_get_selection_updates(geist_object * obj);
void geist_object_unselect(geist_object * obj);
geist_object *geist_object_duplicate(geist_object * obj);
geist_object *geist_object_int_duplicate(geist_object * obj);
unsigned char geist_object_part_is_transparent(geist_object * obj, int x,

                                               int y);
void geist_object_move(geist_object * obj, int x, int y);
unsigned char geist_object_int_part_is_transparent(geist_object * obj, int x,

                                                   int y);
int geist_object_check_resize_click(geist_object * obj, int x, int y);
void geist_object_resize(geist_object * obj, int x, int y);
void geist_object_int_resize(geist_object * obj, int x, int y);
void geist_object_resize_object(geist_object * obj, int x, int y);

void geist_object_int_display_props(geist_object * obj);
void geist_object_display_props(geist_object * obj);
void geist_object_dirty_selection(geist_object * obj);
void geist_object_dirty(geist_object * obj);
GtkWidget *geist_object_generic_properties(geist_object *obj);


#define geist_object_set_state(o, s) (o->state |=  s)
#define geist_object_unset_state(o, s) (o->state &= ~(s))
#define geist_object_get_state(o, s) (o->state & s)
#define geist_object_toggle_state(o, s) ((o->state & s) ? (o->state &= ~(s)) : (o->state |=  s))


#endif
