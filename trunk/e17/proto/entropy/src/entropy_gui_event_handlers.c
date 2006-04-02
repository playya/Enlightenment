#include "entropy.h"
#include "entropy_gui_event_handler.h"

/*N.B. - a lot of these handlers are very similar. We may make a generic wrapper for these*/

Entropy_Gui_Event_Handler* entropy_gui_event_handler_new(
		Entropy_Gui_Event_Handler_Instance_Data* 
			(*notify_event_cb)(entropy_gui_event* event, entropy_gui_component_instance* instance),
		void (*cleanup_cb)(struct Entropy_Gui_Event_Handler_Instance_Data*) )
{
	Entropy_Gui_Event_Handler* handler = entropy_malloc(sizeof(Entropy_Gui_Event_Handler));

	handler->notify_event_cb =  notify_event_cb;
	handler->cleanup_cb = cleanup_cb;

	return handler;
}

void entropy_event_handler_instance_data_generic_cleanup(Entropy_Gui_Event_Handler_Instance_Data* data)
{
	if (data->notify)
		entropy_notify_event_destroy(data->notify);

	entropy_free(data);

}

/*File create*/
Entropy_Gui_Event_Handler* entropy_event_handler_file_create_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_create_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_create_instance_data(entropy_gui_event* event, 
		entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_CREATE;
	ev->processed = 1;
	ev->return_struct = event->data;

	data->notify = ev;

	return data;
}
/*----------------------*/


/*File remove*/
Entropy_Gui_Event_Handler* entropy_event_handler_file_remove_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_remove_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_remove_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_REMOVE;
	ev->processed = 1;
	ev->return_struct = event->data;

	data->notify = ev;

	return data;
}
/*---------------------------*/

/*File change*/
Entropy_Gui_Event_Handler* entropy_event_handler_file_change_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_change_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_change_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_CHANGE;
	ev->processed = 1;
	ev->return_struct = event->data;

	data->notify = ev;

	return data;
}

/*-------------------------------------*/


/*File remove directory*/
Entropy_Gui_Event_Handler* entropy_event_handler_file_remove_directory_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_remove_directory_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_remove_directory_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_REMOVE_DIRECTORY;
	ev->processed = 1;
	ev->return_struct = event->data;

	data->notify = ev;

	return data;
}
/*---------------------------*/


/*File stat (outbound) */
Entropy_Gui_Event_Handler* entropy_event_handler_file_stat_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_stat_instance_data,
			entropy_event_handler_file_stat_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_stat_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	entropy_notify_event *ev = entropy_notify_event_new();
	entropy_file_request* request = entropy_malloc(sizeof(entropy_file_request));

	/*Add a reference to this file*/
	entropy_core_file_cache_add_reference (((entropy_generic_file*)event->data)->md5);
	
	/*Set up the request..*/
	request->file = event->data;
	request->core = entropy_core_get_core();
	request->requester = requestor->layout_parent;
	/*--------------------------------------------*/

	ev->event_type = ENTROPY_NOTIFY_FILE_STAT_EXECUTED;
	ev->processed = 1;

	/*Actually request the stat*/
	entropy_plugin_filesystem_filestat_get(request);

	data->notify = ev;
	data->misc_data1 = request;

	return data;
}

void entropy_event_handler_file_stat_cleanup(Entropy_Gui_Event_Handler_Instance_Data* data)
{
	if (data->notify)
		entropy_notify_event_destroy(data->notify);

	/*Free the file request*/
	entropy_free(data->misc_data1);
			
	entropy_free(data);

}
/*-------------------------------------*/


/*File stat (inbound) */
Entropy_Gui_Event_Handler* entropy_event_handler_file_stat_available_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_stat_available_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_stat_available_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_STAT_AVAILABLE; 
	ev->processed = 1;

	ev->return_struct = event->data;
	if (event->data) ev->data = ((entropy_file_stat*)event->data)->file;

	data->notify = ev;

	return data;
}
/*-----------------------------------*/

/*Action file*/
Entropy_Gui_Event_Handler* entropy_event_handler_file_action_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_file_action_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_file_action_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));
	
	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_ACTION; 
	ev->key = event->key;
	ev->processed = 1;
	ev->return_struct = event->data; /*An entropy generic file*/

	data->notify = ev;

	return data;
}
/*--------------------------------------*/


/*Thumbnail available*/
Entropy_Gui_Event_Handler* entropy_event_handler_thumbnail_available_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_thumbnail_available_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_thumbnail_available_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_THUMBNAIL_REQUEST; 
	ev->return_struct = event->data;
	
	/*if (ev->return_struct)
		ev->data = ((entropy_thumbnail*)event->data)->parent;*/
	ev->processed = 1;

	data->notify = ev;

	return data;
}
/*------------------------------------*/


/*Progress*/	
Entropy_Gui_Event_Handler* entropy_event_handler_progress_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_progress_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_progress_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_PROGRESS; 
	ev->processed = 1;
		
	data->notify = ev;
	ev->return_struct = event->data;

	return data;
}

/*------------------------------------*/



/*Folder change*/
Entropy_Gui_Event_Handler* entropy_event_handler_folder_change_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_folder_change_instance_data,
			entropy_event_handler_folder_change_cleanup);
	
}


Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_folder_change_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	entropy_notify_event *ev = entropy_notify_event_new();
	Ecore_List* res;	
	entropy_file_request* request = entropy_malloc(sizeof(entropy_file_request));
		

	ev->event_type = ENTROPY_NOTIFY_FILELIST_REQUEST;
	ev->processed = 1;

	/*Check if we need to put a slash between the path/file*/
	if (((entropy_file_request*)event->data)->drill_down) {
		printf("Request for drill down\n");
	}

	request->file = ((entropy_file_request*)event->data)->file;
	request->requester = requestor->layout_parent; /*Requester is the layout parent - after all - one dir per layout at one time*/
	request->core = entropy_core_get_core();
	request->file_type = FILE_ALL;
	request->drill_down = ((entropy_file_request*)event->data)->drill_down;

	ev->data = request;

	/*HACK/FIXME - see what happens if we expire events - this should be on request*/
	printf("************* Calling interceptor..\n");
	entropy_notify_event_expire_requestor_layout(requestor);
	
	res = entropy_plugin_filesystem_filelist_get(request);
	ev->return_struct = res;

	data->notify = ev;
	data->notify->return_struct = res;

	/*Nuke the file_request object that was passed to us*/
	data->misc_data1 = event->data;
	data->misc_data2 = request;

	return data;

}

void entropy_event_handler_folder_change_cleanup(Entropy_Gui_Event_Handler_Instance_Data* data)
{
	if (data->notify)
		entropy_notify_event_destroy(data->notify);

	entropy_free(data->misc_data1);
	entropy_free(data->misc_data2);

	entropy_free(data);

}
/*------------------------------*/


/*Metadata (outbound)*/
Entropy_Gui_Event_Handler* entropy_event_handler_metadata_request_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_metadata_request_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_metadata_request_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	entropy_notify_event* ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_METADATA_REQUEST; 
	ev->key = event->key;
	ev->processed = 1;
	ev->return_struct = event->data;

	data->notify = ev;

	return data;
}
/*----------------------------*/

/*Metadata (inbound) */

Entropy_Gui_Event_Handler* entropy_event_handler_metadata_available_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_metadata_available_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_metadata_available_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = NULL;
	entropy_notify_event* ev = NULL;
	
	data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_FILE_METADATA_AVAILABLE; 
	ev->return_struct = event->data;
	ev->processed = 1;

	data->notify = ev;
	

	return data;
}
/*----------------------------*/

/*User interaction */
Entropy_Gui_Event_Handler* entropy_event_handler_user_interaction_handler()
{
	return entropy_gui_event_handler_new(
			entropy_event_handler_user_interaction_instance_data,
			entropy_event_handler_instance_data_generic_cleanup);
	
}

Entropy_Gui_Event_Handler_Instance_Data* entropy_event_handler_user_interaction_instance_data(entropy_gui_event* event, 
	entropy_gui_component_instance* requestor) 
{
	Entropy_Gui_Event_Handler_Instance_Data* data = NULL;
	entropy_notify_event* ev = NULL;
	
	data = entropy_malloc(sizeof(Entropy_Gui_Event_Handler_Instance_Data));

	ev = entropy_notify_event_new();
	ev->event_type = ENTROPY_NOTIFY_USER_INTERACTION_YES_NO_ABORT; 
	ev->return_struct = event->data;
	ev->processed = 1;

	data->notify = ev;
	

	return data;
}


