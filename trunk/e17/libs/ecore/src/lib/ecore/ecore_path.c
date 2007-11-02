/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_private.h"
#include "Ecore_Data.h"
#include "Ecore_Str.h"

static Ecore_List *group_list = NULL;

Ecore_Path_Group *__ecore_path_group_find(const char *name);
Ecore_Path_Group *__ecore_path_group_find_id(int id);

/**
 * @defgroup Ecore_Path_Group Path Group Functions
 *
 * Functions that make it easier to find a file in a set of search paths.
 *
 * @todo Give this a better description.
 */

/**
 * Creates a new path group.
 * @param   group_name The name of the new group.
 * @return  @c 0 on error, the integer id of the new group on success.
 * @ingroup Ecore_Path_Group
 */
EAPI int
ecore_path_group_new(const char *group_name)
{
   int lastid;
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("group_name", group_name, -1);

   if (!group_list)
     {
	group_list = ecore_list_new();
	lastid = 0;
     }
   else
     {
	Ecore_Path_Group *last;

	group = __ecore_path_group_find(group_name);
	if (group)
	  return -1;

	last = ecore_list_last_goto(group_list);
	lastid = last->id;
     }

   group = (Ecore_Path_Group *)malloc(sizeof(Ecore_Path_Group));
   memset(group, 0, sizeof(Ecore_Path_Group));

   group->name = strdup(group_name);
   group->id = lastid + 1;

   ecore_list_append(group_list, group);

   return group->id;
}

/**
 * Destroys a previously created path group
 * @param   group_id The unique identifier for the group.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_del(int group_id)
{
   Ecore_Path_Group *group;

   group = __ecore_path_group_find_id(group_id);

   if (!group)
     return;

   if (group->paths)
     {
	ecore_list_for_each(group->paths, ECORE_FOR_EACH(free), NULL);
	ecore_list_destroy(group->paths);
     }

   if (ecore_list_goto(group_list, group))
     ecore_list_remove(group_list);

   if (ecore_list_empty_is(group_list))
     {
	ecore_list_destroy(group_list);
	group_list = NULL;
     }

   free(group->name);
   free(group);
}

/**
 * Adds a directory to be searched for files.
 * @param   group_id The unique identifier for the group to add the path.
 * @param   path     The new path to be added to the group.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_add(int group_id, const char *path)
{
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER("path", path);

   group = __ecore_path_group_find_id(group_id);

   if (!group)
     return;

   if (!group->paths)
     group->paths = ecore_list_new();

   ecore_list_append(group->paths, strdup(path));
}

/**
 * Removes the given directory from the given group.
 * @param   group_id The identifier for the given group.
 * @param   path     The path of the directory to be removed.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_remove(int group_id, const char *path)
{
   char *found;
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER("path", path);

   group = __ecore_path_group_find_id(group_id);

   if (!group || !group->paths)
     return;

   /*
    * Find the path in the list of available paths
    */
   ecore_list_first_goto(group->paths);

   while ((found = ecore_list_current(group->paths)) && strcmp(found, path))
     ecore_list_next(group->paths);

   /*
    * If the path is found, remove and free it
    */
   if (found)
     {
	ecore_list_remove(group->paths);
	free(found);
     }
}

/**
 * Finds a file in a group of paths.
 * @param   group_id The path group id to search.
 * @param   name     The name of the file to find.
 * @return  A pointer to a newly allocated path location of the found file
 *          on success.  @c NULL on failure.
 * @ingroup Ecore_Path_Group
 */
EAPI char *
ecore_path_group_find(int group_id, const char *name)
{
   int r;
   char *p;
   struct stat st;
   char path[PATH_MAX];
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("name", name, NULL);

   group = __ecore_path_group_find_id(group_id);
   if (!group)
     return NULL;

   /*
    * Search the paths of the path group for the specified file name
    */
   ecore_list_first_goto(group->paths);
   p = ecore_list_next(group->paths);
   do
     {
	snprintf(path, PATH_MAX, "%s/%s", p, name);
	r = stat(path, &st);
     }
   while (((r < 0) || !S_ISREG(st.st_mode)) &&
	  (p = ecore_list_next(group->paths)));

   if (p)
     p = strdup(path);

   return p;
}

/**
 * Retrieves a list of all available files in the given path.
 * @param   group_id The identifier for the given path.
 * @return  A pointer to a newly allocated list of all files found in the paths
 *          identified by @p group_id.  @c NULL otherwise.
 * @ingroup Ecore_Path_Group
 */
EAPI Ecore_List *
ecore_path_group_available(int group_id)
{
   Ecore_List *avail = NULL;
   Ecore_Path_Group *group;
   char *path;

   group = __ecore_path_group_find_id(group_id);

   if (!group || !group->paths || ecore_list_empty_is(group->paths))
     return NULL;

   ecore_list_first_goto(group->paths);

   while ((path = ecore_list_next(group->paths)) != NULL)
     {
	DIR *dir;
	struct stat st;
	struct dirent *d;

	stat(path, &st);

	if (!S_ISDIR(st.st_mode))
	  continue;

	dir = opendir(path);

	if (!dir)
	  continue;

	while ((d = readdir(dir)) != NULL)
	  {
	     char ppath[PATH_MAX];
	     char *ext;
/*	     char n[PATH_MAX];
	     int l;
*/
	     if (!strncmp(d->d_name, ".", 1))
	       continue;

	     ext = strrchr(d->d_name, '.');

#ifndef _WIN32
	     if (!ext || strncmp(ext, ".so", 3))
#else
	     if (!ext || strncmp(ext, ".dll", 4))
#endif /* _WIN32 */
	       continue;

	     snprintf(ppath, PATH_MAX, "%s/%s", path, d->d_name);

	     stat(ppath, &st);

	     if (!S_ISREG(st.st_mode))
	       continue;
/*
	     l = strlen(d->d_name);

	     strncpy(n, d->d_name, l - 2);
*/
	     if (!avail)
	       avail = ecore_list_new();

/*	     ecore_list_append(avail, strdup(n));*/
	     ecore_list_append(avail, strdup(d->d_name));
	  }
     }

   return avail;
}

/**
 * Retrieves a list of all available plugins in the given path.
 * @param   group_id The identifier for the given path.
 * @return  A pointer to a newly allocated list of all plugins found in the
 *          paths identified by @p group_id.  @c NULL otherwise.
 * @ingroup Ecore_Plugin
 */
EAPI Ecore_List *
ecore_plugin_available_get(int group_id)
{
   Ecore_List *avail = NULL;
   Ecore_Hash *plugins = NULL;
   Ecore_Path_Group *group;
   char *path;

   group = __ecore_path_group_find_id(group_id);

   if (!group || !group->paths || ecore_list_empty_is(group->paths))
     return NULL;

   ecore_list_first_goto(group->paths);
   plugins = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   ecore_hash_free_key_cb_set(plugins, free);

   while ((path = ecore_list_next(group->paths)) != NULL)
     {
	DIR *dir;
	struct stat st;
	struct dirent *d;

	stat(path, &st);

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

#ifndef _WIN32
	     if (!ecore_str_has_suffix(d->d_name, ".so"))
#else
	     if (!ecore_str_has_suffix(d->d_name, ".dll"))
#endif /* _WIN32 */
	       continue;

	     snprintf(ppath, PATH_MAX, "%s/%s", path, d->d_name);

	     stat(ppath, &st);

	     if (!S_ISREG(st.st_mode))
	       continue;

	     ecore_strlcpy(ppath, d->d_name, sizeof(ppath));
	     ext = strrchr(ppath, '.');
	     *ext = '\0';

	     if (!ecore_hash_get(plugins, ppath))
	       {
		  char *key;

		  key = strdup(ppath);
		  ecore_hash_set(plugins, key, key);
	       }
	  }
	closedir(dir);
     }

   ecore_hash_free_key_cb_set(plugins, NULL);
   avail = ecore_hash_keys(plugins);
   ecore_list_free_cb_set(avail, free);
   ecore_hash_destroy(plugins);

   return avail;
}
/*
 * Find the specified group name
 */
Ecore_Path_Group *
__ecore_path_group_find(const char *name)
{
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("name", name, NULL);

   ecore_list_first_goto(group_list);

   while ((group = ecore_list_next(group_list)) != NULL)
     if (!strcmp(group->name, name))
       return group;

   return NULL;
}

/*
 * Find the specified group id
 */
Ecore_Path_Group *
__ecore_path_group_find_id(int id)
{
   Ecore_Path_Group *group;

   ecore_list_first_goto(group_list);

   while ((group = ecore_list_next(group_list)) != NULL)
     if (group->id == id)
       return group;

   return NULL;
}
