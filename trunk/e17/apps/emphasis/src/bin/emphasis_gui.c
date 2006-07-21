#include "emphasis.h"
#include "emphasis_gui.h"


/**
 * @brief Build all widgets for Emphasis GUI
 * @param gui A gui to initialize
 */
void
emphasis_init_gui(Emphasis_Gui *gui)
{
  gui->player = malloc(sizeof(Emphasis_Player_Gui));
  /* TODO : check player */
  /* TODO ; check config, state, etc */
  
  emphasis_init_player(gui->player);
  emphasis_init_menu(gui);

  if(!etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(gui->player->small.media)))
    {
      etk_widget_hide_all(gui->player->media.window);
    }

  gui->cover_queue = NULL;
}


/* TODO : documentation */
void
emphasis_clear(Emphasis_Gui *gui)
{
  Emphasis_Player_Gui *player;
  
  player = gui->player;

  etk_tree_clear(ETK_TREE(player->media.artist));
  etk_tree_clear(ETK_TREE(player->media.album));
  etk_tree_clear(ETK_TREE(player->media.track));
  etk_tree_clear(ETK_TREE(player->media.pls));
}

/* TODO : documentation */
/* TODO : enhanced? pfffffffffffffff */
void
emphasis_init_menu(Emphasis_Gui *gui)
{
  gui->config_gui = malloc(sizeof(Emphasis_Config_Gui));
  Emphasis_Player_Gui *player;
	
  /* playlist menu setup*/

  gui->menu = etk_menu_new();
  player = gui->player;
	
  Etk_Widget *separator, *radio_item=NULL;


  emphasis_menu_append(gui->menu,
                       "clear",
                       ETK_STOCK_EDIT_CLEAR, cb_playlist_clear, NULL,
                       "delete",
                       ETK_STOCK_EDIT_DELETE, cb_playlist_delete, player,
                       "update",
                       ETK_STOCK_VIEW_REFRESH, cb_database_update, player,
                       "config",
                       ETK_STOCK_PREFERENCES_SYSTEM, cb_config_show, gui,
                       NULL);
  separator = etk_menu_item_separator_new();
  etk_menu_shell_append(ETK_MENU_SHELL(gui->menu), ETK_MENU_ITEM(separator));

  radio_item = etk_menu_item_radio_new_with_label_from_widget("full", NULL);
  etk_menu_shell_append(ETK_MENU_SHELL(gui->menu), ETK_MENU_ITEM(radio_item));
  etk_signal_connect("activated",
                     ETK_OBJECT(radio_item),
                     ETK_CALLBACK(cb_switch_full), player);
  radio_item = etk_menu_item_radio_new_with_label_from_widget
               	("small", ETK_MENU_ITEM_RADIO(radio_item));
  etk_menu_shell_append(ETK_MENU_SHELL(gui->menu), ETK_MENU_ITEM(radio_item));
  etk_signal_connect("activated",
                     ETK_OBJECT(radio_item),
                     ETK_CALLBACK(cb_switch_small), player);

	/* Do we need all this connect ? */
	etk_signal_connect("mouse_down", ETK_OBJECT(player->full.window), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);
  etk_signal_connect("mouse_down", ETK_OBJECT(player->small.window), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);
	etk_signal_connect("mouse_down", ETK_OBJECT(player->media.window),
                     ETK_CALLBACK(cb_pls_contextual_menu), gui);
	                   
	etk_signal_connect("mouse_down", ETK_OBJECT(player->media.pls), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);
	etk_signal_connect("mouse_down", ETK_OBJECT(player->media.artist), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);
	etk_signal_connect("mouse_down", ETK_OBJECT(player->media.album), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);
	etk_signal_connect("mouse_down", ETK_OBJECT(player->media.track), 
	                   ETK_CALLBACK(cb_pls_contextual_menu), gui);

}

/**
 * @brief Make a menu with small stock image and sets a callback on "activated" on each elements
 * @param menu The Etk_Menu to setup
 * @param ... An (char*)menu_item name, an (Etk_Stock_Id)image id, a Etk_Callback function and 
 * it data ... terminated by NULL
 */
void
emphasis_menu_append(Etk_Widget *menu, ...)
{
  Etk_Widget *menu_item, *item_image = NULL;
  char *item_name;
  Etk_Stock_Id item_image_id;
  void *callback, *data;
  va_list arglist;

  va_start(arglist, menu);

  while ((item_name = va_arg(arglist, char *)) != NULL)
    {
      menu_item = etk_menu_item_image_new_with_label(item_name);
      item_image_id = va_arg(arglist, Etk_Stock_Id);
      if (item_image_id)
        {
          item_image =
            etk_image_new_from_stock(item_image_id, ETK_STOCK_SMALL);
          etk_menu_item_image_set(ETK_MENU_ITEM_IMAGE(menu_item),
                                  ETK_IMAGE(item_image));
        }
      callback = va_arg(arglist, void *);
      data = va_arg(arglist, void *);
      if (callback)
        {
          etk_signal_connect("activated", ETK_OBJECT(menu_item),
                             ETK_CALLBACK(callback), data);
        }

      etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
    }
  va_end(arglist);
}

/**
 * @brief Replace a null string by "Unkown"
 * @param table A table of char** terminated by NULL
 */
void
emphasis_unknow_if_null(char **table[])
{
  int i = 0;

  while (table[i])
    {
      if (!*table[i])
        {
          *table[i] = strdup("Unknown");
        }
      i++;
    }
}

/* TODO : documenation */
void
emphasis_cover_change(Emphasis_Gui *gui, char *artist, char *album)
{
  Cover_Info *ci;

  ci = malloc(sizeof(Cover_Info));

  ci->artist = artist;
  ci->album = album;

  gui->cover_queue = ci;
}

