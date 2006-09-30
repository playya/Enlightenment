#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/**
 * @return Returns a pointer to a new combo on success, NULL on failure.
 * @brief Create a new combo box
 */
Ewl_Widget *
ewl_combo_new(void)
{
	Ewl_Combo *combo;

	DENTER_FUNCTION(DLEVEL_STABLE);

	combo = NEW(Ewl_Combo, 1);
	if (!combo)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_combo_init(combo)) 
	{
		ewl_widget_destroy(EWL_WIDGET(combo));
		combo = NULL;
	}

	DRETURN_PTR(EWL_WIDGET(combo), DLEVEL_STABLE);
}

/**
 * @param combo: The Ewl_Combo to initialize
 * @return Returns TRUE on success or FALSE on failure
 * @brief Initializes a combo to default values 
 */
int
ewl_combo_init(Ewl_Combo *combo)
{

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("combo", combo, FALSE);

	if (!ewl_mvc_init(EWL_MVC(combo)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_inherit(EWL_WIDGET(combo), EWL_COMBO_TYPE);
	ewl_widget_appearance_set(EWL_WIDGET(combo), EWL_COMBO_TYPE);
	ewl_box_orientation_set(EWL_BOX(combo), EWL_ORIENTATION_HORIZONTAL);

	combo->button = ewl_button_new();
	ewl_container_child_append(EWL_CONTAINER(combo), combo->button);
	ewl_widget_appearance_set(combo->button, "decrement");
	ewl_widget_internal_set(combo->button, TRUE);
	ewl_object_alignment_set(EWL_OBJECT(combo->button), 
					EWL_FLAG_ALIGN_RIGHT);
	ewl_callback_append(combo->button, EWL_CALLBACK_CLICKED,
				ewl_combo_cb_decrement_clicked, combo);
	ewl_widget_show(combo->button);

	combo->popup = NEW(Ewl_Menu_Base, 1);
	ewl_menu_base_init(EWL_MENU_BASE(combo->popup));
	ewl_widget_internal_set(EWL_WIDGET(combo->popup), TRUE);

	combo->popup->popup = ewl_floater_new();
	ewl_floater_follow_set(EWL_FLOATER(combo->popup->popup),
						EWL_WIDGET(combo));
	ewl_widget_internal_set(combo->popup->popup, TRUE);
	ewl_widget_layer_top_set(combo->popup->popup, TRUE);
	ewl_widget_appearance_set(combo->popup->popup, EWL_IMENU_TYPE);
	ewl_box_orientation_set(EWL_BOX(combo->popup->popup),
						EWL_ORIENTATION_VERTICAL);
	ewl_object_fill_policy_set(EWL_OBJECT(combo->popup->popup),
						EWL_FLAG_FILL_NONE);
	ewl_object_alignment_set(EWL_OBJECT(combo->popup->popup),
				EWL_FLAG_ALIGN_LEFT | EWL_FLAG_ALIGN_TOP);

	/* default this to -2 so that when we first show we will set it to -1 */
	combo->selected_idx = -2; 
	ewl_callback_append(EWL_WIDGET(combo), EWL_CALLBACK_CONFIGURE,
					ewl_combo_cb_configure, NULL);
	ewl_object_fill_policy_set(EWL_OBJECT(combo), 
				EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_VSHRINK);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param combo: The Ewl_Combo to work with
 * @param idx: The data index to set selected
 * @return Returns no value
 * @brief Sets the given item in the data as selected
 */
void
ewl_combo_selected_set(Ewl_Combo *combo, int idx)
{
	Ewl_View *view;
	Ewl_Model *model;
	void *mvc_data;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("combo", combo);
	DCHECK_TYPE("combo", combo, EWL_COMBO_TYPE);

	view = ewl_mvc_view_get(EWL_MVC(combo));
	model = ewl_mvc_model_get(EWL_MVC(combo));
	mvc_data = ewl_mvc_data_get(EWL_MVC(combo));

	/* we don't bail out early as the user could have prepended widgets
	 * to their data, so the selected_idx will be the same but the
	 * widget is actually different */
	combo->selected_idx = idx;

	/* remove the previously selected value */
	if (combo->selected)
		ewl_widget_destroy(combo->selected);

	/* if we have a selected value then show it in the top, else show
	 * the header */
	if ((idx > -1) && (!combo->editable))
	{
		combo->selected = view->construct();
		view->assign(combo->selected, 
				model->fetch(mvc_data, idx, 0));
	}
	else if (view && view->header_fetch)
		combo->selected = view->header_fetch(mvc_data, 
							combo->selected_idx);

	if (combo->selected)
	{
		ewl_container_child_prepend(EWL_CONTAINER(combo), combo->selected);
		ewl_object_fill_policy_set(EWL_OBJECT(combo->selected),
							EWL_FLAG_FILL_VFILL);
		ewl_widget_show(combo->selected);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param combo: The Ewl_Combo to get the selected value from
 * @return Returns the index of the currently selected item or -1 if none
 * selected.
 * @brief Retrieves the currently selected index from the combo box
 */
int 
ewl_combo_selected_get(Ewl_Combo *combo)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("combo", combo, -1);
	DCHECK_TYPE_RET("combo", combo, EWL_COMBO_TYPE, -1);

	DRETURN_INT(combo->selected_idx, DLEVEL_STABLE);
}

/**
 * @param combo: The Ewl_Combo to use
 * @param editable: Set if the combo is editable or not
 * @return Returns no value
 * @brief This will set if the displayed data in the combo is editable. 
 *  
 * If the editable flag is set to TRUE then the combo will always request
 * the header as the item to display. If it is FALSE then the combo will use
 * the selected widget as the display.
 */
void
ewl_combo_editable_set(Ewl_Combo *combo, unsigned int editable)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("combo", combo);
	DCHECK_TYPE("combo", combo, EWL_COMBO_TYPE);

	if (combo->editable == editable)
		DRETURN(DLEVEL_STABLE);

	combo->editable = !!editable;

	/* force the selected display to change */
	ewl_combo_selected_set(combo, combo->selected_idx);	

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param combo: The Ewl_Combo to use
 * @return Returns the editable status of the combo
 * @brief Retrieves the editable status of the combo
 */
unsigned int
ewl_combo_editable_get(Ewl_Combo *combo)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("combo", combo, FALSE);
	DCHECK_TYPE_RET("combo", combo, EWL_COMBO_TYPE, FALSE);

	DRETURN_INT(combo->editable, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev: UNUSED
 * @param data: UNUSED
 * @return Returns no value
 * @brief The configure callback
 */
void
ewl_combo_cb_configure(Ewl_Widget *w, void *ev __UNUSED__, 
					void *data __UNUSED__)
{
	Ewl_Combo *combo;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);

	combo = EWL_COMBO(w);

	if (EWL_MENU_ITEM(combo->popup)->inmenu)
		ewl_floater_position_set(EWL_FLOATER(combo->popup->popup),
						CURRENT_W(w), 0);
	else
		ewl_floater_position_set(EWL_FLOATER(combo->popup->popup),
						0, CURRENT_H(w));

	if (combo->selected_idx < -1)
		ewl_combo_selected_set(combo, -1);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: UNUSED
 * @param ev: The event data
 * @param data: The combo box
 * @return Returns no value
 * @brief Callback for when the button to expand the combo is pressed
 */
void
ewl_combo_cb_decrement_clicked(Ewl_Widget *w __UNUSED__, void *ev, void *data)
{
	Ewl_Combo *combo;
	Ewl_Model *model;
	Ewl_View *view;
	void *mvc_data;
	int i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);

	combo = data;
	model = ewl_mvc_model_get(EWL_MVC(combo));
	view = ewl_mvc_view_get(EWL_MVC(combo));
	mvc_data = ewl_mvc_data_get(EWL_MVC(combo));

	/* nothing to do if we have no model/view or data */
	if (!model || !view)
		DRETURN(DLEVEL_STABLE);

	/* XXX put checks to make sure all the needed module and view
	 * function callbacks are setup */

	/* change the button appearance and expand the menu */
	ewl_widget_appearance_set(combo->button, "increment");
	ewl_callback_del(combo->button, EWL_CALLBACK_CLICKED, 
					ewl_combo_cb_decrement_clicked);
	ewl_callback_append(combo->button, EWL_CALLBACK_CLICKED,
				ewl_combo_cb_increment_clicked, combo);

	ewl_menu_base_cb_expand(EWL_WIDGET(combo->popup), ev, NULL);
	if (!REALIZED(combo->popup->popup))
	{
		Ewl_Embed *emb;
		emb = ewl_embed_widget_find(EWL_WIDGET(combo));
		ewl_container_child_append(EWL_CONTAINER(emb),
						combo->popup->popup);
	}

	if (!ewl_mvc_dirty_get(EWL_MVC(combo)))
		DRETURN(DLEVEL_STABLE);

	ewl_container_reset(EWL_CONTAINER(combo->popup));
	for (i = 0; i < model->count(mvc_data); i++)
	{
		Ewl_Widget *item;

		item = view->construct();
		view->assign(item, model->fetch(mvc_data, i, 0));
		ewl_container_child_append(EWL_CONTAINER(combo->popup), item);
		ewl_callback_append(item, EWL_CALLBACK_CLICKED,
					ewl_combo_cb_item_clicked, combo);
		ewl_widget_show(item);
	}

	ewl_mvc_dirty_set(EWL_MVC(combo), FALSE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: UNUSED
 * @param ev: UNUSED
 * @param data: The combo widget
 * @return Returns no value
 * @brief Callback for when the button to close the combo is clicked
 */
void
ewl_combo_cb_increment_clicked(Ewl_Widget *w __UNUSED__, 
				void *ev __UNUSED__, void *data)
{
	Ewl_Combo *combo;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);

	combo = data;
	ewl_widget_hide(combo->popup->popup);

	ewl_widget_appearance_set(combo->button, "decrement");
	ewl_callback_del(combo->button, EWL_CALLBACK_CLICKED,
					ewl_combo_cb_increment_clicked);
	ewl_callback_append(combo->button, EWL_CALLBACK_CLICKED,
				ewl_combo_cb_decrement_clicked, combo);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: UNUSED
 * @param ev: UNUSED
 * @param data: The combo widget
 * @return Returns no value
 * @brief Callback for when a combo item is clicked
 */
void
ewl_combo_cb_item_clicked(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
								void *data)
{
	int i;
	Ewl_Combo *combo;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);

	combo = data;

	i = ewl_container_child_index_get(EWL_CONTAINER(combo->popup), w);
	ewl_combo_selected_set(combo, i);
	ewl_combo_cb_increment_clicked(NULL, NULL, data);

	ewl_callback_call(EWL_WIDGET(combo), EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

