#include "menu.h"
#include "config.h"

static Evas_List open_menus = NULL;
static Evas_List menus = NULL;
static Window    menu_event_win = 0;
static int       screen_w, screen_h;
static int       mouse_x, mouse_y;
static int       keyboard_nav = 0;

static void e_idle(void *data);
static void e_wheel(Eevent * ev);
static void e_key_down(Eevent * ev);
static void e_key_up(Eevent * ev);
static void e_mouse_down(Eevent * ev);
static void e_mouse_up(Eevent * ev);
static void e_mouse_move(Eevent * ev);
static void e_mouse_in(Eevent * ev);
static void e_mouse_out(Eevent * ev);
static void e_window_expose(Eevent * ev);

static void 
e_scroller_timer(int val, void *data)
{
   Evas_List l;
   int ok = 0;
   int resist = 5;
   int scroll_speed = 12;
   static double last_time = 0.0;
   double t;
   /* these two lines... */
   E_CFG_INT(cfg_resist, "settings", "/menu/scroll/resist", 5);
   E_CFG_INT(cfg_scroll_speed, "settings", "/menu/scroll/speed", 12);
   
   /* and these 2 should do exactly what tom wants - see e.h */
   E_CONFIG_INT_GET(cfg_resist, resist);
   E_CONFIG_INT_GET(cfg_scroll_speed, scroll_speed);
   
   t = e_get_time();
   if (val != 0)
     scroll_speed = (int)(((t - last_time) / 0.02) * (double)scroll_speed);
   last_time = t;
   
   ok = 0;   
   if (mouse_x >= (screen_w - resist))
     {
	int scroll = 0;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if ((m->current.x + m->current.w) > screen_w)
	       scroll = m->current.x + m->current.w - screen_w;
	  }
	if (scroll)
	  {
	     if (scroll > scroll_speed) scroll = scroll_speed;
	     e_menu_scroll_all_by(-scroll, 0);
	     ok = 1;
	  }
     }
   else if (mouse_x < resist)
     {
	int scroll = 0;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if (m->current.x < 0)
	       scroll = -m->current.x;
	  }
	if (scroll)
	  {
	     if (scroll > scroll_speed) scroll = scroll_speed;
	     e_menu_scroll_all_by(scroll, 0);
	     ok = 1;
	  }
     }
   if (mouse_y >= (screen_h - resist))
     {
	int scroll = 0;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if ((m->current.y + m->current.h) > screen_h)
	       scroll = m->current.y + m->current.h - screen_h;
	  }
	if (scroll)
	  {
	     if (scroll > scroll_speed) scroll = scroll_speed;
	     e_menu_scroll_all_by(0, -scroll);
	     ok = 1;
	  }
     }
   else if (mouse_y < resist)
     {
	int scroll = 0;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if (m->current.y < 0)
	       scroll = -m->current.y;
	  }
	if (scroll)
	  {
	     if (scroll > scroll_speed) scroll = scroll_speed;
	     e_menu_scroll_all_by(0, scroll);
	     ok = 1;
	  }
     }
   if ((ok) && (open_menus))
     e_add_event_timer("menu_scroller", 0.02, e_scroller_timer, val + 1, NULL);   
   return;
   UN(data);
}
  
static void
e_idle(void *data)
{
   Evas_List l;
   
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	e_menu_update_base(m);
     }
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	e_menu_update_shows(m);
     }
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	e_menu_update_hides(m);
     }
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	e_menu_update_finish(m);
     }
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	if (m->first_expose)
	  evas_render(m->evas);
     }
   e_db_flush();
   return;
   UN(data);
}

static void
e_wheel(Eevent * ev)
{
   Ev_Wheel           *e;
   
   e = ev->event;
   if (e->win == menu_event_win)
     {
     }
}

static void
e_key_down(Eevent * ev)
{
   Ev_Key_Down          *e;
   int ok;
   
   e = ev->event;
   ok = 0;
   if (e->win == menu_event_win) ok = 1;
   else
     {
	Evas_List l;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if ((e->win == m->win.main) || (e->win == m->win.evas)) 
	       {
		  ok = 1;
		  break;
	       }
	  }
     }
   if (ok)
     {
	Evas_List l;
	E_Menu *m = NULL;
	E_Menu_Item *mi = NULL;
	
	for (l = open_menus; l; l = l->next)
	  {
	     m = l->data;
	     if (m->selected)
	       {
		  mi = m->selected;
		  break;
	       }
	  }
	if (!strcmp(e->key, "Up"))
	  {
	     keyboard_nav = 1;
	     e_menu_select(0, -1);
	  }
	else if (!strcmp(e->key, "Down"))
	  {
	     keyboard_nav = 1;
	     e_menu_select(0, 1);
	  }
	else if (!strcmp(e->key, "Left"))
	  {
	     keyboard_nav = 1;
	     e_menu_select(-1, 0);
	  }
	else if (!strcmp(e->key, "Right"))
	  {
	     keyboard_nav = 1;
	     e_menu_select(1, 0);
	  }
	else if (!strcmp(e->key, "Escape"))
	  {
	     keyboard_nav = 1;
	     for (l = menus; l; l = l->next)
	       {
		  m = l->data;
		  
		  if (m->current.visible)
		    e_menu_hide(m);
	       }
	  }
	else if (!strcmp(e->key, "Return"))
	  {
	     keyboard_nav = 1;
	     if (mi)
	       {
		  e_menu_callback_item(m, mi);
		  mi->selected = 0;
		  mi->menu->selected = NULL;
	       }
	     for (l = menus; l; l = l->next)
	       {
		  m = l->data;
		  
		  if (m->current.visible)
		    e_menu_hide(m);
	       }
	  }
	else
	  {
	  }
     }
}

static void
e_key_up(Eevent * ev)
{
   Ev_Key_Up          *e;
   
   e = ev->event;
     {
     }
}

/* handling mouse down events */
static void
e_mouse_down(Eevent * ev)
{
   Ev_Mouse_Down      *e;
   
   e = ev->event;
     {
     }
}

/* handling mouse up events */
static void
e_mouse_up(Eevent * ev)
{
   Ev_Mouse_Up      *e;
   
   e = ev->event;
   keyboard_nav = 0;
   if (e->win == menu_event_win)
     {
	if (open_menus)
	  {
	     E_Menu *m;
	     
	     m = open_menus->data;
	     if ((e->time - m->time) > 200)
	       {
		  Evas_List l;
		  
		  for (l = open_menus; l; l = l->next)
		    {
		       m = l->data;
		       if (m->selected)
			 {
			    e_menu_callback_item(m, m->selected);
			    m->selected->selected = 0;
			    m->selected = NULL;
			    break;
			 }
		    }
		  for (l = menus; l; l = l->next)
		    {
		       m = l->data;
		       
		       if (m->current.visible)
			 e_menu_hide(m);
		    }
	       }
	  }
     }
}

/* handling mouse move events */
static void
e_mouse_move(Eevent * ev)
{
   Ev_Mouse_Move      *e;
   
   e = ev->event;
   keyboard_nav = 0;
   if (e->win == menu_event_win)
     {
	Evas_List l;
	
	mouse_x = e->rx;
	mouse_y = e->ry;
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     evas_event_move(m->evas, 
			     e->rx - m->current.x, 
			     e->ry - m->current.y);
	  }
     }
   else
     {
	Evas_List l;
		
	mouse_x = e->rx;
	mouse_y = e->ry;
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     
             evas_event_move(m->evas,
			     e->rx - m->current.x,
			     e->ry - m->current.y);
	  }
     }
   e_scroller_timer(0, NULL);
}

/* handling mouse enter events */
static void
e_mouse_in(Eevent * ev)
{
   Ev_Window_Enter      *e;
   
   e = ev->event;
   keyboard_nav = 0;
   if (e->win == menu_event_win)
     {
     }
}

/* handling mouse leave events */
static void
e_mouse_out(Eevent * ev)
{
   Ev_Window_Leave      *e;
   
   e = ev->event;
   keyboard_nav = 0;
   if (e->win == menu_event_win)
     {
     }
   else
     {
        Evas_List l;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if ((e->win == m->win.main) || (e->win == m->win.evas))
	       {
		  evas_event_move(m->evas, -99999999, -99999999);
	       }
	  }
     }
}

/* handling expose events */
static void
e_window_expose(Eevent * ev)
{
   Ev_Window_Expose      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     if (e->win == m->win.evas)
	       {
		  m->first_expose = 1;
		  evas_update_rect(m->evas, e->x, e->y, e->w, e->h);
		  break;
	       }
	  }
     }
}

static void 
e_menu_item_in_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   E_Menu_Item *mi;

   mi = _data;
   mi->menu->selected = mi;
   mi->selected = 1;
   mi->menu->redo_sel = 1;
   mi->menu->changed = 1;
   e_menu_hide_submenus(mi->menu);
   if (mi->submenu)
     {
	e_menu_move_to(mi->submenu, 
		       mi->menu->current.x + mi->menu->current.w,
		       mi->menu->current.y + mi->y - mi->menu->border.t);
	e_menu_show(mi->submenu);
     }
   return;
   UN(_e);
   UN(_o);
   UN(_b);
   UN(_x);
   UN(_y);
}

static void 
e_menu_item_out_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   E_Menu_Item *mi;
   
   mi = _data;
   if (mi->menu->selected == mi) mi->menu->selected = NULL;
   mi->selected = 0;
   mi->menu->redo_sel = 1;
   mi->menu->changed = 1;
   return;
   UN(_e);
   UN(_o);
   UN(_b);
   UN(_x);
   UN(_y);
}

void
e_menu_callback_item(E_Menu *m, E_Menu_Item *mi)
{
   if (mi->func_select) mi->func_select(m, mi, mi->func_select_data);
}

void
e_menu_item_set_callback(E_Menu_Item *mi, void (*func) (E_Menu *m, E_Menu_Item *mi, void *data), void *data)
{
   mi->func_select = func;
   mi->func_select_data = data;
}

void
e_menu_hide_submenus(E_Menu *menus_after)
{
   Evas_List l;
   
   for (l = open_menus; l; l = l->next)
     {
	if (l->data == menus_after)
	  {
	     l = l->next;
	     for (; l; l = l->next)
	       {
		  E_Menu *m;
		  
		  m = l->data;
		  e_menu_hide(m);
	       }
	     break;
	  }
     }
}

void
e_menu_select(int dx, int dy)
{
   Evas_List l, ll;
   int done = 0;
   
   for (l = open_menus; (l) && (!done); l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	if (m->selected)
	  {
	     for (ll = m->entries; (ll) && (!done); ll = ll->next)
	       {
		  E_Menu_Item *mi;
		  
		  mi = ll->data;
		  if (mi->selected)
		    {
		       if (dy != 0)
			 {
			    int ok = 0;
			    
			    if ((dy < 0) && (ll->prev)) ok = 1;
			    else if ((dy > 0) && (ll->next)) ok = 1;
			    if (ok)
			      {
				 if (m->selected)
				   {
				      m->selected->selected = 0;
				      m->redo_sel = 1;
				      m->changed = 1;
				      m->selected = NULL;
				   }
				 if (dy < 0) mi = ll->prev->data;
				 else mi = ll->next->data;
				 m->selected = mi;
				 mi->selected = 1;
				 mi->menu->redo_sel = 1;
				 mi->menu->changed = 1;
				 e_menu_hide_submenus(mi->menu);
				 if (mi->submenu)
				   {
				      e_menu_move_to(mi->submenu,
						     mi->menu->current.x + mi->menu->current.w,
						     mi->menu->current.y + mi->y - mi->menu->border.t);
				      e_menu_show(mi->submenu);
				   }
			      }
			 }
		       done = 1;
		    }
	       }
	     if (dx != 0)
	       {
		  int ok = 0;
		  
		  if ((dx < 0) && (l->prev)) ok = 1;
		  else if ((dx > 0) && (l->next)) ok = 1;
		  if (ok)
		    {
		       E_Menu_Item *mi = NULL;
		       E_Menu *mm;
		       
		       if (dx < 0) 
			 {
			    Evas_List ll;
			    
			    mm = l->prev->data;
			    for (ll = mm->entries; (ll) && (!mi); ll = ll->next)
			      {
				 E_Menu_Item *mmi;
				 
				 mmi = ll->data;
				 if (mmi->submenu == m) mi = mmi;
			      }
			 }
		       else 
			 {
			    mm = l->next->data;
			    if (mm->entries)
			      mi = mm->entries->data;
			 }
		       if (mi)
			 {
			    if (m->selected)
			      {
				 m->selected->selected = 0;
				 m->redo_sel = 1;
				 m->changed = 1;
				 m->selected = NULL;
			      }
			    mm->selected = mi;
			    mi->selected = 1;
			    mi->menu->redo_sel = 1;
			    mi->menu->changed = 1;
			    e_menu_hide_submenus(mi->menu);
			    if (mi->submenu)
			      {
				 e_menu_move_to(mi->submenu,
						mi->menu->current.x + mi->menu->current.w,
						mi->menu->current.y + mi->y - mi->menu->border.t);
				 e_menu_show(mi->submenu);
			      }
			    e_menu_update_visibility(mm);
			 }
		    }
		  done = 1;
	       }
	     e_menu_update_visibility(m);
	  }
     }
   if (!done)
     {
	if (open_menus)
	  {
	     E_Menu *m;
	     E_Menu_Item *mi;
	     
	     m = open_menus->data;
	     mi = m->entries->data;
	     m->selected = mi;
	     mi->selected = 1;
	     mi->menu->redo_sel = 1;
	     mi->menu->changed = 1;	     
	     if (mi->submenu)
	       {
		  e_menu_move_to(mi->submenu,
				 mi->menu->current.x + mi->menu->current.w,
				 mi->menu->current.y + mi->y - mi->menu->border.t);
		  e_menu_show(mi->submenu);
	       }
	  }
     }
}

void
e_menu_init(void)
{
   e_window_get_geometry(0, NULL, NULL, &screen_w, &screen_h);
   e_event_filter_handler_add(EV_MOUSE_DOWN,               e_mouse_down);
   e_event_filter_handler_add(EV_MOUSE_UP,                 e_mouse_up);
   e_event_filter_handler_add(EV_MOUSE_MOVE,               e_mouse_move);
   e_event_filter_handler_add(EV_MOUSE_IN,                 e_mouse_in);
   e_event_filter_handler_add(EV_MOUSE_OUT,                e_mouse_out);
   e_event_filter_handler_add(EV_WINDOW_EXPOSE,            e_window_expose);
   e_event_filter_handler_add(EV_KEY_DOWN,                 e_key_down);
   e_event_filter_handler_add(EV_KEY_UP,                   e_key_up);
   e_event_filter_handler_add(EV_MOUSE_WHEEL,              e_wheel);
   e_event_filter_idle_handler_add(e_idle, NULL);
}

void
e_menu_event_win_show(void)
{
   /* create it */
   if (!menu_event_win)
     {
	menu_event_win = e_window_input_new(0, 0, 0, screen_w, screen_h);
	e_window_set_events(menu_event_win, XEV_MOUSE_MOVE | XEV_BUTTON | XEV_IN_OUT | XEV_KEY);
	e_window_show(menu_event_win);
	e_keyboard_grab(menu_event_win);
	e_grab_mouse(menu_event_win, 1, 0);
     }
   /* raise it */
   if (menu_event_win) e_window_raise(menu_event_win);
}

void
e_menu_event_win_hide(void)
{
   /* destroy it */
   if (menu_event_win)
     {
	e_keyboard_ungrab();
	e_window_destroy(menu_event_win);
	menu_event_win = 0;
     }
}

void
e_menu_set_background(E_Menu *m)
{
   char *menus;
   char buf[4096];
   char *style = "default";
   char *part;
   int pl, pr, pt, pb;
   
   menus = e_config_get("menus");   
   
   part = "base.bits.db";
   sprintf(buf, "%s%s/%s", menus, style, part);
   if ((m->bg_file) && (!strcmp(m->bg_file, buf))) return;
   
   IF_FREE(m->bg_file);
   m->bg_file = strdup(buf);
   
   if (m->bg) ebits_free(m->bg);
   m->bg = ebits_load(m->bg_file);
   if (m->bg) ebits_set_color_class(m->bg, "Menu BG", 100, 200, 255, 255);
   
   pl = pr = pt = pb = 0;
   if (m->bg) 
     {
	ebits_get_insets(m->bg, &pl, &pr, &pt, &pb);
	ebits_add_to_evas(m->bg, m->evas);
	ebits_move(m->bg, 0, 0);
	ebits_show(m->bg);
	ebits_set_layer(m->bg, 0);
     }
   m->current.w += ((pl + pr) - (m->border.l + m->border.r));
   m->current.h += ((pt + pb) - (m->border.t + m->border.b));
   m->border.l = pl;
   m->border.r = pr;
   m->border.t = pt;
   m->border.b = pb;
   m->changed = 1;
}

void
e_menu_set_sel(E_Menu *m, E_Menu_Item *mi)
{
   char *menus;
   char buf[4096];
   char *style = "default";
   int pl, pr, pt, pb;
   int has_sub = 0;
   int selected = 0;
   
   menus = e_config_get("menus");   
   if (!mi->separator)
     {
	selected = mi->selected;
	if (mi->submenu) has_sub = 1;
	sprintf(buf, "%s%s/selected-%i.submenu-%i.bits.db", menus, style, 
		selected, has_sub);
	if ((mi->bg_file) && (!strcmp(mi->bg_file, buf))) return;
     }
   IF_FREE(mi->bg_file);
   if (!mi->separator)
     mi->bg_file = strdup(buf);
   else
     mi->bg_file = NULL;
   if (mi->bg) ebits_free(mi->bg);
   if (mi->bg_file)
     mi->bg = ebits_load(mi->bg_file);
   if (mi->bg) ebits_set_color_class(mi->bg, "Menu BG", 100, 200, 255, 255);
   
   pl = pr = pt = pb = 0;
   if (mi->bg) 
     {
	ebits_get_insets(mi->bg, &pl, &pr, &pt, &pb);
	ebits_add_to_evas(mi->bg, m->evas);
	ebits_set_layer(mi->bg, 1);
     }
   if (m->sel_border.l < pl) {m->sel_border.l = pl; m->recalc_entries = 1;}
   if (m->sel_border.r < pr) {m->sel_border.r = pr; m->recalc_entries = 1;}
   if (m->sel_border.t < pt) {m->sel_border.t = pt; m->recalc_entries = 1;}
   if (m->sel_border.b < pb) {m->sel_border.b = pb; m->recalc_entries = 1;}
   m->redo_sel = 1;
   m->changed = 1;
}

void
e_menu_set_sep(E_Menu *m, E_Menu_Item *mi)
{
   char *menus;
   char buf[4096];
   char *style = "default";
   int pl, pr, pt, pb, minx, miny;
   
   menus = e_config_get("menus");   
   sprintf(buf, "%s%s/separator.bits.db", menus, style);
   if ((mi->sep_file) && (!strcmp(mi->sep_file, buf))) return;
   
   IF_FREE(mi->sep_file);
   mi->sep_file = strdup(buf);
   
   if (mi->sep) ebits_free(mi->sep);
   mi->sep = ebits_load(mi->sep_file);
   if (mi->sep) ebits_set_color_class(mi->sep, "Menu BG", 100, 200, 255, 255);
   
   pl = pr = pt = pb = 0;
   minx = 0;
   miny = 0;
   if (mi->sep) 
     {
	ebits_get_insets(mi->sep, &pl, &pr, &pt, &pb);
	ebits_add_to_evas(mi->sep, m->evas);
	ebits_set_layer(mi->sep, 1);
	ebits_get_min_size(mi->sep, &minx, &miny);
     }
   if (mi->size.min.w < minx) mi->size.min.w = minx;
   if (mi->size.min.h < miny) mi->size.min.h = miny;
   m->redo_sel = 1;
   m->changed = 1;
}

void
e_menu_set_state(E_Menu *m, E_Menu_Item *mi)
{
   char *menus;
   char buf[4096];
   char *style = "default";
   int   on;
   int pl, pr, pt, pb, minx, miny;
   
   menus = e_config_get("menus");   
   on = mi->on;
   if (mi->check)
     sprintf(buf, "%s%s/check-%i.bits.db", menus, style, on);
   else
     sprintf(buf, "%s%s/radio-%i.bits.db", menus, style, on);
   if ((mi->state_file) && (!strcmp(mi->state_file, buf))) return;
   
   IF_FREE(mi->state_file);
   mi->state_file = strdup(buf);
   
   if (mi->state) ebits_free(mi->state);
   mi->state = ebits_load(mi->state_file);
   if (mi->state) ebits_set_color_class(mi->state, "Menu BG", 100, 200, 255, 255);
   
   pl = pr = pt = pb = 0;
   minx = 0;
   miny = 0;
   if (mi->state) 
     {
	ebits_get_insets(mi->state, &pl, &pr, &pt, &pb);
	ebits_add_to_evas(mi->state, m->evas);
	ebits_set_layer(mi->state, 2);
	ebits_get_min_size(mi->state, &minx, &miny);
     }
   if (mi->size.min.w < minx) mi->size.min.w = minx;
   if (mi->size.min.h < miny) mi->size.min.h = miny;
   m->redo_sel = 1;
   m->changed = 1;
}

void
e_menu_free(E_Menu *m)
{
   Evas_List l;
   
   for (l = m->entries; l; l = l->next)
     {
	E_Menu_Item *mi;
	
	mi = l->data;
	e_menu_item_unrealize(m, mi);
	IF_FREE(mi->str);
	IF_FREE(mi->icon);
	free(mi);
     }
   evas_list_free(m->entries);
   IF_FREE(m->bg_file);
   evas_free(m->evas);
   e_del_child(m->win.main, m->win.evas);
   e_window_destroy(m->win.main);
   menus = evas_list_remove(menus, m);
   open_menus = evas_list_remove(open_menus, m);
   free(m);
}

E_Menu *
e_menu_new(void)
{
   E_Menu *m;
   int max_colors = 216;
   int font_cache = 1024 * 1024;
   int image_cache = 8192 * 1024;
   char *font_dir;
   
   font_dir = e_config_get("fonts");
   
   m = NEW(E_Menu, 1);
   ZERO(m, E_Menu, 1);
   
   OBJ_INIT(m, e_menu_free);
   
   m->win.main = e_window_override_new(0, 0, 0, 1, 1);
   m->evas = evas_new_all(e_display_get(),
			  m->win.main, 
			  0, 0, 1, 1,
			  RENDER_METHOD_ALPHA_SOFTWARE,
			  max_colors,
			  font_cache, 
			  image_cache,
			  font_dir);
   /* aaaaaaaaah. this makes building the menu fast - moves the mouse far */
   /* far far far far away so callbacks and events arent triggerd as we */
   /* create objects that ofter hang around 0,0 - the default place for */
   /* the pointer to be... this means my 2000 entry menu works  and comes up */
   /* pretty damn fast - considering i creating it when i click :) - problem */
   /* you can't fit 2000 entires into a window in X - since the limit is */
   /* 65536x65536 fo X - the other problem is i can only really draw in */
   /* the first 32768x32768 pixels of the window - thus limiting the menu */
   /* size to well - 32768x32768 - normally ok - but in extremes not so */
   /* good. We *COULD* do a workaround that meant we did the menu scrolling */
   /* within the evas and faked a menu window that only gets as big as the */
   /* screen - an then re-render it all - but well.. it's an extreme and */
   /* for now i think people will just have to live with a maximum menu size */
   /* of 32768x32768... didums! */
   evas_event_move(m->evas, -999999999, -99999999);
   
   m->win.evas = evas_get_window(m->evas);
   e_window_set_events(m->win.evas, XEV_EXPOSE | XEV_MOUSE_MOVE | XEV_BUTTON | XEV_IN_OUT | XEV_KEY);
   e_window_set_events(m->win.main, XEV_IN_OUT | XEV_KEY);
   e_window_show(m->win.evas);
   e_add_child(m->win.main, m->win.evas);
   
   e_menu_set_background(m);
   
   m->current.w = m->border.l + m->border.r;
   m->current.h = m->border.t + m->border.b;
   m->changed = 1;
   
   menus = evas_list_prepend(menus, m);
   
   return m;
}

void
e_menu_hide(E_Menu *m)
{
   m->current.visible = 0;
   m->changed = 1;
}

void
e_menu_show(E_Menu *m)
{
   m->current.visible = 1;
   m->changed = 1;
}

void
e_menu_move_to(E_Menu *m, int x, int y)
{
   m->current.x = x;
   m->current.y = y;
   m->changed = 1;
}

void
e_menu_show_at_mouse(E_Menu *m, int x, int y, Time t)
{
   m->current.x = x;
   m->current.y = y;
   m->time = t;
   e_menu_show(m);
}

void
e_menu_add_item(E_Menu *m, E_Menu_Item *mi)
{
   m->entries = evas_list_append(m->entries, mi);
   m->recalc_entries = 1;
   m->changed = 1;
   mi->menu = m;
   e_menu_item_realize(m, mi);
}

void
e_menu_del_item(E_Menu *m, E_Menu_Item *mi)
{
   m->entries = evas_list_remove(m->entries, mi);
   m->recalc_entries = 1;
   m->changed = 1;
   e_menu_item_unrealize(m, mi);
   IF_FREE(mi->str);
   IF_FREE(mi->icon);
   if (mi->menu->selected == mi) mi->menu->selected = NULL;
   free(mi);
   mi->menu = NULL;
}

void
e_menu_item_update(E_Menu *m, E_Menu_Item *mi)
{
   int tx, ty, tw, th, ix, iy, iw, ih, rx, ry, rw, rh;
   double dtw, dth;
   
   if (mi->sep)
     {
	ebits_move(mi->sep, mi->x, mi->y);
	ebits_resize(mi->sep, mi->size.w + m->sel_border.l + m->sel_border.r, mi->size.h);
	ebits_show(mi->sep);
     }
   else
     {
	rx = 0; ry = 0; rh = 0;
	rw = m->size.state;
	if (mi->state) 
	  {
	     ebits_get_min_size(mi->state, &rw, &rh);
	     rx = 0;
	     ry = ((mi->size.h - rh) / 2);
	     ebits_move(mi->state, m->sel_border.l + mi->x + rx, m->sel_border.t + mi->y + ry);
	     ebits_resize(mi->state, rw, rh);
	  }
	
	tx = 0; ty = 0; tw = 0; th = 0;
	if (mi->obj_text) 
	  {
	     evas_get_geometry(m->evas, mi->obj_text, NULL, NULL, &dtw, &dth);
	     tw = (int)dtw; th = (int)dth;
	  }
	
	ix = 0; iy = 0; iw = 0; ih = 0;
	if (mi->obj_icon) 
	  {
	     int sh;
	     
	     evas_get_image_size(m->evas, mi->obj_icon, &iw, &ih);
	     sh = th;
	     if (rh > th) sh = rh;
	     if ((mi->scale_icon) && (ih > sh) && (mi->str))
	       {
		  iw = (iw * sh) / ih;
		  ih = sh;
	       }
	     if (m->size.state) ix = rx + m->size.state + m->pad.state;
	     ix += ((m->size.icon - iw) / 2);
	     iy = ((mi->size.h - ih) / 2);
	     evas_move(m->evas, mi->obj_icon, m->sel_border.l + mi->x + ix, m->sel_border.t + mi->y + iy);
	     evas_resize(m->evas, mi->obj_icon, iw, ih);
	     evas_set_image_fill(m->evas, mi->obj_icon, 0, 0, iw, ih);
	  }
	
	if (mi->obj_text) 
	  {
	     if (m->size.state) tx = rx + m->size.state + m->pad.state;	     
	     if (m->size.icon) tx += m->size.icon + m->pad.icon;	     
	     ty = ((mi->size.h - th) / 2);
	     evas_move(m->evas, mi->obj_text, m->sel_border.l + mi->x + tx, m->sel_border.t + mi->y + ty);
	  }
	
	if (mi->obj_entry)
	  {
	     evas_move(m->evas, mi->obj_entry, mi->x, mi->y);
	     evas_resize(m->evas, mi->obj_entry, mi->size.w + m->sel_border.l + m->sel_border.r, mi->size.h + m->sel_border.t + m->sel_border.b);
	  }
	if (mi->state)
	  {
	     ebits_show(mi->state);
	  }
     }
}

void
e_menu_item_unrealize(E_Menu *m, E_Menu_Item *mi)
{
   if (mi->bg) ebits_free(mi->bg);
   mi->bg = NULL;
   IF_FREE(mi->bg_file);
   mi->bg_file = NULL;
   if (mi->obj_entry) evas_del_object(m->evas, mi->obj_text);
   mi->obj_entry = NULL;
   if (mi->obj_icon) evas_del_object(m->evas, mi->obj_icon);
   mi->obj_icon = NULL;
   if (mi->state) ebits_free(mi->state);
   mi->state = NULL;
   IF_FREE(mi->state_file);
   mi->state_file = NULL;
   if (mi->sep) ebits_free(mi->sep);
   mi->sep = NULL;
   IF_FREE(mi->sep_file);
   mi->sep_file = NULL;
}

void
e_menu_item_realize(E_Menu *m, E_Menu_Item *mi)
{
   double tw, th;
   int iw, ih, rw, rh;

   if (mi->separator)
     {
	e_menu_set_sep(m, mi);
     }
   else
     {
	if (mi->str) 
	  {
	     mi->obj_text = evas_add_text(m->evas, "borzoib", 8, mi->str);
	     evas_set_color(m->evas, mi->obj_text, 0, 0, 0, 255);
	     evas_show(m->evas, mi->obj_text);
	     evas_set_layer(m->evas, mi->obj_text, 10);
	  }
	if (mi->icon)
	  {
	     mi->obj_icon = evas_add_image_from_file(m->evas, mi->icon);
	     evas_show(m->evas, mi->obj_icon);
	     evas_set_layer(m->evas, mi->obj_icon, 10);
	  }	
	mi->obj_entry = evas_add_rectangle(m->evas);
	evas_set_layer(m->evas, mi->obj_entry, 11);
	evas_set_color(m->evas, mi->obj_entry, 0, 0, 0, 0);
	evas_show(m->evas, mi->obj_entry);
	tw = 0; th = 0;
	if (mi->obj_text) evas_get_geometry(m->evas, mi->obj_text, NULL, NULL, &tw, &th);
	iw = 0; ih = 0;
	if (mi->obj_icon) evas_get_image_size(m->evas, mi->obj_icon, &iw, &ih);
	rw = 0; rh = 0;
	if (mi->state) ebits_get_min_size(mi->state, &rw, &rh);
	mi->size.min.w = (int)tw + rw;
	if (rh > th) th = (double)rh;
	if (((!mi->scale_icon) && (ih > th)) ||
	    ((!mi->str) && (ih > th))) th = (double)ih;
	mi->size.min.h = (int)th;
	evas_callback_add(m->evas, mi->obj_entry, CALLBACK_MOUSE_IN, e_menu_item_in_cb, mi);
	evas_callback_add(m->evas, mi->obj_entry, CALLBACK_MOUSE_OUT, e_menu_item_out_cb, mi);
	e_menu_set_sel(m, mi);
	if ((mi->radio) || (mi->check)) e_menu_set_state(m, mi);
     }
}

E_Menu_Item *
e_menu_item_new(char *str)
{
   E_Menu_Item *mi;
   
   mi = NEW(E_Menu_Item, 1);
   ZERO(mi, E_Menu_Item, 1);

   if (str) mi->str = strdup(str);
   
   return mi;
}

void
e_menu_obscure_outside_screen(E_Menu *m)
{
   /* obscure stuff outside the screen boundaries - optimizes rendering */
   evas_clear_obscured_rects(m->evas);
   evas_add_obscured_rect(m->evas, 
			  -m->current.x - 100000,
			  -m->current.y - 100000,
			  200000 + screen_w, 100000);
   evas_add_obscured_rect(m->evas, 
			  -m->current.x - 100000,
			  -m->current.y - 100000,
			  100000, 200000 + screen_h);
   evas_add_obscured_rect(m->evas, 
			  -m->current.x - 100000,
			  screen_h - m->current.y,
			  200000 + screen_w, 100000);
   evas_add_obscured_rect(m->evas, 
			  screen_w - m->current.x,
			  -m->current.y - 100000,
			  100000, 200000 + screen_h);
}

void
e_menu_scroll_all_by(int dx, int dy)
{
   Evas_List l;
   
   for (l = menus; l; l = l->next)
     {
	E_Menu *m;
	
	m = l->data;
	if (m->current.visible)
	  {
	     m->current.x += dx;
	     m->current.y += dy;
	     m->changed = 1;
	  }
     }
   if (!keyboard_nav)
     {
	for (l = open_menus; l; l = l->next)
	  {
	     E_Menu *m;
	     
	     m = l->data;
	     evas_event_move(m->evas, 
			     mouse_x - m->current.x, 
			     mouse_y - m->current.y);
	  }
     }
}

void
e_menu_update_visibility(E_Menu *m)
{
   E_Menu_Item *mi;
	
   mi = m->selected;
   if (mi)
     {
	/* if the entry is off screen - scroll so it's on screen */
	if (m->current.x < 0)
	  e_menu_scroll_all_by(-m->current.x, 0);
	else if ((m->current.x + m->current.w) > screen_w)
	  {
	     e_menu_scroll_all_by(screen_w - (m->current.x + m->current.w), 0);
	  }
	if ((m->current.y + m->sel_border.t + mi->y) < 0)
	  {
	     if (m->current.y < -(screen_h / 4))
	       e_menu_scroll_all_by(0, screen_h / 4);
	     else
	       e_menu_scroll_all_by(0, - m->current.y);
	  }
	else if ((m->current.y + m->sel_border.t + mi->y + mi->size.h) > screen_h)
	  {
	     if ((m->current.y + m->current.h - screen_h) < (screen_h / 4))
	       e_menu_scroll_all_by(0, -(m->current.y + m->current.h - screen_h));
	     else
	       e_menu_scroll_all_by(0, -(screen_h / 4));
	  }
     }
}

void
e_menu_update_base(E_Menu *m)
{
   int size_changed = 0;
   int location_changed = 0;

   if (!m->changed) return;
   
   if (m->recalc_entries)
     {
	Evas_List l;
	int max_w, max_h;
	int i;
	
	max_w = 0;
	max_h = 0;
	for (l = m->entries; l; l = l->next)
	  {
	     E_Menu_Item *mi;
	     
	     mi = l->data;
	     if (mi->size.min.h > max_h) max_h = mi->size.min.h;
	  }
	m->size.state = 0;
	m->size.icon = 0;
	m->size.text = 0;
	for (l = m->entries; l; l = l->next)
	  {
	     E_Menu_Item *mi;
	     int iw, ih, rw, rh;
	     double tw, th;
	     
	     mi = l->data;
	     if (!mi->separator)
	       {
		  tw = 0; th = 0;
		  if (mi->obj_text) evas_get_geometry(m->evas, mi->obj_text, NULL, NULL, &tw, &th);
		  iw = 0; ih = 0;
		  if (mi->obj_icon) evas_get_image_size(m->evas, mi->obj_icon, &iw, &ih);
		  rw = 0; rh = 0;
		  if (mi->state) ebits_get_min_size(mi->state, &rw, &rh);
		  if (m->size.text < tw) m->size.text = tw;
		  if (m->size.state < rw) m->size.state = rw;
		  if ((mi->scale_icon) && (iw > 0) && (ih > 0) && (mi->str))
		    {
		       int iiw;
		       
		       iiw = iw;
		       if (ih > (int)th) iiw = (iw * (int)th) / ih;
		       if (m->size.icon < iiw) m->size.icon = iiw;
		    }
		  else if (m->size.icon < iw) m->size.icon = iw;	
	       }
	  }
	max_w = m->size.state;
	if (m->size.state) max_w += m->pad.state;
	max_w += m->size.icon;
	if (m->size.icon) max_w += m->pad.icon;
	max_w += m->size.text;
		
	i = m->border.t;
	for (l = m->entries; l; l = l->next)
	  {
	     E_Menu_Item *mi;
	     
	     mi = l->data;
	     mi->size.w = max_w;
	     if (mi->separator) mi->size.h = mi->size.min.h;
	     else               mi->size.h = max_h;
	     mi->x = m->border.l;
	     mi->y = i;
	     if (!mi->separator)
	       i += m->sel_border.t + m->sel_border.b;
	     if (mi->separator)
	       i += mi->size.h;
	     else
	       i += max_h;
	     e_menu_item_update(m, mi);
	  }
	m->current.w = m->border.l + m->border.r + max_w + m->sel_border.l + m->sel_border.r;
	m->current.h = m->border.b + i;
	
	m->recalc_entries = 0;
     }
   if (m->redo_sel)
     {
	Evas_List l;
	
	for (l = m->entries; l; l = l->next)
	  {
	     E_Menu_Item *mi;
	
	     mi = l->data;
	     e_menu_set_sel(m, mi);
	     if (mi)
	       {
		  if (mi->bg)
		    {
		       ebits_move(mi->bg, mi->x, mi->y);
		       ebits_resize(mi->bg, 
				    mi->size.w + m->sel_border.l + m->sel_border.r, 
				    mi->size.h + m->sel_border.t + m->sel_border.b);
		       ebits_show(mi->bg);
		    }
	       }
	  }
	m->redo_sel = 0;
     }
   
   if ((m->current.x != m->previous.x) ||
       (m->current.y != m->previous.y))
     location_changed = 1;
   if ((m->current.w != m->previous.w) ||
       (m->current.h != m->previous.h))
     size_changed = 1;
   
   if ((location_changed) && (size_changed))
     {
	e_window_move_resize(m->win.main, m->current.x, m->current.y, m->current.w, m->current.h);
	e_menu_obscure_outside_screen(m);
     }
   else if (location_changed)
     {
	e_window_move(m->win.main, m->current.x, m->current.y);
	e_menu_obscure_outside_screen(m);
     }
   else if (size_changed)
     {
	e_window_resize(m->win.main, m->current.w, m->current.h);
     }
   if (size_changed)
     {
	e_window_resize(m->win.evas, m->current.w, m->current.h);
	evas_set_output_size(m->evas, m->current.w, m->current.h);
	evas_set_output_viewport(m->evas, 0, 0, m->current.w, m->current.h);
	if (m->bg) ebits_resize(m->bg, m->current.w, m->current.h);
     }
}

void
e_menu_update_finish(E_Menu *m)
{
   if (!m->changed) return;
   m->previous = m->current;
   m->changed = 0;
}

void
e_menu_update_shows(E_Menu *m)
{
   if (!m->changed) return;
   if (m->current.visible != m->previous.visible)
     {
	if (m->current.visible) 
	  {
	     e_window_raise(m->win.main);
	     e_menu_event_win_show();
	     e_window_show(m->win.main);
	     if (!open_menus) keyboard_nav = 0;
	     open_menus = evas_list_append(open_menus, m);
	  }
     }
}

void
e_menu_update_hides(E_Menu *m)
{
   if (!m->changed) return;
   if (m->current.visible != m->previous.visible)
     {
	if (!m->current.visible)
	  {
	     if (m->selected)
	       {
		  E_Menu_Item *mi;
		  
		  mi = m->selected;
		  mi->selected = 0;
		  e_menu_set_sel(m, mi);
		  if (mi)
		    {
		       if (mi->bg)
			 {
			    ebits_move(mi->bg, mi->x, mi->y);
			    ebits_resize(mi->bg, 
					 mi->size.w + m->sel_border.l + m->sel_border.r, 
					 mi->size.h + m->sel_border.t + m->sel_border.b);
			    ebits_show(mi->bg);
			 }
		    }
		  m->redo_sel = 1;
		  m->changed = 1;
		  m->selected = NULL;
	       }
	     open_menus = evas_list_remove(open_menus, m);
	     e_window_hide(m->win.main);
	     if (!open_menus) e_menu_event_win_hide();
	  }
     }
}

void
e_menu_update(E_Menu *m)
{
   e_menu_update_base(m);
   e_menu_update_shows(m);
   e_menu_update_hides(m);
   e_menu_update_finish(m);
}

void
e_menu_item_set_icon(E_Menu_Item *mi, char *icon)
{
   IF_FREE(mi->icon);
   mi->icon = NULL;
   if (icon) mi->icon = strdup(icon);
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_text(E_Menu_Item *mi, char *text)
{
   IF_FREE(mi->str);
   mi->str = NULL;
   if (text) mi->str = strdup(text);
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_separator(E_Menu_Item *mi, int sep)
{
   mi->separator = sep;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_radio(E_Menu_Item *mi, int radio)
{
   mi->radio = radio;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_check(E_Menu_Item *mi, int check)
{
   mi->check = check;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_state(E_Menu_Item *mi, int state)
{
   mi->on = state;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->redo_sel = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_submenu(E_Menu_Item *mi, E_Menu *submenu)
{
   if (mi->submenu) e_menu_hide(mi->submenu);
   mi->submenu = submenu;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->redo_sel = 1;
	mi->menu->changed = 1;
     }
}

void
e_menu_item_set_scale_icon(E_Menu_Item *mi, int scale)
{
   mi->scale_icon = scale;
   if (mi->menu) 
     {
	mi->menu->recalc_entries = 1;
	mi->menu->changed = 1;
     }   
}

void
e_menu_set_padding_icon(E_Menu *m, int pad)
{
   m->pad.icon = pad;
   m->recalc_entries = 1;
   m->changed = 1;
}

void
e_menu_set_padding_state(E_Menu *m, int pad)
{
   m->pad.state = pad;
   m->recalc_entries = 1;
   m->changed = 1;
}
