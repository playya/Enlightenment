#include "evas_common.h"
#include "evas_private.h"

/* private magic number for gradient objects */
static const char o_type[] = "gradient";

/* private struct for gradient object internal data */
typedef struct _Evas_Object_Gradient      Evas_Object_Gradient;

struct _Evas_Object_Gradient
{
   DATA32            magic;

   struct {
      Evas_Angle     angle;
      int            spread;
      float          range_offset;
      struct {
         Evas_Coord  x, y, w, h;
      } fill;
      struct {
         char       *name;
         char       *params;
      } type;
   unsigned char    gradient_opaque : 1;
   } cur, prev;

   unsigned char     changed : 1;
   unsigned char     gradient_changed : 1;
   unsigned char     imported_data : 1;

   void             *engine_data;
};

/* private methods for gradient objects */
static void evas_object_gradient_init(Evas_Object *obj);
static void *evas_object_gradient_new(void);
static void evas_object_gradient_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_gradient_free(Evas_Object *obj);
static void evas_object_gradient_render_pre(Evas_Object *obj);
static void evas_object_gradient_render_post(Evas_Object *obj);

static int evas_object_gradient_is_opaque(Evas_Object *obj);
static int evas_object_gradient_was_opaque(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_gradient_free,
     evas_object_gradient_render,
     evas_object_gradient_render_pre,
     evas_object_gradient_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_gradient_is_opaque,
     evas_object_gradient_was_opaque,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a gradient */

/**
 * @defgroup Evas_Object_Gradient_Group Evas Gradient Object Functions
 *
 * Functions that work on evas gradient objects.
 *
 * The following example shows how
 */

/**
 * Adds a gradient object to the given evas.
 * @param   e The given evas.
 * @return  A new evas gradient object if successful.  Otherwise, @c NULL.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI Evas_Object *
evas_object_gradient_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_gradient_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

/**
 * Adds a colour to the given evas gradient object.
 *
 * The @p distance parameter determines the proportion of the gradient
 * object that is to be set to this colour.  For instance, if red is
 * added with @p distance set to 2, and green is added with @p
 * distance set to 1, two-thirds will be red or reddish and one-third
 * will be green or greenish.
 *
 * Colours are added from the top downwards.
 *
 * @param   obj      The given evas gradient object.
 * @param   r        Red component of the given colour.
 * @param   g        Green component of the given colour.
 * @param   b        Blue component of the given colour.
 * @param   a        Alpha component of the given colour.
 * @param   distance Proportion of the gradient object that is this colour.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_color_add(Evas_Object *obj, int r, int g, int b, int a, int distance)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (o->imported_data) return;
   o->engine_data = obj->layer->evas->engine.func->gradient_color_add(obj->layer->evas->engine.data.output,
								      obj->layer->evas->engine.data.context,
								      o->engine_data,
								      r, g, b, a,
								      distance);
   if (a != 255) o->cur.gradient_opaque = 0;
   o->gradient_changed = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Deletes all colours set for the given evas gradient object.
 * @param   obj The given evas gradient object.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_colors_clear(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (o->imported_data) return;
   o->engine_data = obj->layer->evas->engine.func->gradient_colors_clear(obj->layer->evas->engine.data.output,
									 obj->layer->evas->engine.data.context,
									 o->engine_data);
   o->gradient_changed = 1;
   o->changed = 1;
   o->cur.gradient_opaque = 1;
   evas_object_change(obj);
}

/**
 * Sets color data for the given evas gradient object.
 *
 * If data is so set, any existing gradient colors will be cleared,
 * and any calls to add gradient colors are then ignored until the data is unset.
 * To unset any set data, use the evas_object_gradient_data_unset call.
 * The data is not copied, so if it was allocated, do not free it while it's set.
 *
 * @param   obj       The given evas gradient object.
 * @param   data      The color data to be set. Should be in argb32 pixel format.
 * @param   len       The length of the data pointer - multiple of the pixel size.
 * @param   has_alpha A flag indicating if the data has alpha or not.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_data_set(Evas_Object *obj, void *data, int len, int has_alpha)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if ((o->engine_data) && (!o->imported_data))
	o->engine_data = obj->layer->evas->engine.func->gradient_colors_clear(obj->layer->evas->engine.data.output,
									      obj->layer->evas->engine.data.context,
									      o->engine_data);
   else if ((o->engine_data) && (o->imported_data))
	o->engine_data = obj->layer->evas->engine.func->gradient_data_unset(obj->layer->evas->engine.data.output,
									    obj->layer->evas->engine.data.context,
									    o->engine_data);
   o->engine_data = obj->layer->evas->engine.func->gradient_data_set(obj->layer->evas->engine.data.output,
								     obj->layer->evas->engine.data.context,
								     o->engine_data,
								     data, len, has_alpha);
   o->imported_data = 1;
   if (has_alpha) o->cur.gradient_opaque = 0;
   o->gradient_changed = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Unsets color data for the given evas gradient object.
 *
 * If no data is set, this does nothing. If data has been set, then this
 * unsets any such color data. One can then add colors or set other data.
 * The data may now be freed if need be.
 *
 * @param   obj       The given evas gradient object.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_data_unset(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (!o->imported_data) return;
   if (o->engine_data)
	o->engine_data = obj->layer->evas->engine.func->gradient_data_unset(obj->layer->evas->engine.data.output,
									    obj->layer->evas->engine.data.context,
									    o->engine_data);
   o->imported_data = 0;
   o->cur.gradient_opaque = 1;
   o->gradient_changed = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Sets the angle at which the given evas gradient object sits clockwise
 * from vertical.
 * @param   obj   The given evas gradient object.
 * @param   angle Angle in degrees.  Can be negative.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_angle_set(Evas_Object *obj, Evas_Angle angle)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (angle == o->cur.angle) return;
   o->cur.angle = angle;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the angle at which the given evas gradient object sits
 * clockwise from the vertical.
 * @param   obj The given evas gradient object.
 * @return  The current angle if successful. @c 0.0 otherwise.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI Evas_Angle
evas_object_gradient_angle_get(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return 0.0;
   MAGIC_CHECK_END();
   return o->cur.angle;
}

/**
 * @defgroup Evas_Object_Gradient_Fill_Group Gradient Object Fill Rectangle Functions
 *
 * Functions that deal with what areas of the gradient object are to be
 * tiled with the gradient.
 */

/**
 * Sets the rectangle on the gradient object that the gradient will be
 * drawn to.
 *
 * Note that the gradient may be tiled around this one rectangle,
 * according to its spread value - restrict, repeat, or reflect.  To have
 * only one 'cycle' of the gradient drawn, the spread value must be set
 * to restrict, or @p x and @p y must be 0 and @p w and @p h need to be
 * the width and height of the gradient object respectively.
 *
 * The default values for the fill parameters is @p x = 0, @p y = 0,
 * @p w = 32 and @p h = 32.
 *
 * @param   obj The given evas gradient object.
 * @param   x   The X coordinate for the top left corner of the rect.
 * @param   y   The Y coordinate for the top left corner of the rect.
 * @param   w   The width of the rect.
 * @param   h   The height of the rect.
 * @ingroup Evas_Object_Gradient_Fill_Group
 */
EAPI void
evas_object_gradient_fill_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Gradient *o;

   if (w < 0) w = -w;
   if (h < 0) h = -h;
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if ((o->cur.fill.x == x) &&
       (o->cur.fill.y == y) &&
       (o->cur.fill.w == w) &&
       (o->cur.fill.h == h)) return;
   o->cur.fill.x = x;
   o->cur.fill.y = y;
   o->cur.fill.w = w;
   o->cur.fill.h = h;
   o->gradient_changed = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the dimensions of the rectangle on the gradient object that
 * the gradient will use as its fill rect.
 *
 * See @ref evas_object_gradient_fill_set for more details.
 *
 * @param   obj The given evas gradient object.
 * @param   x   Pointer to an Evas_Coord to store the X coordinate in.
 * @param   y   Pointer to an Evas_Coord to store the Y coordinate in.
 * @param   w   Pointer to an Evas_Coord to store the width in.
 * @param   h   Pointer to an Evas_Coord to store the height in.
 * @ingroup Evas_Object_Gradient_Fill_Group
 */
EAPI void
evas_object_gradient_fill_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = o->cur.fill.x;
   if (y) *y = o->cur.fill.y;
   if (w) *w = o->cur.fill.w;
   if (h) *h = o->cur.fill.h;
   return;
}

/**
 * Sets the offset which the given evas gradient object uses in its color data range.
 * @param   obj   The given evas gradient object.
 * @param   offset Values can be negative.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_range_offset_set(Evas_Object *obj, float offset)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (offset == o->cur.range_offset) return;
   o->cur.range_offset = offset;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the range offset
 * @param   obj The given evas gradient object.
 * @return  The current gradient range offset if successful. @c 0.0 otherwise.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI float
evas_object_gradient_range_offset_get(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0.0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return 0.0;
   MAGIC_CHECK_END();
   return o->cur.range_offset;
}

/**
 * Sets the tiling mode for the given evas gradient object.
 * @param   obj   The given evas gradient object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, EVAS_TEXTURE_RESTRICT_REFLECT, EVAS_TEXTURE_RESTRICT_REPEAT,
 * or EVAS_TEXTURE_PAD.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI void
evas_object_gradient_spread_set(Evas_Object *obj, int spread)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (spread == o->cur.spread) return;
   o->cur.spread = spread;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the spread (tiling mode) for the given gradient object.
 * @param   obj The given evas gradient object.
 * @return  The current spread mode of the gradient object.
 * @ingroup Evas_Object_Gradient_Group
 */
EAPI int
evas_object_gradient_spread_get(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTURE_REFLECT;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return EVAS_TEXTURE_REFLECT;
   MAGIC_CHECK_END();
   return o->cur.spread;
}

/**
 * @defgroup Evas_Object_Gradient_Type_Group Gradient Object Type Functions
 *
 * Functions that set or get a gradient's geometric type. Examples are "linear",
 * "radial", "rectangular", "angular", "sinusoidal", ...
 * Some types may accept additional parameters to further specify the look.
 */

/**
 * Sets the geometric type displayed by the given gradient object.
 * @param   obj  The given gradient object.
 * @param   name Name of the geometric type that the gradient is to be drawn as.
 * @param   params List of allowable param_name, param_value pairs. Must be in the
 * format "param_name=param_value; param_name2=param_value2;", eg. "amplitude=0.8;".
 * Can be NULL.
 * @ingroup Evas_Object_Gradient_Type_Group
 */
EAPI void
evas_object_gradient_type_set(Evas_Object *obj, const char *name, const char *params)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (!name || !*name)
     {
	name = "linear";
	params = NULL;
     }
   if (params && !*params)
	params = NULL;
   if ((o->cur.type.name) && (!strcmp(o->cur.type.name, name)))
     {
	if ((!o->cur.type.params) && (!params))
	  return;
	if ((o->cur.type.params) && (params) && (!strcmp(o->cur.type.params, params)))
	  return;
	if (o->cur.type.params)
	  {
	    if (o->prev.type.params == o->cur.type.params)
		o->prev.type.params = strdup(o->cur.type.params);
	    free(o->cur.type.params);
	    o->cur.type.params = NULL;
	  }
	if (params)
	  o->cur.type.params = strdup(params);
	o->changed = 1;
	o->gradient_changed = 1;
	evas_object_change(obj);
	return;
     }

   if (o->cur.type.name)
     {
	if (o->prev.type.name == o->cur.type.name)
	  o->prev.type.name = strdup(o->cur.type.name);
	free(o->cur.type.name);
	o->cur.type.name = NULL;
     }
   o->cur.type.name = strdup(name);

   if (o->cur.type.params)
     {
	if (o->prev.type.params == o->cur.type.params)
	  o->prev.type.params = strdup(o->cur.type.params);
	free(o->cur.type.params);
	o->cur.type.params = NULL;
     }
   if (params)
	o->cur.type.params = strdup(params);
   o->changed = 1;
   o->gradient_changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the type name and params of the given gradient object.
 * @param   obj  The given gradient object.
 * @param   name Pointer to a character pointer to store the pointer to the type
 *               name in.
 * @param   params  Pointer to a character pointer to store the pointer to the type
 *               params string in.
 * @ingroup Evas_Object_Gradient_Type_Group
 */
EAPI void
evas_object_gradient_type_get(Evas_Object *obj, char **name, char **params)
{
   Evas_Object_Gradient *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (name) *name = NULL;
   if (params) *params = NULL;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   if (name) *name = NULL;
   if (params) *params = NULL;
   return;
   MAGIC_CHECK_END();
   if (name) *name = o->cur.type.name;
   if (params) *params = o->cur.type.params;
}


/*
  these two functions don't really belong here as they can apply to other
  objs as well, but for now..
*/

/**
   FIXME: ...
**/
EAPI void
evas_color_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   evas_common_convert_hsv_to_rgb(h, s, v, r, g, b);
}

/**
   FIXME: ...
**/
EAPI void
evas_color_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   evas_common_convert_rgb_to_hsv(r, g, b, h, s, v);
}



/* all nice and private */
static void
evas_object_gradient_init(Evas_Object *obj)
{
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_gradient_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 32.0;
   obj->cur.geometry.h = 32.0;
   obj->cur.layer = 0;
   obj->cur.anti_alias = 1;
   obj->cur.interpolation.color_space = EVAS_COLOR_SPACE_ARGB;
   obj->cur.render_op = EVAS_RENDER_BLEND;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
   obj->changed = 1;
}

static void *
evas_object_gradient_new(void)
{
   Evas_Object_Gradient *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Gradient));
   o->magic = MAGIC_OBJ_GRADIENT;
   o->cur.angle = 0.0;
   o->cur.spread = EVAS_TEXTURE_REFLECT;
   o->cur.range_offset = 0.0;
   o->cur.fill.x = 0;
   o->cur.fill.y = 0;
   o->cur.fill.w = 32;
   o->cur.fill.h = 32;
   o->cur.type.name = strdup("linear");
   o->cur.type.params = NULL;
   o->cur.gradient_opaque = 0;
   o->prev = o->cur;
   o->changed = 1;
   o->gradient_changed = 1;
   return o;
}

static void
evas_object_gradient_free(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Gradient *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Gradient, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   if (o->prev.type.name && (o->prev.type.name != o->cur.type.name))
	free(o->prev.type.name);
   if (o->prev.type.params && (o->prev.type.params != o->cur.type.params))
	free(o->prev.type.params);
   if (o->cur.type.name)
	free(o->cur.type.name);
   if (o->cur.type.params)
	free(o->cur.type.params);
   obj->layer->evas->engine.func->gradient_free(obj->layer->evas->engine.data.output,
						o->engine_data);
   o->magic = 0;
   free(o);
}


static void
evas_object_grad_get_fill_coords(Evas_Object *obj, Evas_Coord fx, Evas_Coord fy, Evas_Coord fw, Evas_Coord fh, Evas_Coord *fx_ret, Evas_Coord *fy_ret, Evas_Coord *fw_ret,
Evas_Coord *fh_ret)
{
   Evas_Coord  x, y, w, h;

   x = ((fx * obj->layer->evas->output.w) /
	    (Evas_Coord)obj->layer->evas->viewport.w);
   w = ((fw * obj->layer->evas->output.w) /
	(Evas_Coord)obj->layer->evas->viewport.w);
   y = ((fy * obj->layer->evas->output.h) /
	    (Evas_Coord)obj->layer->evas->viewport.h);
   h = ((fh * obj->layer->evas->output.h) /
	(Evas_Coord)obj->layer->evas->viewport.h);

   *fx_ret = x;  *fw_ret = w;
   *fy_ret = y;  *fh_ret = h;
}



static void
evas_object_gradient_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Gradient *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Gradient *)(obj->object_data);
   obj->layer->evas->engine.func->context_color_set(output,
						    context,
						    255, 255, 255, 255);
   if ((obj->cur.cache.clip.r == 255) &&
       (obj->cur.cache.clip.g == 255) &&
       (obj->cur.cache.clip.b == 255) &&
       (obj->cur.cache.clip.a == 255))
     obj->layer->evas->engine.func->context_multiplier_unset(output, context);
   else
     obj->layer->evas->engine.func->context_multiplier_set(output, context,
							   obj->cur.cache.clip.r,
							   obj->cur.cache.clip.g,
							   obj->cur.cache.clip.b,
							   obj->cur.cache.clip.a);
   obj->layer->evas->engine.func->context_anti_alias_set(output, context,
							 obj->cur.anti_alias);
   obj->layer->evas->engine.func->context_color_interpolation_set(output, context,
						   obj->cur.interpolation.color_space);
   obj->layer->evas->engine.func->context_render_op_set(output, context,
							obj->cur.render_op);
   if (o->engine_data)
     {
	if (o->gradient_changed)
	    obj->layer->evas->engine.func->gradient_map(output, context, o->engine_data,
	    						o->cur.spread);
	obj->layer->evas->engine.func->gradient_draw(output, context, surface,
						     o->engine_data,
						     obj->cur.cache.geometry.x + x,
						     obj->cur.cache.geometry.y + y,
						     obj->cur.cache.geometry.w,
						     obj->cur.cache.geometry.h,
						     o->cur.angle, o->cur.spread);
     }
}

static void
evas_object_gradient_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Gradient *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */
   o = (Evas_Object_Gradient *)(obj->object_data);
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
	o->gradient_changed = 1;
   if ((obj->cur.cache.clip.r != obj->prev.cache.clip.r) ||
       (obj->cur.cache.clip.g != obj->prev.cache.clip.g) ||
       (obj->cur.cache.clip.b != obj->prev.cache.clip.b) ||
       (obj->cur.cache.clip.a != obj->prev.cache.clip.a))
	o->gradient_changed = 1;
   if (obj->cur.anti_alias != obj->prev.anti_alias)
	o->changed = 1;
   if (obj->cur.interpolation.color_space != obj->prev.interpolation.color_space)
	o->gradient_changed = 1;
   if ((obj->cur.render_op != EVAS_RENDER_COPY) &&
	 (obj->cur.cache.clip.a != 255))
	o->cur.gradient_opaque = 0;
   if (obj->cur.render_op != obj->prev.render_op)
	o->changed = 1;
   if (o->changed && o->engine_data)
     {
	Evas_Coord  fx, fy, fw, fh;

	evas_object_grad_get_fill_coords(obj, o->cur.fill.x, o->cur.fill.y,
					 o->cur.fill.w, o->cur.fill.h,
					 &fx, &fy, &fw, &fh);
	obj->layer->evas->engine.func->gradient_fill_set(obj->layer->evas->engine.data.output, o->engine_data,
							 fx, fy, fw, fh);
	obj->layer->evas->engine.func->gradient_range_offset_set(obj->layer->evas->engine.data.output, o->engine_data,
							 	 o->cur.range_offset);
	obj->layer->evas->engine.func->gradient_type_set(obj->layer->evas->engine.data.output, o->engine_data,
							 o->cur.type.name);
	obj->layer->evas->engine.func->gradient_type_params_set(obj->layer->evas->engine.data.output, o->engine_data,
							        o->cur.type.params);

 	o->engine_data = obj->layer->evas->engine.func->gradient_geometry_init(obj->layer->evas->engine.data.output, o->engine_data, o->cur.spread);
	if (o->engine_data)
	  {
	    o->cur.gradient_opaque &= !(obj->layer->evas->engine.func->gradient_alpha_get(obj->layer->evas->engine.data.output, o->engine_data, o->cur.spread, obj->cur.render_op));
	  }
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
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* gradient changed */
   if (o->gradient_changed)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed geometry */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* o->changed */
   if (o->changed)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
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
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_gradient_render_post(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Gradient *)(obj->object_data);
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
   obj->changed = 0;
   if (o->prev.type.name && (o->prev.type.name != o->cur.type.name))
	free(o->prev.type.name);
   if (o->prev.type.params && (o->prev.type.params != o->cur.type.params))
	free(o->prev.type.params);
   o->prev = o->cur;
   o->changed = 0;
   o->gradient_changed = 0;
}

static int
evas_object_gradient_is_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fully opaque over the entire gradient it occupies */
   o = (Evas_Object_Gradient *)(obj->object_data);
   if (!o->engine_data) return 0;
   o->cur.gradient_opaque &=
   !(obj->layer->evas->engine.func->gradient_alpha_get(obj->layer->evas->engine.data.output,
   o->engine_data, o->cur.spread, obj->cur.render_op));
   return o->cur.gradient_opaque;
 }

static int
evas_object_gradient_was_opaque(Evas_Object *obj)
{
   Evas_Object_Gradient *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* currently fully opaque over the entire gradient it occupies */
   o = (Evas_Object_Gradient *)(obj->object_data);
   if (!o->engine_data) return 0;
   return o->prev.gradient_opaque;
}
