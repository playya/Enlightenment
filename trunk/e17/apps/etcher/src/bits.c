#include "bits.h"
#include "Edb.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static Evas_List __bit_descriptions = NULL;

static Ebits_Object_Description _ebits_find_description(char *file);
static char *_ebits_get_file(Ebits_Object_Bit_Description d, int state);
static void _ebits_sync_bits(Ebits_Object_Bit_State state);
static Ebits_Object_Bit_State _ebits_get_bit_name(Ebits_Object o, char *name);

static Ebits_Object_Bit_State _ebits_get_bit_name(Ebits_Object o, char *name)
{
   return NULL;
}

static void _ebits_sync_bits(Ebits_Object_Bit_State state)
{
   Evas_List l;

   if (state->object)
     {
	double fill_w, fill_h;
	
	evas_set_image_file(state->o->state.evas, state->object,
			    _ebits_get_file(state->description, state->state));
	evas_set_image_border(state->o->state.evas, state->object,
			      state->description->border.l,
			      state->description->border.r,
			      state->description->border.t,
			      state->description->border.b);
	fill_w = state->w;
	if (state->description->tile.w == 1) 
	   {
	      int im_w;
	      
	      evas_get_image_size(state->o->state.evas, state->object, &im_w, NULL);
	      if (im_w > 0) fill_w = im_w;
	   }
	else if (state->description->tile.w == 2)
	   {
	      int im_w;
	      
	      evas_get_image_size(state->o->state.evas, state->object, &im_w, NULL);
	      if (im_w > 0)
		{
		   int num;
		   
		   num = (int)(state->w / (double)im_w);
		   if (num < 1) num = 1;
		   fill_w = state->w / (double)num;
		}
	   }
	fill_h = state->h;
	if (state->description->tile.h == 1) 
	   {
	      int im_h;
	      
	      evas_get_image_size(state->o->state.evas, state->object, NULL, &im_h);
	      if (im_h > 0) fill_h = im_h;
	   }
	else if (state->description->tile.h == 2)
	   {
	      int im_h;
	      
	      evas_get_image_size(state->o->state.evas, state->object, NULL, &im_h);
	      if (im_h > 0)
		{
		   int num;
		   
		   num = (int)(state->h / (double)im_h);
		   if (num < 1) num = 1;
		   fill_h = state->h / (double)num;
		}
	   }
	evas_set_image_fill(state->o->state.evas, state->object,
			    0, 0, fill_w, fill_h);
     }
   for (l = state->description->sync; l; l = l->next)
     {
	Ebits_Object_Bit_State state2;
	
	state2 = _ebits_get_bit_name(state->o, l->data);
	if (state2)
	  {
	     state2->state = state->state;
	     _ebits_sync_bits(state2);
	  }
     }
}

static void
_ebits_handle_mouse_down (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   Ebits_Object_Bit_State state;
   
   state = _data;
   if (state->state == 3) return;
   state->state = 2;
}

static void
_ebits_handle_mouse_up (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   Ebits_Object_Bit_State state;
   
   state = _data;
   if (state->state == 3) return;
   if (state->mouse_in) state->state = 1;
   else state->state = 0;
}

static void
_ebits_handle_mouse_move (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   Ebits_Object_Bit_State state;
   
   if (state->state == 3) return;
   state = _data;
}

static void
_ebits_handle_mouse_in (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   Ebits_Object_Bit_State state;
   
   state = _data;
   if (state->state == 3) return;
   state->mouse_in = 1;
   if (state->state != 2) state->state = 1;
}

static void
_ebits_handle_mouse_out (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   Ebits_Object_Bit_State state;
   
   state = _data;
   if (state->state == 3) return;
   state->mouse_in = 0;
   if (state->state != 2) state->state = 0;
}

static char *
_ebits_get_file(Ebits_Object_Bit_Description d, int state)
{
   if (state == 0)
     {
	if (d->normal.image) return d->normal.image;
	if (d->hilited.image) return d->hilited.image;
	if (d->clicked.image) return d->clicked.image;
	if (d->disabled.image) return d->disabled.image;
     }
   if (state == 1)
     {
	if (d->hilited.image) return d->hilited.image;
	if (d->clicked.image) return d->clicked.image;
	if (d->normal.image) return d->normal.image;
	if (d->disabled.image) return d->disabled.image;
     }
   if (state == 2)
     {
	if (d->clicked.image) return d->clicked.image;
	if (d->hilited.image) return d->hilited.image;
	if (d->normal.image) return d->normal.image;
	if (d->disabled.image) return d->disabled.image;
     }
   if (state == 3)
     {
	if (d->disabled.image) return d->disabled.image;
	if (d->normal.image) return d->normal.image;
	if (d->hilited.image) return d->hilited.image;
	if (d->clicked.image) return d->clicked.image;
     }
   return "";
}

static Ebits_Object_Description
_ebits_find_description(char *file)
{
   Ebits_Object_Description d = NULL;
   Evas_List l;
   E_DB_File *db;
   int version;
   
   /* find bit description in cache */
   for (l = __bit_descriptions; l; l = l->next)
     {
	d = l->data;
	if (!strcmp(d->file, file)) 
	  {
	     if (l != __bit_descriptions)
	       {
		  __bit_descriptions = evas_list_remove(__bit_descriptions, d);
		  __bit_descriptions = evas_list_prepend(__bit_descriptions, d);
	       }
	      return d;
	   }
     }
   /* open db */
   db = e_db_open_read(file);
   /* no db - return NULL */
   if (!db) return NULL;
   
   /* check version of file (and magic key/number) */
   version = 0;
   e_db_int_get(db, "/type/bits", &(version));
   /* got either no key or version != 1 */
   if (version != 1)
     {
	e_db_close(db);
	return NULL;
     }
   /* new description */
   d = malloc(sizeof(struct _Ebits_Object_Description));
   memset(d, 0, sizeof(struct _Ebits_Object_Description));
   d->file = strdup(file);
   d->references = 1;
   
   /* basic bit info */
   e_db_int_get(db, "/base/min/w", &(d->min.w));
   e_db_int_get(db, "/base/min/h", &(d->min.h));
   
   e_db_int_get(db, "/base/max/w", &(d->max.w));
   e_db_int_get(db, "/base/max/h", &(d->max.h));

   e_db_int_get(db, "/base/padding/l", &(d->padding.l));
   e_db_int_get(db, "/base/padding/r", &(d->padding.r));
   e_db_int_get(db, "/base/padding/t", &(d->padding.t));
   e_db_int_get(db, "/base/padding/b", &(d->padding.b));

   e_db_int_get(db, "/base/inset/l", &(d->inset.l));
   e_db_int_get(db, "/base/inset/r", &(d->inset.r));
   e_db_int_get(db, "/base/inset/t", &(d->inset.t));
   e_db_int_get(db, "/base/inset/b", &(d->inset.b));

   /* all he bits */
     {
	int num_bits = 0, i;
	
	e_db_int_get(db, "/bits/count", &(num_bits));
	for (i = 0; i < num_bits; i++)
	  {
	     Ebits_Object_Bit_Description bit;
	     char key[4096];
	     float f;
	     int num_sync, j;
	     
	     bit = malloc(sizeof(struct _Ebits_Object_Bit_Description));
	     memset(bit, 0, sizeof(struct _Ebits_Object_Bit_Description));
	     
	     snprintf(key, sizeof(key), "bits/bit/%i/name", i);
	     bit->name = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/class", i);
	     bit->class = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/normal/image", i);
	     bit->normal.image = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/hilited/image", i);
	     bit->hilited.image = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/clicked/image", i);
	     bit->clicked.image = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/disabled/image", i);
	     bit->disabled.image = e_db_str_get(db, key);
	     
	     snprintf(key, sizeof(key), "bits/bit/%i/border/l", i);
	     e_db_int_get(db, key, &(bit->border.l));
	     snprintf(key, sizeof(key), "bits/bit/%i/border/r", i);
	     e_db_int_get(db, key, &(bit->border.r));
	     snprintf(key, sizeof(key), "bits/bit/%i/border/t", i);
	     e_db_int_get(db, key, &(bit->border.t));
	     snprintf(key, sizeof(key), "bits/bit/%i/border/b", i);
	     e_db_int_get(db, key, &(bit->border.b));

	     snprintf(key, sizeof(key), "bits/bit/%i/tile/w", i);
	     e_db_int_get(db, key, &(bit->tile.w));
	     snprintf(key, sizeof(key), "bits/bit/%i/tile/h", i);
	     e_db_int_get(db, key, &(bit->tile.h));
	     
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/name", i);
	     bit->rel1.name = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/x", i);
	     e_db_int_get(db, key, &(bit->rel1.x));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/y", i);
	     e_db_int_get(db, key, &(bit->rel1.y));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/rx", i);
	     e_db_float_get(db, key, &(f)); bit->rel1.rx = f;
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/ry", i);
	     e_db_float_get(db, key, &(f)); bit->rel1.ry = f;
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/ax", i);
	     e_db_int_get(db, key, &(bit->rel1.ax));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel1/ay", i);
	     e_db_int_get(db, key, &(bit->rel1.ay));
     
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/name", i);
	     bit->rel2.name = e_db_str_get(db, key);
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/x", i);
	     e_db_int_get(db, key, &(bit->rel2.x));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/y", i);
	     e_db_int_get(db, key, &(bit->rel2.y));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/rx", i);
	     e_db_float_get(db, key, &(f)); bit->rel2.rx = f;
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/ry", i);
	     e_db_float_get(db, key, &(f)); bit->rel2.ry = f;
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/ax", i);
	     e_db_int_get(db, key, &(bit->rel2.ax));
	     snprintf(key, sizeof(key), "bits/bit/%i/rel2/ay", i);
	     e_db_int_get(db, key, &(bit->rel2.ay));

	     snprintf(key, sizeof(key), "bits/bit/%i/align/w", i);
	     e_db_float_get(db, key, &(f)); bit->align.w = f;
	     snprintf(key, sizeof(key), "bits/bit/%i/align/h", i);
	     e_db_float_get(db, key, &(f)); bit->align.h = f;

	     snprintf(key, sizeof(key), "bits/bit/%i/aspect/x", i);
	     e_db_int_get(db, key, &(bit->aspect.x));
	     snprintf(key, sizeof(key), "bits/bit/%i/aspect/y", i);
	     e_db_int_get(db, key, &(bit->aspect.y));
	     snprintf(key, sizeof(key), "bits/bit/%i/step/x", i);
	     e_db_int_get(db, key, &(bit->step.x));
	     snprintf(key, sizeof(key), "bits/bit/%i/step/y", i);
	     e_db_int_get(db, key, &(bit->step.y));

	     num_sync = 0;
	     snprintf(key, sizeof(key), "bits/bit/%i/sync/count", i);
	     e_db_int_get(db, key, &(num_sync));
	     
	     for (j = 0; j < num_sync; j++)
	       {
		  char *s;
		  
		  snprintf(key, sizeof(key), "bits/bit/%i/sync/%i", i, j);
		  s = e_db_str_get(db, key);
		  if (s) bit->sync = evas_list_append(bit->sync, s);
	       }
	     
	     d->bits = evas_list_append(d->bits, bit);
	  }
     }

   e_db_close(db);
   __bit_descriptions = evas_list_prepend(__bit_descriptions, d);
   return d;
}

Ebits_Object ebits_load(char *file)
{
   Ebits_Object o;
   Ebits_Object_Description d;   
   Evas_List l;
   
   d = _ebits_find_description(file);
   if (!d) return NULL;
   
   o = ebits_new();
   o->description = d;
   
   for (l = d->bits; l; l = l->next)
     {
	Ebits_Object_Bit_Description bit;
	Ebits_Object_Bit_State state;
#ifdef EDITOR
	char image[4096];
#endif
	
	state = malloc(sizeof(Ebits_Object_Bit_State));
	memset(state, 0, sizeof(Ebits_Object_Bit_State));	
	bit = l->data;
	state->o = o;
	state->description = bit;
	
#ifdef EDITOR
	if (bit->normal.image)
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->normal.image);
	     state->normal.image = imlib_load_image(image);
	  }
	if (bit->hilited.image)
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->hilited.image);
	     state->hilited.image = imlib_load_image(image);
	  }
	if (bit->clicked.image)
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->clicked.image);
	     state->clicked.image = imlib_load_image(image);
	  }
	if (bit->disabled.image)
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->disabled.image);
	     state->disabled.image = imlib_load_image(image);
	  }
#endif	
     }
   
   return o;
}

void ebits_free(Ebits_Object o)
{
   Evas_List l;
   
   o->description->references--;
   if (o->description->references <= 0)
     {
	if (o->description->file) free(o->description->file);
	__bit_descriptions = evas_list_remove(__bit_descriptions, o->description);	
	if (o->description->bits)
	  {
	     for (l = o->description->bits; l; l = l->next)
	       {
		  Ebits_Object_Bit_Description bit;
		  
		  bit = l->data;
		  if (bit->name) free(bit->name);
		  if (bit->class) free(bit->class);
		  if (bit->normal.image) free(bit->normal.image);
		  if (bit->hilited.image) free(bit->hilited.image);
		  if (bit->clicked.image) free(bit->clicked.image);
		  if (bit->disabled.image) free(bit->disabled.image);
		  if (bit->rel1.name) free(bit->rel1.name);
		  if (bit->rel2.name) free(bit->rel2.name);
	       }
	     evas_list_free(o->description->bits);
	  }
	free(o->description);
     }
   if (o->bits)
     {
	for (l = o->bits; l; l = l->next)
	  {
	     Ebits_Object_Bit_State state;
	     
	     state = l->data;
	     if ((state->object) && (o->state.evas))
		evas_del_object(o->state.evas, state->object);
#ifdef EDITOR
	     if (state->normal.image)
	       {
		  imlib_context_set_image(state->normal.image);
		  imlib_free_image_and_decache();
	       }
	     if (state->hilited.image)
	       {
		  imlib_context_set_image(state->hilited.image);
		  imlib_free_image_and_decache();
	       }
	     if (state->clicked.image)
	       {
		  imlib_context_set_image(state->clicked.image);
		  imlib_free_image_and_decache();
	       }
	     if (state->disabled.image)
	       {
		  imlib_context_set_image(state->disabled.image);
		  imlib_free_image_and_decache();
	       }
#endif
	     free(state);
	  }
	evas_list_free(o->bits);
     }
   free(o);
}

void ebits_add_to_evas(Ebits_Object o, Evas e)
{
   Evas_List l;
   
   o->state.evas = e;
   for (l = o->bits; l; l = l->next)
     {
	Ebits_Object_Bit_State state;
	
	state = l->data;
	state->object = evas_add_image_from_file(o->state.evas, 
						 _ebits_get_file(state->description, state->state));
	evas_callback_add(o->state.evas, state->object, CALLBACK_MOUSE_DOWN, _ebits_handle_mouse_down, state);
	evas_callback_add(o->state.evas, state->object, CALLBACK_MOUSE_UP, _ebits_handle_mouse_up, state);
	evas_callback_add(o->state.evas, state->object, CALLBACK_MOUSE_MOVE, _ebits_handle_mouse_move, state);
	evas_callback_add(o->state.evas, state->object, CALLBACK_MOUSE_IN, _ebits_handle_mouse_in, state);
	evas_callback_add(o->state.evas, state->object, CALLBACK_MOUSE_OUT, _ebits_handle_mouse_out, state);
     }
}

void ebits_show(Ebits_Object o){}
void ebits_hide(Ebits_Object o){}
void ebits_set_layer(Ebits_Object o, int l){}
void ebits_raise(Ebits_Object o){}
void ebits_lower(Ebits_Object o){}
void ebits_move(Ebits_Object o, double x, double y){}
void ebits_resize(Ebits_Object o, double w, double h){}
void ebits_get_padding(Ebits_Object o, int *l, int *r, int *t, int *b){}
void ebits_get_insets(Ebits_Object o, int *l, int *r, int *t, int *b){}
void ebits_get_min_size(Ebits_Object o, int *w, int *h){}
void ebits_get_max_size(Ebits_Object o, int *w, int *h){}
void ebits_get_size_step(Ebits_Object o, int *x, int *y){}
void ebits_get_bit_geometry(Ebits_Object o, char *c, double *x, double *y, double *w, double *h){}

Ebits_Object ebits_new(void)
{
   Ebits_Object o;
   
   o = malloc(sizeof(struct _Ebits_Object));
   memset(o, 0, sizeof(struct _Ebits_Object));
   o->description = NULL;
   o->state.x = 0;
   o->state.y = 0;
   o->state.w = 1;
   o->state.h = 1;
   return o;
}

#ifdef EDITOR
void ebits_save(Ebits_Object o, char *file)
{
   Ebits_Object_Description d;
   Evas_List l;
   E_DB_File *db;
   int i, count;
   
   d = o->description;
   /* delete the original */
   unlink(file);
   /* open it now */
   db = e_db_open(file);
   if (!db) return;
   
   e_db_int_set(db, "/type/bits", 1);
   
   e_db_int_set(db, "/base/min/w", d->min.w);
   e_db_int_set(db, "/base/min/h", d->min.h);
   
   e_db_int_set(db, "/base/max/w", d->max.w);
   e_db_int_set(db, "/base/max/h", d->max.h);

   e_db_int_set(db, "/base/padding/l", d->padding.l);
   e_db_int_set(db, "/base/padding/r", d->padding.r);
   e_db_int_set(db, "/base/padding/t", d->padding.t);
   e_db_int_set(db, "/base/padding/b", d->padding.b);

   e_db_int_set(db, "/base/inset/l", d->inset.l);
   e_db_int_set(db, "/base/inset/r", d->inset.r);
   e_db_int_set(db, "/base/inset/t", d->inset.t);
   e_db_int_set(db, "/base/inset/b", d->inset.b);
   
   for (count = 0, l = d->bits; l; l = l->next, count++);
   
   e_db_int_set(db, "/bits/count", count);

   /* save the images */
   for (l = o->bits; l; l = l->next)
     {
	Ebits_Object_Bit_Description bit;
	Ebits_Object_Bit_State state;
	char image[4096];
	
	state = l->data;
	bit = state->description;
	
	if ((!state->normal.image) && (bit->normal.image))
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->normal.image);
	     imlib_context_set_image(state->normal.image);
	     imlib_image_set_format("db");
	     imlib_save_image(image);
	  }
	if ((!state->hilited.image) && (bit->hilited.image))
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->hilited.image);
	     imlib_context_set_image(state->hilited.image);
	     imlib_image_set_format("db");
	     imlib_save_image(image);
	  }
	if ((!state->clicked.image) && (bit->clicked.image))
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->clicked.image);
	     imlib_context_set_image(state->clicked.image);
	     imlib_image_set_format("db");
	     imlib_save_image(image);
	  }
	if ((!state->disabled.image) && (bit->disabled.image))
	  {
	     snprintf(image, sizeof(image), "%s:%s", file, bit->disabled.image);
	     imlib_context_set_image(state->disabled.image);
	     imlib_image_set_format("db");
	     imlib_save_image(image);
	  }
     }
   /* save bit info */
   for (i = 0, l = d->bits; l; l = l->next, i++)
     {
	Ebits_Object_Bit_Description bit;
	Evas_List ll;
	char key[4096];
	int j, sync_count;
	
	bit = l->data;
	
	snprintf(key, sizeof(key), "bits/bit/%i/name", i);
	if (bit->name) e_db_str_set(db, key, bit->name);
	snprintf(key, sizeof(key), "bits/bit/%i/class", i);
	if (bit->class) e_db_str_set(db, key, bit->class);
	snprintf(key, sizeof(key), "bits/bit/%i/normal/image", i);
	if (bit->normal.image) e_db_str_set(db, key, bit->normal.image);
	snprintf(key, sizeof(key), "bits/bit/%i/hilited/image", i);
	if (bit->hilited.image) e_db_str_set(db, key, bit->hilited.image);
	snprintf(key, sizeof(key), "bits/bit/%i/clicked/image", i);
	if (bit->clicked.image) e_db_str_set(db, key, bit->clicked.image);
	snprintf(key, sizeof(key), "bits/bit/%i/disabled/image", i);
	if (bit->disabled.image) e_db_str_set(db, key, bit->disabled.image);
	
	snprintf(key, sizeof(key), "bits/bit/%i/border/l", i);
	e_db_int_set(db, key, bit->border.l);
	snprintf(key, sizeof(key), "bits/bit/%i/border/r", i);
	e_db_int_set(db, key, bit->border.r);
	snprintf(key, sizeof(key), "bits/bit/%i/border/t", i);
	e_db_int_set(db, key, bit->border.t);
	snprintf(key, sizeof(key), "bits/bit/%i/border/b", i);
	e_db_int_set(db, key, bit->border.b);

	snprintf(key, sizeof(key), "bits/bit/%i/tile/w", i);
	e_db_int_set(db, key, bit->tile.w);
	snprintf(key, sizeof(key), "bits/bit/%i/tile/h", i);
	e_db_int_set(db, key, bit->tile.h);
	
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/name", i);
	if (bit->rel1.name) e_db_str_set(db, key, bit->rel1.name);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/x", i);
	e_db_int_set(db, key, bit->rel1.x);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/y", i);
	e_db_int_set(db, key, bit->rel1.y);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/rx", i);
	e_db_float_set(db, key, bit->rel1.rx);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/ry", i);
	e_db_float_set(db, key, bit->rel1.ry);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/ax", i);
	e_db_int_set(db, key, bit->rel1.ax);
	snprintf(key, sizeof(key), "bits/bit/%i/rel1/ay", i);
	e_db_int_set(db, key, bit->rel1.ay);

	snprintf(key, sizeof(key), "bits/bit/%i/rel2/name", i);
	if (bit->rel2.name) e_db_str_set(db, key, bit->rel2.name);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/x", i);
	e_db_int_set(db, key, bit->rel2.x);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/y", i);
	e_db_int_set(db, key, bit->rel2.y);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/rx", i);
	e_db_float_set(db, key, bit->rel2.rx);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/ry", i);
	e_db_float_set(db, key, bit->rel2.ry);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/ax", i);
	e_db_int_set(db, key, bit->rel2.ax);
	snprintf(key, sizeof(key), "bits/bit/%i/rel2/ay", i);
	e_db_int_set(db, key, bit->rel2.ay);
	
	snprintf(key, sizeof(key), "bits/bit/%i/align/w", i);
	e_db_float_set(db, key, bit->align.w);
	snprintf(key, sizeof(key), "bits/bit/%i/align/h", i);
	e_db_float_set(db, key, bit->align.h);
	
	snprintf(key, sizeof(key), "bits/bit/%i/aspect/x", i);
	e_db_int_set(db, key, bit->aspect.x);
	snprintf(key, sizeof(key), "bits/bit/%i/aspect/y", i);
	e_db_int_set(db, key, bit->aspect.y);
	snprintf(key, sizeof(key), "bits/bit/%i/step/x", i);
	e_db_int_set(db, key, bit->step.x);
	snprintf(key, sizeof(key), "bits/bit/%i/step/y", i);
	e_db_int_set(db, key, bit->step.y);
	
	for (sync_count = 0, l = bit->sync; l; l = l->next, sync_count++);
	snprintf(key, sizeof(key), "bits/bit/%i/sync/count", i);
	e_db_int_set(db, key, sync_count);
	for (j = 0, ll = bit->sync; ll; ll = ll->next, j++)
	  {
	     snprintf(key, sizeof(key), "bits/bit/%i/sync/%i", i, j);
	     e_db_str_set(db, key, ll->data);
	  }
     }
   
   e_db_close(db);
   e_db_flush();
}
#endif
