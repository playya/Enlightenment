#include <e.h>
#include "e_winilist.h"

/* internal calls */
typedef struct _Data Data;
typedef struct _Special Special;
  
struct _Data
{
   Evas_Object *o_frame;
   Evas_Object *o_ilist;
   struct {
      void (*func) (void *data, E_Border *bd);
      void *data;
   } select;
   struct {
      Evas_List *prepend;
      Evas_List *append;
      unsigned char changed : 1;
   } special;
   struct {
      Evas_Coord w, h;
   } optimal_size;
   Evas_List *borders;
   Evas_List *labels;
};

struct _Special
{
   Evas_Object *icon;
   const char *label;
   void (*func) (void *data1, void *data2);
   void *data1;
   void *data2;
};

static void _cb_item_sel(void *data, void *data2);
static void _cb_special_sel(void *data, void *data2);
    
static void _cb_object_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cb_object_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _refill(Data *d);

static int _cb_border_add(void *data, int ev_type, void *ev);
static int _cb_border_remove(void *data, int ev_type, void *ev);
static int _cb_border_show(void *data, int ev_type, void *ev);
static int _cb_border_hide(void *data, int ev_type, void *ev);
static int _cb_border_property(void *data, int ev_type, void *ev);

/* state */
static Evas_List *handlers = NULL;
static Evas_List *winilists = NULL;

/* called from the module core */
EAPI int
e_winilist_init(void)
{
   handlers = evas_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_ADD, _cb_border_add, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_REMOVE, _cb_border_remove, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_SHOW, _cb_border_show, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_HIDE, _cb_border_hide, NULL));
   handlers = evas_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_PROPERTY, _cb_border_property, NULL));
   return 1;
}

EAPI int
e_winilist_shutdown(void)
{
   while (handlers)
     {
	ecore_event_handler_del(handlers->data);
	handlers = evas_list_remove_list(handlers, handlers);
     }
   return 1;
}

EAPI Evas_Object *
e_winilist_add(Evas *e)
{
   Data *d;
   
   d = E_NEW(Data, 1);
   d->o_frame = e_scrollframe_add(e);
   d->o_ilist = e_ilist_add(e);
   evas_object_data_set(d->o_frame, "..[winilist]", d);
   e_ilist_selector_set(d->o_ilist, 1);

   e_scrollframe_child_set(d->o_frame, d->o_ilist);
   evas_object_show(d->o_ilist);
   
   winilists = evas_list_append(winilists, d);
   evas_object_event_callback_add(d->o_frame, EVAS_CALLBACK_DEL, _cb_object_del, NULL);
   evas_object_event_callback_add(d->o_frame, EVAS_CALLBACK_RESIZE, _cb_object_resize, NULL);
   
   printf("refill1\n");
   _refill(d);
   
   return d->o_frame;
}

EAPI void
e_winilist_border_select_callback_set(Evas_Object *obj, void (*func) (void *data, E_Border *bd), void *data)
{
   Data *d;
   
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
   d->select.func = func;
   d->select.data = data;
}

EAPI void
e_winilist_special_append(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data1, void *data2), void *data1, void *data2)
{
   Data *d;
   
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
     {
	Special *s;
	
	s = E_NEW(Special, 1);
	d->special.prepend = evas_list_prepend(d->special.prepend, s);
	s->icon = icon;
	if (label) s->label = evas_stringshare_add(label);
	s->func = func;
	s->data1 = data1;
	s->data2 = data2;
	d->special.changed = 1;
     }
   printf("refill2\n");
   _refill(d);
}

EAPI void
e_winilist_special_prepend(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data1, void *data2), void *data1, void *data2)
{
   Data *d;
   
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
     {
	Special *s;
	
	s = E_NEW(Special, 1);
	d->special.append = evas_list_append(d->special.append, s);
	s->icon = icon;
	if (label) s->label = evas_stringshare_add(label);
	s->func = func;
	s->data1 = data1;
	s->data2 = data2;
	d->special.changed = 1;
     }
   printf("refill3\n");
   _refill(d);
}

EAPI void
e_winilist_optimial_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Data *d;
   
   if (w) *w = 0;
   if (h) *h = 0;
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
   printf("OPT: %ix%i\n", d->optimal_size.w, d->optimal_size.h);
   if (w) *w = d->optimal_size.w;
   if (h) *h = d->optimal_size.h;
}

///////////////////////////////////////////////////////////////////////////////

/* internal calls */
static void
_cb_item_sel(void *data, void *data2)
{
   Data *d;
   
   d = data;
   printf("d = %p\n", d);
   if (d->select.func)
     {
	printf("d->select.func = %p\n", d->select.func);
	d->select.func(d->select.data, data2);
     }
}

static void
_cb_special_sel(void *data, void *data2)
{
   Special *s;
   
   s = data;
   printf("s = %p\n", s);
   if (s->func)
     {
	printf("s->func = %p\n", s->func);
	s->func(s->data1, s->data2);
     }
}

static void
_cb_object_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Data *d;
   
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
   evas_object_del(d->o_ilist);
   winilists = evas_list_remove(winilists, d);
   
   while (d->borders)
     {
	e_object_unref(E_OBJECT(d->borders->data));
	d->borders = evas_list_remove_list(d->borders, d->borders);
     }
   while (d->labels)
     {
	evas_stringshare_del(d->labels->data);
	d->labels = evas_list_remove_list(d->labels, d->labels);
     }
   
   while (d->special.prepend)
     {
	Special *s;
	
	s = d->special.prepend->data;
	if (s->icon)
	  {
	     evas_object_del(s->icon);
	     s->icon = NULL;
	  }
	if (s->label)
	  {
	     evas_stringshare_del(s->label);
	     s->label = NULL;
	  }
	free(s);
	d->special.prepend = evas_list_remove_list(d->special.prepend, d->special.prepend);
     }
   while (d->special.append)
     {
	Special *s;
	
	s = d->special.append->data;
	if (s->icon)
	  {
	     evas_object_del(s->icon);
	     s->icon = NULL;
	  }
	if (s->label)
	  {
	     evas_stringshare_del(s->label);
	     s->label = NULL;
	  }
	free(s);
	d->special.append = evas_list_remove_list(d->special.append, d->special.append);
     }
   
   free(d);
}

static void
_cb_object_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Data *d;
   Evas_Coord lw, lh, vw, vh;
   
   d = evas_object_data_get(obj, "..[winilist]");
   if (!d) return;
   e_ilist_min_size_get(d->o_ilist, &lw, &lh);
   e_scrollframe_child_viewport_size_get(d->o_frame, &vw, &vh);
   evas_object_resize(d->o_ilist, vw, lh);
}

static void
_refill(Data *d)
{
   Evas_Coord w, h, lw, lh, vw, vh;
   Evas_List *borders, *l, *l2, *l3;
   
   borders = e_border_client_list();
   if (!d->special.changed)
     {
	int changed = 0;
	
	if ((borders) && (d->borders))
	  {
	     Evas_List *tmp = NULL;
	     
	     changed = 0;
	     for (l = borders; l; l = l->next)
	       {
		  E_Border *bd;
		  
		  bd = l->data;
		  if (e_object_is_del(E_OBJECT(bd))) continue;
		  if ((!bd->client.icccm.accepts_focus) &&
		      (!bd->client.icccm.take_focus)) continue;
		  if (bd->client.netwm.state.skip_taskbar) continue;
		  if (bd->user_skip_winlist) continue;
		  tmp = evas_list_append(tmp, bd);
	       }
	     if (!(tmp && d->borders))
	       {
		  changed = 1;
	       }
	     else
	       {
		  if (evas_list_count(tmp) !=
		      evas_list_count(d->borders))
		    {
		       changed = 1;
		    }
		  else
		    {
		       for (l = tmp, l2 = d->borders, l3 = d->labels;
			    l && l2 && l3; 
			    l = l->next, l2 = l2->next, l3 = l3->next)
			 {
			    E_Border *bd, *bd2;
			    const char *title;
			    
			    bd = l->data;
			    bd2 = l2->data;
			    if (bd != bd2)
			      {
				 changed = 1;
				 break;
			      }
			    
			    title = "???";
			    if (bd->client.netwm.name) title = bd->client.netwm.name;
			    else if (bd->client.icccm.title) title = bd->client.icccm.title;
			    if (strcmp(title, l3->data))
			      {
				 changed = 1;
				 break;
			      }
			 }
		    }
	       }
	     if (tmp) evas_list_free(tmp);
	  }
	else
	  changed = 1;
	if (!changed) return;
     }
   printf("-------REFILL\n");
   d->special.changed = 0;
   while (d->borders)
     {
	e_object_unref(E_OBJECT(d->borders->data));
	d->borders = evas_list_remove_list(d->borders, d->borders);
     }
   while (d->labels)
     {
	evas_stringshare_del(d->labels->data);
	d->labels = evas_list_remove_list(d->labels, d->labels);
     }
   
   e_ilist_freeze(d->o_ilist);
   e_ilist_clear(d->o_ilist);
   for (l = d->special.prepend; l; l = l->next)
     {
	Special *s;
	
	s = l->data;
	e_ilist_append(d->o_ilist, s->icon, s->label, 0, _cb_special_sel, NULL,
		       s, NULL);
     }
   for (l = borders; l; l = l->next)
     {
	E_Border *bd;
	const char *title;
	
	bd = l->data;
	if (e_object_is_del(E_OBJECT(bd))) continue;
	if ((!bd->client.icccm.accepts_focus) &&
	    (!bd->client.icccm.take_focus)) continue;
	if (bd->client.netwm.state.skip_taskbar) continue;
	if (bd->user_skip_winlist) continue;
	
	title = "???";
	if (bd->client.netwm.name) title = bd->client.netwm.name;
	else if (bd->client.icccm.title) title = bd->client.icccm.title;
	e_object_ref(E_OBJECT(bd));
	d->borders = evas_list_append(d->borders, bd);
	d->labels = evas_list_append(d->labels, evas_stringshare_add(title));
	e_ilist_append(d->o_ilist, NULL/*icon*/, title, 0,
		       _cb_item_sel,
		       NULL, d, bd);
     }
   for (l = d->special.append; l; l = l->next)
     {
	Special *s;
	
	s = l->data;
	e_ilist_append(d->o_ilist, s->icon, s->label, 0, _cb_special_sel, NULL,
		       s, NULL);
     }
   
   e_ilist_thaw(d->o_ilist);
   
   /* FIXME: figure out optimal size */
   e_ilist_min_size_get(d->o_ilist, &lw, &lh);
   e_scrollframe_child_viewport_size_get(d->o_frame, &vw, &vh);
   evas_object_geometry_get(d->o_frame, NULL, NULL, &w, &h);
   evas_object_resize(d->o_ilist, vw, lh);

   d->optimal_size.w = lw + (w - vw);
   d->optimal_size.h = lh + (h - vh);
}

static int
_cb_border_add(void *data, int ev_type, void *event)
{
   E_Event_Border_Add *ev;
   
   ev = event;
   printf("refill4\n");
     {
	Evas_List *l;
	
	for (l = winilists; l; l = l->next) _refill(l->data);
     }
   return 1;
}

static int
_cb_border_remove(void *data, int ev_type, void *event)
{
   E_Event_Border_Remove *ev;
   
   ev = event;
   printf("refill5\n");
     {
	Evas_List *l;
	
	for (l = winilists; l; l = l->next) _refill(l->data);
     }
   return 1;
}

static int
_cb_border_show(void *data, int ev_type, void *event)
{
   E_Event_Border_Show *ev;
   
   ev = event;
   printf("refill6\n");
     {
	Evas_List *l;
	
	for (l = winilists; l; l = l->next) _refill(l->data);
     }
   return 1;
}

static int
_cb_border_hide(void *data, int ev_type, void *event)
{
   E_Event_Border_Hide *ev;
   
   ev = event;
   printf("refill7\n");
     {
	Evas_List *l;
	
	for (l = winilists; l; l = l->next) _refill(l->data);
     }
   return 1;
}

static int
_cb_border_property(void *data, int ev_type, void *event)
{
   E_Event_Border_Property *ev;
   
   ev = event;
   printf("refill8\n");
   /* FIXME: should really be optimal on what properties warrant a refill */
     {
	Evas_List *l;
	
	for (l = winilists; l; l = l->next) _refill(l->data);
     }
   return 1;
}

