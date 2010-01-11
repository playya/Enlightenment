#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Button Button
 *
 * This is a push-button. Press it and run some function. It can contain
 * a simple label and icon object.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *icon;
   const char *label;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
	Widget_Data *wd = elm_widget_data_get(obj);
	Evas_Object *top = elm_widget_top_get(obj);

   	if (elm_widget_focus_get(obj)) {
		edje_object_signal_emit(wd->btn, "elm,action,focus", "elm");

		if( top ) {
			elm_win_keyboard_mode_set( top, ELM_WIN_KEYBOARD_ON );
		}

	}else {
		edje_object_signal_emit(wd->btn, "elm,action,unfocus", "elm");

		if ( top ) {
			elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
		}
	}
	
}



static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_theme_set(wd->btn, "button", "base", elm_widget_style_get(obj));
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
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->btn, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,enabled", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!wd) return;
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
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
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

/**
 * Add a new button to the parent
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Button
 */
EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "button");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set( obj, _on_focus_hook, NULL );
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set( obj, 1 );                 

   wd->btn = edje_object_add(e);
   _elm_theme_set(wd->btn, "button", "base", "default");
   edje_object_signal_callback_add(wd->btn, "elm,action,click", "",
                                   _signal_clicked, obj);
   elm_widget_resize_object_set(obj, wd->btn);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the label used in the button
 *
 * @param obj The button object
 * @param label The text will be written on the button
 *
 * @ingroup Button
 */
EAPI void
elm_button_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;

   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
	wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
     }
   else
     {
	wd->label = NULL;
	edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
     }
   edje_object_message_signal_process(wd->btn);
   edje_object_part_text_set(wd->btn, "elm.text", label);
   _sizing_eval(obj);
}

EAPI const char*
elm_button_label_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Set the icon used for the button
 *
 * @param obj The button object
 * @param icon  The image for the button
 *
 * @ingroup Button
 */
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
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	edje_object_part_swallow(wd->btn, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
	edje_object_message_signal_process(wd->btn);
	_sizing_eval(obj);
     }
   else
     wd->icon = icon;
}

EAPI Evas_Object *
elm_button_icon_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Set the button style
 *
 * @param obj The button object
 * @param style The style for the button
 *
 * DEPRECATED. use elm_object_style_set() instead
 *
 * @ingroup Button
 */
EAPI void
elm_button_style_set(Evas_Object *obj, const char *style)
{
   elm_widget_style_set(obj, style);
}
