#ifndef EWL_CHECK_H
#define EWL_CHECK_H

/**
 * @addtogroup Ewl_Check Ewl_Check: The Simple Check for a Checkbutton
 * Defines a widget with single purpose of providing a button with a checked
 * state, without any extra decorating.
 *
 * @remarks Inherits from Ewl_Widget.
 * @image html Ewl_Check_inheritance.png
 *
 * @{
 */

/**
 * @themekey /check/file
 * @themekey /check/group
 */

/**
 * @def EWL_CHECK_TYPE
 * The type name for the Ewl_Check widget
 */
#define EWL_CHECK_TYPE "check"

/**
 * A single purpose extension of Ewl_Widget to provide a checked state.
 */
typedef struct Ewl_Check Ewl_Check;

/**
 * @def EWL_CHECK(c)
 * Typecasts a pointer to an Ewl_Check pointer.
 */
#define EWL_CHECK(c) ((Ewl_Check *)c)

/**
 * Inherits from Ewl_Widget and extends it to provide a checked state.
 */
struct Ewl_Check
{
	Ewl_Widget w; /**< Inherit from Ewl_Widget */
	int checked; /**< Indicates if this is checked */
};

Ewl_Widget 	*ewl_check_new(void);
int 		 ewl_check_init(Ewl_Check *c);

int 		 ewl_check_is_checked(Ewl_Check *c);
void 		 ewl_check_checked_set(Ewl_Check *c, int checked);

/*
 * Internally used callbacks.
 */
void ewl_check_cb_clicked(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_check_cb_update_check(Ewl_Widget *w, void *ev_data, void *user_data);

/**
 * @}
 */

#endif

