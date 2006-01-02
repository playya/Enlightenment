#ifndef __ENTROPY_CORE_H_
#define __ENTROPY_CORE_H_

#include <Ecore.h>
#include "entropy.h"
#include "notification_engine.h"




typedef struct entropy_core entropy_core;
struct entropy_core {
	entropy_config* config;

	Ecore_List* plugin_list;
	Ecore_List* mime_plugins;
	Ecore_Hash* entropy_thumbnailers;
	Ecore_Hash* entropy_thumbnailers_child;
	Ecore_Hash* layout_gui_events;
	Ecore_Timer* notify_executer;
	Ecore_Hash* file_interest_list; /*A file cache of all files we have loaded*/
	Ecore_Ipc_Server* server;

	void* layout_global; /*The global layout context*/
	entropy_notification_engine* notify;
	entropy_plugin* layout_plugin; /* The main layout plugin that we are relying on */
	pthread_mutex_t file_cache_mutex;

	Ecore_List* selected_files;
	Ecore_Hash* descent_hash;
	Ecore_Hash* object_associate_hash;
	Ecore_Hash* mime_action_hint;

	

	char* user_home_dir;
	char* thumbnail_path;


	
};




entropy_core* entropy_core_new();
entropy_core* entropy_core_init();
void entropy_core_destroy(entropy_core* core);


#endif
