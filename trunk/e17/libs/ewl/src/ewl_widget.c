
#include <Ewl.h>


extern Ewl_Widget *last_selected;
extern Ewl_Widget *last_key;
extern Ewl_Widget *last_focused;
extern Ewl_Widget *dnd_widget;

void __ewl_widget_show(Ewl_Widget * w, void *ev_data, void *user_data);
void __ewl_widget_hide_fx_clip_box(Ewl_Widget * w, void *ev_data,
				   void *user_data);
void __ewl_widget_realize(Ewl_Widget * w, void *ev_data, void *user_data);
void
__ewl_widget_configure_ebits_object(Ewl_Widget * w, void *ev_data,
				    void *user_data);
void __ewl_widget_configure_fx_clip_box(Ewl_Widget * w, void *ev_data,
					void *user_data);
void __ewl_widget_destroy(Ewl_Widget * w, void *ev_data, void *user_data);
void __ewl_widget_reparent(Ewl_Widget * w, void *ev_data, void *user_data);
void __ewl_widget_enable(Ewl_Widget * w, void *ev_data, void *user_data);
void __ewl_widget_disable(Ewl_Widget * w, void *ev_data, void *user_data);


/**
 * ewl_widget_init - initialize a widgets to default values and callbacks
 * @w: the widget to initialize
 * @appearance: the key for the widgets theme appearance
 *
 * Returns no value. The widget @w is initialized to default values and is
 * assigned the default callbacks. The appearance key is assigned for easy
 * access to theme information.
 */
void
ewl_widget_init(Ewl_Widget * w, char *appearance)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * Set up the necessary theme structures 
	 */
	ewl_theme_init_widget(w);

	w->state = EWL_STATE_NORMAL;

	/*
	 * The base appearance is used for updating the appearance of the
	 * widget
	 */
	if (appearance)
		w->appearance = strdup(appearance);

	ewl_fx_init_widget(w);

	/*
	 * Add the common callbacks that all widgets must perform
	 */
	ewl_callback_append(w, EWL_CALLBACK_SHOW, __ewl_widget_show, NULL);
	ewl_callback_append(w, EWL_CALLBACK_HIDE,
			    __ewl_widget_hide_fx_clip_box, NULL);
	ewl_callback_append(w, EWL_CALLBACK_REALIZE, __ewl_widget_realize,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    __ewl_widget_configure_fx_clip_box, NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    __ewl_widget_configure_ebits_object, NULL);
	ewl_callback_append(w, EWL_CALLBACK_THEME_UPDATE,
			    __ewl_widget_theme_update, NULL);
	ewl_callback_append(w, EWL_CALLBACK_DESTROY, __ewl_widget_destroy,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_REPARENT, __ewl_widget_reparent,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_WIDGET_ENABLE,
			    __ewl_widget_enable, NULL);
	ewl_callback_append(w, EWL_CALLBACK_WIDGET_DISABLE,
			    __ewl_widget_disable, NULL);

	/*
	 * Set size fields on the object 
	 */
	ewl_object_init(EWL_OBJECT(w));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_realize - realize the specified widget
 * @w: the widget to realize
 *
 * Returns no value. The specified widget is realized (ie. actually displayed to
 * the screen).
 */
void
ewl_widget_realize(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (REALIZED(w))
		DRETURN(DLEVEL_STABLE);

	w->visible |= EWL_VISIBILITY_REALIZED;

	ewl_callback_call(w, EWL_CALLBACK_REALIZE);
	ewl_widget_show(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_show - mark a widget as visible
 * @w: the widget to be marked as visible
 *
 * Returns no value. Marks the widget as visible so that it will be displayed
 * the next time through the rendering loop.
 */
void
ewl_widget_show(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	w->visible |= EWL_VISIBILITY_SHOWN;

	ewl_callback_call(w, EWL_CALLBACK_SHOW);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_hide - mark a widget as invisible
 * @w: the widget to be marked as invisible
 *
 * Returns no value. Marks the widget as invisible so that it will not be
 * displayed the next time through the rendering loop.
 */
void
ewl_widget_hide(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!VISIBLE(w))
		DRETURN(DLEVEL_STABLE);

	w->visible = EWL_VISIBILITY_HIDDEN;

	ewl_callback_call(w, EWL_CALLBACK_HIDE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_destroy - destroy the specified widget
 * @w: the widget to be destroyed
 *
 * Returns no value. The widget calls it's destroy callback to do any clean up
 * necessary and then free's the widget.
 */
void
ewl_widget_destroy(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (last_selected == w)
		last_selected = NULL;

	if (last_key == w)
		last_key = NULL;

	if (last_focused == w)
		last_focused = NULL;

	if (dnd_widget == w)
		dnd_widget = NULL;

	ewl_callback_call(w, EWL_CALLBACK_DESTROY);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_destroy_recursive - destroy the specified widget and it's children
 * @w: the parent widget to be destroyed
 *
 * Returns no value. The widget calls destroy recursive on any of it's child
 * widgets and then destroys itself.
 */
void
ewl_widget_destroy_recursive(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ewl_callback_call(w, EWL_CALLBACK_DESTROY_RECURSIVE);
	ewl_widget_destroy(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_configure - initiate configuring of the specified widget
 * @w: the widget to configure
 *
 * Returns no value. The configure callback is triggered for the specified
 * widget, this should adjust the widgets size and position.
 */
void
ewl_widget_configure(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (REALIZED(w) && VISIBLE(w))
		ewl_callback_call(w, EWL_CALLBACK_CONFIGURE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_theme_update - initiate theme update of the specified widget
 * @w: the widget to update the theme
 *
 * Returns no value. The theme update callback is triggered for the specified
 * widget, this should adjust the widgets appearance.
 */
void
ewl_widget_theme_update(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!REALIZED(w))
		DRETURN(DLEVEL_STABLE);

	ewl_callback_call(w, EWL_CALLBACK_THEME_UPDATE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_reparent - initiate reparent of the specified widget
 * @w: the widget to reparent
 *
 * Returns no value. The reparent callback is triggered for the specified
 * widget, this should adjust the widgets attributes based on the new parent.
 */
void
ewl_widget_reparent(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ewl_callback_call(w, EWL_CALLBACK_REPARENT);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_set_data - attach the specified key / value pair to the widget
 * @w: the widget to own the key value pair
 * @k: the key that is associated with the data
 * @v: the data that is to be tracked
 *
 * Returns no value. Assigns a key / value pair with @k as the key and @v as
 * the value to the specified widget @w.
 */
void
ewl_widget_set_data(Ewl_Widget * w, void *k, void *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("k", k);

	if (!w->data)
		w->data = ewd_hash_new(ewd_direct_hash, ewd_direct_compare);

	ewd_hash_set(w->data, k, v);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_del_data - remove the specified key / value pair from the widget
 * @w: the widget that owns the key value pair
 * @k: the key that is associated with the data
 *
 * Returns no value. Removes a key / value pair with @k as the key from the
 * specified widget @w.
 */
void
ewl_widget_del_data(Ewl_Widget * w, void *k)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("k", k);

	if (!w->data)
		DRETURN(DLEVEL_STABLE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_widget_get_data - retrieve the specified key / value pair from the widget
 * @w: the widget that owns the key value pair
 * @k: the key that is associated with the data
 *
 * Returns the value associated with @k on success, NULL on failure. Retrieves a
 * key / value pair with @k as the key from the specified widget @w.
 */
void *
ewl_widget_get_data(Ewl_Widget * w, void *k)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, NULL);
	DCHECK_PARAM_PTR_RET("k", k, NULL);

	if (!w->data)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	DRETURN_PTR(ewd_hash_get(w->data, k), DLEVEL_STABLE);
}

/**
 * ewl_widget_set_appearance - change the appearance of the specified widget
 * @w: the widget to change the appearance
 * @appearance: the new key for the widgets appearance
 *
 * Returns no value. Changes the key associated with the widgets appearance
 * and calls the theme update callback to initiate the change.
 */
void
ewl_widget_set_appearance(Ewl_Widget * w, char *appearance)
{
	char *oap;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	oap = w->appearance;

	w->appearance = (appearance ? strdup(appearance) : NULL);

	ewl_widget_theme_update(w);

	ewl_callback_call_with_event_data(w, EWL_CALLBACK_APPEARANCE_CHANGED,
					  oap);

	IF_FREE(oap);


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_widget_get_appearance - retrieve the appearance key of the widget
 * @w: the widget to retrieve the appearance key
 *
 * Returns a pointer to the appearance key string on success, NULL on failure.
 */
char *
ewl_widget_get_appearance(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("w", w, NULL);

	return (w->appearance ? strdup(w->appearance) : NULL);
}

/**
 * ewl_widget_update_appearance - update the appearance of the widget to a state
 * @w: the widget to update the appearance
 * @state: the new state of the widget
 *
 * Returns no value. Changes the appearance of the widget @w depending on the
 * state string passed by @state.
 */
void
ewl_widget_update_appearance(Ewl_Widget * w, char *state)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("state", state);

	/*
	 * Set up the ebits object on the widgets evas
	 */
	if (!w->ebits_object)
		ewl_widget_theme_update(w);

	if (!w->ebits_object)
		DRETURN(DLEVEL_STABLE);

	ebits_set_named_bit_state(w->ebits_object, "Base", state);

	DRETURN(DLEVEL_STABLE);
}

/**
 * ewl_widget_set_parent - change the parent of the specified widget
 * @w: the widget to change the parent
 * @p: the new parent of the widget
 *
 * Returns no value. Changes the parent of the widget @w, to the container @p.
 * The reparent callback is triggered to notify children of @w of the change
 * in parent.
 */
void
ewl_widget_set_parent(Ewl_Widget * w, Ewl_Widget * p)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("p", p);

	if (p->recursive && EWL_CONTAINER(p)->forward)
		w->parent = EWL_WIDGET(EWL_CONTAINER(p)->forward);
	else
		w->parent = p;

	ewl_callback_call(w, EWL_CALLBACK_REPARENT);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_widget_enable(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!(w->state & EWL_STATE_DISABLED))
		return;
	else
	  {
		  w->state |= EWL_STATE_NORMAL | ~EWL_STATE_DISABLED;
		  ewl_callback_call(w, EWL_CALLBACK_WIDGET_ENABLE);
	  }

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_widget_disable(Ewl_Widget * w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (w->state & EWL_STATE_DISABLED)
		return;
	else
	  {
		  w->state |= EWL_STATE_DISABLED | ~EWL_STATE_NORMAL;
		  ewl_callback_call(w, EWL_CALLBACK_WIDGET_DISABLE);
	  }

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Perform the series of operations common to every widget when
 * they are destroyed. This should ALWAYS be the the last callback
 * in the chain.
 */
void
__ewl_widget_destroy(Ewl_Widget * w, void *ev_data, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * Destroy the ebits object that gives the widget it's
	 * appearance
	 */
	if (w->ebits_object)
	  {
		  ebits_hide(w->ebits_object);
		  ebits_unset_clip(w->ebits_object);
		  ebits_free(w->ebits_object);
		  w->ebits_object = NULL;
	  }

	/*
	 * Destroy the fx_clip_box of the widget
	 */
	if (w->fx_clip_box)
	  {
		  evas_hide(w->evas, w->fx_clip_box);
		  evas_unset_clip(w->evas, w->fx_clip_box);
		  evas_del_object(w->evas, w->fx_clip_box);
		  w->fx_clip_box = NULL;
	  }

	/*
	 * Free up appearance related information
	 */
	ewl_theme_deinit_widget(w);
	IF_FREE(w->appearance);

	/*
	 * Clear out the callbacks, this is a bit tricky because we don't want
	 * to continue using this widget after the callbacks have been
	 * deleted. So we remove the callbacks of type destroy and then clear
	 * the remaining callbacks. This preserves the list of the destroy
	 * type so we don't get a segfault.
	 */
	ewl_callback_clear(w);

	IF_FREE(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Every widget must show it's fx_clip_box to be seen
 */
void
__ewl_widget_show(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (w->evas && w->fx_clip_box)
		evas_show(w->evas, w->fx_clip_box);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Every widget must hide it's fx_clip_box in order to hide
 */
void
__ewl_widget_hide_fx_clip_box(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (w->evas && w->fx_clip_box)
		evas_hide(w->evas, w->fx_clip_box);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Perform the basic operations necessary for realizing a widget
 */
void
__ewl_widget_realize(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Container *pc;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * Create the fx clip box where special fx can be drawn to affect the
	 * entire contents of the widget
	 */
	w->fx_clip_box = evas_add_rectangle(w->evas);
	evas_set_color(w->evas, w->fx_clip_box, 255, 255, 255, 255);
	evas_set_layer(w->evas, w->fx_clip_box, LAYER(w) - 1);

	pc = EWL_CONTAINER(w->parent);

	/*
	 * Clip the fx_clip_box to the parent clip_box.
	 */
	if (pc && pc->clip_box)
		evas_set_clip(w->evas, w->fx_clip_box, pc->clip_box);

	ewl_widget_theme_update(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Perform the basic operations necessary for configuring a widget
 */
void
__ewl_widget_configure_ebits_object(Ewl_Widget * w, void *ev_data,
				    void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * Move the base ebits object to the correct size and position
	 */
	if (w->ebits_object)
	  {
		  ebits_move(w->ebits_object, CURRENT_X(w), CURRENT_Y(w));
		  ebits_resize(w->ebits_object, CURRENT_W(w), CURRENT_H(w));
	  }

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
__ewl_widget_configure_fx_clip_box(Ewl_Widget * w, void *ev_data,
				   void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ewl_object_apply_requested(w);

	if (w->fx_clip_box)
	  {
		  evas_move(w->evas, w->fx_clip_box, CURRENT_X(w),
			    CURRENT_Y(w));
		  evas_resize(w->evas, w->fx_clip_box, CURRENT_W(w),
			      CURRENT_H(w));
	  }

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * This simplifies updating the appearance of the widget
 */
void
__ewl_widget_theme_update(Ewl_Widget * w, void *ev_data, void *user_data)
{
	int len;
	char *i;
	char *key;
	char *visible;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!w->appearance)
		DRETURN(DLEVEL_STABLE);

	/*
	 * Destroy old image (if any) 
	 */
	if (w->ebits_object)
	  {
		  ebits_hide(w->ebits_object);
		  ebits_unset_clip(w->ebits_object);
		  ebits_free(w->ebits_object);
		  w->ebits_object = NULL;
	  }

	/*
	 * Calculate the length of the base key string, then allocate the
	 * memory for it plus room for placing /visible at the end.
	 */
	len = strlen(w->appearance) + 14;
	key = NEW(char, len);

	snprintf(key, len, "%s/base/visible", w->appearance);

	/*
	 * Determine the widgets visibility, return if not visible
	 */
	visible = ewl_theme_data_get_str(w, key);

	if (!visible || !strncasecmp(visible, "no", 2))
	  {
		  FREE(key);
		  DRETURN(DLEVEL_STABLE);
	  }

	/*
	 * Retrieve the path to the ebits file that will be loaded
	 * return if no file to be loaded.
	 */
	snprintf(key, len, "%s/base", w->appearance);

	i = ewl_theme_image_get(w, key);

	if (!i)
	  {
		  FREE(key);
		  DRETURN(DLEVEL_STABLE);
	  }

	/*
	 * Load the ebits object
	 */
	w->ebits_object = ebits_load(i);

	/*
	 * Set up the ebits object on the widgets evas
	 */
	if (w->ebits_object)
	  {
		  ebits_add_to_evas(w->ebits_object, w->evas);
		  ebits_set_layer(w->ebits_object, LAYER(w));
		  if (w->fx_clip_box)
			  ebits_set_clip(w->ebits_object, w->fx_clip_box);
		  ebits_show(w->ebits_object);

		  if (w->state & EWL_STATE_DISABLED)
			  ebits_set_named_bit_state(w->ebits_object, "Base",
						    "disabled");
	  }

	FREE(key);
	FREE(i);

	DRETURN(DLEVEL_STABLE);
}

/*
 * Perform the basic operations necessary for reparenting a widget
 */
void
__ewl_widget_reparent(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!w->parent)
	  {
		  w->evas = NULL;
		  w->evas_window = 0;
		  if (w->fx_clip_box)
			  LAYER(w) = 0;

		  DRETURN(DLEVEL_STABLE);
	  }

	/*
	 * Grab the evas settings from the parent and set up the clip box
	 * again if necessary
	 */
	w->evas = EWL_WIDGET(w->parent)->evas;
	w->evas_window = EWL_WIDGET(w->parent)->evas_window;
	if (EWL_CONTAINER(w->parent)->clip_box && w->fx_clip_box)
	  {
		  evas_unset_clip(w->evas, w->fx_clip_box);
		  evas_set_clip(w->evas, w->fx_clip_box,
				EWL_CONTAINER(w->parent)->clip_box);
	  }

	LAYER(w) = LAYER(w->parent) + 5;

	if (REALIZED(w))
		ewl_widget_theme_update(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
__ewl_widget_enable(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ewl_widget_update_appearance(w, "normal");

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
__ewl_widget_disable(Ewl_Widget * w, void *ev_data, void *user_data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ewl_widget_update_appearance(w, "disabled");

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
