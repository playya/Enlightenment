#include "evfs.h"

void evfs_cleanup_command(evfs_command* command, int free_command) {
	
	switch (command->type) {
		case EVFS_CMD_STARTMON_FILE:
			evfs_cleanup_file_command(command);
			break;
	}

	if (free_command == EVFS_CLEANUP_FREE_COMMAND) free(command);
}

void evfs_cleanup_file_command(evfs_command* command) {
	int i;
	for (i=0;i<command->file_command.num_files;i++) {
		free(command->file_command.files[i]->path);
		free(command->file_command.files[i]->plugin_uri);
		free(command->file_command.files[i]);
	}
}



void evfs_cleanup_file_monitor(evfs_file_monitor* mon) {
	if (mon->monitor_path) free (mon->monitor_path);
	free(mon);
}



/*----------------------------------*/
void evfs_cleanup_filereference(evfs_filereference* ref) {
	if (ref->plugin_uri) free(ref->plugin_uri);
	if (ref->path) free(ref->path);
	if (ref->username) free(ref->username);
	if (ref->password) free(ref->password);
	free(ref);

}

void evfs_cleanup_monitor_event(evfs_event* event) {
	if (event->file_monitor.plugin) free(event->file_monitor.plugin);
	if (event->file_monitor.filename) free(event->file_monitor.filename);
	
}

void evfs_cleanup_file_list_event(evfs_event* event) {
	evfs_filereference* file;
	
	if (event->file_list.list) {
		ecore_list_goto_first(event->file_list.list);
		while ( (file = ecore_list_remove_first(event->file_list.list))) {
			evfs_cleanup_filereference(file);
		}
		ecore_list_destroy(event->file_list.list);
	}
}

void evfs_cleanup_file_read_event(evfs_event* event) {
	if (event->data.bytes) 
		free(event->data.bytes);
}

void evfs_cleanup_event(evfs_event* event) {
	evfs_cleanup_command(&event->resp_command, EVFS_CLEANUP_PRESERVE_COMMAND);

	switch (event->type) {
		case EVFS_EV_FILE_MONITOR:
			evfs_cleanup_monitor_event(event);
			break;
		case EVFS_EV_DIR_LIST:
			evfs_cleanup_file_list_event(event);
			break;
		case EVFS_EV_FILE_READ:
			evfs_cleanup_file_read_event(event);
			break;
	}

	free(event);
		
}
