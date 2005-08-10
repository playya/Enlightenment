/*
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
#ifndef _EOBJ_H_
#define _EOBJ_H_

typedef struct _eobj EObj;

struct _eobj
{
   Window              win;	/* The top level window */
   short               type;	/* Ewin, button, other, ... */
   short               ilayer;	/* Internal stacking layer */
   short               layer;	/* Stacking layer */
   short               desk;	/* Belongs on desk */
   int                 x, y;
   int                 w, h;
   signed char         stacked;
   char                sticky;
   char                floating;
   char                shown;
   char                gone;
#if USE_COMPOSITE
   char                shadow;	/* Enable shadows */
   char                noredir;	/* Do not redirect */
   unsigned int        opacity;
   void               *cmhook;
#endif
   char               *name;
};

#define EOBJ_TYPE_EWIN      0
#define EOBJ_TYPE_BUTTON    1
#define EOBJ_TYPE_DESK      2
#define EOBJ_TYPE_MISC      3
#define EOBJ_TYPE_EVENT     4
#define EOBJ_TYPE_EXT       5

#define EoObj(eo)               (&((eo)->o))
#define EoGetWin(eo)            ((eo)->o.win)
#define EoGetName(eo)           ((eo)->o.name)
#define EoGetType(eo)           ((eo)->o.type)
#define EoGetX(eo)              ((eo)->o.x)
#define EoGetY(eo)              ((eo)->o.y)
#define EoGetW(eo)              ((eo)->o.w)
#define EoGetH(eo)              ((eo)->o.h)
#define EoIsSticky(eo)          ((eo)->o.sticky)
#define EoIsFloating(eo)        ((eo)->o.floating)
#define EoIsShown(eo)           ((eo)->o.shown)
#define EoGetDesk(eo)           ((eo)->o.desk)
#define EoGetLayer(eo)          ((eo)->o.layer)
#define EoGetPixmap(eo)         EobjGetPixmap(EoObj(eo))

#define EoSetName(eo, _x)       (eo)->o.name = (_x)
#define EoSetSticky(eo, _x)     (eo)->o.sticky = ((_x)?1:0)
#define EoSetFloating(eo, _f)   EobjSetFloating(EoObj(eo), (_f))
#define EoSetDesk(eo, _d)       EobjSetDesk(EoObj(eo), (_d))
#define EoSetLayer(eo, _l)      EobjSetLayer(EoObj(eo), (_l))
#if USE_COMPOSITE
#define EoSetOpacity(eo, _o)    (eo)->o.opacity = (_o)
#define EoGetOpacity(eo)        ((eo)->o.opacity)
#define EoChangeOpacity(eo, _o) EobjChangeOpacity(EoObj(eo), _o)
#define EoSetShadow(eo, _x)     (eo)->o.shadow = (_x)
#define EoGetShadow(eo)         ((eo)->o.shadow)
#define EoSetNoRedirect(eo, _x) (eo)->o.noredir = (_x)
#define EoGetNoRedirect(eo)     ((eo)->o.noredir)
#else
#define EoSetOpacity(eo, _o)
#define EoChangeOpacity(eo, _o)
#define EoSetShadow(eo, _x)
#endif

#define EoMap(eo, raise)                EobjMap(EoObj(eo), raise)
#define EoUnmap(eo)                     EobjUnmap(EoObj(eo))
#define EoMove(eo, x, y)                EobjMove(EoObj(eo), x, y)
#define EoResize(eo, w, h)              EobjResize(EoObj(eo), w, h)
#define EoMoveResize(eo, x, y, w, h)    EobjMoveResize(EoObj(eo), x, y, w, h)
#define EoReparent(eo, d, x, y)         EobjReparent(EoObj(eo), d, x, y)
#define EoRaise(eo)                     EobjRaise(EoObj(eo))
#define EoLower(eo)                     EobjLower(EoObj(eo))
#define EoChangeShape(eo)               EobjChangeShape(EoObj(eo))

/* eobj.c */
void                EobjInit(EObj * eo, int type, Window win, int x, int y,
			     int w, int h, int su, const char *name);
void                EobjFini(EObj * eo);
void                EobjDestroy(EObj * eo);
EObj               *EobjWindowCreate(int type, int x, int y, int w, int h,
				     int su, const char *name);
void                EobjWindowDestroy(EObj * eo);

EObj               *EobjRegister(Window win, int type);
void                EobjUnregister(EObj * eo);
void                EobjMap(EObj * eo, int raise);
void                EobjUnmap(EObj * eo);
void                EobjMove(EObj * eo, int x, int y);
void                EobjResize(EObj * eo, int w, int h);
void                EobjMoveResize(EObj * eo, int x, int y, int w, int h);
void                EobjReparent(EObj * eo, EObj * dst, int x, int y);
int                 EobjRaise(EObj * eo);
int                 EobjLower(EObj * eo);
void                EobjChangeShape(EObj * eo);
void                EobjsRepaint(void);
Pixmap              EobjGetPixmap(const EObj * eo);

#if USE_COMPOSITE
void                EobjChangeOpacity(EObj * eo, unsigned int opacity);
#else
#define             EobjChangeOpacity(eo, opacity)
#endif
void                EobjSetDesk(EObj * eo, int desk);
void                EobjSetLayer(EObj * eo, int layer);
void                EobjSetFloating(EObj * eo, int floating);
int                 EobjIsShaped(const EObj * eo);
void                EobjSlideTo(EObj * eo, int fx, int fy, int tx, int ty,
				int speed);
void                EobjsSlideBy(EObj ** peo, int num, int dx, int dy,
				 int speed);
void                EobjSlideSizeTo(EObj * eo, int fx, int fy, int tx, int ty,
				    int fw, int fh, int tw, int th, int speed);

/* stacking.c */
void                EobjListStackAdd(EObj * eo, int ontop);
void                EobjListStackDel(EObj * eo);
int                 EobjListStackRaise(EObj * eo);
int                 EobjListStackLower(EObj * eo);
EObj               *EobjListStackFind(Window win);
EObj               *const *EobjListStackGet(int *num);
EObj               *const *EobjListStackGetForDesk(int *num, int desk);
void                EobjListFocusAdd(EObj * eo, int ontop);
void                EobjListFocusDel(EObj * eo);
int                 EobjListFocusRaise(EObj * eo);
int                 EobjListFocusLower(EObj * eo);
void                EobjListOrderAdd(EObj * eo);
void                EobjListOrderDel(EObj * eo);

#endif /* _EOBJ_H_ */
