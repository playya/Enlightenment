/*
 * Xinerama code
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static XineramaScreenInfo *_xin_info = NULL;
static int _xin_scr_num = 0;

int
ecore_x_xinerama_screen_count_get(void)
{
#ifdef ECORE_XINERAMA
   if (_xin_info) XFree(_xin_info);
   _xin_info = NULL;
   _xin_info = XineramaQueryScreens(_ecore_x_disp, &_xin_scr_num);
   if (_xin_info) return _xin_scr_num;
   else return 0;
#else
   return 0;
#endif
}

int
ecore_x_xinerama_screen_geometry_get(int screen, int *x, int *y, int *w, int *h)
{
#ifdef ECORE_XINERAMA
   if (_xin_info)
     {
	int i;
	
	for (i = 0; i < _xin_scr_num; i++)
	  {
	     if (_xin_info[i].screen_number == screen)
	       {
		  if (x) *x = _xin_info[i].x_org;
		  if (y) *y = _xin_info[i].y_org;
		  if (w) *w = _xin_info[i].width;
		  if (h) *h = _xin_info[i].height;
		  return 1;
	       }
	  }
     }
#endif
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = DisplayWidth(_ecore_x_disp, 0);
   if (h) *h = DisplayHeight(_ecore_x_disp, 0);
   return 0;
}
