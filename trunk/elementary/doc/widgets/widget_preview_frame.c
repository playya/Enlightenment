#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_frame_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_object_text_set(o, "Frame");

Evas_Object *o2 = elm_label_add(win);
evas_object_size_hint_weight_set(o2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_show(o2);
elm_object_text_set(o2, "Frame content");

elm_frame_content_set(o, o2);

#include "widget_preview_tmpl_foot.c"
