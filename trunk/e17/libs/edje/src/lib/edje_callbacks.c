#include "Edje.h"
#include "edje_private.h"

void
_edje_mouse_in_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_In *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,in", rp->part->name);
   return;
   e = NULL;
}

void
_edje_mouse_out_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   _edje_emit(ed, "mouse,out", rp->part->name);
   return;
   e = NULL;
}

void
_edje_mouse_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
#ifndef EDJE_FB_ONLY
   if (ecore_event_current_type_get() == ECORE_X_EVENT_MOUSE_BUTTON_DOWN)
     {
	Ecore_X_Event_Mouse_Button_Down *evx;
	
	evx = ecore_event_current_event_get();
	if (evx)
	  {
	     if (evx->triple_click)
	       snprintf(buf, sizeof(buf), "mouse,down,%i,triple", ev->button);
	     else if (evx->double_click)
	       snprintf(buf, sizeof(buf), "mouse,down,%i,double", ev->button);
	     else
	       snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
	  }
	else
	  snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
     }
   else
#endif     
     snprintf(buf, sizeof(buf), "mouse,down,%i", ev->button);
   _edje_ref(ed);
   if ((rp->part->dragable.x) || (rp->part->dragable.y))
     {
	if (rp->drag.down.count == 0)
	  {
	     if (rp->part->dragable.x)
	       rp->drag.down.x = ev->canvas.x;
	     if (rp->part->dragable.y)
	       rp->drag.down.y = ev->canvas.y;
	     _edje_emit(ed, "drag,start", rp->part->name);
	  }
	rp->drag.down.count++;
     }
   if (rp->clicked_button == 0)
     {
	rp->clicked_button = ev->button;
	rp->still_in = 1;
     }
   _edje_freeze(ed);
   _edje_emit(ed, buf, rp->part->name);
   _edje_recalc(ed);
   _edje_thaw(ed);   
   _edje_unref(ed);
   return;
   e = NULL;
}

void
_edje_mouse_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,up,%i", ev->button);
   _edje_ref(ed);
   _edje_emit(ed, buf, rp->part->name);
   if ((rp->part->dragable.x) || (rp->part->dragable.y))
     {
	if (rp->drag.down.count > 0)
	  {
	     rp->drag.down.count--;
	     if (rp->drag.down.count == 0)
	       {
		  rp->drag.need_reset = 1;
		  ed->dirty = 1;
		  _edje_emit(ed, "drag,stop", rp->part->name);
	       }
	  }
     }
   _edje_freeze(ed);
   if ((rp->still_in) && (rp->clicked_button == ev->button))
     {
	rp->clicked_button = 0;
	rp->still_in = 0;
	snprintf(buf, sizeof(buf), "mouse,clicked,%i", ev->button);
	_edje_emit(ed, buf, rp->part->name);
     }
   _edje_recalc(ed);
   _edje_thaw(ed);
   _edje_unref(ed);
   return;
   e = NULL;
}

void
_edje_mouse_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   if (rp->still_in)
     {
	Evas_Coord x, y, w, h;
	
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	if ((ev->cur.canvas.x < x) || (ev->cur.canvas.y < y) || 
	    (ev->cur.canvas.x >= (x + w)) || (ev->cur.canvas.y >= (y + h)))
	  rp->still_in = 0;
     }
   else
     {
	Evas_Coord x, y, w, h;
	
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	if ((ev->cur.canvas.x >= x) && (ev->cur.canvas.y >= y) && 
	    (ev->cur.canvas.x < (x + w)) && (ev->cur.canvas.y < (y + h)))
	  rp->still_in = 1;
     }
   _edje_freeze(ed);
   if ((rp->part->dragable.x) || (rp->part->dragable.y))
     {
	if (rp->drag.down.count > 0)
	  {
	     if (rp->part->dragable.x)
	       rp->drag.tmp.x = ev->cur.canvas.x - rp->drag.down.x;
	     if (rp->part->dragable.y)
	       rp->drag.tmp.y = ev->cur.canvas.y - rp->drag.down.y;
	     ed->dirty = 1;
	  }
     }
   _edje_ref(ed);
   _edje_emit(ed, "mouse,move", rp->part->name);
/* FIXME: this FUCKS up badly!!!! */   
/*   ed->calc_only = 1; */
   _edje_recalc(ed);
/*   ed->calc_only = 0; */
   if ((rp->part->dragable.x) || (rp->part->dragable.y))
     {
	if (rp->drag.down.count > 0)
	  {
	     double dx, dy;
	     int dir;
	     
	     dir = _edje_part_dragable_calc(ed, rp, &dx, &dy);
	     if ((dx != rp->drag.val.x) || (dy != rp->drag.val.y))
	       {
		  rp->drag.val.x = dx;
		  rp->drag.val.y = dy;
		  _edje_emit(ed, "drag", rp->part->name);
		  ed->dirty = 1;
		  _edje_recalc(ed);
	       }
	  }
     }
   _edje_unref(ed);
   _edje_thaw(ed); 
   return;
   e = NULL;
}

void
_edje_mouse_wheel_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Edje *ed;
   Edje_Real_Part *rp;
   char buf[256];
   
   ev = event_info;
   ed = data;
   rp = evas_object_data_get(obj, "real_part");
   if (!rp) return;
   snprintf(buf, sizeof(buf), "mouse,wheel,%i,%i", ev->direction, (ev->z < 0) ? (-1) : (1));
   _edje_emit(ed, buf, rp->part->name);
   return;
   e = NULL;
}

int
_edje_timer_cb(void *data)
{
   double t;
   Evas_List *l;
   Evas_List *animl = NULL;
   Edje *ed;
   
   t = ecore_time_get();
   for (l = _edje_animators; l; l = l->next)
     {
	ed = l->data;
	_edje_ref(ed);
	animl = evas_list_append(animl, l->data);
     }
   while (animl)
     {
	Evas_List *newl = NULL;
	
	ed = animl->data;
	_edje_block(ed);
	_edje_freeze(ed);
	animl = evas_list_remove(animl, animl->data);
	if ((!ed->paused) && (!ed->delete_me))
	  {
	     ed->walking_actions = 1;
	     for (l = ed->actions; l; l = l->next)
	       newl = evas_list_append(newl, l->data);
	     while (newl)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = newl->data;
		  newl = evas_list_remove(newl, newl->data);
		  if (!runp->delete_me)
		    _edje_program_run_iterate(runp, t);
		  if (_edje_block_break(ed))
		    {
		       evas_list_free(newl);
		       newl = NULL;
		       goto break_prog;
		    }
	       }
	     for (l = ed->actions; l; l = l->next)
	       newl = evas_list_append(newl, l->data);
	     while (newl)
	       {
		  Edje_Running_Program *runp;
		  
		  runp = newl->data;
		  newl = evas_list_remove(newl, newl->data);
		  if (runp->delete_me)
		    {
		       _edje_anim_count--;		       
		       runp->edje->actions = 
			 evas_list_remove(runp->edje->actions, runp);
		       if (!runp->edje->actions)
			 _edje_animators = 
			 evas_list_remove(_edje_animators, runp->edje);
		       free(runp);
		    }
	       }
	     ed->walking_actions = 0;
	  }
	break_prog:
	_edje_unblock(ed);
	_edje_thaw(ed);
	_edje_unref(ed);
     }
   if (_edje_anim_count > 0) return 1;
   _edje_timer = NULL;
   return 0;
   data = NULL;
}

int
_edje_pending_timer_cb(void *data)
{
   Edje_Pending_Program *pp;
   
   pp = data;
   pp->edje->pending_actions = evas_list_remove(pp->edje->pending_actions, pp);
   _edje_program_run(pp->edje, pp->program, 1, "", "");
   free(pp);
   return 0;
}
