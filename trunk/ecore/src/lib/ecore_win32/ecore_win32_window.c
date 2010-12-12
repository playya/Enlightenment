#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */


typedef enum _Ecore_Win32_Window_Z_Order Ecore_Win32_Window_Z_Order;
enum _Ecore_Win32_Window_Z_Order
{
  ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM,
  ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST,
  ECORE_WIN32_WINDOW_Z_ORDER_TOP,
  ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST
};

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
   if (!AdjustWindowRect(&rect, style, FALSE))
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
   if (!SetWindowLongPtr(w->window, GWL_USERDATA, (LONG_PTR)w) && (GetLastError() != 0))
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

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_Win32_Group Ecore_Win32 library
 *
 * @{
 */

/**
 * @brief Creates a new window.
 *
 * @param parent The parent window.
 * @param x The x coordinate of the top-left corner of the window.
 * @param y The y coordinate of the top-left corner of the window.
 * @param width The width of the window.
 * @param height The height of hte window.
 * @return A newly allocated window.
 *
 * This function creates a new window which parent is @p parent. @p width and
 * @p height are the size of the window content (the client part),
 * without the border and title bar. @p x and @p y are the system
 * coordinates of the top left cerner of the window (that is, of the
 * title bar). This function returns a newly created window on
 * success, and @c NULL on failure.
 */
EAPI Ecore_Win32_Window *
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

/**
 * @brief Creates a new borderless window.
 *
 * @param parent The parent window.
 * @param x The x coordinate of the top-left corner of the window.
 * @param y The y coordinate of the top-left corner of the window.
 * @param width The width of the window.
 * @param height The height of hte window.
 * @return A newly allocated window.
 *
 * This function is the same than ecore_win32_window_override_new()
 * but the returned window is borderless.
 */
EAPI Ecore_Win32_Window *
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

/**
 * @brief Free the given window.
 *
 * @param window The window to free.
 *
 * This function frees @p window. If @p window is @c NULL, this
 * function does nothing.
 */
EAPI void
ecore_win32_window_free(Ecore_Win32_Window *window)
{
   struct _Ecore_Win32_Window *wnd = window;

   if (!window) return;

   INF("destroying window");

   if (wnd->shape.mask)
      free(wnd->shape.mask);

   DestroyWindow(((struct _Ecore_Win32_Window *)window)->window);
   free(window);
}

/**
 * @brief Return the window HANDLE associated to the given window.
 *
 * @param window The window to retrieve the HANDLE from.
 *
 * This function returns the window HANDLE associated to @p window. If
 * @p window is @c NULL, this function returns @c NULL.
 */
EAPI void *
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

/**
 * @brief Move the given window to a given position.
 *
 * @param window The window to move.
 * @param x The x coordinate of the destination position.
 * @param y The y coordinate of the destination position.
 *
 * This function move @p window to the new position of coordinates @p x
 * and @p y. If @p window is @c NULL, or if it is fullscreen, or on
 * error, this function does nothing.
 */
EAPI void
ecore_win32_window_move(Ecore_Win32_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   /* FIXME: on fullscreen, should not move it */
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
                   TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Resize the given window to a given size.
 *
 * @param window The window to resize.
 * @param width The new width.
 * @param height The new height.
 *
 * This function resize @p window to the new @p width and @p height.
 * If @p window is @c NULL, or if it is fullscreen, or on error, this
 * function does nothing.
 */
EAPI void
ecore_win32_window_resize(Ecore_Win32_Window *window,
                          int                 width,
                          int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;
   int                         x;
   int                         y;

   /* FIXME: on fullscreen, should not resize it */
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
   if (!AdjustWindowRect(&rect, style, FALSE))
     {
        ERR("AdjustWindowRect() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Move and resize the given window to a given position and size.
 *
 * @param window The window to move and resize.
 * @param x The x coordinate of the destination position.
 * @param y The x coordinate of the destination position.
 * @param width The new width.
 * @param height The new height.
 *
 * This function resize @p window to the new position of coordinates @p x
 * and @p y and the new @p width and @p height. If @p window is @c NULL,
 * or if it is fullscreen, or on error, this function does nothing.
 */
EAPI void
ecore_win32_window_move_resize(Ecore_Win32_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;

   /* FIXME: on fullscreen, should not move/resize it */
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
   if (!AdjustWindowRect(&rect, style, FALSE))
     {
        ERR("AdjustWindowRect() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Get the geometry of the given window.
 *
 * @param window The window to retrieve the geometry from.
 * @param x The x coordinate of the position.
 * @param y The x coordinate of the position.
 * @param width The width.
 * @param height The height.
 *
 * This function retrieves the position and size of @p window. @p x,
 * @p y, @p width and @p height can be buffers that will be filled with
 * the corresponding values. If one of them is @c NULL, nothing will
 * be done for that parameter. If @p window is @c NULL, and if the
 * buffers are not @c NULL, they will be filled with respectively 0,
 * 0, the size of the screen and the height of the screen.
 */
EAPI void
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

/**
 * @brief Get the size of the given window.
 *
 * @param window The window to retrieve the size from.
 * @param width The width.
 * @param height The height.
 *
 * This function retrieves the size of @p window. @p width and
 * @p height can be buffers that will be filled with the corresponding
 * values. If one of them is @c NULL, nothing will be done for that
 * parameter. If @p window is @c NULL, and if the buffers are not
 * @c NULL, they will be filled with respectively the size of the screen
 * and the height of the screen.
 */
EAPI void
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

/**
 * @brief Set the minimum size of the given window.
 *
 * @param window The window.
 * @param min_width The minimal width.
 * @param min_height The minimal height.
 *
 * This function sets the minimum size of @p window to @p min_width
 * and *p min_height. If @p window is @c NULL, this functions does
 * nothing.
 */
EAPI void
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

/**
 * @brief Get the minimum size of the given window.
 *
 * @param window The window.
 * @param min_width The minimal width.
 * @param min_height The minimal height.
 *
 * This function fills the minimum size of @p window in the buffers
 * @p min_width and *p min_height. They both can be @c NULL. If
 * @p window is @c NULL, this functions does nothing.
 */
EAPI void
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

/**
 * @brief Set the maximum size of the given window.
 *
 * @param window The window.
 * @param max_width The maximal width.
 * @param max_height The maximal height.
 *
 * This function sets the maximum size of @p window to @p max_width
 * and *p max_height. If @p window is @c NULL, this functions does
 * nothing.
 */
EAPI void
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

/**
 * @brief Get the maximum size of the given window.
 *
 * @param window The window.
 * @param max_width The maximal width.
 * @param max_height The maximal height.
 *
 * This function fills the maximum size of @p window in the buffers
 * @p max_width and *p max_height. They both can be @c NULL. If
 * @p window is @c NULL, this functions does nothing.
 */
EAPI void
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

/**
 * @brief Set the base size of the given window.
 *
 * @param window The window.
 * @param base_width The base width.
 * @param base_height The base height.
 *
 * This function sets the base size of @p window to @p base_width
 * and *p base_height. If @p window is @c NULL, this functions does
 * nothing.
 */
EAPI void
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

/**
 * @brief Get the base size of the given window.
 *
 * @param window The window.
 * @param base_width The base width.
 * @param base_height The bas height.
 *
 * This function fills the base size of @p window in the buffers
 * @p base_width and *p base_height. They both can be @c NULL. If
 * @p window is @c NULL, this functions does nothing.
 */
EAPI void
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

/**
 * @brief Set the step size of the given window.
 *
 * @param window The window.
 * @param step_width The step width.
 * @param step_height The step height.
 *
 * This function sets the step size of @p window to @p step_width
 * and *p step_height. If @p window is @c NULL, this functions does
 * nothing.
 */
EAPI void
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

/**
 * @brief Get the step size of the given window.
 *
 * @param window The window.
 * @param step_width The step width.
 * @param step_height The bas height.
 *
 * This function fills the step size of @p window in the buffers
 * @p step_width and *p step_height. They both can be @c NULL. If
 * @p window is @c NULL, this functions does nothing.
 */
EAPI void
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

EAPI void
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

   if (!window)
      return;

   wnd = (struct _Ecore_Win32_Window *)window;

   if (!mask)
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
          if (!SetWindowRgn(wnd->window, NULL, TRUE))
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
       if (wnd->shape.mask)
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
   if (GetVersionEx(&version_info) == TRUE && version_info.dwMajorVersion == 5)
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
   if (!SetWindowRgn(wnd->window, rgn, TRUE))
     {
        ERR("SetWindowRgn() failed");
     }
}

/**
 * @brief Show the given window.
 *
 * @param window The window to show.
 *
 * This function shows @p window. If @p window is @c NULL, or on
 * error, this function does nothing.
 */
EAPI void
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
/**
 * @brief Hide the given window.
 *
 * @param window The window to show.
 *
 * This function hides @p window. If @p window is @c NULL, or on
 * error, this function does nothing.
 */
EAPI void
ecore_win32_window_hide(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("hiding window");

   ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_HIDE);
}

/**
 * @brief Place the given window at the top of the Z order.
 *
 * @param window The window to place at the top.
 *
 * This function places @p window at the top of the Z order. If
 * @p window is @c NULL, this function does nothing.
 */
EAPI void
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

/**
 * @brief Place the given window at the bottom of the Z order.
 *
 * @param window The window to place at the bottom.
 *
 * This function places @p window at the bottom of the Z order. If
 * @p window is @c NULL, this function does nothing.
 */
EAPI void
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

/**
 * @brief Set the title of the given window.
 *
 * @param window The window to set the title.
 * @param title The new title.
 *
 * This function sets the title of @p window to @p title. If @p window
 * is @c NULL, or if @p title is @c NULL or empty, or on error, this
 * function does nothing.
 */
EAPI void
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

/**
 * @brief Set the focus to the given window.
 *
 * @param window The window to give focus to.
 *
 * This function gives the focus to @p window. If @p window is
 * @c NULL, this function does nothing.
 */
EAPI void
ecore_win32_window_focus_set(Ecore_Win32_Window *window)
{
   if (!window) return;

   INF("focusing window");

   if (!SetFocus(((struct _Ecore_Win32_Window *)window)->window))
     {
        ERR("SetFocus() failed");
     }
}

/**
 * @brief Iconify or restore the given window.
 *
 * @param window The window.
 * @param on EINA_TRUE to iconify the window, EINA_FALSE to restore it.
 *
 * This function iconify or restore @p window. If @p on
 * is set to EINA_TRUE, the window will be iconified, if it is set to
 * EINA_FALSE, it will be restored. If @p window is @c NULL or if the
 * state does not change (like iconifying the window while it is
 * already iconified), this function does nothing.
 */
EAPI void
ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                 Eina_Bool           on)
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

/**
 * @brief Remove or restore the border of the given window.
 *
 * @param window The window.
 * @param on EINA_TRUE to remove the border, EINA_FALSE to restore it.
 *
 * This function remove or restore the border of @p window. If @p on
 * is set to EINA_TRUE, the window will have no border, if it is set to
 * EINA_FALSE, it will have a border. If @p window is @c NULL or if the
 * state does not change (like setting to borderless while the window
 * has no border), this function does nothing.
 */
EAPI void
ecore_win32_window_borderless_set(Ecore_Win32_Window *window,
                                  Eina_Bool           on)
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
        if (!AdjustWindowRect (&rect, style, FALSE))
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

/**
 * @brief Set the given window to fullscreen.
 *
 * @param window The window.
 * @param on EINA_TRUE for fullscreen mode, EINA_FALSE for windowed mode.
 *
 * This function set @p window to fullscreen or windowed mode. If @p on
 * is set to EINA_TRUE, the window will be fullscreen, if it is set to
 * EINA_FALSE, it will be windowed. If @p window is @c NULL or if the
 * state does not change (like setting to fullscreen while the window
 * is already fullscreen), this function does nothing.
 */
EAPI void
ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                  Eina_Bool           on)
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

/**
 * @brief Set the given cursor to the given window.
 *
 * @param window The window to modify the cursor.
 * @param cursor The new cursor.
 *
 * This function sets @p cursor to @p window. @p cursor must have been
 * obtained by ecore_win32_cursor_new() or
 * ecore_win32_cursor_shaped_new(). If @p window or @p cursor is
 * @c NULL, the function does nothing.
 */
EAPI void
ecore_win32_window_cursor_set(Ecore_Win32_Window *window,
                              Ecore_Win32_Cursor *cursor)
{
   INF("setting cursor");

   if (!window || !cursor)
     return;

   if (!SetClassLongPtr(((struct _Ecore_Win32_Window *)window)->window,
                     GCL_HCURSOR, (LONG_PTR)cursor))
     {
        ERR("SetClassLong() failed");
     }
}

/**
 * @brief Set the state of the given window.
 *
 * @param window The window to modify the state.
 * @param state An array of the new states.
 * @param num The number of states in the array.
 *
 * This function set the state of @p window. @p state is an array of
 * states of size @p num. If @p window or @p state are @c NULL, or if
 * @p num is less or equal than 0, the function does nothing.
 */
EAPI void
ecore_win32_window_state_set(Ecore_Win32_Window       *window,
                             Ecore_Win32_Window_State *state,
                             unsigned int              num)
{
   unsigned int i;

   if (!window || !state || (num <= 0))
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

/**
 * @brief Apply the modification of the state to the given window.
 *
 * @param window The window.
 * @param state The state to apply changes.
 * @param set The value of the state change.
 *
 * This function applies the modification of the state @p state of
 * @p window. @p set is used only for
 * #ECORE_WIN32_WINDOW_STATE_ICONIFIED and
 * #ECORE_WIN32_WINDOW_STATE_FULLSCREEN. If @p window is @c NULL, the
 * function does nothing.
 */
EAPI void
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
                              TRUE))
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
                              TRUE))
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
                              TRUE))
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

/**
 * @brief Set the type of the given window.
 *
 * @param window The window to modify the type.
 * @param type The new types.
 *
 * This function set the type of @p window to @p type. If
 * @p window is @c NULL, the function does nothing.
 */
EAPI void
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

/**
 * @}
 */
