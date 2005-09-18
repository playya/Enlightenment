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
#include "conf.h"
#include "emodule.h"
#include "eobj.h"
#include "tooltips.h"
#include "xwin.h"

static struct
{
   char                enable;
   char                showroottooltip;
   int                 delay;	/* milliseconds */
   unsigned int        opacity;
} Conf_tooltips;

static struct
{
   int                 inhibit;
   CB_GetAclass       *ac_func;
   void               *ac_data;
} Mode_tooltips;

struct _tooltip
{
   ImageClass         *iclass[5];
   TextClass          *tclass;
   int                 dist;
   Window              iwin;
   EObj               *win[5];
   char                visible;
   ImageClass         *tooltippic;
   unsigned int        ref_count;
};

#define TTWIN win[4]
#define TTICL iclass[4]

static ToolTip     *
TooltipCreate(const char *name, ImageClass * ic0, ImageClass * ic1,
	      ImageClass * ic2, ImageClass * ic3, ImageClass * ic4,
	      TextClass * tclass, int dist, ImageClass * tooltippic)
{
   int                 i, wh;
   ToolTip            *tt;
   EObj               *eo;

   if (ic0 == NULL || tclass == NULL)
      return NULL;

   tt = Ecalloc(1, sizeof(ToolTip));

   tt->iclass[0] = ic1;
   tt->iclass[1] = ic2;
   tt->iclass[2] = ic3;
   tt->iclass[3] = ic4;
   tt->iclass[4] = ic0;
   ic0->ref_count++;
   tt->tclass = tclass;
   tclass->ref_count++;
   tt->tooltippic = tooltippic;
   if (tooltippic)
      tooltippic->ref_count++;

   tt->dist = dist;

   for (i = 0; i < 5; i++)
     {
	if (!tt->iclass[i])
	   continue;

	wh = (i + 1) * 8;

	eo = EobjWindowCreate(EOBJ_TYPE_MISC, -50, -100, wh, wh, 1, NULL);
	tt->iclass[i]->ref_count++;
	EobjChangeOpacity(eo, OpacityExt(Conf_tooltips.opacity));
	tt->win[i] = eo;
     }
   tt->iwin = ECreateWindow(tt->TTWIN->win, 0, 0, 1, 1, 0);
   tt->TTWIN->name = Estrdup(name);

   tt->ref_count = 0;

   AddItem(tt, name, 0, LIST_TYPE_TOOLTIP);

   return tt;
}

#if 0				/* Not used */
static void
TooltipDestroy(ToolTip * tt)
{
   if (!tt)
      return;

   if (tt->ref_count > 0)
     {
	DialogOK(_("ToolTip Error!"), _("%u references remain\n"),
		 tt->ref_count);
     }
}
#endif

int
TooltipConfigLoad(FILE * ConfigFile)
{
   int                 err = 0;
   ToolTip            *tt;
   char               *name = 0;
   ImageClass         *drawiclass = 0;
   ImageClass         *bubble1 = 0, *bubble2 = 0, *bubble3 = 0, *bubble4 = 0;
   TextClass          *tclass = 0;
   ImageClass         *tooltiphelppic = 0;
   char                s[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1;
   int                 distance = 0;
   int                 fields;

   tt = NULL;
   while (GetLine(s, sizeof(s), ConfigFile))
     {
	s2[0] = 0;
	i1 = CONFIG_INVALID;
	fields = sscanf(s, "%i %4000s", &i1, s2);

	if (fields < 1)
	   i1 = CONFIG_INVALID;
	else if (i1 == CONFIG_CLOSE)
	  {
	     if (fields != 1)
		Alert(_("CONFIG: ignoring extra data in \"%s\"\n"), s);
	  }
	else if (i1 != CONFIG_INVALID)
	  {
	     if (fields != 2)
	       {
		  Alert(_("CONFIG: missing required data in \"%s\"\n"), s);
	       }
	  }
	switch (i1)
	  {
	  case CONFIG_CLOSE:
	     if ((drawiclass) && (tclass) && (name))
		tt = TooltipCreate(name, drawiclass, bubble1, bubble2,
				   bubble3, bubble4, tclass, distance,
				   tooltiphelppic);
	     if (name)
		Efree(name);
	     goto done;

	  case CONFIG_CLASSNAME:
	     if (ConfigSkipIfExists(ConfigFile, s2, LIST_TYPE_TOOLTIP))
		goto done;
	     name = Estrdup(s2);
	     break;
	  case TOOLTIP_DRAWICLASS:
	  case CONFIG_IMAGECLASS:
	     drawiclass = ImageclassFind(s2, 0);
	     break;
	  case TOOLTIP_BUBBLE1:
	     bubble1 = ImageclassFind(s2, 0);
	     break;
	  case TOOLTIP_BUBBLE2:
	     bubble2 = ImageclassFind(s2, 0);
	     break;
	  case TOOLTIP_BUBBLE3:
	     bubble3 = ImageclassFind(s2, 0);
	     break;
	  case TOOLTIP_BUBBLE4:
	     bubble4 = ImageclassFind(s2, 0);
	     break;
	  case CONFIG_TEXT:
	     tclass = TextclassFind(s2, 1);
	     break;
	  case TOOLTIP_DISTANCE:
	     distance = atoi(s2);
	     break;
	  case TOOLTIP_HELP_PIC:
	     tooltiphelppic = ImageclassFind(s2, 0);
	     break;
	  default:
	     Alert(_("Warning: unable to determine what to do with\n"
		     "the following text in the middle of current "
		     "ToolTip definition:\n"
		     "%s\nWill ignore and continue...\n"), s);
	     break;
	  }
     }
   err = -1;

 done:
   return err;
}

static ImageClass  *
TooltipCreateIclass(const char *name, const char *file, int *pw, int *ph)
{
   ImageClass         *ic;

   ic = ImageclassFind(name, 0);
   if (!ic)
      ic = ImageclassCreateSimple(name, file);

   if (ic->norm.normal && ic->norm.normal->im)
     {
	imlib_context_set_image(ic->norm.normal->im);
	if (*pw < imlib_image_get_width())
	   *pw = imlib_image_get_width();
	if (*ph < imlib_image_get_height())
	   *ph = imlib_image_get_height();
     }

   return ic;
}

static void
TooltipIclassPaste(ToolTip * tt, const char *ic_name, int x, int y, int *px)
{
   ImageClass         *ic;

   ic = ImageclassFind(ic_name, 0);
   if (!ic || !ic->norm.normal->im)
      return;

   imlib_context_set_image(ic->norm.normal->im);
   imlib_context_set_drawable(tt->TTWIN->win);
   imlib_context_set_blend(1);
   imlib_render_image_on_drawable(x, y);
   imlib_context_set_blend(0);

   *px = x + imlib_image_get_width();
}

void
TooltipShow(ToolTip * tt, const char *text, ActionClass * ac, int x, int y)
{
   int                 i, w, h, ix, iy, iw, ih, dx, dy, xx, yy;
   int                 ww, hh, adx, ady, dist;
   int                 headline_h = 0, headline_w = 0, icons_width =
      0, labels_width = 0, double_w = 0;
   Imlib_Image        *im;
   int                *heights = NULL;
   ImageClass         *ic;
   int                 cols[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   int                 num, modifiers;
   Action             *aa;
   const char         *tts;
   EObj               *eo;

   if (!tt || Mode.mode != MODE_NONE)
      return;

   /* if we get an actionclass, look for tooltip action texts */
   h = 0;
   if (ac)
     {
	num = ActionclassGetActionCount(ac);
	heights = Emalloc(num * sizeof(int));

	for (i = 0; i < num; i++)
	  {
	     int                 temp_w, temp_h;

	     temp_w = 0;
	     temp_h = 0;

	     aa = ActionclassGetAction(ac, i);
	     if (!aa)
		continue;

	     tts = ActionGetTooltipString(aa);
	     if (!tts)
		continue;
	     tts = _(tts);

	     TextSize(tt->tclass, 0, 0, STATE_NORMAL, tts, &temp_w, &temp_h,
		      17);
	     if (temp_w > labels_width)
		labels_width = temp_w;
	     temp_w = 0;

	     if (ActionGetEvent(aa) == EVENT_DOUBLE_DOWN)
	       {
		  TextSize(tt->tclass, 0, 0, STATE_NORMAL, "2x", &double_w,
			   &temp_h, 17);
		  if (cols[0] < double_w)
		     cols[0] = double_w;
	       }

	     if (ActionGetAnybutton(aa))
	       {
		  TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_ANY",
				      "pix/mouse_any.png", &cols[1], &temp_h);
	       }
	     else
		switch (ActionGetButton(aa))
		  {
		  case 1:
		     ic = TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_1",
					      "pix/mouse_1.png", &cols[1],
					      &temp_h);
		     break;
		  case 2:
		     ic = TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_2",
					      "pix/mouse_2.png", &cols[1],
					      &temp_h);
		     break;
		  case 3:
		     ic = TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_3",
					      "pix/mouse_3.png", &cols[1],
					      &temp_h);
		     break;
		  case 4:
		     ic = TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_4",
					      "pix/mouse_4.png", &cols[1],
					      &temp_h);
		     break;
		  case 5:
		     ic = TooltipCreateIclass("TOOLTIP_MOUSEBUTTON_5",
					      "pix/mouse_5.png", &cols[1],
					      &temp_h);
		     break;
		  case 0:
		  default:
		     break;
		  }

	     modifiers = ActionGetModifiers(aa);
	     if (modifiers)
	       {
		  if (modifiers & ShiftMask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_SHIFT",
					      "pix/key_shift.png",
					      &cols[2], &temp_h);
		  if (modifiers & LockMask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_LOCK",
					      "pix/key_lock.png",
					      &cols[3], &temp_h);
		  if (modifiers & ControlMask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_CTRL",
					      "pix/key_ctrl.png",
					      &cols[4], &temp_h);
		  if (modifiers & Mod1Mask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_MOD1",
					      "pix/key_mod1.png",
					      &cols[5], &temp_h);
		  if (modifiers & Mod2Mask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_MOD2",
					      "pix/key_mod2.png",
					      &cols[6], &temp_h);
		  if (modifiers & Mod3Mask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_MOD3",
					      "pix/key_mod3.png",
					      &cols[7], &temp_h);
		  if (modifiers & Mod4Mask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_MOD4",
					      "pix/key_mod4.png",
					      &cols[8], &temp_h);
		  if (modifiers & Mod5Mask)
		     ic = TooltipCreateIclass("TOOLTIP_KEY_MOD5",
					      "pix/key_mod5.png",
					      &cols[9], &temp_h);
	       }

	     temp_w = cols[0] + cols[1] + cols[2] + cols[3] + cols[4] +
		cols[5] + cols[6] + cols[7] + cols[8] + cols[9];

	     if (temp_w > icons_width)
		icons_width = temp_w;
	     heights[i] = temp_h;
	     h += temp_h;
	  }
     }

   TextSize(tt->tclass, 0, 0, STATE_NORMAL, text, &headline_w, &headline_h, 17);
   if (headline_w < icons_width + labels_width)
      w = icons_width + labels_width;
   else
      w = headline_w;
   h += headline_h;

   ic = tt->TTICL;
   iw = 0;
   ih = 0;
   if (tt->tooltippic)
     {
	im = ELoadImage(tt->tooltippic->norm.normal->im_file);
	ix = 0;
	iy = 0;
	if (im)
	  {
	     imlib_context_set_image(im);
	     iw = imlib_image_get_width();
	     ih = imlib_image_get_height();
	     imlib_free_image();
	  }
	w += iw;
	if (h < ih)
	   h = ih;
     }
   w += ic->padding.left + ic->padding.right;
   h += ic->padding.top + ic->padding.bottom;

   if ((tt->tooltippic) && (iw > 0) && (ih > 0))
     {
	ix = ic->padding.left;
	iy = (h - ih) / 2;
	EMoveResizeWindow(tt->iwin, ix, iy, iw, ih);
	EMapWindow(tt->iwin);
	ImageclassApply(tt->tooltippic, tt->iwin, iw, ih, 0, 0, STATE_NORMAL, 0,
			ST_TOOLTIP);
     }
   else
      EUnmapWindow(tt->iwin);

   dx = x - VRoot.w / 2;
   dy = y - VRoot.h / 2;

   if ((dy == 0) && (dx == 0))
      dy = -1;

   adx = dx;
   if (adx < 0)
      adx = -adx;
   ady = dy;
   if (ady < 0)
      ady = -ady;
   if (adx < ady)
      /*   +-------+   */
      /*   |\#####/|   */
      /*   | \###/ |   */
      /*   |  \#/  |   */
      /*   |  /#\  |   */
      /*   | /###\ |   */
      /*   |/#####\|   */
      /*   +-------+   */
     {
	dist = tt->dist;
	ady = ady / dy;

	if (tt->win[0])
	  {
	     yy = y - ((ady * 10 * dist) / 100);
	     xx = x - (dist * 10 * dx) / (100 * VRoot.w / 2);
	     EobjMove(tt->win[0], xx - 4, yy - 4);
	  }

	if (tt->win[1])
	  {
	     yy = y - ((ady * 30 * dist) / 100);
	     xx = x - (dist * 30 * dx) / (100 * VRoot.w / 2);
	     EobjMove(tt->win[1], xx - 8, yy - 8);
	  }

	if (tt->win[2])
	  {
	     yy = y - ((ady * 50 * dist) / 100);
	     xx = x - (dist * 50 * dx) / (100 * VRoot.w / 2);
	     EobjMove(tt->win[2], xx - 12, yy - 12);
	  }

	if (tt->win[3])
	  {
	     yy = y - ((ady * 80 * dist) / 100);
	     xx = x - (dist * 80 * dx) / (100 * VRoot.w / 2);
	     EobjMove(tt->win[3], xx - 16, yy - 16);
	  }

	yy = y - ((ady * 100 * dist) / 100);
	xx = x - (dist * 100 * dx) / (100 * VRoot.w / 2);
	if (ady < 0)
	   hh = 0;
	else
	   hh = h;
	ww = (w / 2) + ((dx * w) / (VRoot.w / 2));
     }
   else
      /*   +-------+   */
      /*   |\     /|   */
      /*   |#\   /#|   */
      /*   |##\ /##|   */
      /*   |##/ \##|   */
      /*   |#/   \#|   */
      /*   |/     \|   */
      /*   +-------+   */
     {
	if (dx == 0)
	  {
	     dx = 1;
	     adx = 1;
	  }
	dist = tt->dist;
	adx = adx / dx;

	if (tt->win[0])
	  {
	     xx = x - ((adx * 10 * dist) / 100);
	     yy = y - (dist * 10 * dy) / (100 * VRoot.h / 2);
	     EobjMove(tt->win[0], xx - 4, yy - 4);
	  }

	if (tt->win[1])
	  {
	     xx = x - ((adx * 30 * dist) / 100);
	     yy = y - (dist * 30 * dy) / (100 * VRoot.h / 2);
	     EobjMove(tt->win[1], xx - 8, yy - 8);
	  }

	if (tt->win[2])
	  {
	     xx = x - ((adx * 50 * dist) / 100);
	     yy = y - (dist * 50 * dy) / (100 * VRoot.h / 2);
	     EobjMove(tt->win[2], xx - 12, yy - 12);
	  }

	if (tt->win[3])
	  {
	     xx = x - ((adx * 80 * dist) / 100);
	     yy = y - (dist * 80 * dy) / (100 * VRoot.h / 2);
	     EobjMove(tt->win[3], xx - 16, yy - 16);
	  }

	xx = x - ((adx * 100 * dist) / 100);
	yy = y - (dist * 100 * dy) / (100 * VRoot.h / 2);
	if (adx < 0)
	   ww = 0;
	else
	   ww = w;
	hh = (h / 2) + ((dy * h) / (VRoot.h / 2));
     }

   EobjMoveResize(tt->TTWIN, xx - ww, yy - hh, w, h);

   for (i = 0; i < 5; i++)
     {
	eo = tt->win[i];
	if (eo)
	   ImageclassApply(tt->iclass[i], eo->win, eo->w, eo->h, 0, 0,
			   STATE_NORMAL, 0, ST_TOOLTIP);
     }

   for (i = 0; i < 5; i++)
      if (tt->win[i])
	 EobjMap(tt->win[i], 0);

   ESync();

   xx = ic->padding.left + iw;

   /* draw the ordinary tooltip text */
   TextDraw(tt->tclass, tt->TTWIN->win, 0, 0, STATE_NORMAL, text, xx,
	    ic->padding.top, headline_w, headline_h, 17, 512);

   /* draw the icons and labels, if any */
   if (ac)
     {
	num = ActionclassGetActionCount(ac);
	y = ic->padding.top + headline_h;
	xx = ic->padding.left + double_w;

	for (i = 0; i < num; i++)
	  {
	     x = xx + iw;

	     aa = ActionclassGetAction(ac, i);
	     if (!aa)
		continue;

	     tts = ActionGetTooltipString(aa);
	     if (!tts)
		continue;
	     tts = _(tts);

	     if (ActionGetEvent(aa) == EVENT_DOUBLE_DOWN)
	       {
		  TextDraw(tt->tclass, tt->TTWIN->win, 0, 0, STATE_NORMAL, "2x",
			   xx + iw - double_w, y, double_w, heights[i], 17, 0);
	       }

	     if (ActionGetAnybutton(aa))
	       {
		  TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_ANY", x, y, &x);
	       }
	     else
		switch (ActionGetButton(aa))
		  {
		  case 1:
		     TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_1", x, y, &x);
		     break;
		  case 2:
		     TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_2", x, y, &x);
		     break;
		  case 3:
		     TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_3", x, y, &x);
		     break;
		  case 4:
		     TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_4", x, y, &x);
		     break;
		  case 5:
		     TooltipIclassPaste(tt, "TOOLTIP_MOUSEBUTTON_5", x, y, &x);
		     break;
		  default:
		     break;
		  }

	     modifiers = ActionGetModifiers(aa);
	     if (modifiers)
	       {
		  if (modifiers & ShiftMask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_SHIFT", x, y, &x);
		  if (modifiers & LockMask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_LOCK", x, y, &x);
		  if (modifiers & ControlMask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_CTRL", x, y, &x);
		  if (modifiers & Mod1Mask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_MOD1", x, y, &x);
		  if (modifiers & Mod2Mask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_MOD2", x, y, &x);
		  if (modifiers & Mod3Mask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_MOD3", x, y, &x);
		  if (modifiers & Mod4Mask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_MOD4", x, y, &x);
		  if (modifiers & Mod5Mask)
		     TooltipIclassPaste(tt, "TOOLTIP_KEY_MOD5", x, y, &x);
	       }

	     TextDraw(tt->tclass, tt->TTWIN->win, 0, 0, STATE_NORMAL, tts,
		      ic->padding.left + icons_width + iw, y,
		      labels_width, heights[i], 17, 0);
	     y += heights[i];

	  }
     }

   if (heights)
      Efree(heights);
}

void
TooltipHide(ToolTip * tt)
{
   int                 i;

   if (!tt || !tt->TTWIN->shown)
      return;

   for (i = 4; i >= 0; i--)
      if (tt->win[i])
	 EobjUnmap(tt->win[i]);

#if 0				/* FIXME - Remove? */
   ESync();
#endif
}

/*
 * Tooltips
 */

void
TooltipsHide(void)
{
   ToolTip           **lst;
   int                 i, j;

   lst = (ToolTip **) ListItemType(&j, LIST_TYPE_TOOLTIP);
   if (lst)
     {
	for (i = 0; i < j; i++)
	  {
	     TooltipHide(lst[i]);
	  }
	Efree(lst);
     }
}

void
TooltipsEnable(int enable)
{
   if (enable)
     {
	if (Mode_tooltips.inhibit > 0)
	   Mode_tooltips.inhibit--;
     }
   else
     {
	Mode_tooltips.inhibit++;
     }
}

static ToolTip     *ttip = NULL;

static void
ToolTipTimeout(int val __UNUSED__, void *data __UNUSED__)
{
   int                 x, y;
   unsigned int        mask;
   ActionClass        *ac;
   const char         *tts;

   if (!ttip)
      ttip = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_TOOLTIP);
   if (!ttip)
      return;

   /* In the case of multiple screens, check to make sure
    * the root window is still where the mouse is... */
   if (!EQueryPointer(VRoot.win, &x, &y, NULL, &mask))
      return;

   /* In case this is a virtual root */
   if (x < 0 || y < 0 || x >= VRoot.w || y >= VRoot.h)
      return;

   /* dont pop up tooltip is mouse button down */
   if (mask &
       (Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask))
      return;

   if (!Mode_tooltips.ac_func)
      return;
   ac = Mode_tooltips.ac_func(Mode_tooltips.ac_data);
   if (!ac)
      return;

   tts = ActionclassGetTooltipString(ac);
   if (!tts)
      return;

   TooltipShow(ttip, _(tts), ac, x, y);
}

/*
 * We want this on
 * ButtonPress, ButtonRelease, MotionNotify, EnterNotify, LeaveNotify
 */
void
TooltipsSetPending(int type, CB_GetAclass * func, void *data)
{
   Mode_tooltips.ac_func = func;
   Mode_tooltips.ac_data = data;

   if (ttip)
      TooltipHide(ttip);

   RemoveTimerEvent("TOOLTIP_TIMEOUT");

   if (!func)
      return;
   if (Mode_tooltips.inhibit || !Conf_tooltips.enable)
      return;
   if (type && !Conf_tooltips.showroottooltip)
      return;

   DoIn("TOOLTIP_TIMEOUT", 0.001 * Conf_tooltips.delay, ToolTipTimeout, 0,
	NULL);
}

/*
 * Tooltips Module
 */

static void
TooltipsSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	memset(&Mode_tooltips, 0, sizeof(Mode_tooltips));
	break;
     case ESIGNAL_AREA_SWITCH_START:
     case ESIGNAL_DESK_SWITCH_START:
	TooltipsHide();
	break;
     }
}

/*
 * Configuration dialog
 */
static char         tmp_tooltips;
static int          tmp_tooltiptime;
static char         tmp_roottip;

static void
CB_ConfigureTooltips(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	Conf_tooltips.enable = tmp_tooltips;
	Conf_tooltips.delay = tmp_tooltiptime * 10;
	Conf_tooltips.showroottooltip = tmp_roottip;
     }
   autosave();
}

static void
SettingsTooltips(void)
{
   Dialog             *d;
   DItem              *table, *di;

   if ((d =
	FindItem("CONFIGURE_TOOLTIPS", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_TOOLTIPS");

   tmp_tooltips = Conf_tooltips.enable;
   tmp_tooltiptime = Conf_tooltips.delay / 10;
   tmp_roottip = Conf_tooltips.showroottooltip;

   d = DialogCreate("CONFIGURE_TOOLTIPS");
   DialogSetTitle(d, _("Tooltip Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 2, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	di = DialogAddItem(table, DITEM_IMAGE);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemImageSetFile(di, "pix/tips.png");

	di = DialogAddItem(table, DITEM_TEXT);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSetText(di, _("Enlightenment Tooltip\n" "Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetColSpan(di, 2);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Display Tooltips"));
   DialogItemCheckButtonSetState(di, tmp_tooltips);
   DialogItemCheckButtonSetPtr(di, &tmp_tooltips);

   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Display Root Window Tips"));
   DialogItemCheckButtonSetState(di, tmp_roottip);
   DialogItemCheckButtonSetPtr(di, &tmp_roottip);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 0, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemSetText(di, _("Tooltip Delay:\n"));

   di = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 0, 300);
   DialogItemSliderSetUnits(di, 10);
   DialogItemSliderSetJump(di, 25);
   DialogItemSliderSetVal(di, tmp_tooltiptime);
   DialogItemSliderSetValPtr(di, &tmp_tooltiptime);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 2);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureTooltips, 1, DIALOG_BUTTON_OK);
   DialogAddButton(d, _("Apply"), CB_ConfigureTooltips, 0, DIALOG_BUTTON_APPLY);
   DialogAddButton(d, _("Close"), CB_ConfigureTooltips, 1, DIALOG_BUTTON_CLOSE);
   DialogSetExitFunction(d, CB_ConfigureTooltips, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureTooltips, 0);
   ShowDialog(d);
}

static void
TooltipsIpc(const char *params, Client * c __UNUSED__)
{
   if (params && !strncmp(params, "cfg", 3))
     {
	SettingsTooltips();
     }
}

IpcItem             TooltipsIpcArray[] = {
   {
    TooltipsIpc,
    "tooltips", "tt",
    "Tooltip functions",
    "  tooltips cfg          Configure tooltips\n"}
};
#define N_IPC_FUNCS (sizeof(TooltipsIpcArray)/sizeof(IpcItem))

static const CfgItem TooltipsCfgItems[] = {
   CFG_ITEM_BOOL(Conf_tooltips, enable, 1),
   CFG_ITEM_BOOL(Conf_tooltips, showroottooltip, 1),
   CFG_ITEM_INT(Conf_tooltips, delay, 1500),
   CFG_ITEM_INT(Conf_tooltips, opacity, 200),
};
#define N_CFG_ITEMS (sizeof(TooltipsCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
EModule             ModTooltips = {
   "tooltips", "tt",
   TooltipsSighan,
   {N_IPC_FUNCS, TooltipsIpcArray},
   {N_CFG_ITEMS, TooltipsCfgItems}
};
