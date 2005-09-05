/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#define INITS 
#define ACT_GO(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.go = _e_actions_act_##name##_go; \
   }
#define ACT_FN_GO(act) \
   static void _e_actions_act_##act##_go(E_Object *obj, char *params)
#define ACT_GO_MOUSE(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.go_mouse = _e_actions_act_##name##_go_mouse; \
   }
#define ACT_FN_GO_MOUSE(act) \
   static void _e_actions_act_##act##_go_mouse(E_Object *obj, char *params, Ecore_X_Event_Mouse_Button_Down *ev)
#define ACT_GO_KEY(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.go_key = _e_actions_act_##name##_go_key; \
   }
#define ACT_FN_GO_KEY(act) \
   static void _e_actions_act_##act##_go_key(E_Object *obj, char *params, Ecore_X_Event_Key_Down *ev)
#define ACT_END(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.end = _e_actions_act_##name##_end; \
   }
#define ACT_FN_END(act) \
   static void _e_actions_act_##act##_end(E_Object *obj, char *params)
#define ACT_END_MOUSE(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.end_mouse = _e_actions_act_##name##_end_mouse; \
   }
#define ACT_FN_END_MOUSE(act) \
   static void _e_actions_act_##act##_end_mouse(E_Object *obj, char *params, Ecore_X_Event_Mouse_Button_Up *ev)
#define ACT_END_KEY(name) \
   { \
      act = e_action_add(#name); \
      if (act) act->func.end_key = _e_actions_act_##name##_end_key; \
   }
#define ACT_FN_END_KEY(act) \
   static void _e_actions_act_##act##_end_key(E_Object *obj, char *params, Ecore_X_Event_Key_Up *ev)

/* local subsystem functions */
static void _e_action_free(E_Action *act);
static Evas_Bool _e_actions_cb_free(Evas_Hash *hash, const char *key, void *data, void *fdata);

/* to save writing this in N places - the sctions are defined here */
/***************************************************************************/
ACT_FN_GO(window_move)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   if (!((E_Border *)obj)->lock_user_location)
     e_border_act_move_begin((E_Border *)obj, NULL);
}
ACT_FN_GO_MOUSE(window_move)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   if (!((E_Border *)obj)->lock_user_location)
     e_border_act_move_begin((E_Border *)obj, ev);
}
ACT_FN_END(window_move)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   e_border_act_move_end((E_Border *)obj, NULL);
}
ACT_FN_END_MOUSE(window_move)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   e_border_act_move_end((E_Border *)obj, ev);
}

/***************************************************************************/
ACT_FN_GO(window_resize)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   if (!((E_Border *)obj)->lock_user_size)
     e_border_act_resize_begin((E_Border *)obj, NULL);
}
ACT_FN_GO_MOUSE(window_resize)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   if (!((E_Border *)obj)->lock_user_size)
     e_border_act_resize_begin((E_Border *)obj, ev);
}
ACT_FN_END(window_resize)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   e_border_act_resize_end((E_Border *)obj, NULL);
}
ACT_FN_END_MOUSE(window_resize)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE) return;
   e_border_act_resize_end((E_Border *)obj, ev);
}

/***************************************************************************/
ACT_FN_GO(window_menu)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   e_border_act_menu_begin((E_Border *)obj, NULL, 0);
}
ACT_FN_GO_MOUSE(window_menu)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   e_border_act_menu_begin((E_Border *)obj, ev, 0);
}
ACT_FN_GO_KEY(window_menu)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   e_border_act_menu_begin((E_Border *)obj, NULL, 1);
}

/***************************************************************************/
ACT_FN_GO(window_raise)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_stacking)
     e_border_raise((E_Border *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_lower)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_stacking)
     e_border_lower((E_Border *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_close)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_close)
     e_border_act_close_begin((E_Border *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_kill)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_close)
     e_border_act_kill_begin((E_Border *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_sticky_toggle)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_sticky)
     {
	E_Border *bd;
	
	bd = (E_Border *)obj;
	if (bd->sticky) e_border_unstick(bd);
	else e_border_stick(bd);
     }
}

/***************************************************************************/
ACT_FN_GO(window_iconic_toggle)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_iconify)
     {
	E_Border *bd;
	
	bd = (E_Border *)obj;
	if (bd->iconic) e_border_uniconify(bd);
	else e_border_iconify(bd);
     }
}

/***************************************************************************/
ACT_FN_GO(window_maximized_toggle)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_maximize)
     {
	E_Border *bd;
	
	bd = (E_Border *)obj;
	if (bd->maximized) e_border_unmaximize(bd);
	else e_border_maximize(bd, e_config->maximize_policy);
     }
}

/***************************************************************************/
ACT_FN_GO(window_shaded_toggle)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
	obj = E_OBJECT(e_border_focused_get());
	if (!obj) return;
     }
   if (!((E_Border *)obj)->lock_user_shade)
     {
	E_Border *bd;
	
	bd = (E_Border *)obj;
	if (bd->shaded) e_border_unshade(bd, E_DIRECTION_UP);
	else e_border_shade(bd, E_DIRECTION_UP);
     }
}

/***************************************************************************/
ACT_FN_GO(move_relative)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
       obj = E_OBJECT(e_border_focused_get());
       if (!obj) return;
     }
   if (params)
     {
	int dx, dy;

	if (sscanf(params, "%i %i", &dx, &dy) == 2)
	  {
	     E_Border *bd;

	     bd = (E_Border *)obj;

	     e_border_move(bd, bd->x + dx, bd->y + dy);

	     if (e_config->focus_policy != E_FOCUS_CLICK)
	       ecore_x_pointer_warp(bd->zone->container->win,
				    bd->x + (bd->w / 2),
				    bd->y + (bd->h / 2));
	}
     }
}

/***************************************************************************/
ACT_FN_GO(move_absolute)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
       obj = E_OBJECT(e_border_focused_get());
       if (!obj) return;
     }
   if (params)
     {
	E_Border *bd;
	int x, y;
	char cx, cy;

	bd = (E_Border *)obj;

	if (sscanf(params, "%c%i %c%i", &cx, &x, &cy, &y) == 4)
	  {
	     // Nothing, both x and y is updated.
	  }
	else if (sscanf(params, "* %c%i", &cy, &y) == 2)
	  {
	     // Updated y, reset x.
	     x = bd->x;
	  }
	else if (sscanf(params, "%c%i *", &cx, &x) == 2)
	  {
	     // Updated x, reset y.
	     y = bd->y;
	  }

	if (cx == '-') x = bd->zone->w - bd->w - x;
	if (cy == '-') y = bd->zone->h - bd->h - y;

	if ((x != bd->x) || (y != bd->y))
	  {
	     e_border_move(bd, x, y);

	     if (e_config->focus_policy != E_FOCUS_CLICK)
	       ecore_x_pointer_warp(bd->zone->container->win,
				    bd->x + (bd->w / 2),
				    bd->y + (bd->h / 2));
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(resize)
{
   if (!obj) obj = E_OBJECT(e_border_focused_get());
   if (!obj) return;
   if (obj->type != E_BORDER_TYPE)
     {
       obj = E_OBJECT(e_border_focused_get());
       if (!obj) return;
     }

   if (params)
     {
	int dw, dh;

	if (sscanf(params, "%i %i", &dw, &dh) == 2) {
	     E_Border *bd;
	     bd = (E_Border *)obj;

	     e_border_resize(bd, bd->w + dw, bd->h + dh);

	     if (e_config->focus_policy != E_FOCUS_CLICK)
	       ecore_x_pointer_warp(bd->zone->container->win,
				    bd->x + (bd->w / 2),
				    bd->y + (bd->h / 2));
	}
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_by)
{
   E_Zone *zone;
   
   if (!obj) return;
   if (obj->type != E_MANAGER_TYPE) return;
   zone = e_util_zone_current_get((E_Manager *)obj);
   if (zone)
     {
	if (params)
	  {
	     int dx = 0, dy = 0;
	
	     if (sscanf(params, "%i %i", &dx, &dy) == 2)
	       e_zone_desk_flip_by(zone, dx, dy);
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_to)
{
   E_Zone *zone;
   
   if (!obj) return;
   if (obj->type != E_MANAGER_TYPE) return;
   zone = e_util_zone_current_get((E_Manager *)obj);
   if (zone)
     {
	if (params)
	  {
	     int dx = 0, dy = 0;
	
	     if (sscanf(params, "%i %i", &dx, &dy) == 2)
	       e_zone_desk_flip_to(zone, dx, dy);
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_by)
{
   E_Zone *zone;
   
   if (!obj) return;
   if (obj->type != E_MANAGER_TYPE) return;
   zone = e_util_zone_current_get((E_Manager *)obj);
   if (zone)
     {
	if (params)
	  {
	     int dx = 0;
	
	     if (sscanf(params, "%i", &dx) == 1)
	       e_zone_desk_linear_flip_by(zone, dx);
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_to)
{
   E_Zone *zone;
   
   if (!obj) return;
   if (obj->type != E_MANAGER_TYPE) return;
   zone = e_util_zone_current_get((E_Manager *)obj);
   if (zone)
     {
	if (params)
	  {
	     int dx = 0;
	
	     if (sscanf(params, "%i", &dx) == 1)
	       e_zone_desk_linear_flip_to(zone, dx);
	  }
     }
}

#define ZONE_DESK_ACTION(con_num, zone_num, zone, act) \
E_Zone *zone; \
if ((con_num < 0) || (zone_num < 0)) { \
   Evas_List *l, *ll, *lll; \
   E_Container *con; \
   E_Manager *man; \
   if ((con_num >= 0) && (zone_num < 0)) /* con=1 zone=all */ { \
	con = e_util_container_number_get(con_num); \
	for (l = con->zones; l; l = l->next) { \
	   zone = l->data; \
	   act; \
	} } \
   else if ((con_num < 0) && (zone_num >= 0)) /* con=all zone=1 */ { \
	for (l = e_manager_list(); l; l = l->next) { \
	   man = l->data; \
	   for (ll = man->containers; ll; ll = ll->next) { \
	      con = ll->data; \
	      zone = e_container_zone_number_get(con, zone_num); \
	      if (zone) \
		act; \
	   } } } \
   else if ((con_num < 0) && (zone_num < 0)) /* con=all zone=all */ { \
      for (l = e_manager_list(); l; l = l->next) { \
	 man = l->data; \
	 for (ll = man->containers; ll; ll = ll->next) { \
	    con = ll->data; \
	    for (lll = con->zones; lll; lll = lll->next) { \
	       zone = lll->data; \
	       act; \
	    } } } } } \
else { \
   zone = e_util_container_zone_number_get(con_num, zone_num); \
   if (zone) act; \
}

/***************************************************************************/
ACT_FN_GO(zone_desk_flip_by)
{
   if (params)
     {
	int con_num = 0, zone_num = 0;
	int dx = 0, dy = 0;
	
	if (sscanf(params, "%i %i %i %i", &con_num, &zone_num, &dx, &dy) == 4)
	  {
	     ZONE_DESK_ACTION(con_num, zone_num, zone,
			      e_zone_desk_flip_by(zone, dx, dy));
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(zone_desk_flip_to)
{
   if (params)
     {
	int con_num = 0, zone_num = 0;
	int dx = 0, dy = 0;
	
	if (sscanf(params, "%i %i %i %i", &con_num, &zone_num, &dx, &dy) == 4)
	  {
	     ZONE_DESK_ACTION(con_num, zone_num, zone,
			      e_zone_desk_flip_to(zone, dx, dy));
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(zone_desk_linear_flip_by)
{
   if (params)
     {
	int con_num = 0, zone_num = 0;
	int dx = 0;
	
	if (sscanf(params, "%i %i %i", &con_num, &zone_num, &dx) == 3)
	  {
	     ZONE_DESK_ACTION(con_num, zone_num, zone,
			      e_zone_desk_linear_flip_by(zone, dx));
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(zone_desk_linear_flip_to)
{
   if (params)
     {
	int con_num = 0, zone_num = 0;
	int dx = 0;
	
	if (sscanf(params, "%i %i %i", &con_num, &zone_num, &dx) == 3)
	  {
	     ZONE_DESK_ACTION(con_num, zone_num, zone,
			      e_zone_desk_linear_flip_to(zone, dx));
	  }
     }
}

/***************************************************************************/
static void
_e_actions_cb_menu_end(void *data, E_Menu *m)
{
   e_object_del(E_OBJECT(m));
}
static E_Menu *
_e_actions_menu_find(char *name)
{
   if (!strcmp(name, "main")) return e_int_menus_main_new();
   else if (!strcmp(name, "favorites")) return e_int_menus_favorite_apps_new();
   else if (!strcmp(name, "clients")) return e_int_menus_clients_new();
   return NULL;
}
ACT_FN_GO(menu_show)
{
   E_Zone *zone = NULL;

   /* menu is active - abort */
   if (e_menu_grab_window_get()) return;
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     E_Menu *m = NULL;
	     
	     m = _e_actions_menu_find(params);	
	     if (m)
	       {
		  int x, y;
		  
		  /* FIXME: this is a bit of a hack... setting m->con - bad hack */
		  m->zone = zone;
		  ecore_x_pointer_xy_get(zone->container->win, &x, &y);
		  e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
		  e_menu_activate_mouse(m, zone, x, y, 1, 1,
					E_MENU_POP_DIRECTION_DOWN, 
					ecore_x_current_time_get());
	       }
	  }
     }
}
ACT_FN_GO_MOUSE(menu_show)
{
   E_Zone *zone = NULL;

   /* menu is active - abort */
   if (e_menu_grab_window_get()) return;
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     E_Menu *m = NULL;
	     
	     m = _e_actions_menu_find(params);	
	     if (m)
	       {
		  int x, y;
		  
		  /* FIXME: this is a bit of a hack... setting m->con - bad hack */
		  m->zone = zone;
		  x = ev->root.x;
		  y = ev->root.y;
		  x -= zone->container->x;
		  y -= zone->container->y;
		  e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
		  e_menu_activate_mouse(m, zone, x, y, 1, 1,
					E_MENU_POP_DIRECTION_DOWN, ev->time);
		  e_util_container_fake_mouse_up_all_later(zone->container);
	       }
	  }
     }
}
ACT_FN_GO_KEY(menu_show)
{
   E_Zone *zone = NULL;

   /* menu is active - abort */
   if (e_menu_grab_window_get()) return;
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     E_Menu *m = NULL;
	     
	     m = _e_actions_menu_find(params);	
	     if (m)
	       {
		  int x, y;
		  
		  /* FIXME: this is a bit of a hack... setting m->con - bad hack */
		  m->zone = zone;
		  ecore_x_pointer_xy_get(zone->container->win, &x, &y);
		  e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
		  e_menu_activate_key(m, zone, x, y, 1, 1,
				      E_MENU_POP_DIRECTION_DOWN);
	       }
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(exec)
{
   if (params)
     {
	Ecore_Exe *exe;
	
	exe = ecore_exe_run(params, NULL);
	if (exe) ecore_exe_free(exe);
     }
}

/***************************************************************************/
ACT_FN_GO(app)
{
   E_Zone *zone;
   
   if (!obj) return;
   if (obj->type != E_MANAGER_TYPE) return;
   zone = e_util_zone_current_get((E_Manager *)obj);
   if (zone)
     {
	if (params)
	  {
	     E_App *a = NULL;
	     char *p, *p2;
	     
	     p2 = strdup(params);
	     if (p2)
	       {
		  p = strchr(p2, ' ');
		  if (p)
		    {
		       *p = 0;
		       if (!strcmp(p2, "file:"))
			 a = e_app_file_find(p + 1);
		       else if (!strcmp(p2, "name:"))
			 a = e_app_name_find(p + 1);
		       else if (!strcmp(p2, "generic:"))
			 a = e_app_generic_find(p + 1);
		       else if (!strcmp(p2, "exe:"))
			 a = e_app_exe_find(p + 1);
		       if (a)
			 e_zone_app_exec(zone, a);
		    }
		  free(p2);
	       }
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(winlist)
{
   E_Zone *zone = NULL;
   
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     if (!strcmp(params, "next"))
	       {
		  if (!e_winlist_show(zone))
		    e_winlist_next();
	       }
	     else if (!strcmp(params, "prev"))
	       {
		  if (!e_winlist_show(zone))
		    e_winlist_prev();
	       }
	  }
	else
	  {
	     if (!e_winlist_show(zone))
	       e_winlist_next();
	  }
     }
}
ACT_FN_GO_MOUSE(winlist)
{
   E_Zone *zone = NULL;
   
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     if (!strcmp(params, "next"))
	       {
		  if (e_winlist_show(zone))
		    e_winlist_modifiers_set(ev->modifiers);
		  else
		    e_winlist_next();
	       }
	     else if (!strcmp(params, "prev"))
	       {
		  if (e_winlist_show(zone))
		    e_winlist_modifiers_set(ev->modifiers);
		  else
		    e_winlist_prev();
	       }
	  }
	else
	  {
	     if (e_winlist_show(zone))
	       e_winlist_modifiers_set(ev->modifiers);
	     else
	       e_winlist_next();
	  }
     }
}
ACT_FN_GO_KEY(winlist)
{
   E_Zone *zone = NULL;
   
   if (!obj) return;
   if (obj->type == E_MANAGER_TYPE)
     zone = e_util_zone_current_get((E_Manager *)obj);
   else if (obj->type == E_ZONE_TYPE)
     zone = (E_Zone *)obj;
   if (zone)
     {
	if (params)
	  {
	     if (!strcmp(params, "next"))
	       {
		  if (e_winlist_show(zone))
		    e_winlist_modifiers_set(ev->modifiers);
		  else
		    e_winlist_next();
	       }
	     else if (!strcmp(params, "prev"))
	       {
		  if (e_winlist_show(zone))
		    e_winlist_modifiers_set(ev->modifiers);
		  else
		    e_winlist_prev();
	       }
	  }
	else
	  {
	     if (e_winlist_show(zone))
	       e_winlist_modifiers_set(ev->modifiers);
	     else
	       e_winlist_next();
	  }
     }
}

/***************************************************************************/
ACT_FN_GO(edit_mode)
{
   if (!obj) obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
   if (!obj) return;
   if (obj->type != E_CONTAINER_TYPE)
     {
	obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
	if (!obj) return;
     }
   e_gadman_mode_set(((E_Container *)obj)->gadman, E_GADMAN_MODE_EDIT);
}
ACT_FN_END(edit_mode)
{
   if (!obj) obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
   if (!obj) return;
   if (obj->type != E_CONTAINER_TYPE)
     {
	obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
	if (!obj) return;
     }
   e_gadman_mode_set(((E_Container *)obj)->gadman, E_GADMAN_MODE_NORMAL);
}

/***************************************************************************/
ACT_FN_GO(edit_mode_toggle)
{
   if (!obj) obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
   if (!obj) return;
   if (obj->type != E_CONTAINER_TYPE)
     {
	obj = E_OBJECT(e_container_current_get(e_manager_current_get()));
	if (!obj) return;
     }
   if (e_gadman_mode_get(((E_Container *)obj)->gadman) == E_GADMAN_MODE_NORMAL)
     e_gadman_mode_set(((E_Container *)obj)->gadman, E_GADMAN_MODE_EDIT);
   else
     e_gadman_mode_set(((E_Container *)obj)->gadman, E_GADMAN_MODE_NORMAL);
}

/* local subsystem globals */
static Evas_Hash *actions = NULL;

/* externally accessible functions */

int
e_actions_init(void)
{
   E_Action *act;

   ACT_GO(window_move);
   ACT_GO_MOUSE(window_move);
   ACT_END(window_move);
   ACT_END_MOUSE(window_move);
   
   ACT_GO(window_resize);
   ACT_GO_MOUSE(window_resize);
   ACT_END(window_resize);
   ACT_END_MOUSE(window_resize);
  
   ACT_GO(window_menu);
   ACT_GO_MOUSE(window_menu);
   ACT_GO_KEY(window_menu);

   ACT_GO(window_raise);

   ACT_GO(window_lower);
   
   ACT_GO(window_close);

   ACT_GO(window_kill);
   
   ACT_GO(window_sticky_toggle);
   
   ACT_GO(window_iconic_toggle);
   
   ACT_GO(window_maximized_toggle);
   
   ACT_GO(window_shaded_toggle);
   
   ACT_GO(desk_flip_by);

   ACT_GO(desk_flip_to);

   ACT_GO(desk_linear_flip_by);
   
   ACT_GO(desk_linear_flip_to);

   ACT_GO(move_absolute);

   ACT_GO(move_relative);

   ACT_GO(resize);

   ACT_GO(menu_show);
   ACT_GO_MOUSE(menu_show);
   ACT_GO_KEY(menu_show);

   ACT_GO(exec);

   ACT_GO(app);
   
   ACT_GO(winlist);
   ACT_GO_MOUSE(winlist);
   ACT_GO_KEY(winlist);
   
   ACT_GO(edit_mode);
   ACT_END(edit_mode);
   
   ACT_GO(edit_mode_toggle);
   
   return 1;
}

int
e_actions_shutdown(void)
{
   if (actions)
     {
	evas_hash_foreach(actions, _e_actions_cb_free, NULL);
	evas_hash_free(actions);
	actions = NULL;
     }
   return 1;
}

E_Action *
e_action_add(char *name)
{
   E_Action *act;
   
   act = e_action_find(name);
   if (!act)
     {
	act = E_OBJECT_ALLOC(E_Action, E_ACTION_TYPE, _e_action_free);
	if (!act) return NULL;
	act->name = strdup(name);
	actions = evas_hash_add(actions, name, act);
     }
   return act;
}

E_Action *
e_action_find(char *name)
{
   E_Action *act;
   
   act = evas_hash_find(actions, name);
   return act;
}

/* local subsystem functions */

static void
_e_action_free(E_Action *act)
{
   actions = evas_hash_del(actions, act->name, act);
   E_FREE(act->name);
   free(act);
}

static Evas_Bool
_e_actions_cb_free(Evas_Hash *hash __UNUSED__, const char *key __UNUSED__,
		   void *data, void *fdata __UNUSED__)
{
   e_object_del(E_OBJECT(data));
   return 1;
}
