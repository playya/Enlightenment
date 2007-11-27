/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include <limits.h>
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include "ewl_button.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int append_test_id(char *buf, int len);
static int prepend_test_id(char *buf, int len);
static int insert_after_test_id(char *buf, int len);
static int shared_test_id(char *buf, int len);
static int unique_test_id(char *buf, int len);
static int del_test_call(char *buf, int len);
static int clear_test_call(char *buf, int len);
static int append_test_call(char *buf, int len);
static int prepend_test_call(char *buf, int len);
static int append_in_chain_test_call(char *buf, int len);
static int prepend_in_chain_test_call(char *buf, int len);
static int insert_after_in_chain_test_call(char *buf, int len);
static int insert_before_in_chain_test_call(char *buf, int len);
static int delete_after_in_chain_test_call(char *buf, int len);
static int delete_before_in_chain_test_call(char *buf, int len);
static int delete_nothing_in_chain_test_call(char *buf, int len);

/*
 * Callbacks for manipulating the tests.
 */
static void base_callback(Ewl_Widget *w, void *event, void *data);
static void differing_callback(Ewl_Widget *w, void *event, void *data);
static void append_callback(Ewl_Widget *w, void *event, void *data);
static void prepend_callback(Ewl_Widget *w, void *event, void *data);
static void insert_after_callback(Ewl_Widget *w, void *event, void *data);
static void insert_before_callback(Ewl_Widget *w, void *event, void *data);
static void delete_callback(Ewl_Widget *w, void *event, void *data);

static Ewl_Unit_Test callback_unit_tests[] = {
		{"append/get id", append_test_id, NULL, -1, 0},
		{"prepend/get id", prepend_test_id, NULL, -1, 0},
		{"insert after/get id", insert_after_test_id, NULL, -1, 0},
		{"shared id", shared_test_id, NULL, -1, 0},
		{"unique id", unique_test_id, NULL, -1, 0},
		{"del/call", del_test_call, NULL, -1, 0},
		{"clear/call", clear_test_call, NULL, -1, 0},
		{"append/call", append_test_call, NULL, -1, 0},
		{"prepend/call", prepend_test_call, NULL, -1, 0},
		{"append during call", append_in_chain_test_call, NULL, -1, 0},
		{"prepend during call", prepend_in_chain_test_call, NULL, -1, 0},
		{"insert after during call", insert_after_in_chain_test_call, NULL, -1, 0},
		{"insert before during call", insert_before_in_chain_test_call, NULL, -1, 0},
		{"delete after during call", delete_after_in_chain_test_call, NULL, -1, 0},
		{"delete before during call", delete_before_in_chain_test_call, NULL, -1, 0},
		{"delete nothing during call", delete_nothing_in_chain_test_call, NULL, -1, 0},
		{NULL, NULL, NULL, -1, 0}
	};

void
test_info(Ewl_Test *test)
{
	test->name = "Callback";
	test->tip = "The base callback manipulation.";
	test->filename = __FILE__;
	test->type = EWL_TEST_TYPE_MISC;
	test->unit_tests = callback_unit_tests;
}

/*
 * Append a callback and verify that the returned id is valid.
 */
static int
append_test_id(char *buf, int len)
{
	Ewl_Widget *w;
	int id;
	int ret = 0;

	w = ewl_widget_new();
	id = ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);

	if (id)
		ret = 1;
	else
		snprintf(buf, len, "invalid callback id returned");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Prepend a callback and verify that the returned id is valid.
 */
static int
prepend_test_id(char *buf, int len)
{
	Ewl_Widget *w;
	int id;
	int ret = 0;

	w = ewl_widget_new();
	id = ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);

	if (id)
		ret = 1;
	else
		snprintf(buf, len, "invalid callback id returned");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Insert a callback and verify that the returned id is valid.
 */
static int
insert_after_test_id(char *buf, int len)
{
	Ewl_Widget *w;
	int id;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);
	id = ewl_callback_insert_after(w, EWL_CALLBACK_CONFIGURE, base_callback,
			w, base_callback, NULL);

	if (id)
		ret = 1;
	else
		snprintf(buf, len, "invalid callback id returned");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Append a duplicate callback and verify that the id's match.
 */
static int
shared_test_id(char *buf, int len)
{
	Ewl_Widget *w;
	int id, id2;
	int ret = 0;

	w = ewl_widget_new();
	id = ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);
	id2 = ewl_callback_append(w, EWL_CALLBACK_REALIZE, base_callback,
			NULL);

	if (id == id2)
		ret = 1;
	else
		snprintf(buf, len, "callback id's don't match");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Append a callbacks that vary in the function and data, and verify that the
 * id's differ.
 */
static int
unique_test_id(char *buf, int len)
{
	Ewl_Widget *w;
	int id, id2;
	int ret = 0;

	w = ewl_widget_new();
	id = ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);
	id2 = ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback,
			w);

	if (id != id2) {
		id = ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
				differing_callback, w);
		if (id != id2)
			ret = 1;
		else
			snprintf(buf, len, "callback with different functions"
				       " id's match");
	}
	else
		snprintf(buf, len, "callback with different data id's match");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Prepend a callback and verify that clearing the chain prevents it from being
 * called.
 */
static int
del_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) != 1)
		ret = 1;
	else
		snprintf(buf, len, "del_type failed to remove callback");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Prepend a callback and verify that clearing the chain prevents it from being
 * called.
 */
static int
clear_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);
	ewl_callback_clear(w);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) != 1)
		ret = 1;
	else
		snprintf(buf, len, "clear failed to remove callback");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Append a callback and verify that calling the chain triggers the callback.
 */
static int
append_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 1)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Prepend a callback and verify that calling the chain triggers the callback.
 */
static int
prepend_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 1)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Append a callback while in the callback chain and verify that calling the
 * chain triggers the callback.
 */
static int
append_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, append_callback, NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 1)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Prepend a callback while in the callback chain and verify that calling the
 * chain does not trigger the callback.
 */
static int
prepend_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, prepend_callback, NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) != 1)
		ret = 1;
	else
		snprintf(buf, len, "callback function called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Insert a callback after the current one, while in the callback chain and
 * verify that calling the chain triggers the callback.
 */
static int
insert_after_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, insert_after_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, differing_callback,
			NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 1)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Insert a callback before the current one, while in the callback chain and
 * verify that calling the chain does not trigger the callback.
 */
static int
insert_before_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, insert_before_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, differing_callback,
			NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 2)
		ret = 1;
	else
		snprintf(buf, len, "callback function called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Delete a callback before the current one, while in the callback chain and
 * verify that calling the chain continues properly.
 */
static int
delete_before_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, differing_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, delete_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 2)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Delete a callback after the current one, while in the callback chain and
 * verify that calling the chain does not call the removed callback.
 */
static int
delete_after_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, delete_callback,
			NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, differing_callback,
			NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 2)
		ret = 1;
	else
		snprintf(buf, len, "callback function called");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Delete a non-existent callback, while in the callback chain and
 * verify that calling the chain does not modify anything
 */
static int
delete_nothing_in_chain_test_call(char *buf, int len)
{
	Ewl_Widget *w;
	int ret = 0;

	w = ewl_widget_new();
	ewl_callback_del_type(w, EWL_CALLBACK_CONFIGURE);
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, delete_callback,
			NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, differing_callback,
			NULL);
	ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	if ((long)ewl_widget_data_get(w, w) == 2)
		ret = 1;
	else
		snprintf(buf, len, "callback function not called");

	ewl_widget_destroy(w);

	return ret;
}

static void
base_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_widget_data_set(w, w, (void *)(long)1);
	event = data = NULL;
	return;
}

static void
differing_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_widget_data_set(w, w, (void *)(long)2);
	event = data = NULL;
	return;
}

static void
append_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);

	event = data = NULL;
	return;
}

static void
prepend_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_callback_prepend(w, EWL_CALLBACK_CONFIGURE, base_callback, NULL);

	event = data = NULL;
	return;
}

static void
insert_after_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_callback_insert_after(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL, insert_after_callback, NULL);

	event = data = NULL;
	return;
}

static void
insert_before_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_callback_insert_after(w, EWL_CALLBACK_CONFIGURE, base_callback,
			NULL, differing_callback, NULL);

	event = data = NULL;
	return;
}

static void
delete_callback(Ewl_Widget *w, void *event, void *data)
{
	ewl_callback_del(w, EWL_CALLBACK_CONFIGURE, base_callback);
	event = data = NULL;
	return;
}
