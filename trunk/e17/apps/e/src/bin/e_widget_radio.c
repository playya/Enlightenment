/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

struct _E_Radio_Group
{
   int *valptr;
   Evas_List *radios;
};

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   E_Radio_Group *group;
   Evas_Object *o_radio;
   int valnum;
};

static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_do(Evas_Object *obj);
static void _e_wid_activate_hook(Evas_Object *obj);
static void _e_wid_signal_cb1(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);
/* local subsystem functions */

/* externally accessible functions */
E_Radio_Group *
e_widget_radio_group_new(int *val)
{
   E_Radio_Group *group;
   
   group = calloc(1, sizeof(E_Radio_Group));
   group->valptr = val;
   return group;
}

Evas_Object *
e_widget_radio_add(Evas *evas, char *label, int valnum, E_Radio_Group *group)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;
   Evas_Coord mw, mh;
   
   obj = e_widget_add(evas);
   
   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   e_widget_activate_hook_set(obj, _e_wid_activate_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   wd->group = group;
   wd->valnum = valnum;
   e_widget_data_set(obj, wd);
   
   o = edje_object_add(evas);
   wd->o_radio = o;
   e_theme_edje_object_set(o, "base/theme/widgets",
			   "widgets/radio");
   edje_object_signal_callback_add(o, "toggled", "*", _e_wid_signal_cb1, obj);
   edje_object_part_text_set(o, "label", label);
   evas_object_show(o);
   edje_object_size_min_calc(o, &mw, &mh);
   e_widget_min_size_set(obj, mw, mh);
   if ((wd->group) && (wd->group->valptr))
     {
	if (*(wd->group->valptr) == valnum) edje_object_signal_emit(o, "toggle_on", "");
     }
   if (wd->group)
     {
	wd->group->radios = evas_list_append(wd->group->radios, obj);
     }
   
   e_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_wid_focus_steal, obj);
   e_widget_resize_object_set(obj, o);
   
   return obj;
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if (wd->group)
     {
	wd->group->radios = evas_list_remove(wd->group->radios, obj);
	if (!wd->group->radios) free(wd->group);
     }
   free(wd);
}

static void
_e_wid_focus_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if (e_widget_focus_get(obj))
     {
	edje_object_signal_emit(wd->o_radio, "focus_in", "");
	evas_object_focus_set(wd->o_radio, 1);
     }
   else
     {
	edje_object_signal_emit(wd->o_radio, "focus_out", "");
	evas_object_focus_set(wd->o_radio, 0);
     }
}

static void
_e_wid_do(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   if ((wd->group) && (wd->group->valptr))
     {
	Evas_List *l;
	int toggled = 0;
	
	for (l = wd->group->radios; l; l = l->next)
	  {
	     wd = e_widget_data_get(l->data);
	     if (l->data != obj)
	       {
		  wd = e_widget_data_get(l->data);
		  if (wd->valnum == *(wd->group->valptr))
		    {
		       edje_object_signal_emit(wd->o_radio, "toggle_off", "");
		       toggled = 1;
		       break;
		    }
	       }
	  }
	if (!toggled) return;
	wd = e_widget_data_get(obj);
	*(wd->group->valptr) = wd->valnum;
     }
}

static void 
_e_wid_activate_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;
   
   wd = e_widget_data_get(obj);
   _e_wid_do(obj);
   edje_object_signal_emit(wd->o_radio, "toggle_on", "");
}

static void
_e_wid_signal_cb1(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   _e_wid_do(data);
}

static void
_e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   e_widget_focus_steal(data);
}
