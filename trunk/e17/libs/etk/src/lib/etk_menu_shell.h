/** @file etk_menu_shell.h */
#ifndef _ETK_MENU_SHELL_H_
#define _ETK_MENU_SHELL_H_

#include "etk_container.h"
#include "etk_types.h"

/**
 * @defgroup Etk_Menu_Shell Etk_Menu_Shell
 * @{
 */

/** @brief Gets the type of a menu_shell */
#define ETK_MENU_SHELL_TYPE       (etk_menu_shell_type_get())
/** @brief Casts the object to an Etk_Menu_Shell */
#define ETK_MENU_SHELL(obj)       (ETK_OBJECT_CAST((obj), ETK_MENU_SHELL_TYPE, Etk_Menu_Shell))
/** @brief Check if the object is an Etk_Menu_Shell */
#define ETK_IS_MENU_SHELL(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_MENU_SHELL_TYPE))

/**
 * @struct Etk_Menu_Shell
 * @brief Etk_Menu_Shell is the base class for Etk_Menu and Etk_Menu_Bar
  */
struct _Etk_Menu_Shell
{
   /* private: */
   /* Inherit from Etk_Container */
   Etk_Container container;
   
   Etk_Menu_Item *parent;
   void (*items_update)(Etk_Menu_Shell *menu_shell);
};

Etk_Type *etk_menu_shell_type_get();
Etk_Widget *etk_menu_shell_new();

void etk_menu_shell_append(Etk_Menu_Shell *menu_shell, Etk_Menu_Item *item);

void etk_menu_shell_update(Etk_Menu_Shell *menu_shell);

/** @} */

#endif
