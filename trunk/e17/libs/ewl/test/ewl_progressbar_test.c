#include "ewl_test.h"

int __increment_progress(void *data);
void __destroy_progressbar_test_window(Ewl_Widget * w, void *ev_data, void *user_data);
void __set_new_range (Ewl_Widget * w, void *ev_data, void *user_data);
void __rerun_progressbars (Ewl_Widget * w, void *ev_data, void *user_data);

static Ewl_Widget *progressbar_button = NULL;
static Ecore_Timer *progress_timer[3];
Ewl_Widget     *progressbar[3];

int __increment_progress(void *data)
{
	double val;
	double value, range;
	char c[30];
	int i;
  Ewl_Progressbar *p = EWL_PROGRESSBAR(data);

	val = ewl_progressbar_get_value(p);

	if (val >= p->range) {
		for (i = 0; i < 3; i++) {
			if (progress_timer[i]) {
				ecore_timer_del(progress_timer[i]);
				progress_timer[i] = NULL;
			}
		}
		return 0;
	}

	val += 1;
	ewl_progressbar_set_value(p, val);


	if (val >= 20 && val < 35 ) {
		ewl_progressbar_set_custom_label (p,
				"%.0lf / %.0lf kbytes");
	}

	if (val >= 35 && val < 60) {
		value = ewl_progressbar_get_value (p);
		range = ewl_progressbar_get_range (p);

		snprintf (c, sizeof (c), "%.0lf of %.0lf beers", value, range);
		ewl_progressbar_set_label (p, c);
	}

	if (val == 60) 
		ewl_progressbar_label_hide (p);

	if (val == 70)
		ewl_progressbar_label_show (p);

	return 1;
}

void __destroy_progressbar_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	int i;

	for (i = 0; i < 3; i++) {
		if (progress_timer[i]) {
			ecore_timer_del(progress_timer[i]);
			progress_timer[i] = NULL;
		}
	}
	ewl_widget_destroy(w);
	ewl_callback_append(progressbar_button, EWL_CALLBACK_CLICKED,
			__create_progressbar_test_window, NULL);

	return;
	ev_data = NULL;
	user_data = NULL;
}

void __set_new_range (Ewl_Widget * w, void *ev_data, void *user_data)
{
	int i;
	int j;

	j = rand() % 500;

	printf ("New random value: %d\n", j);
	
	for (i = 0; i < 3; i++) {
		ewl_progressbar_set_range (EWL_PROGRESSBAR (progressbar[i]), j);
		
		if (ewl_progressbar_get_value (EWL_PROGRESSBAR (progressbar[i])) >= j)
			__rerun_progressbars (EWL_WIDGET (progressbar[i]), NULL, NULL);
	}
	
	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void __rerun_progressbars (Ewl_Widget * w, void *ev_data, void *user_data)
{
	int i;

	for (i = 0; i < 3; i++) {
		/* 
		 * Make sure to autolabel the bar on start again,
		 * if we stop a place where it labels manually.
		 * (since the auto label is turned off when you label manually)
		 */
		ewl_progressbar_label_show (EWL_PROGRESSBAR (progressbar[i]));
		ewl_progressbar_set_value (EWL_PROGRESSBAR (progressbar[i]), 0);
		
		if (progress_timer[i]) {
			ecore_timer_del(progress_timer[i]);
			progress_timer[i] = NULL;
		}

		progress_timer[i] = ecore_timer_add(0.1, __increment_progress,
				        (Ewl_Progressbar *) progressbar[i]);
	}

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}

void __create_progressbar_test_window(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Widget     *progressbar_win;
	Ewl_Widget     *progressbar_box;
	Ewl_Widget     *progressbar_vbox;
	Ewl_Widget     *button;
	int            i;


	ewl_callback_del(w, EWL_CALLBACK_CLICKED, __create_progressbar_test_window);

	progressbar_button = w;

	progressbar_win = ewl_window_new();
	ewl_window_set_title(EWL_WINDOW(progressbar_win), "Progressbar Test");
	ewl_window_set_name(EWL_WINDOW(progressbar_win), "EWL Test Application");
	ewl_window_set_class(EWL_WINDOW(progressbar_win), "EFL Test Application");
	ewl_object_size_request(EWL_OBJECT(progressbar_win), 300, 20);
	ewl_callback_append(progressbar_win, EWL_CALLBACK_DELETE_WINDOW,
			__destroy_progressbar_test_window, NULL);
	ewl_widget_show(progressbar_win);
	
	progressbar_vbox = ewl_vbox_new();
	ewl_container_append_child(EWL_CONTAINER(progressbar_win), progressbar_vbox);
	ewl_box_set_spacing(EWL_BOX(progressbar_vbox), 0);
	ewl_widget_show(progressbar_vbox);

	progressbar_box = ewl_hbox_new();
	ewl_container_append_child(EWL_CONTAINER(progressbar_vbox), 
			progressbar_box);
	ewl_box_set_spacing(EWL_BOX(progressbar_box), 0);
	ewl_widget_show(progressbar_box);

	/*
	 * First and second progressbar 
	 */
	for (i = 0; i < 2; i++) {
		progressbar[i] = ewl_progressbar_new();
		ewl_progressbar_set_value (EWL_PROGRESSBAR(progressbar[i]), 0);
		ewl_widget_show (progressbar[i]);

		progress_timer[i] = ecore_timer_add(0.1, __increment_progress,
				(Ewl_Progressbar *) progressbar[i]);
	
		ewl_container_append_child(EWL_CONTAINER(progressbar_box), progressbar[i]);
	}
	

	/*
	 * Third big progressbar 
	 */
	progressbar[2] = ewl_progressbar_new();
	ewl_progressbar_set_value (EWL_PROGRESSBAR(progressbar[2]), 0);
	ewl_widget_show (progressbar[2]);
	
	progress_timer[2] = ecore_timer_add(0.1, __increment_progress,
			(Ewl_Progressbar *) progressbar[2]);
	
	ewl_container_append_child(EWL_CONTAINER(progressbar_vbox), progressbar[2]);

	
	/*
	 * Add buttons at the bottom
	 */
	progressbar_box = ewl_hbox_new();
	ewl_container_append_child(EWL_CONTAINER(progressbar_vbox),
			progressbar_box);
	ewl_box_set_spacing(EWL_BOX(progressbar_box), 0);
	ewl_widget_show(progressbar_box);
					
	button = ewl_button_new ("Rerun");
	ewl_container_append_child(EWL_CONTAINER(progressbar_box), button);
	ewl_callback_prepend(button, EWL_CALLBACK_CLICKED, 
			__rerun_progressbars, NULL);
		
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_widget_show (button);

	button = ewl_button_new ("Set a random range from 0-500");
	ewl_container_append_child(EWL_CONTAINER(progressbar_box), button);
	ewl_callback_prepend(button, EWL_CALLBACK_CLICKED,
			__set_new_range, NULL);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_widget_show (button);

	return;
	w = NULL;
	ev_data = NULL;
	user_data = NULL;
}
