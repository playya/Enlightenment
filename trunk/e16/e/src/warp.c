/*
 * Author: Merlin Hughes
 *  - merlin@merlin.org
 *
 *  This code is free software.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  TODO: The Warp Focus Title Window could list all windows in the
 *  warp ring with pretty window icons, and display which one is
 *  currently selected instead of just the title of the currently
 *  selected one.
 */

#include "E.h"

#define XK_MISCELLANY
#ifdef WITH_TARTY_WARP
#include <X11/XKBlib.h>
#endif
#include <X11/keysymdef.h>

static int          xkbEventNumber = -1;
static int          warpFocusAltPressed = 0;
static int          warpFocusIndex = 0;
static char         warpFocusTitleShowing = 0;
static Window       warpFocusTitleWindow = 0;
static int          warptitles_num = 0;
static Window      *warptitles = NULL;
static EWin       **warptitles_ewin = NULL;

void
WarpFocusInitEvents(void)
{
   int                 xkbOpCode, xkbEventBase, xkbErrorBase;
   int                 xkbMajor, xkbMinor;

#ifdef WITH_TARTY_WARP
   if (XkbQueryExtension(disp, &xkbOpCode, &xkbEventBase, &xkbErrorBase,
			 &xkbMajor, &xkbMinor))
     {
	xkbEventNumber = xkbEventBase + XkbEventCode;
	XkbSelectEventDetails(disp, XkbUseCoreKbd, XkbStateNotify,
			      XkbAllStateComponentsMask,
			      XkbAllStateComponentsMask);
     }
   else
      mode.display_warp = -1;
#endif
}

int
WarpFocusHandleEvent(XEvent * ev)
{
   EDBUG(5, "WarpFocusHandleEvent");
#ifdef WITH_TARTY_WARP
   if ((!mode.display_warp) || (xkbEventNumber < 0))
     {
	EDBUG_RETURN(0);
     }
   if (ev->type == xkbEventNumber)
     {
	XkbEvent           *xev;

	xev = (XkbEvent *) ev;
	if (xev->any.xkb_type == XkbStateNotify)
	  {
	     XkbStateNotifyEvent *sn;
	     KeySym              keySym;

	     sn = &xev->state;
	     keySym = XKeycodeToKeysym(disp, sn->keycode, 0);
	     if ((keySym == XK_Alt_L) || (keySym == XK_Alt_R) ||
		 (keySym == XK_Shift_L) || (keySym == XK_Shift_R) ||
		 (keySym == XK_Control_L) || (keySym == XK_Control_R) ||
		 (keySym == XK_Meta_L) || (keySym == XK_Meta_R) ||
		 (keySym == XK_Super_L) || (keySym == XK_Super_R) ||
		 (keySym == XK_Hyper_L) || (keySym == XK_Hyper_R))
	       {
		  int                 newAltPressed;

		  newAltPressed = (sn->event_type == KeyPress);
		  if ((warpFocusAltPressed) && (!newAltPressed))
		     WarpFocusFinish();
		  warpFocusAltPressed = newAltPressed;
	       }
	  }
	EDBUG_RETURN(1);
     }
#endif
   EDBUG_RETURN(0);
}

void
WarpFocus(int delta)
{
   EWin              **lst0, **lst, *ewin;
   int                 i, num0, num;

   EDBUG(5, "WarpFocus");
   if ((!mode.display_warp) || (xkbEventNumber < 0))
     {
	EDBUG_RETURN_;
     }
   lst = (EWin **) ListItemType(&num, LIST_TYPE_WARP_RING);
   if (!lst)
     {
	num0 = desks.desk[desks.current].num;
	lst0 = desks.desk[desks.current].list;
	num = 0;
	lst = NULL;
	if (lst0)
	  {
	     for (i = num0 - 1; i >= 0; --i)
	       {
		  ewin = lst0[i];
		  if (((ewin->sticky) || (ewin->desktop == desks.current)) &&
		      (ewin->x + ewin->w > 0) && (ewin->x < root.w) &&
		      (ewin->y + ewin->h > 0) && (ewin->y < root.h) &&
		      (!ewin->skipfocus) && (!ewin->shaded) &&
		      (!ewin->menu) && (!ewin->pager) && (!ewin->ibox) &&
		      (!ewin->iconified)	/*&& (ewin->client.mwm_decor_title) &&
						 * (ewin->client.mwm_decor_border) */ )
		     AddItem(ewin, "", 0, LIST_TYPE_WARP_RING);
	       }
	  }
	MoveItemToListBottom(mode.focuswin, LIST_TYPE_WARP_RING);
	lst = (EWin **) ListItemType(&num, LIST_TYPE_WARP_RING);
	warpFocusIndex = num - 1;
     }
   ewin = NULL;
   if (lst)
     {
	warpFocusIndex = (warpFocusIndex + num + delta) % num;
	ewin = lst[warpFocusIndex];
	if (!FindItem((char *)ewin, 0, LIST_FINDBY_POINTER, LIST_TYPE_EWIN))
	   ewin = NULL;
	if (ewin)
	  {
	     if (mode.raise_on_next_focus)
		RaiseEwin(ewin);
	     if (mode.warp_on_next_focus)
		XWarpPointer(disp, None, ewin->win, 0, 0, 0, 0, ewin->w / 2, ewin->h / 2);
	     /* if (mode.focusmode == FOCUS_CLICK) */
	     /* FocusToEWin(ewin); */
	  }
	WarpFocusShowTitle(ewin);
	Efree(lst);
     }
   EDBUG_RETURN_;
}

void
WarpFocusFinish(void)
{
   EWin              **lst, *ewin;
   int                 num;

   EDBUG(5, "WarpFocusFinish");
   lst = (EWin **) ListItemType(&num, LIST_TYPE_WARP_RING);
   if (lst)
     {
	ewin = lst[warpFocusIndex];

	WarpFocusHideTitle();
	if (!FindItem((char *)ewin, 0, LIST_FINDBY_POINTER, LIST_TYPE_EWIN))
	   ewin = NULL;
	if (ewin)
	  {
	     FocusToEWin(ewin);
	     if (mode.warp_after_next_focus || mode.warp_on_next_focus)
	       {
		  XWarpPointer(disp, None, ewin->win, 0, 0, 0, 0, ewin->w / 2, ewin->h / 2);
	       }
	     if (mode.raise_after_next_focus)
		RaiseEwin(ewin);
	  }
	/*printf ("Raise: %s\n", ewin->client.title); */
	Efree(lst);
	while (RemoveItem("", 0, LIST_FINDBY_NONE, LIST_TYPE_WARP_RING));
     }
   EDBUG_RETURN_;
}

void
WarpFocusShowTitle(EWin * ewin)
{
   TextClass          *tc;
   ImageClass         *ic;
   char                pq;
   int                 i, x, y, w, h, num, ww, hh;
   static int          mw, mh;

   tc = FindItem("WARPFOCUS", 0, LIST_FINDBY_NAME, LIST_TYPE_TCLASS);
   if (!tc)
      tc = FindItem("COORDS", 0, LIST_FINDBY_NAME, LIST_TYPE_TCLASS);
   ic = FindItem("WARPFOCUS", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
   if (!ic)
      ic = FindItem("COORDS", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
   if ((!ic) || (!tc))
      return;
   if (!warpFocusTitleWindow)
      warpFocusTitleWindow = ECreateWindow(root.win, 0, 0, 1, 1, 1);

   pq = queue_up;
   queue_up = 0;
   XRaiseWindow(disp, warpFocusTitleWindow);
   if (!warpFocusTitleShowing)
     {
	EWin              **lst;

	w = 0;
	h = 0;
	lst = (EWin **) ListItemType(&num, LIST_TYPE_WARP_RING);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  warptitles_num++;
		  warptitles =
		     Erealloc(warptitles, (sizeof(Window) * warptitles_num));
		  warptitles_ewin =
		     Erealloc(warptitles_ewin, (sizeof(EWin *) * warptitles_num));
		  warptitles[warptitles_num - 1] =
		     ECreateWindow(warpFocusTitleWindow, 0, 0, 1, 1, 0);
		  EMapWindow(disp, warptitles[warptitles_num - 1]);
		  warptitles_ewin[warptitles_num - 1] = lst[i];
		  TextSize(tc, 0, 0, 0, lst[i]->client.title, &ww, &hh, 17);
		  if (ww > w)
		     w = ww;
		  if (hh > h)
		     h = hh;
	       }
	     Efree(lst);
	  }
	w += (ic->padding.left + ic->padding.right);
	h += (ic->padding.top + ic->padding.bottom);
	x = (root.w - w) / 2;
	y = (root.h - (h * warptitles_num)) / 2;
	mw = w;
	mh = h;
	EMoveResizeWindow(disp, warpFocusTitleWindow,
			  x, y, w, (h * warptitles_num));
	for (i = 0; i < warptitles_num; i++)
	  {
	     EMoveResizeWindow(disp, warptitles[i], 0, (h * i), mw, mh);
	     if (ewin == warptitles_ewin[i])
		IclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_CLICKED, 0);
	     else
		IclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_NORMAL, 0);
	  }
	PropagateShapes(warpFocusTitleWindow);
	EMapWindow(disp, warpFocusTitleWindow);
     }
   for (i = 0; i < warptitles_num; i++)
     {
	if (!FindItem((char *)warptitles_ewin[i], 0,
		      LIST_FINDBY_POINTER, LIST_TYPE_EWIN))
	   warptitles_ewin[i] = NULL;
	if (warptitles_ewin[i])
	  {
	     if (ewin == warptitles_ewin[i])
	       {
		  IclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_CLICKED, 0);
		  TclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_CLICKED, 0,
			      tc, warptitles_ewin[i]->client.title);
	       }
	     else
	       {
		  IclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_NORMAL, 0);
		  TclassApply(ic, warptitles[i], mw, mh, 0, 0, STATE_NORMAL, 0,
			      tc, warptitles_ewin[i]->client.title);
	       }
	  }
     }
   PropagateShapes(warpFocusTitleWindow);
   queue_up = pq;
   XFlush(disp);
   warpFocusTitleShowing = 1;
}

void
WarpFocusHideTitle(void)
{
   int                 i;

   if (warpFocusTitleWindow)
     {
	EUnmapWindow(disp, warpFocusTitleWindow);
	for (i = 0; i < warptitles_num; i++)
	   EDestroyWindow(disp, warptitles[i]);
     }
   if (warptitles)
      Efree(warptitles);
   if (warptitles_ewin)
      Efree(warptitles_ewin);
   warptitles = NULL;
   warptitles_ewin = NULL;
   warptitles_num = 0;
   warpFocusTitleShowing = 0;
}
