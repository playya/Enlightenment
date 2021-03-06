#ifndef EFREET_MENU_H
#define EFREET_MENU_H

/**
 * @file efreet_menu.h
 * @brief Contains the structures and methods to support the Desktop
 * Menu Specification.
 * @addtogroup Efreet_Menu Efreet_Menu: The FDO Desktop Menu Specification
 * functions and structures
 *
 * @{
 */

/**
 * The type of entry
 */
typedef enum Efreet_Menu_Entry_Type
{
    EFREET_MENU_ENTRY_MENU,
    EFREET_MENU_ENTRY_DESKTOP,
    EFREET_MENU_ENTRY_SEPARATOR,
    EFREET_MENU_ENTRY_HEADER
} Efreet_Menu_Entry_Type;

/**
 * Efreet_Menu
 */
typedef struct Efreet_Menu Efreet_Menu;

/**
 * Efreet_Menu
 * Stores information on a entry in the menu
 */
struct Efreet_Menu
{
    Efreet_Menu_Entry_Type type;
    const char *id;   /**< File-id for desktop and relative name for menu */

    const char *name; /**< Name this entry should show */
    const char *icon; /**< Icon for this entry */

    Efreet_Desktop *desktop;   /**< The desktop we refer too */
    Eina_List      *entries;   /**< The menu items */
};


/**
 * @return Returns no value
 * @brief Initialize legacy kde support. This function blocks while
 * the kde-config script is run.
 */
EAPI int              efreet_menu_kde_legacy_init(void);

/**
 * @param name The internal name of the menu
 * @return Returns the Efreet_Menu on success or
 * NULL on failure
 * @brief Creates a new menu
 */
EAPI Efreet_Menu     *efreet_menu_new(const char *name);

/**
 * @brief Override which file is used for menu creation
 * @param file The file to use for menu creation
 *
 * This file is only used if it exists, else the standard files will be used
 * for the menu.
 */
EAPI void             efreet_menu_file_set(const char *file);

/**
 * @return Returns the Efreet_Menu_Internal representation of the default menu or
 * NULL if none found
 * @brief Creates the default menu representation
 */
EAPI Efreet_Menu     *efreet_menu_get(void);

/**
 * @param path The path of the menu to load
 * @return Returns the Efreet_Menu_Internal representation on success or NULL on
 * failure
 * @brief Parses the given .menu file and creates the menu representation
 */
EAPI Efreet_Menu     *efreet_menu_parse(const char *path);

/**
 * @param menu The menu to work with
 * @param path The path where the menu should be saved
 * @return Returns 1 on success, 0 on failure
 * @brief Saves the menu to file
 */
EAPI int              efreet_menu_save(Efreet_Menu *menu, const char *path);

/**
 * @param entry The Efreet_Menu to free
 * @return Returns no value
 * @brief Frees the given structure
 */
EAPI void             efreet_menu_free(Efreet_Menu *menu);


/**
 * @param menu The menu to work with
 * @param desktop The desktop to insert
 * @param pos The position to place the new desktop
 * @return Returns 1 on success, 0 on failure
 * @brief Insert a desktop element in a menu structure. Only accepts desktop files
 * in default directories.
 */
EAPI int              efreet_menu_desktop_insert(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop,
                                                    int pos);

/**
 * @param menu The menu to work with
 * @param desktop The desktop to remove
 * @return Returns 1 on success, 0 on failure
 * @brief Remove a desktop element in a menu structure. Only accepts desktop files
 * in default directories.
 */
EAPI int              efreet_menu_desktop_remove(Efreet_Menu *menu,
                                                    Efreet_Desktop *desktop);


/**
 * @param menu The menu to work with
 * @param menu The menu to work with
 * @param indent The indent level to print the menu at
 * @return Returns no value
 * @brief Dumps the contents of the menu to the command line
 */
EAPI void             efreet_menu_dump(Efreet_Menu *menu, const char *indent);

/**
 * @}
 */

#endif
