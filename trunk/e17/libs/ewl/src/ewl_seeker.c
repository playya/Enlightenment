#include <Ewl.h>

void            __ewl_seeker_configure(Ewl_Widget * w, void *ev_data,
				       void *user_data);
void            __ewl_seeker_theme_update(Ewl_Widget * w, void *ev_data,
					  void *user_data);
void            __ewl_seeker_dragbar_mouse_down(Ewl_Widget * w, void *ev_data,
						void *user_data);
void            __ewl_seeker_dragbar_mouse_up(Ewl_Widget * w, void *ev_data,
					      void *user_data);
void            __ewl_seeker_dragbar_mouse_move(Ewl_Widget * w, void *ev_data,
						void *user_data);
void            __ewl_seeker_mouse_down(Ewl_Widget * w, void *ev_data,
					void *user_data);
void            __ewl_seeker_focus_in(Ewl_Widget * w, void *ev_data,
				      void *user_data);
void            __ewl_seeker_focus_out(Ewl_Widget * w, void *ev_data,
				       void *user_data);
void            __ewl_seeker_appearance_changed(Ewl_Widget * w, void *ev_data,
						void *user_data);


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

	ZERO(s, Ewl_Seeker, 1);

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
void ewl_seeker_init(Ewl_Seeker * s, Ewl_Orientation orientation)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	w = EWL_WIDGET(s);

	/*
	 * Initialize the widget fields and set appropriate orientation and
	 * type
	 */
	if (orientation == EWL_ORIENTATION_HORIZONTAL) {
		ewl_container_init(EWL_CONTAINER(w), "hseeker",
				NULL, NULL, NULL);

		ewl_object_set_fill_policy(EWL_OBJECT(w),
				EWL_FILL_POLICY_HFILL |
				EWL_FILL_POLICY_HSHRINK);
	}
	else {
		ewl_container_init(EWL_CONTAINER(w), "vseeker", NULL, NULL,
				   NULL);

		ewl_object_set_fill_policy(EWL_OBJECT(w),
				EWL_FILL_POLICY_VFILL |
				EWL_FILL_POLICY_VSHRINK);
	}

	/*
	 * Create and add the dragbar portion of the seeker
	 */
	s->dragbar = ewl_button_new(NULL);
	ewl_container_append_child(EWL_CONTAINER(s), s->dragbar);
	ewl_widget_set_appearance(EWL_WIDGET(s->dragbar), "dragbar");
	ewl_widget_show(s->dragbar);

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
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, __ewl_seeker_configure,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN, __ewl_seeker_mouse_down,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE,
			    __ewl_seeker_dragbar_mouse_move, NULL);

	/*
	 * Append a callback for catching mouse movements on the dragbar and
	 * add the dragbar to the seeker
	 */
	ewl_callback_append(s->dragbar, EWL_CALLBACK_MOUSE_DOWN,
			    __ewl_seeker_dragbar_mouse_down, NULL);
	ewl_callback_append(s->dragbar, EWL_CALLBACK_MOUSE_UP,
			    __ewl_seeker_dragbar_mouse_up, NULL);

	/*
	 * We want to catch mouse movement events from the dragbar.
	 */
	ewl_container_notify_callback(EWL_CONTAINER(s),
			EWL_CALLBACK_MOUSE_MOVE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
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
 * coords and position as well as move the dragbar to the correct size and
 * position.
 */
void __ewl_seeker_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Seeker     *s;
	double          s1, s2;
	int             dx, dy;
	unsigned int    dw, dh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	s = EWL_SEEKER(w);
	if (!s->dragbar)
		DRETURN(DLEVEL_STABLE);

	dx = CURRENT_X(s);
	dy = CURRENT_Y(s);
	dw = CURRENT_W(s);
	dh = CURRENT_H(s);

	/*
	 * First determine the size based on the number of steps to span from
	 * min to max values. Then reduce the total scale to keep the dragbar on
	 * the seeker, then position the dragbar.
	 */
	s1 = s->step / s->range;
	s2 = s->value / s->range;
	if (s->orientation == EWL_ORIENTATION_VERTICAL) {
		dh *= s1;
		dy += (CURRENT_H(s) - dh) * s2;
	}
	else {
		dw *= s1;
		dx += (CURRENT_W(s) - dw) * s2;
	}

	ewl_object_request_geometry(EWL_OBJECT(s->dragbar), dx, dy, dw, dh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
__ewl_seeker_dragbar_mouse_down(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ecore_X_Event_Mouse_Button_Down *ev;
	Ewl_Seeker     *s;
	int             xx, yy, ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	ev = ev_data;

	s = EWL_SEEKER(w->parent);

	ewl_object_get_current_geometry(EWL_OBJECT(w), &xx, &yy, &ww, &hh);

	if (s->orientation == EWL_ORIENTATION_HORIZONTAL)
		s->dragstart = ev->x - xx;
	else
		s->dragstart = ev->y - yy;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
__ewl_seeker_dragbar_mouse_up(Ewl_Widget * w, void *ev_data, void *user_data)
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
__ewl_seeker_dragbar_mouse_move(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ecore_X_Event_Mouse_Move *ev;
	Ewl_Seeker *s;
	int mx, my;
	int dx, dy;
	unsigned int dw, dh;
	double scale;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	s = EWL_SEEKER(w);

	/*
	 * If the dragbar is not pressed we don't care about mouse movements.
	 */
	if (!(s->dragbar->state & EWL_STATE_PRESSED))
		DRETURN(DLEVEL_STABLE);

	if (s->dragstart < 1)
		DRETURN(DLEVEL_STABLE);

	ev = ev_data;

	mx = ev->x;
	my = ev->y;

	dx = CURRENT_X(s);
	dy = CURRENT_Y(s);
	dw = CURRENT_W(s);
	dh = CURRENT_H(s);

	if (s->orientation == EWL_ORIENTATION_HORIZONTAL) {
		unsigned int adjust;

		adjust = ewl_object_get_current_w(EWL_OBJECT(s->dragbar));
		dw -= adjust;
		adjust /= 2;
		dx += adjust;

		/*
		 * Wheeha make sure this bizatch doesn't run off the sides of
		 * the seeker.
		 */
		if (mx < dx)
			mx = dx;
		else if (mx > dx + dw)
			mx = dx + dw;

		scale = (double)(mx - dx) / (double)dw;
	}
	else {
		unsigned int adjust;

		adjust = ewl_object_get_current_h(EWL_OBJECT(s->dragbar));
		dh -= adjust;
		adjust /= 2;
		dy += adjust;

		if (my < dy)
			my = dy;
		else if (my > (dy + dh))
			my = dy + dh;

		scale = (double)(my - dy) / (double)dh;
	}

	ewl_seeker_set_value(s, scale * s->range);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void __ewl_seeker_mouse_down(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Seeker     *s;
	Ecore_X_Event_Mouse_Button_Down *ev;
	double          value;
	int             xx, yy, ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("ev_data", ev_data);

	ev = ev_data;

	s = EWL_SEEKER(w);

	ewl_object_get_current_geometry(EWL_OBJECT(s->dragbar),
					&xx, &yy, &ww, &hh);

	value = s->value;

	/*
	 * Increment or decrement the value based on the position of the click
	 * relative to the dragbar and the orientation of the seeker.
	 */
	if (s->orientation == EWL_ORIENTATION_HORIZONTAL) {
		if (ev->x < xx)
			value -= s->step;
		else if (ev->x > xx + ww)
			value += s->step;
	}
	else {
		if (ev->y < yy)
			value -= s->step;
		else if (ev->y > yy + hh)
			value += s->step;
	}

	ewl_seeker_set_value(s, value);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
