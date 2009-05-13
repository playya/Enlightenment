#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *hover;
   Evas_Object *hover_parent;
   Eina_List *items;
};

struct _Elm_Hoversel_Item
{
   Evas_Object *obj;
   const char *label;
   const char *icon_file;
   Elm_Icon_Type icon_type;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *data;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_hoversel_hover_end(obj);
   elm_hoversel_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Elm_Hoversel_Item *it;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_hoversel_hover_end(obj);
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        free(it);
     }
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->btn, &minw, &minh);
   evas_object_size_hint_max_get(wd->btn, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj, void *event_info)
{
   elm_hoversel_hover_end(data);
}

static void
_item_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Hoversel_Item *it = data;
   Evas_Object *obj2 = it->obj;
   elm_hoversel_hover_end(obj2);
   if (it->func) it->func(it->data, obj2, it);
   evas_object_smart_callback_call(obj2, "selected", it);
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *bt, *bx, *ic;
   const Eina_List *l;
   const Elm_Hoversel_Item *it;

   wd->hover = elm_hover_add(obj);
   elm_hover_style_set(wd->hover, "hoversel_vertical");
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, obj);
   elm_hover_parent_set(wd->hover, wd->hover_parent);
   elm_hover_target_set(wd->hover, wd->btn);

   bx = elm_box_add(wd->hover);
   elm_box_homogenous_set(bx, 1);

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        bt = elm_button_add(wd->hover);
        elm_button_style_set(bt, "hoversel_vertical_entry");
        elm_button_label_set(bt, it->label);
        if (it->icon_file)
          {
             ic = elm_icon_add(obj);
             elm_icon_scale_set(ic, 0, 1);
             if (it->icon_type == ELM_ICON_FILE)
               elm_icon_file_set(ic, it->icon_file, NULL);
             else if (it->icon_type == ELM_ICON_STANDARD)
               elm_icon_standard_set(ic, it->icon_file);
             elm_button_icon_set(bt, ic);
             evas_object_show(ic);
          }
        evas_object_size_hint_weight_set(bt, 1.0, 0.0);
        evas_object_size_hint_align_set(bt, -1.0, -1.0);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", _item_clicked, it);
        evas_object_show(bt);
     }

   elm_hover_content_set
     (wd->hover,
      elm_hover_best_content_location_get(wd->hover, ELM_HOVER_AXIS_VERTICAL),
      bx);
   evas_object_show(bx);
   
   evas_object_show(wd->hover);
   evas_object_smart_callback_call(obj, "clicked", NULL);
}

static void
_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
  _activate(data); 
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->hover_parent = NULL;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "hoversel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->btn = elm_button_add(parent);
   elm_button_style_set(wd->btn, "hoversel_vertical");
   elm_widget_resize_object_set(obj, wd->btn);
   evas_object_event_callback_add(wd->btn, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->btn, "clicked", _button_clicked, obj);
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_parent)
     evas_object_event_callback_del(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

EAPI void
elm_hoversel_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_button_label_set(wd->btn, label);
}

EAPI void
elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_button_icon_set(wd->btn, icon);
}

EAPI void
elm_hoversel_hover_begin(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover) return;
   _activate(obj);
}

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover)
     {
        evas_object_del(wd->hover);
        wd->hover = NULL;
        evas_object_smart_callback_call(obj, "dismissed", NULL);
     }
}

EAPI Elm_Hoversel_Item *
elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Hoversel_Item *it = calloc(1, sizeof(Elm_Hoversel_Item));
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon_file = eina_stringshare_add(icon_file);
   it->icon_type = icon_type;
   it->func = func;
   it->data = (void *)data;
   return it;
}

EAPI void
elm_hoversel_item_del(Elm_Hoversel_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   if (!wd) return;
   elm_hoversel_hover_end(it->obj);
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   eina_stringshare_del(it->icon_file);
   free(it);
}

EAPI void *
elm_hoversel_item_data_get(Elm_Hoversel_Item *it)
{
   if (!it) return NULL;
   return it->data;
}
