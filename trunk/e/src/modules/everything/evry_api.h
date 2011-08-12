#ifndef EVRY_API_H
#define EVRY_API_H

#include "evry_types.h"

#define EVRY_API_VERSION     27

#define EVRY_ACTION_OTHER    0
#define EVRY_ACTION_FINISHED 1
#define EVRY_ACTION_CONTINUE 2
#define EVRY_ACTION_CLEAR    3

#define EVRY_UPDATE_ADD	     0
#define EVRY_UPDATE_CLEAR    1
#define EVRY_UPDATE_REFRESH  2

#define EVRY_COMPLETE_NONE   0
#define EVRY_COMPLETE_INPUT  1
#define EVRY_COMPLETE_BROWSE 2

#define VIEW_MODE_NONE	    -1
#define VIEW_MODE_LIST	     0
#define VIEW_MODE_DETAIL     1
#define VIEW_MODE_THUMB	     2

#define EVRY_PLUGIN_SUBJECT  0
#define EVRY_PLUGIN_ACTION   1
#define EVRY_PLUGIN_OBJECT   2

#define EVRY_TYPE_NONE	     0
#define EVRY_TYPE_FILE	     1
#define EVRY_TYPE_DIR	     2
#define EVRY_TYPE_APP	     3
#define EVRY_TYPE_ACTION     4
#define EVRY_TYPE_PLUGIN     5
#define EVRY_TYPE_BORDER     6
#define EVRY_TYPE_TEXT	     7
#define NUM_EVRY_TYPES	     8

#define EVRY_EVENT_ITEM_SELECTED       	0
#define EVRY_EVENT_ITEM_CHANGED		1
#define EVRY_EVENT_ITEMS_UPDATE		2
#define EVRY_EVENT_ACTION_PERFORMED	3
#define EVRY_EVENT_PLUGIN_SELECTED	4
#define NUM_EVRY_EVENTS	                5

typedef struct _Evry_API Evry_API;
typedef struct _Evry_Module Evry_Module;

typedef struct _Evry_Event_Item_Changed     Evry_Event_Item_Changed;
typedef struct _Evry_Event_Item_Selected    Evry_Event_Item_Selected;
typedef struct _Evry_Event_Action_Performed Evry_Event_Action_Performed;

extern Evry_API *evry;

struct _Evry_Module
{
  Eina_Bool active;

  int  (*init)(void);
  void (*shutdown)(void);
};

struct _Evry_API
{
  int  (*api_version_check)(int version);

  Evry_Item *(*item_new)(Evry_Item *base, Evry_Plugin *p, const char *label,
			Evas_Object *(*icon_get) (Evry_Item *it, Evas *e),
			void (*cb_free) (Evry_Item *item));

  void (*item_free)(Evry_Item *it);
  void (*item_ref)(Evry_Item *it);
  /* send EVRY_EVENT_ITEM_CHANGED event */
  void  (*item_changed)(Evry_Item *it, int change_icon, int change_selected);

  Evry_Plugin *(*plugin_new)(Evry_Plugin *base, const char *name,
				  const char *label, const char *icon,
				  Evry_Type item_type,
				  Evry_Plugin *(*begin) (Evry_Plugin *p, const Evry_Item *item),
				  void (*cleanup) (Evry_Plugin *p),
				  int  (*fetch)   (Evry_Plugin *p, const char *input),
				  void (*free) (Evry_Plugin *p));

  void (*plugin_free)(Evry_Plugin *p);
  /* when a new plugin config was created return val is 1. in this
     case you can set defaults of p->config */
  int  (*plugin_register)(Evry_Plugin *p, int type, int priority);
  void (*plugin_unregister)(Evry_Plugin *p);
  void (*plugin_update)(Evry_Plugin *plugin, int state);
  Evry_Plugin *(*plugin_find)(const char *name);
  
  Evry_Action *(*action_new)(const char *name, const char *label,
				  Evry_Type type1, Evry_Type type2,
				  const char *icon,
				  int  (*action)     (Evry_Action *act),
				  int  (*check_item) (Evry_Action *act, const Evry_Item *it));

  void (*action_free)(Evry_Action *act);
  void (*action_register)(Evry_Action *act, int priority);
  void (*action_unregister)(Evry_Action *act);
  Evry_Action *(*action_find)(const char *name);
  Evry_Type (*type_register)(const char *type);

  /* evry_util.c */
  Evas_Object *(*icon_mime_get)(const char *mime, Evas *e);
  Evas_Object *(*icon_theme_get)(const char *icon, Evas *e);
  int   (*fuzzy_match)(const char *str, const char *match);
  int   (*util_exec_app)(const Evry_Item *it_app, const Evry_Item *it_file);
  char *(*util_url_escape)(const char *string, int inlength);
  char *(*util_url_unescape)(const char *string, int length);
  void  (*util_file_detail_set)(Evry_Item_File *file);
  int   (*util_plugin_items_add)(Evry_Plugin *p, Eina_List *items, const char *input, int match_detail, int set_usage);
  char *(*util_md5_sum)(const char *str);
  Evas_Object *(*util_icon_get)(Evry_Item *it, Evas *e);
  int   (*items_sort_func)(const void *data1, const void *data2);

  const char *(*file_path_get)(Evry_Item_File *file);
  const char *(*file_url_get)(Evry_Item_File *file);

  History_Item  *(*history_item_add)(Evry_Item *it, const char *ctxt, const char *input);
  History_Types *(*history_types_get)(Evry_Type type);
  int  (*history_item_usage_set)(Evry_Item *it, const char *input, const char *ctxt);

  Ecore_Event_Handler *(*event_handler_add)(int type, Eina_Bool (*func) (void *data, int type, void *event), const void *data);

  int log_dom;
};

struct _Evry_Event_Item_Changed
{
  Evry_Item *item;
  int changed_selection;
  int changed_icon;
};

struct _Evry_Event_Item_Selected
{
  Evry_Item *item;
};

struct _Evry_Event_Action_Performed
{
  const char *action;
  const Evry_Item *it1;
  const Evry_Item *it2;
};


/*** cast default types ***/
#define EVRY_ITEM(_item) ((Evry_Item *)_item)
#define EVRY_ACTN(_item) ((Evry_Action *) _item)
#define EVRY_PLUGIN(_plugin) ((Evry_Plugin *) _plugin)
#define EVRY_VIEW(_view) ((Evry_View *) _view)
#define EVRY_FILE(_it) ((Evry_Item_File *) _it)

#define GET_APP(_app, _item) Evry_Item_App *_app = (Evry_Item_App *) _item
#define GET_CMD(_app, _item) Evry_Item_App *_app = (Evry_Item_App *) _item
#define GET_FILE(_file, _item) Evry_Item_File *_file = (Evry_Item_File *) _item
#define GET_EVRY_PLUGIN(_p, _plugin) Evry_Plugin *_p = (Evry_Plugin*) _plugin
#define GET_PLUGIN(_p, _plugin) Plugin *_p = (Plugin*) _plugin
#define GET_VIEW(_v, _view) View *_v = (View*) _view
#define GET_ACTION(_act, _item) Evry_Action *_act = (Evry_Action *) _item
#define GET_ITEM(_it, _any) Evry_Item *_it = (Evry_Item *) _any

/*** Evry_Item macros ***/
#define EVRY_ITEM_NEW(_base, _plugin, _label, _icon_get, _free) \
  (_base *) evry->item_new(EVRY_ITEM(E_NEW(_base, 1)),		\
			   EVRY_PLUGIN(_plugin),		\
			   _label, _icon_get, _free)

#define EVRY_ITEM_FREE(_item) evry->item_free((Evry_Item *)_item)
#define EVRY_ITEM_REF(_item) evry->item_ref((Evry_Item *)_item)

#define EVRY_ITEM_DATA_INT_SET(_item, _data) \
  ((Evry_Item *)_item)->data = (void*)(long) _data

#define EVRY_ITEM_DATA_INT_GET(_item) (long) \
  ((Evry_Item *)_item)->data

#define EVRY_ITEM_DETAIL_SET(_it, _detail)			\
  if (EVRY_ITEM(_it)->detail)					\
    eina_stringshare_del(EVRY_ITEM(_it)->detail);		\
  EVRY_ITEM(_it)->detail = eina_stringshare_add(_detail);

#define EVRY_ITEM_LABEL_SET(_it, _label)			\
  if (EVRY_ITEM(_it)->label)					\
    eina_stringshare_del(EVRY_ITEM(_it)->label);		\
  EVRY_ITEM(_it)->label = eina_stringshare_add(_label);

#define EVRY_ITEM_CONTEXT_SET(_it, _context)			\
  if (EVRY_ITEM(_it)->context)					\
    eina_stringshare_del(EVRY_ITEM(_it)->context);		\
  EVRY_ITEM(_it)->context = eina_stringshare_add(_context);

#define EVRY_ITEM_ICON_SET(_it, _icon)				\
  if (EVRY_ITEM(_it)->icon)					\
    eina_stringshare_del(EVRY_ITEM(_it)->icon);			\
  EVRY_ITEM(_it)->icon = eina_stringshare_add(_icon);

#define CHECK_TYPE(_item, _type) \
  (((Evry_Item *)_item)->type && ((Evry_Item *)_item)->type == _type)

#define CHECK_SUBTYPE(_item, _type) \
  (((Evry_Item *)_item)->subtype && ((Evry_Item *)_item)->subtype == _type)

#define IS_BROWSEABLE(_item) ((Evry_Item *)_item)->browseable

/*** Evry_Plugin macros ***/
#define EVRY_PLUGIN_NEW(_base, _name, _icon, _item_type, _begin, _finish, _fetch, _free) \
  evry->plugin_new(EVRY_PLUGIN(E_NEW(_base, 1)), _name, _(_name), _icon, _item_type,     \
		   _begin, _finish, _fetch, _free)

#define EVRY_PLUGIN_FREE(_p) if (_p) evry->plugin_free(EVRY_PLUGIN(_p))

#define EVRY_PLUGIN_ITEMS_FREE(_p) {				\
     Evry_Item *it;						\
     EINA_LIST_FREE(EVRY_PLUGIN(_p)->items, it)			\
       evry->item_free(it); }

#define EVRY_PLUGIN_ITEMS_ADD(_plugin, _items, _input, _match_detail, _set_usage) \
  evry->util_plugin_items_add(EVRY_PLUGIN(_plugin), _items, _input, _match_detail, _set_usage)

#define EVRY_PLUGIN_UPDATE(_p, _action)	\
  if (_p) evry->plugin_update(EVRY_PLUGIN(_p), _action)

#define EVRY_PLUGIN_ITEMS_SORT(_p, _sortcb)			\
  EVRY_PLUGIN(_p)->items = eina_list_sort			\
    (EVRY_PLUGIN(_p)->items, eina_list_count(EVRY_PLUGIN(_p)->items), _sortcb)

#define EVRY_PLUGIN_ITEM_APPEND(_p, _item) \
  EVRY_PLUGIN(_p)->items = eina_list_append(EVRY_PLUGIN(_p)->items, EVRY_ITEM(_item))

typedef void (*Evry_Item_Free_Cb) (Evry_Item *it);

#define EVRY_PLUGIN_INSTANCE(_p, _plugin) {				  \
     _p			   = E_NEW(Plugin, 1);				  \
     _p->base              = *_plugin;					  \
     _p->base.items        = NULL;					  \
     evry->item_new(&_p->base.base, (Evry_Plugin*)_p,			  \
		   _plugin->base.label, NULL,				  \
		   (Evry_Item_Free_Cb)_p->base.finish);			  \
     _p->base.base.detail  = eina_stringshare_add(_plugin->base.detail);  \
     _p->base.base.icon    = eina_stringshare_add(_plugin->base.icon);	  \
     _p->base.base.context = eina_stringshare_add(_plugin->base.context); \
     _p->base.base.id      = eina_stringshare_add(_plugin->base.id);	  \
  }

#define EVRY_PLUGIN_ITEMS_CLEAR(_p) {				\
     Evry_Item *it;						\
     EINA_LIST_FREE(EVRY_PLUGIN(_p)->items, it)			\
       if (it) it->fuzzy_match = 0; }

/*** Evry_Action macros ***/
#define EVRY_ACTION_NEW(_name, _in1, _in2, _icon, _action, _check) \
  evry->action_new(_name, _(_name), _in1, _in2, _icon, _action, _check)

#define EVRY_ACTION_FREE(_act) if (_act) evry->action_free(EVRY_ACTN(_act))

#define EVRY_MODULE_NEW(_module, _init, _shutdown)	\
  {							\
     _module = E_NEW(Evry_Module, 1);			\
     _module->init     = &_init;			\
     _module->shutdown = &_shutdown;			\
     Eina_List *l = e_datastore_get("evry_modules");	\
     l = eina_list_append(l, _module);			\
     e_datastore_set("evry_modules", l);		\
     if ((e_datastore_get("evry_active")))		\
       evry_module->active = _init();			\
  }

#define EVRY_MODULE_FREE(_module)			\
  {							\
     _module->shutdown();				\
     Eina_List *l = e_datastore_get("evry_modules");	\
     l = eina_list_remove(l, _module);			\
     if (l) e_datastore_set("evry_modules", l);		\
     else e_datastore_del("evry_modules");		\
     E_FREE(_module);					\
  }


/*** handy macros ***/

#define IF_RELEASE(x) do {					\
     if (x) {							\
	const char *__tmp; __tmp = (x);				\
	(x) = NULL; eina_stringshare_del(__tmp);		\
     }								\
     (x) = NULL;						\
  } while (0)


#ifndef EVRY_H

#ifndef EINA_LOG_DEFAULT_COLOR
#define EINA_LOG_DEFAULT_COLOR EINA_COLOR_CYAN
#endif

#undef DBG
#undef INF
#undef WRN
#undef ERR

#define DBG(...) EINA_LOG_DOM_DBG(evry->log_dom , __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(evry->log_dom , __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(evry->log_dom , __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(evry->log_dom , __VA_ARGS__)

#endif
#endif
