#include "evfs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void
evfs_file_monitor_event_create(evfs_client * client, int type, const char *path,
                               int filetype, const char *plugin)
{
   /*Create a reply event for a file mon event, send it , destroy event */

   EvfsEventFileMonitor *event = NEW(EvfsEventFileMonitor);

   EVFS_EVENT(event)->type = EVFS_EV_FILE_MONITOR;
   event->file = evfs_filereference_new((char*)plugin,(char*)path,filetype);
   event->type = type;

   evfs_write_event(client, NULL, EVFS_EVENT(event));

   /*Now destroy */
   evfs_cleanup_event(EVFS_EVENT(event));

}

void
evfs_stat_event_create(evfs_client * client, evfs_command * command,
                       struct stat *stat_obj)
{
   /*Create a reply event for a file mon event, send it , destroy event */

   EvfsEventStat *event = NEW(EvfsEventStat);
   EVFS_EVENT(event)->type = EVFS_EV_STAT;

   //memcpy(&event->stat.stat_obj, stat_obj, sizeof(struct stat));
   event->st_mode = stat_obj->st_mode;
   event->st_uid = stat_obj->st_uid;
   event->st_gid = stat_obj->st_gid;
   event->st_uid = stat_obj->st_uid;
   event->st_size = stat_obj->st_size;
   event->ist_atime = stat_obj->st_atime;
   event->ist_mtime = stat_obj->st_mtime;
   event->ist_ctime = stat_obj->st_ctime;

   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Now destroy */
   evfs_cleanup_event(EVFS_EVENT(event));

}

void
evfs_list_dir_event_create(evfs_client * client, evfs_command * command,
                           Ecore_List * files)
{
   EvfsFilereference* file;

   printf("EVFS: sending directory listing\n");

   EvfsEventDirList *event = NEW(EvfsEventDirList);

   EVFS_EVENT(event)->type = EVFS_EV_DIR_LIST;
   event->files = NULL;
   while ((file = ecore_list_remove_first(files))) {
	   event->files = evas_list_append(event->files, file);
   }
   ecore_list_destroy(files);
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Now destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_auth_failure_event_create(evfs_client * client, evfs_command * command)
{
	EvfsEventAuthFailure *event = NEW(EvfsEventAuthFailure);
	EVFS_EVENT(event)->type = EVFS_EV_AUTH_REQUIRED;

	evfs_write_event(client,command,EVFS_EVENT(event));

	/*Now destroy */
	evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_file_progress_event_create(evfs_client * client,
				EvfsFilereference* file_from,
				EvfsFilereference* file_to,
                                evfs_command * root_command, double progress,
                                EvfsEventProgressType type)
{
   /*Create a reply event for a file mon event, send it , destroy event */

   EvfsEventProgress *event = NEW(EvfsEventProgress);

   EVFS_EVENT(event)->type = EVFS_EV_FILE_PROGRESS;
   event->progressAmt = progress;
   event->type = type;
   event->from = file_from;
   event->to = file_to;
   
   evfs_write_event(client, root_command, EVFS_EVENT(event));

   /*Now destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_open_event_create(evfs_client * client, evfs_command * command)
{
   /*Create a reply event for a file mon event, send it , destroy event */

   EvfsEventOpen *event = NEW(EvfsEventOpen);

   EVFS_EVENT(event)->type = EVFS_EV_FILE_OPEN;
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Now destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_read_event_create(evfs_client * client, evfs_command * command,
                       char *bytes, long size)
{
   EvfsEventData *event = NEW(EvfsEventData);
   EVFS_EVENT(event)->type = EVFS_EV_FILE_READ;
   
   event->size = size;
   event->bytes = bytes;
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_operation_event_create(evfs_client * client, evfs_command * command,
                            evfs_operation * op, char* misc)
{
   EvfsEventOperation *event = NEW(EvfsEventOperation);

   EVFS_EVENT(event)->type = EVFS_EV_OPERATION;
   event->operation = op;
   event->misc = misc;
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void
evfs_meta_data_event_create(evfs_client* client,evfs_command* command, Evas_List* ret_list)
{
   EvfsEventMetadata *event = NEW(EvfsEventMetadata);

   EVFS_EVENT(event)->type = EVFS_EV_METADATA;
   event->meta_list = ret_list;
	
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}

void evfs_group_list_event_create(evfs_client* client, evfs_command* command, Evas_List* group_list) 
{
   EvfsEventMetadataGroups *event = NEW(EvfsEventMetadataGroups);

   EVFS_EVENT(event)->type = EVFS_EV_METADATA_GROUPS;
   event->string_list = group_list;
	
   evfs_write_event(client, command, EVFS_EVENT(event));

   /*Destroy */
   evfs_cleanup_event(EVFS_EVENT(event));
}
