#include <Ewl.h>

/**
 * @return Returns a new overlay container on success, or NULL on failure.
 * @brief Allocate and initialize a new overlay container
 */
Ewl_Widget *ewl_overlay_new()
{
	Ewl_Overlay *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = NEW(Ewl_Overlay, 1);
	if (!w)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_overlay_init(w)) {
		ewl_widget_destroy(EWL_WIDGET(w));
		w = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(w), DLEVEL_STABLE);
}

/**
 * @param w: the overlay to be initialized to default values and callbacks
 * @return Returns TRUE or FALSE depending on if initialization succeeds.
 * @brief initialize a overlay to default values and callbacks
 *
 * Sets the values and callbacks of a overlay @a w to their defaults.
 */
int ewl_overlay_init(Ewl_Overlay *w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, FALSE);

	/*
	 * Initialize the fields of the inherited container class
	 */
	if (!ewl_container_init(EWL_CONTAINER(w), "overlay"))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_container_show_notify(EWL_CONTAINER(w), ewl_overlay_child_show_cb);
	ewl_container_resize_notify(EWL_CONTAINER(w),
				    ewl_overlay_child_resize_cb);

	ewl_object_set_fill_policy(EWL_OBJECT(w), EWL_FLAG_FILL_NONE);
	ewl_object_set_toplevel(EWL_OBJECT(w), EWL_FLAG_PROPERTY_TOPLEVEL);

	/*
	 * Override the default configure callbacks since the overlay
	 * has special needs for placement.
	 */
	ewl_callback_prepend(EWL_WIDGET(w), EWL_CALLBACK_CONFIGURE,
			     ewl_overlay_configure_cb, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

void ewl_overlay_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewl_Object *o;
	Ewl_Object *child;

	DENTER_FUNCTION(DLEVEL_STABLE);

	o = EWL_OBJECT(w);

	/*
	 * Configure each of the child widgets.
	 */
	ecore_list_goto_first(EWL_CONTAINER(w)->children);
	while ((child = ecore_list_next(EWL_CONTAINER(w)->children))) {
		/*
		 * Try to give the child the full size of the window from it's
		 * base position. The object will constrict it based on the
		 * fill policy. Don't add the TOP and LEFT insets since
		 * they've already been accounted for.
		 */
		ewl_object_request_size(child,
					CURRENT_W(w) -
					ewl_object_get_current_x(child),
					CURRENT_H(w) -
					ewl_object_get_current_y(child));
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_overlay_child_show_cb(Ewl_Container * o, Ewl_Widget * child)
{
	int size;

	DENTER_FUNCTION(DLEVEL_STABLE);

	size = ewl_object_get_current_x(EWL_OBJECT(child)) +
		ewl_object_get_preferred_w(EWL_OBJECT(child)) - CURRENT_X(o);
	if (size > PREFERRED_W(o))
		ewl_object_set_preferred_w(EWL_OBJECT(o), size);

	size = ewl_object_get_current_y(EWL_OBJECT(child)) +
		ewl_object_get_preferred_h(EWL_OBJECT(child)) - CURRENT_Y(o);
	if (size > PREFERRED_H(o))
		ewl_object_set_preferred_h(EWL_OBJECT(o), size);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_overlay_child_resize_cb(Ewl_Container *c, Ewl_Widget *w,
			       int size, Ewl_Orientation o)
{
	int            maxw = 0, maxh = 0;
	Ewl_Overlay   *overlay;
	Ewl_Object    *child;

	DENTER_FUNCTION(DLEVEL_STABLE);

	child = EWL_OBJECT(w);
	overlay = EWL_OVERLAY(c);

	ecore_list_goto_first(EWL_CONTAINER(overlay)->children);
	while ((child = ecore_list_next(EWL_CONTAINER(overlay)->children))) {
		int             cs;

		/*
		 * FIXME: Do we really want to do this?
		 * Move children within the bounds of the viewable area
		 */
		if (ewl_object_get_current_x(child) < CURRENT_X(overlay))
			ewl_object_request_x(child, CURRENT_X(overlay));
		if (ewl_object_get_current_y(child) < CURRENT_Y(overlay))
			ewl_object_request_y(child, CURRENT_Y(overlay));

		cs = ewl_object_get_current_x(child) +
			ewl_object_get_preferred_w(child);

		/*
		 * Check the width and x position vs. overlay width.
		 */
		if (maxw < cs)
			maxw = cs;

		cs = ewl_object_get_current_y(child) +
			ewl_object_get_preferred_h(child);

		/*
		 * Check the height and y position vs. overlay height.
		 */
		if (maxh < cs)
			maxh = cs;

	}

	ewl_object_set_preferred_size(EWL_OBJECT(overlay), maxw, maxh);
	ewl_object_request_size(EWL_OBJECT(c),
				ewl_object_get_current_w(EWL_OBJECT(c)),
				ewl_object_get_current_h(EWL_OBJECT(c)));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
