#ifndef __EWL_IMENU_H__
#define __EWL_IMENU_H__

/**
 * @file ewl_imenu.h
 * @defgroup Ewl_Imenu Imenu: A Simple Internal Menu
 * Defines a menu used internally. The contents on the menu are not drawn
 * outside of the Evas.
 *
 * @{
 */

/**
 * @themekey /imenu/file
 * @themekey /imenu/group
 */

/**
 * A simple internal menu, it is limited to drawing within the current evas.
 */
typedef struct Ewl_IMenu Ewl_IMenu;

/**
 * @def EWL_IMENU(menu)
 * Typecasts a pointer to an Ewl_IMenu pointer.
 */
#define EWL_IMENU(menu) ((Ewl_IMenu *) menu)

/**
 * @struct Ewl_IMenu
 * Inherits from the Ewl_Menu_Base and does not extend the structure, but
 * provides policy for drawing on the current evas.
 */
struct Ewl_IMenu
{
	Ewl_Menu_Base base;
};

Ewl_Widget     *ewl_imenu_new(void);
void            ewl_imenu_init(Ewl_IMenu * menu);

/*
 * Internally used callbacks, override at your own risk.
 */
void            ewl_imenu_configure_cb(Ewl_Widget *w, void *ev_data,
				       void *user_data);
void            ewl_imenu_expand_cb(Ewl_Widget * w, void *ev_data,
				    void *user_data);

/**
 * @}
 */

#endif				/* __EWL_IMENU_H__ */
