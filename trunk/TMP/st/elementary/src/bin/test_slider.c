#include <Elementary.h>

void
test_slider(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *sl, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "slider", ELM_WIN_BASIC);
   elm_win_title_set(win, "Slider");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label");
   elm_slider_icon_set(sl, ic);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 120);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label 2");
   elm_slider_span_size_set(sl, 80);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, 1);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, 0.0, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label 3");
   elm_slider_unit_format_set(sl, "units");
   elm_slider_span_size_set(sl, 40);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, 1);
   elm_object_scale_set(sl, 2.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   sl = elm_slider_add(win);
   elm_slider_icon_set(sl, ic);
   elm_slider_label_set(sl, "Label 4");
   elm_slider_unit_format_set(sl, "units");
   elm_slider_span_size_set(sl, 60);
   evas_object_size_hint_align_set(sl, 0.5, -1.0);
   evas_object_size_hint_weight_set(sl, 0.0, 1.0);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, 0.2);
   elm_object_scale_set(sl, 1.0);
   elm_slider_horizontal_set(sl, 0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   evas_object_show(win);
}
