#include "engage.h"
#include "config.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

#ifdef HAVE_ETK
#include <Etk.h>

Etk_Widget *_od_config_menu = NULL;
Etk_Widget *_od_config_menu_app;
#endif

typedef enum od_config_colors {
  TT_TXT,
  TT_SHD,
  BG_FORE,
  BG_BACK
} od_config_colors;

OD_Options      options;

/* listeners */

int
zoom_listener(const char *key, const Ecore_Config_Type type, const int tag,
              void *data)
{
  options.zoom = ecore_config_boolean_get(key);
  return 1;
}

int
theme_listener(const char *key, const Ecore_Config_Type type, const int tag,
               void *data)
{
  char           *path;
  Evas_List      *icons;

  if (options.theme)
    free(options.theme);
  options.theme = ecore_config_theme_get(key);

  path = ecore_config_theme_with_path_get(key);

  icons = dock.icons;
  while (icons) {
    od_icon_reload((OD_Icon *) icons->data);
    icons = evas_list_next(icons);
  }
  return 1;
}

int
colour_listener(const char *key, const Ecore_Config_Type type, const int tag, 
                void *data)
{
  long colour;

  colour = ecore_config_argbint_get(key);

  switch (tag) {
    case BG_FORE:
      options.bg_fore = colour;
      break;
    case BG_BACK:
      options.bg_back = colour;
      break;
  }
  return 1;
}

int
od_config_init(void)
{
  int             ret;

  ecore_config_int_create("engage.options.width", 1024, 'W', "width",
                          "The overall width of the application area");
  ecore_config_int_create("engage.options.height", 130, 'H', "height",
                          "The overall height of the application area");
  ecore_config_theme_create("engage.options.theme", "none", 't', "theme",
                            "The theme name to use (minus path and extension)");
  /* not technically correct - iconsets should do this, but it looks better for
   * everything bar 'gentoo' - and we all have the others installed ;) */
  ecore_config_theme_preview_group_set("engage.options.theme", "Terminal");
  ecore_config_theme_search_path_append(PACKAGE_DATA_DIR "/themes/");
  ecore_config_string_create("engage.options.engine", "software", 'e', "engine",
                             "The X11 engine to use - either software or gl");
  options.icon_path = PACKAGE_DATA_DIR "/icons/";
  ecore_config_int_create_bound("engage.options.mode", OM_BELOW, 0, 1, 1, 'm',
                                "mode",
                                "The display mode, 0 = ontop + shaped, 1 = below + transp");

  ecore_config_int_create("engage.options.reserve", 52, 'R', "reserve",
                          "The amount of space reserved at the bottom of the screen");
#ifdef XINERAMA
  ecore_config_int_create("engage.options.head", 0, 'X', "head",
                          "Which Xinerama head to display the docker on");
#endif
  
  ecore_config_boolean_create("engage.options.grab_min_icons", 0, 'g',
                              "grab-min",
                              "Capture the icons of minimised applications");
  ecore_config_boolean_create("engage.options.grab_app_icons", 0, 'G',
                               "grab-app",
                               "Capture the icons of all running applications");
  ecore_config_boolean_create("engage.options.auto_hide", 0, 'A', "auto-hide",
                              "Auto hide the engage bar");

  ecore_config_int_create("engage.options.size", 37, 's', "size",
                          "Size of icons in default state");
  ecore_config_int_create("engage.options.spacing", 4, 'S', "spacing",
                          "Space in pixels between each icon");
  ecore_config_boolean_create("engage.options.zoom", 1, 'z', "zoom",
                                "Should we zoom icons?");
  ecore_config_float_create("engage.options.zoom_factor", 2.0, 'Z',
                            "zoom-factor",
                            "Zoom factor of the icons - 1.0 == 100% == nozoom");
  ecore_config_float_create("engage.options.zoom_duration", 0.2, 'd',
                            "zoom-time",
                            "Time taken (in seconds) for icons to zoom");

  ecore_config_argb_create("engage.options.bg_fore", "#7f000000", 'B', "bg-outline-color", "Background outline color");
  ecore_config_argb_create("engage.options.bg_back", "#7fffffff", 'b', "bg-main-color", "Background main color");

  ecore_config_float_create("engage.options.icon_appear_duration", 0.1, 'D',
                            "appear-time",
                            "Time taken (in seconds) for new icons to appear");

  ecore_config_boolean_create("engage.options.tray", 1, 'T', "tray",
                              "Enable system tray");
  ecore_config_boolean_create("engage.options.ignore_running", 0, 'i',
                              "ignore-running", "Ignore running apps");
  ecore_config_boolean_create("engage.options.ignore_iconified", 0, 'I',
                              "ignore-iconified", "Ignore iconified windows");
  
  
  ecore_config_boolean_create("engage.options.use_composite", 0, 'C',
                              "use-composite","Use with composite manager for nice transparency effects");
                          
                              
  ecore_config_load();
  ret = ecore_config_args_parse();

  options.width = ecore_config_int_get("engage.options.width");
  options.height = ecore_config_int_get("engage.options.height");
  options.engine = ecore_config_string_get("engage.options.engine");
  options.theme = ecore_config_theme_get("engage.options.theme");
  ecore_config_listen("theme", "engage.options.theme", theme_listener, 0, NULL);
  options.mode = ecore_config_int_get("engage.options.mode");

  options.reserve = ecore_config_int_get("engage.options.reserve");
#ifdef XINERAMA
  options.head = ecore_config_int_get("engage.options.head");
#endif
  
  options.grab_min_icons =
    ecore_config_boolean_get("engage.options.grab_min_icons");
  options.grab_app_icons =
    ecore_config_boolean_get("engage.options.grab_app_icons");
  options.auto_hide = ecore_config_boolean_get("engage.options.auto_hide");
  
  options.size = ecore_config_int_get("engage.options.size");
  options.spacing = ecore_config_int_get("engage.options.spacing");
  options.zoom = ecore_config_boolean_get("engage.options.zoom");
  ecore_config_listen("zoom", "engage.options.zoom", zoom_listener, 0, NULL);
  options.zoomfactor = ecore_config_float_get("engage.options.zoom_factor");
  options.dock_zoom_duration =
    ecore_config_float_get("engage.options.zoom_duration");

  options.bg_fore = ecore_config_argbint_get("engage.options.bg_fore");
  ecore_config_listen("colour", "engage.options.bg_fore", 
                      colour_listener, BG_FORE, NULL);
  options.bg_back = ecore_config_argbint_get("engage.options.bg_back");
  ecore_config_listen("colour", "engage.options.bg_back", 
                      colour_listener, BG_BACK, NULL);

  options.icon_appear_duration =
    ecore_config_float_get("engage.options.icon_appear_duration");

  options.tray = ecore_config_boolean_get("engage.options.tray");
  options.ignore_run = ecore_config_boolean_get("engage.options.ignore_running");
  options.ignore_min = ecore_config_boolean_get("engage.options.ignore_iconified");
  
  options.use_composite = ecore_config_boolean_get("engage.options.use_composite");
  return ret;
}

#ifdef HAVE_ETK
void _od_config_menu_zooming_cb(Etk_Object *object, void *data) {
  ecore_config_int_set("engage.options.zoom", options.zoom ? 0 : 1);
}

void _od_config_menu_config_cb(Etk_Object *object, void *data) {
  if (!ecore_exe_run("examine engage", NULL))
    fprintf(stderr, "'examine' could not be launched - is it in your path?\n"); 
}

void _od_config_menu_quit_cb(Etk_Object *object, void *data) {
  ecore_main_loop_quit();
}

void od_config_menu_init(void) {
  Etk_Widget *menu, *menu_item;

  menu = etk_menu_new();
  menu_item = etk_menu_item_new_with_label("<App name here>");
  etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
//  etk_signal_connect("activated", ETK_OBJECT(menu_item), ETK_CALLBACK(_od_config_menu_app_cb), NULL);
  _od_config_menu_app = menu_item;

  menu_item = etk_menu_item_new_with_label("Icon Zooming");
  etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
  etk_signal_connect("activated", ETK_OBJECT(menu_item), ETK_CALLBACK(_od_config_menu_zooming_cb), NULL);
  menu_item = etk_menu_item_new_with_label("Configuration");
  etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
  etk_signal_connect("activated", ETK_OBJECT(menu_item), ETK_CALLBACK(_od_config_menu_config_cb), NULL);
  menu_item = etk_menu_item_separator_new();
  etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
  menu_item = etk_menu_item_new_with_label("Quit");
  etk_menu_shell_append(ETK_MENU_SHELL(menu), ETK_MENU_ITEM(menu_item));
  etk_signal_connect("activated", ETK_OBJECT(menu_item), ETK_CALLBACK(_od_config_menu_quit_cb), NULL);
  etk_widget_show(menu);

  _od_config_menu  = menu;
}

void od_config_menu_draw(Evas_Coord x, Evas_Coord y) {
  Evas_List      *l;
  Evas_Coord      minx, maxx;
  char           *label;

  if (_od_config_menu == NULL)
    od_config_menu_init();

  minx = x - options.size / 2;
  maxx = x + options.size / 2;
  label = "Not over icon";
  etk_widget_hide(_od_config_menu_app);

  l = dock.icons;
  while (l) {
    OD_Icon *icon;
    icon = l->data;
    if (icon->x >= minx && icon->x <= maxx) {
      int len;
      char *full;
      label = icon->a->name;
      etk_widget_show(_od_config_menu_app);
      break;
    }
    l = l->next;
  }

  etk_menu_item_label_set(ETK_MENU_ITEM(_od_config_menu_app), label);
  etk_menu_popup(ETK_MENU(_od_config_menu));
}

#endif

