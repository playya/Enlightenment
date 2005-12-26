#include "exhibit.h"

Ex_Tab *
_ex_tab_new(Exhibit *e, char *dir)
{
   Ex_Tab *tab;
   Etk_Tree_Model *imodel;   
   
   tab = calloc(1, sizeof(Ex_Tab));   
   tab->num = evas_list_count(e->tabs);
   tab->dirs = NULL;
   tab->images = NULL;
   tab->e = e;
   tab->fit_window = FALSE;
   
   tab->dtree = etk_tree_new();
   etk_widget_size_request_set(tab->dtree, 180, 120);
   etk_signal_connect("row_selected", ETK_OBJECT(tab->dtree), ETK_CALLBACK(_ex_main_dtree_item_clicked_cb), e);
   tab->dcol = etk_tree_col_new(ETK_TREE(tab->dtree), "Directories", etk_tree_model_icon_text_new(ETK_TREE(tab->dtree), ETK_TREE_FROM_EDJE), 10);
   etk_tree_headers_visible_set(ETK_TREE(tab->dtree), 0);
   etk_tree_build(ETK_TREE(tab->dtree));

   tab->itree = etk_tree_new();
   etk_widget_size_request_set(tab->itree, 180, 120);
   etk_tree_multiple_select_set(ETK_TREE(tab->itree), TRUE);
   etk_signal_connect("row_selected", ETK_OBJECT(tab->itree), ETK_CALLBACK(_ex_main_itree_item_clicked_cb), e);
   etk_signal_connect("key_down", ETK_OBJECT(tab->itree), ETK_CALLBACK(_ex_main_itree_key_down_cb), e);
   imodel = etk_tree_model_icon_text_new(ETK_TREE(tab->itree), ETK_TREE_FROM_FILE);
   etk_tree_model_icon_text_icon_width_set(imodel, 80);
   tab->icol = etk_tree_col_new(ETK_TREE(tab->itree), "Files", imodel, 10);
   etk_tree_headers_visible_set(ETK_TREE(tab->itree), 0);
   etk_tree_row_height_set(ETK_TREE(tab->itree), 60);
   etk_tree_build(ETK_TREE(tab->itree));

   if(dir)
     tab->dir = strdup(dir);
   else
     tab->dir = strdup(".");

   tab->alignment = etk_alignment_new(0.5, 0.5, 0.0, 0.0);   
   
   tab->image = etk_image_new();
   etk_widget_theme_set(tab->image, PACKAGE_DATA_DIR"/images/images.edj", "image_bg");
   etk_signal_connect("mouse_down", ETK_OBJECT(tab->image), ETK_CALLBACK(_ex_image_mouse_down), e);
   etk_signal_connect("mouse_up", ETK_OBJECT(tab->image), ETK_CALLBACK(_ex_image_mouse_up), e);
   etk_signal_connect("mouse_move", ETK_OBJECT(tab->image), ETK_CALLBACK(_ex_image_mouse_move), e);
   etk_image_keep_aspect_set(ETK_IMAGE(tab->image), TRUE);
   etk_container_add(ETK_CONTAINER(tab->alignment), tab->image);   
      
   tab->scrolled_view = etk_scrolled_view_new();
   etk_scrolled_view_add_with_viewport(ETK_SCROLLED_VIEW(tab->scrolled_view), tab->alignment);
      
   return tab;
}

void
_ex_tab_dir_set(Ex_Tab *tab, char *path)
{
   
}

void
_ex_tab_delete(Ex_Tab *tab)
{
   
}

void
_ex_tab_select(Ex_Tab *tab)
{   
   chdir(tab->cur_path);

   if(tab->fit_window)
     etk_notebook_page_child_set(ETK_NOTEBOOK(tab->e->notebook), tab->num, tab->alignment);
   
   etk_table_attach(ETK_TABLE(tab->e->table), tab->dtree,
		    0, 3, 3, 3,
		    0, 0, ETK_FILL_POLICY_VEXPAND|ETK_FILL_POLICY_VFILL|ETK_FILL_POLICY_HFILL);
   etk_widget_show(tab->dtree);
   
   etk_paned_add2(ETK_PANED(tab->e->vpaned), tab->itree, TRUE);
   etk_widget_show(tab->itree);
      
   etk_widget_show(tab->image);
   etk_widget_show(tab->alignment);   
   etk_widget_show(tab->scrolled_view);
      
   etk_widget_show_all(tab->e->win);
}
