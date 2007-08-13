#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "evfs.h"
#include "evfs_metadata.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <errno.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <inttypes.h>

#include <Eet.h>
#include <Evas.h>
#include <limits.h>

#include <sqlite3.h>

#include "evfs_metadata_db.h"

#define EVFS_METADATA_GROUP_LIST "/evfs/group/list"
#define EVFS_METADATA_BASE_DATA "block"

/*
 * TODO
 *
 * * Handle recursive (i.e. nested) files
 * * Make db access async - use callback properly, and don't block
 */

static int evfs_metadata_state = 0;

static Eet_Data_Descriptor* Evfs_Metadata_String_Edd;
static Eet_Data_Descriptor* Evfs_Metadata_Group_Edd;
static Eet_Data_Descriptor* Evfs_Metadata_File_Groups_Edd;
static Eet_Data_Descriptor* Evfs_Metadata_Root_Edd;

static evfs_metadata_root* metadata_root;

static char* homedir;
static char metadata_file[PATH_MAX];
static char metadata_db[PATH_MAX];
static Eet_File* _evfs_metadata_eet;

static sqlite3 *db;
static int _evfs_metadata_db_wait = 0;
static Ecore_DList* evfs_metdata_db_results = NULL;

/*Directory scan queue*/
static Ecore_List* evfs_metadata_directory_scan_queue = NULL;

/*Deleted scan queue*/
static int deletedPage = 0;

/*--*/
static Ecore_List* evfs_metadata_queue = NULL;
pid_t _metadata_fork= 0;
/*--*/

/*DB Helper types*/
typedef struct {
	char* name;
	char* value;
} evfs_metadata_db_item;

typedef struct {
	int fieldcount;
	evfs_metadata_db_item** fields;
} evfs_metadata_db_result;

int evfs_metadata_extract_runner(void* data);
int evfs_metadata_scan_runner(void* data);
int evfs_metadata_scan_deleted(void* data);


/*DB Helper functions*/
void evfs_metadata_db_results_init()
{
	if (evfs_metdata_db_results) {
		ecore_dlist_destroy(evfs_metdata_db_results);
		evfs_metdata_db_results = NULL;
	}

	evfs_metdata_db_results = ecore_dlist_new();
}

void evfs_metadata_db_results_free()
{
	evfs_metadata_db_result* result;
	ecore_dlist_first_goto(evfs_metdata_db_results);

	while ( (result = ecore_dlist_first_remove(evfs_metdata_db_results))) {
		int i;
		evfs_metadata_db_item* item;
		
		for (i=0;i<result->fieldcount;i++) {
			item = result->fields[i];
			if (item->name) free(item->name);
			if (item->value) free(item->value);
			free(item);
		}
		free(result->fields);
		free(result);
	}
	ecore_dlist_destroy(evfs_metdata_db_results);
	evfs_metdata_db_results = NULL;
	
}

void evfs_metadata_db_response_block()
{
	while (_evfs_metadata_db_wait) {
		usleep(2000);
	}
}

void evfs_metadata_db_response_setup()
{
	_evfs_metadata_db_wait = 1;
}

Eet_Data_Descriptor* _evfs_metadata_edd_create(char* desc, int size) 
{
	Eet_Data_Descriptor* edd;
	
	edd= eet_data_descriptor_new(desc, size,
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);

	return edd;
}

void evfs_metadata_debug_group_list_print()
{
	Evas_List* l;
	evfs_metadata_group_header* g;

	printf("Printing group list:\n");
	for (l = metadata_root->group_list; l; ) {
		g = l->data;
	

		printf("Name: %s, Desc: %s\n", g->name, g->description);
		
		l = l->next;
	}
	printf("Done..\n");
}

void evfs_metadata_debug_file_groups_print(evfs_metadata_file_groups* groups)
{
	Evas_List* l;
	evfs_metadata_group_header* g;

	printf("Printing group list:\n");
	for (l = groups->groups; l; ) {
		g = l->data;
		
		printf("Name: %s, Desc: %s\n", g->name, g->description);
		
		l = l->next;
	}
	printf("Done..\n");
}


void evfs_metadata_group_header_free(evfs_metadata_group_header* g)
{
	if (g->name) free(g->name);
	if (g->description) free(g->description);
	free(g);
}

void evfs_metadata_file_groups_free(evfs_metadata_file_groups* groups) {
	Evas_List* l;
	evfs_metadata_group_header* g;

	for (l = groups->groups; l; ) {
		g = l->data;
	
		evfs_metadata_group_header_free(g);
		
		l = l->next;
	}

}

int evfs_metadata_file_groups_group_check(evfs_metadata_file_groups* groups, char* group) {
	Evas_List* l;
	evfs_metadata_group_header* g;
	int ret = 0;

	for (l = groups->groups; l; ) {
		g = l->data;
	
		if (!strncmp(group, g->name, strlen(g->name))) {
			ret = 1;
			goto JUMP_OUT;
		}
			
		l = l->next;
	}	

	JUMP_OUT:
	return ret;
}

evfs_metadata_group_header*
evfs_metadata_group_header_new(char* name, char* desc) 
{
	evfs_metadata_group_header* group;

	group = calloc(1, sizeof(evfs_metadata_group_header));

	if (name) group->name = strdup(name);
	if (desc) group->description = strdup(desc);

	return group;
}

int evfs_metadata_group_header_exists(char* group)
{
	char query[1024];
	int exists = 0;
	int ret;
	sqlite3_stmt *pStmt;

	evfs_metadata_db_response_setup();
	evfs_metadata_db_results_init();

	snprintf(query, sizeof(query), "select id from MetaGroup where name='%s'", group);
	ret = sqlite3_prepare(db, query, 
			-1, &pStmt, 0);
	if (ret == SQLITE_OK) {
		ret = sqlite3_step(pStmt);
		if (ret == SQLITE_ROW) exists = sqlite3_column_int(pStmt,0);
	} else {
		printf("header_exists: sqlite_error\n");
	}
		
	sqlite3_reset(pStmt);
	sqlite3_finalize(pStmt);

	return exists;
}

void evfs_metadata_initialise(int forker)
{
	struct stat config_dir_stat;
	Evas_List* group;
	char* data;
	int size;
	int ret;
	evfs_filereference* ref;
	
	if (!evfs_metadata_state) {
		evfs_metadata_state++;
	} else return;

	group = NULL;

	

	if (!evfs_object_client_is_get()) {
		printf(". EVFS metadata initialise..\n");

		/*Setup the metadata extract queue*/
		evfs_metadata_queue = ecore_list_new();

		/*String edd*/
		Evfs_Metadata_String_Edd = _evfs_metadata_edd_create("evfs_metadata_string", sizeof(evfs_metadata_object));
		EET_DATA_DESCRIPTOR_ADD_BASIC(Evfs_Metadata_String_Edd, evfs_metadata_object,
	                                 "description", description, EET_T_STRING);
		EET_DATA_DESCRIPTOR_ADD_BASIC(Evfs_Metadata_String_Edd, evfs_metadata_object,
                                 "key", key, EET_T_STRING);
		EET_DATA_DESCRIPTOR_ADD_BASIC(Evfs_Metadata_String_Edd, evfs_metadata_object,
                                 "value", value, EET_T_STRING);

		/*Group edd*/
		Evfs_Metadata_Group_Edd = _evfs_metadata_edd_create("evfs_metadata_group_header", sizeof(evfs_metadata_group_header));
		EET_DATA_DESCRIPTOR_ADD_BASIC(Evfs_Metadata_Group_Edd, evfs_metadata_group_header,
	                                 "description", description, EET_T_STRING);
		EET_DATA_DESCRIPTOR_ADD_BASIC(Evfs_Metadata_Group_Edd, evfs_metadata_group_header,
	                                 "name", name, EET_T_STRING);

		/*Metadata group root*/
		Evfs_Metadata_Root_Edd = _evfs_metadata_edd_create("evfs_metadata_root", sizeof(evfs_metadata_root));
		EET_DATA_DESCRIPTOR_ADD_LIST(Evfs_Metadata_Root_Edd, evfs_metadata_root,
	                                 "group_list", group_list, Evfs_Metadata_Group_Edd);

		/*Metadata group root*/
		Evfs_Metadata_File_Groups_Edd = _evfs_metadata_edd_create("evfs_metadata_file_groups", sizeof(evfs_metadata_file_groups));
		EET_DATA_DESCRIPTOR_ADD_LIST(Evfs_Metadata_File_Groups_Edd, evfs_metadata_file_groups,
	                                 "groups", groups, Evfs_Metadata_Group_Edd);

		homedir = strdup(getenv("HOME"));
		snprintf(metadata_file, PATH_MAX, "%s/.e/evfs", homedir);
	
		if (stat(metadata_file, &config_dir_stat)) {
			mkdir(metadata_file, 0700);
		}

		snprintf(metadata_file, PATH_MAX, "%s/.e/evfs/evfs_metadata.eet", homedir);
		snprintf(metadata_db, PATH_MAX, "%s/.e/evfs/evfs_metadata.db", homedir);

		if (stat(metadata_file, &config_dir_stat)) {
			printf("Making new metadata file..\n");
				
			/*Open/close the file*/
			_evfs_metadata_eet = eet_open(metadata_file, EET_FILE_MODE_WRITE);
	
			metadata_root = calloc(1, sizeof(evfs_metadata_root));
	
			/*Create a starting 'group' list*/
			metadata_root->group_list = evas_list_append(metadata_root->group_list, 
					evfs_metadata_group_header_new("Pictures", "Pictures"));
			metadata_root->group_list = evas_list_append(metadata_root->group_list, 
					evfs_metadata_group_header_new("Video", "Video"));
			metadata_root->group_list = evas_list_append(metadata_root->group_list, 
					evfs_metadata_group_header_new("Audio", "Audio"));

			data = eet_data_descriptor_encode(Evfs_Metadata_Root_Edd, metadata_root, &size);
			ret = eet_write(_evfs_metadata_eet, EVFS_METADATA_GROUP_LIST, data, size, 0);
	
			free(data);
	
			eet_close(_evfs_metadata_eet);
			
		} else {
			printf("Loading pre-existing metadata root..\n");
		
			_evfs_metadata_eet = eet_open(metadata_file, EET_FILE_MODE_READ);	
			data = eet_read(_evfs_metadata_eet, EVFS_METADATA_GROUP_LIST, &size);
			if (data) {
				metadata_root = 
					eet_data_descriptor_decode(Evfs_Metadata_Root_Edd, data, size);
				free(data);

				printf("..Loaded group list..\n");

				evfs_metadata_debug_group_list_print();
			} else {
				printf("Error loading group list..\n");
			}
			eet_close(_evfs_metadata_eet);
		}

		evfs_metadata_db_init(&db);

		/*Setup the directory scan queue*/
		ref = NEW(evfs_filereference);
		ref->plugin_uri = strdup("file");
		ref->path = strdup(homedir);

		evfs_metadata_directory_scan_queue = ecore_list_new();
		ecore_list_append(evfs_metadata_directory_scan_queue, ref);

		if (forker) {
			ecore_timer_add(0.5, evfs_metadata_scan_runner, NULL);
			ecore_timer_add(5, evfs_metadata_scan_deleted, NULL);
			ecore_timer_add(0.5, evfs_metadata_extract_runner, NULL);
		}
	}

}

void evfs_metadata_initialise_worker()
{
	evfs_metadata_db_init(&db);
}

Evas_List* evfs_metadata_groups_get() {
	int ret;
	Evas_List* ret_list = NULL;
	sqlite3_stmt *pStmt;
	evfs_metadata_group_header* g;

	ret = sqlite3_prepare(db, "select name,visualHint from MetaGroup where parent = 0", 
			-1, &pStmt, 0);

	do {
		ret = sqlite3_step(pStmt);
		

		if (ret == SQLITE_ROW) {
			g = calloc(1, sizeof(evfs_metadata_group_header));
			g->name = strdup((char*)sqlite3_column_text(pStmt,0));
			if (sqlite3_column_text(pStmt, 1)) {
				g->visualhint = strdup((char*)sqlite3_column_text(pStmt,1));
			}
			
			ret_list = evas_list_append(ret_list, g);
		}
	} while (ret == SQLITE_ROW);

	sqlite3_reset(pStmt);
	sqlite3_finalize(pStmt);
	
	return ret_list;
}

Ecore_List*
evfs_metadata_file_group_list(char* group) 
{
	Ecore_List* ret_list;
	char query[PATH_MAX];
	sqlite3_stmt *pStmt;
	int ret;

	ret_list = ecore_list_new();

	snprintf(query, sizeof(query), "select f.filename from File f join FileGroup fg on f.id = fg.file join MetaGroup mg on mg.id = fg.metagroup where mg.name= '%s'", group);
	ret = sqlite3_prepare(db, query, 
			-1, &pStmt, 0);

	do {
		ret = sqlite3_step(pStmt);

		if (ret == SQLITE_ROW) 
			ecore_list_append(ret_list, strdup((char*)sqlite3_column_text(pStmt,0)));
	} while (ret == SQLITE_ROW);

	sqlite3_reset(pStmt);
	sqlite3_finalize(pStmt);
	

	return ret_list;
}

void evfs_metadata_group_header_file_add(evfs_filereference* ref, char* group)
{
	char* file_path;
	int ret = 0;

	char* errMsg = 0;
	char query[1024];

	int groupid = 0;
	int file = 0;
	sqlite3_stmt *pStmt;

	/*First make sure this group exists*/

	if ( (groupid = evfs_metadata_group_header_exists(group))) {
		printf("Group exists - proceed\n");
	} else {
		printf("Alert - group not found\n");
		return;
	}

	/*Build a path*/
	file_path = evfs_filereference_to_string(ref);
	printf("File path is: %s\n", file_path);


	snprintf(query, sizeof(query), "select id from File where filename ='%s'", file_path);
	ret = sqlite3_prepare(db, query, 
			-1, &pStmt, 0);

	if (ret == SQLITE_OK) {
		ret = sqlite3_step(pStmt);
		if (ret == SQLITE_ROW)  {
			file = sqlite3_column_int(pStmt,0);
		} else {
			snprintf(query, sizeof(query), "insert into File (filename) select '%s';", file_path);
			ret = sqlite3_exec(db, query, 
			NULL, 0,&errMsg);
	
			file = (int)sqlite3_last_insert_rowid(db);
		}
	} else {
		printf("header_file_add: sqlite error\n");
	}
	sqlite3_reset(pStmt);
	sqlite3_finalize(pStmt);

	if (file && groupid) {
		printf("File id: %d - Group id: %d\n", file, groupid);

		snprintf(query, sizeof(query), "insert into FileGroup (File,MetaGroup) values (%d,%d);",
				file, groupid);
		ret = sqlite3_exec (db, query, NULL, 0, &errMsg);
	}

	evfs_metadata_db_results_free();
	
}



void evfs_metadata_group_header_file_remove(evfs_filereference* ref, char* group)
{
	char* file_path;
	int ret = 0;

	char* errMsg = 0;
	char query[1024];

	int groupid = 0;
	int file = 0;
	sqlite3_stmt *pStmt;

	/*First make sure this group exists*/

	if ( (groupid = evfs_metadata_group_header_exists(group))) {
		printf("Group exists - proceed\n");
	} else {
		printf("Alert - group not found\n");
		return;
	}

	/*Build a path*/
	file_path = evfs_filereference_to_string(ref);
	printf("File path is: %s\n", file_path);


	snprintf(query, sizeof(query), "select id from File where filename ='%s'", file_path);
	ret = sqlite3_prepare(db, query, 
			-1, &pStmt, 0);

	if (ret == SQLITE_OK) {
		ret = sqlite3_step(pStmt);
		if (ret == SQLITE_ROW)  {
			file = sqlite3_column_int(pStmt,0);
		} else {
			printf("File does not exist - cannot remove from group\n");
			return;
		}
	} else {
		printf("header_file_add: sqlite error\n");
	}
	sqlite3_reset(pStmt);
	sqlite3_finalize(pStmt);

	if (file && groupid) {
		printf("File id: %d - Group id: %d\n", file, groupid);

		snprintf(query, sizeof(query), "delete from FileGroup where file = %d and metaGroup = %d",
				file, groupid);
		ret = sqlite3_exec (db, query, NULL, 0, &errMsg);
	}

	evfs_metadata_db_results_free();
	
}




void evfs_metadata_file_set_key_value_string(evfs_filereference* ref, char* key,
		char* value) 
{
	evfs_metadata_object obj;
	char path[PATH_MAX];
	char* file_path;
	char* data;
	int size;
	int ret;

	/*Build a path*/
	file_path = evfs_filereference_to_string(ref);

	snprintf(path, PATH_MAX, "/filedata/%s/custommeta/string/%s", file_path, key);
	_evfs_metadata_eet = eet_open(metadata_file, EET_FILE_MODE_READ_WRITE);
	ret = 0;

	obj.description = "string";
	obj.key = key;
	obj.value = (char*)value;

	data = eet_data_descriptor_encode(Evfs_Metadata_String_Edd, &obj, &size);
	
	if (data) {
		ret = eet_write(_evfs_metadata_eet, path, data, size, 0);
	}
	if (ret) {
		printf("Wrote %s for %s\n", value, path);
	}
	free(data);
	free(file_path);

	eet_close(_evfs_metadata_eet);
}

char* evfs_metadata_file_get_key_value_string(evfs_filereference* ref, char* key) 
{
	evfs_metadata_object* obj = NULL;
	char path[PATH_MAX];
	char* data;
	char* file_path;
	int size;
	int ret;
	char* value = NULL;

	/*Build a path*/
	file_path = evfs_filereference_to_string(ref);

	snprintf(path, PATH_MAX, "/filedata/%s/custommeta/string/%s", file_path, key);
	_evfs_metadata_eet = eet_open(metadata_file, EET_FILE_MODE_READ);
	ret = 0;

	if (_evfs_metadata_eet) {
		data = eet_read(_evfs_metadata_eet, path, &size);
		if (data) {
			obj = eet_data_descriptor_decode(Evfs_Metadata_String_Edd, data, size);
		}

		if (obj) {
			printf ("Got %s -> %s\n", obj->key, (char*)obj->value);
			value = strdup(obj->value);
			free(data);
			free(obj);
		}
	}	
	eet_close(_evfs_metadata_eet);
	free(file_path);

	return value;
}





/*----------------*/
/*This section defines the fork/grab part of the metadata system*/
void evfs_metadata_extract_queue(evfs_filereference* ref)
{
	/*At the moment, we only extract meta from posix folders*/
	/*This may change, but we'll have to copy the file locally,
	 * so libextractor can have a shot at it*/
	if (!strcmp(ref->plugin_uri,"file")) {
		evfs_filereference* clone;

		clone = evfs_filereference_clone(ref);
		ecore_list_append(evfs_metadata_queue, clone);
	}
}


int evfs_metadata_scan_runner(void* data)
{
	evfs_filereference* ref;
	evfs_filereference* iref;

	if ((ref = ecore_list_first_remove(
		evfs_metadata_directory_scan_queue))) {

		evfs_filereference_sanitise(ref);
		if (ref->plugin) {
			Ecore_List* dir_list;
			evfs_command* c = evfs_file_command_single_build(ref);
			
			 (*EVFS_PLUGIN_FILE(ref->plugin)->functions->evfs_dir_list)
				(NULL, c, &dir_list);

			evfs_cleanup_file_command(c);

			ecore_list_first_goto(dir_list);
			while ( (iref = ecore_list_first_remove(dir_list))) {
				struct stat file_stat;
				char* pos = strrchr(iref->path, '/');

				if (pos) {
					/*Dangerous - check str length TODO*/
					/*Are we a hidden dir/file?*/
					if (!(pos[1] == '.')) {
						evfs_command* ci = evfs_file_command_single_build(iref);
						(*EVFS_PLUGIN_FILE(iref->plugin)->functions->evfs_file_lstat) 
							(ci, &file_stat, 0);

						if (S_ISDIR(file_stat.st_mode)) {
							ecore_list_append(evfs_metadata_directory_scan_queue, iref);
							free(ci->file_command.files);
							free(ci);
						} else if (strstr(iref->path, ".mp3") || strstr(iref->path, ".jpg") ||
								strstr(iref->path, ".mpg")) {
							ecore_list_append(evfs_metadata_queue, iref);
							free(ci->file_command.files);
							free(ci);
						} else {
							evfs_cleanup_file_command(ci);
						}
					}
				}
			}
			ecore_list_destroy(dir_list);
		} else {
			evfs_cleanup_filereference(ref);
		}
	}
	
	return 1;
}

int evfs_metadata_scan_deleted(void* data) 
{
	sqlite3* dbi;
	int handleCount=0;
	sqlite3_stmt* pStmt;
	char query[1024];
	int pageSize=30;
	int ret;
	unsigned const char* str;
	int res = 0;
	Ecore_List* delList;

	delList = ecore_list_new();
	
	ret = sqlite3_open(metadata_db, &dbi);
	if( ret ){
	    fprintf(stderr, "Can't open metadata database: %s\n", sqlite3_errmsg(dbi));
	    sqlite3_close(dbi);
	    return 0;
	}

	/*Wait up to 10 seconds in this fork for the db to be available*/
	sqlite3_busy_timeout(dbi,10000);

	snprintf(query,sizeof(query), "select filename,id from File order by id limit %d offset %d", pageSize, deletedPage);
	
	ret = sqlite3_prepare(db, query, -1, &pStmt, 0);
        if (ret == SQLITE_OK) {
		while ( (ret = sqlite3_step(pStmt) == SQLITE_ROW)) {
			str = sqlite3_column_text(pStmt,0);
			handleCount++;
			/*printf("Filename: %s - ", str);*/

			evfs_filereference* file = evfs_parse_uri_single((char*)str);
			if (file) {
				evfs_command* proxy;
				struct stat file_stat;

				proxy = evfs_file_command_single_build(file);
				res = (*EVFS_PLUGIN_FILE(file->plugin)->functions->evfs_file_stat)(proxy, &file_stat,0);

				if (res == 0) {
					/*printf("*\n");*/
				} else {
					/*printf("DELETED\n");*/

					ecore_list_append(delList, (int*)sqlite3_column_int(pStmt,1));
				}
				evfs_cleanup_command(proxy, EVFS_CLEANUP_FREE_COMMAND);
			}
		}
		sqlite3_reset(pStmt);
		sqlite3_finalize(pStmt);
		
		/*If we saw no rows, we're at the end - go back
		 * to the start*/
		if (handleCount ==0) {
			deletedPage = 0;
		} else {
			if (ecore_list_count(delList) > 0) {
				int id;
				ecore_list_first_goto(delList);
				while ( (id = (int)ecore_list_next(delList))) {
					evfs_metadata_db_delete_file(db,id);
				}
			} else {
				/*Only advance page if we didn't delete
				 * This may result in dupe scans - but there's no
				 * way to avoid this, because the position of all the rows
				 * will have changed */
				deletedPage += pageSize;
				/*printf("No deletes, next page\n");*/
			}
		}
	} else {
		printf("Query failed..\n");
	}

	ecore_list_destroy(delList);
	sqlite3_close(dbi);	

	return 1;
}

int evfs_metadata_extract_runner(void* data)
{
	evfs_filereference* ref;
	int status;
	int ret;
	
	if (!_metadata_fork) {
		ecore_list_first_goto(evfs_metadata_queue);
		if ( (ref = ecore_list_current(evfs_metadata_queue))) {
			/*printf("..item on queue..\n");*/
			evfs_metadata_extract_fork(ref);
		}
	} else {
		/*printf("...metadata runner executing..\n");*/

		if ((ret = (waitpid(_metadata_fork, &status, WNOHANG) > 0)) ||
			       errno == ECHILD) {
			_metadata_fork = 0;

			
			ecore_list_first_goto(evfs_metadata_queue);
			ref = ecore_list_current(evfs_metadata_queue);
				
			if (ref) {
				evfs_cleanup_filereference(ref);
				ecore_list_first_remove(evfs_metadata_queue);
			} else {
				printf("EVFS: ugh? no file, and we just processed it for meta? : %d\n", getpid());
			}

		} else {
			printf("Fork ret was: %d\n", ret);
		}
	}
	return 1;
}

int evfs_metadata_extract_fork(evfs_filereference* ref)
{	
	_metadata_fork = fork();
	if (!_metadata_fork) {
		evfs_plugin* plugin;
		evfs_command* command;
		Evas_List* meta_list;
		int ret;
		sqlite3* dbi;
		int file;
		Evas_List* l;
		evfs_meta_obj* o;

		ecore_main_loop_quit();

		ret = sqlite3_open(metadata_db, &dbi);
		if( ret ){
		    fprintf(stderr, "Can't open metadata database: %s\n", sqlite3_errmsg(dbi));
		    sqlite3_close(dbi);
		    return 0;
		}

		/*Wait up to 10 seconds in this fork for the db to be available*/
		sqlite3_busy_timeout(dbi,10000);

		/*printf("Extract fork started: %s..\n", ref->path);*/

		file = evfs_metadata_db_id_for_file(dbi,ref,1);

		if (file) {
			command = NEW(evfs_command);
			command->file_command.files = calloc(1, sizeof(evfs_filereference*));
			command->file_command.files[0] = ref;
			plugin = evfs_meta_plugin_get_for_type(evfs_server_get(), "object/undefined");
			meta_list = (*EVFS_PLUGIN_META(plugin)->functions->evfs_file_meta_retrieve)(NULL,command);

			for (l=meta_list;l;) {
				o=l->data;

				evfs_metadata_db_file_keyword_add(dbi, file, o->key, o->value);	
				
				if (o->key) free(o->key);
				if (o->value) free(o->value);
				free(o);
				l=l->next;
			}
		} else {
			printf("metadata_extract_fork: could not insert file to db\n");
		}

		sqlite3_close(dbi);

		exit(0);
	}

	return 1;
}

