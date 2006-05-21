/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2006 Kim Woelders
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
#ifndef _HINTS_H_
#define _HINTS_H_

/* Misc atoms */
extern Atom         E_XROOTPMAP_ID;
extern Atom         E_XROOTCOLOR_PIXEL;

/* ewmh.c */
void                EWMH_Init(Window win_wm_check);
void                EWMH_SetDesktopCount(void);
void                EWMH_SetDesktopRoots(void);
void                EWMH_SetDesktopNames(void);
void                EWMH_SetDesktopSize(void);
void                EWMH_SetCurrentDesktop(void);
void                EWMH_SetDesktopViewport(void);
void                EWMH_SetWorkArea(void);
void                EWMH_SetClientList(void);
void                EWMH_SetClientStacking(void);
void                EWMH_SetActiveWindow(Window win);
void                EWMH_SetShowingDesktop(int on);
void                EWMH_SetWindowName(Window win, const char *name);
void                EWMH_SetWindowDesktop(const EWin * ewin);
void                EWMH_SetWindowState(const EWin * ewin);
void                EWMH_SetWindowBorder(const EWin * ewin);
void                EWMH_SetWindowOpacity(const EWin * ewin);
void                EWMH_SetWindowActions(const EWin * ewin);
void                EWMH_GetWindowHints(EWin * ewin);
void                EWMH_DelWindowHints(const EWin * ewin);
void                EWMH_ProcessClientMessage(XClientMessageEvent * event);
void                EWMH_ProcessPropertyChange(EWin * ewin, Atom atom_change);

#if ENABLE_GNOME
/* gnome.c */
void                GNOME_SetCurrentDesk(void);
void                GNOME_SetDeskCount(void);
void                GNOME_SetDeskNames(void);
void                GNOME_SetClientList(void);
void                GNOME_SetEwinDesk(const EWin * ewin);
void                GNOME_SetEwinArea(const EWin * ewin);
void                GNOME_SetHint(const EWin * ewin);
void                GNOME_SetCurrentArea(void);
void                GNOME_SetAreaCount(void);
void                GNOME_GetHints(EWin * ewin, Atom atom_change);
void                GNOME_DelHints(const EWin * ewin);
void                GNOME_SetHints(Window win_wm_check);
void                GNOME_ProcessClientMessage(XClientMessageEvent * event);
#endif

/* hints.c */
void                HintsInit(void);
void                HintsSetDesktopConfig(void);
void                HintsSetViewportConfig(void);
void                HintsSetCurrentDesktop(void);
void                HintsSetDesktopViewport(void);
void                HintsSetClientList(void);
void                HintsSetClientStacking(void);
void                HintsSetActiveWindow(Window win);
void                HintsSetWindowName(Win win, const char *name);
void                HintsSetWindowClass(Win win, const char *name,
					const char *clss);
void                HintsSetWindowDesktop(const EWin * ewin);
void                HintsSetWindowArea(const EWin * ewin);
void                HintsSetWindowState(const EWin * ewin);
void                HintsSetWindowOpacity(const EWin * ewin);
void                HintsSetWindowBorder(const EWin * ewin);
void                HintsGetWindowHints(EWin * ewin);
void                HintsDelWindowHints(const EWin * ewin);
void                HintsProcessPropertyChange(EWin * ewin, Atom atom_change);
void                HintsProcessClientMessage(XClientMessageEvent * event);

Pixmap              HintsGetRootPixmap(Win win);
void                HintsSetRootHints(Win win);
void                HintsSetRootInfo(Win win, Pixmap pmap, unsigned int color);

void                EHintsSetInfo(const EWin * ewin);
void                EHintsGetInfo(EWin * ewin);
void                EHintsSetDeskInfo(void);
void                EHintsGetDeskInfo(void);
void                EHintsSetInfoOnAll(void);

/* icccm.c */
void                ICCCM_Init(void);
void                ICCCM_ProcessClientMessage(XClientMessageEvent * event);
void                ICCCM_GetTitle(EWin * ewin, Atom atom_change);
void                ICCCM_Delete(const EWin * ewin);
void                ICCCM_Save(const EWin * ewin);
void                ICCCM_Iconify(const EWin * ewin);
void                ICCCM_DeIconify(const EWin * ewin);
void                ICCCM_SizeMatch(const EWin * ewin, int wi, int hi, int *pwo,
				    int *pho);
void                ICCCM_MatchSize(EWin * ewin);
void                ICCCM_GetIncrementalSize(EWin * ewin, unsigned int w,
					     unsigned int h, unsigned int *wi,
					     unsigned int *hi);
void                ICCCM_SetSizeConstraints(EWin * ewin, unsigned int wmin,
					     unsigned int hmin,
					     unsigned int wmax,
					     unsigned int hmax,
					     unsigned int wbase,
					     unsigned int hbase,
					     unsigned int winc,
					     unsigned int hinc, double amin,
					     double amax);
void                ICCCM_Configure(EWin * ewin);
void                ICCCM_AdoptStart(const EWin * ewin);
void                ICCCM_Adopt(const EWin * ewin);
void                ICCCM_Withdraw(const EWin * ewin);
void                ICCCM_Cmap(EWin * ewin);
void                ICCCM_Focus(const EWin * ewin);
void                ICCCM_GetGeoms(EWin * ewin, Atom atom_change);
void                ICCCM_GetInfo(EWin * ewin, Atom atom_change);
void                ICCCM_GetHints(EWin * ewin, Atom atom_change);
void                ICCCM_ProcessPropertyChange(EWin * ewin, Atom atom_change);

/* mwm.c */
void                MWM_GetHints(EWin * ewin, Atom atom_change);
void                MWM_SetInfo(void);

#endif /* _HINTS_H_ */
