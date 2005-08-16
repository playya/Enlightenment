#ifndef __EVFS_H_
#define __EVFS_H_

#define _GNU_SOURCE
#include "evfs_macros.h"
#include "evfs_debug.h"
#include "evfs_plugin.h"
#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <pthread.h>


#define EVFS_IPC_TITLE "evfs_fs"
#define MAXPATHLEN 512
#define FALSE 0
#define TRUE 1
#define URI_SEP "#"
#define EVFS_META_DIR_NAME  ".e_meta"
#define MAX_CLIENT 9999999

#define EVFS_MONITOR_START "evfs_monitor_start"

typedef enum
{
  EVFS_FS_OP_FORCE      = 1,
  EVFS_FS_OP_RECURSIVE  = 2
}
EfsdFsOps;






typedef struct evfs_server evfs_server;
struct evfs_server {
	Ecore_Hash* client_hash;
	Ecore_Hash* plugin_uri_hash;
	Ecore_Ipc_Server* ipc_server;
	unsigned long clientCounter;

	int num_clients;
};

typedef struct evfs_filereference evfs_filereference;
struct evfs_filereference {
	char* plugin_uri;
	char* path;
};

typedef struct evfs_file_uri_path evfs_file_uri_path;
struct evfs_file_uri_path {
	int num_files;
	evfs_filereference** files;
};

/*Command structures*/
typedef enum evfs_command_type
{
  EVFS_CMD_STARTMON_FILE = 1 
}
evfs_command_type;


typedef struct evfs_command_file {
	evfs_command_type type;
	int num_files;
	evfs_filereference** files;
}
evfs_command_file;

typedef union evfs_command {
	evfs_command_type type;
	evfs_command_file file_command;
}
evfs_command;

/*-----------*/


typedef struct evfs_client evfs_client;
struct evfs_client {
        Ecore_Ipc_Client* client;
        evfs_command* prog_command;
	unsigned long id;
};



typedef struct evfs_file_monitor evfs_file_monitor;
struct evfs_file_monitor {
	evfs_client* client;
	char* monitor_path;
};


/*Event structures*/
typedef enum evfs_eventtype {
	EVFS_EV_REPLY = 1,	
	EVFS_EV_NOTIFY_ID = 2	
} evfs_eventtype;

typedef enum evfs_eventtype_sub {
	EVFS_EV_SUB_MONITOR_NOTIFY = 1
} evfs_eventtype_sub;

typedef enum evfs_eventpart {
	EVFS_EV_PART_TYPE = 1,
	EVFS_EV_PART_SUB_TYPE = 2,
	EVFS_EV_PART_DATA = 3,
	EVFS_EV_PART_END = 1000
} evfs_eventpart;

typedef struct evfs_event evfs_event;
struct evfs_event {
	evfs_eventtype type;
	evfs_eventtype_sub sub_type;
	void* data;
	int data_len;
};

typedef struct evfs_connection evfs_connection;
struct evfs_connection {
	Ecore_Ipc_Server* server;
	unsigned long id;
	void (*callback_func)(void* data);
	evfs_event* prog_event;
};


void evfs_cleanup_client(evfs_client* client);
void evfs_disconnect(evfs_connection* connection);
evfs_connection* evfs_connect(void (*callback_func)(void*));
evfs_file_uri_path* evfs_parse_uri(char* uri);
void evfs_handle_command(evfs_client* client, evfs_command* command);
void evfs_handle_monitor_start_command(evfs_client* client, evfs_command* command);
evfs_plugin* evfs_get_plugin_for_uri(char* uri_base);
unsigned long evfs_server_get_next_id(evfs_server* serve);



#include <evfs_commands.h>
#include <evfs_cleanup.h>
#include <evfs_io.h>
#include <evfs_new.h>
#include <evfs_event_helper.h>

#endif
