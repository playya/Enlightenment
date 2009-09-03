#include <Elementary.h>

void
my_bt_go_300_300(void *data, Evas_Object *obj, void *event_info)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 300, 318, 318);
}

void
my_bt_go_900_300(void *data, Evas_Object *obj, void *event_info)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 300, 318, 318);
}

void
my_bt_go_300_900(void *data, Evas_Object *obj, void *event_info)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 300, 900, 318, 318);
}

void
my_bt_go_900_900(void *data, Evas_Object *obj, void *event_info)
{
   elm_scroller_region_bring_in((Evas_Object *)data, 900, 900, 318, 318);
}

void
test_scroller(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg2, *tb, *tb2, *bg, *sc, *bt;
   int i, j, n;
   char buf[PATH_MAX];
   const char *img[9] =
     {
        "panel_01.jpg", 
          "plant_01.jpg", 
          "rock_01.jpg", 
          "rock_02.jpg",
          "sky_01.jpg", 
          "sky_02.jpg", 
          "sky_03.jpg", 
          "sky_04.jpg",
          "wood_01.jpg"
     };

   win = elm_win_add(NULL, "scroller", ELM_WIN_BASIC);
   elm_win_title_set(win, "Scroller");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);

   n = 0;
   for (j = 0; j < 12; j++)
     {
        for (i = 0; i < 12; i++)
          {
             bg2 = elm_bg_add(win);
             snprintf(buf, sizeof(buf), "%s/images/%s", 
                      PACKAGE_DATA_DIR, img[n]);
             n++;
             if (n >= 9) n = 0;
             elm_bg_file_set(bg2, buf, NULL);
             evas_object_size_hint_weight_set(bg2, 1.0, 1.0);
             evas_object_size_hint_align_set(bg2, -1.0, -1.0);
             evas_object_size_hint_min_set(bg2, 318, 318);
             elm_table_pack(tb, bg2, i, j, 1, 1);
             evas_object_show(bg2);
          }
     }
   
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   elm_win_resize_object_add(win, sc);

   elm_scroller_content_set(sc, tb);
   evas_object_show(tb);

   elm_scroller_page_relative_set(sc, 1.0, 1.0);
//   elm_scroller_page_size_set(sc, 200, 200);
   evas_object_show(sc);

   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, 1.0, 1.0);
   elm_win_resize_object_add(win, tb2);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "to 300 300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_300_300, sc);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "to 900 300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_900_300, sc);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 1, 0, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "to 300 900");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_300_900, sc);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.1, 0.9);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "to 900 900");
   evas_object_smart_callback_add(bt, "clicked", my_bt_go_900_900, sc);
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_table_pack(tb2, bt, 1, 1, 1, 1);
   evas_object_show(bt);
   
   evas_object_show(tb2);
   
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
