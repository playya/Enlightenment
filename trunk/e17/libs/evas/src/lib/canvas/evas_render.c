#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_damage_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Evas_Rectangle *r;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   r = malloc(sizeof(Evas_Rectangle));
   if (!r) return;
   r->x = x; r->y = y; r->w = w; r->h = h;
   e->damages = evas_list_append(e->damages, r);
   e->changed = 1;
}

void
evas_obscured_rectangle_add(Evas *e, int x, int y, int w, int h)
{
   Evas_Rectangle *r;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   r = malloc(sizeof(Evas_Rectangle));
   if (!r) return;
   r->x = x; r->y = y; r->w = w; r->h = h;
   e->obscures = evas_list_append(e->obscures, r);
}

void
evas_obscured_clear(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
   while (e->obscures)
     {
	Evas_Rectangle *r;
	
	r = (Evas_Rectangle *)e->obscures->data;
	e->obscures = evas_list_remove(e->obscures, r);
	free(r);
     }
}

Evas_List *
evas_render_updates(Evas *e)
{
   Evas_List *updates = NULL;
   Evas_List *obscuring_objects = NULL;
   Evas_List *obscuring_objects_orig = NULL;
   Evas_List *active_objects = NULL;
   Evas_List *delete_objects = NULL;
   Evas_List *restack_objects = NULL;
   Evas_List *ll;
   Evas_Object_List *l;
   void *surface;
   int ux, uy, uw, uh;
   int cx, cy, cw, ch;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!e->changed) return NULL;
   
   if (e->output.changed)
     {
	e->engine.func->output_resize(e->engine.data.output,
				      e->output.w, e->output.h);
	e->engine.func->output_redraws_rect_add(e->engine.data.output, 
						0, 0, 
						e->output.w, e->output.h);
     }
   
   /* phase 1. add extra updates for changed objects */
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;
	
	lay = (Evas_Layer *)l;
	for (l2 = (Evas_Object_List *)lay->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     
	     obj = (Evas_Object *)l2;
	     evas_object_clip_recalc(obj);
	     /* build active object list */
	     if (evas_object_is_active(obj))
	       active_objects = evas_list_append(active_objects, obj);
	     /* something changed... maybe... */
	     if (obj->changed)
	       {
		  if ((obj->restack) && 
		      (!obj->clip.clipees) &&
		      evas_object_is_active(obj))		      
		    restack_objects = evas_list_append(restack_objects, obj);
		  else
		    obj->func->render_pre(obj);
	       }
	     /* nothng changed at all */
	     else
	       {
		  if (evas_object_is_opaque(obj) &&
		      evas_object_is_visible(obj) &&
		      (!obj->clip.clipees))
		    e->engine.func->output_redraws_rect_del(e->engine.data.output,
							    obj->cur.cache.clip.x, 
							    obj->cur.cache.clip.y, 
							    obj->cur.cache.clip.w, 
							    obj->cur.cache.clip.h);
	       }
	  }
     }
   /* phase 2. force updates for restacks */
   while (restack_objects)
     {
	Evas_Object *obj;
	
	obj = restack_objects->data;
	restack_objects = evas_list_remove(restack_objects, obj);
	obj->func->render_pre(obj);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						obj->prev.cache.clip.x, 
						obj->prev.cache.clip.y, 
						obj->prev.cache.clip.w, 
						obj->prev.cache.clip.h);
	e->engine.func->output_redraws_rect_add(e->engine.data.output,
						obj->cur.cache.clip.x, 
						obj->cur.cache.clip.y, 
						obj->cur.cache.clip.w, 
						obj->cur.cache.clip.h);
     }
   /* phase 3. add exposes */
   while (e->damages)
     {
	Evas_Rectangle *r;
	
	r = e->damages->data;
	e->damages = evas_list_remove(e->damages, r);
	e->engine.func->output_redraws_rect_add(e->engine.data.output, 
					       r->x, r->y, r->w, r->h);
	free(r);
     }
   /* phase 4. add obscures */
   for (ll = e->obscures; ll; ll = ll->next)
     {
	Evas_Rectangle *r;
	
	r = ll->data;
	e->engine.func->output_redraws_rect_del(e->engine.data.output,
					       r->x, r->y, r->w, r->h);
     }
   /* build obscure objects list of active objects that obscure */
   for (ll = active_objects; ll; ll = ll->next)
     {
	Evas_Object *obj;
	
	obj = (Evas_Object *)(ll->data);
	if (evas_object_is_opaque(obj) && evas_object_is_visible(obj) &&
	    (!obj->clip.clipees) && (obj->cur.color.a >= 255))
	  obscuring_objects = evas_list_append(obscuring_objects, obj);
     }
   /* save this list */
   obscuring_objects_orig = obscuring_objects;
   obscuring_objects = NULL;
   /* phase 5. go thu each update rect and render objects in it*/
   while ((surface = 
	   e->engine.func->output_redraws_next_update_get(e->engine.data.output,
							 &ux, &uy, &uw, &uh,
							 &cx, &cy, &cw, &ch)))
     {
	Evas_Rectangle *rect;
	int off_x, off_y;

	rect = malloc(sizeof(Evas_Rectangle));
	if (rect)
	  {
	     rect->x = ux; rect->y = uy; rect->w = uw; rect->h = uh;
	     updates = evas_list_append(updates, rect);
	  }
	off_x = cx - ux;
	off_y = cy - uy;
	/* build obscuring objects list (in order from bottom to top) */
	for (ll = obscuring_objects_orig; ll; ll = ll->next)
	  {
	     Evas_Object *obj;
	     
	     obj = (Evas_Object *)(ll->data);
	     if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh))
	       obscuring_objects = evas_list_append(obscuring_objects, obj);
	  }
	/* render all object that intersect with rect */
	for (ll = active_objects; ll; ll = ll->next)
	  {
	     Evas_Object *obj;
	     Evas_List *l3;
	     obj = (Evas_Object *)(ll->data);
	     /* if its in our outpout rect and it doesnt clip anything */
	     if (evas_object_is_in_output_rect(obj, ux, uy, uw, uh) &&
		 (!obj->clip.clipees) &&
		 (obj->cur.visible) &&
		 (!obj->delete_me) &&
		 (obj->cur.cache.clip.visible) &&
		 (obj->cur.color.a > 0))
	       {
		  int x, y, w, h;
		  
		  if ((obscuring_objects) && (obscuring_objects->data == obj))
		    obscuring_objects = evas_list_remove(obscuring_objects, obj);
		  x = cx; y = cy; w = cw; h = ch;
		  RECTS_CLIP_TO_RECT(x, y, w, h,
				     obj->cur.cache.clip.x + off_x,
				     obj->cur.cache.clip.y + off_y,
				     obj->cur.cache.clip.w,
				     obj->cur.cache.clip.h);
		  if ((w > 0) && (h > 0))
		    {
		       e->engine.func->context_clip_set(e->engine.data.output, 
							e->engine.data.context, 
							x, y, w, h);
#if 1 /* FIXME: this can slow things down... figure out optimum... coverage */
		       for (l3 = obscuring_objects; l3; l3 = l3->next)
			 {
			    Evas_Object *obj2;
			    
			    obj2 = (Evas_Object *)l3->data;
			    e->engine.func->context_cutout_add(e->engine.data.output,
							       e->engine.data.context,
							       obj2->cur.cache.clip.x + off_x,
							       obj2->cur.cache.clip.y + off_y,
							       obj2->cur.cache.clip.w,
							       obj2->cur.cache.clip.h);
			 }
#endif		  
		       obj->func->render(obj, 
					 e->engine.data.output,
					 e->engine.data.context,
					 surface,
					 off_x, off_y);
		       e->engine.func->context_cutout_clear(e->engine.data.output,
							    e->engine.data.context);
		    }
	       }
	  }
	/* punch rect out */
	e->engine.func->output_redraws_next_update_push(e->engine.data.output, 
						       surface, 
						       ux, uy, uw, uh);
	/* free obscuring objects list */
	obscuring_objects = evas_list_free(obscuring_objects);
     }
   /* flush redraws */
   e->engine.func->output_flush(e->engine.data.output);
   /* clear redraws */
   e->engine.func->output_redraws_clear(e->engine.data.output);   
   /* and do a post render pass */
   for (l = (Evas_Object_List *)e->layers; l; l = l->next)
     {
	Evas_Object_List *l2;
	Evas_Layer *lay;
	
	lay = (Evas_Layer *)l;
	for (l2 = (Evas_Object_List *)lay->objects; l2; l2 = l2->next)
	  {
	     Evas_Object *obj;
	     
	     obj = (Evas_Object *)l2;
	     obj->pre_render_done = 0;
	     if (obj->changed)
	       {
		  obj->func->render_post(obj);
		  obj->restack = 0;
		  obj->changed = 0;
	       }
	     /* if the object is flagged for deletion - note it */
	     if (obj->delete_me == 2)
	       delete_objects = evas_list_append(delete_objects, obj);
	     if (obj->delete_me) obj->delete_me ++;
	  }
     }
   /* delete all objects flagged for deletion now */
   while (delete_objects)
     {
	Evas_Object *obj;
	
	obj = (Evas_Object *)(delete_objects->data);
	delete_objects = evas_list_remove(delete_objects, obj);
	evas_object_free(obj, 1);
     }
   /* free our obscuring object list */
   obscuring_objects_orig = evas_list_free(obscuring_objects_orig);
   /* free our active object list */
   active_objects = evas_list_free(active_objects);
   e->changed = 0;
   e->viewport.changed = 0;
   e->output.changed = 0;
   return updates;
}

void
evas_render_updates_free(Evas_List *updates)
{
   while (updates)
     {
	free(updates->data);
	updates = evas_list_remove(updates, updates->data);
     }
}

void
evas_render(Evas *e)
{
   Evas_List *updates;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   updates = evas_render_updates(e);
   if (updates) evas_render_updates_free(updates);
}
