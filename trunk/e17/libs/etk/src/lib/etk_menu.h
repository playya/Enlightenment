/** @file etk_menu.h */
#ifndef _ETK_MENU_H_
#define _ETK_MENU_H_

#include "etk_window.h"
#include "etk_types.h"

/**
 * @defgroup Etk_Menu Etk_Menu
 * @{
 */

/** @brief Gets the type of a menu */
#define ETK_MENU_TYPE       (etk_menu_type_get())
/** @brief Casts the object to an Etk_Menu */
#define ETK_MENU(obj)       (ETK_OBJECT_CAST((obj), ETK_MENU_TYPE, Etk_Menu))
/** @brief Check if the object is an Etk_Menu */
#define ETK_IS_MENU(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_MENU_TYPE))

struct _Etk_Menu
{
   /* private: */
   /* Inherit from Etk_Window */
   Etk_Window window;
   
   Etk_Widget *vbox;
};

Etk_Type *etk_menu_type_get();
Etk_Widget *etk_menu_new();

void etk_menu_append(Etk_Menu *menu, Etk_Menu_Item *item);

void etk_menu_popup_at_xy(Etk_Menu *menu, int x, int y);
void etk_menu_popup(Etk_Menu *menu);
void etk_menu_popdown(Etk_Menu *menu);

/** @} */

#endif
