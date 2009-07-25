#include "e.h"
#include "e_mod_main.h"

typedef struct _Inst Inst;

struct _Inst
{
  Eina_Hash *added;
  Eina_List *apps;
  Evry_Item *candidate;
};

static int  _begin(Evry_Plugin *p, Evry_Item *item);
static int  _fetch(Evry_Plugin *p, const char *input);
static int  _action(Evry_Plugin *p, Evry_Item *item, const char *input);
static void _cleanup(Evry_Plugin *p);
static void _item_add(Evry_Plugin *p, Efreet_Desktop *desktop, char *file, int prio);
static int  _cb_sort(const void *data1, const void *data2);
static void _item_icon_get(Evry_Plugin *p, Evry_Item *it, Evas *e);
static int _exec_app_action(Evry_Action *act);
static int _exec_app_check_item(Evry_Action *act, Evry_Item *it);
static int _edit_app_action(Evry_Action *act);
static int _edit_app_check_item(Evry_Action *act, Evry_Item *it);
static int _new_app_action(Evry_Action *act);
static int _new_app_check_item(Evry_Action *act, Evry_Item *it);

static Evry_Plugin *p1;
static Evry_Plugin *p2;
static Evry_Action *act;
static Evry_Action *act2;
static Evry_Action *act3;
static Inst *inst;


EAPI int
evry_plug_apps_init(void)
{
   p1 = E_NEW(Evry_Plugin, 1);
   p1->name = "Applications";
   p1->type_in  = "NONE";
   p1->type_out = "APPLICATION";
   p1->need_query = 0;
   p1->begin = &_begin;
   p1->fetch = &_fetch;
   p1->action = &_action;
   p1->cleanup = &_cleanup;
   p1->icon_get = &_item_icon_get;
   evry_plugin_register(p1);

   p2 = E_NEW(Evry_Plugin, 1);
   p2->name = "Open With...";
   p2->type_in  = "FILE";
   p2->type_out = "NONE";
   p2->need_query = 0;
   p2->begin = &_begin;
   p2->fetch = &_fetch;
   p2->action = &_action;
   p2->cleanup = &_cleanup;
   p2->icon_get = &_item_icon_get;
   evry_plugin_register(p2);

   act = E_NEW(Evry_Action, 1);
   act->name = "Open File...";
   act->type_in1 = "APPLICATION";
   act->type_in2 = "FILE";
   act->type_out = "NONE";
   act->action = &_exec_app_action;
   act->check_item = &_exec_app_check_item;
   evry_action_register(act);

   act2 = E_NEW(Evry_Action, 1);
   act2->name = "Edit Application Entry";
   act2->type_in1 = "APPLICATION";
   act2->type_in2 = "NONE";
   act2->type_out = "NONE";
   act2->action = &_edit_app_action;
   act2->check_item = &_edit_app_check_item;
   evry_action_register(act2);

   act3 = E_NEW(Evry_Action, 1);
   act3->name = "New Application Entry";
   act3->type_in1 = "APPLICATION";
   act3->type_in2 = "NONE";
   act3->type_out = "NONE";
   act3->action = &_new_app_action;
   act3->check_item = &_new_app_check_item;
   evry_action_register(act3);

   inst = NULL;

   return 1;
}

EAPI int
evry_plug_apps_shutdown(void)
{
   evry_plugin_unregister(p1);
   evry_plugin_unregister(p2);
   evry_action_unregister(act);
   evry_action_unregister(act2);
   evry_action_unregister(act3);

   return 1;
}

static int
_begin(Evry_Plugin *p __UNUSED__, Evry_Item *it)
{
   const char *mime;

   if (inst) return 0;

   if (it)
     {
	if (!it->uri) return 0;

	if (!it->mime)
	  mime = efreet_mime_type_get(it->uri);
	else
	  mime = it->mime;

	if (!mime) return 0;

	inst = E_NEW(Inst, 1);
	inst->candidate = it;

	inst->apps = efreet_util_desktop_mime_list(mime);

	if (!inst->apps)
	  {
	     Efreet_Desktop *desktop;
	     desktop = e_exehist_mime_desktop_get(mime);
	     if (desktop)
	       inst->apps = eina_list_append(inst->apps, desktop);
	  }
     }
   else
     {
	inst = E_NEW(Inst, 1);
     }

   return 1;
}

static int
_action(Evry_Plugin *p __UNUSED__, Evry_Item *it, const char *input)
{
   E_Zone *zone;
   Evry_App *app = NULL;
   Efreet_Desktop *desktop = NULL;
   Eina_List *files = NULL;

   if (it) app = it->data[0];

   if (app && app->desktop)
     {
	desktop = app->desktop;
     }
   else
     {
	if (app && app->file)
	  input = app->file;

	if (!input || strlen(input) < 1) return EVRY_ACTION_CONTINUE;
	
	desktop = efreet_desktop_empty_new("");
	if (strchr(input, '%'))
	  {
	     desktop->exec = strdup(input);
	  }
	else
	  {
	     int len = strlen(input) + 4;
	     desktop->exec = malloc(len);
	     if (desktop->exec)
	       snprintf(desktop->exec, len, "%s %%U", input);
	  }
     }

   if (desktop)
     {
	if (inst && inst->candidate)
	  files = eina_list_append(files, inst->candidate->uri);

	zone = e_util_zone_current_get(e_manager_current_get());

	e_exec(zone, desktop, NULL, files, "everything");

	if (inst && inst->candidate && inst->candidate->mime)
	  e_exehist_mime_desktop_add(inst->candidate->mime, desktop);
	
	if (!it)
	  efreet_desktop_free(desktop);

	eina_list_free(files);

	return EVRY_ACTION_FINISHED;
     }

   return EVRY_ACTION_CONTINUE;
}

static void
_list_free(Evry_Plugin *p)
{
   Evry_Item *it;
   Evry_App *app;

   EINA_LIST_FREE(p->items, it)
     {
	if (it->label) eina_stringshare_del(it->label);
	app = it->data[0];
	E_FREE(app);
	E_FREE(it);
     }
}

static void
_cleanup(Evry_Plugin *p)
{
   _list_free(p);

   if (inst)
     {
	eina_list_free(inst->apps);
	E_FREE(inst);
     }

   inst = NULL;
}

static int
_fetch(Evry_Plugin *p, const char *input)
{
   Eina_List *l;
   Efreet_Desktop *desktop;
   char *file;
   char match1[4096];
   char match2[4096];
   Evry_Item *it;
   Evry_App *app;
   
   _list_free(p);

   if (inst && inst->apps)
     {
	if (!input)
	  {
	     EINA_LIST_FOREACH(inst->apps, l, desktop)
	       _item_add(p, desktop, NULL, 1);
	  }
	else
	  {
	     snprintf(match1, sizeof(match1), "%s*", input);
	     snprintf(match2, sizeof(match2), "*%s*", input);

	     EINA_LIST_FOREACH(inst->apps, l, desktop)
	       {
		  if (e_util_glob_case_match(desktop->exec, match1))
		    _item_add(p, desktop, NULL, 1);
		  else if (e_util_glob_case_match(desktop->exec, match2))
		    _item_add(p, desktop, NULL, 2);
		  else if (e_util_glob_case_match(desktop->name, match1))
		    _item_add(p, desktop, NULL, 1);
		  else if (e_util_glob_case_match(desktop->name, match2))
		    _item_add(p, desktop, NULL, 2);
		  else if (desktop->comment)
		    {
		       if (e_util_glob_case_match(desktop->comment, match1))
			 _item_add(p, desktop, NULL, 3);
		       else if (e_util_glob_case_match(desktop->comment, match2))
			 _item_add(p, desktop, NULL, 4);
		    }
	       }
	  }
     }


   if (!p->items && input)
     {
	snprintf(match1, sizeof(match1), "%s*", input);
	l = efreet_util_desktop_exec_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 1);

	l = efreet_util_desktop_name_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 3);

	snprintf(match1, sizeof(match1), "*%s*", input);
	l = efreet_util_desktop_exec_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 2);

	l = efreet_util_desktop_name_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 4);

	// TODO make these optional/configurable
	l = efreet_util_desktop_generic_name_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 5);

	l = efreet_util_desktop_comment_glob_list(match1);
	EINA_LIST_FREE(l, desktop)
	  _item_add(p, desktop, NULL, 5);
     }
   else if (!p->items)
     {
   	// TODO option for popular/recent
   	l = e_exehist_list_get();
   	EINA_LIST_FREE(l, file)
   	  _item_add(p, NULL, file, 1);
     }

   if (inst->added)
     {
	eina_hash_free(inst->added);
	inst->added = NULL;
     }

   it = E_NEW(Evry_Item, 1);
   app = E_NEW(Evry_App, 1);
   app->file = eina_stringshare_add(input);
   app->desktop = NULL;
   it->data[0] = app;
   it->priority = 100;
   it->label = eina_stringshare_add("Run Command");
   p->items = eina_list_append(p->items, it);

     if (p->items)
     {
	p->items = eina_list_sort(p->items, eina_list_count(p->items), _cb_sort);
	return 1;
     }

   return 0;
}

static void
_item_add(Evry_Plugin *p, Efreet_Desktop *desktop, char *file, int prio)
{
   Evry_Item *it;
   Evry_App *app;
   Efreet_Desktop *desktop2;

   if (desktop)
     {
	Eina_List *l;
	char *cat;

	/* ignore screensaver.. */
	EINA_LIST_FOREACH(desktop->categories, l, cat)
	  if (cat && !strcmp(cat, "Screensaver"))
	    return;

	file = desktop->exec;
     }

   if (!file) return;

   if (!inst->added)
     inst->added = eina_hash_string_superfast_new(NULL);


   if (!desktop)
   {
      char match[4096];
      Eina_List *l;
      int len;
      char *tmp;
      int found = 0;

      if (eina_hash_find(inst->added, file))
	return;

      len = strlen(file);
      tmp = ecore_file_app_exe_get(file);
      snprintf(match, sizeof(match), "%s*", tmp);
      l = efreet_util_desktop_exec_glob_list(match);

      EINA_LIST_FREE(l, desktop)
      	{
      	   if (desktop->exec && !strncmp(file, desktop->exec, len))
      	     {
      		found = 1;
      		break;
      	     }
      	}
      
      eina_list_free(l);
      free(tmp);

      /* desktop = efreet_desktop_get(file); */
      /* if (!desktop || !desktop->exec) */
      if (!found)
	eina_hash_add(inst->added, file, file);
   }

   if (desktop)
     {
	if ((desktop2 = eina_hash_find(inst->added, file)))
	  if (desktop == desktop2)
	    return;

	eina_hash_add(inst->added, file, desktop);
	file = NULL;
     }

   it = E_NEW(Evry_Item, 1);
   app = E_NEW(Evry_App, 1);
   app->desktop = desktop;
   app->file = file;
   it->data[0] = app;
   it->priority = prio;
   if (desktop)
     it->label = eina_stringshare_add(desktop->name);
   else
     it->label = eina_stringshare_add(file);
   it->o_icon = NULL;

   p->items = eina_list_append(p->items, it);
}

static void
_item_icon_get(Evry_Plugin *p __UNUSED__, Evry_Item *it, Evas *e)
{
   Evry_App *app = it->data[0];

   if (app->desktop)
     it->o_icon = e_util_desktop_icon_add(app->desktop, 24, e);

   if (!it->o_icon)
     {
	it->o_icon = edje_object_add(e);
	/* e_util_icon_theme_set(it->o_icon, "system-run") */
	e_theme_edje_object_set(it->o_icon, "base/theme/fileman", "e/icons/system-run");
     }
}

static int
_cb_sort(const void *data1, const void *data2)
{
   const Evry_Item *it1, *it2;
   Evry_App *app1, *app2;
   const char *e1, *e2;
   double t1, t2;

   it1 = data1;
   it2 = data2;
   app1 = it1->data[0];
   app2 = it2->data[0];

   if (app1->desktop)
     e1 = app1->desktop->exec;
     /* //e1 = efreet_util_path_to_file_id(app1->desktop->orig_path);
      * e1 = app1->desktop->orig_path; */
   else
     e1 = app1->file;

   if (app2->desktop)
     e2 = app2->desktop->exec;
     /* //e2 = efreet_util_path_to_file_id(app2->desktop->orig_path);
      * e2 = app2->desktop->orig_path; */
   else
     e2 = app2->file;

   t1 = e_exehist_newest_run_get(e1);
   t2 = e_exehist_newest_run_get(e2);

   if ((int)(t2 - t1))
     return (int)(t2 - t1);
   else if (it1->priority - it2->priority)
     return (it1->priority - it2->priority);
   // TODO compare exe strings?
   else return 0;
}

static int
_exec_app_check_item(Evry_Action *act __UNUSED__, Evry_Item *it)
{
   Evry_App *app = it->data[0];
   if (app->desktop)
     return 1;

   if (app->file && strlen(app->file) > 0)
     return 1;
     
   return 0;
}


static int
_exec_app_action(Evry_Action *act)
{
   if (act->thing1 && act->thing2)
     {
	E_Zone *zone;
	Evry_App *app = NULL;
	Efreet_Desktop *desktop = NULL;
	Eina_List *files = NULL;

	app = act->thing1->data[0];

	if (app->desktop)
	  desktop = app->desktop;
	else
	  {
	     desktop = efreet_desktop_empty_new("");
	     if (strchr(app->file, '%'))
	       desktop->exec = strdup(app->file);
	     else
	       {
		  int len = strlen(app->file) + 4;
		  desktop->exec = malloc(len);
		  if (desktop->exec)
		    snprintf(desktop->exec, len, "%s %%U", app->file);
	       }
	  }

	if (desktop)
	  {
	     files = eina_list_append(files, act->thing2->uri);

	     zone = e_util_zone_current_get(e_manager_current_get());

	     e_exec(zone, desktop, NULL, files, "everything");

	     if (act->thing2->mime)
	       e_exehist_mime_desktop_add(act->thing2->mime, desktop);
	
	     if (!app->desktop)
	       efreet_desktop_free(desktop);

	     eina_list_free(files);

	     return EVRY_ACTION_FINISHED;
	  }

	return EVRY_ACTION_CONTINUE;
     }

   return 0;
}

static int
_edit_app_check_item(Evry_Action *act __UNUSED__, Evry_Item *it)
{
   Evry_App *app = it->data[0];
   if (app->desktop)
     return 1;

   return 0;
}


static int
_edit_app_action(Evry_Action *act)
{
   if (act->thing1)
     {
	Evry_Item *it = act->thing1;
	Evry_App *app = it->data[0];
	
	Efreet_Desktop *desktop;
	
	if (app->desktop)
	  desktop = app->desktop;
	else
	  {
	     char buf[128];
	     snprintf(buf, 128, "%s/.local/share/applications/%s.desktop", e_user_homedir_get(), app->file);
	     
	     desktop = efreet_desktop_empty_new(eina_stringshare_add(buf));
	     /* XXX check if this gets freed by efreet*/
	     desktop->exec = strdup(app->file); 
	  }

	e_desktop_edit(e_container_current_get(e_manager_current_get()), desktop);
	
	return 1;
     }
   
   return 0;
}


static int
_new_app_check_item(Evry_Action *act __UNUSED__, Evry_Item *it)
{
   Evry_App *app = it->data[0];
   if (app->desktop)
     return 1;

   if (app->file && strlen(app->file) > 0)
     return 1;
     
   return 0;
}


static int
_new_app_action(Evry_Action *act)
{
   if (act->thing1)
     {
	Evry_Item *it = act->thing1;
	Evry_App *app = it->data[0];
	char *name;
	char buf[4096];
	char *end;
	Efreet_Desktop *desktop;
	int i;
	
	if (app->desktop)
	  name = strdup(app->desktop->name);
	else
	  /* TODO replace '/' and remove other special characters */
	  name = strdup(app->file);

	if ((end = strchr(name, ' ')))
	  name[end - name] = '\0';
	
	for (i = 0; i < 10; i++)
	  {
	     snprintf(buf, 4096, "%s/.local/share/applications/%s-%d.desktop", e_user_homedir_get(), name, i);
	     if (ecore_file_exists(buf))
	       {
		  buf[0] = '\0';
		  continue;
	       }
	     else break;
	  }

	free(name);
	   
	if (strlen(buf) == 0)
	  return 0;
	
	if (!app->desktop)
	  {
	     desktop = efreet_desktop_empty_new(buf);
	     desktop->exec = strdup(app->file);
	  }
	else 
	  {
	     efreet_desktop_save_as(app->desktop, buf);
	     /*XXX hackish - desktop is removed on save_as..*/
	     efreet_desktop_new(app->desktop->orig_path);
	     
	     desktop = efreet_desktop_new(buf);
	  }
	
	e_desktop_edit(e_container_current_get(e_manager_current_get()), desktop);

	return 1;
     }
   
   return 0;
}


