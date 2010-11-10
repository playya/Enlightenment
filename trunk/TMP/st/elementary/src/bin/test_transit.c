#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _Custom_Effect Custom_Effect;

struct _Custom_Effect
{
   struct _size
     {
        Evas_Coord w, h;
     } from, to;
};

static void _custom_op(void *data, Elm_Transit *transit, double progress)
{
   if (!data) return;
   Evas_Coord w, h;
   Evas_Object *obj;
   const Eina_List *elist;

   Custom_Effect *custom_effect = data;
   const Eina_List *objs = elm_transit_objects_get(transit);

   if (progress < 0.5)
     {
        h = custom_effect->from.h + \
            (Evas_Coord) ((float)custom_effect->to.h * (float)progress * 2);
        w = custom_effect->from.w;
     }
   else
     {
        h = custom_effect->from.h + (Evas_Coord) ((float)custom_effect->to.h);
        w = custom_effect->from.w + (Evas_Coord) ((float)custom_effect->to.w \
                                                * ((float)progress - 0.5) * 2);
     }

   EINA_LIST_FOREACH(objs, elist, obj)
     evas_object_resize(obj, w, h);

}

static void *_custom_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   Custom_Effect *custom_effect;

   custom_effect = calloc(1, sizeof(Custom_Effect));
   if (!custom_effect) return NULL;

   custom_effect->from.w = from_w;
   custom_effect->from.h = from_h;
   custom_effect->to.w = to_w - from_w;
   custom_effect->to.h = to_h - from_h;

   return custom_effect;
}

static void _custom_context_free(void *data, Elm_Transit *transit __UNUSED__)
{
   free(data);
}

/* Translation, Rotation, Color, Wipe, ImagemAnimation Effect */
void
test_transit(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *ic;
   Elm_Transit *trans;
   void *effect_context;
   char buf[PATH_MAX];
   Eina_List *images = NULL;

   win = elm_win_add(NULL, "transit", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_min_set(bx, 318, 318);
   evas_object_show(bx);


   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_11.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "ImageAnimation Effect");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   snprintf(buf, sizeof(buf), "%s/images/icon_19.png", PACKAGE_DATA_DIR);
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/icon_00.png", PACKAGE_DATA_DIR);
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/icon_11.png", PACKAGE_DATA_DIR);
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   images = eina_list_append(images, eina_stringshare_add(buf));

   /* Transit Creation */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, ic);

   /* ImagemAnimation */
   effect_context = \
             elm_transit_effect_image_animation_context_new(images);
   elm_transit_effect_add(trans,
                          elm_transit_effect_image_animation_op, effect_context,
                          elm_transit_effect_image_animation_context_free);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Color, Rotation and Translation");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   /* Transit Creation */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);
   elm_transit_auto_reverse_set(trans, EINA_TRUE);
   elm_transit_repeat_times_set(trans, 2);

   /* Translation Effect */
   effect_context = elm_transit_effect_translation_context_new(0.0, 0.0,
                                                                 280.0, 280.0);
   elm_transit_effect_add(trans,
                          elm_transit_effect_translation_op, effect_context,
                          elm_transit_effect_translation_context_free);

   /* Color Effect */
   effect_context = elm_transit_effect_color_context_new(100, 255, 100,
                                                        255, 200, 50, 200, 50);
   elm_transit_effect_add(trans,
                          elm_transit_effect_color_op, effect_context,
                          elm_transit_effect_color_context_free);

   /* Rotation Effect */
   effect_context = elm_transit_effect_rotation_context_new(0.0,
                                                            135.0, EINA_FALSE);
   elm_transit_effect_add(trans,
                          elm_transit_effect_rotation_op, effect_context,
                          elm_transit_effect_rotation_context_free);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Wipe Effect");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   /* Transit Creation */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);

   /* Wipe Effect */
   effect_context = elm_transit_effect_wipe_context_new(ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE,
                                                        ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT);
   elm_transit_effect_add(trans,
                          elm_transit_effect_wipe_op, effect_context,
                          elm_transit_effect_wipe_context_free);

   evas_object_show(win);
}

/* Resizing Effect */
void
test_transit2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 2");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button - Resizing Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 250, 150);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   effect_context = elm_transit_effect_resizing_context_new(100, 100, 250, 250);
   elm_transit_object_add(trans, bt);
   elm_transit_effect_add(trans,
                          elm_transit_effect_resizing_op, effect_context,
                          elm_transit_effect_resizing_context_free);

}

/* Flip Effect */
void
test_transit3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 3");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Front Button - Flip Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 200, 50);

   bt2 = elm_button_add(win);
   elm_button_label_set(bt2, "Back Button - Flip Effect");
   evas_object_show(bt2);
   evas_object_move(bt2, 50, 100);
   evas_object_resize(bt2, 200, 50);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);
   elm_transit_object_add(trans, bt2);
   effect_context = elm_transit_effect_flip_context_new(ELM_TRANSIT_EFFECT_FLIP_AXIS_X,
                                                        EINA_TRUE);
   elm_transit_effect_add(trans,
                          elm_transit_effect_flip_op, effect_context,
                          elm_transit_effect_flip_context_free);
}

/* Zoom Effect */
void
test_transit4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 4");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_min_set(bx, 318, 318);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Zoom Effect");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);
   effect_context = elm_transit_effect_zoom_context_new(0.5, 5.0);
   elm_transit_effect_add(trans,
                          elm_transit_effect_zoom_op, effect_context,
                          elm_transit_effect_zoom_context_free);


   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
}

/* Blend Effect */
void
test_transit5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *bt2;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 5");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_min_set(bx, 318, 318);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Before Button - Blend Effect");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt2 = elm_button_add(win);
   elm_button_label_set(bt2, "After BUTTON - Blend Effect");
   elm_box_pack_end(bx, bt2);
   evas_object_show(bt2);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);
   elm_transit_object_add(trans, bt2);
   effect_context = elm_transit_effect_blend_context_new();
   elm_transit_effect_add(trans,
                          elm_transit_effect_blend_op, effect_context,
                          elm_transit_effect_blend_context_free);

}

/* Fade Effect */
void
test_transit6(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-6", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 6");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Before Button - Fade Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 200, 50);

   bt2 = elm_button_add(win);
   elm_button_label_set(bt2, "After Button - Fade Effect");
   evas_object_show(bt2);
   evas_object_move(bt2, 50, 100);
   evas_object_resize(bt2, 200, 50);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt);
   elm_transit_object_add(trans, bt2);
   effect_context = elm_transit_effect_fade_context_new();
   elm_transit_effect_add(trans,
                          elm_transit_effect_fade_op, effect_context,
                          elm_transit_effect_fade_context_free);
}

/* Resizable Flip Effect */
void
test_transit7(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-10", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 10");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Front Button");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 250, 150);

   bt2 = elm_button_add(win);
   elm_button_label_set(bt2, "Back Button");
   evas_object_show(bt2);
   evas_object_move(bt2, 50, 100);
   evas_object_resize(bt2, 100, 30);

   evas_object_show(win);

   /* Adding Transit */
   effect_context = \
         elm_transit_effect_resizable_flip_context_new(ELM_TRANSIT_EFFECT_FLIP_AXIS_X,
                                                       EINA_TRUE);
   trans = elm_transit_add(5.0);
   elm_transit_object_add(trans, bt2);
   elm_transit_object_add(trans, bt);
   elm_transit_effect_add(trans,
                          elm_transit_effect_resizable_flip_op, effect_context,
                          elm_transit_effect_resizable_flip_context_free);

}

/* Custom Effect */
void
test_transit8(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt;
   Elm_Transit *trans;
   void *effect_context;

   win = elm_win_add(NULL, "transit-8", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 8");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button - Custom Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 250, 150);

   evas_object_show(win);

   /* Adding Transit */
   trans = elm_transit_add(5.0);
   elm_transit_auto_reverse_set(trans, EINA_TRUE);
   elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   elm_transit_repeat_times_set(trans, -1);
   effect_context = _custom_context_new(100, 100, 250, 250);
   elm_transit_object_add(trans, bt);
   elm_transit_effect_add(trans,
                          _custom_op, effect_context,
                          _custom_context_free);
}

#endif
