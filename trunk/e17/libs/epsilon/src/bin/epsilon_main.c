#include<stdlib.h>
#include<stdio.h>
#include"Epsilon.h"
#include"../config.h"

int
main (int argc, char *argv[])
{
  Epsilon *e = NULL;

  epsilon_init ();
  while (--argc)
    {
      if ((e = epsilon_new ((const char *) argv[argc])))
	{
	  if (epsilon_exists (e) == EPSILON_FAIL)
	    {
	      fprintf (stderr,
		       "Thumbnail for %s needs to be generated: ",
		       argv[argc]);
	      if (epsilon_generate (e) == EPSILON_OK)
		fprintf (stderr, "OK\n");
	      else
		fprintf (stderr, "FAILED\n");
	    }
	  else
	    {
	      Epsilon_Info *info;
	      Epsilon_Exif_Info *eei;
	      fprintf (stderr, "\nThumbnail already exists\n%s\n",
		       epsilon_thumb_file_get (e));
	      fprintf (stderr, "Thumbnail already exists\n");
	      if ((info = epsilon_info_get (e)))
		{
		  if (info->uri)
		    printf ("URI: %s\n", info->uri);
		  if (info->mimetype)
		    printf ("MimeType: %s\n", info->mimetype);
		  printf ("Source Image Width: %d\n", info->w);
		  printf ("Source Image Height: %d\n", info->h);
		  printf ("Source Image Mtime: %d\n", (int) info->mtime);
		  fprintf (stderr, "Trying EXIF Info: ");
		  if (epsilon_info_has_exif_get (info))
		    {
			fprintf (stderr, "Found!\n");
#if 0
		  fprintf (stderr, "%d is direction\n",
			   epsilon_exif_info_props_as_int_get (eei, 0x0112));
#endif
			epsilon_info_exif_props_print (info);
		    }
		    else {
			fprintf (stderr, "Not Found!\n");
		    }
		  epsilon_info_free (info);
		}
	      else
		{
		  fprintf (stderr, "Meta Info Not Found!\n");
		}
	    }
	  epsilon_free (e);
	}
    }
  return (0);
}
