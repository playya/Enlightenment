#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Box Box
 *
 * A box object arranges objects in a single row within a box. Sub objects can
 * be added at the start, end or before or after any existing object in the
 * box already. It can have its orientation changed too. How a child object is
 * sized and otherwise arranged within the box depends on evas hints.
 * evas_object_size_hint_align_set() will set either the alignment within its
 * region if the region allocated is bigger than the object size. If you want
 * the sub object sized up to fill the allocated region, use -1.0 for the
 * apporpriate horizontal or vertical axes. evas_object_size_hint_weight_set()
 * will set the packing weight. The weights of all items being packed are added
 * up and if items are to be sized up to fit, those with the higher weights get
 * proportionally more space.
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *box;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   evas_object_size_hint_min_get(wd->box, &minw, &minh);
   evas_object_size_hint_max_get(wd->box, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   evas_object_event_callback_del
     (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   _sizing_eval(obj);
}


/**
 * Add a new box to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Box
 */
EAPI Evas_Object *
elm_box_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "box");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);

   wd->box = _els_smart_box_add(e);
   evas_object_event_callback_add(wd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   elm_widget_resize_object_set(obj, wd->box);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   return obj;
}

/**
 * Set the horizontal orientation
 *
 * By default box object arrange their contents vertically from top to bottom.
 * By calling this and providing @p orizontal as true, the box will become
 * horizontal arranging contents left to right.
 *
 * @param obj The box object
 * @param horizontal The horizontal flag (1 = horizontal, 0 = vertical)
 *
 * @ingroup Box
 */
EAPI void
elm_box_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_box_orientation_set(wd->box, horizontal);
}

/**
 * Set homogenous layout
 *
 * If enabled, homogenous layout makes all items the same size. This size is
 * of course governed by the size of the largest item in the box.
 *
 * @param obj The box object
 * @param homogenous The homogenous flag (1 = on, 2 = off)
 *
 * @ingroup Box
 */
EAPI void
elm_box_homogenous_set(Evas_Object *obj, Eina_Bool homogenous)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_box_homogenous_set(wd->box, homogenous);
}

/**
 * This adds a box at the start of the box (top or left based on orientation)
 *
 * This will add the @p subobj to the box object indicated at the beginning
 * of the box (the left or top end).
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 *
 * @ingroup Box
 */
EAPI void
elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_start(wd->box, subobj);
}

/**
 * This adds a box at the end of the box (bottom or right based on orientation)
 *
 * This will add the @p subobj to the box object indicated at the end
 * of the box (the right or bottom end).
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 *
 * @ingroup Box
 */
EAPI void
elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_end(wd->box, subobj);
}

/**
 * This adds adds an object to the box before the indicated object
 *
 * This will add the @p subobj to the box indicated before the object
 * indicated with @p before. If @p before is not already in the box, results
 * are undefined. Before means either to the left of the indicated object or
 * above it depending on orientation.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 * @param before The object before which to add it
 *
 * @ingroup Box
 */
EAPI void
elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_before(wd->box, subobj, before);
}

/**
 * This adds adds an object to the box after the indicated object
 *
 * This will add the @p subobj to the box indicated after the object
 * indicated with @p after. If @p after is not already in the box, results
 * are undefined. After means either to the right of the indicated object or
 * below it depending on orientation.
 *
 * @param obj The box object
 * @param subobj The object to add to the box
 * @param after The object after which to add it
 *
 * @ingroup Box
 */
EAPI void
elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_after(wd->box, subobj, after);
}

/**
 * This clears the box items
 *
 * This delete all members of the box object, but not the box itself.
 *
 * @param obj The box object
 *
 * @ingroup Box
 */
EAPI void
elm_box_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_box_clear(wd->box);
}

/**
 * This unpacks an item from the box
 *
 * This unpacks a single Evas_Object from the box.
 *
 * @param obj The box item
 *
 * @ingroup Box
 */
EAPI void
elm_box_unpack(Evas_Object *obj)
{
   evas_object_hide(obj);
   _els_smart_box_unpack(obj);
}
