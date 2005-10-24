#ifndef EWL_MENU_ITEM_H
#define EWL_MENU_ITEM_H

/**
 * @file ewl_menu_item.h
 * @defgroup Ewl_Menu_Item Menu_Item: The basic menu item
 *
 * @{
 */

/**
 * @themekey /menuitem/file
 * @themekey /menuitem/group
 */

/**
 * This serves as a basis for all menu related entries. It provides the most
 * basic layout facilities for items in a menu.
 */
typedef struct Ewl_Menu_Item Ewl_Menu_Item;

/**
 * @def EWL_MENU_ITEM(mi)
 * Typecasts a pointer to an Ewl_Menu_Item pointer.
 */
#define EWL_MENU_ITEM(mi) ((Ewl_Menu_Item *) mi)

/**
 * @struct Ewl_Menu_Item
 * Inherits from Ewl_Box to gain it's layout abilities, places policy on top
 * of the box framework to provide a simple menu layout of icon and label.
 */
struct Ewl_Menu_Item
{
        Ewl_Box         box; /**< Inherit from Ewl_Box */
        Ewl_Widget     *icon; /**< The image in this menu item */
        Ewl_Widget     *text; /**< The text label for this menu item  */
        Ewl_Widget     *inmenu; /**< Set if inside a menu */
};

Ewl_Widget     *ewl_menu_item_new(void);
int             ewl_menu_item_init(Ewl_Menu_Item *menu);
char           *ewl_menu_item_text_get(Ewl_Menu_Item *item);
void            ewl_menu_item_text_set(Ewl_Menu_Item *item, char *text);
char           *ewl_menu_item_image_get(Ewl_Menu_Item *item);
void            ewl_menu_item_image_set(Ewl_Menu_Item *item, char *image);

/*
 * internally used callbacks, override at your risk
 */
void ewl_menu_item_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_menu_item_clicked_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_menu_item_child_show_cb(Ewl_Container *parent, Ewl_Widget *child);
void ewl_menu_item_child_resize_cb(Ewl_Container *parent, Ewl_Widget *child,
		                             int size, Ewl_Orientation o);

/**
 * @}
 */
#endif

