#include <Ewl.h>

/**
 * @return Returns a new scrollpane on success, NULL on failure.
 * @brief Create a new scrollpane
 */
Ewl_Widget     *ewl_scrollpane_new(void)
{
	Ewl_ScrollPane *s;

	DENTER_FUNCTION(DLEVEL_UNSTABLE);

	s = NEW(Ewl_ScrollPane, 1);
	if (!s)
		DRETURN_PTR(NULL, DLEVEL_UNSTABLE);

	if (!ewl_scrollpane_init(s)) {
		FREE(s);
		s = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_UNSTABLE);
}

/**
 * @param s: the scrollpane to initialize
 * @return Returns no value.
 * @brief Initialize the fields of a scrollpane
 *
 * Sets up default callbacks and field values for the scrollpane @a s.
 */
int ewl_scrollpane_init(Ewl_ScrollPane * s)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_UNSTABLE);
	DCHECK_PARAM_PTR_RET("s", s, FALSE);

	w = EWL_WIDGET(s);

	if (!ewl_container_init(EWL_CONTAINER(s), "scrollpane"))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_container_show_notify(EWL_CONTAINER(s),
				  ewl_scrollpane_child_resize_cb);
	ewl_container_resize_notify(EWL_CONTAINER(s),
				    (Ewl_Child_Resize)
				    ewl_scrollpane_child_resize_cb);
	ewl_object_set_fill_policy(EWL_OBJECT(s), EWL_FLAG_FILL_ALL);

	s->overlay = ewl_overlay_new();
	ewl_object_set_fill_policy(EWL_OBJECT(s->overlay), EWL_FLAG_FILL_ALL);

	/*
	 * Create the container to hold the contents and it's configure
	 * callback to position it's child.
	 */
	s->box = ewl_vbox_new();
	ewl_object_set_fill_policy(EWL_OBJECT(s->box), EWL_FLAG_FILL_FILL);

	/*
	 * Create the scrollbars for the scrollpane.
	 */
	s->hscrollbar = ewl_hscrollbar_new();
	s->vscrollbar = ewl_vscrollbar_new();

	/*
	 * Add the parts to the scrollpane
	 */
	ewl_container_append_child(EWL_CONTAINER(s), s->overlay);
	ewl_container_append_child(EWL_CONTAINER(s->overlay), s->box);
	ewl_container_append_child(EWL_CONTAINER(s), s->hscrollbar);
	ewl_container_append_child(EWL_CONTAINER(s), s->vscrollbar);

	ewl_widget_set_internal(s->overlay, TRUE);
	ewl_widget_set_internal(s->box, TRUE);
	ewl_widget_set_internal(s->hscrollbar, TRUE);
	ewl_widget_set_internal(s->vscrollbar, TRUE);

	ewl_widget_show(s->overlay);
	ewl_widget_show(s->box);
	ewl_widget_show(s->hscrollbar);
	ewl_widget_show(s->vscrollbar);

	ewl_container_set_redirect(EWL_CONTAINER(s), EWL_CONTAINER(s->box));

	/*
	 * Append necessary callbacks for the scrollpane.
	 */
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    ewl_scrollpane_configure_cb, NULL);

	/*
	 * We need to know whent he scrollbars have value changes in order to
	 * know when to scroll.
	 */
	ewl_callback_append(s->hscrollbar, EWL_CALLBACK_VALUE_CHANGED,
			    ewl_scrollpane_hscroll_cb, s);
	ewl_callback_append(s->vscrollbar, EWL_CALLBACK_VALUE_CHANGED,
			    ewl_scrollpane_vscroll_cb, s);

	ewl_callback_append(w, EWL_CALLBACK_MOUSE_WHEEL,
			    ewl_scrollpane_wheel_scroll_cb, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to change
 * @param f: the flags to set on the horizontal scrollbar in @a s
 * @return Returns no value.
 * @brief Set flags for horizontal scrollbar
 *
 * The scrollbar flags for the horizontal scrollbar are set to @a f.
 */
void
ewl_scrollpane_set_hscrollbar_flag(Ewl_ScrollPane * s, Ewl_ScrollBar_Flags f)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	ewl_scrollbar_set_flag(EWL_SCROLLBAR(s->hscrollbar), f);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to change
 * @param f: the flags to set on the vertical scrollbar in @a s
 * @return Returns no value.
 * @brief Set flags for vertical scrollbar
 *
 * The scrollbar flags for the vertical scrollbar are set to @a f.
 */
void
ewl_scrollpane_set_vscrollbar_flag(Ewl_ScrollPane * s, Ewl_ScrollBar_Flags f)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	ewl_scrollbar_set_flag(EWL_SCROLLBAR(s->vscrollbar), f);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to retrieve
 * @return Returns the flags of the horizontal scrollbar, 0 on failure.
 * @brief Get flags for horizontal scrollbar
 */
Ewl_ScrollBar_Flags ewl_scrollpane_get_hscrollbar_flag(Ewl_ScrollPane * s)
{
	Ewl_ScrollBar_Flags f;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0);

	f = ewl_scrollbar_get_flag(EWL_SCROLLBAR(s->hscrollbar));

	DRETURN_INT(f, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane that contains the scrollbar to retrieve
 * @return Returns the flags of the vertical scrollbar on success, 0 on failure.
 * @brief Get flags for vertical scrollbar
 */
Ewl_ScrollBar_Flags ewl_scrollpane_get_vscrollbar_flag(Ewl_ScrollPane * s)
{
	Ewl_ScrollBar_Flags f;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0);

	f = ewl_scrollbar_get_flag(EWL_SCROLLBAR(s->vscrollbar));

	DRETURN_INT(f, DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve it's horizontal scrollbar value
 * @return Returns the value of the horizontal scrollbar in @a s on success.
 * @brief Retrieves the value of the horizontal scrollbar in @a s.
 */
double ewl_scrollpane_get_hscrollbar_value(Ewl_ScrollPane *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0.0);

	DRETURN_FLOAT(ewl_scrollbar_get_value(EWL_SCROLLBAR(s->hscrollbar)),
			DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve it's vertical scrollbar value
 * @return Returns the value of the vertical scrollbar in @a s on success.
 * @brief Retrieves the value of the vertical scrollbar in @a s.
 */
double ewl_scrollpane_get_vscrollbar_value(Ewl_ScrollPane *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0.0);

	DRETURN_FLOAT(ewl_scrollbar_get_value(EWL_SCROLLBAR(s->vscrollbar)),
			DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to set the horizontal scrollbar value
 * @param val: the value to set the scrollbar too
 * @return Returns nothing
 * @brief Set the value of the horizontal scrollbar in @a s to @a val
 */
void ewl_scrollpane_set_hscrollbar_value(Ewl_ScrollPane *s, double val)
{
    DENTER_FUNCTION(DLEVEL_STABLE);
    DCHECK_PARAM_PTR("s", s);

    ewl_scrollbar_set_value(EWL_SCROLLBAR(s->hscrollbar), val);

    DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to set the vertical scrollbar value
 * @param val: the value to set the scrollbar too
 * @return Returns nothing
 * @brief Set the value of the vertical scrollbar in @a s to @a val
 */
void ewl_scrollpane_set_vscrollbar_value(Ewl_ScrollPane *s, double val)
{
    DENTER_FUNCTION(DLEVEL_STABLE);
    DCHECK_PARAM_PTR("s", s);

    ewl_scrollbar_set_value(EWL_SCROLLBAR(s->vscrollbar), val);

    DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve its vertical scrollbar stepping 
 * @return Returns the value of the stepping of the vertical scrollbar 
 *         in @a s on success.
 * @brief Retrives the value of the stepping of the vertical scrollbar in @a s.
 */
double ewl_scrollpane_get_hscrollbar_step(Ewl_ScrollPane *s) 
{
    DENTER_FUNCTION(DLEVEL_STABLE);
    DCHECK_PARAM_PTR_RET("s", s, 0.0);

    DRETURN_FLOAT(ewl_scrollbar_get_step(EWL_SCROLLBAR(s->hscrollbar)),
            DLEVEL_STABLE);
}

/**
 * @param s: the scrollpane to retrieve its vertical scrollbar stepping 
 * @return Returns the value of the stepping of the vertical scrollbar 
 *         in @a s on success.
 * @brief Retrives the value of the stepping of the vertical scrollbar in @a s.
 */
double ewl_scrollpane_get_vscrollbar_step(Ewl_ScrollPane *s) 
{
    DENTER_FUNCTION(DLEVEL_STABLE);
    DCHECK_PARAM_PTR_RET("s", s, 0.0);

    DRETURN_FLOAT(ewl_scrollbar_get_step(EWL_SCROLLBAR(s->vscrollbar)),
            DLEVEL_STABLE);
}

/*
 * Move the contents of the scrollbar into place
 */
void ewl_scrollpane_configure_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	int             b_width, b_height;
	Ewl_ScrollPane *s;
	int             vs_width = 0;
	int             hs_height = 0;
	int             content_w, content_h;
	double          step;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	s = EWL_SCROLLPANE(w);

	/*
	 * Get the space needed by the scrolbars.
	 */
	vs_width = ewl_object_get_preferred_w(EWL_OBJECT(s->vscrollbar));
	hs_height = ewl_object_get_preferred_h(EWL_OBJECT(s->hscrollbar));

	/*
	 * Determine the space used by the contents.
	 */
	content_w = CURRENT_W(w) - vs_width;
	content_h = CURRENT_H(w) - hs_height;

	/*
	 * Position the horizontal scrollbar.
	 */
	ewl_object_request_geometry(EWL_OBJECT(s->hscrollbar),
				    CURRENT_X(w), CURRENT_Y(w) + content_h,
				    content_w, hs_height);

	/*
	 * Position the vertical scrollbar.
	 */
	ewl_object_request_geometry(EWL_OBJECT(s->vscrollbar),
				    CURRENT_X(w) + content_w, CURRENT_Y(w),
				    vs_width, content_h);

	/*
	 * A rare case where we need to know the preferred size over the
	 * minimum size.
	 */
	b_width = ewl_object_get_preferred_w(EWL_OBJECT(s->box));
	b_height = ewl_object_get_preferred_h(EWL_OBJECT(s->box));

	/*
	 * Adjust the scrollbar internal stepping to match the contents.
	 */
	if (content_w < b_width) {
		double val;

		val = ewl_scrollbar_get_value(EWL_SCROLLBAR(s->hscrollbar));
		step = (double)content_w / (double)b_width;
		b_width = (int)(val * (double)(b_width - content_w));
	}
	else {
		step = 1.0;
		b_width = 0;
	}

	ewl_scrollbar_set_step(EWL_SCROLLBAR(s->hscrollbar), step);

	if (content_h < b_height) {
		double val;

		val = ewl_scrollbar_get_value(EWL_SCROLLBAR(s->vscrollbar));
		step = (double)content_h / (double)b_height;
		b_height= (int)(val * (double)(b_height - content_h));
	}
	else {
		step = 1.0;
		b_height = 0;
	}

	ewl_scrollbar_set_step(EWL_SCROLLBAR(s->vscrollbar), step);

	/*
	 * Now move the box into position. For the scrollpane to work we move
	 * the box relative to the scroll value.
	 */
	ewl_object_request_geometry(EWL_OBJECT(s->overlay),
				    CURRENT_X(w), CURRENT_Y(w),
				    content_w, content_h);
	ewl_object_request_geometry(EWL_OBJECT(s->box),
				    CURRENT_X(w) - b_width,
				    CURRENT_Y(w) - b_height,
				    content_w + b_width, content_h + b_height);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * When a horizontal scrollbar is clicked we need to move the contents of the
 * scrollpane horizontally.
 */
void ewl_scrollpane_hscroll_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_callback_call(w, EWL_CALLBACK_VALUE_CHANGED);
	ewl_widget_configure(user_data);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * When a vertical scrollbar is clicked we need to move the contents of the
 * scrollpane vertically.
 */
void ewl_scrollpane_vscroll_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("user_data", user_data);

	ewl_callback_call(user_data, EWL_CALLBACK_VALUE_CHANGED);
	ewl_widget_configure(user_data);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_scrollpane_wheel_scroll_cb(Ewl_Widget *cb, void *ev_data, void *user_data)
{
	Ewl_ScrollPane *s = EWL_SCROLLPANE(cb);
	Ewl_Event_Mouse_Wheel *ev = ev_data;

	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_scrollpane_set_vscrollbar_value(s,
			ewl_scrollpane_get_vscrollbar_value(s) +
			ev->z * ewl_scrollpane_get_vscrollbar_step(s));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
/*
 * This handles all of the various size affecting callbacks.
 */
void ewl_scrollpane_child_resize_cb(Ewl_Container * parent, Ewl_Widget * child)
{
	int pw, ph;
	Ewl_ScrollPane *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("parent", parent);
	DCHECK_PARAM_PTR("child", child);

	s = EWL_SCROLLPANE(parent);

	pw = ewl_object_get_preferred_w(EWL_OBJECT(s->hscrollbar)) +
		ewl_object_get_preferred_w(EWL_OBJECT(s->box));
	ph = ewl_object_get_preferred_h(EWL_OBJECT(s->vscrollbar)) +
		ewl_object_get_preferred_h(EWL_OBJECT(s->box));

	ewl_object_set_preferred_w(EWL_OBJECT(parent), pw);
	ewl_object_set_preferred_h(EWL_OBJECT(parent), ph);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
