/* list.c
 *
 * Copyright (C) 1999 Tom Gilbert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "feh.h"

void
init_list_mode (void)
{
  feh_file *file;
  int j = 0;

  D (("In init_list_mode\n"));

  if(!filelist_length(filelist))
	show_mini_usage();
  
  if (opt.longlist)
    printf
      ("NUM\tFORMAT\tWIDTH\tHEIGHT\tPIXELS\tSIZE(bytes)\tALPHA\tFILENAME\n");
  else
    printf ("NUM\tFORMAT\tWIDTH\tHEIGHT\tSIZE(bytes)\tALPHA\tNAME\n");

  for (file = filelist; file; file = file->next)
    {
      if (opt.longlist)
	printf ("%d\t%s\t%d\t%d\t%d\t%d\t\t%c\t%s\n", ++j, file->info->format,
		file->info->width, file->info->height, file->info->pixels,
		file->info->size, file->info->has_alpha ? 'X' : '-',
		file->filename);
      else
	printf ("%d\t%s\t%d\t%d\t%d\t\t%c\t%s\n", ++j, file->info->format,
		file->info->width, file->info->height, file->info->size,
		file->info->has_alpha ? 'X' : '-', file->name);
    }

  exit (0);
}

void
init_loadables_mode (void)
{
  D (("In init_loadables_mode\n"));

  real_loadables_mode (1);
}

void
init_unloadables_mode (void)
{
  D (("In init_unloadables_mode\n"));

  real_loadables_mode (0);
}


void
real_loadables_mode (int loadable)
{
  feh_file *file;

  if(!filelist_length(filelist))
	show_mini_usage();
  
  opt.quiet = 1;

  for (file = filelist; file; file = file->next)
    {
      Imlib_Image *im = NULL;

      if (feh_load_image (&im, file))
	{
	  /* loaded ok */
	  if (loadable)
	    fprintf (stdout, "%s\n", file->filename);
	  imlib_context_set_image (im);
	  imlib_free_image_and_decache ();
	}
      else
	{
	  /* Oh dear. */
	  if (!loadable)
	    fprintf (stdout, "%s\n", file->filename);
	}
    }
  exit (0);
}
