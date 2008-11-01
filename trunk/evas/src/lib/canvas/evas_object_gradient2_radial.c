#include "evas_common.h"
#include "evas_private.h"

/* private magic number for radial gradient objects */
static const char rg_type[] = "radial_gradient";

/* private struct for gradient object internal data */
typedef struct _Evas_Object_Gradient2_Radial      Evas_Object_Gradient2_Radial;

struct _Evas_Object_Gradient2_Radial
{
   Evas_Object_Gradient2 base;

   DATA32            magic;

   struct {
      struct {
         float  cx, cy, rx, ry;
      } fill;
   } cur, prev;

   void             *engine_data;

   unsigned char     gradient_changed : 1;
   unsigned char     changed : 1;
};

/* private methods for radial gradient objects */
static void evas_object_gradient2_radial_init(Evas_Object *obj);
static void *evas_object_gradient2_radial_new(void);

static void evas_object_gradient2_radial_free(Evas_Object *obj);
static void evas_object_gradient2_radial_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_gradient2_radial_render_pre(Evas_Object *obj);
static void evas_object_gradient2_radial_render_post(Evas_Object *obj);

static unsigned int evas_object_gradient2_radial_id_get(Evas_Object *obj);
static unsigned int evas_object_gradient2_radial_visual_id_get(Evas_Object *obj);
static void *evas_object_gradient2_radial_engine_data_get(Evas_Object *obj);

static int evas_object_gradient2_radial_is_opaque(Evas_Object *obj);
static int evas_object_gradient2_radial_was_opaque(Evas_Object *obj);

static const Evas_Object_Func rg_object_func =
{
   /* methods (compulsory) */
   evas_object_gradient2_radial_free,
     evas_object_gradient2_radial_render,
     evas_object_gradient2_radial_render_pre,
     evas_object_gradient2_radial_render_post,
     evas_object_gradient2_radial_id_get,
     evas_object_gradient2_radial_visual_id_get,
     evas_object_gradient2_radial_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_gradient2_radial_is_opaque,
     evas_object_gradient2_radial_was_opaque,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a gradient */

/**
 * @addtogroup Evas_Object_Gradient2_Group
 * @{
 * @ingroup Evas_Object_Group
 */

/**
 * Adds a gradient object to the given evas.
 * @param   e The given evas.
 * @return  A new evas gradient object if successful.  Otherwise, @c NULL.
 */
EAPI Evas_Object *
evas_object_gradient2_radial_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_gradient2_radial_init(obj);
   evas_object_inject(obj, e);
   if (obj->object_data)
     {
	Evas_Object_Gradient2_Radial *o = (Evas_Object_Gradient2_Radial *)(obj->object_data);

	o->engine_data = e->engine.func->gradient2_radial_new(e->engine.data.output);
     }
   return obj;
}

/**
 * Sets the center and radii of the radial gradient geometry for the object.
 *
 * Note that the gradient may extend beyond these,
 * according to its spread value - restrict, repeat, or reflect.  To have
 * only one 'cycle' of the gradient drawn, the spread value must be set
 * to restrict.
 * The default values for the fill parameters is @p cx = 0, @p cy = 0,
 * @p rx = 0 and @p ry = 0.
 *
 * @param   obj The given evas radial gradient object.
 * @param   cx   The X coordinate of the start point.
 * @param   cy   The Y coordinate of the start point.
 * @param   rx   The length of the x-cordinate radius.
 * @param   ry   The length of the y-coordinate radius.
 */
EAPI void
evas_object_gradient2_radial_fill_set(Evas_Object *obj, float cx, float cy, float rx, float ry)
{
   Evas_Object_Gradient2_Radial *o;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient2_Radial, MAGIC_OBJ_GRADIENT_RADIAL);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.cx == cx) &&
       (o->cur.fill.cy == cy) &&
       (o->cur.fill.rx == rx) &&
       (o->cur.fill.ry == ry)) return;
   o->cur.fill.cx = cx;
   o->cur.fill.cy = cy;
   o->cur.fill.rx = rx;
   o->cur.fill.ry = ry;
   o->gradient_changed = 1;
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_gradient2_radial_fill_get(const Evas_Object *obj, float *cx, float *cy, float *rx, float *ry)
{
   Evas_Object_Gradient2_Radial *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (rx) *rx = 0;
   if (ry) *ry = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient2_Radial, MAGIC_OBJ_GRADIENT_RADIAL);
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (rx) *rx = 0;
   if (ry) *ry = 0;
   return;
   MAGIC_CHECK_END();
   if (cx) *cx = o->cur.fill.cx;
   if (cy) *cy = o->cur.fill.cy;
   if (rx) *rx = o->cur.fill.rx;
   if (ry) *ry = o->cur.fill.ry;
   return;
}

/**
 * @}
 */

/* all nice and private */
static void
evas_object_gradient2_radial_init(Evas_Object *obj)
{
   /* alloc grad ob, setup methods and default values */
   obj->object_data = evas_object_gradient2_radial_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 1;
   obj->cur.interpolation_color_space = EVAS_COLOR_SPACE_ARGB;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &rg_object_func;
   obj->type = rg_type;
   obj->changed = 1;
}

static void *
evas_object_gradient2_radial_new(void)
{
   Evas_Object_Gradient2_Radial *o;
   Evas_Object_Gradient2 *og;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Gradient2_Radial));
   if (!o) return NULL;
   o->magic = MAGIC_OBJ_GRADIENT_RADIAL;
   o->cur.fill.cx = 0;
   o->cur.fill.cy = 0;
   o->cur.fill.rx = 1;
   o->cur.fill.ry = 1;
   og = (Evas_Object_Gradient2 *)o;
   og->magic = MAGIC_OBJ_GRADIENT;
   og->cur.fill.transform.mxx = og->cur.fill.transform.myy = og->cur.fill.transform.mzz = 1;
   og->cur.fill.spread = EVAS_TEXTURE_REPEAT;
   og->cur.gradient_opaque = 0;
   og->prev = og->cur;
   og->gradient_changed = 1;
   o->prev = o->cur;
   o->changed = 1;
   o->gradient_changed = 1;
   return o;
}

static void
evas_object_gradient2_radial_free(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient2_Radial, MAGIC_OBJ_GRADIENT_RADIAL);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->engine_data)
      obj->layer->evas->engine.func->gradient2_radial_free(obj->layer->evas->engine.data.output,
							  o->engine_data);
   free(o);
   obj->object_data = NULL; 
}



static void
evas_object_gradient2_radial_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Gradient2_Radial *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   obj->layer->evas->engine.func->context_multiplier_unset(output, context);
   obj->layer->evas->engine.func->context_anti_alias_set(output, context, obj->cur.anti_alias);
   obj->layer->evas->engine.func->context_render_op_set(output, context, obj->cur.render_op);
   if (o->engine_data)
     {
	obj->layer->evas->engine.func->gradient2_radial_draw(output, context, surface,
						     o->engine_data,
						     obj->cur.geometry.x + x,
						     obj->cur.geometry.y + y,
						     obj->cur.geometry.w,
						     obj->cur.geometry.h);
     }
}

static void
evas_object_gradient2_radial_render_pre(Evas_Object *obj)
{
   Evas_Rectangles rects = { 0, 0, NULL };
   Evas_Object_Gradient2_Radial *o;
   Evas_Object_Gradient2 *og;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   og = (Evas_Object_Gradient2 *)(o);
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     { o->gradient_changed = 1;  o->changed = 1; }
   if ((!o->gradient_changed) && ((obj->cur.cache.clip.r != obj->prev.cache.clip.r) ||
       (obj->cur.cache.clip.g != obj->prev.cache.clip.g) ||
       (obj->cur.cache.clip.b != obj->prev.cache.clip.b) ||
       (obj->cur.cache.clip.a != obj->prev.cache.clip.a)))
     { o->gradient_changed = 1;  o->changed = 1; }
   if (!o->gradient_changed && (obj->cur.interpolation_color_space != obj->prev.interpolation_color_space))
     { o->gradient_changed = 1;  o->changed = 1; }
   if (!o->changed && (obj->cur.render_op != obj->prev.render_op))
	o->changed = 1;
//   if (!o->changed && (obj->cur.anti_alias != obj->prev.anti_alias))
//	o->changed = 1;
   if (og->gradient_changed)
     { o->gradient_changed = 1;  o->changed = 1; }
   if (o->changed && o->engine_data)
     {
	obj->layer->evas->engine.func->context_render_op_set(obj->layer->evas->engine.data.output,
							     obj->layer->evas->engine.data.context, obj->cur.render_op);
	obj->layer->evas->engine.func->context_multiplier_set(obj->layer->evas->engine.data.output,
								obj->layer->evas->engine.data.context,
								obj->cur.cache.clip.r, obj->cur.cache.clip.g,
								obj->cur.cache.clip.b, obj->cur.cache.clip.a);
	obj->layer->evas->engine.func->context_color_interpolation_set(obj->layer->evas->engine.data.output,
									obj->layer->evas->engine.data.context,
									obj->cur.interpolation_color_space);
	if (o->gradient_changed)
	  {
	    obj->layer->evas->engine.func->gradient2_radial_fill_set(obj->layer->evas->engine.data.output, o->engine_data, o->cur.fill.cx, o->cur.fill.cy, o->cur.fill.rx, o->cur.fill.ry);
	    obj->layer->evas->engine.func->gradient2_fill_transform_set(obj->layer->evas->engine.data.output, o->engine_data,
								&og->cur.fill.transform);
	    obj->layer->evas->engine.func->gradient2_fill_spread_set(obj->layer->evas->engine.data.output, o->engine_data,
								og->cur.fill.spread);
	    obj->layer->evas->engine.func->gradient2_radial_render_pre(obj->layer->evas->engine.data.output,
								obj->layer->evas->engine.data.context,
								o->engine_data);
	  }
	og->cur.gradient_opaque = obj->layer->evas->engine.func->gradient2_radial_is_opaque(obj->layer->evas->engine.data.output,
										   obj->layer->evas->engine.data.context,
				  						   o->engine_data,
										   obj->cur.cache.clip.x, obj->cur.cache.clip.y,
										   obj->cur.cache.clip.w, obj->cur.cache.clip.h);

	if (obj->cur.cache.clip.a != 255)
	    og->cur.gradient_opaque = 0;
    }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&rects, obj, is_v, was_v);
	goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&rects, obj);
   /* gradient changed */
   if (o->changed || obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* if it changed geometry */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	evas_object_render_pre_prev_cur_add(&rects, obj);
	goto done;
     }
   /* it obviously didn't change - add a NO obscure - this "unupdates"  this */
   /* area so if there were updates for it they get wiped. don't do it if we */
   /* arent fully opaque and we are visible */
   
   if (evas_object_is_visible(obj) &&
       evas_object_is_opaque(obj))
     obj->layer->evas->engine.func->output_redraws_rect_del(obj->layer->evas->engine.data.output,
							    obj->cur.cache.clip.x,
							    obj->cur.cache.clip.y,
							    obj->cur.cache.clip.w,
							    obj->cur.cache.clip.h);
   
   done:
   evas_object_render_pre_effect_updates(&rects, obj, is_v, was_v);
}

static void
evas_object_gradient2_radial_render_post(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;
   Evas_Object_Gradient2 *og;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   og = (Evas_Object_Gradient2 *)(o);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = eina_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   obj->changed = 0;
   o->prev = o->cur;
   o->changed = 0;
   o->gradient_changed = 0;
   og->prev = og->cur;
   og->gradient_changed = 0;
}

static unsigned int evas_object_gradient2_radial_id_get(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;

   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_GRADIENT_RADIAL;
}

static unsigned int evas_object_gradient2_radial_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;

   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_GRADIENT;
}

static void *evas_object_gradient2_radial_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;

   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}


static int
evas_object_gradient2_radial_is_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;
   Evas_Object_Gradient2 *og;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire region it occupies */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   if (!o->engine_data) return 0;
   og = (Evas_Object_Gradient2 *)(o);
   return og->cur.gradient_opaque;
 }

static int
evas_object_gradient2_radial_was_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient2_Radial *o;
   Evas_Object_Gradient2 *og;

   /* this returns 1 if the internal object data implies that the object was */
   /* currently fully opaque over the entire region it occupies */
   o = (Evas_Object_Gradient2_Radial *)(obj->object_data);
   if (!o->engine_data) return 0;
   og = (Evas_Object_Gradient2 *)(o);
   return og->prev.gradient_opaque;
}
