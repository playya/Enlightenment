#ifndef geist_H
#define geist_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <Imlib2.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <math.h>

#include "config.h"
#include "debug.h"
#include "structs.h"
#include "utils.h"
#include "geist_utils.h"

#define XY_IN_RECT(x, y, rx, ry, rw, rh) \
(((x) >= (rx)) && ((y) >= (ry)) && ((x) <= ((rx) + (rw))) && ((y) <= ((ry) + (rh))))

#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))

#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

#define HALF_SEL_WIDTH 3
#define HALF_SEL_HEIGHT 3
#define TRANS_THRESHOLD 0

#ifndef PACKAGE
#define PACKAGE "geist"
#endif

#ifndef VERSION
#define VERSION "0.0.2"
#endif

#define T(x, y) t[((y) * tw) + (x)]
#define CLIP(x, y, w, h, xx, yy, ww, hh) \
{ \
if (x < (xx)) {w += (x - (xx)); x = (xx);} \
if (y < (yy)) {h += (y - (yy)); y = (yy);} \
if ((x + w) > ((xx) + (ww))) {w = (ww) - x;} \
if ((y + h) > ((yy) + (hh))) {h = (hh) - y;} \
if (w<0) {x += w; w = 0 -w;} \
if (h<0) {y += h; h = 0 -h;} \
}
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))


extern int call_level;
extern gint obj_sel_handler, obj_unsel_handler;

struct
{
   unsigned char debug_level;
}
opt;

void imlib_init(GtkWidget * area);


/* Imlib stuff */
extern Display *disp;
extern Visual *vis;
extern Colormap cm;
extern int depth;
extern Window root;
extern Screen *scr;

extern GtkWidget *mainwin;
extern geist_document *current_doc;
extern geist_list *doc_list;

#endif
