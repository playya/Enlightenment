#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "evas_gl_routines.h"
#include "evas_imlib_routines.h"
#include "evas_image_routines.h"
#include "evas_x11_routines.h"

static void
_evas_free_text(Evas_Object o)
{
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return;
   oo = o;
   if (oo->current.text) free(oo->current.text);
   free(o);
}

static void
_evas_free_text_renderer_data(Evas e, Evas_Object o)
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
     case RENDER_METHOD_IMAGE:
	break;
     default:
	break;
     }
}

Evas_Object
evas_add_text(Evas e, char *font, int size, char *text)
{
   Evas_Object_Text oo;
   Evas_Object_Any   o;
   Evas_List         l;
   Evas_Layer        layer;
   
   o = oo = malloc(sizeof(struct _Evas_Object_Text));
   memset(o, 0, sizeof(struct _Evas_Object_Text));
   o->type = OBJECT_TEXT;
   o->object_free = _evas_free_text;
   o->object_renderer_data_free = _evas_free_text_renderer_data;

   oo->current.text = strdup(text);
   oo->current.font = strdup(font);
   oo->current.size = size;
   
     {
	switch (e->current.render_method)
	  {
	  case RENDER_METHOD_ALPHA_SOFTWARE:
	       {
		  Evas_Imlib_Font *fn;
		  
		  fn = __evas_imlib_text_font_new (e->current.display, 
						   oo->current.font, 
						   oo->current.size);
		  if (fn)
		    {
		       __evas_imlib_text_get_size(fn, oo->current.text, 
						  &oo->current.string.w, 
						  &oo->current.string.h);
		       __evas_imlib_text_font_free(fn);
		    }
	       }
	     break;
	  case RENDER_METHOD_BASIC_HARDWARE:
	       {
		  Evas_X11_Font *fn;
		  
		  fn = __evas_x11_text_font_new (e->current.display, 
						   oo->current.font, 
						   oo->current.size);
		  if (fn)
		    {
		       __evas_x11_text_get_size(fn, oo->current.text, 
						  &oo->current.string.w, 
						  &oo->current.string.h);
		       __evas_x11_text_font_free(fn);
		    }
	       }
	     break;
	  case RENDER_METHOD_3D_HARDWARE:
	       {
		  Evas_GL_Font *fn;
		  
		  fn = __evas_gl_text_font_new (e->current.display, 
						oo->current.font, 
						oo->current.size);
		  if (fn)
		    {
		       __evas_gl_text_get_size(fn, oo->current.text, 
					       &oo->current.string.w, 
					       &oo->current.string.h);
		       __evas_gl_text_font_free(fn);
		    }
	       }
	     break;
	  case RENDER_METHOD_ALPHA_HARDWARE:
	     break;
	  case RENDER_METHOD_IMAGE:
	       {
		  Evas_Image_Font *fn;
		  
		  fn = __evas_image_text_font_new (e->current.display, 
						   oo->current.font, 
						   oo->current.size);
		  if (fn)
		    {
		       __evas_image_text_get_size(fn, oo->current.text, 
						  &oo->current.string.w, 
						  &oo->current.string.h);
		       __evas_image_text_font_free(fn);
		    }
	       }
	     break;
	  default:
	     break;
	  }	
     }
   
   o->current.x = 0;
   o->current.y = 0;
   o->current.w = (double)oo->current.string.w;
   o->current.h = (double)oo->current.string.h;
   
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

char *
evas_get_text_string(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   IF_OBJ(o, OBJECT_TEXT) return "";
   oo = o;
   return oo->current.text;
}

char *
evas_get_text_font(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   IF_OBJ(o, OBJECT_TEXT) return "";
   oo = o;
   return oo->current.font;
}

int
evas_get_text_size(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;

   IF_OBJ(o, OBJECT_TEXT) return 0;
   oo = o;
   return oo->current.size;
}

int
evas_text_at_position(Evas e, Evas_Object o, double x, double y, 
		      int *char_x, int *char_y, int *char_w, int *char_h)
{
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return -1;
   oo = o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     int ret;
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_imlib_text_get_character_at_pos(fn, oo->current.text,
								(int)(x - o->current.x),
								(int)(y - o->current.y),
								char_x, char_y, 
								char_w, char_h);
		  __evas_imlib_text_font_free(fn);
		  return ret;
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     int ret;
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_x11_text_get_character_at_pos(fn, oo->current.text,
							      (int)(x - o->current.x),
							      (int)(y - o->current.y),
							      char_x, char_y, 
							      char_w, char_h);
		  __evas_x11_text_font_free(fn);
		  return ret;
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     int ret;
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_gl_text_get_character_at_pos(fn, oo->current.text,
							     (int)(x - o->current.x),
							     (int)(y - o->current.y),
							     char_x, char_y, 
							     char_w, char_h);
		  __evas_gl_text_font_free(fn);
		  return ret;
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     int ret;
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  ret =  __evas_image_text_get_character_at_pos(fn, oo->current.text,
								(int)(x - o->current.x),
								(int)(y - o->current.y),
								char_x, char_y, 
								char_w, char_h);
		  __evas_image_text_font_free(fn);
		  return ret;
	       }
	  }
	break;
     default:
	break;
     }
   return 0;
}

void
evas_text_at(Evas e, Evas_Object o, int index, 
	     int *char_x, int *char_y, int *char_w, int *char_h)
{
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return;
   oo = o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_imlib_text_get_character_number(fn, oo->current.text,
							 index,
							 char_x, char_y, 
							 char_w, char_h);
		  __evas_imlib_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_x11_text_get_character_number(fn, oo->current.text,
						       index,
						       char_x, char_y, 
						       char_w, char_h);
		  __evas_x11_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_gl_text_get_character_number(fn, oo->current.text,
						      index,
						      char_x, char_y, 
						      char_w, char_h);
		  __evas_gl_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_image_text_get_character_number(fn, oo->current.text,
							 index,
							 char_x, char_y, 
							 char_w, char_h);
		  __evas_image_text_font_free(fn);
	       }
	  }
	break;
     default:
	break;
     }
}

void
evas_text_get_ascent_descent(Evas e, Evas_Object o, 
			     double *ascent, double *descent)
{
   int a, d;
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return;
   oo = o;
   a = 0; d = 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_imlib_text_font_get_ascent(fn);
		  d = __evas_imlib_text_font_get_descent(fn);
		  __evas_imlib_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_x11_text_font_get_ascent(fn);
		  d = __evas_x11_text_font_get_descent(fn);
		  __evas_x11_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_gl_text_font_get_ascent(fn);
		  d = __evas_gl_text_font_get_descent(fn);
		  __evas_gl_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_image_text_font_get_ascent(fn);
		  d = __evas_image_text_font_get_descent(fn);
		  __evas_image_text_font_free(fn);
	       }
	  }
	break;
     default:
	break;
     }
   if (ascent) *ascent = a;
   if (descent) *descent = d;   
}

void
evas_text_get_max_ascent_descent(Evas e, Evas_Object o, 
				 double *ascent, double *descent)
{
   int a, d;
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return;
   oo = o;
   a = 0; d = 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_imlib_text_font_get_max_ascent(fn);
		  d = __evas_imlib_text_font_get_max_descent(fn);
		  __evas_imlib_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_x11_text_font_get_max_ascent(fn);
		  d = __evas_x11_text_font_get_max_descent(fn);
		  __evas_x11_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_gl_text_font_get_max_ascent(fn);
		  d = __evas_gl_text_font_get_max_descent(fn);
		  __evas_gl_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  a = __evas_image_text_font_get_max_ascent(fn);
		  d = __evas_image_text_font_get_max_descent(fn);
		  __evas_image_text_font_free(fn);
	       }
	  }
	break;
     default:
	break;
     }
   if (ascent) *ascent = a;
   if (descent) *descent = d;   
}

void
evas_text_get_advance(Evas e, Evas_Object o, 
		      double *h_advance, double *v_advance)
{
   int a, d;
   Evas_Object_Text oo;
   
   IF_OBJ(o, OBJECT_TEXT) return;
   oo = o;
   a = 0; d = 0;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_imlib_text_font_get_advances(fn, oo->current.text, &a, &d);
		  __evas_imlib_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_x11_text_font_get_advances(fn, oo->current.text, &a, &d);
		  __evas_x11_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_gl_text_font_get_advances(fn, oo->current.text, &a, &d);
		  __evas_gl_text_font_free(fn);
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  __evas_image_text_font_get_advances(fn, oo->current.text, &a, &d);
		  __evas_image_text_font_free(fn);
	       }
	  }
	break;
     default:
	break;
     }
   if (h_advance) *h_advance = a;
   if (v_advance) *v_advance = d;   
}

double
evas_text_get_inset(Evas e, Evas_Object o)
{
   Evas_Object_Text oo;
   int inset;
   
   IF_OBJ(o, OBJECT_TEXT) return 0.0;
   oo = o;
   switch (e->current.render_method)
     {
     case RENDER_METHOD_ALPHA_SOFTWARE:
	  {
	     Evas_Imlib_Font *fn;
	     
	     fn = __evas_imlib_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  inset = __evas_imlib_text_font_get_first_inset(fn, oo->current.text);
		  __evas_imlib_text_font_free(fn);
		  return (double)inset;
	       }
	  }
	break;
     case RENDER_METHOD_BASIC_HARDWARE:
	  {
	     Evas_X11_Font *fn;
	     
	     fn = __evas_x11_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  inset = __evas_x11_text_font_get_first_inset(fn, oo->current.text);
		  __evas_x11_text_font_free(fn);
		  return (double)inset;
	       }
	  }
	break;
     case RENDER_METHOD_3D_HARDWARE:
	  {
	     Evas_GL_Font *fn;
	     
	     fn = __evas_gl_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  inset = __evas_gl_text_font_get_first_inset(fn, oo->current.text);
		  __evas_gl_text_font_free(fn);
		  return (double)inset;
	       }
	  }
	break;
     case RENDER_METHOD_ALPHA_HARDWARE:
	break;
     case RENDER_METHOD_IMAGE:
	  {
	     Evas_Image_Font *fn;
	     
	     fn = __evas_image_text_font_new(e->current.display, oo->current.font, oo->current.size);
	     if (fn)
	       {
		  inset = __evas_image_text_font_get_first_inset(fn, oo->current.text);
		  __evas_image_text_font_free(fn);
		  return (double)inset;
	       }
	  }
	break;
     default:
	break;
     }
   return 0;
}

void
evas_set_text(Evas e, Evas_Object o, char *text)
{
   switch (o->type)
     {
     case OBJECT_TEXT:
	  {
	     Evas_Object_Text oo;
	     
	     oo = (Evas_Object_Text)o;
	     if (oo->current.text) free(oo->current.text);
	     oo->current.text = strdup(text);
	     oo->previous.text = NULL;
	       {	     
		  switch (e->current.render_method)
		    {
		    case RENDER_METHOD_ALPHA_SOFTWARE:
			 {
			    Evas_Imlib_Font *fn;
			    
			    fn = __evas_imlib_text_font_new (e->current.display,
							     oo->current.font,
							     oo->current.size);
			    if (fn)
			      {
				 __evas_imlib_text_get_size(fn, oo->current.text,
							    &oo->current.string.w,
							    &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_imlib_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_BASIC_HARDWARE:
			 {
			    Evas_X11_Font *fn;
			    
			    fn = __evas_x11_text_font_new (e->current.display,
							   oo->current.font,
							   oo->current.size);
			    if (fn)
			      {
				 __evas_x11_text_get_size(fn, oo->current.text,
							  &oo->current.string.w,
							  &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_x11_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_3D_HARDWARE:
			 {
			    Evas_GL_Font *fn;
			    
			    fn = __evas_gl_text_font_new (e->current.display,
							  oo->current.font,
							  oo->current.size);
			    if (fn)
			      {
				 __evas_gl_text_get_size(fn, oo->current.text,
							 &oo->current.string.w,
							 &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_gl_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_ALPHA_HARDWARE:
		       break;
		    case RENDER_METHOD_IMAGE:
			 {
			    Evas_Image_Font *fn;
			    
			    fn = __evas_image_text_font_new (e->current.display,
							     oo->current.font,
							     oo->current.size);
			    if (fn)
			      {
				 __evas_image_text_get_size(fn, oo->current.text,
							    &oo->current.string.w,
							    &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_image_text_font_free(fn);
			      }
			 }
		       break;
		    default:
		       break;
		    }
	       }
	     o->current.w = (double)oo->current.string.w;
	     o->current.h = (double)oo->current.string.h;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     default:
	break;
     }
}

void
evas_set_font(Evas e, Evas_Object o, char *font, int size)
{
   switch (o->type)
     {
     case OBJECT_TEXT:
	  {
	     Evas_Object_Text oo;
	     
	     oo = (Evas_Object_Text)o;
	     if (oo->current.font) free(oo->current.font);
	     oo->current.font = strdup(font);
	     oo->previous.font = NULL;
	     oo->current.size = size;
	       {	     
		  switch (e->current.render_method)
		    {
		    case RENDER_METHOD_ALPHA_SOFTWARE:
			 {
			    Evas_Imlib_Font *fn;
			    
			    fn = __evas_imlib_text_font_new (e->current.display,
							     oo->current.font,
							     oo->current.size);
			    if (fn)
			      {
				 __evas_imlib_text_get_size(fn, oo->current.text,
							    &oo->current.string.w,
							    &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_imlib_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_BASIC_HARDWARE:
			 {
			    Evas_X11_Font *fn;
			    
			    fn = __evas_x11_text_font_new (e->current.display,
							   oo->current.font,
							   oo->current.size);
			    if (fn)
			      {
				 __evas_x11_text_get_size(fn, oo->current.text,
							  &oo->current.string.w,
							  &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_x11_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_3D_HARDWARE:
			 {
			    Evas_GL_Font *fn;
			    
			    fn = __evas_gl_text_font_new (e->current.display,
							  oo->current.font,
							  oo->current.size);
			    if (fn)
			      {
				 __evas_gl_text_get_size(fn, oo->current.text,
							 &oo->current.string.w,
							 &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_gl_text_font_free(fn);
			      }
			 }
		       break;
		    case RENDER_METHOD_ALPHA_HARDWARE:
		       break;
		    case RENDER_METHOD_IMAGE:
			 {
			    Evas_Image_Font *fn;
			    
			    fn = __evas_image_text_font_new (e->current.display,
							     oo->current.font,
							     oo->current.size);
			    if (fn)
			      {
				 __evas_image_text_get_size(fn, oo->current.text,
							    &oo->current.string.w,
							    &oo->current.string.h);
				 evas_resize(e, o, 
					     (double)oo->current.string.w,
					     (double)oo->current.string.h);
				 __evas_image_text_font_free(fn);
			      }
			 }
		       break;
		    default:
		       break;
		    }
	       }
	     o->current.w = (double)oo->current.string.w;
	     o->current.h = (double)oo->current.string.h;
	  }
	o->changed = 1;
	e->changed = 1;
	break;
     default:
	break;
     }
}
