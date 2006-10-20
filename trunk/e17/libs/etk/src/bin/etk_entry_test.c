#include "etk_test.h"

static void _etk_test_entry_normal_print_cb(Etk_Object *object, void *data);
static void _etk_test_entry_password_show_cb(Etk_Object *object, void *data);

static Etk_Widget *_label_normal = NULL;
static Etk_Widget *_entry_normal = NULL;

/* Creates the window for the entry test */
void etk_test_entry_window_create(void *data)
{
   static Etk_Widget *win = NULL;
   Etk_Widget *vbox;
   Etk_Widget *frame;
   Etk_Widget *separator;
   Etk_Widget *table;
   Etk_Widget *button;
   Etk_Widget *password_entry;
   
   if (win)
   {
      etk_widget_show_all(ETK_WIDGET(win));
      return;
   }	
  
   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), "Etk Entry Test");
   etk_signal_connect("delete_event", ETK_OBJECT(win), ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(win), vbox);
   
   /* Normal entry */
   frame = etk_frame_new("Normal Entry");
   etk_box_append(ETK_BOX(vbox), frame, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   table = etk_table_new(2, 2, ETK_FALSE);
   etk_container_add(ETK_CONTAINER(frame), table);

   _entry_normal = etk_entry_new();
   etk_entry_text_set(ETK_ENTRY(_entry_normal), "Here is some text");
   etk_table_attach(ETK_TABLE(table), _entry_normal, 0, 0, 0, 0, 0, 0, ETK_TABLE_HEXPAND | ETK_TABLE_HFILL);

   button = etk_button_new_with_label("Print Text");
   etk_table_attach(ETK_TABLE(table), button, 1, 1, 0, 0, 0, 0, ETK_TABLE_NONE);
   etk_signal_connect("clicked", ETK_OBJECT(button), ETK_CALLBACK(_etk_test_entry_normal_print_cb), NULL);
   
   _label_normal = etk_label_new(NULL);
   etk_table_attach(ETK_TABLE(table), _label_normal, 0, 1, 1, 1, 0, 0, ETK_TABLE_HEXPAND | ETK_TABLE_HFILL);
   
   separator = etk_hseparator_new();
   etk_box_append(ETK_BOX(vbox), separator, ETK_BOX_START, ETK_BOX_NONE, 6);
   
   
   /* Password entry */
   frame = etk_frame_new("Password Entry");
   etk_box_append(ETK_BOX(vbox), frame, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(frame), vbox);

   password_entry = etk_entry_new();
   etk_entry_text_set(ETK_ENTRY(password_entry), "Password");
   etk_entry_password_mode_set(ETK_ENTRY(password_entry), ETK_TRUE);
   etk_box_append(ETK_BOX(vbox), password_entry, ETK_BOX_START, ETK_BOX_FILL, 0);

   button = etk_check_button_new_with_label("Password Visible");
   etk_box_append(ETK_BOX(vbox), button, ETK_BOX_START, ETK_BOX_FILL, 0);
   etk_signal_connect("toggled", ETK_OBJECT(button), ETK_CALLBACK(_etk_test_entry_password_show_cb), password_entry);
   
   etk_widget_show_all(win);
}

/* Prints the text of the normal entry in the label */
static void _etk_test_entry_normal_print_cb(Etk_Object *object, void *data)
{
   etk_label_set(ETK_LABEL(_label_normal), etk_entry_text_get(ETK_ENTRY(_entry_normal)));
}

/* Toggles the password mode of the password entry */
static void _etk_test_entry_password_show_cb(Etk_Object *object, void *data)
{
   etk_entry_password_mode_set(ETK_ENTRY(data), !etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(object)));
}
