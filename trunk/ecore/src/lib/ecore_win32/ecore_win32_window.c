/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define _WIN32_WINNT 0x0500  // For WS_EX_LAYERED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


/***** Private declarations *****/


typedef enum _Ecore_Win32_Window_Z_Order Ecore_Win32_Window_Z_Order;
enum _Ecore_Win32_Window_Z_Order
{
  ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM,
  ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST,
  ECORE_WIN32_WINDOW_Z_ORDER_TOP,
  ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST
};

static Ecore_Win32_Window *ecore_win32_window_internal_new(Ecore_Win32_Window *parent,
                                                           int                 x,
                                                           int                 y,
                                                           int                 width,
                                                           int                 height,
                                                           DWORD               style);


/***** API *****/

Ecore_Win32_Window *
ecore_win32_window_new(Ecore_Win32_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   INF("creating window with border");

   return ecore_win32_window_internal_new(parent,
                                          x, y,
                                          width, height,
                                          WS_OVERLAPPEDWINDOW | WS_SIZEBOX);
}

/* simulate X11 override windows */
Ecore_Win32_Window *
ecore_win32_window_override_new(Ecore_Win32_Window *parent,
                                int                 x,
                                int                 y,
                                int                 width,
                                int                 height)
{
   INF("creating window without border");

   return ecore_win32_window_internal_new(parent,
                                          x, y,
                                          width, height,
                                          WS_POPUP);
}

void
ecore_win32_window_free(Ecore_Win32_Window *window)
{
   struct _Ecore_Win32_Window *wnd = window;

   if (!window) return;

   INF("destroying window");

   if (wnd->shape.mask != NULL)
      free(wnd->shape.mask);

   DestroyWindow(((struct _Ecore_Win32_Window *)window)->window);
   free(window);
}

void *
ecore_win32_window_hwnd_get(Ecore_Win32_Window *window)
{
   if (!window) return NULL;

   return ((struct _Ecore_Win32_Window *)window)->window;
}

/*
void
ecore_win32_window_configure(Ecore_Win32_Window        *window,
                             Ecore_Win32_Window_Z_Order order,
                             int                        x,
                             int                        y,
                             int                        width,
                             int                        height)
{
  HWND w;

  switch (order)
    {
    case ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM:
      w = HWND_BOTTOM;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST:
      w = HWND_NOTOPMOST;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_TOP:
      w = HWND_TOP;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST:
      w = HWND_TOPMOST;
      break;
    default:
      return;
    }
  SetWindowPos((struct _Ecore_Win32_Window *)window->window, w, x, y, width, height, ???);
}
*/

void
ecore_win32_window_move(Ecore_Win32_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   if (!window) return;

   INF("moving window (%dx%d)", x, y);

   w = ((struct _Ecore_Win32_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     {
        ERR("GetWindowRect() failed");
        return;
     }

   if (!MoveWindow(w, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   EINA_TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

void
ecore_win32_window_resize(Ecore_Win32_Window *window,
                          int                 width,
                          int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;
   int                         x;
   int                         y;

   if (!window) return;

   INF("resizing window (%dx%d)", width, height);

   w = (struct _Ecore_Win32_Window *)window;
   if (!GetWindowRect(w->window, &rect))
     {
        ERR("GetWindowRect() failed");
        return;
     }

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
/*    if (width < w->min_width) width = w->min_width; */
/*    if (width > w->max_width) width = w->max_width; */
/*    printf ("ecore_win32_window_resize 1 : %d %d %d\n", w->min_height, w->max_height, height); */
/*    if (height < w->min_height) height = w->min_height; */
/*    printf ("ecore_win32_window_resize 2 : %d %d\n", w->max_height, height); */
/*    if (height > w->max_height) height = w->max_height; */
/*    printf ("ecore_win32_window_resize 3 : %d %d\n", w->max_height, height); */
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRect(&rect, style, EINA_FALSE))
     {
        ERR("AdjustWindowRect() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   EINA_TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

void
ecore_win32_window_move_resize(Ecore_Win32_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;

   if (!window) return;

   INF("moving and resizing window (%dx%d %dx%d)", x, y, width, height);

   w = ((struct _Ecore_Win32_Window *)window);
   rect.left = 0;
   rect.top = 0;
   if ((unsigned int)width < w->min_width) width = w->min_width;
   if ((unsigned int)width > w->max_width) width = w->max_width;
   if ((unsigned int)height < w->min_height) height = w->min_height;
   if ((unsigned int)height > w->max_height) height = w->max_height;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRect(&rect, style, EINA_FALSE))
     {
        ERR("AdjustWindowRect() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   EINA_TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

void
ecore_win32_window_geometry_get(Ecore_Win32_Window *window,
                                int                *x,
                                int                *y,
                                int                *width,
                                int                *height)
{
   RECT rect;
   int  w;
   int  h;

   INF("getting window geometry");

   if (!window)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        ERR("GetClientRect() failed");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   w = rect.right - rect.left;
   h = rect.bottom - rect.top;

   if (!GetWindowRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        ERR("GetWindowRect() failed");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   if (x) *x = rect.left;
   if (y) *y = rect.top;
   if (width) *width = w;
   if (height) *height = h;
}

void
ecore_win32_window_size_get(Ecore_Win32_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   INF("getting window size");

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        ERR("GetClientRect() failed");

        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

void
ecore_win32_window_size_min_set(Ecore_Win32_Window *window,
                                unsigned int        min_width,
                                unsigned int        min_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   printf ("ecore_win32_window_size_min_set : %p  %d %d\n", window, min_width, min_height);
   w = (struct _Ecore_Win32_Window *)window;
   w->min_width = min_width;
   w->min_height = min_height;
}

void
ecore_win32_window_size_min_get(Ecore_Win32_Window *window,
                                unsigned int       *min_width,
                                unsigned int       *min_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_min_get : %p  %d %d\n", window, w->min_width, w->min_height);
   if (min_width) *min_width = w->min_width;
   if (min_height) *min_height = w->min_height;
}

void
ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                unsigned int        max_width,
                                unsigned int        max_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   printf ("ecore_win32_window_size_max_set : %p  %d %d\n", window, max_width, max_height);
   w = (struct _Ecore_Win32_Window *)window;
   w->max_width = max_width;
   w->max_height = max_height;
}

void
ecore_win32_window_size_max_get(Ecore_Win32_Window *window,
                                unsigned int       *max_width,
                                unsigned int       *max_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_max_get : %p  %d %d\n", window, w->max_width, w->max_height);
   if (max_width) *max_width = w->max_width;
   if (max_height) *max_height = w->max_height;
}

void
ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                 unsigned int        base_width,
                                 unsigned int        base_height)
{
   struct _Ecore_Win32_Window *w;

   printf ("ecore_win32_window_size_base_set : %p  %d %d\n", window, base_width, base_height);
   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->base_width = base_width;
   w->base_height = base_height;
}

void
ecore_win32_window_size_base_get(Ecore_Win32_Window *window,
                                 unsigned int       *base_width,
                                 unsigned int       *base_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_base_get : %p  %d %d\n", window, w->base_width, w->base_height);
   if (base_width) *base_width = w->base_width;
   if (base_height) *base_height = w->base_height;
}

void
ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                 unsigned int        step_width,
                                 unsigned int        step_height)
{
   struct _Ecore_Win32_Window *w;

   printf ("ecore_win32_window_size_step_set : %p  %d %d\n", window, step_width, step_height);
   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->step_width = step_width;
   w->step_height = step_height;
}

void
ecore_win32_window_size_step_get(Ecore_Win32_Window *window,
                                 unsigned int       *step_width,
                                 unsigned int       *step_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_step_get : %p  %d %d\n", window, w->step_width, w->step_height);
   if (step_width) *step_width = w->step_width;
   if (step_height) *step_height = w->step_height;
}

void
ecore_win32_window_shape_set(Ecore_Win32_Window *window,
                             unsigned short      width,
                             unsigned short      height,
                             unsigned char      *mask)
{
   struct _Ecore_Win32_Window *wnd;
   HRGN                        rgn;
   int                         x;
   int                         y;
   OSVERSIONINFO               version_info;

   if (window == NULL)
      return;

   wnd = (struct _Ecore_Win32_Window *)window;

   if (mask == NULL)
     {
        wnd->shape.enabled = 0;
        if (wnd->shape.layered != 0)
          {
             wnd->shape.layered = 0;
#if defined(WS_EX_LAYERED)
             SetLastError(0);
             if (!SetWindowLongPtr(wnd->window, GWL_EXSTYLE,
                                   GetWindowLong(wnd->window, GWL_EXSTYLE) & (~WS_EX_LAYERED)) &&
                 (GetLastError() != 0))
               {
                  ERR("SetWindowLongPtr() failed");
                  return;
               }
             if (!RedrawWindow(wnd->window, NULL, NULL,
                               RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN))
               {
                  ERR("RedrawWindow() failed");
                  return;
               }
#endif
          }
        else
          if (!SetWindowRgn(wnd->window, NULL, EINA_TRUE))
            {
               ERR("SetWindowRgn() failed");
            }
        return;
     }

   if (width == 0 || height == 0)
     return;

   wnd->shape.enabled = 1;

   if (width != wnd->shape.width || height != wnd->shape.height)
     {
       wnd->shape.width = width;
       wnd->shape.height = height;
       if (wnd->shape.mask != NULL)
         {
           free(wnd->shape.mask);
           wnd->shape.mask = NULL;
         }
       wnd->shape.mask = malloc(width * height);
     }
   memcpy(wnd->shape.mask, mask, width * height);

   wnd->shape.layered = 0;

#if defined(WS_EX_LAYERED)
   version_info.dwOSVersionInfoSize = sizeof(version_info);
   if (GetVersionEx(&version_info) == EINA_TRUE && version_info.dwMajorVersion == 5)
     {
       SetLastError(0);
       if (!SetWindowLongPtr(wnd->window, GWL_EXSTYLE,
                             GetWindowLong(wnd->window, GWL_EXSTYLE) | WS_EX_LAYERED) &&
           (GetLastError() != 0))
            {
               ERR("SetWindowLongPtr() failed");
               return;
            }
       wnd->shape.layered = 1;
       return;
     }
#endif

   if (!(rgn = CreateRectRgn(0, 0, 0, 0)))
     {
        ERR("CreateRectRgn() failed");
        return;
     }
   for (y = 0; y < height; y++)
     {
        HRGN rgnLine;

        if (!(rgnLine = CreateRectRgn(0, 0, 0, 0)))
          {
             ERR("CreateRectRgn() failed");
             return;
          }
        for (x = 0; x < width; x++)
          {
             if (mask[y * width + x] > 0)
               {
                  HRGN rgnDot;

                  if (!(rgnDot = CreateRectRgn(x, y, x + 1, y + 1)))
                    {
                       ERR("CreateRectRgn() failed");
                       return;
                    }
                  if (CombineRgn(rgnLine, rgnLine, rgnDot, RGN_OR) == ERROR)
                    {
                       ERR("CombineRgn() has not created a new region");
                    }
                  if (!DeleteObject(rgnDot))
                    {
                       ERR("DeleteObject() failed");
                       return;
                    }
               }
          }
        if (CombineRgn(rgn, rgn, rgnLine, RGN_OR) == ERROR)
          {
             ERR("CombineRgn() has not created a new region");
          }
        if (!DeleteObject(rgnLine))
          {
             ERR("DeleteObject() failed");
             return;
          }
     }
   if (!SetWindowRgn(wnd->window, rgn, EINA_TRUE))
     {
        ERR("SetWindowRgn() failed");
     }
}

void
ecore_win32_window_show(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("showing window");

   ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_SHOWNORMAL);
   if (!UpdateWindow(((struct _Ecore_Win32_Window *)window)->window))
     {
        ERR("UpdateWindow() failed");
     }
}

/* FIXME: seems to block the taskbar */
void
ecore_win32_window_hide(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("hiding window");

   ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_HIDE);
}

void
ecore_win32_window_raise(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("raising window");

   if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                     HWND_TOP, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE))
     {
        ERR("SetWindowPos() failed");
     }
}

void
ecore_win32_window_lower(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("lowering window");

   if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                     HWND_BOTTOM, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE))
     {
        ERR("SetWindowPos() failed");
     }
}

void
ecore_win32_window_title_set(Ecore_Win32_Window *window,
                             const char         *title)
{
   if (!window) return;

   if (!title || !title[0]) return;

   INF("setting window title");

   if (!SetWindowText(((struct _Ecore_Win32_Window *)window)->window, title))
     {
        ERR("SetWindowText() failed");
     }
}

void
ecore_win32_window_focus_set(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("focusing window");

   if (!SetFocus(((struct _Ecore_Win32_Window *)window)->window))
     {
        ERR("SetFocus() failed");
     }
}

void
ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                 int                 on)
{
   struct _Ecore_Win32_Window *ew;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->iconified) && (on)) ||
       ((!ew->iconified) && (!on)))
     return;

   INF("iconifying window: %s", on ? "yes" : "no");

   ShowWindow(ew->window, on ? SW_MINIMIZE : SW_RESTORE);
   ew->iconified = on;
}

void
ecore_win32_window_borderless_set(Ecore_Win32_Window *window,
                                  int                 on)
{
   RECT                        rect;
   DWORD                       style;
   struct _Ecore_Win32_Window *ew;
   HWND                        w;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->borderless) && (on)) ||
       ((!ew->borderless) && (!on)))
     return;

   INF("setting window without border: %s", on ? "yes" : "no");

   w = ew->window;

   style = GetWindowLong(w, GWL_STYLE);
   if (on)
     {
        if (!GetClientRect(w, &rect))
          {
             ERR("GetClientRect() failed");
             return;
          }
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_STYLE, style & ~(WS_CAPTION | WS_THICKFRAME)) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
     }
   else
     {
        if (!GetWindowRect(w, &rect))
          {
             ERR("GetWindowRect() failed");
             return;
          }
        style |= WS_CAPTION | WS_THICKFRAME;
        if (!AdjustWindowRect (&rect, style, EINA_FALSE))
          {
             ERR("AdjustWindowRect() failed");
             return;
          }
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_STYLE, style) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
     }
   if (!SetWindowPos(w, HWND_TOPMOST,
                     rect.left, rect.top,
                     rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOMOVE | SWP_FRAMECHANGED))
     {
        ERR("SetWindowPos() failed");
        return;
     }
   ew->borderless = on;
}

void
ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                  int                 on)
{
   struct _Ecore_Win32_Window *ew;
   HWND                        w;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   INF("setting fullscreen: %s", on ? "yes" : "no");

   ew->fullscreen = !!on;
   w = ew->window;

   if (on)
     {
        DWORD style;

        if (!GetWindowRect(w, &ew->rect))
          {
             ERR("GetWindowRect() failed");
             return;
          }
        if (!(ew->style = GetWindowLong(w, GWL_STYLE)))
          {
             ERR("GetWindowLong() failed");
             return;
          }
        style = ew->style & ~WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
        style |= WS_VISIBLE | WS_POPUP;
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_STYLE, style) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_EXSTYLE, WS_EX_TOPMOST) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
        if (!SetWindowPos(w, HWND_TOPMOST, 0, 0,
                          GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN),
                          SWP_NOCOPYBITS | SWP_SHOWWINDOW))
          {
             ERR("SetWindowPos() failed");
             return;
          }
     }
   else
     {
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_STYLE, ew->style) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
        SetLastError(0);
        if (!SetWindowLongPtr(w, GWL_EXSTYLE, 0) && (GetLastError() != 0))
          {
             ERR("SetWindowLongPtr() failed");
             return;
          }
        if (!SetWindowPos(w, HWND_NOTOPMOST,
                          ew->rect.left,
                          ew->rect.top,
                          ew->rect.right - ew->rect.left,
                          ew->rect.bottom - ew->rect.top,
                          SWP_NOCOPYBITS | SWP_SHOWWINDOW))
          {
             ERR("SetWindowPos() failed");
             return;
          }
     }
}

void
ecore_win32_window_cursor_set(Ecore_Win32_Window *window,
                              Ecore_Win32_Cursor *cursor)
{
   INF("setting cursor");

   if (!SetClassLong(((struct _Ecore_Win32_Window *)window)->window,
                     GCL_HCURSOR, (LONG)cursor))
     {
        ERR("SetClassLong() failed");
     }
}

void
ecore_win32_window_state_set(Ecore_Win32_Window       *window,
                             Ecore_Win32_Window_State *state,
                             unsigned int              num)
{
   unsigned int i;

   if (!window || !state || !num)
     return;

   INF("setting cursor state");

   for (i = 0; i < num; i++)
     {
        switch (state[i])
          {
          case ECORE_WIN32_WINDOW_STATE_ICONIFIED:
            ((struct _Ecore_Win32_Window *)window)->state.iconified = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MODAL:
            ((struct _Ecore_Win32_Window *)window)->state.modal = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_STICKY:
            ((struct _Ecore_Win32_Window *)window)->state.sticky = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_vert = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_horz = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_horz = 1;
            ((struct _Ecore_Win32_Window *)window)->state.maximized_vert = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_SHADED:
            ((struct _Ecore_Win32_Window *)window)->state.shaded = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_HIDDEN:
            ((struct _Ecore_Win32_Window *)window)->state.hidden = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_FULLSCREEN:
            ((struct _Ecore_Win32_Window *)window)->state.fullscreen = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_ABOVE:
            ((struct _Ecore_Win32_Window *)window)->state.above = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_BELOW:
            ((struct _Ecore_Win32_Window *)window)->state.below = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION:
            ((struct _Ecore_Win32_Window *)window)->state.demands_attention = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_UNKNOWN:
            /* nothing to be done */
            break;
          }
     }
}

void
ecore_win32_window_state_request_send(Ecore_Win32_Window      *window,
                                      Ecore_Win32_Window_State state,
                                      unsigned int             set)
{
   struct _Ecore_Win32_Window *ew;
   HWND                        w;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   w = ew->window;

   INF("sending cursor state");

   switch (state)
     {
      case ECORE_WIN32_WINDOW_STATE_ICONIFIED:
         if (ew->state.iconified)
           ecore_win32_window_iconified_set(window, set);
         break;
      case ECORE_WIN32_WINDOW_STATE_MODAL:
         ew->state.modal = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_STICKY:
         ew->state.sticky = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT:
         if (ew->state.maximized_vert)
           {
              RECT rect;
              int  y;
              int  height;

              if (!SystemParametersInfo(SPI_GETWORKAREA, 0,
                                        &rect, 0))
                {
                   ERR("SystemParametersInfo() failed");
                   break;
                }
              y = rect.top;
              height = rect.bottom - rect.top;

              if (!GetClientRect(w, &rect))
                {
                   ERR("GetClientRect() failed");
                   break;
                }

              if (!MoveWindow(w, rect.left, y,
                              rect.right - rect.left,
                              height,
                              EINA_TRUE))
                {
                   ERR("MoveWindow() failed");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ:
         if (ew->state.maximized_horz)
           {
              RECT rect;

              if (!GetClientRect(w, &rect))
                {
                   ERR("GetClientRect() failed");
                   break;
                }

              if (!MoveWindow(w, 0, rect.top,
                              GetSystemMetrics(SM_CXSCREEN),
                              rect.bottom - rect.top,
                              EINA_TRUE))
                {
                   ERR("MoveWindow() failed");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED:
         if (ew->state.maximized_vert && ew->state.maximized_horz)
           {
              RECT rect;

              if (!SystemParametersInfo(SPI_GETWORKAREA, 0,
                                        &rect, 0))
                {
                   ERR("SystemParametersInfo() failed");
                   break;
                }

              if (!MoveWindow(w, 0, 0,
                              GetSystemMetrics(SM_CXSCREEN),
                              rect.bottom - rect.top,
                              EINA_TRUE))
                {
                   ERR("MoveWindow() failed");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_SHADED:
         ew->state.shaded = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_HIDDEN:
         ew->state.hidden = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_FULLSCREEN:
         if (ew->state.fullscreen)
           ecore_win32_window_fullscreen_set(window, set);
         break;
      case ECORE_WIN32_WINDOW_STATE_ABOVE:
         if (ew->state.above)
           if (!SetWindowPos(w, HWND_TOP,
                             0, 0,
                             0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW))
             {
                ERR("SetWindowPos() failed");
             }
         break;
      case ECORE_WIN32_WINDOW_STATE_BELOW:
         if (ew->state.below)
           if (!SetWindowPos(w, HWND_BOTTOM,
                             0, 0,
                             0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW))
             {
                ERR("SetWindowPos() failed");
             }
         break;
      case ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION:
         ew->state.demands_attention = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_UNKNOWN:
         /* nothing to be done */
         break;
     }
}

void
ecore_win32_window_type_set(Ecore_Win32_Window      *window,
                            Ecore_Win32_Window_Type  type)
{
   if (!window)
     return;

   INF("setting window type");

   switch (type)
     {
     case ECORE_WIN32_WINDOW_TYPE_DESKTOP:
       ((struct _Ecore_Win32_Window *)window)->type.desktop = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_DOCK:
       ((struct _Ecore_Win32_Window *)window)->type.dock = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_TOOLBAR:
       ((struct _Ecore_Win32_Window *)window)->type.toolbar = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_MENU:
       ((struct _Ecore_Win32_Window *)window)->type.menu = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_UTILITY:
       ((struct _Ecore_Win32_Window *)window)->type.utility = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_SPLASH:
       ((struct _Ecore_Win32_Window *)window)->type.splash = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_DIALOG:
       ((struct _Ecore_Win32_Window *)window)->type.dialog = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_NORMAL:
       ((struct _Ecore_Win32_Window *)window)->type.normal = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_UNKNOWN:
       ((struct _Ecore_Win32_Window *)window)->type.normal = 1;
       break;
     }
}


/***** Private functions definitions *****/

static Ecore_Win32_Window *
ecore_win32_window_internal_new(Ecore_Win32_Window *parent,
                                int                 x,
                                int                 y,
                                int                 width,
                                int                 height,
                                DWORD               style)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   int                         minimal_width;
   int                         minimal_height;

   w = (struct _Ecore_Win32_Window *)calloc(1, sizeof(struct _Ecore_Win32_Window));
   if (!w)
     {
        ERR("malloc() failed");
        return NULL;
     }

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRect(&rect, style, EINA_FALSE))
     {
        ERR("AdjustWindowRect() failed");
        free(w);
        return NULL;
     }

   minimal_width = GetSystemMetrics(SM_CXMIN);
   minimal_height = GetSystemMetrics(SM_CYMIN);
/*    if (((rect.right - rect.left) < minimal_width) || */
/*        ((rect.bottom - rect.top) < minimal_height)) */
/*      { */
/*         fprintf (stderr, "[Ecore] [Win32] ERROR !!\n"); */
/*         fprintf (stderr, "                Wrong size %ld\n", rect.right - rect.left); */
/*         free(w); */
/*         return NULL; */
/*      } */
   if ((rect.right - rect.left) < minimal_width)
     {
       rect.right = rect.left + minimal_width;
     }

   w->window = CreateWindowEx(0,
                              ECORE_WIN32_WINDOW_CLASS, "",
                              style,
                              x, y,
                              rect.right - rect.left,
                              rect.bottom - rect.top,
                              parent ? ((struct _Ecore_Win32_Window *)parent)->window : NULL,
                              NULL, _ecore_win32_instance, NULL);
   if (!w->window)
     {
        ERR("CreateWindowEx() failed");
        free(w);
        return NULL;
     }

   SetLastError(0);
   if (!SetWindowLongPtr(w->window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        ERR("SetWindowLongPtr() failed");
        DestroyWindow(w->window);
        free(w);
        return NULL;
     }

   w->min_width   = 0;
   w->min_height  = 0;
   w->max_width   = 32767;
   w->max_height  = 32767;
   w->base_width  = -1;
   w->base_height = -1;
   w->step_width  = -1;
   w->step_height = -1;

   w->state.iconified         = 0;
   w->state.modal             = 0;
   w->state.sticky            = 0;
   w->state.maximized_vert    = 0;
   w->state.maximized_horz    = 0;
   w->state.shaded            = 0;
   w->state.hidden            = 0;
   w->state.fullscreen        = 0;
   w->state.above             = 0;
   w->state.below             = 0;
   w->state.demands_attention = 0;

   w->type.desktop = 0;
   w->type.dock    = 0;
   w->type.toolbar = 0;
   w->type.menu    = 0;
   w->type.utility = 0;
   w->type.splash  = 0;
   w->type.dialog  = 0;
   w->type.normal  = 0;

   w->pointer_is_in = 0;
   w->borderless    = 0;
   w->iconified     = 0;
   w->fullscreen    = 0;

   return w;
}
