#include "etk_test.h"
#include "config.h"

/* Creates the window for the image test */
void etk_test_image_window_create(void *data)
{
   static Etk_Widget *win = NULL;
   Etk_Widget *images[2];
   Etk_Widget *labels[2];
   Etk_Widget *table;

   if (win)
   {
      etk_widget_show_all(ETK_WIDGET(win));
      return;
   }
   
   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), "Etk Image Test");
   etk_signal_connect("delete_event", ETK_OBJECT(win), ETK_CALLBACK(etk_window_hide_on_delete), NULL);
	
   images[0] = etk_image_new_from_file(PACKAGE_DATA_DIR "/images/test.png", NULL);
   etk_image_keep_aspect_set(ETK_IMAGE(images[0]), ETK_TRUE);
   images[1] = etk_image_new_from_file(PACKAGE_DATA_DIR "/images/test.png", NULL);
   etk_image_keep_aspect_set(ETK_IMAGE(images[1]), ETK_FALSE);

   labels[0] = etk_label_new("Keep aspect");
   labels[1] = etk_label_new("Don't keep aspect");
   table = etk_table_new(2, 2, ETK_FALSE);

   etk_table_attach_default(ETK_TABLE(table), images[0], 0, 0, 0, 0);
   etk_table_attach_default(ETK_TABLE(table), images[1], 1, 1, 0, 0);
   etk_table_attach(ETK_TABLE(table), labels[0], 0, 0, 1, 1, 2, 0, ETK_TABLE_HEXPAND);
   etk_table_attach(ETK_TABLE(table), labels[1], 1, 1, 1, 1, 2, 0, ETK_TABLE_HEXPAND);

   etk_container_add(ETK_CONTAINER(win), table);

   etk_widget_show_all(win);
}
