
/*

Copyright (C) 2000, 2001 Alexander Taylor <alex@logisticchaos.com>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "evfs.h"

#include <Ecore_File.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

static evfs_server* server;

evfs_client* evfs_client_get(Ecore_Ipc_Client* client) {
	return ecore_hash_get(server->client_hash, client);
}

evfs_plugin* evfs_get_plugin_for_uri(char* uri_base) {
	return ecore_hash_get(server->plugin_uri_hash, uri_base);
}

unsigned long evfs_server_get_next_id(evfs_server* serve) {
	serve->clientCounter++;
	printf("Allocated %ld\n", serve->clientCounter-1);
	return serve->clientCounter-1;
}

int
ipc_client_add(void *data, int type, void *event)
{
      Ecore_Ipc_Event_Client_Add *e;
      evfs_client* client;


      e = (Ecore_Ipc_Event_Client_Add *) event;
      /*printf("ERR: EVFS Client Connected!!!\n");*/

      client = NEW(evfs_client);
      client->client = e->client;
      client->prog_command = NULL;
      client->id = evfs_server_get_next_id(server);
      ecore_hash_set(server->client_hash, client->client, client);

      server->num_clients++;

      evfs_event_client_id_notify(client);

      
      return (1);
}

int
ipc_client_del(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Client_Del *e;
   evfs_client* client;


   e = (Ecore_Ipc_Event_Client_Del *) event;

    client = ecore_hash_get(server->client_hash, e->client);
    printf("Client %ld, Client Disconnected!!!\n", client->id);
    ecore_hash_remove(server->client_hash, client);
    evfs_cleanup_client(client);

    
   return (1);
}


int
ipc_client_data(void *data, int type, void *event)
{
	
   Ecore_Ipc_Event_Client_Data *e = (Ecore_Ipc_Event_Client_Data*) event;
   evfs_client* client;
   
   ecore_ipc_message* msg = ecore_ipc_message_new(e->major,e->minor,e->ref,e->ref_to,e->response,e->data,e->size);


   client = evfs_client_get(e->client);

   
   if (!client->prog_command) {
	   client->prog_command = evfs_command_new();
   }

   /*True == command finished*/
   if (evfs_process_incoming_command(client->prog_command, msg)) {
	  evfs_handle_command(client, client->prog_command);
	  

	  evfs_cleanup_command(client->prog_command, EVFS_CLEANUP_FREE_COMMAND); 
	  client->prog_command = NULL;
   }



   return 1;
}


void evfs_handle_command(evfs_client* client, evfs_command* command) {
	switch (command->type) {
		case EVFS_CMD_STARTMON_FILE:
			/*printf("We received a monitor stop request*/
			evfs_handle_monitor_start_command(client, command);
			
			break;
		case EVFS_CMD_STOPMON_FILE:
			/*printf("We received a monitor stop request\n");*/
			evfs_handle_monitor_stop_command(client,command);
			break;

		case EVFS_CMD_RENAME_FILE:
			evfs_handle_file_rename_command(client,command);
			break;

		case EVFS_CMD_MOVE_FILE:
			printf("Move file stub\n");
			break;
		case EVFS_CMD_REMOVE_FILE: 
			evfs_handle_file_remove_command(client,command);
			break;
		case EVFS_CMD_FILE_STAT:
			evfs_handle_file_stat_command(client,command);
			break;
		case EVFS_CMD_LIST_DIR:
			evfs_handle_dir_list_command(client,command);
			printf("List directory stub\n");
			break;
		case EVFS_CMD_FILE_COPY:
			printf("File copy handler\n");
			evfs_handle_file_copy(client,command);
			break;
		default: printf("Warning - unhandled command %d\n", command->type);
			 break;
	}
}



evfs_plugin* evfs_load_plugin(char* filename) {
	evfs_plugin* plugin = NEW(evfs_plugin);

	char* (*evfs_plugin_uri_get)();
	evfs_plugin_functions* (*evfs_plugin_init)();

	printf("Loading plugin: %s\n", filename);	
	plugin->dl_ref = dlopen(filename, RTLD_LAZY);

	if (plugin->dl_ref) {
		evfs_plugin_uri_get = dlsym(plugin->dl_ref, "evfs_plugin_uri_get");
		if (evfs_plugin_uri_get) {
			plugin->uri = (*evfs_plugin_uri_get)();			
			printf("The plugin at '%s' handles '%s'\n", filename, plugin->uri);

			/*Execute the init function, if it's there..*/
			evfs_plugin_init = dlsym(plugin->dl_ref, "evfs_plugin_init");	
			if (evfs_plugin_init) {
				plugin->functions = (*evfs_plugin_init)();
			}
		} else {
			printf("Error - plugin file does not contain uri identify function - %s\n", filename);
			goto exit_error;
		}
			
	} else {
		printf("Error - plugin file invalid - %s\n", filename);
		goto exit_error;
	}


	

	return plugin;


	exit_error:
		free(plugin);
		return NULL;
	
	
}


void evfs_load_plugins() {
        struct dirent* de;
        DIR* dir;
	evfs_plugin* plugin;
	char plugin_path[1024];

	printf("Reading plugins from: %s\n", PACKAGE_PLUGIN_DIR "/plugins/file");
        dir = opendir(PACKAGE_PLUGIN_DIR "/plugins/file");
        if (dir) {
		while ( (de = readdir(dir)) ) {

		   if (!strncmp(de->d_name + strlen(de->d_name) -3, ".so", 3)) {
			snprintf(plugin_path, 1024,"%s/%s", PACKAGE_PLUGIN_DIR "/plugins/file", de->d_name);
			if ( (plugin = evfs_load_plugin(plugin_path))) {
				ecore_hash_set(server->plugin_uri_hash, plugin->uri, plugin);
			}
		   }
		}
	} else {
		fprintf(stderr, "EVFS: Could not location plugin directory '%s'\n", PACKAGE_PLUGIN_DIR "/plugins/file");
		exit(1);
	}

}



int main(int argc, char** argv) {
	/*Init the ipc server*/
	if (ecore_ipc_init() < 1)
		return (1);

	ecore_file_init();

	
	server = NEW(evfs_server);
	server->client_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	server->plugin_uri_hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	server->clientCounter = 0;

	/*Load the plugins*/
	evfs_load_plugins();




	if ((server->ipc_server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_USER, EVFS_IPC_TITLE, 0, NULL))) {
	      ecore_ipc_server_del(server->ipc_server);
	      free(server);
	      printf ("ERR: Server already running...\n");
	      return (1);
	} else {
	      //printf ("ERR: Server created..\n");

	      server->ipc_server = ecore_ipc_server_add(ECORE_IPC_LOCAL_USER, EVFS_IPC_TITLE, 0, NULL);

	      ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD, ipc_client_add,
                 NULL);
	      ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL, ipc_client_del,
                 NULL);
	       ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA, ipc_client_data,
                 NULL);
	}


	ecore_main_loop_begin();
	
	return 0;
}
