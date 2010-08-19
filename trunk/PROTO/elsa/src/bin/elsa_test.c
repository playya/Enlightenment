#include <Elementary.h>
#include <config.h>

static Evas_Object *
_theme_get(Evas_Object *win, const char *group)
{
   char buffer[PATH_MAX];
   Evas_Object *edje = NULL;

   edje = elm_layout_add(win);
   snprintf(buffer, sizeof(buffer), "%s/themes/default.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(edje, buffer, group);
   return edje;
}

static void
_shutdown(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_shutdown();
}


int
elm_main (int argc, char **argv)
{
   Evas_Object *o, *win;
   win = elm_win_add(NULL, "theme_test", ELM_WIN_BASIC);
   elm_win_title_set(win, PACKAGE);
   evas_object_smart_callback_add(win, "delete_request",
                                  _shutdown, NULL);
   o = _theme_get(win, "elsa");
   evas_object_size_hint_weight_set(o,
                                    EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);
   evas_object_resize(win, 640, 480);
   evas_object_show(win);
   elm_run();
}

ELM_MAIN()
