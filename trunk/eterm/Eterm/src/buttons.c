/*
 * Copyright (C) 1997-2000, Michael Jennings
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

static const char cvs_ident[] = "$Id$";

#include "config.h"
#include "feature.h"

#include <X11/cursorfont.h>

#include "../libmej/debug.h"
#include "../libmej/mem.h"
#include "../libmej/strings.h"
#include "buttons.h"
#include "command.h"
#include "draw.h"
#include "e.h"
#include "events.h"
#include "font.h"
#include "startup.h"
#include "menus.h"
#include "misc.h"
#include "options.h"
#include "pixmap.h"
#include "screen.h"
#include "term.h"
#include "windows.h"

static __inline__ void draw_string(buttonbar_t *, Drawable, GC, int, int, char *, size_t);

buttonbar_t *buttonbar = NULL;
long bbar_total_h = -1;

static __inline__ void
draw_string(buttonbar_t *bbar, Drawable d, GC gc, int x, int y, char *str, size_t len)
{

  D_BBAR(("Writing string \"%s\" (length %lu) onto drawable 0x%08x at %d, %d\n", str, len, d, x, y));

#ifdef MULTI_CHARSET
  if (bbar->fontset)
    XmbDrawString(Xdisplay, d, bbar->fontset, gc, x, y, str, len);
  else
#endif
    XDrawString(Xdisplay, d, gc, x, y, str, len);
  return;
  bbar = NULL;
}

buttonbar_t *
bbar_create(void)
{
  buttonbar_t *bbar;
  Cursor cursor;
  long mask;
  XGCValues gcvalue;
  XSetWindowAttributes xattr;

  bbar = (buttonbar_t *) MALLOC(sizeof(buttonbar_t));
  MEMSET(bbar, 0, sizeof(buttonbar_t));

  xattr.border_pixel = BlackPixel(Xdisplay, Xscreen);
  xattr.save_under = FALSE;
  xattr.backing_store = NotUseful;
  xattr.override_redirect = TRUE;
  xattr.colormap = cmap;

  cursor = XCreateFontCursor(Xdisplay, XC_left_ptr);
  mask = EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | ButtonPressMask | ButtonReleaseMask;
  gcvalue.foreground = xattr.border_pixel;

  bbar->font = load_font(etfonts[def_font_idx], "fixed", FONT_TYPE_X);
  bbar->fwidth = bbar->font->max_bounds.width;
  bbar->fheight = bbar->font->ascent + bbar->font->descent + rs_line_space;
  bbar->h = 1;
  bbar->w = 1;
  gcvalue.font = bbar->font->fid;

  bbar->win = XCreateWindow(Xdisplay, Xroot, bbar->x, bbar->y, bbar->w, bbar->h, 0, Xdepth, InputOutput, CopyFromParent,
			    CWOverrideRedirect | CWSaveUnder | CWBackingStore | CWBorderPixel | CWColormap, &xattr);
  XDefineCursor(Xdisplay, bbar->win, cursor);
  XSelectInput(Xdisplay, bbar->win, mask);
  XStoreName(Xdisplay, bbar->win, "Eterm Button Bar");

  bbar->gc = XCreateGC(Xdisplay, bbar->win, GCForeground | GCFont, &gcvalue);
  bbar_set_docked(bbar, BBAR_DOCKED_TOP);

  return bbar;
}

void
bbar_init(buttonbar_t *bbar, int width)
{
  event_register_dispatcher(bbar_dispatch_event, bbar_event_init_dispatcher);
  XSetForeground(Xdisplay, bbar->gc, images[image_bbar].norm->fg);
  bbar_resize(bbar, width);
  bbar_reset_total_height();
  bbar_calc_total_height();
}

void
bbar_event_init_dispatcher(void)
{
  /* FIXME:  The event subsystem needs to be able to pass a pointer to the event data structure. */
  EVENT_DATA_ADD_HANDLER(buttonbar->event_data, EnterNotify, bbar_handle_enter_notify);
  EVENT_DATA_ADD_HANDLER(buttonbar->event_data, LeaveNotify, bbar_handle_leave_notify);
  EVENT_DATA_ADD_HANDLER(buttonbar->event_data, ButtonPress, bbar_handle_button_press);
  EVENT_DATA_ADD_HANDLER(buttonbar->event_data, ButtonRelease, bbar_handle_button_release);
  EVENT_DATA_ADD_HANDLER(buttonbar->event_data, MotionNotify, bbar_handle_motion_notify);

  event_data_add_mywin(&buttonbar->event_data, buttonbar->win);
}

unsigned char
bbar_handle_enter_notify(event_t * ev)
{
  button_t *b;
  Window unused_root, unused_child;
  int unused_root_x, unused_root_y;
  unsigned int unused_mask;

  D_EVENTS(("bbar_handle_enter_notify(ev [%8p] on window 0x%08x)\n", ev, ev->xany.window));

  REQUIRE_RVAL(XEVENT_IS_MYWIN(ev, &buttonbar->event_data), 0);

  bbar_draw(buttonbar, IMAGE_STATE_SELECTED, 0);
  XQueryPointer(Xdisplay, buttonbar->win, &unused_root, &unused_child, &unused_root_x, &unused_root_y, &(ev->xbutton.x), &(ev->xbutton.y), &unused_mask);
  b = find_button_by_coords(buttonbar, ev->xbutton.x, ev->xbutton.y);
  if (b) {
    bbar_select_button(buttonbar, b);
  }
  return 1;
}

unsigned char
bbar_handle_leave_notify(event_t * ev)
{
  D_EVENTS(("bbar_handle_leave_notify(ev [%8p] on window 0x%08x)\n", ev, ev->xany.window));

  REQUIRE_RVAL(XEVENT_IS_MYWIN(ev, &buttonbar->event_data), 0);

  bbar_draw(buttonbar, IMAGE_STATE_NORMAL, 0);
  if (buttonbar->current) {
    bbar_deselect_button(buttonbar, buttonbar->current);
  }
  return 1;
}

unsigned char
bbar_handle_button_press(event_t * ev)
{

  D_EVENTS(("bbar_handle_button_press(ev [%8p] on window 0x%08x)\n", ev, ev->xany.window));

  REQUIRE_RVAL(XEVENT_IS_MYWIN(ev, &buttonbar->event_data), 0);

  if (buttonbar->current) {
    bbar_click_button(buttonbar, buttonbar->current);
    button_check_action(buttonbar, buttonbar->current, 1, ev->xbutton.time);
  }
  return 1;
}

unsigned char
bbar_handle_button_release(event_t * ev)
{
  button_t *b;
  Window unused_root, unused_child;
  int unused_root_x, unused_root_y;
  unsigned int unused_mask;

  D_EVENTS(("bbar_handle_button_release(ev [%8p] on window 0x%08x)\n", ev, ev->xany.window));

  REQUIRE_RVAL(XEVENT_IS_MYWIN(ev, &buttonbar->event_data), 0);

  XQueryPointer(Xdisplay, buttonbar->win, &unused_root, &unused_child, &unused_root_x, &unused_root_y, &(ev->xbutton.x), &(ev->xbutton.y), &unused_mask);

  b = find_button_by_coords(buttonbar, ev->xbutton.x, ev->xbutton.y);
  if (b) {
    if (buttonbar->current && (b != buttonbar->current)) {
      bbar_deselect_button(buttonbar, buttonbar->current);
    } else {
      bbar_select_button(buttonbar, b);
      button_check_action(buttonbar, b, 0, ev->xbutton.time);
    }
  }

  return 1;
}

unsigned char
bbar_handle_motion_notify(event_t * ev)
{
  button_t *b;
  Window unused_root, unused_child;
  int unused_root_x, unused_root_y;
  unsigned int mask;

  D_EVENTS(("bbar_handle_motion_notify(ev [%8p] on window 0x%08x)\n", ev, ev->xany.window));

  REQUIRE_RVAL(XEVENT_IS_MYWIN(ev, &buttonbar->event_data), 0);

  while (XCheckTypedWindowEvent(Xdisplay, ev->xany.window, MotionNotify, ev));
  XQueryPointer(Xdisplay, buttonbar->win, &unused_root, &unused_child, &unused_root_x, &unused_root_y, &(ev->xbutton.x), &(ev->xbutton.y), &mask);
  D_BBAR((" -> Pointer is at %d, %d with mask 0x%08x\n", ev->xbutton.x, ev->xbutton.y, mask));

  b = find_button_by_coords(buttonbar, ev->xbutton.x, ev->xbutton.y);
  if (b != buttonbar->current) {
    if (buttonbar->current) {
      bbar_deselect_button(buttonbar, buttonbar->current);
    }
    if (b) {
      if (mask & (Button1Mask | Button2Mask | Button3Mask)) {
        bbar_click_button(buttonbar, b);
      } else {
        bbar_select_button(buttonbar, b);
      }
    }
  }

  return 1;
}

unsigned char
bbar_dispatch_event(event_t * ev)
{
  if (buttonbar->event_data.handlers[ev->type] != NULL) {
    return ((buttonbar->event_data.handlers[ev->type]) (ev));
  }
  return (0);
}

void
bbar_add(buttonbar_t *bbar)
{
  if (buttonbar) {
    buttonbar_t *bb;

    for (bb = buttonbar; bb->next; bb = bb->next);
    bb->next = bbar;
  } else {
    buttonbar = bbar;
  }
  bbar->next = NULL;
  bbar_reset_total_height();
  bbar_calc_total_height();
}

unsigned short
bbar_calc_height(buttonbar_t *bbar)
{
  button_t *b;
  ImlibBorder *bbord = images[image_bbar].norm->iml->border, *bord = images[image_button].norm->iml->border;

  D_BBAR(("bbar_calc_height(%8p):  fascent == %d, fdescent == %d, h == %d\n", bbar, bbar->fascent, bbar->fdescent, bbar->h));

  bbar->h = bbar->fascent + bbar->fdescent + 1;
  if (bord) {
    bbar->h += bord->top + bord->bottom;
  }
  for (b = bbar->buttons; b; b = b->next) {
    if (b->h != bbar->h) {
      b->h = bbar->h;
      button_calc_size(bbar, b);
    }
  }
  for (b = bbar->rbuttons; b; b = b->next) {
    if (b->h != bbar->h) {
      b->h = bbar->h;
      button_calc_size(bbar, b);
    }
  }
  if (bbord) {
    bbar->h += bbord->top + bbord->bottom;
  }
  D_BBAR(("Final height is %d\n", bbar->h));
  return bbar->h;
}

void
bbar_calc_sizes(buttonbar_t *bbar)
{
  button_t *b;

  D_BBAR(("bbar == %8p\n", bbar));

  for (b = bbar->buttons; b; b = b->next) {
    button_calc_size(bbar, b);
  }
  for (b = bbar->rbuttons; b; b = b->next) {
    button_calc_size(bbar, b);
  }
}

void
bbar_calc_positions(buttonbar_t *bbar)
{
  button_t *b;
  unsigned short x, y;
  ImlibBorder *border = images[image_bbar].norm->iml->border;

  D_BBAR(("bbar == %8p\n", bbar));

  y = ((border) ? (border->top) : 0);
  if (bbar->buttons) {
    x = ((border) ? (border->left) : 0) + MENU_HGAP;
    for (b = bbar->buttons; b; b = b->next) {
      b->x = x;
      b->y = y;
      D_BBAR(("Set button \"%s\" (%8p, width %d) to coordinates %d, %d\n", b->text, b, b->w, x, y));
      x += b->w + MENU_HGAP;
      button_calc_rel_coords(bbar, b);
    }
  }
  if (bbar->rbuttons) {
    x = bbar->w - ((border) ? (border->right) : 0);
    for (b = bbar->rbuttons; b; b = b->next) {
      x -= b->w + MENU_HGAP;
      b->x = x;
      b->y = y;
      button_calc_rel_coords(bbar, b);
      D_BBAR(("Set button \"%s\" (%8p, width %d) to coordinates %d, %d\n", b->text, b, b->w, x, y));
    }
  }
}

void
button_calc_size(buttonbar_t *bbar, button_t *button)
{
  ImlibBorder *bord = images[image_button].norm->iml->border;
  int ascent, descent, direction;
  XCharStruct chars;

  D_BBAR(("button_calc_size(%8p, %8p):  XTextExtents(%8p, %s, %d, ...)\n", bbar, button, bbar->font, button->text, button->len));

  button->w = 0;
  if (button->len) {
    XTextExtents(bbar->font, button->text, button->len, &direction, &ascent, &descent, &chars);
    LOWER_BOUND(bbar->fascent, chars.ascent);
    LOWER_BOUND(bbar->fdescent, chars.descent);
    button->w += chars.width;
  }
  if (bord) {
    button->w += bord->left + bord->right;
  }
  if (button->h == 0) {
    button->h = bbar->fascent + bbar->fdescent + 1;
    if (bord) {
      button->h += bord->top + bord->bottom;
    }
  }
  if (button->icon) {
    unsigned short b = 0;

    if (bord) {
      b = button->h - bord->top - bord->bottom;
    }
    button->icon_w = button->icon->iml->im->rgb_width;
    button->icon_h = button->icon->iml->im->rgb_height;
    D_BBAR((" -> Initial icon dimensions are %hux%hu\n", button->icon_w, button->icon_h));
    if (button->icon_h > b) {
      button->icon_w = (unsigned short) ((float) button->icon_w / button->icon_h * b);
      button->icon_h = b;
    }
    button->w += button->icon_w;
    if (button->len) {
      button->w += MENU_HGAP;
    }
    D_BBAR((" -> Final icon dimensions are %hux%hu\n", button->icon_w, button->icon_h));
  }
  D_BBAR((" -> Set button to %dx%d at %d, %d and icon to %dx%d\n", button->w, button->h, button->x, button->y, button->icon_w, button->icon_h));
}

void
button_calc_rel_coords(buttonbar_t *bbar, button_t *button)
{
  ImlibBorder *bord = images[image_button].norm->iml->border;

  D_BBAR(("bbar == %8p, button == %8p\n", bbar, button));

  if (button->icon) {
    unsigned short b = 0;

    if (bord) {
      b = button->h - bord->top - bord->bottom - 2;
    }
    if (button->icon_h == button->h) {
      button->icon_y = button->y + ((bord) ? (bord->top) : 0);
    } else {
      button->icon_y = button->y + ((b - button->icon_h) / 2) + ((bord) ? (bord->top) : 0);
    }
    button->icon_x = button->x + ((bord) ? (bord->left) : 0);
  }
  if (button->len) {
    button->text_x = button->x + ((button->icon_w) ? (button->icon_w + MENU_HGAP) : 0) + ((bord) ? (bord->left) : (0));
    button->text_y = button->y + button->h - ((bord) ? (bord->bottom) : (0)) - bbar->fdescent;
  }
  D_BBAR((" -> Text is at %d, %d and icon is at %d, %d\n", button->text_x, button->text_y, button->icon_x, button->icon_y));
}

void
bbar_add_button(buttonbar_t *bbar, button_t *button)
{
  button_t *b;

  D_BBAR(("bbar_add_button(%8p, %8p):  Adding button \"%s\".\n", bbar, button, button->text));

  if (bbar->buttons) { 
    for (b = bbar->buttons; b->next; b = b->next);
    b->next = button;
  } else {
    bbar->buttons = button;
  }
  button->next = NULL;
}

void
bbar_add_rbutton(buttonbar_t *bbar, button_t *button)
{
  button_t *b;

  D_BBAR(("bbar_add_rbutton(%8p, %8p):  Adding button \"%s\".\n", bbar, button, button->text));

  b = ((bbar->rbuttons) ? (bbar->rbuttons) : NULL);
  bbar->rbuttons = button;
  button->next = b;
}

unsigned char
bbar_set_font(buttonbar_t *bbar, const char *fontname)
{
  XFontStruct *font;

  ASSERT_RVAL(fontname != NULL, 0);

  D_BBAR(("bbar_set_font(%8p, \"%s\"):  Current font is %8p, dimensions %d/%d/%d\n", bbar, fontname, bbar->font, bbar->fwidth, bbar->fheight, bbar->h));
  if (bbar->font) {
    free_font(bbar->font);
  }
#ifdef MULTI_CHARSET
  if (bbar->fontset) {
    XFreeFontSet(Xdisplay, bbar->fontset);
  }
#endif

  font = (XFontStruct *) load_font(fontname, "fixed", FONT_TYPE_X);
#ifdef MULTI_CHARSET
  bbar->fontset = create_fontset(fontname, etmfonts[def_font_idx]);
#endif

  bbar->font = font;
  bbar->fwidth = font->max_bounds.width;
  bbar->fheight = font->ascent + font->descent + rs_line_space;
  XSetFont(Xdisplay, bbar->gc, font->fid);
  bbar_reset_total_height();
  D_BBAR(("New dimensions are %d/%d/%d\n", bbar->fwidth, bbar->fheight, bbar->h));

  return 1;
}

button_t *
find_button_by_text(buttonbar_t *bbar, char *text)
{
  register button_t *b;

  REQUIRE_RVAL(text != NULL, NULL);

  for (b = bbar->buttons; b; b = b->next) {
    if (!strcasecmp(b->text, text)) {
      return (b);
    }
  }
  for (b = bbar->rbuttons; b; b = b->next) {
    if (!strcasecmp(b->text, text)) {
      return (b);
    }
  }
  return NULL;
}

button_t *
find_button_by_coords(buttonbar_t *bbar, int x, int y)
{
  register button_t *b;

  ASSERT_RVAL(bbar != NULL, NULL);

  for (b = bbar->buttons; b; b = b->next) {
    if ((x >= b->x) && (y >= b->y) && (x < b->x + b->w) && (y < b->y + b->h)) {
      return (b);
    }
  }
  for (b = bbar->rbuttons; b; b = b->next) {
    if ((x >= b->x) && (y >= b->y) && (x < b->x + b->w) && (y < b->y + b->h)) {
      return (b);
    }
  }
  return NULL;
}

button_t *
button_create(char *text)
{
  button_t *button;

  button = (button_t *) MALLOC(sizeof(button_t));
  MEMSET(button, 0, sizeof(button_t));

  if (text) {
    button->text = StrDup(text);
    button->len = strlen(text);
  } else {
    button->text = "";
    button->len = 0;
  }
  return button;
}

unsigned char
button_set_icon(button_t *button, simage_t * icon)
{
  ASSERT_RVAL(button != NULL, 0);
  ASSERT_RVAL(icon != NULL, 0);

  button->icon = icon;
  return 1;
}

unsigned char
button_set_action(button_t *button, action_type_t type, char *action)
{
  ASSERT_RVAL(button != NULL, 0);

  button->type = type;
  switch (type) {
    case ACTION_MENU:
      button->action.menu = find_menu_by_title(menu_list, action);
      break;
    case ACTION_STRING:
    case ACTION_ECHO:
      button->action.string = (char *) MALLOC(strlen(action) + 2);
      strcpy(button->action.string, action);
      parse_escaped_string(button->action.string);
      break;
    default:
      break;
  }
  return 1;
}

void
bbar_select_button(buttonbar_t *bbar, button_t *button)
{
  bbar->current = button;
  if (image_mode_is(image_button, MODE_MASK)) {
    paste_simage(images[image_button].selected, image_button, bbar->win, button->x, button->y, button->w, button->h);
  } else {
    draw_shadow_from_colors(bbar->win, PixColors[menuTopShadowColor], PixColors[menuBottomShadowColor], button->x, button->y, button->w, button->h, 2);
  }
  if (image_mode_is(image_button, MODE_AUTO)) {
    enl_ipc_sync();
  }
  if (button->icon) {
    paste_simage(button->icon, image_max, bbar->win, button->icon_x, button->icon_y, button->icon_w, button->icon_h);
  }
  if (button->len) {
    XSetForeground(Xdisplay, bbar->gc, images[image_bbar].selected->fg);
    draw_string(bbar, bbar->win, bbar->gc, button->text_x, button->text_y, button->text, button->len);
    XSetForeground(Xdisplay, bbar->gc, images[image_bbar].norm->fg);
  }
}

void
bbar_deselect_button(buttonbar_t *bbar, button_t *button)
{
  XClearArea(Xdisplay, bbar->win, button->x, button->y, button->w, button->h, False);
  bbar->current = NULL;
}

void
bbar_click_button(buttonbar_t *bbar, button_t *button)
{
  bbar->current = button;
  if (image_mode_is(image_button, MODE_MASK)) {
    paste_simage(images[image_button].clicked, image_button, bbar->win, button->x, button->y, button->w, button->h);
  } else {
    draw_shadow_from_colors(bbar->win, PixColors[menuBottomShadowColor], PixColors[menuTopShadowColor], button->x, button->y, button->w, button->h, 2);
  }
  if (image_mode_is(image_button, MODE_AUTO)) {
    enl_ipc_sync();
  }
  if (button->icon) {
    paste_simage(button->icon, image_max, bbar->win, button->icon_x, button->icon_y, button->icon_w, button->icon_h);
  }
  if (button->len) {
    XSetForeground(Xdisplay, bbar->gc, images[image_bbar].clicked->fg);
    draw_string(bbar, bbar->win, bbar->gc, button->text_x, button->text_y, button->text, button->len);
    XSetForeground(Xdisplay, bbar->gc, images[image_bbar].norm->fg);
  }
}

void
button_check_action(buttonbar_t *bbar, button_t *button, unsigned char press, Time t)
{
  switch (button->type) {
    case ACTION_MENU:
      if (press) {
        menu_invoke(button->x, button->y + button->h, bbar->win, button->action.menu, t);
      }
      break;
    case ACTION_STRING:
      if (!press) {
        cmd_write((unsigned char *) button->action.string, strlen(button->action.string));
      }
      break;
    case ACTION_ECHO:
      if (!press) {
        tt_write((unsigned char *) button->action.string, strlen(button->action.string));
      }
      break;
    default:
      break;
  }
}

unsigned char
bbar_show(buttonbar_t *bbar, unsigned char visible)
{
  unsigned char changed = 0;

  D_BBAR(("bbar_show(%8p, %d) called.\n", bbar, visible));
  if (visible && !bbar_is_visible(bbar)) {
    D_BBAR((" -> Making bbar visible.\n"));
    XMapWindow(Xdisplay, bbar->win);
    bbar_set_visible(bbar, 1);
    changed = 1;
  } else if (!visible && bbar_is_visible(bbar)) {
    D_BBAR((" -> Making bbar invisible.\n"));
    XUnmapWindow(Xdisplay, bbar->win);
    bbar_set_visible(bbar, 0);
    changed = 1;
  }
  return changed;
}

void
bbar_resize(buttonbar_t *bbar, int w)
{
  D_BBAR(("bbar_resize(%8p, %d) called.\n", bbar, w));
  if (w == -1) {
    bbar_calc_sizes(bbar);
    bbar_calc_height(bbar);
    bbar_reset_total_height();
  } else if (bbar->w != w) {
    bbar->w = w;
    bbar_calc_positions(bbar);
    D_BBAR(("Resizing window 0x%08x to %dx%d\n", bbar->win, bbar->w, bbar->h));
    XResizeWindow(Xdisplay, bbar->win, bbar->w, bbar->h);
    bbar_draw(bbar, IMAGE_STATE_CURRENT, MODE_MASK);
  }
}

void
bbar_resize_all(int width)
{
  buttonbar_t *bbar;

  for (bbar = buttonbar; bbar; bbar = bbar->next) {
    bbar_resize(bbar, width);
  }
  bbar_calc_total_height();
}

void
bbar_draw(buttonbar_t *bbar, unsigned char image_state, unsigned char force_modes)
{
  button_t *button;

  ASSERT(bbar != NULL);

  D_BBAR(("bbar_draw(%8p, 0x%02x, 0x%02x) called.\n", bbar, image_state, force_modes));
  if (image_state != IMAGE_STATE_CURRENT) {
    if ((image_state == IMAGE_STATE_NORMAL) && (images[image_bbar].current != images[image_bbar].norm)) {
      images[image_bbar].current = images[image_bbar].norm;
      force_modes = MODE_MASK;
    } else if ((image_state == IMAGE_STATE_SELECTED) && (images[image_bbar].current != images[image_bbar].selected)) {
      images[image_bbar].current = images[image_bbar].selected;
      force_modes = MODE_MASK;
    } else if ((image_state == IMAGE_STATE_CLICKED) && (images[image_bbar].current != images[image_bbar].clicked)) {
      images[image_bbar].current = images[image_bbar].clicked;
      force_modes = MODE_MASK;
    } else if ((image_state == IMAGE_STATE_DISABLED) && (images[image_bbar].current != images[image_bbar].disabled)) {
      images[image_bbar].current = images[image_bbar].disabled;
      force_modes = MODE_MASK;
    }
  }
  if (image_mode_is(image_bbar, MODE_MASK) && !((images[image_bbar].mode & MODE_MASK) & (force_modes))) {
    return;
  } else {
    render_simage(images[image_bbar].current, bbar->win, bbar->w, bbar->h, image_bbar, RENDER_FORCE_PIXMAP);
    bbar->bg = images[image_bbar].current->pmap->pixmap;
  }
  XSetForeground(Xdisplay, bbar->gc, images[image_bbar].current->fg);
  for (button = bbar->buttons; button; button = button->next) {
    if (button->icon) {
      paste_simage(button->icon, image_max, bbar->bg, button->icon_x, button->icon_y, button->icon_w, button->icon_h);
    }
    if (button->len) {
      draw_string(bbar, bbar->bg, bbar->gc, button->text_x, button->text_y, button->text, button->len);
    }
  }
  for (button = bbar->rbuttons; button; button = button->next) {
    if (button->icon) {
      paste_simage(button->icon, image_max, bbar->bg, button->icon_x, button->icon_y, button->icon_w, button->icon_h);
    }
    if (button->len) {
      draw_string(bbar, bbar->bg, bbar->gc, button->text_x, button->text_y, button->text, button->len);
    }
  }
  XSetWindowBackgroundPixmap(Xdisplay, bbar->win, bbar->bg);
  XClearWindow(Xdisplay, bbar->win);
  XSetForeground(Xdisplay, bbar->gc, images[image_bbar].norm->fg);
}

void
bbar_draw_all(unsigned char image_state, unsigned char force_modes)
{
  buttonbar_t *bbar;

  for (bbar = buttonbar; bbar; bbar = bbar->next) {
    bbar_draw(bbar, image_state, force_modes);
  }
}

void
bbar_dock(buttonbar_t *bbar, unsigned char dock)
{
  D_BBAR(("bbar_dock(%8p, %d) called.\n", bbar, dock));
  bbar_set_docked(bbar, dock);
  if (dock == BBAR_DOCKED_TOP) {
    bbar->x = 0;
    bbar->y = 0;
    XReparentWindow(Xdisplay, bbar->win, TermWin.parent, bbar->x, bbar->y);
    XMoveResizeWindow(Xdisplay, bbar->win, bbar->x, bbar->y, bbar->w, bbar->h);
  }
}

unsigned long
bbar_calc_total_height(void)
{
  buttonbar_t *bbar;

  bbar_total_h = 0;
  for (bbar = buttonbar; bbar; bbar = bbar->next) {
    if (bbar_is_docked(bbar)) {
      bbar_total_h += bbar->h;
    }
  }
  D_BBAR(("Returning %d\n", bbar_total_h));
  return bbar_total_h;
}
