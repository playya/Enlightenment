#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include <stdio.h>

static int create_test(Ewl_Container *win);
static void cb_print_value(Ewl_Widget *w, void *ev, void *data);

void 
test_info(Ewl_Test *test)
{
	test->name = "Seeker";
	test->tip = "A seeker widget.";
	test->filename = "ewl_seeker.c";
	test->func = create_test;
	test->type = EWL_TEST_TYPE_SIMPLE;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *hseeker, *vseeker;

        hseeker = ewl_hseeker_new();
        ewl_object_position_request(EWL_OBJECT(hseeker), 30, 0);
        ewl_callback_append(hseeker, EWL_CALLBACK_VALUE_CHANGED, 
						cb_print_value, NULL);
        ewl_container_child_append(box, hseeker);
        ewl_widget_show(hseeker);

        vseeker = ewl_vseeker_new();
        ewl_object_position_request(EWL_OBJECT(vseeker), 0, 30);
        ewl_callback_append(vseeker, EWL_CALLBACK_VALUE_CHANGED, 
						cb_print_value, NULL);
        ewl_container_child_append(box, vseeker);
        ewl_widget_show(vseeker);

	return 1;
}

static void
cb_print_value(Ewl_Widget *w, void *ev __UNUSED__, void *data __UNUSED__)
{
	Ewl_Seeker *s;

	s = EWL_SEEKER(w);
	printf("Seeker set to %g\n", ewl_seeker_value_get(s));
}


