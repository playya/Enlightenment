
#include <Ewl.h>


void            __ewl_scrollbar_decrement(Ewl_Widget * w, void *ev_data,
					  void *user_data);
void            __ewl_scrollbar_increment(Ewl_Widget * w, void *ev_data,
					  void *user_data);


/**
 * @param orientation: the desired orientation of the scrollbar
 * @return Returns NULL on failure, or a pointer to a new scrollbar on success.
 * @brief Allocate and initialize a new scrollbar widget
 */
Ewl_Widget     *ewl_scrollbar_new(Ewl_Orientation orientation)
{
	Ewl_Scrollbar  *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = NEW(Ewl_Scrollbar, 1);
	if (!s)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	ZERO(s, Ewl_Scrollbar, 1);

	/*
	 * Initialize the objects fields.
	 */
	ewl_scrollbar_init(s, orientation);

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to initialize
 * @param orientation: the orientation for the scrollbar
 * @return Returns no value.
 * @brief Initialize a scrollbar to default values
 */
void ewl_scrollbar_init(Ewl_Scrollbar * s, Ewl_Orientation orientation)
{
	Ewl_Widget     *w;
	char            key[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	w = EWL_WIDGET(s);

	ewl_box_init(EWL_BOX(w), orientation);

	/*
	 * Create the basic widgets that are contained in the scrollbar.
	 */
	s->button_decrement = ewl_button_new(NULL);
	ewl_object_set_alignment(EWL_OBJECT(s->button_decrement),
			EWL_ALIGNMENT_CENTER);
	ewl_object_set_fill_policy(EWL_OBJECT(s->button_decrement),
				   EWL_FILL_POLICY_NONE);
	ewl_widget_show(s->button_decrement);

	/*
	 * Create the increment button.
	 */
	s->button_increment = ewl_button_new(NULL);
	ewl_object_set_alignment(EWL_OBJECT(s->button_increment),
			EWL_ALIGNMENT_CENTER);
	ewl_object_set_fill_policy(EWL_OBJECT(s->button_increment),
				   EWL_FILL_POLICY_NONE);
	ewl_widget_show(s->button_increment);

	/*
	 * Setup the seeker portion
	 */
	s->seeker = ewl_seeker_new(orientation);
	ewl_widget_show(s->seeker);

	/*
	 * Attach callbacks to the buttons and seeker to handle the various
	 * events.
	 */
	if (orientation == EWL_ORIENTATION_HORIZONTAL) {
		ewl_callback_append(s->button_increment,
				EWL_CALLBACK_MOUSE_DOWN,
				__ewl_scrollbar_increment, s);
		ewl_callback_append(s->button_decrement,
				EWL_CALLBACK_MOUSE_DOWN,
				__ewl_scrollbar_decrement, s);
	}
	else {
		ewl_callback_append(s->button_increment,
				EWL_CALLBACK_MOUSE_DOWN,
				__ewl_scrollbar_decrement, s);
		ewl_callback_append(s->button_decrement,
				EWL_CALLBACK_MOUSE_DOWN,
				__ewl_scrollbar_increment, s);
	}

	/*
	 * Set the default alignment for the buttons.
	 */
	ewl_object_set_alignment(EWL_OBJECT(s->button_decrement),
				 EWL_ALIGNMENT_CENTER);
	ewl_object_set_alignment(EWL_OBJECT(s->button_increment),
				 EWL_ALIGNMENT_CENTER);

	/*
	 * Set the default amount of space that the seeker should fill.
	 */
	s->fill_percentage = 1.0;

	/*
	 * Append a value change callback to the seeker to catch when it
	 * moves.
	 */
	ewl_container_notify_callback(EWL_CONTAINER(s),
			EWL_CALLBACK_VALUE_CHANGED);

	/*
	 * Define the maximum value that the seeker can reach, and the
	 * default increments it takes to get there.
	 */
	ewl_seeker_set_range(EWL_SEEKER(s->seeker), 1.0);
	ewl_seeker_set_step(EWL_SEEKER(s->seeker), 0.05);

	/*
	 * Set the appearance strings for the parts of the scrollbar
	 */
	ewl_widget_set_appearance(s->button_decrement, "button_decrement");
	ewl_widget_set_appearance(s->button_increment, "button_increment");

	/*
	 * Set the alignment of the buttons to the seeker.
	 */
	snprintf(key, PATH_MAX, "%s/button_order", w->appearance);
	s->buttons_alignment = ewl_theme_data_get_int(EWL_WIDGET(s), key);

	/*
	 * Setup a few orientation specific variables, such as appearance and
	 * packing order.
	 */
	if (orientation == EWL_ORIENTATION_HORIZONTAL) {

		if (s->buttons_alignment & EWL_ALIGNMENT_LEFT) {

			/*
			 * Place in decrement, increment, seeker order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
		}
		else if (s->buttons_alignment & EWL_ALIGNMENT_RIGHT) {

			/*
			 * Place in seeker, decrement, increment order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
		}
		else {

			/*
			 * Place in decrement, seeker, increment order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
		}

		/*
		 * Set the default value to the beginning of the seeker.
		 */
		ewl_seeker_set_value(EWL_SEEKER(s->seeker), 0);
		ewl_widget_set_appearance(w, "hscrollbar");
		ewl_object_set_fill_policy(EWL_OBJECT(s),
				EWL_FILL_POLICY_HFILL);
	}
	else {

		if (s->buttons_alignment & EWL_ALIGNMENT_TOP) {

			/*
			 * Place in increment, decrement, seeker order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
		}
		else if (s->buttons_alignment & EWL_ALIGNMENT_BOTTOM) {

			/*
			 * Place in seeker, increment, decrement order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
		}
		else {

			/*
			 * Place in increment, seeker, decrement order.
			 */
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_increment);
			ewl_container_append_child(EWL_CONTAINER(s), s->seeker);
			ewl_container_append_child(EWL_CONTAINER(s),
						   s->button_decrement);
		}

		/*
		 * Set the default value to the beginning of the seeker.
		 */
		ewl_seeker_set_value(EWL_SEEKER(s->seeker), 0);
		ewl_widget_set_appearance(w, "vscrollbar");
		ewl_object_set_fill_policy(EWL_OBJECT(s),
				EWL_FILL_POLICY_VFILL);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to get the current value
 * @return Returns the current value of the scrollbar @a s.
 * @brief Get the current value of the dragbar
 */
double ewl_scrollbar_get_value(Ewl_Scrollbar * s)
{
	double          v;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, -1);

	v = ewl_seeker_get_value(EWL_SEEKER(s->seeker));

	/*
	if (EWL_BOX(s)->orientation == EWL_ORIENTATION_VERTICAL)
		v = 1.0 - v;
		*/

	DRETURN_FLOAT(v, DLEVEL_UNSTABLE);
}

/**
 * @param s: the scrollbar to set the current value
 * @param v: the new value for the scrollbar
 * @return Returns no value.
 * @brief Set the current value of the dragbar
 *
 * Sets the current value of the scrollbar @a s.
 */
void ewl_scrollbar_set_value(Ewl_Scrollbar * s, double v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	/*
	if (EWL_BOX(s)->orientation == EWL_ORIENTATION_VERTICAL)
		v = 1.0 - v;
		*/

	ewl_seeker_set_value(EWL_SEEKER(s->seeker), v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to retrieve step size
 * @return Returns the current step size of the scrollbar.
 * @brief Get the current step size of the scrollbar
 */
double
ewl_scrollbar_get_step(Ewl_Scrollbar *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("s", s, 0);

	DRETURN_INT(ewl_seeker_get_step(EWL_SEEKER(s->seeker)), DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to change step size
 * @param v: the new step size of the scrollbar
 * @return Returns no value.
 * @brief Change the step size of a scrollbar
 *
 * Changes the step size of the scrollbar @a s to @a v.
 */
void ewl_scrollbar_set_step(Ewl_Scrollbar *s, double v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("s", s);

	ewl_seeker_set_step(EWL_SEEKER(s->seeker), v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to set the flags
 * @param f: the flags to set for the scrollbar
 * @return Returns no value.
 * @brief Set the flag mask for a scrollbar
 *
 * Sets the flags @a f for the scrollbar @a s.
 */
void ewl_scrollbar_set_flag(Ewl_Scrollbar * s, Ewl_ScrollBar_Flags f)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	s->flag = f;
	ewl_widget_configure(EWL_WIDGET(s));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollbar to retrieve the flags
 * @return Returns the flags from the scrollbars @a s.
 * @brief Retrieve the current flags of a scrollbar
 */
Ewl_ScrollBar_Flags ewl_scrollbar_get_flag(Ewl_Scrollbar * s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0);

	DRETURN_INT(s->flag, DLEVEL_STABLE);
}


/*
 * Decrement the value of the scrollbar's seeker portion
 */
void __ewl_scrollbar_decrement(Ewl_Widget * w, void *ev_data, void *user_data)
{
	double          v;
	Ewl_Scrollbar  *s;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	s = EWL_SCROLLBAR(user_data);

	/*
	 * ewl_seeker_decrease(EWL_SEEKER(s->seeker));
	 */

	v = ewl_seeker_get_value(EWL_SEEKER(s->seeker));
	v -= 0.05;

	if (v < 0.0)
		v = 0.0;

	ewl_seeker_set_value(EWL_SEEKER(s->seeker), v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Decrement the value of the scrollbar's seeker portion
 */
void __ewl_scrollbar_increment(Ewl_Widget * w, void *ev_data, void *user_data)
{
	double          v;
	Ewl_Scrollbar  *s;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	s = EWL_SCROLLBAR(user_data);

	/*
	 * ewl_seeker_increase(EWL_SEEKER(s->seeker));
	 */

	v = ewl_seeker_get_value(EWL_SEEKER(s->seeker));
	v += 0.05;

	if (v > 1.0)
		v = 1.0;

	ewl_seeker_set_value(EWL_SEEKER(s->seeker), v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
