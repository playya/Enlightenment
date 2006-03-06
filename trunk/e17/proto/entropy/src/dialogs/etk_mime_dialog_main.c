#include "entropy.h"
#include <Etk.h>

static int _etk_mime_dialog_displayed = 0;
static Etk_Widget* mime_dialog_window;
static Etk_Widget* mime_dialog_add_edit_window;
static	Etk_Widget* etk_mime_mime_entry;
static	Etk_Widget* etk_mime_program_entry;
static Etk_Widget* _etk_mime_dialog_main_tree = NULL;


void _entropy_etk_mime_dialog_add_cb(Etk_Object* w, void* user_data);
static Etk_Bool _etk_window_deleted_cb (Etk_Object * object, void *data);
void etk_mime_dialog_add_edit_create();
void etk_mime_dialog_create();
void etk_mime_dialog_tree_populate();


static Etk_Bool
_etk_window_deleted_cb (Etk_Object * object, void *data)
{
	_etk_mime_dialog_displayed = 0;
	etk_object_destroy(ETK_OBJECT(mime_dialog_window));

	return ETK_TRUE;
}



void _entropy_etk_mime_dialog_add_edit_cancel_cb(Etk_Object* w, void* user_data)
{
	if (mime_dialog_add_edit_window) {
		etk_object_destroy(ETK_OBJECT(mime_dialog_add_edit_window));
	}
}

void _entropy_etk_mime_dialog_add_edit_final_cb(Etk_Object* w, void* user_data)
{
	const char *type_text = etk_entry_text_get(ETK_ENTRY(etk_mime_mime_entry));
	const char *action_text = etk_entry_text_get(ETK_ENTRY(etk_mime_program_entry));

	entropy_core_mime_action_add((char*)type_text, (char*)action_text);

	etk_object_destroy(ETK_OBJECT(mime_dialog_add_edit_window));
	mime_dialog_add_edit_window= NULL;

	etk_mime_dialog_tree_populate();
}

void etk_mime_dialog_tree_populate()
{
	Etk_Widget* tree = _etk_mime_dialog_main_tree;
	Etk_Tree_Col* col1;
	Etk_Tree_Col* col2;
	
	char* key;
	entropy_mime_action* action;
	Ecore_List* keys;

	etk_tree_freeze(ETK_TREE(tree));

	etk_tree_clear(ETK_TREE(tree));

	/*Populate the tree*/
	col1 = etk_tree_nth_col_get(ETK_TREE(tree), 0);
	col2 = etk_tree_nth_col_get(ETK_TREE(tree), 1);
	
	keys = ecore_hash_keys(entropy_core_get_core()->mime_action_hint);
	while ((key = ecore_list_remove_first(keys))) {
	     etk_tree_append(ETK_TREE(tree), 
		  col1, key, 
		  col2,  ((entropy_mime_action*)ecore_hash_get(
		entropy_core_get_core()->mime_action_hint, 
		key))->executable,
		  NULL);
	}

	etk_tree_thaw(ETK_TREE(tree));
}


void etk_mime_dialog_add_edit_create(char* mime, char* program) {
	Etk_Widget* table;
	Etk_Widget* label;
	Etk_Widget* button;

	mime_dialog_add_edit_window = etk_window_new();
	etk_window_title_set(ETK_WINDOW(mime_dialog_add_edit_window), "Program Associations");
	etk_window_wmclass_set(ETK_WINDOW(mime_dialog_add_edit_window), "mimedialog", "mimedialog");
	etk_widget_size_request_set(ETK_WIDGET(mime_dialog_add_edit_window), 250, 80);

	table = etk_table_new(2,3,ETK_FALSE);
	
	label = etk_label_new("MIME-Type");
	etk_table_attach(ETK_TABLE(table), label,0,0,0,0,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);

	etk_mime_mime_entry = etk_entry_new();
	if (mime) {
		printf("Setting mime to '%s'...\n", mime);
		etk_entry_text_set(ETK_ENTRY(etk_mime_mime_entry), mime);
	}
	etk_table_attach(ETK_TABLE(table), etk_mime_mime_entry,1,1,0,0,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);

	label = etk_label_new("Program");
	etk_table_attach(ETK_TABLE(table), label,0,0,1,1,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);

	etk_mime_program_entry = etk_entry_new();
	if (program) etk_entry_text_set(ETK_ENTRY(etk_mime_program_entry), program);	
	etk_table_attach(ETK_TABLE(table), etk_mime_program_entry,1,1,1,1,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);


	button = etk_button_new_with_label("OK");
	etk_table_attach(ETK_TABLE(table), button,0,0,2,2,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(_entropy_etk_mime_dialog_add_edit_final_cb), 
				NULL );

	button = etk_button_new_with_label("Cancel");
	etk_table_attach(ETK_TABLE(table), button,1,1,2,2,
				0,0,
				ETK_FILL_POLICY_HFILL | ETK_FILL_POLICY_VFILL | ETK_FILL_POLICY_HEXPAND);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(_entropy_etk_mime_dialog_add_edit_cancel_cb), 
				NULL );


	

	etk_container_add(ETK_CONTAINER(mime_dialog_add_edit_window), table);



	etk_widget_show_all(mime_dialog_add_edit_window);

        if (mime) {
                printf("Setting mime to '%s'...\n", mime);
                etk_entry_text_set(ETK_ENTRY(etk_mime_mime_entry), mime);
        }
        if (program) etk_entry_text_set(ETK_ENTRY(etk_mime_program_entry), program);

}

void _entropy_etk_mime_dialog_edit_cb(Etk_Object* w, void* user_data)
{
	Etk_Tree_Row* row = etk_tree_selected_row_get(ETK_TREE(_etk_mime_dialog_main_tree));
	char* mime = NULL;
	char* program = NULL;
	Etk_Tree_Col* col1;
	Etk_Tree_Col* col2;

	col1 = etk_tree_nth_col_get(ETK_TREE(_etk_mime_dialog_main_tree), 0);
	col2 = etk_tree_nth_col_get(ETK_TREE(_etk_mime_dialog_main_tree), 1);

	etk_tree_row_fields_get(row, col1, &mime, col2, &program,NULL);
	
	etk_mime_dialog_add_edit_create(mime,program);
}


void _entropy_etk_mime_dialog_add_cb(Etk_Object* w, void* user_data)
{
	etk_mime_dialog_add_edit_create(NULL,NULL);
}

void etk_mime_dialog_create()
{
	Etk_Widget* tree;
	Etk_Widget* vbox = NULL;
	Etk_Widget* hbox = NULL;
	Etk_Tree_Col* tree_col;
	Etk_Widget* button;

	if (_etk_mime_dialog_displayed == 1)
		return;

	_etk_mime_dialog_displayed = 1;

	mime_dialog_window = etk_window_new();

	etk_window_title_set(ETK_WINDOW(mime_dialog_window), "Program Associations");
	etk_window_wmclass_set(ETK_WINDOW(mime_dialog_window), "mimedialog", "mimedialog");

	etk_widget_size_request_set(ETK_WIDGET(mime_dialog_window), 450, 500);

	vbox = etk_vbox_new(ETK_FALSE,0);
	etk_container_add(ETK_CONTAINER(mime_dialog_window), vbox);

	tree = etk_tree_new();
	_etk_mime_dialog_main_tree = tree;
	
	etk_tree_mode_set(ETK_TREE(tree), ETK_TREE_MODE_LIST);
	tree_col = etk_tree_col_new(ETK_TREE(tree), _("File Type"), 
		  etk_tree_model_text_new(ETK_TREE(tree)), 125);

	tree_col = etk_tree_col_new(ETK_TREE(tree), _("Executable"), 
		  etk_tree_model_text_new(ETK_TREE(tree)), 150);
        etk_tree_col_expand_set(tree_col, ETK_TRUE);

	etk_tree_build(ETK_TREE(tree));
	etk_box_pack_start(ETK_BOX(vbox), tree, ETK_TRUE, ETK_TRUE, 0);

	etk_mime_dialog_tree_populate();

	etk_signal_connect ("delete_event", ETK_OBJECT (mime_dialog_window),
	      ETK_CALLBACK (_etk_window_deleted_cb), NULL);


	hbox = etk_hbox_new(ETK_FALSE,0);
	etk_box_pack_end(ETK_BOX(vbox), hbox, ETK_FALSE, ETK_FALSE, 0);

	button = etk_button_new_with_label("Edit Selected");
	etk_box_pack_start(ETK_BOX(hbox), button, ETK_FALSE, ETK_FALSE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(_entropy_etk_mime_dialog_edit_cb), NULL);

	button = etk_button_new_with_label("Remove Selected");
	etk_box_pack_start(ETK_BOX(hbox), button, ETK_FALSE, ETK_FALSE, 0);

	button = etk_button_new_with_label("Add New..");
	etk_box_pack_start(ETK_BOX(hbox), button, ETK_FALSE, ETK_FALSE, 0);
	etk_signal_connect("pressed", ETK_OBJECT(button), ETK_CALLBACK(_entropy_etk_mime_dialog_add_cb), NULL);

	etk_widget_show_all(mime_dialog_window);
}

