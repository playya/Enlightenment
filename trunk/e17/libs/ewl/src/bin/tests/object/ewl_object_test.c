/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include <limits.h>
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include "ewl_widget.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * While this set of tests is targeted at objects, it requires setting up full
 * widgets since objects assume they are widgets.
 */

#define MATCH_SIZE 32
#define DIFFER_WIDTH 30
#define DIFFER_HEIGHT 20

static int place_test_center_get(char *buf, int len);
static int place_test_top_get(char *buf, int len);
static int place_test_bottom_get(char *buf, int len);
static int place_test_left_get(char *buf, int len);
static int place_test_right_get(char *buf, int len);
static int position_test_set_get(char *buf, int len);
static int position_size_test_set_get(char *buf, int len);
static int preferred_inner_size_test_set_get(char *buf, int len);
static int preferred_size_test_set_get(char *buf, int len);
static int minimum_size_test_set_get(char *buf, int len);
static int maximum_size_test_set_get(char *buf, int len);
static int minimum_size_test_set_request(char *buf, int len);
static int maximum_size_test_set_request(char *buf, int len);
static int padding_test_set_get(char *buf, int len);
static int insets_test_set_get(char *buf, int len);
static int padding_test_set_size_get(char *buf, int len);
static int insets_test_set_size_get(char *buf, int len);
static int insets_padding_test_set_size_get(char *buf, int len);
static int fill_policy_test_set_get(char *buf, int len);
static int alignment_test_set_get(char *buf, int len);

static Ewl_Unit_Test object_unit_tests[] = {
		{"place center/get", place_test_center_get, NULL, -1, 0},
		{"place top/get", place_test_top_get, NULL, -1, 0},
		{"place bottom/get", place_test_bottom_get, NULL, -1, 0},
		{"place left/get", place_test_left_get, NULL, -1, 0},
		{"place right/get", place_test_right_get, NULL, -1, 0},
		{"position set/get", position_test_set_get, NULL, -1, 0},
		{"position size set/get", position_size_test_set_get, NULL, -1, 0},
		{"preferred inner size set/get", preferred_inner_size_test_set_get, NULL, -1, 0},
		{"preferred size set/get", preferred_size_test_set_get, NULL, -1, 0},
		{"minimum size set/get", minimum_size_test_set_get, NULL, -1, 0},
		{"maximum size set/get", maximum_size_test_set_get, NULL, -1, 0},
		{"minimum size set/request", minimum_size_test_set_request, NULL, -1, 0},
		{"maximum size set/request", maximum_size_test_set_request, NULL, -1, 0},
		{"padding set/get", padding_test_set_get, NULL, -1, 0},
		{"insets set/get", insets_test_set_get, NULL, -1, 0},
		{"padding set/size get", padding_test_set_size_get, NULL, -1, 0},
		{"insets set/size get", insets_test_set_size_get, NULL, -1, 0},
		{"insets padding set/size get", insets_padding_test_set_size_get, NULL, -1, 0},
		{"fill policy set/get", fill_policy_test_set_get, NULL, -1, 0},
		{"alignment set/get", alignment_test_set_get, NULL, -1, 0},
		{NULL, NULL, NULL, -1, 0}
	};

void
test_info(Ewl_Test *test)
{
	test->name = "Object";
	test->tip = "The base object type.";
	test->filename = __FILE__;
	test->type = EWL_TEST_TYPE_MISC;
	test->unit_tests = object_unit_tests;
}

/*
 * Center align an object and check it's positioning and size.
 */
static int
place_test_center_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_set(EWL_OBJECT(w), 10, 20);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_CENTER);
	ewl_object_place(EWL_OBJECT(w), 0, 0, 20, 40);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 5 && y == 10 && width == 10 && height == 20)
			ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect center align placement");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Top align an object and check it's positioning and size.
 */
static int
place_test_top_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_set(EWL_OBJECT(w), 10, 20);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_TOP);
	ewl_object_place(EWL_OBJECT(w), 0, 0, 20, 40);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 5 && y == 0 && width == 10 && height == 20)
			ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect top align placement");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Bottom align an object and check it's positioning and size.
 */
static int
place_test_bottom_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_set(EWL_OBJECT(w), 10, 20);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_BOTTOM);
	ewl_object_place(EWL_OBJECT(w), 0, 0, 20, 40);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 5 && y == 20 && width == 10 && height == 20)
			ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect bottom align placement");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Left align an object and check it's positioning and size.
 */
static int
place_test_left_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_set(EWL_OBJECT(w), 10, 20);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_LEFT);
	ewl_object_place(EWL_OBJECT(w), 0, 0, 20, 40);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 0 && y == 10 && width == 10 && height == 20)
			ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect left align placement");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Right align an object and check it's positioning and size.
 */
static int
place_test_right_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_set(EWL_OBJECT(w), 10, 20);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_RIGHT);
	ewl_object_place(EWL_OBJECT(w), 0, 0, 20, 40);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 10 && y == 10 && width == 10 && height == 20)
			ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect right align placement");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the position and verify that it gets the same position back.
 */
static int
position_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_position_request(EWL_OBJECT(w), 11, 23);

	x = ewl_object_current_x_get(EWL_OBJECT(w));
	y = ewl_object_current_y_get(EWL_OBJECT(w));

	if (x == 11 && y == 23)
		ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect positions returned");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the position and size and verify that it gets the same info back.
 */
static int
position_size_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int x, y, width, height;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_geometry_request(EWL_OBJECT(w), 11, 23, 58, 13);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &x, &y, &width, &height);

	if (x == 11 && y == 23 && width == 58 && height == 13)
		ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect position or size returned");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the preferred inner size and verify that it gets the same size back under
 * various combinations of sizes.
 */
static int
preferred_inner_size_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_inner_size_get(EWL_OBJECT(w), &width, &height);
	if (width == 0 && height == 0) {
		ewl_object_preferred_inner_size_set(EWL_OBJECT(w), MATCH_SIZE,
							MATCH_SIZE);
		ewl_object_preferred_inner_size_get(EWL_OBJECT(w), &width,
							&height);
		if (width == MATCH_SIZE && height == MATCH_SIZE) {
			ewl_object_preferred_inner_size_set(EWL_OBJECT(w),
								DIFFER_WIDTH,
								DIFFER_HEIGHT);
			ewl_object_preferred_inner_size_get(EWL_OBJECT(w),
								&width,
								&height);
			if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "preferred inner sizes match");
		}
		else
			LOG_FAILURE(buf, len, "preferred inner sizes differ");
	}
	else
		LOG_FAILURE(buf, len, "default preferred inner size %dx%d",
			width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the preferred size under a variety of cases and verify that it gets the
 * same sizes back.
 */
static int
preferred_size_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_preferred_size_get(EWL_OBJECT(w), &width, &height);
	if (width == EWL_OBJECT_MIN_SIZE && height == EWL_OBJECT_MIN_SIZE) {
		ewl_object_preferred_inner_size_set(EWL_OBJECT(w), MATCH_SIZE,
							MATCH_SIZE);
		ewl_object_preferred_inner_size_get(EWL_OBJECT(w), &width,
							&height);
		if (width == MATCH_SIZE && height == MATCH_SIZE) {
			ewl_object_preferred_inner_size_set(EWL_OBJECT(w),
					DIFFER_WIDTH,
					DIFFER_HEIGHT);
			ewl_object_preferred_inner_size_get(EWL_OBJECT(w),
								&width,
								&height);
			if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "preferred sizes match");
		}
		else
			LOG_FAILURE(buf, len, "preferred sizes differ");
	}
	else
		LOG_FAILURE(buf, len, "default preferred size %dx%d",
			width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the minimum size and verify that it returns the same minimum size when
 * get is called.
 */
static int
minimum_size_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();
	ewl_object_size_request(EWL_OBJECT(w), MATCH_SIZE - 1,
			MATCH_SIZE - 1);

	ewl_object_minimum_size_get(EWL_OBJECT(w), &width, &height);
	if (width == EWL_OBJECT_MIN_SIZE && height == EWL_OBJECT_MIN_SIZE) {
		ewl_object_minimum_size_set(EWL_OBJECT(w), DIFFER_WIDTH,
							DIFFER_HEIGHT);
		ewl_object_minimum_size_get(EWL_OBJECT(w), &width, &height);
		if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT) {
			ewl_object_minimum_size_set(EWL_OBJECT(w),
					MATCH_SIZE,
					MATCH_SIZE);
			ewl_object_minimum_size_get(EWL_OBJECT(w), &width,
					&height);
			if (width == MATCH_SIZE && height == MATCH_SIZE) {
				ewl_object_current_size_get(EWL_OBJECT(w),
						&width, &height);
				if (width == MATCH_SIZE && height == MATCH_SIZE)
					ret = 1;
				else
					LOG_FAILURE(buf, len,
							"current size wrong");
			}
			else
				LOG_FAILURE(buf, len, "minimum sizes match");
		}
		else
			LOG_FAILURE(buf, len, "minimum sizes differ");
	}
	else
		LOG_FAILURE(buf, len, "default minimum size %dx%d",
			width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the maximum size with a variety of aspects and test that get returns the
 * same sizes set.
 */
static int
maximum_size_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();
	ewl_object_size_request(EWL_OBJECT(w), DIFFER_WIDTH + 1,
			DIFFER_HEIGHT + 1);

	ewl_object_maximum_size_get(EWL_OBJECT(w), &width, &height);
	if (width == EWL_OBJECT_MAX_SIZE && height == EWL_OBJECT_MAX_SIZE) {
		ewl_object_maximum_size_set(EWL_OBJECT(w), MATCH_SIZE,
							MATCH_SIZE);
		ewl_object_maximum_size_get(EWL_OBJECT(w), &width, &height);
		if (width == MATCH_SIZE && height == MATCH_SIZE) {
			ewl_object_maximum_size_set(EWL_OBJECT(w),
					DIFFER_WIDTH,
					DIFFER_HEIGHT);
			ewl_object_maximum_size_get(EWL_OBJECT(w), &width,
					&height);
			if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT) {
				ewl_object_current_size_get(EWL_OBJECT(w),
						&width, &height);
				if (width == DIFFER_WIDTH &&
						height == DIFFER_HEIGHT)
					ret = 1;
				else
					LOG_FAILURE(buf, len,
							"current size wrong");
			}
			else
				LOG_FAILURE(buf, len, "maximum sizes match");
		}
		else
			LOG_FAILURE(buf, len, "maximum sizes differ");
	}
	else
		LOG_FAILURE(buf, len, "default maximum size %dx%d",
			width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the minimum size to different aspects and verify that requesting the size
 * for the object respects the minimum sizes properly.
 */
static int
minimum_size_test_set_request(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	/*
	 * Test the base case of the requested size equal to the minimum.
	 */
	ewl_object_minimum_size_set(EWL_OBJECT(w), DIFFER_WIDTH, DIFFER_HEIGHT);
	ewl_object_size_request(EWL_OBJECT(w), DIFFER_WIDTH, DIFFER_HEIGHT);
	ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);

	if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT) {

		/*
		 * Verify a valid size not equal to the boundary.
		 */
		ewl_object_size_request(EWL_OBJECT(w), DIFFER_WIDTH + 1,
				DIFFER_HEIGHT + 1);
		ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);

		if (width == DIFFER_WIDTH + 1 && height == DIFFER_HEIGHT + 1) {

			/*
			 * Verify an invalid size is forced to the boundary.
			 */
			ewl_object_size_request(EWL_OBJECT(w),
					DIFFER_WIDTH - 1,
					DIFFER_HEIGHT - 1);
			ewl_object_current_size_get(EWL_OBJECT(w), &width,
					&height);
			if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "min size ignored %dx%d",
						width, height);
		}
		else
			LOG_FAILURE(buf, len, "differing sizes wrong %dx%d",
					width, height);
	}
	else
		LOG_FAILURE(buf, len, "same sizes wrong %dx%d", width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that size requests respect the maximum size settings under a variety of
 * cases.
 */
static int
maximum_size_test_set_request(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	/*
	 * Test the base case of the requested size equal to the maximum.
	 */
	ewl_object_maximum_size_set(EWL_OBJECT(w), DIFFER_WIDTH, DIFFER_HEIGHT);
	ewl_object_size_request(EWL_OBJECT(w), DIFFER_WIDTH, DIFFER_HEIGHT);
	ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);

	if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT) {

		/*
		 * Verify a valid size not equal to the boundary.
		 */
		ewl_object_size_request(EWL_OBJECT(w), DIFFER_WIDTH - 1,
				DIFFER_HEIGHT - 1);
		ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);

		if (width == DIFFER_WIDTH - 1 && height == DIFFER_HEIGHT - 1) {

			/*
			 * Verify an invalid size is forced to the boundary.
			 */
			ewl_object_size_request(EWL_OBJECT(w),
					DIFFER_WIDTH + 1,
					DIFFER_HEIGHT + 1);
			ewl_object_current_size_get(EWL_OBJECT(w), &width,
					&height);
			if (width == DIFFER_WIDTH && height == DIFFER_HEIGHT)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "max size ignored %dx%d",
						width, height);
		}
		else
			LOG_FAILURE(buf, len, "differing sizes wrong %dx%d",
					width, height);
	}
	else
		LOG_FAILURE(buf, len, "same sizes wrong %dx%d", width, height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that setting the padding on an object returns the same results on get.
 */
static int
padding_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int l, r, t, b;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_padding_get(EWL_OBJECT(w), &l, &r, &t, &b);
	if (l || r || t || b)
		LOG_FAILURE(buf, len, "initial padding not 0");
	else {
		ewl_object_padding_set(EWL_OBJECT(w), 1, 2, 3, 4);
		ewl_object_padding_get(EWL_OBJECT(w), &l, &r, &t, &b);
		if (l == 1 && r == 2 && t == 3 && b == 4) {
			l = ewl_object_padding_left_get(EWL_OBJECT(w));
			r = ewl_object_padding_right_get(EWL_OBJECT(w));
			t = ewl_object_padding_top_get(EWL_OBJECT(w));
			b = ewl_object_padding_bottom_get(EWL_OBJECT(w));
			if (l == 1 && r == 2 && t == 3 && b == 4)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "incorrect individual");
		}
		else
			LOG_FAILURE(buf, len, "incorrect returned padding");
	}

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that setting the insets on an object returns the same results on get.
 */
static int
insets_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	int l, r, t, b;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_insets_get(EWL_OBJECT(w), &l, &r, &t, &b);
	if (l || r || t || b)
		LOG_FAILURE(buf, len, "initial insets not 0");
	else {
		ewl_object_insets_set(EWL_OBJECT(w), 1, 2, 3, 4);
		ewl_object_insets_get(EWL_OBJECT(w), &l, &r, &t, &b);
		if (l == 1 && r == 2 && t == 3 && b == 4) {
			l = ewl_object_insets_left_get(EWL_OBJECT(w));
			r = ewl_object_insets_right_get(EWL_OBJECT(w));
			t = ewl_object_insets_top_get(EWL_OBJECT(w));
			b = ewl_object_insets_bottom_get(EWL_OBJECT(w));
			if (l == 1 && r == 2 && t == 3 && b == 4)
				ret = 1;
			else
				LOG_FAILURE(buf, len, "incorrect individual");
		}
		else
			LOG_FAILURE(buf, len, "incorrect returned insets");
	}

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that setting the padding on an object returns the expected results after
 * a size request.
 */
static int
padding_test_set_size_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_padding_set(EWL_OBJECT(w), 1, 2, 3, 4);

	ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);
	if (width == (3 + EWL_OBJECT_MIN_SIZE) &&
			height == (7 + EWL_OBJECT_MIN_SIZE))
		ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect returned size %dx%d", width,
				height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that setting the insets on an object returns the expected results after
 * a size request.
 */
static int
insets_test_set_size_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_insets_set(EWL_OBJECT(w), 1, 2, 3, 4);

	ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);
	if (width == 4 && height == 8)
		ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect returned size %dx%d", width,
				height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Test that setting the insets and padding on an object returns the expected
 * results after a size request.
 */
static int
insets_padding_test_set_size_get(char *buf, int len)
{
	Ewl_Widget *w;
	int width = 0, height = 0;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_padding_set(EWL_OBJECT(w), 1, 2, 3, 4);
	ewl_object_insets_set(EWL_OBJECT(w), 4, 3, 2, 1);

	ewl_object_current_size_get(EWL_OBJECT(w), &width, &height);
	if (width == (10 + EWL_OBJECT_MIN_SIZE) &&
			height == (10 + EWL_OBJECT_MIN_SIZE))
		ret = 1;
	else
		LOG_FAILURE(buf, len, "incorrect returned size %dx%d", width,
				height);

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Verify that setting a fill policy on an object returns the proper value when
 * get is called.
 */
static int
fill_policy_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	unsigned int fill;
	int ret = 0;

	w = ewl_widget_new();

	/*
	 * Since fill all should simply be a mask of all other values, this
	 * tests each bit being set.
	 */
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_ALL);
	fill = ewl_object_fill_policy_get(EWL_OBJECT(w));

	if ((fill & EWL_FLAG_FILL_HSHRINK) && (fill & EWL_FLAG_FILL_VSHRINK) &&
	    (fill & EWL_FLAG_FILL_HFILL) && (fill & EWL_FLAG_FILL_VFILL)) {
		ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);
		fill = ewl_object_fill_policy_get(EWL_OBJECT(w));
		if (!fill)
			ret = 1;
		else
			LOG_FAILURE(buf, len, "fill none incorrect");
	}
	else
		LOG_FAILURE(buf, len, "fill all missing flags");

	ewl_widget_destroy(w);

	return ret;
}

/*
 * Set the alignment of an object to a variety of options and check that get
 * returns the correctly set alignment.
 */
static int
alignment_test_set_get(char *buf, int len)
{
	Ewl_Widget *w;
	unsigned int align;
	int ret = 0;

	w = ewl_widget_new();

	ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_LEFT |
			EWL_FLAG_ALIGN_RIGHT | EWL_FLAG_ALIGN_TOP |
			EWL_FLAG_ALIGN_BOTTOM);
	align = ewl_object_alignment_get(EWL_OBJECT(w));

	if ((align & EWL_FLAG_ALIGN_LEFT) && (align & EWL_FLAG_ALIGN_RIGHT) &&
	    (align & EWL_FLAG_ALIGN_TOP) && (align & EWL_FLAG_ALIGN_BOTTOM)) {
		ewl_object_alignment_set(EWL_OBJECT(w), EWL_FLAG_ALIGN_CENTER);
		align = ewl_object_alignment_get(EWL_OBJECT(w));
		if (!align)
			ret = 1;
		else
			LOG_FAILURE(buf, len, "align none incorrect");
	}
	else
		LOG_FAILURE(buf, len, "alignment missing flags");

	ewl_widget_destroy(w);

	return ret;
}
