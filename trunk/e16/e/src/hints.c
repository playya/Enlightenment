/*
 * Copyright (C) 2003 Kim Woelders
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
/*
 * Feeble attempt to collect hint stuff in one place
 */
#include "E.h"

/*
 * Functions that set X11-properties from E-internals
 */

void
HintsInit(void)
{
   EDBUG(6, "HintsInit");
#if ENABLE_KDE
   /* ??? */
#endif
#if ENABLE_GNOME
   GNOME_SetHints();
#endif
#if ENABLE_EWMH
   EWMH_Init();
#endif
   EDBUG_RETURN_;
}

void
HintsSetClientList(void)
{
   EDBUG(6, "HintsSetClientList");
#if ENABLE_GNOME
   GNOME_SetClientList();
#endif
#if ENABLE_EWMH
   EWMH_SetClientList();
#endif
   EDBUG_RETURN_;
}

void
HintsSetDesktopConfig(void)
{
   EDBUG(6, "HintsSetDesktopConfig");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_SetNumDesktops();
#endif
#if ENABLE_GNOME
   GNOME_SetDeskCount();
   GNOME_SetDeskNames();
#endif
#if ENABLE_EWMH
   EWMH_SetDesktopCount();
   EWMH_SetDesktopNames();
#endif
   EDBUG_RETURN_;
}

void
HintsSetViewportConfig(void)
{
   EDBUG(6, "HintsSetViewportConfig");
#if ENABLE_GNOME
   GNOME_SetAreaCount();
#endif
#if ENABLE_EWMH
   EWMH_SetDesktopSize();
#endif
   EDBUG_RETURN_;
}

void
HintsSetCurrentDesktop(void)
{
   EDBUG(6, "HintsSetCurrentDesktop");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_SetRootArea();
#endif
#if ENABLE_GNOME
   GNOME_SetCurrentDesk();
#endif
#if ENABLE_EWMH
   EWMH_SetCurrentDesktop();
#endif
   HintsSetDesktopViewport();
   EDBUG_RETURN_;
}

void
HintsSetDesktopViewport(void)
{
   EDBUG(6, "HintsSetDesktopViewport");
#if ENABLE_GNOME
   GNOME_SetCurrentArea();
#endif
#if ENABLE_EWMH
   EWMH_SetDesktopViewport();
#endif
   EDBUG_RETURN_;
}

void
HintsSetActiveWindow(EWin * ewin)
{
   EDBUG(6, "HintsSetActiveWindow");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_UpdateFocusedWindow();
#endif
#if ENABLE_EWMH
   EWMH_SetActiveWindow(ewin);
#endif
   EDBUG_RETURN_;
}

void
HintsSetWindowDesktop(EWin * ewin)
{
   EDBUG(6, "HintsSetWindowDesktop");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_UpdateClient(ewin);
#endif
#if ENABLE_GNOME
   GNOME_SetEwinDesk(ewin);
#endif
#if ENABLE_EWMH
   if (!ewin->menu)
      EWMH_SetWindowDesktop(ewin);
#endif
   EDBUG_RETURN_;
}

void
HintsSetWindowArea(EWin * ewin)
{
   EDBUG(6, "HintsSetWindowArea");
#if ENABLE_GNOME
   GNOME_SetEwinArea(ewin);
#endif
   EDBUG_RETURN_;
}

void
HintsSetWindowState(EWin * ewin)
{
   EDBUG(6, "HintsSetWindowState");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_UpdateClient(ewin);
#endif
#if ENABLE_GNOME
   GNOME_SetHint(ewin);
#endif
#if ENABLE_EWMH
   if (!ewin->menu)
      EWMH_SetWindowState(ewin);
#endif
   EDBUG_RETURN_;
}

void
HintsSetWindowHints(EWin * ewin)
{
   int                 kde_support = 0;

   EDBUG(6, "HintsSetWindowHints");
#if ENABLE_KDE
   kde_support = mode.kde_support;
   if (mode.kde_support)
      KDE_UpdateClient(ewin);
   mode.kde_support = 0;
#endif
   HintsSetWindowDesktop(ewin);
   HintsSetWindowState(ewin);
#if ENABLE_KDE
   mode.kde_support = kde_support;
#endif
   EDBUG_RETURN_;
}

/*
 * Functions that set E-internals from X11-properties
 */

void
HintsGetWindowHints(EWin * ewin)
{
   EDBUG(6, "HintsGetWindowHints");
#if ENABLE_GNOME
   GNOME_GetHints(ewin, 0);
#endif
#if ENABLE_EWMH
   EWMH_GetWindowHints(ewin);
#endif
   EDBUG_RETURN_;
}

/*
 * Functions that delete X11-properties
 */

void
HintsDelWindowHints(EWin * ewin)
{
   EDBUG(6, "HintsDelWindowHints");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_RemoveWindow(ewin);
#endif
#if ENABLE_GNOME
   GNOME_DelHints(ewin);
#endif
#if ENABLE_EWMH
   EWMH_DelWindowHints(ewin);
#endif
   EDBUG_RETURN_;
}

/*
 * Functions processing received X11 messages
 */

void
HintsProcessPropertyChange(EWin * ewin, Atom atom_change)
{
   EDBUG(6, "HintsHandlePropertyChange");
#if ENABLE_GNOME
   GNOME_GetHints(ewin, atom_change);
#endif
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_UpdateFocusedWindow();
#endif
#if ENABLE_EWMH
   EWMH_ProcessPropertyChange(ewin, atom_change);
#endif
   EDBUG_RETURN_;
}

void
HintsProcessClientMessage(XClientMessageEvent * event)
{
   EDBUG(6, "HintsHandleClientMessage");
#if ENABLE_KDE
   if (mode.kde_support)
      KDE_ProcessClientMessage(event);
#endif
#if ENABLE_GNOME
   GNOME_ProcessClientMessage(event);
#endif
#if ENABLE_EWMH
   EWMH_ProcessClientMessage(event);
#endif
   EDBUG_RETURN_;
}
