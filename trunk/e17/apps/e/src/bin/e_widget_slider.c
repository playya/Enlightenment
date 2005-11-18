/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *o_slider;
};

static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);

/* externally accessible functions */
Evas_Object *
e_widget_slider_add(Evas *evas)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;
   Evas_Coord mw, mh;
   
   obj = e_widget_add(evas);
   
   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   e_widget_data_set(obj, wd);

   o = e_slider_add(evas);
   wd->o_slider = o;
   evas_object_show(o);
   e_widget_sub_object_add(obj, o);
   e_widget_resize_object_set(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_wid_focus_steal, obj);
   return obj;
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
//	edje_object_signal_emit(e_scrollframe_edje_object_get(wd->o_scrollframe), "focus_in", "");
	evas_object_focus_set(wd->o_slider, 1);
     }
   else
     {
//	edje_object_signal_emit(e_scrollframe_edje_object_get(wd->o_scrollframe), "focus_out", "");
	evas_object_focus_set(wd->o_slider, 0);
     }
}

static void
_e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   e_widget_focus_steal(data);
}
