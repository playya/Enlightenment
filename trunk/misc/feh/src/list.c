/* list.c
 *
 * Copyright (C) 2000 Tom Gilbert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "feh.h"
#include "filelist.h"
#include "options.h"

void
init_list_mode(void)
{
   feh_file *file;
   int j = 0;

   D_ENTER;

   if (!opt.customlist)
      printf
         ("NUM\tFORMAT\tWIDTH\tHEIGHT\tPIXELS\tSIZE(bytes)\tALPHA\tFILENAME\n");

   for (file = filelist; file; file = file->next)
   {
      if (opt.customlist)
         printf("%s\n", feh_printf(opt.customlist, file));
      else
         printf("%d\t%s\t%d\t%d\t%d\t%d\t\t%c\t%s\n", ++j, file->info->format,
                file->info->width, file->info->height, file->info->pixels,
                file->info->size, file->info->has_alpha ? 'X' : '-',
                file->filename);
      if (opt.action)
         feh_action_run(file);
   }
   exit(0);
}

void
init_loadables_mode(void)
{
   D_ENTER;
   real_loadables_mode(1);
   D_RETURN_;
}

void
init_unloadables_mode(void)
{
   D_ENTER;
   real_loadables_mode(0);
   D_RETURN_;
}


void
real_loadables_mode(int loadable)
{
   feh_file *file;

   D_ENTER;
   opt.quiet = 1;

   for (file = filelist; file; file = file->next)
   {
      Imlib_Image im = NULL;

      if (feh_load_image(&im, file))
      {
         /* loaded ok */
         if (loadable)
            fprintf(stdout, "%s\n", file->filename);
         feh_imlib_free_image_and_decache(im);
      }
      else
      {
         /* Oh dear. */
         if (!loadable)
            fprintf(stdout, "%s\n", file->filename);
      }
   }
   exit(0);
}
