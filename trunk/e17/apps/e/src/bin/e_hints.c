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
	     ecore_x_netwm_wm_identify(roots[i], win, "Enlightenment");
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
e_hints_active_window_set(E_Manager *man, Ecore_X_Window win)
{
   E_OBJECT_CHECK(man);
   ecore_x_netwm_client_active_set(man->root, win);
}

void
e_hints_window_name_set(Ecore_X_Window win, const char *name)
{
   ecore_x_icccm_title_set(win, name);
   ecore_x_netwm_name_set(win, name);
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
	ecore_x_netwm_desk_roots_set(m->root, num, vroots);
	ecore_x_netwm_desk_workareas_set(m->root, num, areas);
     }
   free(vroots);
   free(areas);
}

void
e_hints_window_state_set(Ecore_X_Window win)
{
   E_Border	*bd;

   bd = e_border_find_by_client_window(win);
   
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_MODAL, 
				  bd->client.netwm.state.modal);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_STICKY,
				  bd->sticky);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_MAXIMIZED_VERT,
				  bd->client.netwm.state.maximized_v);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ,
				  bd->client.netwm.state.maximized_h);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_SHADED,
				  bd->shaded);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_SKIP_TASKBAR,
				  bd->client.netwm.state.skip_taskbar);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_SKIP_PAGER,
				  bd->client.netwm.state.skip_pager);
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_HIDDEN,
				  !(bd->visible));
   ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_FULLSCREEN,
				  bd->client.netwm.state.fullscreen);
   
   switch (bd->client.netwm.state.stacking)
     {
      case 1:
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_ABOVE, 1);
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_BELOW, 0);
      case 2:
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_ABOVE, 0);
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_BELOW, 1);
      default:
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_ABOVE, 0);
	 ecore_x_netwm_window_state_set(win, ECORE_X_WINDOW_STATE_BELOW, 0);
	 break;
     }
}

void
e_hints_window_name_get(Ecore_X_Window win)
{
   E_Border	*bd;
   char		*name;

   name = ecore_x_netwm_name_get(win);
   bd = e_border_find_by_client_window(win);
   if (bd->client.icccm.name)
     free(bd->client.icccm.name);
   bd->client.icccm.name = name;
   bd->changed = 1;
}

void
e_hints_window_icon_name_get(Ecore_X_Window win)
{
   E_Border	*bd;
   char		*name;

   name = ecore_x_netwm_icon_name_get(win);
   bd = e_border_find_by_client_window(win);
   if (bd->client.icccm.icon_name)
     free(bd->client.icccm.icon_name);
   bd->client.icccm.icon_name = name;
   bd->changed = 1;
}




