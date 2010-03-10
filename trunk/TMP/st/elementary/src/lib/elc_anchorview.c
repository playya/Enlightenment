#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Anchorview Anchorview
 *
 * This is just like the Anchorblock object, but provides a scroller to hold
 * the text automatically.
 *
 * Signals that you can add callbacks for are:
 *
 * anchor,clicked - achor called was clicked. event_info is anchor info -
 * Elm_Entry_Anchorview_Info
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller, *entry;
   Evas_Object *hover_parent;
   Evas_Object *pop, *hover;
   const char *hover_style;
};

static const char *widtype = NULL;
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_anchorview_hover_end(obj);
   elm_anchorview_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_anchorview_hover_end(data);
}

static void
_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info *info = event_info;
   Evas_Object *hover_parent;
   Elm_Entry_Anchorview_Info ei;
   Evas_Coord x, w, y, h, px, py;
   if (!wd) return;
   wd->pop = elm_icon_add(obj);
   evas_object_move(wd->pop, info->x, info->y);
   evas_object_resize(wd->pop, info->w, info->h);
   wd->hover = elm_hover_add(obj);
   if (wd->hover_style) elm_object_style_set(wd->hover, wd->hover_style);
   hover_parent = wd->hover_parent;
   if (!hover_parent) hover_parent = obj;
   elm_hover_parent_set(wd->hover, hover_parent);
   elm_hover_target_set(wd->hover, wd->pop);
   ei.name = info->name;
   ei.button = info->button;
   ei.hover = wd->hover;
   ei.anchor.x = info->x;
   ei.anchor.y = info->y;
   ei.anchor.w = info->w;
   ei.anchor.h = info->h;
   evas_object_geometry_get(hover_parent, &x, &y, &w, &h);
   ei.hover_parent.x = x;
   ei.hover_parent.y = y;
   ei.hover_parent.w = w;
   ei.hover_parent.h = h;
   px = info->x + (info->w / 2);
   py = info->y + (info->h / 2);
   ei.hover_left = 1;
   if (px < (x + (w / 3))) ei.hover_left = 0;
   ei.hover_right = 1;
   if (px > (x + ((w * 2) / 3))) ei.hover_right = 0;
   ei.hover_top = 1;
   if (py < (y + (h / 3))) ei.hover_top = 0;
   ei.hover_bottom = 1;
   if (py > (y + ((h * 2) / 3))) ei.hover_bottom = 0;
   evas_object_smart_callback_call(data, "anchor,clicked", &ei);
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, data);
   evas_object_show(wd->hover);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->hover_parent = NULL;
}

/**
 * Add a new Anchorview object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Anchorview
 */
EAPI Evas_Object *
elm_anchorview_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "anchorview");
   elm_widget_type_set(obj, "anchorview");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);

   wd->scroller = elm_scroller_add(parent);
   elm_widget_resize_object_set(obj, wd->scroller);
   wd->entry = elm_entry_add(parent);
   elm_entry_editable_set(wd->entry, 0);
   evas_object_size_hint_weight_set(wd->entry, 1.0, 1.0);
   evas_object_size_hint_align_set(wd->entry, -1.0, -1.0);
   elm_scroller_content_set(wd->scroller, wd->entry);
   evas_object_show(wd->entry);

   evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);

   elm_entry_entry_set(wd->entry, "");

   evas_object_smart_callback_add(wd->entry, "anchor,clicked", _anchor_clicked, obj);

   _sizing_eval(obj);
   return obj;
}

/**
  * Set the text markup of the anchorview
  *
  * This sets the text of the anchorview to be the text given as @p text. This
  * text is in markup format with \<a href=XXX\> beginning an achor with the
  * string link of 'XXX', and \</\> or \</a\> ending the link. Other markup can
  * be used dependign on the style support.
  *
  * @param obj The anchorview object
  * @param text The text to set, or NULL to clear
  *
  * @ingroup Anchorview
  */
EAPI void
elm_anchorview_text_set(Evas_Object *obj, const char *text)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_entry_entry_set(wd->entry, text);
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
   _sizing_eval(obj);
}

/**
  * Set the parent of the hover popup
  *
  * This sets the parent of the hover that anchorview will create. See hover
  * objects for more information on this.
  *
  * @param obj The anchorview object
  * @param parent The parent the hover should use
  *
  * @ingroup Anchorview
  */
EAPI void
elm_anchorview_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_parent)
     evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

/**
  * Set the style that the hover should use
  *
  * This sets the style for the hover that anchorview will create. See hover
  * objects for more information
  *
  * @param obj The anchorview object
  * @param style The style to use
  *
  * @ingroup Anchorview
  */
EAPI void
elm_anchorview_hover_style_set(Evas_Object *obj, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   wd->hover_style = NULL;
   if (style) wd->hover_style = eina_stringshare_add(style);
}

/**
  * Stop the hover popup in the anchorview
  *
  * This will stop the hover popup in the anchorview if it is currently active.
  *
  * @param obj The anchorview object
  *
  * @ingroup Anchorview
  */
EAPI void
elm_anchorview_hover_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
}

/**
 * Set bounce mode
 *
 * This will enable or disable the scroller bounce mode for the anchorview. See
 * elm_scroller_bounce_set() for details
 *
 * @param obj The anchorview anchorview
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Anchorview
 */
EAPI void
elm_anchorview_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_scroller_bounce_set(wd->scroller, h_bounce, v_bounce);
}
