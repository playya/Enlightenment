#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller, *box;
   Eina_List *items;
   Eina_List *selected;
   Elementary_List_Mode mode;
   Evas_Coord minw[2], minh[2];
   Evas_Bool on_hold : 1;
   Evas_Bool multi : 1;
};

struct _Elm_List_Item
{
   Evas_Object *obj;
   Evas_Object *base;
   const char *label;
   Evas_Object *icon, *end;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   const void *data;
   Evas_Bool even : 1;
   Evas_Bool is_even : 1;
   Evas_Bool fixed : 1;
   Evas_Bool selected : 1;
   Evas_Bool dummy_icon : 1;
   Evas_Bool dummy_end : 1;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _fix_items(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   while (wd->items)
     {
        Elm_List_Item *it = wd->items->data;
        wd->items = eina_list_remove_list(wd->items, wd->items);
        eina_stringshare_del(it->label);
        if (!it->fixed)
          {
             if (it->icon) evas_object_del(it->icon);
             if (it->end) evas_object_del(it->end);
          }
        if (it->base) evas_object_del(it->base);
        free(it);
     }
   eina_list_free(wd->selected);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   evas_object_size_hint_min_get(wd->scroller, &minw, &minh);
   evas_object_size_hint_max_get(wd->scroller, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
//   if (elm_widget_focus_get(obj))
//     elm_widget_focus_steal(wd->entry);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
//   _fix_items(data);
//   _sizing_eval(data);
}

static void 
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   Eina_List *l;
   
   for (l = wd->items; l; l = l->next)
     {
        Elm_List_Item *it = l->data;
        
        if ((sub == it->icon) || (sub == it->end))
          {
             if (it->icon == sub) it->icon = NULL;
             if (it->end == sub) it->end = NULL;
             evas_object_event_callback_del
               (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
             _fix_items(obj);
             _sizing_eval(obj);
             break;
          }
     }
}

static void
_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = 1;
   else wd->on_hold = 0;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(it->obj, "clicked", it);
}

static void
_item_select(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   const char *selectraise;
   if (it->selected) return;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     evas_object_raise(it->base);
   it->selected = 1;
   wd->selected = eina_list_append(wd->selected, it);
   if (it->func) it->func((void *)it->data, it->obj, it);
   evas_object_smart_callback_call(it->obj, "selected", it);
}

static void
_item_unselect(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   const char *stacking, *selectraise;
   if (!it->selected) return;
   edje_object_signal_emit(it->base, "elm,state,unselected", "elm");
   stacking = edje_object_data_get(it->base, "stacking");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
        if ((stacking) && (!strcmp(stacking, "below")))
          evas_object_lower(it->base);
     }
   it->selected = 0;
   wd->selected = eina_list_remove(wd->selected, it);
   evas_object_smart_callback_call(it->obj, "unselected", it);
}

static void
_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_List *l;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = 1;
   else wd->on_hold = 0;
   if (wd->on_hold)
     {
        wd->on_hold = 0;
        return;
     }
   if (wd->multi)
     {
        if (!it->selected) _item_select(it);
        else _item_unselect(it);
     }
   else
     {
        for (l = wd->selected; l;)
          {
             Elm_List_Item *it2 = l->data;
             l = l->next;
             if ((it2 != it) && (it2->selected)) _item_unselect(it2);
          }
        if (!it->selected) _item_select(it);
     }
}

static Elm_List_Item *
_item_new(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it;
   
   it = calloc(1, sizeof(Elm_List_Item));
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->end = end;
   it->func = func;
   it->data = data;
   it->base = edje_object_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_size_hint_weight_set(it->base, 1.0, 1.0);
   evas_object_size_hint_align_set(it->base, -1.0, -1.0);
   if (it->icon)
     {
        elm_widget_sub_object_add(obj, it->icon);
        evas_object_event_callback_add(it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
     }
   if (it->end)
     {
        elm_widget_sub_object_add(obj, it->end);
        evas_object_event_callback_add(it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
     }
   return it;
}

static void
_fix_items(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Evas_Coord minw[2] = { 0, 0 }, minh[2] = { 0, 0 };
   Evas_Coord mw, mh;
   int i, redo = 0;
   
   for (l = wd->items; l; l = l->next)
     {
        Evas_Coord mw, mh;
        Elm_List_Item *it = l->data;
        if (it->icon)
          {
             evas_object_size_hint_min_get(it->icon, &mw, &mh);
             if (mw > minw[0]) minw[0] = mw;
             if (mh > minh[0]) minh[0] = mh;
          }
        if (it->end)
          {
             evas_object_size_hint_min_get(it->end, &mw, &mh);
             if (mw > minw[1]) minw[1] = mw;
             if (mh > minh[1]) minh[1] = mh;
          }
     }
   if ((minw[0] != wd->minw[0]) || (minw[1] != wd->minw[1]) || 
       (minw[0] != wd->minh[0]) || (minh[1] != wd->minh[1]))
     {
        wd->minw[0] = minw[0];
        wd->minw[1] = minw[1];
        wd->minh[0] = minh[0];
        wd->minh[1] = minh[1];
        redo = 1;
     }
   for (i = 0, l = wd->items; l; l = l->next, i++)
     {
        Elm_List_Item *it = l->data;
        it->even = i & 0x1;
        if ((it->even != it->is_even) || (!it->fixed) || (redo))
          {
             const char *stacking;
             
             if (wd->mode == ELM_LIST_COMPRESS) 
               {
                  if (it->even)
                    _elm_theme_set(it->base, "list", "item_compress", "default");
                  else
                    _elm_theme_set(it->base, "list", "item_compress_odd", "default");
               }
             else
               {
                  if (it->even)
                    _elm_theme_set(it->base, "list", "item", "default");
                  else
                    _elm_theme_set(it->base, "list", "item_odd", "default");
               }
             stacking = edje_object_data_get(it->base, "stacking");
             if (stacking)
               {
                  if (!strcmp(stacking, "below"))
                    evas_object_lower(it->base);
                  else if (!strcmp(stacking, "above"))
                    evas_object_raise(it->base);
               }
             edje_object_part_text_set(it->base, "elm.text", it->label);
             if ((!it->icon) && (minh[0] > 0))
               {
                  it->icon = evas_object_rectangle_add(evas_object_evas_get(it->base));
                  evas_object_color_set(it->icon, 0, 0, 0, 0);
                  it->dummy_icon = 1;
               }
             if ((!it->end) && (minh[1] > 0))
               {
                  it->end = evas_object_rectangle_add(evas_object_evas_get(it->base));
                  evas_object_color_set(it->end, 0, 0, 0, 0);
                  it->dummy_end = 1;
               }
             if (it->icon)
               {
                  evas_object_size_hint_min_set(it->icon, minw[0], minh[0]);
                  evas_object_size_hint_max_set(it->icon, 99999, 99999);
                  edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
               }
             if (it->end)
               {
                  evas_object_size_hint_min_set(it->end, minw[1], minh[1]);
                  evas_object_size_hint_max_set(it->end, 99999, 99999);
                  edje_object_part_swallow(it->base, "elm.swallow.end", it->end);
               }
             if (!it->fixed)
               {
                  edje_object_message_signal_process(it->base);
                  mw = mh = -1;
                  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
                  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  evas_object_size_hint_min_set(it->base, mw, mh);
                  evas_object_show(it->base);
               }
             it->fixed = 1;
             it->is_even = it->even;
          }
     }
   mw = 0; mh = 0;
   evas_object_size_hint_min_get(wd->box, &mw, &mh);
   if (wd->mode == ELM_LIST_LIMIT)
     elm_scroller_content_min_limit(wd->scroller, 1, 0);
   else
     elm_scroller_content_min_limit(wd->scroller, 0, 0);
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_list_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_can_focus_set(obj, 1);
   
   wd->scroller = elm_scroller_add(parent);
   elm_widget_resize_object_set(obj, wd->scroller);
   
   wd->box = elm_box_add(parent);
   elm_box_homogenous_set(wd->box, 1);
   evas_object_size_hint_weight_set(wd->box, 1.0, 0.0);
   evas_object_size_hint_align_set(wd->box, -1.0, 0.0);
   elm_scroller_content_set(wd->scroller, wd->box);
   evas_object_show(wd->box);

   wd->mode = ELM_LIST_SCROLL;

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI Elm_List_Item *
elm_list_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_append(wd->items, it);
   elm_box_pack_end(wd->box, it->base);
   return it;
}

EAPI Elm_List_Item *
elm_list_item_prepend(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_prepend(wd->items, it);
   elm_box_pack_start(wd->box, it->base);
   return it;
}

EAPI Elm_List_Item *
elm_list_item_insert_before(Evas_Object *obj, Elm_List_Item *before, const char *label, Evas_Object *icon, Evas_Object *end, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_prepend_relative(wd->items, it, before);
   elm_box_pack_before(wd->box, it->base, before->base);
   return it;
}

EAPI Elm_List_Item *
elm_list_item_insert_after(Evas_Object *obj, Elm_List_Item *after, const char *label, Evas_Object *icon, Evas_Object *end, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_append_relative(wd->items, it, after);
   elm_box_pack_after(wd->box, it->base, after->base);
   return it;
}

EAPI void
elm_list_go(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _fix_items(obj);
}

EAPI void
elm_list_multi_select_set(Evas_Object *obj, Evas_Bool multi)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->multi = multi;
}

EAPI void
elm_list_horizontal_mode_set(Evas_Object *obj, Elementary_List_Mode mode)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->mode == mode) return;
   wd->mode = mode;
   if (wd->mode == ELM_LIST_LIMIT)
     elm_scroller_content_min_limit(wd->scroller, 1, 0);
   else
     elm_scroller_content_min_limit(wd->scroller, 0, 0);
}

EAPI const Eina_List *
elm_list_items_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->items;
}

EAPI Elm_List_Item *
elm_list_selected_item_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->selected) return wd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_list_selected_items_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->selected;
}

EAPI void
elm_list_item_selected_set(Elm_List_Item *it, Evas_Bool selected)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);

   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
        if (!wd->multi)
          {
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
          }
        _item_select(it);
     }
   else
     _item_unselect(it);
}

EAPI void
elm_list_item_show(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord bx, by, bw, bh;
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(wd->box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   x -= bx;
   y -= by;
   elm_scroller_region_show(wd->scroller, x, y, w, h);
}

EAPI void
elm_list_item_del(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   if (it->selected) _item_unselect(it);
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   if (it->icon) evas_object_del(it->icon);
   if (it->end) evas_object_del(it->end);
   if (it->base) evas_object_del(it->base);
   free(it);
}

EAPI const void *
elm_list_item_data_get(const Elm_List_Item *it)
{
   return it->data;
}

EAPI Evas_Object *
elm_list_item_icon_get(const Elm_List_Item *it)
{
   if (it->dummy_icon) return NULL;
   return it->icon;
}

EAPI Evas_Object *
elm_list_item_end_get(const Elm_List_Item *it)
{
   if (it->dummy_end) return NULL;
   return it->end;
}
