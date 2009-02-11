#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn;
   Evas_Object *icon;
   const char *label;
   const char *style;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->style) eina_stringshare_del(wd->style);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->style) _elm_theme_set(wd->btn, "button", "base", wd->style);
   else _elm_theme_set(wd->btn, "button", "base", "default");
   if (wd->icon)
     edje_object_part_swallow(wd->btn, "elm.swallow.content", wd->icon);
   if (wd->label)
     edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
   if (wd->icon)
     edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
   edje_object_part_text_set(wd->btn, "elm.text", wd->label);
   edje_object_message_signal_process(wd->btn);
   edje_object_scale_set(wd->btn, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
 
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->btn, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   edje_object_part_swallow(wd->btn, "elm.swallow.content", obj);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *sub = event_info;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->icon = NULL;
        edje_object_message_signal_process(wd->btn);
	_sizing_eval(obj);
     }
}

static void
_signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, "clicked", NULL);
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
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
   
   wd->btn = edje_object_add(e);
   _elm_theme_set(wd->btn, "button", "base", "default");
   edje_object_signal_callback_add(wd->btn, "elm,action,click", "", _signal_clicked, obj);
   elm_widget_resize_object_set(obj, wd->btn);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_button_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord mw, mh;

   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
        wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->btn);
     }
   else
     {
        wd->label = NULL;
	edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->btn);
     }
   edje_object_part_text_set(wd->btn, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_button_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   if ((icon) && (wd->icon != icon))
     {
        wd->icon = icon;
	elm_widget_sub_object_add(obj, icon);
	edje_object_part_swallow(wd->btn, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
        edje_object_message_signal_process(wd->btn);
	_sizing_eval(obj);
     }
   else
     wd->icon = icon;
}

EAPI void
elm_button_style_set(Evas_Object *obj, const char *style)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->style) eina_stringshare_del(wd->style);
   if (style) wd->style = eina_stringshare_add(style);
   else wd->style = NULL;
   _theme_hook(obj);
}
