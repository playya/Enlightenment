#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

void
_sel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Flippicker_Item *it;

   it = event_info;
   printf("label of selected item is: %s\n", elm_flippicker_item_label_get(it));
}

void
_underflow_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("underflow!\n");
}

void
_overflow_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("overflow!\n");
}

void
test_flippicker(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[8];
   unsigned int i;
   Evas_Object *win, *bg, *bx, *fp;
   static const char *lbl[] = {
     "Elementary",
     "Evas",
     "Eina",
     "Edje",
     "Eet",
     "Ecore",
     "Efreet",
     "Edbus"
   };

   win = elm_win_add(NULL, "flippicker", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flippicker");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fp = elm_flippicker_add(win);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(fp, "selected", _sel_cb, NULL);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   for (i = 0; i < sizeof(lbl)/sizeof(char *); i++)
     elm_flippicker_item_append(fp, lbl[i], NULL, NULL);
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   fp = elm_flippicker_add(win);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   for (i = 1990; i < 2099; i++)
     {
	snprintf(buf, 8, "%d", i);
	elm_flippicker_item_append(fp, buf, _sel_cb, NULL);
     }
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   evas_object_show(win);
}
#endif
