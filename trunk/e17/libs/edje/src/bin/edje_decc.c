/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/* ugly ugly. avert your eyes. */
#include "edje_decc.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

char *progname = NULL;
char *file_in = NULL;

Edje_File *edje_file = NULL;
SrcFile_List *srcfiles = NULL;
Font_List *fontlist = NULL;

int line = 0;

int        decomp(void);
void       output(void);
int        e_file_is_dir(char *file);
int        e_file_mkdir(char *dir);
int        e_file_mkpath(char *path);
static int compiler_cmd_is_sane();

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s input_file.eet \n"
      "\n"
      ,progname);
}

int
main(int argc, char **argv)
{
   int i;

   setlocale(LC_NUMERIC, "C");
   
   progname = argv[0];
   for (i = 1; i < argc; i++)
     {
	if (!file_in)
	  file_in = argv[i];
     }
   if (!file_in)
     {
	fprintf(stderr, "%s: Error: no input file specified.\n", progname);
	main_help();
	exit(-1);
     }

   edje_init();
   eet_init();
   source_edd();
   
   if (!decomp()) return -1;
   output();

   eet_shutdown();
   return 0;
}

int
decomp(void)
{
   Eet_File *ef;
   ef = eet_open(file_in, EET_FILE_MODE_READ);
   if (!ef)
     {
	printf("ERROR: cannot open %s\n", file_in);
	return 0;
     }
   
   srcfiles = source_load(ef);
   if (!srcfiles)
     {
	printf("ERROR: %s has no decompile information\n", file_in);
	eet_close(ef);
	return 0;
     }
   edje_file = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
   if (!edje_file)
     {
	printf("ERROR: %s does not appear to be an edje file\n", file_in);
	eet_close(ef);
	return 0;
     }
   if (!edje_file->compiler)
     {
	edje_file->compiler = strdup("edje_cc");
     }
   else if (!compiler_cmd_is_sane())
     {
	printf("ERROR: invalid compiler executable: '%s'\n", edje_file->compiler);
	eet_close(ef);
	return 0;
     }
   fontlist = source_fontmap_load(ef);
   eet_close(ef);
   return 1;
}

void
output(void)
{
   Evas_List *l;
   Eet_File *ef;
   char *outdir, *p;
   
   p = strrchr(file_in, '/');
   if (p)
     outdir = strdup(p + 1);
   else
     outdir = strdup(file_in);
   p = strrchr(outdir, '.');
   if (p) *p = 0;
   
   e_file_mkpath(outdir);
   
   ef = eet_open(file_in, EET_FILE_MODE_READ);

#ifdef HAVE_IMLIB
   if (edje_file->image_dir)
     {
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *ei;
	     
	     ei = l->data;
	     if ((ei->source_type) && (ei->entry))
	       {
		  DATA32 *pix;
		  int w, h, alpha, comp, qual, lossy;
		  char buf[4096];
		  
		  snprintf(buf, sizeof(buf), "images/%i", ei->id);
		  pix = eet_data_image_read(ef, buf, &w, &h, &alpha, &comp, &qual, &lossy);
		  if (pix)
		    {
		       Imlib_Image im;
		       char out[4096];
		       char *pp;
		       
		       snprintf(out, sizeof(out), "%s/%s", outdir, ei->entry);
		       printf("Output Image: %s\n", out);
		       pp = strdup(out);
		       p = strrchr(pp, '/');
		       *p = 0;
		       if (strstr(pp, "../"))
			 {
			    printf("ERROR: potential security violation. attempt to write in parent dir.\n");
			    exit (-1);
			 }
		       e_file_mkpath(pp);
		       free(pp);
		       im = imlib_create_image_using_data(w, h, pix);
		       imlib_context_set_image(im);
		       if (alpha)
			 imlib_image_set_has_alpha(1);
		       if ((lossy) && (!alpha))
			 {
			    imlib_image_set_format("jpg");
			    imlib_image_attach_data_value("quality", NULL, qual, NULL);
			 }
		       else
			 {
			    imlib_image_set_format("png");
			 }
		       if (strstr(out, "../"))
			 {
			    printf("ERROR: potential security violation. attempt to write in parent dir.\n");
			    exit (-1);
			 }
		       imlib_save_image(out);
		       imlib_free_image();
		       free(pix);
		    }
	       }
	  }
     }
#endif

   for (l = srcfiles->list; l; l = l->next)
     {
	SrcFile *sf;
	char out[4096];
	FILE *f;
	char *pp;
	
	sf = l->data;
	snprintf(out, sizeof(out), "%s/%s", outdir, sf->name);
	printf("Output Source File: %s\n", out);
	pp = strdup(out);
	p = strrchr(pp, '/');
	*p = 0;
	if (strstr(pp, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	e_file_mkpath(pp);
	free(pp);
	if (strstr(out, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	f = fopen(out, "w");
	if (!f) 
	  {
	     printf("ERROR: unable to write file (%s).\n", out);
	     exit (-1);
	  }
	fputs(sf->file, f);
	fclose(f);
     }
   if (fontlist)
     {
	for (l = fontlist->list; l; l = l->next)
	  {
	     Font *fn;
	     void *font;
	     int fontsize;
	     char out[4096];
	     
	     fn = l->data;
	     snprintf(out, sizeof(out), "fonts/%s", fn->name);
	     font = eet_read(ef, out, &fontsize);
	     if (font)
	       {
		  FILE *f;
		  char *pp;
		  
		  snprintf(out, sizeof(out), "%s/%s", outdir, fn->file);
		  printf("Output Font: %s\n", out);
		  pp = strdup(out);
		  p = strrchr(pp, '/');
		  *p = 0;
		  if (strstr(pp, "../"))
		    {
		       printf("ERROR: potential security violation. attempt to write in parent dir.\n");
		       exit (-1);
		    }
		  e_file_mkpath(pp);
		  free(pp);
		  if (strstr(out, "../"))
		    {
		       printf("ERROR: potential security violation. attempt to write in parent dir.\n");
		       exit (-1);
		    }
		  f = fopen(out, "wb");
		  fwrite(font, fontsize, 1, f);
		  fclose(f);
		  free(font);
	       }
	  }
     }
     {
	char out[4096];
	FILE *f;
	
	snprintf(out, sizeof(out), "%s/build.sh", outdir);
	printf("Output Build Script: %s\n", out);
	if (strstr(out, "../"))
	  {
	     printf("ERROR: potential security violation. attempt to write in parent dir.\n");
	     exit (-1);
	  }
	f = fopen(out, "w");
	fprintf(f, "#!/bin/sh\n");
	fprintf(f, "%s $@ -id . -fd . main_edje_source.edc -o %s.eet\n", edje_file->compiler, outdir);
	fclose(f);

#ifndef WIN32
	chmod(out, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
#endif

	printf("\n*** CAUTION ***\n"
	      "Please check the build script for anythin malicious "
	      "before running it!\n\n");
     }
   eet_close(ef);
}

int
e_file_is_dir(char *file)
{
   struct stat st;
   
   if (stat(file, &st) < 0) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

int
e_file_mkdir(char *dir)
{
#ifndef WIN32
   static mode_t default_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

   if (mkdir(dir, default_mode) < 0) return 0;
#else
   if (mkdir(dir) < 0) return 0;
#endif
   return 1;
}

int
e_file_mkpath(char *path)
{
   char ss[PATH_MAX];
   int  i, ii;
   
   ss[0] = 0;
   i = 0;
   ii = 0;
   while (path[i])
     {
	if (ii == sizeof(ss) - 1) return 0;
	ss[ii++] = path[i];
	ss[ii] = 0;
	if (path[i] == '/')
	  {
	     if (!e_file_is_dir(ss)) e_file_mkdir(ss);
	     else if (!e_file_is_dir(ss)) return 0;
	  }
	i++;
     }
   if (!e_file_is_dir(ss)) e_file_mkdir(ss);
   else if (!e_file_is_dir(ss)) return 0;
   return 1;
}

static int
compiler_cmd_is_sane()
{
   char *c = edje_file->compiler, *ptr;

   if (!c || !*c)
     {
	return 0;
     }

   for (ptr = c; ptr && *ptr; ptr++)
     {
	/* only allow [a-z][A-Z][0-9]_- */
	if (!isalnum(*ptr) && *ptr != '_' && *ptr != '-')
	  {
	     return 0;
	  }
     }

   return 1;
}
