/* events.c

Copyright (C) 1999,2000 Tom Gilbert.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "feh.h"
#include "feh_list.h"
#include "filelist.h"
#include "winwidget.h"
#include "timers.h"
#include "options.h"
#include "events.h"
#include "thumbnail.h"

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

   D_ENTER(4);
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

   D_RETURN_(4);
}

static void
feh_event_handle_KeyPress(XEvent * ev)
{
   D_ENTER(4);
   while (XCheckTypedWindowEvent(disp, ev->xkey.window, KeyPress, ev));
   handle_keypress_event(ev, ev->xkey.window);
   D_RETURN_(4);
}

static void
feh_event_handle_ButtonPress(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER(4);
   /* hide the menus and get the heck out if it's a mouse-click on the
      cover */
   if (ev->xbutton.window == menu_cover)
   {
      feh_menu_hide(menu_root);
      D_RETURN_(4);
   }

   if (!opt.no_menus
       && ((ev->xbutton.button == opt.menu_button) || (opt.menu_button == 0))
       && (((!opt.menu_ctrl_mask) && (!(ev->xbutton.state & ControlMask)))
           || ((ev->xbutton.state & ControlMask) && (opt.menu_ctrl_mask))))
   {
      D(3, ("Menu Button Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         int x, y, b;
         unsigned int c;
         Window r;

         XQueryPointer(disp, winwid->win, &r, &r, &x, &y, &b, &b, &c);
         if (winwid->type == WIN_TYPE_ABOUT)
         {
            if (!menu_about_win)
               feh_menu_init_about_win();
            feh_menu_show_at_xy(menu_about_win, winwid, x, y);
         }
         else if (winwid->type == WIN_TYPE_SINGLE)
         {
            if (!menu_single_win)
               feh_menu_init_single_win();
            feh_menu_show_at_xy(menu_single_win, winwid, x, y);
         }
         else if (winwid->type == WIN_TYPE_THUMBNAIL)
         {
            if (!menu_thumbnail_win)
               feh_menu_init_thumbnail_win();
            feh_menu_show_at_xy(menu_thumbnail_win, winwid, x, y);
         }
         else if (winwid->type == WIN_TYPE_THUMBNAIL_VIEWER)
         {
            if (!menu_single_win)
               feh_menu_init_thumbnail_viewer();
            feh_menu_show_at_xy(menu_thumbnail_viewer, winwid, x, y);
         }
         else
         {
            if (!menu_main)
               feh_menu_init_main();
            feh_menu_show_at_xy(menu_main, winwid, x, y);
         }
      }
   }
   else if ((ev->xbutton.button == opt.rotate_button)
            && ((opt.no_rotate_ctrl_mask)
                || (ev->xbutton.state & ControlMask)))
   {
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         opt.mode = MODE_ROTATE;
         winwid->mode = MODE_ROTATE;
         D(3, ("rotate starting at %d, %d\n", ev->xbutton.x, ev->xbutton.y));
      }
   }
   else if ((ev->xbutton.button == opt.blur_button)
            && ((opt.no_blur_ctrl_mask) || (ev->xbutton.state & ControlMask)))
   {
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         opt.mode = MODE_BLUR;
         winwid->mode = MODE_BLUR;
         D(3, ("blur starting at %d, %d\n", ev->xbutton.x, ev->xbutton.y));
      }
   }
   else if (ev->xbutton.button == opt.next_button)
   {
      D(3, ("Next Button Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         D(3, ("Next button, but could be pan mode\n"));
         opt.mode = MODE_NEXT;
         winwid->mode = MODE_NEXT;
         D(3, ("click offset is %d,%d\n", ev->xbutton.x, ev->xbutton.y));
         winwid->click_offset_x = ev->xbutton.x - winwid->im_x;
         winwid->click_offset_y = ev->xbutton.y - winwid->im_y;
      }
   }
   else if (ev->xbutton.button == opt.zoom_button)
   {
      D(3, ("Zoom Button Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         D(3, ("Zoom mode baby!\n"));
         opt.mode = MODE_ZOOM;
         winwid->mode = MODE_ZOOM;
         D(3, ("click offset is %d,%d\n", ev->xbutton.x, ev->xbutton.y));
         winwid->click_offset_x = ev->xbutton.x - winwid->im_x;
         winwid->click_offset_y = ev->xbutton.y - winwid->im_y;
         winwid->im_click_offset_x = winwid->click_offset_x / winwid->zoom;
         winwid->im_click_offset_y = winwid->click_offset_y / winwid->zoom;
         winwid->zoom = 1.0;
         if (opt.full_screen)
         {
            winwid->im_x = (scr->width - winwid->im_w) >> 1;
            winwid->im_y = (scr->height - winwid->im_h) >> 1;
         }
         else if (opt.geom)
         {
            winwid->im_x = (opt.geom_w - winwid->im_w) >> 1;
            winwid->im_y = (opt.geom_h - winwid->im_h) >> 1;
         }
         else
         {
            winwid->im_x = 0;
            winwid->im_y = 0;
         }
         if (winwid->im_click_offset_x < 0)
            winwid->im_click_offset_x = 0;
         if (winwid->im_click_offset_y < 0)
            winwid->im_click_offset_y = 0;
         if (winwid->im_click_offset_x > winwid->im_w)
            winwid->im_click_offset_x = winwid->im_w;
         if (winwid->im_click_offset_y > winwid->im_h)
            winwid->im_click_offset_y = winwid->im_h;
         if (winwid->click_offset_x < 0)
            winwid->click_offset_x = 0;
         if (winwid->click_offset_y < 0)
            winwid->click_offset_y = 0;
         if (winwid->click_offset_x > winwid->im_w)
            winwid->click_offset_x = winwid->im_w;
         if (winwid->click_offset_y > winwid->im_h)
            winwid->click_offset_y = winwid->im_h;
         winwidget_render_image(winwid, 0, 0);
      }
   }
   else if (ev->xbutton.button == opt.reload_button)
   {
      D(3, ("Reload Button Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
         feh_reload_image(winwid, 0);
   }
   else if (ev->xbutton.button == 4 /* this is bad */ )
   {
      D(3, ("Button 4 Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
         slideshow_change_image(winwid, SLIDE_PREV);
   }
   else if (ev->xbutton.button == 5 /* this is bad */ )
   {
      D(3, ("Button 5 Press event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
         slideshow_change_image(winwid, SLIDE_NEXT);
   }
   else
   {
      D(3, ("Received other ButtonPress event\n"));
   }
   D_RETURN_(4);
}

static void
feh_event_handle_ButtonRelease(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER(4);
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
      D_RETURN_(4);
   }

   if ((ev->xbutton.button == opt.menu_button)
       &&
       (((!opt.menu_ctrl_mask)
         && ((!(ev->xbutton.state & ControlMask))
             || ((ev->xbutton.state & ControlMask) && (opt.menu_ctrl_mask)))))
       && (opt.no_menus))
      winwidget_destroy_all();
   else if (ev->xbutton.button == opt.next_button)
   {
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (opt.mode == MODE_PAN)
      {
         if (winwid != NULL)
         {
            D(3, ("Disabling pan mode\n"));
            opt.mode = MODE_NORMAL;
            winwid->mode = MODE_NORMAL;
            winwidget_sanitise_offsets(winwid);
            winwidget_render_image(winwid, 0, 1);
         }
      }
      else
      {
         opt.mode = MODE_NORMAL;
         winwid->mode = MODE_NORMAL;
         if ((winwid != NULL) && (winwid->type == WIN_TYPE_SLIDESHOW))
         {
            slideshow_change_image(winwid, SLIDE_NEXT);
         }
         else if ((winwid != NULL) && (winwid->type == WIN_TYPE_THUMBNAIL))
         {
            feh_file *thumbfile;
            winwidget thumbwin = NULL;
            int x, y;
            char *s;

            x = ev->xbutton.x;
            y = ev->xbutton.y;
            x -= winwid->im_x;
            y -= winwid->im_y;
            x /= winwid->zoom;
            y /= winwid->zoom;
            thumbfile = feh_thumbnail_get_file_from_coords(x, y);
            if (thumbfile)
            {
               if (!opt.thumb_title)
                  s = thumbfile->name;
               else
                  s = feh_printf(opt.thumb_title, thumbfile);
               thumbwin =
                  winwidget_get_first_window_of_type
                  (WIN_TYPE_THUMBNAIL_VIEWER);
               if (!thumbwin)
               {
                  thumbwin =
                     winwidget_create_from_file(feh_list_add_front
                                                (NULL, thumbfile), s,
                                                WIN_TYPE_THUMBNAIL_VIEWER);
                  winwidget_show(thumbwin);
               }
               else if (FEH_FILE(thumbwin->file->data) != thumbfile)
               {
                  free(thumbwin->file);
                  thumbwin->file = feh_list_add_front(NULL, thumbfile);
                  winwidget_rename(thumbwin, s);
                  feh_reload_image(thumbwin, 1);
               }
            }
         }
      }
   }
   else if ((ev->xbutton.button == opt.rotate_button)
            || (ev->xbutton.button == opt.zoom_button))
   {
      D(3, ("Mode-based Button Release event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         D(3, ("Disabling mode\n"));
         opt.mode = MODE_NORMAL;
         winwid->mode = MODE_NORMAL;
         winwidget_sanitise_offsets(winwid);
         winwidget_render_image(winwid, 0, 1);
      }
   }
   else if ((ev->xbutton.button == opt.blur_button)
            && ((opt.no_blur_ctrl_mask) || (ev->xbutton.state & ControlMask)))
   {
      D(3, ("Blur Button Release event\n"));
      winwid = winwidget_get_from_window(ev->xbutton.window);
      if (winwid != NULL)
      {
         D(3, ("Disabling Blur mode\n"));
         opt.mode = MODE_NORMAL;
         winwid->mode = MODE_NORMAL;
      }
   }
   D_RETURN_(4);
}

static void
feh_event_handle_ConfigureNotify(XEvent * ev)
{
   D_ENTER(4);
   while (XCheckTypedWindowEvent
          (disp, ev->xconfigure.window, ConfigureNotify, ev));
   if (!menu_root)
   {
      winwidget w = winwidget_get_from_window(ev->xconfigure.window);

      if (w)
      {
         D(3,
           ("configure size %dx%d\n", ev->xconfigure.width,
            ev->xconfigure.height));
         if ((w->w != ev->xconfigure.width)
             || (w->h != ev->xconfigure.height))
         {
            D(3, ("assigning size and rerendering\n"));
            w->w = ev->xconfigure.width;
            w->h = ev->xconfigure.height;
            w->had_resize = 1;
            winwidget_render_image(w, 0, 1);
         }
      }
   }

   D_RETURN_(4);
}

static void
feh_event_handle_EnterNotify(XEvent * ev)
{
   D_ENTER(4);
   D_RETURN_(4);
   ev = NULL;
}

static void
feh_event_handle_LeaveNotify(XEvent * ev)
{
   D_ENTER(4);
   if ((menu_root) && (ev->xcrossing.window == menu_root->win))
   {
      feh_menu_item *ii;

      D(4, ("It is for a menu\n"));
      for (ii = menu_root->items; ii; ii = ii->next)
      {
         if (MENU_ITEM_IS_SELECTED(ii))
         {
            D(4, ("Unselecting menu\n"));
            MENU_ITEM_SET_NORMAL(ii);
            menu_root->updates =
               imlib_update_append_rect(menu_root->updates, ii->x, ii->y,
                                        ii->w, ii->h);
            menu_root->needs_redraw = 1;
         }
      }
      feh_raise_all_menus();
   }

   D_RETURN_(4);
}

static void
feh_event_handle_MotionNotify(XEvent * ev)
{
   winwidget winwid = NULL;
   int dx, dy;

   D_ENTER(5);
   if (menu_root)
   {
      feh_menu *m;
      feh_menu_item *selected_item, *mouseover_item;

      D(3, ("motion notify with menus open\n"));
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));

      if (ev->xmotion.window == menu_cover)
      {
         D_RETURN_(5);
      }
      else if ((m = feh_menu_get_from_window(ev->xmotion.window)))
      {
         selected_item = feh_menu_find_selected(m);
         mouseover_item =
            feh_menu_find_at_xy(m, ev->xmotion.x, ev->xmotion.y);
	 
	 if (selected_item != mouseover_item)
         {
            D(4, ("selecting a menu item\n"));
            if (selected_item)
               feh_menu_deselect_selected(m);
            if ((mouseover_item)
                && ((mouseover_item->func) || (mouseover_item->submenu)
                    || (mouseover_item->func_gen_sub)))
               feh_menu_select(m, mouseover_item);
         }
	 /* check if we are close to the right and/or the bottom edge of the
	  * screen. If so, and if the menu we are currently over is partially
	  * hidden, slide the menu to the left and/or up until it is
	  * fully visible */
	 if ( mouseover_item && (
	      (scr->width - (ev->xmotion.x + m->x)) < 50 ||
              (scr->height -(ev->xmotion.y + m->y)) < 50)
	    )
	 {
	    dx = scr->width - (m->x + m->w);
	    dy = scr->height - (m->y + m->h);
	    dx = dx<0?dx:0;
	    dy = dy<0?dy:0;
	    if (dx || dy)
	       feh_menu_slide_all_menus_relative(dx, dy);
	 }
	 /* if a submenu is open we want to see that also */
	 if (mouseover_item && m->next && (
	        (scr->width - (ev->xmotion.x + m->next->x)) < 50 ||
		(scr->height -(ev->xmotion.y + m->next->y)) < 50)
	    )
	 {
	    dx = scr->width - (m->next->x + m->next->w);
	    dy = scr->height - (m->next->y + m->next->h);
	    dx = dx<0?dx:0;
	    dy = dy<0?dy:0;
	    if (dx || dy) 
	       feh_menu_slide_all_menus_relative(dx, dy);
	 }
      }
   }
   else if (opt.mode == MODE_ZOOM)
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));

      winwid = winwidget_get_from_window(ev->xmotion.window);
      if (winwid)
      {
         winwid->zoom =
            ((double) ev->xmotion.x - (double) winwid->click_offset_x) / 64.0;
         if (winwid->zoom < 0)
            winwid->zoom =
               1.0 +
               ((winwid->zoom * 64.0) /
                ((double) (winwid->click_offset_x + 1)));
         else
            winwid->zoom += 1.0;

         if (winwid->zoom < 0.01)
            winwid->zoom = 0.01;

         /* calculate change in zoom and move im_x and im_y respectively to
            enable zooming to the clicked spot... */
         /* for now, center around im_click_offset_x and im_click_offset_y */
         winwid->im_x =
            (winwid->w / 2) - (winwid->im_click_offset_x * winwid->zoom);
         winwid->im_y =
            (winwid->h / 2) - (winwid->im_click_offset_y * winwid->zoom);

         winwidget_render_image(winwid, 0, 0);
      }
   }
   else if ((opt.mode == MODE_PAN) || (opt.mode == MODE_NEXT))
   {
      int orig_x, orig_y;

      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      winwid = winwidget_get_from_window(ev->xmotion.window);
      if (winwid)
      {
         if (opt.mode = MODE_NEXT)
         {
            opt.mode = MODE_PAN;
            winwid->mode = MODE_PAN;
         }
         D(5, ("Panning\n"));
         orig_x = winwid->im_x;
         orig_y = winwid->im_y;

         winwid->im_x = ev->xmotion.x - winwid->click_offset_x;
         winwid->im_y = ev->xmotion.y - winwid->click_offset_y;

         winwidget_sanitise_offsets(winwid);

         if ((winwid->im_x != orig_x) || (winwid->im_y != orig_y))
            winwidget_render_image(winwid, 0, 0);
      }
   }
   else if (opt.mode == MODE_ROTATE)
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      winwid = winwidget_get_from_window(ev->xmotion.window);
      if (winwid)
      {
         D(5, ("Rotating\n"));
         if (!winwid->has_rotated)
         {
            Imlib_Image temp;

            temp = feh_imlib_create_rotated_image(winwid->im, 0.0);
            winwid->im_w = feh_imlib_image_get_width(temp);
            winwid->im_h = feh_imlib_image_get_height(temp);
            feh_imlib_free_image_and_decache(temp);
            if (!opt.full_screen && !opt.geom)
               winwidget_resize(winwid, winwid->im_w, winwid->im_h);
            winwid->has_rotated = 1;
         }
         winwid->im_angle =
            (ev->xmotion.x -
             winwid->w / 2) / ((double) winwid->w / 2) * 3.1415926535;
         D(5, ("angle: %f\n", winwid->im_angle));
         winwidget_render_image(winwid, 0, 0);
      }
   }
   else if (opt.mode == MODE_BLUR)
   {
      while (XCheckTypedWindowEvent
             (disp, ev->xmotion.window, MotionNotify, ev));
      winwid = winwidget_get_from_window(ev->xmotion.window);
      if (winwid)
      {
         Imlib_Image temp, ptr;
         signed int blur_radius;

         D(5, ("Blurring\n"));

         temp = feh_imlib_clone_image(winwid->im);
         blur_radius = (((double) ev->xmotion.x / winwid->w) * 20) - 10;
         D(5, ("angle: %d\n", blur_radius));
         if (blur_radius > 0)
            feh_imlib_image_sharpen(temp, blur_radius);
         else
            feh_imlib_image_blur(temp, 0 - blur_radius);
         ptr = winwid->im;
         winwid->im = temp;
         winwidget_render_image(winwid, 0, 0);
         feh_imlib_free_image_and_decache(winwid->im);
         winwid->im = ptr;
      }
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
            Imlib_Image orig_im;
            int x, y;

            x = ev->xmotion.x - winwid->im_x;
            y = ev->xmotion.y - winwid->im_y;
            orig_im = winwid->im;
            winwid->im = feh_imlib_clone_image(orig_im);
            imlib_context_set_image(winwid->im);
            imlib_apply_filter("bump_map_point(x=[],y=[],map=" PREFIX
                               "/share/feh/images/about.png);", &x, &y);
            winwidget_render_image(winwid, 0, 1);
            feh_imlib_free_image_and_decache(winwid->im);
            winwid->im = orig_im;
         }
         else if (winwid->type == WIN_TYPE_THUMBNAIL)
         {
            static feh_thumbnail *last_thumb = NULL;
            feh_thumbnail *thumbnail;
            int x, y;

            x = (ev->xbutton.x - winwid->im_x) / winwid->zoom;
            y = (ev->xbutton.y - winwid->im_y) / winwid->zoom;
            thumbnail = feh_thumbnail_get_thumbnail_from_coords(x, y);
            if (thumbnail != last_thumb)
            {
               if (thumbnail)
               {
                  Imlib_Image origwin;

                  origwin = winwid->im;
                  winwid->im = feh_imlib_clone_image(origwin);
                  feh_imlib_image_fill_rectangle(winwid->im, thumbnail->x,
                                                 thumbnail->y, thumbnail->w,
                                                 thumbnail->h, 50, 50, 255,
                                                 100);
                  feh_imlib_image_draw_rectangle(winwid->im, thumbnail->x,
                                                 thumbnail->y, thumbnail->w,
                                                 thumbnail->h, 255, 255, 255,
                                                 255);
                  feh_imlib_image_draw_rectangle(winwid->im, thumbnail->x + 1,
                                                 thumbnail->y + 1,
                                                 thumbnail->w - 2,
                                                 thumbnail->h - 2, 0, 0, 0,
                                                 255);
                  feh_imlib_image_draw_rectangle(winwid->im, thumbnail->x + 2,
                                                 thumbnail->y + 2,
                                                 thumbnail->w - 4,
                                                 thumbnail->h - 4, 255, 255,
                                                 255, 255);
                  winwidget_render_image(winwid, 0, 1);
                  feh_imlib_free_image_and_decache(winwid->im);
                  winwid->im = origwin;
               }
               else
                  winwidget_render_image(winwid, 0, 1);
            }
            last_thumb = thumbnail;
         }
      }
   }
   D_RETURN_(5);
}

static void
feh_event_handle_ClientMessage(XEvent * ev)
{
   winwidget winwid = NULL;

   D_ENTER(4);
   if (ev->xclient.format == 32
       && ev->xclient.data.l[0] == (signed) wmDeleteWindow)
   {
      winwid = winwidget_get_from_window(ev->xclient.window);
      if (winwid)
         winwidget_destroy(winwid);
   }

   D_RETURN_(4);
}
