#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a pointer to a new dialog on success, NULL on failure.
 * @brief Create a new internal dialog
 */
Ewl_Widget *
ewl_dialog_new(void)
{
	Ewl_Dialog *d;

	DENTER_FUNCTION(DLEVEL_STABLE);

	d = NEW(Ewl_Dialog, 1);
	if (!d) {
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	if (!ewl_dialog_init(d)) {
		ewl_widget_destroy(EWL_WIDGET(d));
		d = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(d), DLEVEL_STABLE);
}

/**
 * @param dialog: the dialog to initialize.
 * @return Return TRUE on success, FALSE otherwise.
 * @brief Initialize an internal dialog to starting values
 */
int
ewl_dialog_init(Ewl_Dialog *dialog)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("dialog", dialog, FALSE);

	w = EWL_WIDGET(dialog);

	if (!ewl_window_init(EWL_WINDOW(dialog))) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	ewl_widget_appearance_set(w, "window");
	ewl_widget_inherit(w, "dialog");

	dialog->position = EWL_POSITION_BOTTOM;

	/*
	 * Create a box for laying out the whole window
	 */
	dialog->box = ewl_vbox_new();
	if (!dialog->box) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}
	ewl_container_child_append(EWL_CONTAINER(dialog), dialog->box);
	ewl_widget_internal_set(dialog->box, TRUE);
	ewl_object_fill_policy_set(EWL_OBJECT(dialog->box), EWL_FLAG_FILL_ALL);
	ewl_widget_show(dialog->box);

	/*
	 * Setup a vertical box for the displayed window contents.
	 */
	dialog->vbox = ewl_vbox_new();
	if (!dialog->vbox) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	ewl_container_child_append(EWL_CONTAINER(dialog->box), dialog->vbox);
	ewl_widget_internal_set(dialog->vbox, TRUE);
	ewl_box_homogeneous_set(EWL_BOX(dialog->vbox), FALSE);
	ewl_object_fill_policy_set(EWL_OBJECT(dialog->vbox), EWL_FLAG_FILL_ALL);
	ewl_widget_show(dialog->vbox);

	dialog->separator = ewl_hseparator_new();
	if (!dialog->separator) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}
	ewl_container_child_append(EWL_CONTAINER(dialog->box), dialog->separator);
	ewl_widget_internal_set(dialog->separator, TRUE);
	ewl_widget_show(dialog->separator);

	/*
	 * Create an action area for buttons
	 */
	dialog->action_area = ewl_hbox_new();
	if (!dialog->action_area) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}
	ewl_container_child_append(EWL_CONTAINER(dialog->box),
					   dialog->action_area);
	ewl_object_fill_policy_set(EWL_OBJECT(dialog->action_area),
			   EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_VSHRINK);
	ewl_box_homogeneous_set(EWL_BOX(dialog->action_area), FALSE);
	ewl_widget_internal_set(dialog->action_area, TRUE);
	ewl_widget_show(dialog->action_area);

	ewl_dialog_active_area_set(dialog, dialog->position);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param d: dialog to change action area position
 * @param pos: the new position for the new action area
 * @return Returns no value.
 * @brief Changes the action area position for a dialog.
 */
void
ewl_dialog_action_position_set(Ewl_Dialog *d, Ewl_Position pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("d", d);
	DCHECK_TYPE("d", d, "dialog");

	if (pos == d->position) {
		DRETURN(DLEVEL_STABLE);
	}

	d->position = pos;

	/*
	 * First determine the orientation of the dialog area.
	 */
	if (pos & (EWL_POSITION_LEFT | EWL_POSITION_RIGHT)) {
		ewl_box_orientation_set(EWL_BOX(d->box),
					EWL_ORIENTATION_HORIZONTAL);
		ewl_box_orientation_set(EWL_BOX(d->separator),
					EWL_ORIENTATION_VERTICAL);
		ewl_box_orientation_set(EWL_BOX(d->action_area),
					EWL_ORIENTATION_VERTICAL);
	}
	else {
		ewl_box_orientation_set(EWL_BOX(d->box),
					EWL_ORIENTATION_VERTICAL);
		ewl_box_orientation_set(EWL_BOX(d->separator),
					EWL_ORIENTATION_HORIZONTAL);
		ewl_box_orientation_set(EWL_BOX(d->action_area),
					EWL_ORIENTATION_HORIZONTAL);
	}

	ewl_container_child_remove(EWL_CONTAINER(d->box), d->separator);
	ewl_container_child_remove(EWL_CONTAINER(d->box), d->action_area);

	/*
	 * Repack order of the widgets to match new position
	 */
	if (pos & (EWL_POSITION_LEFT | EWL_POSITION_TOP)) {
		ewl_container_child_prepend(EWL_CONTAINER(d->box),
					    d->separator);
		ewl_container_child_prepend(EWL_CONTAINER(d->box),
					    d->action_area);
	}
	else {
		ewl_container_child_append(EWL_CONTAINER(d->box),
					   d->separator);
		ewl_container_child_append(EWL_CONTAINER(d->box),
					   d->action_area);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param d: dialog to check action area position
 * @return Returns the current action area position.
 * @brief Checks the action area position for a dialog.
 */
Ewl_Position
ewl_dialog_action_position_get(Ewl_Dialog *d)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("d", d, EWL_POSITION_BOTTOM);
	DCHECK_TYPE_RET("d", d, "dialog", EWL_POSITION_BOTTOM);

	DRETURN_INT(d->position, DLEVEL_STABLE);
}

/**
 * @param dialog: the dialog.
 * @return Returns TRUE if @a dialog has a separator.
 * @brief Checks if @a dialog has a separator or not.
 */
unsigned int
ewl_dialog_has_separator_get(Ewl_Dialog *dialog)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("dialog", dialog, 0);
	DCHECK_TYPE_RET("dialog", dialog, "dialog", 0);

	if (!dialog) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	DRETURN_INT(dialog->separator != NULL, DLEVEL_STABLE);
}

/**
 * @param dialog: the dialog.
 * @param has_sep: TRUE to draw the separator, FALSE to hide it.
 * @return Returns no value.
 * @brief Sets the separator of @a dialog.
 */
void
ewl_dialog_has_separator_set(Ewl_Dialog *dialog, unsigned int has_sep)
{
	Ewl_Widget *child;
	int n;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("dialog", dialog);
	DCHECK_TYPE("dialog", dialog, "dialog");

	if (!dialog) {
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}

	if (has_sep && (dialog->separator == NULL)) {
		ewl_container_child_iterate_begin(EWL_CONTAINER(EWL_DIALOG(dialog)->vbox));
		n = 0;
		child = ewl_container_child_next(EWL_CONTAINER(EWL_DIALOG(dialog)->vbox));
		while (child) {
			n++;
			child = ewl_container_child_next(EWL_CONTAINER(EWL_DIALOG(dialog)->vbox));
		}
		dialog->separator = ewl_hseparator_new();
		ewl_container_child_insert(EWL_CONTAINER(dialog->vbox),
					   dialog->separator, n);
		ewl_object_fill_policy_set(EWL_OBJECT(dialog->separator),
					   EWL_FLAG_FILL_SHRINK);
		ewl_widget_show(dialog->separator);

	} else if (!has_sep && (dialog->separator != NULL)) {
		ewl_widget_destroy(dialog->separator);
		dialog->separator = NULL;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_dialog_active_area_set(Ewl_Dialog *d, Ewl_Position pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("d", d);
	DCHECK_TYPE("d", d, "dialog");

	d->active_area = pos;

	if (pos == d->position)
		ewl_container_redirect_set(EWL_CONTAINER(d),
					   EWL_CONTAINER(d->action_area));
	else
		ewl_container_redirect_set(EWL_CONTAINER(d),
					   EWL_CONTAINER(d->vbox));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

Ewl_Position
ewl_dialog_active_area_get(Ewl_Dialog *d)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("d", d, EWL_POSITION_TOP);
	DCHECK_TYPE_RET("d", d, "dialog", EWL_POSITION_TOP);

	DRETURN_INT(d->active_area, DLEVEL_STABLE);
}


