/* events.c
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

#include "feh.h"
#include "filelist.h"
#include "winwidget.h"
#include "timers.h"
#include "options.h"
#include "events.h"

feh_event_handler *ev_handler[LASTEvent];

static void feh_event_handle_KeyPress(XEvent * ev);
static void feh_event_handle_ButtonPress(XEvent * ev);
static void feh_event_handle_ButtonRelease(XEvent * ev);
static void feh_event_handle_ConfigureNotify(XEvent * ev);
static void feh_event_handle_EnterNotify(XEvent * ev);
static void feh_event_handle_LeaveNotify(XEvent * ev);
static void feh_event_handle_MotionNotify(XEvent * ev);
static void feh_event_handle_ClientMessage(XEvent * ev);

void
feh_event_init(void)
{
   int i;

   D_ENTER;
   for (i = 0; i < LASTEvent; i++)
      ev_handler[i] = NULL;

   ev_handler[KeyPress] = feh_event_handle_KeyPress;
   ev_handler[ButtonPress] = feh_event_handle_ButtonPress;
   ev_handler[ButtonRelease] = feh_event_handle_ButtonRelease;
   ev_handler[ConfigureNotify] = feh_event_handle_ConfigureNotify;
   ev_handler[EnterNotify] = feh_event_handle_EnterNotify;
   ev_handler[LeaveNotify] = feh_event_handle_LeaveNotify;
   ev_handler[MotionNotify] = feh_event_handle_MotionNotify;
   ev_handler[ClientMessage] = feh_event_handle_ClientMessage;

   D_RETURN_;
}

static void
feh_event_handle_KeyPress(XEvent * ev)
{
   D_ENTER;
   while (XCheckTypedWindowEvent(disp, ev->xkey.window, KeyPress, ev));
   handle_keypress_event(ev, ev->xkey.window);
   D_RETURN_;
}

static void
feh_event_handle_ButtonPress(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER;
   /* hide the menus and get the heck out if it's a mouse-click on the
      cover */
   if (ev->xbutton.window == menu_cover)
   {
      feh_menu_hide(menu_root);
      D_RETURN_;
   }

   switch (ev->xbutton.button)
   {
     case 1:
        D(("Button 1 Press event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
           slideshow_change_image(winwid, SLIDE_NEXT);
        break;
     case 2:
        D(("Button 2 Press event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if (winwid != NULL)
        {
           if (ev->xbutton.state & ControlMask)
           {
              D(("Zoom mode baby!\n"));
              opt.mode = MODE_ZOOM;
              winwid->mode = MODE_ZOOM;
           }
           else
           {
              D(("Pan mode baby!\n"));
              opt.mode = MODE_PAN;
              winwid->mode = MODE_PAN;
           }
           D(("click offset is %d,%d\n", ev->xbutton.x, ev->xbutton.y));
           winwid->click_offset_x = ev->xbutton.x - winwid->im_x;
           winwid->click_offset_y = ev->xbutton.y - winwid->im_y;
        }
        break;
     case 3:
        D(("Button 3 Press event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if (winwid != NULL)
        {
           if (!opt.no_menus)
           {
              int x, y, b;
              unsigned int c;
              Window r;

              if (!menu_main)
                 feh_menu_init();
              if (winwid->type == WIN_TYPE_ABOUT)
              {
                 /* winwidget_destroy(winwid); */
                 XQueryPointer(disp, winwid->win, &r, &r, &x, &y, &b, &b, &c);
                 feh_menu_show_at_xy(menu_close, winwid, x, y);
              }
              else
              {
                 XQueryPointer(disp, winwid->win, &r, &r, &x, &y, &b, &b, &c);
                 feh_menu_show_at_xy(menu_main, winwid, x, y);
              }
           }
        }
        break;
     case 4:
        D(("Button 4 Press event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
           slideshow_change_image(winwid, SLIDE_PREV);
        break;
     case 5:
        D(("Button 5 Press event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
           slideshow_change_image(winwid, SLIDE_NEXT);
        break;
     default:
        D(("Received other ButtonPress event\n"));
        break;
   }
   D_RETURN_;
}

static void
feh_event_handle_ButtonRelease(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER;
   if (menu_root)
   {
      /* if menus are open, close them, and execute action if needed */

      if (ev->xbutton.window == menu_cover)
         feh_menu_hide(menu_root);
      else if (menu_root)
      {
         feh_menu *m;

         if (ev->xbutton.window == menu_cover)
            feh_menu_hide(menu_root);
         else if ((m = feh_menu_get_from_window(ev->xbutton.window)))
         {
            feh_menu_item *i = NULL;

            i = feh_menu_find_selected(m);
            /* watch out for this. I put it this way around so the menu
               goes away *before* we perform the action, if we start
               freeing menus on hiding, it will break ;-) */
            feh_menu_hide(menu_root);
            feh_main_iteration(0);
            if ((i) && (i->func))
               (i->func) (m, i, i->data);
         }
      }
      D_RETURN_;
   }
   switch (ev->xbutton.button)
   {
     case 1:
        D(("Button 1 Release event\n"));
        break;
     case 2:
        D(("Button 2 Release event\n"));
        winwid = winwidget_get_from_window(ev->xbutton.window);
        if (winwid != NULL)
        {
           D(("Disabling Pan/Zoom mode baby!\n"));
           opt.mode = MODE_NORMAL;
           winwid->mode = MODE_NORMAL;
        }
        break;
     case 3:
        D(("Button 3 Release event\n"));
        if (opt.no_menus)
           winwidget_destroy_all();
        break;
     default:
        break;
   }
   D_RETURN_;
}

static void
feh_event_handle_ConfigureNotify(XEvent * ev)
{
   D_ENTER;
   while (XCheckTypedWindowEvent
          (disp, ev->xconfigure.window, ConfigureNotify, ev));
   if (!menu_root)
   {
      winwidget w = winwidget_get_from_window(ev->xconfigure.window);

      if (w)
      {
         D(
           ("configure size %dx%d\n", ev->xconfigure.width,
            ev->xconfigure.height));
         if ((w->w != ev->xconfigure.width)
             || (w->h != ev->xconfigure.height))
         {
            D(("assigning size and rerendering\n"));
            w->w = ev->xconfigure.width;
            w->h = ev->xconfigure.height;
            w->had_resize = 1;
            winwidget_render_image(w, 0, 0);
         }
      }
   }

   D_RETURN_;
}

static void
feh_event_handle_EnterNotify(XEvent * ev)
{
   D_ENTER;
   D_RETURN_;
   ev = NULL;
}

static void
feh_event_handle_LeaveNotify(XEvent * ev)
{
   D_ENTER;
   if ((menu_root) && (ev->xcrossing.window == menu_root->win))
   {
      feh_menu_item *ii;

      D(("It is for a menu\n"));
      for (ii = menu_root->items; ii; ii = ii->next)
      {
         if (MENU_ITEM_IS_SELECTED(ii))
         {
            D(("Unselecting menu\n"));
            MENU_ITEM_SET_NORMAL(ii);
            menu_root->updates =
               imlib_update_append_rect(menu_root->updates, ii->x, ii->y,
                                        ii->w, ii->h);
            menu_root->needs_redraw = 1;
         }
      }
      feh_raise_all_menus();
   }

   D_RETURN_;
}

static void
feh_event_handle_MotionNotify(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER;
   if (menu_root)
   {
      feh_menu *m;
      feh_menu_item *selected_item, *mouseover_item;

      D(("motion notify with menus open\n"));
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));

      if (ev->xmotion.window == menu_cover)
      {
         D_RETURN_;
      }
      else if ((m = feh_menu_get_from_window(ev->xmotion.window)))
      {
         selected_item = feh_menu_find_selected(m);
         mouseover_item =
            feh_menu_find_at_xy(m, ev->xmotion.x, ev->xmotion.y);
         if (selected_item != mouseover_item)
         {
            D(("selecting a menu item\n"));
            if (selected_item)
               feh_menu_deselect_selected(m);
            if ((mouseover_item)
                && ((mouseover_item->func) || (mouseover_item->submenu)
                    || (mouseover_item->func_gen_sub)))
               feh_menu_select(m, mouseover_item);
         }
      }
   }
   else if (opt.mode == MODE_ZOOM)
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      printf("ZOOM - IMPLEMENT ME BITCH!\n");
   }
   else if (opt.mode == MODE_PAN)
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      winwid = winwidget_get_from_window(ev->xmotion.window);
      winwid->im_x = ev->xmotion.x - winwid->click_offset_x;
      winwid->im_y = ev->xmotion.y - winwid->click_offset_y;
      winwidget_render_image(winwid, 0, 0);
   }
   else
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      winwid = winwidget_get_from_window(ev->xmotion.window);
      if (winwid != NULL)
      {
         if (winwid->type == WIN_TYPE_ABOUT)
         {
            Imlib_Image im2, temp;

            im2 = feh_imlib_clone_image(winwid->im);
            imlib_context_set_image(im2);
            imlib_apply_filter("bump_map_point(x=[],y=[],map=" PREFIX
                               "/share/feh/images/about.png);",
                               &ev->xmotion.x, &ev->xmotion.y);
            temp = winwid->im;
            winwid->im = im2;
            winwidget_render_image(winwid, 0, 0);
            winwid->im = temp;
            feh_imlib_free_image_and_decache(im2);
         }
      }
   }

   D_RETURN_;
}

static void
feh_event_handle_ClientMessage(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER;
   if (ev->xclient.format == 32
       && ev->xclient.data.l[0] == (signed) wmDeleteWindow)
   {
      winwid = winwidget_get_from_window(ev->xclient.window);
      if (winwid)
         winwidget_destroy(winwid);
   }

   D_RETURN_;
}
