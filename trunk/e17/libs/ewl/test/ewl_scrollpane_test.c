#include "ewl_test.h"

static Ewl_Widget *scrollpane_button;

void
__destroy_scrollpane_test_window(Ewl_Widget * w, void *ev_data,
				 void *user_data)
{
	ewl_widget_destroy(w);

	ewl_callback_append(scrollpane_button, EWL_CALLBACK_CLICKED,
			    __create_scrollpane_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_scrollpane_test_window(Ewl_Widget * w, void *ev_data,
				void *user_data)
{
	Ewl_Widget *scrollpane_win;
	Ewl_Widget *scrollpane;
	Ewl_Widget *button;

	ewl_callback_del(w, EWL_CALLBACK_CLICKED,
			 __create_scrollpane_test_window);

	scrollpane_button = w;

	scrollpane_win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(scrollpane_win), "Scrollpane Test");
	ewl_window_name_set(EWL_WINDOW(scrollpane_win), "EWL Test Application");
	ewl_window_class_set(EWL_WINDOW(scrollpane_win), "EFL Test Application");
	ewl_callback_append(scrollpane_win, EWL_CALLBACK_DELETE_WINDOW,
			    __destroy_scrollpane_test_window, NULL);
	ewl_object_set_minimum_size(EWL_OBJECT(scrollpane_win), 200, 200);
	ewl_widget_show(scrollpane_win);

	scrollpane = ewl_scrollpane_new();
	ewl_container_child_append(EWL_CONTAINER(scrollpane_win), scrollpane);
	ewl_widget_show(scrollpane);

	button = ewl_button_new(NULL);
	ewl_object_set_custom_size(EWL_OBJECT(button), 500, 500);
	ewl_object_set_padding(EWL_OBJECT(button), 20, 20, 20, 20);
	ewl_container_child_append(EWL_CONTAINER(scrollpane), button);
	ewl_widget_show(button);

	return;
	ev_data = NULL;
	user_data = NULL;
}
