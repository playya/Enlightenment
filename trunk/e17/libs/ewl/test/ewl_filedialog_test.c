#include "ewl_test.h"

static Ewl_Widget *fd_button = NULL;
static Ewl_Widget *vbox;


void __start_fd (Ewl_Widget *w, void *ev_data, void *user_data);
void __open_file (Ewl_Widget *, void *ev_data, void *user_data);

void
__destroy_filedialog_test_window(Ewl_Widget * w, void *ev_data,
				    void *user_data)
{
	ewl_widget_destroy(w);

	ewl_callback_append(fd_button, EWL_CALLBACK_CLICKED,
			    __create_filedialog_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void
__create_filedialog_test_window(Ewl_Widget * w, void *ev_data,
				   void *user_data)
{
	Ewl_Widget *button;
	Ewl_Widget *fd_win;
	Ewl_Widget *separator;
	
	ewl_callback_del(w, EWL_CALLBACK_CLICKED,
			 __create_filedialog_test_window);

	fd_button = w;

	fd_win = ewl_window_new();
	ewl_callback_append(fd_win, EWL_CALLBACK_DELETE_WINDOW,
			    __destroy_filedialog_test_window, NULL);
	ewl_widget_show(fd_win);

	vbox = ewl_vbox_new ();
	ewl_object_set_fill_policy(EWL_OBJECT(vbox), EWL_FLAG_FILL_FILL);
	ewl_container_append_child(EWL_CONTAINER(fd_win), vbox);
	ewl_widget_show (vbox);

	button = ewl_button_new("Start filedialog");
	ewl_object_set_fill_policy(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_object_set_alignment(EWL_OBJECT(button), EWL_FLAG_ALIGN_LEFT);
	ewl_container_append_child(EWL_CONTAINER(vbox), button);
	ewl_widget_show(button);
	ewl_callback_append(button, EWL_CALLBACK_CLICKED, __start_fd, NULL);

	separator = ewl_hseparator_new ();
	ewl_container_append_child(EWL_CONTAINER(vbox), separator);
	ewl_widget_show (separator);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}


void __start_fd (Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewl_Widget *fd;
	
	fd = ewl_filedialog_new(w->parent->parent, EWL_FILEDIALOG_TYPE_OPEN,
			__open_file);

	ewl_object_set_minimum_size (EWL_OBJECT (w->parent->parent), 400, 300);
	ewl_object_set_fill_policy(EWL_OBJECT(fd), EWL_FLAG_FILL_FILL);
	ewl_container_append_child(EWL_CONTAINER(vbox), fd);
	ewl_widget_show(fd);

	ev_data = NULL;
	user_data = NULL;
}


void __open_file (Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewl_Fileselector *fs;
	Ewl_Table *t;

	fs = EWL_FILESELECTOR(user_data);
	t = EWL_TABLE(fs->files);

	if (t)
		printf("file clicked = %s\n", ewl_table_get_selected(t));

	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
