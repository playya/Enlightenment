/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
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
#include "E.h"

struct _efont
{
   Imlib_Font         *face;
};

static void
ImlibSetFgColorFromGC(Display * dpy, GC gc, Colormap cm)
{
   XGCValues           xgcv;
   XColor              xclr;
   int                 r, g, b;

   XGetGCValues(dpy, gc, GCForeground, &xgcv);
   xclr.pixel = xgcv.foreground;
   XQueryColor(dpy, cm, &xclr);
   EGetColor(&xclr, &r, &g, &b);
   imlib_context_set_color(r, g, b, 255);
}

void
EFont_draw_string(Display * dpy, Drawable win, GC gc, int x, int y,
		  const char *text, Efont * f, Visual * vis __UNUSED__,
		  Colormap cm)
{
   Imlib_Image         im;
   int                 w, h, ascent, descent;

   Efont_extents(f, text, &ascent, &descent, &w, NULL, NULL, NULL, NULL);
   h = ascent + descent;

   imlib_context_set_drawable(win);
   im = imlib_create_image_from_drawable(0, x, y - ascent, w, h, 0);
   imlib_context_set_image(im);

   imlib_context_set_font(f->face);
   ImlibSetFgColorFromGC(dpy, gc, cm);
   imlib_text_draw(0, 0, text);
   imlib_render_image_on_drawable(x, y - ascent);
   imlib_free_image();
}

void
Efont_free(Efont * f)
{
   if (!f)
      return;

   imlib_context_set_font(f->face);
   imlib_free_font();

   Efree(f);
}

Efont              *
Efont_load(const char *file, int size)
{
   char                s[4096];
   Efont              *f;
   Imlib_Font         *ff;

   Esnprintf(s, sizeof(s), "%s/%d", file, size);
   ff = imlib_load_font(s);
   if (ff == NULL)
      return NULL;

   f = Emalloc(sizeof(Efont));
   f->face = ff;

   return f;
}

void
Efont_extents(Efont * f, const char *text, int *font_ascent_return,
	      int *font_descent_return, int *width_return,
	      int *max_ascent_return, int *max_descent_return,
	      int *lbearing_return __UNUSED__, int *rbearing_return __UNUSED__)
{
   int                 w, h;

   if (!f)
      return;

   imlib_context_set_font(f->face);
   imlib_get_text_advance(text, &w, &h);
   if (width_return)
      *width_return = w;
   if (font_ascent_return)
      *font_ascent_return = imlib_get_font_ascent();
   if (font_descent_return)
      *font_descent_return = imlib_get_font_descent();
   if (max_ascent_return)
      *max_ascent_return = imlib_get_maximum_font_ascent();
   if (max_descent_return)
      *max_descent_return = imlib_get_maximum_font_descent();
}

#if TEST_TTFONT

Display            *disp;

int
main(int argc, char **argv)
{
   Efont              *f;
   GC                  gc;
   XGCValues           gcv;
   Window              win;
   int                 i, j;

   disp = XOpenDisplay(NULL);

   imlib_context_set_display(disp);
   imlib_context_set_visual(DefaultVisual(disp, DefaultScreen(disp)));
   imlib_context_set_colormap(DefaultColormap(disp, DefaultScreen(disp)));

   srand(time(NULL));
   win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, 640, 480, 0,
			     0, 0);
   XMapWindow(disp, win);
   ecore_x_sync();

   gcv.subwindow_mode = IncludeInferiors;
   gc = XCreateGC(disp, win, GCSubwindowMode, &gcv);
   for (;; j++)
     {
	for (i = 3; i < argc; i++)
	  {
	     XSetForeground(disp, gc, rand() << 16 | rand());
	     f = Efont_load(argv[i], atoi(argv[1]));
	     if (f)
		EFont_draw_string(disp, win, gc, 20,
				  atoi(argv[1]) * (i - 2), argv[2], f,
				  DefaultVisual(disp, DefaultScreen(disp)),
				  DefaultColormap(disp, DefaultScreen(disp)));
	     Efont_free(f);
	     f = NULL;
	  }
     }
   return 0;
}

#ifndef USE_ECORE_X
void
ecore_x_grab(void)
{
   XGrabServer(disp);
}

void
ecore_x_ungrab(void)
{
   XUngrabServer(disp);
   XFlush(disp);
}
#endif
#endif
