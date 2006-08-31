#ifndef _ENTROPY_CONFIG_H_
#define _ENTROPY_CONFIG_H_

#include "entropy.h"
#include "entropy_gui.h"
#include <Evas.h>

struct Entropy_Config_Mime_Binding {
	char* desc;
	char* mime_type;
	Evas_List* actions;
};
typedef struct Entropy_Config_Mime_Binding Entropy_Config_Mime_Binding;

struct Entropy_Config_Mime_Binding_Action {
	char* app_description;
	char* executable;
	char* args;
};
typedef struct Entropy_Config_Mime_Binding_Action Entropy_Config_Mime_Binding_Action;

struct Entropy_Config_Structure {
	char* name;
	char* uri;
};
typedef struct Entropy_Config_Structure Entropy_Config_Structure;

struct Entropy_Config_Loaded {
	int config_version;
	Evas_List* mime_bindings;
	Evas_List* structures;
};
typedef struct Entropy_Config_Loaded Entropy_Config_Loaded;

struct Entropy_Config_Item {
	char* name;
	char* value;
};
typedef struct Entropy_Config_Item Entropy_Config_Item;

struct Entropy_Config {
	char* config_dir;
	char* config_dir_and_file;
	char* config_dir_and_file_eet;

	Entropy_Config_Loaded* Loaded_Config;
	Ecore_Hash* Misc_Config;
	Evas_List* MisC_Config_Load;
};
typedef struct Entropy_Config Entropy_Config;


Entropy_Config_Mime_Binding* entropy_config_mime_binding_for_type_get(char* type);
Entropy_Config* entropy_config_init(entropy_core* core);
void entropy_config_destroy(Entropy_Config* config);
char* entropy_config_str_get(char* module, char* key);
void entropy_config_str_set(char* module, char* key, char* value);


Evas_List* entropy_config_standard_structures_parse (entropy_gui_component_instance * instance, char *config);
Entropy_Config_Structure* entropy_config_standard_structures_add (char *name, char *uri);
Entropy_Config_Mime_Binding*
entropy_config_binding_new(char* mime_type, char* description, Entropy_Config_Mime_Binding_Action* action1, ...);
void entropy_config_standard_structure_remove(Entropy_Config_Structure* structure);
void entropy_config_standard_structures_create ();
void entropy_config_eet_config_save();
void entropy_config_version_check();
void entropy_config_edd_build();
void entropy_config_loaded_config_free();
void entropy_config_defaults_populate(Entropy_Config_Loaded* config);
Entropy_Config_Structure* entropy_config_structure_new(char* name, char* uri);

void entropy_config_items_init();
void entropy_config_misc_item_set_str(char* item, char* value);
char* entropy_config_misc_item_get_str(char* item);

#define ENTROPY_CONFIG_INT_UNDEFINED 65535

#endif
