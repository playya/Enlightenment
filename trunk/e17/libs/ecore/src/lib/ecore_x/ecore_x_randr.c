/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdlib.h>
#include "ecore_x_private.h"

int
ecore_x_randr_events_select(Ecore_X_Window win, int on)
{
#ifdef ECORE_XRANDR
   if (on)
     XRRSelectInput(_ecore_x_disp, win, RRScreenChangeNotifyMask);
   else
     XRRSelectInput(_ecore_x_disp, win, 0);

   return 1;
#else
   return 0;
#endif
}

Ecore_X_Screen_Size *
ecore_x_randr_screen_sizes_get(Ecore_X_Window root, int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Screen_Size *ret;
   XRRScreenSize *sizes;
   int i, n;

   if (num) *num = 0;

   /* we don't have to free sizes, no idea why not */
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   ret = calloc(n, sizeof(Ecore_X_Screen_Size));
   if (!ret) return NULL;

   if (num) *num = n;
   for (i = 0; i < n; i++)
     {
	ret[i].width = sizes[i].width;
	ret[i].height = sizes[i].height;
     }
   return ret;
#else
   if (num) *num = 0;
   return NULL;
#endif
}

int
ecore_x_randr_screen_size_set(Ecore_X_Window root, Ecore_X_Screen_Size *size)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc;
   XRRScreenSize *sizes;
   int i, n, size_index = -1;

   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   for (i = 0; i < n; i++)
     {
	if ((sizes[i].width == size->width) && (sizes[i].height == size->height))
	  {
	     size_index = i;
	     break;
	  }
     }
   if (size_index == -1) return 0;

   printf("Size: %d\n", size_index);
   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (XRRSetScreenConfig(_ecore_x_disp, sc,
			  root, size_index,
			  RR_Rotate_0, CurrentTime))
     {
	printf("ERROR: Can't set new screen size!\n");
	XRRFreeScreenConfigInfo(sc);
	return 0;
     }
   XRRFreeScreenConfigInfo(sc);
   return 1;
#else
   return 0;
#endif
}
