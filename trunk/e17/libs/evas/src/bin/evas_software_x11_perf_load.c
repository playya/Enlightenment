#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Evas.h"
#include "Evas_Engine_Software_X11.h"

Display            *disp = NULL;
Window              win = 0;

Evas               *evas = NULL;

int                 win_w = 240;
int                 win_h = 240;

int
main(int argc, char **argv)
{
   int                 exposed = 0;

   XSetWindowAttributes attr;
   XClassHint          chint;
   XSizeHints          szhints;

   disp = XOpenDisplay(NULL);
   if (!disp)
     {
	printf("Error: cannot open display.\n");
	exit(-1);
     }
   attr.backing_store = NotUseful;
   attr.colormap = DefaultColormap(disp, DefaultScreen(disp));
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.event_mask =
      ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
   attr.bit_gravity = ForgetGravity;
   win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0, win_w, win_h, 0,
		       DefaultDepth(disp, DefaultScreen(disp)), InputOutput,
		       DefaultVisual(disp, DefaultScreen(disp)),
		       CWBackingStore | CWColormap |
		       CWBackPixmap | CWBorderPixel |
		       CWBitGravity | CWEventMask, &attr);
   XStoreName(disp, win, "Evas Performance Test");
   chint.res_name = "Evas_Test";
   chint.res_class = "Main";
   XSetClassHint(disp, win, &chint);
   szhints.flags = PMinSize | PMaxSize | PSize | USSize;
   szhints.max_width = win_w;
   szhints.max_height = win_h;
   szhints.min_width = win_w;
   szhints.min_height = win_h;
   XSetWMNormalHints(disp, win, &szhints);
   XMapWindow(disp, win);

   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Software_X11 *einfo;

      einfo = (Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.display = disp;
      einfo->info.visual = DefaultVisual(disp, DefaultScreen(disp));
      einfo->info.colormap = DefaultColormap(disp, DefaultScreen(disp));
      einfo->info.drawable = win;
      einfo->info.depth = DefaultDepth(disp, DefaultScreen(disp));
      einfo->info.rotation = 0;

      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   for (;;)
     {
	XEvent              ev;

	while (XCheckMaskEvent(disp,
			       ExposureMask |
			       StructureNotifyMask |
			       KeyPressMask |
			       KeyReleaseMask |
			       ButtonPressMask |
			       ButtonReleaseMask | PointerMotionMask, &ev))
	  {
	     switch (ev.type)
	       {
	       case ButtonPress:
/*		  evas_event_button_down(evas, ev.xbutton.x, ev.xbutton.y, ev.xbutton.button);*/
		  break;
	       case ButtonRelease:
/*		  evas_event_button_up(evas, ev.xbutton.x, ev.xbutton.y, ev.xbutton.button);*/
		  break;
	       case MotionNotify:
/*		  evas_event_move(evas, ev.xmotion.x, ev.xmotion.y);*/
		  break;
	       case Expose:
		  exposed = 1;
		  evas_damage_rectangle_add(evas,
					    ev.xexpose.x,
					    ev.xexpose.y,
					    ev.xexpose.width,
					    ev.xexpose.height);
		  break;
	       default:
		  break;
	       }
	  }
	evas_render(evas);
	XFlush(disp);
	if (exposed)
	  {
	     Evas_Engine_Info_Software_X11 *einfo;
	     Evas_Performance   *perf;
	     char                buf[4096];
	     char               *data, *key;
	     FILE               *f;

	     data = NULL;
	     einfo =
		(Evas_Engine_Info_Software_X11 *) evas_engine_info_get(evas);
	     perf =
		einfo->func.performance_new(evas, disp,
					    DefaultVisual(disp,
							  DefaultScreen(disp)),
					    DefaultColormap(disp,
							    DefaultScreen
							    (disp)), win,
					    DefaultDepth(disp,
							 DefaultScreen(disp)));
	     key = einfo->func.performance_key_get(perf);
	     snprintf(buf, sizeof(buf), "%s/.evas/%s", getenv("HOME"), key);
	     free(key);
	     f = fopen(buf, "r");
	     if (f)
	       {
		  char                buf[4096];
		  size_t              sz;

		  sz = fread(buf, 1, 4000, f);
		  if (sz > 0)
		    {
		       data = malloc(sz + 1);
		       if (data)
			 {
			    strncpy(data, buf, sz);
			    data[sz] = 0;
			 }
		    }
		  fclose(f);
	       }
	     if (data)
	       {
		  einfo->func.performance_build(perf, data);
		  einfo->func.performance_device_store(perf);
		  free(data);
	       }
	     else
	       {
		  printf("No cached performance values for your display.\n");
		  printf("Please run evas_perf_test to generate them.\n");
		  einfo->func.performance_free(perf);
		  exit(-1);
	       }
	     einfo->func.performance_free(perf);
	     exit(0);
	  }
     }

   evas_shutdown();
   return 0;
}
