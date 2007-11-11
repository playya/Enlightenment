/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "Ewl_Test.h"
#include "ewl_text_fmt.h"
#include "ewl_text_trigger.h"
#include "ewl_test_private.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @addtogroup Ewl_Text
 * @section text_tut Tutorial
 *
 * The Ewl_Text widget provides for a multi-line text layout widget. It can
 * be utilized whenever the display of text is required in an application.
 * It works well with the Ewl_Scrollpane to provide a scrollable text area.
 *
 * @code
 * Ewl_Widget *text;
 * text = ewl_text_new();
 * ewl_text_text_set(EWL_TEXT(text), "set the text here");
 * ewl_widget_show(text);
 * @endcode
 *
 * Once the text object is created you can change the text, retrieve the
 * current text contents or get the text length. These things can be done
 * with:
 *
 * @code
 * void ewl_text_text_set(Ewl_Text *t, const char *txt);
 * void ewl_text_text_prepend(Ewl_Text *t, const char *txt);
 * void ewl_text_text_append(Ewl_Text *t, const char *txt);
 * void ewl_text_text_insert(Ewl_Text *t, const char *txt, int len);
 * char *ewl_text_text_get(Ewl_Text *t);
 * int ewl_text_length_get(Ewl_Text *t);
 * void ewl_text_clear(Ewl_Text *t);
 * void ewl_text_text_delete(Ewl_Text *t, unsigned int len);
 * @endcode
 *
 * The Ewl_Text widget allows you to perform style changes to the text in
 * the widget. Different portions of the text can be different colours,
 * fonts or styles. You can either set the styling, colours or fonts before
 * the text is set, or you can apply the settings to the text afterwards.
 *
 * The colour settings of the text can be manipulated with the following:
 * @code
 * void ewl_text_color_set(Ewl_Text *t, int r, int g, int b, int a);
 * void ewl_text_color_get(Ewl_Text *t, int *r, int *g, int *b, int *a);
 * void ewl_text_color_apply(Ewl_Text *t, int r, int g, int b, int a, unsigned int len);
 * @endcode
 *
 * There are similar calls to mainipluate the font, font size, font colour,
 * background colour, glow colour, outline colour, strikethrough colour,
 * underline colour, double underline colour, alignment, wrap and style information.
 *
 * Styles have a few extra calls to make them easier to use. These include:
 * @code
 * void ewl_text_style_add(Ewl_Text *t, Ewl_Text_Style style, unsigned int len);
 * void ewl_text_style_del(Ewl_Text *t, Ewl_Text_Style style, unsigned int len);
 * void ewl_text_style_invert(Ewl_Text *t, Ewl_Text_Style style, unsigned int len);
 * unsigned int ewl_text_style_has(Ewl_Text *t, Ewl_Text_Style style, unsigned int idx);
 * @endcode
 *
 * If you want users to be able to select text from the Ewl_Text widget
 * you'll use:
 * @code
 * void ewl_text_selectable_set(Ewl_Text *t, unsigned int selectable);
 * unsigned int ewl_text_selectable_get(Ewl_Text *t);
 * @endcode
 *
 * After the user has made a selection it can be checked and retrieved with:
 * @code
 * unsigned int ewl_text_has_selection(Ewl_Text *t);
 * Ewl_Text_Trigger *ewl_text_selection_get(Ewl_Text *t);
 * char *ewl_text_selection_text_get(Ewl_Text *t);
 * @endcode
 *
 * This should hopefully give you some idea of the capabilities of the
 * Ewl_Text widget. Take a look at the test code and the header file for
 * more information.
 */

static int create_test(Ewl_Container *box);
static void trigger_cb_mouse_out(Ewl_Widget *w, void *ev, void *data);
static void trigger_cb_mouse_in(Ewl_Widget *w, void *ev, void *data);
static void trigger_cb(Ewl_Widget *w, void *ev, void *data);

static int text_test_set_get(char *buf, int len);
static int text_valid_utf8_set_get(char *buf, int len);
static int text_invalid_utf8_set_get(char *buf, int len);

static Ewl_Unit_Test text_unit_tests[] = {
		{"text set/get", text_test_set_get, NULL, -1, 0},
		{"valid UTF-8 text set/get", text_valid_utf8_set_get, NULL, -1, 0},
		{"invalid UTF-8 text set/get", text_invalid_utf8_set_get, NULL, -1, 0},
		{NULL, NULL, NULL, -1, 0}
	};

void
test_info(Ewl_Test *test)
{
	test->name = "Text";
	test->tip = "Defines a class for multi-line text layout\n"
		"and formatting.";
	test->filename = __FILE__;
	test->func = create_test;
	test->type = EWL_TEST_TYPE_SIMPLE;
	test->unit_tests = text_unit_tests;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *o;
	Ewl_Text_Trigger *trigger;
	int len;

	o = ewl_text_new();
	ewl_widget_name_set(o, "text");
	ewl_text_selectable_set(EWL_TEXT(o), TRUE);
	ewl_container_child_append(EWL_CONTAINER(box), o);
	ewl_widget_show(o);

	printf("Insert 'The first bunch of text\\n' [24]\n");
	ewl_text_text_insert(EWL_TEXT(o), "The first bunch of text\n", 0); /* 24 */

	printf("Cursor position\n");
	ewl_text_cursor_position_set(EWL_TEXT(o), 10);

	printf("Colour apply\n");
	ewl_text_color_apply(EWL_TEXT(o), 0, 0, 255, 255, 5);

	printf("Appending 'The second bunch of text\\n' [49]\n");
	ewl_text_text_append(EWL_TEXT(o), "The second bunch of text\n"); /* 25 */

	printf("Font size set\n");
	ewl_text_font_size_set(EWL_TEXT(o), 20);

	printf("Styles set\n");
	ewl_text_styles_set(EWL_TEXT(o), EWL_TEXT_STYLE_DOUBLE_UNDERLINE
						| EWL_TEXT_STYLE_OUTLINE
						| EWL_TEXT_STYLE_SOFT_SHADOW);
	printf("Double underline colour set\n");
	ewl_text_double_underline_color_set(EWL_TEXT(o), 50, 50, 50, 255);

	printf("Shadow colour set\n");
	ewl_text_shadow_color_set(EWL_TEXT(o), 128, 128, 128, 128);

	printf("Outline colour set\n");
	ewl_text_outline_color_set(EWL_TEXT(o), 200, 200, 200, 200);

	printf("Appending 'The third bunch of text\\n' [73]\n");
	ewl_text_text_append(EWL_TEXT(o), "The third bunch of text\n"); /* 24 */

	printf("Inserting 'The fourth bunch of text\\n' [98]\n");
	ewl_text_text_insert(EWL_TEXT(o), "The fourth bunch of text\n", 31); /* 25 */

	printf("Creating trigger [115]\n");
	trigger = EWL_TEXT_TRIGGER(ewl_text_trigger_new(EWL_TEXT_TRIGGER_TYPE_TRIGGER));
	ewl_text_trigger_start_pos_set(trigger, ewl_text_length_get(EWL_TEXT(o)));
	ewl_text_cursor_position_set(EWL_TEXT(o), ewl_text_length_get(EWL_TEXT(o)));
	ewl_text_styles_set(EWL_TEXT(o), EWL_TEXT_STYLE_NONE);
	ewl_text_text_append(EWL_TEXT(o), "This is the link."); /* 17 */

	len = ewl_text_cursor_position_get(EWL_TEXT(o)) -
			ewl_text_trigger_start_pos_get(trigger);
	ewl_text_trigger_length_set(trigger, len);

	ewl_container_child_append(EWL_CONTAINER(o), EWL_WIDGET(trigger));
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_UP,
			trigger_cb, "You clicked the trigger, have a cookie.");
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_IN,
			trigger_cb_mouse_in, NULL);
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_OUT,
			trigger_cb_mouse_out, NULL);

	printf("Inserting 'The fifth bunch of text\\n' [139]\n");
	ewl_text_text_insert(EWL_TEXT(o), "The fifth bunch of text\n", 0); /* 24 */

	printf("Cursor position\n");
	ewl_text_cursor_position_set(EWL_TEXT(o), 0);

	printf("Colour apply\n");
	ewl_text_color_apply(EWL_TEXT(o), 255, 0, 0, 255, 24);

	printf("Inserting 'The sixth bunch of text\\n' [163]\n");
	ewl_text_text_insert(EWL_TEXT(o), "The sixth bunch of text\n", 24); /* 24 */

	printf("Cursor position\n");
	ewl_text_cursor_position_set(EWL_TEXT(o), 43);

	printf("Colour apply\n");
	ewl_text_color_apply(EWL_TEXT(o), 0, 255, 0, 255, 14);

	printf("Cursor position\n");
	ewl_text_cursor_position_set(EWL_TEXT(o), ewl_text_length_get(EWL_TEXT(o)));

	printf("Colour set\n");
	ewl_text_color_set(EWL_TEXT(o), 255, 0, 0, 255);

	printf("Appending 'And in red\\n' [174]\n");
	ewl_text_text_append(EWL_TEXT(o), "And in red\n"); /* 11 */

	printf("Colour set\n");
	ewl_text_color_set(EWL_TEXT(o), 0, 0, 0, 255);

	printf("Appending 'Once more with feeling. ' [198]\n");
	ewl_text_text_append(EWL_TEXT(o), "Once more with feeling. "); /* 24 */

	printf("Trigger\n");
	trigger = EWL_TEXT_TRIGGER(ewl_text_trigger_new(EWL_TEXT_TRIGGER_TYPE_TRIGGER));
	ewl_text_trigger_start_pos_set(trigger, ewl_text_length_get(EWL_TEXT(o)));

	printf("Appending 'This is the multi\\n\\nline link.' [226]\n");
	ewl_text_text_append(EWL_TEXT(o), "This is the multi\n\nline link."); /* 29 */
	len = ewl_text_cursor_position_get(EWL_TEXT(o)) -
			ewl_text_trigger_start_pos_get(trigger);
	ewl_text_trigger_length_set(trigger, len);

	ewl_container_child_append(EWL_CONTAINER(o), EWL_WIDGET(trigger));
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_UP,
			trigger_cb, "You clicked the multi-line trigger, have a coke.");
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_IN,
			trigger_cb_mouse_in, NULL);
	ewl_callback_append(EWL_WIDGET(trigger), EWL_CALLBACK_MOUSE_OUT,
			trigger_cb_mouse_out, NULL);

	printf("Colour set\n");
	ewl_text_color_set(EWL_TEXT(o), 255, 0, 255, 255);

	printf("Appending 'ONE MORE SEGV\\n\\n' [241]\n");
	ewl_text_text_append(EWL_TEXT(o), "ONE MORE SEGV\n\n"); /* 15 */

	printf("Colour set\n");
	ewl_text_color_set(EWL_TEXT(o), 0, 0, 0, 255);

	printf("Appending 'Align Left\\n' [252]\n");
	ewl_text_text_append(EWL_TEXT(o), "Align Left\n");  /* 11 */

	printf("Align set\n");
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_CENTER);

	printf("Appending 'Align Center.\\n' [266]\n");
	ewl_text_text_append(EWL_TEXT(o), "Align Center.\n");  /* 14 */

	printf("Align set\n");
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_RIGHT);

	printf("Appending 'Align Right.\\n' [279]\n");
	ewl_text_text_append(EWL_TEXT(o), "Align Right.\n");  /* 13 */

	return 1;
}

static void
trigger_cb(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, void *data)
{
	printf("%s\n", (char *)data);
}

static void
trigger_cb_mouse_in(Ewl_Widget *w, void *ev __UNUSED__,
					void *data __UNUSED__)
{
	Ewl_Text_Trigger *t;

	t = EWL_TEXT_TRIGGER(w);

	ewl_text_cursor_position_set(EWL_TEXT(t->text_parent), t->char_pos);
	ewl_text_color_apply(EWL_TEXT(t->text_parent), 255, 0, 0, 255, t->char_len);
}

static void
trigger_cb_mouse_out(Ewl_Widget *w, void *ev __UNUSED__,
					void *data __UNUSED__)
{
	Ewl_Text_Trigger *t;

	t = EWL_TEXT_TRIGGER(w);

	ewl_text_cursor_position_set(EWL_TEXT(t->text_parent), t->char_pos);
	ewl_text_color_apply(EWL_TEXT(t->text_parent), 0, 0, 0, 255, t->char_len);
}

static int
text_test_set_get(char *buf, int len)
{
	Ewl_Widget *o;
	char *t;
	int ret = 0;

	o = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(o), "This is the test text.");
	t = ewl_text_text_get(EWL_TEXT(o));

	if (strcmp(t, "This is the test text."))
		snprintf(buf, len, "text_get did not match text_set.");
	else
		ret = 1;

	return ret;
}

static int
text_valid_utf8_set_get(char *buf, int len)
{
	Ewl_Widget *t;
	char text[] = "This a valid UTF-8 string, although it contains "
			"at the moment only 7-bit ascii code.\n"
			" This will be changed later!";
	const char * returned_text;
	unsigned char *utext;

	utext = (unsigned char *)text;

	t = ewl_text_new();
	ewl_widget_show(t);

	/*
	 * append the US-ASCII string
	 */
	ewl_text_text_set(EWL_TEXT(t), text);
	returned_text = ewl_text_text_get(EWL_TEXT(t));

	if (strcmp(text, returned_text)) {
		snprintf(buf, len, "Incorrect UTF-8 validation during"
				"setting a US-ASCII string");
		return FALSE;
	}

	ewl_text_clear(EWL_TEXT(t));

	/* insert the umlaut a with two dots */
	utext[2] = 0xC3;
	utext[3] = 0xA4;
	/* insert the euro sign */
	utext[10] = 0xE2;
	utext[11] = 0x82;
	utext[12] = 0xAC;
	/*
	 * set a valid UTF-8 string
	 */
	ewl_text_text_set(EWL_TEXT(t), text);
	returned_text = ewl_text_text_get(EWL_TEXT(t));

	if (strcmp(text, returned_text)) {
		snprintf(buf, len, "Incorrect UTF-8 validation during"
				"setting a valid UTF-8 string");
		return FALSE;
	}
	return TRUE;
}


static int
text_invalid_utf8_set_get(char *buf, int len)
{
	Ewl_Widget *t;
	char text[] = "This a valid UTF-8 string, although it contains "
			"at the moment only 7-bit ascii code.\n"
			" This will be changed later!";
	const char * returned_text;
	unsigned char *utext;

	utext = (unsigned char *)text;

	t = ewl_text_new();
	ewl_widget_show(t);

	/* insert some invalid bytes */
	utext[2] = 254;
	utext[4] = 0xA4;
	/* insert the euro sign without the 3rd byte */
	utext[10] = 0xE2;
	utext[11] = 0x82;
	/*
	 * set a valid UTF-8 string
	 */
	ewl_text_text_set(EWL_TEXT(t), text);
	returned_text = ewl_text_text_get(EWL_TEXT(t));

	if (!returned_text) {
		snprintf(buf, len, "ewl_text_text_get() returned a "
				"NULL-pointer");
	}
	else if (!strcmp(text, returned_text)
		|| returned_text[2] < 0
		|| returned_text[4] < 0
		|| returned_text[10] < 0
		|| returned_text[11] < 0)
	{
		snprintf(buf, len, "Incorrect UTF-8 validation during"
				"setting a invalid UTF-8 string");
		return FALSE;
	}
	return TRUE;
}

