#include <Ewl.h>

static Ewl_Widget *button_button = NULL;

void            __create_button_test_window(Ewl_Widget * w, void *ev_data,
					    void *user_data);

void
__delete_button_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_widget_destroy(w);

	ewl_callback_append(button_button, EWL_CALLBACK_CLICKED,
			    __create_button_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_button_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget     *button_win;
	Ewl_Widget     *button_box;
	Ewl_Widget     *separator[2];
	Ewl_Widget     *button[2];
	Ewl_Widget     *check_button[2];
	Ewl_Widget     *radio_button[2];

	ewl_callback_del(w, EWL_CALLBACK_CLICKED, __create_button_test_window);

	button_button = w;

	button_win = ewl_window_new();
	ewl_callback_append(button_win, EWL_CALLBACK_DELETE_WINDOW,
			    __delete_button_test_window, NULL);
	ewl_widget_show(button_win);

	/*
	 * Create the main box for holding the button widgets
	 */
	button_box = ewl_vbox_new();
	ewl_container_append_child(EWL_CONTAINER(button_win), button_box);
	ewl_box_set_spacing(EWL_BOX(button_box), 10);
	ewl_widget_show(button_box);

	/*
	 * Create a button to be displayed witha label.
	 */
	button[0] = ewl_button_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(button_box), button[0]);
	ewl_object_set_alignment(EWL_OBJECT(button[0]), EWL_ALIGNMENT_LEFT);
	ewl_widget_show(button[0]);

	/*
	 * Create a button that does not contain a label
	 */
	button[1] = ewl_button_new(NULL);
	ewl_container_append_child(EWL_CONTAINER(button_box), button[1]);
	ewl_object_set_alignment(EWL_OBJECT(button[1]), EWL_ALIGNMENT_LEFT);
	ewl_widget_show(button[1]);

	/*
	 * Add a separator between the classic buttons and the check buttons.
	 */
	separator[0] = ewl_hseparator_new();
	ewl_container_append_child(EWL_CONTAINER(button_box), separator[0]);
	ewl_widget_show(separator[0]);

	/*
	 * Create a check button with a label.
	 */
	check_button[0] = ewl_checkbutton_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(button_box), check_button[0]);
	ewl_object_set_alignment(EWL_OBJECT(check_button[0]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(check_button[0]);

	/*
	 * Create a check button w/o a label.
	 */
	check_button[1] = ewl_checkbutton_new(NULL);
	ewl_container_append_child(EWL_CONTAINER(button_box), check_button[1]);
	ewl_object_set_alignment(EWL_OBJECT(check_button[1]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(check_button[1]);

	/*
	 * Add a separator between the check buttons and the radio buttons
	 */
	separator[1] = ewl_hseparator_new();
	ewl_container_append_child(EWL_CONTAINER(button_box), separator[1]);
	ewl_widget_show(separator[1]);

	/*
	 * Add a radio button with
	 */
	radio_button[0] = ewl_radiobutton_new("With Label");
	ewl_container_append_child(EWL_CONTAINER(button_box), radio_button[0]);
	ewl_object_set_alignment(EWL_OBJECT(radio_button[0]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(radio_button[0]);

	radio_button[1] = ewl_radiobutton_new(NULL);
	ewl_radiobutton_set_chain(radio_button[1], radio_button[0]);
	ewl_container_append_child(EWL_CONTAINER(button_box), radio_button[1]);
	ewl_object_set_alignment(EWL_OBJECT(radio_button[1]),
				 EWL_ALIGNMENT_LEFT);
	ewl_widget_show(radio_button[1]);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
