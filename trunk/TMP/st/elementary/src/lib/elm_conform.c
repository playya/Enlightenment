#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data 
{
   Evas_Object *base;
   Evas_Object *shelf, *panel;
   Evas_Object *content;
};

/* local function prototypes */
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

/* local functions */
static void 
_del_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void 
_theme_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   _elm_theme_set(wd->base, "conformant", "base", elm_widget_style_get(obj));
   if (wd->content)
     edje_object_part_swallow(wd->base, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void 
_sizing_eval(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw = -1, mh = -1;

   edje_object_size_min_calc(wd->base, &mw, &mh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EAPI Evas_Object *
elm_conformant_add(Evas_Object *parent) 
{
   Evas_Object *obj;
   Evas *evas;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   evas = evas_object_evas_get(parent);
   obj = elm_widget_add(evas);
   elm_widget_type_set(obj, "conformant");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->base = edje_object_add(evas);
   _elm_theme_set(wd->base, "conformant", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

   // FIXME: broken. doesnt handle property changes, keyboard popup or not
   // etc. doesnt gettop helf geometry either.
#ifdef HAVE_ELEMENTARY_X
   int sh = -1;

   ecore_x_e_illume_top_shelf_geometry_get(ecore_x_window_root_first_get(), 
                                           NULL, NULL, NULL, &sh);
   printf("sh0: %i\n", sh);
   if (sh < 1) sh = 1;

   wd->shelf = evas_object_rectangle_add(evas);
   evas_object_color_set(wd->shelf, 0, 0, 0, 0);
   evas_object_size_hint_min_set(wd->shelf, 0, sh);
   evas_object_size_hint_max_set(wd->shelf, -1, -1);
   edje_object_part_swallow(wd->base, "elm.swallow.shelf", wd->shelf);

   sh = -1;
   ecore_x_e_illume_bottom_panel_geometry_get(ecore_x_window_root_first_get(), 
                                              NULL, NULL, NULL, &sh);
   printf("sh1: %i\n", sh);
   if (sh < 1) sh = 1;

   wd->panel = evas_object_rectangle_add(evas);
   evas_object_color_set(wd->panel, 0, 0, 0, 0);
   evas_object_size_hint_min_set(wd->panel, 0, sh);
   evas_object_size_hint_max_set(wd->panel, -1, -1);
   edje_object_part_swallow(wd->base, "elm.swallow.panel", wd->panel);
#endif

   _sizing_eval(obj);
   return obj;
}

EAPI void 
elm_conformant_content_set(Evas_Object *obj, Evas_Object *content) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content) 
     {
        elm_widget_sub_object_add(obj, content);
        edje_object_part_swallow(wd->base, "elm.swallow.content", content);
        _sizing_eval(obj);
     }
}
