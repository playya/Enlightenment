#include <Ewl.h>

static Ewl_Widget *button_button = NULL;

void __create_button_test_window(Ewl_Widget * w, void *ev_data,
				 void *user_data);

void
__destroy_button_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_widget_destroy_recursive(w);

	ewl_callback_append(button_button, EWL_CALLBACK_CLICKED,
			    __create_button_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_button_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget *button_win;
	Ewl_Widget *main_vbox;
	Ewl_Widget *separator[2];
	Ewl_Widget *button[2];
	Ewl_Widget *check_button[2];
	Ewl_Widget *radio_button[2];

	ewl_callback_del(w, EWL_CALLBACK_CLICKED,
			 __create_button_test_window);

	button_button = w;

	button_win = ewl_window_new();
	ewl_window_resize(button_win, 145, 230);
	ewl_window_set_min_size(button_win, 145, 230);
	ewl_callback_append(button_win, EWL_CALLBACK_DELETE_WINDOW,
			    __destroy_button_test_window, NULL);
	ewl_widget_show(button_win);

	main_vbox = ewl_vbox_new();
	ewl_container_append_child(EWL_CONTAINER(button_win), main_vbox);
	ewl_box_set_spacing(EWL_BOX(main_vbox), 10);
	ewl_widget_show(main_vbox);

	button[0] = ewl_button_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(main_vbox), button[0]);
	ewl_object_set_alignment(EWL_OBJECT(button[0]), EWL_ALIGNMENT_LEFT);
	ewl_object_set_custom_size(EWL_OBJECT(button[0]), 100, 17);
	ewl_widget_show(button[0]);

	button[1] = ewl_button_new(NULL);
	ewl_container_append_child(EWL_CONTAINER(main_vbox), button[1]);
	ewl_object_set_alignment(EWL_OBJECT(button[1]), EWL_ALIGNMENT_LEFT);
	ewl_object_set_custom_size(EWL_OBJECT(button[1]), 100, 17);
	ewl_widget_show(button[1]);

	separator[0] = ewl_vseparator_new();
	ewl_container_append_child(EWL_CONTAINER(main_vbox), separator[0]);
	ewl_widget_realize(separator[0]);

	ewl_object_set_padding(EWL_OBJECT(separator[0]), 2, 2, 5, 5);

	check_button[0] = ewl_checkbutton_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(main_vbox), check_button[0]);
	ewl_object_set_alignment(EWL_OBJECT(check_button[0]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(check_button[0]);

	check_button[1] = ewl_checkbutton_new(NULL);
	ewl_container_append_child(EWL_CONTAINER(main_vbox), check_button[1]);
	ewl_object_set_alignment(EWL_OBJECT(check_button[1]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(check_button[1]);

	separator[1] = ewl_vseparator_new();
	ewl_container_append_child(EWL_CONTAINER(main_vbox), separator[1]);
	ewl_widget_realize(separator[1]);

	ewl_object_set_padding(EWL_OBJECT(separator[1]), 2, 2, 5, 5);

	radio_button[0] = ewl_radiobutton_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(main_vbox), radio_button[0]);
	ewl_object_set_alignment(EWL_OBJECT(radio_button[0]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(radio_button[0]);

	radio_button[1] = ewl_radiobutton_new(NULL);
	ewl_radiobutton_set_chain(radio_button[1], radio_button[0]);
	ewl_container_append_child(EWL_CONTAINER(main_vbox), radio_button[1]);
	ewl_object_set_alignment(EWL_OBJECT(radio_button[1]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(radio_button[1]);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
