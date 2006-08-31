#include <limits.h>
#include <Ecore_File.h>
#include <Ecore_Data.h>
#include <Etk.h>
#include "config.h"

void etk_prefs_show();
void etk_prefs_standard_item_add(Etk_Widget *tree, char *icon, char *label, void (*func) (void));

static void _etk_prefs_row_clicked(Etk_Object *object, Etk_Tree_Row *row, Etk_Event_Mouse_Up *event, void *data);
static void _etk_prefs_destroy(void *data);    
static Etk_Widget *_etk_prefs_theme_tab_create();
static void _etk_prefs_theme_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data);
static Etk_Widget *_etk_prefs_theme_preview_get(const char *theme);
static void _etk_prefs_response_cb(Etk_Object *object, int response_id, void *data);
static void _etk_prefs_apply();
  
static char *_etk_prefs_widget_theme = NULL;

int main(int argc, char **argv)
{
   etk_init(&argc, &argv);
   
   etk_prefs_show();
   
   etk_main();
   etk_shutdown();
   
   return 0;
}

void etk_prefs_show()
{
   Etk_Widget *dialog;
   Etk_Widget *tree;
   Etk_Widget *notebook;
   Etk_Widget *paned;
   Etk_Tree_Col *col;
   
   /* main dialog to hold everything */
   dialog = etk_dialog_new();
   etk_window_title_set(ETK_WINDOW(dialog), _("Etk Preferences"));
   etk_window_wmclass_set(ETK_WINDOW(dialog), "Etk Preferences", "Etk Preferences");

   /* this will hold the current pref's contents */
   notebook = etk_notebook_new();
   etk_notebook_tabs_visible_set(ETK_NOTEBOOK(notebook), ETK_FALSE);
   
   /* tree to show the possible preferences */
   tree = etk_tree_new();
   etk_tree_headers_visible_set(ETK_TREE(tree), ETK_FALSE);
   etk_signal_connect("row_clicked", ETK_OBJECT(tree), ETK_CALLBACK(_etk_prefs_row_clicked), notebook);
   etk_widget_size_request_set(tree, 180, 240);
   etk_tree_mode_set(ETK_TREE(tree), ETK_TREE_MODE_LIST);
   etk_tree_multiple_select_set(ETK_TREE(tree), ETK_FALSE);
   etk_tree_row_height_set(ETK_TREE(tree), 52);   
   col = etk_tree_col_new(ETK_TREE(tree), _("Category"), etk_tree_model_icon_text_new(ETK_TREE(tree), ETK_TREE_FROM_EDJE), 90);
   etk_tree_build(ETK_TREE(tree));
   etk_tree_freeze(ETK_TREE(tree));
   etk_prefs_standard_item_add(tree, "apps/preferences-desktop-theme_48", _("Theme"), NULL);
   etk_prefs_standard_item_add(tree, "apps/preferences-desktop-font_48", _("Fonts"), NULL);
   etk_prefs_standard_item_add(tree, "apps/preferences-desktop-locale_48", _("Language"), NULL);
   etk_prefs_standard_item_add(tree, "apps/system-users_48", _("User Preferences"), NULL);
   etk_prefs_standard_item_add(tree, "categories/preferences-system_48", _("General"), NULL);   
   etk_tree_thaw(ETK_TREE(tree));
   
   /* paned to hold the tree on one side and the pref's contents on the other */
   paned = etk_hpaned_new();
   etk_paned_child1_set(ETK_PANED(paned), tree, ETK_FALSE);
   etk_paned_child2_set(ETK_PANED(paned), notebook, ETK_TRUE);   

   /* Some buttons */
   etk_dialog_button_add_from_stock(ETK_DIALOG(dialog), ETK_STOCK_DIALOG_CLOSE, ETK_RESPONSE_CLOSE);
   etk_dialog_button_add_from_stock(ETK_DIALOG(dialog), ETK_STOCK_DIALOG_APPLY, ETK_RESPONSE_APPLY);
   etk_dialog_button_add_from_stock(ETK_DIALOG(dialog), ETK_STOCK_DIALOG_OK, ETK_RESPONSE_OK);   
   etk_signal_connect("response", ETK_OBJECT(dialog), ETK_CALLBACK(_etk_prefs_response_cb), dialog);
   
   etk_container_border_width_set(ETK_CONTAINER(dialog), 5);
   etk_dialog_pack_in_main_area(ETK_DIALOG(dialog), paned, ETK_TRUE, ETK_TRUE,
				0, ETK_FALSE);
   
   /* create tabs */
   etk_notebook_page_append(ETK_NOTEBOOK(notebook), "Themes", _etk_prefs_theme_tab_create());
   
   etk_widget_show_all(dialog);
}

void etk_prefs_standard_item_add(Etk_Widget *tree,
				 char *icon, char *label, 
				 void (*func) (void))
{
   static int i = 0;
   int *j;
   const char *file;
   Etk_Tree_Row *row;
   
   file = etk_theme_icon_theme_get();   
   row = etk_tree_append(ETK_TREE(tree), etk_tree_nth_col_get(ETK_TREE(tree),
							      0),
                         file, icon, label, NULL);
   i++;
   j = malloc(sizeof(int));
   *j = i;
   etk_tree_row_data_set(row, j);
}

static void _etk_prefs_row_clicked(Etk_Object *object, Etk_Tree_Row *row, Etk_Event_Mouse_Up *event, void *data)
{  
   int *num = NULL;
   
   num = etk_tree_row_data_get(row);
   
   if (num)
     etk_notebook_current_page_set(ETK_NOTEBOOK(data), *num);
}

static void _etk_prefs_destroy(void *data)
{
   etk_object_destroy(ETK_OBJECT(data));
   etk_main_quit();
}

static Etk_Widget *_etk_prefs_theme_tab_create()
{
   Evas_List *themes;
   Evas_List *l;   
   char *theme;
   
   Etk_Widget *preview;
   Etk_Widget *theme_list;
   
   Etk_Widget *preview_hbox;
   Etk_Widget *option_vbox;
   Etk_Widget *frame;
   Etk_Widget *button;
   Etk_Widget *vbox;
   Etk_Tree_Col *col1;
   Etk_Tree_Row *row;
      
   /* main vbox */
   vbox = etk_vbox_new(ETK_FALSE, 0);
   
   /* hbox to hold tree and preview */
   preview_hbox = etk_hbox_new(ETK_FALSE, 5);
   etk_box_append(ETK_BOX(vbox), preview_hbox, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   
   /* the preview */
   preview = etk_frame_new("Preview");
      
   /* tree to hold the theme list */
   theme_list = etk_tree_new();
   etk_widget_size_request_set(theme_list, 180, 240);
   etk_tree_mode_set(ETK_TREE(theme_list), ETK_TREE_MODE_LIST);
   etk_tree_multiple_select_set(ETK_TREE(theme_list), ETK_FALSE);
   col1 = etk_tree_col_new(ETK_TREE(theme_list), "Themes", etk_tree_model_text_new(ETK_TREE(theme_list)), 150);
   etk_tree_row_height_set(ETK_TREE(theme_list), 60);   
   etk_tree_headers_visible_set(ETK_TREE(theme_list), ETK_FALSE);
   etk_signal_connect("row_selected", ETK_OBJECT(theme_list), ETK_CALLBACK(_etk_prefs_theme_row_selected_cb), preview);
   etk_tree_build(ETK_TREE(theme_list));   
   
   /* scan for themes and add them to the list */
   themes = etk_theme_widget_theme_available_get();
   for(l = themes; l; l = l->next)
     {
        const char *widget_theme = etk_config_widget_theme_get();
	
	theme = l->data;
        row = etk_tree_append(ETK_TREE(theme_list), col1, theme,  NULL);
	if (widget_theme)
        if (!strcmp(theme, widget_theme))
	    etk_tree_row_select(row);
     }
   
   /* pack tree + preview widget */
   etk_box_append(ETK_BOX(preview_hbox), theme_list, ETK_BOX_START, ETK_BOX_NONE, 0);
   etk_box_append(ETK_BOX(preview_hbox), preview, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);

   /* box to store the check buttons */
   frame = etk_frame_new("Options");
   option_vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(frame), option_vbox);
   etk_box_append(ETK_BOX(vbox), frame, ETK_BOX_START, ETK_BOX_NONE, 0);
   
   /* check buttons for various options */
   button = etk_check_button_new_with_label("Use built in font");
   etk_box_append(ETK_BOX(option_vbox), button, ETK_BOX_START, ETK_BOX_NONE, 0);
   //etk_signal_connect("clicked", ETK_OBJECT(button), ETK_CALLBACK(_e_theme_apply_perm_cb), NULL);
      
   etk_container_border_width_set(ETK_CONTAINER(vbox), 5);

   frame = etk_frame_new("Themes");
   etk_container_add(ETK_CONTAINER(frame), vbox);
   
   return frame;
}

static void _etk_prefs_theme_row_selected_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   static Etk_Widget *child = NULL;
   Etk_Tree *tree;
   char *icol_string;
   Etk_Widget *preview;
     
   tree = ETK_TREE(object);
   preview = data;

   etk_tree_row_fields_get(row, etk_tree_nth_col_get(tree, 0), &icol_string, NULL, NULL, NULL);
   if(child)
   {
      etk_container_remove(ETK_CONTAINER(preview), child);
      etk_object_destroy(ETK_OBJECT(child));
   }
   child = _etk_prefs_theme_preview_get(icol_string);
   etk_container_add(ETK_CONTAINER(preview), child);
   etk_widget_show_all(child);
}

static Etk_Widget *_etk_prefs_theme_preview_get(const char *theme)
{
   char file[PATH_MAX];
   Etk_Widget *box;
   Etk_Widget *widget;
   Etk_Widget *vbox;
   Etk_Widget *frame;
   
   snprintf(file, sizeof(file), PACKAGE_DATA_DIR"/themes/%s.edj", theme);
   if (!ecore_file_exists(file))
   {
      char *home;
      
      home = getenv("HOME");
      if (!home)
	return NULL;
      
      snprintf(file, sizeof(file), "%s/.e/etk/themes/%s.edj", home, theme);
      if (!ecore_file_exists(file))
	return NULL;
   }
   
   free(_etk_prefs_widget_theme);
   _etk_prefs_widget_theme = strdup(theme);
   
   box = etk_vbox_new(ETK_FALSE, 0);
   etk_widget_theme_file_set(box, file);
   
   frame = etk_frame_new("Buttons");
   etk_box_append(ETK_BOX(box), frame, ETK_BOX_START, ETK_BOX_NONE, 0);
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(frame), vbox);
   widget = etk_button_new_with_label("Regular Button");
   etk_box_append(ETK_BOX(vbox), widget, ETK_BOX_START, ETK_BOX_NONE, 0);   
   widget = etk_check_button_new_with_label("Check Button");
   etk_box_append(ETK_BOX(vbox), widget, ETK_BOX_START, ETK_BOX_NONE, 0);   
   widget = etk_radio_button_new_with_label("Radio Button", NULL);
   etk_box_append(ETK_BOX(vbox), widget, ETK_BOX_START, ETK_BOX_NONE, 0);   
   
   frame = etk_frame_new("Text");
   etk_box_append(ETK_BOX(box), frame, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
   vbox = etk_vbox_new(ETK_FALSE, 0);
   etk_container_add(ETK_CONTAINER(frame), vbox);
   widget = etk_entry_new();
   etk_entry_text_set(ETK_ENTRY(widget), "Sample text...");
   etk_box_append(ETK_BOX(vbox), widget, ETK_BOX_START, ETK_BOX_NONE, 0);
   widget = etk_text_view_new();
   etk_textblock_text_set(etk_text_view_textblock_get(ETK_TEXT_VIEW(widget)),
			  "Multi-line text widget!\nHow about that! (=",
			  ETK_TRUE);
   etk_widget_size_request_set(widget, 320, 50);
   etk_box_append(ETK_BOX(vbox), widget, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
         
   return box;
}

static void _etk_prefs_response_cb(Etk_Object *object, int response_id, void *data)
{
   switch(response_id)
   {
      case ETK_RESPONSE_OK:
      _etk_prefs_apply();
      etk_config_save();
      etk_main_quit();
      break;

      case ETK_RESPONSE_APPLY:
      _etk_prefs_apply();      
      etk_config_save();
      break;
      
      case ETK_RESPONSE_CLOSE:
      etk_main_quit();
      break;
   }
}

static void _etk_prefs_apply()
{
   if (_etk_prefs_widget_theme)
     etk_config_widget_theme_set(_etk_prefs_widget_theme);
}
