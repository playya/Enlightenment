/*

 * Esetroot -- Set the root pixmap.  This program enables non-Enlightenment
 *             users to use Eterm's support for pseudotransparency.
 *
 * Written by Nat Friedman <ndf@mit.edu> with modifications by Gerald Britton
 * <gbritton@mit.edu> and Michael Jennings <mej@eterm.org>
 *
 */

static const char cvs_ident[] = "$Id$";

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <Imlib.h>

void set_pixmap_property(Pixmap p);

Display *Xdisplay;
Screen *scr;
Window Xroot;
int screen;
unsigned char debug = 0;

#define Xdepth	 (DefaultDepth(Xdisplay, screen))

void
set_pixmap_property(Pixmap p)
{

  Atom prop_root, prop_esetroot, type;
  int format;
  unsigned long length, after;
  unsigned char *data_root, *data_esetroot;

  prop_root = XInternAtom(Xdisplay, "_XROOTPMAP_ID", True);
  prop_esetroot = XInternAtom(Xdisplay, "ESETROOT_PMAP_ID", True);

  if (debug) {
    fprintf(stderr, "%s:%d:  set_pixmap_property(0x%08x):  prop_root == 0x%08x, prop_esetroot == 0x%08x\n", __FILE__, __LINE__,
	    (unsigned int)  p, (unsigned int) prop_root, (unsigned int) prop_esetroot);
  }
  if (prop_root != None && prop_esetroot != None) {
    XGetWindowProperty(Xdisplay, Xroot, prop_root, 0L, 1L, False, AnyPropertyType,
		       &type, &format, &length, &after, &data_root);
    if (type == XA_PIXMAP) {
      XGetWindowProperty(Xdisplay, Xroot, prop_esetroot, 0L, 1L, False, AnyPropertyType,
			 &type, &format, &length, &after, &data_esetroot);
      if (data_root && data_esetroot) {
	if (debug) {
	  fprintf(stderr, "%s:%d:  set_pixmap_property(0x%08x):  data_root == 0x%08x, data_esetroot == 0x%08x\n", __FILE__, __LINE__,
		  (unsigned int) p, (unsigned int) *((Pixmap *) data_root), (unsigned int) *((Pixmap *) data_esetroot));
	}
	if (type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_esetroot)) {
	  if (debug) {
	    fprintf(stderr, "%s:%d:  set_pixmap_property(0x%08x):  XKillClient() is being called.\n", __FILE__, __LINE__, (unsigned int) p);
	  }
	  XKillClient(Xdisplay, *((Pixmap *) data_root));
	}
      }
    }
  }
  /* This will locate the property, creating it if it doesn't exist */
  prop_root = XInternAtom(Xdisplay, "_XROOTPMAP_ID", False);
  prop_esetroot = XInternAtom(Xdisplay, "ESETROOT_PMAP_ID", False);

  /* The call above should have created it.  If that failed, we can't continue. */
  if (prop_root == None || prop_esetroot == None) {
    fprintf(stderr, "Esetroot:  creation of pixmap property failed.\n");
    exit(1);
  }
  XChangeProperty(Xdisplay, Xroot, prop_root, XA_PIXMAP, 32, PropModeReplace,
		  (unsigned char *) &p, 1);
  XChangeProperty(Xdisplay, Xroot, prop_esetroot, XA_PIXMAP, 32, PropModeReplace,
		  (unsigned char *) &p, 1);
  if (debug) {
    fprintf(stderr, "%s:%d:  set_pixmap_property(0x%08x):  _XROOTPMAP_ID and ESETROOT_PMAP_ID set to 0x%08x.\n", __FILE__, __LINE__, (unsigned int) p, (unsigned int) p);
  }
  XSetCloseDownMode(Xdisplay, RetainPermanent);
  XFlush(Xdisplay);
}

int
main(int argc, char *argv[])
{

  unsigned char scale = 0, center = 0, fit = 0;
  char *displayname = NULL;
  char *fname = NULL;
  ImlibData *id;
  ImlibImage *im;
  ImlibInitParams params;
  Pixmap p, temp_pmap;
  register unsigned char i;
  GC gc;
  XGCValues gcv;
  int w, h, x, y;

  if (argc < 2) {
    fprintf(stderr, "%s [-display <display_name>] [-scale] [-center] [-fit] pixmap\n", *argv);
    fprintf(stderr, "\t Short options are also recognized (-d, -s, -c, and -f)\n");
    exit(0);
  }
  for (i = 1; i < argc; i++) {
    if (*argv[i] != '-') {
      break;
    }
    if (argv[i][1] == 'd') {
      displayname = argv[++i];
    } else if (argv[i][1] == 's') {
      scale = 1;
    } else if (argv[i][1] == 'c') {
      center = 1;
    } else if (argv[i][1] == 'f') {
      fit = 1;
    } else if (argv[i][1] == 'x') {
      fprintf(stderr, "Debugging activated.\n");
      debug = 1;
    } else {
      fprintf(stderr, "%s:  Unrecognized option \'%c\'\n\n", *argv, argv[i][1]);
      fprintf(stderr, "%s [-display display] [-scale] pixmap\n", *argv);
      fprintf(stderr, "\t Short options are also recognized (-d and -s)\n");
      exit(2);
    }
  }

  fname = argv[i];
  if (scale)
    center = 0;

  if (debug) {
    fprintf(stderr, "%s:%d:  Display name is \"%s\"\n", __FILE__, __LINE__, displayname ? displayname : "(nil)");
    fprintf(stderr, "%s:%d:  Image will be %s\n", __FILE__, __LINE__, scale ? "scaled" : (center ? "centered" : (fit ? "fit" : "tiled")));
    fprintf(stderr, "%s:%d:  Image file is %s\n", __FILE__, __LINE__, fname ? fname : "(nil)");
  }
  if (!displayname) {
    displayname = getenv("DISPLAY");
    if (debug) {
      fprintf(stderr, "%s:%d:  Display name set to %s via getenv(\"DISPLAY\")\n", __FILE__, __LINE__, displayname ? displayname : "(nil)");
    }
  }
  if (!displayname) {
    displayname = ":0.0";
    if (debug) {
      fprintf(stderr, "%s:%d:  Display name defaulted to %s\n", __FILE__, __LINE__, displayname ? displayname : "(nil)");
    }
  }
  if ((Xdisplay = XOpenDisplay(displayname)) == 0) {
    fprintf(stderr, "%s:  Unable to open display %s\n", *argv, displayname);
    exit(1);
  }
  screen = DefaultScreen(Xdisplay);
  Xroot = RootWindow(Xdisplay, screen);
  scr = ScreenOfDisplay(Xdisplay, screen);
  if (debug) {
    fprintf(stderr, "%s:%d:  Chose screen %d\n", __FILE__, __LINE__, screen);
    fprintf(stderr, "%s:%d:  Root window is 0x%08x\n", __FILE__, __LINE__, (unsigned int) Xroot);
    fprintf(stderr, "%s:%d:  Found screen information at 0x%08x\n", __FILE__, __LINE__, (unsigned int) scr);
  }
  params.flags = PARAMS_VISUALID;
  params.visualid = (DefaultVisual(Xdisplay, screen))->visualid;
  id = Imlib_init_with_params(Xdisplay, &params);
  im = Imlib_load_image(id, fname);
  if (debug) {
    fprintf(stderr, "%s:%d:  The Imlib Data is at 0x%08x\n", __FILE__, __LINE__, (unsigned int) id);
    fprintf(stderr, "%s:%d:  The Imlib Image is at 0x%08x\n", __FILE__, __LINE__, (unsigned int) im);
  }
  if (scale) {
    w = scr->width;
    h = scr->height;
  } else {
    w = im->rgb_width;
    h = im->rgb_height;
  }
  if (fit) {
    double x_ratio, y_ratio;

    x_ratio = ((double) scr->width) / ((double) w);
    y_ratio = ((double) scr->height) / ((double) h);
    if (x_ratio > y_ratio) {
      x_ratio = y_ratio;
    }
    w = (int) (w * x_ratio);
    h = (int) (h * x_ratio);
  }
  p = XCreatePixmap(Xdisplay, Xroot, scr->width, scr->height, Xdepth);
  gcv.foreground = gcv.background = BlackPixel(Xdisplay, screen);
  gc = XCreateGC(Xdisplay, p, ((center || fit) ? (GCForeground | GCBackground) : 0), &gcv);
  if (scale) {
    XFillRectangle(Xdisplay, p, gc, 0, 0, w, h);
  }
  if (center || fit) {
    XFillRectangle(Xdisplay, p, gc, 0, 0, scr->width, scr->height);
    x = (scr->width - w) >> 1;
    y = (scr->height - h) >> 1;
  } else {
    x = 0;
    y = 0;
  }
  if (debug) {
    fprintf(stderr, "%s:%d:  Assigned width and height for rendering as %dx%d\n", __FILE__, __LINE__, w, h);
    fprintf(stderr, "%s:%d:  Created %dx%d+%d+%d pixmap 0x%08x\n", __FILE__, __LINE__, scr->width, scr->height, x, y, (unsigned int) p);
    fprintf(stderr, "%s:%d:  Applied Graphics Context 0x%08x to pixmap.\n", __FILE__, __LINE__, (unsigned int) gc);
  }
  Imlib_render(id, im, w, h);
  temp_pmap = Imlib_move_image(id, im);
  if (debug) {
    fprintf(stderr, "%s:%d:  Rendered at %dx%d onto pixmap 0x%08x\n", __FILE__, __LINE__, w, h, (unsigned int) temp_pmap);
  }
  if (temp_pmap != None) {
    XSetTile(Xdisplay, gc, temp_pmap);
    XSetTSOrigin(Xdisplay, gc, x, y);
    XSetFillStyle(Xdisplay, gc, FillTiled);
    if (center || fit) {
      XFillRectangle(Xdisplay, p, gc, x, y, w, h);
    } else {
      XFillRectangle(Xdisplay, p, gc, x, y, scr->width, scr->height);
    }
    set_pixmap_property(p);
    XFlush(Xdisplay);
    XSetWindowBackgroundPixmap(Xdisplay, Xroot, p);
    XClearWindow(Xdisplay, Xroot);
    XFlush(Xdisplay);
  }
  return 0;
}
