#include "evas_common.h"
#include "evas_private.h"
#ifdef BUILD_FONT_LOADER_EET
#include <Eet.h>
#endif
#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

/* font dir cache */
static Evas_Hash *font_dirs = NULL;
static Evas_List *fonts_cache = NULL;
static Evas_List *fonts_zero = NULL;

typedef struct _Fndat Fndat;

struct _Fndat
{
   const char *name;
   const char *source;
   int         size;
   void       *font;
   int         ref;
};

/* private methods for font dir cache */
static Evas_Bool font_cache_dir_free(Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Font_Dir *object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd);
static Evas_Font *object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font);
static Evas_Font_Dir *object_text_font_cache_dir_add(char *dir);
static void object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd);
static int evas_object_text_font_string_parse(char *buffer, char dest[14][256]);

void
evas_font_dir_cache_free(void)
{
   if (!font_dirs) return;

   evas_hash_foreach(font_dirs, font_cache_dir_free, NULL);
   evas_hash_free(font_dirs);
   font_dirs = NULL;
}

const char *
evas_font_dir_cache_find(char *dir, char *font)
{
   Evas_Font_Dir *fd;

   fd = evas_hash_find(font_dirs, dir);
   fd = object_text_font_cache_dir_update(dir, fd);
   if (fd)
     {
	Evas_Font *fn;

	fn = object_text_font_cache_font_find(fd, font);
	if (fn)
	  {
	     return fn->path;
	  }
     }
   return NULL;
}

static Evas_List *
evas_font_set_get(const char *name)
{
   Evas_List *fonts = NULL;
   char *p;

   p = strchr(name, ',');
   if (!p)
     {
	fonts = evas_list_append(fonts, evas_stringshare_add(name));
     }
   else
     {
	const char *pp;
	char *nm;

	pp = name;
	while (p)
	  {
	     nm = alloca(p - pp + 1);
	     strncpy(nm, pp, p - pp);
	     nm[p - pp] = 0;
	     fonts = evas_list_append(fonts, evas_stringshare_add(nm));
	     pp = p + 1;
	     p = strchr(pp, ',');
	     if (!p) fonts = evas_list_append(fonts, evas_stringshare_add(pp));
	  }
     }
   return fonts;
}

void
evas_font_free(Evas *evas, void *font)
{
   Evas_List *l;

   for (l = fonts_cache; l; l = l->next)
     {
	Fndat *fd;

	fd = l->data;
	if (fd->font == font)
	  {
	     fd->ref--;
	     if (fd->ref == 0)
	       {
		  fonts_cache = evas_list_remove_list(fonts_cache, l);
		  fonts_zero = evas_list_append(fonts_zero, fd);
	       }
	     break;
	  }
     }
   while ((fonts_zero) &&
	  (evas_list_count(fonts_zero) > 4)) /* 4 is arbitrary */
     {
	Fndat *fd;

	fd = evas_list_data(fonts_zero);
	if (fd->ref != 0) break;
	fonts_zero = evas_list_remove_list(fonts_zero, fonts_zero);
	if (fd->name) evas_stringshare_del(fd->name);
	if (fd->source) evas_stringshare_del(fd->source);
	evas->engine.func->font_free(evas->engine.data.output, fd->font);
	free(fd);
     }
}

void *
evas_font_load(Evas *evas, const char *name, const char *source, int size)
{
   void *font = NULL;
   Evas_List *fonts, *l;
   Fndat *fd;

   if (!name) return NULL;
   if (name[0] == 0) return NULL;

   for (l = fonts_cache; l; l = l->next)
     {
	fd = l->data;
	if (!strcmp(name, fd->name))
	  {
	     if (((!source) && (!fd->source)) ||
		 ((source) && (fd->source) && (!strcmp(source, fd->source))))
	       {
		  if (size == fd->size)
		    {
		       fonts_cache = evas_list_promote_list(fonts_cache, l);
		       fd->ref++;
		       return fd->font;
		    }
	       }
	  }
     }
   for (l = fonts_zero; l; l = l->next)
     {
	fd = l->data;
	if (!strcmp(name, fd->name))
	  {
	     if (((!source) && (!fd->source)) ||
		 ((source) && (fd->source) && (!strcmp(source, fd->source))))
	       {
		  if (size == fd->size)
		    {
		       fonts_zero = evas_list_remove_list(fonts_zero, l);
		       fonts_cache = evas_list_prepend(fonts_cache, fd);
		       fd->ref++;
		       return fd->font;
		    }
	       }
	  }
     }
   fonts = evas_font_set_get(name);
   for (l = fonts; l; l = l->next) /* Load each font in append */
     {
	char *nm;

	nm = l->data;
	if ((l == fonts) || (!font)) /* First iteration OR no font */
	  {
#ifdef BUILD_FONT_LOADER_EET
	     if (source) /* Load Font from "eet" source */
	       {
		  Eet_File *ef;
		  char *fake_name;

		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       font = evas->engine.func->font_load(evas->engine.data.output, fake_name, size);
		       if (!font) /* Load from fake name failed, probably not cached */
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;

				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      font = evas->engine.func->font_memory_load(evas->engine.data.output, fake_name, size, fdata, fsize);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       free(fake_name);
		    }
	       }
	     if (!font) /* Source load failed */
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm)) /* Try filename */
		    font = evas->engine.func->font_load(evas->engine.data.output, (char *)nm, size);
		  else /* search font path */
		    {
		       Evas_List *l;

		       for (l = evas->font_path; l; l = l->next)
			 {
			    const char *f_file;

			    f_file = evas_font_dir_cache_find(l->data, (char *)nm);
			    if (f_file)
			      {
				 font = evas->engine.func->font_load(evas->engine.data.output, f_file, size);
				 if (font) break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	else /* Base font loaded, append others */
	  {
#ifdef BUILD_FONT_LOADER_EET
	     void *ok = NULL;

	     if (source)
	       {
		  Eet_File *ef;
		  char *fake_name;

		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       /* FIXME: make an engine func */
		       if (!evas->engine.func->font_add(evas->engine.data.output, font, fake_name, size))
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;

				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      ok = evas->engine.func->font_memory_add(evas->engine.data.output, font, fake_name, size, fdata, fsize);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       else
			 ok = (void *)1;
		       free(fake_name);
		    }
	       }
	     if (!ok)
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm))
		    evas->engine.func->font_add(evas->engine.data.output, font, (char *)nm, size);
		  else
		    {
		       Evas_List *l;

		       for (l = evas->font_path; l; l = l->next)
			 {
			    const char *f_file;

			    f_file = evas_font_dir_cache_find(l->data, (char *)nm);
			    if (f_file)
			      {
				 if (evas->engine.func->font_add(evas->engine.data.output, font, f_file, size))
				   break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	evas_stringshare_del(nm);
     }
   evas_list_free(fonts);

#ifdef HAVE_FONTCONFIG

   if (!font) /* Search using fontconfig */
     {
	FcPattern *p_nm = NULL;
	FcFontSet *set;
	FcResult res;
	int i;

	p_nm = FcNameParse((FcChar8 *)name);
	FcConfigSubstitute(NULL, p_nm, FcMatchPattern);
	FcDefaultSubstitute(p_nm);

	/* do matching */
	set = FcFontSort(NULL, p_nm, FcTrue, NULL, &res);

	/* Do loading for all in family */
	for (i = 0; i < set->nfont; i++)
	  {
	     FcValue filename;

	     FcPatternGet(set->fonts[i], FC_FILE, 0, &filename);

	     if (font)
	       evas->engine.func->font_add(evas->engine.data.output, font, (char *)filename.u.s, size);
	     else
	       font = evas->engine.func->font_load(evas->engine.data.output, (char *)filename.u.s, size);
	  }

	FcFontSetDestroy(set);
	FcPatternDestroy(p_nm);
     }
#endif

   fd = calloc(1, sizeof(Fndat));
   if (fd)
     {
	fd->name = evas_stringshare_add(name);
	if (source) fd->source = evas_stringshare_add(source);
	fd->size = size;
	fd->font = font;
	fd->ref = 1;
	fonts_cache = evas_list_prepend(fonts_cache, fd);
     }
   if (font)
     evas->engine.func->font_hinting_set(evas->engine.data.output, font,
					 evas->hinting);
   return font;
}

void
evas_font_load_hinting_set(Evas *evas, void *font, int hinting)
{
   evas->engine.func->font_hinting_set(evas->engine.data.output, font,
				       hinting);
}

Evas_List *
evas_font_dir_available_list(Evas *evas)
{
   Evas_List *l;
   Evas_List *ll;
   Evas_List *available = NULL;

#ifdef HAVE_FONTCONFIG
   /* Add font config fonts */
   FcPattern *p;
   FcFontSet *set = NULL;
   FcObjectSet *os;
   int i;

   p = FcPatternCreate();
   os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, NULL);

   if (p && os) set = FcFontList(NULL, p, os);

   if (p) FcPatternDestroy(p);
   if (os) FcObjectSetDestroy(os);

   if (set)
     {
	for (i = 0; i < set->nfont; i++)
	  {
	     char *font;

	     font = (char *)FcNameUnparse(set->fonts[i]);
	     available = evas_list_append(available, evas_stringshare_add(font));
	     free(font);
	  }

	FcFontSetDestroy(set);
     }
#endif

   /* Add fonts in evas font_path*/
   if (!evas->font_path)
     return available;

   for (l = evas->font_path; l; l = l->next)
     {
	Evas_Font_Dir *fd;

	fd = evas_hash_find(font_dirs, (char *)l->data);
	fd = object_text_font_cache_dir_update((char *)l->data, fd);
	if (fd && fd->aliases)
	  {
	     for (ll = fd->aliases; ll; ll = ll->next)
	       {
		  Evas_Font_Alias *fa;

		  fa = ll->data;
		  available = evas_list_append(available, evas_stringshare_add((char *)fa->alias));
	       }
	  }
     }

   return available;
}

void
evas_font_dir_available_list_free(Evas_List *available)
{
   while (available)
     {
	evas_stringshare_del(available->data);
	available = evas_list_remove(available, available->data);
     }
}

/* private stuff */
static Evas_Bool
font_cache_dir_free(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   object_text_font_cache_dir_del((char *) key, data);
   return 1;
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
   char *tmp, *tmp2;
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
			      fn->x.prop[i] = evas_stringshare_add(font_prop[i]);
			    tmp2 = evas_file_path_join(dir, fname);
			    if (tmp2)
			      {
				 fn->path = evas_stringshare_add(tmp2);
				 free(tmp2);
			      }
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
		  char *p;

		  fn->type = 0;
		  tmp2 = alloca(strlen(fdir->data) + 1);
		  strcpy(tmp2, fdir->data);
		  p = strrchr(tmp2, '.');
		  if (p) *p = 0;
		  fn->simple.name = evas_stringshare_add(tmp2);
		  tmp2 = evas_file_path_join(dir, fdir->data);
		  if (tmp2)
		    {
		       fn->path = evas_stringshare_add(tmp2);
		       free(tmp2);
		    }
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
		  if ((fname[0] == '!') || (fname[0] == '#')) continue;
		  fa = calloc(1, sizeof(Evas_Font_Alias));
		  if (fa)
		    {
		       fa->alias = evas_stringshare_add(fname);
		       fa->fn = object_text_font_cache_font_find_x(fd, fdef);
		       if ((!fa->alias) || (!fa->fn))
			 {
			    if (fa->alias) evas_stringshare_del(fa->alias);
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
	     if (fn->x.prop[i]) evas_stringshare_del(fn->x.prop[i]);
	  }
	if (fn->simple.name) evas_stringshare_del(fn->simple.name);
	if (fn->path) evas_stringshare_del(fn->path);
	free(fn);
     }
   while (fd->aliases)
     {
	Evas_Font_Alias *fa;

	fa = fd->aliases->data;
	fd->aliases = evas_list_remove(fd->aliases, fa);
	if (fa->alias) evas_stringshare_del(fa->alias);
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
