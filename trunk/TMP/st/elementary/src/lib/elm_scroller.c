#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Scroller Scroller
 *
 * A scroller holds a single object and "scrolls it around". This means that
 * it allows the user to use a scrollbar (or a finger) to drag the viewable
 * region around, allowing to move through a much larger object that is
 * contained in the scroller. The scroiller will always have a small minimum
 * size by default as it won't be limited by the contents of the scroller.
 *
 * Signals that you can add callbacks for are:
 *
 * edge,left - the left edge of the content has been reached
 *
 * edge,right - the right edge of the content has been reached
 *
 * edge,top - the top edge of the content has been reached
 *
 * edge,bottom - the bottom edge of the content has been reached
 *
 * scroll - the content has been scrolled (moved)
 *
 * scroll,anim,start - scrolling animation has started
 *
 * scroll,anim,stop - scrolling animation has stopped
 *
 * scroll,drag,start - dragging the contents around has started
 *
 * scroll,drag,stop - dragging the contents around has stopped
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *content;
   Eina_Bool min_w : 1;
   Eina_Bool min_h : 1;
   double pagerel_h, pagerel_v;
   Evas_Coord pagesize_h, pagesize_v;
};

static const char *widtype = NULL;
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
	evas_object_event_callback_del_full (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
           _changed_size_hints, obj);
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
_edge_left(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge,left", NULL);
}

static void
_edge_right(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge,right", NULL);
}

static void
_edge_top(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge,top", NULL);
}

static void
_edge_bottom(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "edge,bottom", NULL);
}

static void
_scroll(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll", NULL);
}

static void
_scroll_anim_start(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,anim,start", NULL);
}

static void
_scroll_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,anim,stop", NULL);
}

static void
_scroll_drag_start(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,drag,start", NULL);
}

static void
_scroll_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,drag,stop", NULL);
}

/**
 * Add a new scroller to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Scroller
 */
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
   ELM_SET_WIDTYPE(widtype, "scroller");
   elm_widget_type_set(obj, "scroller");
   elm_widget_sub_object_add(parent, obj);
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
   evas_object_smart_callback_add(wd->scr, "animate,start", _scroll_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scroll_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scroll_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scroll_drag_stop, obj);

   _sizing_eval(obj);
   return obj;
}


/**
 * Set the content object
 *
 * XXX
 *
 * @param obj The scroller object
 * @param content The new content object
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
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

/**
 * Make the scroller minimum size limited to the minimum size of the content
 *
 * By default the scroller will be as small as its design allows, irrespective
 * of its content. This will make the scroller minimum size the right size
 * horizontally and/or vertically to perfectly fit its content.
 *
 * @param obj The scroller object
 * @param w Enable limiting minimum size horizontally
 * @param h Enable limiting minimum size vertically
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_content_min_limit(Evas_Object *obj, Eina_Bool w, Eina_Bool h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->min_w = w;
   wd->min_h = h;
   _sizing_eval(obj);
}

/**
 * Show a specific virtual region within the scroller content object
 *
 * This will ensure all (or part if it does not fit) of the designated
 * region in the virtual content object (0, 0 starting at the top-left of the
 * virtual content object) is shown within the scroller.
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

/**
 * Set the scroller scrollbar policy
 *
 * This sets the scrollbar visibility policy for the given scroller.
 * ELM_SMART_SCROLLER_POLICY_AUTO means the scrollber is made visible if it
 * is needed, and otherwise kept hidden. ELM_SMART_SCROLLER_POLICY_ON turns
 * it on all the time, and ELM_SMART_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 *
 * @param obj The scroller object
 * @param policy_h Horizontal scrollbar policy
 * @param policy_v Vertical scrollbar policy
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
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

/**
 * Get the currently visible content region
 *
 * This gets the current region in the content object that is visible through
 * the scroller. Also see elm_scroller_region_show(). The region co-ordinates
 * are returned in the @p x, @p y, @p w, @p h values pointed to.
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_region_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((x) && (y)) elm_smart_scroller_child_pos_get(wd->scr, x, y);
   if ((w) && (h)) elm_smart_scroller_child_viewport_size_get(wd->scr, w, h);
}

/**
 * Get the size of the content child object
 *
 * This gets the size of the child object of the scroller. Actually the
 * content of a scroller doesn't specifically need to be an actual object
 * as it can be virtual and defined purely by callbacks.
 *
 * @param obj The scroller object
 * @param w Width return
 * @param h Height return
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_geometry_get(wd->content, NULL, NULL, w, h);
}

/**
 * Set bouncing behavior
 *
 * When scrolling, the scroller may "bounce" when reaching an edge of the child
 * object. This is a visual way to indicate the end has been reached. This is
 * enabled by default for both axes. This will set if it is enabled for that
 * axis with the boolean parameers for each axis.
 *
 * @param obj The scroller object
 * @param h_bounce Will the scroller bounce horizontally or not
 y Y coordinate of the region
 w Width of the region
 h Height of the region

 EAPI void elm_scroller_region_show ( Evas_Object *  obj, * @param v_bounce Will the scroller bounce vertically or not
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}

/**
 * Set scroll page size relative to viewport size
 *
 * The scroller is sapale of limiting scrolling by the user to "pages". That
 * is to jump by and only show a "whole page" at a time as if the continuous
 * area of the scroller conent is split into page sized pieces. This sets
 * the size of a page relative to the viewport of the scroller. 1.0 is "1
 * viewport" is size (horizontally or vertically). 0.0 turns it off in that
 * axis. This is mutually exclusive with page size
 * (see elm_scroller_page_size_set()  for more information). likewise 0.5
 * is "half a viewport". Sane usable valus are normally between 0.0 and 1.0
 * including 1.0. If you only want 1 axis to be page "limited", use 0.0 for
 * the other axis.
 *
 * @param obj The scroller object
 * @param h_pagerel The horizontal page relative size
 * @param v_pagerel The vertical page relative size
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->pagerel_h = h_pagerel;
   wd->pagerel_v = v_pagerel;
   elm_smart_scroller_paging_set(wd->scr, wd->pagerel_h, wd->pagerel_v,
                                 wd->pagesize_h, wd->pagesize_v);
}

/**
 * Set scroll page size
 *
 * See also elm_scroller_page_relative_set(). This, instead of a page size
 * being relaive to the viewport, sets it to an absolute fixed value, with
 * 0 turning it off for that axis.
 *
 * @param obj The scroller object
 * @param h_pagesize The horizontal page size
 * @param v_pagesize The vertical page size
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->pagesize_h = h_pagesize;
   wd->pagesize_v = v_pagesize;
   elm_smart_scroller_paging_set(wd->scr, wd->pagerel_h, wd->pagerel_v,
                                 wd->pagesize_h, wd->pagesize_v);
}

/**
 * Show a specific virtual region within the scroller content object
 *
 * This will ensure all (or part if it does not fit) of the designated
 * region in the virtual content object (0, 0 starting at the top-left of the
 * virtual content object) is shown within the scroller. Unlike
 * elm_scroller_region_show(), this allow the scroller to "smoothly slide"
 * to this location (if configuration in general calls for transitions). It
 * may not jump immediately to the new location and make take a while and
 * show other content along the way.
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * @ingroup Scroller
 */
EAPI void
elm_scroller_region_bring_in(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_region_bring_in(wd->scr, x, y, w, h);
}
