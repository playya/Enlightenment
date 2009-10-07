#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void
icon_clicked(void *data, Evas_Object *obj, void *event_info)
{
   printf("clicked!\n");
}

void
test_icon(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "icon-transparent", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Transparent");
   elm_win_autodel_set(win, 1);
   elm_win_alpha_set(win, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", icon_clicked, NULL);
   
   evas_object_show(win);
}
#endif
