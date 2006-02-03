#ifndef __EVFS_COMMANDS_H_
#define __EVFS_COMMANDS_H_

void evfs_monitor_add(evfs_connection * conn, evfs_filereference * ref);
void evfs_monitor_remove(evfs_connection * conn, evfs_filereference * ref);
void evfs_client_file_remove(evfs_connection * conn, evfs_filereference * ref);
void evfs_client_file_rename(evfs_connection * conn, evfs_filereference * from,
                             evfs_filereference * to);
void evfs_client_file_stat(evfs_connection * conn, evfs_filereference * file);
void evfs_client_dir_list(evfs_connection * conn, evfs_filereference * file);
void evfs_client_file_open(evfs_connection * conn, evfs_filereference * file);
void evfs_client_file_copy(evfs_connection * conn, evfs_filereference * from,
                           evfs_filereference * to);
void evfs_client_file_open(evfs_connection * conn, evfs_filereference * file);
void evfs_client_file_read(evfs_connection * conn, evfs_filereference * file,
                           int read_size);
void evfs_client_operation_respond(evfs_connection * conn, long opid,
                                   evfs_operation_response response);

#endif
