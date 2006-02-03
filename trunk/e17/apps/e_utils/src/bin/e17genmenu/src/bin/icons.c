#include <stdlib.h>

#include "global.h"
#include "config.h"
#include "fdo_paths.h"
#include "icons.h"
#include "parse.h"

char *
set_icon(char *token)
{
#ifdef DEBUG
   fprintf(stderr, "Setting Icon: %s\n", token);
#endif
   if (strstr(token, "Core") != NULL)
      return COREICON;
   if (strstr(token, "Development") != NULL)
      return PROGRAMMINGICON;
   if (strstr(token, "Editors") != NULL)
      return EDITORICON;
   if (strstr(token, "Edutainment") != NULL)
      return EDUTAINMENTICON;
   if (strstr(token, "Game") != NULL)
      return GAMESICON;
   if (strstr(token, "Graphics") != NULL)
      return GRAPHICSICON;
   if (strstr(token, "Internet") != NULL)
      return INTERNETICON;
   if (strstr(token, "Office") != NULL)
      return OFFICEICON;
   if (strstr(token, "Programming") != NULL)
      return PROGRAMMINGICON;
   if (strstr(token, "Toys") != NULL)
      return TOYSICON;
   if (strstr(token, "Utilities") != NULL)
      return UTILITYICON;
   if ((strstr(token, "Accessories") != NULL) ||
       (strstr(token, "Applications") != NULL))
      return APPLICATIONICON;
   if ((strstr(token, "Multimedia") != NULL) ||
       (strstr(token, "Sound_Video") != NULL))
      return MULTIMEDIAICON;
   if ((strstr(token, "Preferences") != NULL) ||
       (strstr(token, "Settings") != NULL))
      return SETTINGSICON;
   if (strstr(token, "System") != NULL)
      return SYSTEMICON;
   return token;
}

char *
find_icon(char *icon)
{
   char icn[MAX_PATH], path[MAX_PATH];
   char *dir, *icon_size, *icon_theme, *home;

   if (icon == NULL)
      return DEFAULTICON;

   home = get_home();

   snprintf(icn, sizeof(icn), "%s", icon);
#ifdef DEBUG
   fprintf(stderr, "\tTrying To Find Icon %s\n", icn);
#endif

   /* Check For Unsupported Extension */
   if ((!strcmp(icon + strlen(icon) - 4, ".svg"))
       || (!strcmp(icon + strlen(icon) - 4, ".ico")))
      return DEFAULTICON;

   /* Check For An Extension, Append PNG If Missing */
   if (strrchr(icon, '.') == NULL)
      snprintf(icn, sizeof(icn), "%s.png", icon);

   /* Check If Dir Supplied In Desktop File */
   dir = ecore_file_get_dir(icn);
   if (!strcmp(dir, icn) == 0)
     {
        snprintf(path, MAX_PATH, "%s", icn);
        /* Check Supplied Dir For Icon */
        if (ecore_file_exists(path))
           return strdup(icn);
     }

   snprintf(path, MAX_PATH, PIXMAPDIR "/%s", icn);
   if (ecore_file_exists(path))
      return strdup(path);

   /* Get Icon Options */
   icon_size = get_icon_size();
   icon_theme = get_icon_theme();

   /* Check User Supplied Icon Theme */
   if (icon_theme != NULL)
     {
        fprintf(stderr, "\tUsing Icon Theme: %s\n", icon_theme);
        snprintf(path, MAX_PATH, "%s/%s/apps/%s", icon_theme, icon_size, icn);
        if (ecore_file_exists(path))
           return strdup(path);
        snprintf(path, MAX_PATH, "%s/%s/devices/%s", icon_theme, icon_size,
                 icn);
        if (ecore_file_exists(path))
           return strdup(path);
        snprintf(path, MAX_PATH, "%s/%s/filesystems/%s", icon_theme, icon_size,
                 icn);
        if (ecore_file_exists(path))
           return strdup(path);
     }

   snprintf(path, MAX_PATH, CRYSTALSVGDIR "/%s/apps/%s", icon_size, icn);
   if (ecore_file_exists(path))
      return strdup(path);
   snprintf(path, MAX_PATH, CRYSTALSVGDIR "/%s/devices/%s", icon_size, icn);
   if (ecore_file_exists(path))
      return strdup(path);
   snprintf(path, MAX_PATH, CRYSTALSVGDIR "/%s/filesystems/%s", icon_size, icn);
   if (ecore_file_exists(path))
      return strdup(path);

   /* We Did Not Find the icon in theme dir,
    * check default theme before setting a default icon */
   snprintf(path, MAX_PATH, ICONDIR "/hicolor/%s/apps/%s", icon_size, icn);
   if (ecore_file_exists(path))
      return strdup(path);
   snprintf(path, MAX_PATH, ICONDIR "/hicolor/%s/devices/%s", icon_size, icn);
   if (ecore_file_exists(path))
      return strdup(path);
   snprintf(path, MAX_PATH, ICONDIR "/hicolor/%s/filesystems/%s", icon_size,
            icn);
   if (ecore_file_exists(path))
      return strdup(path);

   return DEFAULTICON;
}


char *
find_fdo_icon(char *icon)
{
   char icn[MAX_PATH], path[MAX_PATH];
   char *dir, *icon_size, *icon_theme, *theme_path;

   if (icon == NULL)
      return DEFAULTICON;

#ifdef DEBUG
   fprintf(stderr, "\tTrying To Find Icon %s\n", icon);
#endif

   /* Check For Unsupported Extension */
   if ((!strcmp(icon + strlen(icon) - 4, ".svg"))
       || (!strcmp(icon + strlen(icon) - 4, ".ico"))
       || (!strcmp(icon + strlen(icon) - 4, ".xpm")))
      return DEFAULTICON;

   /* Check If Dir Supplied In Desktop File */
   dir = ecore_file_get_dir(icon);
   if (!strcmp(dir, icon) == 0)
     {
        snprintf(path, MAX_PATH, "%s", icon);
        /* Check Supplied Dir For Icon */
        if (ecore_file_exists(path))
           return strdup(icon);
     }

   /* Get Icon Options */
   icon_size = get_icon_size();
   icon_theme = get_icon_theme();

   snprintf(icn, MAX_PATH, "%s/index.theme", icon_theme);
   printf("SEARCHING FOR %s\n", icn);
   theme_path = fdo_paths_search_for_file(FDO_PATHS_TYPE_ICON, icn, NULL, NULL);
   if (theme_path)
      {
         Ecore_Hash *theme;

         printf("Path to %s is %s\n", icn, theme_path);
         theme = parse_ini_file(theme_path);
	 if (theme)
	    {
	       Ecore_Hash *icon_group;

               icon_group = (Ecore_Hash *) ecore_hash_get(theme, "Icon Theme");
	       if (icon_group)
	          {
	             char *directories, *inherits;

                     directories = (char *) ecore_hash_get(icon_group, "Directories");
                     inherits = (char *) ecore_hash_get(icon_group, "Inherits");
		     if ((directories) && (inherits))
		        {
                           Fdo_Path_List *directory_paths;

                           printf("Inherits %s Directories %s\n", inherits, directories);
                           directory_paths = fdo_paths_paths_to_list(directories);
			   if (directory_paths)
			      {
			         int wanted_size;
			         char i;

                                 wanted_size = atoi(icon_size);
                                 for (i = 0; i < directory_paths->size; i++)
				    {
	                               Ecore_Hash *sub_group;

                                       printf("FDO icon path = %s\n", directory_paths->list[i]);
                                       sub_group = (Ecore_Hash *) ecore_hash_get(theme, directory_paths->list[i]);
				       if (sub_group)
				          {
	                                     char *size, *type, *minsize, *maxsize, *threshold;

                                             size = (char *) ecore_hash_get(sub_group, "Size");
                                             type = (char *) ecore_hash_get(sub_group, "Type");
                                             minsize = (char *) ecore_hash_get(sub_group, "MinSize");
                                             maxsize = (char *) ecore_hash_get(sub_group, "MaxSize");
                                             threshold = (char *) ecore_hash_get(sub_group, "Threshold");
					     if (size)
					        {
						   int match = 0;
						   int this_size;

                                                   this_size = atoi(size);
						   if (!type)
						      type = "Threshold";
						   switch (type[0])
						      {
						         case 'F' :   /* Fixed. */
							    {
							       match = (wanted_size == this_size);
							       break;
							    }
						         case 'S' :   /* Scaled. */
							    {
						               int min_size, max_size;

                                                               if (!minsize)
							          minsize = size;
                                                               if (!maxsize)
							          maxsize = size;
                                                               min_size = atoi(minsize);
                                                               max_size = atoi(maxsize);
							       match = ((min_size <= wanted_size) && (wanted_size <= max_size));
							       break;
							    }
						         default :    /* Threshold. */
							    {
						               int thresh_size;

                                                               if (!threshold)
							          threshold = "2";
                                                               thresh_size = atoi(threshold);
							       match = ( ((this_size - thresh_size) <= wanted_size) && (wanted_size <= (this_size + thresh_size)) );
							       break;
							    }
						      }
						   if (match)
						      {
						         char *found;

                                                         /* First try a .png file. */
                                                         snprintf(path, MAX_PATH, "%s/%s/%s.png", icon_theme, directory_paths->list[i], icon);
                                                         printf("FDO icon = %s\n", path);
                                                         found = fdo_paths_search_for_file(FDO_PATHS_TYPE_ICON, path, NULL, NULL);
							 if (found)
							    return found;
							 else
							    {  /* Then a .svg file. */
                                                               snprintf(path, MAX_PATH, "%s/%s/%s.svg", icon_theme, directory_paths->list[i], icon);
                                                               printf("FDO icon = %s\n", path);
                                                               found = fdo_paths_search_for_file(FDO_PATHS_TYPE_ICON, path, NULL, NULL);
							       if (found)
							          return found;
							       else
							          {  /* Then a .xpm file. */
                                                                     snprintf(path, MAX_PATH, "%s/%s/%s.xpm", icon_theme, directory_paths->list[i], icon);
                                                                     printf("FDO icon = %s\n", path);
                                                                     found = fdo_paths_search_for_file(FDO_PATHS_TYPE_ICON, path, NULL, NULL);
							             if (found)
							                return found;
							             else
							                {   /* Finally, try without an extension, in case one was given. */
                                                                           snprintf(path, MAX_PATH, "%s/%s/%s", icon_theme, directory_paths->list[i], icon);
                                                                           printf("FDO icon = %s\n", path);
                                                                           found = fdo_paths_search_for_file(FDO_PATHS_TYPE_ICON, path, NULL, NULL);
							                   if (found)
							                      return found;
							                }
							          }
							    }
						      }
						}
					  }
				    }
			      }
			}
		  }
	    }
	 free(theme_path);
      }

   return DEFAULTICON;
}
