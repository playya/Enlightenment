/* winwidget.h
 *
 * Copyright (C) 1999 Tom Gilbert
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

#ifndef WINWIDGET_H
#define WINWIDGET_H

/* This MWM stuff pinched from Eterm/src/command.h */

# include <X11/X.h>
# include <X11/Xproto.h>

/* Motif window hints */
#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS        (1L << 3)
/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)
/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)
/* bit definitions for MwmHints.inputMode */
#define MWM_INPUT_MODELESS                  0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL              2
#define MWM_INPUT_FULL_APPLICATION_MODAL    3
#define PROP_MWM_HINTS_ELEMENTS             5

/* Motif window hints */
typedef struct _mwmhints
{
   CARD32 flags;
   CARD32 functions;
   CARD32 decorations;
   INT32 input_mode;
   CARD32 status;
}
MWMHints;

struct __winwidget
{
   Window win;
   int w;
   int h;
   int im_w;
   int im_h;
   unsigned char type;
   unsigned char had_resize;
   Imlib_Image *im;
   GC gc;
   Pixmap bg_pmap;
   char *name;
   feh_file *file;

   /* Stuff for zooming */
   unsigned char zoom_mode;
   int zx;
   int zy;
   double zoom;
};

enum win_type
{
   WIN_TYPE_UNSET, WIN_TYPE_SLIDESHOW, WIN_TYPE_SINGLE, WIN_TYPE_ABOUT
};

int winwidget_loadimage(winwidget winwid, feh_file * filename);
void winwidget_show(winwidget winwid);
void winwidget_hide(winwidget winwid);
void winwidget_destroy_all(void);
void winwidget_free_image(winwidget w);
void winwidget_render_image(winwidget winwid, int resize);
void winwidget_resize(winwidget winwid, int w, int h);
void winwidget_setup_pixmaps(winwidget winwid, int w, int h);
void winwidget_update_title(winwidget ret);
winwidget winwidget_get_from_window(Window win);
winwidget winwidget_create_from_file(feh_file * filename, char *name,

                                     char type);
winwidget winwidget_create_from_image(Imlib_Image * im, char *name,

                                      char type);
void winwidget_rename(winwidget winwid, char *newname);
void winwidget_destroy(winwidget winwid);
void winwidget_create_window(winwidget ret, int w, int h);

extern int window_num;             /* For window list */
extern winwidget *windows;      /* List of windows to loop though */

#endif
