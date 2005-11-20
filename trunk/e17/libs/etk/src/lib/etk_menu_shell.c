/** @file etk_menu_shell.c */
#include "etk_menu_shell.h"
#include <stdlib.h>
#include "etk_menu_item.h"
#include "etk_utils.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"

/**
 * @addtogroup Etk_Menu_Shell
* @{
 */

enum _Etk_Widget_Signal_Id
{
   ETK_MENU_SHELL_NUM_SIGNALS
};

static void _etk_menu_shell_constructor(Etk_Menu_Shell *menu_shell);
static void _etk_menu_shell_destructor(Etk_Menu_Shell *menu_shell);

static Etk_Signal *_etk_menu_shell_signals[ETK_MENU_SHELL_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Menu_Shell
 * @return Returns the type on an Etk_Menu_Shell
 */
Etk_Type *etk_menu_shell_type_get()
{
   static Etk_Type *menu_shell_type = NULL;

   if (!menu_shell_type)
   {
      menu_shell_type = etk_type_new("Etk_Menu_Shell", ETK_CONTAINER_TYPE, sizeof(Etk_Menu_Shell), ETK_CONSTRUCTOR(_etk_menu_shell_constructor), ETK_DESTRUCTOR(_etk_menu_shell_destructor));
   }

   return menu_shell_type;
}

/**
 * @brief Creates a new menu_shell
 * @return Returns the new menu_shell widget
 */
Etk_Widget *etk_menu_shell_new()
{
   return etk_widget_new(ETK_MENU_SHELL_TYPE, "theme_group", "menu_shell", NULL);
}

/** 
 * @brief Adds a menu_shell item at the end of the menu_shell
 * @param menu_shell a menu_shell
 * @param item the menu_shell item to add
 */
void etk_menu_shell_append(Etk_Menu_Shell *menu_shell, Etk_Menu_Item *item)
{
   if (!menu_shell || !item)
      return;
   
   etk_widget_parent_set(ETK_WIDGET(item), ETK_CONTAINER(menu_shell));
   item->parent = menu_shell;
}

/**
 * @brief Update the child items if needed. You don't need to call it manually, it's automatically called
 * @param menu shell a menu shell
 */
void etk_menu_shell_update(Etk_Menu_Shell *menu_shell)
{
   if (!menu_shell || !menu_shell->items_update)
      return;
   menu_shell->items_update(menu_shell);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the members */
static void _etk_menu_shell_constructor(Etk_Menu_Shell *menu_shell)
{
   if (!menu_shell)
      return;
   menu_shell->parent = NULL;
   menu_shell->items_update = NULL;
}

/* Destroys the menu_shell */
static void _etk_menu_shell_destructor(Etk_Menu_Shell *menu_shell)
{
   if (!menu_shell)
      return;
   /* TODO menu_shell_destructor */
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/


/**************************
 *
 * Private functions
 *
 **************************/

/** @} */
