/*****************************************************************************/
/* Enlightenment - The Window Manager that dares to do what others don't     */
/*****************************************************************************/
/* Copyright (C) 1997 - 1999 Carsten Haitzler (The Rasterman)                */
/*                                                                           */
/* This program and utilites is free software; you can redistribute it       */
/* and/or modify it under the terms of the GNU General Public License as     */
/* published by the Free Software Foundation; either version 2 of the        */
/* License, or (at your option) any later version.                           */
/*                                                                           */
/* This software is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         */
/* General Public License for more details.                                  */
/*                                                                           */
/* You should have received a copy of the GNU Library General Public         */
/* License along with this software; if not, write to the                    */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,              */
/* Boston, MA 02111-1307, USA.                                               */
/*****************************************************************************/

#include "econfig.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/Xlocale.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XShm.h>
#include <Imlib.h>
#include <Fnlib.h>
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
#include <freetype.h>

#define FILEPATH_LEN_MAX 4096

#define DEFAULT_LINKCOLOR_R 30
#define DEFAULT_LINKCOLOR_G 50
#define DEFAULT_LINKCOLOR_B 160

#define TT_VALID( handle )  ( ( handle ).z != NULL )
#ifndef MAX
#define MAX(a,b)  ((a)>(b)?(a):(b))
#endif

#define Esetenv(var, val, overwrite) \
{ \
  static char envvar[1024]; \
  sprintf(envvar, "%500s=%500s", var, val);\
  putenv(envvar);\
}

typedef struct _efont
  {
     TT_Engine           engine;
     TT_Face             face;
     TT_Instance         instance;
     TT_Face_Properties  properties;
     int                 num_glyph;
     TT_Glyph           *glyphs;
     TT_Raster_Map     **glyphs_cached;
     int                 max_descent;
     int                 max_ascent;
  }
Efont;

typedef struct _textstate
  {
     char               *fontname;
     FnlibStyle          style;
     FnlibFont          *font;
     ImlibColor          fg_col;
     ImlibColor          bg_col;
     int                 effect;
     Efont              *efont;
     XFontStruct        *xfont;
     XFontSet            xfontset;
     int                 xfontset_ascent;
     int                 height;
  }
TextState;

typedef enum _type
  {
     IMG,
     BR,
     FONT,
     P,
     TEXT,
     PAGE
  }
Type;

typedef struct _img
  {
     char               *src;
     char               *src2;
     char               *src3;
     int                 x, y;
     char               *link;
     int                 w, h;
  }
Img_;

typedef struct _font
  {
     char               *face;
     int                 r, g, b;
  }
Font_;

typedef struct _p
  {
     float               align;
  }
P_;

typedef struct _object
  {
     Type                type;
     void               *object;
  }
Object;

typedef struct _page
  {
     char               *name;
     int                 count;
     Object             *obj;
     int                 columns;
     int                 padding;
     int                 linkr, linkg, linkb;
     char               *background;
  }
Page;

typedef struct _link
  {
     char               *name;
     int                 x, y, w, h;
     struct _link       *next;
  }
Link;

void                Efont_extents(Efont * f, char *text,
				  int *font_ascent_return,
				  int *font_descent_return, int *width_return,
				  int *max_ascent_return,
				  int *max_descent_return,
				  int *lbearing_return, int *rbearing_return);
Efont              *Efont_load(char *file, int size);
void                Efont_free(Efont * f);
void                EFont_draw_string(Display * disp, Drawable win, GC gc,
				      int x, int y, char *text,
				      Efont * font, Visual * vis, Colormap cm);

char              **TextGetLines(char *text, int *count);
void                TextStateLoadFont(TextState * ts);
void                TextSize(TextState * ts, char *text,
			     int *width, int *height, int fsize);
void                TextDraw(TextState * ts, Window win, char *text,
			     int x, int y, int w, int h, int fsize,
			     int justification);

char               *FileExtension(char *file);
void                md(char *s);
int                 exists(char *s);
void                mkdirs(char *s);
int                 isfile(char *s);
int                 isdir(char *s);
char              **ls(char *dir, int *num);
void                freestrlist(char **l, int num);
void                rm(char *s);
void                mv(char *s, char *ss);
void                cp(char *s, char *ss);
time_t              moddate(char *s);
int                 filesize(char *s);
int                 fileinode(char *s);
int                 filedev(char *s);
void                cd(char *s);
char               *cwd(void);
int                 permissions(char *s);
int                 owner(char *s);
int                 group(char *s);
char               *username(int uid);
char               *homedir(int uid);
char               *usershell(int uid);
char               *atword(char *s, int num);
char               *atchar(char *s, char c);
char               *getword(char *s, int num);
void                word(char *s, int num, char *wd);
int                 canread(char *s);
int                 canwrite(char *s);
int                 canexec(char *s);
char               *fileof(char *s);
char               *fullfileof(char *s);
char               *pathtoexec(char *file);
char               *pathtofile(char *file);

void                AddPage(Object * obj);
void                AddObject(Object * obj);
void                BuildObj(Object * obj, char *var, char *param);
int                 GetNextTag(Object * obj);
char               *GetTextUntilTag(void);
int                 GetObjects(FILE * f);
int                 FixPage(int p);
int                 GetPage(char *name);
void                GetLinkColors(int page_num, int *r, int *g, int *b);
Link               *RenderPage(Window win, int page_num, int w, int h);

extern Display     *disp;
extern ImlibData   *id;
extern FnlibData   *fd;
extern char        *docdir;
