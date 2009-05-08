#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *content;
   Evas_Bool min_w : 1;
   Evas_Bool min_h : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_theme_set(wd->scr, "scroller", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord  vw, vh, minw, minh, maxw, maxh, w, h, vmw, vmh;
   double xw, xy;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->content, &minw, &minh);
   evas_object_size_hint_max_get(wd->content, &maxw, &maxh);
   evas_object_size_hint_weight_get(wd->content, &xw, &xy);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (xw > 0.0)
     {
	if ((minw > 0) && (vw < minw)) vw = minw;
	else if ((maxw > 0) && (vw > maxw)) vw = maxw;
     }
   else if (minw > 0) vw = minw;
   if (xy > 0.0)
     {
	if ((minh > 0) && (vh < minh)) vh = minh;
	else if ((maxh > 0) && (vh > maxh)) vh = maxh;
     }
   else if (minh > 0) vh = minh;
   evas_object_resize(wd->content, vw, vh);
   w = -1;
   h = -1;
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &vmw, &vmh);
   if (wd->min_w) w = vmw + minw;
   if (wd->min_h) h = vmh + minh;
   evas_object_size_hint_min_set(obj, w, h);
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
   if (!wd) return;
   if (sub == wd->content)
     {
	elm_widget_on_show_region_hook_set(wd->content, NULL, NULL);
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->content = NULL;
	_sizing_eval(obj);
     }
}

static void
_hold_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_edge_left(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge_left", NULL);
}

static void
_edge_right(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge_right", NULL);
}

static void
_edge_top(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge_top", NULL);
}

static void
_edge_bottom(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge_bottom", NULL);
}

static void
_scroll(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll", NULL);
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord vw, vh, minw, minh;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   
   wd->scr = elm_smart_scroller_add(e);
   elm_widget_resize_object_set(obj, wd->scr);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
                                  _changed_size_hints, obj);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);
   
   evas_object_smart_callback_add(wd->scr, "edge,left", _edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _edge_right, obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _edge_bottom, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scroll, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_scroller_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content)
     {
	elm_widget_on_show_region_hook_set(content, _show_region_hook, obj);
	elm_widget_sub_object_add(obj, content);
	elm_smart_scroller_child_set(wd->scr, content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_scroller_content_min_limit(Evas_Object *obj, Evas_Bool w, Evas_Bool h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->min_w = w;
   wd->min_h = h;
   _sizing_eval(obj);
}

EAPI void
elm_scroller_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

EAPI void
elm_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Elm_Scroller_Policy map[3] = 
     {
        ELM_SMART_SCROLLER_POLICY_AUTO,
          ELM_SMART_SCROLLER_POLICY_ON,
          ELM_SMART_SCROLLER_POLICY_OFF
     };
   if (!wd) return;
   if ((policy_h < 0) || (policy_h >= 3) || (policy_v < 0) || (policy_v >= 3))
     return;
   elm_smart_scroller_policy_set(wd->scr, map[policy_h], map[policy_v]);
}

EAPI void
elm_scroller_region_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((x) && (y)) elm_smart_scroller_child_pos_get(wd->scr, x, y);
   if ((w) && (h)) elm_smart_scroller_child_viewport_size_get(wd->scr, w, h);
}

EAPI void
elm_scroller_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_geometry_get(wd->content, NULL, NULL, w, h);
}

EAPI void
elm_scroller_bounce_set(Evas_Object *obj, Evas_Bool h_bounce, Evas_Bool v_bounce)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}
