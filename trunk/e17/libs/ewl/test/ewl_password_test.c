#include "ewl_test.h"

static Ewl_Widget *password_button;
static Ewl_Widget *password[2];

void
__destroy_password_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_widget_destroy(w);

	ewl_callback_append(password_button, EWL_CALLBACK_CLICKED,
			    __create_password_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__fetch_password_text(Ewl_Widget * w, void *ev_data, void *user_data)
{
	char           *s;

	s = ewl_password_get_text(EWL_PASSWORD(password[0]));
	printf("First password covers: %s\n", s);
	FREE(s);

	s = ewl_password_get_text(EWL_PASSWORD(password[1]));
	printf("Second password covers: %s\n", s);
	FREE(s);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;

}

void
__set_password_text(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_password_set_text(EWL_PASSWORD(password[0]), "Play with me ?");
	ewl_password_set_text(EWL_PASSWORD(password[1]), "E W L ! ! !");

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_password_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget     *password_win;
	Ewl_Widget     *password_box;
	Ewl_Widget     *button_hbox;
	Ewl_Widget     *button[2];

	ewl_callback_del(w, EWL_CALLBACK_CLICKED, __create_password_test_window);

	password_button = w;

	password_win = ewl_window_new();
	ewl_window_set_title(EWL_WINDOW(password_win), "Password Entry Test");
	ewl_window_set_name(EWL_WINDOW(password_win), "EWL Test Application");
	ewl_window_set_class(EWL_WINDOW(password_win), "EFL Test Application");
	ewl_callback_append(password_win, EWL_CALLBACK_DELETE_WINDOW,
			    __destroy_password_test_window, NULL);
	ewl_widget_show(password_win);

	/*
	 * Create the main box for holding the widgets
	 */
	password_box = ewl_vbox_new();
	ewl_container_append_child(EWL_CONTAINER(password_win), password_box);
	ewl_box_set_spacing(EWL_BOX(password_box), 10);
	ewl_widget_show(password_box);

	password[0] = ewl_password_new("Play with me ?");
	ewl_object_set_padding(EWL_OBJECT(password[0]), 5, 5, 5, 0);
	ewl_container_append_child(EWL_CONTAINER(password_box), password[0]);
	ewl_callback_append(password[0], EWL_CALLBACK_VALUE_CHANGED,
			    __fetch_password_text, NULL);
	ewl_widget_show(password[0]);

	password[1] = ewl_password_new("E W L ! ! !");
	ewl_object_set_padding(EWL_OBJECT(password[1]), 5, 5, 0, 0);
	ewl_container_append_child(EWL_CONTAINER(password_box), password[1]);
	ewl_callback_append(password[1], EWL_CALLBACK_VALUE_CHANGED,
			    __fetch_password_text, NULL);
	ewl_widget_show(password[1]);

	button_hbox = ewl_hbox_new();
	ewl_object_set_alignment(EWL_OBJECT(button_hbox), EWL_FLAG_ALIGN_CENTER);
	ewl_container_append_child(EWL_CONTAINER(password_box), button_hbox);
	ewl_box_set_spacing(EWL_BOX(button_hbox), 5);
	ewl_widget_show(button_hbox);

	button[0] = ewl_button_new("Fetch text");
	ewl_container_append_child(EWL_CONTAINER(button_hbox), button[0]);
	ewl_callback_append(button[0], EWL_CALLBACK_CLICKED,
			    __fetch_password_text, NULL);
	ewl_widget_show(button[0]);

	button[1] = ewl_button_new("Set Text");
	ewl_container_append_child(EWL_CONTAINER(button_hbox), button[1]);
	ewl_callback_append(button[1], EWL_CALLBACK_CLICKED,
			    __set_password_text, NULL);
	ewl_widget_show(button[1]);

	return;
	ev_data = NULL;
	user_data = NULL;
}
