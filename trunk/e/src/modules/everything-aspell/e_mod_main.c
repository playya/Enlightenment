/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Evry.h"
#include "e_mod_main.h"
#include <ctype.h>

static const char TRIGGER[] = "s ";
static const char LANG_MODIFIER[] = "lang=";

typedef struct _Plugin Plugin;
typedef struct _Module_Config Module_Config;

static Module_Config *_conf;
static char _config_path[] =  "extensions/everthing-aspell";
static char _config_domain[] = "module.everyhing-aspell";
static char _module_icon[] = "accessories-dictionary";

static E_Config_DD *_conf_edd = NULL;

struct _Plugin
{
  Evry_Plugin base;
  struct
  {
    Ecore_Event_Handler *data;
    Ecore_Event_Handler *del;
  } handler;
  Ecore_Exe *exe;
  const char *lang;
  const char *input;
  Eina_Bool is_first;
};

struct _Module_Config
{
  int version;

  const char *lang;
  const char *custom;
  int command;

  E_Config_Dialog *cfd;
  E_Module *module;
};

static Plugin *_plug = NULL;

static char *commands[] =
  {
    "aspell -a --encoding=UTF-8 %s %s",
    "hunspell -a -i utf-8 %s %s"
  };
#define CMD_ASPELL   1
#define CMD_HUNSPELL 2

static Eina_Bool
_exe_restart(Plugin *p)
{
   char cmd[1024];
   const char *lang_opt, *lang_val;
   int len;

   if (p->lang && (p->lang[0] != '\0'))
     {
	if (_conf->command == CMD_ASPELL)
	  {
	     lang_opt = "-l";
	     lang_val = p->lang;
	  }
	else if (_conf->command == CMD_HUNSPELL)
	  {
	     lang_opt = "-d";
	     lang_val = p->lang;
	  }
	else
	  {
	     lang_opt = "";
	     lang_val = "";
	  }
     }
   else if (_conf->lang)
     {
	if (_conf->command == CMD_ASPELL)
	  {
	     lang_opt = "-l";
	     lang_val = _conf->lang;
	  }
	else if (_conf->command == CMD_HUNSPELL)
	  {
	     lang_opt = "-d";
	     lang_val = _conf->lang;
	  }
	else
	  {
	     lang_opt = "";
	     lang_val = "";
	  }
     }
   else
     {
	lang_opt = "";
	lang_val = "";
     }

   len = snprintf(cmd, sizeof(cmd),
		  commands[_conf->command - 1],
		  lang_opt, lang_val);
   if (len >= (int)sizeof(cmd))
     return 0;

   if (p->exe)
     ecore_exe_quit(p->exe);
   p->exe = ecore_exe_pipe_run
     (cmd,
      ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_READ_LINE_BUFFERED |
      ECORE_EXE_PIPE_WRITE,
      NULL);
   p->is_first = 1;
   return !!p->exe;
}

static const char *
_space_skip(const char *line)
{
   for (; *line != '\0'; line++)
     if (!isspace(*line))
       break;
   return line;
}

static const char *
_space_find(const char *line)
{
   for (; *line != '\0'; line++)
     if (isspace(*line))
       break;
   return line;
}

static void
_item_add(Plugin *p, const char *word, int word_size, int prio)
{
   Evry_Item *it;

   it = EVRY_ITEM_NEW(Evry_Item, p, NULL, NULL, NULL);
   if (!it) return;
   it->priority = prio;
   it->label = eina_stringshare_add_length(word, word_size);

   EVRY_PLUGIN_ITEM_APPEND(p, it);
}

static void
_suggestions_add(Plugin *p, const char *line)
{
   const char *s;

   s = strchr(line, ':');
   if (!s)
     {
	ERR("ASPELL: ERROR missing suggestion delimiter: '%s'", line);
	return;
     }
   s++;

   line = _space_skip(s);
   while (*line)
     {
	int len;

	s = strchr(line, ',');
	if (s)
	  len = s - line;
	else
	  len = strlen(line);

	_item_add(p, line, len, 1);

	if (s)
	  line = _space_skip(s + 1);
	else
	  break;
     }
}

static int
_cb_data(void *data, int type __UNUSED__, void *event)
{
   GET_PLUGIN(p, data);
   Ecore_Exe_Event_Data *e = event;
   Ecore_Exe_Event_Data_Line *l;
   const char *word;

   if (e->exe != p->exe)
     return 1;

   EVRY_PLUGIN_ITEMS_FREE(p);

   word = p->input;
   for (l = e->lines; l && l->line; l++)
     {
	const char *word_end;
	int word_size;

	if (p->is_first)
	  {
	     ERR("ASPELL: %s", l->line);
	     p->is_first = 0;
	     continue;
	  }

	word_end = _space_find(word);
	word_size = word_end - word;

	switch (l->line[0])
	  {
	   case '*':
	      _item_add(p, word, word_size, 1);
	      break;
	   case '&':
	      _item_add(p, word, word_size, 1);
	      _suggestions_add(p, l->line);
	      break;
	   case '#':
	      break;
	   case '\0':
	      break;
	   default:
	      ERR("ASPELL: unknown output: '%s'", l->line);
	  }

	if (*word_end)
	  word = _space_skip(word_end + 1);
     }

   if (EVRY_PLUGIN(p)->items)
     {
	evry_list_win_show();
     }

   if (p->base.items)
     EVRY_PLUGIN_UPDATE(p, EVRY_UPDATE_ADD);

   return 1;
}

static int
_cb_del(void *data, int type __UNUSED__, void *event)
{
   Plugin *p = data;
   Ecore_Exe_Event_Del *e = event;

   if (e->exe != p->exe)
     return 1;

   p->exe = NULL;
   return 1;
}

static int
_begin(Evry_Plugin *plugin, const Evry_Item *it __UNUSED__)
{
   GET_PLUGIN(p, plugin);

   if (!p->handler.data)
     p->handler.data = ecore_event_handler_add
       (ECORE_EXE_EVENT_DATA, _cb_data, p);
   if (!p->handler.del)
     p->handler.del = ecore_event_handler_add
       (ECORE_EXE_EVENT_DEL, _cb_del, p);

   return _exe_restart(p);
}

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
   GET_PLUGIN(p, plugin);
   const char *s;
   int len;

   if (!input) return 1;

   if (strlen(input) < plugin->config->min_query)
     {
	EVRY_PLUGIN_ITEMS_FREE(p);
	return 0;
     }

   if (!p->handler.data && !_begin(plugin, NULL)) return 0;

   len = sizeof(LANG_MODIFIER) - 1;
   if (strncmp(input, LANG_MODIFIER, len) == 0)
     {
	const char *lang;

	EVRY_PLUGIN_ITEMS_FREE(p);

	input += len;
	for (s = input; *s != '\0'; s++)
	  if (isspace(*s) || *s == ';')
	    break;

	if (*s == '\0') /* just apply language on ' ' or ';' */
	  return 1;

	if (s - input > 0)
	  lang = eina_stringshare_add_length(input, s - input);
	else
	  lang = NULL;

	if (p->lang) eina_stringshare_del(p->lang);
	if (p->lang != lang)
	  {
	     p->lang = lang;
	     if (!_exe_restart(p))
	       return 1;
	  }

	if (*s == '\0')
	  return 1;

	input = s + 1;
     }

   input = _space_skip(input);
   for (s = input; *s != '\0'; s++)
     ;
   for (s--; s > input; s--)
     if (!isspace(*s))
       break;

   len = s - input + 1;
   if (len < 1)
     return 1;
   input = eina_stringshare_add_length(input, len);
   if (p->input) eina_stringshare_del(p->input);
   if (p->input == input)
     return 1;

   p->input = input;
   if (!p->exe)
     return 1;

   ecore_exe_send(p->exe, (char *)p->input, len);
   ecore_exe_send(p->exe, "\n", 1);

   return 1;
}

static void
_cleanup(Evry_Plugin *plugin)
{
   GET_PLUGIN(p, plugin);

   EVRY_PLUGIN_ITEMS_FREE(p);

   if (p->handler.data)
     {
	ecore_event_handler_del(p->handler.data);
	p->handler.data = NULL;
     }
   if (p->handler.del)
     {
	ecore_event_handler_del(p->handler.del);
	p->handler.del = NULL;
     }
   if (p->exe)
     {
	ecore_exe_quit(p->exe);
	ecore_exe_free(p->exe);
	p->exe = NULL;
     }
   if (p->lang)
     {
	eina_stringshare_del(p->lang);
	p->lang = NULL;
     }
   if (p->input)
     {
	eina_stringshare_del(p->input);
	p->input = NULL;
     }
}

static Eina_Bool
_plugins_init(void)
{
   Evry_Plugin *p;

   if (!evry_api_version_check(EVRY_API_VERSION))
     return EINA_FALSE;

   p = EVRY_PLUGIN_NEW(Plugin, N_("Spell Checker"),
		       _module_icon,
		       EVRY_TYPE_TEXT,
		       NULL, _cleanup, _fetch, NULL);
   p->config_path = _config_path;
   p->history     = EINA_FALSE;
   p->async_fetch = EINA_TRUE;
   
   if (evry_plugin_register(p, EVRY_PLUGIN_SUBJECT, 100))
     {
	Plugin_Config *pc = p->config;
	pc->view_mode = VIEW_MODE_LIST;
	pc->aggregate = EINA_FALSE;
	pc->top_level = EINA_FALSE;
	pc->trigger = eina_stringshare_add(TRIGGER);
	pc->trigger_only = EINA_TRUE;
	pc->min_query = 2;
     }
   
   _plug = (Plugin *) p;

   return EINA_TRUE;
}

static void
_plugins_shutdown(void)
{
   EVRY_PLUGIN_FREE(_plug);
}

/***************************************************************************/

struct _E_Config_Dialog_Data
{
  int  command;
  char *custom;
  char *lang;
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

   if (e_config_dialog_find(_config_path, _config_path))
     return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   cfd = e_config_dialog_new(con, _("Everything Aspell"),
			     _config_path, _config_path, _module_icon, 0, v, NULL);

   _conf->cfd = cfd;
   return cfd;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;
   E_Radio_Group *rg;
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("General"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);

   rg = e_widget_radio_group_new(&cfdata->command);

   ow = e_widget_label_add(evas, _("Spell checker"));
   e_widget_framelist_object_append(of, ow);

   ow = e_widget_radio_add(evas, _("Aspell"), 1, rg);
   e_widget_framelist_object_append(of, ow);

   ow = e_widget_radio_add(evas, _("Hunspell"), 2, rg);
   e_widget_framelist_object_append(of, ow);

   ow = e_widget_radio_add(evas, _("Custom"), 0, rg);
   e_widget_disabled_set(ow, 1);
   e_widget_framelist_object_append(of, ow);

   ow = e_widget_label_add(evas, _("Custom Command"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_entry_add(evas, &cfdata->custom, NULL, NULL, NULL);
   e_widget_disabled_set(ow, 1);
   e_widget_framelist_object_append(of, ow);


   ow = e_widget_label_add(evas, _("Language"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_entry_add(evas, &cfdata->lang, NULL, NULL, NULL);
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
   E_FREE(cfdata->custom);
   E_FREE(cfdata->lang);
   _conf->cfd = NULL;
   E_FREE(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
#define CP(_name) cfdata->_name = _conf->_name ? strdup(_conf->_name) : strdup("");
#define C(_name) cfdata->_name = _conf->_name;
   C(command);
   CP(custom);
   CP(lang);
#undef CP
#undef C
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
#define CP(_name)					\
   if (_conf->_name)					\
     eina_stringshare_del(_conf->_name);		\
   _conf->_name = eina_stringshare_add(cfdata->_name);
#define C(_name) _conf->_name = cfdata->_name;
   C(command);
   CP(custom);
   CP(lang);
#undef CP
#undef C

   e_config_domain_save(_config_domain, _conf_edd, _conf);
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
   _conf->command = 1;
   _conf->custom = NULL;
   _conf->lang = eina_stringshare_add("en_US");
   IFMODCFGEND;

   _conf->version = MOD_CONFIG_FILE_VERSION;

   e_config_save_queue();
}

static void
_conf_free(void)
{
   if (_conf)
     {
	if (_conf->custom) eina_stringshare_del(_conf->custom);
	if (_conf->lang)   eina_stringshare_del(_conf->lang);

	E_FREE(_conf);
     }
}

static void
_conf_init(E_Module *m)
{
   e_configure_registry_item_add(_config_path, 110, _("Everything Aspell"),
				 NULL, _module_icon, _conf_dialog);

   _conf_edd = E_CONFIG_DD_NEW("Module_Config", Module_Config);

#undef T
#undef D
#define T Module_Config
#define D _conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, command, INT);
   E_CONFIG_VAL(D, T, custom, STR);
   E_CONFIG_VAL(D, T, lang, STR);
#undef T
#undef D

   _conf = e_config_domain_load(_config_domain, _conf_edd);

   if (_conf && !evry_util_module_config_check(_("Everything Aspell"), _conf->version,
					       MOD_CONFIG_FILE_EPOCH, MOD_CONFIG_FILE_VERSION))
     _conf_free();

   if (!_conf) _conf_new();

   _conf->module = m;
}

static void
_conf_shutdown(void)
{
   _conf_free();

   E_CONFIG_DD_FREE(_conf_edd);
}

/***************************************************************************/

static Eina_Bool active = EINA_FALSE;

EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "everything-aspell"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   if (e_datastore_get("everything_loaded"))
     active = _plugins_init();

   _conf_init(m);

   e_module_delayed_set(m, 1);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   if (active && e_datastore_get("everything_loaded"))
     _plugins_shutdown();

   _conf_shutdown();

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}

/***************************************************************************/
