#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @addtogroup Ewl_Password
 * @section text_tut Tutorial
 *
 * The password extends a basic Ewl_Entry to provide an obscured view of the
 * text entered.
 *
 * @code
 * Ewl_Widget *password;
 * password = ewl_password_new();
 * ewl_widget_show(password);
 * @endcode
 *
 * Manipulation of the password widget is generally done through the Ewl_Entry
 * API since the password widget inherits from the entry. The first point it
 * diverges from the entry API is for retrieving the current string in the
 * widget. This is necessary because the entry API returns the text that is
 * hiding the actual contents entered.
 *
 * @code
 * void ewl_password_text_set(Ewl_Password *e, const char *t);
 * char *ewl_password_text_get(Ewl_Password *e);
 * @endcode
 *
 * The character used to hide the text of the password is also configurable at
 * runtime, the default obscuring character is '*'.
 *
 * @code
 * void ewl_password_obscure_set(Ewl_Password *e, char o);
 * char ewl_password_obscure_get(Ewl_Password *e);
 * @endcode
 *
 */

static Ewl_Widget *password[2];

static int create_test(Ewl_Container *win);
static void cb_fetch_password_text(Ewl_Widget *w, void *ev, void *data);
static void cb_set_password_text(Ewl_Widget *w, void *ev, void *data);

void 
test_info(Ewl_Test *test)
{
	test->name = "Password";
	test->tip = "Defines the Ewl_Password class to allow\n"
			"for single line obscured text.";
	test->filename = __FILE__;
	test->func = create_test;
	test->type = EWL_TEST_TYPE_SIMPLE;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *button_hbox, *button[2];

        password[0] = ewl_password_new();
        ewl_password_text_set(EWL_PASSWORD(password[0]), "Play with me ?");
        ewl_object_padding_set(EWL_OBJECT(password[0]), 5, 5, 5, 0);
        ewl_container_child_append(box, password[0]);
        ewl_callback_append(password[0], EWL_CALLBACK_VALUE_CHANGED,
                            cb_fetch_password_text, NULL);
        ewl_object_fill_policy_set(EWL_OBJECT(password[0]),
                                   EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_HSHRINK);
        ewl_widget_show(password[0]);

        password[1] = ewl_password_new();
        ewl_password_text_set(EWL_PASSWORD(password[1]), "E W L ! ! !");
        ewl_object_padding_set(EWL_OBJECT(password[1]), 5, 5, 0, 0);
        ewl_container_child_append(box, password[1]);
        ewl_callback_append(password[1], EWL_CALLBACK_VALUE_CHANGED,
                            cb_fetch_password_text, NULL);
        ewl_object_fill_policy_set(EWL_OBJECT(password[1]),
                                   EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_HSHRINK);
        ewl_widget_show(password[1]);

        button_hbox = ewl_hbox_new();
        ewl_object_alignment_set(EWL_OBJECT(button_hbox), EWL_FLAG_ALIGN_CENTER);
        ewl_container_child_append(box, button_hbox);
        ewl_box_spacing_set(EWL_BOX(button_hbox), 5);
        ewl_widget_show(button_hbox);

        button[0] = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(button[0]), "Fetch text");
        ewl_container_child_append(EWL_CONTAINER(button_hbox), button[0]);
        ewl_callback_append(button[0], EWL_CALLBACK_CLICKED,
                            cb_fetch_password_text, NULL);
        ewl_widget_show(button[0]);

        button[1] = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(button[1]), "Set Text");
        ewl_container_child_append(EWL_CONTAINER(button_hbox), button[1]);
        ewl_callback_append(button[1], EWL_CALLBACK_CLICKED,
                            cb_set_password_text, NULL);
        ewl_widget_show(button[1]);

	return 1;
}

static void
cb_fetch_password_text(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
	                                        void *data __UNUSED__)
{
        char *s;

        s = ewl_password_text_get(EWL_PASSWORD(password[0]));
        printf("First password covers: %s\n", s);
        free(s);

        s = ewl_password_text_get(EWL_PASSWORD(password[1]));
        printf("Second password covers: %s\n", s);
        free(s);
}

static void
cb_set_password_text(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
	                                        void *data __UNUSED__)
{
        ewl_password_text_set(EWL_PASSWORD(password[0]), "Play with me ?");
        ewl_password_text_set(EWL_PASSWORD(password[1]), "E W L ! ! !");
}

