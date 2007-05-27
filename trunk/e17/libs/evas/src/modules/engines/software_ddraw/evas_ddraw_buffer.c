#include <string.h>

#include "evas_engine.h"


DDraw_Output_Buffer *
evas_software_ddraw_output_buffer_new(int   depth,
                                      int   width,
                                      int   height,
                                      void *data)
{
   DDraw_Output_Buffer *ddob;

   ddob = calloc(1, sizeof(DDraw_Output_Buffer));
   if (!ddob) return NULL;

   ddob->image = data;
   ddob->depth = depth;
   ddob->width = width;
   ddob->height = height;
   ddob->pitch = width * depth / 8;

   if (!ddob->image)
     {
        ddob->image = malloc(ddob->pitch * height);
        if (!ddob->image)
          {
            free(ddob);
            return NULL;
          }
     }

   return ddob;
}

void
evas_software_ddraw_output_buffer_free(DDraw_Output_Buffer *ddob)
{
  if (ddob->image) free(ddob->image);
  free(ddob);
}

void
evas_software_ddraw_output_buffer_paste(DDraw_Output_Buffer *ddob,
                                        DDSURFACEDESC2      *surface_desc,
                                        int                 x,
                                        int                 y)
{
   DATA8         *dd_data;
   DATA8         *evas_data;
   int            width;
   int            height;
   int            pitch;
   int            j;

   if ((x >= surface_desc->dwWidth) || (y >= surface_desc->dwHeight))
     return;

   /* compute the size of the data to copy on the back surface */
   width = ((x + ddob->width) > surface_desc->dwWidth)
     ? surface_desc->dwWidth - x
     : ddob->width;
   height = ((y + ddob->height) > surface_desc->dwHeight)
     ? surface_desc->dwHeight - y
     : ddob->height;
   pitch = width * ddob->depth / 8;

   dd_data = (DATA8 *)surface_desc->lpSurface + y * surface_desc->lPitch + x * surface_desc->ddpfPixelFormat.dwRGBBitCount / 8;
   evas_data = (unsigned char *)ddob->image;
   for (j = 0; j < height; j++, evas_data += ddob->pitch, dd_data += surface_desc->lPitch)
     memcpy(dd_data, evas_data, pitch);
}

DATA8 *
evas_software_ddraw_output_buffer_data(DDraw_Output_Buffer *ddob,
                                       int                 *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = ddob->pitch;
   return ddob->image;
}

int
evas_software_ddraw_output_buffer_depth(DDraw_Output_Buffer *ddob)
{
   return ddob->depth;
}
