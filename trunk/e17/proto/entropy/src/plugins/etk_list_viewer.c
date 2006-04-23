#include "entropy.h"
#include "entropy_gui.h"
#include <dlfcn.h>
#include <strings.h>
#include <limits.h>
#include <time.h>
#include <Etk.h>
#include "entropy_etk_context_menu.h"
#include "etk_properties_dialog.h"

#define EN_DND_COL_NUM 5

static int etk_callback_setup = 0;
static Ecore_Hash* etk_list_viewer_row_hash;

typedef struct entropy_etk_file_list_viewer entropy_etk_file_list_viewer;
struct entropy_etk_file_list_viewer
{
  entropy_core *ecore;		/*A reference to the core object passed from init */
  //Etk_Row *current_row;
  Etk_Widget *tree;
  Etk_Tree_Col* tree_col1;
  Etk_Widget* parent_visual; 

  Ecore_Hash* etk_list_viewer_row_hash;
  Ecore_Hash* gui_hash;

  Ecore_List *gui_events;
  Ecore_List *files;		/*The entropy_generic_file references we copy. */

  /*Current folder - TODO - move to core - per layout API*/
  entropy_generic_file* current_folder;

};

typedef struct event_file_core event_file_core;
struct event_file_core
{
  entropy_generic_file *file;
  entropy_gui_component_instance *instance;
  void *data;
};

typedef struct gui_file gui_file;
struct gui_file
{
  entropy_generic_file *file;
  entropy_thumbnail *thumbnail;
  entropy_gui_component_instance *instance;
  Etk_Tree_Row *icon;
};


/*Headers*/
void
gui_event_callback (entropy_notify_event * eevent, void *requestor,
		    void *el, entropy_gui_component_instance * comp);
void gui_file_destroy (gui_file * file);
int entropy_plugin_type_get ();

/*-------------*/


void
gui_file_destroy (gui_file * file)
{
  entropy_free (file);

}


int
entropy_plugin_type_get ()
{
  return ENTROPY_PLUGIN_GUI_COMPONENT;
}

int
entropy_plugin_sub_type_get ()
{
  return ENTROPY_PLUGIN_GUI_COMPONENT_LOCAL_VIEW;
}

char *
entropy_plugin_identify ()
{
  return (char *) "listviewer";
}

char*
entropy_plugin_toolkit_get() 
{
	return ENTROPY_TOOLKIT_ETK;
}



/* Compares two rows of the tree */
static int _entropy_etk_list_type_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   gui_file *file1, *file2;
   int val;
   
   if (!tree || !row1 || !row2 || !col)
      return 0;

   file1 = ecore_hash_get(etk_list_viewer_row_hash, row1);
   file2 = ecore_hash_get(etk_list_viewer_row_hash, row2);
  
   if (file1 && file2) {
	 val = strcasecmp(file1->file->mime_type, file2->file->mime_type);
	 return val;
   } else {
	   printf("Could not locate file!\n");
	   return 0;
   }
}

static int _entropy_etk_list_filename_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   gui_file *file1, *file2;
   int val;
   
   if (!tree || !row1 || !row2 || !col)
      return 0;

   file1 = ecore_hash_get(etk_list_viewer_row_hash, row1);
   file2 = ecore_hash_get(etk_list_viewer_row_hash, row2);
  
   if (file1 && file2) {
	 val = strcasecmp(file1->file->filename, file2->file->filename);
	 
	 if ( !strcmp(file1->file->mime_type, "file/folder") && strcmp(file2->file->mime_type, "file/folder"))
		 return -1;
	 else if (!strcmp(file2->file->mime_type, "file/folder") && strcmp(file1->file->mime_type, "file/folder"))
		 return 1;
	 else 
		 return val;
   } else {
	   printf("Could not locate file!\n");
	   return 0;
   }
}

/* Compares two rows of the tree */
static int _entropy_etk_list_size_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   gui_file *file1, *file2;
   
   if (!tree || !row1 || !row2 || !col)
      return 0;
   
   file1 = ecore_hash_get(etk_list_viewer_row_hash, row1);
   file2 = ecore_hash_get(etk_list_viewer_row_hash, row2);
   
   if (file1 && file2) {
	   if (file1->file->properties.st_size > file2->file->properties.st_size) {
		   return 1;
	   } else if (file1->file->properties.st_size < file2->file->properties.st_size) {
		   return -1;
	   } else return 0;
   } else {
	   printf("Could not locate file!\n");
	   return 0;
   }
}

/* Compares two rows of the tree */
static int _entropy_etk_list_date_compare_cb(Etk_Tree *tree, Etk_Tree_Row *row1, Etk_Tree_Row *row2, Etk_Tree_Col *col, void *data)
{
   gui_file *file1, *file2;
   
   if (!tree || !row1 || !row2 || !col)
      return 0;
   
   file1 = ecore_hash_get(etk_list_viewer_row_hash, row1);
   file2 = ecore_hash_get(etk_list_viewer_row_hash, row2);
   
   if (file1 && file2) {
	   if (file1->file->properties.st_mtime > file2->file->properties.st_mtime) {
		   return 1;
	   } else if (file1->file->properties.st_mtime < file2->file->properties.st_mtime) {
		   return -1;
	   } else return 0;
   } else {
	   printf("Could not locate file!\n");
	   return 0;
   }
}

/* Called when the user presses a key */
static void _etk_entropy_list_viewer_key_down_cb(Etk_Object *object, void *event, void *data)
{
   Etk_Event_Key_Up_Down *key_event = event;

   Etk_Tree* tree;
   Evas_List* row_list;
   gui_file* file;

   /*entropy_gui_component_instance* instance;
   entropy_etk_file_list_viewer* viewer;*/
	
   tree = ETK_TREE(object);
   row_list = etk_tree_selected_rows_get(tree);


   if (!strcmp(key_event->key, "Delete")) {
	   printf("Delete pressed!\n");

	  for (; row_list; row_list = row_list->next ) {
	  	file = ((gui_file*)ecore_hash_get(etk_list_viewer_row_hash, row_list->data));

		if (file) {
			printf("Deleting '%s'...\n", file->file->filename);
			entropy_plugin_filesystem_file_remove(file->file, (entropy_gui_component_instance*)data);
		}

	  }

   }

}


static void _entropy_etk_list_viewer_drag_begin_cb(Etk_Object *object, void *data)
{
   Etk_Tree *tree;
   const char **types;
   unsigned int num_types;
   Etk_Drag *drag;
   Etk_Widget *image;
   entropy_gui_component_instance* instance;
   entropy_etk_file_list_viewer* viewer;
   char buffer[8192]; /* Um - help - what do we size this to? */
   int count = 0;
   Evas_List* rows;
   Etk_Widget* table;
   int l=0,r=0,t=0,b=0;
   int added_object = 0;
   gui_file* file;
   Etk_Widget* vbox;
   Etk_Widget* label;
   char label_buffer[50];

   instance = data;
   viewer = instance->data;

   tree = ETK_TREE(object);
   rows = etk_tree_selected_rows_get(tree);
   
   drag = (ETK_WIDGET(tree))->drag;

   table = etk_table_new(5,5,ETK_FALSE);
   count = evas_list_count(rows);
   bzero(buffer,8192);
   for (; rows; rows = rows->next ) {
	   file = ((gui_file*)ecore_hash_get(etk_list_viewer_row_hash, rows->data));
	   
	   printf("Row %p resolves to %p:%s!\n", rows->data, ecore_hash_get(etk_list_viewer_row_hash, rows->data),
			   ((gui_file*)ecore_hash_get(etk_list_viewer_row_hash, rows->data))->file->uri );
	   strcat(buffer, ((gui_file*)ecore_hash_get(etk_list_viewer_row_hash, rows->data))->file->uri);
	   strcat(buffer, "\r\n");

	   if (added_object < (EN_DND_COL_NUM*5)-1) {
		   /*Build the drag widget*/
		   vbox = etk_vbox_new(ETK_TRUE,0);

		   /*Print the label*/
		   bzero(label_buffer, sizeof(label_buffer));

		   if (strlen(file->file->filename) > 5) {
			   snprintf(label_buffer,5,"%s", file->file->filename);
			   strcat(label_buffer, "...");
		   } else {
			   sprintf(label_buffer,"%s", file->file->filename);
		   }
   		   label = etk_label_new(label_buffer);
		   
		  if (file->file->thumbnail && file->file->thumbnail->thumbnail_filename) {
			image = etk_image_new_from_file(file->file->thumbnail->thumbnail_filename);
		  } else {
			image = etk_image_new_from_file(PACKAGE_DATA_DIR "/icons/default.png");
		  }
		 etk_image_keep_aspect_set(ETK_IMAGE(image), ETK_TRUE);
		 etk_widget_size_request_set(image, 48, 48);
		 etk_box_pack_start(ETK_BOX(vbox), image, ETK_FALSE, ETK_FALSE, 0);
		  
		  etk_box_pack_start(ETK_BOX(vbox), label, ETK_FALSE, ETK_FALSE, 0);
		  

		  etk_table_attach(ETK_TABLE(table), vbox, l, r, t, b, 3, 3,
			   ETK_FILL_POLICY_NONE);
		  
		  ++l; ++r;
		  added_object++;
		  if(l == EN_DND_COL_NUM) {
		       l = r = 0;
		       ++t; ++b;
		    }	 
	  }
	  
   }

   etk_container_add(ETK_CONTAINER(drag), table);
  

   types = entropy_malloc(sizeof(char*));
   num_types = 1;
   types[0] = strdup("text/uri-list");
    

   printf("Drag buffer: %s\n", buffer);
   
   etk_drag_types_set(drag, types, num_types);
   etk_drag_data_set(drag, buffer, strlen(buffer)+1);



   
   /*image = etk_image_new_from_file(icol1_string);
   etk_image_keep_aspect_set(ETK_IMAGE(image), ETK_TRUE);
   etk_widget_size_request_set(image, 96, 96);
   etk_container_add(ETK_CONTAINER(drag), image);*/

   evas_list_free(rows);
}


void
gui_file_remove_destroy_single(entropy_gui_component_instance * comp,
		gui_file* file)
{
	entropy_etk_file_list_viewer *view = comp->data;
	
	ecore_hash_remove(view->gui_hash, file->file);
	ecore_hash_remove(etk_list_viewer_row_hash, file->icon);

	entropy_free(file);
	
}

Ecore_List* 
gui_object_destroy_and_free (entropy_gui_component_instance * comp,
			     Ecore_Hash * gui_hash)
{

  Ecore_List *list;
  Ecore_List *file_remove_ref_list;
  entropy_generic_file *obj;
  gui_file *freeobj;
  Etk_Tree_Row* row;
  entropy_etk_file_list_viewer *view = comp->data;


  file_remove_ref_list = ecore_list_new();
  
  /*Temporarily stop callbacks, we don't want to clobber an in-op process */
  entropy_notify_lock_loop (comp->core->notify);

  list = ecore_hash_keys (gui_hash);

  ecore_list_goto_first (list);
  while ((obj = ecore_list_next (list))) {


    freeobj = ecore_hash_get (gui_hash, obj);
    if (freeobj) {
      /*De-Associate this icon with this file in the core, so DND works */
      entropy_core_object_file_disassociate (freeobj->icon);

      gui_file_destroy (freeobj);
    }

    /*Tell the core we no longer need this file - it might free it now */
    ecore_list_append(file_remove_ref_list, obj->md5);
  }
  ecore_hash_destroy (gui_hash);
  view->gui_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
  ecore_list_destroy (list);


  ecore_list_goto_first(view->files);
  while ((row = ecore_list_remove_first(view->files))) {
	  ecore_hash_remove(etk_list_viewer_row_hash, row);
  }

  entropy_notify_unlock_loop (comp->core->notify);


  return file_remove_ref_list;

}


static void _etk_list_viewer_row_clicked(Etk_Object *object, Etk_Tree_Row *row, Etk_Event_Mouse_Up_Down *event, void *data)
{
   entropy_gui_component_instance* instance;
   entropy_etk_file_list_viewer* viewer;
   entropy_gui_event *gui_event;
   gui_file* file;
   
  
   file = ecore_hash_get(etk_list_viewer_row_hash, row);
   instance = file->instance;
   viewer = instance->data;
	

	  
   if (event->flags & EVAS_BUTTON_DOUBLE_CLICK && event->button == 1) {
	   printf("Row clicked, file is: %s\n", file->file->filename); 

	  gui_event = entropy_malloc (sizeof (entropy_gui_event));
	  gui_event->event_type =
	    entropy_core_gui_event_get (ENTROPY_GUI_EVENT_ACTION_FILE);
	  gui_event->data = file->file;
	  entropy_core_layout_notify_event (file->instance, gui_event, ENTROPY_EVENT_GLOBAL);
   } else if (event->button == 2) {
	etk_tree_row_select(row);
	printf("MetaData request\n");

	  gui_event = entropy_malloc (sizeof (entropy_gui_event));
	  gui_event->event_type =
	    entropy_core_gui_event_get (ENTROPY_GUI_EVENT_FILE_METADATA);
	  gui_event->data = file->file;
	  entropy_core_layout_notify_event (file->instance, gui_event, ENTROPY_EVENT_GLOBAL);
   } else if (event->button == 3) {
	etk_tree_row_select(row);

	file = ecore_hash_get(etk_list_viewer_row_hash, row);

	entropy_etk_context_menu_popup(instance, file->file);
   }
}


void
list_viewer_remove_row(entropy_gui_component_instance* instance,
		entropy_generic_file* file)
{
	entropy_etk_file_list_viewer* viewer = instance->data;
	gui_file* event_file = NULL;

	event_file = ecore_hash_get(viewer->gui_hash,file);

	etk_tree_row_del(event_file->icon);

	/*Destroy the gui_file object..*/
	gui_file_remove_destroy_single(instance,event_file);
	
}

void
list_viewer_add_row (entropy_gui_component_instance * instance,
			  entropy_generic_file * file)
{
  Etk_Tree_Row* new_row;
  entropy_etk_file_list_viewer* viewer;
  gui_file *e_file = NULL;
  entropy_gui_event *gui_event;
  Etk_Tree_Col* col1;
  Etk_Tree_Col* col2;
  Etk_Tree_Col* col3;
  Etk_Tree_Col* col4;
  Etk_Tree_Col* col5;
  char buffer[50];
  char date_buffer[26];
  char* thumbnail_filename;


  viewer = instance->data;


  entropy_core_file_cache_add_reference (file->md5);


  if (!strlen (file->mime_type)) {
	entropy_mime_file_identify (file);
  }

  if (!file->thumbnail) {
	  entropy_plugin_thumbnail_request(instance, file, (void*)gui_event_callback); 
	  thumbnail_filename= PACKAGE_DATA_DIR "/icons/default.png"; 
  } else {
	  thumbnail_filename = file->thumbnail->thumbnail_filename;
  }
  
  col1 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 0);
  col2 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 1);
  col3 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 2);
  col4 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 3);
  col5 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 4);
  
  
  etk_tree_freeze(ETK_TREE(viewer->tree));
  
  if (!file->retrieved_stat) {
	  new_row = etk_tree_append(ETK_TREE(viewer->tree), 
		  col1, thumbnail_filename, 
		  col2,   file->filename, 
		  col4, file->mime_type,
		  NULL);

  } else {
	 // time_t stime = file->properties.st_mtime
	  
	  snprintf(buffer,50, "%lld Kb", file->properties.st_size / 1024);
	  ctime_r(&file->properties.st_mtime, date_buffer);
	  date_buffer[strlen(date_buffer)-1] = '\0';
	  
	  new_row = etk_tree_append(ETK_TREE(viewer->tree), 
		  col1, thumbnail_filename, 
		  col2,   file->filename,
		  col3,   buffer,
		  col4,   file->mime_type,
		  col5,   date_buffer,
		  NULL);
	  
  }

  e_file = entropy_malloc(sizeof(gui_file));
  e_file->file = file;		/*Create a clone of this file, and add it to the event */
  e_file->instance = instance;
  e_file->icon=new_row;

  ecore_hash_set(viewer->gui_hash, file, e_file);
  ecore_hash_set(etk_list_viewer_row_hash, new_row, e_file);

  /*Save this file in this list of files we're responsible for */
  ecore_list_append (viewer->files, new_row);


  if (!file->retrieved_stat) {
	  /*And request the properties...*/
	  gui_event = entropy_malloc (sizeof (entropy_gui_event));
	  gui_event->event_type =
	  entropy_core_gui_event_get (ENTROPY_GUI_EVENT_FILE_STAT);
	  gui_event->data = file;
	   entropy_core_layout_notify_event (instance, gui_event,
				      ENTROPY_EVENT_LOCAL);
  }

  etk_tree_thaw(ETK_TREE(viewer->tree));
}



void
gui_event_callback (entropy_notify_event * eevent, void *requestor,
		    void *el, entropy_gui_component_instance * comp)
{
  entropy_etk_file_list_viewer *viewer =
    (entropy_etk_file_list_viewer *) comp->data;

  switch (eevent->event_type) {
  	  case ENTROPY_NOTIFY_FILELIST_REQUEST_EXTERNAL:
	  case ENTROPY_NOTIFY_FILELIST_REQUEST:{
	      entropy_generic_file *file;
	      Ecore_List* remove_ref;
	      char* ref;

	      entropy_generic_file *event_file =
		((entropy_file_request *) eevent->data)->file;

	      viewer->current_folder = event_file;

	      remove_ref = gui_object_destroy_and_free(comp, viewer->gui_hash);

	      printf("Clearing tree..\n");
	      etk_tree_clear(ETK_TREE(viewer->tree));

		ecore_list_goto_first (el);
		while ((file = ecore_list_next (el))) {

		  /*We need the file's mime type, 
		   * so get it here if it's not here already...*/
		  
		      list_viewer_add_row (comp, file);
		}

		while ( (ref = ecore_list_remove_first(remove_ref)))  {
			entropy_core_file_cache_remove_reference (ref);
		}
		ecore_list_destroy(remove_ref);


	      }
	      break;

     case ENTROPY_NOTIFY_FILE_STAT_AVAILABLE:{

	/*We have two cases here: 1. Properties are coming back destined to the list, or
	 * 			  2. Properties for a properties dialog. 
	 * 			  Look at the hash to figure out which one */

	entropy_file_stat *file_stat = (entropy_file_stat *) eevent->return_struct;	
	gui_file* obj = ecore_hash_get (viewer->gui_hash, file_stat->file);
	char buffer[50];
	char date_buffer[26];

	Etk_Tree_Col* col1;
	Etk_Tree_Col* col2;
	Etk_Tree_Col* col3;
	Etk_Tree_Col* col4;
	Etk_Tree_Col* col5;
	
	/*If !obj, it has been deleted - fail silently*/
	if (obj) {
		
			col1 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 0);
			col2 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 1);
			col3 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 2);
			col4 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 3);
			col5 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 4);
		
			snprintf(buffer,50, "%lld Kb", file_stat->stat_obj->st_size / 1024);
			ctime_r(&file_stat->stat_obj->st_mtime, date_buffer);
			date_buffer[strlen(date_buffer)-1] = '\0';
	
			etk_tree_freeze(ETK_TREE(viewer->tree));
			etk_tree_row_fields_set((Etk_Tree_Row*)obj->icon, 
					col3, buffer,
					col5, date_buffer,
					NULL);
			etk_tree_thaw(ETK_TREE(viewer->tree));
	}
     }
     break;					 

    case ENTROPY_NOTIFY_FILE_CHANGE: {
	  entropy_generic_file* file = el;
	  entropy_gui_event *gui_event = NULL;
	  
	  /*And request the properties...*/
	  gui_event = entropy_malloc (sizeof (entropy_gui_event));
	  gui_event->event_type =
	  entropy_core_gui_event_get (ENTROPY_GUI_EVENT_FILE_STAT);
	  gui_event->data = file;
	   entropy_core_layout_notify_event (comp, gui_event,
				      ENTROPY_EVENT_LOCAL);
    }
    break;

     case ENTROPY_NOTIFY_FILE_CREATE:{
      entropy_generic_file* file = el;
      
      /*Check that this file is the current dir we are displaying*/
      entropy_generic_file* parent_folder = entropy_core_parent_folder_file_get(file);

      if (parent_folder && parent_folder == viewer->current_folder) {
	      list_viewer_add_row (comp, file);				      
      }
     }
     break;	  

     case ENTROPY_NOTIFY_FILE_REMOVE_DIRECTORY:
     case ENTROPY_NOTIFY_FILE_REMOVE:{
 	    list_viewer_remove_row(comp, (entropy_generic_file *) el);
     }
     break;

     case ENTROPY_NOTIFY_THUMBNAIL_REQUEST:{

   	   /*Only bother if we have a thumbnail, and a component */
	      if (el && comp) {
		gui_file *obj;
		entropy_thumbnail *thumb = (entropy_thumbnail *) el;
		entropy_etk_file_list_viewer *view = comp->data;
	
		obj = ecore_hash_get (view->gui_hash, thumb->parent);

		if (obj) {
		  Etk_Tree_Col* col1;
		  obj->thumbnail = thumb;

		  col1 = etk_tree_nth_col_get(ETK_TREE(viewer->tree), 0);
		  etk_tree_freeze(ETK_TREE(viewer->tree));

		  etk_tree_row_fields_set((Etk_Tree_Row*)obj->icon, 
		  col1, obj->thumbnail->thumbnail_filename, 
		  NULL);

		  etk_tree_thaw(ETK_TREE(viewer->tree));

		} else {
		  /*printf ("ERR: Couldn't find a hash reference for this file!\n");*/
		}

	      }
	    }				//End case
	    break;					    
	      
  }

}


Entropy_Plugin*
entropy_plugin_init (entropy_core * core)
{
  Entropy_Plugin_Gui* plugin;
  Entropy_Plugin* base;
	
  plugin = entropy_malloc(sizeof(Entropy_Plugin_Gui));
  base = ENTROPY_PLUGIN(plugin);
  
  return base;
}


entropy_gui_component_instance *
entropy_plugin_gui_instance_new (entropy_core * core,
		     entropy_gui_component_instance * layout, void *data)
{	
  entropy_gui_component_instance *instance;	
  entropy_etk_file_list_viewer *viewer;
  char  **dnd_types;
  int dnd_types_num=0;
  Etk_Widget* new_menu;
  Etk_Widget* menu_item;

    
  instance = entropy_gui_component_instance_new ();
  viewer = entropy_malloc (sizeof (entropy_etk_file_list_viewer));

  viewer->files = ecore_list_new();
  viewer->gui_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
  
  viewer->tree = etk_tree_new(); 
  etk_tree_mode_set(ETK_TREE(viewer->tree), ETK_TREE_MODE_LIST);
 
  viewer->tree_col1 = etk_tree_col_new(ETK_TREE(viewer->tree), _("Icon"), 
		  etk_tree_model_image_new(ETK_TREE(viewer->tree), ETK_TREE_FROM_FILE), 48);
  viewer->tree_col1 = etk_tree_col_new(ETK_TREE(viewer->tree), _("Filename"), 
		  etk_tree_model_text_new(ETK_TREE(viewer->tree)), 150);
  etk_tree_col_sort_func_set(viewer->tree_col1, _entropy_etk_list_filename_compare_cb, NULL);

  viewer->tree_col1 = etk_tree_col_new(ETK_TREE(viewer->tree), _("Size"), 
		  etk_tree_model_text_new(ETK_TREE(viewer->tree)),60);
  etk_tree_col_sort_func_set(viewer->tree_col1, _entropy_etk_list_size_compare_cb, NULL);

  viewer->tree_col1 = etk_tree_col_new(ETK_TREE(viewer->tree), _("Type"), 
		  etk_tree_model_text_new(ETK_TREE(viewer->tree)),65);
  etk_tree_col_sort_func_set(viewer->tree_col1, _entropy_etk_list_type_compare_cb, NULL);

  viewer->tree_col1 = etk_tree_col_new(ETK_TREE(viewer->tree), _("Date Modified"), 
		  etk_tree_model_text_new(ETK_TREE(viewer->tree)),90);
  etk_tree_col_sort_func_set(viewer->tree_col1, _entropy_etk_list_date_compare_cb, NULL);


  /*DND Setup*/
   dnd_types_num = 1;
   dnd_types = entropy_malloc(dnd_types_num* sizeof(char*));
   dnd_types[0] = strdup("text/uri-list");  
  etk_widget_dnd_source_set(viewer->tree, ETK_TRUE);
  etk_signal_connect("drag_begin", ETK_OBJECT(viewer->tree) , ETK_CALLBACK(_entropy_etk_list_viewer_drag_begin_cb), instance);
  etk_tree_multiple_select_set(ETK_TREE(viewer->tree), ETK_TRUE); 
  etk_tree_build(ETK_TREE(viewer->tree));

  etk_widget_size_request_set(viewer->tree, 600, 600);

  instance->data = viewer;
  instance->core = core;
  instance->gui_object = viewer->tree;

  instance->layout_parent = layout;

  /*Register out interest in receiving folder notifications */
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS));
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS_EXTERNAL));

  /*Register our interest in receiving file mod/create/delete notifications */
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_CHANGE));
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_CREATE));
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_REMOVE));
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_REMOVE_DIRECTORY));

  /*Register interest in getting stat events */
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_STAT));
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_FILE_STAT_AVAILABLE));

  /*We want to know about thumbnail available events */
  entropy_core_component_event_register (instance,
					 entropy_core_gui_event_get
					 (ENTROPY_GUI_EVENT_THUMBNAIL_AVAILABLE));


  
  if (!etk_callback_setup) {
	  etk_callback_setup = 1;
	  etk_list_viewer_row_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
  }

  etk_signal_connect("row_clicked", ETK_OBJECT( viewer->tree  ), 
		  ETK_CALLBACK(_etk_list_viewer_row_clicked), NULL);

  etk_signal_connect("key_down", ETK_OBJECT(viewer->tree), 
		  ETK_CALLBACK(_etk_entropy_list_viewer_key_down_cb), instance);
  
  printf("Initialising ETK list viewer...%p\n", instance);

  return instance;

}

