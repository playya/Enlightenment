/* menu.h
 *
 * Copyright (C) 2000 Tom Gilbert
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

typedef struct _feh_menu feh_menu;
typedef struct _feh_menu_item feh_menu_item;
typedef struct _feh_menu_list feh_menu_list;


#define MENU_ITEM_STATE_NORMAL   0x00
#define MENU_ITEM_STATE_SELECTED 0x01

#define MENU_ITEM_IS_SELECTED(item) \
((item)->state & MENU_ITEM_STATE_SELECTED)
#define MENU_ITEM_SET_NORMAL(item) \
(item)->state = (item)->state & (~MENU_ITEM_STATE_SELECTED)
#define MENU_ITEM_SET_SELECTED(item) \
(item)->state = (item)->state | MENU_ITEM_STATE_SELECTED

#define XY_IN_RECT(x, y, rx, ry, rw, rh) \
(((x) >= (rx)) && ((y) >= (ry)) && ((x) < ((rx) + (rw))) && ((y) < ((ry) + (rh))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))


#define FEH_MENU_FONT "20thcent/12"

#define FEH_MENU_PAD_LEFT 2
#define FEH_MENU_PAD_RIGHT 2
#define FEH_MENU_PAD_TOP 2
#define FEH_MENU_PAD_BOTTOM 2

#define FEH_MENUITEM_PAD_LEFT 2
#define FEH_MENUITEM_PAD_RIGHT 2
#define FEH_MENUITEM_PAD_TOP 2
#define FEH_MENUITEM_PAD_BOTTOM 2

#define FEH_MENU_SEP_MAX_H 2

#define FEH_MENU_SUBMENU_H 14
#define FEH_MENU_SUBMENU_W 8

typedef void (*menu_func) (feh_menu * m, feh_menu_item * i, void *data);
typedef feh_menu *(*menuitem_func_gen) (feh_menu * m, feh_menu_item * i,

                                        void *data);

struct _feh_menu_list
{
   feh_menu *menu;
   feh_menu_list *next;
};

struct _feh_menu_item
{
   int state;
   Imlib_Image *icon;
   char *text;
   char *submenu;
   menu_func func;
   void (*func_free) (void *data);
   void *data;
   feh_menu_item *next;
   int text_x, icon_x, sub_x;
   int x, y, w, h;
   menuitem_func_gen func_gen_sub;
};

struct _feh_menu
{
   char *name;
   winwidget fehwin;
   Window win;
   Pixmap pmap;
   int x, y, w, h;
   int item_w, item_h;
   int visible;
   feh_menu_item *items;
   feh_menu *next;
   Imlib_Updates updates;
   int needs_redraw;
   void *data;
   int calc;
   void (*func_free) (feh_menu * m, void *data);
};

feh_menu *feh_menu_new(void);

feh_menu_item *feh_menu_find_selected(feh_menu * m);
feh_menu_item *feh_menu_find_at_xy(feh_menu * m, int x, int y);
void feh_menu_deselect_selected(feh_menu * m);
void feh_menu_select(feh_menu * m, feh_menu_item * i);
void feh_menu_show_at(feh_menu * m, int x, int y);
void feh_menu_show_at_xy(feh_menu * m, winwidget winwin, int x, int y);
void feh_menu_show_at_submenu(feh_menu * m, feh_menu * parent_m,
                              feh_menu_item * i);
void feh_menu_hide(feh_menu * m);
void feh_menu_show(feh_menu * m);
feh_menu_item *feh_menu_add_entry(feh_menu * m, char *text,
                                  Imlib_Image * icon, char *submenu,
                                  menu_func func, void *data,
                                  void (*func_free) (void *data));
void feh_menu_entry_get_size(feh_menu * m, feh_menu_item * i, int *w, int *h);
void feh_menu_calc_size(feh_menu * m);
void feh_menu_draw_item(feh_menu * m, feh_menu_item * i, Imlib_Image im,
                        int ox, int oy);
void feh_menu_redraw(feh_menu * m);
void feh_menu_move(feh_menu * m, int x, int y);
void feh_menu_init(void);
void feh_menu_draw_to_buf(feh_menu * m, Imlib_Image im, int ox, int oy);
void feh_menu_draw_menu_bg(feh_menu * m, Imlib_Image im, int ox, int oy);
void feh_menu_draw_submenu_at(int x, int y, int w, int h, Imlib_Image dst,
                              int ox, int oy, int selected);
void feh_menu_draw_separator_at(int x, int y, int w, int h, Imlib_Image dst,
                                int ox, int oy);
void feh_menu_item_draw_at(int x, int y, int w, int h, Imlib_Image dst,
                           int ox, int oy, int selected);
void feh_redraw_menus(void);
feh_menu *feh_menu_find(char *name);
void feh_redraw_menus(void);
feh_menu *feh_menu_get_from_window(Window win);
void feh_raise_all_menus(void);
