#ifndef __PLUGIN_HELPER_H_
#define __PLUGIN_HELPER_H_

#include <Ecore.h>

void entropy_thumbnailer_plugin_print(Ecore_Hash* mime_register);
Ecore_List* entropy_plugins_type_get(int type, int subtype);
char* entropy_plugin_plugin_identify(entropy_plugin* plugin);

void entropy_plugin_filesystem_file_remove(entropy_plugin* plugin, entropy_generic_file* file);

#endif
