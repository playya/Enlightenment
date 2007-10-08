#include "evas_common.h"
#include "evas_engine.h"


DDraw_Output_Buffer *
evas_software_ddraw_output_buffer_new(HWND                window,
                                      LPDIRECTDRAW        object,
                                      LPDIRECTDRAWSURFACE surface_primary,
                                      LPDIRECTDRAWSURFACE surface_back,
                                      LPDIRECTDRAWSURFACE surface_source,
                                      int                 width,
                                      int                 height)
{
   DDSURFACEDESC2       surface_desc;
   DDraw_Output_Buffer *ddob;

   ddob = calloc(1, sizeof(DDraw_Output_Buffer));
   if (!ddob) return NULL;

   ddob->dd.window = window;
   ddob->dd.object = object;
   ddob->dd.surface_primary = surface_primary;
   ddob->dd.surface_back = surface_back;
   ddob->dd.surface_source = surface_source;
   ddob->width = width;
   ddob->height = height;
   ddob->pitch = width * 2;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   if (FAILED(IDirectDrawSurface7_Lock(ddob->dd.surface_source, NULL,
                                       &surface_desc,
                                       DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                                       NULL)))
     {
        free(ddob);
        return NULL;
     }

   ddob->im.pixels = surface_desc.lpSurface;
   ddob->im.w = width;
   ddob->im.h = height;
   ddob->im.stride = width;
   ddob->im.references = 1;

   if (FAILED(IDirectDrawSurface7_Unlock(ddob->dd.surface_source, NULL)))
     {
        free(ddob);
        return NULL;
     }

   return ddob;
}

void
evas_software_ddraw_output_buffer_free(DDraw_Output_Buffer *ddob, int sync)
{
   free(ddob);
}

void
evas_software_ddraw_output_buffer_paste(DDraw_Output_Buffer *ddob)
{
   RECT    dst_rect;
   RECT    src_rect;
   POINT   p;

   SetRect(&src_rect, 0, 0, ddob->width, ddob->height);

   if (FAILED(IDirectDrawSurface7_BltFast(ddob->dd.surface_back,
                                          0, 0,
                                          ddob->dd.surface_source,
                                          &src_rect,
                                          DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT)))
     return;

   p.x = 0;
   p.y = 0;
   ClientToScreen(ddob->dd.window, &p);
   GetClientRect(ddob->dd.window, &dst_rect);
   OffsetRect(&dst_rect, p.x, p.y);
   IDirectDrawSurface7_Blt(ddob->dd.surface_primary, &dst_rect,
                           ddob->dd.surface_back, &src_rect,
                           DDBLT_WAIT, NULL);
}
