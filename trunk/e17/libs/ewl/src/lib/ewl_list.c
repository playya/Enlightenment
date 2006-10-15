#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a new Ewl_Widget on success or NULL on failure
 * @brief Creates and initializes a new Ewl_List widget
 */
Ewl_Widget *
ewl_list_new(void)
{
	Ewl_Widget *list = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	list = NEW(Ewl_List, 1);
	if (!list)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_list_init(EWL_LIST(list)))
	{
		ewl_widget_destroy(list);
		list = NULL;
	}

	DRETURN_PTR(list, DLEVEL_STABLE);
}

/**
 * @param list: The list to initialize
 * @return Returns TRUE on success or FALSE on failure
 * @brief Initialises an Ewl_List widget to default values
 */
int
ewl_list_init(Ewl_List *list)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("list", list, FALSE);

	if (!ewl_mvc_init(EWL_MVC(list)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_appearance_set(EWL_WIDGET(list), EWL_LIST_TYPE);
	ewl_widget_inherit(EWL_WIDGET(list), EWL_LIST_TYPE);

	ewl_mvc_selected_change_cb_set(EWL_MVC(list), ewl_list_cb_selected_change);

	ewl_callback_append(EWL_WIDGET(list), EWL_CALLBACK_CONFIGURE, 
						ewl_list_cb_configure, NULL);

	ewl_container_add_notify_set(EWL_CONTAINER(list), ewl_list_cb_child_add);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The list to be configured
 * @param ev: UNUSED
 * @param data: UNUSED
 * @return Returns no value
 * @brief Configures the given list 
 */
void
ewl_list_cb_configure(Ewl_Widget *w, void *ev __UNUSED__, 
					void *data __UNUSED__)
{
	Ewl_List *list;
	Ewl_Model *model;
	Ewl_View *view;
	void *mvc_data;
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);

	list = EWL_LIST(w);

	model = ewl_mvc_model_get(EWL_MVC(list));
	view = ewl_mvc_view_get(EWL_MVC(list));
	mvc_data = ewl_mvc_data_get(EWL_MVC(list));

	/* if either the list isn't dirty or some of the MVC controls have
	 * not been set on the list just leave this up to the box to handle */
	if ((!ewl_mvc_dirty_get(EWL_MVC(list))) 
			|| !model || !view || !mvc_data)
		DRETURN(DLEVEL_STABLE);

	/* create all the widgets and pack into the container */
	ewl_container_reset(EWL_CONTAINER(list));
	for (i = 0; i < model->count(mvc_data); i++)
	{
		Ewl_Widget *o;

		o = view->construct();
		view->assign(o, model->fetch(mvc_data, i, 0));
		ewl_widget_show(o);

		ewl_container_child_append(EWL_CONTAINER(list), o);
	}

	ewl_list_cb_selected_change(EWL_MVC(list));
	ewl_mvc_dirty_set(EWL_MVC(list), FALSE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param c: The container to work with
 * @param w: The widget that was added
 * @return Returns no value
 * @brief Adds the needed callbacks to the widget
 */
void
ewl_list_cb_child_add(Ewl_Container *c, Ewl_Widget *w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("c", c, EWL_CONTAINER_TYPE);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);

	ewl_callback_append(w, EWL_CALLBACK_CLICKED, 
				ewl_list_cb_item_clicked, c);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget that was clicked
 * @param ev: The event data
 * @param data: The list widget
 * @return Returns no value
 * @brief Sets the clicked widget as selected
 */
void
ewl_list_cb_item_clicked(Ewl_Widget *w, void *ev __UNUSED__, void *data)
{
	Ewl_List *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("data", data);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);
	DCHECK_TYPE("data", data, EWL_LIST_TYPE);

	list = data;

	ewl_mvc_selected_set(EWL_MVC(list), 
		ewl_container_child_index_get(EWL_CONTAINER(list), w), 0);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param mvc: The MVC to work with
 * @return Returns no value
 * @brief Called when the selected widgets changes
 */
void
ewl_list_cb_selected_change(Ewl_MVC *mvc)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("mvc", mvc);
	DCHECK_TYPE("mvc", mvc, EWL_MVC_TYPE);

	/* XXX selection highlight code here */

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


