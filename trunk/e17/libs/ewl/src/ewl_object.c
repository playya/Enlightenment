
#include <Ewl.h>

/**
 * @param o: the object to initialize
 * @return Returns no value.
 * @brief Initialize the fields of an object
 *
 * Sets all of the fields of the object @a o to their default
 * values. NEVER, EVER inherit directly from this class, inherit from the
 * widget instead. The separation is really just a convenience factor, a
 * Widget really is the lowest common class.
 */
int ewl_object_init(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, FALSE);

	/*
	 * Set the default minimum sizes.
	 */
	ewl_object_set_minimum_size(o, EWL_OBJECT_MIN_SIZE,
			EWL_OBJECT_MIN_SIZE);

	/*
	 * Set the default maximum sizes.
	 */
	ewl_object_set_maximum_size(o, EWL_OBJECT_MAX_SIZE,
			EWL_OBJECT_MAX_SIZE);

	/*
	 * Set the default fill policy and alignment for the object.
	 */
	o->flags = EWL_FLAG_FILL_NORMAL | EWL_FLAG_ALIGN_LEFT |
		   EWL_FLAG_ALIGN_TOP;

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve size and position
 * @param x: a pointer to the integer to store the x coordinate
 * @param y: a pointer to the integer to store the y coordinate
 * @param w: a pointer to the integer to store the width
 * @param h: a pointer to the integer to store the height
 * @return Returns no value.
 * @brief Retrieve the size and position of object
 *
 * Examines @a o and stores it's size and position into the
 * integers pointed to by the parameters @a x, @a y, @a w, and @a h.
 */
void
ewl_object_get_current_geometry(Ewl_Object * o, int *x, int *y, int *w, int *h)
{
	DCHECK_PARAM_PTR("o", o);

	if (x)
		*x = ewl_object_get_current_x(o);
	if (y)
		*y = ewl_object_get_current_y(o);
	if (w)
		*w = ewl_object_get_current_w(o);
	if (h)
		*h = ewl_object_get_current_h(o);
}

/**
 * @param o: the object to retrieve size information
 * @param w: a pointer to the integer to store the width of the object
 * @param h: a pointer to the integer to store the height of the object
 * @return Returns no value.
 * @brief Retrieve the current size of an object
 *
 * Stores the width and height of the object into @a w and @a h
 * respectively.
 */
void
ewl_object_get_current_size(Ewl_Object * o, int *w, int *h)
{
	DCHECK_PARAM_PTR("o", o);

	if (w)
		*w = ewl_object_get_current_w(o);
	if (h)
		*h = ewl_object_get_current_h(o);
}

/**
 * @param o: the object to retrieve the current x position
 * @return Returns the current x position of the object @a o.
 * @brief get the current x position of the object
 */
int ewl_object_get_current_x(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(CURRENT_X(o) - PADDING_LEFT(o) - INSET_LEFT(o),
			DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve the current y position
 * @return Returns the current y position of the object.
 * @brief Get the current y position of the object
 */
int ewl_object_get_current_y(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(CURRENT_Y(o) - PADDING_TOP(o) - INSET_TOP(o),
			DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve the current width
 * @return Returns the current width of the object.
 * @brief Get the current width of the object
 */
int ewl_object_get_current_w(Ewl_Object * o)
{
	int w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	w = CURRENT_W(o);
	if (w < MINIMUM_W(o))
		w = MINIMUM_W(o);

	if (w < PREFERRED_W(o) && !(ewl_object_get_fill_policy(o) &
				EWL_FLAG_FILL_HSHRINK))
		w = PREFERRED_W(o);

	if (w > MAXIMUM_W(o))
		w = MAXIMUM_W(o);

	if ((w + PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o)) > w)
		w += PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o);

	DRETURN_INT(w, DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve the current height
 * @return Returns the current height of the object.
 * @brief Get the current height of the object
 */
int ewl_object_get_current_h(Ewl_Object * o)
{
	int h;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	h = CURRENT_H(o);
	if (h < MINIMUM_H(o))
		h = MINIMUM_H(o);

	if (h < PREFERRED_H(o) && !(ewl_object_get_fill_policy(o) &
				EWL_FLAG_FILL_VSHRINK))
		h = PREFERRED_H(o);

	if (h > MAXIMUM_H(o))
		h = MAXIMUM_H(o);

	if ((h + PADDING_VERTICAL(o) + INSET_VERTICAL(o)) > h)
		h += PADDING_VERTICAL(o) + INSET_VERTICAL(o);

	DRETURN_INT(h, DLEVEL_STABLE);
}

/**
 * @param o: the object to change size
 * @param w: the new width of the object
 * @param h: the new height of the object
 * @return Returns no value.
 * @brief Set the preferred size of the object
 *
 * The dimensions of the object @a o are set to the values of
 * the parameters @a w, and @a h unless these values are greater than the objects
 * maximum value or smaller than the objects minimum value. If they are
 * outside these bounds, the size is not altered.
 */
void ewl_object_set_preferred_size(Ewl_Object * o, int w, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	ewl_object_set_preferred_w(o, w);
	ewl_object_set_preferred_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to change preferred width
 * @param w: the value to use as the preferred width
 * @return Returns no value.
 * @brief Set the preferred width of the object
 *
 * Sets the preferred of @a o width to @a w or as close as possible according to
 * the bounds.
 */
void ewl_object_set_preferred_w(Ewl_Object * o, int w)
{
	int             old_size, new_size, resize;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	/*
	 * Store the previous size.
	 */
	old_size = PREFERRED_W(o);

	PREFERRED_W(o) = w;
	if (w < MINIMUM_W(o))
		new_size = MINIMUM_W(o);
	else if (w > MAXIMUM_W(o))
		new_size = MAXIMUM_W(o);
	else
		new_size = w;

	resize = ewl_object_get_fill_policy(EWL_OBJECT(o)) &
		EWL_FLAG_FILL_HSHRINK;

	/*
	 * Now update the widgets parent of the change in size.
	 */
	if (CURRENT_W(o) < PREFERRED_W(o) && resize) {
		resize = 0;
	}
	else
		resize = 1;

	if (resize)
		ewl_container_resize_child(EWL_WIDGET(o), new_size - old_size,
				EWL_ORIENTATION_HORIZONTAL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to change preferred height 
 * @param h: the value to use as the preferred height
 * @return Returns no value.
 * @brief Set the preferred height of the object
 *
 * Sets the preferred of @a o height to @a w or as close as possible according
 * to the bounds.
 */
void ewl_object_set_preferred_h(Ewl_Object * o, int h)
{
	int             old_size, new_size, resize;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	/*
	 * Store the previous size
	 */
	old_size = PREFERRED_H(o);

	PREFERRED_H(o) = h;
	if (h < MINIMUM_H(o))
		new_size = MINIMUM_H(o);
	else if (h > MAXIMUM_H(o))
		new_size = MAXIMUM_H(o);
	else
		new_size = h;

	resize = ewl_object_get_fill_policy(EWL_OBJECT(o)) &
		EWL_FLAG_FILL_VSHRINK;

	/*
	 * Now update the widgets parent of the change in size.
	 */
	if (CURRENT_H(o) < PREFERRED_H(o) && resize) {
		resize = 0;
	}
	else
		resize = 1;

	if (resize)
		ewl_container_resize_child(EWL_WIDGET(o), new_size - old_size,
				EWL_ORIENTATION_VERTICAL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve size information
 * @param w: a pointer to the integer to store the width of the object
 * @param h: a pointer to the integer to store the height of the object
 * @return Returns no value.
 * @brief Retrieve the preferred size of an object
 *
 * Stores the width and height of object @a o into @a w and @a h
 * respectively.
 */
void ewl_object_get_preferred_size(Ewl_Object * o, int *w, int *h)
{
	DCHECK_PARAM_PTR("o", o);

	if (w)
		*w = ewl_object_get_preferred_w(o);
	if (h)
		*h = ewl_object_get_preferred_h(o);
}

/**
 * @param o: the object to retrieve the preferred width
 * @return Returns the preferred width of the object @a o.
 * @brief Get the preferred width of the object
 */
int ewl_object_get_preferred_w(Ewl_Object * o)
{
	int add, temp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	add = INSET_HORIZONTAL(o) + PADDING_HORIZONTAL(o);

	if (PREFERRED_W(o) < MINIMUM_W(o))
		temp = MINIMUM_W(o);
	else if (PREFERRED_W(o) > MAXIMUM_W(o))
		temp = MAXIMUM_W(o);
	else
		temp = PREFERRED_W(o);

	temp += add;

	DRETURN_INT(temp, DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve the preferred height
 * @return Returns the preferred height of the object.
 * @brief Get the preferred height of the object
 */
int ewl_object_get_preferred_h(Ewl_Object * o)
{
	int add, temp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	add = INSET_VERTICAL(o) + PADDING_VERTICAL(o);

	if (PREFERRED_H(o) < MINIMUM_H(o))
		temp = MINIMUM_H(o);
	else if (PREFERRED_H(o) > MAXIMUM_H(o))
		temp = MAXIMUM_H(o);
	else
		temp = PREFERRED_H(o);

	temp += add;

	DRETURN_INT(temp, DLEVEL_STABLE);
}

/**
 * @param o: the object to request a new size
 * @param x: the x coordinate to request be applied to the object
 * @param y: the y coordinate to request be applied to the object
 * @param w: the width to request be applied to the object
 * @param h: the height to request be applied to the object
 * @return Returns no value.
 * @brief Request a new geometry be applied to an object
 *
 * The given geometry is requested to be applied to the object. This is the
 * usual method for requesting a new geometry for an object.
 */
void ewl_object_request_geometry(Ewl_Object * o, int x, int y, int w, int h)
{
	/*
	 * Pass the parameters on to the appropriate object request functions.
	 */
	ewl_object_request_position(o, x, y);
	ewl_object_request_size(o, w, h);
}


/**
 * @param o: the object to request a new size
 * @param w: the width to request be applied to the object
 * @param h: the height to request be applied to the object
 * @return Returns no value.
 * @brief Request a new size be applied to an object
 *
 * The given size is requested to be applied to the object @a o at a later time.
 * This is the usual method for requesting a new size for an object.
 */
void ewl_object_request_size(Ewl_Object * o, int w, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_object_request_w(o, w);
	ewl_object_request_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to request a new size
 * @param x: the x coordinate to request be applied to the object
 * @param y: the y coordinate to request be applied to the object
 * @return Returns no value.
 * @brief Request a new position be applied to an object
 *
 * The given position is requested to be applied to the object @a o at a later
 * time. This is the usual method for requesting a new position for an object.
 */
void ewl_object_request_position(Ewl_Object * o, int x, int y)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_object_request_x(o, x);
	ewl_object_request_y(o, y);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to request a new x position
 * @param x: the new x coordinate to be applied to the object
 * @return Returns no value.
 * @brief Request a new x position for an object
 *
 * The given x coordinate is stored to be applied to the object @a o at a later
 * time.
 */
inline void ewl_object_request_x(Ewl_Object * o, int x)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	CURRENT_X(o) = x + PADDING_LEFT(o) + INSET_LEFT(o);
	ewl_widget_configure(EWL_WIDGET(o));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to request a new y position
 * @param y: the new y coordinate to be applied to the object
 * @return Returns no value.
 * @brief Request a new y position for an object
 *
 * The given y coordinate is stored to be applied to the object @a o at a later
 * time.
 */
inline void ewl_object_request_y(Ewl_Object * o, int y)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	CURRENT_Y(o) = y + PADDING_TOP(o) + INSET_TOP(o);
	ewl_widget_configure(EWL_WIDGET(o));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to request a new width
 * @param w: the new width to be applied to the object
 * @return Returns no value.
 * @brief Request a new width for an object
 *
 * The given width is stored to be applied to the object @a o at a later time.
 */
void ewl_object_request_w(Ewl_Object * o, int w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	if (w > PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o))
		w -= PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o);
	else
		w = 0;

	/*
	 * Bound the width by the preferred size first.
	 */
	if ((w < o->preferred.w && !(o->flags & EWL_FLAG_FILL_HSHRINK))
	    || (o->preferred.w && w > o->preferred.w &&
		!(o->flags & EWL_FLAG_FILL_HFILL)))
		w = o->preferred.w;

	/*
	 * Now bound it by the min's and max's to achieve the desired size.
	 */
	if (w < MINIMUM_W(o))
		w = MINIMUM_W(o);
	else if (w > MAXIMUM_W(o))
		w = MAXIMUM_W(o);

	CURRENT_W(o) = w;
	ewl_widget_configure(EWL_WIDGET(o));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to request a new height
 * @param h: the new height to be applied to the object
 * @return Returns no value.
 * @brief Request a new width for an object
 *
 * The given height is stored to be applied to the object @a o at a later time.
 */
void ewl_object_request_h(Ewl_Object * o, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	if (h > PADDING_VERTICAL(o) + INSET_VERTICAL(o))
		h -= PADDING_VERTICAL(o) + INSET_VERTICAL(o);
	else
		h = 0;

	/*
	 * Bound the width by the preferred size first.
	 */
	if ((h < o->preferred.h && !(o->flags & EWL_FLAG_FILL_VSHRINK))
	    || (o->preferred.h && h > o->preferred.h &&
		!(o->flags & EWL_FLAG_FILL_VFILL)))
		h = o->preferred.h;

	/*
	 * Now bound it by the min's and max's to achieve the desired size.
	 */
	if (h < MINIMUM_H(o))
		h = MINIMUM_H(o);
	else if (h > MAXIMUM_H(o))
		h = MAXIMUM_H(o);

	CURRENT_H(o) = h;
	ewl_widget_configure(EWL_WIDGET(o));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to change the minimum size
 * @param w: the new minimum width
 * @param h: the new minimum height
 * @return Returns no value.
 * @brief Set the minimum size of an object
 *
 * Sets the minimum size of the object @a o to @a w x @a h. If the
 * current size or maximum size are less than the new minimum, they are set to
 * the new minimum size.
 */
void ewl_object_set_minimum_size(Ewl_Object * o, int w, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	ewl_object_set_minimum_w(o, w);
	ewl_object_set_minimum_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to change the minimum width
 * @param w: the new minimum width
 * @return Returns no value.
 * @brief Set the minimum width of an object
 *
 * Sets the minimum width of the object @a o to @a w. If the current width or
 * maximum width are less than the new minimum, they are set to the new minimum
 * width.
 */
inline void ewl_object_set_minimum_w(Ewl_Object * o, int w)
{
	int old_size, new_size;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (w < 1) {
		ewl_print_warning();
		DRETURN(DLEVEL_STABLE);
	}

	old_size = MINIMUM_W(o);

	new_size = MINIMUM_W(o) = w;

	if (MAXIMUM_W(o) < w)
		MAXIMUM_W(o) = w;

	if (PREFERRED_W(o) < MINIMUM_W(o))
		ewl_container_resize_child(EWL_WIDGET(o),
					   MINIMUM_W(o) - PREFERRED_W(o),
					   EWL_ORIENTATION_HORIZONTAL);

	if (CURRENT_W(o) < w)
		ewl_object_request_w(o, w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to change the minimum height
 * @param h: the new minimum height
 * @brief Set the minimum height of an object
 *
 * @return Returns no value.
 * Sets the minimum height of the object @a o to @a h. If the current height or
 * maximum height are less than the new minimum, they are set to the new minimum
 * height.
 */
inline void ewl_object_set_minimum_h(Ewl_Object * o, int h)
{
	int old_size, new_size;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (h < 1) {
		ewl_print_warning();
		DRETURN(DLEVEL_STABLE);
	}

	old_size = MINIMUM_H(o);

	new_size = MINIMUM_H(o) = h;

	if (MAXIMUM_H(o) < h)
		MAXIMUM_H(o) = h;

	if (PREFERRED_H(o) < MINIMUM_H(o))
		ewl_container_resize_child(EWL_WIDGET(o),
					   MINIMUM_H(o) - PREFERRED_H(o),
					   EWL_ORIENTATION_VERTICAL);

	if (CURRENT_H(o) < h)
		ewl_object_request_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to get the minimum width
 * @return Returns the minimum width of the object @a o.
 * @brief Get the minimum width of an object
 */
inline int ewl_object_get_minimum_w(Ewl_Object * o)
{
	int val;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	if (o->flags & EWL_FLAG_FILL_HSHRINK || MINIMUM_W(o) > PREFERRED_W(o))
		val = MINIMUM_W(o);
	else
		val = PREFERRED_W(o);

	DRETURN_INT(val + PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o),
			DLEVEL_STABLE);
}

/**
 * @param o: the object to get the minimum height
 * @return Returns the minimum height of the object.
 * @brief Get the minimum height of an object
 */
inline int ewl_object_get_minimum_h(Ewl_Object * o)
{
	int val;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	if (o->flags & EWL_FLAG_FILL_VSHRINK || MINIMUM_H(o) > PREFERRED_H(o))
		val = MINIMUM_H(o);
	else
		val = PREFERRED_H(o);

	DRETURN_INT(val + PADDING_VERTICAL(o) + INSET_VERTICAL(o),
			DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve the minimum dimensions
 * @param w: a pointer to an integer to store the minimum width
 * @param h: a pointer to an integer to store the minimum height
 * @return Returns no value.
 * @brief Retrieve the minimum dimensions of an object
 *
 * Stores the minimum height and width of object @a o into the integers pointed
 * to by  @a w and @a h respectively.
 */
void
ewl_object_get_minimum_size(Ewl_Object * o, int *w, int *h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	if (w)
		*w = ewl_object_get_minimum_w(o);
	if (h)
		*h = ewl_object_get_minimum_h(o);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to change the maximum size
 * @param w: the new maximum width
 * @param h: the new maximum height
 * @return Returns no value.
 * @brief Set the maximum size of an object
 *
 * Sets the maximum size of the object @a o to @a w x @a h. If the
 * current size or minimum size are less than the new maximum, they are set to
 * the new maximum size.
 */
void
ewl_object_set_maximum_size(Ewl_Object * o, int w, int h)
{
	DCHECK_PARAM_PTR("o", o);
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_object_set_maximum_w(o, w);
	ewl_object_set_maximum_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to change the maximum width
 * @param w: the new maximum width
 * @return Returns no value.
 * @brief Set the minimum width of an object
 *
 * Sets the maximum width of the object @a o to @a w. If the current width or
 * minimum width are less than the new maximum, they are set to the new
 * maximum width.
 */
inline void ewl_object_set_maximum_w(Ewl_Object * o, int w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (w < 1) {
		ewl_print_warning();
		DRETURN(DLEVEL_STABLE);
	}

	MAXIMUM_W(o) = w;

	if (MINIMUM_W(o) > w)
		MINIMUM_W(o) = w;

	if (PREFERRED_W(o) > MAXIMUM_W(o))
		ewl_object_set_preferred_w(o, PREFERRED_W(o));

	if (CURRENT_W(o) > w)
		ewl_object_request_h(o, w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to change the maximum height
 * @param h: the new maximum height
 * @return Returns no value.
 * @brief Set the minimum height of an object
 *
 * Sets the maximum height of the object @a o to @a h. If the
 * current height or minimum width are less than the new maximum, they are set
 * to the new maximum height.
 */
inline void ewl_object_set_maximum_h(Ewl_Object * o, int h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (h < 1) {
		ewl_print_warning();
		DRETURN(DLEVEL_STABLE);
	}

	o->maximum.h = h;

	if (MINIMUM_H(o) > h)
		o->minimum.h = h;

	if (PREFERRED_H(o) > MAXIMUM_H(o))
		ewl_object_set_preferred_h(o, PREFERRED_H(o));

	if (CURRENT_H(o) > h)
		ewl_object_request_h(o, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param o: the object to get the maximum width
 * @return Returns the maximum width of the object.
 * @brief Get the maximum width of an object
 */
inline int ewl_object_get_maximum_w(Ewl_Object * o)
{
	int val;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	if (o->flags & EWL_FLAG_FILL_HFILL)
		val = MAXIMUM_W(o);
	else
		val = PREFERRED_W(o);

	if ((val + PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o)) > val)
		val += PADDING_HORIZONTAL(o) + INSET_HORIZONTAL(o);

	DRETURN_INT(val, DLEVEL_STABLE);
}


/**
 * @param o: the object to get the maximum height
 * @return Returns the maximum height of the object.
 * @brief Get the maximum height of an object
 */
inline int ewl_object_get_maximum_h(Ewl_Object * o)
{
	int val;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	if (o->flags & EWL_FLAG_FILL_VFILL)
		val = MAXIMUM_H(o);
	else
		val = PREFERRED_H(o);

	if ((val + PADDING_VERTICAL(o) + INSET_VERTICAL(o)) > val)
		val += PADDING_VERTICAL(o) + INSET_VERTICAL(o);

	DRETURN_INT(val, DLEVEL_STABLE);
}


/**
 * @param o: the object to retrieve the maximum dimensions
 * @param w: a pointer to an integer to store the maximum width
 * @param h: a pointer to an integer to store the maximum height
 * @return Returns no value.
 * @brief Retrieve the minimum dimensions of an object
 *
 * Stores the maximum height and width of the object into the
 * integers pointed to by  @a w and @a h respectively.
 */
void
ewl_object_get_maximum_size(Ewl_Object * o, int *w, int *h)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	if (w)
		*w = ewl_object_get_maximum_w(o);
	if (h)
		*h = ewl_object_get_maximum_h(o);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to change the padding
 * @param l: the new padding along the left side of the object
 * @param r: the new padding along the right side of the object
 * @param t: the new padding along the top side of the object
 * @param b: the new padding along the bottom side of the object
 * @return Returns no value.
 * @brief Set the padding around an objects edges
 *
 * Stores the values of @a l, @a r, @a t and @a b into the object to
 * be used for distancing it's edges from other widgets when laying out.
 */
void ewl_object_set_padding(Ewl_Object * o, int l, int r, int t, int b)
{
	int dh, dv;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	dh = (l - o->pad.l) + (r - o->pad.r);
	dv = (t - o->pad.t) + (b - o->pad.t);

	o->pad.l = l;
	o->pad.r = r;
	o->pad.t = t;
	o->pad.b = b;

	/*
	 * Now update the widgets parent of the change in size.
	 */
	ewl_container_resize_child(EWL_WIDGET(o), dh,
				   EWL_ORIENTATION_HORIZONTAL);
	ewl_container_resize_child(EWL_WIDGET(o), dv,
				   EWL_ORIENTATION_VERTICAL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve padding
 * @param l: a pointer to an integer that receives padding of the left side
 * @param r: a pointer to an integer that receives padding of the right side
 * @param t: a pointer to an integer that receives padding of the top side
 * @param b: a pointer to an integer that receives padding of the bottom side
 * @return Returns no value.
 * @brief Retrieve the edge padding of an object
 *
 * Stores the edge padding of the object @a o into the integer pointer
 * parameters passed in.
 */
void ewl_object_get_padding(Ewl_Object * o, int *l, int *r, int *t, int *b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (l)
		*l = o->pad.l;
	if (r)
		*r = o->pad.r;
	if (t)
		*t = o->pad.t;
	if (b)
		*b = o->pad.b;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the top padding
 * @return Returns the top padding for the object.
 * @brief Get the top padding for the object
 */
int ewl_object_top_padding(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(PADDING_TOP(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the bottom padding
 * @return Returns the bottom padding for the object.
 * @brief Get the bottom padding for the object
 */
int ewl_object_bottom_padding(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(PADDING_BOTTOM(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the left padding
 * @return Returns the left padding for the object.
 * @brief Get the left padding for the object
 */
int ewl_object_left_padding(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(PADDING_LEFT(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the right padding
 * @return Returns the right padding for the object.
 * @brief Get the right padding for the object
 */
int ewl_object_right_padding(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(PADDING_RIGHT(o), DLEVEL_STABLE);
}


/**
 * @param o: the object to change the insets
 * @param l: the new insets along the left side of the object
 * @param r: the new insets along the right side of the object
 * @param t: the new insets along the top side of the object
 * @param b: the new insets along the bottom side of the object
 * @return Returns no value.
 * @brief Set the insets around an objects edges
 *
 * Stores the values of @a l, @a r, @a t and @a b into the object to
 * be used for distancing it's edges from other widgets when laying out.
 */
void ewl_object_set_insets(Ewl_Object * o, int l, int r, int t, int b)
{
	int dh, dv;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	dh = (l - o->insets.l) + (r - o->insets.r);
	dv = (t - o->insets.t) + (b - o->insets.t);

	o->insets.l = l;
	o->insets.r = r;
	o->insets.t = t;
	o->insets.b = b;

	/*
	 * Now update the widgets parent of the change in size.
	 */
	ewl_container_resize_child(EWL_WIDGET(o), dh,
				EWL_ORIENTATION_HORIZONTAL);
	ewl_container_resize_child(EWL_WIDGET(o), dv,
				EWL_ORIENTATION_VERTICAL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to retrieve insets
 * @param l: a pointer to an integer that receives the insets of the left side
 * @param r: a pointer to an integer that receives the insets of the right side
 * @param t: a pointer to an integer that receives the insets of the top side
 * @param b: a pointer to an integer that receives the insets of the bottom side
 * @return Returns no value.
 * @brief Retrieve the edge insets of an object
 *
 * Stores the edge insets of the object @a o into the integer pointer parameters
 * passed in.
 */
void ewl_object_get_insets(Ewl_Object * o, int *l, int *r, int *t, int *b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	if (l)
		*l = o->insets.l;
	if (r)
		*r = o->insets.r;
	if (t)
		*t = o->insets.t;
	if (b)
		*b = o->insets.b;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the top insets
 * @return Returns the top insets for the object.
 * @brief Get the top insets for the object
 */
int ewl_object_top_insets(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(INSET_TOP(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the bottom insets
 * @return Returns the bottom insets for the object.
 * @brief Get the bottom insets for the object
 */
int ewl_object_bottom_insets(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(INSET_BOTTOM(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the left insets
 * @return Returns the left insets for the object.
 * @brief Get the left insets for the object
 */
int ewl_object_left_insets(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(INSET_LEFT(o), DLEVEL_STABLE);
}

/**
 * @param o: the ewl object to retrieve the right insets
 * @return Returns the right insets for the object.
 * @brief Get the right insets for the object
 */
int ewl_object_right_insets(Ewl_Object * o)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(INSET_RIGHT(o), DLEVEL_STABLE);
}

/**
 * @param o: the object to change alignment
 * @param align: the new alignment for the object
 * @return Returns no value.
 * @brief Change the alignment of the specified object
 *
 * Stores the new alignment value into the object for use
 * when laying out the object.
 */
inline void ewl_object_set_alignment(Ewl_Object * o, unsigned int align)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	ewl_object_remove_flags(o, EWL_FLAGS_ALIGN_MASK, EWL_FLAGS_ALIGN_MASK);
	ewl_object_add_flags(o, align, EWL_FLAGS_ALIGN_MASK);

	if (EWL_WIDGET(o)->parent)
		ewl_widget_configure(EWL_WIDGET(o)->parent);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to place
 * @param x: the x coordinate of the available area
 * @param y: the y coordinate of the available area
 * @param w: the width of the available area
 * @param h: the height of the available area
 * @return Returns no value.
 * @brief Assign a specific area to an object
 *
 * Attempts to fill the object to the specified area, aligns
 * the object within that area.
 */
void
ewl_object_place(Ewl_Object *o, int x, int y, int w, int h)
{
	int x_pos, y_pos;
	int w_accept, h_accept;

	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_object_request_size(o, w, h);

	w_accept = ewl_object_get_current_w(o);
	h_accept = ewl_object_get_current_h(o);

	/*
	 * Horizontal position
	 */
	if (o->flags & EWL_FLAG_ALIGN_LEFT)
		x_pos = x;
	else if (o->flags & EWL_FLAG_ALIGN_RIGHT)
		x_pos = x + w - w_accept;
	else
		x_pos = x + ((w - w_accept) / 2);

	/*
	 * Vertical position
	 */
	if (o->flags & EWL_FLAG_ALIGN_TOP)
		y_pos = y;
	else if (o->flags & EWL_FLAG_ALIGN_BOTTOM)
		y_pos = y + h - h_accept;
	else
		y_pos = y + ((h - h_accept) / 2);

	ewl_object_request_position(o, x_pos, y_pos);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to change fill policy
 * @param fill: the new fill policy for the object
 * @return Returns no value.
 * @brief Change the fill policy of the specified object
 *
 * Stores the new fill policy value into the object for use when laying out
 * the object.
 */
inline void ewl_object_set_fill_policy(Ewl_Object * o, unsigned int fill)
{
	unsigned int old_mask;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("o", o);

	if ((o->flags & EWL_FLAGS_FILL_MASK) == fill)
		DRETURN(DLEVEL_STABLE);

	old_mask = o->flags;
	ewl_object_remove_flags(o, EWL_FLAGS_FILL_MASK, EWL_FLAGS_FILL_MASK);
	ewl_object_add_flags(o, fill, EWL_FLAGS_FILL_MASK);

	/*
	 * Notify the parent of a resize if the fill policy causes the returned
	 * minimum size to change. Check horizontal change first.
	 */
	/* FIXME:
	if ((old_mask & EWL_FLAG_FILL_HSHRINK) &&
			!(fill & EWL_FLAG_FILL_HSHRINK))
		ewl_container_resize_child(EWL_WIDGET(o), PREFERRED_W(o) -
				MINIMUM_W(o), EWL_ORIENTATION_HORIZONTAL);
	else if (!(old_mask & EWL_FLAG_FILL_HSHRINK) &&
			(fill & EWL_FLAG_FILL_HSHRINK))
		ewl_container_resize_child(EWL_WIDGET(o), MINIMUM_W(o) -
				PREFERRED_W(o), EWL_ORIENTATION_HORIZONTAL);
	*/

	/*
	 * Now the vertical change
	 */
	/* FIXME:
	if ((old_mask & EWL_FLAG_FILL_VSHRINK) &&
			!(fill & EWL_FLAG_FILL_VSHRINK))
		ewl_container_resize_child(EWL_WIDGET(o), PREFERRED_H(o) -
				MINIMUM_H(o), EWL_ORIENTATION_VERTICAL);
	else if (!(old_mask & EWL_FLAG_FILL_VSHRINK) &&
			(fill & EWL_FLAG_FILL_VSHRINK))
		ewl_container_resize_child(EWL_WIDGET(o), MINIMUM_H(o) -
				PREFERRED_H(o), EWL_ORIENTATION_VERTICAL);
	*/

	if (EWL_WIDGET(o)->parent)
		ewl_widget_configure(EWL_WIDGET(o)->parent);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to set the specified object flags
 * @param flags: a bitmask of new flags to be set in the object
 * @param mask: a bitmask limiting added flags to a certain set
 * @return Returns no value.
 * @brief Add the set of flags specified in @a flags to @a o.
 */
void ewl_object_add_flags(Ewl_Object *o, unsigned int flags, unsigned int mask)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	o->flags |= (flags & mask);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the object to remove specified state flags
 * @param flags: a bitmask of flags to be removed from the object
 * @param mask: a bitmask limiting removed flags to a certain set
 * @return Returns no value.
 * @brief Removes the set of state flags specified in @a flags from @a o.
 */
void
ewl_object_remove_flags(Ewl_Object *o, unsigned int flags, unsigned int mask)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("o", o);

	o->flags &= ~(flags & mask);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param o: the parameter to retrieve the current value of object flags
 * @param mask: get only the flags specified in mask
 * @return Returns the current setting of the object flags for @a o.
 * @brief Retrieves the current setting of the object flags for @a o.
 */
unsigned int ewl_object_get_flags(Ewl_Object *o, unsigned int mask)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT(o->flags & mask, DLEVEL_STABLE);
}

/**
 * @param o: the object to check for a specified flags
 * @param flags: the bitmask of flags to check on the object
 * @param mask: get only the flags specified in mask
 * @return Returns TRUE if the specified flags are set, FALSE otherwise.
 * @brief Determines if an object has the requested @a flags set.
 */
unsigned int
ewl_object_has_flags(Ewl_Object *o, unsigned int flags, unsigned int mask)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("o", o, 0);

	DRETURN_INT((o->flags & (flags & mask)) == (flags & mask),
			DLEVEL_STABLE);
}
