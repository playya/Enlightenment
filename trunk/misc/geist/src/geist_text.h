#ifndef GEIST_TEXT_H
#define GEIST_TEXT_H

#include "geist.h"
#include "geist_object.h"
#include "geist.h"
#include "geist_imlib.h"
#include "geist_image.h"

#define GEIST_TEXT(O) ((geist_text *) O)

struct __geist_text
{
   geist_object object;
   char *name;
   char *fontname;
   char *text;
   Imlib_Font fn;
   int w;
   int h;
   Imlib_Image im;
   int alias;
   int r, g, b, a;
};

geist_object *geist_text_new(void);
geist_object *geist_text_new_with_text(int x, int y, char *fontname,
                                       char *text, int a, int r, int g,

                                       int b);
void geist_text_init(geist_text * txt);
void geist_text_free(geist_object * obj);
void geist_text_render(geist_object * obj, Imlib_Image dest);
void geist_text_render_partial(geist_object * obj, Imlib_Image dest, int x,
                               int y, int w, int h);
void geist_text_change_text(geist_text * obj, char *newtext);
Imlib_Image geist_text_create_image(geist_text * txt, int *w, int *h);
Imlib_Image geist_text_get_rendered_image(geist_object * obj);
geist_object *geist_text_duplicate(geist_object * obj);
void geist_text_resize(geist_object * obj, int x, int y);
void geist_text_display_props (geist_object *obj);

#endif
