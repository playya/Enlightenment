#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
my_ph_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("clicked\n");
}

static void
my_ph_press(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("press\n");
}

static void
my_ph_longpressed(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("longpressed\n");
}

static void
my_ph_clicked_double(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("clicked,double\n");
}

static void
my_ph_load(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("load\n");
}

static void
my_ph_loaded(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("loaded\n");
}

static void
my_ph_load_details(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("load,details\n");
}

static void
my_ph_loaded_details(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("loaded,details\n");
}

static void
my_ph_zoom_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,start\n");
}

static void
my_ph_zoom_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,stop\n");
}

static void
my_ph_zoom_change(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("zoom,change\n");
}

static void
my_ph_anim_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("anim,start\n");
}

static void
my_ph_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("anim,stop\n");
}

static void
my_ph_drag_start(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("drag,start\n");
}

static void
my_ph_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   printf("drag_stop\n");
}

static void
my_ph_scroll(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   int x, y, w, h;
   elm_photocam_region_get(obj, &x, &y, &w, &h);
   printf("scroll %i %i %ix%i\n", x, y, w, h);
}

static void
sel_done(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ph, *iw;

   ph = data;
   iw = evas_object_data_get(ph, "inwin");
   elm_photocam_file_set(ph, elm_fileselector_selected_get(obj));
   evas_object_del(iw);
}

static void
my_bt_open(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ph, *win;
   Evas_Object *iw, *fs;
   
   ph = data;
   win = evas_object_data_get(ph, "window");
   iw = elm_win_inwin_add(win);
   
   fs = elm_fileselector_add(win);
   elm_fileselector_expandable_set(fs, EINA_TRUE);
   elm_fileselector_path_set(fs, getenv("HOME"));
   evas_object_smart_callback_add(fs, "done", sel_done, ph);

   evas_object_data_set(ph, "inwin", iw);
   
   elm_win_inwin_content_set(iw, fs);
   evas_object_show(fs);
   elm_win_inwin_activate(iw);
}

static void
my_bt_show_reg(void *data, Evas_Object *obj, void *event_info)
{
   elm_photocam_image_region_show(data, 30, 50, 500, 300);
}

static void
my_bt_bring_reg(void *data, Evas_Object *obj, void *event_info)

{
   elm_photocam_image_region_bring_in(data, 800, 300, 500, 300);
}

static void
my_bt_zoom_in(void *data, Evas_Object *obj, void *event_info)
{
   double zoom;
   
   zoom = elm_photocam_zoom_get(data);
   zoom -= 0.5;
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom >= (1.0 / 32.0)) elm_photocam_zoom_set(data, zoom);
}

static void
my_bt_zoom_out(void *data, Evas_Object *obj, void *event_info)
{
   double zoom;
   
   zoom = elm_photocam_zoom_get(data);
   zoom += 0.5;
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
   if (zoom <= 256.0) elm_photocam_zoom_set(data, zoom);
}

static void
my_bt_pause(void *data, Evas_Object *obj, void *event_info)
{
   elm_photocam_paused_set(data, !elm_photocam_paused_get(data));
}

static void
my_bt_zoom_fit(void *data, Evas_Object *obj, void *event_info)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
}

static void
my_bt_zoom_fill(void *data, Evas_Object *obj, void *event_info)
{
   elm_photocam_zoom_mode_set(data, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL);
}

void
test_photocam(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ph, *tb2, *bt;
   char buf[PATH_MAX];
   // these were just testing - use the "select photo" browser to select one
   const char *img[5] =
     {
        "/home/raster/t1.jpg",  //   5 mpixel
        "/home/raster/t2.jpg",  //  18 mpixel
        "/home/raster/t3.jpg",  //  39 mpixel
        "/home/raster/t4.jpg",  // 192 mpixel
        "/home/raster/t5.jpg"   // 466 mpixel
     };

   win = elm_win_add(NULL, "photocam", ELM_WIN_BASIC);
   elm_win_title_set(win, "Photocam");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   ph = elm_photocam_add(win);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ph);
   evas_object_data_set(ph, "window", win);
   
   evas_object_smart_callback_add(ph, "clicked", my_ph_clicked, win);
   evas_object_smart_callback_add(ph, "press", my_ph_press, win);
   evas_object_smart_callback_add(ph, "longpressed", my_ph_longpressed, win);
   evas_object_smart_callback_add(ph, "clicked,double", my_ph_clicked_double, win);
   evas_object_smart_callback_add(ph, "load", my_ph_load, win);
   evas_object_smart_callback_add(ph, "loaded", my_ph_loaded, win);
   evas_object_smart_callback_add(ph, "load,details", my_ph_load_details, win);
   evas_object_smart_callback_add(ph, "loaded,details", my_ph_loaded_details, win);
   evas_object_smart_callback_add(ph, "zoom,start", my_ph_zoom_start, win);
   evas_object_smart_callback_add(ph, "zoom,stop", my_ph_zoom_stop, win);
   evas_object_smart_callback_add(ph, "zoom,change", my_ph_zoom_change, win);
   evas_object_smart_callback_add(ph, "scroll,anim,start", my_ph_anim_start, win);
   evas_object_smart_callback_add(ph, "scroll,anim,stop", my_ph_anim_stop, win);
   evas_object_smart_callback_add(ph, "scroll,drag,start", my_ph_drag_start, win);
   evas_object_smart_callback_add(ph, "scroll,drag,stop", my_ph_drag_stop, win);
   evas_object_smart_callback_add(ph, "scroll", my_ph_scroll, win);
   
   elm_photocam_file_set(ph, img[1]);
   
   evas_object_show(ph);
   
   tb2 = elm_table_add(win);
   evas_object_size_hint_weight_set(tb2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb2);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z -");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_out, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.1);
   elm_table_pack(tb2, bt, 0, 0, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Select Photo");
   evas_object_smart_callback_add(bt, "clicked", my_bt_open, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.1);
   elm_table_pack(tb2, bt, 1, 0, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Z +");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_in, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.1);
   elm_table_pack(tb2, bt, 2, 0, 1, 1);
   evas_object_show(bt);


   bt = elm_button_add(win);
   elm_button_label_set(bt, "Show 30,50 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_show_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.5);
   elm_table_pack(tb2, bt, 0, 1, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Bring 800,300 500x300");
   evas_object_smart_callback_add(bt, "clicked", my_bt_bring_reg, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.5);
   elm_table_pack(tb2, bt, 2, 1, 1, 1);
   evas_object_show(bt);
   
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Pause On/Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_pause, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.1, 0.9);
   elm_table_pack(tb2, bt, 0, 2, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Fit");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fit, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.5, 0.9);
   elm_table_pack(tb2, bt, 1, 2, 1, 1);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Fill");
   evas_object_smart_callback_add(bt, "clicked", my_bt_zoom_fill, ph);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, 0.9, 0.9);
   elm_table_pack(tb2, bt, 2, 2, 1, 1);
   evas_object_show(bt);
   
   evas_object_show(tb2);
   
   evas_object_resize(win, 800, 800);
   evas_object_show(win);
}
#endif
