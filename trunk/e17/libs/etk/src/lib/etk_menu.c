/** @file etk_menu.c */
#include "etk_menu.h"
#include <stdlib.h>
#include "etk_popup_window.h"
#include "etk_menu_item.h"
#include "etk_utils.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"

/**
 * @addtogroup Etk_Menu
 * @{
 */

enum Etk_Menu_Signal_Id
{
   ETK_MENU_POPPED_DOWN_SIGNAL,
   ETK_MENU_POPPED_UP_SIGNAL,
   ETK_MENU_NUM_SIGNALS
};

static void _etk_menu_constructor(Etk_Menu *menu);
static void _etk_menu_destructor(Etk_Menu *menu);
static void _etk_menu_size_request(Etk_Widget *widget, Etk_Size *size);
static void _etk_menu_size_allocate(Etk_Widget *widget, Etk_Geometry geometry);
static void _etk_menu_window_popped_up_cb(Etk_Object *object, void *data);
static void _etk_menu_window_popped_down_cb(Etk_Object *object, void *data);
static void _etk_menu_window_key_down_cb(Etk_Object *object, void *event_info, void *data);
static void _etk_menu_item_added_cb(Etk_Object *object, void *item, void *data);
static void _etk_menu_item_removed_cb(Etk_Object *object, void *item, void *data);
static void _etk_menu_item_enter_cb(Etk_Object *object, void *data);
static void _etk_menu_item_leave_cb(Etk_Object *object, void *data);
static void _etk_menu_item_mouse_up_cb(Etk_Object *object, void *event, void *data);
static void _etk_menu_item_selected_cb(Etk_Object *object, void *data);
static void _etk_menu_item_deselected_cb(Etk_Object *object, void *data);
static void _etk_menu_item_activated_cb(Etk_Object *object, void *data);
static void _etk_menu_item_submenu_changed_cb(Etk_Object *object, const char *property_name, void *data);

static Etk_Signal *_etk_menu_signals[ETK_MENU_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Menu
 * @return Returns the type of an Etk_Menu
 */
Etk_Type *etk_menu_type_get()
{
   static Etk_Type *menu_type = NULL;

   if (!menu_type)
   {
      menu_type = etk_type_new("Etk_Menu", ETK_MENU_SHELL_TYPE, sizeof(Etk_Menu),
         ETK_CONSTRUCTOR(_etk_menu_constructor), ETK_DESTRUCTOR(_etk_menu_destructor));
      
      _etk_menu_signals[ETK_MENU_POPPED_UP_SIGNAL] = etk_signal_new("popped_up",
         menu_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
      _etk_menu_signals[ETK_MENU_POPPED_DOWN_SIGNAL] = etk_signal_new("popped_down",
         menu_type, -1, etk_marshaller_VOID__VOID, NULL, NULL);
   }

   return menu_type;
}

/**
 * @brief Creates a new menu
 * @return Returns the new menu widget
 */
Etk_Widget *etk_menu_new()
{
   return etk_widget_new(ETK_MENU_TYPE, "theme_group", "menu", NULL);
}

/**
 * @brief Pops up the menu at the position (x, y)
 * @param menu a menu
 * @param x the x component of the position where to popup the menu
 * @param y the y component of the position where to popup the menu
 */
void etk_menu_popup_at_xy(Etk_Menu *menu, int x, int y)
{
   if (!menu)
      return;
   etk_popup_window_popup_at_xy(menu->window, x, y);
}

/**
 * @brief Pops up the menu at the mouse position
 * @param menu a menu
 */
void etk_menu_popup(Etk_Menu *menu)
{
   if (!menu)
      return;
   etk_popup_window_popup(menu->window);
}

/**
 * @brief Pops down the menu and all its submenus (menus attached to its items)
 * @param menu a menu
 */
void etk_menu_popdown(Etk_Menu *menu)
{
   if (!menu)
      return;
   etk_popup_window_popdown(menu->window);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the menu */
static void _etk_menu_constructor(Etk_Menu *menu)
{
   if (!menu)
      return;
      
   menu->window = ETK_POPUP_WINDOW(etk_widget_new(ETK_POPUP_WINDOW_TYPE, NULL));
   etk_signal_connect("popped_up", ETK_OBJECT(menu->window), ETK_CALLBACK(_etk_menu_window_popped_up_cb), menu);
   etk_signal_connect("popped_down", ETK_OBJECT(menu->window), ETK_CALLBACK(_etk_menu_window_popped_down_cb), menu);
   etk_signal_connect("key_down", ETK_OBJECT(menu->window), ETK_CALLBACK(_etk_menu_window_key_down_cb), menu);
   
   etk_container_add(ETK_CONTAINER(menu->window), ETK_WIDGET(menu));
   ETK_WIDGET(menu)->size_request = _etk_menu_size_request;
   ETK_WIDGET(menu)->size_allocate = _etk_menu_size_allocate;
   
   etk_signal_connect("item_added", ETK_OBJECT(menu), ETK_CALLBACK(_etk_menu_item_added_cb), menu);
   etk_signal_connect("item_removed", ETK_OBJECT(menu), ETK_CALLBACK(_etk_menu_item_removed_cb), NULL);
}

/* Destroys the menu */
static void _etk_menu_destructor(Etk_Menu *menu)
{
   if (!menu)
      return;
   etk_container_remove(ETK_CONTAINER(menu->window), ETK_WIDGET(menu));
   etk_object_destroy(ETK_OBJECT(menu->window));
}

/* Calculates the ideal size of the menu */
static void _etk_menu_size_request(Etk_Widget *widget, Etk_Size *size)
{
   Evas_List *l;
   Etk_Menu_Shell *menu_shell;
   
   if (!(menu_shell = ETK_MENU_SHELL(widget)) || !size)
      return;
   
   size->w = 0;
   size->h = 0;
   for (l = menu_shell->items; l; l = l->next)
   {
      Etk_Size child_size;
      
      etk_widget_size_request(ETK_WIDGET(l->data), &child_size);
      size->w = ETK_MAX(size->w, child_size.w);
      size->h += child_size.h;
   }
   /* TODO: FIXME: incorrect calculated width */
   size->w += 15;
}

/* Resizes the menu to the allocated size */
static void _etk_menu_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   Etk_Geometry child_geometry;
   Etk_Menu_Shell *menu_shell;
   Evas_List *l;
   Etk_Menu_Item *item;
   int y_offset;
   Etk_Bool items_have_left_widget = ETK_FALSE;
   Etk_Bool items_have_submenu = ETK_FALSE;
   
   if (!(menu_shell = ETK_MENU_SHELL(widget)))
      return;
   
   for (l = menu_shell->items; l; l = l->next)
   {
      item = ETK_MENU_ITEM(l->data);
      if (item->left_widget)
         items_have_left_widget = ETK_TRUE;
      if (item->submenu)
         items_have_submenu = ETK_TRUE;
   }
   
   for (l = menu_shell->items; l; l = l->next)
   {
      item = ETK_MENU_ITEM(l->data);
   
      if (items_have_submenu)
         etk_widget_theme_signal_emit(ETK_WIDGET(item), item->submenu ? "arrow_show" : "arrow_spacer");
      else
         etk_widget_theme_signal_emit(ETK_WIDGET(item), "arrow_hide");
      
      etk_widget_theme_signal_emit(ETK_WIDGET(item), items_have_left_widget ? "left_widget_show" : "left_widget_hide");
   }
   
   y_offset = geometry.y;
   child_geometry.x = geometry.x;
   for (l = menu_shell->items; l; l = l->next)
   {
      Etk_Size child_size;
      
      item = ETK_MENU_ITEM(l->data);
      etk_widget_size_request(ETK_WIDGET(item), &child_size);
      child_geometry.y = y_offset;
      child_geometry.w = geometry.w;
      child_geometry.h = child_size.h;
      
      etk_widget_size_allocate(ETK_WIDGET(item), child_geometry);
      y_offset += child_geometry.h;
   }
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the menu window is popped up */
static void _etk_menu_window_popped_up_cb(Etk_Object *object, void *data)
{
   Etk_Widget *menu_widget;
   
   if (!(menu_widget = ETK_WIDGET(data)))
      return;
   
   etk_widget_show(menu_widget);
   etk_signal_emit(_etk_menu_signals[ETK_MENU_POPPED_UP_SIGNAL], ETK_OBJECT(menu_widget), NULL);
   if (ETK_MENU_SHELL(menu_widget)->parent)
      etk_signal_emit_by_name("submenu_popped_up", ETK_OBJECT(ETK_MENU_SHELL(menu_widget)->parent), NULL);
}

/* Called when the menu window is popped down */
static void _etk_menu_window_popped_down_cb(Etk_Object *object, void *data)
{
   Evas_List *l;
   Etk_Menu_Shell *menu_shell;
   
   if (!(menu_shell = ETK_MENU_SHELL(data)))
      return;
   
   for (l = menu_shell->items; l; l = l->next)
      etk_menu_item_deselect(ETK_MENU_ITEM(l->data));
   
   etk_signal_emit(_etk_menu_signals[ETK_MENU_POPPED_DOWN_SIGNAL], ETK_OBJECT(menu_shell), NULL);
   if (menu_shell->parent)
      etk_signal_emit_by_name("submenu_popped_down", ETK_OBJECT(menu_shell->parent), NULL);
}

/* Called when a key is pressed on the menu window */
static void _etk_menu_window_key_down_cb(Etk_Object *object, void *event_info, void *data)
{
   Etk_Menu_Shell *menu_shell;
   Evas_Event_Key_Down *event;
   
   if (!(menu_shell = ETK_MENU_SHELL(data)) || !(event = event_info))
      return;
   
   /* TODO: keyboard navigation */
}

/* Called when an item is added to the menu */
static void _etk_menu_item_added_cb(Etk_Object *object, void *item, void *data)
{
   Etk_Object *item_object;
   
   if (!(item_object = ETK_OBJECT(item)))
      return;
   
   etk_signal_connect("enter", item_object, ETK_CALLBACK(_etk_menu_item_enter_cb), NULL);
   etk_signal_connect("leave", item_object, ETK_CALLBACK(_etk_menu_item_leave_cb), NULL);
   etk_signal_connect("mouse_up", item_object, ETK_CALLBACK(_etk_menu_item_mouse_up_cb), NULL);
   etk_signal_connect("selected", item_object, ETK_CALLBACK(_etk_menu_item_selected_cb), NULL);
   etk_signal_connect("deselected", item_object, ETK_CALLBACK(_etk_menu_item_deselected_cb), NULL);
   etk_signal_connect("activated", item_object, ETK_CALLBACK(_etk_menu_item_activated_cb), NULL);
   etk_object_notification_callback_add(item_object, "submenu", _etk_menu_item_submenu_changed_cb, data);
}

/* Called when an item is removed from the menu */
static void _etk_menu_item_removed_cb(Etk_Object *object, void *item, void *data)
{
   Etk_Object *item_object;
   
   if (!(item_object = ETK_OBJECT(item)))
      return;
   
   etk_signal_disconnect("enter", item_object, ETK_CALLBACK(_etk_menu_item_enter_cb));
   etk_signal_disconnect("leave", item_object, ETK_CALLBACK(_etk_menu_item_leave_cb));
   etk_signal_disconnect("mouse_up", item_object, ETK_CALLBACK(_etk_menu_item_mouse_up_cb));
   etk_signal_disconnect("selected", item_object, ETK_CALLBACK(_etk_menu_item_selected_cb));
   etk_signal_disconnect("deselected", item_object, ETK_CALLBACK(_etk_menu_item_deselected_cb));
   etk_signal_disconnect("activated", item_object, ETK_CALLBACK(_etk_menu_item_activated_cb));
   etk_object_notification_callback_remove(item_object, "submenu", _etk_menu_item_submenu_changed_cb);
}

/* Called when the mouse pointer enters the item */ 
static void _etk_menu_item_enter_cb(Etk_Object *object, void *data)
{
   etk_menu_item_select(ETK_MENU_ITEM(object));
}

/* Called when the mouse pointer leaves the item */
static void _etk_menu_item_leave_cb(Etk_Object *object, void *data)
{
   Etk_Menu_Item *item;
   
   if (!(item = ETK_MENU_ITEM(object)) || item->submenu)
      return;
   etk_menu_item_deselect(item);
}

/* Called when the user has released the item */
static void _etk_menu_item_mouse_up_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Menu_Item *item;
   
   if (!(item = ETK_MENU_ITEM(object)) || item->submenu)
      return;
   etk_menu_item_activate(item);
}

/* Called when the item is selected */
static void _etk_menu_item_selected_cb(Etk_Object *object, void *data)
{
   Etk_Menu_Item *item;
   Etk_Menu *menu;
   Evas_List *l;
   
   if (!(item = ETK_MENU_ITEM(object)) || !(menu = ETK_MENU(item->parent)))
      return;

   /* First, we deactivate all the items that are on the same menu than the item */
   for (l = ETK_MENU_SHELL(menu)->items; l; l = l->next)
   {
      if (ETK_MENU_ITEM(l->data) == item)
         continue;
      etk_menu_item_deselect(ETK_MENU_ITEM(l->data));
   }
   
   /* Then we popup the child menu */
   if (item->submenu)
   {
      int mx, my, mw, item_y;
      
      item_y = ETK_WIDGET(item)->geometry.y;
      etk_window_geometry_get(ETK_WINDOW(menu->window), &mx, &my, &mw, NULL);
      etk_menu_popup_at_xy(item->submenu, mx + mw, my + item_y);
   }
}

/* Called when the item is deselected */
static void _etk_menu_item_deselected_cb(Etk_Object *object, void *data)
{
   Etk_Menu_Item *item;
   
   if (!(item = ETK_MENU_ITEM(object)))
      return;
   
   if (item->submenu)
      etk_menu_popdown(item->submenu);
}

/* Called when the item is activated */
static void _etk_menu_item_activated_cb(Etk_Object *object, void *data)
{
   etk_popup_window_popdown_all();
}

/* Called when the submenu of an item of the menu has been changed */
static void _etk_menu_item_submenu_changed_cb(Etk_Object *object, const char *property_name, void *data)
{
   Etk_Menu *menu, *submenu;
   Etk_Menu_Item *item;
   
   if (!(item = ETK_MENU_ITEM(object)) || !(menu = ETK_MENU(data)))
      return;
   
   if ((submenu = etk_menu_item_submenu_get(item)))
      etk_popup_window_parent_set(submenu->window, menu->window);
   
   //TODO: Remove the parent of the previous submenu
}

/** @} */

/**************************
 *
 * Documentation
 *
 **************************/

/**
 * @addtogroup Etk_Menu
 *
 * @image html widgets/menu.png
 * The items of the menu are packed vertically. @n
 * To add or remove items, you have to use the functions provided by the Etk_Menu_Shell:
 * etk_menu_shell_append(), etk_menu_shell_remove(), ... @n @n
 * A menu is usually popped up by clicking on an item of a menu bar, or by activating an item of another menu. @n
 * You can also pop up a menu at the mouse position with etk_menu_popup() or at a specific position with
 * etk_menu_popup_xy(). The menu could then be popped down with etk_menu_popdown().
 * 
 * 
 * \par Object Hierarchy:
 * - Etk_Object
 *   - Etk_Widget
 *     - Etk_Menu_Shell
 *       - Etk_Menu
 *
 * \par Signals:
 * @signal_name "popped_up": Emitted when the the menu has been popped up
 * @signal_cb void callback(Etk_Menu *menu, void *data)
 * @signal_arg menu: the menu that has been popped up
 * @signal_data
 * \par
 * @signal_name "popped_down": Emitted when the the menu has been popped down
 * @signal_cb void callback(Etk_Menu *menu, void *data)
 * @signal_arg menu: the menu that has been popped down
 * @signal_data
 */
