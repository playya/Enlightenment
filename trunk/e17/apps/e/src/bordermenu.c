#include "menu.h"
#include "border.h"
#include "desktops.h"
#include "debug.h"
#include "util.h"
#include "icccm.h"
#include "bordermenu.h"

static void e_bordermenu_cb_close(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_kill(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_raise(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_lower(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_sticky(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_iconify(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_max(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_zoom(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_remember_location(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_remember_size(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_remember_desktop(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_remember_sticky(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_remember_prog_location_ignore(E_Menu *m, E_Menu_Item *mi, void  *data);
static void e_bordermenu_cb_menu_hide(E_Menu *m, void  *data);

static void
e_bordermenu_cb_close(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   if (b->win.client) e_icccm_delete(b->win.client);
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_kill(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   if (b->win.client) ecore_window_kill_client(b->win.client);   
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_raise(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   e_border_raise(b);   
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_lower(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   e_border_lower(b);   
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_sticky(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);
   
   b->client.sticky = mi->on;
   b->changed = 1;
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_iconify(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
         
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_max(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   if (b->client.is_desktop) D_RETURN;
   if (b->current.shaded > 0) D_RETURN;
   if ((b->mode.move) || (b->mode.resize)) D_RETURN;
   b->mode.move = 0;
   b->mode.resize = 0;

   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);
   
   if (mi->on)
     {
	b->max.x = b->current.x;
	b->max.y = b->current.y;
	b->max.w = b->current.w;
	b->max.h = b->current.h;
	b->current.requested.x = 0;
	b->current.requested.y = 0;
	b->current.requested.w = b->desk->real.w;
	b->current.requested.h = b->desk->real.h;
	b->changed = 1;
	b->max.is = mi->on;
	e_border_adjust_limits(b);
	b->current.requested.x = b->current.x;
	b->current.requested.y = b->current.y;
	b->current.requested.w = b->current.w;
	b->current.requested.h = b->current.h;
     }
   else
     {
	b->current.requested.x = b->max.x;
	b->current.requested.y = b->max.y;
	b->current.requested.w = b->max.w;
	b->current.requested.h = b->max.h;
	b->changed = 1;
	b->max.is = mi->on;
	e_border_adjust_limits(b);
	b->current.requested.x = b->current.x;
	b->current.requested.y = b->current.y;
	b->current.requested.w = b->current.w;
	b->current.requested.h = b->current.h;
     }
   
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_zoom(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
         
   D_RETURN;
   UN(m);
   UN(mi);
}

static void
e_bordermenu_cb_remember_location(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);

   b->client.matched.matched = 1;
   b->client.matched.location.matched = mi->on;
   
   D_RETURN;
   UN(m);
}

static void
e_bordermenu_cb_remember_size(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);

   b->client.matched.matched = 1;
   b->client.matched.size.matched = mi->on;
   
   D_RETURN;
   UN(m);
}

static void
e_bordermenu_cb_remember_desktop(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);

   b->client.matched.matched = 1;
   b->client.matched.desktop.matched = mi->on;
   
   D_RETURN;
   UN(m);
}

static void
e_bordermenu_cb_remember_sticky(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);

   b->client.matched.matched = 1;
   b->client.matched.sticky.matched = mi->on;
   
   D_RETURN;
   UN(m);
}

static void
e_bordermenu_cb_remember_prog_location_ignore(E_Menu *m, E_Menu_Item *mi, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;
   
   if (mi->on) e_menu_item_set_state(mi, 0);
   else        e_menu_item_set_state(mi, 1);
   e_menu_set_state(m, mi);

   b->client.matched.matched = 1;
   b->client.matched.prog_location.matched = mi->on;
   b->client.matched.prog_location.ignore = 1;
   
   D_RETURN;
   UN(m);
}

static void
e_bordermenu_cb_menu_hide(E_Menu *m, void  *data)
{
   E_Border *b;
   
   D_ENTER;
   
   b = data;

   while (b->menus)
     {
	m = b->menus->data;
	m->delete_me = 1;
	b->menus = evas_list_remove(b->menus, m);
     }
   
   D_RETURN;   
   UN(m);
}

void
e_bordermenu_do(E_Border *b)
{
   E_Menu *menu;
   E_Menu_Item *menuitem;
   
   D_ENTER;
   
   if (!b->menus)
     {
	menu = e_menu_new();
	b->menus = evas_list_append(b->menus, menu);
	e_menu_set_padding_icon(menu, 2);
	e_menu_set_padding_state(menu, 2);
	e_menu_hide_callback(menu, e_bordermenu_cb_menu_hide, b);
	
	menuitem = e_menu_item_new("Close");
	/* e_menu_item_set_icon(menuitem, icon);   */
	/* e_menu_item_set_scale_icon(menuitem, 1);*/
	/* e_menu_item_set_separator(menuitem, 1);*/
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_close, b);
	e_menu_add_item(menu, menuitem);

	menuitem = e_menu_item_new("Raise");
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_raise, b);
	e_menu_add_item(menu, menuitem);

	menuitem = e_menu_item_new("Lower");
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_lower, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Iconify");
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_iconify, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Zoom");
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_zoom, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Maximise");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->max.is);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_max, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Sticky");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.sticky);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_sticky, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("");
	e_menu_item_set_separator(menuitem, 1);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Kill");
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_kill, b);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("");
	e_menu_item_set_separator(menuitem, 1);
	e_menu_add_item(menu, menuitem);
	
	menuitem = e_menu_item_new("Remember Location");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.matched.location.matched);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_remember_location, b);
	e_menu_add_item(menu, menuitem);
	e_menu_set_state(menu, menuitem);

	menuitem = e_menu_item_new("Remember Size");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.matched.size.matched);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_remember_size, b);
	e_menu_add_item(menu, menuitem);
	e_menu_set_state(menu, menuitem);

	menuitem = e_menu_item_new("Remember Desktop");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.matched.desktop.matched);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_remember_desktop, b);
	e_menu_add_item(menu, menuitem);
	e_menu_set_state(menu, menuitem);

	menuitem = e_menu_item_new("Remember Stickiness");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.matched.sticky.matched);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_remember_sticky, b);
	e_menu_add_item(menu, menuitem);
	e_menu_set_state(menu, menuitem);

	menuitem = e_menu_item_new("Ignore Program Specified Position");
	e_menu_item_set_check(menuitem, 1);
	e_menu_item_set_state(menuitem, b->client.matched.prog_location.matched);
	e_menu_item_set_callback(menuitem, e_bordermenu_cb_remember_prog_location_ignore, b);
	e_menu_add_item(menu, menuitem);
	e_menu_set_state(menu, menuitem);
     }

     {
	int pl, pr, pt, pb;
	int crx, cry, crw, crh;
	int mx, my;
	
	menu = b->menus->data;
	pl = pr = pt = pb = 0;
	if (b->bits.t) ebits_get_insets(b->bits.t, &pl, &pr, &pt, &pb);	
	crx = b->current.x + pl;
	cry = b->current.y + pt;
	crw = b->client.w;
	crh = b->client.h;
	ecore_pointer_xy_get(&mx, &my);
	if (mx + menu->current.w > crx + crw) mx = crx + crw - menu->current.w;
	if (my + menu->current.h > cry + crh) my = cry + crh - menu->current.h;
	if (mx < crx) mx = crx;
	if (my < cry) my = cry;
	e_menu_show_at_mouse(menu, mx, my, CurrentTime);
     }
   
   D_RETURN;
}
