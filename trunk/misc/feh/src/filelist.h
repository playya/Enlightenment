/* filelist.h
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

#ifndef FILELIST_H
#define FILELIST_H

struct __feh_file
{
   char *filename;
   char *name;

   /* info stuff */
   feh_file_info *info;
};

struct __feh_file_info
{
   int width;
   int height;
   int size;
   int pixels;
   unsigned char has_alpha;
   char *format;
   char *extension;
};

#define FEH_FILE(l) ((feh_file *) l)

enum filelist_recurse
{ FILELIST_FIRST, FILELIST_CONTINUE, FILELIST_LAST };

enum sort_type
{ SORT_NONE, SORT_NAME, SORT_FILENAME, SORT_WIDTH, SORT_HEIGHT, SORT_PIXELS,
   SORT_SIZE, SORT_FORMAT
};

feh_file *feh_file_new(char *filename);
void feh_file_free(feh_file * file);
feh_file_info *feh_file_info_new(void);
void feh_file_info_free(feh_file_info * info);
feh_list *feh_file_rm_and_free(feh_list * list, feh_list * file);
void add_file_to_filelist_recursively(char *origpath, unsigned char level);
void add_file_to_rm_filelist(char *file);
void delete_rm_files(void);
feh_list *feh_file_info_preload(feh_list * list);
int feh_file_info_load(feh_file * file, Imlib_Image im);
void feh_prepare_filelist(void);
int feh_write_filelist(feh_list * list, char *filename);
feh_list *feh_read_filelist(char *filename);
char *feh_absolute_path(char *path);
feh_list *feh_file_remove_from_list(feh_list * list, feh_list * l);

int feh_cmp_name(void *file1, void *file2);
int feh_cmp_filename(void *file1, void *file2);
int feh_cmp_width(void *file1, void *file2);
int feh_cmp_height(void *file1, void *file2);
int feh_cmp_pixels(void *file1, void *file2);
int feh_cmp_size(void *file1, void *file2);
int feh_cmp_format(void *file1, void *file2);

extern feh_list *filelist;
extern feh_list *current_file;

#endif
