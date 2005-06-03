/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

void
e_hints_init(void)
{
   Ecore_X_Window *roots = NULL;
   int num;

   roots = ecore_x_window_root_list(&num);
   if (roots)
     {
	int i;
	
	for (i = 0; i < num; i++)
	  {
	     Ecore_X_Window win;
	     
	     win = ecore_x_window_new(roots[i], -200, -200, 5, 5);
/*	     
 * I don't FUCKING believe it. if we PRETENT we are Kwin - java is happy.
 * why? it expects a double reparenting wm then. java insists on finding this
 * out when it should be irrelevant! stupid code! I can't believe the time we
 * just wasted hunting a bug that wasn't and that is due to sheer stupid
 * coding.
 */
/* Now for more stupidity... Openoffice.org will change its look and feel 
 * depending on what wm it thinks there is... so if we pretend to be Kwin...
 * it tries to use kde preferences, if found.
 */
/* I have disabled tyhis now by pretending to be E16 with e16 comms. this
 * means java plays nice and uses our FRAMe property.. but we had to do other
 * evil stuff as java EXPECTS all this at REPARENT time... i've deferred
 * reparenting... i hate java!
 */
/*	     ecore_x_netwm_wm_identify(roots[i], win, "KWin");*/
	     ecore_x_netwm_wm_identify(roots[i], win, "Enlightenment");

	     /* Set what hints we support */
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_ACTIVE_WINDOW, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_CLIENT_LIST, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_CLIENT_LIST_STACKING, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_FRAME_EXTENTS, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_SUPPORTED, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_VIRTUAL_ROOTS, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_ICON, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_ICON_NAME, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_NAME, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_PID, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_STICKY, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_SHADED, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_HIDDEN, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_ABOVE, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STATE_BELOW, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STRUT, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_USER_TIME, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_VISIBLE_NAME, 1);

	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL, 1);
	     ecore_x_netwm_supported(roots[i], ECORE_X_ATOM_NET_WORKAREA, 1);
	  }
        free(roots);
     }
}

/* 
 * This is here so we don't have to pretend to be Kwin anymore - we pretend
 * to do old e16 style ipc. in fact we just ignore it... but set up the
 * window port anyway
 */
void
e_hints_e16_comms_pretend(E_Manager *man)
{
   Ecore_X_Window win;
   Ecore_X_Atom enlightenment_comms, string;
   char buf[256];
   
   enlightenment_comms = ecore_x_atom_get("ENLIGHTENMENT_COMMS");
   string = ecore_x_atom_get("STRING");
   win = ecore_x_window_input_new(man->root, -100, -100, 1, 1);
   snprintf(buf, sizeof(buf), "WINID %8x", (int)win);
   ecore_x_window_prop_property_set(win, enlightenment_comms, string, 8, buf, 14);
   ecore_x_window_prop_property_set(man->root, enlightenment_comms, string, 8, buf, 14);
}

void
e_hints_manager_init(E_Manager *man)
{
   /* Set desktop count, desktop names and workarea */
   int			i = 0, num = 0;
   unsigned int		*areas = NULL;
   Evas_List		*ml, *cl;
   Ecore_X_Window	*vroots = NULL;
   /* FIXME: Desktop names not yet implemented */
/*   char			**names; */

   e_hints_e16_comms_pretend(man);

   num = evas_list_count(man->containers);

   vroots = calloc(num, sizeof(Ecore_X_Window));
   if (!vroots) return;
   
/*   names = calloc(num, sizeof(char *));*/
   
   areas = calloc(4 * num, sizeof(unsigned int));
   if (!areas)
     {
	free(vroots);
	return;
     }
   
   for (cl = man->containers; cl; cl = cl->next)
     {
	E_Container *c;

	c = cl->data;
	areas[4 * i] = c->x;
	areas[4 * i + 1] = c->y;
	areas[4 * i + 2] = c->w;
	areas[4 * i + 3] = c->h;
	vroots[i++] = c->win;
     }

#if 0
   ecore_x_netwm_desk_count_set(man->root, num);
#endif
   if (e_config->use_virtual_roots)
     {
	ecore_x_netwm_desk_roots_set(man->root, num, vroots);
     }
   ecore_x_netwm_desk_workareas_set(man->root, num, areas);

   free(vroots);
   free(areas);
}

/* FIXME, this should set the list in map order, not stack order */
void
e_hints_client_list_set(void)
{
   Evas_List *ml = NULL, *cl = NULL;
   unsigned int i = 0, num = 0;
   E_Manager *m;
   E_Container *c;
   E_Border_List *bl;
   E_Border *b;
   Ecore_X_Window *clients = NULL;

   /* Get client count by adding client lists on all containers */
   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	for (cl = m->containers; cl; cl = cl->next)
	  {
	     c = cl->data;
	     num += e_container_borders_count(c);
	  }
     }
   
   clients = calloc(num, sizeof(Ecore_X_Window));
   if (!clients)
      return;

   /* Fetch window IDs and add to array */
   if (num > 0)
     {
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     for (cl = m->containers; cl; cl = cl->next)
	       {
		  c = cl->data;
		  bl = e_container_border_list_first(c);
		  while ((b = e_container_border_list_next(bl)))
		    clients[i++] = b->win;
		  e_container_border_list_free(bl);
	       }
	  }
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     ecore_x_netwm_client_list_set(m->root, num, clients);
	     ecore_x_netwm_client_list_stacking_set(m->root, num, clients);
	  }
     }
   else
     {
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     ecore_x_netwm_client_list_set(m->root, 0, NULL);
	     ecore_x_netwm_client_list_stacking_set(m->root, 0, NULL);
	  }
     }
   IF_FREE(clients);
}

/* Client list is already in stacking order, so this function is nearly
 * identical to the previous one */
void
e_hints_client_stacking_set(void)
{
   Evas_List *ml = NULL, *cl = NULL;
   unsigned int i = 0, num = 0;
   E_Manager *m;
   E_Container *c;
   E_Border_List *bl;
   E_Border *b;
   Ecore_X_Window *clients = NULL;

   /* Get client count */
   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	for (cl = m->containers; cl; cl = cl->next)
	  {
	     c = cl->data;
	     num += e_container_borders_count(c);
	  }
     }
   
   clients = calloc(num, sizeof(Ecore_X_Window));
   if (!clients)
      return;

   if (num > 0)
     {
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     for (cl = m->containers; cl; cl = cl->next)
	       {
		  c = cl->data;
		  bl = e_container_border_list_first(c);
		  while ((b = e_container_border_list_next(bl)))
		    clients[i++] = b->win;
		  e_container_border_list_free(bl);
	       }
	  }
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     ecore_x_netwm_client_list_stacking_set(m->root, num, clients);
	  }
     }
   else
     {
	for (ml = e_manager_list(); ml; ml = ml->next)
	  {
	     m = ml->data;
	     ecore_x_netwm_client_list_stacking_set(m->root, 0, NULL);
	  }
     }
   IF_FREE(clients);
}

void
e_hints_active_window_set(E_Manager *man, E_Border *bd)
{
   E_OBJECT_CHECK(man);
   if (bd)
     ecore_x_netwm_client_active_set(man->root, bd->client.win);
   else
     ecore_x_netwm_client_active_set(man->root, 0);
}

void
e_hints_window_init(E_Border *bd)
{
   /* Don't need fetch flag, is onlt called from new border */
   e_hints_window_state_get(bd);
   e_hints_window_type_get(bd);

   bd->client.icccm.state = ecore_x_icccm_state_get(bd->client.win);
   if (bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_NONE)
     {
	if (bd->client.netwm.state.hidden)
	  bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
	else
	  bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
     }

   if (bd->client.netwm.type == ECORE_X_WINDOW_TYPE_DESKTOP)
     bd->layer = 0;
   else if (bd->client.netwm.state.stacking == E_STACKING_BELOW)
     bd->layer = 50;
   else if (bd->client.netwm.state.stacking == E_STACKING_ABOVE)
     bd->layer = 150;
   else if (bd->client.netwm.type == ECORE_X_WINDOW_TYPE_DOCK)
     bd->layer = 150;
   else
     bd->layer = 100;
   e_border_raise(bd);

#if 0
   /* Ignore this, E has incompatible desktop setup */
   if (ecore_x_netwm_desktop_get(bd->client.win, &bd->client.netwm.desktop))
     {
	if (bd->client.netwm.desktop == 0xffffffff)
	  {
//	     printf("CLIENT asks for all desks!\n");
	     e_border_stick(bd);
	  }
	else if (bd->client.netwm.desktop < (bd->zone->desk_x_count * bd->zone->desk_y_count))
	  {
	     E_Desk *desk;

	     desk = e_desk_at_pos_get(bd->zone, bd->client.netwm.desktop);
	     if (desk)
	       e_border_desk_set(bd, desk);
	  }
	else
	  {
	     /* Update netwm desktop with current desktop */
	     e_hints_window_desktop_set(bd);
	  }
     }
   else
     {
	/* Update netwm desktop with current desktop */
	e_hints_window_desktop_set(bd);
     }
#endif

   /* It's ok not to have fetch flag, should only be set on startup
    * and not changed. */
   if (!ecore_x_netwm_pid_get(bd->client.win, &bd->client.netwm.pid))
     bd->client.netwm.pid = -1;

   if (bd->client.netwm.state.sticky)
     e_border_stick(bd);
   if (bd->client.netwm.state.shaded)
     e_border_shade(bd, e_hints_window_shade_direction_get(bd));
   if (bd->client.netwm.state.maximized_v && bd->client.netwm.state.maximized_h)
     e_border_maximize(bd);
   if (bd->client.netwm.state.fullscreen)
     e_border_fullscreen(bd);
   if ((bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
       && (bd->client.netwm.state.hidden))
     e_border_iconify(bd);
   /* If a window isn't iconic, and is one the current desk,
    * show it! */
   else if (bd->desk == e_desk_current_get(bd->zone))
     e_border_show(bd);

   /* Update stacking */
   e_hints_client_list_set();
   e_hints_client_stacking_set();
}

void
e_hints_window_state_set(E_Border *bd)
{
   Ecore_X_Window_State state[10];
   int num;

   num = 0;

   if (bd->client.netwm.state.modal)
     state[num++] = ECORE_X_WINDOW_STATE_MODAL;
   if (bd->client.netwm.state.sticky)
     state[num++] = ECORE_X_WINDOW_STATE_STICKY;
   if (bd->client.netwm.state.maximized_v)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   if (bd->client.netwm.state.maximized_h)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   if (bd->client.netwm.state.shaded)
     state[num++] = ECORE_X_WINDOW_STATE_SHADED;
   if (bd->client.netwm.state.skip_taskbar)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   if (bd->client.netwm.state.skip_pager)
     state[num++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
   if (bd->client.netwm.state.hidden)
     state[num++] = ECORE_X_WINDOW_STATE_HIDDEN;
   if (bd->client.netwm.state.fullscreen)
     state[num++] = ECORE_X_WINDOW_STATE_FULLSCREEN;

   switch (bd->client.netwm.state.stacking)
     {
      case E_STACKING_ABOVE:
	 state[num++] = ECORE_X_WINDOW_STATE_ABOVE;
	 break;
      case E_STACKING_BELOW:
	 state[num++] = ECORE_X_WINDOW_STATE_BELOW;
	 break;
      case E_STACKING_NONE:
      default:
	 break;
     }
   ecore_x_netwm_window_state_list_set(bd->client.win, state, num);
}

void e_hints_window_type_set(E_Border *bd)
{
   ecore_x_netwm_window_type_set(bd->client.win, bd->client.netwm.type);
}

void e_hints_window_type_get(E_Border *bd)
{
   /* Don't need fetch flag, only called on new border */
   bd->client.netwm.type = ecore_x_netwm_window_type_get(bd->client.win);
}

void
e_hints_window_state_update(E_Border *bd, Ecore_X_Window_State state,
			    Ecore_X_Window_State_Action action)
{
   int changed;

   switch (state)
     {
      case ECORE_X_WINDOW_STATE_ICONIFIED:
	 if (action != ECORE_X_WINDOW_STATE_ACTION_ADD) return;
	 if (bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_ICONIC) return;
	 e_border_iconify(bd);
	 break;
      case ECORE_X_WINDOW_STATE_MODAL:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.modal)
		 {
		    bd->client.netwm.state.modal = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.modal)
		 {
		    bd->client.netwm.state.modal = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.modal = !bd->client.netwm.state.modal;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	   }
	 break;
      case ECORE_X_WINDOW_STATE_STICKY:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.sticky)
		 {
		    bd->client.netwm.state.sticky = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.sticky)
		 {
		    bd->client.netwm.state.sticky = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.sticky = !bd->client.netwm.state.sticky;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if (bd->client.netwm.state.sticky)
		e_border_stick(bd);
	      else
		e_border_unstick(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.maximized_v)
		 {
		    bd->client.netwm.state.maximized_v = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.maximized_v)
		 {
		    bd->client.netwm.state.maximized_v = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.maximized_v = !bd->client.netwm.state.maximized_v;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if ((bd->client.netwm.state.maximized_v)
		  && (bd->client.netwm.state.maximized_h))
		e_border_maximize(bd);
	      else if ((!bd->client.netwm.state.maximized_v)
		       && (!bd->client.netwm.state.maximized_h))
		e_border_unmaximize(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.maximized_h)
		 {
		    bd->client.netwm.state.maximized_h = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.maximized_h)
		 {
		    bd->client.netwm.state.maximized_h = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.maximized_h = !bd->client.netwm.state.maximized_h;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if ((bd->client.netwm.state.maximized_v)
		  && (bd->client.netwm.state.maximized_h))
		e_border_maximize(bd);
	      else if ((!bd->client.netwm.state.maximized_v)
		       && (!bd->client.netwm.state.maximized_h))
		e_border_unmaximize(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_SHADED:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.shaded)
		 {
		    bd->client.netwm.state.shaded = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.shaded)
		 {
		    bd->client.netwm.state.shaded = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.shaded = !bd->client.netwm.state.shaded;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if (bd->client.netwm.state.shaded)
		e_border_shade(bd, e_hints_window_shade_direction_get(bd));
	      else
		e_border_unshade(bd, e_hints_window_shade_direction_get(bd));
	   }
	 break;
      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.skip_taskbar)
		 {
		    bd->client.netwm.state.skip_taskbar = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.skip_taskbar)
		 {
		    bd->client.netwm.state.skip_taskbar = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.skip_taskbar = !bd->client.netwm.state.skip_taskbar;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	   }
	 break;
      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.skip_pager)
		 {
		    bd->client.netwm.state.skip_pager = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.skip_pager)
		 {
		    bd->client.netwm.state.skip_pager = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.skip_pager = !bd->client.netwm.state.skip_pager;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      /* FIXME, we have a pager, care about this! */
	      bd->client.netwm.update.state = 1;
	   }
	 break;
      case ECORE_X_WINDOW_STATE_HIDDEN:
	 /* Ignore */
	 break;
      case ECORE_X_WINDOW_STATE_FULLSCREEN:
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.fullscreen)
		 {
		    bd->client.netwm.state.fullscreen = 0;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (!bd->client.netwm.state.fullscreen)
		 {
		    bd->client.netwm.state.fullscreen = 1;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       bd->client.netwm.state.fullscreen = !bd->client.netwm.state.fullscreen;
	       changed = 1;
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if (bd->client.netwm.state.fullscreen)
		e_border_fullscreen(bd);
	      else
		e_border_unfullscreen(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_ABOVE:
	 /* FIXME: Should this require that BELOW is set to 0 first, or just
	  * do it? */
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.stacking == E_STACKING_ABOVE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_NONE;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (bd->client.netwm.state.stacking == E_STACKING_NONE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_ABOVE;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       if (bd->client.netwm.state.stacking == E_STACKING_ABOVE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_NONE;
		    changed = 1;
		 }
	       else if (bd->client.netwm.state.stacking == E_STACKING_NONE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_ABOVE;
		    changed = 1;
		 }
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if (bd->client.netwm.state.stacking)
		bd->layer = 150;
	      else
		bd->layer = 100;
	      e_border_raise(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_BELOW:
	 /* FIXME: Should this require that ABOVE is set to 0 first, or just
	  * do it? */
	 changed = 0;
	 switch (action)
	   {
	    case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
	       if (bd->client.netwm.state.stacking == E_STACKING_BELOW)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_NONE;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_ADD:
	       if (bd->client.netwm.state.stacking == E_STACKING_NONE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_BELOW;
		    changed = 1;
		 }
	       break;
	    case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
	       if (bd->client.netwm.state.stacking == E_STACKING_BELOW)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_NONE;
		    changed = 1;
		 }
	       else if (bd->client.netwm.state.stacking == E_STACKING_NONE)
		 {
		    bd->client.netwm.state.stacking = E_STACKING_BELOW;
		    changed = 1;
		 }
	       break;
	   }
	 if (changed)
	   {
	      bd->client.netwm.update.state = 1;
	      if (bd->client.netwm.state.stacking)
		bd->layer = 50;
	      else
		bd->layer = 100;
	      e_border_raise(bd);
	   }
	 break;
      case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
	 /* FIXME */
	 break;
      case ECORE_X_WINDOW_STATE_UNKNOWN:
	 /* Ignore */
	 break;
     }
}

void
e_hints_window_state_get(E_Border *bd)
{
   int i, num;
   Ecore_X_Window_State *state;

   bd->client.netwm.state.modal = 0;
   bd->client.netwm.state.sticky = 0;
   bd->client.netwm.state.maximized_v = 0;
   bd->client.netwm.state.maximized_h = 0;
   bd->client.netwm.state.shaded = 0;
   bd->client.netwm.state.skip_taskbar = 0;
   bd->client.netwm.state.skip_pager = 0;
   bd->client.netwm.state.hidden = 0;
   bd->client.netwm.state.fullscreen = 0;
   bd->client.netwm.state.stacking = 0;

   state = ecore_x_netwm_window_state_list_get(bd->client.win, &num);
   if (state)
     {
	for (i = 0; i < num; i++)
	  {
	     switch (state[i])
	       {
		case ECORE_X_WINDOW_STATE_ICONIFIED:
		   /* Ignore */
		   break;
		case ECORE_X_WINDOW_STATE_MODAL:
		   bd->client.netwm.state.modal = 1;
		   break;
		case ECORE_X_WINDOW_STATE_STICKY:
		   bd->client.netwm.state.sticky = 1;
		   break;
		case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
		   bd->client.netwm.state.maximized_v = 1;
		   break;
		case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
		   bd->client.netwm.state.maximized_h = 1;
		   break;
		case ECORE_X_WINDOW_STATE_SHADED:
		   bd->client.netwm.state.shaded = 1;
		   break;
		case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
		   bd->client.netwm.state.skip_taskbar = 1;
		   break;
		case ECORE_X_WINDOW_STATE_SKIP_PAGER:
		   bd->client.netwm.state.skip_pager = 1;
		   break;
		case ECORE_X_WINDOW_STATE_HIDDEN:
		   bd->client.netwm.state.hidden = 1;
		   break;
		case ECORE_X_WINDOW_STATE_FULLSCREEN:
		   bd->client.netwm.state.fullscreen = 1;
		   break;
		case ECORE_X_WINDOW_STATE_ABOVE:
		   bd->client.netwm.state.stacking = E_STACKING_ABOVE;
		   break;
		case ECORE_X_WINDOW_STATE_BELOW:
		   bd->client.netwm.state.stacking = E_STACKING_BELOW;
		   break;
		case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
		   /* FIXME */
		   break;
		case ECORE_X_WINDOW_STATE_UNKNOWN:
		   /* Ignore */
		   break;
	       }
	  }
	free(state);
     }
}

void
e_hints_window_visible_set(E_Border *bd)
{
   if (bd->client.icccm.state != ECORE_X_WINDOW_STATE_HINT_NORMAL)
     {
	ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_NORMAL);
	bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
     }
   if (bd->client.netwm.state.hidden)
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.hidden = 0;
     }
}

void
e_hints_window_iconic_set(E_Border *bd)
{
   if (bd->client.icccm.state != ECORE_X_WINDOW_STATE_HINT_ICONIC)
     {
	ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_ICONIC);
	bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }
   if (!bd->client.netwm.state.hidden)
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.hidden = 1;
     }
}

void
e_hints_window_hidden_set(E_Border *bd)
{
   if (bd->client.icccm.state != ECORE_X_WINDOW_STATE_HINT_ICONIC)
     {
	ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_ICONIC);
	bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }
   if (bd->client.netwm.state.hidden)
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.hidden = 0;
     }
}

void
e_hints_window_shaded_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.shaded) && (on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.shaded = 1;
     }
   else if ((bd->client.netwm.state.shaded) && (!on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.shaded = 0;
     }
}

void
e_hints_window_shade_direction_set(E_Border *bd, E_Direction dir)
{
   ecore_x_window_prop_card32_set(bd->client.win, E_ATOM_SHADE_DIRECTION, &dir, 1);
}

E_Direction
e_hints_window_shade_direction_get(E_Border *bd)
{
   int ret;
   E_Direction dir;

   ret = ecore_x_window_prop_card32_get(bd->client.win,
	 E_ATOM_SHADE_DIRECTION,
	 &dir, 1);
   if (ret == 1)
     return dir;

   return E_DIRECTION_UP;
}

void
e_hints_window_maximized_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.maximized_v) && (on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.maximized_v = 1;
     }
   else if ((bd->client.netwm.state.maximized_v) && (!on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.maximized_v = 0;
     }
   if ((!bd->client.netwm.state.maximized_h) && (on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.maximized_h = 1;
     }
   else if ((bd->client.netwm.state.maximized_h) && (!on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.maximized_h = 0;
     }
}

void
e_hints_window_fullscreen_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.fullscreen) && (on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.fullscreen = 1;
     }
   else if ((bd->client.netwm.state.fullscreen) && (!on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.fullscreen = 0;
     }
}

void
e_hints_window_sticky_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.sticky) && (on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.sticky = 1;
     }
   else if ((bd->client.netwm.state.sticky) && (!on))
     {
	bd->client.netwm.update.state = 1;
	bd->client.netwm.state.sticky = 0;
     }
}

void
e_hints_window_stacking_set(E_Border *bd, E_Stacking stacking)
{
   if (bd->client.netwm.state.stacking == stacking) return;
   bd->client.netwm.update.state = 1;
   bd->client.netwm.state.stacking = stacking;
}

void
e_hints_window_desktop_set(E_Border *bd)
{
   unsigned int deskpos[2];
   unsigned int current;

   current = (bd->desk->y * bd->zone->desk_x_count) + bd->desk->x;
   /* if valgrind complains here it is complaining bd->client.netwm.desktop
    * is an uninitialised variable - but it isn't. it can't be. its part of
    * a calloc()'d struct and thus has to have been set to 0. hell even
    * e_border.c explicitly sets it to 0 on creation of the border object.
    */
   if (bd->client.netwm.desktop != current)
     {
	deskpos[0] = bd->desk->x;
	deskpos[1] = bd->desk->y;
	ecore_x_window_prop_card32_set(bd->client.win, E_ATOM_DESK, deskpos, 2);

#if 0
	ecore_x_netwm_desktop_set(bd->client.win, current);
#endif
	bd->client.netwm.desktop = current;
     }
}
