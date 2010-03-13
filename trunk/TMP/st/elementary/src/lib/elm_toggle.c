#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tgl;
   Evas_Object *icon;
   Eina_Bool state;
   Eina_Bool *statep;
   const char *label;
   const char *ontext, *offtext;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->ontext) eina_stringshare_del(wd->ontext);
   if (wd->offtext) eina_stringshare_del(wd->offtext);
   free(wd);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->tgl, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,enabled", "elm");
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_theme_set(wd->tgl, "toggle", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->tgl, "elm.text", wd->label);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);
   edje_object_message_signal_process(wd->tgl);
   edje_object_scale_set(wd->tgl, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->tgl, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del_full
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
	wd->icon = NULL;
	_sizing_eval(obj);
     }
}

static void
_signal_toggle_off(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = 0;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_signal_toggle_on(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->state = 1;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, "changed", NULL);
}

EAPI Evas_Object *
elm_toggle_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "toggle");
   elm_widget_type_set(obj, "toggle");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);

   wd->tgl = edje_object_add(e);
   _elm_theme_set(wd->tgl, "toggle", "base", "default");
   wd->ontext = eina_stringshare_add("ON");
   wd->offtext = eina_stringshare_add("OFF");
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,on", "",
                                   _signal_toggle_on, obj);
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,off", "",
                                   _signal_toggle_off, obj);
   elm_widget_resize_object_set(obj, wd->tgl);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_toggle_label_set(Evas_Object *obj, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(wd->tgl);
   edje_object_part_text_set(wd->tgl, "elm.text", label);
   _sizing_eval(obj);
}

EAPI const char *
elm_toggle_label_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->label;
}

EAPI void
elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   wd->icon = icon;
   if (!icon) return;
   elm_widget_sub_object_add(obj, icon);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   edje_object_part_swallow(wd->tgl, "elm.swallow.content", icon);
   edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_toggle_icon_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

EAPI void
elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->ontext, onlabel);
   eina_stringshare_replace(&wd->offtext, offlabel);
   edje_object_part_text_set(wd->tgl, "elm.ontext", onlabel);
   edje_object_part_text_set(wd->tgl, "elm.offtext", offlabel);
   _sizing_eval(obj);
}

EAPI void
elm_toggle_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (state != wd->state)
     {
	wd->state = state;
	if (wd->statep) *wd->statep = wd->state;
	if (wd->state)
	  edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
	else
	  edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
     }
}

EAPI Eina_Bool
elm_toggle_state_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->state;
}

EAPI void
elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (statep)
     {
	wd->statep = statep;
	if (*wd->statep != wd->state)
	  {
	     wd->state = *wd->statep;
	     if (wd->state)
	       edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
	     else
	       edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
	  }
     }
   else
     wd->statep = NULL;
}
