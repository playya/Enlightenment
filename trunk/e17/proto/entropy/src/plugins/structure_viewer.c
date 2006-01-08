#include <Ewl.h>
#include "entropy.h"
#include "entropy_gui.h"
#include <dlfcn.h>
#include <limits.h>

void dnd_leave_callback(Ewl_Widget *main_win, void *ev_data, void *user_data);
void dnd_enter_callback(Ewl_Widget *main_win, void *ev_data, void *user_data);
void dnd_drop_callback(Ewl_Widget* w, void* ev_data, void* user_data);



typedef struct entropy_file_structure_viewer entropy_file_structure_viewer;
struct entropy_file_structure_viewer {
	entropy_core* ecore; /*A reference to the core object passed from init */
	Ewl_Row* current_row;
	Ewl_Widget* tree;

	Ecore_List* gui_events;
	Ecore_List* files; /*The entropy_generic_file references we copy.*/

	Ecore_Hash* loaded_dirs; /*A hash of the directories we have already loaded directories for - mostly for cleanup*/
	Ecore_Hash* row_folder_hash;

	Ewl_Widget* last_selected_label;
};

typedef struct event_file_core event_file_core;
struct event_file_core {
	entropy_generic_file* file;
	entropy_gui_component_instance* instance;
	void* data;
};

void structure_viewer_add_row(entropy_gui_component_instance* instance, 
	entropy_generic_file* file, Ewl_Row* prow);

int entropy_plugin_type_get() {
        return ENTROPY_PLUGIN_GUI_COMPONENT;
}

int entropy_plugin_sub_type_get() {
	return ENTROPY_PLUGIN_GUI_COMPONENT_STRUCTURE_VIEW;
}

char* entropy_plugin_identify() {
	        return (char*)"File system tree structure viewer";
}

/*Ewl_Widget* entropy_plugin_gui_component_visual_get() {
	return itree;
}*/

void gui_event_callback(entropy_notify_event* eevent, void* requestor, 
	void* el, entropy_gui_component_instance* comp) {
	
   entropy_file_structure_viewer* viewer = 
	(entropy_file_structure_viewer*)comp->data;

   switch (eevent->event_type) {
       case ENTROPY_NOTIFY_FILE_REMOVE_DIRECTORY: {
		entropy_generic_file* event_file = (entropy_generic_file*)el;
       
		Ewl_Row* row = ecore_hash_get(viewer->row_folder_hash, 
			event_file);

		if (row) {
			ewl_tree_row_destroy(EWL_TREE(viewer->tree), row);
		}
	}
	break;
   
      case ENTROPY_NOTIFY_FILELIST_REQUEST_EXTERNAL:
      case ENTROPY_NOTIFY_FILELIST_REQUEST: {
							  
	entropy_generic_file* file;
	entropy_generic_file* event_file = 
		((entropy_file_request*)eevent->data)->file;
	
	Ewl_Row* row = ecore_hash_get(viewer->row_folder_hash, event_file);

	if (row) viewer->current_row = row;
	
	/*If we don't own this row, forget about doing something 
	 * - we don't know about this*/
	if (row && !ecore_hash_get(viewer->loaded_dirs, row)) {


			ecore_list_goto_first(el);
			while ( (file = ecore_list_next(el)) ) {

				/*We need the file's mime type, 
				 * so get it here if it's not here already...*/
				if (!strlen(file->mime_type)) {
					entropy_mime_file_identify(
					comp->core->mime_plugins, file);
				}
				
				if (file->filetype == FILE_FOLDER || 
				  entropy_core_descent_for_mime_get(comp->core, 
				  file->mime_type)  ) {
					char *c = entropy_malloc(sizeof(char));
					*c = 1;

					/*Tell the core we're watching 
					 * this file*/
					entropy_core_file_cache_add_reference(
						file->md5);
					structure_viewer_add_row(comp, file, 
						row);
					ecore_hash_set(viewer->loaded_dirs, 
						row, c);
				}
			}
		/*ecore_list_destroy(el);*/

		ewl_tree_row_expand_set(row, EWL_TREE_NODE_EXPANDED);
		/*Highlight this row*/
		/*TODO Find some way to cleanly find the 
		 * text member of the row*/
	} else {
		/*printf("-> This row already has children!!\n");*/
	}
      }
      break;
   }

}


void dnd_enter_callback(Ewl_Widget *main_win, void *ev_data, void *user_data) {
	event_file_core* event = (event_file_core*)user_data;

	ewl_text_cursor_position_set(EWL_TEXT(event->data), 0);
	ewl_text_color_apply(EWL_TEXT(event->data), 255, 0, 0, 255, ewl_text_length_get(EWL_TEXT(event->data)));

	
	//printf("Entered text %p\n", main_win);
}

void dnd_leave_callback(Ewl_Widget *main_win, void *ev_data, void *user_data) {
	event_file_core* event = (event_file_core*)user_data;

	ewl_text_cursor_position_set(EWL_TEXT(event->data), 0);
	ewl_text_color_apply(EWL_TEXT(event->data), 0, 0, 0, 255, ewl_text_length_get(EWL_TEXT(event->data)));

	
	//printf("Left text %p\n", main_win);
}

void dnd_drop_callback(Ewl_Widget* w, void* ev_data, void* user_data) {
	Ewl_Widget* widget = ewl_dnd_drag_widget_get();
	event_file_core* event = (event_file_core*)user_data;


	if (widget) {
		printf("Drop widget: '%s'\n", widget->inheritance);
		if (ewl_widget_type_is(widget, "icon")) {
			Ewl_Iconbox* iconbox = EWL_ICONBOX_ICON(widget)->icon_box_parent;
			Ecore_List* sel_list = ewl_iconbox_get_selection(iconbox);
			Ewl_Iconbox_Icon* icon;
			entropy_generic_file* file;
			char dest_dir[PATH_MAX];

			entropy_plugin* plugin = 
				entropy_plugins_type_get_first(ENTROPY_PLUGIN_BACKEND_FILE ,ENTROPY_PLUGIN_SUB_TYPE_ALL);
			void (*copy_func)(entropy_generic_file* source, char* dest_uri, entropy_gui_component_instance* requester);
			copy_func = dlsym(plugin->dl_ref, "entropy_filesystem_file_copy");

			snprintf(dest_dir, PATH_MAX, "%s://%s/%s", event->file->uri_base, event->file->path, event->file->filename);
			

			ecore_list_goto_first(sel_list);
			while ( (icon = ecore_list_remove_first(sel_list))) {
				if ( (file = entropy_core_object_file_association_get(icon))) {
					printf("Filename: '%s' - '%s/%s'\n", file->uri_base, file->path, file->filename);
					(*copy_func)(file, dest_dir, event->instance);	
				}
			}

		} else if (ewl_widget_type_is(widget, "row")) {
			entropy_generic_file* file = 
			 entropy_core_object_file_association_get(widget);

			entropy_plugin* plugin = 
				entropy_plugins_type_get_first(
				ENTROPY_PLUGIN_BACKEND_FILE ,
				ENTROPY_PLUGIN_SUB_TYPE_ALL);
			
			void (*copy_func)(entropy_generic_file* source, 
			char* dest_uri, 
			entropy_gui_component_instance* requester);

			copy_func = dlsym(plugin->dl_ref, 
				"entropy_filesystem_file_copy");
			
			char* folder;
			
			if (file) {
				printf("Detected row drop.. (%s/%s)\n",
					file->path, file->filename);
				
				folder = entropy_core_generic_file_uri_create(
					event->file, 0);

				(*copy_func)(file, folder, event->instance);

				free(folder);
				
			}

		}
	}
}



void row_clicked_callback(Ewl_Widget *main_win, void *ev_data, void *user_data) {
	event_file_core* event = (event_file_core*)user_data;
	entropy_file_structure_viewer* viewer = (entropy_file_structure_viewer*)event->instance->data;
	entropy_gui_event* gui_event;

	if (!ewl_widget_type_is(main_win, "row") || EWL_WIDGET(viewer->current_row) == main_win) return;
	viewer->current_row = EWL_ROW(main_win);

	/*-----------*/
	/*Send an event to the core*/
	
	gui_event = entropy_malloc(sizeof(entropy_gui_event));
	gui_event->event_type = entropy_core_gui_event_get(ENTROPY_GUI_EVENT_ACTION_FILE);
	gui_event->data = event->file;
	entropy_core_layout_notify_event(event->instance, gui_event, ENTROPY_EVENT_GLOBAL); 

	if (viewer->last_selected_label) {
		ewl_text_cursor_position_set(EWL_TEXT(viewer->last_selected_label), 0);

		/*TODO theme this color stuff*/
		ewl_text_color_apply(EWL_TEXT(viewer->last_selected_label), 0, 0, 0, 255, 
			ewl_text_length_get(EWL_TEXT(viewer->last_selected_label)));		
	}

	/*Highlight this row*/
	ewl_text_cursor_position_set(EWL_TEXT(event->data), 0);
	ewl_text_color_apply(EWL_TEXT(event->data), 0, 0, 255, 255, ewl_text_length_get(EWL_TEXT(event->data)));
	viewer->last_selected_label = event->data;
}

void structure_viewer_add_row(entropy_gui_component_instance* instance, entropy_generic_file* file, Ewl_Row* prow) {
		Ewl_Widget* row;
		Ewl_Widget* image;
		Ewl_Widget* hbox = ewl_hbox_new();
		Ewl_Widget* children[2];
		Ewl_Widget* label = ewl_text_new();
		ewl_text_text_set(EWL_TEXT(label), file->filename);
		event_file_core* event;
		entropy_file_structure_viewer* viewer = (entropy_file_structure_viewer*)instance->data;

		image = ewl_image_new();
		ewl_image_file_set(EWL_IMAGE(image), PACKAGE_DATA_DIR "/icons/folder_structure.png", NULL);
		ewl_widget_show(image);
		
		/*printf ("  Added %s'\n", file->filename);*/
		
		ewl_container_child_append(EWL_CONTAINER(hbox), image);
		ewl_container_child_append(EWL_CONTAINER(hbox), label);

		ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_HFILL);
		ewl_object_fill_policy_set(EWL_OBJECT(label), EWL_FLAG_FILL_HFILL);

		ewl_widget_show(label);
		ewl_widget_show(hbox);

		children[0] = hbox;
		children[1] = NULL;

		/*printf("Adding row %s to existing row\n", file->filename);*/
		row = ewl_tree_row_add(EWL_TREE(viewer->tree), prow, children);

		ewl_object_fill_policy_set(EWL_OBJECT(row), EWL_FLAG_FILL_VSHRINK | EWL_FLAG_FILL_HFILL);
		ewl_container_callback_intercept(EWL_CONTAINER(row), EWL_CALLBACK_MOUSE_DOWN);
		
		ewl_object_custom_h_set(EWL_OBJECT(row), 15);
		ewl_widget_show(row);

		event = entropy_malloc(sizeof(event_file_core)); 
		event->file = file; /*Create a clone of this file, and add it to the event */
		event->instance = instance;
		event->data = label; /*So we can highlight the current directory*/
	
		 /*Save this file in this list of files we're responsible for */
		ecore_list_append(viewer->files, event->file);
		

		ewl_callback_append(row, EWL_CALLBACK_MOUSE_DOWN, row_clicked_callback, event);
		ewl_callback_append(row, EWL_CALLBACK_DND_ENTER, dnd_enter_callback, event);
		ewl_callback_append(row, EWL_CALLBACK_DND_LEAVE, dnd_leave_callback, event);
		ewl_callback_append(row, EWL_CALLBACK_DND_DROP, dnd_drop_callback, event);

		ecore_list_append(viewer->gui_events, event);

		/*Add this row to our map of files -> rows */
		/*printf ("Adding row to hash %p, file %p\n", row, file);*/
		ecore_hash_set(viewer->row_folder_hash, file, row);

}



void entropy_plugin_destroy(entropy_gui_component_instance* comp) {
	//printf("Destroying structure viewer...\n");
}



entropy_gui_component_instance* entropy_plugin_init(entropy_core* core, entropy_gui_component_instance* layout, void* data) {
	entropy_gui_component_instance* instance;
	entropy_file_structure_viewer* viewer;
	Ewl_Widget* child;
	

	 /*entropy_file_request* file_request = entropy_malloc(sizeof(entropy_file_request));*/

	 instance = entropy_gui_component_instance_new();
	 viewer = entropy_malloc(sizeof(entropy_file_structure_viewer));
	 instance->data = viewer;

	instance->layout_parent = layout;

	/*Register out interest in receiving folder notifications*/
	entropy_core_component_event_register(instance, 
		entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS));
	entropy_core_component_event_register(instance, 
		entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS_EXTERNAL));
	entropy_core_component_event_register(instance, 
		entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_REMOVE_DIRECTORY));



	viewer->gui_events = ecore_list_new();
	viewer->files = ecore_list_new();
	viewer->ecore = core;
	instance->core = core;
	
	viewer->tree = ewl_tree_new(1);	
	ewl_tree_headers_visible_set(EWL_TREE(viewer->tree),0);
	viewer->loaded_dirs = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	viewer->row_folder_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	
	ewl_object_fill_policy_set(EWL_OBJECT(EWL_TREE(viewer->tree)->scrollarea),
		EWL_FLAG_FILL_HFILL);
	
	instance->gui_object = viewer->tree;

	structure_viewer_add_row(instance, (entropy_generic_file*)data, NULL);

	ewl_widget_show(viewer->tree);

	return instance;
}
