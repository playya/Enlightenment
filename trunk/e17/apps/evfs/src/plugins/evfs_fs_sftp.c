/*
 * vim:ts=3:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*

   Copyright (C) 2005 Alex Taylor

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
#include "sftp.h"

typedef struct {
	char* filename;
	int type;
} evfs_file_info;

static long sftp_file_request_id = 1;
static long sftp_open_handle_id = 1;
static Ecore_Hash* sftp_open_handles = NULL;

/*----------------------------------------------------------------*/
#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#  define GNUC_EXTENSION __extension__
#else
#  define GNUC_EXTENSION
#endif

GNUC_EXTENSION typedef signed long long int64;
GNUC_EXTENSION typedef unsigned long long uint64;



#define INT64_CONSTANT(val)	(GNUC_EXTENSION (val##LL))

#define INT32_TO_BE(val)	((unsigned int) ( \
    (((unsigned int) (val) & (unsigned int) 0x000000ffU) << 24) | \
    (((unsigned int) (val) & (unsigned int) 0x0000ff00U) <<  8) | \
    (((unsigned int) (val) & (unsigned int) 0x00ff0000U) >>  8) | \
    (((unsigned int) (val) & (unsigned int) 0xff000000U) >> 24)))

#define UINT64_TO_BE(val)	((uint64) ( \
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x00000000000000ffU)) << 56) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x000000000000ff00U)) << 40) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x0000000000ff0000U)) << 24) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x00000000ff000000U)) <<  8) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x000000ff00000000U)) >>  8) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x0000ff0000000000U)) >> 24) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0x00ff000000000000U)) >> 40) |	\
      (((uint64) (val) &						\
	(uint64) INT64_CONSTANT (0xff00000000000000U)) >> 56)))


int
sftp_request_id_get_next()
{
   sftp_file_request_id++;
   return sftp_file_request_id;
}

long
sftp_open_handle_get_next()
{
   sftp_open_handle_id++;
   return sftp_open_handle_id;
}


////////////////////
int read_int32(char** c) {
	int val;
	
	memcpy(&val, *c, 4);
	val = INT32_TO_BE(val);
	*c += 4; /*Jump identifier*/

	return val;
}

long read_int64(char** c) {
	long val;
	
	//memcpy(&val, *c, 8);
	//val = INT32_TO_BE(val);
	*c += 8; /*Jump identifier*/

	return val;
}


char read_char(char** c)
{
	char val;
	memcpy(&val, *c, 1);
	*c += 1;
	return val;
}

char* read_string(char** c, int* len) {
	int length = read_int32(c);
	char* str = malloc(length+1);

	//printf("Reading string of length %d\n", length);

	memcpy(str, *c, length);
	str[length] = '\0';
	*c += length;
	*len = length;

	//printf("String: '%s'\n", str);

	return str;
}

void read_sftp_attr(char** c, evfs_file_info** info) {
	int flags;
	int id;
	char type;
	
	/*Read the flags*/
	flags = read_int32(c);

	//printf("Attribute flags: %d\n", flags);

	/*printf("Attr type is %d\n", (int)type);*/
	
	if (flags & SSH2_FILEXFER_ATTR_SIZE) {
		long size = read_int64(c);
		//printf ("Size: %d\n", );
	}

	if (flags & SSH2_FILEXFER_ATTR_UIDGID) {
		read_int32(c);
		read_int32(c);
	}

	if (flags & SSH2_FILEXFER_ATTR_PERMISSIONS) {
		int perms = read_int32(c);

		if (S_ISREG(perms)) (*info)->type = EVFS_FILE_NORMAL;
		else if (S_ISDIR(perms)) (*info)->type = EVFS_FILE_DIRECTORY;
		else (*info)->type = EVFS_FILE_NORMAL;
	}

	if (flags & SSH2_FILEXFER_ATTR_ACMODTIME) {
		read_int32(c);
		read_int32(c);
	}
	if (flags & SSH2_FILEXFER_ATTR_EXTENDED) printf("Attr: Extended\n");
}

#define INIT_BUFFER_ALLOC   128
Ecore_Hash* sftp_connection_hash;

typedef struct
{
	char  *base;
	char  *read_ptr;
	char  *write_ptr;
	int     alloc;
} Buffer;

typedef struct 
{
	char* host;
	int version;
	int reference_count;
	int status;
	Ecore_Exe* ssh_conn;
	Ecore_Hash* handle_hash;
	Ecore_Hash* id_open_hash;

	char* packet_cache;
	int packet_cache_size;
	int packet_length;
} SftpConnection;

typedef struct 
{
	char                *sftp_handle;
	int                  sftp_handle_len;
	
	uint64    offset;
	uint64                 info_alloc;
	uint64                info_read_ptr;
	uint64                info_write_ptr;

	long int_id;

	SftpConnection* conn;
} SftpOpenHandle;

typedef enum {
	READ_PROGRESS,
	READ_FINISHED
} SftpReadStatus;

typedef struct 
{
	SftpOpenHandle* open_handle;
	Ecore_List* file_list;
	int status;
} SftpReadDirHandle;


typedef enum
{
	SFTP_INIT,
	SFTP_CONNECTED,
	SFTP_CLOSED
} sftp_connection_status;





static void
buffer_init (Buffer *buf) 
{
	buf->base = calloc (INIT_BUFFER_ALLOC, sizeof(char));
	buf->read_ptr = buf->base + sizeof (unsigned int);
	buf->write_ptr = buf->base + sizeof (unsigned int);
	buf->alloc = INIT_BUFFER_ALLOC;
}

static int 
buffer_send (Buffer *buf, Ecore_Exe* fd) 
{
	unsigned int bytes_written = 0;
	unsigned int len = buf->write_ptr - buf->read_ptr;
	unsigned int w_len = INT32_TO_BE(len);
	int res=0;

	printf("Writing %d bytes\n", len);

	buf->read_ptr -= sizeof (int);

	*((int *) buf->read_ptr) = w_len;

	char* c = buf->read_ptr;
	int i = 0;
	/*for (i=0;i<9;i++) {
		printf("Char %d: %d\n", i, (int)(*(c+i)));
	}*/

	//if ((bytes_written = atomic_io ((read_write_fn) write, fd, buf->read_ptr,
	//				buf->write_ptr - buf->read_ptr)) < 0)
	//{
	if (!ecore_exe_send(fd, buf->read_ptr, buf->write_ptr - buf->read_ptr)) {
		//g_critical ("Could not write entire buffer: %s", g_strerror (errno));
		//res = GNOME_VFS_ERROR_IO;
		printf("Could not write to buffer!\n");
	} else {
		bytes_written = buf->write_ptr - buf->read_ptr;
	
		printf("Sent %d bytes to client\n", bytes_written);
		
		if (bytes_written == buf->write_ptr - buf->read_ptr)
			buf->read_ptr = buf->write_ptr = buf->base + sizeof (unsigned int);
		else
			buf->read_ptr += bytes_written;

	}

	return res;
}


static void
buffer_check_alloc (Buffer *buf, int size)
{
	int r_len, w_len;

	while (buf->write_ptr - buf->base + size > buf->alloc) {
		buf->alloc *= 2;
		r_len = buf->read_ptr - buf->base;
		w_len = buf->write_ptr - buf->base;
		buf->base = realloc (buf->base, buf->alloc);
		buf->read_ptr = buf->base + r_len;
		buf->write_ptr = buf->base + w_len;
	}
}

void buffer_write(Buffer* buf, const void* data, int size)
{
	int s = 0;
	buffer_check_alloc (buf, size);
	memcpy (buf->write_ptr, data, size);
	buf->write_ptr += size;
	//printf("Moved write ptr to %d\n", s);
}

void buffer_write_char(Buffer* buf, char data) 
{
	buffer_write(buf, &data, sizeof(char));
}

void buffer_write_int(Buffer* buf, int data)
{
	int w_data = INT32_TO_BE(data);
	buffer_write(buf, &w_data, sizeof(int));
}

void buffer_write_long(Buffer* buf, uint64 data)
{
	uint64 w_data = UINT64_TO_BE(data);
	printf("Post-be-long::::::: %ld\n", w_data);
	buffer_write(buf, &w_data, sizeof(uint64));
}

void buffer_write_block(Buffer* buf, const char* ptr, int len)
{
	buffer_write_int(buf,len);
	buffer_write(buf,ptr,len);
}


void buffer_write_string(Buffer* buf, const char* data)
{
	int len;
	len = strlen(data);
	buffer_write_block(buf,data,len);
}

void buffer_write_attributes(Buffer* buf)
{
	buffer_write_int(buf, 0);
}

#define SSH_PROGRAM "/usr/bin/ssh"
typedef enum {
	SFTP_VENDOR_INVALID = 0,
	SFTP_VENDOR_OPENSSH,
	SFTP_VENDOR_SSH
} SFTPClientVendor;

int
sftp_file_open(SftpConnection* conn, char* filename, int attr)
{
	Buffer msg;
	int nid;

	nid = sftp_request_id_get_next();
	buffer_init(&msg);
	buffer_write_char(&msg, SSH2_FXP_OPEN);
	buffer_write_int(&msg, nid);
	buffer_write_string(&msg, filename);
	buffer_write_int(&msg, SSH2_FXF_READ | SSH2_FXF_WRITE | SSH2_FXF_CREAT);
	
	buffer_write_attributes(&msg);
	buffer_send(&msg, conn->ssh_conn);

	return nid;
	
}

int
sftp_file_write(SftpOpenHandle* handle, char* bytes, int size)
{
	Buffer msg;
	int nid;

	nid = sftp_request_id_get_next();
	buffer_init(&msg);

	/*Write type*/
	buffer_write_char(&msg, SSH2_FXP_WRITE);

	/*Write id*/
	buffer_write_int(&msg, nid);

	/*Write handle*/
	buffer_write_block(&msg, handle->sftp_handle, handle->sftp_handle_len);

	/*Write offset*/
	printf("   [*****] Write offset %ld\n", handle->info_write_ptr);
	buffer_write_long(&msg, handle->info_write_ptr);
	handle->info_write_ptr += size; /*FIXME do we want to update it here, and assume
					  the server has written the bytes? */

	buffer_write_block(&msg, bytes, size);

	buffer_send(&msg, handle->conn->ssh_conn);
	
}


int sftp_open_dir(SftpConnection* conn, char* dir)
{
	Buffer msg;
	int nid;

	nid = sftp_request_id_get_next();
	
	buffer_init(&msg);
	buffer_write_char(&msg, SSH2_FXP_OPENDIR);
	buffer_write_int(&msg,  nid);
	buffer_write_string(&msg, dir);
	buffer_send(&msg, conn->ssh_conn);

	return nid;
}

SftpReadDirHandle* sftp_read_dir(SftpConnection* conn, SftpOpenHandle* handle, SftpReadDirHandle* read)
{
	SftpReadDirHandle *rhandle = NULL;
	
	if (!read) {
		rhandle = NEW(SftpReadDirHandle);

		rhandle->open_handle = handle;
		rhandle->file_list = ecore_list_new();
		rhandle->status = READ_PROGRESS;
	} else {
		rhandle = read;
	}
	
	printf("Sending readdir..len %d...\n\n\n\n", handle->sftp_handle_len);

	int id;

	id = sftp_request_id_get_next();
	printf("Read dir with id: %d\n", id);
	ecore_hash_set(conn->id_open_hash, (int*)id, rhandle);
	
	Buffer msg;
	buffer_init(&msg);
	buffer_write_char(&msg, SSH2_FXP_READDIR);
	buffer_write_int(&msg,  id);
	buffer_write_block(&msg, handle->sftp_handle, handle->sftp_handle_len);
	buffer_send(&msg, conn->ssh_conn);	

	return rhandle;
}


SftpConnection* sftp_connect(char* host);
SftpConnection* sftp_get_connection_for_host(char* host);

void sftp_read_handle(SftpConnection* conn, char **c) {
	int id, length;
	char* str;
	SftpOpenHandle* handle = NEW(SftpOpenHandle);
	
	printf("Reading a handle..\n");
	
	/*Read the identifier*/
	id = read_int32(c);

	/*Read the string*/
	str = read_string(c, &length);	
	handle->sftp_handle= str;
	handle->sftp_handle_len = length;

	printf("  [*] Reading handle with id %d, length %d\n", id, length);

	printf("  [*] Writing handle to hash (%p) , with id %d\n", handle, id);
	ecore_hash_set(conn->handle_hash, (int*)id, handle);
}

void sftp_read_names(SftpConnection* conn, char** c) {
	int id,count,i;
	SftpReadDirHandle *rhandle;
	
	/*Read the identifier*/
	id = read_int32(c);

	rhandle = ecore_hash_get(conn->id_open_hash, (int*)id);
	//printf("Names identifier: %d:%p\n", id, rhandle);	

	/*Read the count of entries in this list*/
	count =	read_int32(c); 

	/*printf("********* Grabbing %d files\n", count);*/

	char* offset = *c;
	for (i=0;i<count;i++) {
		/*printf("Offset current: %d:%d\n", (int)(*c - offset), i);*/
	
		int len;
		evfs_file_info* file;	
		char* name = read_string(c, &len);
		char* longname = read_string(c, &len);

		file = NEW(evfs_file_info);
		file->filename = name;

		/*Get the file attributes*/
		read_sftp_attr(c, &file);

		//printf("Name: %s\n", name);
		ecore_list_append(rhandle->file_list, file);

		free(longname);

		//printf("                    * File count: %d\n", i);
	}

	sftp_read_dir(conn, rhandle->open_handle,rhandle);
}

void sftp_handle_status(SftpConnection* conn, char** c) {
	int id;
	SftpReadDirHandle* rhandle;
	
	/*Read the identifier*/
	id = read_int32(c);

	printf("Got a status for id %d\n", id);
	rhandle = ecore_hash_get(conn->id_open_hash, (int*)id);

	printf("Rhandle is %p\n", rhandle);
	if (rhandle) rhandle->status = READ_FINISHED;
}



static int
sftp_exe_data(void *data, int type, void *event)
{
   Ecore_Exe_Event_Data *ev = event;
   char* c;
   int i;
   int length;
   char sftp_type;
   
   SftpConnection* conn = (SftpConnection*)ecore_exe_data_get(ev->exe);
   printf("  [*] DATA RET EXE %p - %p [%i bytes]\n", ev->exe, ev->data, ev->size);

   if (!conn->packet_cache) {
	char *d = ev->data;
   
   	length = read_int32(&d);
	conn->packet_length = length;
	/*printf("Init new packet, Length: %d\n", length);*/

   	conn->packet_cache = malloc(ev->size- sizeof(int) );
	conn->packet_cache_size = ev->size- sizeof(int) ;
	memcpy(conn->packet_cache, ev->data+ sizeof(int), ev->size- sizeof(int) );

	//printf("Max byte is %d %d %d\n", *((int*)ev->data+4094), *((int*)ev->data+4095), *((int*)ev->data+4096));
   } else {
   	/*printf("Continuing packet, making new size %d + %d = %d..\n", 
	conn->packet_cache_size, ev->size, conn->packet_cache_size + ev->size );*/

	conn->packet_cache = realloc(conn->packet_cache, conn->packet_cache_size + ev->size  );	
	memcpy(conn->packet_cache + conn->packet_cache_size, ev->data, ev->size);
	conn->packet_cache_size += ev->size;
   }

   /*printf("****** RATIOS %d:%d\n", conn->packet_cache_size, conn->packet_length);*/
  
   if (conn->packet_cache_size < conn->packet_length) {
   	/*printf("Incomplete packet! %d:%d\n", conn->packet_cache_size, conn->packet_length);*/
	return;
  	
   }

   c= conn->packet_cache;
   sftp_type = read_char(&c);

   switch (sftp_type) {
	   case SSH2_FXP_HANDLE:
		   printf ("  [*] TYPE: HANDLE: %d\n", sftp_type);
		   sftp_read_handle(conn, &c);
		   break;
	   case SSH2_FXP_STATUS:
		   printf ("  [*] TYPE: STATUS: %d\n",sftp_type);
		   sftp_handle_status(conn, &c);

		   break;
	   case SSH2_FXP_VERSION:
		   printf ("  [*] TYPE: VERSION: %d\n",sftp_type);
		   conn->status = SFTP_CONNECTED;
		   break;	 
	   case SSH2_FXP_NAME:
		   printf ("  [*] TYPE: NAME: %d\n", sftp_type);
		   sftp_read_names(conn, &c);
		   break;
	   default:
		   printf ("  [*] TYPE: UNKNOWN: %d\n", sftp_type);
		   break;
   }


   /*printf("        ********************* Freeing last packet ***\n");*/
   free(conn->packet_cache);
   conn->packet_cache = NULL;
   conn->packet_cache_size = 0;
   conn->packet_length = 0;
   /*printf("        ********************* Freed last packet *** \n");*/
   
}


/*----------------------------------------------------------------*/

void
evfs_dir_list(evfs_client * client, evfs_command * command, Ecore_List ** directory_list);
int evfs_file_open(evfs_client * client, evfs_filereference * file);
int evfs_file_close(evfs_filereference * file);
int evfs_file_seek(evfs_filereference * file, long offset, int whence);
int evfs_file_read(evfs_client * client, evfs_filereference * file,
                   char *bytes, long size);
int evfs_file_write(evfs_filereference * file, char *bytes, long size);
int evfs_file_create(evfs_filereference * file);
int evfs_file_remove(char *file);
int evfs_file_stat(evfs_command* command, struct stat *dst_stat, int);

int
evfs_client_disconnect(evfs_client * client)
{
   printf("Received disconnect for client at evfs_fs_sftp.c for client %d\n",
          client->id);
}

/*------------------------------------------------------------------------*/
evfs_plugin_functions *
evfs_plugin_init()
{
   int err;

   printf("Initialising the sftp plugin..\n");
   evfs_plugin_functions *functions = malloc(sizeof(evfs_plugin_functions));

   functions->evfs_dir_list = &evfs_dir_list;
   functions->evfs_client_disconnect = &evfs_client_disconnect;
   functions->evfs_file_open = &evfs_file_open;
   functions->evfs_file_create = &evfs_file_create;
   functions->evfs_file_close = &evfs_file_close;
   /*functions->evfs_file_seek = &evfs_file_seek;
   functions->evfs_file_read = &evfs_file_read;*/
   functions->evfs_file_write = &evfs_file_write;
   functions->evfs_file_stat = &evfs_file_stat;
   functions->evfs_file_lstat = &evfs_file_stat; 
   /*functions->evfs_file_mkdir = &smb_evfs_file_mkdir;
   functions->evfs_file_remove = &evfs_file_remove;
   */

   sftp_connection_hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   sftp_open_handles = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);

   /*Init the data receive functions*/
   ecore_event_handler_add(ECORE_EXE_EVENT_DATA, sftp_exe_data, NULL);
   ecore_event_handler_add(ECORE_EXE_EVENT_ERROR, sftp_exe_data, NULL);
   //ecore_event_handler_add(ECORE_EXE_EVENT_DEL, exe_exit, NULL);
   
   return functions;
}

char *
evfs_plugin_uri_get()
{
   return "sftp";
}

void sftp_split_host_path(char* input, char** host, char** path) 
{
	int host_len = strchr(input + 1, '/') - 1 - input;
	*host = malloc(host_len+1);
	strncpy(*host, input + 1, host_len);

	*path = strdup( input + 
			( strchr(input + 1, '/') -
			  input)) ;
}

int
evfs_file_create(evfs_filereference* file) 
{
	return evfs_file_open(NULL, file);
}

int
evfs_file_stat(evfs_command* command, struct stat *dst_stat, int i)
{
	return EVFS_ERROR;
}

int
evfs_file_open(evfs_client * client, evfs_filereference * file) 
{
	SftpConnection* conn;
	char* host, *path;
	int rid;
	SftpOpenHandle* handle;


	sftp_split_host_path(file->path, &host, &path);
	
	
	if ( !(conn = sftp_get_connection_for_host(host))) {
		conn = sftp_connect(host);
	}

	while (conn->status == SFTP_INIT) {
		ecore_main_loop_iterate();
		usleep(10);		
	}

	printf("Opening file '%s' with sftp..\n", file->path);
	rid = sftp_file_open(conn, path, 0);

	/*Wait till we have a handle*/
	/*FIXME - is there a better way of waiting-till-event in ecore?*/
	while (! (handle = ecore_hash_get(conn->handle_hash, (int*)rid))) {
		ecore_main_loop_iterate();
		usleep(10);
	}
	file->fd = sftp_open_handle_get_next();
	handle->int_id = file->fd;
	handle->conn = conn;
	ecore_hash_set(sftp_open_handles, (long*)file->fd, handle);

	printf("Opened!\n");

	return file->fd;
}

int evfs_file_close(evfs_filereference * file) {
	printf("SFTP_CLOSE: STUB\n");
}

int evfs_file_write(evfs_filereference * file, char *bytes, long size) {
	SftpOpenHandle* handle;


	handle = ecore_hash_get(sftp_open_handles, (long*)file->fd);
	if (handle) {
		sftp_file_write(handle, bytes, size);
	} else {
		printf("Could not find handle for write!\n");
	}
}

void
evfs_dir_list(evfs_client * client, evfs_command * command,
                  /*Returns.. */
                  Ecore_List ** directory_list)
{
	int rid;
	
	SftpOpenHandle* handle = NULL;
	SftpReadDirHandle* rhandle = NULL;
	evfs_file_info* file;
	
	char* host, *schar;
	SftpConnection* conn = NULL;

	sftp_split_host_path(command->file_command.files[0]->path, &host,&schar);

	printf("Original: %s\n", command->file_command.files[0]->path);
	printf("Listing directory '%s' on host '%s', using sftp\n", schar, host );

	if ( !(conn = sftp_get_connection_for_host(host))) {
		conn = sftp_connect(host);
	}

	while (conn->status == SFTP_INIT) {
		ecore_main_loop_iterate();
		usleep(10);		
	}


	/*Open directory*/
	rid = sftp_open_dir(conn, schar);

	/*Wait till we have a handle*/
	/*FIXME - is there a better way of waiting-till-event in ecore?*/
	while (! (handle = ecore_hash_get(conn->handle_hash, (int*)rid))) {
		ecore_main_loop_iterate();
		usleep(10);
	}
	
	printf("Time to send readdirs...\n");
	rhandle = sftp_read_dir(conn, handle,NULL);

	while (! (rhandle->status == READ_FINISHED)) {
		ecore_main_loop_iterate();
		usleep(10);
			
	}

	printf("Directory list finished!\n");

	*directory_list = ecore_list_new();
	while ( (file = ecore_list_remove_first(rhandle->file_list))) {
		evfs_filereference* ref = NEW(evfs_filereference);
		ref->path = malloc(strlen(host) + 1 + strlen(schar) + strlen(file->filename) + 2);
		snprintf(ref->path, strlen(host) + 1 + strlen(schar) + strlen(file->filename) + 2, "/%s%s/%s", host, schar, file->filename);
		ref->file_type = file->type;
		ref->plugin_uri = strdup("sftp");

		ecore_list_append(*directory_list, ref);

		/*Free the ref*/
		free(file->filename);
		free(file);

	}
	ecore_list_destroy(rhandle->file_list);

	free(host);

}

SftpConnection* sftp_get_connection_for_host(char* host) {
	return ecore_hash_get(sftp_connection_hash, host);
}


SftpConnection* sftp_connect(char* host) {
	Buffer msg;

	char connection_str[4096];
	SftpConnection* connection = NEW(SftpConnection);
	connection->handle_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	connection->id_open_hash = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
	connection->host = strdup(host);
	connection->status = SFTP_INIT;
	connection->packet_cache = NULL;
	connection->packet_cache_size = 0;
	connection->packet_length = 0;

	snprintf(connection_str, 4096, "/usr/bin/ssh -o ForwardX11=no -o ForwardAgent=no -o "
				       "ClearAllForwardings=yes -o Protocol=2 -o "
				       "NoHostAuthenticationForLocalhost=yes -o BatchMode=yes -s %s sftp", 
				       connection->host);

	connection->ssh_conn = ecore_exe_pipe_run(connection_str,
			ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE  | ECORE_EXE_PIPE_ERROR, connection);

	if (connection->ssh_conn) {

		ecore_hash_set(sftp_connection_hash, connection->host, connection);
	
		/*Say hello - send the welcome message!*/
		buffer_init(&msg);
		buffer_write_char(&msg, SSH2_FXP_INIT);
		buffer_write_int(&msg, SSH2_FILEXFER_VERSION);
		buffer_send(&msg, connection->ssh_conn);
		

		/*usleep(2000000);
		printf("Writing password...\n");*/
		
	} else {
		free(connection->host);
		free(connection);
		connection= NULL;
	}

	return connection;
}



