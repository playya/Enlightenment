#include "etk_test.h"
#include "config.h"

static Etk_Widget *_etk_test_notebook_page1_widget_create();
static Etk_Widget *_etk_test_notebook_page2_widget_create();
static void _etk_test_notebook_hide_tabs_toggled_cb(Etk_Object *object, void *data);

/* Creates the window for the notebook test */
void etk_test_notebook_window_create(void *data)
{
   static Etk_Widget *win = NULL;
   Etk_Widget *notebook;
   Etk_Widget *page_widget;
   Etk_Widget *alignment;
   Etk_Widget *hbox;
   Etk_Widget *vbox;
   Etk_Widget *button;
   
   if (win)
   {
      etk_widget_show_all(ETK_WIDGET(win));
      return;
   }
   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), "Etk Notebook Test");
   etk_container_border_width_set(ETK_CONTAINER(win), 5);
   etk_signal_connect("delete_event", ETK_OBJECT(win), ETK_CALLBACK(etk_window_hide_on_delete), NULL);	
   
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(win), vbox);

   /* Create the notebook */
   notebook = etk_notebook_new();
   etk_box_append(ETK_BOX(vbox), notebook, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   /* Add the page */
   page_widget = _etk_test_notebook_page1_widget_create();
   etk_notebook_page_append(ETK_NOTEBOOK(notebook), "Tab 1 - Table test", page_widget);
   page_widget = _etk_test_notebook_page2_widget_create();
   etk_notebook_page_append(ETK_NOTEBOOK(notebook), "Tab 2 - Button test", page_widget);
   
   etk_box_append(ETK_BOX(vbox), etk_hseparator_new(), ETK_BOX_START, ETK_BOX_NONE, 5);
   
   /* Create the prev/next buttons */
   alignment = etk_alignment_new(0.5, 0.5, 0.0, 1.0);
   etk_box_append(ETK_BOX(vbox), alignment, ETK_BOX_START, ETK_BOX_NONE, 0);
   hbox = etk_hbox_new(ETK_TRUE, 0);
   etk_container_add(ETK_CONTAINER(alignment), hbox);
   
   button = etk_button_new_from_stock(ETK_STOCK_GO_PREVIOUS);
   etk_button_label_set(ETK_BUTTON(button), "Previous");
   etk_signal_connect_swapped("clicked", ETK_OBJECT(button), ETK_CALLBACK(etk_notebook_page_prev), notebook);
   etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_FILL, 0);
   
   button = etk_button_new_from_stock(ETK_STOCK_GO_NEXT);
   etk_button_label_set(ETK_BUTTON(button), "Next");
   etk_signal_connect_swapped("clicked", ETK_OBJECT(button), ETK_CALLBACK(etk_notebook_page_next), notebook);
   etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_FILL, 0);
   
   /* Create the "Hide tabs" toggle button */
   button = etk_toggle_button_new_with_label("Hide tabs");
   etk_signal_connect("toggled", ETK_OBJECT(button), ETK_CALLBACK(_etk_test_notebook_hide_tabs_toggled_cb), notebook);
   etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_FILL, 0);

   etk_widget_show_all(win);
}

/* Creates the widget for the first page */
static Etk_Widget *_etk_test_notebook_page1_widget_create()
{
   Etk_Widget *table;
   Etk_Widget *image;
   Etk_Widget *buttons[3];
   Etk_Widget *labels[8];
   Etk_Widget *entries[6];
   int i;
   
   image = etk_image_new_from_file(PACKAGE_DATA_DIR "/images/test.png");
   
   buttons[0] = etk_button_new_from_stock(ETK_STOCK_DOCUMENT_OPEN);
   buttons[1] = etk_check_button_new();
   buttons[2] = etk_check_button_new();
   
   labels[0] = etk_label_new("App Name");
   labels[1] = etk_label_new("Generic Info");
   labels[2] = etk_label_new("Comments");
   labels[3] = etk_label_new("Executable");
   labels[4] = etk_label_new("Window Name");
   labels[5] = etk_label_new("Window Class");
   labels[6] = etk_label_new("Startup Notify");
   labels[7] = etk_label_new("Wait Exit");
   
   for (i = 0; i < 6; i++)
      entries[i] = etk_entry_new();
   

   table = etk_table_new(2, 10, ETK_FALSE);
   etk_table_attach(ETK_TABLE(table), image, 0, 0, 0, 0, 0, 0, ETK_FILL_POLICY_NONE);
   etk_table_attach(ETK_TABLE(table), buttons[0], 1, 1, 0, 0, 0, 0, ETK_FILL_POLICY_HEXPAND);
   
   for (i = 0; i < 6; i++)
   {
      etk_table_attach(ETK_TABLE(table), labels[i], 0, 0, 2 + i, 2 + i, 0, 0, ETK_FILL_POLICY_HFILL);
      etk_table_attach_defaults(ETK_TABLE(table), entries[i], 1, 1, 2 + i, 2 + i);
   }
   
   etk_table_attach(ETK_TABLE(table), labels[6], 0, 0, 8, 8, 0, 0, ETK_FILL_POLICY_HFILL);
   etk_table_attach_defaults(ETK_TABLE(table), buttons[1], 1, 1, 8, 8);
   etk_table_attach(ETK_TABLE(table), labels[7], 0, 0, 9, 9, 0, 0, ETK_FILL_POLICY_HFILL);
   etk_table_attach_defaults(ETK_TABLE(table), buttons[2], 1, 1, 9, 9);
   
   
   return table;
}

/* Creates the widget for the second page */
static Etk_Widget *_etk_test_notebook_page2_widget_create()
{
   Etk_Widget *alignment;
   Etk_Widget *vbox;
   Etk_Widget *button_normal;
   Etk_Widget *button_toggle;
   Etk_Widget *button_check;
   Etk_Widget *button_radio;
   
   alignment = etk_alignment_new(0.5, 0.5, 0.2, 0.0);
   
   vbox = etk_vbox_new(ETK_FALSE, 3);
   etk_container_add(ETK_CONTAINER(alignment), vbox);

   button_normal = etk_button_new_with_label("Normal button");
   etk_box_append(ETK_BOX(vbox), button_normal, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   button_toggle = etk_toggle_button_new_with_label("Toggle button");
   etk_box_append(ETK_BOX(vbox), button_toggle, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   button_check = etk_check_button_new_with_label("Check button");
   etk_box_append(ETK_BOX(vbox), button_check, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   button_check = etk_check_button_new();
   etk_box_append(ETK_BOX(vbox), button_check, ETK_BOX_START, ETK_BOX_NONE, 0);

   button_radio = etk_radio_button_new_with_label("Radio button", NULL);
   etk_box_append(ETK_BOX(vbox), button_radio, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   button_radio = etk_radio_button_new_from_widget(ETK_RADIO_BUTTON(button_radio));
   etk_box_append(ETK_BOX(vbox), button_radio, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   return alignment;
}

/* Shows/hides the tab bar when the "hide tabs" button is toggled */
static void _etk_test_notebook_hide_tabs_toggled_cb(Etk_Object *object, void *data)
{
   Etk_Bool state;
   
   state = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object));
   etk_notebook_tabs_visible_set(ETK_NOTEBOOK(data), !state);
}
