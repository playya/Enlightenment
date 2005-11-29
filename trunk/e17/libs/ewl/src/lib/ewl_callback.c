#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/*
 * Note: The callback list is either a single callback or a
 * pointer to an array of callbacks. This can be checked with the
 * EWL_CALLBACK_TYPE_DIRECT flag. If the list is set to direct then the list
 * itself is the callback. If it isn't direct then the list points to an
 * array of callbacks. (You can't use the length to determine this as if
 * the list has several items and you remove them down to the first item in
 * the array the list won't be direct, but will have only one item.) 
 */

static unsigned int ewl_callback_hash(void *key);
static int ewl_callback_compare(void *key1, void *key2);
static Ewl_Callback *ewl_callback_register(Ewl_Callback * cb);
static void ewl_callback_unregister(Ewl_Callback * cb);

static void ewl_callback_rm(Ewl_Widget *w, Ewl_Callback_Type t, 
						unsigned int pos);
static int ewl_callback_insert(Ewl_Widget *w, Ewl_Callback_Type t, 
				Ewl_Callback *cb, unsigned int pos);

static int callback_id = 0;
static Ecore_Hash *cb_registration = NULL;

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
 *
 *
 * Ecore_list as the callback storage with all tests open has a top line of:
 *  9121 dsinclai  15   0 71156  17m 4276 S  0.0  1.8   0:11.06 ewl_test
 * Using an array as the callback storage with all tests open has a top line of:
 * 21727 dsinclai  15   0 68360  15m 4304 S  0.0  1.5   0:09.73 ewl_test
 *
 * So using an array for the callbacks saves us about 2MB of memory in this
 * case.
 */
void
ewl_callbacks_init(void)
{
	cb_registration = ecore_hash_new(ewl_callback_hash,
				       ewl_callback_compare);
}

/**
 * @return Returns no value.
 * @brief Destroy internal registration variables for callbacks
 *
 * Destroys some important variables for tracking callbacks that allow shared
 * callbacks.
 */
void
ewl_callbacks_shutdown(void)
{
	ecore_hash_destroy(cb_registration);
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
static Ewl_Callback *
ewl_callback_register(Ewl_Callback *cb)
{
	Ewl_Callback *found;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("cb", cb, NULL);

	found = ecore_hash_get(cb_registration, cb);
	if (!found) {
		found = NEW(Ewl_Callback, 1);
		memcpy(found, cb, sizeof(Ewl_Callback));
		found->id = ++callback_id;
		ecore_hash_set(cb_registration, found, found);
	}

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
static void
ewl_callback_unregister(Ewl_Callback *cb)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("cb", cb);

	cb->references--;
	if (cb->references < 1) {
		ecore_hash_remove(cb_registration, cb);
		FREE(cb);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_callback_rm(Ewl_Widget *w, Ewl_Callback_Type t, unsigned int pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	/* don't type check here as this will get called after most of a
	 * widget is already destroyed */

	/* deal with the direct case first */
	if (w->callbacks[t].mask & EWL_CALLBACK_TYPE_DIRECT)
	{
		ewl_callback_unregister((Ewl_Callback *)w->callbacks[t].list);

		w->callbacks[t].len = 0;
		w->callbacks[t].list = NULL;
		EWL_CALLBACK_SET_NODIRECT(w, t);

		DRETURN(DLEVEL_STABLE);
	}
	ewl_callback_unregister(w->callbacks[t].list[pos]);

	/* if this will empty the list (we've already handled direct) */
	if ((EWL_CALLBACK_LEN(w, t) - 1) == 0)
	{
		w->callbacks[t].len = 0;
		w->callbacks[t].list[0] = NULL;
		FREE(w->callbacks[t].list);

		DRETURN(DLEVEL_STABLE);
	}

	/* not the last position */
	if ((int)pos != (EWL_CALLBACK_LEN(w, t) - 1))
	{
		memmove(w->callbacks[t].list + pos, 
			w->callbacks[t].list + (pos + 1), 
			(w->callbacks[t].len - pos - 1) * sizeof(void *));
	}

	w->callbacks[t].len -= 1;
	w->callbacks[t].list[EWL_CALLBACK_LEN(w, t)] = NULL;
	w->callbacks[t].list = realloc(w->callbacks[t].list, 
					w->callbacks[t].len * sizeof(void *));

	if (pos < EWL_CALLBACK_POS(w, t))
		EWL_CALLBACK_POS(w, t)--;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ewl_callback_insert(Ewl_Widget *w, Ewl_Callback_Type t, 
				Ewl_Callback *cb, unsigned int pos)
{
	Ewl_Callback *old = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, 0);
	DCHECK_PARAM_PTR_RET("cb", cb, 0);
	DCHECK_TYPE_RET("w", w, "widget", 0);

	if (EWL_CALLBACK_LEN(w, t) == 255) {
		DERROR("Maximum number of callbacks of one type exceeded on a widget\n");
		DRETURN_INT(0, DLEVEL_STABLE);
	}

	/* set direct if possible */
	if (!EWL_CALLBACK_LEN(w, t))
	{
		w->callbacks[t].list = (void *)cb;
		w->callbacks[t].len = 1;
		EWL_CALLBACK_SET_DIRECT(w, t);

		DRETURN_INT(cb->id, DLEVEL_STABLE);
	}
	w->callbacks[t].len ++;

	/* if we have a type direct then we need to save off the direct
	 * pointer and set the list to NULL so it'll be allocd' correctly */
	if (w->callbacks[t].mask & EWL_CALLBACK_TYPE_DIRECT)
	{
		old = (Ewl_Callback *)w->callbacks[t].list;
		w->callbacks[t].list = NULL;
		EWL_CALLBACK_SET_NODIRECT(w, t);
	}

	w->callbacks[t].list = realloc(w->callbacks[t].list, 
					w->callbacks[t].len * sizeof(void *));

	/* if old is set this was a direct so we can just set 0, 1 and be
	 * done with it */
	if (old)
	{
		w->callbacks[t].list[0] = (!pos ? cb : old);
		w->callbacks[t].list[1] = ( pos ? cb : old);
	}
	else
	{
		/* only have to move if we aren't at the end (of the
		 * original lenth already */
		if ((int)pos != (w->callbacks[t].len - 1))
		{
			memmove(w->callbacks[t].list + (pos + 1), 
				w->callbacks[t].list + pos, 
				(w->callbacks[t].len - 1) * sizeof(void *));
		}
		w->callbacks[t].list[pos] = cb;
	}

	if (pos < EWL_CALLBACK_POS(w, t))
		EWL_CALLBACK_POS(w, t)++;

	DRETURN_INT(cb->id, DLEVEL_STABLE);
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
ewl_callback_append(Ewl_Widget *w, Ewl_Callback_Type t,
		    Ewl_Callback_Function f, void *user_data)
{
	Ewl_Callback cb;
	Ewl_Callback *found;
	int ret;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, 0);
	DCHECK_PARAM_PTR_RET("f", f, 0);
	DCHECK_TYPE_RET("w", w, "widget", 0);

	cb.func = f;
	cb.user_data = user_data;
	cb.references = 0;
	cb.id = 0;

	found = ewl_callback_register(&cb);
	ret = ewl_callback_insert(w, t, found, EWL_CALLBACK_LEN(w, t));

	DRETURN_INT(ret, DLEVEL_STABLE);
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
ewl_callback_prepend(Ewl_Widget *w, Ewl_Callback_Type t,
		     Ewl_Callback_Function f, void *user_data)
{
	Ewl_Callback cb;
	Ewl_Callback *found;
	int ret;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, 0);
	DCHECK_PARAM_PTR_RET("f", f, 0);
	DCHECK_TYPE_RET("w", w, "widget", 0);

	cb.func = f;
	cb.user_data = user_data;
	cb.references = 0;
	cb.id = 0;

	found = ewl_callback_register(&cb);
	ret = ewl_callback_insert(w, t, found, 0);

	DRETURN_INT(ret, DLEVEL_STABLE);
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
ewl_callback_insert_after(Ewl_Widget *w, Ewl_Callback_Type t,
			  Ewl_Callback_Function f, void *user_data,
			  Ewl_Callback_Function after, void *after_data)
{
	Ewl_Callback cb;
	Ewl_Callback *found;
	Ewl_Callback *search;
	int ret, pos = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, 0);
	DCHECK_PARAM_PTR_RET("f", f, 0);
	DCHECK_TYPE_RET("w", w, "widget", 0);

	cb.func = f;
	cb.user_data = user_data;
	cb.references = 0;
	cb.id = 0;

	found = ewl_callback_register(&cb);

	/*
	 * Step 1 position past the callback we want to insert after.
	 */
	for (pos = 0; pos < EWL_CALLBACK_LEN(w, t); pos++)
	{
		search = EWL_CALLBACK_GET(w, t, pos);
		if ((search->func == after) && (search->user_data == after_data))
		{
			pos ++;
			break;
		}
	}
	ret = ewl_callback_insert(w, t, found, pos);

	DRETURN_INT(ret, DLEVEL_STABLE);
}

/**
 * @param w: the widget to execute the callbacks
 * @param t: the type of the callbacks to be executed
 * @return Returns no value.
 * @brief Execute callbacks of specified types for the widget
 *
 * Executes the callback chain for the specified widget @a w, with event @a t.
 */
void
ewl_callback_call(Ewl_Widget *w, Ewl_Callback_Type t)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

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
ewl_callback_call_with_event_data(Ewl_Widget *w, Ewl_Callback_Type t,
				  void *ev_data)
{
	Ewl_Callback *cb, *oldcb;
	Ewl_Widget *parent, *top = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	/*
	 * First search up the tree to find the topmost intercepting widget.
	 */
	parent = w->parent;
	while (parent) {
		if (EWL_CALLBACK_FLAGS(parent, t) & EWL_CALLBACK_NOTIFY_INTERCEPT)
			top = parent;
		parent = parent->parent;
	}

	if (top)
		w = top;

	/*
	 * Now search up the tree to find the first notified widget. This may
	 * result in some recursion.
	 */
	top = NULL;
	parent = w->parent;
	while (parent) {
		if (EWL_CALLBACK_FLAGS(parent, t) & EWL_CALLBACK_NOTIFY_NOTIFY)
			top = parent;
		parent = parent->parent;
	}

	if (top)
		ewl_callback_call_with_event_data(top, t, ev_data);

	/*
	 * Make sure the widget has callbacks of the given type
	 */
	if (!EWL_CALLBACK_LEN(w, t))
		DRETURN(DLEVEL_STABLE);

	/*
	 * Loop through and execute each of the callbacks of a certain type for
	 * the specified widget.
	 */
	EWL_CALLBACK_POS(w, t) = 0;
	while (EWL_CALLBACK_POS(w, t) < EWL_CALLBACK_LEN(w, t))
	{
		oldcb = cb = EWL_CALLBACK_GET(w, t, EWL_CALLBACK_POS(w, t));
		if (cb->func)
			cb->func(w, ev_data, cb->user_data);
		cb = EWL_CALLBACK_GET(w, t, EWL_CALLBACK_POS(w, t));

		if (cb == oldcb)
			EWL_CALLBACK_POS(w, t)++;
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
void
ewl_callback_del_type(Ewl_Widget *w, Ewl_Callback_Type t)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	/* don't type check this as most of the widget will probably be
	 * destroyed by the time we get here */

	if (!EWL_CALLBACK_LEN(w, t))
		DRETURN(DLEVEL_STABLE);

	while (EWL_CALLBACK_LEN(w, t))
		ewl_callback_rm(w, t, EWL_CALLBACK_LEN(w, t) - 1);

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
void
ewl_callback_del_cb_id(Ewl_Widget *w, Ewl_Callback_Type t, int cb_id)
{
	Ewl_Callback *cb;
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	if (!EWL_CALLBACK_LEN(w, t) || cb_id > callback_id)
		DRETURN(DLEVEL_STABLE);

	for (i = 0; i < EWL_CALLBACK_LEN(w, t); i++)
	{
		cb = EWL_CALLBACK_GET(w, t, i);
		if (cb->id == cb_id) {
			ewl_callback_rm(w, t, i);
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
void
ewl_callback_clear(Ewl_Widget *w)
{
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	for (i = 0; i < EWL_CALLBACK_MAX; i++) 
		ewl_callback_del_type(w, i);

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
ewl_callback_del(Ewl_Widget *w, Ewl_Callback_Type t, Ewl_Callback_Function f)
{
	Ewl_Callback *cb;
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	if (!EWL_CALLBACK_LEN(w, t))
		DRETURN(DLEVEL_STABLE);

	for (i = 0; i < EWL_CALLBACK_LEN(w, t); i++)
	{
		cb = EWL_CALLBACK_GET(w, t, i);
		if (cb->func == f) {
			ewl_callback_rm(w, t, i);
			break;
		}
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
ewl_callback_del_with_data(Ewl_Widget *w, Ewl_Callback_Type t,
			   Ewl_Callback_Function f, void *d)
{
	Ewl_Callback *cb;
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	if (!EWL_CALLBACK_LEN(w, t))
		DRETURN(DLEVEL_STABLE);

	for (i = 0; i < EWL_CALLBACK_LEN(w, t); i++)
	{
		cb = EWL_CALLBACK_GET(w, t, i);
		if ((cb->func == f) && (cb->user_data == d)) {
			ewl_callback_rm(w, t, i);
			break;
		}
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Hashes the value of a callback based on it's type, function, and user data.
 */
static unsigned int
ewl_callback_hash(void *key)
{
	Ewl_Callback *cb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("key", key, 0);

	cb = EWL_CALLBACK(key);

	DRETURN_INT((unsigned int) (cb->func) ^
		    (unsigned int) (cb->user_data), DLEVEL_STABLE);
}

/*
 * Simple comparison of callbacks, always returns -1 unless there is an exact
 * match, in which case it returns 0.
 */
static int
ewl_callback_compare(void *key1, void *key2)
{
	Ewl_Callback *cb1;
	Ewl_Callback *cb2;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("key1", key1, -1);
	DCHECK_PARAM_PTR_RET("key2", key2, -1);

	cb1 = EWL_CALLBACK(key1);
	cb2 = EWL_CALLBACK(key2);

	if ((cb1->func == cb2->func) && (cb1->user_data == cb2->user_data))
		DRETURN_INT(0, DLEVEL_STABLE);

	DRETURN_INT(-1, DLEVEL_STABLE);
}

