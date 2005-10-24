#include <Ewl.h>
#include "entropy.h"
#include "entropy_gui.h"
#include "entropy_config.h"
#include <dlfcn.h>
#include <time.h>

typedef struct gui_file gui_file;
struct gui_file {
	        entropy_generic_file* file;
	        entropy_thumbnail* thumbnail;
		entropy_gui_component_instance* instance;
	        Ewl_Widget* icon;
};
gui_file* gui_file_new();
void gui_file_destroy(gui_file*);

void gui_event_callback(entropy_notify_event* eevent, void* requestor, void* ret, void* user_data);



typedef struct entropy_icon_viewer entropy_icon_viewer;
struct entropy_icon_viewer {
	Ewl_Widget* iconbox;
	Ecore_Hash* gui_hash; /*A list of our current directory's files*/
	Ecore_Hash* icon_hash; /*A hash for ewl callbacks*/
	Ecore_List* current_events; /*A ref to the events we have waiting on the queue.  perhaps we should handle this in the notify queue,
				      one less thing for plugins to handle */

	Ewl_Widget* file_dialog;
	Ewl_Widget* file_dialog_parent;
	
	char current_dir[1024]; /* We should handle this at the core.  FUTURE API TODO */
};



void
 __destroy_properties_dialog(Ewl_Widget *dialog, void *ev_data, void *user_data)
 {
	 ewl_widget_destroy(EWL_WIDGET(user_data));
}




void ewl_icon_local_viewer_show_stat(entropy_file_stat* file_stat) {
	Ewl_Widget* window;
	Ewl_Widget* vbox;
	Ewl_Widget* image;
	Ewl_Widget* hbox;
	Ewl_Widget* text;
	Ewl_Widget* button;
	char itext[100];
	time_t stime;
	
	window = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(window), "File Properties");
	ewl_object_custom_size_set(EWL_OBJECT(window), 300, 400);
	
	vbox = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(window), vbox);
	ewl_widget_show(vbox);


	/*----------------------------*/
	/*The icon*/
	if (file_stat->file->thumbnail) {
		hbox = ewl_hbox_new();
		ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
		ewl_widget_show(hbox);

		image = ewl_image_new();
		ewl_image_file_set(EWL_IMAGE(image), file_stat->file->thumbnail->thumbnail_filename, NULL);
		ewl_container_child_append(EWL_CONTAINER(hbox), image);
		ewl_widget_show(image);

		text = ewl_text_new();
		ewl_text_text_set(EWL_TEXT(text), file_stat->file->filename);
		ewl_container_child_append(EWL_CONTAINER(hbox), text);
		ewl_widget_show(text);


	}

	

	/*---------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Location");
	ewl_object_minimum_w_set(EWL_OBJECT(text), 90);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file_stat->file->path);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	/*--------------------------*/



	/*----------------------------------------*/
	/*hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Filename");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);*/



	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Type: ");
	ewl_object_minimum_w_set(EWL_OBJECT(text), 90);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file_stat->file->mime_type);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);
	/*--------------------------------*/
	

	/*----------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Plugin URI");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_object_minimum_w_set(EWL_OBJECT(text), 90);
	ewl_widget_show(text);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file_stat->file->uri_base);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	/*---------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Size: ");
	ewl_object_minimum_w_set(EWL_OBJECT(text), 90);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	snprintf(itext, 100, "%d kb", file_stat->stat_obj->st_size / 1024);
	ewl_text_text_set(EWL_TEXT(text), itext);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);
	/*-------------------------------------*/

	/*---------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Modified Time");
	ewl_object_minimum_w_set(EWL_OBJECT(text), 90);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	stime = file_stat->stat_obj->st_mtime;
	ewl_text_text_set(EWL_TEXT(text), ctime(&stime));
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);
	/*-------------------------------------*/


	

	button = ewl_button_new();
	ewl_button_label_set(EWL_BUTTON(button), "OK");
	ewl_container_child_append(EWL_CONTAINER(vbox), button);
	ewl_object_maximum_h_set(EWL_OBJECT(button), 15);
	ewl_widget_show(button);
	ewl_callback_append(EWL_WIDGET(button), EWL_CALLBACK_CLICKED, __destroy_properties_dialog, window);

	

	
	
	/*printf("Got a 'stat available' object\n");
	printf("File size: %d\n", file_stat->stat_obj->st_size);
	printf("File inode: %d\n", file_stat->stat_obj->st_ino);
	printf("File uid: %d\n", file_stat->stat_obj->st_uid);
	printf("File gid: %d\n", file_stat->stat_obj->st_gid);
	printf("Last access: %d\n", file_stat->stat_obj->st_atime);
	printf("Last modify : %d\n", file_stat->stat_obj->st_mtime);*/


	ewl_widget_show(window);
}


/*---------------------------*/
/*Functions to handle custom background setting*/
void ewl_iconbox_background_remove_cb(Ewl_Widget *w , void *ev, void *user_data ) {
	entropy_gui_component_instance* instance = user_data;
	entropy_icon_viewer* viewer = instance->data;

	entropy_config_str_set("iconbox_viewer", viewer->current_dir,NULL);
}


void ewl_iconbox_background_set_file_cb(Ewl_Widget *w , void *ev, void *user_data ) {
        Ewl_Filedialog_Event *e;
	entropy_gui_component_instance* instance = user_data;
	entropy_icon_viewer* viewer = instance->data;

        e = EWL_FILEDIALOG_EVENT(ev);
        if (e->response == EWL_STOCK_OPEN) {
                printf("file open from test program: %s\n",
                                ewl_filedialog_file_get (EWL_FILEDIALOG (w)));

		printf("Curent directory is '%s'\n", viewer->current_dir);
		entropy_config_str_set("iconbox_viewer", viewer->current_dir, ewl_filedialog_file_get (EWL_FILEDIALOG (w)));

	} else if (e->response == EWL_STOCK_CANCEL) {
		ewl_widget_destroy(viewer->file_dialog_parent);
	}

}


void ewl_iconbox_background_set_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	entropy_gui_component_instance* instance = user_data;
	entropy_icon_viewer* viewer = instance->data;
	
	viewer->file_dialog = ewl_filedialog_new();
	viewer->file_dialog_parent = ewl_window_new();

	ewl_filedialog_type_set(EWL_FILEDIALOG(viewer->file_dialog), EWL_FILEDIALOG_TYPE_OPEN);
        ewl_callback_append (viewer->file_dialog, EWL_CALLBACK_VALUE_CHANGED, ewl_iconbox_background_set_file_cb, instance);
	ewl_container_child_append(EWL_CONTAINER(viewer->file_dialog_parent), viewer->file_dialog);
	ewl_widget_show(viewer->file_dialog);
	ewl_widget_show(viewer->file_dialog_parent);

}
/*---------------------------*/


void ewl_iconbox_file_paste_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	Ecore_List* selected;
	entropy_generic_file* file;
	entropy_gui_component_instance* instance = ((entropy_gui_component_instance*)user_data);
	entropy_plugin* plugin = entropy_plugins_type_get_first(instance->core->plugin_list, ENTROPY_PLUGIN_BACKEND_FILE ,ENTROPY_PLUGIN_SUB_TYPE_ALL);

	void (*copy_func)(entropy_generic_file* source, char* dest_uri);


	/*Get the func ref*/
	copy_func = dlsym(plugin->dl_ref, "entropy_filesystem_file_copy");
	
	//printf("Paste the following files:\n");

	selected = entropy_core_selected_files_get( instance->core);
	ecore_list_goto_first(selected);

	while ( (file = ecore_list_next(selected))  ) {
		//printf("File '%s'\n", file->filename);
		(*copy_func)(file, ((entropy_icon_viewer*)instance->data)->current_dir );

		
	}
	

}

void ewl_iconbox_file_copy_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	Ecore_List* icon_list;
	gui_file* file;
	Ewl_IconBox_Icon* list_item;
	entropy_gui_component_instance* instance = (entropy_gui_component_instance*)user_data;

	/*Clear the existing contents*/
	entropy_core_selected_files_clear(instance->core);
	
	//printf("Copy files to clipboard..\n");

	icon_list = ewl_iconbox_get_selection( EWL_ICONBOX(((entropy_icon_viewer*)instance->data)->iconbox) );

	ecore_list_goto_first(icon_list);
	while ( (list_item = ecore_list_next(icon_list)) )  {
		file = ecore_hash_get( ((entropy_icon_viewer*)instance->data)->icon_hash, list_item);
		entropy_core_selected_file_add(instance->core, file->file);

		
		
	}


	ecore_list_destroy(icon_list);
	


	
}


void icon_properties_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	entropy_gui_component_instance* instance = (entropy_gui_component_instance*)user_data;
	entropy_icon_viewer* viewer = instance->data;
	entropy_gui_event* gui_event;
	gui_file* local_file = ecore_hash_get( viewer->icon_hash, EWL_ICONBOX(viewer->iconbox)->select_icon);

	
	//Stat test..
	/*Send an event to the core*/
	if (local_file) {
	
		gui_event = entropy_malloc(sizeof(entropy_gui_event));
		gui_event->event_type = entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_STAT);
		gui_event->data = local_file->file;
		entropy_core_layout_notify_event(instance , gui_event, ENTROPY_EVENT_LOCAL); 
	} else {
		printf("Could not find selected icon!\n");
	}
}


void icon_click_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	Ewl_Event_Mouse_Down *ev = ev_data;
	entropy_gui_event* gui_event;
	gui_file* local_file = ecore_hash_get( ((entropy_icon_viewer*)user_data)->icon_hash, w);

	if (!local_file) { printf ("*Alert* Couldn't find a local file reference for icon\n"); }

	
	
        if (ev->clicks > 1) {

		if (ev->button == 1) {
			//printf("Icon clicked %d, widget %p!\n", ev->clicks, w);

			/*Send an event to the core*/
			gui_event = entropy_malloc(sizeof(entropy_gui_event));
			gui_event->event_type = entropy_core_gui_event_get(ENTROPY_GUI_EVENT_ACTION_FILE);
			gui_event->data = local_file->file;
			entropy_core_layout_notify_event(  local_file->instance , gui_event, ENTROPY_EVENT_GLOBAL); 

		} else if (ev->button == 2) {

		}

	}

	

	
}


gui_file* gui_file_new() {
	 allocated_gui_file++;
         return entropy_malloc(sizeof(gui_file));
}

void gui_file_destroy(gui_file* file) {
	allocated_gui_file--;
	entropy_free(file);

}

int entropy_plugin_type_get() {
        return ENTROPY_PLUGIN_GUI_COMPONENT;
}

int entropy_plugin_sub_type_get() {
	return ENTROPY_PLUGIN_GUI_COMPONENT_LOCAL_VIEW;
}

char* entropy_plugin_identify() {
	        return (char*)"EWL local viewer";
}

void gui_object_destroy_and_free(entropy_gui_component_instance* comp, Ecore_Hash* gui_hash) {
	
	Ecore_List* list;
	entropy_generic_file* obj;	
	gui_file* freeobj;

	/*Temporarily stop callbacks, we don't want to clobber an in-op process*/
	entropy_notify_lock_loop(comp->core->notify);

	list = ecore_hash_keys(gui_hash);
	
	ecore_list_goto_first(list);
	while ( (obj = ecore_list_next(list)) ) {
		
		
		freeobj = ecore_hash_get( gui_hash, obj);
		
		if (freeobj) gui_file_destroy(freeobj);

		/*Tell the core we no longer need this file - it might free it now*/
		entropy_core_file_cache_remove_reference(comp->core, obj->md5);
	}
	ecore_hash_destroy( gui_hash);
	ecore_list_destroy(list);

	entropy_notify_unlock_loop(comp->core->notify);
	
	
}


void entropy_plugin_destroy(entropy_gui_component_instance* comp) {
//	printf ("Destroying icon viewer...\n");
}

entropy_gui_component_instance* entropy_plugin_init(entropy_core* core,entropy_gui_component_instance* layout) {
	Ewl_Widget* context;
	entropy_gui_component_instance* instance = entropy_malloc(sizeof(entropy_gui_component_instance));
	entropy_icon_viewer* viewer = entropy_malloc(sizeof(entropy_icon_viewer));



	/*Save a reference to our local data*/
	instance->data = viewer;
	instance->layout_parent = layout;
	
        viewer->iconbox = ewl_iconbox_new();
	viewer->current_events = ecore_list_new();
	instance->gui_object = viewer->iconbox;
	ewl_widget_show(EWL_WIDGET(viewer->iconbox));

	/*Add some context menu items*/
	context = ewl_menu_item_new();
	ewl_menu_item_text_set(EWL_MENU_ITEM(context), "Copy selection");
	ewl_iconbox_context_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_callback_append(context, EWL_CALLBACK_MOUSE_DOWN, ewl_iconbox_file_copy_cb, instance);
	ewl_widget_show(context);

	/*Add some context menu items*/
	context = ewl_menu_item_new();
	ewl_menu_item_text_set(EWL_MENU_ITEM(context), "Paste");
	ewl_iconbox_context_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_callback_append(context, EWL_CALLBACK_MOUSE_DOWN, ewl_iconbox_file_paste_cb, instance);
	ewl_widget_show(context);

	/*Add some context menu items*/
	context = ewl_separator_new();
	ewl_iconbox_context_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_widget_show(context);

	/*Add some context menu items*/
	context = ewl_menu_item_new();
	ewl_menu_item_text_set(EWL_MENU_ITEM(context), "Set custom folder background...");
	ewl_iconbox_context_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_callback_append(context, EWL_CALLBACK_MOUSE_DOWN, ewl_iconbox_background_set_cb, instance);
	ewl_widget_show(context);

	/*Add some context menu items*/
	context = ewl_menu_item_new();
	ewl_menu_item_text_set(EWL_MENU_ITEM(context), "Remove current custom background");
	ewl_iconbox_context_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_callback_append(context, EWL_CALLBACK_MOUSE_DOWN, ewl_iconbox_background_remove_cb, instance);
	ewl_widget_show(context);



	/*Icon menu*/
	context = ewl_menu_item_new();
	ewl_menu_item_text_set(EWL_MENU_ITEM(context), "Properties");
	ewl_widget_show(context);
	ewl_iconbox_icon_menu_item_add(EWL_ICONBOX(viewer->iconbox), context);
	ewl_callback_append(context, EWL_CALLBACK_MOUSE_DOWN, icon_properties_cb, instance);
	


	/*FIXME remove the hardocded var*/
	ewl_iconbox_icon_size_custom_set(EWL_ICONBOX(viewer->iconbox), 60,60);
	

	/*Init the hash*/
	viewer->gui_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	viewer->icon_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);

	/*Set the core back reference*/
	instance->core = core;

	/*Register out interest in receiving folder notifications*/
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS));
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FOLDER_CHANGE_CONTENTS_EXTERNAL));
	/*Register our interest in receiving file mod/create/delete notifications*/
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_CHANGE));
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_CREATE));
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_REMOVE));
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_REMOVE_DIRECTORY));
	

	/*Register interest in getting stat events*/
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_STAT));
	entropy_core_component_event_register(core, instance, entropy_core_gui_event_get(ENTROPY_GUI_EVENT_FILE_STAT_AVAILABLE));

	return instance;
}


ewl_icon_local_viewer_remove_icon(entropy_gui_component_instance* comp, entropy_generic_file* list_item ) {
	entropy_icon_viewer* view = comp->data;
	gui_file* gui_object;
	
	if ( (gui_object = ecore_hash_get(view->gui_hash, list_item)) ) {
		ewl_iconbox_icon_remove(EWL_ICONBOX(view->iconbox), EWL_ICONBOX_ICON(gui_object->icon));
	}
}


void ewl_icon_local_viewer_add_icon(entropy_gui_component_instance* comp, entropy_generic_file* list_item) {
		entropy_icon_viewer* view = comp->data;
	
		entropy_plugin* thumb;
		Ewl_IconBox_Icon* icon;
		gui_file* gui_object;

		if (!ecore_hash_get(view->gui_hash, list_item)) {	
			 entropy_core_file_cache_add_reference(comp->core, list_item->md5);			
			 char* mime;
			 /*printf("%s\n", list_item->filename);*/
	                 mime = entropy_mime_file_identify(comp->core->mime_plugins, list_item);

			 if (mime && strcmp(mime, ENTROPY_NULL_MIME)) {
	                        thumb = entropy_thumbnailer_retrieve(comp->core->entropy_thumbnailers, mime);
			 } else {
				 thumb = NULL;
			}
			icon = ewl_iconbox_icon_add(EWL_ICONBOX(view->iconbox), list_item->filename, PACKAGE_DATA_DIR "/icons/default.png");
			

			gui_object = gui_file_new();
	                gui_object->file = list_item;
	                gui_object->thumbnail = NULL;
			gui_object->instance = comp; /*This instance associated with this icon, for clicks*/
	                gui_object->icon = EWL_WIDGET(icon);

			ewl_callback_append(EWL_WIDGET(icon), EWL_CALLBACK_MOUSE_DOWN, icon_click_cb, view); 
		
			ecore_hash_set(view->gui_hash, list_item, gui_object);
			ecore_hash_set(view->icon_hash, icon, gui_object);
			
			/*If thre's a thumbnailer for this, Register request to thumbnail for this filename*/
	                if (thumb) {
				entropy_notify_event* ev = entropy_notify_request_register(comp->core->notify, comp, ENTROPY_NOTIFY_THUMBNAIL_REQUEST,thumb, "entropy_thumbnailer_thumbnail_get", list_item,NULL);
			
				entropy_notify_event_callback_add(ev, (void*)gui_event_callback, comp);
				entropy_notify_event_commit(comp->core->notify,ev);
			}
		}
			
}

void gui_event_callback(entropy_notify_event* eevent, void* requestor, void* ret, void* user_data) {
     entropy_gui_component_instance* comp = (entropy_gui_component_instance*)user_data;
	
     switch (eevent->event_type) {	
	case ENTROPY_NOTIFY_FILELIST_REQUEST_EXTERNAL:
	case ENTROPY_NOTIFY_FILELIST_REQUEST: {
		Ecore_List* el = (Ecore_List*)ret;
		entropy_file_request* request = eevent->data; /*A file request's data is the dest dir*/
		
		entropy_icon_viewer* view = comp->data;
		Ecore_Hash* tmp_gui_hash;
		Ecore_Hash* tmp_icon_hash;
		entropy_generic_file* list_item;

		
		/*Set the current path from the event source...*/
		snprintf(view->current_dir, 1024, "%s://%s/%s", request->file->uri_base, request->file->path, request->file->filename);


		/*Keep a reference to our existing hash*/
		tmp_gui_hash = view->gui_hash;
		tmp_icon_hash = view->icon_hash;
		view->gui_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);	
		view->icon_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);


		
		
		
		/*Clear the view, if there's anything to nuke*/
		ewl_iconbox_clear(EWL_ICONBOX(view->iconbox));

	
		ecore_list_goto_first(el);
		while ( (list_item = ecore_list_next(el)) ) {
			ewl_icon_local_viewer_add_icon(comp, list_item);
		}


		/*Before we begin, see if our file hash is initialized, if so - we must destroy it first*/
		/*TODO*/
		gui_object_destroy_and_free(comp, tmp_gui_hash);
		ecore_hash_destroy(tmp_icon_hash);



		/*First, see if there is a custom BG image for this folder*/
		if (entropy_config_str_get("iconbox_viewer", view->current_dir)) {
			ewl_iconbox_background_set(EWL_ICONBOX(view->iconbox), entropy_config_str_get("iconbox_viewer", view->current_dir));
		} else {
			ewl_iconbox_background_set(EWL_ICONBOX(view->iconbox), NULL);
		}

		/*Goto the root*/
		ewl_iconbox_scrollpane_recalculate(EWL_ICONBOX(view->iconbox));
		ewl_iconbox_scrollpane_goto_root(EWL_ICONBOX(view->iconbox));
	}
	break;


	case ENTROPY_NOTIFY_THUMBNAIL_REQUEST: {
	        /*Only bother if we have a thumbnail, and a component*/
		if (ret && user_data) {
			gui_file* obj;
		        entropy_thumbnail* thumb = (entropy_thumbnail*)ret;
			entropy_icon_viewer* view = comp->data;

		        obj = ecore_hash_get(view->gui_hash, thumb->parent );

		        if (obj) {
		                obj->thumbnail = thumb;

	        	        /*printf("Received callback notify from notify event..\n");
		                printf("Created thumbnail (at callback): '%s', for file: '%s'\n", obj->thumbnail->thumbnail_filename, obj->file->filename);*/
		                /*Make sure the icon still exists*/
				/*if (obj->icon) {*/
				ewl_iconbox_icon_image_set(EWL_ICONBOX_ICON(obj->icon), obj->thumbnail->thumbnail_filename);

				/*FIXME This is inefficient as all hell - find a better way to do this*/
				//ewl_iconbox_icon_arrange(EWL_ICONBOX(view->iconbox)); 
	        	} else {
	                	printf("ERR: Couldn't find a hash reference for this file!\n");
	        	}
		}
	} //End case
	break;


       case ENTROPY_NOTIFY_FILE_CHANGE: {
		//printf ("Received file change event at icon viewer for file %s \n", ((entropy_generic_file*)ret)->filename);
       }
       break;

       case ENTROPY_NOTIFY_FILE_CREATE: {
		//printf ("Received file create event at icon viewer for file %s \n", ((entropy_generic_file*)ret)->filename);
		ewl_icon_local_viewer_add_icon(comp, (entropy_generic_file*)ret);
       }
       break;

       case ENTROPY_NOTIFY_FILE_REMOVE_DIRECTORY: 
       case ENTROPY_NOTIFY_FILE_REMOVE: {
		printf("Received a remove file notify\n");
		ewl_icon_local_viewer_remove_icon(comp, (entropy_generic_file*)ret);
	}
	break;

       case ENTROPY_NOTIFY_FILE_STAT_EXECUTED: {
		//printf("STAT EXECUTED Response back at ewl_icon_local_viewer\n");
       }
       break;

       case ENTROPY_NOTIFY_FILE_STAT_AVAILABLE: {
							
		entropy_file_stat* file_stat = (entropy_file_stat*)ret;
		if (file_stat->file == NULL) { printf ( "***** File stat file is null\n"); }
		ewl_icon_local_viewer_show_stat(file_stat);
		

       }
       break;

    } //End switch

	
}
