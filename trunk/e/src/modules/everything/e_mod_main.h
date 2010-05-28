#ifndef EVRY_H
#define EVRY_H

#include "e.h"
#include "evry_api.h"

#define MOD_CONFIG_FILE_EPOCH 0x0005
#define MOD_CONFIG_FILE_GENERATION 0x0001
#define MOD_CONFIG_FILE_VERSION					\
  ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

#define SLIDE_LEFT   1
#define SLIDE_RIGHT -1

typedef struct _Evry_View	Evry_View;
typedef struct _History		Evry_History;
typedef struct _Config		Evry_Config;
typedef struct _Evry_Selector	Evry_Selector;
typedef struct _Tab_View	Tab_View;
typedef struct _Evry_Window	Evry_Window;

struct _Evry_Window
{
  E_Popup *popup;
  Evas_Object *o_main;

  Eina_Bool request_selection;
  Eina_Bool plugin_dedicated;

  Eina_Bool visible;
  Ecore_Timer *show_timer;
  Ecore_Timer *hide_timer;

  Eina_List *handlers;

  Evry_Selector  *selector;
  Evry_Selector **selectors;
  Evry_Selector **sel_list;

  int level;

  int mouse_out;
  int mouse_button;

  Evry_View *view_clearing;
  Evry_View *view_freeing;
};

struct _Evry_Selector
{
  Evry_Window *win;
  
  /* current state */
  Evry_State  *state;

  /* stack of states (for browseable plugins) */
  Eina_List   *states;

  /* provides collection of items from other plugins */
  Evry_Plugin *aggregator;

  /* action selector plugin */
  Evry_Plugin *actions;

  /* all plugins that belong to this selector*/
  Eina_List   *plugins;

  /* list view instance */
  Evry_View   *view;

  Evas_Object *o_icon;
  Evas_Object *o_thumb;
  Eina_Bool    do_thumb;

  Ecore_Timer *update_timer;
  Ecore_Timer *action_timer;
};

struct _Evry_State
{
  Evry_Selector *selector;

  char *inp; /* alloced input */

  char *input; /* pointer to input + trigger */
  /* all available plugins for current state */
  Eina_List   *plugins;

  /* currently active plugins, i.e. those that provide items */
  Eina_List   *cur_plugins;

  /* active plugin */
  Evry_Plugin *plugin;

  /* selected item */
  Evry_Item   *cur_item;

  /* marked items */
  Eina_List   *sel_items;

  Eina_Bool plugin_auto_selected;
  Eina_Bool item_auto_selected;

  /* current view instance */
  Evry_View *view;

  Eina_Bool changed;
  Eina_Bool trigger_active;

  unsigned int request;
};

struct _Evry_View
{
  Evry_View  *id;
  const char *name;
  const char *trigger;
  int active;
  Evas_Object *o_list;
  Evas_Object *o_bar;

  Evry_View *(*create) (Evry_View *view, const Evry_State *s, const Evas_Object *swallow);
  void (*destroy)      (Evry_View *view);
  int  (*cb_key_down)  (Evry_View *view, const Ecore_Event_Key *ev);
  int  (*update)       (Evry_View *view);
  void (*clear)        (Evry_View *view);

  Ecore_Timer *clear_timer;
  
  int priority;
};

struct _Tab_View
{
  const Evry_State *state;

  Evry_View *view;
  Evas *evas;

  Evas_Object *o_tabs;
  Eina_List *tabs;

  void (*update) (Tab_View *tv);
  void (*clear) (Tab_View *tv);
  int (*key_down) (Tab_View *tv, const Ecore_Event_Key *ev);

  double align;
  double align_to;
  Ecore_Animator *animator;
  Ecore_Timer *timer;
};

struct _Config
{
  int version;
  /* position */
  double rel_x, rel_y;
  /* size */
  int width, height;
  int edge_width, edge_height;

  Eina_List *modules;

  /* generic plugin config */
  Eina_List *conf_subjects;
  Eina_List *conf_actions;
  Eina_List *conf_objects;
  Eina_List *conf_views;
  Eina_List *collections;

  int scroll_animate;
  double scroll_speed;

  int hide_input;
  int hide_list;

  /* quick navigation mode */
  int quick_nav;

  /* default view mode */
  int view_mode;
  int view_zoom;

  int history_sort_mode;

  /* use up/down keys for prev/next in thumb view */
  int cycle_mode;

  unsigned char first_run;

  /* not saved data */
  Eina_List *actions;
  Eina_List *views;

  int min_w, min_h;
};

struct _History
{
  int version;
  Eina_Hash *subjects;
  double begin;

  Eina_Bool changed;
};

/* evry.c */
void  evry_item_select(const Evry_State *s, Evry_Item *it);
void  evry_item_mark(const Evry_State *state, Evry_Item *it, Eina_Bool mark);
void  evry_plugin_select(Evry_Plugin *p);
/* int   evry_list_win_show(void);
 * void  evry_list_win_hide(void); */
Evry_Item *evry_item_new(Evry_Item *base, Evry_Plugin *p, const char *label,
			      Evas_Object *(*icon_get) (Evry_Item *it, Evas *e),
			      void (*cb_free) (Evry_Item *item));
void  evry_item_free(Evry_Item *it);
void  evry_item_ref(Evry_Item *it);

void  evry_plugin_update(Evry_Plugin *plugin, int state);
void  evry_clear_input(Evry_Plugin *p);

/* evry_util.c */
Evas_Object *evry_icon_mime_get(const char *mime, Evas *e);
Evas_Object *evry_icon_theme_get(const char *icon, Evas *e);
int   evry_fuzzy_match(const char *str, const char *match);
Eina_List *evry_fuzzy_match_sort(Eina_List *items);
int   evry_util_exec_app(const Evry_Item *it_app, const Evry_Item *it_file);
char *evry_util_url_escape(const char *string, int inlength);
char *evry_util_url_unescape(const char *string, int length);
void  evry_util_file_detail_set(Evry_Item_File *file);
int   evry_util_module_config_check(const char *module_name, int conf, int epoch, int version);
Evas_Object *evry_util_icon_get(Evry_Item *it, Evas *e);
int   evry_util_plugin_items_add(Evry_Plugin *p, Eina_List *items, const char *input, int match_detail, int set_usage);
int   evry_items_sort_func(const void *data1, const void *data2);
void  evry_item_changed(Evry_Item *it, int change_icon, int change_selected);
char *evry_util_md5_sum(const char *str);

const char *evry_file_path_get(Evry_Item_File *file);
const char *evry_file_url_get(Evry_Item_File *file);

int   evry_plugin_register(Evry_Plugin *p, int type, int priority);
void  evry_plugin_unregister(Evry_Plugin *p);
Evry_Plugin *evry_plugin_find(const char *name);
void  evry_action_register(Evry_Action *act, int priority);
void  evry_action_unregister(Evry_Action *act);
void  evry_view_register(Evry_View *view, int priority);
void  evry_view_unregister(Evry_View *view);
Evry_Action *evry_action_find(const char *name);

void  evry_history_load(void);
void  evry_history_unload(void);
History_Item *evry_history_item_add(Evry_Item *it, const char *ctxt, const char *input);
int   evry_history_item_usage_set(Evry_Item *it, const char *input, const char *ctxt);
History_Types *evry_history_types_get(Evry_Type type);

Evry_Plugin *evry_plugin_new(Evry_Plugin *base, const char *name, const char *label, const char *icon,
			     Evry_Type item_type,
			     Evry_Plugin *(*begin) (Evry_Plugin *p, const Evry_Item *item),
			     void (*cleanup) (Evry_Plugin *p),
			     int  (*fetch)   (Evry_Plugin *p, const char *input),
			     void (*free) (Evry_Plugin *p));

void  evry_plugin_free(Evry_Plugin *p);

Evry_Action *evry_action_new(const char *name, const char *label,
			     Evry_Type type1, Evry_Type type2,
			     const char *icon,
			     int  (*action)     (Evry_Action *act),
			     int  (*check_item) (Evry_Action *act, const Evry_Item *it));

void  evry_action_free(Evry_Action *act);

int   evry_api_version_check(int version);

Evry_Type evry_type_register(const char *type);
const char *evry_type_get(Evry_Type type);


Tab_View *evry_tab_view_new(Evry_View *view, const Evry_State *s, Evas *e);
void  evry_tab_view_free(Tab_View *v);

Eina_Bool evry_view_init(void);
void  evry_view_shutdown(void);

Eina_Bool evry_view_help_init(void);
void  evry_view_help_shutdown(void);

Eina_Bool evry_plug_clipboard_init(void);
void  evry_plug_clipboard_shutdown(void);

Eina_Bool evry_plug_text_init(void);
void  evry_plug_text_shutdown(void);

Eina_Bool evry_plug_collection_init(void);
void  evry_plug_collection_shutdown(void);

int   evry_init(void);
int   evry_shutdown(void);
int   evry_show(E_Zone *zone, E_Zone_Edge edge, const char *params);
void  evry_hide(int clear);

int   evry_plug_actions_init();
void  evry_plug_actions_shutdown();
Evry_Plugin *evry_plug_actions_new(Evry_Selector *selector, int type);

Evry_Plugin *evry_aggregator_new(Evry_Window *win, int type);

void  evry_history_init(void);
void  evry_history_free(void);

int   evry_browse_item(Evry_Item *it);
int   evry_browse_back(Evry_Selector *sel);

void  evry_plugin_action(int finished);

int   evry_state_push(Evry_Selector *sel, Eina_List *plugins);
int   evry_selectors_switch(int dir, int slide);
int   evry_view_toggle(Evry_State *s, const char *trigger);

Ecore_Event_Handler *evry_event_handler_add(int type, int (*func) (void *data, int type, void *event), const void *data);

extern Evry_History *evry_hist;
extern Evry_Config  *evry_conf;
extern int  _evry_events[4];

#define EVRY_ITEM_NEW(_base, _plugin, _label, _icon_get, _free)		\
  (_base *) evry_item_new(EVRY_ITEM(E_NEW(_base, 1)), EVRY_PLUGIN(_plugin), \
			  _label, _icon_get, _free)

#define EVRY_ITEM_FREE(_item) evry_item_free((Evry_Item *)_item)
#define EVRY_ITEM_REF(_item) evry_item_ref((Evry_Item *)_item)

#define EVRY_PLUGIN_NEW(_base, _name, _icon, _item_type, _begin, _cleanup, _fetch, _free) \
  evry_plugin_new(EVRY_PLUGIN(E_NEW(_base, 1)), _name, _(_name), _icon, _item_type, \
		  _begin, _cleanup, _fetch, _free)

#define EVRY_ACTION_NEW(_name, _in1, _in2, _icon, _action, _check)	\
  evry_action_new(_name, _(_name), _in1, _in2, _icon, _action, _check)

#define EVRY_PLUGIN_FREE(_p)			\
  if (_p) evry_plugin_free(EVRY_PLUGIN(_p))

#define EVRY_PLUGIN_UPDATE(_p, _action)			\
  if (_p) evry_plugin_update(EVRY_PLUGIN(_p), _action)

#define EVRY_PLUGIN_ITEMS_FREE(_p) {		\
     Evry_Item *it;				\
     EINA_LIST_FREE(EVRY_PLUGIN(_p)->items, it) \
       evry_item_free(it); }

#define EVRY_PLUGIN_ITEMS_ADD(_plugin, _items, _input, _match_detail, _set_usage) \
  evry_util_plugin_items_add(EVRY_PLUGIN(_plugin), _items, _input, _match_detail, _set_usage)


/*** Common Logging  ***/
extern int _e_module_evry_log_dom;

#ifndef EINA_LOG_DEFAULT_COLOR
#define EINA_LOG_DEFAULT_COLOR EINA_COLOR_CYAN
#endif

#undef DBG
#undef INF
#undef WRN
#undef ERR

#define DBG(...) EINA_LOG_DOM_DBG(_e_module_evry_log_dom , __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_e_module_evry_log_dom , __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_e_module_evry_log_dom , __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_e_module_evry_log_dom , __VA_ARGS__)

/*** E Module ***/
EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI E_Config_Dialog *evry_config_dialog(E_Container *con, const char *params);

EAPI extern E_Module_Api e_modapi;

#endif
