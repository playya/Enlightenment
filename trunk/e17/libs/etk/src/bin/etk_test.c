#include "etk_test.h"

static void _etk_test_main_quit_cb(void *data);

static Etk_Test_Set tests[] =
{
   {
      "Button",
      etk_test_button_window_create
   },
   {
      "Entry",
      etk_test_entry_window_create
   },
   {
      "Table",
      etk_test_table_window_create
   },
   {
      "Image",
      etk_test_image_window_create
   },
   {
      "Scale",
      etk_test_scale_window_create
   },
   {
      "Canvas",
      etk_test_canvas_window_create
   },
   {
      "Colorpicker",
      etk_test_colorpicker_window_create
   },
   {
      "Tree",
      etk_test_tree_window_create
   }
};
static int num_tests = sizeof(tests) / sizeof (tests[0]);

static void _etk_test_main_quit_cb(void *data)
{
   etk_main_quit();
}

static void _etk_test_main_window()
{
   Etk_Widget *win;
   Etk_Widget *table;
   Etk_Widget *button;
   int i;

   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), "Etk Test Application");
   etk_signal_connect("destroy", ETK_OBJECT(win), ETK_CALLBACK(_etk_test_main_quit_cb), NULL);
	
   table = etk_table_new((num_tests + 2) / 3, 3, 1);
   etk_container_add(ETK_CONTAINER(win), table);

   for (i = 0; i < num_tests; i++)
   {
      button = etk_button_new_with_label(tests[i].name);
      etk_signal_connect_swapped("clicked", ETK_OBJECT(button), ETK_CALLBACK(tests[i].func), NULL);
      etk_table_attach_defaults(ETK_TABLE(table), button, i / 3, i / 3, i % 3, i % 3);
   }
   etk_widget_show_all(win);
}


int main(int argc, char *argv[])
{
   if (!etk_init())
   {
      fprintf(stderr, "Could not init etk. Exiting...\n");
      return 0;
   };

   _etk_test_main_window();
   etk_main();
   etk_shutdown();

   return 1;
}
