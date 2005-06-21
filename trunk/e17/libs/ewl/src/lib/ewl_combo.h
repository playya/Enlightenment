#ifndef __EWL_COMBO_H__
#define __EWL_COMBO_H__

/**
 * @file ewl_combo.h
 * @defgroup Ewl_Combo Combo: A Simple Internal Combo Box
 * Defines a combo box used internally. The contents on the box are not drawn
 * outside of the Evas.
 *
 * @{
 */

/**
 * @themekey /combo/file
 * @themekey /combo/group
 */

/**
 * A simple internal combo box, it is limited to drawing within the current
 * evas.
 */
typedef struct Ewl_Combo Ewl_Combo;

/**
 * @def EWL_COMBO(combo)
 * Typecasts a pointer to an Ewl_Combo pointer.
 */
#define EWL_COMBO(combo) ((Ewl_Combo *) combo)

/**
 * @struct Ewl_Combo
 * Inherits from the Ewl_Menu_Base and does not extend the structure, but
 * provides policy for drawing on the current evas.
 */
struct Ewl_Combo
{
	Ewl_Menu_Base base; /**< Inherits from menu_base. */
	Ewl_Widget *button; /**< The button used to expand/collapse the selector. */
	Ewl_Widget *selected; /**< An entry that contains the current selection. */
};

Ewl_Widget     *ewl_combo_new(char *title);
void            ewl_combo_init(Ewl_Combo * combo, char *title);
Ewl_Widget     *ewl_combo_selected_get(Ewl_Combo * combo);
void            ewl_combo_selected_set(Ewl_Combo * combo, Ewl_Widget *item);

/*
 * Internally used callbacks, override at your own risk.
 */
void ewl_combo_item_select_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_combo_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_combo_value_changed_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_combo_expand_cb(Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_combo_collapse_cb(Ewl_Widget * w, void *ev_data, void *user_data);

/**
 * @}
 */

#endif				/* __EWL_COMBO_H__ */
