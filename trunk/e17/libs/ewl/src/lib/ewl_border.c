#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a new border container on success, NULL on failure.
 * @brief Allocate and initialize a new border container
 */
Ewl_Widget     *ewl_border_new(void)
{
	Ewl_Border     *b;

	DENTER_FUNCTION(DLEVEL_STABLE);

	b = NEW(Ewl_Border, 1);
	if (!b) {
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	ewl_border_init(b);

	DRETURN_PTR(EWL_WIDGET(b), DLEVEL_STABLE);
}

/**
 * @param b: the border container to initialize
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize a border container to default values
 */
int ewl_border_init(Ewl_Border * b)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("b", b, FALSE);

	w = EWL_WIDGET(b);

	if (!ewl_box_init(EWL_BOX(w))) {
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}
	ewl_box_orientation_set(EWL_BOX(w), EWL_ORIENTATION_VERTICAL);
	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_FILL);

	ewl_widget_appearance_set(EWL_WIDGET(b), "border");
	ewl_widget_inherit(EWL_WIDGET(b), "border");

	b->label = ewl_text_new();
	ewl_widget_internal_set(b->label, TRUE);
	ewl_container_child_append(EWL_CONTAINER(b), b->label);
	ewl_widget_show(b->label);

	b->body = ewl_vbox_new();
	ewl_widget_internal_set(b->body, TRUE);
	ewl_container_child_append(EWL_CONTAINER(b), b->body);
	ewl_widget_show(b->body);

	b->label_position = EWL_POSITION_LEFT;

	ewl_container_redirect_set(EWL_CONTAINER(b), EWL_CONTAINER(b->body));

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param b: the border widget to change the text
 * @param t: the text to set for the border label
 * @return Returns no value.
 * @brief Set the text for an border label
 *
 * Change the text of the border label to the string @a t.
 */
void ewl_border_text_set(Ewl_Border * b, char *t)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("b", b);
	DCHECK_TYPE("b", b, "border");

	ewl_text_text_set(EWL_TEXT(b->label), t);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param b: the border to retrieve the label text
 * @return Returns the border label text on success, NULL on failure.
 * @brief Get the text label from a border widget
 */
char *ewl_border_text_get(Ewl_Border * b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("b", b, NULL);
	DCHECK_TYPE_RET("b", b, "border", NULL);

	DRETURN_PTR(ewl_text_text_get(EWL_TEXT(b->label)), DLEVEL_STABLE);
}

/**
 * @param b: The Ewl_Border to set the label position on
 * @param pos: The Ewl_Position to set on for the label.
 * @return Returns no value.
 * @brief Sets the position of the lable in the border container
 */
void
ewl_border_label_position_set(Ewl_Border *b, Ewl_Position pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("b", b);
	DCHECK_TYPE("b", b, "border");

	if (b->label_position == pos) {
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}

	b->label_position = pos;
	switch (b->label_position) {
		case EWL_POSITION_LEFT:
		case EWL_POSITION_RIGHT:
			ewl_box_orientation_set(EWL_BOX(b),
						EWL_ORIENTATION_HORIZONTAL);
			break;

		case EWL_POSITION_TOP:
		case EWL_POSITION_BOTTOM:
		default:
			ewl_box_orientation_set(EWL_BOX(b),
						EWL_ORIENTATION_VERTICAL);
			break;
	}
	ewl_container_child_remove(EWL_CONTAINER(b), b->label);

	/* need to remove the redirect so the label gets added back into the
	 * border and not into the body. We put the redirect back on after
	 */
	ewl_container_redirect_set(EWL_CONTAINER(b), NULL);
	if ((b->label_position == EWL_POSITION_LEFT)
			|| (b->label_position == EWL_POSITION_TOP))
		ewl_container_child_prepend(EWL_CONTAINER(b), b->label);
	else
		ewl_container_child_append(EWL_CONTAINER(b), b->label);

	ewl_container_redirect_set(EWL_CONTAINER(b), EWL_CONTAINER(b->body));
	ewl_widget_configure(EWL_WIDGET(b));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param b: The Ewl_Border to get the label position from
 * @return Returns the Ewl_Position setting of the label on this border
 */
Ewl_Position
ewl_border_label_position_get(Ewl_Border *b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("b", b, EWL_POSITION_LEFT);
	DCHECK_TYPE_RET("b", b, "border", EWL_POSITION_LEFT);

	DRETURN_INT(b->label_position, DLEVEL_STABLE);
}

/**
 * @param b: The Ewl_Border to set the alignment on
 * @param align: The alignment to set on the label
 * @return Retruns no value
 * @brief alters the alignment setting of the label on the border
 */
void
ewl_border_label_alignment_set(Ewl_Border *b, unsigned int align)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("b", b);
	DCHECK_TYPE("b", b, "border");

	ewl_object_alignment_set(EWL_OBJECT(b->label), align);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param b: The Ewl_Border to get the alignment from
 * @return Returns the alignment of the label for the border.
 * @brief Retruns the alignment setting of the label for this border container
 */
unsigned int
ewl_border_label_alignment_get(Ewl_Border *b)
{
	unsigned int align;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("b", b, 0);
	DCHECK_TYPE_RET("b", b, "border", 0);

	align = ewl_object_alignment_get(EWL_OBJECT(b->label));

	DRETURN_INT(align, DLEVEL_STABLE);
}


