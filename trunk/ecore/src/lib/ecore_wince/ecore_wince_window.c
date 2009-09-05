/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Evil.h>
#include <Eina.h>

#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"


/***** Private declarations *****/

typedef BOOL (__stdcall *UnregisterFunc1Proc)(UINT, UINT);

static int _ecore_wince_hardware_keys_register(HWND window);


/***** API *****/

Ecore_WinCE_Window *
ecore_wince_window_new(Ecore_WinCE_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   struct _Ecore_WinCE_Window *w;
   HWND                        window;
   RECT                        rect;

   MESSAGE_INFO("creating window\n");

   w = (struct _Ecore_WinCE_Window *)calloc(1, sizeof(struct _Ecore_WinCE_Window));
   if (!w)
     {
        MESSAGE_ERR("malloc() failed\n");
        return NULL;
     }

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRectEx(&rect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, FALSE, WS_EX_TOPMOST))
     {
        MESSAGE_ERR("AdjustWindowRectEx() failed\n");
        free(w);
        return NULL;
     }

   window = CreateWindowEx(WS_EX_TOPMOST,
                           ECORE_WINCE_WINDOW_CLASS,
                           L"",
                           WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                           x, y,
                           rect.right - rect.left, rect.bottom - rect.top,
                           parent ? ((struct _Ecore_WinCE_Window *)parent)->window : NULL,
                           NULL, _ecore_wince_instance, NULL);
   if (!window)
     {
        MESSAGE_ERR("CreateWindowEx() failed\n");
        free(w);
        return NULL;
     }

   if (!_ecore_wince_hardware_keys_register(window))
     {
        MESSAGE_ERR("_ecore_wince_hardware_keys_register() failed\n");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->window = window;

   SetLastError(0);
   if (!SetWindowLong(window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        MESSAGE_ERR("SetWindowLong() failed\n");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->pointer_is_in = 0;

   return w;
}

void
ecore_wince_window_free(Ecore_WinCE_Window *window)
{
   if (!window) return;

   MESSAGE_INFO("destroying window\n");

   DestroyWindow(((struct _Ecore_WinCE_Window *)window)->window);
   free(window);
}

void *
ecore_wince_window_hwnd_get(Ecore_WinCE_Window *window)
{
   if (!window)
     return NULL;

   return ((struct _Ecore_WinCE_Window *)window)->window;
}

void
ecore_wince_window_move(Ecore_WinCE_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   MESSAGE_INFO("moving window (%dx%d)\n", x, y);

   w = ((struct _Ecore_WinCE_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     {
        MESSAGE_ERR("GetWindowRect() failed\n");
        return;
     }

   if (!MoveWindow(w, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        MESSAGE_ERR("MoveWindow() failed\n");
     }
}

void
ecore_wince_window_resize(Ecore_WinCE_Window *window,
                          int                 width,
                          int                 height)
{
   RECT                        rect;
   struct _Ecore_WinCE_Window *w;
   DWORD                       style;
   DWORD                       exstyle;
   int                         x;
   int                         y;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   MESSAGE_INFO("resizing window (%dx%d)\n", width, height);

   w = (struct _Ecore_WinCE_Window *)window;
   if (!GetWindowRect(w->window, &rect))
     {
        MESSAGE_ERR("GetWindowRect() failed\n");
        return;
     }

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        MESSAGE_ERR("GetWindowLong() failed\n");
        return;
     }
   if (!(exstyle = GetWindowLong(w->window, GWL_EXSTYLE)))
     {
        MESSAGE_ERR("GetWindowLong() failed\n");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        MESSAGE_ERR("AdjustWindowRectEx() failed\n");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   FALSE))
     {
        MESSAGE_ERR("MoveWindow() failed\n");
     }
}

void
ecore_wince_window_move_resize(Ecore_WinCE_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                        rect;
   struct _Ecore_WinCE_Window *w;
   DWORD                       style;
   DWORD                       exstyle;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   MESSAGE_INFO("moving and resizing window (%dx%d %dx%d)\n", x, y, width, height);

   w = ((struct _Ecore_WinCE_Window *)window);
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        MESSAGE_ERR("GetWindowLong() failed\n");
        return;
     }
   if (!(exstyle = GetWindowLong(w->window, GWL_EXSTYLE)))
     {
        MESSAGE_ERR("GetWindowLong() failed\n");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        MESSAGE_ERR("AdjustWindowRectEx() failed\n");
        return;
     }

   if (!MoveWindow(w->window, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE))
     {
        MESSAGE_ERR("MoveWindow() failed\n");
     }
}

void
ecore_wince_window_show(Ecore_WinCE_Window *window)
{
   if (!window) return;

   MESSAGE_INFO("showing window\n");

   if (!ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_SHOWNORMAL))
     {
        MESSAGE_ERR("ShowWindow() failed\n");
        return;
     }
   if (!UpdateWindow(((struct _Ecore_WinCE_Window *)window)->window))
     {
        MESSAGE_ERR("UpdateWindow() failed\n");
     }
   if (!SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 1, 0))
     {
        MESSAGE_ERR("SendMessage() failed\n");
     }
}

void
ecore_wince_window_hide(Ecore_WinCE_Window *window)
{
   if (!window) return;

   MESSAGE_INFO("hiding window\n");

   if (!ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_HIDE))
     {
        MESSAGE_ERR("ShowWindow() failed\n");
        return;
     }
   if (!SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 0, 0))
     {
        MESSAGE_ERR("SendMessage() failed\n");
     }
}

void
ecore_wince_window_title_set(Ecore_WinCE_Window *window,
                             const char         *title)
{
   wchar_t *wtitle;

   if (!window) return;

   if (!title || !title[0]) return;

   MESSAGE_INFO("setting window title\n");

   wtitle = evil_char_to_wchar(title);
   if (!wtitle) return;

   if (!SetWindowText(((struct _Ecore_WinCE_Window *)window)->window, wtitle))
     {
        MESSAGE_ERR("SetWindowText() failed\n");
     }
   free(wtitle);
}

void
ecore_wince_window_backend_set(Ecore_WinCE_Window *window, int backend)
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   MESSAGE_INFO("setting backend\n");

   w = (struct _Ecore_WinCE_Window *)window;
   w->backend = backend;
}

void
ecore_wince_window_suspend_set(Ecore_WinCE_Window *window, int (*suspend)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   MESSAGE_INFO("setting suspend callback\n");

   w = (struct _Ecore_WinCE_Window *)window;
   w->suspend = suspend;
}

void
ecore_wince_window_resume_set(Ecore_WinCE_Window *window, int (*resume)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   MESSAGE_INFO("setting resume callback\n");

   w = (struct _Ecore_WinCE_Window *)window;
   w->resume = resume;
}

void
ecore_wince_window_geometry_get(Ecore_WinCE_Window *window,
                                int                *x,
                                int                *y,
                                int                *width,
                                int                *height)
{
   RECT rect;
   int  w;
   int  h;

   MESSAGE_INFO("getting window geometry\n");

   if (!window)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        MESSAGE_ERR("GetClientRect() failed\n");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   w = rect.right - rect.left;
   h = rect.bottom - rect.top;

   if (!GetWindowRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        MESSAGE_ERR("GetWindowRect() failed\n");

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
ecore_wince_window_size_get(Ecore_WinCE_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   MESSAGE_INFO("getting window size\n");

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        MESSAGE_ERR("GetClientRect() failed\n");

        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

void
ecore_wince_window_fullscreen_set(Ecore_WinCE_Window *window,
                                  int                 on)
{
   struct _Ecore_WinCE_Window *ew;
   HWND                        w;
   HWND                        task_bar;

   if (!window) return;

   ew = (struct _Ecore_WinCE_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   MESSAGE_INFO("setting fullscreen: %s\n", on ? "yes" : "no");

   ew->fullscreen = !!on;
   w = ew->window;

   if (on)
     {
        /* save the position and size of the window */
        if (!GetWindowRect(w, &ew->rect))
          {
             MESSAGE_ERR("GetWindowRect() failed\n");
             return;
          }

        /* hide task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             MESSAGE_INFO("FindWindow(): can not find task bar\n");
          }
        if (!ShowWindow(task_bar, SW_HIDE))
          {
             MESSAGE_INFO("ShowWindow(): task bar already hidden\n");
          }
        if (!EnableWindow(task_bar, FALSE))
          {
             MESSAGE_INFO("EnableWindow(): input already disabled\n");
          }

        /* style: visible + popup */
        if (!SetWindowLong(w, GWL_STYLE, WS_POPUP | WS_VISIBLE))
          {
             MESSAGE_INFO("SetWindowLong() failed\n");
          }

        /* resize window to fit the entire screen */
        if (!SetWindowPos(w, HWND_TOPMOST,
                          0, 0,
                          GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             MESSAGE_INFO("SetWindowPos() failed\n");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(w,
                        0, 0,
                        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                        TRUE))
          {
             MESSAGE_INFO("MoveWindow() failed\n");
          }
     }
   else
     {
        /* show task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             MESSAGE_INFO("FindWindow(): can not find task bar\n");
          }
        if (!ShowWindow(task_bar, SW_SHOW))
          {
             MESSAGE_INFO("ShowWindow(): task bar already visible\n");
          }
        if (!EnableWindow(task_bar, TRUE))
          {
             MESSAGE_INFO("EnableWindow():  input already enabled\n");
          }

        /* style: visible + caption + sysmenu */
        if (!SetWindowLong(w, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_VISIBLE))
          {
             MESSAGE_INFO("SetWindowLong() failed\n");
          }
        /* restaure the position and size of the window */
        if (!SetWindowPos(w, HWND_TOPMOST,
                          ew->rect.left,
                          ew->rect.top,
                          ew->rect.right - ew->rect.left,
                          ew->rect.bottom - ew->rect.top,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             MESSAGE_INFO("SetWindowLong() failed\n");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(w,
                        ew->rect.left,
                        ew->rect.top,
                        ew->rect.right - ew->rect.left,
                        ew->rect.bottom - ew->rect.top,
                        TRUE))
          {
             MESSAGE_INFO("MoveWindow() failed\n");
          }
     }
}


/***** Private functions definitions *****/

static int
_ecore_wince_hardware_keys_register(HWND window)
{
   HINSTANCE           core_dll;
   UnregisterFunc1Proc unregister_fct;
   int                 i;

   core_dll = LoadLibrary(L"coredll.dll");
   if (!core_dll)
     {
        MESSAGE_ERR("LoadLibrary() failed\n");
        return 0;
     }

   unregister_fct = (UnregisterFunc1Proc)GetProcAddress(core_dll, L"UnregisterFunc1");
   if (!unregister_fct)
     {
        MESSAGE_ERR("GetProcAddress() failed\n");
        FreeLibrary(core_dll);
        return 0;
     }

   for (i = 0xc1; i <= 0xcf; i++)
     {
        unregister_fct(MOD_WIN, i);
        RegisterHotKey(window, i, MOD_WIN, i);
     }

   FreeLibrary(core_dll);

   return 1;
}
