#ifndef __EVFS_COMMANDS_H_
#define __EVFS_COMMANDS_H_

void evfs_monitor_add(evfs_connection* conn, evfs_filereference* ref);
void evfs_client_file_remove(evfs_connection* conn, evfs_filereference* ref);

#endif

