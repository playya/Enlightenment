/* winwidget.c
 *
 * Copyright (C) 1999 Tom Gilbert
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "feh.h"
#include "winwidget.h"

static winwidget winwidget_allocate (void)
{
  winwidget ret = NULL;

  if ((ret = malloc (sizeof (_winwidget))) == NULL)
    exit (1);

  ret->win = 0;
  ret->w = 0;
  ret->h = 0;
  ret->visible = 0;
  ret->bg_pmap = 0;
  ret->im = NULL;
  ret->name = NULL;

  return ret;
}

winwidget winwidget_create_from_image (Imlib_Image * im)
{
  winwidget ret = NULL;

  D (("In winwidget_create_from_image\n"));

  if (im == NULL)
    return NULL;

  ret = winwidget_allocate ();

  ret->im = im;
  imlib_context_set_image (ret->im);
  ret->w = imlib_image_get_width ();
  ret->h = imlib_image_get_height ();

  ret->name = strdup (PACKAGE);

  winwidget_create_window (ret);

  return ret;
}

winwidget winwidget_create_from_file (char *filename)
{
  winwidget ret = NULL;

  D (("In winwidget_create_from_file\n"));

  if (filename == NULL)
    return NULL;

  ret = winwidget_allocate ();
  ret->name = strdup (filename);

  if (winwidget_loadimage (ret, filename) == 0)
    return NULL;

  imlib_context_set_image (ret->im);
  ret->w = imlib_image_get_width ();
  ret->h = imlib_image_get_height ();

  winwidget_create_window (ret);

  return ret;
}

static void
winwidget_create_window (winwidget ret)
{
  XSetWindowAttributes attr;
  XClassHint *xch;

  D (("In winwidget_create_window\n"));

  attr.backing_store = NotUseful;
  attr.override_redirect = False;
  attr.colormap = cm;
  attr.border_pixel = 0;
  attr.background_pixel = 0;
  attr.save_under = False;
  attr.event_mask = StructureNotifyMask | ButtonPressMask |
    ButtonReleaseMask | PointerMotionMask | EnterWindowMask |
    LeaveWindowMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask |
    ExposureMask | FocusChangeMask | PropertyChangeMask |
    VisibilityChangeMask;
  ret->win =
    XCreateWindow (disp, DefaultRootWindow (disp), 0, 0, ret->w, ret->h, 0,
                   depth, InputOutput, vis,
                   CWOverrideRedirect | CWSaveUnder | CWBackingStore |
                   CWColormap | CWBackPixel | CWBorderPixel | CWEventMask,
                   &attr);

  wmDeleteWindow = XInternAtom (disp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (disp, ret->win, &wmDeleteWindow, 1);
  if (ret->name)
    XStoreName (disp, ret->win, ret->name);
  else
    XStoreName (disp, ret->win, "feh");
  xch = XAllocClassHint ();
  xch->res_name = "feh";
  xch->res_class = "feh";
  XSetClassHint (disp, ret->win, xch);
  XFree (xch);
  /* set the icons name property */
  XSetIconName (disp, ret->win, "feh");
  /* set the command hint */
  XSetCommand (disp, ret->win, cmdargv, cmdargc);

  winwidget_register (ret);

  ret->bg_pmap = XCreatePixmap (disp, ret->win, ret->w, ret->h, depth);

  imlib_context_set_image (ret->im);
  imlib_context_set_drawable (ret->bg_pmap);
  imlib_render_image_on_drawable (0, 0);

  imlib_free_image ();

  XSetWindowBackgroundPixmap (disp, ret->win, ret->bg_pmap);
  XClearWindow (disp, ret->win);
}

void
winwidget_destroy (winwidget winwid)
{
  D (("In winwidget_destroy\n"));
  winwidget_unregister (winwid);
  if (winwid->visible)
    winwidget_hide (winwid);
  if (winwid->bg_pmap)
    XFreePixmap (disp, winwid->bg_pmap);
  if (winwid->name)
    free (winwid->name);
  XDestroyWindow (disp, winwid->win);
  free (winwid);
  winwid = NULL;
}

int
winwidget_loadimage (winwidget winwid, char *filename)
{
  D (("In winwidget_loadimage: filename %s\n", filename));

  return feh_load_image (&(winwid->im), filename);
}

void
winwidget_show (winwidget winwid)
{
  XEvent ev;

  D (("Got Here\n"));
  XMapWindow (disp, winwid->win);
  /* wait for the window to map */
  D (("In winwidget_show: Waiting for window to map\n"));
  XMaskEvent (disp, StructureNotifyMask, &ev);
  D (("In winwidget_show: Window mapped\n"));
  D (("Got Here\n"));
  winwid->visible = 1;
}

void
winwidget_hide (winwidget winwid)
{
  D (("In winwidget_hide\n"));
  XUnmapWindow (disp, winwid->win);
  winwid->visible = 0;
}

static void
winwidget_register (winwidget win)
{
  D (("In winwidget_register\n"));
  window_num++;
  if (windows)
    windows = realloc (windows, window_num * sizeof (winwidget));
  else
    windows = malloc (window_num * sizeof (winwidget));
  windows[window_num - 1] = win;
}

static void
winwidget_unregister (winwidget win)
{
  int i, j;
  D (("In winwidget_unregister\n"));

  for (i = 0; i < window_num; i++)
    {
      if (windows[i] == win)
        {
          for (j = i; j < window_num - 1; j++)
            windows[j] = windows[j + 1];
          window_num--;
          if (window_num > 0)
            windows = realloc (windows, window_num * sizeof (winwidget));
          else
            {
              free (windows);
              windows = NULL;
            }
        }
    }
}

winwidget winwidget_get_from_window (Window win)
{
  /* Loop through windows */
  int i;
  D (("In winwidget_get_from_window\n"));

  for (i = 0; i < window_num; i++)
    {
      if (windows[i]->win == win)
        return windows[i];
    }
  return NULL;
}
