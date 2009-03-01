#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *base[2];
   Eina_List *stack;
   Evas_Object *top, *oldtop;
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
   edje_object_scale_set(wd->base[0], elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_scale_set(wd->base[1], elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord minw2 = -1, minh2 = -1;
   
   edje_object_size_min_calc(wd->base[0], &minw, &minh);
   edje_object_size_min_calc(wd->base[1], &minw2, &minh2);
   if (minw < minw2) minw = minw2;
   if (minh < minh2) minh = minh2;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   // FIXME: if sub is top of stack
}    

static void
_eval_top(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *stacktop = elm_pager_content_top_get(obj);
   if (stacktop != wd->top)
     {
        wd->oldtop = wd->top;
        wd->top = stacktop;
        // FIXME: transition from oldtop to top
        edje_object_part_swallow(wd->base[1], "elm.swallow.content", wd->top);
     }
}

EAPI Evas_Object *
elm_pager_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   
   wd->base[1] = edje_object_add(e);
   _elm_theme_set(wd->base[1], "pager", "base", "default");
   elm_widget_resize_object_set(obj, wd->base[1]);
   
   wd->base[0] = edje_object_add(e);
   _elm_theme_set(wd->base[0], "pager", "base", "default");
   // FIXME: only 1 resize obj!
   elm_widget_resize_object_set(obj, wd->base[0]);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_pager_content_push(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->stack = eina_list_append(wd->stack, content);
   // FIXME: adjust min size, show new page
}

EAPI void
elm_pager_content_pop(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME actuall make pop animated - promote 2nd last in stack then
   // when anim finished delete 2nd last (which was top).
   Evas_Object *top = elm_pager_content_top_get(obj);
   if (wd->top) evas_object_del(wd->top);
}

EAPI void
elm_pager_content_promote(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME: add to end of stack list and animate
}

EAPI Evas_Object *
elm_pager_content_bottom_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->stack) return wd->stack->data;
   return NULL;
}

EAPI Evas_Object *
elm_pager_content_top_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->stack) return eina_list_last(wd->stack)->data;
   return NULL;
}

