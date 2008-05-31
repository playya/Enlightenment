/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <aygshell.h>

#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"

char *
_wchar_to_char(const wchar_t *text)
{
   char * atext;
   int    size;
   int    asize;

   size = wcslen(text) + 1;

   asize = WideCharToMultiByte(CP_ACP, 0, text, size, NULL, 0, NULL, NULL);
   if (asize == 0)
     return NULL;

   atext = (char*)malloc((asize + 1) * sizeof(char));

   if (atext)
     if (!WideCharToMultiByte(CP_ACP, 0, text, size, atext, asize, NULL, NULL))
       return NULL;
   atext[asize] = '\0';

   return atext;
}

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

   w = (struct _Ecore_WinCE_Window *)calloc(1, sizeof(struct _Ecore_WinCE_Window));
   if (!w)
     return NULL;

   SetRect(&rect, 0, 0,
           GetSystemMetrics(SM_CXSCREEN),
           GetSystemMetrics(SM_CYSCREEN));

   window = CreateWindowEx(WS_EX_TOPMOST,
                           ECORE_WINCE_WINDOW_CLASS,
                           L"",
                           WS_VISIBLE | WS_POPUP,
                           rect.left, rect.top,
                           rect.right - rect.left,
                           rect.bottom - rect.top,
                            parent ? ((struct _Ecore_WinCE_Window *)parent)->window : NULL,
                           NULL, _ecore_wince_instance, NULL);
   if (!window)
     {
        free(w);
        return NULL;
     }

   SHFullScreen(window,
                SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);

   if (!_ecore_wince_hardware_keys_register(window))
     {
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->window = window;

   if (!SetWindowLong(window, GWL_USERDATA, (LONG)w))
     {
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   return w;
}

void
ecore_wince_window_del(Ecore_WinCE_Window *window)
{
   Ecore_WinCE_Window *w;

   if (!window) return;

   DestroyWindow(((struct _Ecore_WinCE_Window *)window)->window);
   free(window);
   fprintf (stderr, "ecore_wince_window_del\n");
}

void
ecore_wince_window_show(Ecore_WinCE_Window *window)
{
   if (!window) return;

   fprintf (stderr, " ** ecore_wince_window_show  %p\n", window);
   ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_SHOWNORMAL);
   UpdateWindow(((struct _Ecore_WinCE_Window *)window)->window);
}

void
ecore_wince_window_hide(Ecore_WinCE_Window *window)
{
   if (!window) return;

   fprintf (stderr, " ** ecore_wince_window_hide  %p\n", window);
   ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_HIDE);
}

void
ecore_wince_window_suspend_set(Ecore_WinCE_Window *window, int (*suspend)(void))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_WinCE_Window *)window;
   w->suspend = suspend;
}

void
ecore_wince_window_resume_set(Ecore_WinCE_Window *window, int (*resume)(void))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_WinCE_Window *)window;
   w->resume = resume;
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
     return 0;

   unregister_fct = (UnregisterFunc1Proc)GetProcAddress(core_dll, L"UnregisterFunc1");
   if (!unregister_fct)
     {
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
