#include "Evas_private.h"
#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int
_evas_point_in_object(Evas e, Evas_Object o, int x, int y)
{
   int ox, oy, ow, oh;

   if (o->delete_me) return 0;
   _evas_object_get_current_translated_coords(e, o, &ox, &oy, &ow, &oh);
   if ((x >= ox) && (x < (ox + ow)) && (y >= oy) && (y < (oy + oh)))
      return 1;
   return 0;
}

Evas_Object
_evas_highest_object_at_point(Evas e, int x, int y)
{
   Evas_List l, ll;
   Evas_Layer layer;
   Evas_Object o;
   
   o = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me))
	       {
		  if (_evas_point_in_object(e, ob, x, y)) 
		     o = ob;
	       }
	  }
     }
   return o;
}

Evas_List
_evas_objects_at_point(Evas e, int x, int y)
{
   Evas_List l, ll, objs;
   Evas_Layer layer;
   
   objs = NULL;
   for (l = e->layers; l ; l = l->next)
     {
	layer = l->data;
	
	for (ll = layer->objects; ll; ll = ll->next)
	  {
	     Evas_Object ob;
	     
	     ob = ll->data;
	     if ((ob->current.visible) && (!ob->pass_events) && (!ob->delete_me))
	       {
		  if (_evas_point_in_object(e, ll->data, x, y))
		     objs = evas_list_prepend(objs, ll->data);
	       }
	  }
     }
   return objs;
}	 
   
/* events */
void
evas_event_button_down(Evas e, int x, int y, int b)
{
   Evas_Object o;
   
   if (!e) return;
   if ((b < 1) || (b > 32)) return;
   if (!e->mouse.buttons) 
      {
	 o = _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
	 e->mouse.button_object = o;
      }
   e->mouse.buttons |= (1 << (b - 1));
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_DOWN, 
			  b, x, y);
}

void
evas_event_button_up(Evas e, int x, int y, int b)
{
   Evas_Object o;
   
   if (!e) return;
   if ((b < 1) || (b > 32)) return;
   e->mouse.buttons &= ~(1 << (b - 1));
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_UP, 
			  b, x, y);
   if (!e->mouse.buttons) 
      {
	 if (e->mouse.object != e->mouse.button_object)
	    _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_OUT, 
				e->mouse.buttons, e->mouse.x, e->mouse.y);
	 e->mouse.button_object = NULL;
      }
}

void
evas_event_move(Evas e, int x, int y)
{
   Evas_Object o;

   if (!e) return;
   o = _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
   if (o != e->mouse.object)
     {
	if (e->mouse.object)
	  {
	     if (!e->mouse.button_object)
	       {
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_OUT, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
	       }
	     else
		_evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_MOVE, 
				    e->mouse.buttons, e->mouse.x, e->mouse.y);
	  }
	e->mouse.x = x;
	e->mouse.y = y;
	e->mouse.object = o;
	if (e->mouse.object)
	  {
	     if (!e->mouse.button_object)
	       {
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_IN, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
		  _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
				      e->mouse.buttons, e->mouse.x, e->mouse.y);
	       }
	  }
	return;
     }   
   e->mouse.x = x;
   e->mouse.y = y;
   if (e->mouse.button_object)
      _evas_callback_call(e, e->mouse.button_object, CALLBACK_MOUSE_MOVE, 
			  e->mouse.buttons, e->mouse.x, e->mouse.y);
   else if (e->mouse.object)
      _evas_callback_call(e, e->mouse.object, CALLBACK_MOUSE_MOVE, 
			  e->mouse.buttons, e->mouse.x, e->mouse.y);
}

void
evas_event_enter(Evas e)
{
   if (!e) return;
   e->mouse.in = 1;
}

void
evas_event_leave(Evas e)
{
   if (!e) return;
   e->mouse.in = 0;
}

Evas_Object
evas_get_object_under_mouse(Evas e)
{
   if (!e) return NULL;
   return _evas_highest_object_at_point(e, e->mouse.x, e->mouse.y);
}

int
evas_pointer_in(Evas e)
{
   if (!e) return 0;
   return e->mouse.in;
}

void
evas_pointer_pos(Evas e, int *x, int *y)
{
   if (!e) return;
   if (x) *x = e->mouse.x;
   if (y) *y = e->mouse.y;
}

int
evas_pointer_buttons(Evas e)
{
   if (!e) return 0;
   return e->mouse.buttons;
}

void
evas_pinter_ungrab(Evas e)
{
   e->mouse.buttons = 0;
   e->mouse.button_object = NULL;
}
