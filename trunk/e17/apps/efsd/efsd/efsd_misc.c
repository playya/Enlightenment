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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef __EMX__
#include <stdlib.h>
#include <malloc.h>
#define mkdir _mkdir2
#endif

#include <efsd_misc.h>


int    
efsd_misc_file_exists(char *filename)
{
  struct stat st;

  if (!filename)
    return (0);

  if (stat(filename, &st) < 0)
    return (0);

  return (1);
}


int 
efsd_misc_file_is_dir(char *filename)
{
  struct stat st;

  if (!filename)
    return (0);

  if (stat(filename, &st) < 0)
    return (0);

  return (S_ISDIR(st.st_mode));
}


void 
efsd_misc_remove_trailing_slashes(char *s)
{
  int len;

  if (!s || s[0] == '\0')
    return;

  len = strlen(s);

  while (len > 0 && s[len-1] == '/')
    {
      s[len-1] = '\0';
      len--;
    }
}


int  
efsd_misc_is_absolute_path(char *s)
{
  if (!s || s[0] == '\0')
    return (0);

#ifndef __EMX__    
  if (s[0] == '/')
#else  
  if (_fnisabs(s))
#endif  
    return (1);

  return (0);
}


char **
efsd_misc_get_path_dirs(char *s, int *num_dirs)
{
  int     num = 1;
  char   *p, *q, old;
  char  **result;
#ifdef __EMX__  
  int    drive_present = 0;
#endif
  
  if (!s || s[0] == '\0')
    {
      *num_dirs = 0;
      return NULL;
    }

  efsd_misc_remove_trailing_slashes(s);
#ifdef __EMX__  
  efsd_slashify(s);	
#endif  
  while ((*s) == '/') s++;

  p = s;
  while ( (p = strchr(p, '/')) != NULL)
    {
      while ((*p) == '/') p++;
      num++;
    }

#ifdef __EMX__  
   q = strchr(s, ':');
   /* if colon is chars 0, 1, or 2 it is a drive letter for os/2 */
   if ((q) && (q - s) < 3) 
     { 
      num--; 
      drive_present = 1;
     } 
#endif
    
  *num_dirs = num;
  result = (char**) malloc(sizeof(char*) * num);

  p = q = s;
  num = 0;
#ifdef __EMX__  
  if (drive_present) 
    { p = strchr(p, ':');    
      if (p) 
        if (*(p+1) == '/') p += 2;
    }  
#endif      
  while ( (p = strchr(p, '/')) != NULL)
    {
      old = *p;
      *p = '\0';
      result[num] = strdup(q);
      *p = old;

      num++;
      while ((*p) == '/') p++;
      q = p;
    }

  result[num] = strdup(q);

  return (result);
}

void efsd_slashify(char *s)
{
 int	 	i;
 
 if ( s == NULL )
     return;
 
 for (i = 0; i<strlen(s); i++ ) 
     {
      if ( s[i] ==  '\\' )
	   s[i] = '/';
     }
}
