#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"
#ifdef BUILD_FONT_LOADER_EET
#include <Eet.h>
#endif

/* private magic number for text objects */
static const char o_type[] = "text";

/* private struct for text object internal data */
typedef struct _Evas_Object_Text      Evas_Object_Text;

struct _Evas_Object_Text
{
   DATA32            magic;
   
   struct {
      char          *text;
      char          *font;
      char          *source;
      Evas_Font_Size size;
   } cur, prev;
   char              changed : 1;
   
   float             ascent, descent;
   float             max_ascent, max_descent;
   
   void             *engine_data;
};




/* font dir cache */
typedef struct _Evas_Font_Dir   Evas_Font_Dir;
typedef struct _Evas_Font       Evas_Font;
typedef struct _Evas_Font_Alias Evas_Font_Alias;

struct _Evas_Font_Dir
{
   Evas_Hash *lookup;
   Evas_List *fonts;
   Evas_List *aliases;
   DATA64     dir_mod_time;
   DATA64     fonts_dir_mod_time;
   DATA64     fonts_alias_mod_time;
};

struct _Evas_Font
{
   char     type;
   struct {
      char *prop[14];
   } x;
   struct {
      char *name;
   } simple;
   char    *path;
};

struct _Evas_Font_Alias
{
   char      *alias;
   Evas_Font *fn;
};

/* font dir cache */
static Evas_Hash *font_dirs = NULL;

/* private methods for font dir cache */
static char *object_text_font_cache_find(char *dir, char *font);
static Evas_Font_Dir *object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd);
static Evas_Font *object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font);
static Evas_Font_Dir *object_text_font_cache_dir_add(char *dir);
static void object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd);
static int evas_object_text_font_string_parse(char *buffer, char dest[14][256]);

static Evas_Bool font_cache_dir_free(Evas_Hash *hash, const char *key,
                                     void *data, void *fdata)
{
   object_text_font_cache_dir_del((char *) key, data);

   return 1;
}

void
evas_font_dir_cache_free(void)
{
   if (!font_dirs) return;

   evas_hash_foreach (font_dirs, font_cache_dir_free, NULL);
   evas_hash_free (font_dirs);
   font_dirs = NULL;
}

static char *
object_text_font_cache_find(char *dir, char *font)
{
   Evas_Font_Dir *fd;
   
   fd = evas_hash_find(font_dirs, dir);
   fd = object_text_font_cache_dir_update(dir, fd);
   if (fd)
     {
	Evas_Font *fn;
	
	fn = object_text_font_cache_font_find(fd, font);
	if (fn) return fn->path;
     }
   return NULL;
}

static Evas_Font_Dir *
object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd)
{
   DATA64 mt;
   char *tmp;

   if (fd)
     {
	mt = evas_file_modified_time(dir);
	if (mt != fd->dir_mod_time)
    {
	  object_text_font_cache_dir_del(dir, fd);
      font_dirs = evas_hash_del(font_dirs, dir, fd);
	}
    else
	  {
	     tmp = evas_file_path_join(dir, "fonts.dir");
	     if (tmp)
	       {
		  mt = evas_file_modified_time(tmp);
		  free(tmp);
		  if (mt != fd->fonts_dir_mod_time)
          {
		    object_text_font_cache_dir_del(dir, fd);
            font_dirs = evas_hash_del(font_dirs, dir, fd);
		  }
          else
		    {
		       tmp = evas_file_path_join(dir, "fonts.alias");
		       if (tmp)
			 {
			    mt = evas_file_modified_time(tmp);
			    free(tmp);
			 }
		       if (mt != fd->fonts_alias_mod_time)
               {
			     object_text_font_cache_dir_del(dir, fd);
                 font_dirs = evas_hash_del(font_dirs, dir, fd);
			   }
               else
			 return fd;
		    }
	       }
	  }
     }
   return object_text_font_cache_dir_add(dir);
}

static Evas_Font *
object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   char font_prop[14][256];
   int num;
   
   num = evas_object_text_font_string_parse(font, font_prop);
   if (num != 14) return NULL;
   for (l = fd->fonts; l; l = l->next)
     {
	Evas_Font *fn;
	
	fn = l->data;
	if (fn->type == 1)
	  {
	     int i;
	     int match = 0;
	     
	     for (i = 0; i < 14; i++)
	       {
		  if ((font_prop[i][0] == '*') && (font_prop[i][1] == 0))
		    match++;
		  else
		    {
		       if (!strcasecmp(font_prop[i], fn->x.prop[i])) match++;
		       else break;
		    }
	       }
	     if (match == 14) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   
   for (l = fd->fonts; l; l = l->next)
     {
	Evas_Font *fn;
	
	fn = l->data;
	if (fn->type == 0)
	  {
	     if (!strcasecmp(font, fn->simple.name)) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font)
{
   Evas_List *l;
   
   for (l = fd->aliases; l; l = l->next)
     {
	Evas_Font_Alias *fa;
	
	fa = l->data;
	if (!strcasecmp(fa->alias, font)) return fa->fn;
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font)
{
   Evas_Font *fn;
   
   fn = evas_hash_find(fd->lookup, font);
   if (fn) return fn;
   fn = object_text_font_cache_font_find_alias(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_x(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_file(fd, font);
   if (!fn) return NULL;
   fd->lookup = evas_hash_add(fd->lookup, font, fn);
   return fn;
}

static Evas_Font_Dir *
object_text_font_cache_dir_add(char *dir)
{
   Evas_Font_Dir *fd;
   char *tmp;
   Evas_List *fdir;
   
   fd = calloc(1, sizeof(Evas_Font_Dir));
   if (!fd) return NULL;
   font_dirs = evas_hash_add(font_dirs, dir, fd);
   
   /* READ fonts.alias, fonts.dir and directory listing */

   /* fonts.dir */
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	FILE *f;
	
	f = fopen(tmp, "r");
	if (f)
	  {
	     int num;
	     char fname[4096], fdef[4096];
	     
	     if (fscanf(f, "%i\n", &num) != 1) goto cant_read;
	     /* read font lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  char font_prop[14][256];
		  int i;
		  
		  /* skip comments */
		  if ((fdef[0] == '!') || (fdef[0] == '#')) continue;
		  /* parse font def */
		  num = evas_object_text_font_string_parse((char *)fdef, font_prop);
		  if (num == 14)
		    {
		       Evas_Font *fn;
		       
		       fn = calloc(1, sizeof(Evas_Font));
		       if (fn)
			 {
			    fn->type = 1;
			    for (i = 0; i < 14; i++)
			      {
				 fn->x.prop[i] = strdup(font_prop[i]);
				 /* FIXME: what if strdup fails! */
			      }
			    fn->path = evas_file_path_join(dir, fname);
			    /* FIXME; what is evas_file_path_join fails! */
			    fd->fonts = evas_list_append(fd->fonts, fn);
			 }
		    }
	       }
	     cant_read: ;
	     fclose(f);
	  }
	free(tmp);
     }
   
   /* directoy listing */
   fdir = evas_file_path_list(dir, "*.ttf", 0);
   while (fdir)
     {
	tmp = evas_file_path_join(dir, fdir->data);
	if (tmp)
	  {
	     Evas_Font *fn;
	     
	     fn = calloc(1, sizeof(Evas_Font));
	     if (fn)
	       {
		  fn->type = 0;
		  fn->simple.name = strdup(fdir->data);
		  if (fn->simple.name)
		    {
		       char *p;
		       
		       p = strrchr(fn->simple.name, '.');
		       if (p) *p = 0;
		    }
		  fn->path = evas_file_path_join(dir, fdir->data);
		  fd->fonts = evas_list_append(fd->fonts, fn);
	       }
	     free(tmp);
	  }
	free(fdir->data);
	fdir = evas_list_remove(fdir, fdir->data);
     }
   
   /* fonts.alias */
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	FILE *f;
	
	f = fopen(tmp, "r");
	if (f)
	  {
	     char fname[4096], fdef[4096];
	     
	     /* read font alias lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  Evas_Font_Alias *fa;
		  
		  /* skip comments */
		  if ((fdef[0] == '!') || (fdef[0] == '#')) continue;
		  fa = calloc(1, sizeof(Evas_Font_Alias));
		  if (fa)
		    {
		       fa->alias = strdup(fname);
		       fa->fn = object_text_font_cache_font_find_x(fd, fdef);
		       if ((!fa->alias) || (!fa->fn))
			 {
			    if (fa->alias) free(fa->alias);
			    free(fa);
			 }
		       else
			 fd->aliases = evas_list_append(fd->aliases, fa);
		    }
	       }
	     fclose(f);
	  }
	free(tmp);
     }
   
   fd->dir_mod_time = evas_file_modified_time(dir);
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	fd->fonts_dir_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	fd->fonts_alias_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }
   
   return fd;
}

static void
object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd)
{
   if (fd->lookup) evas_hash_free(fd->lookup);
   while (fd->fonts)
     {
	Evas_Font *fn;
	int i;
	
	fn = fd->fonts->data;
	fd->fonts = evas_list_remove(fd->fonts, fn);
	for (i = 0; i < 14; i++)
	  {
	     if (fn->x.prop[i]) free(fn->x.prop[i]);
	  }
	if (fn->simple.name) free(fn->simple.name);
	if (fn->path) free(fn->path);
	free(fn);
     }
   while (fd->aliases)
     {
	Evas_Font_Alias *fa;
	
	fa = fd->aliases->data;
	fd->aliases = evas_list_remove(fd->aliases, fa);
	if (fa->alias) free(fa->alias);
	free(fa);
     }
   free(fd);
}

static int
evas_object_text_font_string_parse(char *buffer, char dest[14][256])
{
   char *p;
   int n, m, i;

   n = 0;
   m = 0;
   p = buffer;
   if (p[0] != '-') return 0;
   i = 1;
   while (p[i])
     {
	dest[n][m] = p[i];
	if ((p[i] == '-') || (m == 256))
	  {
	     dest[n][m] = 0;
	     n++;
	     m = -1;
	  }
	i++;
	m++;
	if (n == 14) return n;
     }
   dest[n][m] = 0;
   n++;
   return n;
}


/* private methods for text objects */
static void evas_object_text_init(Evas_Object *obj);
static void *evas_object_text_new(void);
static void evas_object_text_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_text_free(Evas_Object *obj);
static void evas_object_text_render_pre(Evas_Object *obj);
static void evas_object_text_render_post(Evas_Object *obj);

static int evas_object_text_is_opaque(Evas_Object *obj);
static int evas_object_text_was_opaque(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_text_free,
     evas_object_text_render,
     evas_object_text_render_pre,
     evas_object_text_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_text_is_opaque,
     evas_object_text_was_opaque,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
evas_font_load.c */
Evas_Object *
evas_object_text_add(Evas *e)
{
   Evas_Object *obj;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_text_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_text_font_source_set(Evas_Object *obj, const char *font_source)
{
   Evas_Object_Text *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.source) && (font_source) && 
       (!strcmp(o->cur.source, font_source))) 
     return;
   if (o->cur.source) free(o->cur.source);
   if (font_source) o->cur.source = strdup(font_source);
   else o->cur.source = NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
const char *
evas_object_text_font_source_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return NULL;
   MAGIC_CHECK_END();
   return o->cur.source;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_text_font_set(Evas_Object *obj, const char *font, Evas_Font_Size size)
{
   Evas_Object_Text *o;
   int is, was = 0;
   int same_font = 0;
      
   if (!font) return;
   if (size <= 0) return;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.font) && (font) && (!strcmp(o->cur.font, font))) 
     {
	same_font = 1;
	if (size == o->cur.size) return;
     }
   if (!evas_event_passes_through(obj))     
     was = evas_object_is_in_output_rect(obj,
					 obj->layer->evas->pointer.x,
					 obj->layer->evas->pointer.y, 1, 1);
   /* DO IT */
   if (o->engine_data)
     {
	obj->layer->evas->engine.func->font_free(obj->layer->evas->engine.data.output,
						 o->engine_data);
	o->engine_data = NULL;
     }
#ifdef BUILD_FONT_LOADER_EET
   if (o->cur.source)
     {
	Eet_File *ef;
	char *fake_name;
	
	fake_name = evas_file_path_join(o->cur.source, font);
	if (fake_name)
	  {
	     o->engine_data = 
	       obj->layer->evas->engine.func->font_load
	       (obj->layer->evas->engine.data.output, fake_name, 
		size);
	     if (!o->engine_data)
	       {
		  /* read original!!! */
		  ef = eet_open(o->cur.source, EET_FILE_MODE_READ);
		  if (ef)
		    {
		       void *fdata;
		       int fsize = 0;
		       
		       fdata = eet_read(ef, font, &fsize);
		       if ((fdata) && (fsize > 0))
			 {
			    o->engine_data = 
			      obj->layer->evas->engine.func->font_memory_load
			      (obj->layer->evas->engine.data.output,
			       fake_name, size, fdata, fsize);
			    free(fdata);
			 }
		       eet_close(ef);
		    }
	       }
	     free(fake_name);
	  }
     }
   if (!o->engine_data)
     {
#endif
	if (evas_file_path_is_full_path((char *)font))
	  o->engine_data = obj->layer->evas->engine.func->font_load(obj->layer->evas->engine.data.output,
								    (char *)font, size);
	else
	  {
	     Evas_List *l;
	     
	     for (l = obj->layer->evas->font_path; l; l = l->next)
	       {
		  char *f_file;
		  
		  f_file = object_text_font_cache_find(l->data, (char *)font);
		  if (f_file)
		    {
		       o->engine_data = obj->layer->evas->engine.func->font_load(obj->layer->evas->engine.data.output,
										 f_file, size);
		       if (o->engine_data) break;
		    }
	       }
#ifdef BUILD_FONT_LOADER_EET
	  }
#endif
     } 
   if (o->cur.font) free(o->cur.font);
   if (font) o->cur.font = strdup(font);
   else o->cur.font = NULL;
   if (!same_font) o->prev.font = NULL;

   o->cur.size = size;
   if ((o->engine_data) && (o->cur.text))
     {
	int w, h;
	
	obj->layer->evas->engine.func->font_string_size_get(obj->layer->evas->engine.data.output,
							    o->engine_data,
							    o->cur.text,
							    &w, &h);
	obj->cur.geometry.w = w;
        obj->cur.geometry.h = h;
        obj->cur.cache.geometry.validity = 0;
	o->ascent = obj->layer->evas->engine.func->font_ascent_get(obj->layer->evas->engine.data.output,
								   o->engine_data);
	o->descent = obj->layer->evas->engine.func->font_descent_get(obj->layer->evas->engine.data.output,
								     o->engine_data);
	o->max_ascent = obj->layer->evas->engine.func->font_max_ascent_get(obj->layer->evas->engine.data.output,
									   o->engine_data);
	o->max_descent = obj->layer->evas->engine.func->font_max_descent_get(obj->layer->evas->engine.data.output,
									     o->engine_data);
     }
   else
     {
	obj->cur.geometry.w = 0;
        obj->cur.geometry.h = o->max_ascent + o->max_descent;
        obj->cur.cache.geometry.validity = 0;
	if (o->engine_data)
	  {
	     o->ascent = obj->layer->evas->engine.func->font_ascent_get(obj->layer->evas->engine.data.output,
									o->engine_data);
	     o->descent = obj->layer->evas->engine.func->font_descent_get(obj->layer->evas->engine.data.output,
									  o->engine_data);
	     o->max_ascent = obj->layer->evas->engine.func->font_max_ascent_get(obj->layer->evas->engine.data.output,
										o->engine_data);
	     o->max_descent = obj->layer->evas->engine.func->font_max_descent_get(obj->layer->evas->engine.data.output,
										  o->engine_data);
	  }
	else
	  {
	     o->ascent = 0;
	     o->descent = 0;
	     o->max_ascent = 0;
	     o->max_descent = 0;
	  }
     }
   o->changed = 1;
   evas_object_change(obj);
   evas_object_coords_recalc(obj);
   if (!evas_event_passes_through(obj))
     {
	is = evas_object_is_in_output_rect(obj,
					   obj->layer->evas->pointer.x,
					   obj->layer->evas->pointer.y, 1, 1);
	if ((is ^ was) && obj->cur.visible)
	  evas_event_feed_mouse_move(obj->layer->evas,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y);
     }
   evas_object_inform_call_resize(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_text_font_get(Evas_Object *obj, char **font, Evas_Font_Size *size)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (font) *font = "";
   if (size) *size = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   if (font) *font = "";
   if (size) *size = 0;
   return;
   MAGIC_CHECK_END();
   if (font) *font = o->cur.font;
   if (size) *size = o->cur.size;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_text_text_set(Evas_Object *obj, const char *text)
{
   Evas_Object_Text *o;
   int is, was;

   if (!text) text = "";
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.text) && (text) && (!strcmp(o->cur.text, text))) return;
   was = evas_object_is_in_output_rect(obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   /* DO II */
   if (o->cur.text) free(o->cur.text);
   if (text) o->cur.text = strdup(text);
   else o->cur.text = NULL;
   o->prev.text = NULL;
   if ((o->engine_data) && (o->cur.text))
     {
	int w, h;
	
	obj->layer->evas->engine.func->font_string_size_get(obj->layer->evas->engine.data.output,
							    o->engine_data,
							    o->cur.text,
							    &w, &h);
	obj->cur.geometry.w = w;
        obj->cur.geometry.h = h;
        obj->cur.cache.geometry.validity = 0;
     }
   else
     {
	obj->cur.geometry.w = 0;
        obj->cur.geometry.h = o->max_ascent + o->max_descent;
        obj->cur.cache.geometry.validity = 0;	
     }
   o->changed = 1;
   evas_object_change(obj);
   evas_object_coords_recalc(obj);
   is = evas_object_is_in_output_rect(obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur.visible)
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y);   
   evas_object_inform_call_resize(obj);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
const char *
evas_object_text_text_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return NULL;
   MAGIC_CHECK_END();
   return o->cur.text;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_ascent_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->ascent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_descent_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->descent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_max_ascent_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->max_ascent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_max_descent_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   return o->max_descent;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_inset_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return 0;
   return obj->layer->evas->engine.func->font_inset_get(obj->layer->evas->engine.data.output,
							o->engine_data,
							o->cur.text);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_horiz_advance_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return 0;
   return obj->layer->evas->engine.func->font_h_advance_get(obj->layer->evas->engine.data.output,
							    o->engine_data,
							    o->cur.text);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Coord
evas_object_text_vert_advance_get(Evas_Object *obj)
{
   Evas_Object_Text *o;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return o->ascent + o->descent;
   return obj->layer->evas->engine.func->font_v_advance_get(obj->layer->evas->engine.data.output,
							    o->engine_data,
							    o->cur.text);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
int
evas_object_text_char_pos_get(Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Text *o;
   int ret, x = 0, y = 0, w = 0, h = 0;
   int inset;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return 0;
   MAGIC_CHECK_END();
   if (!o->engine_data) return 0;
   if (!o->cur.text) return 0;
   inset = 
     obj->layer->evas->engine.func->font_inset_get(obj->layer->evas->engine.data.output,
						   o->engine_data,
						   o->cur.text);
   ret = obj->layer->evas->engine.func->font_char_coords_get(obj->layer->evas->engine.data.output,
							     o->engine_data,
							     o->cur.text,
							     pos, 
							     &x, &y, 
							     &w, &h);
   y += o->max_ascent;
   x -= inset;
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if (x + w > obj->cur.geometry.w) w = obj->cur.geometry.w - x;
   if (w < 0) w = 0;
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if (y + h > obj->cur.geometry.h) h = obj->cur.geometry.h - y;
   if (h < 0) h = 0;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
int
evas_object_text_char_coords_get(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Text *o;
   int ret, rx = 0, ry = 0, rw = 0, rh = 0;
   int inset;
      
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return -1;
   MAGIC_CHECK_END();
   if (!o->engine_data) return -1;
   if (!o->cur.text) return -1;
   inset = 
     obj->layer->evas->engine.func->font_inset_get(obj->layer->evas->engine.data.output,
						   o->engine_data,
						   o->cur.text);
   ret = obj->layer->evas->engine.func->font_char_at_coords_get(obj->layer->evas->engine.data.output,
								o->engine_data,
								o->cur.text,
								x + inset,
								y - o->max_ascent, 
								&rx, &ry, 
								&rw, &rh);
   ry += o->max_ascent;
   rx -= inset;
   if (rx < 0)
     {
	rw += rx;
	rx = 0;
     }
   if (rx + rw > obj->cur.geometry.w) rw = obj->cur.geometry.w - rx;
   if (rw < 0) rw = 0;
   if (ry < 0)
     {
	rh += ry;
	ry = 0;
     }
   if (ry + rh > obj->cur.geometry.h) rh = obj->cur.geometry.h - ry;
   if (rh < 0) rh = 0;
   if (cx) *cx = rx;
   if (cy) *cy = ry;
   if (cw) *cw = rw;
   if (ch) *ch = rh;
   return ret;
}




/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_font_path_clear(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   while (e->font_path)
     {
	free(e->font_path->data);
	e->font_path = evas_list_remove(e->font_path, e->font_path->data);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_font_path_append(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!path) return;
   e->font_path = evas_list_append(e->font_path, strdup(path));
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_font_path_prepend(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!path) return;
   e->font_path = evas_list_prepend(e->font_path, strdup(path));
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
const Evas_List *
evas_font_path_list(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->font_path;
}





/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_font_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   e->engine.func->font_cache_flush(e->engine.data.output);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_font_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   if (size < 0) size = 0;
   e->engine.func->font_cache_set(e->engine.data.output, size);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
int
evas_font_cache_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   
   return e->engine.func->font_cache_get(e->engine.data.output);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
int
evas_string_char_next_get(const char *str, int pos, int *decoded)
{
   int p, d;
   
   if (decoded) *decoded = 0;
   if (!str) return 0;
   if (pos < 0) return 0;
   p = pos;
   d = evas_common_font_utf8_get_next((char *)str, &p);
   if (decoded) *decoded = d;
   return p;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
int
evas_string_char_prev_get(const char *str, int pos, int *decoded)
{
   int p, d;
   
   if (decoded) *decoded = 0;
   if (!str) return 0;
   if (pos < 0) return 0;
   p = pos;
   d = evas_common_font_utf8_get_prev((char *)str, &p);
   if (decoded) *decoded = d;
   return p;
}



/* all nice and private */
static void
evas_object_text_init(Evas_Object *obj)
{
   /* alloc text ob, setup methods and default values */
   obj->object_data = evas_object_text_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 0.0;
   obj->cur.geometry.h = 0.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_text_new(void)
{
   Evas_Object_Text *o;
   
   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Text));
   o->magic = MAGIC_OBJ_TEXT;
   o->prev = o->cur;
   return o;
}

static void
evas_object_text_free(Evas_Object *obj)
{
   Evas_Object_Text *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Text *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Text, MAGIC_OBJ_TEXT);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->cur.text) free(o->cur.text);
   if (o->cur.font) free(o->cur.font);
   if (o->cur.source) free(o->cur.source);
   if (o->engine_data)
     obj->layer->evas->engine.func->font_free(obj->layer->evas->engine.data.output,
					      o->engine_data);
   o->magic = 0;
   free(o);
}

static void
evas_object_text_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Text *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Text *)(obj->object_data);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
/*   
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    230, 160, 30, 100);
   obj->layer->evas->engine.func->rectangle_draw(output,
						 context,
						 surface,
						 obj->cur.cache.geometry.x + x,
						 obj->cur.cache.geometry.y + y,
						 obj->cur.cache.geometry.w,
						 obj->cur.cache.geometry.h);
 */
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    obj->cur.cache.clip.r,
						    obj->cur.cache.clip.g,
						    obj->cur.cache.clip.b,
						    obj->cur.cache.clip.a);
   if ((o->engine_data) && (o->cur.text))
     obj->layer->evas->engine.func->font_draw(output,
					      context,
					      surface,
					      o->engine_data,
					      obj->cur.cache.geometry.x + x -
					      obj->layer->evas->engine.func->font_inset_get(obj->layer->evas->engine.data.output,
											    o->engine_data,
											    o->cur.text),
					      obj->cur.cache.geometry.y + y + 
					      (int)
					      (((o->max_ascent * obj->cur.cache.geometry.h) / obj->cur.geometry.h) - 0.5),
					      obj->cur.cache.geometry.w,
					      obj->cur.cache.geometry.h,
					      obj->cur.geometry.w,
					      obj->cur.geometry.h,
					      o->cur.text);
}

static void
evas_object_text_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Text *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */
   o = (Evas_Object_Text *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* caluclate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   if (o->changed)
     {
	if ((o->cur.size != o->prev.size) ||
	    ((o->cur.font) && (o->prev.font) && (strcmp(o->cur.font, o->prev.font))) ||
	    ((o->cur.font) && (!o->prev.font)) ||
	    ((!o->cur.font) && (o->prev.font)) ||
	    ((o->cur.text) && (o->prev.text) && (strcmp(o->cur.text, o->prev.text))) ||
	    ((o->cur.text) && (!o->prev.text)) ||
	    ((!o->cur.text) && (o->prev.text)))
	  {
	     updates = evas_object_render_pre_prev_cur_add(updates, obj);
	     goto done;
	  }
     }
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_text_render_post(Evas_Object *obj)
{
   Evas_Object_Text *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Text *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
}

static int
evas_object_text_is_opaque(Evas_Object *obj)
{
   Evas_Object_Text *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Text *)(obj->object_data);
   return 0;
}

static int
evas_object_text_was_opaque(Evas_Object *obj)
{
   Evas_Object_Text *o;
   
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Text *)(obj->object_data);
   return 0;
}
