/**
 * selected widget - automatically sets up any widget passed to it as a visual
 * selection
 */
#include <Ewl.h>

#include "selected.h"

/**
 * @param w: the child widget to be selected
 * @return Returns NULL on failure, or a newly allocated selected on success.
 * @brief Allocate and initialize a new selected with given child
 */
Ewl_Widget *
ewler_selected_new(Ewl_Widget *w)
{
	Ewler_Selected *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = NEW(Ewler_Selected, 1);
	if (!s)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewler_selected_init(s, w)) {
		ewl_widget_destroy(EWL_WIDGET(s));
		s = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}

/**
 * @param s: the selected to initialize
 * @param w: the child widget to be selected
 * @return Returns 0 on failure, or non-zero on success
 * @brief initialize a new selected with given child
 */
int
ewler_selected_init(Ewler_Selected *s, Ewl_Widget *w)
{
	Ewl_Widget *sw;
	Ewl_Container *parent;
	int index;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, FALSE);
	DCHECK_PARAM_PTR_RET("w", w, FALSE);

	sw = EWL_WIDGET(s);
	parent = EWL_CONTAINER(w->parent);

	ecore_list_goto(parent->children, w);
	index = ecore_list_index(parent->children);

	if (!ewl_box_init(EWL_BOX(s), EWL_ORIENTATION_VERTICAL))
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	ewl_widget_set_appearance(sw, "entry");

	ewl_container_insert_child(parent, sw, index);
	ewl_container_append_child(EWL_CONTAINER(s), w);
	ewl_object_request_geometry(EWL_OBJECT(s), CURRENT_X(w), CURRENT_Y(w),
															CURRENT_W(w), CURRENT_H(w));
	ewl_object_set_fill_policy(EWL_OBJECT(s), EWL_FLAG_FILL_NONE);

	ewl_callback_append(sw, EWL_CALLBACK_CONFIGURE,
											ewler_selected_configure_cb, NULL);
	ewl_callback_append(w, EWL_CALLBACK_REALIZE,
											ewler_selected_realize_cb, NULL);
	ewl_callback_append(sw, EWL_CALLBACK_DESELECT,
											ewler_selected_deselect_cb, NULL);

	s->index = index;
	s->selected = w;

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param s: the selected item to obtain the widget of
 * @return Returns the currently selected widget
 * @brief Get the widget this selected item refers to
 */
Ewl_Widget *
ewler_selected_get(Ewler_Selected *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", s, NULL);

	DRETURN_PTR(s->selected, DLEVEL_STABLE);
}

void
ewler_selected_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewler_Selected *s;
	int x, y, width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = EWLER_SELECTED(w);
	
	/* the width comes in from the selected, the position is set by the parent */
	ewl_object_get_preferred_size(EWL_OBJECT(s->selected), &width, &height);
	x = CURRENT_X(s);
	y = CURRENT_Y(s);

	if( x != CURRENT_X(s->selected) || y != CURRENT_Y(s->selected) )
		ewl_object_request_position(EWL_OBJECT(s->selected), x, y);
	if( width != CURRENT_W(s) || height != CURRENT_H(s) )
		ewl_object_set_preferred_size(EWL_OBJECT(s), width, height);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewler_selected_realize_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewler_Selected *s;
	int width = 0, height = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = EWLER_SELECTED(w->parent);

	ewl_object_get_preferred_size(EWL_OBJECT(s->selected), &width, &height);

	ewl_object_request_size(EWL_OBJECT(s), width, height);
	ewl_object_set_preferred_size(EWL_OBJECT(s), width, height);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewler_selected_deselect_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewler_Selected *s;
	int x, y, width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = EWLER_SELECTED(w);

	ewl_object_get_current_geometry(EWL_OBJECT(s), &x, &y, &width, &height);
	ewl_container_insert_child(EWL_CONTAINER(w->parent), s->selected, s->index);
	ewl_object_request_geometry(EWL_OBJECT(s->selected), x, y, width, height);
#if 0
	ewl_object_set_preferred_size(EWL_OBJECT(s->selected), width, height);
#endif


	s->selected = NULL;
	s->index = -1;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
