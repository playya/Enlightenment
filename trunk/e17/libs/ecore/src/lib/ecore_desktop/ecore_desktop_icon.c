#include <limits.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"

//#define DEBUG 1

static const char   *_ecore_desktop_icon_find0(const char *icon, 
					      const char *icon_size,
					      const char *icon_theme);

static int _ecore_desktop_icon_theme_list_add(void *data, const char *path);
void _ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme * icon_theme);

/* FIXME: We need a way for the client to disable searching for any of these that they don't support. */
static const char  *ext[] = { ".edj", ".png", ".svgz", ".svg", ".xpm", "", NULL };
static int          init_count = 0;
static Ecore_Hash  *icon_theme_cache;
static int          loaded = 0;


/**
 * @defgroup Ecore_Desktop_Icon_Group icon theme Functions
 *
 * Functions that deal with freedesktop.org icon themes.
 *
 * This conforms with the freedesktop.org XDG Icon Theme Specification version 0.11
 */

/**
 * Find the path to an icon.
 *
 * Using the search algorithm specified by freedesktop.org,
 * search for an icon in the currently installed set of icon themes.
 *
 * The returned string needs to be freed eventually.
 *
 * @param   icon The name of the required icon.
 * @param   icon_size The size of the required icon.
 * @param   icon_theme The theme of the required icon.
 * @return  The full path to an icon file, or NULL.
 * @ingroup Ecore_Desktop_Icon_Group
 */

const char               *
ecore_desktop_icon_find(const char *icon, const char *icon_size, const char *icon_theme)
{
   const char         *dir = NULL, *icn;
   Ecore_List         *icons;

   if (icon == NULL)
      return NULL;

   /* Easy check first, was a full path supplied? */
   if ((icon[0] == '/') && (ecore_file_exists(icon)))
      return strdup(icon);

   if (icon_size == NULL)
      icon_size="48x48";
   if (icon_theme == NULL)
      icon_theme="hicolor";

   icons = ecore_desktop_paths_to_list(icon);
   if (!icons) return NULL;
   ecore_list_goto_first(icons);
   while ((icn = (char *) ecore_list_next(icons)))
      {
#ifdef DEBUG
         fprintf(stderr, "\tTrying To Find Icon %s\n", icn);
#endif
         /* Check for unsupported extension */
         if ((strlen(icn) > 4) && 
	     (!strcmp(icn + strlen(icn) - 4, ".ico")))
	   continue;

         dir = _ecore_desktop_icon_find0(icn, icon_size, icon_theme);
         if (dir)
	   {
//	      dir = strdup(dir);
	      break;
	   }
      }
   ecore_list_destroy(icons);

   return dir;
}

/** Search for an icon the fdo way.
 *
 * This complies with the freedesktop.org Icon Theme Specification version 0.7
 *
 * @param   icon The icon to search for.
 * @param   icon_size The icon size to search for.
 * @param   icon_theme The icon theme to search in.
 * @return  The full path to the found icon.
 */
static const char        *
_ecore_desktop_icon_find0(const char *icon, const char *icon_size, const char *icon_theme)
{
   /*  NOTES ON OPTIMIZATIONS
    *
    * The spec has this to say -
    *
    * "The algorithm as described in this document works by always looking up 
    * filenames in directories (a stat in unix terminology). A good 
    * implementation is expected to read the directories once, and do all 
    * lookups in memory using that information.
    *
    * "This caching can make it impossible for users to add icons without having 
    * to restart applications. In order to handle this, any implementation that 
    * does caching is required to look at the mtime of the toplevel icon 
    * directories when doing a cache lookup, unless it already did so less than 
    * 5 seconds ago. This means that any icon editor or theme installation 
    * program need only to change the mtime of the the toplevel directory where 
    * it changed the theme to make sure that the new icons will eventually get 
    * used."
    *
    * On the other hand, OS caching (at least in linux) seems to do a reasonable 
    * job here.
    *
    * We could also precalculate and cache all the information extracted from 
    * the .theme files.
    */

   char                icn[PATH_MAX], path[PATH_MAX];
   char               *theme_path;
   const char         *found;

   if ((icon == NULL) || (icon[0] == '\0'))
      return NULL;

#ifdef DEBUG
   fprintf(stderr, "\tTrying To Find Icon %s (%s) in theme %s\n", icon,
	   icon_size, icon_theme);
#endif

   /* Get the theme description file. */
   snprintf(icn, PATH_MAX, "%s/index.theme", icon_theme);
#ifdef DEBUG
   printf("SEARCHING FOR %s\n", icn);
#endif
   theme_path =
     ecore_desktop_paths_file_find(ecore_desktop_paths_icons, icn, 2,
				   NULL, NULL);
   if (theme_path)
     {
	Ecore_Hash         *theme;

	/* Parse the theme description file. */
#ifdef DEBUG
	printf("Path to %s is %s\n", icn, theme_path);
#endif
	theme = ecore_desktop_ini_get(theme_path);
	if (theme)
	  {
	     Ecore_Hash         *icon_group;

	     /* Grab the themes directory list, and what it inherits. */
	     icon_group = (Ecore_Hash *) ecore_hash_get(theme, "Icon Theme");
	     if (icon_group)
	       {
		  char               *directories, *inherits;

		  directories =
		     (char *)ecore_hash_get(icon_group, "Directories");
		  inherits = (char *)ecore_hash_get(icon_group, "Inherits");
		  if (directories)
		    {
		       Ecore_List         *directory_paths;

		       /* Split the directory list. */
#ifdef DEBUG
		       printf("Inherits %s Directories %s\n", inherits,
			      directories);
#endif
		       directory_paths =
			  ecore_desktop_paths_to_list(directories);
		       if (directory_paths)
			 {
			    int                 wanted_size;
			    int                 minimal_size = INT_MAX;
			    int                 i;
			    const char         *closest = NULL;
			    char               *directory;

			    wanted_size = atoi(icon_size);
			    /* Loop through the themes directories. */

			    ecore_list_goto_first(directory_paths);
			    while ((directory =
				    ecore_list_next(directory_paths)) != NULL)
			      {
				 Ecore_Hash         *sub_group;

#ifdef DEBUG
				 printf("FDO icon path = %s\n",
					directory_paths);
#endif
				 /* Get the details for this theme directory. */
				 sub_group =
				    (Ecore_Hash *) ecore_hash_get(theme,
								  directory);
				 if (sub_group)
				   {
				      char               *size, *type, *minsize,
					 *maxsize, *threshold;
				      int                 j;

				      size =
					 (char *)ecore_hash_get(sub_group,
								"Size");
				      type =
					 (char *)ecore_hash_get(sub_group,
								"Type");
				      minsize =
					 (char *)ecore_hash_get(sub_group,
								"MinSize");
				      maxsize =
					 (char *)ecore_hash_get(sub_group,
								"MaxSize");
				      threshold =
					 (char *)ecore_hash_get(sub_group,
								"Threshold");
				      if (size)
					{
					   int                 match = 0;
					   int                 this_size,
					      result_size =
					      0, min_size, max_size,
					      thresh_size;

					   if (!minsize)
					      minsize = size;
					   if (!maxsize)
					      maxsize = size;
					   if (!threshold)
					      threshold = "2";
					   min_size = atoi(minsize);
					   max_size = atoi(maxsize);
					   thresh_size = atoi(threshold);

					   /* Does this theme directory match the required icon size? */
					   this_size = atoi(size);
					   if (!type)
					      type = "Threshold";
					   switch (type[0])
					     {
					     case 'F':	/* Fixed. */
						{
						   match =
						      (wanted_size ==
						       this_size);
						   result_size =
						      abs(this_size -
							  wanted_size);
						   break;
						}
					     case 'S':	/* Scaled. */
						{
						   match =
						      ((min_size <= wanted_size)
						       && (wanted_size <=
							   max_size));
						   if (wanted_size < min_size)
						      result_size =
							 min_size - wanted_size;
						   if (wanted_size > max_size)
						      result_size =
							 wanted_size - max_size;
						   break;
						}
					     default:	/* Threshold. */
						{
						   match =
						      (((this_size -
							 thresh_size) <=
							wanted_size)
						       && (wanted_size <=
							   (this_size +
							    thresh_size)));
						   if (wanted_size <
						       (this_size -
							thresh_size))
						      result_size =
							 min_size - wanted_size;
						   if (wanted_size >
						       (this_size +
							thresh_size))
						      result_size =
							 wanted_size - max_size;
						   break;
						}
					     }

					   /* Look for icon with all extensions. */
					   for (j = 0; ext[j] != NULL; j++)
					     {
						snprintf(path, PATH_MAX,
							 "%s/%s/%s%s",
							 icon_theme, directory,
							 icon, ext[j]);
#ifdef DEBUG
						printf("FDO icon = %s\n", path);
#endif
						found =
						   ecore_desktop_paths_file_find
						   (ecore_desktop_paths_icons,
						    path, 0, NULL, NULL);
						if (found)
						  {
						     if (match)	/* If there is a match in sizes, return the icon. */
						       {
							  ecore_list_destroy(directory_paths);
							  free(theme_path);
							  return found;
						       }
						     if (result_size < minimal_size)	/* While we are here, figure out our next fallback strategy. */
						       {
							  minimal_size =
							     result_size;
							  if (closest) free(closest);
							  closest = found;
						       }
						     else
						       free(found);
						  }
					     }

					}
				   }
			      }	/* while ((directory = ecore_list_next(directory_paths)) != NULL) */

			    /* Fall back strategy #1, look for closest size in this theme. */
			    if (closest)
			      {
				 ecore_list_destroy(directory_paths);
				 free(theme_path);
				 return closest;
			      }

			    /* Fall back strategy #2, Try again with the parent theme. */
			    if ((inherits) && (inherits[0] != '\0')
				&& (strcmp(icon_theme, "hicolor") != 0))
			      {
				 found =
				    _ecore_desktop_icon_find0(icon, icon_size,
							      inherits);
				 if (found != NULL)
				   {
				      ecore_list_destroy(directory_paths);
				      free(theme_path);
				      return found;
				   }
			      }

			    /* Fall back strategy #3, Try the default hicolor theme. */
			    if ((!((inherits) && (inherits[0] != '\0')))
				&& (strcmp(icon_theme, "hicolor") != 0))
			      {
				 found =
				    _ecore_desktop_icon_find0(icon, icon_size,
							      "hicolor");
				 if (found != NULL)
				   {
				      ecore_list_destroy(directory_paths);
				      free(theme_path);
				      return found;
				   }
			      }

			    /* Fall back strategy #4, Just search in the base of the icon directories. */
			    for (i = 0; ext[i] != NULL; i++)
			      {
				 snprintf(path, PATH_MAX, "%s%s", icon, ext[i]);
#ifdef DEBUG
				 printf("FDO icon = %s\n", path);
#endif
				 found =
				    ecore_desktop_paths_file_find
				    (ecore_desktop_paths_icons, path, 0, NULL,
				     NULL);
				 if (found)
				   {
				      ecore_list_destroy(directory_paths);
				      free(theme_path);
				      return found;
				   }
			      }
			    ecore_list_destroy(directory_paths);
			 }
		    }
	       }
	  }
	free(theme_path);
     }

   return NULL;
}


Ecore_Hash *
ecore_desktop_icon_theme_list(void)
{
   if (!loaded)
      ecore_desktop_paths_file_find(ecore_desktop_paths_icons, "index.theme", 2, _ecore_desktop_icon_theme_list_add, NULL);
   loaded = 1;
   return icon_theme_cache;
}


static int 
_ecore_desktop_icon_theme_list_add(void *data, const char *path)
{
   char                icn[PATH_MAX];

   snprintf(icn, PATH_MAX, "%sindex.theme", path);
   if (ecore_desktop_icon_theme_get(icn, NULL))
      return 1;  /* Should stop it from recursing this directory, but let it continue searching the next. */
   return 0;
}


/**
 * Setup what ever needs to be setup to support ecore_desktop_icon.
 *
 * There are internal structures that are needed for ecore_desktop_icon
 * functions to operate, this sets them up.
 *
 * @ingroup Ecore_Desktop_Icon_Group
 */
EAPI int
ecore_desktop_icon_init()
{
   if (++init_count != 1) return init_count;

   if (!icon_theme_cache)
     {
	icon_theme_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	if (icon_theme_cache)
	  {
	     ecore_hash_set_free_key(icon_theme_cache, free);
	     ecore_hash_set_free_value(icon_theme_cache,
				       (Ecore_Free_Cb) _ecore_desktop_icon_theme_destroy);
	  }
     }

   return init_count;   
}

/**
 * Tear down what ever needs to be torn down to support ecore_desktop_ycon.
 *
 * There are internal structures that are needed for ecore_desktop_icon
 * functions to operate, this tears them down.
 *
 * @ingroup Ecore_Desktop_Icon_Group
 */
EAPI int
ecore_desktop_icon_shutdown()
{
   if (--init_count != 0) return init_count;

   if (icon_theme_cache)
     {
	ecore_hash_destroy(icon_theme_cache);
	icon_theme_cache = NULL;
     }

   return init_count;   
}


/**
 * Get the contents of an index.theme file.
 *
 * Everything that is in the index.theme file is returned in the
 * data member of the Ecore_Desktop_Icon_Theme structure, it's an Ecore_Hash 
 * as returned by ecore_desktop_ini_get().  Some of the data in the
 * index.theme file is decoded into specific members of the returned 
 * structure.
 *
 * Use ecore_desktop_icon_theme_destroy() to free this structure.
 * 
 * @param   icon_theme Name of the icon theme, or full path to the index.theme file.
 * @param   lang Language to use, or NULL for default.
 * @return  An Ecore_Desktop_Icon_Theme containing the files contents.
 * @ingroup Ecore_Desktop_Icon_Group
 */
Ecore_Desktop_Icon_Theme      *
ecore_desktop_icon_theme_get(const char *icon_theme, const char *lang)
{
   Ecore_Desktop_Icon_Theme      *result;

   result = (Ecore_Desktop_Icon_Theme *) ecore_hash_get(icon_theme_cache, (char *) icon_theme);
   if (!result)
     {
        char  icn[PATH_MAX], *theme_path;

        if (icon_theme[0] == '/')
	   {
              char *dir;

	      theme_path = strdup(icon_theme);
              dir = ecore_file_get_dir(theme_path);
	      if (dir)
	         icon_theme = (char *) ecore_file_get_file(dir);
#ifdef DEBUG
              printf("LOADING THEME %s  -   %s\n", icon_theme, theme_path);
#endif
	   }
	else
	   {
              snprintf(icn, PATH_MAX, "%s/index.theme", icon_theme);
#ifdef DEBUG
              printf("SEARCHING FOR %s\n", icn);
#endif
              theme_path = ecore_desktop_paths_file_find(ecore_desktop_paths_icons, icn, 2, NULL, NULL);
	   }
        if (theme_path)
	   {
	      result = calloc(1, sizeof(Ecore_Desktop_Icon_Theme));
	      if (result)
	        {
	           result->data = ecore_desktop_ini_get(theme_path);
	           if (result->data)
	             {
		        result->group =
		           (Ecore_Hash *) ecore_hash_get(result->data,
						   "Icon Theme");
		        if (result->group)
		          {
		             char               *value;

                             /* According to the spec, name and comment are required, but we can fake those easily enough. */
		             value = (char *)ecore_hash_get(result->group, "Name");
			     if (!value)
			        value = (char *) icon_theme;
			     result->name = strdup(value);
		             value = (char *)ecore_hash_get(result->group, "Comment");
			     if (!value)
			        value = "No comment provided.";
			     result->comment = strdup(value);
		             result->directories = (char *)ecore_hash_get(result->group, "Directories");
                             /* FIXME: Directories is also required, don't feel like faking it for now. */
                             if (result->directories)
			        {
		                   result->inherits = (char *)ecore_hash_get(result->group, "Inherits");
		                   value = (char *)ecore_hash_get(result->group, "Example");
			           if (!value)
			              value = "exec";
			           result->example = strdup(value);

                                   /* This passes the basic validation tests, mark it as real and cache it. */
		                   result->path = theme_path;
		                   ecore_hash_set(icon_theme_cache, strdup(icon_theme), result);
				}
		          }
	             }

	           if (!result->path)
	             {
		        _ecore_desktop_icon_theme_destroy(result);
		        free(theme_path);
		        result = NULL;
	             }
	        }
	   }
     }

   return result;
}


/**
 * Free whatever resources are used by an Ecore_Desktop_Icon_Theme.
 *
 * There are internal resources used by each Ecore_Desktop_Icon_Theme
 * This releases those resources.
 *
 * @param  icon_theme  An Ecore_Desktop_Icon_Theme.
 * @ingroup Ecore_Desktop_Icon_Group
 */
void
ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme * icon_theme)
{
  /* This is just a dummy, because these structures are cached. */
  /* Later versions of the cache may reference count, then this will be useful. */
}

void
_ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme * icon_theme)
{
   if (icon_theme->path) free(icon_theme->path);
   if (icon_theme->name) free(icon_theme->name);
   if (icon_theme->comment) free(icon_theme->comment);
   if (icon_theme->example) free(icon_theme->example);
   free(icon_theme);
}
