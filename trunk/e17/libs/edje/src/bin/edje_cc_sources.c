#include "edje_cc.h"

static Eet_Data_Descriptor *_srcfile_edd = NULL;
static Eet_Data_Descriptor *_srcfile_list_edd = NULL;

static Eet_Data_Descriptor *_font_edd = NULL;
static Eet_Data_Descriptor *_font_list_edd = NULL;

static SrcFile_List srcfiles = {NULL};

void
source_edd(void)
{
   _srcfile_edd = eet_data_descriptor_new("srcfile", sizeof(SrcFile),
					  (void *(*) (void *))evas_list_next,
					  (void *(*) (void *, void *))evas_list_append,
					  (void *(*) (void *))evas_list_data,
					  (void *(*) (void *))evas_list_free,
					  (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					  (void *(*) (void *, const char *, void *))evas_hash_add,
					  (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "file", file, EET_T_STRING);
   _srcfile_list_edd = eet_data_descriptor_new("srcfile_list", sizeof(SrcFile_List),
					       (void *(*) (void *))evas_list_next,
					       (void *(*) (void *, void *))evas_list_append,
					       (void *(*) (void *))evas_list_data,
					       (void *(*) (void *))evas_list_free,
					       (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					       (void *(*) (void *, const char *, void *))evas_hash_add,
					       (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_LIST(_srcfile_list_edd, SrcFile_List, "list", list, _srcfile_edd);
   
   _font_edd = eet_data_descriptor_new("font", sizeof(Font),
				       (void *(*) (void *))evas_list_next,
				       (void *(*) (void *, void *))evas_list_append,
				       (void *(*) (void *))evas_list_data,
				       (void *(*) (void *))evas_list_free,
				       (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
				       (void *(*) (void *, const char *, void *))evas_hash_add,
				       (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font, "file", file, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font, "name", name, EET_T_STRING);
   _font_list_edd = eet_data_descriptor_new("font_list", sizeof(Font_List),
					    (void *(*) (void *))evas_list_next,
					    (void *(*) (void *, void *))evas_list_append,
					    (void *(*) (void *))evas_list_data,
					    (void *(*) (void *))evas_list_free,
					    (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach,
					    (void *(*) (void *, const char *, void *))evas_hash_add,
					    (void  (*) (void *))evas_hash_free);
   EET_DATA_DESCRIPTOR_ADD_LIST(_font_list_edd, Font_List, "list", list, _font_edd);
}

void
source_fetch(void)
{
   FILE *f;
   char buf[256 * 1024];
   
   f = fopen(file_in, "r");
   if (!f)
     {
	return;
     }
   else
     {
	long sz;
	SrcFile *sf;
	
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	sf = mem_alloc(SZ(SrcFile));
	sf->name = strdup("main_edje_source.edc");
	sf->file = mem_alloc(sz);
	fread(sf->file, sz, 1, f);
	fseek(f, 0, SEEK_SET);
	srcfiles.list = evas_list_append(srcfiles.list, sf);
    }
   
   while (fgets(buf, sizeof(buf), f))
     {
	char *p, *pp;
	int notyet;
	int forgetit;
	int haveinclude;
	int havefile;
	char *file;
	
	p = buf;
	notyet = 1;
	forgetit = 0;
	haveinclude = 0;
	havefile = 0;
	file = NULL;
	while ((!forgetit) && (*p))
	  {
	     if (notyet)
	       {
		  if (!isblank(*p))
		    {
		       if (*p == '#')
			 notyet = 0;
		       else
			 forgetit = 1;
		    }
		  p++;
	       }
	     else
	       {
		  if (!haveinclude)
		    {
		       if (!isblank(*p))
			 {
			    if (!strncmp(p, "include", 7))
			      {
				 haveinclude = 1;
				 p += 7;
			      }
			    else
			      forgetit = 1;
			 }
		    }
		  else
		    {
		       if (!havefile)
			 {
			    if (!isblank(*p))
			      {
				 if (*p == '"')
				   {
				      pp = strchr(p + 1, '"');
				      if (!pp)
					forgetit = 1;
				      else
					{
					   file = mem_alloc(pp - p);
					   strncpy(file, p + 1, pp - p - 1);
					   file[pp - p - 1] = 0;
					   forgetit = 1;
					}
				   }
				 else if (*p == '<')
				   {
				      pp = strchr(p + 1, '>');
				      if (!pp)
					forgetit = 1;
				      else
					{
					   file = mem_alloc(pp - p);
					   strncpy(file, p + 1, pp - p - 1);
					   file[pp - p - 1] = 0;
					   forgetit = 1;
					}
				   }
				 else
				   forgetit = 1;
			      }
			 }
		    }
	       }
	  }
	if (file)
	  {
	     FILE *ff;
	     
	     ff = fopen(file, "r");
	     if (ff)
	       {
		  long sz;
		  SrcFile *sf;
		  
		  fseek(ff, 0, SEEK_END);
		  sz = ftell(ff);
		  fseek(ff, 0, SEEK_SET);
		  sf = mem_alloc(SZ(SrcFile));
		  sf->name = file;
		  sf->file = mem_alloc(sz);
		  fread(sf->file, sz, 1, ff);
		  fclose(ff);
		  srcfiles.list = evas_list_append(srcfiles.list, sf);
	       }
	     else
	       {
		  free(file);
	       }
	  }
     }
   fclose(f);
}

int
source_append(Eet_File *ef)
{
   return eet_data_write(ef, _srcfile_list_edd, "edje_sources", &srcfiles, 1);
}

SrcFile_List *
source_load(Eet_File *ef)
{
   SrcFile_List *s;
   
   s = eet_data_read(ef, _srcfile_list_edd, "edje_sources");
   return s;
}

int
source_fontmap_save(Eet_File *ef, Evas_List *fonts)
{
   Font_List fl;
   
   fl.list = fonts;
   return eet_data_write(ef, _font_list_edd, "edje_source_fontmap", &fl, 1);
}

Font_List *
source_fontmap_load(Eet_File *ef)
{
   Font_List *fl;
   
   fl = eet_data_read(ef, _font_list_edd, "edje_source_fontmap");
   return fl;
}
