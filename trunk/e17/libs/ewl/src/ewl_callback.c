#include <Ewl.h>

static unsigned int  ewl_callback_hash(void *key);
static int           ewl_callback_compare(void *key1, void *key2);
static Ewl_Callback *ewl_callback_register(Ewl_Callback * cb);
static void          ewl_callback_unregister(Ewl_Callback * cb);

static int           callback_id = 0;
static Ewd_Hash     *cb_registration = NULL;

/**
 * @return Returns no value.
 * @brief Setup internal registration variables for callbacks
 *
 * Sets up some important variables for tracking callbacks that allow shared
 * callbacks.
 *
 * W/o shared callbacks ewl_test with all windows open has a top line of:
 * 21279 ningerso  19   0 22972  22M  9412 R     6.0  8.0   0:40 ewl_test
 * With shared callbacks ewl_test with all windows open has a top line of:
 * 15901 ningerso  10   0 20120  19M  9148 S     0.0  7.0   0:34 ewl_test
 * 
 * So using shared callbacks saves us over 2 MB of memory in this case.
 */
void ewl_callbacks_init()
{
	cb_registration = ewd_hash_new(ewl_callback_hash,
				       ewl_callback_compare);
}

/**
 * @return Returns no value.
 * @brief Destroy internal registration variables for callbacks
 *
 * Destroys some important variables for tracking callbacks that allow shared
 * callbacks.
 */
void ewl_callbacks_deinit()
{
	ewd_hash_destroy(cb_registration);
}

/*
 * ewl_callback_register - register a callback to check for duplicates
 * @cb: the callback to register
 *
 * Returns a pointer to the callback that should be used instead of the passed
 * @cb on success, NULL on failure. The returned callback may in fact be @cb,
 * but this can not be counted on. The callback @cb will be freed if this is
 * not the case.
 */
static Ewl_Callback *ewl_callback_register(Ewl_Callback * cb)
{
	Ewl_Callback   *found;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("cb", cb, NULL);

	found = ewd_hash_get(cb_registration, cb);
	if (!found) {
		cb->id = ++callback_id;
		ewd_hash_set(cb_registration, cb, cb);
		found = cb;
	} else
		FREE(cb);

	found->references++;

	DRETURN_PTR(found, DLEVEL_STABLE);
}

/*
 * ewl_callback_unregister - unreference a callback and free if appropriate
 * @cb: the callback to unregister
 *
 * Returns no value. Checks to see if @cb has nay remaining references, if not
 * it is removed from the registration system and freed.
 */
static void ewl_callback_unregister(Ewl_Callback * cb)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("cb", cb);

	cb->references--;
	if (cb->references < 1) {
		ewd_hash_remove(cb_registration, cb);
		FREE(cb);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to attach the callback
 * @param t: the type of the callback that is being attached
 * @param f: the function to attach as a callback
 * @param user_data: the data to be passed to the callback function
 * @return Returns 0 on failure, the id of the new callback on success.
 * @brief Append a callback of the specified type
 *
 * Allocates a new callback for the specified widget that calls @a f with @a
 * user_data as the data parameter when event @a ta  occurs. This event is
 * placed at the end of the callback chain.
 */
int
ewl_callback_append(Ewl_Widget * w, Ewl_Callback_Type t,
		    Ewl_Callback_Function f, void *user_data)
{
	Ewl_Callback   *cb;
	Ewd_List       *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, -1);
	DCHECK_PARAM_PTR_RET("f", f, -1);

	cb = NEW(Ewl_Callback, 1);
	if (!cb)
		DRETURN_INT(0, DLEVEL_STABLE);

	cb->func = f;
	cb->user_data = user_data;

	cb = ewl_callback_register(cb);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list) {
		list = ewd_list_new();
		EWL_CALLBACK_LIST_ASSIGN(w, t, list);
	}

	ewd_list_append(list, cb);

	DRETURN_INT(cb->id, DLEVEL_STABLE);
}

/**
 * @param w: the widget to attach the callback
 * @param t: the type of the callback that is being attached
 * @param f: the function to attach as a callback
 * @param user_data: the data to be passed to the callback function
 * @return Returns 0 on failure, the id of the new callback on success.
 * @brief prepend a callback of the specified type
 *
 * Same functionality as ewl_callback_append, but the callback is placed at the
 * beginning of the callback chain.
 */
int
ewl_callback_prepend(Ewl_Widget * w, Ewl_Callback_Type t,
		     Ewl_Callback_Function f, void *user_data)
{
	Ewl_Callback   *cb;
	Ewd_List       *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, -1);
	DCHECK_PARAM_PTR_RET("f", f, -1);

	cb = NEW(Ewl_Callback, 1);
	if (!cb)
		DRETURN_INT(0, DLEVEL_STABLE);

	cb->func = f;
	cb->user_data = user_data;

	cb = ewl_callback_register(cb);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list) {
		list = ewd_list_new();
		EWL_CALLBACK_LIST_ASSIGN(w, t, list);
	}

	ewd_list_prepend(list, cb);

	DRETURN_INT(cb->id, DLEVEL_STABLE);
}

/**
 * @param w: the widget to insert the callback
 * @param t: the type of the callback that is being attached
 * @param f: the function to attach as a callback
 * @param user_data: the data to be passed to the callback function
 * @param after: the function of the callback to append after
 * @param after_data: the user data of the callback to append after
 * @return Returns 0 on failure, the id of the new callback on success.
 * @brief Add a callback after a previous callback in list
 *
 * Same functionality as ewl_callback_append, but the callback is placed after
 * the specified callback on the callback chain.
 */
int
ewl_callback_insert_after(Ewl_Widget * w, Ewl_Callback_Type t,
			  Ewl_Callback_Function f, void *user_data,
			  Ewl_Callback_Function after, void *after_data)
{
	Ewl_Callback   *cb;
	Ewl_Callback   *search;
	Ewd_List       *list;

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("w", w, 0);
	DCHECK_PARAM_PTR_RET("f", f, 0);

	cb = NEW(Ewl_Callback, 1);
	if (!cb)
		DRETURN_INT(0, DLEVEL_STABLE);

	cb->func = f;
	cb->user_data = user_data;

	cb = ewl_callback_register(cb);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list) {
		list = ewd_list_new();
		EWL_CALLBACK_LIST_ASSIGN(w, t, list);
	}

	/*
	 * Step 1 position past the callback we want to insert after.
	 */
	ewd_list_goto_first(list);
	while ((search = ewd_list_next(list)) &&
	       (search->func != f || search->user_data != after_data));

	ewd_list_insert(list, cb);

	DRETURN_INT(cb->id, DLEVEL_STABLE);
}

/**
 * @param w: the widget to execute the callbacks
 * @param t: the type of the callbacks to be executed
 * @return Returns no value.
 * @brief Execute callbacks of specified types for the widget
 *
 * Executes the callback chain for the specified widget @a w, with event @a t.
 */
void ewl_callback_call(Ewl_Widget * w, Ewl_Callback_Type t)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_callback_call_with_event_data(w, t, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to execute the callbacks
 * @param t: the type of the callbacks to be executed
 * @param ev_data: the event data to pass to the callbacks
 * @return Returns no value.
 * @brief Execute callbacks with event data
 *
 * Similar to ewl_callback_call, but the event data is substituted by @a
 * ev_data.
 */
void
ewl_callback_call_with_event_data(Ewl_Widget * w, Ewl_Callback_Type t,
				  void *ev_data)
{
	Ewd_List       *list;
	Ewl_Callback   *cb;
	Ewl_Widget     *parent, *top = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * First search up the tree to find the topmost intercepting widget.
	 */
	parent = w->parent;
	while (parent) {
		if (EWL_CALLBACK_FLAGS(parent, t) & EWL_CALLBACK_NOTIFY_INTERCEPT)
			top = parent;
		parent = parent->parent;
	}

	if (top) {
		ewl_callback_call_with_event_data(top, t, ev_data);
		DRETURN(DLEVEL_STABLE);
	}

	/*
	 * Now search up the tree to find the first notified widget. This may
	 * result in some recursion.
	 */
	parent = w->parent;
	while (parent) {
		if (EWL_CALLBACK_FLAGS(parent, t) & EWL_CALLBACK_NOTIFY_NOTIFY)
			top = parent;
		parent = parent->parent;
	}

	if (top)
		ewl_callback_call_with_event_data(top, t, ev_data);

	/*
	 * Finally execute the callback on the current widget
	 */
	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list || ewd_list_is_empty(list))
		DRETURN(DLEVEL_STABLE);

	/*
	 * Loop through and execute each of the callbacks of a certain type for
	 * the specified widget.
	 */
	ewd_list_goto_first(list);
	while (list && (cb = ewd_list_next(list))) {
		if (cb->func)
			cb->func(w, ev_data, cb->user_data);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to delete the callbacks
 * @param t: the type of the callbacks to be deleted
 * @return Returns no value.
 * @brief Delete all callbacks of the specified type
 *
 * Delete all callbacks of type @a t from widget @a w.
 */
void ewl_callback_del_type(Ewl_Widget * w, Ewl_Callback_Type t)
{
	Ewd_List       *list;
	Ewl_Callback   *rm;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list)
		DRETURN(DLEVEL_STABLE);

	while ((rm = ewd_list_remove_first(list)))
		ewl_callback_unregister(rm);

	ewd_list_destroy(list);
	EWL_CALLBACK_LIST_ASSIGN(w, t, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to delete the id
 * @param t: the type of event the callback is attached to
 * @param cb_id: the id of the callback to delete
 * @return Returns no value.
 * @brief Delete the specified callback id from the widget
 *
 * Delete the specified callback id from the widget @a w.
 */
void ewl_callback_del_cb_id(Ewl_Widget * w, Ewl_Callback_Type t, int cb_id)
{
	Ewd_List       *list;
	Ewl_Callback   *cb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list ||
	    ewd_list_is_empty(list) || cb_id > callback_id)
		DRETURN(DLEVEL_STABLE);

	while ((cb = ewd_list_next(list))) {
		if (cb->id == cb_id) {
			ewd_list_remove(list);
			ewl_callback_unregister(cb);
			break;
		}
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * @param w: the widget to remove the callbacks
 * @return Returns no value.
 * @brief Remove all callbacks from the specified widget
 *
 * Removes and frees all callbacks associated with widget @a w.
 */
void ewl_callback_clear(Ewl_Widget * w)
{
	int             i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	for (i = 0; i < EWL_CALLBACK_MAX; i++) {
		if (EWL_CALLBACK_LIST_POINTER(w, i))
			ewl_callback_del_type(w, i);
	}


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to delete the callback
 * @param t: the type of event associated with the callback
 * @param f: the function called by the callback
 * @brief Delete the specified callback function from the widget
 *
 * @return Returns no value.
 * Delete and frees the callback that calls function @a f when event @a t occurs
 * to widget @a w.
 */
void
ewl_callback_del(Ewl_Widget * w, Ewl_Callback_Type t, Ewl_Callback_Function f)
{
	Ewl_Callback   *cb;
	Ewd_List       *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list || ewd_list_is_empty(list))
		DRETURN(DLEVEL_STABLE);

	ewd_list_goto_first(list);

	while ((cb = ewd_list_current(list)) != NULL) {
		if (cb->func == f) {
			ewd_list_remove(list);
			ewl_callback_unregister(cb);
			break;
		}

		ewd_list_next(list);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param w: the widget to delete the callback
 * @param t: the type of event associated with the callback
 * @param f: the function called by the callback
 * @param d: the data passed to the callback
 * @brief Delete the specified callback function from the widget
 *
 * @return Returns no value.
 * Delete and frees the callback that calls function @a f when event @a t occurs
 * to widget @a w.
 */
void
ewl_callback_del_with_data(Ewl_Widget * w, Ewl_Callback_Type t,
			   Ewl_Callback_Function f, void *d)
{
	Ewl_Callback   *cb;
	Ewd_List       *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	list = EWL_CALLBACK_LIST_POINTER(w, t);

	if (!list || ewd_list_is_empty(list))
		DRETURN(DLEVEL_STABLE);

	ewd_list_goto_first(list);

	while ((cb = ewd_list_current(list)) != NULL) {
		if (cb->func == f && cb->user_data == d) {
			ewd_list_remove(list);
			ewl_callback_unregister(cb);
			break;
		}

		ewd_list_next(list);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Hashes the value of a callback based on it's type, function, and user data.
 */
static unsigned int ewl_callback_hash(void *key)
{
	Ewl_Callback   *cb = EWL_CALLBACK(key);

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("key", key, 0);

	DRETURN_INT((unsigned int) (cb->func) ^
		    (unsigned int) (cb->user_data), DLEVEL_STABLE);
}

/*
 * Simple comparison of callbacks, always returns -1 unless there is an exact
 * match, in which case it returns 0.
 */
static int ewl_callback_compare(void *key1, void *key2)
{
	Ewl_Callback   *cb1 = EWL_CALLBACK(key1);
	Ewl_Callback   *cb2 = EWL_CALLBACK(key2);

	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("key1", key1, -1)
	    DCHECK_PARAM_PTR_RET("key2", key2, -1)
	    if (cb1->func == cb2->func && cb1->user_data == cb2->user_data)
		DRETURN_INT(0, DLEVEL_STABLE);

	DRETURN_INT(-1, DLEVEL_STABLE);
}
