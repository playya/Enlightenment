/*
 * Copyright (C) 2000 Carsten Haitzler, Geoff Harrison and various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"

#ifndef M_PI_2
#define M_PI_2 (3.141592654 / 2)
#endif

typedef struct _fxhandler
{
   char               *name;
   void                (*init_func) (char *name);
   void                (*desk_func) (void);
   void                (*quit_func) (void);
   void                (*pause_func) (void);
   char                in_use;
   char                paused;
}
FXHandler;

void                FX_Ripple_Init(char *name);
void                FX_Ripple_Desk(void);
void                FX_Ripple_Quit(void);
void                FX_Ripple_Pause(void);
void                FX_Raindrops_Init(char *name);
void                FX_Raindrops_Desk(void);
void                FX_Raindrops_Quit(void);
void                FX_Raindrops_Pause(void);
void                FX_Waves_Init(char *name);
void                FX_Waves_Desk(void);
void                FX_Waves_Quit(void);
void                FX_Waves_Pause(void);
void                FX_ImageSpinner_Init(char *name);
void                FX_ImageSpinner_Desk(void);
void                FX_ImageSpinner_Quit(void);
void                FX_ImageSpinner_Pause(void);

static int          num_fx_handlers = 4;
static FXHandler    fx_handlers[] = {
   {"ripples",
    FX_Ripple_Init, FX_Ripple_Desk, FX_Ripple_Quit, FX_Ripple_Pause,
    0, 0},
   {"raindrops",
    FX_Raindrops_Init, FX_Raindrops_Desk, FX_Raindrops_Quit,
    FX_Raindrops_Pause,
    0, 0},
   {"waves",
    FX_Waves_Init, FX_Waves_Desk, FX_Waves_Quit, FX_Waves_Pause,
    0, 0},
   {"imagespinner",
    FX_ImageSpinner_Init, FX_ImageSpinner_Desk, FX_ImageSpinner_Quit,
    FX_ImageSpinner_Pause,
    0, 0}
};

/****************************** Effect handlers *****************************/

void
FX_Start(char *name)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (!strcmp(fx_handlers[i].name, name))
	  {
	     if ((fx_handlers[i].in_use) && (fx_handlers[i].quit_func))
	       {
		  (*(fx_handlers[i].quit_func)) ();
		  fx_handlers[i].in_use = 0;
	       }
	     else
	       {
		  if (fx_handlers[i].init_func)
		     (*(fx_handlers[i].init_func)) (name);
		  fx_handlers[i].in_use = 1;
	       }
	  }
     }
}

void
FX_DeskChange(void)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (fx_handlers[i].in_use)
	  {
	     if (fx_handlers[i].desk_func)
		(*(fx_handlers[i].desk_func)) ();
	  }
     }
}

void
FX_Pause(void)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (fx_handlers[i].in_use)
	  {
	     if (fx_handlers[i].paused)
	       {
		  if (fx_handlers[i].pause_func)
		     (*(fx_handlers[i].pause_func)) ();
		  fx_handlers[i].paused = 1;
	       }
	     else
	       {
		  if (fx_handlers[i].pause_func)
		     (*(fx_handlers[i].pause_func)) ();
		  fx_handlers[i].paused = 0;
	       }
	  }
     }
}

char              **
FX_Active(int *num)
{
   int                 i;
   char              **list = NULL;

   *num = 0;
   for (i = 0; i < num_fx_handlers; i++)
     {
	if (fx_handlers[i].in_use)
	  {
	     (*num)++;
	     list = Erealloc(list, sizeof(char *) * (*num));

	     list[(*num) - 1] = duplicate(fx_handlers[i].name);
	  }
     }
   return list;
}

int
FX_IsOn(char *effect)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (!strcmp(fx_handlers[i].name, effect))
	  {
	     if (fx_handlers[i].in_use)
		return 1;
	     return 0;
	  }
     }
   return 0;

}

void
FX_Deactivate(char *effect)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (!strcmp(fx_handlers[i].name, effect))
	  {
	     if (fx_handlers[i].in_use)
	       {
		  fx_handlers[i].in_use = 0;
		  (*(fx_handlers[i].quit_func)) ();
	       }
	  }
     }
   return;
}

void
FX_Activate(char *effect)
{
   int                 i;

   for (i = 0; i < num_fx_handlers; i++)
     {
	if (!strcmp(fx_handlers[i].name, effect))
	  {
	     if (!fx_handlers[i].in_use)
	       {
		  fx_handlers[i].in_use = 1;
		  (*(fx_handlers[i].init_func)) (effect);
	       }
	  }
     }
   return;
}

/****************************** RIPPLES *************************************/

#define fx_ripple_waterh 64
static Pixmap       fx_ripple_above = 0;
static Window       fx_ripple_win = 0;
static int          fx_ripple_count = 0;

void                FX_ripple_timeout(int val, void *data);

void
FX_ripple_timeout(int val, void *data)
{
   static char         before = 0;
   static double       incv = 0, inch = 0;
   static GC           gc1 = 0, gc = 0;
   int                 y;

   if (!fx_ripple_above)
     {
	XGCValues           gcv;

	fx_ripple_win = desks.desk[desks.current].win;
	if ((desks.current == 0) && (ird))
	   fx_ripple_above =
	      ECreatePixmap(disp, fx_ripple_win, root.w, fx_ripple_waterh * 2,
			    ird->x.depth);
	else
	   fx_ripple_above =
	      ECreatePixmap(disp, fx_ripple_win, root.w, fx_ripple_waterh * 2,
			    id->x.depth);
	if (gc)
	   XFreeGC(disp, gc);
	if (gc1)
	   XFreeGC(disp, gc1);
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(disp, fx_ripple_win, GCSubwindowMode, &gcv);
	gc1 = XCreateGC(disp, fx_ripple_win, 0L, &gcv);
	if (!before)
	   DIALOG_OK(_("Starting up Ripples FX..."),
		     _("\n" "You have just started the Ripples Effect.\n" "\n"
		       "If you look closely on your desktop background, and if it\n"
		       "doesn't have a solid colour (i.e. has a background texture or\n"
		       "image), you will see a pool of water at the bottom of your\n"
		       "screen that reflects everything above it and \"ripples\".\n"
		       "\n"
		       "To disable this effect just select this option again to toggle\n"
		       "it off.\n"));
	before = 1;
     }
   if (fx_ripple_count == 0)
      XCopyArea(disp, fx_ripple_win, fx_ripple_above, gc, 0,
		root.h - (fx_ripple_waterh * 3), root.w, fx_ripple_waterh * 2,
		0, 0);
   fx_ripple_count++;
   if (fx_ripple_count > 32)
      fx_ripple_count = 0;
   incv += 0.40;
   if (incv > (M_PI_2 * 4))
     {
	incv = 0;
     }
   inch += 0.32;
   if (inch > (M_PI_2 * 4))
     {
	inch = 0;
     }
   for (y = 0; y < fx_ripple_waterh; y++)
     {
	double              aa, a, p;
	int                 yoff, off, yy;

	p = (((double)(fx_ripple_waterh - y)) / ((double)fx_ripple_waterh));
	a = p * p * 48 + incv;
	yoff = y + (int)(sin(a) * 7) + 1;
	yy = (fx_ripple_waterh * 2) - yoff;
	aa = p * p * 64 + inch;
	off = (int)(sin(aa) * 10 * (1 - p));
	XCopyArea(disp, fx_ripple_above, fx_ripple_win, gc1, 0, yy, root.w, 1,
		  off, root.h - fx_ripple_waterh + y);
     }
   DoIn("FX_RIPPLE_TIMEOUT", 0.066, FX_ripple_timeout, 0, NULL);
   return;
   val = 0;
   data = NULL;
}

void
FX_Ripple_Init(char *name)
{
   fx_ripple_count = 0;
   DoIn("FX_RIPPLE_TIMEOUT", 0.066, FX_ripple_timeout, 0, NULL);
   name = NULL;
}

void
FX_Ripple_Desk(void)
{
   EFreePixmap(disp, fx_ripple_above);
   fx_ripple_count = 0;
   fx_ripple_above = 0;
}

void
FX_Ripple_Quit(void)
{
   RemoveTimerEvent("FX_RIPPLE_TIMEOUT");
   XClearArea(disp, fx_ripple_win, 0, root.h - fx_ripple_waterh, root.w,
	      fx_ripple_waterh, False);
}

void
FX_Ripple_Pause(void)
{
   static char         paused = 0;

   if (!paused)
     {
	FX_Ripple_Quit();
	paused = 1;
     }
   else
     {
	FX_Ripple_Init(NULL);
	paused = 0;
     }
}

/****************************************************************************/

/****************************** RAIN DROPS **********************************/

#define fx_raindrop_size 96
#define fx_raindrop_size2 (fx_raindrop_size / 2)
#define fx_raindrop_duration 32
#define fx_frequency 4
#define fx_amplitude 48
static Window       fx_raindrops_win = 0;
static int          fx_raindrops_number = 4;
static PixImg      *fx_raindrops_draw = NULL;

void                FX_raindrops_timeout(int val, void *data);

typedef struct _drop_context
{
   int                 x, y;
   int                 count;
   PixImg             *buf;
}
DropContext;

static DropContext  fx_raindrops[4];

void
FX_raindrops_timeout(int val, void *data)
{
   static char         before = 0;
   static GC           gc1 = 0, gc = 0;
   int                 i, x, y, xx, yy;
   int                 percent_done;
   static char         first = 1;
   static char         sintab[256];
   static unsigned char disttab[fx_raindrop_size][fx_raindrop_size];

   if (!fx_raindrops_win)
     {
	XGCValues           gcv;

	if (!id->x.shm)
	  {
	     DIALOG_OK(_("Unable to display raindrops"),
		       _("\n"
			 "Enlightenment is unable to display raindrops on this\n"
			 "display because Shared memory is not available on this\n"
			 "X-Server.\n" "\n"
			 "This may be due to Enlightenment being a remote client\n"
			 "running over the network, a MIT-SHM incapable X-server,\n"
			 "having run out of SHM ID's on the system or Shared\n"
			 "Memory support being turned off in Imlib\n" "\n"
			 "You may correct this by either running `imlib_config'\n"
			 "or copying the system imrc (/usr/etc/imrc) to ~/.imrc\n"
			 "and editing it, enabling shared memory.\n" "\n"));
	     return;
	  }
	if (!id->x.shmp)
	  {
	     DIALOG_OK(_("Unable to display raindrops"),
		       _("\n"
			 "Enlightenment is unable to display raindrops on this\n"
			 "display because shared pixmaps are not available on this\n"
			 "X-Server.\n" "\n"
			 "This may be due to either the X-Server not implementing\n"
			 "shared pixmaps, or shared pixmaps being disabled in\n"
			 "Imlib's configuration.\n" "\n"
			 "You may correct this by either running `imlib_config'\n"
			 "or copying the system imrc (/usr/etc/imrc) to ~/.imrc\n"
			 "and editing it, enabling shared pixmaps.\n" "\n"));
	     return;
	  }
	if (!before)
	   DIALOG_OK(_("Starting up Raindrops FX..."),
		     _("\n" "You have just started the Raindrops Effect.\n"
		       "\n"
		       "If you look closely on your desktop background, and if it\n"
		       "doesn't have a solid colour (i.e. has a background texture or\n"
		       "image), you will see \"raindrops\" hit the background and\n"
		       "make little splashes. This Effect can be VERY CPU intensive.\n"
		       "\n"
		       "To disable this effect just select this option again to toggle\n"
		       "it off.\n"));
	before = 1;
	if (first)
	  {
	     int                 j;

	     first = 0;
	     for (i = 0; i < 256; i++)
		sintab[i] =
		   (char)(sin(((double)i) * M_PI_2 * 4 / 256) * fx_amplitude);
	     for (j = 0; j < fx_raindrop_size; j++)
	       {
		  for (i = 0; i < fx_raindrop_size; i++)
		    {
		       xx = i - fx_raindrop_size2;
		       yy = j - fx_raindrop_size2;
		       disttab[i][j] =
			  (unsigned char)sqrt((double)((xx * xx) + (yy * yy)));
		    }
	       }
	  }
	fx_raindrops_win = desks.desk[desks.current].win;
	if (gc)
	   XFreeGC(disp, gc);
	if (gc1)
	   XFreeGC(disp, gc1);
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(disp, fx_raindrops_win, GCSubwindowMode, &gcv);
	gc1 = XCreateGC(disp, fx_raindrops_win, 0L, &gcv);
	fx_raindrops_draw =
	   ECreatePixImg(fx_raindrops_win, fx_raindrop_size, fx_raindrop_size);
	if (!fx_raindrops_draw)
	   return;
	for (i = 0; i < fx_raindrops_number; i++)
	  {
	     fx_raindrops[i].buf =
		ECreatePixImg(fx_raindrops_win, fx_raindrop_size,
			      fx_raindrop_size);
	     if (fx_raindrops[i].buf)
		XShmGetImage(disp, fx_raindrops_win, fx_raindrops[i].buf->xim,
			     fx_raindrops[i].x, fx_raindrops[i].y, 0xffffffff);
	     if (!fx_raindrops[i].buf)
		return;
	  }
     }
   for (i = 0; i < fx_raindrops_number; i++)
     {
	fx_raindrops[i].count++;
	if (fx_raindrops[i].count == fx_raindrop_duration)
	  {
	     int                 j, count = 0;
	     char                intersect = 1;

	     XClearArea(disp, fx_raindrops_win, fx_raindrops[i].x,
			fx_raindrops[i].y, fx_raindrop_size, fx_raindrop_size,
			False);
	     fx_raindrops[i].count = 0;
	     while (intersect)
	       {
		  count++;
		  if (count > 10240)
		     break;
		  intersect = 0;
		  for (j = 0; j < fx_raindrops_number; j++)
		    {
		       fx_raindrops[i].x = rand() % (root.w - fx_raindrop_size);
		       fx_raindrops[i].y = rand() % (root.h - fx_raindrop_size);
		       if (fx_raindrops[i].x < 0)
			  fx_raindrops[i].x = 0;
		       else if (fx_raindrops[i].x > (root.w - fx_raindrop_size))
			  fx_raindrops[i].x = root.w - fx_raindrop_size;
		       if (fx_raindrops[i].y < 0)
			  fx_raindrops[i].y = 0;
		       else if (fx_raindrops[i].y > (root.h - fx_raindrop_size))
			  fx_raindrops[i].y = root.h - fx_raindrop_size;
		       if (i != j)
			 {
			    if (((fx_raindrops[i].x >= fx_raindrops[j].x)
				 && (fx_raindrops[i].x <
				     fx_raindrops[j].x + fx_raindrop_size)
				 && (fx_raindrops[i].y >= fx_raindrops[j].y)
				 && (fx_raindrops[i].y <
				     fx_raindrops[j].y + fx_raindrop_size))
				||
				((fx_raindrops
				  [i].x + fx_raindrop_size >= fx_raindrops[j].x)
				 && (fx_raindrops[i].x + fx_raindrop_size <
				     fx_raindrops[j].x + fx_raindrop_size)
				 && (fx_raindrops[i].y >= fx_raindrops[j].y)
				 && (fx_raindrops[i].y <
				     fx_raindrops[j].y + fx_raindrop_size))
				|| ((fx_raindrops[i].x >= fx_raindrops[j].x)
				    && (fx_raindrops[i].x <
					fx_raindrops[j].x + fx_raindrop_size)
				    && (fx_raindrops[i].y + fx_raindrop_size >=
					fx_raindrops[j].y)
				    && (fx_raindrops[i].y + fx_raindrop_size <
					fx_raindrops[j].y + fx_raindrop_size))
				||
				((fx_raindrops
				  [i].x + fx_raindrop_size >= fx_raindrops[j].x)
				 && (fx_raindrops[i].x + fx_raindrop_size <
				     fx_raindrops[j].x + fx_raindrop_size)
				 && (fx_raindrops[i].y + fx_raindrop_size >=
				     fx_raindrops[j].y)
				 && (fx_raindrops[i].y + fx_raindrop_size <
				     fx_raindrops[j].y + fx_raindrop_size)))
			       intersect = 1;
			 }
		    }
	       }
	     XShmGetImage(disp, fx_raindrops_win, fx_raindrops[i].buf->xim,
			  fx_raindrops[i].x, fx_raindrops[i].y, 0xffffffff);
	  }
	percent_done =
	   1 + ((fx_raindrops[i].count << 8) / fx_raindrop_duration);
	for (y = 0; y < fx_raindrop_size; y++)
	  {
	     for (x = 0; x < fx_raindrop_size; x++)
	       {
		  int                 dist;

		  dist = disttab[x][y];
		  if (dist > fx_raindrop_size2)
		     XPutPixel(fx_raindrops_draw->xim, x, y,
			       XGetPixel(fx_raindrops[i].buf->xim, x, y));
		  else
		    {
		       int                 percent;

		       percent = 1 + ((dist << 8) / fx_raindrop_size2);
		       if (percent > percent_done)
			  XPutPixel(fx_raindrops_draw->xim, x, y,
				    XGetPixel(fx_raindrops[i].buf->xim, x, y));
		       else
			 {
			    int                 varx, vary;
			    int                 phase, divisor, multiplier;

			    phase =
			       ((percent - percent_done) * fx_frequency) & 0xff;
			    xx = x - fx_raindrop_size2;
			    yy = y - fx_raindrop_size2;
			    divisor = 1 + (dist << 8);
			    multiplier =
			       (int)sintab[phase] * (256 - percent_done);
			    varx = ((-xx) * multiplier) / divisor;
			    vary = ((-yy) * multiplier) / divisor;
			    xx = x + varx;
			    yy = y + vary;
			    if (xx < 0)
			       xx = 0;
			    else if (xx >= fx_raindrop_size)
			       xx = fx_raindrop_size - 1;
			    if (yy < 0)
			       yy = 0;
			    else if (yy >= fx_raindrop_size)
			       yy = fx_raindrop_size - 1;
			    XPutPixel(fx_raindrops_draw->xim, x, y,
				      XGetPixel(fx_raindrops[i].buf->xim, xx,
						yy));
			 }
		    }
	       }
	  }
	XShmPutImage(disp, fx_raindrops_win, gc1, fx_raindrops_draw->xim, 0, 0,
		     fx_raindrops[i].x, fx_raindrops[i].y, fx_raindrop_size,
		     fx_raindrop_size, False);
	XSync(disp, False);
     }
   DoIn("FX_RAINDROPS_TIMEOUT", (0.066 /*/ (float)fx_raindrops_number */ ),
	FX_raindrops_timeout, 0, NULL);
   return;
   val = 0;
   data = NULL;
}

void
FX_Raindrops_Init(char *name)
{
   int                 i;

   fx_raindrops_win = 0;
   for (i = 0; i < fx_raindrops_number; i++)
     {
	fx_raindrops[i].count = rand() % fx_raindrop_duration;
	fx_raindrops[i].x = rand() % (root.w - fx_raindrop_size);
	fx_raindrops[i].y = rand() % (root.h - fx_raindrop_size);
     }
   DoIn("FX_RAINDROPS_TIMEOUT", 0.066, FX_raindrops_timeout, 0, NULL);
   name = NULL;
}

void
FX_Raindrops_Desk(void)
{
   fx_raindrops_win = 0;
}

void
FX_Raindrops_Quit(void)
{
   int                 i;

   RemoveTimerEvent("FX_RAINDROPS_TIMEOUT");
   for (i = 0; i < fx_raindrops_number; i++)
     {
	XClearArea(disp, fx_raindrops_win, fx_raindrops[i].x, fx_raindrops[i].y,
		   fx_raindrop_size, fx_raindrop_size, False);
	if (fx_raindrops[i].buf)
	   EDestroyPixImg(fx_raindrops[i].buf);
	fx_raindrops[i].buf = NULL;
     }
   if (fx_raindrops_draw)
      EDestroyPixImg(fx_raindrops_draw);
   fx_raindrops_draw = NULL;
   fx_raindrops_win = 0;
}

void
FX_Raindrops_Pause(void)
{
   static char         paused = 0;

   if (!paused)
     {
	FX_Raindrops_Quit();
	paused = 1;
     }
   else
     {
	FX_Raindrops_Init(NULL);
	paused = 0;
     }
}

/****************************************************************************/

/****************************** WAVES ***************************************/
/* by tsade :)                                                              */
/****************************************************************************/

#define FX_WAVE_WATERH 64
#define FX_WAVE_WATERW 64
#define FX_WAVE_DEPTH  10
#define FX_WAVE_GRABH  (FX_WAVE_WATERH + FX_WAVE_DEPTH)
#define FX_WAVE_CROSSPERIOD 0.42
static Pixmap       fx_wave_above = 0;
static Window       fx_wave_win = 0;
static int          fx_wave_count = 0;

void                FX_Wave_timeout(int val, void *data);

void
FX_Wave_timeout(int val, void *data)
{
   /* Variables */
   static char         before = 0;
   static double       incv = 0, inch = 0;
   static double       incx = 0;
   double              incx2;
   static GC           gc1 = 0, gc = 0;
   int                 y;

   /* Check to see if we need to create stuff */
   if (!fx_wave_above)
     {
	XGCValues           gcv;

	fx_wave_win = desks.desk[desks.current].win;
	if ((desks.current == 0) && (ird))
	   fx_wave_above =
	      XCreatePixmap(disp, fx_wave_win, root.w, FX_WAVE_WATERH * 2,
			    ird->x.depth);
	else
	   fx_wave_above =
	      XCreatePixmap(disp, fx_wave_win, root.w, FX_WAVE_WATERH * 2,
			    id->x.depth);
	if (gc)
	   XFreeGC(disp, gc);
	if (gc1)
	   XFreeGC(disp, gc1);
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(disp, fx_wave_win, GCSubwindowMode, &gcv);
	gc1 = XCreateGC(disp, fx_wave_win, 0L, &gcv);
	if (!before)
	   DIALOG_OK(_("Starting up Waves FX..."),
		     _("\n" "You have just started the Waves Effect.\n" "\n"
		       "If you look closely on your desktop background, and if it\n"
		       "doesn't have a solid colour (i.e. has a background texture or\n"
		       "image), you will see a pool of water at the bottom of your\n"
		       "screen that reflects everything above it and \"waves\".\n"
		       "\n"
		       "To disable this effect just select this option again to toggle\n"
		       "it off.\n"));
	before = 1;
     }

   /* On the zero, grab the desktop again. */
   if (fx_wave_count == 0)
     {
	XCopyArea(disp, fx_wave_win, fx_wave_above, gc, 0,
		  root.h - (FX_WAVE_WATERH * 3), root.w, FX_WAVE_WATERH * 2, 0,
		  0);
     }

   /* Increment and roll the counter */
   fx_wave_count++;
   if (fx_wave_count > 32)
      fx_wave_count = 0;

   /* Increment and roll some other variables */
   incv += 0.40;
   if (incv > (M_PI_2 * 4))
      incv = 0;

   inch += 0.32;
   if (inch > (M_PI_2 * 4))
      inch = 0;

   incx += 0.32;
   if (incx > (M_PI_2 * 4))
      incx = 0;

   /* Copy the area to correct bugs */
   if (fx_wave_count == 0)
     {
	XCopyArea(disp, fx_wave_above, fx_wave_win, gc1, 0,
		  root.h - FX_WAVE_GRABH, root.w, FX_WAVE_DEPTH * 2, 0,
		  root.h - FX_WAVE_GRABH);
     }

   /* Go through the bottom couple (FX_WAVE_WATERH) lines of the window */
   for (y = 0; y < FX_WAVE_WATERH; y++)
     {
	/* Variables */
	double              aa, a, p;
	int                 yoff, off, yy;
	int                 x;

	/* Figure out the side-to-side movement */
	p = (((double)(FX_WAVE_WATERH - y)) / ((double)FX_WAVE_WATERH));
	a = p * p * 48 + incv;
	yoff = y + (int)(sin(a) * 7) + 1;
	yy = (FX_WAVE_WATERH * 2) - yoff;
	aa = p * p * FX_WAVE_WATERH + inch;
	off = (int)(sin(aa) * 10 * (1 - p));

	/* Set up the next part */
	incx2 = incx;

	/* Go through the width of the screen, in block sizes */
	for (x = 0; x < root.w; x += FX_WAVE_WATERW)
	  {
	     /* Variables */
	     int                 sx;

	     /* Add something to incx2 and roll it */
	     incx2 += FX_WAVE_CROSSPERIOD;

	     if (incx2 > (M_PI_2 * 4))
		incx2 = 0;

	     /* Figure it out */
	     sx = (int)(sin(incx2) * FX_WAVE_DEPTH);

	     /* Display this block */
	     XCopyArea(disp, fx_wave_above, fx_wave_win, gc1, x, yy,	/* x, y */
		       FX_WAVE_WATERW, 1,	/* w, h */
		       off + x, root.h - FX_WAVE_WATERH + y + sx	/* dx, dy */
		);
	  }
     }

   /* Make noise */
   DoIn("FX_WAVE_TIMEOUT", 0.066, FX_Wave_timeout, 0, NULL);

   /* Return */
   return;

   /* Never gets here */
   val = 0;
   data = NULL;
}

void
FX_Waves_Init(char *name)
{
   fx_wave_count = 0;
   DoIn("FX_WAVE_TIMEOUT", 0.066, FX_Wave_timeout, 0, NULL);
   name = NULL;
}

void
FX_Waves_Desk(void)
{
   XFreePixmap(disp, fx_wave_above);
   fx_wave_count = 0;
   fx_wave_above = 0;
}

void
FX_Waves_Quit(void)
{
   RemoveTimerEvent("FX_WAVE_TIMEOUT");
   XClearArea(disp, fx_wave_win, 0, root.h - FX_WAVE_WATERH, root.w,
	      FX_WAVE_WATERH, False);
}

void
FX_Waves_Pause(void)
{
   static char         paused = 0;

   if (!paused)
     {
	FX_Waves_Quit();
	paused = 1;
     }
   else
     {
	FX_Waves_Init(NULL);
	paused = 0;
     }
}

/****************************************************************************/

/****************************** IMAGESPINNER ********************************/

static Window       fx_imagespinner_win = 0;
static int          fx_imagespinner_count = 3;
static ImlibData   *fx_imagespinner_imd = NULL;
static char        *fx_imagespinner_params = NULL;
static void         FX_imagespinner_timeout(int val, void *data);

void
FX_imagespinner_timeout(int val, void *data)
{
   static char         before = 0;
   char               *string = NULL;

   if (!fx_imagespinner_win)
     {
	fx_imagespinner_win = desks.desk[desks.current].win;
	if ((desks.current == 0) && (ird))
	   fx_imagespinner_imd = ird;
	else
	   fx_imagespinner_imd = id;
	if (!before)
	   DIALOG_OK(_("Starting up imagespinners FX..."),
		     _("\n" "You have just started the imagespinners Effect.\n"
		       "\n"
		       "To disable this effect just select this option again to toggle\n"
		       "it off.\n"));
	before = 1;
     }
/* do stuff here */
   string = getword(fx_imagespinner_params, fx_imagespinner_count);
   if (!string)
     {
	fx_imagespinner_count = 3;
	string = getword(fx_imagespinner_params, fx_imagespinner_count);
     }
   fx_imagespinner_count++;
   if (string)
     {
	ImlibImage         *im;

	im = ELoadImageImlibData(fx_imagespinner_imd, string);
	if (im)
	  {
	     int                 x, y, w, h;

	     w = im->rgb_width;
	     h = im->rgb_height;
	     sscanf(fx_imagespinner_params, "%*s %i %i ", &x, &y);
	     x = ((root.w * x) >> 10) - ((w * x) >> 10);
	     y = ((root.h * y) >> 10) - ((h * y) >> 10);
	     Imlib_paste_image(fx_imagespinner_imd, im, fx_imagespinner_win, x,
			       y, w, h);
	     Imlib_destroy_image(fx_imagespinner_imd, im);
	  }
	Efree(string);
     }

   DoIn("FX_IMAGESPINNER_TIMEOUT", 0.066, FX_imagespinner_timeout, 0, NULL);
   return;
   val = 0;
   data = NULL;
}

void
FX_ImageSpinner_Init(char *name)
{
   fx_imagespinner_count = 3;
   DoIn("FX_IMAGESPINNER_TIMEOUT", 0.066, FX_imagespinner_timeout, 0, NULL);
   fx_imagespinner_params = duplicate(name);
}

void
FX_ImageSpinner_Desk(void)
{
   fx_imagespinner_win = desks.desk[desks.current].win;
   if ((desks.current == 0) && (ird))
      fx_imagespinner_imd = ird;
   else
      fx_imagespinner_imd = id;
}

void
FX_ImageSpinner_Quit(void)
{
   RemoveTimerEvent("FX_IMAGESPINNER_TIMEOUT");
   XClearArea(disp, fx_imagespinner_win, 0, 0, root.w, root.h, False);
   if (fx_imagespinner_params)
      Efree(fx_imagespinner_params);
   fx_imagespinner_params = NULL;
   fx_imagespinner_win = 0;
}

void
FX_ImageSpinner_Pause(void)
{
   static char         paused = 0;

   if (!paused)
     {
	FX_ImageSpinner_Quit();
	paused = 1;
     }
   else
     {
	FX_ImageSpinner_Init(NULL);
	paused = 0;
     }
}

/****************************************************************************/
