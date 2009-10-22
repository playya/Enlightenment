#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Frame Frame
 *
 * This holds some content and has a title. Looks like a frame, but
 * supports styles so multple frames are avaible
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *content;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
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
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   _elm_theme_set(wd->frm, "frame", "base", elm_widget_style_get(obj));
   if (wd->content)
     edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   edje_object_size_min_calc(wd->frm, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   _sizing_eval(data);
}

static void
_content_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minw, minh;
   edje_object_size_min_calc(wd->content, &minw, &minh);
   evas_object_size_hint_min_set(wd->content, minw, minh);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;

   if (sub == wd->content)
     {
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   evas_object_event_callback_del_full(sub, EVAS_CALLBACK_RESIZE,
                                       _content_resize, obj);
	wd->content = NULL;
	_sizing_eval(obj);
     }
}

/**
 * Add a new frame to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Frame
 */
EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "frame");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->frm = edje_object_add(e);
   _elm_theme_set(wd->frm, "frame", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the frame label
 *
 * @param obj The frame object
 * @param label The label of this frame object
 *
 * @ingroup Frame
 */
EAPI void
elm_frame_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   edje_object_part_text_set(wd->frm, "elm.text", label);
   _sizing_eval(obj);
}

/**
 * Get the frame label
 *
 * @param obj The frame object
 *
 * @return The label of this frame objet or NULL if unable to get frame
 *
 * @ingroup Frame
 */
EAPI const char*
elm_frame_label_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->frm)) return NULL;
   return edje_object_part_text_get(wd->frm, "elm.text");
}

/**
 * Set the frame content
 *
 * @param obj The frame object
 * @param content The content will be filled in this frame object
 *
 * @ingroup Frame
 */
EAPI void
elm_frame_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->frm, "elm.swallow.content", content);
	evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
   evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_RESIZE,
				       _content_resize, obj);
	_sizing_eval(obj);
     }
}

/**
 * Set the frame style
 *
 * @param obj The frame object
 * @param style The style will be applied in this frame
 *
 * DEPRECATED. use elm_object_style_set() instead
 *
 * @ingroup Frame
 */
EAPI void
elm_frame_style_set(Evas_Object *obj, const char *style)
{
   elm_widget_style_set(obj, style);
}
