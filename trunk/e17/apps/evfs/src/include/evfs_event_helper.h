#ifndef __EVFS_EVENT_HELPER_
#define __EVFS_EVENT_HELPER_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "evfs.h"

void evfs_file_monitor_event_create(evfs_client* client, int type, const char* pathi, const char* plugin);
void evfs_stat_event_create(evfs_client* client, evfs_command* command, struct stat* stat_obj);
void evfs_list_dir_event_create(evfs_client* client, evfs_command* command, Ecore_List* files);
void evfs_file_progress_event_create(evfs_client* client, evfs_command* command, double progress,evfs_progress_type type);
void evfs_open_event_create(evfs_client* client, evfs_command* command);
void evfs_read_event_create(evfs_client* client, evfs_command* command, char* bytes, long size);

#endif
