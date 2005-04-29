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
//	     I don't FUCKING believe it. if we PRETENT we are Kwin - java is
//	     happy. why? it expects a double reparenting wm then. java insists
//	     on finding this out when it shoudl be irrelevant! stupid FUCKS.
	     ecore_x_netwm_wm_identify(roots[i], win, "KWin");
//	     ecore_x_netwm_wm_identify(roots[i], win, "Enlightenment");
	  }
        free(roots);
     }
}

void
e_hints_client_list_set(void)
{
   Evas_List *ml = NULL, *cl = NULL, *bl = NULL;
   unsigned int i = 0, num = 0;
   E_Manager *m;
   E_Container *c;
   E_Border *b;
   Ecore_X_Window *clients = NULL;

   /* Get client count by adding client lists on all containers */
   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	for (cl = m->containers; cl; cl = cl->next)
	  {
	     c = cl->data;
	     num += evas_list_count(c->clients);
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
		  for (bl = c->clients; bl; bl = bl->next)
		    {
		       b = bl->data;
		       clients[i++] = b->win;
		    }
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

}

/* Client list is already in stacking order, so this function is nearly
 * identical to the previous one */
void
e_hints_client_stacking_set(void)
{
   Evas_List *ml = NULL, *cl = NULL, *bl = NULL;
   unsigned int i = 0, num = 0;
   E_Manager *m;
   E_Container *c;
   E_Border *b;
   Ecore_X_Window *clients = NULL;

   /* Get client count */
   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	for (cl = m->containers; cl; cl = cl->next)
	  {
	     c = cl->data;
	     num += evas_list_count(c->clients);
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
		  for (bl = c->clients; bl; bl = bl->next)
		    {
		       b = bl->data;
		       clients[i++] = b->win;
		    }
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
e_hints_window_name_set(E_Border *bd)
{
   ecore_x_icccm_title_set(bd->client.win, bd->client.icccm.title);
   ecore_x_netwm_name_set(bd->client.win, bd->client.icccm.title);
}

void
e_hints_window_name_get(E_Border *bd)
{
   char		*name;

   name = ecore_x_netwm_name_get(bd->client.win);
   if (!name)
     name = ecore_x_icccm_title_get(bd->client.win);
   if (!name)
     name = strdup("No name!!");
   if (bd->client.icccm.title) free(bd->client.icccm.title);
   bd->client.icccm.title = name;
}

void
e_hints_desktop_config_set(void)
{
   /* Set desktop count, desktop names and workarea */
   
   int			i = 0, num = 0;
   unsigned int		*areas = NULL;
   E_Manager		*m;
   E_Container		*c;
   Evas_List		*ml, *cl;
   Ecore_X_Window	*vroots = NULL;
   /* FIXME: Desktop names not yet implemented */
/*   char			**names; */

   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	num += evas_list_count(m->containers);
     }

   vroots = calloc(num, sizeof(Ecore_X_Window));
   if (!vroots) return;
   
/*   names = calloc(num, sizeof(char *));*/
   
   areas = calloc(4 * num, sizeof(unsigned int));
   if (!areas)
     {
	free(vroots);
	return;
     }
   
   for (ml = e_manager_list(); ml; ml=ml->next)
     {
	m = ml->data;
	for (cl = m->containers; cl; cl=cl->next)
	  {
	     c = cl->data;
	     areas[4 * i] = c->x;
	     areas[4 * i + 1] = c->y;
	     areas[4 * i + 2] = c->w;
	     areas[4 * i + 3] = c->h;
	     vroots[i++] = c->win;
	  }
     }
   
   for (ml = e_manager_list(); ml; ml = ml->next)
     {
	m = ml->data;
	ecore_x_netwm_desk_count_set(m->root, num);
	if (e_config->use_virtual_roots)
	  {
	     ecore_x_netwm_desk_roots_set(m->root, num, vroots);
	  }
	ecore_x_netwm_desk_workareas_set(m->root, num, areas);
     }
   free(vroots);
   free(areas);
}

void
e_hints_window_init(E_Border *bd)
{
   e_hints_window_state_get(bd);
   e_hints_window_type_get(bd);

   bd->client.icccm.state = ecore_x_icccm_state_get(bd->client.win);
   if (bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_NONE)
     {
	if (bd->client.netwm.state.hidden)
	  bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
	else
	  bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
     }

   if (bd->client.netwm.type == ECORE_X_WINDOW_TYPE_DESKTOP)
     bd->layer = 0;
   else if (bd->client.netwm.state.stacking == 2)
     bd->layer = 50;
   else if (bd->client.netwm.state.stacking == 1)
     bd->layer = 150;
   else if (bd->client.netwm.type == ECORE_X_WINDOW_TYPE_DOCK)
     bd->layer = 150;
   else if (bd->client.netwm.state.fullscreen)
     bd->layer = 200;
   else
     bd->layer = 100;
   e_container_window_raise(bd->zone->container, bd->win, bd->layer);

   if (bd->client.netwm.state.sticky)
     e_border_stick(bd);
   if (bd->client.netwm.state.shaded)
     e_border_shade(bd, e_hints_window_shade_direction_get(bd));
   if (bd->client.netwm.state.maximized_v && bd->client.netwm.state.maximized_h)
     e_border_maximize(bd);
   if (bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     e_border_iconify(bd);
   /* If a window isn't iconic, and is one the current desk,
    * show it! */
   else if ((bd->client.icccm.state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
	    && (bd->desk == e_desk_current_get(bd->zone)))
     e_border_show(bd);
}

void
e_hints_window_state_set(E_Border *bd)
{
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MODAL, 
				  bd->client.netwm.state.modal);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_STICKY,
				  bd->client.netwm.state.sticky);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_VERT,
				  bd->client.netwm.state.maximized_v);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ,
				  bd->client.netwm.state.maximized_h);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_SHADED,
				  bd->client.netwm.state.shaded);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_SKIP_TASKBAR,
				  bd->client.netwm.state.skip_taskbar);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_SKIP_PAGER,
				  bd->client.netwm.state.skip_pager);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HIDDEN,
				  bd->client.netwm.state.hidden);
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_FULLSCREEN,
				  bd->client.netwm.state.fullscreen);
   
   switch (bd->client.netwm.state.stacking)
     {
      case 1:
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_ABOVE, 1);
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_BELOW, 0);
      case 2:
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_ABOVE, 0);
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_BELOW, 1);
      default:
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_ABOVE, 0);
	 ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_BELOW, 0);
	 break;
     }
}

void e_hints_window_type_set(E_Border *bd)
{
   ecore_x_netwm_window_type_set(bd->client.win, bd->client.netwm.type);
}

void e_hints_window_type_get(E_Border *bd)
{
   bd->client.netwm.type = ecore_x_netwm_window_type_get(bd->client.win);
}

void
e_hints_window_state_get(E_Border *bd)
{
   int		 above, below;

   bd->client.netwm.state.modal = 
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_MODAL);
   bd->client.netwm.state.sticky = 
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_STICKY);
   bd->client.netwm.state.maximized_v = 
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_VERT);
   bd->client.netwm.state.maximized_h = 
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ);
   bd->client.netwm.state.shaded =
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_SHADED);
   bd->client.netwm.state.skip_taskbar =
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_SKIP_TASKBAR);
   bd->client.netwm.state.skip_pager =
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_SKIP_PAGER);
   bd->client.netwm.state.hidden =
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_HIDDEN);
   bd->client.netwm.state.fullscreen =
      ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_FULLSCREEN);
   
   above = ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_ABOVE);
   below = ecore_x_netwm_window_state_isset(bd->client.win, ECORE_X_WINDOW_STATE_BELOW);
   bd->client.netwm.state.stacking = (above << 0) + (below << 1);
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
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HIDDEN, 0);
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
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HIDDEN, 1);
	bd->client.netwm.state.hidden = 1;
     }
}

void
e_hints_window_hidden_set(E_Border *bd)
{
   if (bd->client.icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     {
	ecore_x_icccm_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HINT_WITHDRAWN);
	bd->client.icccm.state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
     }
   if (!bd->client.netwm.state.hidden)
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_HIDDEN, 1);
	bd->client.netwm.state.hidden = 1;
     }
}

void
e_hints_window_shaded_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.shaded) && (on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_SHADED, 1);
	bd->client.netwm.state.shaded = 1;
     }
   else if ((bd->client.netwm.state.shaded) && (!on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_SHADED, 0);
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
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_VERT, 1);
	bd->client.netwm.state.maximized_v = 1;
     }
   else if ((bd->client.netwm.state.maximized_v) && (!on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_VERT, 0);
	bd->client.netwm.state.maximized_v = 0;
     }
   if ((!bd->client.netwm.state.maximized_h) && (on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ, 1);
	bd->client.netwm.state.maximized_h = 1;
     }
   else if ((bd->client.netwm.state.maximized_h) && (!on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ, 0);
	bd->client.netwm.state.maximized_h = 0;
     }
}

void
e_hints_window_fullscreen_set(E_Border *bd, int on)
{
   if ((!bd->client.netwm.state.fullscreen) && (on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_FULLSCREEN, 1);
	bd->client.netwm.state.fullscreen = 1;
     }
   else if ((bd->client.netwm.state.fullscreen) && (!on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_FULLSCREEN, 0);
	bd->client.netwm.state.fullscreen = 0;
     }
}

void
e_hints_window_sticky_set(E_Border *bd, int on)
{
   ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_STICKY, on);
   if ((!bd->client.netwm.state.sticky) && (on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_STICKY, 1);
	bd->client.netwm.state.sticky = 1;
     }
   else if ((bd->client.netwm.state.sticky) && (!on))
     {
	ecore_x_netwm_window_state_set(bd->client.win, ECORE_X_WINDOW_STATE_STICKY, 0);
	bd->client.netwm.state.sticky = 0;
     }
}

/*
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_MODAL, on);
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_SKIP_TASKBAR, on);
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_SKIP_PAGER, on);
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_FULLSCREEN, on);
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_ABOVE, on);
ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_BELOW, on);
*/

void
e_hints_window_icon_name_get(E_Border *bd)
{
   char		*name;

   name = ecore_x_netwm_icon_name_get(bd->client.win);
   if (bd->client.icccm.icon_name)
     free(bd->client.icccm.icon_name);
   bd->client.icccm.icon_name = name;
   bd->changed = 1;
}
