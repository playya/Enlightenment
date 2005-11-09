/*

Copyright (C) 2005 <alex@logisticchaos.com> - Alex Taylor

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

#include <math.h>
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
#include <tarpet.h>

#define uintmax_t long
#define TYPE_MAXIMUM(t) ((t) (~ (t) 0 - TYPE_MINIMUM (t)))
static uintmax_t from_header (const char *, size_t, const char *,
			      uintmax_t, uintmax_t);


evfs_filereference* evfs_file_top_level_find(evfs_filereference* file) {
	evfs_filereference* top = file;
	
	while (top->parent) {
		top = top->parent;
	}

	return top;
}


/*Main file wrappers*/
int evfs_file_remove(char* src);
int evfs_file_rename(char* src, char* dst);

int evfs_client_disconnect(evfs_client* client);
int evfs_monitor_start(evfs_client* client, evfs_command* command);
int evfs_monitor_stop(evfs_client* client, evfs_command* command);
int evfs_file_open(evfs_client* client, evfs_filereference* file);
int evfs_file_close(evfs_filereference* file);
int evfs_file_stat(evfs_command* command, struct stat* file_stat);
int evfs_file_seek(evfs_filereference* file, long offset, int whence);
int evfs_file_read(evfs_filereference* file, char* bytes, long size);
int evfs_file_write(evfs_filereference* file, char* bytes, long size);
int evfs_file_create(evfs_filereference* file);
void evfs_dir_list(evfs_client* client, evfs_command* file);





struct tar_file {
	Ecore_Hash* hierarchy;
	Ecore_Hash* link_in;
} tar_file;

struct tar_element {
	char* path;
	char* name;
	evfs_file_type type;
	Ecore_Hash* children;
} tar_element;


struct tar_file* tar_file_new() {
	struct tar_file* obj = NEW(struct tar_file);
	obj->hierarchy = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	obj->link_in = ecore_hash_new(ecore_str_hash, ecore_str_compare);

	return obj;
}



Ecore_Hash* tar_cache;


int octal_checksum_to_int(char checksum[8]) {
	int i;
	int r_val=0;
	int c_val=0;
	int c=0;
	int go=0;
	for (i=7;i>=0;i--) {
		if (go) {
			c_val = checksum[i] - '0';
			//printf("%d: cval is %d\n",i, c_val);
			r_val += c_val * (pow(8,c));
			//printf("r_val becomes: %d ---- r_val += %d * 8 ^ %d\n", r_val, c_val, c);
			c++;
		}

		if (!go && (checksum[i] == 0)) go=1;

	}

	return r_val;
}

//////////////////////////////////////////////////////////////////
///*Narfed from gnu-tar
int tar_checksum (union TARPET_block* block)
{
  size_t i;
  int unsigned_sum = 0;		/* the POSIX one :-) */
  int signed_sum = 0;		/* the Sun one :-( */
  int recorded_sum;
  uintmax_t parsed_sum;
  char *p;
  
  p = (char*)block->raw.data;
  for (i = sizeof *block; i-- != 0;)
    {
      unsigned_sum += (unsigned char) *p;
      signed_sum += (signed char) (*p++);
    }

  if (unsigned_sum == 0)
    return 0;

  /* Adjust checksum to count the "chksum" field as blanks.  */

  for (i = sizeof block->p.checksum; i-- != 0;)
    {
      unsigned_sum -= (unsigned char) block->p.checksum[i];
      signed_sum -= (signed char) (block->p.checksum[i]);
    }
  unsigned_sum += ' ' * sizeof block->p.checksum;
  signed_sum += ' ' * sizeof block->p.checksum;

  /*parsed_sum = from_header (block->p.checksum,
			    sizeof block->p.checksum, 0,
			    (uintmax_t) 0,
			    (uintmax_t) TYPE_MAXIMUM (int));
  if (parsed_sum == (uintmax_t) -1)
    return 0;

  recorded_sum = parsed_sum;*/

  //printf("Unsigned sum is %d\n", unsigned_sum);
  //printf("Signed sum is: %d\n", signed_sum);
  
  
  /*if (unsigned_sum != recorded_sum && signed_sum != recorded_sum)
    return 0;*/

  return unsigned_sum;
}
///////////////////////////////////////////////////////

void tar_name_split(union TARPET_block* block , struct tar_file* tar) {
	char* tok;
	char* path = strdup(block->p.name);
	char* dir;
	Ecore_Hash* search_hash;
	char* fname;
	char buf[512];
	struct tar_element* ele;
	int count=0;

	bzero(buf,512);
	search_hash = tar->hierarchy;
	
	tok = strtok(path, "/");
	ele = ecore_hash_get(search_hash, tok);
	do {
		/*printf ("Buf is '%s'\n", buf);*/
		
		if (count) {
			/*Search for the next child*/
			ele = ecore_hash_get(search_hash, tok); /*Looking for /usr in 2.6.11.6 */
		}
		count++;




		
		if (!ele) {
			ele = NEW(struct tar_element);
			ele->path = strdup(buf);
			ele->name = strdup(tok);
			ele->children = ecore_hash_new(ecore_str_hash, ecore_str_compare);

			//printf ("%s/%s: ", buf, tok);
			
			switch (block->p.typeflag) {
				case TARPET_TYPE_REGULAR:
				case TARPET_TYPE_REGULAR2:
					//printf("Regular file\n");
					ele->type = EVFS_FILE_NORMAL;
					break;
				case TARPET_TYPE_DIRECTORY:
					//printf("Directory\n");
					ele->type = EVFS_FILE_DIRECTORY;
					break;
				default:
					//printf("Default fallback\n");
					ele->type = EVFS_FILE_NORMAL;
					break;
			}

			ecore_hash_set(search_hash, strdup(tok), ele);


			//printf("***** Couldn't find child '%s' at node '%s' in hash %p\n", tok, buf, search_hash);
			//printf("Created a new child: '%s',with children at %p  appending to hash %p\n", tok, ele->children,search_hash);

			/*Our concated path*/
			strcat(buf, "/");
			strcat(buf, tok);
			

			/*Set a ref to this object at the root*/
			ecore_hash_set(tar->link_in, strdup(buf), ele); 

			search_hash = ele->children;
		} else {
			/*Our concated path*/
			strcat(buf, "/");
			strcat(buf, tok);
			
			
			/*printf("Found '%s' in '%s' and hash '%p'", tok, buf, search_hash);*/
			search_hash = ele->children; /*Children of linux-2.6.11.6*/
			/*printf("....Recursing into hash '%p'\n", search_hash);*/
		}



	} while (tok = strtok(NULL, "/"));

	free(path);
	/*printf("Return...\n\n\n");*/

}





evfs_plugin_functions* evfs_plugin_init() {
        printf("Initialising the tar plugin..\n");
        evfs_plugin_functions* functions = calloc(1, sizeof(evfs_plugin_functions));

        functions->evfs_client_disconnect = &evfs_client_disconnect;
	functions->evfs_dir_list = &evfs_dir_list;
	functions->evfs_file_stat = &evfs_file_stat;

        /*functions->evfs_file_remove= &evfs_file_remove;
        functions->evfs_monitor_start = &evfs_monitor_start;
        functions->evfs_monitor_stop = &evfs_monitor_stop;
        
        functions->evfs_file_open = &evfs_file_open;
        functions->evfs_file_close = &evfs_file_close;


        functions->evfs_file_seek = &evfs_file_seek;
        functions->evfs_file_read = &evfs_file_read;
        functions->evfs_file_write = &evfs_file_write;
        functions->evfs_file_create = &evfs_file_create;*/

	tar_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);

	
        return functions;


}

char* evfs_plugin_uri_get() {
        return "tar";
}

int
evfs_client_disconnect(evfs_client* client) {
        printf ("Received disconnect for client at evfs_fs_tar.c for client %d\n", client->id);
}


struct tar_file* evfs_tar_load_tar(evfs_client* client, evfs_filereference* ref) {
	evfs_filereference* p_ref;
	union TARPET_block block;
	char parent_file[1024];

	struct tar_file* tar = tar_file_new();
	
	struct tar_element* ele, *ele_new;
	Ecore_List* keys;
	char* dir, *key;
	int count;
	int find = 0;
	evfs_filereference* top_ref;
	
	printf("At tar dir_list handler\n");

	/*We make an assumption here that we have a parent - FIXME, no error checking
	 * This is normally ok - a TAR dir list must have a parent - we have to know where we're getting
	 * data from*/

	p_ref = ref->parent;
	printf("Parent ref is '%s'\n", p_ref->plugin_uri);
	evfs_uri_open(client, p_ref);

	while (evfs_uri_read(client,p_ref, &block, 512) == 512) {
		if ( (!memcmp(block.p.magic, TARPET_GNU_MAGIC, strlen(TARPET_GNU_MAGIC))) || (!memcmp(block.p.magic, TARPET_GNU_MAGIC_OLD, strlen(TARPET_GNU_MAGIC_OLD))) ) {
			/*printf("Block matches GNU Tar\n");
			printf("Magic is '%s'\n", block.p.magic);
			printf("Flag is %d\n", block.p.typeflag);*/

			
			//printf("Checksum is: '%s'\n", block.p.checksum);
			//printf("Int checksum is: %d\n", );

			tar_name_split(&block, tar);
			find++;

			/*Iterate the ecore mainloop*/
			ecore_main_loop_iterate();
		} else if (tar_checksum(&block) == octal_checksum_to_int(block.p.checksum)) {
			//printf("old magic\n");
			tar_name_split(&block, tar);
			find++;

		} else {
			/*int i = tar_checksum(&block);
			int j = octal_checksum_to_int(block.p.checksum);
			printf("%d : %d\n", i,j);*/
		}
	}
	evfs_uri_close(client, p_ref);

	if (!find) { printf("*** No GNU-TAR blocks found in file\n"); }
	else { printf("Found %d tar blocks total\n"); }
	
	printf("Recording tar file as '%s'\n", evfs_file_top_level_find(p_ref)->path);
	ecore_hash_set(tar_cache, strdup(evfs_file_top_level_find(p_ref)->path), tar);


	return tar;



}


void evfs_dir_list(evfs_client* client, evfs_command* com) {
	struct tar_file* file;
	struct tar_element* ele, *ele_new;
	Ecore_List* keys;
	Ecore_List* files = ecore_list_new();
	char* key;
	
	printf("Listing tar file dir: '%s'\n", com->file_command.files[0]->path);

	if (!(file = ecore_hash_get(tar_cache, evfs_file_top_level_find(com->file_command.files[0])->path))) {
		file = evfs_tar_load_tar(client, com->file_command.files[0]);
	}

	if (!strcmp(com->file_command.files[0]->path, "/")) {
		printf("They want the root dir..\n");

		keys = ecore_hash_keys(file->hierarchy);
		while ( (key = ecore_list_next(keys))) {
			evfs_filereference* reference = NEW(evfs_filereference);
			int size=0;
			ele = ecore_hash_get(file->hierarchy, key);

			
			printf("Filename: '%s/%s'\n", ele->path, ele->name);
			size = strlen(ele->path)+strlen("/")+strlen(ele->name)+1;
			reference->path = malloc(size);
			reference->file_type = ele->type;
			reference->plugin_uri = "tar";
			snprintf(reference->path, size, "%s/%s", ele->path, ele->name);
			ecore_list_append(files, reference);
			
		}
	} else {
		
		ele = ecore_hash_get(file->link_in, com->file_command.files[0]->path );
		if (ele) {
			printf("Got node..%s/%s\n", ele->path, ele->name);
			keys = ecore_hash_keys(ele->children);
			ecore_list_goto_first(keys);
			while ( (key = ecore_list_next(keys))) {
				evfs_filereference* reference = NEW(evfs_filereference);
				int size=0;

				ele_new = ecore_hash_get(ele->children, key);
				

				size = strlen(ele_new->path)+strlen("/")+strlen(ele_new->name)+1;
				reference->path = malloc(size);
				reference->file_type = ele_new->type;
				reference->plugin_uri = "tar";
				snprintf(reference->path, size, "%s/%s", ele_new->path, ele_new->name);
				ecore_list_append(files, reference);
			}
		}
	}


	evfs_list_dir_event_create(client, com, files);

}


int evfs_file_stat(evfs_command* command, struct stat* file_stat) {
	printf("evfs_fs_tar.c: evfs_file_stat -> stub\n");

	return 0;
}
