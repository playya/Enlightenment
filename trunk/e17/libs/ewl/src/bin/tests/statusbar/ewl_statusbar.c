/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include "ewl_button.h"
#include "ewl_statusbar.h"

#include <stdio.h>

static int current_num = 0;

static int create_test(Ewl_Container *win);
static void button_push_cb(Ewl_Widget *w, void *ev, void *data);
static void button_pop_cb(Ewl_Widget *w, void *ev, void *data);

void
test_info(Ewl_Test *test)
{
	test->name = "Statusbar";
	test->tip = "Provides a status bar.";
	test->filename = __FILE__;
	test->func = create_test;
	test->type = EWL_TEST_TYPE_SIMPLE;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *statusbar = NULL, *button = NULL;

	statusbar = ewl_statusbar_new();
	ewl_container_child_append(EWL_CONTAINER(box), statusbar);
	ewl_statusbar_left_hide(EWL_STATUSBAR(statusbar));
	ewl_widget_show(statusbar);

	button = ewl_button_new();
	ewl_button_label_set(EWL_BUTTON(button), "push");
	ewl_callback_append(button, EWL_CALLBACK_CLICKED, button_push_cb,
								statusbar);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(box), button);
	ewl_widget_show(button);

	button = ewl_button_new();
	ewl_button_label_set(EWL_BUTTON(button), "pop");
	ewl_callback_append(button, EWL_CALLBACK_CLICKED, button_pop_cb,
								statusbar);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(box), button);
	ewl_widget_show(button);

	return 1;
}

static void
button_push_cb(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
						void *data)
{
	Ewl_Statusbar *sb;
	char msg[20];

	sb = data;

	snprintf(msg, 20, "at %d", current_num++);
	ewl_statusbar_push(sb, msg);
}

static void
button_pop_cb(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
						void *data)
{
	Ewl_Statusbar *sb;

	sb = data;
	ewl_statusbar_pop(sb);
}


