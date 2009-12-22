/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef HAVE_EVIL_H
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Data.h"
#include "Ecore_Str.h"

static Eina_Bool _hash_keys(const Eina_Hash	*hash,
			    const char		*key,
                            void		*list);

/**
 * @defgroup Ecore_Plugin Plugin Functions
 *
 * Functions that load modules of compiled code into memory.
 */

/**
 * Loads the specified plugin from the specified path group.
 * @param   group       The path group to search for the plugin to load
 * @param   plugin_name The name of the plugin to load.
 * @param   version     The interface version of the plugin. With version
 *                      equal to NULL the default will be loaded.
 * @return  A pointer to the newly loaded plugin on success, @c NULL on
 *          failure.
 * @ingroup Ecore_Plugin
 */
EAPI Ecore_Plugin *
ecore_plugin_load(Ecore_Path_Group *group, const char *plugin_name, const char *version)
{
   char *path;
   char temp[PATH_MAX];

   Ecore_Plugin *plugin;
   void *handle = NULL;

   CHECK_PARAM_POINTER_RETURN("plugin_name", plugin_name, NULL);

   if (!version || *version == '\0')
     snprintf(temp, sizeof(temp), "%s" SHARED_LIB_SUFFIX, plugin_name);
   else
#ifndef _WIN32
     snprintf(temp, sizeof(temp), "%s" SHARED_LIB_SUFFIX ".%s", plugin_name, version);
#else
     snprintf(temp, sizeof(temp), "%s-%s" SHARED_LIB_SUFFIX, plugin_name, version);
#endif

   path = ecore_path_group_find(group, temp);

   if (!path && version)
     {
	/* if this file doesn't exist try a different order */
	snprintf(temp, sizeof(temp), "%s.%s" SHARED_LIB_SUFFIX, plugin_name, version);
	path = ecore_path_group_find(group, temp);
     }

   if (!path)
     return NULL;

   handle = dlopen(path, RTLD_LAZY);
   if (!handle)
     {
	FREE(path);
	return NULL;
     }

   /*
    * Allocate the new plugin and initialize it's fields
    */
   plugin = malloc(sizeof(Ecore_Plugin));
   if (!plugin)
     {
       dlclose(handle);
       FREE(path);
       return NULL;
     }
   memset(plugin, 0, sizeof(Ecore_Plugin));

   plugin->handle = handle;

   FREE(path);

   return plugin;
}

/**
 * Unloads the given plugin from memory.
 * @param   plugin The given plugin.
 * @ingroup Ecore_Plugin
 */
EAPI void
ecore_plugin_unload(Ecore_Plugin *plugin)
{
   CHECK_PARAM_POINTER("plugin", plugin);

   if (plugin->handle)
	dlclose(plugin->handle);

   FREE(plugin);
}

/*
 * Searches for the specified symbol in the given plugin.
 * @param   plugin      The given plugin.
 * @param   symbol_name The symbol to search for.
 * @return  Address of the given symbol if successful.  Otherwise, @c NULL.
 * @ingroup Ecore_Plugin
 */
EAPI void *
ecore_plugin_symbol_get(Ecore_Plugin *plugin, const char *symbol_name)
{
   void *ret;

   CHECK_PARAM_POINTER_RETURN("plugin", plugin, NULL);
   CHECK_PARAM_POINTER_RETURN("symbol_name", symbol_name, NULL);

   if (!plugin->handle)
     return NULL;

   ret = dlsym(plugin->handle, symbol_name);

   return ret;
}

/**
 * Retrieves a list of all available plugins in the given path.
 * @param   group_id The identifier for the given path.
 * @return  A pointer to a newly allocated list of all plugins found in the
 *          paths identified by @p group_id.  @c NULL otherwise.
 * @ingroup Ecore_Plugin
 */
EAPI Eina_List *
ecore_plugin_available_get(Ecore_Path_Group *group)
{
   Eina_List *avail = NULL;
   Eina_List *l;
   Eina_Hash *plugins = NULL;
   Eina_Iterator *it = NULL;
   char *path;

   CHECK_PARAM_POINTER_RETURN("group", group, NULL);

   if (!group->paths || !eina_list_count(group->paths))
     return NULL;

   plugins = eina_hash_string_superfast_new(NULL);

   EINA_LIST_FOREACH(group->paths, l, path)
     {
	DIR *dir;
	struct stat st;
	struct dirent *d;

	if (stat(path, &st) < 0)
	  continue;

	if (!S_ISDIR(st.st_mode))
	  continue;

	dir = opendir(path);

	if (!dir)
	  continue;

	while ((d = readdir(dir)) != NULL)
	  {
	     char ppath[PATH_MAX];
	     char *ext;

	     if (*d->d_name == '.')
	       continue;

	     if (!ecore_str_has_suffix(d->d_name, SHARED_LIB_SUFFIX))
	       continue;

	     snprintf(ppath, PATH_MAX, "%s/%s", path, d->d_name);

	     stat(ppath, &st);

	     if (!S_ISREG(st.st_mode))
	       continue;

	     ecore_strlcpy(ppath, d->d_name, sizeof(ppath));
	     ext = strrchr(ppath, '.');
	     *ext = '\0';

	     if (!eina_hash_find(plugins, ppath))
	       {
		  char *key;

		  key = strdup(ppath);
		  eina_hash_add(plugins, key, key);
	       }
	  }
	closedir(dir);
     }

   it = eina_hash_iterator_data_new(plugins);
   if (it)
     {
	eina_iterator_foreach(it, EINA_EACH(_hash_keys), &avail);
	eina_iterator_free(it);
     }

   eina_hash_free(plugins);


   return avail;
}

static Eina_Bool
_hash_keys(const Eina_Hash *hash __UNUSED__, const char *key, void *list)
{
   *(Eina_List **)list = eina_list_append(*(Eina_List **)list, key);
   return EINA_TRUE;
}
