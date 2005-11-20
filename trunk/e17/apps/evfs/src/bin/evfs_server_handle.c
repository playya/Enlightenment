#include "evfs.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>


/*---------------------------------------------------*/
/*Move these functions somewhere*/

int evfs_uri_open(evfs_client* client, evfs_filereference* uri) {
	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server,uri->plugin_uri);
	if (plugin) {
		printf("Opening file..\n");
		return (*plugin->functions->evfs_file_open)(client, uri);		
	} else {
		printf("Could not get plugin for uri '%s' at evfs_uri_open\n", uri->plugin_uri);
		return -1;
	}
}

int evfs_uri_close(evfs_client* client, evfs_filereference* uri) {
	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server,uri->plugin_uri);
	if (plugin) {
		printf("Closing file..\n");
		return (*plugin->functions->evfs_file_close)(uri);		
	} else {
		printf("Could not get plugin for uri '%s' at evfs_uri_close\n", uri->plugin_uri);
		return -1;
	}

}
		

int evfs_uri_read(evfs_client* client, evfs_filereference* uri, char* bytes, long size) {
	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server,uri->plugin_uri);
	if (plugin) {
		return (*plugin->functions->evfs_file_read)(client,uri,bytes,size);		
	} else {
		printf("Could not get plugin for uri '%s' at evfs_uri_open\n", uri->plugin_uri);
		return -1;
	}
}

/*---------------------------------------------------*/








void evfs_handle_monitor_start_command(evfs_client* client, evfs_command* command) {
	/*First get the plugin responsible for this file*/


	if (command->file_command.num_files > 0) {
		evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server,command->file_command.files[0]->plugin_uri);

		if (!plugin) {
			printf("No plugin able to monitor this uri type\n");
		} else {
			printf("Requesting a file monitor from this plugin for uri type '%s'\n", command->file_command.files[0]->plugin_uri );
			(*plugin->functions->evfs_monitor_start)(client,command);
			
		}
	} else {
		printf("No files to monitor!\n");
	}
}

void evfs_handle_monitor_stop_command(evfs_client* client, evfs_command* command) {
	/*First get the plugin responsible for this file*/

	void (*evfs_monitor_start)(evfs_client* client, evfs_command* command);

	if (command->file_command.num_files > 0) {
		evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);

		if (!plugin) {
			printf("No plugin able to monitor this uri type\n");
		} else {
			printf("Requesting a file monitor end from this plugin for uri type '%s'\n", command->file_command.files[0]->plugin_uri );
			
			evfs_monitor_start = dlsym(plugin->dl_ref, EVFS_FUNCTION_MONITOR_STOP);
			if (evfs_monitor_start) {
				(*evfs_monitor_start)(client,command);
			}
			
		}
	} else {
		printf("No files to monitor!\n");
	}
}

void evfs_handle_file_remove_command(evfs_client* client, evfs_command* command) {
	printf("At remove handle\n");

	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);
	if (plugin) {
		printf("Pointer here: %p\n", plugin->functions->evfs_file_remove);
		(*plugin->functions->evfs_file_remove)(command->file_command.files[0]->path);
	}
}


void evfs_handle_file_rename_command(evfs_client* client, evfs_command* command) {
	printf("At rename handle\n");

	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);
	if (plugin) {
		printf("Pointer here: %p\n", plugin->functions->evfs_file_rename);
		(*plugin->functions->evfs_file_rename)(client,command);
	}
}

void evfs_handle_file_stat_command(evfs_client* client, evfs_command* command) {
	struct stat file_stat;
	
	printf ("At file stat handler\n");
	printf("Looking for plugin for '%s'\n", command->file_command.files[0]->plugin_uri);
	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);
	if (plugin) {
		printf("Pointer here: %p\n", plugin->functions->evfs_file_stat);
		(*(plugin->functions->evfs_file_stat))(command, &file_stat);

		

		evfs_stat_event_create(client, command, &file_stat);
	}
	printf("Handled event, client is %p\n", client);

}


void evfs_handle_dir_list_command(evfs_client* client, evfs_command* command) {

	
	evfs_plugin* plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);
	if (plugin) {
		Ecore_List* directory_list = NULL;
		(*plugin->functions->evfs_dir_list)(client,command, &directory_list);

		if (directory_list) {
			evfs_list_dir_event_create(client, command, directory_list);
		} else {
			printf("evfs_handle_dir_list_command: Recevied null from plugin for directory_list\n");
		}
	} else {
		printf("No plugin for '%s'\n", command->file_command.files[0]->plugin_uri);
	}

}

void evfs_handle_file_copy(evfs_client* client, evfs_command* command) {
	evfs_plugin* plugin;
	evfs_plugin* dst_plugin;
	
	char bytes[COPY_BLOCKSIZE];
	long count;
	char destination_file[PATH_MAX];
	long read_write_bytes = 0;
	struct stat file_stat;
	int progress = 0;
	int last_notify_progress = 0;
	evfs_filereference* ref = NEW(evfs_filereference);

 	plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[0]->plugin_uri);
	dst_plugin = evfs_get_plugin_for_uri(client->server, command->file_command.files[1]->plugin_uri);
	
	if (plugin && dst_plugin) {


		/*Get the source file size*/
		(*plugin->functions->evfs_file_stat)(command, &file_stat);
		//printf("Source file size: %d bytes\n", (int)file_stat.st_size);
		
		
		if (!S_ISDIR(file_stat.st_mode)) {
			(*dst_plugin->functions->evfs_file_create)(command->file_command.files[1]);
			(*plugin->functions->evfs_file_open)(client, command->file_command.files[0]);
			
			count = 0;
			while (count < file_stat.st_size) {
				(*plugin->functions->evfs_file_seek)(command->file_command.files[0], count, SEEK_SET);

				read_write_bytes = (file_stat.st_size > count + COPY_BLOCKSIZE) ? COPY_BLOCKSIZE : (file_stat.st_size - count);
				/*printf("Reading/writing %d bytes\n", read_write_bytes);*/
			
				(*plugin->functions->evfs_file_read)(client,command->file_command.files[0], bytes, read_write_bytes );
				(*dst_plugin->functions->evfs_file_write)(command->file_command.files[1], bytes, read_write_bytes );
		
				progress = (double)((double)count / (double)file_stat.st_size * 100);
				if (progress % 1 == 0 && last_notify_progress < progress) {
					/*printf ("Percent complete: %d\n", progress);*/
					evfs_file_progress_event_create(client,command,progress, EVFS_PROGRESS_TYPE_CONTINUE);
					last_notify_progress = progress;
				}
			

				count+= COPY_BLOCKSIZE;


	
				/*Iterate*/
				ecore_main_loop_iterate();
			}
			(*dst_plugin->functions->evfs_file_close)(command->file_command.files[1]);
			(*plugin->functions->evfs_file_close)(command->file_command.files[0]);
		} else {
			Ecore_List* directory_list = NULL;
			
			/*First, we need a directory list...*/
			(*plugin->functions->evfs_dir_list)(client,command,&directory_list);
			if (directory_list) {
				int ret =0;
						
				/*OK, so the directory exists at the source, and contains files.
				 * Let's make the destination directory first..*/
				printf("Making new directory '%s'", command->file_command.files[1]->path);
				ret=  (*dst_plugin->functions->evfs_file_mkdir)(command->file_command.files[1]);
				printf("....ret was %d\n", ret);
				
				evfs_filereference* file = NULL;
				//printf("Recursive directory list for '%s' received..\n", command->file_command.files[0]->path);
				while ((file = ecore_list_remove_first(directory_list))) {
					evfs_filereference* source = NEW(evfs_filereference);
					evfs_filereference* dest = NEW(evfs_filereference);
					evfs_command* recursive_command = NEW(evfs_command);

					
					
					snprintf(destination_file, PATH_MAX, "%s%s", 
						command->file_command.files[1]->path, strrchr(file->path, '/'));

					source->path = strdup(file->path);
					source->plugin_uri = strdup(command->file_command.files[0]->plugin_uri);
					source->parent = NULL; /*TODO - handle nested uris*/
					dest->path = strdup(destination_file);
					dest->plugin_uri = strdup(command->file_command.files[1]->plugin_uri);
					dest->parent = NULL; /*TODO - handle nested uris*/

					recursive_command->file_command.files = malloc(sizeof(evfs_filereference*)*2);
					recursive_command->type = EVFS_CMD_FILE_COPY;
					recursive_command->file_command.files[0] = source;
					recursive_command->file_command.files[1] = dest;
					recursive_command->file_command.num_files = 2;
					
					//printf("Copy file '%s' to %s\n", file->path, destination_file);

					evfs_handle_file_copy(client, recursive_command);
					
					evfs_cleanup_filereference(file);
					evfs_cleanup_command(recursive_command, 1);
				}
				ecore_list_destroy(directory_list);
				
			}
		
			
		}
		
		

		

		evfs_file_progress_event_create(client,command,100, EVFS_PROGRESS_TYPE_DONE);
		
	} else {
		printf("Could not get plugins for both source and dest: (%s:%s)\n",command->file_command.files[0]->plugin_uri, command->file_command.files[1]->plugin_uri );
	}

}
