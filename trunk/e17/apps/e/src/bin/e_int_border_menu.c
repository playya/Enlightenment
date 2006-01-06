/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

static void _e_border_cb_border_menu_end(void *data, E_Menu *m);
static void _e_border_menu_cb_locks(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_remember(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_border(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_close(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_iconify(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_kill(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_maximize(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_maximize_verticaly(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_maximize_horizontaly(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_shade(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_icon_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_stick(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_on_top(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_normal(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_below(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_borderless(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_fullscreen(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_skip_winlist(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_sendto_pre(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_border_menu_cb_sendto(void *data, E_Menu *m, E_Menu_Item *mi);

void
e_int_border_menu_show(E_Border *bd, Evas_Coord x, Evas_Coord y, int key, Ecore_X_Time timestamp)
{
   E_Menu *m;
   E_Menu_Item *mi;

   if (bd->border_menu) return;
   
   m = e_menu_new();
   e_menu_category_set(m,"border/stacking");
   e_menu_category_data_set("border/stacking",bd);
   bd->border_stacking_menu = m;
   /* Only allow to change layer for windows in "normal" layers */
   if ((!bd->lock_user_stacking) &&
       ((bd->layer == 50) || (bd->layer == 100) || (bd->layer == 150)))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Always On Top"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	e_menu_item_toggle_set(mi, (bd->layer == 150 ? 1 : 0));
	e_menu_item_callback_set(mi, _e_border_menu_cb_on_top, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/stack_on_top"),
				  "widgets/border/default/stack_on_top");
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Normal"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	e_menu_item_toggle_set(mi, (bd->layer == 100 ? 1 : 0));
	e_menu_item_callback_set(mi, _e_border_menu_cb_normal, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/stack_normal"),
				  "widgets/border/default/stack_normal");
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Always Below"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	e_menu_item_toggle_set(mi, (bd->layer == 50 ? 1 : 0));
	e_menu_item_callback_set(mi, _e_border_menu_cb_below, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/stack_below"),
				  "widgets/border/default/stack_below");
     }

   m = e_menu_new();
   e_menu_category_set(m,"border/maximize");
   e_menu_category_data_set("border/maximize",bd);
   bd->border_maximize_menu = m;
   /* Only allow to change layer for windows in "normal" layers */
   if ((!bd->lock_user_maximize) &&
       ((bd->layer == 50) || (bd->layer == 100) || (bd->layer == 150)))
   { 
     int __fullmaximization = 0;
     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized"));
     e_menu_item_check_set(mi, 1);
     //e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_toggle_set( mi, ( ( (__fullmaximization = bd->maximized &&
					bd->maximized != E_MAXIMIZE_VERTICAL &&
					bd->maximized != E_MAXIMIZE_HORIZONTAL)) ? 1 : 0 ));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize, bd);
     e_menu_item_icon_edje_set(mi,
       			(char *)e_theme_edje_file_get("base/theme/borders",
       						      "widgets/border/default/maximize"),
       			"widgets/border/default/maximize");

     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized verticaly"));
     e_menu_item_check_set(mi, 1);
     //e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_toggle_set( mi, ( ( (bd->maximized &&
				      bd->maximized == E_MAXIMIZE_VERTICAL &&
				      bd->maximized != E_MAXIMIZE_HORIZONTAL) ||
				     __fullmaximization ) ? 1 : 0 ));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize_verticaly, bd);
     e_menu_item_icon_edje_set(mi,
       			(char *)e_theme_edje_file_get("base/theme/borders",
       						      "widgets/border/default/maximize"),
       			"widgets/border/default/maximize");

     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized horizontaly"));
     e_menu_item_check_set(mi, 1);
     //e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_toggle_set( mi, ( ( (bd->maximized &&
				      bd->maximized != E_MAXIMIZE_VERTICAL &&
				      bd->maximized == E_MAXIMIZE_HORIZONTAL) ||
				     __fullmaximization ) ? 1 : 0 ));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize_horizontaly, bd);
     e_menu_item_icon_edje_set(mi,
				(char *)e_theme_edje_file_get("base/theme/borders",
							      "widgets/border/default/maximize"),
				"widgets/border/default/maximize");
   }

   m = e_menu_new();
   e_menu_category_set(m,"border");
   e_menu_category_data_set("border",bd);
   e_object_data_set(E_OBJECT(m), bd);
   bd->border_menu = m;
   e_menu_post_deactivate_callback_set(m, _e_border_cb_border_menu_end, NULL);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Window Locks"));
   e_menu_item_callback_set(mi, _e_border_menu_cb_locks, bd);
   e_menu_item_icon_edje_set(mi,
			     (char *)e_theme_edje_file_get("base/theme/borders",
							   "widgets/border/default/locks"),
			     "widgets/border/default/locks");
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Remember"));
   e_menu_item_callback_set(mi, _e_border_menu_cb_remember, bd);
   e_menu_item_icon_edje_set(mi,
			     (char *)e_theme_edje_file_get("base/theme/borders",
							   "widgets/border/default/remember"),
			     "widgets/border/default/remember");
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Borders"));
   e_menu_item_callback_set(mi, _e_border_menu_cb_border, bd);
   e_menu_item_icon_edje_set(mi,
			     (char *)e_theme_edje_file_get("base/theme/borders",
							   "widgets/border/default/borders"),
			     "widgets/border/default/borders");
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Send to Desktop"));
   e_menu_item_submenu_pre_callback_set(mi, _e_border_menu_cb_sendto_pre, bd);
   e_menu_item_icon_edje_set(mi,
			     (char *)e_theme_edje_file_get("base/theme/borders",
							   "widgets/border/default/sendto"),
			     "widgets/border/default/sendto");
   
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Stacking"));
   e_menu_item_submenu_set(mi, bd->border_stacking_menu);
   e_menu_item_icon_edje_set(mi,
			     (char *)e_theme_edje_file_get("base/theme/borders",
							   "widgets/border/default/stacking"),
			     "widgets/border/default/stacking");
   
   if (!(((bd->client.icccm.min_w == bd->client.icccm.max_w) &&
	  (bd->client.icccm.min_h == bd->client.icccm.max_h)) ||
	 (bd->lock_user_maximize)))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Maximize"));
	e_menu_item_submenu_set(mi, bd->border_maximize_menu);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/maximize"),
				  "widgets/border/default/maximize");
     }

   if ((!bd->lock_user_shade) && (!(!strcmp("borderless", bd->client.border.name))))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Shaded"));
	e_menu_item_check_set(mi, 1);
	e_menu_item_toggle_set(mi, (bd->shaded ? 1 : 0));
	e_menu_item_callback_set(mi, _e_border_menu_cb_shade, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/shade"),
				  "widgets/border/default/shade");
     }

   /*if (!bd->lock_user_maximize)
   {
     mi = e_menu_item_new(m);
     e_menu_item_separator_set(mi, 1);

     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized"));
     e_menu_item_check_set(mi, 1);
     e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize, bd);
     e_menu_item_icon_edje_set(mi,
       			(char *)e_theme_edje_file_get("base/theme/borders",
       						      "widgets/border/default/maximize"),
       			"widgets/border/default/maximize");

     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized verticaly"));
     e_menu_item_check_set(mi, 1);
     e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize, bd);
     e_menu_item_icon_edje_set(mi,
       			(char *)e_theme_edje_file_get("base/theme/borders",
       						      "widgets/border/default/maximize"),
       			"widgets/border/default/maximize");

     mi = e_menu_item_new(m);
     e_menu_item_label_set(mi, _("Maximized horizontaly"));
     e_menu_item_check_set(mi, 1);
     e_menu_item_toggle_set(mi, (bd->maximized ? 1 : 0));
     e_menu_item_callback_set(mi, _e_border_menu_cb_maximize, bd);
     e_menu_item_icon_edje_set(mi,
				(char *)e_theme_edje_file_get("base/theme/borders",
							      "widgets/border/default/maximize"),
				"widgets/border/default/maximize");

     mi = e_menu_item_new(m);
     e_menu_item_separator_set(mi, 1);
   }*/
   
   if (!bd->lock_user_sticky)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Sticky"));
	e_menu_item_check_set(mi, 1);
	e_menu_item_toggle_set(mi, (bd->sticky ? 1 : 0));
	e_menu_item_callback_set(mi, _e_border_menu_cb_stick, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/stick"),
				  "widgets/border/default/stick");
     }
  
   if ((!bd->shaded) && (!bd->fullscreen) && (!bd->lock_border))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Borderless"));
	e_menu_item_check_set(mi, 1);
	e_menu_item_toggle_set(mi, !strcmp("borderless", bd->client.border.name));
	e_menu_item_callback_set(mi, _e_border_menu_cb_borderless, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/borderless"),
				  "widgets/border/default/borderless");
     }
   
   if (!bd->lock_user_fullscreen)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Fullscreen"));
	e_menu_item_check_set(mi, 1);
	e_menu_item_toggle_set(mi, bd->fullscreen);
	e_menu_item_callback_set(mi, _e_border_menu_cb_fullscreen, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/fullscreen"),
				  "widgets/border/default/fullscreen");
     }

   if ((bd->client.icccm.accepts_focus || bd->client.icccm.take_focus) &&
       (!bd->client.netwm.state.skip_taskbar))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Skip Window List"));
	e_menu_item_check_set(mi, 1);
	e_menu_item_toggle_set(mi, bd->user_skip_winlist);
	e_menu_item_callback_set(mi, _e_border_menu_cb_skip_winlist, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/skip_winlist"),
				  "widgets/border/default/skip_winlist");
     }
   
   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);
   
   if (bd->app)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Edit Icon"));
	e_menu_item_callback_set(mi, _e_border_menu_cb_icon_edit, bd);
	e_menu_item_icon_edje_set(mi, bd->app->path, "icon");
     }
   else if (bd->client.icccm.class) /* icons with no class useless to borders */
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Create Icon"));
	e_menu_item_callback_set(mi, _e_border_menu_cb_icon_edit, bd);
     }

   mi = e_menu_item_new(m);
   e_menu_item_separator_set(mi, 1);

   if ((!bd->lock_close) && (!bd->internal))
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Kill"));
	e_menu_item_callback_set(mi, _e_border_menu_cb_kill, bd);
	e_menu_item_icon_edje_set(mi, 
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/kill"),
				  "widgets/border/default/kill");
	mi = e_menu_item_new(m);
	e_menu_item_separator_set(mi, 1);
     }
   
   if (!bd->lock_user_iconify)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Iconify"));
	e_menu_item_callback_set(mi, _e_border_menu_cb_iconify, bd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/minimize"),
				  "widgets/border/default/minimize");
     }
   
   if (!bd->lock_close)
     {
	mi = e_menu_item_new(m);
	e_menu_item_label_set(mi, _("Close"));
	e_menu_item_callback_set(mi, _e_border_menu_cb_close, bd);
	e_menu_item_icon_edje_set(mi, 
				  (char *)e_theme_edje_file_get("base/theme/borders",
								"widgets/border/default/close"), 
				  "widgets/border/default/close");
     }

   if (key)
     e_menu_activate_key(m, bd->zone, x, y, 1, 1,
			 E_MENU_POP_DIRECTION_DOWN);
   else
     e_menu_activate_mouse(m, bd->zone, x, y, 1, 1,
			   E_MENU_POP_DIRECTION_DOWN, timestamp);
}

void
e_int_border_menu_del(E_Border *bd)
{
   int was_menu = 0;
     
   if (bd->border_stacking_menu)
     {
	e_object_del(E_OBJECT(bd->border_stacking_menu));
	bd->border_stacking_menu = NULL;
	was_menu = 1;
     }
   if( bd->border_maximize_menu )
   {
     e_object_del(E_OBJECT(bd->border_maximize_menu));
     bd->border_maximize_menu = NULL;
     was_menu = 1;
   }
   if (bd->border_menu)
     {
	e_object_del(E_OBJECT(bd->border_menu));
	bd->border_menu = NULL;
	was_menu = 1;
     }
}

static void
_e_border_cb_border_menu_end(void *data, E_Menu *m)
{
   E_Border *bd;

   bd = e_object_data_get(E_OBJECT(m));
   if (bd)
     {
	/* If the border exists, delete all associated menus */
	e_int_border_menu_del(bd);
     }
   else
     {
	/* Just delete this menu */
	e_object_del(E_OBJECT(m));
     }
}

static void
_e_border_menu_cb_locks(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   
   bd = data;
   if (bd->border_locks_dialog) return;
   e_int_border_locks(bd);
}
					  
static void
_e_border_menu_cb_remember(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   bd = data;
   if (bd->border_remember_dialog) return;
   e_int_border_remember(bd);
}
   
static void
_e_border_menu_cb_border(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   bd = data;
   if (bd->border_border_dialog) return;
   e_int_border_border(bd);
}
   
static void
_e_border_menu_cb_close(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_close)
     e_border_act_close_begin(bd);
}

static void
_e_border_menu_cb_iconify(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_iconify)
     {
	if (bd->iconic) e_border_uniconify(bd);
	else e_border_iconify(bd);
     }
}

static void
_e_border_menu_cb_kill(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if ((!bd->lock_close) && (!bd->internal))
     e_border_act_kill_begin(bd);
}

static void
_e_border_menu_cb_maximize(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_maximize)
     {
	if (bd->maximized != E_MAXIMIZE_NONE &&
	    bd->maximized != E_MAXIMIZE_VERTICAL &&
	    bd->maximized != E_MAXIMIZE_HORIZONTAL ) e_border_unmaximize(bd);
	else e_border_maximize(bd, e_config->maximize_policy);
     }
}
/*** sndev : menu_cb_miximize_verticaly callback **************/
static void
_e_border_menu_cb_maximize_verticaly(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_maximize)
     {
	if (bd->maximized && bd->maximized != E_MAXIMIZE_HORIZONTAL ) 
	  e_border_unmaximize_vh(bd, E_MAXIMIZE_VERTICAL );
	else e_border_maximize(bd, E_MAXIMIZE_VERTICAL );
     }
}
/*** sndev : menu_cb_miximize_verticaly callback **************/
static void
_e_border_menu_cb_maximize_horizontaly(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_maximize)
     {
	if (bd->maximized && bd->maximized != E_MAXIMIZE_VERTICAL ) 
	  e_border_unmaximize_vh(bd, E_MAXIMIZE_HORIZONTAL );
	else e_border_maximize(bd, E_MAXIMIZE_HORIZONTAL );
     }
}
/*************************************************************/
static void
_e_border_menu_cb_shade(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_shade)
     {
	if (bd->shaded) e_border_unshade(bd, E_DIRECTION_UP);
	else e_border_shade(bd, E_DIRECTION_UP);
     }
}

static void
_e_border_menu_cb_icon_edit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_App *a;
   E_Border *bd;
   
   bd = data;
   a = bd->app;
   if ((!a) && (bd->client.icccm.class))
     {
	static char buf[PATH_MAX];
	char *name, *homedir, *p;
	int instance;

	name = alloca(strlen(bd->client.icccm.class) + 1);
	strcpy(name, bd->client.icccm.class);
	p = name;
	while (*p)
	  {
	     if (*p == ' ') *p = '_';
	     else if (*p == '/') *p = '_';
	     else if (*p == '.') *p = '_';
	     p++;
	  }
	homedir = e_user_homedir_get();
	snprintf(buf, sizeof(buf), "%s/.e/e/applications/all/%s.eap", homedir, name);
	instance = 0;
	while (ecore_file_exists(buf))
	  {
	     snprintf(buf, sizeof(buf), "%s/.e/e/applications/all/%s-%i.eap", homedir, name, instance);
	     instance++;
	  }
	free(homedir);
	a = e_app_empty_new(buf);
	if (a)
	  {
	     if (bd->client.icccm.name) a->win_name = evas_stringshare_add(bd->client.icccm.name);
	     if (bd->client.icccm.class) a->win_class = evas_stringshare_add(bd->client.icccm.class);
	     if (bd->client.icccm.window_role)
	       a->win_role = evas_stringshare_add(bd->client.icccm.window_role);
	     if (bd->client.icccm.class) a->icon_class = evas_stringshare_add(bd->client.icccm.class);
	     if (bd->client.icccm.class) a->name = evas_stringshare_add(bd->client.icccm.class);
	     if (bd->client.icccm.name) a->exe = evas_stringshare_add(bd->client.icccm.name);
	     if (bd->client.netwm.startup_id > 0)
	       a->startup_notify = 1;
	  }
     }
   if (!a) return;
   e_eap_edit_show(m->zone->container, a);
}

static void
_e_border_menu_cb_stick(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd->lock_user_sticky)
     {
	if (bd->sticky) e_border_unstick(bd);
	else e_border_stick(bd);
     }
}

static void
_e_border_menu_cb_on_top(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (bd->layer != 150)
     {
	e_border_layer_set(bd, 150);
	e_hints_window_stacking_set(bd, E_STACKING_ABOVE);
     }
}

static void
_e_border_menu_cb_below(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (bd->layer != 50)
     {
	e_border_layer_set(bd, 50);
	e_hints_window_stacking_set(bd, E_STACKING_BELOW);
     }
}

static void
_e_border_menu_cb_normal(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (bd->layer != 100)
     {
	e_border_layer_set(bd, 100);
	e_hints_window_stacking_set(bd, E_STACKING_NONE);
     }
}

static void
_e_border_menu_cb_borderless(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   int toggle;

   bd = data;
   if (!bd) return;
   
   if ((!bd->lock_border) && (!bd->shaded))
     {
	if (bd->client.border.name) evas_stringshare_del(bd->client.border.name);
	toggle = e_menu_item_toggle_get(mi);
	if (toggle)
	  bd->client.border.name = evas_stringshare_add("borderless");
	else
	  bd->client.border.name = evas_stringshare_add("default");
	bd->client.border.changed = 1;
	bd->changed = 1;
     }
}

static void
_e_border_menu_cb_fullscreen(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;
   int toggle;

   bd = data;
   if (!bd) return;
   
   if (!bd->lock_user_fullscreen)
     {
	toggle = e_menu_item_toggle_get(mi);
	if (toggle)
	  e_border_fullscreen(bd, e_config->fullscreen_policy);
	else
	  e_border_unfullscreen(bd);
     }
}

static void
_e_border_menu_cb_skip_winlist(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Border *bd;

   bd = data;
   if (!bd) return;
   
   if ((bd->client.icccm.accepts_focus || bd->client.icccm.take_focus) &&
       (!bd->client.netwm.state.skip_taskbar))
     bd->user_skip_winlist = e_menu_item_toggle_get(mi);
   else
     bd->user_skip_winlist = 0;
   if (bd->remember) e_remember_update(bd->remember, bd);
}

static void
_e_border_menu_cb_sendto_pre(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Menu *subm;
   E_Menu_Item *submi;
   E_Border *bd;
   int i;

   bd = data;

   subm = e_menu_new();
   e_object_data_set(E_OBJECT(subm), bd);
   e_menu_item_submenu_set(mi, subm);

   for (i = 0; i < bd->zone->desk_x_count * bd->zone->desk_y_count; i++)
     {
	E_Desk *desk;

	desk = bd->zone->desks[i];
	submi = e_menu_item_new(subm);
	e_menu_item_label_set(submi, desk->name);
	e_menu_item_callback_set(submi, _e_border_menu_cb_sendto, desk);
     }
}

static void
_e_border_menu_cb_sendto(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Desk *desk;
   E_Border *bd;

   desk = data;
   bd = e_object_data_get(E_OBJECT(m));
   if ((bd) && (desk))
     {
	e_border_desk_set(bd, desk);
     }
}
