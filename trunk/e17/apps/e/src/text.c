#include "text.h"

E_Text *
e_text_new(Evas evas, char *text, char *class)
{
   E_Text *t;
   
   t = NEW(E_Text, 1);
   ZERO(t, E_Text, 1);
   t->state = strdup("normal");
   if (class) t->class = strdup(class);
   else t->class = strdup("");
   if (text) t->text = strdup(text);
   else t->text = strdup("");
   t->evas = evas;
   t->obj.o1 = evas_add_text(t->evas, "borzoib", 8, t->text);
   t->obj.o2 = evas_add_text(t->evas, "borzoib", 8, t->text);
   t->obj.o3 = evas_add_text(t->evas, "borzoib", 8, t->text);
   t->obj.o4 = evas_add_text(t->evas, "borzoib", 8, t->text);
   t->obj.text = evas_add_text(t->evas, "borzoib", 8, t->text);
   evas_set_color(t->evas, t->obj.o1, 0, 0, 0, 255);
   evas_set_color(t->evas, t->obj.o2, 0, 0, 0, 255);
   evas_set_color(t->evas, t->obj.o3, 0, 0, 0, 255);
   evas_set_color(t->evas, t->obj.o4, 0, 0, 0, 255);
   evas_set_color(t->evas, t->obj.text, 255, 255, 255, 255);
   evas_move(t->evas, t->obj.o1, t->x + 1, t->y);
   evas_move(t->evas, t->obj.o2, t->x, t->y + 1);
   evas_move(t->evas, t->obj.o3, t->x + 2, t->y + 1);
   evas_move(t->evas, t->obj.o4, t->x + 1, t->y + 2);
   evas_move(t->evas, t->obj.text, t->x + 1, t->y + 1);
   t->color.r = 255;
   t->color.g = 255;
   t->color.b = 255;
   t->color.a = 255;
   t->w = evas_get_text_width(t->evas, t->obj.text) + 2;
   t->h = evas_get_text_height(t->evas, t->obj.text) + 2;
   t->min.w = t->w;
   t->min.h = t->h;
   t->max.w = t->w;
   t->max.h = t->h;
   return t;
}

void
e_text_free(E_Text *t)
{
   IF_FREE(t->state);
   IF_FREE(t->class);
   IF_FREE(t->text);
   
   if ((t->evas) && (t->obj.text))
     {
	evas_del_object(t->evas, t->obj.o1);
	evas_del_object(t->evas, t->obj.o2);
	evas_del_object(t->evas, t->obj.o3);
	evas_del_object(t->evas, t->obj.o4);
	evas_del_object(t->evas, t->obj.text);
     }
   FREE(t);
}

void
e_text_set_text(E_Text *t, char *text)
{
   if (!text) text = "";
   if (!strcmp(t->text, text)) return;
   IF_FREE(t->text);
   t->text = strdup(text);
   evas_set_text(t->evas, t->obj.text, t->text);
   t->w = evas_get_text_width(t->evas, t->obj.text) + 2;
   t->h = evas_get_text_height(t->evas, t->obj.text) + 2;
   t->min.w = t->w;
   t->min.h = t->h;
   t->max.w = t->w;
   t->max.h = t->h;
}

void
e_text_set_layer(E_Text *t, int l)
{
   if (t->layer == l) return;
   t->layer = l;
   evas_set_layer(t->evas, t->obj.o1, t->layer);
   evas_set_layer(t->evas, t->obj.o2, t->layer);
   evas_set_layer(t->evas, t->obj.o3, t->layer);
   evas_set_layer(t->evas, t->obj.o4, t->layer);
   evas_set_layer(t->evas, t->obj.text, t->layer);
}

void
e_text_set_clip(E_Text *t, Evas_Object clip)
{
   evas_set_clip(t->evas, t->obj.o1, clip);
   evas_set_clip(t->evas, t->obj.o2, clip);
   evas_set_clip(t->evas, t->obj.o3, clip);
   evas_set_clip(t->evas, t->obj.o4, clip);
   evas_set_clip(t->evas, t->obj.text, clip);
}

void
e_text_unset_clip(E_Text *t)
{
   evas_unset_clip(t->evas, t->obj.o1);
   evas_unset_clip(t->evas, t->obj.o2);
   evas_unset_clip(t->evas, t->obj.o3);
   evas_unset_clip(t->evas, t->obj.o4);
   evas_unset_clip(t->evas, t->obj.text);
}

void
e_text_raise(E_Text *t)
{
   evas_raise(t->evas, t->obj.o1);
   evas_raise(t->evas, t->obj.o2);
   evas_raise(t->evas, t->obj.o3);
   evas_raise(t->evas, t->obj.o4);
   evas_raise(t->evas, t->obj.text);
}

void
e_text_lower(E_Text *t)
{
   evas_lower(t->evas, t->obj.text);
   evas_lower(t->evas, t->obj.o4);
   evas_lower(t->evas, t->obj.o3);
   evas_lower(t->evas, t->obj.o2);
   evas_lower(t->evas, t->obj.o1);
}

void
e_text_show(E_Text *t)
{
   if (t->visible) return;
   t->visible = 1;
   evas_show(t->evas, t->obj.o1);
   evas_show(t->evas, t->obj.o2);
   evas_show(t->evas, t->obj.o3);
   evas_show(t->evas, t->obj.o4);
   evas_show(t->evas, t->obj.text);
}

void
e_text_hide(E_Text *t)
{
   if (!t->visible) return;
   t->visible = 0;
   evas_hide(t->evas, t->obj.o1);
   evas_hide(t->evas, t->obj.o2);
   evas_hide(t->evas, t->obj.o3);
   evas_hide(t->evas, t->obj.o4);
   evas_hide(t->evas, t->obj.text);
}

void
e_text_set_color(E_Text *t, int r, int g, int b, int a)
{
   if ((r == t->color.r) &&
       (g == t->color.g) &&
       (b == t->color.b) &&
       (a == t->color.a)) return;
   t->color.r = r;
   t->color.g = g;
   t->color.b = b;
   t->color.a = a;
   evas_set_color(t->evas, t->obj.text, t->color.r, t->color.g, t->color.b, t->color.a);
}

void
e_text_move(E_Text *t, double x, double y)
{
   if ((t->x == x) && (t->y == y)) return;
   t->x = x;
   t->y = y;
   evas_move(t->evas, t->obj.o1, t->x + 1, t->y);
   evas_move(t->evas, t->obj.o2, t->x, t->y + 1);
   evas_move(t->evas, t->obj.o3, t->x + 2, t->y + 1);
   evas_move(t->evas, t->obj.o4, t->x + 1, t->y + 2);
   evas_move(t->evas, t->obj.text, t->x + 1, t->y + 1);
}

void
e_text_resize(E_Text *t, double w, double h)
{
   UN(t);
   UN(w);
   UN(h);
}

void
e_text_get_geometry(E_Text *t, double *x, double *y, double *w, double *h)
{
   if (x) *x = t->x;
   if (y) *y = t->y;
   if (w) *w = t->w;
   if (h) *h = t->h;
}

void
e_text_get_min_size(E_Text *t, double *w, double *h)
{
   if (w) *w = t->min.w;
   if (h) *h = t->min.h;
}

void
e_text_get_max_size(E_Text *t, double *w, double *h)
{
   if (w) *w = t->max.w;
   if (h) *h = t->max.h;
}

void
e_text_set_state(E_Text *t, char *state)
{
   UN(t);
   UN(state);
}

void
e_text_set_class(E_Text *t, char *class)
{
   UN(t);
   UN(class);
}

void
e_text_update_class(E_Text *t)
{
   UN(t);
}
