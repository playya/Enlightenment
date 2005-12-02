#include "etk_test.h"
#include "config.h"

typedef enum _Etk_Test_Menu_Item_Type
{
   ETK_TEST_MENU_ITEM_NORMAL,
   ETK_TEST_MENU_ITEM_SEPARATOR
} Etk_Test_Menu_Item_Type;

static void _etk_test_menu_window_down_cb(Etk_Object *object, void *event_info, void *data);
static Etk_Widget *_etk_test_menu_item_new(Etk_Test_Menu_Item_Type item_type, const char *label,
   Etk_Stock_Id stock_id, Etk_Menu_Shell *menu_shell, Etk_Widget *statusbar);
static void _etk_test_menu_item_selected_cb(Etk_Object *object, void *data);
static void _etk_test_menu_item_deselected_cb(Etk_Object *object, void *data);

/* Creates the window for the menu test */
void etk_test_menu_window_create(void *data)
{
   static Etk_Widget *win = NULL;
   Etk_Widget *vbox;
   Etk_Widget *menu_bar;
   Etk_Widget *menu;
   Etk_Widget *menu_item;
   Etk_Widget *label;
   Etk_Widget *statusbar;

   if (win)
   {
      etk_widget_show_all(ETK_WIDGET(win));
      return;
   }
   
   win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(win), _("Etk Menu Test"));
   etk_signal_connect("delete_event", ETK_OBJECT(win), ETK_CALLBACK(etk_window_hide_on_delete), NULL);
   etk_widget_size_request_set(win, 300, 200);
   
   /****************
    * The window
    ****************/
   vbox = etk_vbox_new(FALSE, 0);
   etk_container_add(ETK_CONTAINER(win), vbox);
   
   menu_bar = etk_menu_bar_new();
   etk_box_pack_start(ETK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
   
   label = etk_label_new(_("Click me :)"));
   etk_label_alignment_set(ETK_LABEL(label), 0.5, 0.5);
   etk_widget_pass_events_set(label, TRUE);
   etk_box_pack_start(ETK_BOX(vbox), label, TRUE, TRUE, 0);
   
   statusbar = etk_statusbar_new();
   etk_box_pack_end(ETK_BOX(vbox), statusbar, FALSE, FALSE, 0);
   
   /****************
    * Menu Bar
    ****************/
   /* File Menu */
   menu_item = _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("File"), -1, ETK_MENU_SHELL(menu_bar), NULL);
   menu = etk_menu_new();
   etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Open"), ETK_STOCK_DOCUMENT_OPEN, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Save"), ETK_STOCK_DOCUMENT_SAVE, ETK_MENU_SHELL(menu), statusbar);
   
   /* Edit Menu */
   menu_item = _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Edit"), -1, ETK_MENU_SHELL(menu_bar), NULL);
   menu = etk_menu_new();
   etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Cut"), ETK_STOCK_EDIT_CUT, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Copy"), ETK_STOCK_EDIT_COPY, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Paste"), ETK_STOCK_EDIT_PASTE, ETK_MENU_SHELL(menu), statusbar);
   
   /* Help Menu */
   menu_item = _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Help"), -1, ETK_MENU_SHELL(menu_bar), NULL);
   menu = etk_menu_new();
   etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("About"), -1, ETK_MENU_SHELL(menu), statusbar);

   /****************
    * Popup Menu
    ****************/
   /* Main menu */
   menu = etk_menu_new();
   etk_signal_connect("mouse_down", ETK_OBJECT(win), ETK_CALLBACK(_etk_test_menu_window_down_cb), menu);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Open"), ETK_STOCK_DOCUMENT_OPEN, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Save"), ETK_STOCK_DOCUMENT_SAVE, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_SEPARATOR, NULL, -1, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Cut"), ETK_STOCK_EDIT_CUT, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Copy"), ETK_STOCK_EDIT_COPY, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Paste"), ETK_STOCK_EDIT_PASTE, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_SEPARATOR, NULL, -1, ETK_MENU_SHELL(menu), statusbar);
   menu_item = _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Menu Item Test"), -1, ETK_MENU_SHELL(menu), statusbar);
   
   /* Sub menu 1 */
   menu = etk_menu_new();
   etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Child Menu Test"), -1, ETK_MENU_SHELL(menu), statusbar);
   menu_item = _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Item with child"), -1, ETK_MENU_SHELL(menu), statusbar);
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Item with image"), ETK_STOCK_DOCUMENT_SAVE, ETK_MENU_SHELL(menu), statusbar);
   
   /* Sub menu 2 */
   menu = etk_menu_new();
   etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
   _etk_test_menu_item_new(ETK_TEST_MENU_ITEM_NORMAL, _("Child Menu Test"), -1, ETK_MENU_SHELL(menu), statusbar);

   etk_widget_show_all(win);
}

/* Called when the user clicks on the window */
static void _etk_test_menu_window_down_cb(Etk_Object *object, void *event_info, void *data)
{
   etk_menu_popup(ETK_MENU(data));
}

/* Creates a new menu item */
static Etk_Widget *_etk_test_menu_item_new(Etk_Test_Menu_Item_Type item_type, const char *label,
   Etk_Stock_Id stock_id, Etk_Menu_Shell *menu_shell, Etk_Widget *statusbar)
{
   Etk_Widget *menu_item = NULL;
   
   switch (item_type)
   {
      case ETK_TEST_MENU_ITEM_NORMAL:
         menu_item = etk_menu_item_new_with_label(label);
         break;
      case ETK_TEST_MENU_ITEM_SEPARATOR:
         menu_item = etk_menu_separator_new();
         break;
      default:
         return NULL;
   }
   if (stock_id >= 0)
   {
      Etk_Widget *image;
      
      image = etk_image_new_from_stock(stock_id);
      etk_menu_item_image_set(ETK_MENU_ITEM(menu_item), ETK_IMAGE(image));
   }
   etk_menu_shell_append(menu_shell, ETK_MENU_ITEM(menu_item));
   
   etk_signal_connect("selected", ETK_OBJECT(menu_item), ETK_CALLBACK(_etk_test_menu_item_selected_cb), statusbar);
   etk_signal_connect("deselected", ETK_OBJECT(menu_item), ETK_CALLBACK(_etk_test_menu_item_deselected_cb), statusbar);
   
   return menu_item;
}

/* Called when a menu item is selected */
static void _etk_test_menu_item_selected_cb(Etk_Object *object, void *data)
{
   Etk_Menu_Item *item;
   Etk_Statusbar *statusbar;
   
   if (!(item = ETK_MENU_ITEM(object)) || !(statusbar = ETK_STATUSBAR(data)))
      return;
   
   etk_statusbar_push(statusbar, etk_menu_item_label_get(item), 0);
}

/* Called when a menu item is deselected */
static void _etk_test_menu_item_deselected_cb(Etk_Object *object, void *data)
{
   Etk_Statusbar *statusbar;
   
   if (!(statusbar = ETK_STATUSBAR(data)))
      return;
   etk_statusbar_pop(statusbar, 0);
}

