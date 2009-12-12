#include "elm_softkey.h"
#include <Ecore_X.h>

#ifndef ELM_LIB_QUICKLAUNCH

static void _cb_win_del(void *data, Evas_Object *obj, void *event);
static void _cb_btn_close_clicked(void *data, Evas_Object *obj, void *event);
static void _cb_btn_back_clicked(void *data, Evas_Object *obj, void *event);

static Evas_Object *win = NULL;

EAPI int 
elm_main(int argc, char **argv) 
{
   Evas_Object *bg, *box, *btn, *icon;
   Ecore_X_Window xwin;
   char buff[PATH_MAX];

   win = elm_win_add(NULL, "elm_softkey", ELM_WIN_DOCK);
   elm_win_title_set(win, "Illume Softkey Window");
   evas_object_smart_callback_add(win, "delete-request", _cb_win_del, NULL);

   xwin = elm_win_xwindow_get(win);
   ecore_x_icccm_hints_set(xwin, 0, 0, 0, 0, 0, 0, 0);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   icon = elm_icon_add(win);
   snprintf(buff, sizeof(buff), "%s/images/back.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon, buff, NULL);
   evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   btn = elm_button_add(win);
   elm_button_icon_set(btn, icon);
   evas_object_smart_callback_add(btn, "clicked", _cb_btn_back_clicked, win);
   evas_object_size_hint_align_set(btn, 1.0, 0.5);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_show(icon);

   icon = elm_icon_add(win);
   snprintf(buff, sizeof(buff), "%s/images/close.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon, buff, NULL);
   evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   btn = elm_button_add(win);
   elm_button_icon_set(btn, icon);
   evas_object_smart_callback_add(btn, "clicked", _cb_btn_close_clicked, win);
   evas_object_size_hint_align_set(btn, 1.0, 0.5);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);
   evas_object_show(icon);

   evas_object_resize(win, 200, 32);
   evas_object_show(win);

   elm_run();
   elm_shutdown();
   return 0;
}

static void 
_cb_win_del(void *data, Evas_Object *obj, void *event) 
{
   elm_exit();
}

static void 
_cb_btn_close_clicked(void *data, Evas_Object *obj, void *event) 
{
   Evas_Object *win;
   Ecore_X_Window xwin;

   win = data;
   xwin = elm_win_xwindow_get(win);
   ecore_x_e_illume_close_send(xwin);
}

static void 
_cb_btn_back_clicked(void *data, Evas_Object *obj, void *event) 
{
   Evas_Object *win;
   Ecore_X_Window xwin;

   win = data;
   xwin = elm_win_xwindow_get(win);
   ecore_x_e_illume_back_send(xwin);
}

#endif
ELM_MAIN();
