/*

Copyright (C) 2005 John Kha

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

#include <evfs.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <Ecore_File.h>
#include <curl/curl.h>

CURLcode* connection_handle_get(evfs_filereference* ref);

void ftp_evfs_dir_list(evfs_client* client, evfs_command* command);
int ftp_evfs_file_stat(evfs_command* command, struct stat* file_stat);
int evfs_file_open(evfs_client* client, evfs_filereference* file);
int evfs_file_close(evfs_filereference* file);
int evfs_file_seek(evfs_filereference* file, long offset, int whence);
int evfs_file_read(evfs_client* client, evfs_filereference* file, char* bytes, long size);
int evfs_file_write(evfs_filereference* file, char* bytes, long size);
int evfs_file_create(evfs_filereference* file);
int evfs_client_disconnect(evfs_client* client);

typedef enum evfs_ftp_data{
	EVFS_FTP_CLIENT,
	EVFS_FTP_COMMAND
}evfs_ftp_data;

/****************Globals****************/
Ecore_Hash* connections;


/******************Begin Internal Functions*******************************/

CURLcode* connection_handle_get(evfs_filereference* ref)
{
	CURLcode* conn_handle;

	/*Check for an existing connection, return it if  avaliable
		create/initialize one if not*/
	if(!(conn_handle = ecore_hash_get(connections, ref->path)))
	{
		char *url;
		int len = 1;
		len += strlen("ftp://");
		len += strlen(ref->username) + 1 + strlen(ref->password);
		len += 1 + strlen(ref->path);
		len *= sizeof(char);
		url = malloc(len);
		snprintf(url,len,"ftp://%s:%s@%s", ref->username, ref->password, ref->path);
		conn_handle = curl_easy_init();
		printf("Setting CURLOPT_URL to %s\n", url);
		curl_easy_setopt(conn_handle, CURLOPT_URL, url);
		ecore_hash_set(connections, ref->path, conn_handle);
	}
	
	return conn_handle;
}
/******************CURL Callbacks*****************************************/

size_t listdir(void *buffer, size_t size, size_t nmemb, void *cbdata)
{
	char* dirs = buffer;
	Ecore_List* files = ecore_list_new();
	evfs_client* client = ecore_hash_get((Ecore_Hash *)cbdata, (int *)EVFS_FTP_CLIENT);
	evfs_command* command = ecore_hash_get((Ecore_Hash *)cbdata, (int *)EVFS_FTP_COMMAND);
	
	evfs_filereference* ref;
	char* curline;
	
	printf("libcurl says, \"I have returned %s\"\n", buffer);
	
	while (dirs)
	{
		curline = strdup(strsep(&dirs, "\r\n"));
		if(strlen(curline))
		{
			ref = NEW(evfs_filereference);
			printf("dir: %s\n", curline);
			ref->path = strdup(curline);
			ref->file_type = EVFS_FILE_NORMAL;
			ecore_list_append(files, ref);
		}
	}
	printf("There are %i list nodes.\n", ecore_list_nodes(files));
	ref = NEW(evfs_filereference);
	ecore_list_goto_first(files);
	while ((ref = (evfs_filereference *)ecore_list_next(files)))
	{
		printf("DIR: %s\n", ref->path);
	}
	evfs_list_dir_event_create(client, command, files);
	return strlen(buffer);
}


/******************Begin EVFS Plugin Functions****************************/

evfs_plugin_functions* evfs_plugin_init() {
	int err;
	
	
	printf("Initialising the ftp plugin..\n");
	
	/*Set up callbacks for the evfs server*/
	evfs_plugin_functions* functions = malloc(sizeof(evfs_plugin_functions));
	functions->evfs_dir_list = &ftp_evfs_dir_list;
	functions->evfs_file_stat = &ftp_evfs_file_stat;
	functions->evfs_file_open = &evfs_file_open;
	functions->evfs_file_close = &evfs_file_close;
	functions->evfs_file_seek = &evfs_file_seek;
	functions->evfs_file_read = &evfs_file_read;
	functions->evfs_file_write = &evfs_file_write;
	functions->evfs_file_create = &evfs_file_create;
	
	functions->evfs_client_disconnect = &evfs_client_disconnect;
	
	/*initialize the connection cache*/
	connections = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	
	printf("Functions at '%p'\n", &functions);

	return functions;

	
}

char* evfs_plugin_uri_get() {
	return "ftp";
}

void ftp_evfs_dir_list(evfs_client* client, evfs_command* command) {
	printf("FTP: Listing dir.\n");
	Ecore_Hash* data = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	
	ecore_hash_set(data, (int*)EVFS_FTP_CLIENT, client);
	ecore_hash_set(data, (int*)EVFS_FTP_COMMAND, command);
	CURLcode* handle = connection_handle_get(command->file_command.files[0]);
	//curl_easy_setopt(handle, CURLOPT_FTPLISTONLY, 1);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, (FILE *)data);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, listdir);
	printf("Executing curl_easy_perform()...\n");
	curl_easy_perform(handle);
}

int ftp_evfs_file_stat(evfs_command* command, struct stat* file_stat) {
	
	return 0;
}


int evfs_file_open(evfs_client* client, evfs_filereference* file) {

	return 0;
}

int evfs_file_close(evfs_filereference* file) {
	
	return 0;
}

int evfs_file_seek(evfs_filereference* file, long pos, int whence) {
	
	return 0;
}

int evfs_file_read(evfs_client* client, evfs_filereference* file, char* bytes, long size) {
	
	return 0;
}

int evfs_file_write(evfs_filereference* file, char* bytes, long size) {
	
	return 0;
}

int evfs_file_create(evfs_filereference* file) {
	
	return 0;
}

int evfs_client_disconnect(evfs_client* client)
{
	/*Temp, move to plugin unloading when avaliable.  Closes the curl library, and
		resets the connection cache.  This will just destroy the cache when moved.*/
	printf ("Received disconnect for client at evfs_fs_ftp.c for client %d\n", client->id);
	curl_global_cleanup();
	ecore_hash_destroy(connections);
	ecore_hash_new(ecore_str_hash, ecore_str_compare);
}
