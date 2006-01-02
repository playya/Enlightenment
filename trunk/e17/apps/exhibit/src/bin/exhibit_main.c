#include "exhibit.h"
#include <Ecore_File.h>

extern pid_t pid;
extern Evas_List *thumb_list;

Ecore_Evas *ee_buf;
Evas       *evas_buf;

char *viewables[] =
{
   ".jpg",
   ".png",
   ".edj"
};

Evas_List *event_handlers;

void
_ex_main_statusbar_zoom_update(Exhibit *e)
{
   if (e->cur_tab->fit_window)
     {
        etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[2]), 0);
        etk_statusbar_push(ETK_STATUSBAR(e->statusbar[2]), _("Fit to window"), 0);
     }
   else if(e->zoom > 0)
     {
	char zoom[6];
	etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[2]), 0);
	snprintf(zoom, sizeof(zoom), "%d:1", abs(e->zoom));
	etk_statusbar_push(ETK_STATUSBAR(e->statusbar[2]), zoom, 0);
     }
   else if(e->zoom < 0)
     {
	char zoom[6];
	etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[2]), 0);
	snprintf(zoom, sizeof(zoom), "1:%d", abs(e->zoom));
	etk_statusbar_push(ETK_STATUSBAR(e->statusbar[2]), zoom, 0);;
     }
   else
     {
	etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[2]), 0);
	etk_statusbar_push(ETK_STATUSBAR(e->statusbar[2]), "1:1", 0);
     }      
}


void
_ex_main_button_zoom_in_cb(Etk_Object *obj, void *data)
{
   Exhibit      *e;
   
   e = data;
   
   if (e->cur_tab->fit_window)
     {
	if(evas_list_count(e->tabs) == 1)
	  etk_paned_add2(ETK_PANED(e->hpaned), e->cur_tab->scrolled_view, TRUE);
	else
	  etk_notebook_page_child_set(ETK_NOTEBOOK(e->notebook), e->cur_tab->num, e->cur_tab->scrolled_view);
	  
        etk_widget_size_request_set(e->cur_tab->alignment, -1, -1);
        etk_scrolled_view_add_with_viewport(ETK_SCROLLED_VIEW(e->cur_tab->scrolled_view), e->cur_tab->alignment);
        e->cur_tab->fit_window = FALSE;
     }
   
   if(e->zoom == ZOOM_MAX)
     e->zoom = ZOOM_MAX;
   else
     e->zoom += 2;
   
   _ex_image_zoom(ETK_IMAGE(e->cur_tab->image), e->zoom);
   _ex_main_statusbar_zoom_update(e);     
}

void
_ex_main_button_zoom_out_cb(Etk_Object *obj, void *data)
{
   Exhibit      *e;
   
   e = data;
   
   if (e->cur_tab->fit_window)
     {
	if(evas_list_count(e->tabs) == 1)	  
	  etk_paned_add2(ETK_PANED(e->hpaned), e->cur_tab->scrolled_view, TRUE);
	else
	  etk_notebook_page_child_set(ETK_NOTEBOOK(e->notebook), e->cur_tab->num, e->cur_tab->scrolled_view);
	
        etk_widget_size_request_set(e->cur_tab->alignment, -1, -1);
        etk_scrolled_view_add_with_viewport(ETK_SCROLLED_VIEW(e->cur_tab->scrolled_view), e->cur_tab->alignment);
        e->cur_tab->fit_window = FALSE;
     }
   
   if(e->zoom <= ZOOM_MIN)
     e->zoom = ZOOM_MIN;
   else
     e->zoom -= 2;
   
   _ex_image_zoom(ETK_IMAGE(e->cur_tab->image), e->zoom);
   _ex_main_statusbar_zoom_update(e);   
}

void
_ex_main_button_zoom_one_to_one_cb(Etk_Object *obj, void *data)
{
   Exhibit      *e;
   
   e = data;
   
   if (e->cur_tab->fit_window)
     {
	if(evas_list_count(e->tabs) == 1)
	  etk_paned_add2(ETK_PANED(e->hpaned), e->cur_tab->scrolled_view, TRUE);
	else
	  etk_notebook_page_child_set(ETK_NOTEBOOK(e->notebook), e->cur_tab->num, e->cur_tab->scrolled_view);
	
        etk_widget_size_request_set(e->cur_tab->alignment, -1, -1);
        etk_scrolled_view_add_with_viewport(ETK_SCROLLED_VIEW(e->cur_tab->scrolled_view), e->cur_tab->alignment);
        e->cur_tab->fit_window = FALSE;
     }
   
   e->zoom = 0;
   e->brightness = 128;
   e->contrast = 0;
   
   _ex_image_zoom(ETK_IMAGE(e->cur_tab->image), e->zoom);
   _ex_main_statusbar_zoom_update(e);   
}

void
_ex_main_button_fit_to_window_cb(Etk_Object *obj, void *data)
{
   Exhibit      *e;
   
   e = data;
   
   if (e->cur_tab->fit_window)
      return;
   
   etk_widget_size_request_set(e->cur_tab->alignment, 10, 10);
   if(evas_list_count(e->tabs) == 1)
     etk_paned_add2(ETK_PANED(e->hpaned), e->cur_tab->alignment, TRUE);
   else          
     etk_notebook_page_child_set(ETK_NOTEBOOK(e->notebook), e->cur_tab->num, e->cur_tab->alignment);
     
   etk_widget_size_request_set(e->cur_tab->image, -1, -1);
   
   e->cur_tab->fit_window = TRUE;
   _ex_main_statusbar_zoom_update(e);
}

void
_ex_main_itree_item_clicked_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Tree *tree;
   Etk_Range *hs, *vs;
   char *icol_string;
   char *title;
   int   w, h;
   int   bytes;
   char  size[30];
   Exhibit *e;

   e = data;
   e->zoom = 0;
   _ex_main_statusbar_zoom_update(e);
   
   tree = ETK_TREE(object);

   etk_tree_row_fields_get(row, etk_tree_nth_col_get(tree, 0), NULL, &icol_string, etk_tree_nth_col_get(tree, 1),NULL);
   title = calloc(strlen(icol_string) + strlen(WINDOW_TITLE) + 5, sizeof(char));
   snprintf(title, strlen(icol_string) + strlen(WINDOW_TITLE) + 5, "%s - %s", icol_string, WINDOW_TITLE);
   etk_window_title_set(ETK_WINDOW(e->win), title);
      
   if(_ex_file_is_ebg(icol_string))
     {
	/* can we do this without the size request? it doesnt look good */
	etk_widget_size_request_set(ETK_WIDGET(e->cur_tab->image), 800, 600);	
	etk_image_set_from_edje(ETK_IMAGE(e->cur_tab->image), icol_string, "desktop/background");
     }
   else
     {
	etk_image_set_from_file(ETK_IMAGE(e->cur_tab->image), icol_string);
	etk_image_size_get(ETK_IMAGE(e->cur_tab->image), &w, &h);	
	etk_widget_size_request_set(ETK_WIDGET(e->cur_tab->image), w, h);
     }
   
   bytes = ecore_file_size(icol_string);
   snprintf(size, sizeof(size), "%d K", (int)(bytes/1024));
   etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[0]), 0);
   etk_statusbar_push(ETK_STATUSBAR(e->statusbar[0]), size, 0);
      
   etk_image_size_get(ETK_IMAGE(e->cur_tab->image), &w, &h);   
   snprintf(size, sizeof(size), "( %d x %d )", w, h);
   etk_statusbar_pop(ETK_STATUSBAR(e->statusbar[1]), 0);
   etk_statusbar_push(ETK_STATUSBAR(e->statusbar[1]), size, 0);
   
   hs = etk_scrolled_view_hscrollbar_get(ETK_SCROLLED_VIEW(e->cur_tab->scrolled_view));
   vs = etk_scrolled_view_vscrollbar_get(ETK_SCROLLED_VIEW(e->cur_tab->scrolled_view));
      
   etk_range_value_set(hs, (double)w/2);
   etk_range_value_set(vs, (double)h/2);   
}

void
_ex_main_dtree_item_clicked_cb(Etk_Object *object, Etk_Tree_Row *row, void *data)
{
   Etk_Tree *tree;
   char *dcol_string;
   Exhibit *e;

   e = data;
   _ex_slideshow_stop(e);
   
   tree = ETK_TREE(object);
   etk_tree_row_fields_get(row, etk_tree_nth_col_get(tree, 0), NULL, NULL, &dcol_string, NULL);

   free(e->dir);
   e->cur_tab->dir = strdup(dcol_string);
   etk_tree_clear(ETK_TREE(e->cur_tab->itree));
   etk_tree_clear(ETK_TREE(e->cur_tab->dtree));
   _ex_main_populate_files(e, NULL);
   etk_notebook_page_tab_label_set(ETK_NOTEBOOK(e->notebook), e->cur_tab->num, _ex_file_get(e->cur_tab->cur_path));
}


int
_ex_main_dtree_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   char *dir1, *dir2;
   
   if (!row1 || !row2 || !col)
      return 0;
   
   etk_tree_row_fields_get(row1, col, NULL, NULL, &dir1, NULL);
   etk_tree_row_fields_get(row2, col, NULL, NULL, &dir2, NULL);
   return strcasecmp(dir1, dir2);
}

int
_ex_main_itree_name_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   char *f1, *f2;
   
   if (!row1 || !row2 || !col)
      return 0;
   
   etk_tree_row_fields_get(row1, col, NULL, &f1, NULL,NULL);
   etk_tree_row_fields_get(row2, col, NULL, &f2, NULL,NULL);
   
   return strcasecmp(f1, f2);
}

int
_ex_main_itree_size_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   char *f1, *f2;
   struct stat s1, s2;
   
   if (!row1 || !row2 || !col)
      return 0;
   
   etk_tree_row_fields_get(row1, col, NULL, &f1, NULL,NULL);
   etk_tree_row_fields_get(row2, col, NULL, &f2, NULL,NULL);
   
   stat(f1, &s1);
   stat(f2, &s2);
   
   if(s1.st_size > s2.st_size)
     return 1;
   else
     return -1;
}

int
_ex_main_itree_date_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   char *f1, *f2;
   struct stat s1, s2;
   
   if (!row1 || !row2 || !col)
      return 0;
   
   etk_tree_row_fields_get(row1, col, NULL, &f1, NULL,NULL);
   etk_tree_row_fields_get(row2, col, NULL, &f2, NULL,NULL);
   
   stat(f1, &s1);
   stat(f2, &s2);
   
   if(s1.st_mtime > s2.st_mtime)
     return 1;
   else
     return -1;
}

int
_ex_main_itree_resol_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   char *f1, *f2;
   Evas_Object *i1, *i2;
   int w1, h1, w2, h2;
   
   if (!row1 || !row2 || !col)
      return 0;
   
   etk_tree_row_fields_get(row1, col, NULL, &f1, NULL,NULL);
   etk_tree_row_fields_get(row2, col, NULL, &f2, NULL,NULL);

   if(!ee_buf)
     {
	ee_buf = ecore_evas_buffer_new(0, 0);
	evas_buf = ecore_evas_get(ee_buf);
     }   
   
   if(_ex_file_is_ebg(f1))
     {
	w1 = 800;
	h1 = 600;
     }
   else
     {
	i1 = evas_object_image_add(evas_buf);
	evas_object_image_file_set(i1, f1, NULL);
	evas_object_image_size_get(i1, &w1, &h1);
	evas_object_del(i1);
     }
   
   if(_ex_file_is_ebg(f2))
     {
	w2 = 800;
	h2 = 600;
     }
   else
     {
	i2 = evas_object_image_add(evas_buf);
	evas_object_image_file_set(i2, f2, NULL);
	evas_object_image_size_get(i2, &w2, &h2);
	evas_object_del(i2);	
     }      
   
   if(w1 * h1 > w2 * h2)
     return 1;
   else
     return -1;   
}

int
_ex_file_is_viewable(char *file)
{
   char *ext;
   int i;

   ext = strrchr(file, '.');
   if(!ext) return 0;

   for(i = 0; i < VIEWABLES; i++)
     {
	if(!strcasecmp(ext, viewables[i]))
	  return 1;
     }

   return 0;
}

void
_ex_main_populate_files(Exhibit *e, char *selected_file)
{
   char back[PATH_MAX];
   DIR *dir;
   struct dirent *dir_entry;
   Etk_Tree_Row *row, *selected_row;

   selected_row = NULL;
   snprintf(back, PATH_MAX, "..");
   etk_tree_append(ETK_TREE(e->cur_tab->dtree), e->cur_tab->dcol, 
		   ETK_DEFAULT_ICON_SET_FILE, "actions/go-up", back, NULL);

   chdir(e->cur_tab->dir);

   if ((dir = opendir(".")) == NULL)
     return ;

   etk_tree_freeze(ETK_TREE(e->cur_tab->itree));
   etk_tree_freeze(ETK_TREE(e->cur_tab->dtree));
   
   getcwd(e->cur_tab->cur_path, PATH_MAX);
   if(strlen(e->cur_tab->cur_path) < PATH_MAX - 2)
     {
	int len = strlen(e->cur_tab->cur_path);
	e->cur_tab->cur_path[len] = '/';
	e->cur_tab->cur_path[len + 1] = '\0';
     }
   etk_entry_text_set(ETK_ENTRY(e->entry[0]), e->cur_tab->cur_path);
   
   while ((dir_entry = readdir(dir)) != NULL)
     {
	char image[PATH_MAX];
	char imagereal[PATH_MAX];
	struct stat st;
	Epsilon *ep;

        /* Do not include hidden files */
	if (dir_entry->d_name[0] == '.')
	  continue;

	snprintf(image, PATH_MAX, "%s", dir_entry->d_name);
	if(stat(image, &st) == -1) continue;
	if(S_ISDIR(st.st_mode))
	  {
	     etk_tree_append(ETK_TREE(e->cur_tab->dtree), e->cur_tab->dcol,
			     ETK_DEFAULT_ICON_SET_FILE,
			     "mimetypes/x-directory-normal",
			     dir_entry->d_name, NULL);
	     e->cur_tab->dirs = evas_list_append(e->cur_tab->dirs, dir_entry->d_name);
	     continue;
	  }

	if(!_ex_file_is_viewable(dir_entry->d_name))
	  continue;

	if(!realpath(image, imagereal))
	  snprintf(imagereal, PATH_MAX, "%s", image);

	ep = epsilon_new(imagereal);
	epsilon_thumb_size(ep, EPSILON_THUMB_NORMAL);

	if(epsilon_exists(ep) == EPSILON_OK)
	  {
	     char *thumb;

	     thumb = (char*)epsilon_thumb_file_get(ep);
	     row = etk_tree_append(ETK_TREE(e->cur_tab->itree), e->cur_tab->icol, thumb, dir_entry->d_name, NULL);
	     if(selected_file)	       		  		  
	       if(!strcmp(selected_file, dir_entry->d_name))
		 selected_row = row;		    	     
	     free(thumb);
	  }
	else {
	   Ex_Thumb *thumb;

	   thumb = calloc(1, sizeof(Ex_Thumb));
	   thumb->ep = ep;
	   thumb->e = e;
	   thumb->name = strdup(dir_entry->d_name);
	   thumb_list = evas_list_append(thumb_list, thumb);
	   if(selected_file)
	     {
		if(!strcmp(selected_file, dir_entry->d_name))
		  thumb->selected = TRUE;
	     }
	   else
	     thumb->selected = FALSE;
	   if(pid == -1) _ex_thumb_generate();
	}
     }

   etk_tree_thaw(ETK_TREE(e->cur_tab->itree));
   etk_tree_thaw(ETK_TREE(e->cur_tab->dtree));
   etk_tree_sort(ETK_TREE(e->cur_tab->dtree), _ex_main_dtree_compare_cb, TRUE, e->cur_tab->dcol, NULL);

   if(selected_row)
     {
	etk_tree_row_select(selected_row);
	etk_tree_row_scroll_to(selected_row, TRUE);
     }
   
   closedir(dir);
}

void
_ex_main_itree_key_down_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Key_Up_Down *ev;
   Exhibit *e;

   e = data;
   ev = event;

   if(!strcmp(ev->key, "Return") || !strcmp(ev->key, "KP_Enter"))
     {
        e->cur_tab->dir = strdup((char*)etk_entry_text_get(ETK_ENTRY(e->entry[0])));
        etk_tree_clear(ETK_TREE(e->cur_tab->itree));
        etk_tree_clear(ETK_TREE(e->cur_tab->dtree));
        _ex_main_populate_files(e, NULL);
     }
}

void
_ex_main_entry_dir_key_down_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Key_Up_Down *ev;
   Exhibit *e;

   e = data;
   ev = event;

   if(!strcmp(ev->key, "Tab"))
     {
	const char *path;
	char *dir;
	const char *file;
	Evas_List *l;
	
	path = etk_entry_text_get(ETK_ENTRY(e->entry[0]));
	dir = (const char*)ecore_file_get_dir((char*)path);
	file = ecore_file_get_file(path);
	
	if(!dir || !strcmp(dir, ""))
	  dir = strdup("/");
		
	if(!file || !strcmp(file, path))
	  return;	
	
	printf("dir='%s' file='%s'\n", dir, file);
	
	//if(strcmp(dir, e->cur_path))
	  {
	     DIR *dirfd;
	     struct dirent *dir_entry;
	     struct stat st;
	     	     
	     while(e->cur_tab->dirs)
	       {
		  e->cur_tab->dirs = evas_list_remove_list(e->cur_tab->dirs, e->cur_tab->dirs);
	       }
	     
	     if ((dirfd = opendir(dir)) == NULL)
	       return;
	     
	     while ((dir_entry = readdir(dirfd)) != NULL)
	       {
		  char fullpath[PATH_MAX];
		  
		  if (!strcmp(dir_entry->d_name, ".")||!strcmp(dir_entry->d_name, ".."))
		    continue;
		  
		  snprintf(fullpath, PATH_MAX, "%s/%s", dir, dir_entry->d_name);
		  if(stat(fullpath, &st) == -1) continue;

		  if(S_ISDIR(st.st_mode))
		    e->cur_tab->dirs = evas_list_append(e->cur_tab->dirs, strdup(dir_entry->d_name));
	       }
	     
	     closedir(dirfd);
	  }			
	
	for(l = e->cur_tab->dirs; l; l = l->next)
	  {
	     if(!strncmp(file, l->data, strlen(file)))
	       {
		  char fullpath[PATH_MAX];
		  snprintf(fullpath, PATH_MAX, "%s/%s/", dir, (char*)l->data);
		  etk_entry_text_set(ETK_ENTRY(e->entry[0]), fullpath);
		  break;
	       }	
	  }
	
	free(dir);
     }
   
   if(!strcmp(ev->key, "Return") || !strcmp(ev->key, "KP_Enter"))
     {
	_ex_slideshow_stop(e);
        e->cur_tab->dir = strdup((char*)etk_entry_text_get(ETK_ENTRY(e->entry[0])));
        etk_tree_clear(ETK_TREE(e->cur_tab->itree));
        etk_tree_clear(ETK_TREE(e->cur_tab->dtree));
        _ex_main_populate_files(e, NULL);
     }
}

void
_ex_main_goto_dir_clicked_cb(Etk_Object *object, void *data)
{
   Exhibit *e;

   e = data;

   e->cur_tab->dir = strdup((char*)etk_entry_text_get(ETK_ENTRY(e->entry[0])));
   etk_tree_clear(ETK_TREE(e->cur_tab->itree));
   etk_tree_clear(ETK_TREE(e->cur_tab->dtree));
   _ex_main_populate_files(e, NULL);
}

Etk_Bool
_ex_main_window_deleted_cb(void *data)
{
   Exhibit *e;

   e = data;
   etk_main_quit();
   return 1;
}

void
_ex_main_window_key_down_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Key_Up_Down *ev;
   Exhibit *e;
   Evas *evas;
   
   e = data;
   ev = event;
   evas = ecore_evas_get(ETK_WINDOW(e->win)->ecore_evas);
   
   if(evas_key_modifier_is_set(evas_key_modifier_get(evas), "Control"))
     {
	if(!strcmp(ev->key, "t"))
	  {
	     Ex_Tab *tab;
	     
	     tab = _ex_tab_new(e, e->cur_tab->cur_path);
	     _ex_main_window_tab_append(e, tab);
	     _ex_main_populate_files(e, NULL);
	  }
	else if(!strcmp(ev->key, "w"))
	  {
	     if(evas_list_count(e->tabs) > 1)
	       {
		  //_ex_main_window_tab_remove(e->cur_tab);
		  //_ex_tab_del(e->cur_tab);
	       }
	  }
	else if(!strcmp(ev->key, "q"))
	  {
	     etk_main_quit();
	  }
	else if(!strcmp(ev->key, "s"))
	  {
	     _ex_main_window_slideshow_toggle(e);
	  }
     }
}

void
_ex_main_window_slideshow_toggle(Exhibit *e)
{
   if(e->slideshow.active)
     {
	_ex_slideshow_stop(e);
     }
   else
     {
	_ex_slideshow_start(e);
     }
}

void 
_ex_main_window_tab_toggled_cb(Etk_Object *object, void *data)
{     
   Exhibit *e;
   Ex_Tab  *tab;
   
   e = data;
   _ex_slideshow_stop(e);
   tab = evas_list_nth(e->tabs, etk_notebook_current_page_get(ETK_NOTEBOOK(object)));

   e->cur_tab = tab;
   _ex_tab_select(tab);
   etk_entry_text_set(ETK_ENTRY(e->entry[0]), e->cur_tab->cur_path);   
}

void
_ex_main_window_tab_remove(Ex_Tab *tab)
{

}

void
_ex_main_window_tab_append(Exhibit *e, Ex_Tab *tab)
{
   if(evas_list_count(e->tabs) == 1)
     {
	/* adding first "real" tab, copy existing tab, and create new one */
	e->notebook = etk_notebook_new();        
	etk_paned_add2(ETK_PANED(e->hpaned), e->notebook, TRUE);	
	etk_notebook_page_append(ETK_NOTEBOOK(e->notebook), _ex_file_get(e->cur_tab->dir), e->cur_tab->scrolled_view);
	etk_signal_connect("current_page_changed", ETK_OBJECT(e->notebook), ETK_CALLBACK(_ex_main_window_tab_toggled_cb), e);
	etk_widget_show(ETK_WIDGET(e->notebook));
     }

   e->tabs = evas_list_append(e->tabs, tab);
   e->cur_tab = tab;
   etk_notebook_page_append(ETK_NOTEBOOK(e->notebook), _ex_file_get(e->cur_tab->dir), e->cur_tab->scrolled_view);
   etk_notebook_current_page_set(ETK_NOTEBOOK(e->notebook), evas_list_count(e->tabs) - 1);
}

void
_ex_main_window_show(char *dir)
{
   Exhibit *e;
   Ex_Tab  *tab;
   char    *file;

   e = calloc(1, sizeof(Exhibit));
   e->mouse.down = 0;
   e->menu = NULL;
   e->tabs = NULL;
   e->slideshow.active = FALSE;
   e->slideshow.interval = 5.0;
   
   file = NULL;
   tab = NULL;
   
   e->win = etk_window_new();
   etk_window_title_set(ETK_WINDOW(e->win), WINDOW_TITLE " - Image Viewing the Kewl Way!");
   etk_window_wmclass_set(ETK_WINDOW(e->win), "Exhibit", "Exhibit");
   etk_window_resize(ETK_WINDOW(e->win), WINDOW_WIDTH, WINDOW_HEIGHT);   
   etk_signal_connect("delete_event", ETK_OBJECT(e->win), ETK_CALLBACK(_ex_main_window_deleted_cb), e);
   etk_signal_connect("key_down", ETK_OBJECT(e->win), ETK_CALLBACK(_ex_main_window_key_down_cb), e);

   e->vbox = etk_vbox_new(FALSE, 0);
   etk_container_add(ETK_CONTAINER(e->win), e->vbox);
               
   e->hpaned = etk_hpaned_new();
   //etk_container_add(ETK_CONTAINER(e->win), e->hpaned);
   etk_box_pack_start(ETK_BOX(e->vbox), e->hpaned, TRUE, TRUE, 0);   

   e->vpaned = etk_vpaned_new();
   etk_paned_add1(ETK_PANED(e->hpaned), e->vpaned, FALSE);
   
   e->table = etk_table_new(4, 4, FALSE);
   etk_paned_add1(ETK_PANED(e->vpaned), e->table, FALSE);

   e->menu_bar = etk_menu_bar_new();
   etk_table_attach(ETK_TABLE(e->table), e->menu_bar, 0, 4, 0, 0, 0, 0, ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);
   
     {
	Etk_Widget *menu, *submenu;
	Etk_Widget *menu_item, *menu_item2;
	
	menu_item = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("File"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(e->menu_bar), NULL, NULL);
	menu = etk_menu_new();
	etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("New window"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_new_window_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Save image"), ETK_STOCK_DOCUMENT_SAVE, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_save_image_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Save image as"), ETK_STOCK_DOCUMENT_SAVE, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_save_image_as_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Search"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_search_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Rename"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_rename_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Delete"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_delete_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Close window"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_close_window_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Quit"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_quit_cb), e);
			
	menu_item = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Edit"), -99, ETK_MENU_SHELL(e->menu_bar), NULL, NULL);
	menu = etk_menu_new();
	etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));       	
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("in The Gimp"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_run_in_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("in XV"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_run_in_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("in Xpaint"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_run_in_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Rotate clockwise"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_rot_clockwise_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Rotate counterclockwise"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_rot_counter_clockwise_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Flip horizontally"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_flip_horizontal_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Flip vertically"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_flip_vertical_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Blur"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_blur_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sharpen"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_sharpen_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Brighten"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_brighten_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Set as wallpaper"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_set_wallpaper_cb), e);
	
	menu_item = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("View"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(e->menu_bar), NULL, NULL);
	menu = etk_menu_new();
	etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
	
	menu_item2 = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sort"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_run_in_cb), e);
	submenu = etk_menu_new();
	etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item2), ETK_MENU(submenu));
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Date"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(submenu), ETK_CALLBACK(_ex_sort_date_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Size"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(submenu), ETK_CALLBACK(_ex_sort_size_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Name"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(submenu), ETK_CALLBACK(_ex_sort_name_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Resolution"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(submenu), ETK_CALLBACK(_ex_sort_resol_cb), e);
	
	
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Zoom in"), EX_IMAGE_ZOOM_IN, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_zoom_in_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Zoom out"), EX_IMAGE_ZOOM_OUT, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_zoom_out_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Zoom 1:1"), EX_IMAGE_ONE_TO_ONE, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_zoom_one_to_one_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Fit to window"), EX_IMAGE_FIT_TO_WINDOW, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_fit_to_window_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Toggle slideshow"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_toggle_slideshow_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Refresh"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_refresh_cb), e);	
	
	menu_item = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Help"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(e->menu_bar), NULL, NULL);
	menu = etk_menu_new();
	etk_menu_item_submenu_set(ETK_MENU_ITEM(menu_item), ETK_MENU(menu));
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Release notes"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_release_notes_cb), e);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("About"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), ETK_CALLBACK(_ex_menu_about_cb), e);
     }   
   
   e->zoom_in[0] = etk_button_new();
   e->zoom_in[1] = etk_image_new_from_edje(PACKAGE_DATA_DIR"/images/images.edj", "zoom_in");
   etk_widget_pass_events_set(e->zoom_in[1], TRUE);   
   etk_widget_size_request_set(e->zoom_in[1], 33, 27);
   //etk_button_image_set(ETK_BUTTON(button), ETK_IMAGE(e->zoom_in[1]));
   etk_container_add(ETK_CONTAINER(e->zoom_in[0]), e->zoom_in[1]);
   etk_signal_connect("clicked", ETK_OBJECT(e->zoom_in[0]), ETK_CALLBACK(_ex_main_button_zoom_in_cb), e);
   etk_table_attach(ETK_TABLE(e->table), e->zoom_in[0],
		    0, 0, 1, 1,
		    0, 0, ETK_FILL_POLICY_NONE);

   e->zoom_out[0] = etk_button_new();
   e->zoom_out[1] = etk_image_new_from_edje(PACKAGE_DATA_DIR"/images/images.edj", "zoom_out");
   etk_widget_pass_events_set(e->zoom_out[1], TRUE);   
   etk_widget_size_request_set(e->zoom_out[1], 33, 27);
   //etk_button_image_set(ETK_BUTTON(e->zoom_out[0]), ETK_IMAGE(e->zoom_out[1]));
   etk_container_add(ETK_CONTAINER(e->zoom_out[0]), e->zoom_out[1]);
   etk_signal_connect("clicked", ETK_OBJECT(e->zoom_out[0]), ETK_CALLBACK(_ex_main_button_zoom_out_cb), e);   
   etk_table_attach(ETK_TABLE(e->table), e->zoom_out[0],
		    1, 1, 1, 1,
		    0, 0, ETK_FILL_POLICY_NONE);

   e->fit[0] = etk_button_new();
   e->fit[1] = etk_image_new_from_edje(PACKAGE_DATA_DIR"/images/images.edj", "fit_to_window");
   etk_widget_pass_events_set(e->fit[1], TRUE);   
   etk_widget_size_request_set(e->fit[1], 33, 27);
   //etk_button_image_set(ETK_BUTTON(button), ETK_IMAGE(e->fit[1]));
   etk_container_add(ETK_CONTAINER(e->fit[0]), e->fit[1]);
   etk_signal_connect("clicked", ETK_OBJECT(e->fit[0]), ETK_CALLBACK(_ex_main_button_fit_to_window_cb), e);   
   etk_table_attach(ETK_TABLE(e->table), e->fit[0],
		    2, 2, 1, 1,
		    0, 0, ETK_FILL_POLICY_NONE);

   e->original[0] = etk_button_new();
   e->original[1] = etk_image_new_from_edje(PACKAGE_DATA_DIR"/images/images.edj", "one_to_one");
   etk_widget_pass_events_set(e->original[1], TRUE);
   etk_widget_size_request_set(e->original[1], 33, 27);
   //etk_button_image_set(ETK_BUTTON(e->original[0]), ETK_IMAGE(e->original[1]));
   etk_container_add(ETK_CONTAINER(e->original[0]), e->original[1]);
   etk_signal_connect("clicked", ETK_OBJECT(e->original[0]), ETK_CALLBACK(_ex_main_button_zoom_one_to_one_cb), e);
   etk_table_attach(ETK_TABLE(e->table), e->original[0],
		    3, 3, 1, 1,
		    0, 0, ETK_FILL_POLICY_NONE);

   e->entry[0] = etk_entry_new();
   etk_object_properties_set(ETK_OBJECT(e->entry[0]), "can_pass_focus", FALSE, NULL);
   etk_table_attach(ETK_TABLE(e->table), e->entry[0], 0, 2, 2, 2, 0, 0, ETK_FILL_POLICY_HEXPAND | ETK_FILL_POLICY_HFILL);
   etk_signal_connect("key_down", ETK_OBJECT(e->entry[0]), ETK_CALLBACK(_ex_main_entry_dir_key_down_cb), e);

   e->entry[1] = etk_button_new_with_label("Go");
   etk_table_attach(ETK_TABLE(e->table), e->entry[1],
		    3, 3, 2, 2,
		    0, 0, ETK_FILL_POLICY_NONE);
   etk_signal_connect("clicked", ETK_OBJECT(e->entry[1]), ETK_CALLBACK(_ex_main_goto_dir_clicked_cb), e);
   
   /* create first tab but dont place it in notebook */
   if(dir)
     {
	if(ecore_file_is_dir(dir))	  	     
	  tab = _ex_tab_new(e, dir);   
	else if(ecore_file_exists(dir))
	  {	     
	     char *dir2;
	     
	     dir2 = ecore_file_get_dir(dir);
	     tab = _ex_tab_new(e, dir2);
	     free(dir2);
	     file = (const char*)ecore_file_get_file((const char*)dir);
	  }
	else     
	  tab = _ex_tab_new(e, ".");
     }
   else
     tab = _ex_tab_new(e, ".");
   
	
   e->cur_tab = tab;   
   e->tabs = evas_list_append(e->tabs, tab);   
   _ex_tab_select(tab);
   etk_paned_add2(ETK_PANED(tab->e->hpaned), tab->scrolled_view, TRUE);   
   _ex_main_populate_files(e, file);
      
   e->hbox = etk_hbox_new(TRUE, 0);   
   etk_box_pack_end(ETK_BOX(e->vbox), e->hbox, FALSE, FALSE, 0);

   e->sort = etk_label_new("Sort by date");
   /* size is different than statusbar, how do we make it look good? */
   //etk_box_pack_start(ETK_BOX(e->hbox), e->sort, TRUE, TRUE, 0);
     {
	Etk_Widget *menu;
	Etk_Widget *menu_item;

	menu = etk_menu_new();
	etk_signal_connect("mouse_down", ETK_OBJECT(e->sort), ETK_CALLBACK(_ex_sort_label_mouse_down_cb), menu);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sort by name"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sort by date"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sort by size"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	_ex_menu_item_new(EX_MENU_ITEM_SEPERATOR, NULL, ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);	
	_ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Ascending"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
	menu_item = _ex_menu_item_new(EX_MENU_ITEM_NORMAL, _("Sort"), ETK_STOCK_NO_STOCK, ETK_MENU_SHELL(menu), NULL, NULL);
     }
   
   e->statusbar[0] = etk_statusbar_new();
   etk_statusbar_has_resize_grip_set(ETK_STATUSBAR(e->statusbar[0]), FALSE);
   etk_box_pack_start(ETK_BOX(e->hbox), e->statusbar[0], TRUE, TRUE, 0);
   
   e->statusbar[1] = etk_statusbar_new();
   etk_statusbar_has_resize_grip_set(ETK_STATUSBAR(e->statusbar[1]), FALSE);
   etk_box_pack_start(ETK_BOX(e->hbox), e->statusbar[1], TRUE, TRUE, 0);

   e->statusbar[2] = etk_statusbar_new();
   etk_statusbar_has_resize_grip_set(ETK_STATUSBAR(e->statusbar[2]), FALSE);   
   etk_box_pack_start(ETK_BOX(e->hbox), e->statusbar[2], TRUE, TRUE, 0);   
   etk_statusbar_push(ETK_STATUSBAR(e->statusbar[2]), "1:1", 0);
   
   e->statusbar[3] = etk_statusbar_new();
   etk_box_pack_start(ETK_BOX(e->hbox), e->statusbar[3], FALSE, FALSE, 0);
   
   etk_widget_show_all(e->win);
}

int
main(int argc, char *argv[])
{
   if (!etk_init())
     {
	fprintf(stderr, "Could not init etk. Exiting...\n");
	return 0;
     };

   event_handlers = evas_list_append(event_handlers,
				     ecore_event_handler_add(ECORE_EVENT_EXE_EXIT,
							     _ex_thumb_exe_exit,
							     NULL));
   
   epsilon_init();
   if(argc > 1)
     _ex_main_window_show(argv[1]);     
   else
     _ex_main_window_show(NULL);   
     
   etk_main();
   etk_shutdown();

   return 1;
}
