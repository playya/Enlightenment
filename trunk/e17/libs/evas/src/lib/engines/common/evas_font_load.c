#include "evas_common.h"

extern FT_Library         evas_ft_lib;

static int                font_cache_usage = 0;
static int                font_cache = 0;
static Evas_Object_List * fonts_src = NULL;
static Evas_Object_List * fonts = NULL;

static Evas_Bool font_modify_cache_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Bool font_flush_free_glyph_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);

RGBA_Font_Source *
evas_common_font_source_memory_load(const char *name, const void *data, int data_size)
{
   int error;
   RGBA_Font_Source *fs;

   fs = calloc(1, sizeof(RGBA_Font_Source) + data_size);
   if (!fs) return NULL;
   fs->data = ((unsigned char *)fs) + sizeof(RGBA_Font_Source);
   fs->data_size = data_size;
   fs->current_size = 0;
   memcpy(fs->data, data, data_size);
   error = FT_New_Memory_Face(evas_ft_lib, fs->data, fs->data_size, 0, &(fs->ft.face));
   if (error)
     {
	free(fs);
	return NULL;
     }
   fs->name = evas_stringshare_add(name);
   fs->file = NULL;
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   fs->ft.orig_upem = fs->ft.face->units_per_EM;
   fs->references = 1;
   fonts_src = evas_object_list_prepend(fonts_src, fs);
   return fs;
}

RGBA_Font_Source *
evas_common_font_source_load(const char *name)
{
   int error;
   RGBA_Font_Source *fs;

   fs = calloc(1, sizeof(RGBA_Font_Source));
   if (!fs) return NULL;
   fs->data = NULL;
   fs->data_size = 0;
   fs->current_size = 0;
   error = FT_New_Face(evas_ft_lib, name, 0, &(fs->ft.face));
   if (error)
     {
	free(fs);
	return NULL;
     }
   fs->name = evas_stringshare_add(name);
   fs->file = fs->name;
   error = FT_Select_Charmap(fs->ft.face, ft_encoding_unicode);
   fs->ft.orig_upem = fs->ft.face->units_per_EM;
   fs->references = 1;
   fonts_src = evas_object_list_prepend(fonts_src, fs);
   return fs;
}

RGBA_Font_Source *
evas_common_font_source_find(const char *name)
{
   Evas_Object_List *l;

   if (!name) return NULL;
   for (l = fonts_src; l; l = l->next)
     {
	RGBA_Font_Source *fs;

	fs = (RGBA_Font_Source *)l;
	if ((fs->name) && (!strcmp(name, fs->name)))
	  {
	     fs->references++;
	     fonts_src = evas_object_list_remove(fonts_src, fs);
	     fonts_src = evas_object_list_prepend(fonts_src, fs);
	     return fs;
	  }
     }
   return NULL;
}

void
evas_common_font_source_free(RGBA_Font_Source *fs)
{
   fs->references--;
   if (fs->references > 0) return;

   fonts_src = evas_object_list_remove(fonts_src, fs);
   FT_Done_Face(fs->ft.face);
   if (fs->name) evas_stringshare_del(fs->name);
   free(fs);
}

void
evas_common_font_size_use(RGBA_Font *fn)
{
   Evas_List *l;

   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = l->data;
	if (fi->src->current_size != fi->size)
	  {
	     FT_Activate_Size(fi->ft.size);
	     fi->src->current_size = fi->size;
	  }
     }
}

RGBA_Font_Int *
evas_common_font_int_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src) fi->src = evas_common_font_source_memory_load(name, data, data_size);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   return evas_common_font_int_load_init(fi);
}

RGBA_Font_Int *
evas_common_font_int_load(const char *name, int size)
{
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_find(name, size);
   if (fi) return fi;

   fi = calloc(1, sizeof(RGBA_Font_Int));
   if (!fi) return NULL;

   fi->src = evas_common_font_source_find(name);
   if (!fi->src)
     fi->src = evas_common_font_source_load(name);

   if (!fi->src)
     {
	free(fi);
	return NULL;
     }

   fi->size = size;

   return evas_common_font_int_load_init(fi);
}

RGBA_Font_Int *
evas_common_font_int_load_init(RGBA_Font_Int *fi)
{
   int error;

   error = FT_New_Size(fi->src->ft.face, &(fi->ft.size));
   if (!error)
     {
	FT_Activate_Size(fi->ft.size);
     }
   fi->real_size = fi->size * 64;
   error = FT_Set_Char_Size(fi->src->ft.face, 0, fi->real_size, 75, 75);
   if (error)
     {
	fi->real_size = fi->size;
	error = FT_Set_Pixel_Sizes(fi->src->ft.face, 0, fi->real_size);
     }
   if (error)
     {
	int i;
	int chosen_size = 0;
	int chosen_width = 0;

	for (i = 0; i < fi->src->ft.face->num_fixed_sizes; i++)
	  {
	     int s;
	     int d, cd;

	     s = fi->src->ft.face->available_sizes[i].height;
	     cd = chosen_size - fi->size;
	     if (cd < 0) cd = -cd;
	     d = s - fi->size;
	     if (d < 0) d = -d;
	     if (d < cd)
	       {
		  chosen_width = fi->src->ft.face->available_sizes[i].width;
		  chosen_size = s;
	       }
	     if (d == 0) break;
	  }
	fi->real_size = chosen_size;
	error = FT_Set_Pixel_Sizes(fi->src->ft.face, chosen_width, fi->real_size);
	if (error)
	  {
	     /* couldn't choose the size anyway... what now? */
	  }
     }
   fi->src->current_size = fi->size;

   fi->glyphs = NULL;
   fi->usage = 0;
   fi->references = 1;
   fonts = evas_object_list_prepend(fonts, fi);
   return fi;
}

RGBA_Font *
evas_common_font_memory_load(const char *name, int size, const void *data, int data_size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;
   
   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (!fi) return NULL;
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
	free(fi);
	return NULL;
     }
   fn->fonts = evas_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   return fn;
}

RGBA_Font *
evas_common_font_load(const char *name, int size)
{
   RGBA_Font *fn;
   RGBA_Font_Int *fi;

   fi = evas_common_font_int_load(name, size);
   if (!fi) return NULL;
   fn = calloc(1, sizeof(RGBA_Font));
   if (!fn)
     {
	free(fi);
	return NULL;
     }
   fn->fonts = evas_list_append(fn->fonts, fi);
   fn->hinting = FONT_BYTECODE_HINT;
   fi->hinting = fn->hinting;
   return fn;
}

RGBA_Font *
evas_common_font_add(RGBA_Font *fn, const char *name, int size)
{
   RGBA_Font_Int *fi;

   if (!fn)
      return NULL;
   fi = evas_common_font_int_load(name, size);
   if (fi)
     {
	fn->fonts = evas_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
	return fn;
     }
   return NULL;
}

RGBA_Font *
evas_common_font_memory_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size)
{
   RGBA_Font_Int *fi;

   if (!fn)
      return NULL;
   fi = evas_common_font_int_memory_load(name, size, data, data_size);
   if (fi)
     {
	fn->fonts = evas_list_append(fn->fonts, fi);
	fi->hinting = fn->hinting;
	return fn;
     }
   return NULL;
}

void
evas_common_font_free(RGBA_Font *fn)
{
   Evas_List *l;

   if (!fn)
      return;
   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = l->data;
	fi->references--;
	if (fi->references == 0)
	  {
	     evas_common_font_int_modify_cache_by(fi, 1);
	     evas_common_font_flush();
	  }
     }
   evas_list_free(fn->fonts);
   free(fn);
}

void
evas_common_font_hinting_set(RGBA_Font *fn, Font_Hint_Flags hinting)
{
   Evas_List *l;
   
   if (!fn)
     return;
   fn->hinting = hinting;
   for (l = fn->fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = l->data;
	fi->hinting = fn->hinting;
     }
}

Evas_Bool
evas_common_hinting_available(Font_Hint_Flags hinting)
{
   if (hinting == FONT_NO_HINT) return 1;
   else if (hinting == FONT_AUTO_HINT)
     {
#ifdef TT_CONFIG_OPTION_UNPATENTED_HINTING
	return 1;
#else	
	return 0;
#endif
     }
   else if (hinting == FONT_BYTECODE_HINT)
     {
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
	return 1;
#else
	return 0;
#endif
     }
   return 0;
}

RGBA_Font *
evas_common_font_memory_hinting_load(const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting)
{
   RGBA_Font *fn;

   fn = evas_common_font_memory_load(name, size, data, data_size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

RGBA_Font *
evas_common_font_hinting_load(const char *name, int size, Font_Hint_Flags hinting)
{
   RGBA_Font *fn;

   fn = evas_common_font_load(name, size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

RGBA_Font *
evas_common_font_hinting_add(RGBA_Font *fn, const char *name, int size, Font_Hint_Flags hinting)
{
   fn = evas_common_font_add(fn, name, size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

RGBA_Font *
evas_common_font_memory_hinting_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting)
{
   fn = evas_common_font_memory_add(fn, name, size, data, data_size);
   if (fn) evas_common_font_hinting_set(fn, hinting);
   return fn;
}

static Evas_Bool
font_modify_cache_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   int *dir;
   RGBA_Font_Glyph *fg;

   fg = data;
   dir = fdata;
   font_cache_usage += (*dir) *
     ((fg->glyph_out->bitmap.width * fg->glyph_out->bitmap.rows) +
      sizeof(RGBA_Font_Glyph) + sizeof(Evas_List) + 400); /* fudge values */
   return 1;
   hash = 0;
   key = 0;
}

void
evas_common_font_int_modify_cache_by(RGBA_Font_Int *fi, int dir)
{
   int sz_hash = 0;

   if (fi->glyphs) sz_hash = sizeof(Evas_Hash);
   evas_hash_foreach(fi->glyphs, font_modify_cache_cb, &dir);
   font_cache_usage += dir * (sizeof(RGBA_Font) + sz_hash +
			      sizeof(FT_FaceRec) + 16384); /* fudge values */
}

int
evas_common_font_cache_get(void)
{
   return font_cache;
}

void
evas_common_font_cache_set(int size)
{
   font_cache = size;
   evas_common_font_flush();
}

void
evas_common_font_flush(void)
{
   if (font_cache_usage < font_cache) return;
   while (font_cache_usage > font_cache) evas_common_font_flush_last();
}

static Evas_Bool
font_flush_free_glyph_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   RGBA_Font_Glyph *fg;

   fg = data;
   FT_Done_Glyph(fg->glyph);
   /* extension calls */
   if (fg->ext_dat_free) fg->ext_dat_free(fg->ext_dat);
   free(fg);
   return 1;
   hash = 0;
   key = 0;
   fdata = 0;
}

void
evas_common_font_flush_last(void)
{
   Evas_Object_List *l;
   RGBA_Font_Int *fi = NULL;

   for (l = fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi_tmp;

	fi_tmp = (RGBA_Font_Int *)l;
	if (fi_tmp->references == 0) fi = fi_tmp;
     }
   if (!fi) return;

   FT_Done_Size(fi->ft.size);

   fonts = evas_object_list_remove(fonts, fi);
   evas_common_font_int_modify_cache_by(fi, -1);

   evas_hash_foreach(fi->glyphs, font_flush_free_glyph_cb, NULL);
   evas_hash_free(fi->glyphs);

   evas_common_font_source_free(fi->src);

   free(fi);
}

RGBA_Font_Int *
evas_common_font_int_find(const char *name, int size)
{
   Evas_Object_List *l;

   for (l = fonts; l; l = l->next)
     {
	RGBA_Font_Int *fi;

	fi = (RGBA_Font_Int *)l;
	if ((fi->size == size) && (!strcmp(name, fi->src->name)))
	  {
	     if (fi->references == 0) evas_common_font_int_modify_cache_by(fi, -1);
	     fi->references++;
	     fonts = evas_object_list_remove(fonts, fi);
	     fonts = evas_object_list_prepend(fonts, fi);
	     return fi;
	  }
     }
   return NULL;
}
