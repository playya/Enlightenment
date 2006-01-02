#ifndef __ENTROPY_GENERIC_H_
#define __ENTROPY_GENERIC_H_

#include "plugin_base.h"
#include "entropy_core.h"
#include "thumbnail_generic.h"

#define MIME_LENGTH 40
#define FILENAME_LENGTH 255

typedef struct entropy_generic_file entropy_generic_file;
struct entropy_generic_file {
        char path[255];
        char filename[FILENAME_LENGTH];
        char mime_type[MIME_LENGTH];
	char uri_base[15];
	
	char perms[10];
	char filetype;

	char* username;  /*Do we have a cached auth reference for this file/location? */
	char* password;

	entropy_thumbnail* thumbnail;

	struct entropy_generic_file* parent;

	char retrieved_stat;
	struct stat properties;

	char* md5; /*A reference to the md5sum made for this file*/
};

typedef struct entropy_file_listener entropy_file_listener;
struct entropy_file_listener {
	entropy_generic_file* file;
	int count;
};

typedef struct entropy_file_request entropy_file_request;
struct entropy_file_request {
	entropy_generic_file* file;
	entropy_generic_file* file2;
	entropy_generic_file* reparent_file;
	
	entropy_core* core; /*A reference into the system core */
	void* requester; /*The object that requested this file/directory, for reference tracking purposes*/
	int file_type;
	int drill_down; /*Indicate if this request should drill down through the child's mime type
			  I.e. if the file's uri is 'posix', and the file's mime is 'application/x-tar,
			  produce a construct of the form posix://path/to/file#tar:/// ..etc */

	int set_parent;
	int value;
};


#define TYPE_CONTINUE 0
#define TYPE_END 1 
/*A temporary structure until I find a better way to do this*/
typedef struct entropy_file_progress {
	char* file_from;
	char* file_to;

	float progress;
	int type;
} entropy_file_progress;

typedef struct entropy_file_stat entropy_file_stat;
struct entropy_file_stat {
	entropy_generic_file* file;
	struct stat* stat_obj;
};

	

enum entropy_generic_file_type {
	FILE_ALL,
	FILE_UNKNOWN,
	FILE_STANDARD,
	FILE_FOLDER
};

#endif
