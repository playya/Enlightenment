/* This file has been automatically generated by geneet.py */
/*                      DO NOT MODIFY                      */

#ifndef __EVE_STATE_H__
#define __EVE_STATE_H__

#include <Eina.h>
#include <Eet.h>

typedef struct _Config Config;
typedef struct _Hist_Item Hist_Item;
typedef struct _Hist Hist;
typedef struct _Fav_Item Fav_Item;
typedef struct _Fav Fav;
typedef struct _Session_Item Session_Item;
typedef struct _Session_Window Session_Window;
typedef struct _Session Session;

/* Config */
Config *config_new(unsigned char allow_popup, unsigned char enable_auto_load_images, unsigned char enable_auto_shrink_images, unsigned char enable_javascript, unsigned char enable_mouse_cursor, unsigned char enable_plugins, unsigned char enable_private_mode, unsigned char enable_touch_interface, const char * home_page, const char * proxy, unsigned char restore_state, const char * user_agent);
void config_free(Config *config);

void config_allow_popup_set(Config *config, unsigned char allow_popup);
unsigned char config_allow_popup_get(const Config *config);
void config_enable_auto_load_images_set(Config *config, unsigned char enable_auto_load_images);
unsigned char config_enable_auto_load_images_get(const Config *config);
void config_enable_auto_shrink_images_set(Config *config, unsigned char enable_auto_shrink_images);
unsigned char config_enable_auto_shrink_images_get(const Config *config);
void config_enable_javascript_set(Config *config, unsigned char enable_javascript);
unsigned char config_enable_javascript_get(const Config *config);
void config_enable_mouse_cursor_set(Config *config, unsigned char enable_mouse_cursor);
unsigned char config_enable_mouse_cursor_get(const Config *config);
void config_enable_plugins_set(Config *config, unsigned char enable_plugins);
unsigned char config_enable_plugins_get(const Config *config);
void config_enable_private_mode_set(Config *config, unsigned char enable_private_mode);
unsigned char config_enable_private_mode_get(const Config *config);
void config_enable_touch_interface_set(Config *config, unsigned char enable_touch_interface);
unsigned char config_enable_touch_interface_get(const Config *config);
void config_home_page_set(Config *config, const char * home_page);
const char * config_home_page_get(const Config *config);
void config_proxy_set(Config *config, const char * proxy);
const char * config_proxy_get(const Config *config);
void config_restore_state_set(Config *config, unsigned char restore_state);
unsigned char config_restore_state_get(const Config *config);
void config_user_agent_set(Config *config, const char * user_agent);
const char * config_user_agent_get(const Config *config);

Config *config_load(const char *filename);
Eina_Bool config_save(Config *config, const char *filename);

/* Hist_Item */
Hist_Item *hist_item_new(const char * title, const char * url, unsigned int visit_count, double last_visit);
void hist_item_free(Hist_Item *hist_item);

void hist_item_title_set(Hist_Item *hist_item, const char * title);
const char * hist_item_title_get(const Hist_Item *hist_item);
void hist_item_url_set(Hist_Item *hist_item, const char * url);
const char * hist_item_url_get(const Hist_Item *hist_item);
void hist_item_visit_count_set(Hist_Item *hist_item, unsigned int visit_count);
unsigned int hist_item_visit_count_get(const Hist_Item *hist_item);
void hist_item_last_visit_set(Hist_Item *hist_item, double last_visit);
double hist_item_last_visit_get(const Hist_Item *hist_item);

/* Hist */
Hist *hist_new();
void hist_free(Hist *hist);

void hist_items_add(Hist *hist, const char * url, Hist_Item *hist_item);
void hist_items_del(Hist *hist, const char * url);
Hist_Item *hist_items_get(const Hist *hist, const char * key);
Eina_Hash *hist_items_hash_get(const Hist *hist);
void hist_items_modify(Hist *hist, const char * key, void *value);

Hist *hist_load(const char *filename);
Eina_Bool hist_save(Hist *hist, const char *filename);

/* Fav_Item */
Fav_Item *fav_item_new(const char * url, const char * title, unsigned int visit_count);
void fav_item_free(Fav_Item *fav_item);

void fav_item_url_set(Fav_Item *fav_item, const char * url);
const char * fav_item_url_get(const Fav_Item *fav_item);
void fav_item_title_set(Fav_Item *fav_item, const char * title);
const char * fav_item_title_get(const Fav_Item *fav_item);
void fav_item_visit_count_set(Fav_Item *fav_item, unsigned int visit_count);
unsigned int fav_item_visit_count_get(const Fav_Item *fav_item);

/* Fav */
Fav *fav_new();
void fav_free(Fav *fav);

void fav_items_add(Fav *fav, const char * url, Fav_Item *fav_item);
void fav_items_del(Fav *fav, const char * url);
Fav_Item *fav_items_get(const Fav *fav, const char * key);
Eina_Hash *fav_items_hash_get(const Fav *fav);
void fav_items_modify(Fav *fav, const char * key, void *value);

Fav *fav_load(const char *filename);
Eina_Bool fav_save(Fav *fav, const char *filename);

/* Session_Item */
Session_Item *session_item_new(const char * url, unsigned char focused, double scroll_x, double scroll_y);
void session_item_free(Session_Item *session_item);

void session_item_url_set(Session_Item *session_item, const char * url);
const char * session_item_url_get(const Session_Item *session_item);
void session_item_focused_set(Session_Item *session_item, unsigned char focused);
unsigned char session_item_focused_get(const Session_Item *session_item);
void session_item_scroll_x_set(Session_Item *session_item, double scroll_x);
double session_item_scroll_x_get(const Session_Item *session_item);
void session_item_scroll_y_set(Session_Item *session_item, double scroll_y);
double session_item_scroll_y_get(const Session_Item *session_item);

/* Session_Window */
Session_Window *session_window_new(Eina_List * tabs, unsigned char focused);
void session_window_free(Session_Window *session_window);

void session_window_tabs_add(Session_Window *session_window, Session_Item *session_item);
void session_window_tabs_del(Session_Window *session_window, Session_Item *session_item);
Session_Item *session_window_tabs_get(const Session_Window *session_window, unsigned int nth);
unsigned int session_window_tabs_count(const Session_Window *session_window);
Eina_List *session_window_tabs_list_get(const Session_Window *session_window);
void session_window_tabs_list_clear(Session_Window *session_window);
void session_window_tabs_list_set(Session_Window *session_window, Eina_List *list);
void session_window_focused_set(Session_Window *session_window, unsigned char focused);
unsigned char session_window_focused_get(const Session_Window *session_window);

/* Session */
Session *session_new(Eina_List * windows);
void session_free(Session *session);

void session_windows_add(Session *session, Session_Window *session_window);
void session_windows_del(Session *session, Session_Window *session_window);
Session_Window *session_windows_get(const Session *session, unsigned int nth);
unsigned int session_windows_count(const Session *session);
Eina_List *session_windows_list_get(const Session *session);
void session_windows_list_clear(Session *session);
void session_windows_list_set(Session *session, Eina_List *list);

Session *session_load(const char *filename);
Eina_Bool session_save(Session *session, const char *filename);

/* Global initializer / shutdown functions */
void eve_state_init(void);
void eve_state_shutdown(void);

#endif /* __EVE_STATE_H__ */
