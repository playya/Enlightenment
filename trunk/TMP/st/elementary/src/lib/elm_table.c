#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Table Table
 *
 * Arranges widgets in a table where items can also span multiple
 * columns or rows - even overlap (and then be raised or lowered
 * accordingly to adjust stacking if they do overlap).
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tbl;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
    Widget_Data *wd = elm_widget_data_get(obj);

    evas_object_event_callback_del_full
        (wd->tbl, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
   evas_object_del(wd->tbl);
}

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

   evas_object_size_hint_min_get(wd->tbl, &minw, &minh);
   evas_object_size_hint_max_get(wd->tbl, &maxw, &maxh);
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
   /* We do not add this callback, consequently we do not need to delete it
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_Object *sub = event_info;

   evas_object_event_callback_del_full
     (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
     */
   _sizing_eval(obj);
}

/**
 * Add a new table to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Table
 */
EAPI Evas_Object *
elm_table_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "table");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);

   wd->tbl = evas_object_table_add(e);
   evas_object_event_callback_add(wd->tbl, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   elm_widget_resize_object_set(obj, wd->tbl);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   return obj;
}

/**
 * Set the homogenous layout in the table
 *
 * @param obj The layout object
 * @param homogenous A boolean to set (or no) layout homogenous
 * in the table
 * (1 = homogenous,  0 = no homogenous)
 *
 * @ingroup Table
 */
EAPI void
elm_table_homogenous_set(Evas_Object *obj, Eina_Bool homogenous)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_table_homogeneous_set(wd->tbl, homogenous);
}

/**
 * Set padding between cells.
 *
 * @param obj The layout object.
 * @param horizontal set the horizontal padding. 
 * @param vertical set the vertical padding.
 *
 * @ingroup Table
 */
EAPI void
elm_table_padding_set(Evas_Object *obj, Evas_Coord horizontal, Evas_Coord vertical)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_table_padding_set(wd->tbl, horizontal, vertical);
}

/**
 * Add a subobject on the table with the coordinates passed
 *
 * @param obj The table object
 * @param subobj The subobject to be added to the table
 * @param x Coordinate to X axis
 * @param y Coordinate to Y axis
 * @param w Horizontal length
 * @param h Vertical length
 *
 * @ingroup Table
 */
EAPI void
elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   evas_object_table_pack(wd->tbl, subobj, x, y, w, h);
}
