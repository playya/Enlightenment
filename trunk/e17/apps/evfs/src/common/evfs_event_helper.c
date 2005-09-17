#include <evfs.h>

void evfs_file_monitor_event_create(evfs_client* client, int type, const char* path) {
	/*Create a reply event for a file mon event, send it , destroy event*/

	evfs_event* event = NEW(evfs_event);
	event->type = EVFS_EV_FILE_MONITOR;
	event->file_monitor.filename = (char*)path;
	event->file_monitor.fileev_type = type;
	event->file_monitor.filename_len = strlen(path)+1;

	evfs_write_event(client, event);

	/*Now destroy*/
	free(event);
	/*No need to free data -it's probably a const raised by the ecore - it will free it.*/
	
}
