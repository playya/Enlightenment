/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
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
#include "ecompmgr.h"		/* FIXME - Remove */

#define BUTTON_EVENT_MASK \
  (ExposureMask | KeyPressMask | KeyReleaseMask | \
   ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask)
/* PointerMotionMask */

typedef struct _bgeometry
{
   Constraints         width, height;
   int                 xorigin, yorigin;
   int                 xabs, xrel;
   int                 yabs, yrel;
   int                 xsizerel, xsizeabs;
   int                 ysizerel, ysizeabs;
   char                size_from_image;
}
BGeometry;

struct _button
{
   EObj                o;
   char               *name;
   BGeometry           geom;
   ImageClass         *iclass;
   ActionClass        *aclass;
   TextClass          *tclass;
   char               *label;
   int                 flags;
   char                visible;
   char                internal;
   char                default_show;
   char                used;

   int                 cx, cy, cw, ch;
   int                 state;
   char                expose;
   Window              inside_win;
   Window              event_win;
   char                destroy_inside_win;
   char                left;
   unsigned int        ref_count;
};

static struct
{
   Button             *button;
} Mode_button;

static void         ButtonHandleEvents(XEvent * ev, void *btn);

Button             *
ButtonCreate(const char *name, int id, ImageClass * iclass,
	     ActionClass * aclass, TextClass * tclass, char *label, char ontop,
	     int flags, int minw, int maxw, int minh, int maxh, int xo, int yo,
	     int xa, int xr, int ya, int yr, int xsr, int xsa, int ysr, int ysa,
	     char simg, int desk, char sticky)
{
   Button             *b;

   EDBUG(5, "ButtonCreate");

   if (desk < 0 || desk >= DesksGetNumber())
      return NULL;
   if (sticky && ontop == 1)
      desk = 0;

   b = Ecalloc(1, sizeof(Button));

   b->name = Estrdup(name);
   b->label = Estrdup(label);

   b->iclass = iclass;
   if (!b->iclass)
      b->iclass = ImageclassFind(NULL, 0);
   if (b->iclass)
      b->iclass->ref_count++;

   b->aclass = aclass;
   if (b->aclass)
      ActionclassIncRefcount(b->aclass);

   b->tclass = tclass;
   if (!b->tclass && b->label)
      b->tclass = TextclassFind(NULL, 0);
   if (b->tclass)
      b->tclass->ref_count++;

   b->label = label;

   b->flags = flags;
   b->visible = 0;
   b->geom.width.min = minw;
   b->geom.width.max = maxw;
   b->geom.height.min = minh;
   b->geom.height.max = maxh;
   b->geom.xorigin = xo;
   b->geom.yorigin = yo;
   b->geom.xabs = xa;
   b->geom.xrel = xr;
   b->geom.yabs = ya;
   b->geom.yrel = yr;
   b->geom.xsizeabs = xsa;
   b->geom.xsizerel = xsr;
   b->geom.ysizeabs = ysa;
   b->geom.ysizerel = ysr;
   b->geom.size_from_image = simg;
   b->inside_win = 0;
   b->event_win = 0;
   b->destroy_inside_win = 0;
   b->internal = 0;
   b->default_show = 1;
   b->used = 0;
   b->left = 0;
   b->cx = -10;
   b->cy = -10;
   b->cw = -10;
   b->ch = -10;
   b->state = 0;
   b->expose = 0;
   b->ref_count = 0;

   EoSetWin(b, ECreateWindow(DeskGetWin(desk), -100, -100, 50, 50, 0));
   XSelectInput(disp, EoGetWin(b), BUTTON_EVENT_MASK);
   EventCallbackRegister(EoGetWin(b), 0, ButtonHandleEvents, b);

   EobjInit(&b->o, EOBJ_TYPE_BUTTON, -1, -1, -1, -1);
   EoSetSticky(b, sticky);
   EoSetDesk(b, desk);
   EoSetLayer(b, ontop);
   EoDisableShadows(b);

   EobjListStackAdd(&b->o, 0);
   EwinListStackRaise(b);

   AddItem(b, b->name, id, LIST_TYPE_BUTTON);

   EDBUG_RETURN(b);
}

void
ButtonDestroy(Button * b)
{
   EDBUG(5, "ButtonDestroy");

   if (!b)
      EDBUG_RETURN_;

   if (b->ref_count > 0)
     {
	DialogOK(_("Button Error!"), _("%u references remain\n"), b->ref_count);
	EDBUG_RETURN_;
     }

   while (RemoveItemByPtr(b, LIST_TYPE_BUTTON));

   EobjListStackDel(&b->o);

   if (b->name)
      Efree(b->name);

   if (EoGetWin(b))
      EDestroyWindow(disp, EoGetWin(b));

   if (b->iclass)
      b->iclass->ref_count--;

   if (b->aclass)
      ActionclassDecRefcount(b->aclass);

   if (b->tclass)
      b->tclass->ref_count--;

   if (b->label)
      Efree(b->label);

   Efree(b);

   EDBUG_RETURN_;
}

static void
ButtonCalc(Button * b)
{
   int                 w, h, x, y, xo, yo;
   Imlib_Image        *im;

   EDBUG(4, "ButtonCalc");
   x = 0;
   y = 0;
   w = 32;
   h = 32;
   if (b->geom.size_from_image)
     {
	if ((b->iclass) && (b->iclass->norm.normal->im_file))
	  {
	     im = ELoadImage(b->iclass->norm.normal->im_file);
	     if (im)
	       {
		  imlib_context_set_image(im);
		  w = imlib_image_get_width();
		  h = imlib_image_get_height();
		  imlib_free_image();
	       }
	     else
	       {
		  w = 32;
		  h = 32;
	       }
	  }
	else
	  {
	     if (!b->iclass)
		b->iclass = ImageclassFind(NULL, 0);
	     w = 32;
	     h = 32;
	  }
     }
   else
     {
	w = ((b->geom.xsizerel * VRoot.w) >> 10) + b->geom.xsizeabs;
	h = ((b->geom.ysizerel * VRoot.h) >> 10) + b->geom.ysizeabs;
     }
   if (w > b->geom.width.max)
      w = b->geom.width.max;
   else if (w < b->geom.width.min)
      w = b->geom.width.min;
   if (h > b->geom.height.max)
      h = b->geom.height.max;
   else if (h < b->geom.height.min)
      h = b->geom.height.min;
   xo = (w * b->geom.xorigin) >> 10;
   yo = (h * b->geom.yorigin) >> 10;
   x = ((b->geom.xrel * VRoot.w) >> 10) + b->geom.xabs - xo;
   y = ((b->geom.yrel * VRoot.h) >> 10) + b->geom.yabs - yo;
   EoSetX(b, x);
   EoSetY(b, y);
   EoSetW(b, w);
   EoSetH(b, h);

   EDBUG_RETURN_;
}

void
ButtonShow(Button * b)
{
   char                move, resize;

   EDBUG(4, "ButtonShow");

   ButtonCalc(b);

   move = 0;
   resize = 0;
   if ((EoGetX(b) != b->cx) || (EoGetY(b) != b->cy))
      move = 1;
   if ((EoGetW(b) != b->cw) || (EoGetH(b) != b->ch))
      resize = 1;

   if ((move) && (resize))
      EMoveResizeWindow(disp, EoGetWin(b), EoGetX(b), EoGetY(b), EoGetW(b),
			EoGetH(b));
   else if (move)
      EMoveWindow(disp, EoGetWin(b), EoGetX(b), EoGetY(b));
   else if (resize)
      EResizeWindow(disp, EoGetWin(b), EoGetW(b), EoGetH(b));
   if (EoIsSticky(b))
      XRaiseWindow(disp, EoGetWin(b));

   ButtonDraw(b);
   b->visible = 1;
   EMapWindow(disp, EoGetWin(b));
   b->cx = EoGetX(b);
   b->cy = EoGetY(b);
   b->cw = EoGetW(b);
   b->ch = EoGetH(b);

   EDBUG_RETURN_;
}

void
ButtonMoveToDesktop(Button * b, int desk)
{
   int                 pdesk;

   EDBUG(3, "ButtonMoveToDesktop");

   if (desk < 0 || desk >= DesksGetNumber())
      EDBUG_RETURN_;

   if (EoIsSticky(b) && EoGetLayer(b) == 1)
      desk = 0;
   pdesk = EoGetDesk(b);
   EoSetDesk(b, desk);

   if (desk != pdesk)
      EReparentWindow(disp, EoGetWin(b), DeskGetWin(desk), EoGetX(b),
		      EoGetY(b));

   EDBUG_RETURN_;
}

void
ButtonHide(Button * b)
{
   EDBUG(3, "ButtonHide");

   EUnmapWindow(disp, EoGetWin(b));
   b->visible = 0;

   EDBUG_RETURN_;
}

void
ButtonToggle(Button * b)
{
   EDBUG(3, "ButtonToggle");

   if (b->used)
      EDBUG_RETURN_;

   if (b->visible)
      ButtonHide(b);
   else
      ButtonShow(b);

   EDBUG_RETURN_;
}

void
ButtonDraw(Button * b)
{
   EDBUG(3, "ButtonDraw");

   ImageclassApply(b->iclass, EoGetWin(b), EoGetW(b), EoGetH(b), 0, 0, b->state,
		   0, ST_BUTTON);

   if (b->label)
      TextclassApply(b->iclass, EoGetWin(b), EoGetW(b), EoGetH(b), 0, 0,
		     b->state, 0, b->tclass, b->label);
   EDBUG_RETURN_;
}

void
ButtonDrawWithState(Button * b, int state)
{
   b->state = state;
   ButtonDraw(b);
}

void
ButtonMoveToCoord(Button * b, int x, int y)
{
   int                 rx, ry, relx, rely, absx, absy;
   char                move, resize;

   EDBUG(3, "ButtonMoveToCoord");
   if (b->flags & FLAG_FIXED)
      EDBUG_RETURN_;

   if ((x + (EoGetW(b) >> 1)) < (VRoot.w / 3))
      relx = 0;
   else if ((x + (EoGetW(b) >> 1)) > ((VRoot.w * 2) / 3))
      relx = 1024;
   else
      relx = 512;
   rx = (relx * VRoot.w) >> 10;
   absx = x - rx;
   if ((y + (EoGetH(b) >> 1)) < (VRoot.h / 3))
      rely = 0;
   else if ((y + (EoGetH(b) >> 1)) > ((VRoot.h * 2) / 3))
      rely = 1024;
   else
      rely = 512;
   ry = (rely * VRoot.h) >> 10;
   absy = y - ry;
   if (!(b->flags & FLAG_FIXED_HORIZ))
     {
	b->geom.xorigin = 0;
	b->geom.xabs = absx;
	b->geom.xrel = relx;
     }
   if (!(b->flags & FLAG_FIXED_VERT))
     {
	b->geom.yorigin = 0;
	b->geom.yabs = absy;
	b->geom.yrel = rely;
     }
   ButtonCalc(b);
   move = 0;
   resize = 0;
   if ((EoGetX(b) != b->cx) || (EoGetY(b) != b->cy))
      move = 1;
   if ((EoGetW(b) != b->cw) || (EoGetH(b) != b->ch))
      resize = 1;
   if ((move) && (resize))
      EMoveResizeWindow(disp, EoGetWin(b), EoGetX(b), EoGetY(b), EoGetW(b),
			EoGetH(b));
   else if (move)
      EMoveWindow(disp, EoGetWin(b), EoGetX(b), EoGetY(b));
   else if (resize)
      EResizeWindow(disp, EoGetWin(b), EoGetW(b), EoGetH(b));
   if (EoIsSticky(b))
      XRaiseWindow(disp, EoGetWin(b));
   b->cx = EoGetX(b);
   b->cy = EoGetY(b);
   b->cw = EoGetW(b);
   b->ch = EoGetH(b);

   EDBUG_RETURN_;
}

void
ButtonMoveRelative(Button * b, int dx, int dy)
{
   ButtonMoveToCoord(b, EoGetX(b) + dx, EoGetY(b) + dy);
}

void
ButtonIncRefcount(Button * b)
{
   b->ref_count++;
}

void
ButtonDecRefcount(Button * b)
{
   b->ref_count--;
}

void
ButtonSetSwallowed(Button * b)
{
   b->internal = 1;
   b->default_show = 0;
   b->flags |= FLAG_FIXED;
   b->used = 1;
   b->ref_count++;
   EobjListStackDel(&b->o);
}

const char         *
ButtonGetName(const Button * b)
{
   return b->name;
}

int
ButtonGetRefcount(const Button * b)
{
   return b->ref_count;
}

int
ButtonGetDesk(const Button * b)
{
   return EoGetDesk(b);
}

void
ButtonGetGeometry(const Button * b, int *x, int *y, unsigned int *w,
		  unsigned int *h)
{
   if (x)
      *x = EoGetX(b);
   if (y)
      *y = EoGetY(b);
   if (w)
      *w = EoGetW(b);
   if (h)
      *h = EoGetH(b);
}

int
ButtonGetInfo(const Button * b, RectBox * r, int desk)
{
   if (!b->visible || b->internal)
      return -1;
   if (!EoIsSticky(b) && EoGetDesk(b) != desk)
      return -1;

   r->data = NULL;
   r->x = EoGetX(b);
   r->y = EoGetY(b);
   r->w = EoGetW(b);
   r->h = EoGetH(b);
   r->p = EoIsSticky(b);

   return 0;
}

ActionClass        *
ButtonGetAClass(const Button * b)
{
   return b->aclass;
}

Window
ButtonGetWindow(const Button * b)
{
   return EoGetWin(b);
}

int
ButtonGetWidth(const Button * b)
{
   return EoGetW(b);
}

int
ButtonGetHeight(const Button * b)
{
   return EoGetH(b);
}

int
ButtonIsFixed(const Button * b)
{
   return b->flags & FLAG_FIXED;
}

int
ButtonIsInternal(const Button * b)
{
   return b->internal;
}

int
ButtonIsAbove(const Button * b, int desk)
{
   return !b->internal && EoGetDesk(b) == desk && EoGetLayer(b) > 0;
}

int
ButtonIsNormal(const Button * b, int desk)
{
   return !b->internal && EoGetDesk(b) == desk && EoGetLayer(b) == 0;
}

int
ButtonIsBelow(const Button * b, int desk)
{
   return !b->internal && EoGetDesk(b) == desk && EoGetLayer(b) < 0;
}

int
ButtonDoShowDefault(const Button * b)
{
   return !b->internal && b->default_show;
}

int
ButtonEmbedWindow(Button * b, Window WindowToEmbed)
{

   unsigned int        w, h;

   EDBUG(4, "ButtonEmbedWindow");
   EReparentWindow(disp, WindowToEmbed, EoGetWin(b), 0, 0);
   b->inside_win = WindowToEmbed;
   GetWinWH(WindowToEmbed, &w, &h);
   EMoveWindow(disp, b->inside_win, (EoGetW(b) - w) >> 1, (EoGetH(b) - h) >> 1);
   b->event_win = ECreateEventWindow(EoGetWin(b), 0, 0, w, h);
   EventCallbackRegister(b->event_win, 0, ButtonHandleEvents, b);

   XSelectInput(disp, b->event_win,
		ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
		LeaveWindowMask | ButtonMotionMask);

   EMoveWindow(disp, b->event_win, (EoGetW(b) - w) >> 1, (EoGetH(b) - h) >> 1);
   EMapRaised(disp, b->event_win);

   EDBUG_RETURN(0);
}

static void
ButtonDragStart(Button * b)
{
   if (ButtonIsFixed(b))
      return;

   GrabPointerSet(EoGetWin(b), ECSR_GRAB, 0);
   Mode.mode = MODE_BUTTONDRAG;
   Mode.button_move_pending = 1;
   Mode.start_x = Mode.x;
   Mode.start_y = Mode.y;
   ButtonGetGeometry(b, &Mode.win_x, &Mode.win_y, NULL, NULL);
}

static void
ButtonDragEnd(Button * b)
{
   int                 d;

   Mode.mode = MODE_NONE;

   if (!Mode.button_move_pending)
     {
	d = DesktopAt(Mode.x, Mode.y);
	ButtonMoveToDesktop(b, d);
	d = EoGetDesk(b);
	ButtonMoveRelative(b, -DeskGetX(d), -DeskGetY(d));
     }
   else
      Mode.button_move_pending = 0;

   autosave();
}

/*
 * Functions operating on all buttons
 */

Button             *
FindButton(Window win)
{
   Button             *b;
   Button            **buttons;
   int                 i, num;

   EDBUG(6, "FindButton");

   buttons = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
   for (i = 0; i < num; i++)
     {
	if ((win == EoGetWin(buttons[i])) || (win == buttons[i]->inside_win)
	    || (win == buttons[i]->event_win))
	  {
	     b = buttons[i];
	     Efree(buttons);
	     EDBUG_RETURN(b);
	  }
     }
   if (buttons)
      Efree(buttons);
   EDBUG_RETURN(NULL);
}

/*
 * Button event handlers
 */

static void
ButtonEventExpose(Button * b, XEvent * ev __UNUSED__)
{
   if (!ECompMgrActive())	/* FIXME - Remove */
      ButtonDraw(b);
}

static void
ButtonEventMouseDown(Button * b, XEvent * ev)
{
   Window              win = ev->xbutton.window;
   ActionClass        *ac;

   Mode_button.button = b;

   GrabPointerSet(win, ECSR_GRAB, 0);

   if (b->inside_win)
     {
	ev->xbutton.window = b->inside_win;
	XSendEvent(disp, b->inside_win, False, ButtonPressMask, ev);
	ev->xbutton.window = win;
     }

   b->state = STATE_CLICKED;
   ButtonDraw(b);

   ac = FindItem("ACTION_BUTTON_DRAG", 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
   if (ac)
      EventAclass(ev, NULL, ac);
   if (b->aclass)
      EventAclass(ev, NULL, b->aclass);
}

static void
ButtonEventMouseUp(Button * b, XEvent * ev)
{
   Window              win = ev->xbutton.window;

   if (b->inside_win && !Mode.action_inhibit)
     {
	ev->xbutton.window = b->inside_win;
	XSendEvent(disp, b->inside_win, False, ButtonReleaseMask, ev);
	ev->xbutton.window = win;
     }

   if ((b->state == STATE_CLICKED) && (!b->left))
      b->state = STATE_HILITED;
   else
      b->state = STATE_NORMAL;
   ButtonDraw(b);

   if (b->aclass && !b->left)
      EventAclass(ev, NULL, b->aclass);

   b->left = 0;

   if (Mode.mode == MODE_BUTTONDRAG)
      ButtonDragEnd(Mode_button.button);
   Mode_button.button = NULL;

   GrabPointerRelease();
}

static void
ButtonEventMotion(Button * b, XEvent * ev __UNUSED__)
{
   int                 dx, dy;

   if (Mode.mode != MODE_BUTTONDRAG)
      return;

   dx = Mode.x - Mode.px;
   dy = Mode.y - Mode.py;

   if (Mode.button_move_pending)
     {
	int                 x, y;

	x = Mode.x - Mode.start_x;
	y = Mode.y - Mode.start_y;
	if (x < 0)
	   x = -x;
	if (y < 0)
	   y = -y;
	if ((x > Conf.button_move_resistance) ||
	    (y > Conf.button_move_resistance))
	   Mode.button_move_pending = 0;
	Mode.action_inhibit = 1;
     }
   if (!Mode.button_move_pending)
     {
	if (b)
	  {
	     ButtonMoveRelative(b, dx, dy);
	     if (Conf.deskmode == MODE_DESKRAY)
	       {
		  MoveDesktop(Mode.deskdrag, DeskGetX(Mode.deskdrag),
			      DeskGetY(Mode.deskdrag) + dy);
	       }
	  }
     }
}

static void
ButtonEventMouseIn(Button * b, XEvent * ev)
{
   if (b->state == STATE_CLICKED)
      b->left = 0;
   else
     {
	b->state = STATE_HILITED;
	ButtonDraw(b);
	if (b->aclass)
	   EventAclass(ev, NULL, b->aclass);
     }
}

static void
ButtonEventMouseOut(Button * b, XEvent * ev)
{
   if (b->state == STATE_CLICKED)
      b->left = 1;
   else
     {
	b->state = STATE_NORMAL;
	ButtonDraw(b);
	if (b->aclass)
	   EventAclass(ev, NULL, b->aclass);
     }
}

static void
ButtonHandleEvents(XEvent * ev, void *prm)
{
   Button             *b = (Button *) prm;

   switch (ev->type)
     {
     case ButtonPress:
	ButtonEventMouseDown(b, ev);
	break;
     case ButtonRelease:
	ButtonEventMouseUp(b, ev);
	break;
     case MotionNotify:
	ButtonEventMotion(b, ev);
	break;
     case EnterNotify:
	ButtonEventMouseIn(b, ev);
	break;
     case LeaveNotify:
	ButtonEventMouseOut(b, ev);
	break;
     case Expose:
	ButtonEventExpose(b, ev);
	break;
     }
}

/*
 * Configuration load/save
 */
#include "conf.h"

int
ButtonsConfigLoad(FILE * ConfigFile)
{
   int                 err = 0;
   char                s[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1;
   char               *name = NULL;
   char               *label = NULL;
   ActionClass        *ac = NULL;
   ImageClass         *ic = NULL;
   TextClass          *tc = NULL;
   Button             *bt = NULL;
   Button             *pbt = NULL;
   char                ontop = 0;
   int                 flags = 0, minw = 1, maxw = 99999, minh = 1;
   int                 maxh = 99999, xo = 0, yo = 0, xa = 0;
   int                 xr = 0, ya = 0, yr = 0;
   int                 xsr = 0, xsa = 0, ysr = 0, ysa = 0;
   char                simg = 0;
   int                 desk = 0;
   char                sticky = 0;
   char                show = 1;
   char                internal = 0;
   int                 fields;

   while (GetLine(s, sizeof(s), ConfigFile))
     {
	s2[0] = 0;
	i1 = CONFIG_INVALID;
	fields = sscanf(s, "%i %4000s", &i1, s2);

	if (fields < 1)
	  {
	     i1 = CONFIG_INVALID;
	  }
	else if (i1 == CONFIG_CLOSE)
	  {
	     if (fields != 1)
	       {
		  RecoverUserConfig();
		  Alert(_("CONFIG: ignoring extra data in \"%s\"\n"), s);
	       }
	  }
	else if (i1 != CONFIG_INVALID)
	  {
	     if (fields != 2)
	       {
		  RecoverUserConfig();
		  Alert(_("CONFIG: missing required data in \"%s\"\n"), s);
		  i1 = CONFIG_INVALID;
	       }
	  }
	switch (i1)
	  {
	  case CONFIG_CLOSE:
	     if (!pbt)
	       {
		  bt = ButtonCreate(name, 0, ic, ac, tc, label, ontop, flags,
				    minw, maxw, minh, maxh, xo, yo, xa, xr,
				    ya, yr, xsr, xsa, ysr, ysa, simg, desk,
				    sticky);
		  bt->default_show = show;
		  bt->internal = internal;
	       }
	     goto done;
	  case CONFIG_TEXT:
	     tc = TextclassFind(s2, 1);
	     if (pbt)
		pbt->tclass = tc;
	     break;
	  case BUTTON_LABEL:
	     label = Estrdup(atword(s, 2));
	     if (pbt)
		pbt->label = label;
	     break;
	  case BORDERPART_ONTOP:
	     ontop = atoi(s2);
	     if (pbt)
		EoSetLayer(pbt, ontop);
	     break;
	  case CONFIG_CLASSNAME:
	  case BUTTON_NAME:
	     if (name)
		Efree(name);
	     name = Estrdup(s2);
	     pbt = FindItem(name, 0, LIST_FINDBY_NAME, LIST_TYPE_BUTTON);
	     break;
	  case CONFIG_ACTIONCLASS:
	  case BUTTON_ACLASS:
	     ac = FindItem(s2, 0, LIST_FINDBY_NAME, LIST_TYPE_ACLASS);
	     if (pbt)
		pbt->aclass = ac;
	     break;
	  case CONFIG_IMAGECLASS:
	  case BUTTON_ICLASS:
	     ic = ImageclassFind(s2, 1);
	     if (pbt)
		pbt->iclass = ic;
	     break;
	  case BORDERPART_WMIN:
	     minw = atoi(s2);
	     if (pbt)
		pbt->geom.width.min = minw;
	     break;
	  case BORDERPART_WMAX:
	     maxw = atoi(s2);
	     if (pbt)
		pbt->geom.width.max = maxw;
	     break;
	  case BORDERPART_HMIN:
	     minh = atoi(s2);
	     if (pbt)
		pbt->geom.height.min = minh;
	     break;
	  case BORDERPART_FLAGS:
	     flags = atoi(s2);
	     if (pbt)
		pbt->flags = flags;
	     break;
	  case BORDERPART_HMAX:
	     maxh = atoi(s2);
	     if (pbt)
		pbt->geom.height.max = maxh;
	     break;
	  case BUTTON_XO:
	     xo = atoi(s2);
	     if (pbt)
		pbt->geom.xorigin = xo;
	     break;
	  case BUTTON_YO:
	     yo = atoi(s2);
	     if (pbt)
		pbt->geom.yorigin = yo;
	     break;
	  case BUTTON_XA:
	     xa = atoi(s2);
	     if (pbt)
		pbt->geom.xabs = xa;
	     break;
	  case BUTTON_XR:
	     xr = atoi(s2);
	     if (pbt)
		pbt->geom.xrel = xr;
	     break;
	  case BUTTON_YA:
	     ya = atoi(s2);
	     if (pbt)
		pbt->geom.yabs = ya;
	     break;
	  case BUTTON_YR:
	     yr = atoi(s2);
	     if (pbt)
		pbt->geom.yrel = yr;
	     break;
	  case BUTTON_XSR:
	     xsr = atoi(s2);
	     if (pbt)
		pbt->geom.xsizerel = xsr;
	     break;
	  case BUTTON_XSA:
	     xsa = atoi(s2);
	     if (pbt)
		pbt->geom.xsizeabs = xsa;
	     break;
	  case BUTTON_YSR:
	     ysr = atoi(s2);
	     if (pbt)
		pbt->geom.ysizerel = ysr;
	     break;
	  case BUTTON_YSA:
	     ysa = atoi(s2);
	     if (pbt)
		pbt->geom.ysizeabs = ysa;
	     break;
	  case BUTTON_SIMG:
	     simg = atoi(s2);
	     if (pbt)
		pbt->geom.size_from_image = simg;
	     break;
	  case BUTTON_DESK:
	     desk = atoi(s2);
	     if (pbt)
		ButtonMoveToDesktop(pbt, desk);
	     break;
	  case BUTTON_STICKY:
	     sticky = atoi(s2);
	     if (pbt)
		EoSetSticky(pbt, sticky);
	     break;
	  case BUTTON_INTERNAL:
	     internal = atoi(s2);
	     if (pbt)
		pbt->internal = internal;
	     break;
	  case BUTTON_SHOW:
	     show = atoi(s2);
	     if (pbt)
		pbt->default_show = show;
	     break;
	  default:
	     break;
	  }
     }
   err = -1;

 done:
   if (name)
      Efree(name);

   return err;
}

int
ButtonsConfigSave(FILE * fs)
{
#if 0
   int                 i, num;
   Button            **blst;
   int                 flags;

   blst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
   if (!blst)
      return 0;

   for (i = 0; i < num; i++)
     {
	if (blst[i]->internal)
	   continue;

	fprintf(fs, "4 999\n");
	fprintf(fs, "100 %s\n", blst[i]->name);
	if (blst[i]->iclass)
	   fprintf(fs, "12 %s\n", blst[i]->iclass->name);
	if (blst[i]->aclass)
	   fprintf(fs, "11 %s\n", ActionclassGetName(blst[i]->aclass));
	if (EoGetLayer(blst[i]) >= 0)
	   fprintf(fs, "453 %i\n", EoGetLayer(blst[i]));
	fprintf(fs, "456 %i\n", blst[i]->geom.width.min);
	fprintf(fs, "457 %i\n", blst[i]->geom.width.max);
	fprintf(fs, "468 %i\n", blst[i]->geom.height.min);
	fprintf(fs, "469 %i\n", blst[i]->geom.height.max);
	fprintf(fs, "528 %i\n", blst[i]->geom.xorigin);
	fprintf(fs, "529 %i\n", blst[i]->geom.yorigin);
	fprintf(fs, "530 %i\n", blst[i]->geom.xabs);
	fprintf(fs, "531 %i\n", blst[i]->geom.xrel);
	fprintf(fs, "532 %i\n", blst[i]->geom.yabs);
	fprintf(fs, "533 %i\n", blst[i]->geom.yrel);
	fprintf(fs, "534 %i\n", blst[i]->geom.xsizerel);
	fprintf(fs, "535 %i\n", blst[i]->geom.xsizeabs);
	fprintf(fs, "536 %i\n", blst[i]->geom.ysizerel);
	fprintf(fs, "537 %i\n", blst[i]->geom.ysizeabs);
	fprintf(fs, "538 %i\n", blst[i]->geom.size_from_image);
	fprintf(fs, "539 %i\n", EoGetDesk(blst[i]));
	fprintf(fs, "540 %i\n", EoIsSticky(blst[i]));
	fprintf(fs, "542 %i\n", blst[i]->visible);

	if (blst[i]->flags)
	  {
	     flags = 0;
	     if (((blst[i]->flags & FLAG_FIXED_HORIZ)
		  && (blst[i]->flags & FLAG_FIXED_VERT))
		 || (blst[i]->flags & FLAG_FIXED))
		flags = 2;
	     else if (blst[i]->flags & FLAG_FIXED_HORIZ)
		flags = 3;
	     else if (blst[i]->flags & FLAG_FIXED_VERT)
		flags = 4;
	     else if (blst[i]->flags & FLAG_TITLE)
		flags = 0;
	     else if (blst[i]->flags & FLAG_MINIICON)
		flags = 1;
	     fprintf(fs, "454 %i\n", flags);
	  }
	fprintf(fs, "1000\n");
     }
   Efree(blst);
#else
   fs = NULL;
#endif

   return 0;
}

/*
 * Buttons Module
 */

static void
ButtonsSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     default:
	break;
     }
}

static void
doHideShowButton(const char *params)
{
   Button            **lst, *b;
   char                s[1024];
   const char         *ss;
   int                 num, i;

   if (params)
     {
	sscanf(params, "%1000s", s);
	if (!strcmp(s, "button"))
	  {
	     sscanf(params, "%*s %1000s", s);
	     b = (Button *) FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_BUTTON);
	     if (b)
		ButtonToggle(b);
	  }
	else if (!strcmp(s, "buttons"))
	  {
	     ss = atword(params, 2);
	     if (ss)
	       {
		  lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
		  if (lst)
		    {
		       for (i = 0; i < num; i++)
			 {
			    if (matchregexp(ss, ButtonGetName(lst[i])))
			      {
				 if (strcmp(ButtonGetName(lst[i]),
					    "_DESKTOP_DESKRAY_DRAG_CONTROL"))
				    ButtonToggle(lst[i]);
			      }
			 }
		    }
	       }
	  }
	else if (!strcmp(s, "all_buttons_except"))
	  {
	     ss = atword(params, 2);
	     if (ss)
	       {
		  lst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
		  if (lst)
		    {
		       for (i = 0; i < num; i++)
			 {
			    if (!matchregexp(ss, ButtonGetName(lst[i])))
			      {
				 if (strcmp(ButtonGetName(lst[i]),
					    "_DESKTOP_DESKRAY_DRAG_CONTROL"))
				    ButtonToggle(lst[i]);
			      }
			 }
		    }
	       }
	  }
	else if (!strcmp(s, "all"))
	  {
	     lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	     if (lst)
	       {
		  for (i = 0; i < num; i++)
		    {
		       if (strcmp
			   (ButtonGetName(lst[i]),
			    "_DESKTOP_DESKRAY_DRAG_CONTROL"))
			  ButtonToggle(lst[i]);
		    }
	       }
	  }
     }
   else
     {
	lst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  ButtonToggle(lst[i]);
	       }
	  }
     }
   StackDesktops();
   autosave();
}

static void
ButtonsIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 i, len, num;

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
     }
   else if (!strncmp(cmd, "list", 2))
     {
	Button            **lst, *b;

	IpcPrintf("Win       d  s  l     x     y     w     h name\n");
	lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	for (i = 0; i < num; i++)
	  {
	     b = lst[i];
	     IpcPrintf("%#lx %2d %2d %2d %5d+%5d %5dx%5d %s\n",
		       EoGetWin(b), EoGetDesk(b), EoIsSticky(b), EoGetLayer(b),
		       EoGetX(b), EoGetY(b), EoGetW(b), EoGetH(b),
		       ButtonGetName(lst[i]));
	  }
	if (lst)
	   Efree(lst);

     }
   else if (!strncmp(cmd, "move", 2))
     {
	if (Mode_button.button)
	   ButtonDragStart(Mode_button.button);
     }
}

static void
IPC_ButtonShow(const char *params, Client * c __UNUSED__)
{
   doHideShowButton(params);
}

IpcItem             ButtonsIpcArray[] = {
   {
    ButtonsIpc,
    "button", "btn",
    "Button functions",
    "  button list               List buttons\n"},
   {
    IPC_ButtonShow,
    "button_show", NULL,
    "Show or Hide buttons on desktop",
    "use \"button_show <button/buttons/all_buttons_except/all> "
    "<BUTTON_STRING>\"\nexamples: \"button_show buttons all\" "
    "(removes all buttons and the dragbar)\n\"button_show\" "
    "(removes all buttons)\n \"button_show buttons CONFIG*\" "
    "(removes all buttons with CONFIG in the start)\n"},
};
#define N_IPC_FUNCS (sizeof(ButtonsIpcArray)/sizeof(IpcItem))

#if 0
static const CfgItem ButtonsCfgItems[] = {
   CFG_ITEM_BOOL(Conf.buttons, enable, 1),
};
#define N_CFG_ITEMS (sizeof(ButtonsCfgItems)/sizeof(CfgItem))
#endif

/*
 * Module descriptor
 */
EModule             ModButtons = {
   "buttons", "btn",
   ButtonsSighan,
   {N_IPC_FUNCS, ButtonsIpcArray},
   {0, NULL}
};
