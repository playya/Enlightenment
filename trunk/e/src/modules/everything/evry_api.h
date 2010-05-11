#ifndef EVRY_API_H
#define EVRY_API_H

#ifndef EVRY_H
#include "evry_types.h"

#define EVRY_API_VERSION 18

#define EVRY_ACTION_OTHER    0
#define EVRY_ACTION_FINISHED 1
#define EVRY_ACTION_CONTINUE 2

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

EAPI extern Evry_Type EVRY_TYPE_NONE;
EAPI extern Evry_Type EVRY_TYPE_FILE;
EAPI extern Evry_Type EVRY_TYPE_DIR;
EAPI extern Evry_Type EVRY_TYPE_APP;
EAPI extern Evry_Type EVRY_TYPE_ACTION;
EAPI extern Evry_Type EVRY_TYPE_PLUGIN;
EAPI extern Evry_Type EVRY_TYPE_BORDER;
EAPI extern Evry_Type EVRY_TYPE_TEXT;

#endif

typedef struct _Evry_API Evry_API;
typedef struct _Evry_Module Evry_Module;

/***************************************************
  register module struct:

   Eina_List *l;
   Evry_Module *em;
   
   em = E_NEW(Evry_Module, 1);
   em->init     = &_your_init_func;
   em->shutdown = &_your_shutdown_func;

   l = e_datastore_get("everything_modules");
   l = eina_list_append(l, em);
   e_datastore_set("everything_modules", l);
***************************************************/

struct _Evry_Module
{
  int  (*init)(const Evry_API *api);
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

  Evry_Plugin *(*plugin_new)(Evry_Plugin *base, const char *name,
				  const char *label, const char *icon,
				  Evry_Type item_type,
				  Evry_Plugin *(*begin) (Evry_Plugin *p, const Evry_Item *item),
				  void (*cleanup) (Evry_Plugin *p),
				  int  (*fetch)   (Evry_Plugin *p, const char *input),
				  void (*free) (Evry_Plugin *p));

  void (*plugin_free)(Evry_Plugin *p);
  /* when a new plugin config was created return val is 1. in this
     case you can set defaults of p->config otherwise zero */
  int  (*plugin_register)(Evry_Plugin *p, int type, int priority);
  void (*plugin_unregister)(Evry_Plugin *p);
  void (*plugin_update)(Evry_Plugin *plugin, int state);
  
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
  void  (*event_item_changed)(Evry_Item *it, int change_icon, int change_selected);

  const char *(*file_path_get)(Evry_Item_File *file);
  const char *(*file_url_get)(Evry_Item_File *file);

  int log_dom;
};

#ifndef EVRY_H
#define EVRY_ITEM(_item) ((Evry_Item *)_item)
#define EVRY_ACTN(_item) ((Evry_Action *) _item)
#define EVRY_PLUGIN(_plugin) ((Evry_Plugin *) _plugin)
#define EVRY_VIEW(_view) ((Evry_View *) _view)
#define EVRY_FILE(_it) ((Evry_Item_File *) _it)

#define CHECK_TYPE(_item, _type)					\
  (((Evry_Item *)_item)->type && ((Evry_Item *)_item)->type == _type)

#define CHECK_SUBTYPE(_item, _type)					\
  (((Evry_Item *)_item)->subtype && ((Evry_Item *)_item)->subtype == _type)

#define IS_BROWSEABLE(_item) ((Evry_Item *)_item)->browseable

#define GET_APP(_app, _item) Evry_Item_App *_app = (Evry_Item_App *) _item
#define GET_FILE(_file, _item) Evry_Item_File *_file = (Evry_Item_File *) _item
#define GET_EVRY_PLUGIN(_p, _plugin) Evry_Plugin *_p = (Evry_Plugin*) _plugin
#define GET_VIEW(_v, _view) View *_v = (View*) _view
#define GET_ACTION(_act, _item) Evry_Action *_act = (Evry_Action *) _item
#define GET_PLUGIN(_p, _plugin) Plugin *_p = (Plugin*) _plugin
#define GET_ITEM(_it, _any) Evry_Item *_it = (Evry_Item *) _any

#define EVRY_ITEM_DATA_INT_SET(_item, _data) ((Evry_Item *)_item)->data = (void*)(long) _data
#define EVRY_ITEM_DATA_INT_GET(_item) (long) ((Evry_Item *)_item)->data
#define EVRY_ITEM_ICON_SET(_item, _icon) ((Evry_Item *)_item)->icon = _icon

#define EVRY_ITEM_DETAIL_SET(_it, _detail) \
  if (EVRY_ITEM(_it)->detail) eina_stringshare_del(EVRY_ITEM(_it)->detail); \
  EVRY_ITEM(_it)->detail = eina_stringshare_add(_detail);

#define EVRY_ITEM_LABEL_SET(_it, _label) \
  if (EVRY_ITEM(_it)->label) eina_stringshare_del(EVRY_ITEM(_it)->label); \
  EVRY_ITEM(_it)->label = eina_stringshare_add(_label);

#define EVRY_ITEM_CONTEXT_SET(_it, _context) \
  if (EVRY_ITEM(_it)->context) eina_stringshare_del(EVRY_ITEM(_it)->context); \
  EVRY_ITEM(_it)->context = eina_stringshare_add(_context);

#define EVRY_ITEM_NEW(_base, _plugin, _label, _icon_get, _free)	\
  (_base *) evry->item_new(EVRY_ITEM(E_NEW(_base, 1)), EVRY_PLUGIN(_plugin), \
			  _label, _icon_get, _free)

#define EVRY_ITEM_FREE(_item) evry_item_free((Evry_Item *)_item)

#define EVRY_PLUGIN_NEW(_base, _name, _icon, _item_type, _begin, _cleanup, _fetch, _free) \
  evry->plugin_new(EVRY_PLUGIN(E_NEW(_base, 1)), _name, _(_name), _icon, _item_type, \
		  _begin, _cleanup, _fetch, _free)


#define EVRY_ACTION_NEW(_name, _in1, _in2, _icon, _action, _check) \
  evry->action_new(_name, _(_name), _in1, _in2, _icon, _action, _check)


#define EVRY_PLUGIN_FREE(_p) if (_p) evry->plugin_free(EVRY_PLUGIN(_p))


#define EVRY_PLUGIN_ITEMS_CLEAR(_p) { \
     Evry_Item *it;		      \
     EINA_LIST_FREE(EVRY_PLUGIN(_p)->items, it) \
       it->fuzzy_match = 0; }

#define EVRY_PLUGIN_ITEMS_FREE(_p) { \
     Evry_Item *it;		     \
     EINA_LIST_FREE(EVRY_PLUGIN(_p)->items, it) \
       evry->item_free(it); }


#define EVRY_PLUGIN_ITEMS_SORT(_p, _sortcb) \
  EVRY_PLUGIN(_p)->items = eina_list_sort   \
    (EVRY_PLUGIN(_p)->items, eina_list_count(EVRY_PLUGIN(_p)->items), _sortcb)

#define EVRY_PLUGIN_ITEM_APPEND(_p, _item)  \
  EVRY_PLUGIN(_p)->items = eina_list_append(EVRY_PLUGIN(_p)->items, EVRY_ITEM(_item))

// should be renamed to ITEMS_FILTER
#define EVRY_PLUGIN_ITEMS_ADD(_plugin, _items, _input, _match_detail, _set_usage) \
  evry->util_plugin_items_add(EVRY_PLUGIN(_plugin), _items, _input, _match_detail, _set_usage)

#define EVRY_PLUGIN_UPDATE(_p, _action)	\
  if (_p) evry->plugin_update(EVRY_PLUGIN(_p), _action)

#define IF_RELEASE(x) do {						\
     if (x) {								\
	const char *__tmp; __tmp = (x); (x) = NULL; eina_stringshare_del(__tmp); \
     }									\
     (x) = NULL;							\
  } while (0)

#define EVRY_MODULE_REGISTER(_module) { \
     Eina_List *l = e_datastore_get("everything_modules");	\
     l = eina_list_append(l, _module);				\
     e_datastore_set("everything_modules", l); }

#define EVRY_MODULE_UNREGISTER(_module) { \
     Eina_List *l = e_datastore_get("everything_modules");	\
     l = eina_list_remove(l, _module);				\
     e_datastore_set("everything_modules", l); }


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

