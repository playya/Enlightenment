/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2005 Kim Woelders
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
#include "desktops.h"		/* FIXME - Should not be here */
#include "emodule.h"
#include "ewins.h"
#include "xwin.h"

#define EwinListFocusRaise(ewin) EobjListFocusRaise(EoObj(ewin))
#define EwinListFocusLower(ewin) EobjListFocusLower(EoObj(ewin))

static char         focus_inhibit = 1;
static char         focus_is_set = 0;
static int          focus_pending_why = 0;
static EWin        *focus_pending_ewin = NULL;
static EWin        *focus_pending_new = NULL;

void
FocusEnable(int on)
{
   if (on)
     {
	if (focus_inhibit > 0)
	   focus_inhibit--;
     }
   else
     {
	focus_inhibit++;
     }

   if (EventDebug(EDBUG_TYPE_FOCUS))
      Eprintf("FocusEnable inhibit=%d\n", focus_inhibit);
}

/*
 * Return !0 if it is OK to focus ewin.
 */
static int
FocusEwinValid(EWin * ewin, int want_on_screen, int click, int want_visible)
{
   if (!ewin)
      return 0;

#if 0
   Eprintf("FocusEwinValid %#lx %s: cl=%d(%d) vis=%d(%d)\n",
	   _EwinGetClientXwin(ewin), EwinGetName(ewin), click,
	   ewin->props.focusclick, want_visible, ewin->state.visibility);
#endif

   if (ewin->state.inhibit_focus)
      return 0;

   if (!EwinIsMapped(ewin) || !EoIsShown(ewin))
      return 0;

   if (ewin->props.focusclick && !click)
      return 0;

   if (want_visible && ewin->state.visibility == VisibilityFullyObscured)
      return 0;

   return !want_on_screen || EwinIsOnScreen(ewin);
}

/*
 * Return the ewin to focus after entering area or losing focused window.
 */
static EWin        *
FocusEwinSelect(void)
{
   EWin               *const *lst, *ewin;
   int                 num, i;

   switch (Conf.focus.mode)
     {
     default:
     case MODE_FOCUS_POINTER:
	ewin = GetEwinPointerInClient();
	break;

     case MODE_FOCUS_SLOPPY:
	ewin = GetEwinPointerInClient();
	if (ewin && FocusEwinValid(ewin, 1, 0, 0))
	   break;
	goto do_select;

     case MODE_FOCUS_CLICK:
	goto do_select;

      do_select:
	ewin = NULL;
	lst = EwinListFocusGet(&num);
	for (i = 0; i < num; i++)
	  {
	     if (!FocusEwinValid(lst[i], 1, 0, 0) ||
		 lst[i]->props.skip_focuslist)
		continue;
	     ewin = lst[i];
	     break;
	  }
	break;
     }

   return ewin;
}

static void
AutoraiseTimeout(int val, void *data __UNUSED__)
{
   EWin               *ewin;

   if (Conf.focus.mode == MODE_FOCUS_CLICK)
      return;

   ewin = EwinFindByClient(val);
   if (ewin)
      RaiseEwin(ewin);
}

static void
ReverseTimeout(int val, void *data __UNUSED__)
{
   EWin               *ewin;

   ewin = EwinFindByClient(val);
   if (ewin)
      EwinListFocusRaise(ewin);
}

void
FocusGetNextEwin(void)
{
   EWin               *const *lst;
   EWin               *ewin;
   int                 i, num;

   lst = EwinListFocusGet(&num);
   if (num <= 1)
      return;

   ewin = NULL;
   for (i = num - 1; i >= 0; i--)
     {
	if (!FocusEwinValid(lst[i], 1, 0, 0) || lst[i]->props.skip_focuslist)
	   continue;
	ewin = lst[i];
	break;
     }

   if (ewin)
      FocusToEWin(ewin, FOCUS_NEXT);
}

void
FocusGetPrevEwin(void)
{
   EWin               *const *lst;
   EWin               *ewin;
   int                 i, num;

   lst = EwinListFocusGet(&num);
   if (num <= 1)
      return;

   ewin = NULL;
   for (i = 1; i < num; i++)
     {
	if (!FocusEwinValid(lst[i], 1, 0, 0) || lst[i]->props.skip_focuslist)
	   continue;
	ewin = lst[i];
	break;
     }

   if (ewin)
      FocusToEWin(ewin, FOCUS_PREV);
}

void
FocusEwinSetGrabs(EWin * ewin)
{
   if ((Conf.focus.mode != MODE_FOCUS_CLICK &&
	ewin->state.active && Conf.focus.clickraises &&
	ewin != EwinListStackGetTop()) ||
       (Conf.focus.mode == MODE_FOCUS_CLICK && !ewin->state.active))
     {
	GrabButtonSet(AnyButton, AnyModifier, _EwinGetContainerWin(ewin),
		      ButtonPressMask, ECSR_PGRAB, 1);
#if 0
	Eprintf("FocusEwinSetGrabs: %#lx grab %s\n", _EwinGetClientXwin(ewin),
		EwinGetName(ewin));
#endif
     }
   else
     {
	GrabButtonRelease(AnyButton, AnyModifier, _EwinGetContainerWin(ewin));
#if 0
	Eprintf("FocusEwinSetGrabs: %#lx ungrab %s\n", _EwinGetClientXwin(ewin),
		EwinGetName(ewin));
#endif
     }
}

static void
FocusEwinSetActive(EWin * ewin, int active)
{
   ewin->state.active = active;
   EwinBorderUpdateState(ewin);

   FocusEwinSetGrabs(ewin);

   if (active && ewin->state.attention)
     {
	ewin->state.attention = 0;
	HintsSetWindowState(ewin);
     }
}

void
FocusFix(void)
{
   EWin               *const *lst, *ewin;
   int                 i, num;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	ewin = lst[i];
	GrabButtonRelease(AnyButton, AnyModifier, _EwinGetContainerWin(ewin));
	FocusEwinSetGrabs(ewin);
     }
}

static void
doFocusToEwin(EWin * ewin, int why)
{
   int                 do_focus = 0;
   int                 do_raise = 0, do_warp = 0;

   if (focus_inhibit)
      return;

   if (EventDebug(EDBUG_TYPE_FOCUS))
      Eprintf("doFocusToEWin %#lx %s why=%d\n",
	      (ewin) ? _EwinGetClientXwin(ewin) : 0,
	      (ewin) ? EwinGetName(ewin) : "None", why);

   switch (why)
     {
     case FOCUS_NEXT:
     case FOCUS_PREV:
	if (Conf.focus.raise_on_next)
	   do_raise = 1;
	if (Conf.focus.warp_on_next)
	   do_warp = 1;
	/* Fall thru */
     default:
     case FOCUS_SET:
     case FOCUS_ENTER:
     case FOCUS_LEAVE:		/* Unused */
     case FOCUS_CLICK:
	if (ewin == Mode.focuswin)
	   return;
	if (ewin == NULL)	/* Unfocus */
	   break;
	if (!FocusEwinValid(ewin, 1, why == FOCUS_CLICK, 0))
	   return;
	break;

     case FOCUS_INIT:
     case FOCUS_DESK_ENTER:
	ewin = FocusEwinSelect();
	break;

     case FOCUS_NONE:
     case FOCUS_DESK_LEAVE:
	ewin = NULL;
	if (ewin == Mode.focuswin)
	   return;
	break;

     case FOCUS_EWIN_GONE:
	if (Mode.focuswin)
	   return;
	ewin = FocusEwinSelect();
	if (ewin == Mode.focuswin)
	   ewin = NULL;
	break;

     case FOCUS_EWIN_NEW:
	if (Conf.focus.all_new_windows_get_focus)
	  {
	     do_focus = 1;
	  }
	else if (Mode.place.doing_manual)
	  {
	     do_focus = 1;
	  }

	if (EwinIsTransient(ewin))
	  {
	     if (Conf.focus.new_transients_get_focus)
	       {
		  do_focus = 2;
	       }
	     else if (Conf.focus.new_transients_get_focus_if_group_focused)
	       {
		  EWin               *ewin2;

		  ewin2 = EwinFindByClient(EwinGetTransientFor(ewin));
		  if ((ewin2) && (Mode.focuswin == ewin2))
		     do_focus = 2;
	       }

	     if (do_focus == 2)
		DeskGotoByEwin(ewin);
	  }

	if (!do_focus)
	   return;
	if (!FocusEwinValid(ewin, 1, 0, 0))
	   return;
	break;
     }

   if (ewin == Mode.focuswin && focus_is_set)
      return;

   /* Check if ewin is a valid focus window target */

   if (!ewin)
      goto done;

   /* NB! ewin != NULL */

   if (why != FOCUS_CLICK && ewin->props.focusclick)
      return;

   if (Conf.autoraise.enable)
     {
	RemoveTimerEvent("AUTORAISE_TIMEOUT");

	if (Conf.focus.mode != MODE_FOCUS_CLICK)
	   DoIn("AUTORAISE_TIMEOUT", 0.001 * Conf.autoraise.delay,
		AutoraiseTimeout, _EwinGetClientXwin(ewin), NULL);
     }

   if (do_raise)
      RaiseEwin(ewin);

   if (Conf.focus.warp_always)
      do_warp = 1;
   if (do_warp && ewin != Mode.mouse_over_ewin)
     {
	EWarpPointer(EoGetWin(ewin), EoGetW(ewin) / 2, EoGetH(ewin) / 2);
	Mode.mouse_over_ewin = ewin;
     }

   RemoveTimerEvent("REVERSE_FOCUS_TIMEOUT");
   switch (why)
     {
     case FOCUS_PREV:
	DoIn("REVERSE_FOCUS_TIMEOUT", 0.5, ReverseTimeout,
	     _EwinGetClientXwin(ewin), NULL);
	break;
     case FOCUS_DESK_ENTER:
	break;
     default:
     case FOCUS_INIT:
     case FOCUS_NEXT:
	EwinListFocusRaise(ewin);
	break;
     }

   SoundPlay("SOUND_FOCUS_SET");
 done:

   /* Unset old focus window (if any) highlighting */
   if (Mode.focuswin)
      FocusEwinSetActive(Mode.focuswin, 0);
   ICCCM_Cmap(ewin);

   /* Quit if pointer is not on our screen */
   if (!EQueryPointer(None, NULL, NULL, NULL, NULL))
     {
	Mode.focuswin = NULL;
	return;
     }

   /* Set new focus window (if any) highlighting */
   Mode.focuswin = ewin;
   if (Mode.focuswin)
      FocusEwinSetActive(Mode.focuswin, 1);
   if (why != FOCUS_DESK_LEAVE)
      ICCCM_Focus(ewin);
   focus_is_set = 1;
}

void
FocusToEWin(EWin * ewin, int why)
{
   if (EventDebug(EDBUG_TYPE_FOCUS))
      Eprintf("FocusToEWin(%d) %#lx %s why=%d\n", focus_inhibit,
	      (ewin) ? _EwinGetClientXwin(ewin) : 0,
	      (ewin) ? EwinGetName(ewin) : "None", why);

   switch (why)
     {
     case FOCUS_EWIN_NEW:
	if (!FocusEwinValid(ewin, 1, 0, 0))
	   break;
	focus_pending_new = ewin;
	focus_pending_why = why;
	focus_pending_ewin = ewin;
	break;

     default:
	if (ewin && !FocusEwinValid(ewin, 1, why == FOCUS_CLICK, 0))
	   break;
	focus_pending_why = why;
	focus_pending_ewin = ewin;
	break;

     case FOCUS_EWIN_GONE:
	focus_pending_why = why;
	focus_pending_ewin = NULL;
	if (ewin == Mode.focuswin)
	  {
	     Mode.focuswin = NULL;
	     focus_is_set = 0;
	  }
	if (ewin == focus_pending_new)
	   focus_pending_new = NULL;
	break;
     }
}

static void
FocusSet(void)
{
   if (focus_pending_new && Conf.focus.all_new_windows_get_focus)
      doFocusToEwin(focus_pending_new, FOCUS_EWIN_NEW);
   else
      doFocusToEwin(focus_pending_ewin, focus_pending_why);
   focus_pending_why = 0;
   focus_pending_ewin = focus_pending_new = NULL;
}

void
FocusNewDeskBegin(void)
{
   FocusToEWin(NULL, FOCUS_DESK_LEAVE);
}

void
FocusNewDesk(void)
{
   /* Set the mouse-over window */
   Mode.mouse_over_ewin = GetEwinByCurrentPointer();

   FocusToEWin(NULL, FOCUS_DESK_ENTER);
}

static void
FocusInit(void)
{
   /* Start focusing windows */
   FocusEnable(1);

   /* Set the mouse-over window */
   Mode.mouse_over_ewin = GetEwinByCurrentPointer();

   focus_pending_why = 0;
   focus_pending_ewin = focus_pending_new = NULL;
   FocusToEWin(NULL, FOCUS_INIT);
   FocusSet();

   /* Enable window placement features */
   Mode.place.enable_features = 1;
}

static void
FocusExit(void)
{
}

/*
 * Focus event handlers
 */

void
FocusHandleEnter(EWin * ewin, XEvent * ev)
{
   Window              win = ev->xcrossing.window;

   Mode.mouse_over_ewin = ewin;

   if (!ewin)
     {
	/* Entering root may mean entering this screen */
	if (win == VRoot.win &&
	    (ev->xcrossing.mode == NotifyNormal &&
	     ev->xcrossing.detail != NotifyInferior))
	  {
	     FocusToEWin(NULL, FOCUS_DESK_ENTER);
	     return;
	  }
     }

   switch (Conf.focus.mode)
     {
     default:
     case MODE_FOCUS_CLICK:
	break;
     case MODE_FOCUS_SLOPPY:
	if (FocusEwinValid(ewin, 1, 0, 0))
	   FocusToEWin(ewin, FOCUS_ENTER);
	break;
     case MODE_FOCUS_POINTER:
	if (!ewin || FocusEwinValid(ewin, 1, 0, 0))
	   FocusToEWin(ewin, FOCUS_ENTER);
	break;
     }
}

void
FocusHandleLeave(EWin * ewin __UNUSED__, XEvent * ev)
{
   Window              win = ev->xcrossing.window;

   /* Leaving root may mean entering other screen */
   if (win == VRoot.win &&
       (ev->xcrossing.mode == NotifyNormal &&
	ev->xcrossing.detail != NotifyInferior))
      FocusToEWin(NULL, FOCUS_DESK_LEAVE);
}

void
FocusHandleClick(EWin * ewin, Window win)
{
#if 0
   EWin               *ewin;

   ewin = EwinFindByChildren(win);
   if (!ewin)
      ewin = EwinFindByFrame(win);
   if (!ewin)
      return;
#endif

   if ((Conf.focus.clickraises) || (Conf.focus.mode == MODE_FOCUS_CLICK))
     {
	RaiseEwin(ewin);
	FocusToEWin(ewin, FOCUS_CLICK);

	/* Remove grabs if on top of stacking list */
	FocusEwinSetGrabs(ewin);

	/* allow click to pass thorugh */
	if (EventDebug(EDBUG_TYPE_GRABS))
	   Eprintf("FocusHandleClick %#lx %#lx\n", win,
		   _EwinGetContainerXwin(ewin));
	if (win == _EwinGetContainerXwin(ewin))
	  {
	     ESync();
	     XAllowEvents(disp, ReplayPointer, CurrentTime);
	     ESync();
	  }
     }
   else if (ewin->props.focusclick)
     {
	FocusToEWin(ewin, FOCUS_CLICK);
     }
}

/*      
 * Configuration dialog
 */
static int          tmp_focus;
static char         tmp_clickalways;
static char         tmp_new_focus;
static char         tmp_popup_focus;
static char         tmp_owner_popup_focus;
static char         tmp_raise_focus;
static char         tmp_warp_focus;
static char         tmp_warp_always;

static char         tmp_display_warp;
static char         tmp_warp_after_focus;
static char         tmp_raise_after_focus;
static char         tmp_showsticky;
static char         tmp_showshaded;
static char         tmp_showiconified;
static char         tmp_warpfocused;
static int          tmp_warp_icon_mode;

static void
CB_ConfigureFocus(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	Conf.focus.mode = tmp_focus;
	Conf.focus.all_new_windows_get_focus = tmp_new_focus;
	Conf.focus.new_transients_get_focus = tmp_popup_focus;
	Conf.focus.new_transients_get_focus_if_group_focused =
	   tmp_owner_popup_focus;
	Conf.focus.raise_on_next = tmp_raise_focus;
	Conf.focus.warp_on_next = tmp_warp_focus;
	Conf.focus.warp_always = tmp_warp_always;

	Conf.warplist.enable = tmp_display_warp;
	Conf.warplist.warp_on_select = tmp_warp_after_focus;
	Conf.warplist.raise_on_select = tmp_raise_after_focus;
	Conf.warplist.showsticky = tmp_showsticky;
	Conf.warplist.showshaded = tmp_showshaded;
	Conf.warplist.showiconified = tmp_showiconified;
	Conf.warplist.warpfocused = tmp_warpfocused;
	Conf.warplist.icon_mode = tmp_warp_icon_mode;

	Conf.focus.clickraises = tmp_clickalways;
	FocusFix();
     }
   autosave();
}

static void
SettingsFocus(void)
{
   Dialog             *d;
   DItem              *table, *di, *radio, *radio2;

   if ((d = FindItem("CONFIGURE_FOCUS", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_FOCUS");

   tmp_focus = Conf.focus.mode;
   tmp_new_focus = Conf.focus.all_new_windows_get_focus;
   tmp_popup_focus = Conf.focus.new_transients_get_focus;
   tmp_owner_popup_focus = Conf.focus.new_transients_get_focus_if_group_focused;
   tmp_raise_focus = Conf.focus.raise_on_next;
   tmp_warp_focus = Conf.focus.warp_on_next;
   tmp_warp_always = Conf.focus.warp_always;

   tmp_raise_after_focus = Conf.warplist.raise_on_select;
   tmp_warp_after_focus = Conf.warplist.warp_on_select;
   tmp_display_warp = Conf.warplist.enable;
   tmp_showsticky = Conf.warplist.showsticky;
   tmp_showshaded = Conf.warplist.showshaded;
   tmp_showiconified = Conf.warplist.showiconified;
   tmp_warpfocused = Conf.warplist.warpfocused;
   tmp_warp_icon_mode = Conf.warplist.icon_mode;

   tmp_clickalways = Conf.focus.clickraises;

   d = DialogCreate("CONFIGURE_FOCUS");
   DialogSetTitle(d, _("Focus Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	di = DialogAddItem(table, DITEM_IMAGE);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemImageSetFile(di, "pix/focus.png");

	di = DialogAddItem(table, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Enlightenment Focus\n" "Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetColSpan(di, 2);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   radio = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Focus follows pointer"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 0);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Focus follows pointer sloppily"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Focus follows mouse clicks"));
   DialogItemRadioButtonSetFirst(di, radio);
   DialogItemRadioButtonGroupSetVal(di, 2);
   DialogItemRadioButtonGroupSetValPtr(radio, &tmp_focus);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Clicking in a window always raises it"));
   DialogItemCheckButtonSetState(di, tmp_clickalways);
   DialogItemCheckButtonSetPtr(di, &tmp_clickalways);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("All new windows first get the focus"));
   DialogItemCheckButtonSetState(di, tmp_new_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_new_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Only new dialog windows get the focus"));
   DialogItemCheckButtonSetState(di, tmp_popup_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_popup_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _
		     ("Only new dialogs whose owner is focused get the focus"));
   DialogItemCheckButtonSetState(di, tmp_owner_popup_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_owner_popup_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Raise windows while switching focus"));
   DialogItemCheckButtonSetState(di, tmp_raise_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_raise_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _("Send mouse pointer to window while switching focus"));
   DialogItemCheckButtonSetState(di, tmp_warp_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_warp_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _("Always send mouse pointer to window on focus switch"));
   DialogItemCheckButtonSetState(di, tmp_warp_always);
   DialogItemCheckButtonSetPtr(di, &tmp_warp_always);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Display and use focus list"));
   DialogItemCheckButtonSetState(di, tmp_display_warp);
   DialogItemCheckButtonSetPtr(di, &tmp_display_warp);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Include sticky windows in focus list"));
   DialogItemCheckButtonSetState(di, tmp_showsticky);
   DialogItemCheckButtonSetPtr(di, &tmp_showsticky);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Include shaded windows in focus list"));
   DialogItemCheckButtonSetState(di, tmp_showshaded);
   DialogItemCheckButtonSetPtr(di, &tmp_showshaded);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Include iconified windows in focus list"));
   DialogItemCheckButtonSetState(di, tmp_showiconified);
   DialogItemCheckButtonSetPtr(di, &tmp_showiconified);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Focus windows while switching"));
   DialogItemCheckButtonSetState(di, tmp_warpfocused);
   DialogItemCheckButtonSetPtr(di, &tmp_warpfocused);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Raise windows after focus switch"));
   DialogItemCheckButtonSetState(di, tmp_raise_after_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_raise_after_focus);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Send mouse pointer to window after focus switch"));
   DialogItemCheckButtonSetState(di, tmp_warp_after_focus);
   DialogItemCheckButtonSetPtr(di, &tmp_warp_after_focus);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _
		     ("Focuslist image display policy (if one operation fails, try the next):"));

   radio2 = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("First E Icon, then App Icon"));
   DialogItemRadioButtonSetFirst(di, radio2);
   DialogItemRadioButtonGroupSetVal(di, 3);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("First App Icon, then E Icon"));
   DialogItemRadioButtonSetFirst(di, radio2);
   DialogItemRadioButtonGroupSetVal(di, 4);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("None"));
   DialogItemRadioButtonSetFirst(di, radio2);
   DialogItemRadioButtonGroupSetVal(di, 0);
   DialogItemRadioButtonGroupSetValPtr(radio2, &tmp_warp_icon_mode);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureFocus, 1, DIALOG_BUTTON_OK);
   DialogAddButton(d, _("Apply"), CB_ConfigureFocus, 0, DIALOG_BUTTON_APPLY);
   DialogAddButton(d, _("Close"), CB_ConfigureFocus, 1, DIALOG_BUTTON_CLOSE);
   DialogSetExitFunction(d, CB_ConfigureFocus, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureFocus, 0);
   ShowDialog(d);
}

/*
 * Focus Module
 */

static void
FocusInitTimeout(int val __UNUSED__, void *data __UNUSED__)
{
   FocusInit();
}

static void
FocusSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_START:
	/* Delay focusing a bit to allow things to settle down */
	DoIn("FOCUS_INIT_TIMEOUT", 0.5, FocusInitTimeout, 0, NULL);
	break;

     case ESIGNAL_EXIT:
	FocusExit();
	break;

     case ESIGNAL_IDLE:
	if (focus_inhibit || !focus_pending_why)
	   break;
	FocusSet();
	break;
     }
}

static void
FocusIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 len;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %4000s %n", cmd, prm, &len);
	p += len;
     }

   if (!p || cmd[0] == '?')
     {
	EWin               *ewin;

	ewin = GetFocusEwin();
	if (ewin)
	   IpcPrintf("Focused: %#lx\n", _EwinGetClientXwin(ewin));
	else
	   IpcPrintf("Focused: none\n");
     }
   else if (!strcmp(cmd, "cfg"))
     {
	SettingsFocus();
     }
   else if (!strncmp(cmd, "mode", 2))
     {
	int                 mode = Conf.focus.mode;

	if (!strcmp(prm, "click"))
	  {
	     mode = MODE_FOCUS_CLICK;
	     Mode.grabs.pointer_grab_active = 1;
	  }
	else if (!strcmp(prm, "clicknograb"))
	  {
	     mode = MODE_FOCUS_CLICK;
	     Mode.grabs.pointer_grab_active = 0;
	  }
	else if (!strcmp(prm, "pointer"))
	  {
	     mode = MODE_FOCUS_POINTER;
	  }
	else if (!strcmp(prm, "sloppy"))
	  {
	     mode = MODE_FOCUS_SLOPPY;
	  }
	else if (!strcmp(prm, "?"))
	  {
	     if (Conf.focus.mode == MODE_FOCUS_CLICK)
	       {
		  if (Mode.grabs.pointer_grab_active)
		     p = "click";
		  else
		     p = "clicknograb";
	       }
	     else if (Conf.focus.mode == MODE_FOCUS_SLOPPY)
		p = "sloppy";
	     else if (Conf.focus.mode == MODE_FOCUS_POINTER)
		p = "pointer";
	     else
		p = "unknown";
	     IpcPrintf("Focus Mode: %s\n", p);
	  }
	else
	  {
	     IpcPrintf("Error: unknown focus type\n");
	  }
	if (Conf.focus.mode != mode)
	  {
	     Conf.focus.mode = mode;
	     FocusFix();
	     autosave();
	  }
     }
   else if (!strncmp(cmd, "next", 2))
     {
	if (Conf.warplist.enable)
	   WarpFocus(1);
	else
	   FocusGetNextEwin();
     }
   else if (!strncmp(cmd, "prev", 2))
     {
	if (Conf.warplist.enable)
	   WarpFocus(-1);
	else
	   FocusGetPrevEwin();
     }
}

IpcItem             FocusIpcArray[] = {
   {
    FocusIpc,
    "focus", "sf",
    "Focus functions",
    "  focus ?                     Show focus info\n"
    "  focus mode                  Set focus mode. Modes:\n"
    "    click:       The traditional click-to-focus mode.\n"
    "    clicknograb: A similar focus mode, but without the grabbing of the click\n"
    "      (you cannot click anywhere in a window to focus it)\n"
    "    pointer:     The focus will follow the mouse pointer\n"
    "    sloppy:      The focus follows the mouse, but when over the desktop background\n"
    "                 the last window does not lose the focus\n"}
   ,
};
#define N_IPC_FUNCS (sizeof(FocusIpcArray)/sizeof(IpcItem))

static const CfgItem FocusCfgItems[] = {
   CFG_ITEM_INT(Conf.focus, mode, MODE_FOCUS_SLOPPY),
   CFG_ITEM_BOOL(Conf.focus, clickraises, 0),
   CFG_ITEM_BOOL(Conf.focus, transientsfollowleader, 1),
   CFG_ITEM_BOOL(Conf.focus, switchfortransientmap, 1),
   CFG_ITEM_BOOL(Conf.focus, all_new_windows_get_focus, 0),
   CFG_ITEM_BOOL(Conf.focus, new_transients_get_focus, 0),
   CFG_ITEM_BOOL(Conf.focus, new_transients_get_focus_if_group_focused, 1),
   CFG_ITEM_BOOL(Conf.focus, raise_on_next, 1),
   CFG_ITEM_BOOL(Conf.focus, warp_on_next, 0),
   CFG_ITEM_BOOL(Conf.focus, warp_always, 0),

   CFG_ITEM_BOOL(Conf, autoraise.enable, 0),
   CFG_ITEM_INT(Conf, autoraise.delay, 500),
};
#define N_CFG_ITEMS (sizeof(FocusCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
EModule             ModFocus = {
   "focus", NULL,
   FocusSighan,
   {N_IPC_FUNCS, FocusIpcArray},
   {N_CFG_ITEMS, FocusCfgItems}
};
