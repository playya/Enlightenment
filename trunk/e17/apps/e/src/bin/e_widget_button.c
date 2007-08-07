/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *o_button;
   Evas_Object *o_icon;
   void (*func) (void *data, void *data2);
   void *data;
   void *data2;   
};

static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_activate_hook(Evas_Object *obj);
static void _e_wid_disable_hook(Evas_Object *obj);
static void _e_wid_signal_cb1(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);
/* local subsystem functions */

/* externally accessible functions */
EAPI Evas_Object *
e_widget_button_add(Evas *evas, const char *label, const char *icon, void (*func) (void *data, void *data2), void *data, void *data2)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;
   Evas_Coord mw, mh;
   
   obj = e_widget_add(evas);
   
   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   e_widget_activate_hook_set(obj, _e_wid_activate_hook);
   e_widget_disable_hook_set(obj, _e_wid_disable_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   wd->func = func;
   wd->data = data;
   wd->data2 = data2;
   e_widget_data_set(obj, wd);
   
   o = edje_object_add(evas);
   wd->o_button = o;
   e_theme_edje_object_set(o, "base/theme/widgets",
			   "e/widgets/button");
   edje_object_signal_callback_add(o, "e,action,click", "", _e_wid_signal_cb1, obj);
   edje_object_part_text_set(o, "e.text.label", label);
   evas_object_show(o);
   
   e_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_wid_focus_steal, obj);
   e_widget_resize_object_set(obj, o);
   
   if (icon)
     {
	o = edje_object_add(evas);
	wd->o_icon = o;
	e_util_edje_icon_set(o, icon);
	edje_object_part_swallow(wd->o_button, "e.swallow.icon", o);
	edje_object_signal_emit(wd->o_button, "e,action,show,icon", "e");
	edje_object_message_signal_process(wd->o_button);
	evas_object_show(o);
	e_widget_sub_object_add(obj, o);
     }
   
   edje_object_size_min_calc(wd->o_button, &mw, &mh);
   e_widget_min_size_set(obj, mw, mh);
   
   return obj;
}

EAPI void
e_widget_button_label_set(Evas_Object *obj, const char *label)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   edje_object_part_text_set(wd->o_button, "e.text.label", label);
}

EAPI void
e_widget_button_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   if (wd->o_icon)
     {
	e_widget_sub_object_del(obj, wd->o_icon);
	evas_object_hide(wd->o_icon);
	edje_object_signal_emit(wd->o_button, "e,action,hide,icon", "e");
	edje_object_message_signal_process(wd->o_button);
	edje_object_part_unswallow(wd->o_button, wd->o_icon);
	evas_object_del(wd->o_icon);
	wd->o_icon = NULL;
     }
   if (icon)
     {
	wd->o_icon = icon;
	edje_object_part_swallow(wd->o_button, "e.swallow.icon", icon);
	edje_object_signal_emit(wd->o_button, "e,action,show,icon", "e");
	edje_object_message_signal_process(wd->o_button);
	evas_object_pass_events_set(icon, 1);
	evas_object_show(icon);
	e_widget_sub_object_add(obj, icon);
     }
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   free(wd);
}

static void
_e_wid_focus_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if (e_widget_focus_get(obj))
     {
	edje_object_signal_emit(wd->o_button, "e,state,focused", "e");
	evas_object_focus_set(wd->o_button, 1);
     }
   else
     {
	edje_object_signal_emit(wd->o_button, "e,state,unfocused", "e");
	evas_object_focus_set(wd->o_button, 0);
     }
}

static void
_e_wid_activate_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if (wd->func) wd->func(wd->data, wd->data2);
}

static void
_e_wid_disable_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if (e_widget_disabled_get(obj))
     edje_object_signal_emit(wd->o_button, "e,state,disabled", "e");
   else
     edje_object_signal_emit(wd->o_button, "e,state,enabled", "e");
}

static void
_e_wid_signal_cb1(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Evas_Object *wid;

   wid = data;
   if (!wid || e_widget_disabled_get(wid)) return;
   e_widget_focus_steal(wid);
   e_widget_change(wid);
   _e_wid_activate_hook(wid);
}

static void
_e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   e_widget_focus_steal(data);
}
