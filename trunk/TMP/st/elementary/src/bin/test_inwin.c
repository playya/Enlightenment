#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_inwin(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *inwin, *lb;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "inwin", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "This is an \"inwin\" - a window in a<br>"
		       "window. This is handy for quick popups<br>"
		       "you want centered, taking over the window<br>"
		       "until dismissed somehow. Unlike hovers they<br>"
		       "don't hover over their target.");
   elm_win_inwin_content_set(inwin, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}

void
test_inwin2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *inwin, *lb;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "inwin", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   elm_object_style_set(inwin, "minimal_vertical");
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "This is an \"inwin\" - a window in a<br>"
		       "window. This is handy for quick popups<br>"
		       "you want centered, taking over the window<br>"
		       "until dismissed somehow. Unlike hovers they<br>"
		       "don't hover over their target.<br>"
		       "<br>"
		       "This inwin style compacts itself vertically<br>"
		       "to the size of its contents minimum size.");
   elm_win_inwin_content_set(inwin, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}
#endif
