#include "E.h"

static void         PagerUpdateTimeout(int val, void *data);

#define HIQ mode.pager_hiq
#define SNAP mode.pager_snap

void
PagerScaleLine(Pixmap dest, Window src, int dx, int dy, int sw, int pw, int sy, int sh)
{
   static GC           gc = 0;
   XGCValues           gcv;
   int                 x, x2;
   PixImg             *p_grab = NULL, *p_grab2 = NULL, *p_buf = NULL;
   XImage             *px_grab = NULL, *px_grab2 = NULL, *px_buf = NULL;

   if (!gc)
     {
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(disp, src, GCSubwindowMode, &gcv);
     }
   p_grab = ECreatePixImg(dest, sw, 1);
   if (p_grab)
     {
	if (HIQ)
	  {
	     p_grab2 = ECreatePixImg(dest, sw, 1);
	     if (!p_grab2)
	       {
		  EDestroyPixImg(p_grab);
		  p_grab = NULL;
	       }
	  }
	if (p_grab)
	  {
	     p_buf = ECreatePixImg(dest, pw, 1);
	     if (p_buf)
	       {
		  XCopyArea(disp, src, p_grab->pmap, gc, 0, sy, sw, 1, 0, 0);
		  if (HIQ)
		     XCopyArea(disp, src, p_grab2->pmap, gc, 0, sy + (sh / 2),
			       sw, 1, 0, 0);
		  XSync(disp, False);
	       }
	  }
	else
	  {
	     EDestroyPixImg(p_grab);
	     if (p_grab2)
		EDestroyPixImg(p_grab2);
	     p_grab = NULL;
	  }
     }
   if (!p_grab)
     {
	px_grab = XGetImage(disp, src, 0, sy, sw, 1, 0xffffffff, ZPixmap);
	if (HIQ)
	   px_grab2 = XGetImage(disp, src, 0, sy + (sh / 2), sw, 1, 0xffffffff, ZPixmap);
	px_buf = XCreateImage(disp, root.vis, root.depth, ZPixmap, 0, NULL, pw, 1, 32, 0);
	px_buf->data = malloc(px_buf->bytes_per_line * px_buf->height);
     }
   if (HIQ)
     {
	int                 v1, v2, v3, v4, difx;
	XImage             *xim1 = NULL, *xim2 = NULL, *xim3 = NULL;

	if (p_grab)
	  {
	     xim1 = p_grab->xim;
	     xim2 = p_grab2->xim;
	     xim3 = p_buf->xim;
	  }
	else
	  {
	     xim1 = px_grab;
	     xim2 = px_grab2;
	     xim3 = px_buf;
	  }
	difx = (sw / pw) / 2;
	switch (id->x.render_depth)
	  {
	  case 24:
	  case 32:
	     for (x = 0; x < pw; x++)
	       {
		  x2 = (sw * x) / pw;
		  v1 = XGetPixel(xim1, x2, 0);
		  v2 = XGetPixel(xim1, x2 + difx, 0);
		  v3 = XGetPixel(xim2, x2, 0);
		  v4 = XGetPixel(xim2, x2 + difx, 0);
		  v1 = ((v1 >> 2) & 0x3f3f3f3f) + ((v2 >> 2) & 0x3f3f3f3f) +
		     ((v3 >> 2) & 0x3f3f3f3f) + ((v4 >> 2) & 0x3f3f3f3f) +
		     (v1 & v2 & v3 & v4 & 0x03030303);
		  XPutPixel(xim3, x, 0, v1);
	       }
	     break;
	  case 16:
	     for (x = 0; x < pw; x++)
	       {
		  x2 = (sw * x) / pw;
		  v1 = XGetPixel(xim1, x2, 0);
		  v2 = XGetPixel(xim1, x2 + difx, 0);
		  v3 = XGetPixel(xim2, x2, 0);
		  v4 = XGetPixel(xim2, x2 + difx, 0);
		  v1 =
		     ((v1 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
		     ((v2 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
		     ((v3 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
		     ((v4 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
		     (v1 & v2 & v3 & v4 & ((0x3 << 11) | (0x3 << 5) | (0x3)));
		  XPutPixel(xim3, x, 0, v1);
	       }
	     break;
	  case 15:
	     /*        break; */
	  default:
	     for (x = 0; x < pw; x++)
	       {
		  x2 = (sw * x) / pw;
		  XPutPixel(xim3, x, 0, XGetPixel(xim1, x2, 0));
	       }
	     break;
	  }
     }
   else
     {
	XImage             *xim1 = NULL, *xim3 = NULL;

	if (p_grab)
	  {
	     xim1 = p_grab->xim;
	     xim3 = p_buf->xim;
	  }
	else
	  {
	     xim1 = px_grab;
	     xim3 = px_buf;
	  }
	for (x = 0; x < pw; x++)
	  {
	     x2 = (sw * x) / pw;
	     XPutPixel(xim3, x, 0, XGetPixel(xim1, x2, 0));
	  }
     }
   if (p_buf)
     {
	XShmPutImage(disp, dest, gc, p_buf->xim, 0, 0, dx, dy, pw, 1, False);
	XSync(disp, False);
	if (p_grab)
	   EDestroyPixImg(p_grab);
	if (p_grab2)
	   EDestroyPixImg(p_grab2);
	if (p_buf)
	   EDestroyPixImg(p_buf);
     }
   else
     {
	XPutImage(disp, dest, gc, px_buf, 0, 0, dx, dy, pw, 1);
	if (px_grab)
	   XDestroyImage(px_grab);
	if (px_grab2)
	   XDestroyImage(px_grab2);
	if (px_buf)
	   XDestroyImage(px_buf);
     }
}

void
PagerScaleRect(Pixmap dest, Window src, int sx, int sy, int dx, int dy, int sw, int sh, int dw, int dh)
{
   static GC           gc = 0, gc2 = 0;
   XGCValues           gcv;
   int                 y, y2, x, x2;
   PixImg             *p_grab = NULL, *p_buf = NULL;
   XImage             *px_grab = NULL, *px_buf = NULL;

   if (!gc)
     {
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC(disp, src, GCSubwindowMode, &gcv);
	gc2 = XCreateGC(disp, src, 0, &gcv);
     }
   if (HIQ)
      p_grab = ECreatePixImg(src, sw, dh * 2);
   else
      p_grab = ECreatePixImg(src, sw, dh);
   if (p_grab)
     {
	p_buf = ECreatePixImg(dest, dw, dh);
	if (p_buf)
	  {
	     if (HIQ)
	       {
		  for (y = 0; y < (dh * 2); y++)
		    {
		       y2 = (sh * y) / (dh * 2);
		       XCopyArea(disp, src, p_grab->pmap, gc, sx, sy + y2, sw, 1, 0, y);
		    }
	       }
	     else
	       {
		  for (y = 0; y < dh; y++)
		    {
		       y2 = (sh * y) / dh;
		       XCopyArea(disp, src, p_grab->pmap, gc, sx, sy + y2, sw, 1, 0, y);
		    }
	       }
	     XSync(disp, False);
	  }
	else
	  {
	     EDestroyPixImg(p_grab);
	     p_grab = NULL;
	  }
     }
   if (!p_grab)
     {
	if (HIQ)
	  {
	     Pixmap              pmap;

	     pmap = ECreatePixmap(disp, src, sw, dh * 2, root.depth);
	     for (y = 0; y < (dh * 2); y++)
	       {
		  y2 = (sh * y) / (dh * 2);
		  XCopyArea(disp, src, pmap, gc, sx, sy + y2, sw, 1, 0, y);
	       }
	     px_grab = XGetImage(disp, pmap, 0, 0, sw, dh * 2, 0xffffffff, ZPixmap);
	     EFreePixmap(disp, pmap);
	  }
	else
	  {
	     Pixmap              pmap;

	     pmap = ECreatePixmap(disp, src, sw, dh, root.depth);
	     for (y = 0; y < dh; y++)
	       {
		  y2 = (sh * y) / dh;
		  XCopyArea(disp, src, pmap, gc, sx, sy + y2, sw, 1, 0, y);
	       }
	     px_grab = XGetImage(disp, pmap, 0, 0, sw, dh, 0xffffffff, ZPixmap);
	     EFreePixmap(disp, pmap);
	  }
	px_buf = XCreateImage(disp, root.vis, root.depth, ZPixmap, 0, NULL, dw, dh, 32, 0);
	px_buf->data = malloc(px_buf->bytes_per_line * px_buf->height);
     }
   if (HIQ)
     {
	int                 v1, v2, v3, v4, difx;
	XImage             *xim1 = NULL, *xim3 = NULL;

	if (p_grab)
	  {
	     xim1 = p_grab->xim;
	     xim3 = p_buf->xim;
	  }
	else
	  {
	     xim1 = px_grab;
	     xim3 = px_buf;
	  }
	difx = (sw / dw) / 2;
	switch (root.depth)
	  {
	  case 24:
	  case 32:
	     for (y = 0; y < dh; y++)
	       {
		  for (x = 0; x < dw; x++)
		    {
		       y2 = (y * 2);
		       x2 = (sw * x) / dw;
		       v1 = XGetPixel(xim1, x2, y2);
		       v2 = XGetPixel(xim1, x2 + difx, y2);
		       v3 = XGetPixel(xim1, x2, y2 + 1);
		       v4 = XGetPixel(xim1, x2 + difx, y2 + 1);
		       v1 = ((v1 >> 2) & 0x3f3f3f3f) + ((v2 >> 2) & 0x3f3f3f3f) +
			  ((v3 >> 2) & 0x3f3f3f3f) + ((v4 >> 2) & 0x3f3f3f3f) +
			  (v1 & v2 & v3 & v4 & 0x03030303);
		       XPutPixel(xim3, x, y, v1);
		    }
	       }
	     break;
	  case 16:
	     for (y = 0; y < dh; y++)
	       {
		  for (x = 0; x < dw; x++)
		    {
		       y2 = (y * 2);
		       x2 = (sw * x) / dw;
		       v1 = XGetPixel(xim1, x2, y2);
		       v2 = XGetPixel(xim1, x2 + difx, y2);
		       v3 = XGetPixel(xim1, x2, y2 + 1);
		       v4 = XGetPixel(xim1, x2 + difx, y2 + 1);
		       v1 =
			  ((v1 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
			  ((v2 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
			  ((v3 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
			  ((v4 >> 2) & ((0x70 << 7) | (0x78 << 2) | (0x70 >> 4))) +
			  (v1 & v2 & v3 & v4 & ((0x3 << 11) | (0x3 << 5) | (0x3)));
		       XPutPixel(xim3, x, y, v1);
		    }
	       }
	     break;
	  case 15:
	     /*        break; */
	  default:
	     for (y = 0; y < dh; y++)
	       {
		  for (x = 0; x < dw; x++)
		    {
		       y2 = (y * 2);
		       x2 = (sw * x) / dw;
		       XPutPixel(xim3, x, y, XGetPixel(xim1, x2, y2));
		    }
	       }
	     break;
	  }
     }
   else
     {
	XImage             *xim1 = NULL, *xim3 = NULL;

	if (p_grab)
	  {
	     xim1 = p_grab->xim;
	     xim3 = p_buf->xim;
	  }
	else
	  {
	     xim1 = px_grab;
	     xim3 = px_buf;
	  }
	for (y = 0; y < dh; y++)
	  {
	     for (x = 0; x < dw; x++)
	       {
		  x2 = (sw * x) / dw;
		  XPutPixel(xim3, x, y, XGetPixel(xim1, x2, y));
	       }
	  }
     }
   if (p_buf)
     {
	XShmPutImage(disp, dest, gc2, p_buf->xim, 0, 0, dx, dy, dw, dh, False);
	XSync(disp, False);
	if (p_grab)
	   EDestroyPixImg(p_grab);
	if (p_buf)
	   EDestroyPixImg(p_buf);
     }
   else
     {
	XPutImage(disp, dest, gc, px_buf, 0, 0, dx, dy, dw, dh);
	if (px_grab)
	   XDestroyImage(px_grab);
	if (px_buf)
	   XDestroyImage(px_buf);
     }
}

static void
PagerUpdateTimeout(int val, void *data)
{
   Pager              *p;
   char                s[4096];
   int                 y, y2, phase, ax, ay, cx, cy, ww, hh, xx, yy;
   static int          offsets[8] =
   {0, 4, 2, 6, 1, 5, 3, 7};

   p = (Pager *) data;
   Esnprintf(s, sizeof(s), "__.%x", p->win);
   if (mode.pager_scanspeed > 0)
      DoIn(s, 1 / ((double)mode.pager_scanspeed), PagerUpdateTimeout, 0, p);
   if (!SNAP)
      return;
   if (!p->visible)
      return;
   if (p->desktop != desks.current)
      return;
   if (mode.mode != MODE_NONE)
      return;

   GetAreaSize(&ax, &ay);
   GetCurrentArea(&cx, &cy);
   ww = p->w / ax;
   hh = p->h / ay;
   xx = cx * ww;
   yy = cy * hh;
   phase = p->update_phase;
   y = ((phase & 0xfffffff8) + offsets[phase % 8]) % hh;
   y2 = (y * root.h) / hh;

   PagerScaleLine(p->pmap, root.win, xx, yy + y, root.w, ww, y2, (root.h / hh));
   XClearArea(disp, p->win, xx, yy + y, ww, 1, False);

   p->update_phase++;
   if (p->update_phase >= p->h)
     {
	int                 i;

	for (i = 0; i < desks.desk[p->desktop].num; i++)
	   PagerEwinUpdateFromPager(p, desks.desk[p->desktop].list[i]);
	p->update_phase = 0;
     }
   val = 0;
}

Pager              *
CreatePager(void)
{
   Pager              *p;
   int                 ax, ay;
   char                pq;
   ImageClass         *ic;
   XSetWindowAttributes attr;
   static char         did_dialog = 0;

   if (!mode.show_pagers)
      return NULL;
   if ((!did_dialog) && (SNAP))
     {
	if (id->x.shm)
	  {
	     if (!id->x.shmp)
	       {
		  if (XShmPixmapFormat(disp) != ZPixmap)
		    {
		       SettingsPager();
		       DIALOG_OK
			  ("Warning!",
			   "\n"
			   "You seem to have an X Server capable of Shared Memory\n"
		       "but it is incapable of doing ZPixmap Shared pixmaps\n"
			"(The server does not claim to be able to do them).\n"
			   "\n"
			   "The pager in enlightenment will run slowly in snapshot\n"
			"mode if you continue to use that mode of the pager\n"
			   "under these conditions.\n"
			   "\n"
			   "It is suggested you change the settings on your pager to\n"
			   "disable snapshots to improve performance.\n"
			   "\n");
		    }
		  else
		     DIALOG_OK
			("Warning!",
			 "\n"
			 "Your X Server is capable of doing Shared Memory but you do\n"
			 "not have Shared Pixmaps enabled in your Imlib configuration.\n"
			 "\n"
			 "Please enable Shared Pixmaps in your Imlib configuration\n"
			 "then restart enlightenment to gain better performance for\n"
			 "the pagers when snapshot mode is enabled.\n"
			 "\n");
	       }
	  }
	else
	  {
	     SettingsPager();
	     DIALOG_OK
		("Warning!",
		 "\n"
	     "You seem to be running Enlightenment over a network Connection\n"
	     "or on an X Server that does not support Shared Memory, or you\n"
		 "have disabled MIT-SHM Shared memory in your Imlib configuration.\n"
	     "This means the Enlightenment Pager will perform slowly and use\n"
		 "more system resources than it would when Shared Memory is\n"
		 "available.\n"
		 "\n"
		 "To improve performance please either enable MIT-SHM Shared Memory\n"
		 "in your Imlib config, if you disabled it, or disable Pager\n"
		 "snapshots.\n"
		 "\n");
	  }
	did_dialog = 1;
     }
   GetAreaSize(&ax, &ay);
   p = Emalloc(sizeof(Pager));
   p->name = NULL;
   attr.colormap = root.cmap;
   attr.border_pixel = 0;
   attr.background_pixel = 0;
   attr.save_under = False;
   p->win = ECreateWindow(root.win, 0, 0, ((48 * root.w) / root.h) * ax, 48 * ay, 0);
   p->pmap = ECreatePixmap(disp, p->win, ((48 * root.w) / root.h) * ax, 48 * ay, root.depth);
   p->bgpmap = ECreatePixmap(disp, p->win, ((48 * root.w) / root.h) * ax, 48, root.depth);
   ESetWindowBackgroundPixmap(disp, p->win, p->pmap);
   XSelectInput(disp, p->win, ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask);
   p->hi_win = ECreateWindow(root.win, 0, 0, 3, 3, 0);
   p->hi_visible = 0;
   p->hi_ewin = NULL;
   XSelectInput(disp, p->hi_win, ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask);
   p->desktop = 0;
   p->w = ((48 * root.w) / root.h) * ax;
   p->h = 48 * ay;
   p->dw = ((48 * root.w) / root.h) * ax;
   p->dh = 48;
   p->visible = 0;
   p->update_phase = 0;
   p->ewin = NULL;
   p->border_name = NULL;
   p->sel_win = ECreateWindow(p->win, 0, 0, ((48 * root.w) / root.h) * ax, 48, 0);
   pq = queue_up;
   queue_up = 0;
   ic = FindItem("PAGER_SEL", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
   if (ic)
      IclassApply(ic, p->sel_win, ((48 * root.w) / root.h) * ax, 48, 0, 0, STATE_NORMAL, 0);
   queue_up = pq;
   return p;
}

void
PagerResize(Pager * p, int w, int h)
{
   int                 ax, ay, i;
   char                pq;
   ImageClass         *ic;

   if (!mode.show_pagers)
      return;
   if ((w == p->w) && (h == p->h))
      return;
   GetAreaSize(&ax, &ay);
   EFreePixmap(disp, p->pmap);
   EFreePixmap(disp, p->bgpmap);
   EResizeWindow(disp, p->win, w, h);
   p->w = w;
   p->h = h;
   p->dw = w / ax;
   p->dh = h / ay;
   p->pmap = ECreatePixmap(disp, p->win, p->w, p->h, root.depth);
   p->bgpmap = ECreatePixmap(disp, p->win, p->w / ax, p->h / ay, root.depth);
   if (p->visible)
      PagerRedraw(p, 1);
   ESetWindowBackgroundPixmap(disp, p->win, p->pmap);
   XClearWindow(disp, p->win);
   if (p->ewin)
     {
	double              aspect;

	aspect = ((double)root.w) / ((double)root.h);
	p->ewin->client.w_inc = ax * 4;
	p->ewin->client.h_inc = ay * 8;
	p->ewin->client.aspect_min = aspect * ((double)ax / (double)ay);
	p->ewin->client.aspect_max = aspect * ((double)ax / (double)ay);
     }
   pq = queue_up;
   queue_up = 0;
   ic = FindItem("PAGER_SEL", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
   if (ic)
     {
	EResizeWindow(disp, p->sel_win, w / ax, h / ay);
	IclassApply(ic, p->sel_win, w / ax, h / ay, 0, 0, STATE_NORMAL, 0);
     }
   queue_up = pq;
   for (i = 0; i < desks.desk[p->desktop].num; i++)
      PagerEwinUpdateMini(p, desks.desk[p->desktop].list[i]);
}

void
PagerShow(Pager * p)
{
   EWin               *ewin = NULL;
   XClassHint         *xch;
   char                s[64];

   if (!mode.show_pagers)
      return;
   if (p->ewin)
     {
	ShowEwin(p->ewin);
	return;
     }
   Esnprintf(s, sizeof(s), "%i", p->desktop);
   xch = XAllocClassHint();
   xch->res_name = s;
   xch->res_class = "Enlightenment_Pager";
   XSetClassHint(disp, p->win, xch);
   XFree(xch);
   if (p->border_name)
      ewin = AddInternalToFamily(p->win, 1, p->border_name);
   else
      ewin = AddInternalToFamily(p->win, 1, "PAGER");
   if (ewin)
     {
	char                s[4096];
	int                 ax, ay, pw, ph;
	Snapshot           *sn;
	double              aspect;

	aspect = ((double)root.w) / ((double)root.h);
	GetAreaSize(&ax, &ay);
	ewin->client.aspect_min = aspect * ((double)ax / (double)ay);
	ewin->client.aspect_max = aspect * ((double)ax / (double)ay);
	ewin->client.w_inc = ax * 4;
	ewin->client.h_inc = ay * 8;
	ewin->client.width.min = 10 * ax;
	ewin->client.height.min = 8 * ay;
	ewin->client.width.max = 320 * ax;
	ewin->client.height.max = 240 * ay;
	ewin->pager = p;
	ewin->desktop = desks.current;
	if (!pager_group)
	  {
	     BuildWindowGroup(&ewin, 1);
	     pager_group = ewin->group;
	  }
	else
	   AddEwinToGroup(ewin, pager_group);
	p->ewin = ewin;
	p->visible = 1;
	DesktopRemoveEwin(ewin);
	ewin->sticky = 1;
	sn = FindSnapshot(ewin);
	/* get the size right damnit! */
	if (sn)
	  {
	     if (sn->use_wh)
		ResizeEwin(ewin, sn->w, sn->h);
	  }
	/* no snapshots ? first time ? make a row on the top left */
	else
	   MoveEwin(ewin, 0, p->desktop * ewin->h);
	DesktopAddEwinToTop(ewin);
	RestackEwin(ewin);
	/* force a redraw & resize */
	pw = p->w;
	ph = p->h;
	p->w = 0;
	p->h = 0;
	PagerResize(p, pw, ph);
	PagerRedraw(p, 1);
	/* show the pager ewin */
	ShowEwin(ewin);
	SnapshotEwinBorder(ewin);
	SnapshotEwinDesktop(ewin);
	SnapshotEwinSize(ewin);
	SnapshotEwinLocation(ewin);
	SnapshotEwinLayer(ewin);
	SnapshotEwinSticky(ewin);
	SnapshotEwinShade(ewin);
	if (SNAP)
	  {
	     Esnprintf(s, sizeof(s), "__.%x", p->win);
	     if (mode.pager_scanspeed > 0)
		DoIn(s, 1 / ((double)mode.pager_scanspeed), PagerUpdateTimeout, 0, p);
	  }
	AddItem(p, "PAGER", p->win, LIST_TYPE_PAGER);
     }
}

void
PagerKill(Pager * p)
{
   char                s[4096];

   RemoveItem("PAGER", p->win, LIST_FINDBY_ID, LIST_TYPE_PAGER);
   Esnprintf(s, sizeof(s), "__.%x", p->win);
   RemoveTimerEvent(s);
   if (p->name)
      Efree(p->name);
   EDestroyWindow(disp, p->win);
   if (p->hi_win)
      EDestroyWindow(disp, p->hi_win);
   EFreePixmap(disp, p->pmap);
   EFreePixmap(disp, p->bgpmap);
   if (p->border_name)
      Efree(p->border_name);
   Efree(p);
}

Pager             **
PagersForDesktop(int d, int *num)
{
   Pager             **pp = NULL;
   Pager             **pl = NULL;
   int                 i, pnum;

   if (!mode.show_pagers)
      return NULL;
   *num = 0;
   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   if (pl)
     {
	for (i = 0; i < pnum; i++)
	  {
	     if (pl[i]->desktop == d)
	       {
		  (*num)++;
		  pp = Erealloc(pp, sizeof(Pager *) * (*num));
		  pp[(*num) - 1] = pl[i];
	       }
	  }
	Efree(pl);
     }
   return pp;
}

void
RedrawPagersForDesktop(int d, char newbg)
{
   Pager             **pl;
   int                 i, num;

   if (!mode.show_pagers)
      return;
   pl = PagersForDesktop(d, &num);
   if (pl)
     {
	for (i = 0; i < num; i++)
	   PagerRedraw(pl[i], newbg);
	Efree(pl);
     }
}

void
ForceUpdatePagersForDesktop(int d)
{
   Pager             **pl;
   int                 i, num;

   if (!mode.show_pagers)
      return;
   pl = PagersForDesktop(d, &num);
   if (pl)
     {
	for (i = 0; i < num; i++)
	   PagerForceUpdate(pl[i]);
	Efree(pl);
     }
}

void
PagerEwinUpdateMini(Pager * p, EWin * ewin)
{
   int                 w, h, ax, ay, cx, cy;

   if (!mode.show_pagers)
      return;
   GetAreaSize(&ax, &ay);
   cx = desks.desk[p->desktop].current_area_x;
   cy = desks.desk[p->desktop].current_area_y;

   w = ((ewin->w) * (p->w / ax)) / root.w;
   h = ((ewin->h) * (p->h / ay)) / root.h;

   if (w < 1)
      w = 1;
   if (h < 1)
      h = 1;
   if ((ewin->mini_w != w) || (ewin->mini_h != h))
     {
	ewin->mini_w = w;
	ewin->mini_h = h;

	if (ewin->mini_pmap)
	   EFreePixmap(disp, ewin->mini_pmap);
	if (ewin->mini_mask)
	   EFreePixmap(disp, ewin->mini_mask);
	ewin->mini_pmap = 0;
	ewin->mini_mask = 0;

	if ((ewin->desktop != desks.current) || (ewin->area_x != cx) ||
	    (ewin->area_y != cy) || (!SNAP))
	  {
	     ImageClass         *ic = NULL;

	     ic = FindItem("PAGER_WIN", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
	     if (ic)
		IclassApplyCopy(ic, ewin->win, w, h, 0, 0, STATE_NORMAL,
				&(ewin->mini_pmap), &(ewin->mini_mask));
	  }
	else
	  {
	     ewin->mini_pmap = ECreatePixmap(disp, p->win, w, h, root.depth);
	     PagerScaleRect(ewin->mini_pmap, ewin->win, 0, 0, 0, 0,
			    ewin->w, ewin->h, w, h);
	  }
     }
   if (p->hi_ewin == ewin)
     {
	ImlibImage         *im;

	im = Imlib_create_image_from_drawable(id, ewin->mini_pmap, 0, 0, 0,
					      ewin->mini_w, ewin->mini_h);
	Imlib_apply_image(id, im, p->hi_win);
	Imlib_kill_image(id, im);
     }
}

void
PagerEwinUpdateFromPager(Pager * p, EWin * ewin)
{
   int                 x, y, w, h, ax, ay, cx, cy;
   static GC           gc = 0;
   XGCValues           gcv;

   if (!SNAP)
     {
	PagerEwinUpdateMini(p, ewin);
	return;
     }
   if (!mode.show_pagers)
      return;
   GetAreaSize(&ax, &ay);
   cx = desks.desk[p->desktop].current_area_x;
   cy = desks.desk[p->desktop].current_area_y;
   x = ((ewin->x + (cx * root.w)) * (p->w / ax)) / root.w;
   y = ((ewin->y + (cy * root.h)) * (p->h / ay)) / root.h;
   w = ((ewin->w) * (p->w / ax)) / root.w;
   h = ((ewin->h) * (p->h / ay)) / root.h;
   if (!gc)
      gc = XCreateGC(disp, p->pmap, 0, &gcv);
   if ((ewin->mini_w != w) || (ewin->mini_h != h))
     {
	if (ewin->mini_pmap)
	   EFreePixmap(disp, ewin->mini_pmap);
	if (ewin->mini_mask)
	   EFreePixmap(disp, ewin->mini_mask);
	ewin->mini_pmap = 0;
	ewin->mini_mask = 0;
     }
   if (!ewin->mini_pmap)
     {
	ewin->mini_w = w;
	ewin->mini_h = h;
	ewin->mini_pmap = ECreatePixmap(disp, p->win, w, h, root.depth);
     }
   XCopyArea(disp, p->pmap, ewin->mini_pmap, gc, x, y, w, h, 0, 0);
   if (p->hi_ewin == ewin)
     {
	ImlibImage         *im;

	im = Imlib_create_image_from_drawable(id, ewin->mini_pmap, 0, 0, 0,
					      ewin->mini_w, ewin->mini_h);
	Imlib_apply_image(id, im, p->hi_win);
	Imlib_kill_image(id, im);
     }
}

void
PagerRedraw(Pager * p, char newbg)
{
   int                 i, x, y, ax, ay, cx, cy;
   static GC           gc = 0;
   XGCValues           gcv;
   int                 c1, c2, r, g, b;

   if (!mode.show_pagers)
      return;
   if (queue_up)
     {
	DrawQueue          *dq;

	dq = Emalloc(sizeof(DrawQueue));
	dq->win = p->win;
	dq->iclass = NULL;
	dq->w = p->w;
	dq->h = p->h;
	dq->active = 0;
	dq->sticky = 0;
	dq->state = 0;
	dq->expose = 0;
	dq->tclass = NULL;
	dq->text = NULL;
	dq->shape_propagate = 0;
	dq->pager = NULL;
	dq->redraw_pager = p;
	dq->newbg = newbg;
	AddItem(dq, "DRAW", dq->win, LIST_TYPE_DRAW);
	return;
     }
   p->update_phase = 0;
   GetAreaSize(&ax, &ay);
   cx = desks.desk[p->desktop].current_area_x;
   cy = desks.desk[p->desktop].current_area_y;
   if (!gc)
      gc = XCreateGC(disp, p->pmap, 0, &gcv);
   r = 0;
   g = 0;
   b = 0;
   c1 = Imlib_best_color_match(id, &r, &g, &b);
   r = 255;
   g = 255;
   b = 255;
   c2 = Imlib_best_color_match(id, &r, &g, &b);
   if ((newbg > 0) && (newbg < 3))
     {
	if (!SNAP)
	  {
	     ImageClass         *ic = NULL;

	     Imlib_free_pixmap(id, p->bgpmap);
	     ic = FindItem("PAGER_BACKGROUND", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
	     if (ic)
		IclassApplyCopy(ic, p->win, p->w / ax, p->h / ay, 0, 0, STATE_NORMAL,
				&(p->bgpmap), NULL);
	  }
	else
	  {
	     if (desks.desk[p->desktop].bg)
	       {
		  char                s[4096];
		  char               *home, *uniq;
		  ImlibImage         *im;

		  home = homedir(getuid());
		  uniq = GetUniqueBGString(desks.desk[p->desktop].bg);
		  Esnprintf(s, sizeof(s),
			    "%s/.enlightenment/cached/pager/%s.%i.%i.%s",
			    home, desks.desk[p->desktop].bg->name, (p->w / ax),
			    (p->h / ay), uniq);
		  Efree(home);
		  Efree(uniq);

		  im = Imlib_load_image(id, s);
		  if (im)
		    {
		       Imlib_free_pixmap(id, p->bgpmap);
		       Imlib_render(id, im, (p->w / ax), (p->h / ay));
		       p->bgpmap = Imlib_move_image(id, im);
		       Imlib_destroy_image(id, im);
		    }
		  else
		    {
		       SetBackgroundTo(id, p->bgpmap, desks.desk[p->desktop].bg, 0);
		       im = Imlib_create_image_from_drawable(id, p->bgpmap, 0, 0, 0,
							     (p->w / ax),
							     (p->h / ay));
		       Imlib_save_image_to_ppm(id, im, s);
		       Imlib_changed_image(id, im);
		       Imlib_kill_image(id, im);
		    }
	       }
	     else
	       {
		  XSetForeground(disp, gc, c1);
		  XDrawRectangle(disp, p->bgpmap, gc, 0, 0, p->dw, p->dh);
		  XSetForeground(disp, gc, c2);
		  XFillRectangle(disp, p->bgpmap, gc, 1, 1, p->dw - 2, p->dh - 2);
	       }
	  }
     }
   for (y = 0; y < ay; y++)
     {
	for (x = 0; x < ax; x++)
	   XCopyArea(disp, p->bgpmap, p->pmap, gc, 0, 0, p->w / ax, p->h / ay,
		     x * (p->w / ax), y * (p->h / ay));
     }
   for (i = desks.desk[p->desktop].num - 1; i >= 0; i--)
     {
	EWin               *ewin;
	int                 wx, wy, ww, wh;

	ewin = desks.desk[p->desktop].list[i];
	if (!ewin->iconified)
	  {
	     wx = ((ewin->x + (cx * root.w)) * (p->w / ax)) / root.w;
	     wy = ((ewin->y + (cy * root.h)) * (p->h / ay)) / root.h;
	     ww = ((ewin->w) * (p->w / ax)) / root.w;
	     wh = ((ewin->h) * (p->h / ay)) / root.h;
	     PagerEwinUpdateMini(p, ewin);
	     if (ewin->mini_pmap)
	       {
		  if (ewin->mini_mask)
		    {
		       XSetClipMask(disp, gc, ewin->mini_mask);
		       XSetClipOrigin(disp, gc, wx, wy);
		    }
		  XCopyArea(disp, ewin->mini_pmap, p->pmap, gc, 0, 0, ww, wh, wx, wy);
		  if (ewin->mini_mask)
		    {
		       XSetClipMask(disp, gc, None);
		    }
	       }
	     else
	       {
		  XSetForeground(disp, gc, c1);
		  XDrawRectangle(disp, p->pmap, gc, wx - 1, wy - 1, ww + 1, wh + 1);
		  XSetForeground(disp, gc, c2);
		  XFillRectangle(disp, p->pmap, gc, wx, wy, ww, wh);
	       }
	  }
     }
   if (newbg < 2)
      XClearWindow(disp, p->win);
}

void
PagerForceUpdate(Pager * p)
{
   int                 ww, hh, xx, yy, ax, ay, cx, cy, i;

   if (!mode.show_pagers)
      return;
   if (queue_up)
     {
	DrawQueue          *dq;

	dq = Emalloc(sizeof(DrawQueue));
	dq->win = p->win;
	dq->iclass = NULL;
	dq->w = p->w;
	dq->h = p->h;
	dq->active = 0;
	dq->sticky = 0;
	dq->state = 0;
	dq->expose = 0;
	dq->tclass = NULL;
	dq->text = NULL;
	dq->shape_propagate = 0;
	dq->pager = p;
	dq->redraw_pager = NULL;
	AddItem(dq, "DRAW", dq->win, LIST_TYPE_DRAW);
	return;
     }
   if ((p->desktop != desks.current) || (!SNAP))
     {
	PagerRedraw(p, 0);
	return;
     }
   p->update_phase = 0;
   GetAreaSize(&ax, &ay);
   cx = desks.desk[p->desktop].current_area_x;
   cy = desks.desk[p->desktop].current_area_y;
   ww = p->w / ax;
   hh = p->h / ay;
   xx = cx * ww;
   yy = cy * hh;

   PagerScaleRect(p->pmap, root.win, 0, 0, xx, yy, root.w, root.h, ww, hh);
   XClearWindow(disp, p->win);

   for (i = 0; i < desks.desk[p->desktop].num; i++)
      PagerEwinUpdateFromPager(p, desks.desk[p->desktop].list[i]);
}

void
PagerReArea(void)
{
   Pager             **pl = NULL;
   int                 i, pnum, w, h, ax, ay;

   if (!mode.show_pagers)
      return;
   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   GetAreaSize(&ax, &ay);
   if (pl)
     {
	for (i = 0; i < pnum; i++)
	  {
	     pl[i]->w = 0;
	     pl[i]->h = 0;
	     w = pl[i]->dw * ax;
	     h = pl[i]->dh * ay;
	     PagerResize(pl[i], w, h);
	     if (pl[i]->ewin)
		ResizeEwin(pl[i]->ewin, w, h);
	  }
	Efree(pl);
     }
}

void
PagerEwinOutsideAreaUpdate(EWin * ewin)
{
   if (!mode.show_pagers)
      return;
   if (ewin->sticky)
     {
	int                 i;

	for (i = 0; i < mode.numdesktops; i++)
	   RedrawPagersForDesktop(i, 0);
	ForceUpdatePagersForDesktop(ewin->desktop);
	return;
     }
   else if (ewin->desktop != desks.current)
     {
	RedrawPagersForDesktop(ewin->desktop, 0);
	ForceUpdatePagersForDesktop(ewin->desktop);
	return;
     }
   if ((ewin->x < 0) || (ewin->y < 0) || ((ewin->x + ewin->w) > root.w) ||
       ((ewin->y + ewin->h) > root.h))
      RedrawPagersForDesktop(ewin->desktop, 3);
   ForceUpdatePagersForDesktop(ewin->desktop);
}

EWin               *
EwinInPagerAt(Pager * p, int x, int y)
{
   int                 i, wx, wy, ww, wh, ax, ay, cx, cy;

   if (!mode.show_pagers)
      return NULL;
   GetAreaSize(&ax, &ay);
   cx = desks.desk[p->desktop].current_area_x;
   cy = desks.desk[p->desktop].current_area_y;
   for (i = 0; i < desks.desk[p->desktop].num; i++)
     {
	EWin               *ewin;

	ewin = desks.desk[p->desktop].list[i];
	if ((ewin->visible) && (!ewin->iconified))
	  {
	     wx = ((ewin->x + (cx * root.w)) * (p->w / ax)) / root.w;
	     wy = ((ewin->y + (cy * root.h)) * (p->h / ay)) / root.h;
	     ww = ((ewin->w) * (p->w / ax)) / root.w;
	     wh = ((ewin->h) * (p->h / ay)) / root.h;
	     if ((x >= wx) && (y >= wy) && (x < (wx + ww)) && (y < (wy + wh)))
		return ewin;
	  }
     }
   return NULL;
}

void
PagerAreaAt(Pager * p, int x, int y, int *ax, int *ay)
{
   int                 asx, asy;

   if (!mode.show_pagers)
      return;
   GetAreaSize(&asx, &asy);
   *ax = x / (p->w / asx);
   *ay = y / (p->h / asy);
}

void
PagerShowMenu(Pager * p, int x, int y)
{
   static Menu        *p_menu = NULL, *pw_menu = NULL;
   MenuItem           *mi;
   EWin               *ewin;
   char                s[1024];
   int                 ax, ay;

   if (!mode.show_pagers)
      return;
   ewin = EwinInPagerAt(p, x, y);
   if (ewin)
     {
	if (pw_menu)
	   DestroyMenu(pw_menu);
	pw_menu = CreateMenu();
	AddTitleToMenu(pw_menu, "Window Options");
	pw_menu->name = duplicate("__DESK_WIN_MENU");
	pw_menu->style = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_MENU_STYLE);

	Esnprintf(s, sizeof(s), "%i", ewin->client.win);
	mi = CreateMenuItem("Iconify", NULL, ACTION_ICONIFY, s, NULL);
	AddItemToMenu(pw_menu, mi);

	mi = CreateMenuItem("Close", NULL, ACTION_KILL, s, NULL);
	AddItemToMenu(pw_menu, mi);

	mi = CreateMenuItem("Annihilate", NULL, ACTION_KILL_NASTY, s, NULL);
	AddItemToMenu(pw_menu, mi);

	mi = CreateMenuItem("Stick / Unstick", NULL, ACTION_STICK, s, NULL);
	AddItemToMenu(pw_menu, mi);

	AddItem(pw_menu, pw_menu->name, 0, LIST_TYPE_MENU);
	Esnprintf(s, sizeof(s), "named %s", pw_menu->name);
	spawnMenu(s);
	return;
     }
   PagerAreaAt(p, x, y, &ax, &ay);
   if (p_menu)
      DestroyMenu(p_menu);
   p_menu = CreateMenu();
   AddTitleToMenu(p_menu, "Desktop Options");
   p_menu->name = duplicate("__DESK_MENU");
   p_menu->style = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_MENU_STYLE);

   mi = CreateMenuItem("Pager Settings...", NULL, ACTION_CONFIG, "pager", NULL);
   AddItemToMenu(p_menu, mi);

   mi = CreateMenuItem("  ", NULL, 0, NULL, NULL);
   AddItemToMenu(p_menu, mi);

   mi = CreateMenuItem("Snapshotting On", NULL, ACTION_SET_PAGER_SNAP, "1", NULL);
   AddItemToMenu(p_menu, mi);

   mi = CreateMenuItem("Snapshotting Off", NULL, ACTION_SET_PAGER_SNAP, "0", NULL);
   AddItemToMenu(p_menu, mi);

   if (SNAP)
     {
	mi = CreateMenuItem("  ", NULL, 0, NULL, NULL);
	AddItemToMenu(p_menu, mi);

	mi = CreateMenuItem("High Quality On", NULL, ACTION_SET_PAGER_HIQ, "1", NULL);
	AddItemToMenu(p_menu, mi);

	mi = CreateMenuItem("High Quality Off", NULL, ACTION_SET_PAGER_HIQ, "0", NULL);
	AddItemToMenu(p_menu, mi);
     }
   AddItem(p_menu, p_menu->name, 0, LIST_TYPE_MENU);
   Esnprintf(s, sizeof(s), "named %s", p_menu->name);
   spawnMenu(s);
}

void
PagerHide(Pager * p)
{
   if (p->ewin)
      HideEwin(p->ewin);
}

void
PagerTitle(Pager * p, char *title)
{
   XTextProperty       xtp;

   if (!mode.show_pagers)
      return;
   xtp.encoding = XA_STRING;
   xtp.format = 8;
   xtp.value = (unsigned char *)(title);
   xtp.nitems = strlen((char *)(xtp.value));
   XSetWMName(disp, p->win, &xtp);
}

void
UpdatePagerSel(void)
{
   Pager             **pl;
   Pager              *p;
   int                 i, pnum, cx, cy;

   if (!mode.show_pagers)
      return;
   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   if (pl)
     {
	for (i = 0; i < pnum; i++)
	  {
	     p = pl[i];
	     if (p->desktop != desks.current)
		EUnmapWindow(disp, p->sel_win);
	     else
	       {
		  cx = desks.desk[p->desktop].current_area_x;
		  cy = desks.desk[p->desktop].current_area_y;
		  EMoveWindow(disp, p->sel_win, cx * p->dw, cy * p->dh);
		  EMapWindow(disp, p->sel_win);
	       }
	  }
	Efree(pl);
     }
}

void
PagerHideAllHi(void)
{
   Pager             **pl = NULL;
   int                 i, pnum;

   if (!mode.show_pagers)
      return;
   pl = (Pager **) ListItemType(&pnum, LIST_TYPE_PAGER);
   if (pl)
     {
	for (i = 0; i < pnum; i++)
	   PagerHideHi(pl[i]);
	Efree(pl);
     }
}

void
PagerHideHi(Pager * p)
{
   ToolTip            *tt = NULL;

   if (p->hi_visible)
     {
	p->hi_visible = 0;
	EUnmapWindow(disp, p->hi_win);

	tt = FindItem("PAGER", 0, LIST_FINDBY_NAME,
		      LIST_TYPE_TOOLTIP);
	if (tt)
	   HideToolTip(tt);
     }
}

void
PagerShowHi(Pager * p, EWin * ewin, int x, int y, int w, int h)
{
   ImageClass         *ic = NULL;

   if (mode.pager_zoom)
     {
	ic = FindItem("PAGER_WIN", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
	EMoveResizeWindow(disp, p->hi_win, x, y, w, h);
	EMapRaised(disp, p->hi_win);
	if (ewin->mini_pmap)
	  {
	     ImlibImage         *im;
	     int                 xx, yy, ww, hh, i;

	     im = Imlib_create_image_from_drawable(id, ewin->mini_pmap, 0, 0, 0,
						   ewin->mini_w, ewin->mini_h);
	     if (w > h)
	       {
		  for (i = w; i < (w * 2); i++)
		    {
		       Pixmap              pmap;

		       ww = i;
		       hh = (i * h) / w;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       Imlib_render(id, im, ww, hh);
		       pmap = Imlib_move_image(id, im);
		       ESetWindowBackgroundPixmap(disp, p->hi_win, pmap);
		       Imlib_free_pixmap(id, pmap);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       Imlib_kill_image(id, im);
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     else
	       {
		  for (i = h; i < (h * 2); i++)
		    {
		       Pixmap              pmap;

		       ww = (i * w) / h;
		       hh = i;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       Imlib_render(id, im, ww, hh);
		       pmap = Imlib_move_image(id, im);
		       ESetWindowBackgroundPixmap(disp, p->hi_win, pmap);
		       Imlib_free_pixmap(id, pmap);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       Imlib_kill_image(id, im);
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     EMoveResizeWindow(disp, p->hi_win, x - (w / 2), y - (h / 2),
			       w * 2, h * 2);
	     Imlib_apply_image(id, im, p->hi_win);
	     Imlib_kill_image(id, im);
	  }
	else if (ic)
	  {
	     char                pq;
	     int                 xx, yy, ww, hh, i;

	     pq = queue_up;
	     queue_up = 0;
	     if (w > h)
	       {
		  for (i = w; i < (w * 2); i++)
		    {
		       ww = i;
		       hh = (i * h) / w;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       IclassApply(ic, p->hi_win, ww, hh, 0, 0, STATE_NORMAL, 0);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     else
	       {
		  for (i = h; i < (h * 2); i++)
		    {
		       ww = (i * w) / h;
		       hh = i;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       IclassApply(ic, p->hi_win, ww, hh, 0, 0, STATE_NORMAL, 0);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     EMoveResizeWindow(disp, p->hi_win, x - (w / 2), y - (h / 2),
			       w * 2, h * 2);
	     queue_up = pq;
	  }
	else
	  {
	     Pixmap              pmap;
	     GC                  gc = 0;
	     XGCValues           gcv;
	     int                 c1, c2, r, g, b;
	     int                 xx, yy, ww, hh, i;

	     r = 0;
	     g = 0;
	     b = 0;
	     c1 = Imlib_best_color_match(id, &r, &g, &b);
	     r = 255;
	     g = 255;
	     b = 255;
	     c2 = Imlib_best_color_match(id, &r, &g, &b);
	     pmap = ECreatePixmap(disp, p->hi_win, w * 2, h * 2, root.depth);
	     ESetWindowBackgroundPixmap(disp, p->hi_win, pmap);
	     if (!gc)
		gc = XCreateGC(disp, pmap, 0, &gcv);
	     if (w > h)
	       {
		  for (i = w; i < (w * 2); i++)
		    {
		       ww = i;
		       hh = (i * h) / w;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       XSetForeground(disp, gc, c1);
		       XFillRectangle(disp, pmap, gc, 0, 0, ww, hh);
		       XSetForeground(disp, gc, c2);
		       XFillRectangle(disp, pmap, gc, 1, 1, ww - 2, hh - 2);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       EFreePixmap(disp, pmap);
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     else
	       {
		  for (i = h; i < (h * 2); i++)
		    {
		       ww = (i * w) / h;
		       hh = i;
		       xx = x + ((w - ww) / 2);
		       yy = y + ((h - hh) / 2);
		       XSetForeground(disp, gc, c1);
		       XFillRectangle(disp, pmap, gc, 0, 0, ww, hh);
		       XSetForeground(disp, gc, c2);
		       XFillRectangle(disp, pmap, gc, 1, 1, ww - 2, hh - 2);
		       EMoveResizeWindow(disp, p->hi_win, xx, yy, ww, hh);
		       XClearWindow(disp, p->hi_win);
		       {
			  int                 px, py;

			  PointerAt(&px, &py);
			  if ((px < x) || (py < y) || (px >= (x + w)) || (py >= (y + h)))
			    {
			       EFreePixmap(disp, pmap);
			       EUnmapWindow(disp, p->hi_win);
			       return;
			    }
		       }
		    }
	       }
	     EFreePixmap(disp, pmap);
	     EMoveResizeWindow(disp, p->hi_win, x - (w / 2), y - (h / 2),
			       w * 2, h * 2);
	  }
	p->hi_visible = 1;
     }
   if (mode.pager_title)
     {
	ToolTip            *tt = NULL;

	tt = FindItem("PAGER", 0, LIST_FINDBY_NAME,
		      LIST_TYPE_TOOLTIP);
	if (tt)
	  {
	     ShowToolTip(tt, ewin->client.title, NULL, mode.x, mode.y);
	     p->hi_visible = 1;
	  }
     }
}

void
PagerHandleMotion(Pager * p, Window win, int x, int y)
{
   int                 hx, hy;
   int                 px, py;
   Window              dw;
   EWin               *ewin = NULL;

   if (!mode.show_pagers)
      return;
   if (!p)
     {
	PagerHideAllHi();
	return;
     }
   XTranslateCoordinates(disp, p->win, root.win, 0, 0, &px, &py, &dw);
   if (win == p->hi_win)
     {
	XTranslateCoordinates(disp, p->hi_win, p->win, 0, 0, &hx, &hy, &dw);
	x += hx;
	y += hy;
     }
   ewin = EwinInPagerAt(p, x, y);
   if (ewin != p->hi_ewin)
     {

	PagerHideHi(p);
	p->hi_ewin = ewin;
	if (ewin)
	  {
	     int                 wx, wy, ww, wh, ax, ay, cx, cy;

	     GetAreaSize(&ax, &ay);
	     cx = desks.desk[p->desktop].current_area_x;
	     cy = desks.desk[p->desktop].current_area_y;
	     wx = ((ewin->x + (cx * root.w)) * (p->w / ax)) / root.w;
	     wy = ((ewin->y + (cy * root.h)) * (p->h / ay)) / root.h;
	     ww = ((ewin->w) * (p->w / ax)) / root.w;
	     wh = ((ewin->h) * (p->h / ay)) / root.h;
	     PagerShowHi(p, ewin, px + wx, py + wy, ww, wh);
	  }
     }
}

void
NewPagerForDesktop(int desk)
{

   Pager              *p;
   char                s[1024];

   p = CreatePager();
   if (p)
     {
	p->desktop = desk;
	Esnprintf(s, sizeof(s), "%i", desk);
	PagerTitle(p, s);
	PagerShow(p);
     }
}

void
EnableSinglePagerForDesktop(int desk)
{

   Pager              *p;
   Pager             **pl;
   char                s[1024];
   int                 num;

   pl = PagersForDesktop(desk, &num);
   if (!pl)
     {
	p = CreatePager();
	if (p)
	  {
	     p->desktop = desk;
	     Esnprintf(s, sizeof(s), "%i", desk);
	     PagerTitle(p, s);
	     PagerShow(p);
	  }
     }
   else
     {
	Efree(pl);
     }

}

void
EnableAllPagers(void)
{

   int                 i;

   if (!mode.show_pagers)
     {
	mode.show_pagers = 1;
	for (i = 0; i < mode.numdesktops; i++)
	  {
	     EnableSinglePagerForDesktop(i);
	  }
	UpdatePagerSel();
     }
   return;
}

int
PagerForDesktop(int desk)
{
   Pager             **pl;
   int                 num;

   pl = PagersForDesktop(desk, &num);
   if (pl)
     {
	Efree(pl);
     }
   return num;

}

void
DisablePagersForDesktop(int desk)
{
   Pager             **pl;

   int                 i, num;

   pl = PagersForDesktop(desk, &num);
   if (pl)
     {
	for (i = 0; i < num; i++)
	  {
	     if (pl[i]->ewin)
		ICCCM_Delete(pl[i]->ewin);
	  }
	Efree(pl);
     }
}

void
DisableAllPagers(void)
{

   int                 i;

   if (mode.show_pagers)
     {
	for (i = 0; i < mode.numdesktops; i++)
	  {
	     DisablePagersForDesktop(i);
	  }
	mode.show_pagers = 0;
     }
   return;
}

void
PagerSetHiQ(char onoff)
{
   Pager             **pl;
   EWin              **lst;
   int                 i, num;

   HIQ = onoff;

   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     lst[i]->mini_w = 0;
	     lst[i]->mini_h = 0;
	  }
	Efree(lst);
     }
   pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
   if (pl)
     {
	for (i = 0; i < num; i++)
	  {
	     PagerHideHi(pl[i]);
	     PagerRedraw(pl[i], 2);
	     PagerForceUpdate(pl[i]);
	  }
	Efree(pl);
     }
}

void
PagerSetSnap(char onoff)
{
   Pager             **pl;
   EWin              **lst;
   int                 i, num;

   SNAP = onoff;

   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     lst[i]->mini_w = 0;
	     lst[i]->mini_h = 0;
	  }
	Efree(lst);
     }
   pl = (Pager **) ListItemType(&num, LIST_TYPE_PAGER);
   if (pl)
     {
	for (i = 0; i < num; i++)
	  {
	     PagerHideHi(pl[i]);
	     PagerRedraw(pl[i], 2);
	     PagerForceUpdate(pl[i]);
	  }
	Efree(pl);
     }
}
