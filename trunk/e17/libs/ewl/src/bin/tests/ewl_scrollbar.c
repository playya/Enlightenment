#include "Ewl_Test.h"
#include <stdio.h>

static int create_test(Ewl_Container *win);

void 
test_info(Ewl_Test *test)
{
	test->name = "Scrollbar";
	test->tip = "A scrollbar.";
	test->filename = "ewl_scrollbar.c";
	test->func = create_test;
	test->type = EWL_TEST_TYPE_SIMPLE;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *hscrollbar, *vscrollbar;

        hscrollbar = ewl_hscrollbar_new();
        ewl_object_padding_set(EWL_OBJECT(hscrollbar), 10, 10, 10, 0);
        ewl_container_child_append(box, hscrollbar);
        ewl_widget_show(hscrollbar);

        vscrollbar = ewl_vscrollbar_new();
        ewl_object_padding_set(EWL_OBJECT(vscrollbar), 10, 10, 10, 10);
        ewl_container_child_append(box, vscrollbar);
        ewl_widget_show(vscrollbar);
	
	return 1;
}

