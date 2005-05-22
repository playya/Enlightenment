#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private magic number for polygon objects */
static const char o_type[] = "polygon";

/* private struct for line object internal data */
typedef struct _Evas_Object_Polygon      Evas_Object_Polygon;
typedef struct _Evas_Polygon_Point       Evas_Polygon_Point;

struct _Evas_Object_Polygon
{
   DATA32            magic;
   Evas_List        *points;
   char              changed : 1;

   void             *engine_data;
};

struct _Evas_Polygon_Point
{
   Evas_Coord x, y;
};

/* private methods for polygon objects */
static void evas_object_polygon_init(Evas_Object *obj);
static void *evas_object_polygon_new(void);
static void evas_object_polygon_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_polygon_free(Evas_Object *obj);
static void evas_object_polygon_render_pre(Evas_Object *obj);
static void evas_object_polygon_render_post(Evas_Object *obj);

static int evas_object_polygon_is_opaque(Evas_Object *obj);
static int evas_object_polygon_was_opaque(Evas_Object *obj);
static int evas_object_polygon_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static int evas_object_polygon_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_polygon_free,
     evas_object_polygon_render,
     evas_object_polygon_render_pre,
     evas_object_polygon_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_polygon_is_opaque,
     evas_object_polygon_was_opaque,
     evas_object_polygon_is_inside,
     evas_object_polygon_was_inside,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

/**
 * @defgroup Evas_Polygon_Group Polygon Functions
 *
 * Functions that operate on evas polygon objects.
 */

/**
 * Adds a new evas polygon object to the given evas.
 * @param   e The given evas.
 * @return  A new evas polygon object.
 * @ingroup Evas_Polygon_Group
 */
Evas_Object *
evas_object_polygon_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_polygon_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * Adds the given point to the given evas polygon object.
 * @param obj The given evas polygon object.
 * @param x   The X coordinate of the given point.
 * @param y   The Y coordinate of the given point.
 * @ingroup Evas_Polygon_Group
 */
void
evas_object_polygon_point_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Polygon *o;
   Evas_Polygon_Point *p;
   Evas_Coord min_x, max_x, min_y, max_y;
   int is, was = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Polygon *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Polygon, MAGIC_OBJ_POLYGON);
   return;
   MAGIC_CHECK_END();
   if (!evas_event_passes_through(obj))
     was = evas_object_is_in_output_rect(obj,
					 obj->layer->evas->pointer.x,
					 obj->layer->evas->pointer.y, 1, 1);
   p = malloc(sizeof(Evas_Polygon_Point));
   if (!p) return;
   p->x = x;
   p->y = y;

   if (!o->points)
     {
	obj->cur.geometry.x = p->x;
	obj->cur.geometry.y = p->y;
	obj->cur.geometry.w = 2.0;
	obj->cur.geometry.h = 2.0;
     }
   else
     {
	if (x < obj->cur.geometry.x) min_x = x;
	else min_x = obj->cur.geometry.x;
	if (x > (obj->cur.geometry.x + obj->cur.geometry.w - 2.0)) max_x = x;
	else max_x = obj->cur.geometry.x + obj->cur.geometry.w - 2.0;
	if (y < obj->cur.geometry.y) min_y = y;
	else min_y = obj->cur.geometry.y;
	if (y > (obj->cur.geometry.y + obj->cur.geometry.h - 2.0)) max_y = y;
	else max_y = obj->cur.geometry.y + obj->cur.geometry.h - 2.0;
	obj->cur.geometry.x = min_x;
	obj->cur.geometry.y = min_y;
	obj->cur.geometry.w = max_x - min_x + 2.0;
	obj->cur.geometry.h = max_y - min_y + 2.0;
     }
   o->points = evas_list_append(o->points, p);

   obj->cur.cache.geometry.validity = 0;
   o->changed = 1;
   evas_object_change(obj);
   evas_object_coords_recalc(obj);
   is = evas_object_is_in_output_rect(obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if (!evas_event_passes_through(obj))
     {
	if ((is ^ was) && obj->cur.visible)
	  evas_event_feed_mouse_move(obj->layer->evas,
				     obj->layer->evas->pointer.x,
				     obj->layer->evas->pointer.y,
				     NULL);
     }
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}

/**
 * Removes all of the points from the given evas polygon object.
 * @param   obj The given polygon object.
 * @ingroup Evas_Polygon_Group
 */
void
evas_object_polygon_points_clear(Evas_Object *obj)
{
   Evas_Object_Polygon *o;
   int is, was;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Polygon *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Polygon, MAGIC_OBJ_POLYGON);
   return;
   MAGIC_CHECK_END();
   was = evas_object_is_in_output_rect(obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   while (o->points)
     {
	free(o->points->data);
	o->points = evas_list_remove(o->points, o->points->data);
     }
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.cache.geometry.validity = 0;
   o->changed = 1;
   evas_object_change(obj);
   evas_object_coords_recalc(obj);
   is = evas_object_is_in_output_rect(obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur.visible)
     evas_event_feed_mouse_move(obj->layer->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				NULL);
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}





/* all nice and private */
static void
evas_object_polygon_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_polygon_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 0.0;
   obj->cur.geometry.h = 0.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_polygon_new(void)
{
   Evas_Object_Polygon *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Polygon));
   o->magic = MAGIC_OBJ_POLYGON;
   return o;
}

static void
evas_object_polygon_free(Evas_Object *obj)
{
   Evas_Object_Polygon *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Polygon *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Polygon, MAGIC_OBJ_POLYGON);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   while (o->points)
     {
	free(o->points->data);
	o->points = evas_list_remove(o->points, o->points->data);
     }
   o->engine_data = obj->layer->evas->engine.func->polygon_points_clear(obj->layer->evas->engine.data.output,
									obj->layer->evas->engine.data.context,
									o->engine_data);
   o->magic = 0;
   free(o);
}

static void
evas_object_polygon_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Polygon *o;
   Evas_List *l;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Polygon *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    obj->cur.cache.clip.r,
						    obj->cur.cache.clip.g,
						    obj->cur.cache.clip.b,
						    obj->cur.cache.clip.a);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   o->engine_data = obj->layer->evas->engine.func->polygon_points_clear(obj->layer->evas->engine.data.output,
									obj->layer->evas->engine.data.context,
									o->engine_data);
   for (l = o->points; l; l = l->next)
     {
	Evas_Polygon_Point *p;
	int px, py;

	p = l->data;
	px = evas_coord_world_x_to_screen(obj->layer->evas, p->x);
	py = evas_coord_world_y_to_screen(obj->layer->evas, p->y);
	o->engine_data = obj->layer->evas->engine.func->polygon_point_add(obj->layer->evas->engine.data.output,
									  obj->layer->evas->engine.data.context,
									  o->engine_data,
									  px + x, py + y);
     }
   if (o->engine_data)
     obj->layer->evas->engine.func->polygon_draw(output,
						 context,
						 surface,
						 o->engine_data);
}

static void
evas_object_polygon_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Polygon *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw lines */
   o = (Evas_Object_Polygon *)(obj->object_data);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* caluclate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h) ||
       (o->changed))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_polygon_render_post(Evas_Object *obj)
{
   Evas_Object_Polygon *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Polygon *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->changed = 0;
}

static int
evas_object_polygon_is_opaque(Evas_Object *obj)
{
   Evas_Object_Polygon *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire line it occupies */
   o = (Evas_Object_Polygon *)(obj->object_data);
   return 0;
}

static int
evas_object_polygon_was_opaque(Evas_Object *obj)
{
   Evas_Object_Polygon *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* previously fully opaque over the entire line it occupies */
   o = (Evas_Object_Polygon *)(obj->object_data);
   return 0;
}

static int
evas_object_polygon_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Polygon *o;

   /* this returns 1 if the canvas co-ordinates are inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Polygon *)(obj->object_data);
   return 1;
   x = 0;
   y = 0;
}

static int
evas_object_polygon_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Polygon *o;

   /* this returns 1 if the canvas co-ordinates were inside the object based */
   /* on object private data. not much use for rects, but for polys, images */
   /* and other complex objects it might be */
   o = (Evas_Object_Polygon *)(obj->object_data);
   return 1;
   x = 0;
   y = 0;
}
