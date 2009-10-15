#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Pager Pager
 *
 * The pager is an object that allows flipping (with animation) between 1 or
 * more “pages” of objects, much like a stack of windows within the window.
 *
 * Objects can be pushed or popped from he stack or deleted as normal.
 * Pushes and pops will animate (and a pop will delete the object once the
 * animation is finished). Any object in the pager can be promoted to the top
 * (from its current stacking position) as well. Objects are pushed to the
 * top with elm_pager_content_push() and when the top item is no longer
 * wanted, simply pop it with elm_pager_content_pop() and it will also be
 * deleted. Any object you wish to promote to the top that is already in the
 * pager, simply use elm_pager_content_promote(). If an object is no longer
 * needed and is not the top item, just delete it as normal. You can query
 * which objects are the top and bottom with elm_pager_content_bottom_get()
 * and elm_pager_content_top_get().
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;

struct _Widget_Data
{
   Eina_List *stack;
   Item *top, *oldtop;
};

struct _Item
{
   Evas_Object *obj, *base, *content;
   Evas_Coord minw, minh;
   Eina_Bool popme : 1;
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
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     edje_object_scale_set(it->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     {
	if (it->minw > minw) minw = it->minw;
	if (it->minh > minh) minh = it->minh;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Item *it = data;
   edje_object_size_min_calc(it->base, &it->minw, &it->minh);
   _sizing_eval(it->obj);
}

static void
_eval_top(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *ittop;
   if (!wd->stack) return;
   ittop = eina_list_last(wd->stack)->data;
   if (ittop != wd->top)
     {
	Evas_Object *o;
	const char *onshow, *onhide;

	if (wd->top)
	  {
	     o = wd->top->base;
	     edje_object_signal_emit(o, "elm,action,hide", "elm");
	     onhide = edje_object_data_get(o, "onhide");
	     if (onhide)
	       {
		  if (!strcmp(onhide, "raise")) evas_object_raise(o);
		  else if (!strcmp(onhide, "lower")) evas_object_lower(o);
	       }
	  }
	wd->top = ittop;
	o = wd->top->base;
	evas_object_show(o);
	edje_object_signal_emit(o, "elm,action,show", "elm");
	onshow = edje_object_data_get(o, "onshow");
	if (onshow)
	  {
	     if (!strcmp(onshow, "raise")) evas_object_raise(o);
	     else if (!strcmp(onshow, "lower")) evas_object_lower(o);
	  }
     }
}

static void
_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y;
   Eina_List *l;
   Item *it;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   EINA_LIST_FOREACH(wd->stack, l, it)
     evas_object_move(it->base, x, y);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Object *sub = event_info;
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     {
	if (it->content == sub)
	  {
	     wd->stack = eina_list_remove_list(wd->stack, l);
	     evas_object_event_callback_del
	       (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	     evas_object_del(it->base);
	     _eval_top(it->obj);
	     free(it);
	     return;
	  }
     }
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord w, h;
   Eina_List *l;
   Item *it;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(wd->stack, l, it)
     evas_object_resize(it->base, w, h);
}

static void
_signal_hide_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Item *it = data;
   Evas_Object *obj2 = it->obj;
   evas_object_hide(it->base);
   edje_object_signal_emit(it->base, "elm,action,reset", "elm");
   edje_object_message_signal_process(it->base);
   if (it->popme)
     {
	evas_object_del(it->content);
     }
   _sizing_eval(obj2);
}

/**
 * Add a new pager to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Pager
 */
EAPI Evas_Object *
elm_pager_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "pager");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Push an object to the top of the pager stack (and show it)
 *
 * The object pushed becomes a child of the pager and will be controlled
 * it and deleted when the pager is deleted.
 *
 * @param obj The pager object
 * @param content The object to push
 *
 * @ingroup Pager
 */
EAPI void
elm_pager_content_push(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it = calloc(1, sizeof(Item));
   Evas_Coord x, y, w, h;
   if (!it) return;
   it->obj = obj;
   it->content = content;
   it->base = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(it->base, obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(it->base, x, y);
   evas_object_resize(it->base, w, h);
   elm_widget_sub_object_add(obj, it->base);
   elm_widget_sub_object_add(obj, it->content);
   _elm_theme_set(it->base,  "pager", "base", elm_widget_style_get(obj));
   edje_object_signal_callback_add(it->base, "elm,action,hide,finished", "", _signal_hide_finished, it);
   edje_object_part_swallow(it->base, "elm.swallow.content", it->content);
   edje_object_size_min_calc(it->base, &it->minw, &it->minh);
   evas_object_show(it->content);
   evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, it);
   wd->stack = eina_list_append(wd->stack, it);
   _eval_top(obj);
   _sizing_eval(obj);
}

/**
 * Pop the object that is on top of the stack
 *
 * This pops the object that is on top (visible) in the pager, makes it
 * disappear, then deletes the object. The object that was underneath it
 * on the stack will become visible.
 *
 * @param obj The pager object
 *
 * @ingroup Pager
 */
EAPI void
elm_pager_content_pop(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *ll;
   Item *it;
   if (!wd->stack) return;
   it = eina_list_last(wd->stack)->data;
   it->popme = EINA_TRUE;
   ll = eina_list_last(wd->stack);
   if (ll)
     {
	ll = ll->prev;
	if (!ll)
	  {
	     Evas_Object *o;
	     const char *onhide;

	     wd->top = it;
	     o = wd->top->base;
	     edje_object_signal_emit(o, "elm,action,hide", "elm");
	     onhide = edje_object_data_get(o, "onhide");
	     if (onhide)
	       {
		  if (!strcmp(onhide, "raise")) evas_object_raise(o);
		  else if (!strcmp(onhide, "lower")) evas_object_lower(o);
	       }
	     wd->top = NULL;
	  }
	else
	  {
	     it = ll->data;
	     elm_pager_content_promote(obj, it->content);
	  }
     }
}

/**
 * Promote an object already in the pager stack to the top of the stack
 *
 * This will take the indicated object and promote it to the top of the stack
 * as if it had been pushed there. The object must already be inside the
 * pager stack to work.
 *
 * @param obj The pager object
 * @param content The object to promote
 *
 * @ingroup Pager
 */
EAPI void
elm_pager_content_promote(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->stack, l, it)
     {
	if (it->content == content)
	  {
	     wd->stack = eina_list_remove_list(wd->stack, l);
	     wd->stack = eina_list_append(wd->stack, it);
	     _eval_top(obj);
	     return;
	  }
     }
}

/**
 * Return the object at the bottom of the pager stack
 *
 * @param obj The pager object
 * @return The bottom object or NULL if none
 *
 * @ingroup Pager
 */
EAPI Evas_Object *
elm_pager_content_bottom_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd->stack) return NULL;
   it = wd->stack->data;
   return it->content;
}

/**
 * Return the object at the top of the pager stack
 *
 * @param obj The pager object
 * @return The top object or NULL if none
 *
 * @ingroup Pager
 */
EAPI Evas_Object *
elm_pager_content_top_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd->stack) return NULL;
   it = eina_list_last(wd->stack)->data;
   return it->content;
}

/**
 * Set the pager style
 *
 * @param obj The pager object
 * @param style The style will be applied in this pager
 *
 * @ingroup Pager
 */
EAPI void
elm_pager_style_set(Evas_Object *obj, const char *style)
{
   elm_widget_style_set(obj, style);
}
