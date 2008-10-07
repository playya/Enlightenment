#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tgl;
   Evas_Bool state;
   Evas_Bool *statep;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->tgl, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   edje_object_part_swallow(wd->tgl, "elm.swallow.content", obj);
   _sizing_eval(data);
}

static void
_signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->state = 0;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
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
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->tgl = edje_object_add(e);
   _elm_theme_set(wd->tgl, "toggle", "toggle");
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,on", "", _signal_toggle_on, obj);
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,off", "", _signal_toggle_off, obj);
   elm_widget_resize_object_set(obj, wd->tgl);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_toggle_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;
   
   if (label)
     {
	edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->tgl);
     }
   else
     {
	edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->tgl);
     }
   edje_object_part_text_set(wd->tgl, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_swallow(wd->tgl, "elm.swallow.content", icon);
   edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
   elm_widget_sub_object_add(obj, icon);
   evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   // FIXME: track new sub obj...
   _sizing_eval(obj);
}

EAPI void
elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_text_set(wd->tgl, "elm.ontext", onlabel);
   edje_object_part_text_set(wd->tgl, "elm.offtext", offlabel);
   _sizing_eval(obj);
}

EAPI void
elm_toggle_state_set(Evas_Object *obj, Evas_Bool state)
{
   Widget_Data *wd = elm_widget_data_get(obj);
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

EAPI void
elm_toggle_state_pointer_set(Evas_Object *obj, Evas_Bool *statep)
{
   Widget_Data *wd = elm_widget_data_get(obj);

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
}
