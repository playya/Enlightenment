/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

typedef struct _Part_Lookup Part_Lookup;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Image_Lookup Image_Lookup;
typedef struct _Slave_Lookup Slave_Lookup;
typedef struct _Code_Lookup Code_Lookup;

struct _Part_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Program_Lookup
{
   Edje_Part_Collection *pc;
   char *name;
   int *dest;
};

struct _Image_Lookup
{
   char *name;
   int *dest;
};

struct _Slave_Lookup
{
   int *master;
   int *slave;
};

struct _Code_Lookup
{
   char *ptr;
   int   len;
   int   val;
};

static void data_queue_image_pc_lookup(Edje_Part_Collection *pc, char *name, int *dest);
static void data_process_string(Edje_Part_Collection *pc, char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, int *val));

Edje_File *edje_file = NULL;
Evas_List *edje_collections = NULL;
Evas_List *fonts = NULL;
Evas_List *codes = NULL;
Evas_List *code_lookups = NULL;

static Eet_Data_Descriptor *edd_edje_file = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory = NULL;
static Eet_Data_Descriptor *edd_edje_image_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_program = NULL;
static Eet_Data_Descriptor *edd_edje_program_target = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection_directory_entry = NULL;
static Eet_Data_Descriptor *edd_edje_part_collection = NULL;
static Eet_Data_Descriptor *edd_edje_part = NULL;
static Eet_Data_Descriptor *edd_edje_part_description = NULL;
static Eet_Data_Descriptor *edd_edje_part_image_id = NULL;

static Evas_List *part_lookups = NULL;
static Evas_List *program_lookups = NULL;
static Evas_List *image_lookups = NULL;
static Evas_List *part_slave_lookups = NULL;
static Evas_List *image_slave_lookups= NULL;

#define ABORT_WRITE(eet_file, file) \
   eet_close(eet_file); \
   unlink(file); \
   exit(-1);

void
data_setup(void)
{
   edd_edje_file = _edje_edd_edje_file;
   edd_edje_image_directory = _edje_edd_edje_image_directory;
   edd_edje_image_directory_entry = _edje_edd_edje_image_directory_entry;
   edd_edje_program = _edje_edd_edje_program;
   edd_edje_program_target = _edje_edd_edje_program_target;
   edd_edje_part_collection_directory = _edje_edd_edje_part_collection_directory;
   edd_edje_part_collection_directory_entry = _edje_edd_edje_part_collection_directory_entry;
   edd_edje_part_collection = _edje_edd_edje_part_collection;
   edd_edje_part = _edje_edd_edje_part;
   edd_edje_part_description = _edje_edd_edje_part_description;
   edd_edje_part_image_id = _edje_edd_edje_part_image_id;
}

void
data_write(void)
{
   Eet_File *ef;
   Evas_List *l;
   int bytes;
   int input_bytes;
   int total_bytes;
   int src_bytes;
   int fmap_bytes;
   int input_raw_bytes;
   int image_num;
   int font_num;
   int collection_num;
   int i;
   
   bytes = 0;
   input_bytes = 0;
   total_bytes = 0;
   src_bytes = 0;
   fmap_bytes = 0;
   input_raw_bytes = 0;
   image_num = 0;
   font_num = 0;
   collection_num = 0;
   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open \"%s\" for writing output\n",
		progname, file_out);
	exit(-1);
     }
   if (edje_file)
     {
	bytes = eet_data_write(ef, edd_edje_file, "edje_file", edje_file, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"edje_file\" entry to \"%s\" \n",
		     progname, file_out);	
	     ABORT_WRITE(ef, file_out);
	  }
	else
	  total_bytes += bytes;
     }
   if (verbose)
     {
	printf("%s: Wrote %9i bytes (%4iKb) for \"edje_file\" header\n",
	       progname, bytes, (bytes + 512) / 1024);
     }
   for (l = fonts; l; l = l->next)
     {
	Font *fn;
	void *fdata = NULL;
	int fsize = 0;
	Evas_List *ll;
	FILE *f;
	
	fn = l->data;
	f = fopen(fn->file, "rb");
	if (f)
	  {
	     long pos;
	     
	     fseek(f, 0, SEEK_END);
	     pos = ftell(f);
	     rewind(f);
	     fdata = malloc(pos);
	     if (fdata)
	       {
		  if (fread(fdata, pos, 1, f) != 1)
		    {
		       fprintf(stderr, "%s: Error. unable to read all of font file \"%s\"\n",
			       progname, fn->file);
		       ABORT_WRITE(ef, file_out);
		    }
		  fsize = pos;
	       }
	     fclose(f);
	  }
	else
	  {
	     for (ll = fnt_dirs; ll; ll = ll->next)
	       {
		  char buf[4096];
		  
		  snprintf(buf, sizeof(buf), "%s/%s", (char *)(ll->data), fn->file);
		  f = fopen(buf, "rb");
		  if (f)
		    {
		       long pos;
		       
		       fseek(f, 0, SEEK_END);
		       pos = ftell(f);
		       rewind(f);
		       fdata = malloc(pos);
		       if (fdata)
			 {
			    if (fread(fdata, pos, 1, f) != 1)
			      {
				 fprintf(stderr, "%s: Error. unable to read all of font file \"%s\"\n",
					 progname, buf);
				 ABORT_WRITE(ef, file_out);
			      }
			    fsize = pos;
			 }
		       fclose(f);
		       if (fdata) break;
		    }
	       }
	  }
	if (!fdata)
	  {
	     fprintf(stderr, "%s: Error. unable to load font part \"%s\" entry to %s \n",
		     progname, fn->file, file_out);
	     ABORT_WRITE(ef, file_out);
	  }
	else
	  {
	     char buf[4096];
	     
	     snprintf(buf, sizeof(buf), "fonts/%s", fn->name);
	     bytes = eet_write(ef, buf, fdata, fsize, 1);
	     if (bytes <= 0)
	       {
		  fprintf(stderr, "%s: Error. unable to write font part \"%s\" as \"%s\" part entry to %s \n",
			  progname, fn->file, buf, file_out);	
		  ABORT_WRITE(ef, file_out);
	       }
	     else
	       {
		  font_num++;
		  total_bytes += bytes;
		  input_bytes += fsize;
		  input_raw_bytes += fsize;
	       }
	     if (verbose)
	       {
		  printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" font entry \"%s\" compress: [real: %2.1f%%]\n",
			 progname, bytes, (bytes + 512) / 1024, buf, fn->file,
			 100 - (100 * (double)bytes) / ((double)(fsize))
			 );
	       }
	     free(fdata);
	  }
     }
   if ((edje_file) && (edje_file->image_dir))
     {
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *img;
	     
	     img = l->data;	
	     if (img->source_type != EDJE_IMAGE_SOURCE_TYPE_EXTERNAL)
	       {
		  Imlib_Image im;
		  Evas_List *l;
		  
		  im = NULL;
		  imlib_set_cache_size(0);	     
		  for (l = img_dirs; l; l = l->next)
		    {
		       char buf[4096];
		       
		       snprintf(buf, sizeof(buf), "%s/%s", 
				(char *)(l->data), img->entry);
		       im = imlib_load_image(buf);
		       if (im) break;
		    }
		  if (!im) im = imlib_load_image(img->entry);
		  if (im)
		    {
		       DATA32 *im_data;
		       int  im_w, im_h;
		       int  im_alpha;
		       char buf[256];
		       
		       imlib_context_set_image(im);
		       im_w = imlib_image_get_width();
		       im_h = imlib_image_get_height();
		       if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
			   (img->source_param == 0) &&
			   (scale_raw != 100))
			 {
			    im = imlib_create_cropped_scaled_image(0, 0,
								   im_w, im_h,
								   (im_w * scale_raw) / 100,
								   (im_h * scale_raw) / 100);
			    imlib_free_image();
			    imlib_context_set_image(im);
			    im_w = imlib_image_get_width();
			    im_h = imlib_image_get_height();
			 }
		       else if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
				(img->source_param == 1) &&
				(scale_comp != 100))
			 {
			    im = imlib_create_cropped_scaled_image(0, 0,
								   im_w, im_h,
								   (im_w * scale_comp) / 100,
								   (im_h * scale_comp) / 100);
			    imlib_free_image();
			    imlib_context_set_image(im);
			    im_w = imlib_image_get_width();
			    im_h = imlib_image_get_height();
			 }
		       else if (scale_lossy != 100)
			 {
			    im = imlib_create_cropped_scaled_image(0, 0,
								   im_w, im_h,
								   (im_w * scale_lossy) / 100,
								   (im_h * scale_lossy) / 100);
			    imlib_free_image();
			    imlib_context_set_image(im);
			    im_w = imlib_image_get_width();
			    im_h = imlib_image_get_height();
			 }
		       im_alpha = imlib_image_has_alpha();
		       im_data = imlib_image_get_data_for_reading_only();
		       if ((im_data) && (im_w > 0) && (im_h > 0))
			 {
			    int mode, qual;
			    
			    snprintf(buf, sizeof(buf), "images/%i", img->id);
			    mode = 2;
			    qual = 80;
			    if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
				(img->source_param == 0))
			      mode = 0; /* RAW */
			    else if ((img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT) &&
				     (img->source_param == 1))
			      mode = 1; /* COMPRESS */
			    else
			      mode = 2; /* LOSSY */
			    if ((mode == 0) && (no_raw))
			      {
				 mode = 1; /* promote compression */
				 img->source_param = 95;
			      }
			    if ((mode == 2) && (no_lossy)) mode = 1; /* demote compression */
			    if ((mode == 1) && (no_comp))
			      {
				 if (no_lossy) mode = 0; /* demote compression */
				 else if (no_raw)
				   {
				      img->source_param = 90;
				      mode = 2; /* no choice. lossy */
				   }
			      }
			    if (mode == 2)
			      {
				 qual = img->source_param;
				 if (qual < min_quality) qual = min_quality;
				 if (qual > max_quality) qual = max_quality;
			      }
			    if (mode == 0)
			      bytes = eet_data_image_write(ef, buf, 
							   im_data, im_w, im_h,
							   im_alpha, 
							   0, 0, 0);
			    else if (mode == 1)
			      bytes = eet_data_image_write(ef, buf, 
							   im_data, im_w, im_h,
							   im_alpha, 
							   1, 0, 0);
			    else if (mode == 2)
			      bytes = eet_data_image_write(ef, buf, 
							   im_data, im_w, im_h,
							   im_alpha,
							   0, qual, 1);
			    if (bytes <= 0)
			      {
				 fprintf(stderr, "%s: Error. unable to write image part \"%s\" as \"%s\" part entry to %s \n",
					 progname, img->entry, buf, file_out);	
				 ABORT_WRITE(ef, file_out);
			      }
			    else
			      {
				 image_num++;
				 total_bytes += bytes;
			      }
			 }
		       else
			 {
			    fprintf(stderr, "%s: Error. unable to load image for image part \"%s\" as \"%s\" part entry to %s \n",
				    progname, img->entry, buf, file_out);	
			    ABORT_WRITE(ef, file_out);
			 }
		       if (verbose)
			 {
			    struct stat st;
		       
			    if (stat(imlib_image_get_filename(), &st) != 0)
			      st.st_size = 0;
			    input_bytes += st.st_size;
			    input_raw_bytes += im_w * im_h * 4;
			    printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" image entry \"%s\" compress: [raw: %2.1f%%] [real: %2.1f%%]\n",
				   progname, bytes, (bytes + 512) / 1024, buf, img->entry,
				   100 - (100 * (double)bytes) / ((double)(im_w * im_h * 4)),
				   100 - (100 * (double)bytes) / ((double)(st.st_size))
				   );
			 }
		       if (im_data) imlib_image_put_back_data(im_data);
		       imlib_free_image();
		    }
		  else
		    {
		       fprintf(stderr, "%s: Error. unable to load image for image \"%s\" part entry to %s \n",
			       progname, img->entry, file_out);	
		       ABORT_WRITE(ef, file_out);
		    }
	       }
	  }
     }
   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	Evas_List *ll;
	
	pc = l->data;
	for (ll = pc->parts; ll; ll = ll->next)
	  {
	     Edje_Part *ep;
	     Edje_Part_Description *epd;
	     
	     ep = ll->data;
	     epd = ep->default_desc;

	     if (!epd)
	       {
		  fprintf(stderr, "%s: Error. description missing for part \"%s\"\n",
			  progname, ep->name);
		  ABORT_WRITE(ef, file_out);
	       }
/*
	     if (epd->text.font)
	       {
		  Evas_List *lll;
		  
		  for (lll = fonts; lll; lll = lll->next)
		    {
		       Font *fn;
		       
		       fn = lll->data;
		       if (!strcmp(fn->name, epd->text.font))
			 {
			    char *s;
			    
			    s = malloc(strlen(epd->text.font) + strlen("fonts/") + 1);
			    strcpy(s, "fonts/");
			    strcat(s, epd->text.font);
			    free(epd->text.font);
			    epd->text.font = s;
			 }
		    }
	       }
	     for (l3 = ep->other_desc; l3; l3 = l3->next)
	       {
		  epd = l3->data;
		  if (epd->text.font)
		    {
		       Evas_List *lll;
		       
		       for (lll = fonts; lll; lll = lll->next)
			 {
			    Font *fn;
			    
			    fn = lll->data;
			    if (!strcmp(fn->name, epd->text.font))
			      {
				 char *s;
				 
				 s = malloc(strlen(epd->text.font) + strlen("fonts/") + 1);
				 strcpy(s, "fonts/");
				 strcat(s, epd->text.font);
				 free(epd->text.font);
				 epd->text.font = s;
			      }
			 }
		    }
	       }
 */
	  }
     }
   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	char buf[4096];
	
	pc = l->data;
	snprintf(buf, sizeof(buf), "collections/%i", pc->id);
	bytes = eet_data_write(ef, edd_edje_part_collection, buf, pc, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"%s\" part entry to %s \n",
		     progname, buf, file_out);	
	     ABORT_WRITE(ef, file_out);
	  }
	else
	  {
	     collection_num++;
	     total_bytes += bytes;
	  }
	if (verbose)
	  {
	     printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" collection entry\n",
		    progname, bytes, (bytes + 512) / 1024, buf);
	  }
     }
   for (i = 0, l = codes; l; l = l->next, i++)
     {
	Code *cd;
	int ln = 0;
	
	cd = l->data;
	if ((cd->shared) || (cd->programs))
	  {
	     int fd;
	     char tmpn[4096];
	     
	     strcpy(tmpn, "/tmp/edje_cc.sma-tmp-XXXXXX");
	     fd = mkstemp(tmpn);
	     if (fd >= 0)
	       {
		  FILE *f;
		  char buf[4096];
		  char tmpo[4096];
		  int ret;
		  
		  f = fopen(tmpn, "w");
		  if (f)
		    {
		       Evas_List *ll;
		       
		       fprintf(f, "#include <edje>\n");
		       ln = 2;
		       if (cd->shared)
			 {
			    while (ln < (cd->l1 - 1))
			      {
				 fprintf(f, " \n");
				 ln++;
			      }
			      {
				 char *sp;
				 int hash = 0;
				 int newlined = 0;
				 
				 for (sp = cd->shared; *sp; sp++)
				   {
				      if ((sp[0] == '#') && (newlined))
					{
					   hash = 1;
					}
				      newlined = 0;
				      if (sp[0] == '\n') newlined = 1;
				      if (!hash) fputc(sp[0], f);
				      else if (sp[0] == '\n') hash = 0;
				   }
				 fputc('\n', f);
			      }
			    ln += cd->l2 - cd->l1 + 1;
			 }
		       for (ll = cd->programs; ll; ll = ll->next)
			 {
			    Code_Program *cp;
			    
			    cp = ll->data;
			    if (cp->script)
			      {
				 while (ln < (cp->l1 - 1))
				   {
				      fprintf(f, " \n");
				      ln++;
				   }
				 /* FIXME: this prototype needs to be */
				 /* formalised and set in stone */
				 fprintf(f, "public _p%i(sig[], src[]) {", cp->id);
				   {
				      char *sp;
				      int hash = 0;
				      int newlined = 0;
				      
				      for (sp = cp->script; *sp; sp++)
					{
					   if ((sp[0] == '#') && (newlined))
					     {
						hash = 1;
					     }
					   newlined = 0;
					   if (sp[0] == '\n') newlined = 1;
					   if (!hash) fputc(sp[0], f);
					   else if (sp[0] == '\n') hash = 0;
					}
				   }
				 fprintf(f, "}");
				 ln += cp->l2 - cp->l1 + 1;
			      }
			 }
		       fclose(f);
		    }
		  close(fd);
		  strcpy(tmpo, "/tmp/edje_cc.amx-tmp-XXXXXX");
		  fd = mkstemp(tmpo);
		  if (fd >= 0)
		    {
		       snprintf(buf, sizeof(buf), "embryo_cc -i %s -o %s %s", 
				DAT"data/include", tmpo, tmpn);
		       ret = system(buf);
		       if (ret != 0)
			 {
			    fprintf(stderr, "%s: Warning. Compiling script code not clean.\n",
				    progname);	
			    ABORT_WRITE(ef, file_out);
			 }
		       close(fd);
		    }
		  f = fopen(tmpo, "r");
		  if (f)
		    {
		       int size;
		       void *data;
		       
		       fseek(f, 0, SEEK_END);
		       size = ftell(f);
		       rewind(f);
		       if (size > 0)
			 {
			    int bt;
			    
			    data = malloc(size);
			    if (data)
			      {
				 fread(data, size, 1, f);
				 snprintf(buf, sizeof(buf), "scripts/%i", i);
				 bt = eet_write(ef, buf, data, size, 1);
				 free(data);
			      }
			 }
		       fclose(f);
		    }
		  unlink(tmpn);
		  unlink(tmpo);
	       }
	  }
     }
   src_bytes = source_append(ef);
   total_bytes += src_bytes;
   fmap_bytes = source_fontmap_save(ef, fonts);
   total_bytes += fmap_bytes;
   eet_close(ef);
   if (verbose)
     {
	struct stat st;
	
	if (stat(file_in, &st) != 0)
	  st.st_size = 0;
	input_bytes += st.st_size;
	input_raw_bytes += st.st_size;
	printf("Summary:\n"
	       "  Wrote %i collections\n"
	       "  Wrote %i images\n"
	       "  Wrote %i fonts\n"
	       "  Wrote %i bytes (%iKb) of original source data\n"
	       "  Wrote %i bytes (%iKb) of original source font map\n"
	       "Conservative compression summary:\n"
	       "  Wrote total %i bytes (%iKb) from %i (%iKb) input data\n"
	       "  Output file is %3.1f%% the size of the input data\n"
	       "  Saved %i bytes (%iKb)\n"
	       "Raw compression summary:\n"
	       "  Wrote total %i bytes (%iKb) from %i (%iKb) raw input data\n"
	       "  Output file is %3.1f%% the size of the raw input data\n"
	       "  Saved %i bytes (%iKb)\n"
	       ,
	       collection_num,
	       image_num,
	       font_num,
	       src_bytes, (src_bytes + 512) / 1024,
	       fmap_bytes, (fmap_bytes + 512) / 1024,
	       total_bytes, (total_bytes + 512) / 1024,
	       input_bytes, (input_bytes + 512) / 1024,
	       (100.0 * (double)total_bytes) / (double)input_bytes,
	       input_bytes - total_bytes,
	       (input_bytes - total_bytes + 512) / 1024,
	       total_bytes, (total_bytes + 512) / 1024,
	       input_raw_bytes, (input_raw_bytes + 512) / 1024,
	       (100.0 * (double)total_bytes) / (double)input_raw_bytes,
	       input_raw_bytes - total_bytes,
	       (input_raw_bytes - total_bytes + 512) / 1024);
     }
}

void
data_queue_part_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Part_Lookup *pl;
   
   pl = mem_alloc(SZ(Part_Lookup));
   part_lookups = evas_list_append(part_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_program_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   Program_Lookup *pl;
   
   pl = mem_alloc(SZ(Program_Lookup));
   program_lookups = evas_list_append(program_lookups, pl);
   pl->pc = pc;
   pl->name = mem_strdup(name);
   pl->dest = dest;
}

void
data_queue_image_lookup(char *name, int *dest)
{
   Image_Lookup *il;
   
   il = mem_alloc(SZ(Image_Lookup));
   image_lookups = evas_list_append(image_lookups, il);
   il->name = mem_strdup(name);
   il->dest = dest;
}

void
data_queue_part_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   part_slave_lookups = evas_list_append(part_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
data_queue_image_slave_lookup(int *master, int *slave)
{
   Slave_Lookup *sl;

   sl = mem_alloc(SZ(Slave_Lookup));
   image_slave_lookups = evas_list_append(image_slave_lookups, sl);
   sl->master = master;
   sl->slave = slave;
}

void
handle_slave_lookup(Evas_List *list, int *master, int value)
{
   Evas_List *l;

   for (l = list; l; l = l->next)
   {
      Slave_Lookup *sl = l->data;

      if (sl->master == master)
	 *sl->slave = value;
   }
}

void
data_process_lookups(void)
{
   Evas_List *l;
   
   while (part_lookups)
     {
	Part_Lookup *pl;
	
	pl = part_lookups->data;
	
	for (l = pl->pc->parts; l; l = l->next)
	  {
	     Edje_Part *ep;
	     
	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  handle_slave_lookup(part_slave_lookups, pl->dest, ep->id);
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find part name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	part_lookups = evas_list_remove(part_lookups, pl);
	free(pl->name);
	free(pl);
     }

   while (program_lookups)
     {
	Program_Lookup *pl;
	
	pl = program_lookups->data;
	
	for (l = pl->pc->programs; l; l = l->next)
	  {
	     Edje_Program *ep;
	     
	     ep = l->data;
	     if ((ep->name) && (!strcmp(ep->name, pl->name)))
	       {
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find program name %s\n",
		     progname, pl->name);
	     exit(-1);
	  }
	program_lookups = evas_list_remove(program_lookups, pl);
	free(pl->name);
	free(pl);
     }
   
   while (image_lookups)
     {
	Image_Lookup *il;
	
	il = image_lookups->data;
	
	if (!edje_file->image_dir)
	  l = NULL;
	else
	  {
	     for (l = edje_file->image_dir->entries; l; l = l->next)
	       {
		  Edje_Image_Directory_Entry *de;
		  
		  de = l->data;
		  if ((de->entry) && (!strcmp(de->entry, il->name)))
		    {
		       handle_slave_lookup(image_slave_lookups, il->dest, de->id);
		       *(il->dest) = de->id;
		       break;
		    }
	       }
	  }
	
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable to find image name %s\n",
		     progname, il->name);
	     exit(-1);
	  }
	image_lookups = evas_list_remove(image_lookups, il);
	free(il->name);
	free(il);
     }

   while (part_slave_lookups)
     {
	free(part_slave_lookups->data);
	part_slave_lookups = evas_list_remove_list(part_slave_lookups, part_slave_lookups);
     }

   while (image_slave_lookups)
     {
	free(image_slave_lookups->data);
	image_slave_lookups = evas_list_remove_list(image_slave_lookups, image_slave_lookups);
     }
}

static void
data_process_string(Edje_Part_Collection *pc, char *prefix, char *s, void (*func)(Edje_Part_Collection *pc, char *name, int *val))
{
   char *p;
   char *key;
   int keyl;
   int quote, escape;
   
   key = alloca(strlen(prefix) + 2 + 1);
   if (!key) return;
   strcpy(key, prefix);
   strcat(key, ":\"");
   keyl = strlen(key);
   quote = 0;
   escape = 0;
   for (p = s; (p) && (*p); p++)
     {
	if (!quote)
	  {
	     if (*p == '\"')
	       {
		  quote = 1;
		  p++;
	       }
	  }
	if (!quote)
	  {
	     if (!strncmp(p, key, keyl))
	       {
		  Code_Lookup *cl;
		  
		  cl = mem_alloc(SZ(Code_Lookup));
		  if (cl)
		    {
		       int inesc = 0;
		       char *name;
		       
		       cl->ptr = p;
		       p += keyl;
		       while ((*p))
			 {
			    if (!inesc)
			      {
				 if (*p == '\\') inesc = 1;
				 else if (*p == '\"')
				 {
				    /* string concatenation, see below */
				    if (*(p + 1) != '\"')
				       break;
				    else
				       p++;
				 }
			      }
			    else
			      inesc = 0;
			    p++;
			 }
		       cl->len = p - cl->ptr + 1;
		       name = alloca(cl->len);
		       if (name)
			 {
			    char *pp;
			    int i;
			    
			    name[0] = 0;
			    pp = cl->ptr + keyl;
			    inesc = 0;
			    i = 0;
			    while (*pp)
			      {
				 if (!inesc)
				   {
				      if (*pp == '\\') inesc = 1;
				      else if (*pp == '\"')
					{
					   /* concat strings like "foo""bar" to "foobar" */
					   if (*(pp + 1) == '\"')
					      pp++;
					   else
					   {
					      name[i] = 0;
					      break;
					   }
					}
				      else
					{
					   name[i] = *pp;
					   name[i + 1] = 0;
					   i++;
					}
				   }
				 else
				   inesc = 0;
				 pp++;
			      }
			    func(pc, name, &(cl->val));
			 }
		       code_lookups = evas_list_append(code_lookups, cl);
		    }
		  else break;
	       }
	  }
	else
	  {
	     if (!escape)
	       {
		  if (*p == '\"') quote = 0;
		  else if (*p == '\\') escape = 1;
	       }
	     else if (escape)
	       {
		  escape = 0;
	       }
	  }
     }
}

static void
data_queue_image_pc_lookup(Edje_Part_Collection *pc, char *name, int *dest)
{
   data_queue_image_lookup(name, dest);
}

void
data_process_scripts(void)
{
   Evas_List *l, *l2;
   
   for (l = codes, l2 = edje_collections; (l) && (l2); l = l->next, l2 = l2->next)
     {
	Code *cd;
	Edje_Part_Collection *pc;
	
	cd = l->data;
	pc = l2->data;
	if ((cd->shared) || (cd->programs))
	  {
	     Evas_List *ll;
	     
	     if (cd->shared)
	       {
		  data_process_string(pc, "PART",    cd->shared, data_queue_part_lookup);
		  data_process_string(pc, "PROGRAM", cd->shared, data_queue_program_lookup);
		  data_process_string(pc, "IMAGE",   cd->shared, data_queue_image_pc_lookup);
	       }
	     for (ll = cd->programs; ll; ll = ll->next)
	       {
		  Code_Program *cp;
		  
		  cp = ll->data;
		  if (cp->script)
		    {
		       data_process_string(pc, "PART",    cp->script, data_queue_part_lookup);
		       data_process_string(pc, "PROGRAM", cp->script, data_queue_program_lookup);
		       data_process_string(pc, "IMAGE",   cp->script, data_queue_image_pc_lookup);
		    }
	       }
	  }
     }
}

void
data_process_script_lookups(void)
{
   Evas_List *l;
   
   for (l = code_lookups; l; l = l->next)
     {
	Code_Lookup *cl;
	char buf[256];
	int i, n;

	cl = l->data;
	snprintf(buf, sizeof(buf), "%i", cl->val);
	n = strlen(buf);
	if (n > cl->len)
	  {
	     fprintf(stderr, "%s: Error. The unexpected happened. A numeric replacement string was larger than the original!\n",
		     progname);
	     exit(-1);
	  }
	for (i = 0; i < cl->len; i++) cl->ptr[i] = ' ';
	strncpy(cl->ptr, buf, n);
     }
}
