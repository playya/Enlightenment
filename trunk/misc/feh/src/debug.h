/* debug.h
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

#ifndef DEBUG_H
#define DEBUG_H

/* #define DEBUG */

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#define emalloc(a) malloc(a)
#define estrdup(a) strdup(a)
#define erealloc(a,b) realloc(a,b)
#else
#define emalloc(a) _emalloc(a)
#define estrdup(a) _estrdup(a)
#define erealloc(a,b) _erealloc(a,b)
#endif

#ifdef DEBUG
#ifdef __GNUC__
#define D(a) \
  { \
      printf("%s +%u %s() %s ",__FILE__,__LINE__,__FUNCTION__, stroflen(' ', call_level)); \
      printf a; \
      fflush(stdout); \
  }
#define D_ENTER \
  { \
      call_level++; \
      printf("%s +%u %s() %s ENTER\n",__FILE__,__LINE__,__FUNCTION__, stroflen('>', call_level)); \
      fflush(stdout); \
  }
#define D_RETURN(a) \
  { \
      printf("%s +%u %s() %s LEAVE\n",__FILE__,__LINE__,__FUNCTION__, stroflen('<', call_level)); \
      fflush(stdout); \
      call_level--; \
      return (a); \
  }
#define D_RETURN_ \
  { \
      printf("%s +%u %s() %s LEAVE\n",__FILE__,__LINE__,__FUNCTION__, stroflen('<', call_level)); \
      fflush(stdout); \
      call_level--; \
      return; \
  }
#else
#define D(a) \
  { \
      printf("%s +%u : ",__FILE__,__LINE__); \
      printf a; \
      fflush(stdout); \
  }
#define D_ENTER
#define D_RETURN(a) \
  { \
      return(a); \
  }
#define D_RETURN_ \
  { \
      return; \
  }
#endif
#else
#define D(a)
#define D_ENTER
#define D_RETURN(a) \
  { \
      return (a); \
  }
#define D_RETURN_ \
  { \
      return; \
  }
#endif

#endif
