/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include <stdio.h>

static int create_test(Ewl_Container *win);

void 
test_info(Ewl_Test *test)
{
	test->name = "Datepicker";
	test->tip = "Defines a datepicker widget.";
	test->filename = __FILE__;
	test->func = create_test;
	test->type = EWL_TEST_TYPE_MISC;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *datepicker;

	datepicker = ewl_datepicker_new();
	ewl_container_child_append(EWL_CONTAINER(box), datepicker);
	ewl_widget_show(datepicker);

	return 1;
}

