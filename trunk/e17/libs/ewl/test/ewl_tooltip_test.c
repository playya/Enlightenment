#include "ewl_test.h"

Ewl_Widget *tooltip_button;

void __destroy_tooltip_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_widget_destroy(w);
	ewl_callback_append(tooltip_button, EWL_CALLBACK_CLICKED,
			__create_tooltip_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void __create_tooltip_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget     *tooltip_win;
	Ewl_Widget     *tooltip_vbox;
	Ewl_Widget     *button;
	Ewl_Widget     *tooltip;


	ewl_callback_del(w, EWL_CALLBACK_CLICKED, __create_tooltip_test_window);

	tooltip_button = w;

	tooltip_win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(tooltip_win), "Tooltip Test");
	ewl_window_name_set(EWL_WINDOW(tooltip_win), "EWL Test Application");
	ewl_window_class_set(EWL_WINDOW(tooltip_win), "EFL Test Application");
	ewl_object_size_request(EWL_OBJECT(tooltip_win), 200, 100);
	ewl_callback_append(tooltip_win, EWL_CALLBACK_DELETE_WINDOW,
			__destroy_tooltip_test_window, NULL);
	ewl_widget_show(tooltip_win);
	
	tooltip_vbox = ewl_vbox_new();
	ewl_container_append_child(EWL_CONTAINER(tooltip_win), tooltip_vbox);
	ewl_box_set_spacing(EWL_BOX(tooltip_vbox), 0);
	ewl_widget_show(tooltip_vbox);

	button = ewl_button_new ("Hoover on this button");
	ewl_container_append_child(EWL_CONTAINER (tooltip_vbox), button);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_widget_show (button);

	tooltip = ewl_tooltip_new (button);
	ewl_tooltip_set_delay (EWL_TOOLTIP (tooltip), 2.5);
	ewl_container_append_child(EWL_CONTAINER (tooltip_win), tooltip);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
