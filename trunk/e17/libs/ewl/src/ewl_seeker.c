#include <Ewl.h>


/**
 * @param o: the orientation for the new seeker
 * @return Returns NULL on failure, or a pointer to the new seeker on success.
 * @brief Allocate and initialize a new seeker with orientation
 */
Ewl_Widget     *ewl_seeker_new(Ewl_Orientation o)
{
	Ewl_Seeker     *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = NEW(Ewl_Seeker, 1);
	if (!s)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	ewl_seeker_init(s, o);

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}


/**
 * @param s: the seeker to be initialized
 * @param orientation: the orientation of the seeker
 * @return Returns no value.
 * @brief Initialize the seeker to some sane starting values
 *
 * Initializes the seeker @a s to the orientation @a orientation to default
 * values and callbacks.
 */
int ewl_seeker_init(Ewl_Seeker * s, Ewl_Orientation orientation)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, FALSE);

	w = EWL_WIDGET(s);

	/*
	 * Initialize the widget fields and set appropriate orientation and
	 * type
	 */
	if (orientation == EWL_ORIENTATION_HORIZONTAL) {
		if (!ewl_container_init(EWL_CONTAINER(w), "hseeker"))
			DRETURN_INT(FALSE, DLEVEL_STABLE);

		ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_HFILL |
				EWL_FLAG_FILL_HSHRINK);
	}
	else {
		if (!ewl_container_init(EWL_CONTAINER(w), "vseeker"))
			DRETURN_INT(FALSE, DLEVEL_STABLE);

		ewl_object_fill_policy_set(EWL_OBJECT(w),
				EWL_FLAG_FILL_VFILL |
				EWL_FLAG_FILL_VSHRINK);
	}

	ewl_container_show_notify(EWL_CONTAINER(w), ewl_seeker_child_show_cb);

	/*
	 * Create and add the button portion of the seeker
	 */
	s->button = ewl_button_new(NULL);
	ewl_widget_set_internal(s->button, TRUE);
	ewl_container_append_child(EWL_CONTAINER(s), s->button);
	ewl_widget_show(s->button);

	/*
	 * Set the starting orientation, range and values
	 */
	s->orientation = orientation;
	s->range = 100.0;
	s->value = 0.0;
	s->step = 10.0;

	/*
	 * Add necessary configuration callbacks
	 */
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, ewl_seeker_configure_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
			    ewl_seeker_mouse_down_cb, NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE,
			    ewl_seeker_button_mouse_move_cb, NULL);

	/*
	 * Append a callback for catching mouse movements on the button and
	 * add the button to the seeker
	 */
	ewl_callback_append(s->button, EWL_CALLBACK_MOUSE_DOWN,
			    ewl_seeker_button_mouse_down_cb, NULL);
	ewl_callback_append(s->button, EWL_CALLBACK_MOUSE_UP,
			    ewl_seeker_button_mouse_up_cb, NULL);

	/*
	 * We want to catch mouse movement events from the button.
	 */
	ewl_container_notify_callback(EWL_CONTAINER(s),
			EWL_CALLBACK_MOUSE_MOVE);

	DRETURN_INT(FALSE, DLEVEL_STABLE);
}


/**
 *
 * @param s: the seeker whose value will be changed
 * @param v: the new value of the locator, checked against the valid range
 * @return Returns no value.
 * @brief Set the value of pointer of the seekers locator
 */
void ewl_seeker_set_value(Ewl_Seeker * s, double v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	if (v == s->value)
		DRETURN(DLEVEL_STABLE);

	if (v < 0)
		v = 0;

	if (v > s->range)
		v = s->range;

	s->value = v;

	ewl_widget_configure(EWL_WIDGET(s));
	ewl_callback_call(EWL_WIDGET(s), EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param s: the seekers to retrieve the value
 * @return Returns 0 on failure, the value of the seekers locator on success.
 * @brief Retrieve the current value of the seekers locator
 */
double ewl_seeker_get_value(Ewl_Seeker * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, -1);

	DRETURN_FLOAT(s->value, DLEVEL_STABLE);
}


/**
 * @param s: the seeker to change the range
 * @param r: the largest bound on the range of the seekers value
 * @return Returns no value.
 * @brief specify the range of values represented by the seeker
 */
void ewl_seeker_set_range(Ewl_Seeker * s, double r)
{
	int             new_val;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	new_val = r * (s->value / s->range);

	s->range = r;
	s->value = new_val;

	ewl_widget_configure(EWL_WIDGET(s));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param s: the seeker to return the range of values
 * @return Returns 0 on failure, or the upper bound on the seeker on success.
 * @brief Retrieve the range of values represented by the seeker
 */
double ewl_seeker_get_range(Ewl_Seeker * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, -1);

	DRETURN_FLOAT(s->range, DLEVEL_STABLE);
}


/**
 * @param s: the seeker to change step
 * @param step: the new step value for the seeker
 * @return Returns no value.
 * @brief Set the steps between increments
 *
 * Changes the amount that each increment or decrement changes the value of the
 * seeker @a s.
 */
void ewl_seeker_set_step(Ewl_Seeker * s, double step)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	if (step > s->range)
		step = s->range;

	s->step = step;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param s: the seeker to retrieve step size
 * @return Returns the step size of the seeker @a s.
 * @brief Retrieve the step size of the seeker
 */
double ewl_seeker_get_step(Ewl_Seeker * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, -1);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
	DRETURN_FLOAT(s->step, DLEVEL_STABLE);

}


/**
 * @param s: the seeker to change autohide
 * @param v: the new boolean value for autohiding
 * @return Returns no value.
 * @brief Changes the autohide setting on the seeker to @a v.
 *
 * Alter the autohide boolean of the seeker @a s to value @a v. If @a v is
 * TRUE, the seeker will be hidden whenever the button is the full size of
 * the seeker.
 */
void ewl_seeker_set_autohide(Ewl_Seeker *s, int v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("s", s);

	if (s->autohide == v || s->autohide == -v)
		DRETURN(DLEVEL_STABLE);

	if (!v) {
		s->autohide = v;
		if (REALIZED(s))
			ewl_widget_show(EWL_WIDGET(s));
	}
	else if (s->autohide < 0)
		s->autohide = -v;
	else
		s->autohide = v;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the seeker to retrieve autohide value
 * @return Returns TRUE if autohide set, otherwise FALSE.
 * @brief Retrieves the current autohide setting on a seeker
 */
int ewl_seeker_get_autohide(Ewl_Seeker *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("s", s, 0);

	DRETURN_INT(abs(s->autohide), DLEVEL_STABLE);
}

/**
 * @param s: the seeker to set invert property
 * @param invert: the new value for the seekers invert property
 * @return Returns no value.
 * @brief Changes the invert property on the seeker for inverting it's scale.
 */
void ewl_seeker_set_invert(Ewl_Seeker *s, int invert)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	if (s->invert != invert) {
		s->invert = invert;
		ewl_widget_configure(EWL_WIDGET(s));
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the seeker to retrieve invert property value
 * @return Returns the current value of the invert property in the seeker.
 * @brief Retrieve the current invert value from a seeker.
 */
int ewl_seeker_get_invert(Ewl_Seeker *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, FALSE);

	DRETURN_INT(s->invert, DLEVEL_STABLE);
}

/**
 * @param s: the seeker to increase
 * @return Returns no value.
 * @brief Increase the value of a seeker by it's step size
 *
 * Increases the value of the seeker @a s by one increment of it's step size.
 */
void ewl_seeker_increase(Ewl_Seeker * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	s->value += s->step;

	if (s->value > s->range)
		s->value = s->range;
	else if (s->value < 0.0)
		s->value = 0.0;

	ewl_widget_configure(EWL_WIDGET(s));

	ewl_callback_call(EWL_WIDGET(s), EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param s: the seeker to decrease
 * @return Returns no value.
 * @brief Decrease the value of a seeker by it's step size
 *
 * Decreases the value of the seeker @a s by one increment of it's step size.
 */
void ewl_seeker_decrease(Ewl_Seeker * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	s->value -= s->step;

	if (s->value > s->range)
		s->value = s->range;
	else if (s->value < 0.0)
		s->value = 0.0;

	ewl_widget_configure(EWL_WIDGET(s));

	ewl_callback_call(EWL_WIDGET(s), EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * On a configure event we need to adjust the seeker to fit into it's new
 * coords and position as well as move the button to the correct size and
 * position.
 */
void ewl_seeker_configure_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Seeker     *s;
	double          s1, s2;
	int             dx, dy;
	int             dw, dh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	s = EWL_SEEKER(w);
	if (!s->button)
		DRETURN(DLEVEL_STABLE);

	dx = CURRENT_X(s);
	dy = CURRENT_Y(s);
	dw = CURRENT_W(s);
	dh = CURRENT_H(s);

	/*
	 * First determine the size based on the number of steps to span from
	 * min to max values. Then reduce the total scale to keep the button on
	 * the seeker, then position the button.
	 */
	s1 = s->step / s->range;
	if (s->autohide && s1 >= 1.0) {
		ewl_widget_hide(w);
		s->autohide = -abs(s->autohide);
	}

	if (s->invert)
		s2 = (s->range - s->value) / s->range;
	else
		s2 = s->value / s->range;

	if (s->orientation == EWL_ORIENTATION_VERTICAL) {
		dh *= s1;
		dy += (CURRENT_H(s) - dh) * s2;
	}
	else {
		dw *= s1;
		dx += (CURRENT_W(s) - dw) * s2;
	}

	ewl_object_geometry_request(EWL_OBJECT(s->button), dx, dy, dw, dh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
ewl_seeker_button_mouse_down_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Event_Mouse_Down *ev;
	Ewl_Seeker     *s;
	int             xx, yy, ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	ev = ev_data;

	s = EWL_SEEKER(w->parent);

	ewl_object_current_geometry_get(EWL_OBJECT(w), &xx, &yy, &ww, &hh);

	if (s->orientation == EWL_ORIENTATION_HORIZONTAL)
		s->dragstart = ev->x - xx;
	else
		s->dragstart = ev->y - yy;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
ewl_seeker_button_mouse_up_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	EWL_SEEKER(w->parent)->dragstart = 0;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/*
 * Move the cursor to the correct position
 */
void
ewl_seeker_button_mouse_move_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Event_Mouse_Move *ev;
	Ewl_Seeker *s;
	int m;
	int dc, dg;
	int adjust;
	double scale;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	s = EWL_SEEKER(w);

	/*
	 * If the button is not pressed we don't care about mouse movements.
	 */
	if (!ewl_object_state_has(EWL_OBJECT(s->button),
					EWL_FLAG_STATE_PRESSED))
		DRETURN(DLEVEL_STABLE);

	if (s->dragstart < 1)
		DRETURN(DLEVEL_STABLE);

	if (s->step == s->range)
		DRETURN(DLEVEL_STABLE);

	ev = ev_data;

	if (s->orientation == EWL_ORIENTATION_HORIZONTAL) {

		m = ev->x;

		dc = CURRENT_X(s);
		dg = CURRENT_W(s);

		adjust = ewl_object_current_w_get(EWL_OBJECT(s->button));
	}
	else {
		m = ev->y;
		dc = CURRENT_Y(s);
		dg = CURRENT_H(s);

		adjust = ewl_object_current_h_get(EWL_OBJECT(s->button));
	}

	dg -= adjust;
	adjust /= 2;
	dc += adjust;

	/*
	 * Wheeha make sure this bizatch doesn't run off the sides of
	 * the seeker.
	 */
	if (m < dc)
		m = dc;
	else if (m > dc + dg)
		m = dc + dg;

	scale = s->range * (double)(m - dc) / (double)dg;
	if (s->invert)
		scale = s->range - scale;

	ewl_seeker_set_value(s, scale);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_seeker_mouse_down_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Seeker     *s;
	Ewl_Event_Mouse_Down *ev;
	double          value, step = 0;
	int             xx, yy, ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	ev = ev_data;

	s = EWL_SEEKER(w);

	ewl_object_current_geometry_get(EWL_OBJECT(s->button),
					&xx, &yy, &ww, &hh);

	value = s->value;

	/*
	 * Increment or decrement the value based on the position of the click
	 * relative to the button and the orientation of the seeker.
	 */
	if (s->orientation == EWL_ORIENTATION_HORIZONTAL) {
		if (ev->x < xx) {
			step = -s->step;
		}
		else if (ev->x > xx + ww) {
			value = s->step;
		}
	}
	else {
		if (ev->y < yy)
			step = -s->step;
		else if (ev->y > yy + hh)
			step = s->step;
	}

	if (s->invert)
		step = -step;
	value += step;

	ewl_seeker_set_value(s, value);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_seeker_child_show_cb(Ewl_Container *p, Ewl_Widget *w)
{
	int pw, ph;

	Ewl_Seeker *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = EWL_SEEKER(p);

	pw = ewl_object_preferred_w_get(EWL_OBJECT(w));
	ph = ewl_object_preferred_h_get(EWL_OBJECT(w));

	if (s->orientation == EWL_ORIENTATION_HORIZONTAL)
		pw *= s->range / s->step;
	else
		ph *= s->range / s->step;

	ewl_object_preferred_inner_size_set(EWL_OBJECT(p), pw, ph);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
