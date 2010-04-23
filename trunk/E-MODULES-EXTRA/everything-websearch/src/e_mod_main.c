/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <Evry.h>
#include "e_mod_main.h"

typedef struct _Plugin Plugin;
typedef int (*Handler_Func) (void *data, int type, void *event);
typedef struct _Module_Config Module_Config;
struct _Plugin
{
  Evry_Plugin base;

  Ecore_Con_Server *svr;
  Ecore_Event_Handler *handler;
  Ecore_Timer *timer;
  
  const char *input;  
  const char *server_address;
  const char *request;
  
  int (*fetch) (void *data);
  
};

struct _Module_Config
{
  int version;

  const char *lang;
  const char *browser;
  const char *trigger_google;
  const char *trigger_wiki;
  unsigned char search_google;
  unsigned char search_wiki;
  
  E_Config_Dialog *cfd;
  E_Module *module;
  char *theme;
};

static Module_Config *_conf;

static Plugin *_plug1 = NULL;
static Plugin *_plug2 = NULL;
static Evry_Action *_act1 = NULL;
static Evry_Action *_act2 = NULL;
static char _header[] =
  "User-Agent: Wget/1.12 (linux-gnu)\n"
  "Accept: */*\n"
  "Connection: Keep-Alive\n\n";

int
_server_data(void *data, int ev_type, Ecore_Con_Event_Server_Data *ev)
{
  Plugin *p = data;
  char *result = (char *)ev->data;
  Evry_Item *it;
  char *list;

  /* printf("- %s\n", result); */
  
  if (ev->server != p->svr) return 1;

  EVRY_PLUGIN_ITEMS_FREE(p);

  if ((list = strstr(result, "[[\""))) 
    {
      list += 3;

      char **items = eina_str_split(list, "\"],[\"", 0);
      char **i;
	  
      for(i = items; *i; i++)
	{
	  char **item= eina_str_split(*i, "\",\"", 2);	      
	  it = evry_item_new(NULL, EVRY_PLUGIN(p), *item, NULL);
	  it->detail = eina_stringshare_add(*(item + 1));
	  EVRY_PLUGIN_ITEM_APPEND(p, it);
	  free(*item);
	  free(item);
	}
	  
      free(*items);
      free(items);

      evry_plugin_async_update (EVRY_PLUGIN(p), EVRY_ASYNC_UPDATE_ADD);
    }
  else if ((list = strstr(result, ",[\""))) 
    {
      list += 3;

      char **items = eina_str_split(list, "\"", 0);
      char **i;
      for(i = items; *i; i++)
	{
	  if (**i == ',' || **i == ']') continue;
	  it = evry_item_new(NULL, EVRY_PLUGIN(p), *i, NULL);
	  it->detail = eina_stringshare_add("Wikipedia");
	  EVRY_PLUGIN_ITEM_APPEND(p, it);
	}
	  
      free(*items);
      free(items);

    }
  if (EVRY_PLUGIN(p)->items)
    evry_plugin_async_update (EVRY_PLUGIN(p), EVRY_ASYNC_UPDATE_ADD);
  
  return 1;
}

static Evry_Plugin *
_begin(Evry_Plugin *plugin, const Evry_Item *it)
{
  PLUGIN(p, plugin);

  if (!_conf->search_google && (p == _plug1)) return NULL;
  if (!_conf->search_wiki   && (p == _plug2)) return NULL;
  
  p->handler = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
				       (Handler_Func)_server_data, p);
  return plugin;
}

static void
_cleanup(Evry_Plugin *plugin)
{
  PLUGIN(p, plugin);

  if (p->svr) ecore_con_server_del(p->svr);
  p->svr = NULL;
  
  ecore_event_handler_del(p->handler);
  p->handler = NULL;
  
  EVRY_PLUGIN_ITEMS_FREE(p);
}

static int
_send_request(void *data)
{
  Plugin *p = data;
  char buf[1024];
  char *query;

  query = evry_util_url_escape(p->input, 0);

  if (p->svr) ecore_con_server_del(p->svr);
  p->svr = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM,
				    p->server_address, 80, NULL);

  if (p->svr)
    {
      query = evry_util_url_escape(p->input, 0); 
  
      snprintf(buf, sizeof(buf), p->request,
  	       _conf->lang, query, _header);

      /* printf("send: %s\n", buf); */
  
      ecore_con_server_send(p->svr, buf, strlen(buf));
    }

  free(query);
  
  p->timer = NULL;
  
  return 0;
}

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
  PLUGIN(p, plugin);
  
  if (p->input)
    eina_stringshare_del(p->input);
  p->input = NULL;

  if (p->timer)
    ecore_timer_del(p->timer);
  p->timer = NULL;
  
  if (input && strlen(input) > 2)
    {        
      p->input = eina_stringshare_add(input);  
      p->timer = ecore_timer_add(0.3, _send_request, p);
    }
  
  return 0;
}

static int
_action(Evry_Action *act)
{
  Evry_Item_App *app = E_NEW(Evry_Item_App, 1);
  Evry_Item_File *file = E_NEW(Evry_Item_File, 1);
  char buf[1024];
  Eina_List *l;
  E_Border *bd;
  
  app->desktop = efreet_util_desktop_exec_find(_conf->browser);
  if (!app->desktop)
    app->file = "xdg-open";

  if (!strncmp((char *)act->data, "g", 1))
    {
      snprintf(buf, sizeof(buf), "http://www.google.com/search?hl=%s&q=%s",
	       _conf->lang, act->item1->label);
    }
  else if (!strncmp((char *)act->data, "w", 1))
    {
      snprintf(buf, sizeof(buf), "http://%s.wikipedia.org/wiki/%s",
	       _conf->lang, act->item1->label);
    }
  
  file->path = buf;
  
  evry_util_exec_app(EVRY_ITEM(app), EVRY_ITEM(file));

  if (app->desktop)
    {
      EINA_LIST_FOREACH(e_border_client_list(), l, bd)
	{
	  if (bd->desktop && bd->desktop == app->desktop)
	    {
	      e_desk_show(bd->desk);
	      e_border_raise(bd);
	      break;
	    }
	}
      efreet_desktop_free(app->desktop);
    }

  free(app);
  free(file);
}

Evas_Object *
_act_icon_get(Evry_Action *act, Evas *e)
{
  Evas_Object *o = e_icon_add(e);
  if (e_icon_file_edje_set(o, _conf->theme, act->data))
    return o;
  
  evas_object_del(o);

  return NULL;
}


static Eina_Bool
module_init(void)
{
  if (!evry_api_version_check(EVRY_API_VERSION))
    return EINA_FALSE;
  
  if (!ecore_con_init())
    return EINA_FALSE;

  _plug1 = E_NEW(Plugin, 1);
  _plug1->server_address = "www.google.com";
  _plug1->request =
    "GET http://www.google.com/complete/search?hl=%s&output=text&q=\"%s\n%s";
  EVRY_PLUGIN_NEW(_plug1, "GSuggest", type_subject, "", "TEXT",
		  _begin, _cleanup, _fetch, NULL, NULL);

  EVRY_PLUGIN(_plug1)->trigger = _conf->trigger_google;
  EVRY_PLUGIN(_plug1)->icon = "text-html";
  evry_plugin_register(EVRY_PLUGIN(_plug1), 10);

  _plug2 = E_NEW(Plugin, 1);
  _plug2->server_address = "www.wikipedia.org";
  _plug2->request =
    "GET http://%s.wikipedia.org/w/api.php?action=opensearch&search=%s HTTP/1.0\n%s";
  EVRY_PLUGIN_NEW(_plug2, "Wikipedia", type_subject, "", "TEXT",
		  _begin, _cleanup, _fetch, NULL, NULL);
  EVRY_PLUGIN(_plug2)->trigger = _conf->trigger_wiki;
  EVRY_PLUGIN(_plug2)->icon = "text-html";
  evry_plugin_register(EVRY_PLUGIN(_plug2), 9);

  
  _act1 = EVRY_ACTION_NEW("Google for it", "TEXT", NULL, "go-next", _action, NULL);
  evry_action_register(_act1, 1);
  _act1->data = "google";
  _act1->icon_get = &_act_icon_get;
  
  _act2 = EVRY_ACTION_NEW("Wikipedia Page", "TEXT", NULL, "go-next", _action, NULL);
  evry_action_register(_act2, 1);
  _act2->data = "wikipedia";
  _act2->icon_get = &_act_icon_get;
  
  return EINA_TRUE;
}

static void
module_shutdown(void)
{
  EVRY_PLUGIN_FREE(_plug1);
  EVRY_PLUGIN_FREE(_plug2);

  evry_action_free(_act1); 
  evry_action_free(_act2); 
  ecore_con_shutdown();
}

/***************************************************************************/


static E_Config_DD *conf_edd = NULL;

struct _E_Config_Dialog_Data
{
  char *browser;
  char *lang;
  char *trigger_google;
  char *trigger_wiki;
  int search_wiki;
  int search_google;  
};

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static E_Config_Dialog *
_conf_dialog(E_Container *con, const char *params)
{
  E_Config_Dialog *cfd = NULL;
  E_Config_Dialog_View *v = NULL;
  char buf[4096];

  if (e_config_dialog_find("everything-websearch", "extensions/everything-websearch"))
    return NULL;

  v = E_NEW(E_Config_Dialog_View, 1);
  if (!v) return NULL;

  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.create_widgets = _basic_create;
  v->basic.apply_cfdata = _basic_apply;

  snprintf(buf, sizeof(buf), "%s/e-module.edj", _conf->module->dir);

  cfd = e_config_dialog_new(con, _("Everything Websearch"), "everything-websearch",
			    "extensions/everything-websearch", buf, 0, v, NULL);

  /* e_dialog_resizable_set(cfd->dia, 1); */
  _conf->cfd = cfd;
  return cfd;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
  Evas_Object *o = NULL, *of = NULL, *ow = NULL;

  o = e_widget_list_add(evas, 0, 0);

  of = e_widget_framelist_add(evas, _("General"), 0);
  e_widget_framelist_content_align_set(of, 0.0, 0.0);

  ow = e_widget_label_add(evas, _("Browser"));
  e_widget_framelist_object_append(of, ow);
  ow = e_widget_entry_add(evas, &cfdata->browser, NULL, NULL, NULL); 
  e_widget_framelist_object_append(of, ow);

  ow = e_widget_label_add(evas, _("Language"));
  e_widget_framelist_object_append(of, ow);
  ow = e_widget_entry_add(evas, &cfdata->lang, NULL, NULL, NULL); 
  e_widget_framelist_object_append(of, ow);

  ow = e_widget_check_add(evas, _("Search Google"),
			  &(cfdata->search_google));
  e_widget_framelist_object_append(of, ow);

  ow = e_widget_label_add(evas, _("Trigger for Google"));
  e_widget_framelist_object_append(of, ow);
  ow = e_widget_entry_add(evas, &cfdata->trigger_google, NULL, NULL, NULL); 
  e_widget_framelist_object_append(of, ow);

  ow = e_widget_check_add(evas, _("Search Wikipedia"),
			  &(cfdata->search_wiki));
  e_widget_framelist_object_append(of, ow);
  
  ow = e_widget_label_add(evas, _("Trigger for Wikipedia"));
  e_widget_framelist_object_append(of, ow);
  ow = e_widget_entry_add(evas, &cfdata->trigger_wiki, NULL, NULL, NULL); 
  e_widget_framelist_object_append(of, ow);

  e_widget_list_object_append(o, of, 1, 1, 0.5);
  return o;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
  E_Config_Dialog_Data *cfdata = NULL;

  cfdata = E_NEW(E_Config_Dialog_Data, 1);
  _fill_data(cfdata);
  return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
  E_FREE(cfdata->browser);
  E_FREE(cfdata->lang);
  E_FREE(cfdata->trigger_google);
  E_FREE(cfdata->trigger_wiki);
  _conf->cfd = NULL;
  E_FREE(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
#define CP(_name) cfdata->_name = strdup(_conf->_name);
#define C(_name) cfdata->_name = _conf->_name;
  CP(browser);
  CP(lang);
  CP(trigger_google);
  CP(trigger_wiki);
  C(search_google);
  C(search_wiki);  
#undef CP
#undef C
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
#define CP(_name)						\
  if (_conf->_name)						\
    eina_stringshare_del(_conf->_name);				\
  _conf->_name = eina_stringshare_add(cfdata->_name);
#define C(_name) _conf->_name = cfdata->_name;  
  CP(browser);
  CP(lang);
  CP(trigger_google);
  CP(trigger_wiki);
  C(search_google);
  C(search_wiki);  
#undef CP
#undef C

  e_config_domain_save("module.everything-websearch", conf_edd, _conf);
  e_config_save_queue();
  return 1;
}

static void
_conf_new(void)
{
  _conf = E_NEW(Module_Config, 1);
  _conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

  /* setup defaults */
  IFMODCFG(0x008d);
  _conf->browser = eina_stringshare_add("firefox");
  _conf->lang = eina_stringshare_add("en");
  _conf->trigger_google = eina_stringshare_add("g ");
  _conf->trigger_wiki = eina_stringshare_add("w ");
  _conf->search_wiki = 1;
  _conf->search_google = 1;
  IFMODCFGEND;

  _conf->version = MOD_CONFIG_FILE_VERSION;

  e_config_domain_save("module.everything-websearch", conf_edd, _conf);
  e_config_save_queue();
}

static void
_conf_free(void)
{
  if (_conf)
    {
      eina_stringshare_del(_conf->trigger_google); 
      eina_stringshare_del(_conf->trigger_wiki); 
      eina_stringshare_del(_conf->browser); 
      eina_stringshare_del(_conf->lang); 

      free(_conf->theme);

      E_FREE(_conf);
    }
}

static void
_conf_init(E_Module *m)
{
  char buf[4096];

  snprintf(buf, sizeof(buf), "%s/e-module.edj", m->dir);

  e_configure_registry_category_add("extensions", 80, _("Extensions"),
				    NULL, "preferences-extensions");

  e_configure_registry_item_add("extensions/everything-websearch", 110, _("Everything Websearch"),
				NULL, buf, _conf_dialog);

  conf_edd = E_CONFIG_DD_NEW("Module_Config", Module_Config);
   
#undef T
#undef D
#define T Module_Config
#define D conf_edd
  E_CONFIG_VAL(D, T, version, INT);
  E_CONFIG_VAL(D, T, browser, STR);
  E_CONFIG_VAL(D, T, lang, STR);
  E_CONFIG_VAL(D, T, search_google, UCHAR);
  E_CONFIG_VAL(D, T, trigger_google, STR);
  E_CONFIG_VAL(D, T, search_wiki, UCHAR);
  E_CONFIG_VAL(D, T, trigger_wiki, STR);
#undef T
#undef D

  _conf = e_config_domain_load("module.everything-websearch", conf_edd);

  if (_conf && !evry_util_module_config_check(_("Everything Websearch"), _conf->version,
					      MOD_CONFIG_FILE_EPOCH, MOD_CONFIG_FILE_VERSION))
    _conf_free();

  if (!_conf) _conf_new();

  _conf->module = m;
  _conf->theme = strdup(buf);
}

static void
_conf_shutdown(void)
{
  _conf_free();

  E_CONFIG_DD_FREE(conf_edd);
}

static E_Module *module = NULL;
static Eina_Bool _active = EINA_FALSE;

/***************************************************************************/
/**/
/* module setup */
EAPI E_Module_Api e_modapi = 
  {
    E_MODULE_API_VERSION,
    PACKAGE
  };

EAPI void *
e_modapi_init(E_Module *m)
{
  char buf[4096];

  snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
  bindtextdomain(PACKAGE, buf);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  
  if (e_datastore_get("everything_loaded"))
    {
      _conf_init(m);      
      _active = module_init();
    }
  
  e_module_delayed_set(m, 1); 

  return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
  if (_active && e_datastore_get("everything_loaded"))
    module_shutdown();

  _conf_shutdown();
  
  return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
  return 1;
}

/**/
/***************************************************************************/

