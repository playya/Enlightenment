#include "edje_cc.h"

static void main_help(void);

Evas_List *img_dirs = NULL;
Evas_List *fnt_dirs = NULL;
char      *file_in = NULL;
char      *file_out = NULL;
char      *progname = NULL;
int        verbose = 0;

int        no_lossy = 0;
int        no_comp = 0;
int        no_raw = 0;
int        min_quality = 0;
int        max_quality = 100;

static void
main_help(void)
{
   printf
     ("Usage:\n"
      "\t%s [OPTIONS] input_file.edc [output_file.eet]\n"
      "\n"
      "Where OPTIONS is one or more of:\n"
      "\n"
      "-id image/directory      Add a directory to look in for relative path images\n"
      "-fd font/directory       Add a directory to look in for relative path fonts\n"
      "-v                       Verbose output\n"
      "-no-lossy                Do NOT allow images to be lossy\n"
      "-no-comp                 Do NOT allow images to be lossless compression\n"
      "-no-raw                  Do NOT allow images to be zero compression\n"
      "-min-quality VAL         Do NOT allow lossy images with quality < VAL (0-100)\n"
      "-max-quality VAL         Do NOT allow lossy images with quality > VAL (0-100)\n"
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
	if (!strcmp(argv[i], "-h"))
	  {
	     main_help();
	     exit(0);
	  }
	else if (!strcmp(argv[i], "-v"))
	  {
	     verbose = 1;
	  }
	else if (!strcmp(argv[i], "-no-lossy"))
	  {
	     no_lossy = 1;
	  }
	else if (!strcmp(argv[i], "-no-comp"))
	  {
	     no_comp = 1;
	  }
	else if (!strcmp(argv[i], "-no-raw"))
	  {
	     no_raw = 1;
	  }
	else if ((!strcmp(argv[i], "-id")) && (i < (argc - 1)))
	  {
	     i++;	     
	     img_dirs = evas_list_append(img_dirs, argv[i]);
	  }
	else if ((!strcmp(argv[i], "-fd")) && (i < (argc - 1)))
	  {
	     i++;	     
	     fnt_dirs = evas_list_append(fnt_dirs, argv[i]);
	  }
	else if ((!strcmp(argv[i], "-min-quality")) && (i < (argc - 1)))
	  {
	     i++;	     
	     min_quality = atoi(argv[i]);
	     if (min_quality < 0) min_quality = 0;
	     if (min_quality > 100) min_quality = 100;
	  }
	else if ((!strcmp(argv[i], "-max-quality")) && (i < (argc - 1)))
	  {
	     i++;	     
	     max_quality = atoi(argv[i]);
	     if (max_quality < 0) max_quality = 0;
	     if (max_quality > 100) max_quality = 100;
	  }
	else if (!file_in)
	  file_in = argv[i];
	else if (!file_out)
	  file_out = argv[i];
     }
   if (!file_in)
     {
	fprintf(stderr, "%s: Error: no input file specified.\n", progname);
	main_help();
	exit(-1);
     }
   if (!file_out)
      {
         char *suffix;
      
         if ((suffix = strstr(file_in,".edc")) && (suffix[4] == 0))
            {
               file_out = strdup(file_in);
               if (file_out)
                  {
                     suffix = strstr(file_out,".edc");
                     strcpy(suffix,".eet");
                  }
            }
      }
   if (!file_out)
     {
	fprintf(stderr, "%s: Error: no output file specified.\n", progname);
	main_help();
	exit(-1);
     }

   edje_init();

   edje_file = mem_alloc(SZ(Edje_File));
   edje_file->version = EDJE_FILE_VERSION;
   
   source_edd();
   source_fetch();
   
   data_setup();   
   compile();
   data_process_scripts();
   data_process_lookups();
   data_process_script_lookups();
   data_write();

   edje_shutdown();
   
   return 0;
}
