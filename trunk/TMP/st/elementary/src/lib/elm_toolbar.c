#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *bx;
   Eina_List *items;
   int icon_size;
   Eina_Bool scrollable : 1;
};

struct _Elm_Toolbar_Item
{
   Evas_Object *obj;
   Evas_Object *base;
   const char *label;
   Evas_Object *icon;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   const void *data;
   Eina_Bool selected : 1;
   Eina_Bool disabled : 1;
   Eina_Bool separator : 1;
};

static void _item_show(Elm_Toolbar_Item *it);
static void _item_select(Elm_Toolbar_Item *it);
static void _item_disable(Elm_Toolbar_Item *it, Eina_Bool disabled);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_item_show(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord x, y, w, h, bx, by;

   if (!wd) return;
   evas_object_geometry_get(wd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x - bx, y - by, w, h);
}

static void
_item_select(Elm_Toolbar_Item *it)
{
   Elm_Toolbar_Item *it2;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2;
   const Eina_List *l;

   if (!wd) return;
   if ((it->selected) || (it->disabled) || (it->separator)) return;
   EINA_LIST_FOREACH(wd->items, l, it2)
     {
	if (it2->selected)
	  {
	     it2->selected = EINA_FALSE;
	     edje_object_signal_emit(it2->base, "elm,state,unselected", "elm");
	     break;
	  }
     }
   it->selected = EINA_TRUE;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   _item_show(it);
   obj2 = it->obj;
   if (it->func) it->func((void *)(it->data), it->obj, it);
   evas_object_smart_callback_call(obj2, "clicked", it);
}

static void 
_item_disable(Elm_Toolbar_Item *it, Eina_Bool disabled)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);

   if (!wd) return;
   if (it->disabled == disabled) return;
   it->disabled = disabled;
   if (it->disabled) 
     edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(it->base, "elm,state,enabled", "elm");
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Toolbar_Item *it;

   if (!wd) return;
   EINA_LIST_FREE(wd->items, it)
     {
	eina_stringshare_del(it->label);
	if (it->icon) evas_object_del(it->icon);
	evas_object_del(it->base);
	free(it);
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Elm_Toolbar_Item *it;
   const char *style = elm_widget_style_get(obj);
   int scale = 0;

   if (!wd) return;
   scale = (elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_scale_set(wd->scr, scale);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        Evas_Coord mw, mh;

	edje_object_scale_set(it->base, scale);
        if (!it->separator) 
          {
             if (it->selected) 
               edje_object_signal_emit(it->base, "elm,state,selected", "elm");
             if (it->disabled)
               edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
             _elm_theme_set(it->base, "toolbar", "item", style);
             if (it->icon)
               {
                  int ms = 0;

                  ms = ((double)wd->icon_size * _elm_config->scale);
                  edje_extern_object_min_size_set(it->icon, ms, ms);
                  edje_object_part_swallow(it->base, "elm.swallow.icon", 
                                           it->icon);
               }
             edje_object_part_text_set(it->base, "elm.text", it->label);
          }
        else 
          {
             _elm_theme_set(it->base, "toolbar", "separator", style);
          }
	mw = mh = -1;
	elm_coords_finger_size_adjust(1, &mw, 1, &mh);
	edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
	elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        evas_object_size_hint_min_set(it->base, mw, mh);
        evas_object_size_hint_max_set(it->base, 9999, mh);
     }
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord vw = 0, vh = 0;
   Evas_Coord w, h;

   if (!wd) return;
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), 
                             &minw, &minh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if(w<minw) w = minw;
   if(h<minh) h = minh;

   evas_object_resize(wd->scr, w, h);

   evas_object_size_hint_min_get(wd->bx, &minw, &minh);
   if(w>minw) minw = w;
   evas_object_resize(wd->bx, minw, minh);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (wd->scrollable)
     {
	minw = w - vw;
	minh = minh + (h - vh);
     }
   else
     {
	minw = minw + (w - vw);
	minh = minh + (h - vh);
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord mw, mh, vw, vh, w, h;
   const Eina_List *l;
   Elm_Toolbar_Item *it;

   if (!wd) return;
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   evas_object_geometry_get(wd->bx, NULL, NULL, &w, &h);
   if (vw >= mw)
     {
	if (w != vw) evas_object_resize(wd->bx, vw, h);
     }
   EINA_LIST_FOREACH(wd->items, l, it)
     {
	if (it->selected)
	  {
	     _item_show(it);
	     break;
	  }
     }
}

static void
_select(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   _item_select(data);
}

EAPI Evas_Object *
elm_toolbar_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "toolbar");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 0);
   elm_smart_scroller_theme_set(wd->scr, "toolbar", "base", "default");
   elm_widget_resize_object_set(obj, wd->scr);
   elm_smart_scroller_policy_set(wd->scr,
				 ELM_SMART_SCROLLER_POLICY_AUTO,
				 ELM_SMART_SCROLLER_POLICY_OFF);

   wd->icon_size = 32;
   wd->scrollable = EINA_TRUE;

   wd->bx = _els_smart_box_add(e);
   _els_smart_box_orientation_set(wd->bx, 1);
   _els_smart_box_homogenous_set(wd->bx, 1);
   elm_widget_sub_object_add(obj, wd->bx);
   elm_smart_scroller_child_set(wd->scr, wd->bx);
   evas_object_show(wd->bx);

   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE, _resize, obj);

   _sizing_eval(obj);
   return obj;
}

EAPI void 
elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (icon_size > 48) return;
   if (wd->icon_size == icon_size) return;
   wd->icon_size = icon_size;
   _theme_hook(obj);
}

EAPI int 
elm_toolbar_icon_size_get(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return 0;
   return wd->icon_size;
}

EAPI Elm_Toolbar_Item *
elm_toolbar_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;
   Elm_Toolbar_Item *it;

   if (!wd) return NULL;
   it = calloc(1, sizeof(Elm_Toolbar_Item));
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->func = func;
   it->data = data;
   it->separator = EINA_FALSE;
   it->base = edje_object_add(evas_object_evas_get(obj));
   _elm_theme_set(it->base, "toolbar", "item", elm_widget_style_get(obj));
   edje_object_signal_callback_add(it->base, "elm,action,click", "elm",
				   _select, it);
   elm_widget_sub_object_add(obj, it->base);
   if (it->icon)
     {
        int ms = 0;

        ms = ((double)wd->icon_size * _elm_config->scale);
	edje_extern_object_min_size_set(it->icon, ms, ms);
	edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
	evas_object_show(it->icon);
	elm_widget_sub_object_add(obj, it->icon);
     }
   edje_object_part_text_set(it->base, "elm.text", it->label);
   mw = mh = -1;
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_weight_set(it->base, 0.0, 0.0);
   evas_object_size_hint_align_set(it->base, -1.0, -1.0);
   evas_object_size_hint_min_set(it->base, mw, mh);
   evas_object_size_hint_max_set(it->base, 9999, mh);
   _els_smart_box_pack_end(wd->bx, it->base);
   evas_object_show(it->base);
   _sizing_eval(obj);
   return it;
}

EAPI Evas_Object *
elm_toolbar_item_icon_get(Elm_Toolbar_Item *item)
{
   if (!item) return NULL;
   return item->icon;
}

EAPI const char *
elm_toolbar_item_label_get(Elm_Toolbar_Item *item)
{
   if (!item) return NULL;
   return item->label;
}

EAPI void 
elm_toolbar_item_label_set(Elm_Toolbar_Item *item, const char *label) 
{
   if (!item) return;
   eina_stringshare_del(item->label);
   item->label = eina_stringshare_add(label);
   edje_object_part_text_set(item->base, "elm.text", item->label);
}

EAPI void
elm_toolbar_item_del(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2 = it->obj;

   if ((!wd) || (!it)) return;
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   if (it->icon) evas_object_del(it->icon);
   evas_object_del(it->base);
   free(it);
   _theme_hook(obj2);
}

EAPI void
elm_toolbar_item_select(Elm_Toolbar_Item *item)
{
   if (!item) return;
   _item_select(item);
}

EAPI Eina_Bool 
elm_toolbar_item_disabled_get(Elm_Toolbar_Item *item) 
{
   if (!item) return EINA_FALSE;
   return item->disabled;
}

EAPI void 
elm_toolbar_item_disabled_set(Elm_Toolbar_Item *item, Eina_Bool disabled) 
{
   if (!item) return;
   _item_disable(item, disabled);
}

EAPI void 
elm_toolbar_item_separator_set(Elm_Toolbar_Item *item, Eina_Bool separator) 
{
   if (!item) return;
   if (item->separator == separator) return;
   item->separator = separator;
   _theme_hook(item->obj);
}

EAPI Eina_Bool 
elm_toolbar_item_separator_get(Elm_Toolbar_Item *item) 
{
   if (!item) return EINA_FALSE;
   return item->separator;
}

EAPI void
elm_toolbar_scrollable_set(Evas_Object *obj, Eina_Bool scrollable)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->scrollable = scrollable;
   _sizing_eval(obj);
}

EAPI void
elm_toolbar_homogenous_set(Evas_Object *obj, Eina_Bool homogenous)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _els_smart_box_homogenous_set(wd->bx, homogenous);
}

