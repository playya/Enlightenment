/*

Copyright (C) 2000, 2001 Christian Kreibich <kreibich@aciri.org>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef __efsd_misc_h
#define __efsd_misc_h

#define NEW(X)  ((X*) malloc(sizeof(X)))
#define FREE(X) { if (X) { free(X); X = NULL; } }


int    efsd_misc_file_exists(char *filename);
int    efsd_misc_file_is_dir(char *filename);
int    efsd_misc_mkdir(char *filename);
void   efsd_misc_remove_trailing_slashes(char *path);
int    efsd_misc_is_absolute_path(char *path);
char **efsd_misc_get_path_dirs(char *path, int *num_dirs);

/* Could somebody tell me if memdup() is portable? */
void  *efsd_misc_memdup(void *data, int size);

#ifdef __EMX__  
void   efsd_slashify(char *path);
#endif

#endif
