#include <Etk.h>
#include "entropy.h"

static int _entropy_etk_options_visible = 0;
static Etk_Widget* _entropy_etk_options_dialog = NULL;
static Ecore_Hash* _entropy_global_options_hash;
static Etk_Widget* _entropy_etk_options_local_box;

typedef struct _Entropy_Etk_Options_Object {
	char* name;
	char* value;
} Entropy_Etk_Options_Object;

Entropy_Etk_Options_Object* entropy_etk_options_object_create(char* name) 
{
	Entropy_Etk_Options_Object* obj;

	obj = calloc(1, sizeof(Entropy_Etk_Options_Object));
	obj->name = strdup(name);

	ecore_hash_set(_entropy_global_options_hash, obj->name, obj);

	return obj;
}

/*CB Handlers*/
void entropy_etk_options_radio_generic_cb(Etk_Object* obj, void* data)
{
	char* name;
	Etk_Bool status;
	Entropy_Etk_Options_Object* opt;

	name = (char*)data;
	opt = ecore_hash_get(_entropy_global_options_hash, name);
	
	if (opt) {
		status = etk_toggle_button_active_get(ETK_TOGGLE_BUTTON(obj));	

		if (status == ETK_TRUE) {
			opt->value = "1";
		} else {
			opt->value = "0";
		}

		printf("Set '%s' for '%s'\n", opt->name, opt->value);
	}
}

void entropy_etk_options_slider_generic_cb(Etk_Object* obj, double value, void* data)
{
	char* name;
	char px[10];
	Entropy_Etk_Options_Object* opt;

	name = (char*)data;
	
	opt = ecore_hash_get(_entropy_global_options_hash, name);	
	if (opt) {
		snprintf(px,sizeof(px), "%.0f", value);
		if (opt->value) free(opt->value);
		opt->value = strdup(px);

		printf("Set '%s' for '%s'\n", opt->name, opt->value);
	}	
}

void entropy_etk_options_dialog_frame_set(Etk_Object* obj, void* data)
{
	Etk_Widget* frame;
	Etk_Widget* widget;
	Evas_List* children;

	frame = data;

	for (children = etk_container_children_get(ETK_CONTAINER(_entropy_etk_options_local_box)); children; ) {
		widget = children->data;
		etk_container_remove(ETK_CONTAINER(_entropy_etk_options_local_box), widget);
			
		children = children->next;
	}

	etk_box_append(ETK_BOX(_entropy_etk_options_local_box), frame , ETK_BOX_START, ETK_BOX_NONE, 0);
	etk_widget_show_all(frame);
}

void entropy_etk_options_dialog_close(Etk_Object* obj, void* data)
{
	Entropy_Etk_Options_Object* c_obj;
	Ecore_List* keys;
	char* key;
	
	etk_widget_hide(_entropy_etk_options_dialog);

	if ((int)data == 0) {
		printf("Save config selected..\n");

		keys = ecore_hash_keys(_entropy_global_options_hash);
		while ((key = ecore_list_remove_first(keys))) {
			c_obj = ecore_hash_get(_entropy_global_options_hash, key);
			printf("'%s' -> '%s'\n", key, c_obj->value);

			entropy_config_misc_item_str_set(key,c_obj->value, ENTROPY_CONFIG_LOC_HASH);
		}
		ecore_list_destroy(keys);
	}
}

void etk_options_dialog_slider_cb(Etk_Object* obj, double value, void* data)
{
	Etk_Widget* label;
	char px[10];

	label = data;
	snprintf(px,sizeof(px), "%.0f", value);
	etk_label_set(ETK_LABEL(label), px);

	
}

void entropy_etk_options_dialog_create()
{
	Etk_Widget* toolbar;
	Etk_Widget* button;
	Etk_Widget* vbox;
	Etk_Widget* frame;
	Etk_Widget* iframe;
	Etk_Widget* ivbox;
	Etk_Widget* iivbox;
	Etk_Widget* radio;
	Etk_Widget* check;
	Etk_Widget* hbox;
	Etk_Widget* slider;
	Etk_Widget* label;
	
	_entropy_etk_options_dialog = etk_window_new();

	vbox = etk_vbox_new(ETK_FALSE,0);
	_entropy_etk_options_local_box = etk_vbox_new(ETK_FALSE,0);
	etk_container_add(ETK_CONTAINER(_entropy_etk_options_dialog), vbox);
	
	toolbar = etk_toolbar_new();
	etk_toolbar_orientation_set(ETK_TOOLBAR(toolbar), ETK_TOOLBAR_HORIZ);
	etk_toolbar_style_set(ETK_TOOLBAR(toolbar), ETK_TOOLBAR_ICONS);
	etk_box_append(ETK_BOX(vbox), toolbar, ETK_BOX_START, ETK_BOX_NONE, 0);

	etk_box_append(ETK_BOX(vbox),_entropy_etk_options_local_box , ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
	
	/*General settings*/
	button = etk_tool_button_new_from_stock(ETK_STOCK_APPLICATIONS_SYSTEM);
	etk_toolbar_append(ETK_TOOLBAR(toolbar), button);

	/*General frame*/
	frame = etk_frame_new("General Settings");
	etk_box_append(ETK_BOX(_entropy_etk_options_local_box), frame, ETK_BOX_START, ETK_BOX_NONE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_frame_set), frame);
	
	ivbox = etk_vbox_new(ETK_FALSE,0);
	etk_container_add(ETK_CONTAINER(frame), ivbox);
	   iframe = etk_frame_new("Default Local Viewer");
	   etk_box_append(ETK_BOX(ivbox), iframe, ETK_BOX_START, ETK_BOX_NONE, 0);
	      iivbox = etk_vbox_new(ETK_FALSE,0);
	      etk_container_add(ETK_CONTAINER(iframe), iivbox);

	      radio = etk_radio_button_new_with_label("List view", NULL);
	      etk_box_append(ETK_BOX(iivbox), radio, ETK_BOX_START, ETK_BOX_NONE, 0);
	      etk_signal_connect("toggled", ETK_OBJECT(radio), 
		  ETK_CALLBACK(entropy_etk_options_radio_generic_cb), "general.listviewer" );
	      radio = etk_radio_button_new_with_label_from_widget("Icon view", ETK_RADIO_BUTTON(radio));
 	      etk_box_append(ETK_BOX(iivbox), radio, ETK_BOX_START, ETK_BOX_NONE, 0);
	      etk_signal_connect("toggled", ETK_OBJECT(radio), 
		  ETK_CALLBACK(entropy_etk_options_radio_generic_cb), "general.iconviewer" );
	     
           check = etk_check_button_new_with_label("Show trackback viewer");
	   etk_box_append(ETK_BOX(ivbox), check, ETK_BOX_START, ETK_BOX_NONE, 0);
	   etk_signal_connect("toggled", ETK_OBJECT(check), 
		ETK_CALLBACK(entropy_etk_options_radio_generic_cb), "general.trackback");
	      
           check = etk_check_button_new_with_label("Sort folders before files");
	   etk_box_append(ETK_BOX(ivbox), check, ETK_BOX_START, ETK_BOX_NONE, 0);
	   etk_signal_connect("toggled", ETK_OBJECT(check), 
		ETK_CALLBACK(entropy_etk_options_radio_generic_cb), "general.presortfolders");
           check = etk_check_button_new_with_label("Show hidden and backup files");
	   etk_box_append(ETK_BOX(ivbox), check, ETK_BOX_START, ETK_BOX_NONE, 0);
	   etk_signal_connect("toggled", ETK_OBJECT(check), 
		ETK_CALLBACK(entropy_etk_options_radio_generic_cb), "general.hiddenbackup");

	   iframe = etk_frame_new("Icon View Settings");
	   etk_box_append(ETK_BOX(ivbox), iframe, ETK_BOX_START, ETK_BOX_NONE, 0);
	      iivbox = etk_vbox_new(ETK_FALSE,0);
	      etk_container_add(ETK_CONTAINER(iframe), iivbox);

	      label = etk_label_new("Icon size (pixels)");
	      etk_box_append(ETK_BOX(iivbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);

	      hbox = etk_hbox_new(ETK_FALSE,0);  
	      etk_box_append(ETK_BOX(iivbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);
	      slider = etk_hslider_new(10,128, 48, 1, 1);
	      etk_box_append(ETK_BOX(hbox), slider, ETK_BOX_START, ETK_BOX_EXPAND_FILL, 0);
	      label = etk_label_new("");
	      etk_box_append(ETK_BOX(hbox), label, ETK_BOX_START, ETK_BOX_NONE, 0);
	      etk_signal_connect("value_changed", ETK_OBJECT(slider), ETK_CALLBACK(etk_options_dialog_slider_cb), 
			      label);
	      etk_signal_connect("value_changed", ETK_OBJECT(slider), ETK_CALLBACK(entropy_etk_options_slider_generic_cb), 
			      "general.iconsize");
	   

	   iframe = etk_frame_new("List View Settings");
	   etk_box_append(ETK_BOX(ivbox), iframe, ETK_BOX_START, ETK_BOX_NONE, 0);



	   

	/*Advanced*/
	button = etk_tool_button_new_from_stock(ETK_STOCK_PREFERENCES_SYSTEM);
	etk_toolbar_append(ETK_TOOLBAR(toolbar), button);
	frame = etk_frame_new("Advanced Settings");
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_frame_set), frame);

	/*Thumbnail*/
	button = etk_tool_button_new_from_stock(ETK_STOCK_IMAGE_X_GENERIC);
	etk_toolbar_append(ETK_TOOLBAR(toolbar), button);

	frame = etk_frame_new("Thumbnail Settings");
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_frame_set), frame);	


	etk_widget_size_request_set(_entropy_etk_options_dialog, 560, 460);


	hbox = etk_hbox_new(ETK_FALSE,5);
	etk_box_append(ETK_BOX(vbox), hbox, ETK_BOX_START, ETK_BOX_NONE, 0);

	button = etk_button_new_from_stock(ETK_STOCK_DIALOG_OK);
	etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_NONE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_close), (void*)0);
	button = etk_button_new_from_stock(ETK_STOCK_DIALOG_APPLY);
	etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_NONE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_close), (void*)1);
	button = etk_button_new_from_stock(ETK_STOCK_DIALOG_CANCEL);
	etk_box_append(ETK_BOX(hbox), button, ETK_BOX_START, ETK_BOX_NONE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(entropy_etk_options_dialog_close), (void*)2);
}


void entropy_etk_options_dialog_show()
{
	if (!_entropy_etk_options_dialog) {
		Entropy_Etk_Options_Object* obj;

		_entropy_global_options_hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);
		
		entropy_etk_options_object_create("general.listviewer");
		entropy_etk_options_object_create("general.iconviewer");
		entropy_etk_options_object_create("general.trackback");
		entropy_etk_options_object_create("general.presortfolders");
		entropy_etk_options_object_create("general.hiddenbackup");
		entropy_etk_options_object_create("general.iconsize");
		
		entropy_etk_options_dialog_create();
	}

	etk_widget_show_all(_entropy_etk_options_dialog);
}
