#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a newly allocated cell on success, NULL on failure.
 * @brief Allocate and initialize a new cell
 */
Ewl_Widget *ewl_cell_new()
{
	Ewl_Widget *cell;

	DENTER_FUNCTION(DLEVEL_STABLE);

	cell = NEW(Ewl_Cell, 1);
	if (!cell)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_cell_init(EWL_CELL(cell))) {
		FREE(cell);
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	DRETURN_PTR(cell, DLEVEL_STABLE);
}

/**
 * @param cell: the cell object to initialize
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize the cell fields of an inheriting object
 *
 * The fields of the @a cell object are initialized to their defaults.
 */
int ewl_cell_init(Ewl_Cell *cell)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("cell", cell, FALSE);

	if (!ewl_container_init(EWL_CONTAINER(cell), "cell"))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_container_show_notify_set(EWL_CONTAINER(cell), ewl_cell_child_show_cb);
	ewl_container_resize_notify_set(EWL_CONTAINER(cell),
				    ewl_cell_child_resize_cb);
	ewl_callback_append(EWL_WIDGET(cell), EWL_CALLBACK_CONFIGURE,
			    ewl_cell_configure_cb, NULL);

	ewl_widget_inherit(EWL_WIDGET(cell), "cell");

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

void
ewl_cell_configure_cb(Ewl_Widget * w, void *ev_data __UNUSED__, 
					void *user_data __UNUSED__)
{
	Ewl_Cell *cell;
	Ewl_Container *c;
	Ewl_Object *child;

	DENTER_FUNCTION(DLEVEL_STABLE);

	cell = EWL_CELL(w);
	c = EWL_CONTAINER(w);

	child = ecore_list_goto_first(c->children);

	if (child)
		ewl_object_place(child, CURRENT_X(w), CURRENT_Y(w),
				CURRENT_W(w), CURRENT_H(w));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_cell_child_show_cb(Ewl_Container *c, Ewl_Widget *w)
{
	Ewl_Widget *child;

	DENTER_FUNCTION(DLEVEL_STABLE);

	/*
	 * Cell's only allow one child, so remove the rest, this may cause a
	 * leak, but they should know better.
	 */
	ecore_list_goto_first(c->children);
	while ((child = ecore_list_next(c->children))) {
		if (child != w)
			ewl_container_child_remove(c, child);
	}

	ewl_object_preferred_inner_size_set(EWL_OBJECT(c),
			ewl_object_preferred_w_get(EWL_OBJECT(w)),
			ewl_object_preferred_h_get(EWL_OBJECT(w)));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_cell_child_resize_cb(Ewl_Container *c, Ewl_Widget *w, 
			int size __UNUSED__, Ewl_Orientation o __UNUSED__)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_object_preferred_inner_size_set(EWL_OBJECT(c),
			ewl_object_preferred_w_get(EWL_OBJECT(w)),
			ewl_object_preferred_h_get(EWL_OBJECT(w)));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
