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
#define DECLARE_STRUCT_MENU 1
#include "E.h"
#include <sys/stat.h>
#include <errno.h>
#include <X11/keysym.h>

#define MENU_ITEM_EVENT_MASK \
	ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | PointerMotionMask

static void         MenuRedraw(Menu * m);
static void         MenuActivateItem(Menu * m, MenuItem * mi);
static void         MenuDrawItem(Menu * m, MenuItem * mi, char shape);

static void         FileMenuUpdate(int val, void *data);
static void         FillFlatFileMenu(Menu * m, MenuStyle * ms, char *name,
				     char *file, Menu * parent);

static Menu        *active_menu = NULL;
static MenuItem    *active_item = NULL;

static void
GrabKeyboard(Window win)
{
   int                 rc;

   rc = XGrabKeyboard(disp, win, False, GrabModeAsync, GrabModeAsync,
		      CurrentTime);
}

static void
UngrabKeyboard(void)
{
   int                 rc;

   rc = XUngrabKeyboard(disp, CurrentTime);
}

static Menu        *
FindMenuItem(Window win, MenuItem ** mi)
{
   Menu               *menu = NULL;
   Menu              **menus;
   int                 i, j, num;

   EDBUG(6, "FindMenuItem");

   menus = (Menu **) ListItemType(&num, LIST_TYPE_MENU);
   for (i = 0; i < num; i++)
     {
	for (j = 0; j < menus[i]->num; j++)
	  {
	     if ((win == menus[i]->items[j]->win) ||
		 (win == menus[i]->items[j]->icon_win))
	       {
		  *mi = menus[i]->items[j];
		  menu = menus[i];
		  break;
	       }
	  }
     }
   if (menus)
      Efree(menus);

   EDBUG_RETURN(menu);
}

Menu               *
FindMenu(Window win)
{
   Menu               *menu = NULL;
   Menu              **menus;
   int                 i, num;

   EDBUG(6, "FindMenu");

   menus = (Menu **) ListItemType(&num, LIST_TYPE_MENU);
   for (i = 0; i < num; i++)
     {
	if (menus[i]->win != win)
	   continue;
	menu = menus[i];
	break;
     }
   if (menus)
      Efree(menus);

   EDBUG_RETURN(menu);
}

static EWin        *
FindEwinSpawningMenu(Menu * m)
{
   EWin               *ewin = NULL;
   EWin               *const *ewins;
   int                 i, num;

   EDBUG(6, "FindEwinSpawningMenu");

   ewins = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (ewins[i]->shownmenu == m->win)
	   return ewins[i];
     }

   EDBUG_RETURN(ewin);
}

static void
MenuHide(Menu * m)
{
   EWin               *ewin;

   EDBUG(5, "MenuHide");

   if (m->win)
      EUnmapWindow(disp, m->win);

   MenuActivateItem(m, NULL);

   m->stuck = 0;
   m->shown = 0;

   ewin = FindEwinSpawningMenu(m);
   if (ewin)
      ewin->shownmenu = 0;

   ewin = FindEwinByMenu(m);
   if (ewin)
      HideEwin(ewin);

   EDBUG_RETURN_;
}

static void
MenuEwinMoveResize(EWin * ewin, int resize __UNUSED__)
{
   Menu               *m = ewin->menu;

   if (!m)
      return;

   if (Conf.theme.transparency)
      m->redraw = 1;

   if ((!m->style->use_item_bg && m->pmm.pmap == 0) || m->redraw)
      MenuRedraw(m);
}

static void
MenuEwinRefresh(EWin * ewin)
{
   MenuEwinMoveResize(ewin, 0);
}

static void
MenuEwinClose(EWin * ewin)
{
   if (ewin->menu == active_menu)
     {
	UngrabKeyboard();
	active_menu = NULL;
	active_item = NULL;
     }

   ewin->menu = NULL;
}

static void
MenuEwinInit(EWin * ewin, void *ptr)
{
   ewin->menu = (Menu *) ptr;
   ewin->MoveResize = MenuEwinMoveResize;
   ewin->Refresh = MenuEwinRefresh;
   ewin->Close = MenuEwinClose;
}

static void
MenuShow(Menu * m, char noshow)
{
   EWin               *ewin;
   int                 x, y;
   int                 wx = 0, wy = 0;	/* wx, wy added to stop menus */
   unsigned int        w, h, mw, mh;	/* from appearing offscreen */
   int                 head_num = 0;

   EDBUG(5, "MenuShow");
   if ((m->num <= 0) || (!m->style))
      EDBUG_RETURN_;

   if (m->shown)
      return;

   if (m->stuck)
     {
	Button             *button;
	EWin               *ewin99;

	if ((button = FindButton(Mode.context_win)))
	  {
	     ButtonDrawWithState(button, STATE_NORMAL);
	  }
	else if ((ewin99 = FindEwinByDecoration(Mode.context_win)))
	  {
	     int                 i99;

	     for (i99 = 0; i99 < ewin99->border->num_winparts; i99++)
	       {
		  if (Mode.context_win == ewin99->bits[i99].win)
		    {
		       ewin99->bits[i99].state = STATE_NORMAL;
		       BorderWinpartChange(ewin99, i99, 0);
		       i99 = ewin99->border->num_winparts;
		    }
	       }
	  }
	EDBUG_RETURN_;
     }

   if (!m->win)
      MenuRealize(m);

   ewin = FindEwinByMenu(m);
   if (ewin)
     {
	if ((Mode.button) &&
	    FindItem((char *)Mode.button, 0, LIST_FINDBY_POINTER,
		     LIST_TYPE_BUTTON))
	  {
	     ButtonDrawWithState(Mode.button, STATE_NORMAL);
	  }
#if 0				/* ??? */
	RaiseEwin(ewin);
	ShowEwin(ewin);
	EDBUG_RETURN_;
#else
	MenuHide(m);
#endif
     }

   GetWinXY(m->items[0]->win, &x, &y);
   GetWinWH(m->items[0]->win, &w, &h);
   GetWinWH(m->win, &mw, &mh);

   wx = 0;
   wy = 0;
   if (Conf.menusonscreen)
     {
	Border             *b;

	b = (Border *) FindItem(m->style->border_name, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BORDER);
	if (b)
	  {
	     int                 width;
	     int                 height;
	     int                 x_origin;
	     int                 y_origin;

	     head_num =
		GetPointerScreenGeometry(&x_origin, &y_origin, &width, &height);

	     if (Mode.x - x - ((int)mw / 2) > (x_origin + width))
		wx = x_origin + (int)b->border.left;
	     else if (Mode.x + ((int)mw / 2) > (int)(x_origin + width))
		wx = (x_origin + width) - (int)mw - (int)b->border.right;
	     else
		wx = Mode.x - x - ((int)w / 2);

	     if ((wx - ((int)w / 2)) < x_origin)
		wx = x_origin + (int)b->border.left;

	     if (Mode.y + (int)mh > (int)VRoot.h)
		wy = (y_origin + height) - (int)mh - (int)b->border.bottom;
	     else
		wy = Mode.y - y - ((int)h / 2);

	     if ((wy - ((int)h / 2) - (int)b->border.top) < y_origin)
		wy = y_origin + (int)b->border.top;
	  }
     }

   if ((Mode.x >= 0) && (Mode.y >= 0))
     {
	if (Conf.menusonscreen)
	   EMoveWindow(disp, m->win, wx, wy);
	else
	   EMoveWindow(disp, m->win, Mode.x - x - (w / 2),
		       Mode.y - y - (h / 2));
     }
   else if ((Mode.x >= 0) && (Mode.y < 0))
     {
	if (((-Mode.y) + (int)mh) > (int)VRoot.h)
	   Mode.y = -((-Mode.y) - Mode.context_h - mh);
	if (Conf.menusonscreen)
	   EMoveWindow(disp, m->win, wx, -Mode.y);
	else
	   EMoveWindow(disp, m->win, Mode.x - x - (w / 2), -Mode.y);
     }
   else if ((Mode.x < 0) && (Mode.y >= 0))
     {
	if (((-Mode.x) + (int)mw) > (int)VRoot.w)
	   Mode.x = -((-Mode.x) - Mode.context_w - mw);
	if (Conf.menusonscreen)
	   EMoveWindow(disp, m->win, -Mode.x, wy);
	else
	   EMoveWindow(disp, m->win, -Mode.x, Mode.y - y - (h / 2));
     }
   else
     {
	if (((-Mode.x) + (int)mw) > (int)VRoot.w)
	   Mode.x = -((-Mode.x) - Mode.context_w - mw);
	if (((-Mode.y) + (int)mh) > (int)VRoot.h)
	   Mode.y = -((-Mode.y) - Mode.context_h - mh);
	EMoveWindow(disp, m->win, -Mode.x, -Mode.y);
     }

   ewin = AddInternalToFamily(m->win, m->style->border_name, EWIN_TYPE_MENU, m,
			      MenuEwinInit);
   if (ewin)
     {
	ewin->client.event_mask |= KeyPressMask;
	XSelectInput(disp, m->win, ewin->client.event_mask);

	ewin->head = head_num;
	if (Conf.menuslide)
	   EwinInstantShade(ewin, 0);
	ICCCM_Cmap(NULL);
	MoveEwin(ewin, ewin->x, ewin->y);
	if (!noshow)
	  {
	     ShowEwin(ewin);
	     if (Conf.menuslide)
		EwinUnShade(ewin);
	  }
     }

   m->stuck = 0;

   if (!FindMenu(m->win))
      AddItem(m, m->name, m->win, LIST_TYPE_MENU);

   {
      Button             *button;
      EWin               *ewin99;

      if ((button = FindButton(Mode.context_win)))
	{
	   ButtonDrawWithState(button, STATE_NORMAL);
	}
      else if ((ewin99 = FindEwinByDecoration(Mode.context_win)))
	{
	   int                 i99;

	   for (i99 = 0; i99 < ewin99->border->num_winparts; i99++)
	     {
		if (Mode.context_win == ewin99->bits[i99].win)
		  {
		     ewin99->bits[i99].state = STATE_NORMAL;
		     BorderWinpartChange(ewin99, i99, 0);
		     i99 = ewin99->border->num_winparts;
		  }
	     }
	}
   }

   m->shown = 1;
   if (Mode.cur_menu_depth == 0)
     {
	XSync(disp, False);
	GrabKeyboard(m->win);
     }

   EDBUG_RETURN_;
}

MenuStyle          *
MenuStyleCreate(void)
{
   MenuStyle          *ms;

   EDBUG(5, "MenuStyleCreate");

   ms = Ecalloc(1, sizeof(MenuStyle));
   ms->iconpos = ICON_LEFT;

   EDBUG_RETURN(ms);
}

MenuItem           *
MenuItemCreate(const char *text, ImageClass * iclass, int action_id,
	       const char *action_params, Menu * child)
{
   MenuItem           *mi;

   EDBUG(5, "MenuItemCreate");
   mi = Ecalloc(1, sizeof(MenuItem));

   mi->icon_iclass = iclass;
   if (iclass)
      iclass->ref_count++;

   mi->text = (text) ? Estrdup((text[0]) ? _(text) : "?!?") : NULL;
   mi->act_id = action_id;
   mi->params = Estrdup(action_params);
   mi->child = child;
   mi->state = STATE_NORMAL;

   EDBUG_RETURN(mi);
}

Menu               *
MenuCreate(const char *name)
{
   Menu               *m;

   EDBUG(5, "MenuCreate");

   m = Ecalloc(1, sizeof(Menu));
   MenuAddName(m, name);

   EDBUG_RETURN(m);
}

void
MenuDestroy(Menu * m)
{
   int                 i, j;
   char                s[4096];

   EDBUG(5, "MenuDestroy");

   if (!m)
      EDBUG_RETURN_;

   MenuHide(m);

   if (m->win)
      EDestroyWindow(disp, m->win);

   Esnprintf(s, sizeof(s), "__.%s", m->name);
   RemoveTimerEvent(s);
   RemoveItem((char *)m, m->win, LIST_FINDBY_POINTER, LIST_TYPE_MENU);
   if (m->name)
      Efree(m->name);
   if (m->title)
      Efree(m->title);

   for (i = 0; i < m->num; i++)
     {
	if (m->items[i])
	  {
	     if (m->items[i]->child)
	       {
		  if (FindItem
		      ((char *)m->items[i]->child, 0, LIST_FINDBY_POINTER,
		       LIST_TYPE_MENU))
		     MenuDestroy(m->items[i]->child);
	       }
	     if (m->items[i]->text)
		Efree(m->items[i]->text);
	     if (m->items[i]->params)
		Efree(m->items[i]->params);
	     for (j = 0; j < 3; j++)
		FreePmapMask(&(m->items[i]->pmm[j]));
	     if (m->items[i]->icon_iclass)
		m->items[i]->icon_iclass->ref_count--;
	     if (m->items[i])
		Efree(m->items[i]);
	  }
     }

   if (m->items)
      Efree(m->items);
   if (m->data)
      Efree(m->data);
   FreePmapMask(&m->pmm);

   Efree(m);

   EDBUG_RETURN_;
}

/* NB - this doesnt free imageclasses if we created them for the menu
 * FIXME: so it will leak if we create new imageclasses and stop using
 * old ones for menu icons. we need to add some ref counting in menu icon
 * imageclasses to knw to free them when not used
 */
static void
MenuEmpty(Menu * m)
{
   int                 i, j;

   EDBUG(5, "MenuEmpty");
   for (i = 0; i < m->num; i++)
     {
	if (m->items[i])
	  {
	     if (m->items[i]->child)
		MenuDestroy(m->items[i]->child);
	     if (m->items[i]->text)
		Efree(m->items[i]->text);
	     if (m->items[i]->params)
		Efree(m->items[i]->params);
	     for (j = 0; j < 3; j++)
		FreePmapMask(&(m->items[i]->pmm[j]));
	     if (m->items[i]->win)
		EDestroyWindow(disp, m->items[i]->win);
	     if (m->items[i])
		Efree(m->items[i]);
	  }
     }
   if (m->items)
      Efree(m->items);
   m->items = NULL;
   m->num = 0;
   EDBUG_RETURN_;
}

static void
MenuRepack(Menu * m)
{
   EWin               *ewin;
   unsigned int        w, h;

   EDBUG(5, "MenuRepack");

   m->redraw = 1;
   if (m->win)
      MenuRealize(m);

   ewin = FindEwinByMenu(m);
   if (ewin)
     {
	GetWinWH(m->win, &w, &h);
	ewin->client.height.min = h;
	ewin->client.height.max = h;
	ewin->client.width.min = w;
	ewin->client.width.max = w;
	ResizeEwin(ewin, w, h);
	RaiseEwin(ewin);
     }

   EDBUG_RETURN_;
}

void
MenuAddItem(Menu * menu, MenuItem * item)
{
   EDBUG(5, "MenuAddItem");
   menu->num++;
   menu->items = Erealloc(menu->items, sizeof(MenuItem *) * menu->num);
   menu->items[menu->num - 1] = item;
   EDBUG_RETURN_;
}

void
MenuAddName(Menu * menu, const char *name)
{
   EDBUG(5, "MenuAddName");
   if (menu->name)
      Efree(menu->name);
   menu->name = Estrdup(name);
   AddItem(menu, menu->name, menu->win, LIST_TYPE_MENU);
   EDBUG_RETURN_;
}

void
MenuAddTitle(Menu * menu, const char *title)
{
   EDBUG(5, "MenuAddTitle");
   if (menu->title)
      Efree(menu->title);
   menu->title = (title) ? Estrdup(_(title)) : NULL;
   EDBUG_RETURN_;
}

void
MenuAddStyle(Menu * menu, const char *style)
{
   EDBUG(5, "MenuAddStyle");
   menu->style = FindItem(style, 0, LIST_FINDBY_NAME, LIST_TYPE_MENU_STYLE);
   EDBUG_RETURN_;
}

void
MenuRealize(Menu * m)
{
   int                 i, maxh = 0, maxw = 0;
   int                 maxx1, maxx2, w, h, x, y, r, mmw, mmh;
   unsigned int        iw, ih;
   Imlib_Image        *im;
   XSetWindowAttributes att;
   char                pq, has_i, has_s;

   EDBUG(5, "MenuRealize");
   if (!m->style)
      EDBUG_RETURN_;

   if (!m->win)
      m->win = ECreateWindow(VRoot.win, 0, 0, 1, 1, 0);

   if (m->title)
     {
	HintsSetWindowName(m->win, m->title);
     }

   maxh = 0;
   maxx1 = 0;
   maxx2 = 0;
   has_i = 0;
   has_s = 0;
   att.event_mask = MENU_ITEM_EVENT_MASK;

   for (i = 0; i < m->num; i++)
     {
	if (m->items[i]->child)
	   has_s = 1;
	else
	   has_i = 1;
	m->items[i]->win = ECreateWindow(m->win, 0, 0, 1, 1, 0);
	XChangeWindowAttributes(disp, m->items[i]->win, CWEventMask, &att);
	EMapWindow(disp, m->items[i]->win);
	if ((m->style) && (m->style->tclass) && (m->items[i]->text))
	  {
	     TextSize(m->style->tclass, 0, 0, 0, m->items[i]->text, &w, &h, 17);
	     if (h > maxh)
		maxh = h;
	     if (w > maxx1)
		maxx1 = w;
	     m->items[i]->text_w = w;
	     m->items[i]->text_h = h;
	  }
	if (m->items[i]->icon_iclass)
	  {
	     im = ELoadImage(m->items[i]->icon_iclass->norm.normal->im_file);
	     if (im)
	       {
		  imlib_context_set_image(im);
		  m->items[i]->icon_win =
		     ECreateWindow(m->items[i]->win, 0, 0,
				   imlib_image_get_width(),
				   imlib_image_get_height(), 0);
		  EMapWindow(disp, m->items[i]->icon_win);
		  XChangeWindowAttributes(disp, m->items[i]->icon_win,
					  CWEventMask, &att);
		  m->items[i]->icon_w = imlib_image_get_width();
		  m->items[i]->icon_h = imlib_image_get_height();
		  if (imlib_image_get_height() > maxh)
		     maxh = imlib_image_get_height();
		  if (imlib_image_get_width() > maxx2)
		     maxx2 = imlib_image_get_width();
		  imlib_free_image();
	       }
	     else
		m->items[i]->icon_iclass = NULL;
	  }
     }
   if (((has_i) && (has_s)) || ((!has_i) && (!has_s)))
     {
	if (m->style->item_iclass->padding.top >
	    m->style->sub_iclass->padding.top)
	   maxh += m->style->item_iclass->padding.top;
	else
	   maxh += m->style->sub_iclass->padding.top;
	if (m->style->item_iclass->padding.bottom >
	    m->style->sub_iclass->padding.bottom)
	   maxh += m->style->item_iclass->padding.bottom;
	else
	   maxh += m->style->sub_iclass->padding.bottom;
	maxw = maxx1 + maxx2;
	if (m->style->item_iclass->padding.left >
	    m->style->sub_iclass->padding.left)
	   maxw += m->style->item_iclass->padding.left;
	else
	   maxw += m->style->sub_iclass->padding.left;
	if (m->style->item_iclass->padding.right >
	    m->style->sub_iclass->padding.right)
	   maxw += m->style->item_iclass->padding.right;
	else
	   maxw += m->style->sub_iclass->padding.right;
     }
   else if (has_i)
     {
	maxh += m->style->item_iclass->padding.top;
	maxh += m->style->item_iclass->padding.bottom;
	maxw = maxx1 + maxx2;
	maxw += m->style->item_iclass->padding.left;
	maxw += m->style->item_iclass->padding.right;
     }
   else if (has_s)
     {
	maxh += m->style->sub_iclass->padding.top;
	maxh += m->style->sub_iclass->padding.bottom;
	maxw = maxx1 + maxx2;
	maxw += m->style->sub_iclass->padding.left;
	maxw += m->style->sub_iclass->padding.right;
     }
   x = 0;
   y = 0;
   if ((m->style->bg_iclass) && (!m->style->use_item_bg))
     {
	x = m->style->bg_iclass->padding.left;
	y = m->style->bg_iclass->padding.top;
     }

   r = 0;
   mmw = 0;
   mmh = 0;
   pq = Mode.queue_up;
   Mode.queue_up = 0;

   for (i = 0; i < m->num; i++)
     {
	EMoveResizeWindow(disp, m->items[i]->win, x, y, maxw, maxh);
	if (m->style->iconpos == ICON_LEFT)
	  {
	     m->items[i]->text_x = m->style->item_iclass->padding.left + maxx2;
	     m->items[i]->text_w = maxx1;
	     m->items[i]->text_y = (maxh - m->items[i]->text_h) / 2;
	     if (m->items[i]->icon_win)
		EMoveWindow(disp, m->items[i]->icon_win,
			    m->style->item_iclass->padding.left +
			    ((maxx2 - m->items[i]->icon_w) / 2),
			    ((maxh - m->items[i]->icon_h) / 2));
	  }
	else
	  {
	     m->items[i]->text_x = m->style->item_iclass->padding.left;
	     m->items[i]->text_w = maxx1;
	     m->items[i]->text_y = (maxh - m->items[i]->text_h) / 2;
	     if (m->items[i]->icon_win)
		EMoveWindow(disp, m->items[i]->icon_win,
			    maxw - m->style->item_iclass->padding.right -
			    maxx2 + ((maxx2 - w) / 2), ((maxh - h) / 2));
	  }
	if (m->items[i]->icon_iclass)
	  {
	     iw = 0;
	     ih = 0;
	     GetWinWH(m->items[i]->icon_win, &iw, &ih);
	     IclassApply(m->items[i]->icon_iclass, m->items[i]->icon_win, iw,
			 ih, 0, 0, STATE_NORMAL, 0, ST_MENU_ITEM);
	  }
	if (x + maxw > mmw)
	   mmw = x + maxw;
	if (y + maxh > mmh)
	   mmh = y + maxh;
	if ((m->style->maxx) || (m->style->maxy))
	  {
	     if (m->style->maxy)
	       {
		  y += maxh;
		  r++;
		  if (r >= m->style->maxy)
		    {
		       r = 0;
		       x += maxw;
		       y = 0;
		    }
	       }
	     else
	       {
		  x += maxw;
		  r++;
		  if (r >= m->style->maxx)
		    {
		       r = 0;
		       y += maxh;
		       x = 0;
		    }
	       }
	  }
	else
	   y += maxh;
     }
   if ((m->style->bg_iclass) && (!m->style->use_item_bg))
     {
	mmw += m->style->bg_iclass->padding.right;
	mmh += m->style->bg_iclass->padding.bottom;
     }

   m->redraw = 1;
   EResizeWindow(disp, m->win, mmw, mmh);

   Mode.queue_up = pq;
   EDBUG_RETURN_;
}

static void
MenuRedraw(Menu * m)
{
   int                 i, j, w, h;

   if (m->redraw)
     {
	for (i = 0; i < m->num; i++)
	  {
	     for (j = 0; j < 3; j++)
		FreePmapMask(&(m->items[i]->pmm[j]));

	  }
	m->redraw = 0;
     }

   if (!m->style->use_item_bg)
     {
	GetWinWH(m->win, &w, &h);
	FreePmapMask(&m->pmm);
	IclassApplyCopy(m->style->bg_iclass, m->win, w, h, 0, 0,
			STATE_NORMAL, &m->pmm, 1, ST_MENU);
	ESetWindowBackgroundPixmap(disp, m->win, m->pmm.pmap);
	EShapeCombineMask(disp, m->win, ShapeBounding, 0, 0, m->pmm.mask,
			  ShapeSet);
	for (i = 0; i < m->num; i++)
	   MenuDrawItem(m, m->items[i], 0);
     }
   else
     {
	for (i = 0; i < m->num; i++)
	   MenuDrawItem(m, m->items[i], 0);
	PropagateShapes(m->win);
     }
}

static void
MenuDrawItem(Menu * m, MenuItem * mi, char shape)
{
   PmapMask           *mi_pmm;
   char                pq;

   EDBUG(5, "MenuDrawItem");
   pq = Mode.queue_up;
   Mode.queue_up = 0;

   mi_pmm = &(mi->pmm[(int)(mi->state)]);

   if (!mi_pmm->pmap)
     {
	GC                  gc;
	XGCValues           gcv;
	unsigned int        w, h;
	int                 x, y;
	int                 item_type;
	ImageClass         *ic;

	GetWinWH(mi->win, &w, &h);
	GetWinXY(mi->win, &x, &y);

	mi_pmm->type = 0;
	mi_pmm->pmap = ECreatePixmap(disp, mi->win, w, h, VRoot.depth);
	mi_pmm->mask = None;

	ic = (mi->child) ? m->style->sub_iclass : m->style->item_iclass;
	item_type = (mi->state != STATE_NORMAL) ? ST_MENU_ITEM : ST_MENU;

	if (!m->style->use_item_bg)
	  {
	     gc = XCreateGC(disp, m->pmm.pmap, 0, &gcv);
	     XCopyArea(disp, m->pmm.pmap, mi_pmm->pmap, gc, x, y, w, h, 0, 0);
	     if ((mi->state != STATE_NORMAL) || (mi->child))
	       {
		  PmapMask            pmm;

		  IclassApplyCopy(ic, mi->win, w, h, 0, 0, mi->state, &pmm, 1,
				  item_type);
		  if (pmm.mask)
		    {
		       XSetClipMask(disp, gc, pmm.mask);
		       XSetClipOrigin(disp, gc, 0, 0);
		    }
		  XCopyArea(disp, pmm.pmap, mi_pmm->pmap, gc, 0, 0, w, h, 0, 0);
		  FreePmapMask(&pmm);
	       }
	     XFreeGC(disp, gc);
	  }
	else
	  {
	     IclassApplyCopy(ic, mi_pmm->pmap, w, h, 0, 0, mi->state, mi_pmm, 1,
			     item_type);
	  }

	if (mi->text)
	  {
	     TextDraw(m->style->tclass, mi_pmm->pmap, 0, 0, mi->state,
		      mi->text, mi->text_x, mi->text_y, mi->text_w, mi->text_h,
		      17, m->style->tclass->justification);
	  }
     }

   ESetWindowBackgroundPixmap(disp, mi->win, mi_pmm->pmap);
   EShapeCombineMask(disp, mi->win, ShapeBounding, 0, 0, mi_pmm->mask,
		     ShapeSet);
   XClearWindow(disp, mi->win);

   if ((shape) && (m->style->use_item_bg))
      PropagateShapes(m->win);

   if (mi->state == STATE_HILITED)
     {
	active_item = mi;
	if (active_menu != m)
	  {
	     active_menu = m;
	     UngrabKeyboard();
	     GrabKeyboard(m->win);
	  }
     }

   Mode.queue_up = pq;
   EDBUG_RETURN_;
}

Menu               *
MenuCreateFromDirectory(const char *name, MenuStyle * ms, const char *dir)
{
   Progressbar        *p = NULL;
   Menu               *m, *mm;
   int                 i, num;
   char              **list, s[4096], ss[4096], cs[4096];
   const char         *ext;
   MenuItem           *mi;
   struct stat         st;
   const char         *chmap =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
   FILE               *f;

   EDBUG(5, "MenuCreateFromDirectory");
   m = MenuCreate(name);
   m->style = ms;
   if (stat(dir, &st) >= 0)
     {
	int                 aa, bb, cc;

	aa = (int)st.st_ino;
	bb = (int)st.st_dev;
	cc = 0;
	if (st.st_mtime > st.st_ctime)
	   cc = st.st_mtime;
	else
	   cc = st.st_ctime;
	Esnprintf(cs, sizeof(cs),
		  "%s/cached/img/.%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		  EDirUserCache(), chmap[(aa >> 0) & 0x3f],
		  chmap[(aa >> 6) & 0x3f], chmap[(aa >> 12) & 0x3f],
		  chmap[(aa >> 18) & 0x3f], chmap[(aa >> 24) & 0x3f],
		  chmap[(aa >> 28) & 0x3f], chmap[(bb >> 0) & 0x3f],
		  chmap[(bb >> 6) & 0x3f], chmap[(bb >> 12) & 0x3f],
		  chmap[(bb >> 18) & 0x3f], chmap[(bb >> 24) & 0x3f],
		  chmap[(bb >> 28) & 0x3f], chmap[(cc >> 0) & 0x3f],
		  chmap[(cc >> 6) & 0x3f], chmap[(cc >> 12) & 0x3f],
		  chmap[(cc >> 18) & 0x3f], chmap[(cc >> 24) & 0x3f],
		  chmap[(cc >> 28) & 0x3f]);
	/* cached dir listing - use it */
	if (exists(cs))
	  {
	     f = fopen(cs, "r");
	     while (fgets(s, sizeof(s), f))
	       {
		  s[strlen(s) - 1] = 0;
		  word(s, 1, ss);
		  if (!strcmp(ss, "BG"))
		    {
		       Background         *bg;
		       char                ok = 1;
		       char                s2[4096], s3[512];

		       word(s, 3, s3);
		       bg = (Background *) FindItem(s3, 0, LIST_FINDBY_NAME,
						    LIST_TYPE_BACKGROUND);
		       if (!bg)
			 {
			    Imlib_Image        *im;

			    word(s, 2, s2);
			    Esnprintf(ss, sizeof(ss), "%s/%s", dir, s2);
			    im = imlib_load_image(ss);
			    if (im)
			      {
				 Imlib_Image        *im2;
				 XColor              xclr;
				 char                tile = 1, keep_asp = 0;
				 int                 width, height;
				 int                 scalex = 0, scaley = 0;
				 int                 scr_asp, im_asp;
				 int                 w2, h2;
				 int                 maxw = 48, maxh = 48;
				 int                 justx = 512, justy = 512;

				 Esnprintf(s2, sizeof(s2), "%s/cached/img/%s",
					   EDirUserCache(), s3);
				 imlib_context_set_image(im);
				 width = imlib_image_get_width();
				 height = imlib_image_get_height();
				 h2 = maxh;
				 w2 =
				    (imlib_image_get_width() * h2) /
				    imlib_image_get_height();
				 if (w2 > maxw)
				   {
				      w2 = maxw;
				      h2 =
					 (imlib_image_get_height() * w2) /
					 imlib_image_get_width();
				   }
				 im2 = imlib_create_cropped_scaled_image(0, 0,
									 imlib_image_get_width
									 (),
									 imlib_image_get_height
									 (), w2,
									 h2);
				 imlib_free_image_and_decache();
				 imlib_context_set_image(im2);
				 imlib_image_set_format("ppm");
				 imlib_save_image(s2);
				 imlib_free_image_and_decache();

				 scr_asp = (VRoot.w << 16) / VRoot.h;
				 im_asp = (width << 16) / height;
				 if (width == height)
				   {
				      justx = 0;
				      justy = 0;
				      scalex = 0;
				      scaley = 0;
				      tile = 1;
				      keep_asp = 0;
				   }
				 else if ((!(IN_RANGE(scr_asp, im_asp, 16000)))
					  && ((width < 480) && (height < 360)))
				   {
				      justx = 0;
				      justy = 0;
				      scalex = 0;
				      scaley = 0;
				      tile = 1;
				      keep_asp = 0;
				   }
				 else if (IN_RANGE(scr_asp, im_asp, 16000))
				   {
				      justx = 0;
				      justy = 0;
				      scalex = 1024;
				      scaley = 1024;
				      tile = 0;
				      keep_asp = 0;
				   }
				 else if (im_asp > scr_asp)
				   {
				      justx = 512;
				      justy = 512;
				      scalex = 1024;
				      scaley = 0;
				      tile = 0;
				      keep_asp = 1;
				   }
				 else
				   {
				      justx = 512;
				      justy = 512;
				      scalex = 0;
				      scaley = 1024;
				      tile = 0;
				      keep_asp = 1;
				   }
				 ESetColor(&xclr, 0, 0, 0);
				 bg = BackgroundCreate(s3, &xclr, ss, tile,
						       keep_asp, justx, justy,
						       scalex, scaley, NULL, 0,
						       0, 0, 0, 0);
			      }
			    else
			       ok = 0;
			 }
		       if (ok)
			 {
			    ImageClass         *ic = NULL;
			    char                stmp[4096];

			    ic = CreateIclass();
			    ic->name = Estrdup("`");
			    ic->norm.normal = CreateImageState();
			    Esnprintf(stmp, sizeof(stmp), "%s/cached/img/%s",
				      EDirUserCache(), s3);
			    ic->norm.normal->im_file = Estrdup(stmp);
			    ic->norm.normal->unloadable = 1;
			    IclassPopulate(ic);
			    AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);
			    mi = MenuItemCreate(NULL, ic, ACTION_BACKGROUND_SET,
						s3, NULL);
			    MenuAddItem(m, mi);
			 }
		    }
		  else if (!strcmp(ss, "EXE"))
		    {
		       word(s, 2, ss);
		       Esnprintf(s, sizeof(s), "%s/%s", dir, ss);
		       mi = MenuItemCreate(NULL, NULL, ACTION_EXEC, s, NULL);
		       MenuAddItem(m, mi);
		    }
		  else if (!strcmp(ss, "DIR"))
		    {
		       char                tmp[4096];

		       word(s, 2, tmp);
		       Esnprintf(s, sizeof(s), "%s/%s:%s", dir, tmp, name);
		       Esnprintf(ss, sizeof(ss), "%s/%s", dir, tmp);
		       mm = MenuCreateFromDirectory(s, ms, ss);
		       mm->parent = m;
		       mi = MenuItemCreate(tmp, NULL, 0, NULL, mm);
		       MenuAddItem(m, mi);
		    }
	       }
	     fclose(f);
	     EDBUG_RETURN(m);
	  }
     }
   list = E_ls(dir, &num);
   Esnprintf(s, sizeof(s), "Scanning %s", dir);
   if (!init_win_ext)
      p = CreateProgressbar(s, 600, 16);
   if (p)
      ShowProgressbar(p);
   f = fopen(cs, "w");
   for (i = 0; i < num; i++)
     {
	if (p)
	   SetProgressbar(p, (i * 100) / num);
	Esnprintf(ss, sizeof(ss), "%s/%s", dir, list[i]);
	/* skip "dot" files and dirs - senisble */
	if ((*(list[i]) != '.') && (stat(ss, &st) >= 0))
	  {
	     ext = FileExtension(ss);
	     if (S_ISDIR(st.st_mode))
	       {
		  Esnprintf(s, sizeof(s), "%s/%s:%s", dir, list[i], name);
		  Esnprintf(ss, sizeof(ss), "%s/%s", dir, list[i]);
		  mm = MenuCreateFromDirectory(s, ms, ss);
		  mm->parent = m;
		  mi = MenuItemCreate(list[i], NULL, 0, NULL, mm);
		  MenuAddItem(m, mi);
		  if (f)
		     fprintf(f, "DIR %s\n", list[i]);
	       }
/* that's it - people are stupid and have executable images and just */
/* don't get it - so I'm disablign this to save people from their own */
/* stupidity */
/*           else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
 * {
 * mi = MenuItemCreate(list[i], NULL, ACTION_EXEC, ss, NULL);
 * MenuAddItem(m, mi);
 * if (f)
 * fprintf(f, "EXE %s\n", list[i]);
 * }
 */
	     else if ((!strcmp(ext, "jpg")) || (!strcmp(ext, "JPG"))
		      || (!strcmp(ext, "jpeg")) || (!strcmp(ext, "Jpeg"))
		      || (!strcmp(ext, "JPEG")) || (!strcmp(ext, "Jpg"))
		      || (!strcmp(ext, "gif")) || (!strcmp(ext, "Gif"))
		      || (!strcmp(ext, "GIF")) || (!strcmp(ext, "png"))
		      || (!strcmp(ext, "Png")) || (!strcmp(ext, "PNG"))
		      || (!strcmp(ext, "tif")) || (!strcmp(ext, "Tif"))
		      || (!strcmp(ext, "TIFF")) || (!strcmp(ext, "tiff"))
		      || (!strcmp(ext, "Tiff")) || (!strcmp(ext, "TIFF"))
		      || (!strcmp(ext, "xpm")) || (!strcmp(ext, "Xpm"))
		      || (!strcmp(ext, "XPM")) || (!strcmp(ext, "ppm"))
		      || (!strcmp(ext, "PPM")) || (!strcmp(ext, "pgm"))
		      || (!strcmp(ext, "PGM")) || (!strcmp(ext, "pnm"))
		      || (!strcmp(ext, "PNM")) || (!strcmp(ext, "bmp"))
		      || (!strcmp(ext, "Bmp")) || (!strcmp(ext, "BMP")))
	       {
		  Background         *bg;
		  char                ok = 1;
		  char                s2[4096], s3[512];
		  int                 aa, bb, cc;

		  aa = (int)st.st_ino;
		  bb = (int)st.st_dev;
		  cc = 0;
		  if (st.st_mtime > st.st_ctime)
		     cc = st.st_mtime;
		  else
		     cc = st.st_ctime;
		  Esnprintf(s3, sizeof(s3),
			    ".%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
			    chmap[(aa >> 0) & 0x3f], chmap[(aa >> 6) & 0x3f],
			    chmap[(aa >> 12) & 0x3f], chmap[(aa >> 18) & 0x3f],
			    chmap[(aa >> 24) & 0x3f], chmap[(aa >> 28) & 0x3f],
			    chmap[(bb >> 0) & 0x3f], chmap[(bb >> 6) & 0x3f],
			    chmap[(bb >> 12) & 0x3f], chmap[(bb >> 18) & 0x3f],
			    chmap[(bb >> 24) & 0x3f], chmap[(bb >> 28) & 0x3f],
			    chmap[(cc >> 0) & 0x3f], chmap[(cc >> 6) & 0x3f],
			    chmap[(cc >> 12) & 0x3f], chmap[(cc >> 18) & 0x3f],
			    chmap[(cc >> 24) & 0x3f], chmap[(cc >> 28) & 0x3f]);
		  bg = (Background *) FindItem(s3, 0, LIST_FINDBY_NAME,
					       LIST_TYPE_BACKGROUND);
		  if (!bg)
		    {
		       Imlib_Image        *im;

		       im = imlib_load_image(ss);
		       if (im)
			 {
			    Imlib_Image        *im2;
			    XColor              xclr;
			    char                tile = 1, keep_asp = 0;
			    int                 width, height, scalex =
			       0, scaley = 0;
			    int                 scr_asp, im_asp, w2, h2;
			    int                 maxw = 48, maxh = 48;

			    Esnprintf(s2, sizeof(s2), "%s/cached/img/%s",
				      EDirUserCache(), s3);
			    imlib_context_set_image(im);
			    width = imlib_image_get_width();
			    height = imlib_image_get_height();
			    h2 = maxh;
			    w2 =
			       (imlib_image_get_width() * h2) /
			       imlib_image_get_height();
			    if (w2 > maxw)
			      {
				 w2 = maxw;
				 h2 =
				    (imlib_image_get_height() * w2) /
				    imlib_image_get_width();
			      }
			    im2 = imlib_create_cropped_scaled_image(0, 0,
								    imlib_image_get_width
								    (),
								    imlib_image_get_height
								    (), w2, h2);
			    imlib_free_image_and_decache();
			    imlib_context_set_image(im2);
			    imlib_image_set_format("ppm");
			    imlib_save_image(s2);
			    imlib_free_image_and_decache();

			    scr_asp = (VRoot.w << 16) / VRoot.h;
			    im_asp = (width << 16) / height;
			    if (width == height)
			      {
				 scalex = 0;
				 scaley = 0;
				 tile = 1;
				 keep_asp = 0;
			      }
			    else if ((!(IN_RANGE(scr_asp, im_asp, 16000)))
				     && ((width < 480) && (height < 360)))
			      {
				 scalex = 0;
				 scaley = 0;
				 tile = 1;
				 keep_asp = 0;
			      }
			    else if (IN_RANGE(scr_asp, im_asp, 16000))
			      {
				 scalex = 1024;
				 scaley = 1024;
				 tile = 0;
				 keep_asp = 0;
			      }
			    else if (im_asp > scr_asp)
			      {
				 scalex = 1024;
				 scaley = 0;
				 tile = 0;
				 keep_asp = 1;
			      }
			    else
			      {
				 scalex = 0;
				 scaley = 1024;
				 tile = 0;
				 keep_asp = 1;
			      }
			    ESetColor(&xclr, 0, 0, 0);
			    bg = BackgroundCreate(s3, &xclr, ss, tile, keep_asp,
						  512, 512, scalex, scaley,
						  NULL, 0, 0, 0, 0, 0);
			 }
		       else
			  ok = 0;
		    }
		  if (ok)
		    {
		       ImageClass         *ic = NULL;
		       char                stmp[4096];

		       ic = CreateIclass();
		       ic->name = Estrdup("`");
		       ic->norm.normal = CreateImageState();
		       Esnprintf(stmp, sizeof(stmp), "%s/cached/img/%s",
				 EDirUserCache(), s3);
		       ic->norm.normal->im_file = Estrdup(stmp);
		       ic->norm.normal->unloadable = 1;
		       IclassPopulate(ic);
		       AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);
		       mi = MenuItemCreate(NULL, ic, ACTION_BACKGROUND_SET, s3,
					   NULL);
		       MenuAddItem(m, mi);
		    }
		  if (f)
		     fprintf(f, "BG %s %s\n", list[i], s3);
	       }
	  }
     }
   if (f)
      fclose(f);
   if (p)
      FreeProgressbar(p);
   if (list)
      freestrlist(list, num);
   EDBUG_RETURN(m);
}

Menu               *
MenuCreateFromFlatFile(const char *name, MenuStyle * ms, const char *file,
		       Menu * parent)
{
   Menu               *m;
   char                s[4096], *ff = NULL;
   static int          calls = 0;

   EDBUG(5, "MenuCreateFromFlatFile");
   if (calls > 255)
      EDBUG_RETURN(NULL);

   ff = FindFile(file);
   if (!ff)
      EDBUG_RETURN(NULL);

   if (canread(ff))
     {
	m = MenuCreate(name);
	m->style = ms;
	m->last_change = moddate(ff);
	if (parent)
	   FillFlatFileMenu(m, m->style, m->name, ff, parent);
	else
	   FillFlatFileMenu(m, m->style, m->name, ff, m);
	m->data = ff;
	m->ref_menu = parent;
	Esnprintf(s, sizeof(s), "__.%s", m->name);
	DoIn(s, 2.0, FileMenuUpdate, 0, m);
	calls--;
	EDBUG_RETURN(m);
     }
   Efree(ff);

   calls--;
   EDBUG_RETURN(NULL);
}

static void
FillFlatFileMenu(Menu * m, MenuStyle * ms, char *name, char *file,
		 Menu * parent)
{
   FILE               *f;
   char                first = 1;
   char                s[4096];

   f = fopen(file, "r");
   if (!f)
     {
	fprintf(stderr, "Unable to open menu file %s -- %s\n", file,
		strerror(errno));
	return;
     }

   while (fgets(s, 4096, f))
     {
	s[strlen(s) - 1] = 0;
	if ((s[0]) && s[0] != '#')
	  {
	     if (first)
	       {
		  char               *wd;

		  wd = field(s, 0);
		  if (wd)
		    {
		       MenuAddTitle(m, wd);
		       Efree(wd);
		    }
		  first = 0;
	       }
	     else
	       {
		  char               *txt = NULL, *icon = NULL, *act = NULL;
		  char               *params = NULL, *tmp = NULL, wd[4096];

		  MenuItem           *mi;
		  ImageClass         *icc = NULL;
		  Menu               *mm;
		  static int          count = 0;

		  txt = field(s, 0);
		  icon = field(s, 1);
		  act = field(s, 2);
		  params = field(s, 3);
		  tmp = NULL;
		  if (icon)
		    {
		       Esnprintf(wd, sizeof(wd), "__FM.%s", icon);
		       icc =
			  FindItem(wd, 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
		       if (!icc)
			 {
			    icc = CreateIclass();
			    icc->name = Estrdup(wd);
			    icc->norm.normal = CreateImageState();
			    icc->norm.normal->im_file = icon;
			    IclassPopulate(icc);
			    AddItem(icc, icc->name, 0, LIST_TYPE_ICLASS);
			 }
		       else
			 {
			    Efree(icon);
			 }
		    }
		  if ((act) && (!strcmp(act, "exec")) && (params))
		    {
		       word(params, 1, wd);
		       tmp = pathtoexec(wd);
		       if (tmp)
			 {
			    Efree(tmp);

			    mi = MenuItemCreate(txt, icc, ACTION_EXEC, params,
						NULL);
			    MenuAddItem(m, mi);
			 }
		    }
		  else if ((act) && (!strcmp(act, "menu")) && (params))
		    {
		       Esnprintf(wd, sizeof(wd), "__FM.%s.%i", name, count);
		       count++;
		       mm = MenuCreateFromFlatFile(wd, ms, params, parent);
		       if (mm)
			 {
			    mm->parent = m;
			    mi = MenuItemCreate(txt, icc, 0, NULL, mm);
			    MenuAddItem(m, mi);
			 }
		    }
		  else
		    {
		       mi = MenuItemCreate(txt, icc, 0, NULL, NULL);
		       MenuAddItem(m, mi);
		    }
		  if (txt)
		     Efree(txt);
		  if (act)
		     Efree(act);
		  if (params)
		     Efree(params);
	       }
	  }
     }
   fclose(f);
}

#if 0				/* We travelled up the tree. Why? Leaving this around for now. */
static void
FileMenuUpdate(int val, void *data)
{
   Menu               *m, *mm;
   time_t              lastmod = 0;
   char                s[4096];

   m = (Menu *) data;
   if (!m)
      return;
   if (!FindItem((char *)m, m->win, LIST_FINDBY_POINTER, LIST_TYPE_MENU))
      return;
   /* if the menu is up dont update */
   if (((Mode.cur_menu_mode) || (clickmenu)) && (Mode.cur_menu_depth > 0))
     {
	Esnprintf(s, sizeof(s), "__.%s", m->name);
	DoIn(s, 2.0, FileMenuUpdate, 0, m);
	return;
     }
   mm = m;
   if (m->ref_menu)
      mm = m->ref_menu;
   if (!exists(m->data))
     {
	MenuHide(m);
	MenuEmpty(m);
	return;
     }
   if (m->data)
      lastmod = moddate(m->data);
   if (lastmod > m->last_change)
     {
	m->last_change = lastmod;
	if (m == mm)
	  {
	     Esnprintf(s, sizeof(s), "__.%s", m->name);
	     DoIn(s, 2.0, FileMenuUpdate, 0, m);
	  }
	MenuEmpty(mm);
	FillFlatFileMenu(mm, mm->style, mm->name, mm->data, mm);
	MenuRepack(mm);
	return;
     }
   Esnprintf(s, sizeof(s), "__.%s", m->name);
   DoIn(s, 2.0, FileMenuUpdate, 0, m);
   val = 0;
}
#else

static void
FileMenuUpdate(int val __UNUSED__, void *data)
{
   Menu               *m;
   time_t              lastmod = 0;
   char                s[4096];

   m = (Menu *) data;
   if (!m)
      return;

   if (!FindItem((char *)m, m->win, LIST_FINDBY_POINTER, LIST_TYPE_MENU))
      return;

   /* if the menu is up dont update */
   if (((Mode.cur_menu_mode) || (clickmenu)) && (Mode.cur_menu_depth > 0))
      goto done;

   if (!exists(m->data))
     {
	MenuHide(m);
	MenuEmpty(m);
	return;
     }

   if (m->data)
      lastmod = moddate(m->data);
   if (lastmod > m->last_change)
     {
	m->last_change = lastmod;
	MenuEmpty(m);
	FillFlatFileMenu(m, m->style, m->name, m->data, m);
	MenuRepack(m);
     }

 done:
   Esnprintf(s, sizeof(s), "__.%s", m->name);
   DoIn(s, 5.0, FileMenuUpdate, 0, m);
}
#endif

Menu               *
MenuCreateFromGnome(const char *name, MenuStyle * ms, const char *dir)
{
   Menu               *m, *mm;
   int                 i, num;
   char              **list, s[4096], ss[4096];

   MenuItem           *mi;
   FILE               *f;
   char               *lang, name_buf[20];

   EDBUG(5, "MenuCreateFromGnome");

   if ((lang = setlocale(LC_MESSAGES, NULL)) != NULL)
      Esnprintf(name_buf, sizeof(name_buf), "Name[%s]=", lang);
   else
      name_buf[0] = '\0';

   m = MenuCreate(name);
   m->style = ms;
   list = E_ls(dir, &num);
   for (i = 0; i < num; i++)
     {
	if ((strcmp(list[i], ".")) && (strcmp(list[i], "..")))
	  {
	     Esnprintf(ss, sizeof(ss), "%s/%s", dir, list[i]);
	     if (isdir(ss))
	       {
		  Esnprintf(s, sizeof(s), "%s/%s:%s", dir, list[i], name);
		  mm = MenuCreateFromGnome(s, ms, ss);
		  mm->parent = m;
		  name = list[i];
		  if (name_buf[0])
		    {
		       Esnprintf(s, sizeof(s), "%s/.directory", ss);
		       if ((f = fopen(s, "r")) != NULL)
			 {
			    while (fgets(s, sizeof(s), f))
			      {
				 if (!strncmp(s, name_buf, strlen(name_buf)))
				   {
				      if (s[strlen(s) - 1] == '\n')
					 s[strlen(s) - 1] = 0;
				      name = &(s[strlen(name_buf)]);
				      break;
				   }
			      }
			    fclose(f);
			 }
		    }
		  mi = MenuItemCreate(name, NULL, 0, NULL, mm);
		  MenuAddItem(m, mi);
	       }
	     else
	       {
		  f = fopen(ss, "r");
		  if (f)
		    {
		       char               *iname = NULL, *exec = NULL, *texec =
			  NULL, *tmp;
		       char               *en_name = NULL;

		       while (fgets(s, sizeof(s), f))
			 {
			    if (s[strlen(s) - 1] == '\n')
			       s[strlen(s) - 1] = 0;
			    if (!strncmp(s, "Name=", strlen("Name=")))
			       en_name = Estrdup(&(s[strlen("Name=")]));
			    else if (name_buf[0]
				     && !strncmp(s, name_buf, strlen(name_buf)))
			       iname = Estrdup(&(s[strlen(name_buf)]));
			    else if (!strncmp
				     (s, "TryExec=", strlen("TryExec=")))
			       texec = Estrdup(&(s[strlen("TryExec=")]));
			    else if (!strncmp(s, "Exec=", strlen("Exec=")))
			       exec = Estrdup(&(s[strlen("Exec=")]));
			 }
		       if (iname)
			 {
			    if (en_name)
			       Efree(en_name);
			 }
		       else
			 {
			    if (en_name)
			       iname = en_name;
			 }
		       fclose(f);
		       if ((iname) && (exec))
			 {
			    tmp = NULL;
			    if (texec)
			       tmp = pathtoexec(texec);
			    if ((tmp) || (!texec))
			      {
				 if (tmp)
				    Efree(tmp);

				 mi = MenuItemCreate(iname, NULL, ACTION_EXEC,
						     exec, NULL);
				 MenuAddItem(m, mi);
			      }
			 }
		       if (iname)
			  Efree(iname);
		       if (exec)
			  Efree(exec);
		       if (texec)
			  Efree(texec);
		    }
	       }
	  }
     }
   if (list)
      freestrlist(list, num);
   EDBUG_RETURN(m);
}

Menu               *
MenuCreateFromThemes(const char *name, MenuStyle * ms)
{
   Menu               *m;
   char              **lst;
   int                 i, num;
   char                ss[4096], *s;

   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromThemes");

   m = MenuCreate(name);
   m->style = ms;
   lst = ListThemes(&num);
   for (i = 0; i < num; i++)
     {
	s = fullfileof(lst[i]);
	Esnprintf(ss, sizeof(ss), "restart_theme %s", s);
	mi = MenuItemCreate(s, NULL, ACTION_EXIT, ss, NULL);
	Efree(s);
	MenuAddItem(m, mi);
     }
   if (lst)
      freestrlist(lst, i);

   EDBUG_RETURN(m);
}

static int
BorderNameCompare(void *b1, void *b2)
{
   if (b1 && b2)
      return strcmp(((Border *) b1)->name, ((Border *) b2)->name);

   return 0;
}

Menu               *
MenuCreateFromBorders(const char *name, MenuStyle * ms)
{
   Menu               *m;
   Border            **lst;
   int                 i, num;

   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromBorders");

   m = MenuCreate(name);
   m->style = ms;
   lst = (Border **) ListItemType(&num, LIST_TYPE_BORDER);
   if (lst)
      Quicksort((void **)lst, 0, num - 1, BorderNameCompare);
   for (i = 0; i < num; i++)
     {
	/* if its not internal (ie doesnt start with _ ) */
	if (lst[i]->name[0] != '_')
	  {
	     mi = MenuItemCreate(lst[i]->name, NULL, ACTION_SET_WINDOW_BORDER,
				 lst[i]->name, NULL);
	     MenuAddItem(m, mi);
	  }
     }
   if (lst)
      Efree(lst);
   EDBUG_RETURN(m);
}

Menu               *
MenuCreateFromAllEWins(const char *name, MenuStyle * ms)
{
   Menu               *m;
   EWin               *const *lst;
   int                 i, num;
   char                s[256];

   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromEWins");
   m = MenuCreate(name);
   m->style = ms;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (lst[i]->skipwinlist || !EwinGetTitle(lst[i]))
	   continue;

	Esnprintf(s, sizeof(s), "%lu", lst[i]->client.win);
	mi = MenuItemCreate(EwinGetTitle(lst[i]), NULL,
			    ACTION_FOCUS_SET, s, NULL);
	MenuAddItem(m, mi);
     }

   EDBUG_RETURN(m);
}

#if 0				/* Not used */
static Menu        *
MenuCreateFromDesktopEWins(char *name, MenuStyle * ms, int desk)
{
   Menu               *m;
   EWin               *const *lst;
   int                 i, num;
   char                s[256];

   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromDesktopEWins");
   m = MenuCreate(name);
   m->style = ms;

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	if (lst[i]->skipwinlist || !EwinGetTitle(lst[i]) ||
	    EwinGetDesk(lst[i]) != j)
	   continue;

	Esnprintf(s, sizeof(s), "%lu", lst[i]->client.win);
	mi = MenuItemCreate(lst[i]->client.title, NULL,
			    ACTION_FOCUS_SET, s, NULL);
	MenuAddItem(m, mi);
     }

   EDBUG_RETURN(m);
   desk = 0;
}
#endif

Menu               *
MenuCreateFromDesktops(const char *name, MenuStyle * ms)
{
   Menu               *m, *mm;
   EWin               *const *lst;
   int                 j, i, num;
   char                s[256];
   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromDesktops");

   m = MenuCreate(name);
   m->style = ms;
   lst = EwinListGetAll(&num);
   for (j = 0; j < Conf.desks.num; j++)
     {
	mm = MenuCreate("__SUBMENUDESK_E");
	mm->style = ms;
	Esnprintf(s, sizeof(s), "%i", j);
	mi = MenuItemCreate(_("Go to this Desktop"), NULL, ACTION_GOTO_DESK, s,
			    NULL);
	MenuAddItem(mm, mi);
	for (i = 0; i < num; i++)
	  {
	     if (lst[i]->skipwinlist || !EwinGetTitle(lst[i]) ||
		 EwinGetDesk(lst[i]) != j)
		continue;

	     Esnprintf(s, sizeof(s), "%lu", lst[i]->client.win);
	     mi = MenuItemCreate(EwinGetTitle(lst[i]), NULL,
				 ACTION_FOCUS_SET, s, NULL);
	     MenuAddItem(mm, mi);
	  }
	mm->parent = m;
	Esnprintf(s, sizeof(s), _("Desktop %i"), j);
	mi = MenuItemCreate(s, NULL, 0, NULL, mm);
	MenuAddItem(m, mi);
     }

   EDBUG_RETURN(m);
}

#if 0				/* Not finished */
Menu               *
MenuCreateMoveToDesktop(char *name, MenuStyle * ms)
{
   Menu               *m;
   int                 i;
   char                s1[256], s2[256];

   MenuItem           *mi;

   EDBUG(5, "MenuCreateDesktops");
   m = MenuCreate(name);
   m->style = ms;
   for (i = 0; i < Mode.numdesktops; i++)
     {
	Esnprintf(s1, sizeof(s1), _("Desktop %i"), i);
	Esnprintf(s2, sizeof(s2), "%i", i);
	mi = MenuItemCreate(s1, NULL, ACTION_MOVE_TO_DESK, s2, NULL);
	MenuAddItem(m, mi);
     }
   EDBUG_RETURN(m);
}
#endif

static Menu        *
MenuCreateFromGroups(const char *name, MenuStyle * ms)
{
   Menu               *m, *mm;
   Group             **lst;
   int                 i, j, num;
   char                s[256];

   MenuItem           *mi;

   EDBUG(5, "MenuCreateFromEWins");
   m = MenuCreate(name);
   m->style = ms;
   lst = (Group **) ListItemType(&num, LIST_TYPE_GROUP);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     mm = MenuCreate("__SUBMENUGROUP_E");
	     mm->style = ms;
	     Esnprintf(s, sizeof(s), "%li", lst[i]->members[0]->client.win);
	     mi = MenuItemCreate(_("Show/Hide this group"), NULL,
				 ACTION_SHOW_HIDE_GROUP, s, NULL);
	     MenuAddItem(mm, mi);
	     mi = MenuItemCreate(_("Iconify this group"), NULL, ACTION_ICONIFY,
				 s, NULL);
	     MenuAddItem(mm, mi);

	     for (j = 0; j < lst[i]->num_members; j++)
	       {
		  Esnprintf(s, sizeof(s), "%li",
			    lst[i]->members[j]->client.win);
		  mi =
		     MenuItemCreate(EwinGetTitle(lst[i]->members[j]), NULL,
				    ACTION_FOCUS_SET, s, NULL);
		  MenuAddItem(mm, mi);
	       }
	     mm->parent = m;
	     Esnprintf(s, sizeof(s), _("Group %i"), i);
	     mi = MenuItemCreate(s, NULL, 0, NULL, mm);
	     MenuAddItem(m, mi);
	  }
	Efree(lst);
     }
   EDBUG_RETURN(m);
}

void
MenuShowMasker(Menu * m)
{
   EWin               *ewin;

   ewin = FindEwinByMenu(m);
   if ((ewin) && (!Mode.menu_cover_win))
     {
	Window              parent;
	Window              wl[2];

	parent = desks.desk[ewin->desktop].win;
	Mode.menu_cover_win =
	   ECreateEventWindow(parent, 0, 0, VRoot.w, VRoot.h);
	Mode.menu_win_covered = ewin->win;
	wl[0] = Mode.menu_win_covered;
	wl[1] = Mode.menu_cover_win;
	XSelectInput(disp, Mode.menu_cover_win,
		     ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
		     LeaveWindowMask);
	XRestackWindows(disp, wl, 2);
	EMapWindow(disp, Mode.menu_cover_win);
     }
}

void
MenuHideMasker(void)
{
   if (Mode.menu_cover_win)
     {
	EDestroyWindow(disp, Mode.menu_cover_win);
	Mode.menu_cover_win = 0;
	Mode.menu_win_covered = 0;
     }
}

void
ShowNamedMenu(const char *name)
{
   Menu               *m;

   EDBUG(5, "ShowNamedMenu");

   m = FindItem(name, 0, LIST_FINDBY_NAME, LIST_TYPE_MENU);
   if (m)
     {
	Mode.cur_menu_mode = 1;
	XUngrabPointer(disp, CurrentTime);
	if (!FindEwinByMenu(m))	/* Don't show if already shown */
	   MenuShow(m, 0);
	Mode.cur_menu[0] = m;
	Mode.cur_menu_depth = 1;
	MenuShowMasker(m);
	m->ref_count++;
     }
   else
     {
	Mode.cur_menu[0] = NULL;
	Mode.cur_menu_depth = 0;
	MenuHideMasker();
     }

   EDBUG_RETURN_;
}

void
MenusDestroyLoaded(void)
{
   Menu               *menu;
   Menu              **menus;
   int                 i, num, found_one;

   /* Free all menustyles first (gulp) */
   do
     {
	found_one = 0;
	menus = (Menu **) ListItemType(&num, LIST_TYPE_MENU);
	for (i = 0; i < num; i++)
	  {
	     menu = menus[i];
	     if (menu->internal)
		continue;

	     MenuDestroy(menu);
	     /* Destroying a menu may result in sub-menus being
	      * destroyed too, so we have to re-find all menus
	      * afterwards. Inefficient yes, but it works...
	      */
	     found_one = 1;
	     break;
	  }
	if (menus)
	   Efree(menus);
     }
   while (found_one);
}

void
MenusHideByWindow(Window win)
{
   Menu               *m;
   int                 i, ok;

   m = FindMenu(win);
   if (m)
     {
	MenuHide(m);
	ok = 0;
	for (i = 0; i < Mode.cur_menu_depth; i++)
	  {
	     if (ok)
		MenuHide(Mode.cur_menu[i]);
	     if (Mode.cur_menu[i] == m)
		ok = 1;
	  }
	MenuHideMasker();
     }
}

/*
 * Internal menus
 */

#if 0				/* Not used */
static Menu        *task_menu[ENLIGHTENMENT_CONF_NUM_DESKTOPS];
#endif

void
MenusInit(void)
{
#if 0				/* Not used */
   int                 i;

   for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; task_menu[i++] = NULL);
#endif
}

static Menu        *
RefreshInternalMenu(Menu * m, MenuStyle * ms,
		    Menu * (mcf) (const char *xxx, MenuStyle * ms))
{
   char                was = 0;
   int                 lx = 0, ly = 0;
   EWin               *ewin;

   EDBUG(5, "RefreshInternalMenu");

   if (m)
     {
	ewin = FindEwinByMenu(m);
	if ((m->win) && (ewin))
	  {
	     lx = ewin->x;
	     ly = ewin->y;
	     was = 1;
	  }
	MenuDestroy(m);
	m = NULL;
     }

   if (!ms)
      EDBUG_RETURN(NULL);

   m = mcf("MENU", ms);
   if ((was) && (m))
     {
	m->internal = 1;
	MenuShow(m, 1);
	ewin = FindEwinByMenu(m);
	if (ewin)
	  {
	     MoveEwin(ewin, lx, ly);
	     ShowEwin(ewin);
	  }
	Mode.cur_menu[0] = m;
	Mode.cur_menu_depth = 1;
	MenuShowMasker(m);
     }

   EDBUG_RETURN(m);
}

static void
ShowInternalMenu(Menu ** pm, MenuStyle ** pms, const char *style,
		 Menu * (mcf) (const char *name, MenuStyle * ms))
{
   Menu               *m = *pm;
   MenuStyle          *ms = *pms;

   EDBUG(5, "ShowInternalMenu");

   XUngrabPointer(disp, CurrentTime);

   if (!ms)
     {
	ms = FindItem(style, 0, LIST_FINDBY_NAME, LIST_TYPE_MENU_STYLE);
	if (!ms)
	   ms = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_MENU_STYLE);
	if (!ms)
	   EDBUG_RETURN_;
	*pms = ms;
     }

   Mode.cur_menu_mode = 1;

   *pm = m = RefreshInternalMenu(m, ms, mcf);
   if (m)
     {
	if (!FindEwinByMenu(m))
	   MenuShow(m, 0);
	Mode.cur_menu[0] = m;
	Mode.cur_menu_depth = 1;
	MenuShowMasker(m);
     }
   else
     {
	Mode.cur_menu[0] = NULL;
	Mode.cur_menu_depth = 0;
	MenuHideMasker();
     }

   EDBUG_RETURN_;
}

#if 0				/* Not used */
static Menu        *
RefreshTaskMenu(int desk)
{
   char                was = 0;
   int                 lx = 0, ly = 0;
   EWin               *ewin;

   EDBUG(5, "RefreshTaskMenu");
   if (task_menu[desk])
     {
	ewin = FindEwinByMenu(task_menu[desk]);
	if ((task_menu[desk]->win) && (ewin))
	  {
	     lx = ewin->x;
	     ly = ewin->y;
	     was = 1;
	  }
	MenuDestroy(task_menu[desk]);
     }
   task_menu[desk] = NULL;
   if (!task_menu_style)
     {
	EDBUG_RETURN(NULL);
     }
   task_menu[desk] = MenuCreateFromDesktopEWins("MENU", task_menu_style, desk);
   if ((was) && (task_menu[desk]))
     {
	task_menu[desk]->internal = 1;
	MenuShow(task_menu[desk], 1);
	ewin = FindEwinByMenu(task_menu[desk]);
	if (ewin)
	  {
	     MoveEwin(ewin, lx, ly);
	     ShowEwin(ewin);
	  }
	Mode.cur_menu[0] = task_menu[desk];
	Mode.cur_menu_depth = 1;
	MenuShowMasker(task_menu[desk]);
     }
   EDBUG_RETURN(task_menu[desk]);
}

void
ShowTaskMenu(void)
{
   EDBUG(5, "ShowTaskMenu");
   EDBUG_RETURN_;
}
#endif

void
ShowAllTaskMenu(void)
{
   static MenuStyle   *ms = NULL;
   static Menu        *m = NULL;

   EDBUG(5, "ShowAllTaskMenu");
   ShowInternalMenu(&m, &ms, "TASK_MENU", MenuCreateFromAllEWins);
   EDBUG_RETURN_;
}

void
ShowDeskMenu(void)
{
   static MenuStyle   *ms = NULL;
   static Menu        *m = NULL;

   EDBUG(5, "ShowDeskMenu");
   ShowInternalMenu(&m, &ms, "DESK_MENU", MenuCreateFromDesktops);
   EDBUG_RETURN_;
}

void
ShowGroupMenu(void)
{
   static MenuStyle   *ms = NULL;
   static Menu        *m = NULL;

   EDBUG(5, "ShowGroupMenu");
   ShowInternalMenu(&m, &ms, "GROUP_MENU", MenuCreateFromGroups);
   EDBUG_RETURN_;
}

void
MenusHide(void)
{
   int                 i;

   for (i = 0; i < Mode.cur_menu_depth; i++)
     {
	if (!Mode.cur_menu[i]->stuck)
	   MenuHide(Mode.cur_menu[i]);
     }
   MenuHideMasker();
   Mode.cur_menu_depth = 0;
   Mode.cur_menu_mode = 0;
   clickmenu = 0;

#if 0
   /* If all done properly this shouldn't be necessary... */
   UngrabKeyboard();
   active_menu = NULL;
   active_item = NULL;
#endif
}

Window
MenuWindow(Menu * m)
{
   return m->win;
}

/*
 * Menu event handlers
 */

static MenuItem    *
MenuFindNextItem(Menu * m, MenuItem * mi, int inc)
{
   int                 i;

   if (mi == NULL)
     {
	if (m->num > 0)
	   return m->items[0];
	else
	   return NULL;
     }

   for (i = 0; i < m->num; i++)
      if (m->items[i] == mi)
	{
	   i = (i + inc + m->num) % m->num;
	   return m->items[i];
	}

   return NULL;
}

static MenuItem    *
MenuFindParentItem(Menu * m)
{
   int                 i;
   Menu               *mp;

   mp = m->parent;
   if (mp == NULL)
      return NULL;

   for (i = 0; i < mp->num; i++)
      if (mp->items[i]->child == m)
	 return mp->items[i];

   return NULL;
}

static EWin        *
MenuFindContextEwin(Menu * m)
{
   while (m && m->parent)
      m = m->parent;

   if (!m)
      return NULL;

   return FindEwinSpawningMenu(m);
}

int
MenusEventKeyPress(XEvent * ev)
{
   Window              win = ev->xkey.window;
   KeySym              key;
   Menu               *m;
   MenuItem           *mi;
   EWin               *ewin;

   m = FindMenu(win);
   if (m == NULL)
      return 0;

   mi = NULL;
   if (active_menu)
     {
	m = active_menu;
	mi = active_item;
     }

   /* NB! m != NULL */

   key = XLookupKeysym(&ev->xkey, 0);
   switch (key)
     {
     case XK_Escape:
	MenusHide();
	break;
     case XK_Down:
      check_next:
	mi = MenuFindNextItem(m, mi, 1);
	goto check_activate;
     case XK_Up:
	mi = MenuFindNextItem(m, mi, -1);
	goto check_activate;
     case XK_Left:
	mi = MenuFindParentItem(m);
	m = m->parent;
	goto check_menu;
     case XK_Right:
	if (mi == NULL)
	   goto check_next;
	m = mi->child;
	if (!m || m->num <= 0)
	   break;
	ewin = FindEwinByMenu(m);
	if (ewin == NULL || ewin->state != EWIN_STATE_MAPPED)
	   break;
	mi = m->items[0];
	goto check_menu;
      check_menu:
	if (!m)
	   break;
	goto check_activate;
      check_activate:
	if (!mi)
	   break;
	if (active_menu && active_item && active_menu != m)
	   MenuActivateItem(active_menu, NULL);
	MenuActivateItem(m, mi);
	break;
     case XK_Return:
	if (!mi)
	   break;
	if (!mi->act_id)
	   break;
	MenusHide();
	ActionsCall(mi->act_id, NULL, mi->params);
	break;
     }

   return 1;
}

int
MenusEventMouseDown(XEvent * ev)
{
   Menu               *m;
   MenuItem           *mi;
   EWin               *ewin;

   m = FindMenuItem(ev->xbutton.window, &mi);
   if (m == NULL)
      return 0;
   if (mi == NULL)
      goto done;

   Mode.cur_menu_mode = 1;

   mi->state = STATE_CLICKED;
   MenuDrawItem(m, mi, 1);

   if (mi->child && mi->child->shown == 0)
     {
	int                 mx, my;
	unsigned int        mw, mh;
	EWin               *ewin2;

	Mode.cur_menu[0] = m;
	Mode.cur_menu_depth = 1;
	MenuShowMasker(m);
	XUngrabPointer(disp, CurrentTime);
	ewin = FindEwinByMenu(m);
	if (ewin)
	  {
	     GetWinXY(mi->win, &mx, &my);
	     GetWinWH(mi->win, &mw, &mh);
#if 1				/* Whatgoesonhere ??? */
	     MenuShow(mi->child, 1);
	     ewin2 = FindEwinByMenu(mi->child);
	     if (ewin2)
	       {
		  MoveEwin(ewin2,
			   ewin->x + ewin->border->border.left + mx + mw,
			   ewin->y + ewin->border->border.top + my -
			   ewin2->border->border.top);
		  RaiseEwin(ewin2);
		  ShowEwin(ewin2);
		  if (Conf.menuslide)
		     EwinUnShade(ewin2);
		  Mode.cur_menu[Mode.cur_menu_depth++] = mi->child;
	       }
#else
	     ewin2 = FindEwinByMenu(mi->child);
	     if (!ewin2)
		MenuShow(mi->child, 1);
#endif
	  }
     }

 done:
   return 1;
}

int
MenusEventMouseUp(XEvent * ev)
{
   Menu               *m;
   MenuItem           *mi;
   EWin               *ewin;

   m = FindMenuItem(ev->xbutton.window, &mi);
   if ((m) && (mi->state))
     {
	mi->state = STATE_HILITED;
	MenuDrawItem(m, mi, 1);
	if ((mi->act_id) && (!Mode.justclicked))
	  {
	     ewin = MenuFindContextEwin(m);
	     MenusHide();
	     ActionsCall(mi->act_id, ewin, mi->params);
	     return 1;
	  }
     }

   if ((Mode.cur_menu_mode) && (!clickmenu))
     {
	if (!m)
	  {
	     Window              ww;

	     ww = WindowAtXY(Mode.x, Mode.y);
	     if ((ewin = FindEwinByChildren(ww)))
	       {
		  int                 i;

		  for (i = 0; i < ewin->border->num_winparts; i++)
		    {
		       if (ww == ewin->bits[i].win)
			 {
			    if ((ewin->border->part[i].flags & FLAG_TITLE)
				&& (ewin->menu))
			      {
				 ewin->menu->stuck = 1;
				 i = ewin->border->num_winparts;
			      }
			 }
		    }
	       }
	  }
	MenusHide();
	return 1;
     }

   if ((Mode.cur_menu_mode) && (!Mode.justclicked))
     {
	MenusHide();
	return 1;
     }

   return 0;
}

struct _mdata
{
   Menu               *m;
   MenuItem           *mi;
   EWin               *ewin;
};

static void
MenusSetEvents(int on)
{
   int                 i, j;
   Menu               *m;
   long                event_mask;

   event_mask = (on) ? MENU_ITEM_EVENT_MASK : 0;

   for (i = 0; i < Mode.cur_menu_depth; i++)
     {
	m = Mode.cur_menu[i];
	if (!m)
	   continue;

	for (j = 0; j < m->num; j++)
	   XSelectInput(disp, m->items[j]->win, event_mask);
     }
}

static void
SubmenuShowTimeout(int val, void *dat)
{
   int                 mx, my;
   unsigned int        mw, mh;
   MenuItem           *mi;
   EWin               *ewin2, *ewin;
   struct _mdata      *data;

   data = (struct _mdata *)dat;
   if (!data)
      return;
   if (!data->m)
      return;
   if (!FindEwinByMenu(data->m))
      return;

   mi = data->mi;
   GetWinXY(mi->win, &mx, &my);
   GetWinWH(mi->win, &mw, &mh);
   MenuShow(mi->child, 1);
   ewin2 = FindEwinByMenu(mi->child);
   if (ewin2)
     {
	MoveEwin(ewin2,
		 data->ewin->x + data->ewin->border->border.left + mx + mw,
		 data->ewin->y + data->ewin->border->border.top + my -
		 ewin2->border->border.top);
	RaiseEwin(ewin2);
	ShowEwin(ewin2);

	if (Conf.menuslide)
	   EwinUnShade(ewin2);

	if (Mode.cur_menu[Mode.cur_menu_depth - 1] != mi->child)
	   Mode.cur_menu[Mode.cur_menu_depth++] = mi->child;

	if (Conf.menusonscreen)
	  {
	     EWin               *menus[256];
	     int                 fx[256];
	     int                 fy[256];
	     int                 tx[256];
	     int                 ty[256];
	     int                 i;
	     int                 xdist = 0, ydist = 0;

	     if (ewin2->x + ewin2->w > VRoot.w)
		xdist = VRoot.w - (ewin2->x + ewin2->w);
	     if (ewin2->y + ewin2->h > VRoot.h)
		ydist = VRoot.h - (ewin2->y + ewin2->h);
	     if ((xdist != 0) || (ydist != 0))
	       {
		  for (i = 0; i < Mode.cur_menu_depth; i++)
		    {
		       menus[i] = NULL;
		       if (Mode.cur_menu[i])
			 {
			    ewin = FindEwinByMenu(Mode.cur_menu[i]);
			    if (ewin)
			      {
				 menus[i] = ewin;
				 fx[i] = ewin->x;
				 fy[i] = ewin->y;
				 tx[i] = ewin->x + xdist;
				 ty[i] = ewin->y + ydist;
			      }
			 }
		    }

		  /* Disable menu item events while sliding */
		  MenusSetEvents(0);
		  SlideEwinsTo(menus, fx, fy, tx, ty, Mode.cur_menu_depth,
			       Conf.shadespeed);
		  MenusSetEvents(1);

		  if (Conf.warpmenus)
		     XWarpPointer(disp, None, mi->win, 0, 0, 0, 0,
				  mi->text_w / 2, mi->text_h / 2);
	       }
	  }
     }
   val = 0;
}

static void
MenuActivateItem(Menu * m, MenuItem * mi)
{
   static struct _mdata mdata;
   int                 i, j;

   if (m->sel_item)
     {
	m->sel_item->state = STATE_NORMAL;
	MenuDrawItem(m, m->sel_item, 1);
     }

   m->sel_item = mi;

   if (mi == NULL)
      return;

   mi->state = STATE_HILITED;
   MenuDrawItem(m, mi, 1);

   RemoveTimerEvent("SUBMENU_SHOW");

   for (i = 0; i < Mode.cur_menu_depth; i++)
     {
	if (Mode.cur_menu[i] == m)
	  {
	     if ((!mi->child) ||
		 ((mi->child) && (Mode.cur_menu[i + 1] != mi->child)))
	       {
		  for (j = i + 1; j < Mode.cur_menu_depth; j++)
		     MenuHide(Mode.cur_menu[j]);
		  Mode.cur_menu_depth = i + 1;
		  i = Mode.cur_menu_depth;
		  break;
	       }
	  }
     }

   if ((mi->child) && (!mi->child->shown) && (Mode.cur_menu_mode))
     {
	EWin               *ewin;

	mi->child->parent = m;
	ewin = FindEwinByMenu(m);
	if (ewin)
	  {
	     mdata.m = m;
	     mdata.mi = mi;
	     mdata.ewin = ewin;
	     DoIn("SUBMENU_SHOW", 0.2, SubmenuShowTimeout, 0, &mdata);
	  }
     }
}

int
MenusEventMouseIn(XEvent * ev)
{
   Window              win = ev->xcrossing.window;
   Menu               *m;
   MenuItem           *mi;

   m = FindMenuItem(win, &mi);
   if (m == NULL)
      return 0;
   if (mi == NULL)
      goto done;

   PagerHideAllHi();

   if ((win == mi->icon_win) && (ev->xcrossing.detail == NotifyAncestor))
      goto done;
   if ((win == mi->win) && (ev->xcrossing.detail == NotifyInferior))
      goto done;

   MenuActivateItem(m, mi);

 done:
   return 1;
}

int
MenusEventMouseOut(XEvent * ev)
{
   Window              win = ev->xcrossing.window;
   Menu               *m;
   MenuItem           *mi;

   m = FindMenuItem(win, &mi);
   if (m == NULL)
      return 0;
   if (mi == NULL)
      goto done;

   if ((win == mi->icon_win) && (ev->xcrossing.detail == NotifyAncestor))
      goto done;
   if ((win == mi->win) && (ev->xcrossing.detail == NotifyInferior))
      goto done;

   MenuActivateItem(m, NULL);

 done:
   return 1;
}
