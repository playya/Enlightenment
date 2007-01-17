/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2007 Kim Woelders
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "alert.h"
#include "lang.h"
#include "session.h"
#include "util.h"

/* Should be elsewhere */
void                SoundPlay(const char *name);

#define ExTextExtents XmbTextExtents
#define ExDrawString XmbDrawString

#define ExSetColor(pxc, r, g, b) \
  do { \
    (pxc)->red = (r << 8) | r; (pxc)->green = (g << 8) | g; (pxc)->blue = (b << 8) | b; \
  } while (0)

static XFontSet     xfs = NULL;

#define DRAW_BOX_OUT(mdd, mgc, mwin, mx, my, mw, mh) \
        AlertDrawBox(mdd, mgc, mwin, mx, my, mw, mh, \
        colorful, cols[0], cols[2], cols[3])
#define DRAW_BOX_IN(mdd, mgc, mwin, mx, my, mw, mh) \
        AlertDrawBox(mdd, mgc, mwin, mx, my, mw, mh, \
        colorful, cols[2], cols[0], cols[3])
static void
AlertDrawBox(Display * mdd, GC mgc, Window mwin, int mx, int my, int mw, int mh,
	     int colorful, unsigned long c1, unsigned long c2, unsigned long cb)
{
   if (colorful)
     {
	XSetForeground(mdd, mgc, cb);
	XDrawRectangle(mdd, mwin, mgc, mx, my, mw - 1, mh - 1);
	XSetForeground(mdd, mgc, c1);
	XDrawLine(mdd, mwin, mgc, mx + 1, my + 1, mx + mw - 3, my + 1);
	XDrawLine(mdd, mwin, mgc, mx + 1, my + 1, mx + 1, my + mh - 3);
	XSetForeground(mdd, mgc, c2);
	XDrawLine(mdd, mwin, mgc, mx + 2, my + mh - 2, mx + mw - 2,
		  my + mh - 2);
	XDrawLine(mdd, mwin, mgc, mx + mw - 2, my + 2, mx + mw - 2,
		  my + mh - 2);
     }
   else
     {
	XDrawRectangle(mdd, mwin, mgc, mx, my, mw - 1, mh - 1);
     }
}

#define DRAW_THIN_BOX_IN(mdd, mgc, mwin, mx, my, mw, mh) \
        AlertDrawThinBoxIn(mdd, mgc, mwin, mx, my, mw, mh, \
        colorful, cols[2], cols[0])
static void
AlertDrawThinBoxIn(Display * mdd, GC mgc, Window mwin, int mx, int my, int mw,
		   int mh, int colorful, unsigned long c1, unsigned long c2)
{
   if (colorful)
     {
	XSetForeground(mdd, mgc, c1);
	XDrawLine(mdd, mwin, mgc, mx + 1, my + 1, mx + mw - 3, my + 1);
	XDrawLine(mdd, mwin, mgc, mx + 1, my + 1, mx + 1, my + mh - 3);
	XSetForeground(mdd, mgc, c2);
	XDrawLine(mdd, mwin, mgc, mx + 2, my + mh - 2, mx + mw - 2,
		  my + mh - 2);
	XDrawLine(mdd, mwin, mgc, mx + mw - 2, my + 2, mx + mw - 2,
		  my + mh - 2);
     }
}

#define DRAW_HEADER(mdd, mgc, mwin, mx, my, mstr) \
        AlertDrawHeader(mdd, mgc, mwin, mx, my, mstr, \
        colorful, cols[2], cols[3], cols[4])
static void
AlertDrawHeader(Display * mdd, GC mgc, Window mwin, int mx, int my,
		const char *mstr, int colorful, unsigned long cb,
		unsigned long ct1, unsigned long ct2)
{
   int                 len = strlen(mstr);

   if (colorful)
     {
	XSetForeground(mdd, mgc, cb);
	ExDrawString(mdd, mwin, xfs, mgc, mx + 1, my + 1, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx + 2, my + 1, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx + 2, my + 2, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx + 1, my + 2, mstr, len);
	XSetForeground(mdd, mgc, ct1);
	ExDrawString(mdd, mwin, xfs, mgc, mx - 1, my, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx, my - 1, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx + 1, my, mstr, len);
	ExDrawString(mdd, mwin, xfs, mgc, mx, my + 1, mstr, len);
	XSetForeground(mdd, mgc, ct2);
	ExDrawString(mdd, mwin, xfs, mgc, mx, my, mstr, len);
     }
   else
     {
	ExDrawString(mdd, mwin, xfs, mgc, mx, my, mstr, len);
     }
}

#define DRAW_STRING(mdd, mgc, mwin, mx, my, mstr) \
        AlertDrawString(mdd, mgc, mwin, mx, my, mstr, \
        colorful, cols[3])
static void
AlertDrawString(Display * mdd, GC mgc, Window mwin, int mx, int my,
		const char *mstr, int colorful, unsigned long ct1)
{
   int                 len = strlen(mstr);

   if (colorful)
     {
	XSetForeground(mdd, mgc, ct1);
	ExDrawString(mdd, mwin, xfs, mgc, mx, my, mstr, len);
     }
   else
     {
	ExDrawString(mdd, mwin, xfs, mgc, mx, my, mstr, len);
     }
}

static char        *
AlertButtonText(int btn, const char *text)
{
   char               *s;

   if (!text)
      return NULL;

   s = EMALLOC(char, strlen(text) + 6);
   if (!s)
      return NULL;

   sprintf(s, "(F%d) %s", btn, text);

   return s;
}

static void
ShowAlert(const char *title,
	  const char *ignore, const char *restart, const char *quit, char *text)
{
   Window              win = 0, b1 = 0, b2 = 0, b3 = 0;
   Display            *dd;
   int                 wid, hih, w, h, i, j, k, mask;
   XGCValues           gcv;
   GC                  gc;
   char                line[1024];
   XEvent              ev;
   XSetWindowAttributes att;
   XRectangle          rect1, rect2;
   char                colorful;
   unsigned long       cols[5];
   XColor              xcl;
   Colormap            cmap;
   int                 cnum, fh, x, y, ww, hh, mh;
   char               *str1, *str2, *str3;
   KeyCode             key;
   int                 button;
   char              **missing_charset_list_return, *def_string_return;
   int                 missing_charset_count_return;
   XFontStruct       **font_struct_list_return;
   char              **font_name_list_return;

   SoundPlay("SOUND_ALERT");

   if (!text)
      return;

   /*
    * We may get here from obscure places like an X-error or signal handler
    * and things seem to work properly only if we do a new XOpenDisplay().
    */
   dd = XOpenDisplay(NULL);
   if (!dd)
     {
	fprintf(stderr, text);
	fflush(stderr);
	return;
     }

   cmap = DefaultColormap(dd, DefaultScreen(dd));

   if (!title)
      title = _("Enlightenment Error");
   str1 = AlertButtonText(1, ignore);
   str2 = AlertButtonText(2, restart);
   str3 = AlertButtonText(3, quit);

   cnum = 0;
   colorful = 0;
   cols[0] = cols[1] = cols[2] = cols[3] = cols[4] = 0;
   if (DefaultDepth(dd, DefaultScreen(dd)) > 4)
     {
	ExSetColor(&xcl, 220, 220, 220);
	if (!XAllocColor(dd, cmap, &xcl))
	   goto CN;
	cols[cnum++] = xcl.pixel;
	ExSetColor(&xcl, 160, 160, 160);
	if (!XAllocColor(dd, cmap, &xcl))
	   goto CN;
	cols[cnum++] = xcl.pixel;
	ExSetColor(&xcl, 100, 100, 100);
	if (!XAllocColor(dd, cmap, &xcl))
	   goto CN;
	cols[cnum++] = xcl.pixel;
	ExSetColor(&xcl, 0, 0, 0);
	if (!XAllocColor(dd, cmap, &xcl))
	   goto CN;
	cols[cnum++] = xcl.pixel;
	ExSetColor(&xcl, 255, 255, 255);
	if (!XAllocColor(dd, cmap, &xcl))
	   goto CN;
	cols[cnum++] = xcl.pixel;
	colorful = 1;
     }
 CN:

   wid = DisplayWidth(dd, DefaultScreen(dd));
   hih = DisplayHeight(dd, DefaultScreen(dd));
   w = (wid - 600) / 2;
   h = (hih - 440) / 2;

   if (colorful)
      att.background_pixel = cols[1];
   else
      att.background_pixel = BlackPixel(dd, DefaultScreen(dd));
   if (colorful)
      att.border_pixel = cols[3];
   else
      att.border_pixel = WhitePixel(dd, DefaultScreen(dd));
   att.backing_store = Always;
   att.save_under = True;
   att.override_redirect = True;
   mask = CWBackPixel | CWBorderPixel | CWOverrideRedirect | CWSaveUnder |
      CWBackingStore;
   win = XCreateWindow(dd, DefaultRootWindow(dd), -100, -100, 1, 1, 0,
		       CopyFromParent, InputOutput, CopyFromParent, mask, &att);

   if (str1 && sscanf(str1, "%s", line) > 0)
     {
	b1 = XCreateWindow(dd, win, -100, -100, 1, 1, 0, CopyFromParent,
			   InputOutput, CopyFromParent, mask, &att);
	XMapWindow(dd, b1);
     }
   if (str2 && sscanf(str2, "%s", line) > 0)
     {
	b2 = XCreateWindow(dd, win, -100, -100, 1, 1, 0, CopyFromParent,
			   InputOutput, CopyFromParent, mask, &att);
	XMapWindow(dd, b2);
     }
   if (str3 && sscanf(str3, "%s", line) > 0)
     {
	b3 = XCreateWindow(dd, win, -100, -100, 1, 1, 0, CopyFromParent,
			   InputOutput, CopyFromParent, mask, &att);
	XMapWindow(dd, b3);
     }

   gc = XCreateGC(dd, win, 0, &gcv);
   if (colorful)
      XSetForeground(dd, gc, cols[3]);
   else
      XSetForeground(dd, gc, att.border_pixel);

   xfs = XCreateFontSet(dd, "-*-sans unicode-*-r-*-*-12-*-*-*-*-*-*-*,"
			"-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*,fixed",
			&missing_charset_list_return,
			&missing_charset_count_return, &def_string_return);
   if (!xfs)
      goto done;

   if (missing_charset_list_return)
      XFreeStringList(missing_charset_list_return);

   k = XFontsOfFontSet(xfs, &font_struct_list_return, &font_name_list_return);
   fh = 0;
   for (i = 0; i < k; i++)
     {
	h = font_struct_list_return[i]->ascent +
	   font_struct_list_return[i]->descent;
	if (fh < h)
	   fh = h;
     }

   XSelectInput(dd, win, KeyPressMask | KeyReleaseMask | ExposureMask);
   XMapWindow(dd, win);
   XGrabPointer(dd, win, True, ButtonPressMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
   XGrabKeyboard(dd, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
   XSetInputFocus(dd, win, RevertToPointerRoot, CurrentTime);

   XGrabServer(dd);
   XSync(dd, False);

   for (i = 40; i < 600; i += 40)
     {
	ww = i;
	hh = (i * 440) / 600;
	x = (wid - ww) >> 1;
	y = (hih - hh) >> 1;
	XMoveResizeWindow(dd, win, x, y, ww, hh);
	DRAW_BOX_OUT(dd, gc, win, 0, 0, ww, hh);
	XSync(dd, False);
     }
   ww = 600;
   hh = 440;
   x = (wid - 600) >> 1;
   y = (hih - 440) >> 1;
   XMoveResizeWindow(dd, win, x, y, ww, hh);
   XUngrabServer(dd);
   XSync(dd, False);

   mh = 0;
   if (str1)
     {
	ExTextExtents(xfs, str1, strlen(str1), &rect1, &rect2);
	mh = (rect2.width > mh) ? rect2.width : mh;
     }
   if (str2)
     {
	ExTextExtents(xfs, str2, strlen(str2), &rect1, &rect2);
	mh = (rect2.width > mh) ? rect2.width : mh;
     }
   if (str3)
     {
	ExTextExtents(xfs, str3, strlen(str3), &rect1, &rect2);
	mh = (rect2.width > mh) ? rect2.width : mh;
     }
   mh += 10;

   if (str1 && sscanf(str1, "%s", line) > 0)
     {
	w = 5 + (((580 - mh) * 0) / 4);
	XMoveResizeWindow(dd, b1, w, 440 - 15 - fh, mh + 10, fh + 10);
	XSelectInput(dd, b1,
		     ButtonPressMask | ButtonReleaseMask | ExposureMask);
     }
   if (str2 && sscanf(str2, "%s", line) > 0)
     {
	w = 5 + (((580 - mh) * 1) / 2);
	XMoveResizeWindow(dd, b2, w, 440 - 15 - fh, mh + 10, fh + 10);
	XSelectInput(dd, b2,
		     ButtonPressMask | ButtonReleaseMask | ExposureMask);
     }
   if (str3 && sscanf(str3, "%s", line) > 0)
     {
	w = 5 + (((580 - mh) * 2) / 2);
	XMoveResizeWindow(dd, b3, w, 440 - 15 - fh, mh + 10, fh + 10);
	XSelectInput(dd, b3,
		     ButtonPressMask | ButtonReleaseMask | ExposureMask);
     }
   XSync(dd, False);

   button = 0;
   for (; button == 0;)
     {
	XNextEvent(dd, &ev);
	switch (ev.type)
	  {
	  case KeyPress:
	     key = XKeysymToKeycode(dd, XStringToKeysym("F1"));
	     if (key == ev.xkey.keycode)
	       {
		  DRAW_BOX_IN(dd, gc, b1, 0, 0, mh + 10, fh + 10);
		  XSync(dd, False);
		  sleep(1);
		  DRAW_BOX_OUT(dd, gc, b1, 0, 0, mh + 10, fh + 10);
		  button = 1;
		  break;
	       }
	     key = XKeysymToKeycode(dd, XStringToKeysym("F2"));
	     if (key == ev.xkey.keycode)
	       {
		  DRAW_BOX_IN(dd, gc, b2, 0, 0, mh + 10, fh + 10);
		  XSync(dd, False);
		  sleep(1);
		  DRAW_BOX_OUT(dd, gc, b2, 0, 0, mh + 10, fh + 10);
		  button = 2;
		  break;
	       }
	     key = XKeysymToKeycode(dd, XStringToKeysym("F3"));
	     if (key == ev.xkey.keycode)
	       {
		  DRAW_BOX_IN(dd, gc, b3, 0, 0, mh + 10, fh + 10);
		  XSync(dd, False);
		  sleep(1);
		  DRAW_BOX_OUT(dd, gc, b3, 0, 0, mh + 10, fh + 10);
		  button = 3;
		  break;
	       }
	     XSync(dd, False);
	     break;

	  case ButtonPress:
	     if (ev.xbutton.window == b1)
	       {
		  DRAW_BOX_IN(dd, gc, b1, 0, 0, mh + 10, fh + 10);
	       }
	     else if (ev.xbutton.window == b2)
	       {
		  DRAW_BOX_IN(dd, gc, b2, 0, 0, mh + 10, fh + 10);
	       }
	     else if (ev.xbutton.window == b3)
	       {
		  DRAW_BOX_IN(dd, gc, b3, 0, 0, mh + 10, fh + 10);
	       }
	     XSync(dd, False);
	     break;

	  case ButtonRelease:
	     if (ev.xbutton.window == b1)
	       {
		  DRAW_BOX_OUT(dd, gc, b1, 0, 0, mh + 10, fh + 10);
		  button = 1;
	       }
	     else if (ev.xbutton.window == b2)
	       {
		  DRAW_BOX_OUT(dd, gc, b2, 0, 0, mh + 10, fh + 10);
		  button = 2;
	       }
	     else if (ev.xbutton.window == b3)
	       {
		  DRAW_BOX_OUT(dd, gc, b3, 0, 0, mh + 10, fh + 10);
		  button = 3;
	       }
	     XSync(dd, False);
	     break;

	  case Expose:
	     /* Flush all other Expose events */
	     while (XCheckTypedWindowEvent(dd, ev.xexpose.window, Expose, &ev))
		;

	     ExTextExtents(xfs, title, strlen(title), &rect1, &rect2);
	     w = rect2.width;

	     DRAW_HEADER(dd, gc, win, (600 - w) / 2, 5 - rect2.y, title);
	     DRAW_BOX_OUT(dd, gc, win, 0, 0, ww, fh + 10);
	     DRAW_BOX_OUT(dd, gc, win, 0, fh + 10 - 1, ww,
			  hh - fh - fh - 30 + 2);
	     DRAW_BOX_OUT(dd, gc, win, 0, 440 - fh - 20, ww, fh + 20);
	     i = 0;
	     j = 0;
	     k = fh + 10;
	     while (text[i])
	       {
		  line[j++] = text[i++];
		  if (line[j - 1] == '\n')
		    {
		       line[j - 1] = 0;
		       j = 0;
		       DRAW_STRING(dd, gc, win, 6, 6 + k + fh, line);
		       k += fh + 2;
		    }
	       }
	     if (str1 && sscanf(str1, "%s", line) > 0)
	       {
		  ExTextExtents(xfs, str1, strlen(str1), &rect1, &rect2);
		  h = rect2.width;
		  w = 3 + (((580 - mh) * 0) / 4);
		  DRAW_HEADER(dd, gc, b1, 5 + (mh - h) / 2, 5 - rect2.y, str1);
		  DRAW_BOX_OUT(dd, gc, b1, 0, 0, mh + 10, fh + 10);
		  DRAW_THIN_BOX_IN(dd, gc, win, w, 440 - 17 - fh, mh + 14,
				   fh + 14);
	       }
	     if (str2 && sscanf(str2, "%s", line) > 0)
	       {
		  ExTextExtents(xfs, str2, strlen(str2), &rect1, &rect2);
		  h = rect2.width;
		  w = 3 + (((580 - mh) * 1) / 2);
		  DRAW_HEADER(dd, gc, b2, 5 + (mh - h) / 2, 5 - rect2.y, str2);
		  DRAW_BOX_OUT(dd, gc, b2, 0, 0, mh + 10, fh + 10);
		  DRAW_THIN_BOX_IN(dd, gc, win, w, 440 - 17 - fh, mh + 14,
				   fh + 14);
	       }
	     if (str3 && sscanf(str3, "%s", line) > 0)
	       {
		  ExTextExtents(xfs, str3, strlen(str3), &rect1, &rect2);
		  h = rect2.width;
		  w = 3 + (((580 - mh) * 2) / 2);
		  DRAW_HEADER(dd, gc, b3, 5 + (mh - h) / 2, 5 - rect2.y, str3);
		  DRAW_BOX_OUT(dd, gc, b3, 0, 0, mh + 10, fh + 10);
		  DRAW_THIN_BOX_IN(dd, gc, win, w, 440 - 17 - fh, mh + 14,
				   fh + 14);
	       }
	     XSync(dd, False);
	     break;

	  default:
	     break;
	  }
     }

   XDestroyWindow(dd, win);
   XFreeGC(dd, gc);
   XFreeFontSet(dd, xfs);
   if (cnum > 0)
      XFreeColors(dd, cmap, cols, cnum, 0);
   XCloseDisplay(dd);

   switch (button)
     {
     default:
     case 1:
	break;
     case 2:
	SessionExit(EEXIT_RESTART, NULL);
	break;
     case 3:
	SessionExit(EEXIT_EXIT, NULL);
	break;
     }

 done:
   if (str1)
      Efree(str1);
   if (str2)
      Efree(str2);
   if (str3)
      Efree(str3);
}

void
AlertX(const char *title, const char *ignore,
       const char *restart, const char *quit, const char *fmt, ...)
{
   char                text[10240];
   va_list             args;

   va_start(args, fmt);
   Evsnprintf(text, sizeof(text), fmt, args);
   va_end(args);

   ShowAlert(title, ignore, restart, quit, text);
}

void
Alert(const char *fmt, ...)
{
   char                text[10240];
   va_list             args;

   va_start(args, fmt);
   Evsnprintf(text, sizeof(text), fmt, args);
   va_end(args);

   ShowAlert(_("Enlightenment Message Dialog"), _("Ignore this"),
	     _("Restart Enlightenment"), _("Quit Enlightenment"), text);
}
