/*****************************************************************************/
/* Enlightenment - The Window Manager that dares to do what others don't     */
/*****************************************************************************/
/*
  Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies of the Software, its documentation and marketing & publicity
  materials, and acknowledgment shall be given in the documentation, materials
  and software packages that this Software was used.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  */
#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/Xlocale.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <Imlib2.h>

#define EAllocColor(pxc) \
	XAllocColor(disp, VRoot.cmap, pxc)

#define FILEPATH_LEN_MAX 4096

#ifndef MAX
#define MAX(a,b)  ((a)>(b)?(a):(b))
#endif

#if defined(HAVE_WCTYPE_H) && defined(HAVE_WCHAR_T) && !defined(__FreeBSD__)
#define USE_WORD_MB
#endif

typedef struct _efont Efont;

typedef struct _root
{
   Window              win;
   Visual             *vis;
   unsigned int        depth;
   Colormap            cmap;
   int                 scr;
   unsigned int        w, h;
} Root;

typedef struct _textstate
{
   char               *fontname;
   XColor              fg_col;
   XColor              bg_col;
   int                 effect;
   Efont              *efont;
   XFontStruct        *xfont;
   XFontSet            xfontset;
   int                 xfontset_ascent;
   int                 height;
} TextState;

typedef struct _link
{
   char               *name;
   int                 x, y, w, h;
   struct _link       *next;
} Link;

/* dox.c */
Imlib_Image         ImageLoadDoc(const char *file);
void                ESetColor(XColor * pxc, int r, int g, int b);
void                EGetColor(XColor * pxc, int *pr, int *pg, int *pb);

/* ttfont.c */
void                Efont_extents(Efont * f, const char *text,
				  int *font_ascent_return,
				  int *font_descent_return, int *width_return,
				  int *max_ascent_return,
				  int *max_descent_return,
				  int *lbearing_return, int *rbearing_return);
Efont              *Efont_load(const char *file, int size);
void                Efont_free(Efont * f);
void                EFont_draw_string(Display * disp, Drawable win, GC gc,
				      int x, int y, const char *text,
				      Efont * font, Visual * vis, Colormap cm);

/* text.c */
void                TextStateLoadFont(TextState * ts);
void                TextSize(TextState * ts, const char *text,
			     int *width, int *height, int fsize);
void                TextDraw(TextState * ts, Window win, char *text,
			     int x, int y, int w, int h, int fsize,
			     int justification);

/* file.c */
void                freestrlist(char **l, int num);
void                word(char *s, int num, char *wd);

#ifdef USE_WORD_MB
void                word_mb(char *s, int num, char *wd, int *spaceflag);
#endif
int                 findLocalizedFile(char *fname);

/* format.c */
int                 GetObjects(FILE * f);
int                 FixPage(int p);
int                 GetPage(char *name);
void                GetLinkColors(int page_num, int *r, int *g, int *b);
Link               *RenderPage(Window win, int page_num, int w, int h);

extern Display     *disp;
extern Root         VRoot;
extern char        *docdir;

#define Efree       free
#define Emalloc     malloc
#define Erealloc    realloc

#define EMALLOC(type, num) (type*)Emalloc((num)*sizeof(type))
#define EREALLOC(type, ptr, num) (type*)Erealloc(ptr, (num)*sizeof(type))

#define Esnprintf snprintf
