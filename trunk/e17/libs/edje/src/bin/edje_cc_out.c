#include "edje_cc.h"

typedef struct _Part_Lookup Part_Lookup;
typedef struct _Program_Lookup Program_Lookup;
typedef struct _Image_Lookup Image_Lookup;

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

Edje_File *edje_file = NULL;
Evas_List *edje_collections = NULL;

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
   
   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
	fprintf(stderr, "%s: Error. unable to open \"%s\" for writing output\n",
		progname, file_out);
	exit(-1);
     }
   bytes = eet_data_write(ef, edd_edje_file, "edje_file", edje_file, 1);
   if (bytes <= 0)
     {
	fprintf(stderr, "%s: Error. unable to write \"edje_file\" entry to \"%s\" \n",
		progname, file_out);	
	exit(-1);	
     }
   if (verbose)
     {
	printf("%s: Wrote %9i bytes (%4iKb) for \"edje_file\" header\n",
	       progname, bytes, (bytes + 512) / 1024);
     }
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
		  
		  snprintf(buf, sizeof(buf), "%s/%s", l->data, img->entry);
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
		  im_alpha = imlib_image_has_alpha();
		  im_data = imlib_image_get_data_for_reading_only();
		  snprintf(buf, sizeof(buf), "images/%i", img->id);
		  if (img->source_type == EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT)
		    bytes = eet_data_image_write(ef, buf, 
						 im_data, im_w, im_h,
						 im_alpha, 
						 img->source_param, 0, 0);
		  else
		    bytes = eet_data_image_write(ef, buf, 
						 im_data, im_w, im_h,
						 im_alpha,
						 0, img->source_param, 1);
		  if (bytes <= 0)
		    {
		       fprintf(stderr, "%s: Error. unable to write image part \"%s\" as \"%s\" part entry to %s \n",
			       progname, img->entry, buf, file_out);	
		       exit(-1);
		    }
		  if (verbose)
		    {
		       printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" image entry \"%s\"\n",
			      progname, bytes, (bytes + 512) / 1024, buf, img->entry);
		    }
		  imlib_image_put_back_data(im_data);
		  imlib_free_image();
	       }
	     else
	       {
		  fprintf(stderr, "%s: Warning. unable to open image \"%s\" for inclusion in output\n",
			  progname, img->entry);			  
	       }
	  }
     }
   for (l = edje_collections; l; l = l->next)
     {
	Edje_Part_Collection *pc;
	char buf[456];
	
	pc = l->data;
	
	snprintf(buf, sizeof(buf), "collections/%i", pc->id);
	bytes = eet_data_write(ef, edd_edje_part_collection, buf, pc, 1);
	if (bytes <= 0)
	  {
	     fprintf(stderr, "%s: Error. unable to write \"%s\" part entry to %s \n",
		     progname, buf, file_out);	
	     exit(-1);
	  }
	if (verbose)
	  {
	     printf("%s: Wrote %9i bytes (%4iKb) for \"%s\" collection entry\n",
		    progname, bytes, (bytes + 512) / 1024, buf);
	  }
     }
   eet_close(ef);
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
		  *(pl->dest) = ep->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable find part name %s\n",
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
	     fprintf(stderr, "%s: Error. unable find program name %s\n",
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
	
	for (l = edje_file->image_dir->entries; l; l = l->next)
	  {
	     Edje_Image_Directory_Entry *de;
	     
	     de = l->data;
	     if ((de->entry) && (!strcmp(de->entry, il->name)))
	       {
		  *(il->dest) = de->id;
		  break;
	       }
	  }
	if (!l)
	  {
	     fprintf(stderr, "%s: Error. unable find image name %s\n",
		     progname, il->name);
	     exit(-1);
	  }
	image_lookups = evas_list_remove(image_lookups, il);
	free(il->name);
	free(il);
     }
}
