#include "Edje.h"
#include "edje_private.h"

int
edje_object_part_exists(Evas_Object *obj, const char *part)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return 0;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part)) return 1;
     }
   return 0;
}

void
edje_object_part_geometry_get(Evas_Object *obj, const char *part, double *x, double *y, double *w, double *h )
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if ((!strcmp(rp->part->name, part)) && (rp->calculated))
	  {
	     if (x) *x = rp->x;
	     if (y) *y = rp->y;
	     if (w) *w = rp->w;
	     if (h) *h = rp->h;
	     return;
	  }
     }
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
}

void
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part))
	  {
	     if (rp->part->type == EDJE_PART_TYPE_TEXT)
	       {
		  if ((!rp->text.text) && (!text))
		    return;
		  if ((rp->text.text) && (text) && 
		      (!strcmp(rp->text.text, text)))
		    return;
		  if (rp->text.text) free(rp->text.text);
		  rp->text.text = strdup(text);
		  ed->dirty = 1;
		  _edje_recalc(ed);
	       }
	     return;
	  }
     }
}

const char *
edje_object_part_text_get(Evas_Object *obj, const char *part)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return NULL;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part))
	  {
	     if (rp->part->type == EDJE_PART_TYPE_TEXT)
	       return evas_object_text_text_get(rp->object);
	     else
	       return NULL;
	  }
     }
   return NULL;
}

int
edje_object_freeze(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   return _edje_freeze(ed);
}

int
edje_object_thaw(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   return _edje_thaw(ed);
}

void
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Evas_List *l;
   Ejde_Color_Class *cc;

   ed = _edje_fetch(obj);
   if ((!ed) || (!color_class)) return;
   if (r < 0)   r = 0;
   if (r > 255) r = 255;
   if (g < 0)   g = 0;
   if (g > 255) g = 255;
   if (b < 0)   b = 0;
   if (b > 255) b = 255;
   if (a < 0)   a = 0;
   if (a > 255) a = 255;
   for (l = ed->color_classes; l; l = l->next)
     {
	cc = l->data;
	if (!strcmp(cc->name, color_class))
	  {
	     if ((cc->r == r) && (cc->g == g) && 
		 (cc->b == b) && (cc->a == a) &&
		 (cc->r2 == r2) && (cc->g2 == g2) && 
		 (cc->b2 == b2) && (cc->a2 == a2) &&
		 (cc->r3 == r3) && (cc->g3 == g3) && 
		 (cc->b3 == b3) && (cc->a3 == a3))
	       return;
	     cc->r = r;
	     cc->g = g;
	     cc->b = b;
	     cc->a = a;
	     cc->r2 = r2;
	     cc->g2 = g2;
	     cc->b2 = b2;
	     cc->a2 = a2;
	     cc->r3 = r3;
	     cc->g3 = g3;
	     cc->b3 = b3;
	     cc->a3 = a3;
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }
   cc = malloc(sizeof(Ejde_Color_Class));
   cc->name = strdup(color_class);
   if (!cc->name)
     {
	free(cc);
	return;
     }
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;
   ed->color_classes = evas_list_append(ed->color_classes, cc);
   ed->dirty = 1;
   _edje_recalc(ed);
}

void
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, double size)
{
   Edje *ed;
   Evas_List *l;
   Ejde_Text_Class *tc;

   ed = _edje_fetch(obj);
   if ((!ed) || (!text_class)) return;
   if (size < 0.0) size = 0.0;
   for (l = ed->text_classes; l; l = l->next)
     {
	tc = l->data;
	if (!strcmp(tc->name, text_class))
	  {
	     if ((tc->font) && (font) && 
		 (!strcmp(tc->font, font)) &&
		 (tc->size == size))
	       return;
	     if ((!tc->font) && (!font) && 
		 (tc->size == size))
	       return;
	     if (tc->font) free(tc->font);
	     if (font) tc->font = strdup(font);
	     else tc->font = NULL;
	     tc->size = size;
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }
   tc = malloc(sizeof(Ejde_Text_Class));
   tc->name = strdup(text_class);
   if (!tc->name)
     {
	free(tc);
	return;
     }
   if (font) tc->font = strdup(font);
   else tc->font = NULL;
   tc->size = size;
   ed->text_classes = evas_list_append(ed->text_classes, tc);
   ed->dirty = 1;
   _edje_recalc(ed);
}

Ejde_Color_Class *
_edje_color_class_find(Edje *ed, char *color_class)
{
   Evas_List *l;
   
   if (!color_class) return NULL;
   for (l = ed->color_classes; l; l = l->next)
     {
	Ejde_Color_Class *cc;
	
	cc = l->data;
	if (!strcmp(color_class, cc->name)) return cc;
     }
   return NULL;
}

Ejde_Text_Class *
_edje_text_class_find(Edje *ed, char *text_class)
{
   Evas_List *l;
   
   if (!text_class) return NULL;
   for (l = ed->text_classes; l; l = l->next)
     {
	Ejde_Text_Class *tc;
	
	tc = l->data;
	if (!strcmp(text_class, tc->name)) return tc;
     }
   return NULL;
}

Edje *
_edje_fetch(Evas_Object *obj)
{
   Edje *ed;
   char *type;
   
   type = (char *)evas_object_type_get(obj);
   if (!type) return NULL;
   if (strcmp(type, "edje")) return NULL;
   ed = evas_object_smart_data_get(obj);
   return ed;
}

int
_edje_glob_match(char *str, char *glob)
{
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

int
_edje_freeze(Edje *ed)
{
   ed->freeze++;
   return ed->freeze;
}

int
_edje_thaw(Edje *ed)
{
   ed->freeze--;
   if ((ed->freeze <= 0) && (ed->recalc))
     _edje_recalc(ed);
   return ed->freeze;
}
