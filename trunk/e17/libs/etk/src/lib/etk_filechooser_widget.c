/** @file etk_filechooser_widget.c */
#include "etk_filechooser_widget.h"
#include <stdlib.h>
#include <string.h>
#include <Ecore_Data.h>
#include <Ecore_File.h>
#include "etk_theme.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_utils.h"
#include "etk_button.h"
#include "etk_tree.h"
#include "etk_tree_model.h"
#include "etk_paned.h"
#include "etk_box.h"
#include "etk_hbox.h"
#include "etk_vbox.h"
#include "etk_entry.h"

/**
 * @addtogroup Etk_Filechooser_Widget
 * @{
 */

enum _Etk_Filechooser_Widget_Signal_Id
{
   ETK_FILECHOOSER_WIDGET_NUM_SIGNALS
};

enum _Etk_Filechooser_Widget_Property_Id
{
   ETK_FILECHOOSER_WIDGET_PATH_PROPERTY
};

typedef struct _Etk_Filechooser_Widget_Icons
{
   void *extension;
   char *icon;
} Etk_Filechooser_Widget_Icons;

static void _etk_filechooser_widget_constructor(Etk_Filechooser_Widget *filechooser_widget);
static void _etk_filechooser_widget_destructor(Etk_Filechooser_Widget *filechooser_widget);
static void _etk_filechooser_widget_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_filechooser_widget_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_filechooser_widget_favs_get(Etk_Filechooser_Widget *filechooser_widget);
static void _etk_filechooser_widget_dir_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data);
static void _etk_filechooser_widget_fav_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data);

static Etk_Filechooser_Widget_Icons _etk_file_chooser_icons[] =
{
   { "jpg", "mimetypes/image-x-generic_16" },
   { "jpeg", "mimetypes/image-x-generic_16" },
   { "png", "mimetypes/image-x-generic_16" },
   { "bmp", "mimetypes/image-x-generic_16" },
   { "gif", "mimetypes/image-x-generic_16" },
   { "mp3", "mimetypes/audio-x-generic_16" },
   { "ogg", "mimetypes/audio-x-generic_16" },
   { "wav", "mimetypes/audio-x-generic_16" },
   { "avi", "mimetypes/video-x-generic_16" },
   { "mpg", "mimetypes/video-x-generic_16" },
   { "mpeg", "mimetypes/video-x-generic_16" },
   { "gz", "mimetypes/package-x-generic_16" },
   { "tgz", "mimetypes/package-x-generic_16" },
   { "bz2", "mimetypes/package-x-generic_16" },
   { "tbz2", "mimetypes/package-x-generic_16" },
   { "zip", "mimetypes/package-x-generic_16" },
   { "rar", "mimetypes/package-x-generic_16" },
};
static int _etk_file_chooser_num_icons = sizeof(_etk_file_chooser_icons) / sizeof (_etk_file_chooser_icons[0]);
static Etk_Signal *_etk_filechooser_widget_signals[ETK_FILECHOOSER_WIDGET_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Filechooser_Widget
 * @return Returns the type on an Etk_Filechooser_Widget
 */
Etk_Type *etk_filechooser_widget_type_get()
{
   static Etk_Type *filechooser_widget_type = NULL;

   if (!filechooser_widget_type)
   {
      filechooser_widget_type = etk_type_new("Etk_Filechooser_Widget", ETK_BIN_TYPE, sizeof(Etk_Filechooser_Widget), ETK_CONSTRUCTOR(_etk_filechooser_widget_constructor), ETK_DESTRUCTOR(_etk_filechooser_widget_destructor));

      //_etk_filechooser_widget_signals[ETK_FILECHOOSER_WIDGET_TEXT_POPPED_SIGNAL] = etk_signal_new("text_popped", filechooser_widget_type, -1, etk_marshaller_VOID__INT_POINTER, NULL, NULL);
      
      etk_type_property_add(filechooser_widget_type, "path", ETK_FILECHOOSER_WIDGET_PATH_PROPERTY, ETK_PROPERTY_STRING, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_string(NULL));
      
      filechooser_widget_type->property_set = _etk_filechooser_widget_property_set;
      filechooser_widget_type->property_get = _etk_filechooser_widget_property_get;
   }

   return filechooser_widget_type;
}

/**
 * @brief Creates a new file chooser widget
 * @return Returns the new file chooser widget
 */
Etk_Widget *etk_filechooser_widget_new()
{
   return etk_widget_new(ETK_FILECHOOSER_WIDGET_TYPE, NULL);
}

/* TODO */
void etk_filechooser_widget_current_folder_set(Etk_Filechooser_Widget *filechooser_widget, const char *folder)
{
   Ecore_List *files;
   char *file_name;
   char *file_path = NULL;
   time_t mod_time;
   struct tm *mod_time2;
   char mod_time_string[128];
   
   if (!filechooser_widget)
      return;
   if (!folder && !(folder = getenv("HOME")))
      return;
   if (!(files = ecore_file_ls(folder)))
      return;
   
   free(filechooser_widget->current_folder);
   filechooser_widget->current_folder = strdup(folder);
   etk_tree_clear(ETK_TREE(filechooser_widget->dir_tree));
   etk_tree_clear(ETK_TREE(filechooser_widget->files_tree));
   /* TODO: fix */
   etk_tree_append(ETK_TREE(filechooser_widget->dir_tree),
      filechooser_widget->dir_col, etk_theme_icon_theme_get(), "actions/go-up_16", "..", NULL);
   
   ecore_list_goto_first(files);
   while ((file_name = ecore_list_next(files)))
   {
      if (file_name[0] == '.')
         continue;
      
      free(file_path);
      file_path = malloc(strlen(folder) + strlen(file_name) + 2);
      sprintf(file_path, "%s/%s", folder, file_name);
      
      if (!ecore_file_is_dir(file_path))
         continue;
      
      mod_time = ecore_file_mod_time(file_path);
      mod_time2 = gmtime(&mod_time);
      strftime(mod_time_string, 128, "%x", mod_time2);
      
      etk_tree_append(ETK_TREE(filechooser_widget->dir_tree),
         filechooser_widget->dir_col, etk_theme_icon_theme_get(), "places/folder_16", file_name, NULL);
      etk_tree_append(ETK_TREE(filechooser_widget->files_tree),
         filechooser_widget->files_name_col, etk_theme_icon_theme_get(), "places/folder_16", file_name,
         filechooser_widget->files_date_col, mod_time_string, NULL);
   }
   
   ecore_list_goto_first(files);
   while ((file_name = ecore_list_next(files)))
   {
      const char *ext;
      char *icon = NULL;
      int i;
      
      if (file_name[0] == '.')
         continue;
      
      free(file_path);
      file_path = malloc(strlen(folder) + strlen(file_name) + 2);
      sprintf(file_path, "%s/%s", folder, file_name);
      
      if (ecore_file_is_dir(file_path))
         continue;
      
      if ((ext = strrchr(file_name, '.')) && (ext = ext + 1))
      {
         for (i = 0; i < _etk_file_chooser_num_icons; i++)
         {
            if (strcasecmp(ext, _etk_file_chooser_icons[i].extension) == 0)
            {
               icon = _etk_file_chooser_icons[i].icon;
               break;
            }
         }
      }
      mod_time = ecore_file_mod_time(file_path);
      mod_time2 = gmtime(&mod_time);
      strftime(mod_time_string, 128, "%x", mod_time2);
      
      etk_tree_append(ETK_TREE(filechooser_widget->files_tree),
         filechooser_widget->files_name_col, etk_theme_icon_theme_get(), icon ? icon : "mimetypes/text-x-generic_16", file_name,
         filechooser_widget->files_date_col, mod_time_string, NULL);
   }
   
   free(file_path);
   ecore_list_destroy(files);
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the members */
static void _etk_filechooser_widget_constructor(Etk_Filechooser_Widget *fcw)
{
   Etk_Widget *hpaned, *vpaned;
   
   if (!fcw)
      return;
   
   hpaned = etk_hpaned_new();
   etk_widget_visibility_locked_set(hpaned, ETK_TRUE);
   etk_container_add(ETK_CONTAINER(fcw), hpaned);
   etk_widget_show(hpaned);
   
   vpaned = etk_vpaned_new();
   etk_widget_visibility_locked_set(vpaned, ETK_TRUE);
   etk_paned_add1(ETK_PANED(hpaned), vpaned, ETK_FALSE);
   etk_widget_show(vpaned);
   
   fcw->dir_tree = etk_tree_new();
   etk_widget_visibility_locked_set(fcw->dir_tree, ETK_TRUE);
   etk_widget_size_request_set(fcw->dir_tree, 180, 240);
   etk_paned_add1(ETK_PANED(vpaned), fcw->dir_tree, ETK_TRUE);
   fcw->dir_col = etk_tree_col_new(ETK_TREE(fcw->dir_tree), "Directories", etk_tree_model_icon_text_new(ETK_TREE(fcw->dir_tree), ETK_TREE_FROM_EDJE), 120);
   etk_tree_build(ETK_TREE(fcw->dir_tree));
   etk_widget_show(fcw->dir_tree);
   etk_signal_connect("row_selected", ETK_OBJECT(fcw->dir_tree), ETK_CALLBACK(_etk_filechooser_widget_dir_row_selected_cb), fcw);
   
   fcw->fav_tree = etk_tree_new();
   etk_widget_visibility_locked_set(fcw->fav_tree, ETK_TRUE);
   etk_widget_size_request_set(fcw->fav_tree, 180, 120);
   etk_paned_add2(ETK_PANED(vpaned), fcw->fav_tree, ETK_FALSE);
   fcw->fav_col = etk_tree_col_new(ETK_TREE(fcw->fav_tree), "Favorites", etk_tree_model_icon_text_new(ETK_TREE(fcw->fav_tree), ETK_TREE_FROM_EDJE), 120);
   etk_tree_build(ETK_TREE(fcw->fav_tree));
   etk_widget_show(fcw->fav_tree);
   
   etk_signal_connect("row_selected", ETK_OBJECT(fcw->fav_tree), ETK_CALLBACK(_etk_filechooser_widget_fav_row_selected_cb), fcw);
   
   fcw->files_tree = etk_tree_new();
   etk_widget_visibility_locked_set(fcw->files_tree, ETK_TRUE);
   etk_widget_size_request_set(fcw->files_tree, 400, 120);
   etk_paned_add2(ETK_PANED(hpaned), fcw->files_tree, ETK_TRUE);
   fcw->files_name_col = etk_tree_col_new(ETK_TREE(fcw->files_tree), "Filename", etk_tree_model_icon_text_new(ETK_TREE(fcw->files_tree), ETK_TREE_FROM_EDJE), 100);
   etk_tree_col_expand_set(fcw->files_name_col, ETK_TRUE);
   fcw->files_date_col = etk_tree_col_new(ETK_TREE(fcw->files_tree), "Date", etk_tree_model_text_new(ETK_TREE(fcw->files_tree)), 60);
   etk_tree_build(ETK_TREE(fcw->files_tree));
   etk_widget_show(fcw->files_tree);

   _etk_filechooser_widget_favs_get(ETK_FILECHOOSER_WIDGET(fcw));   
   
   fcw->current_folder = NULL;
   /* Go to home */
   etk_filechooser_widget_current_folder_set(ETK_FILECHOOSER_WIDGET(fcw), NULL);
}

/* Destroys the status bar */
static void _etk_filechooser_widget_destructor(Etk_Filechooser_Widget *filechooser_widget)
{
   if (!filechooser_widget)
      return;
   free(filechooser_widget->current_folder);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_filechooser_widget_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Filechooser_Widget *filechooser_widget;

   if (!(filechooser_widget = ETK_FILECHOOSER_WIDGET(object)) || !value)
      return;

   switch (property_id)
   {/*
      case ETK_FILECHOOSER_WIDGET_HAS_RESIZE_GRIP_PROPERTY:
         etk_filechooser_widget_has_resize_grip_set(filechooser_widget, etk_property_value_bool_get(value));
         break;*/
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_filechooser_widget_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Filechooser_Widget *filechooser_widget;

   if (!(filechooser_widget = ETK_FILECHOOSER_WIDGET(object)) || !value)
      return;

   switch (property_id)
   {/*
      case ETK_FILECHOOSER_WIDGET_HAS_RESIZE_GRIP_PROPERTY:
         etk_property_value_bool_set(value, filechooser_widget->has_resize_grip);
         break;*/
      default:
         break;
   }
}

/* Get favorites from file in ~/ETK_FILECHOOSER_FAVS */
static void _etk_filechooser_widget_favs_get(Etk_Filechooser_Widget *filechooser_widget)
{
   char *folder;
   char file_path[PATH_MAX];
   char fav[PATH_MAX];
   char line[PATH_MAX];
   FILE *f;
   
   if (!filechooser_widget)
     return;
   if (!(folder = getenv("HOME")))
     return;
   
   snprintf(file_path, sizeof(file_path), "%s/%s", folder, ETK_FILECHOOSER_FAVS);
     
   if((f = fopen (file_path, "r")) == NULL)
     return;
   
   etk_tree_freeze(ETK_TREE(filechooser_widget->fav_tree));
   etk_tree_clear(ETK_TREE(filechooser_widget->fav_tree));
   
   while(fgets(line, PATH_MAX, f) != NULL)
     {
	Etk_Tree_Row *row;
	sscanf(line,"file://%s",fav);
	row = etk_tree_append(ETK_TREE(filechooser_widget->fav_tree),
			      filechooser_widget->fav_col, etk_theme_icon_theme_get(), "places/folder_16", ecore_file_get_file(fav), NULL);
	etk_tree_row_data_set(row, (void*)strdup(fav)); /* any special things to do here to free? */
     }
   
   fclose (f);
   etk_tree_thaw(ETK_TREE(filechooser_widget->fav_tree));
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when a row of the "directories" tree is selected */
static void _etk_filechooser_widget_dir_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Filechooser_Widget *filechooser_widget;
   char *selected_dir;
   char *new_dir;
   
   if (!(filechooser_widget = ETK_FILECHOOSER_WIDGET(data)))
      return;
   etk_tree_row_fields_get(row, filechooser_widget->dir_col, NULL, NULL, &selected_dir, NULL);
   
   new_dir = malloc(strlen(filechooser_widget->current_folder) + strlen(selected_dir) + 2);
   sprintf(new_dir, "%s/%s", filechooser_widget->current_folder, selected_dir);
   etk_filechooser_widget_current_folder_set(filechooser_widget, new_dir);
   free(new_dir);
}

static void _etk_filechooser_widget_fav_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Filechooser_Widget *filechooser_widget;
   char *selected_dir;
   
   if (!(filechooser_widget = ETK_FILECHOOSER_WIDGET(data)))
     return;
      
   selected_dir = etk_tree_row_data_get(row);
   etk_filechooser_widget_current_folder_set(filechooser_widget, selected_dir);
   
}    

/**************************
 *
 * Private functions
 *
 **************************/

/** @} */
