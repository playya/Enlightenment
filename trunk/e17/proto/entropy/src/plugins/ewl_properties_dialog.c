#include <Ewl.h>
#include "entropy.h"
#include "entropy_gui.h"
#include "entropy_config.h"
#include <dlfcn.h>
#include <time.h>

void
 __destroy_properties_dialog(Ewl_Widget *dialog, void *ev_data, void *user_data)
 {
	 ewl_widget_destroy(EWL_WIDGET(user_data));
}


void open_with_cb(Ewl_Widget *w , void *ev_data , void *user_data ) 


void ewl_properties_dialog_openwith_cb(Ewl_Widget *w , void *ev_data , void *user_data ) {
	Ewl_Widget* file_dialog = ewl_filedialog_new();
	Ewl_Widget* window = ewl_window_new();

	ewl_filedialog_type_set(EWL_FILEDIALOG(file_dialog), EWL_FILEDIALOG_TYPE_OPEN);
        ewl_callback_append (file_dialog, EWL_CALLBACK_VALUE_CHANGED, open_with_cb, NULL);
	ewl_container_child_append(EWL_CONTAINER(window), file_dialog);
	ewl_widget_show(file_dialog);
	ewl_widget_show(window);

}

void ewl_icon_local_viewer_show_stat(entropy_file_stat* file_stat) {
	Ewl_Widget* window;
	Ewl_Widget* vbox;
	Ewl_Widget* image;
	Ewl_Widget* hbox;
	Ewl_Widget* text;
	Ewl_Widget* button;
	char itext[100];
	time_t stime;

	
	window = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(window), "File Properties");
	ewl_object_custom_size_set(EWL_OBJECT(window), 300, 400);
	
	vbox = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(window), vbox);
	ewl_widget_show(vbox);


	/*----------------------------*/
	/*The icon*/
	if (file_stat->file->thumbnail) {
		hbox = ewl_hbox_new();
		ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
		ewl_widget_show(hbox);

		image = ewl_image_new();
		ewl_image_constrain_set(EWL_IMAGE(image), 64);
		ewl_image_file_set(EWL_IMAGE(image), file_stat->file->thumbnail->thumbnail_filename, NULL);
		ewl_container_child_append(EWL_CONTAINER(hbox), image);
		ewl_widget_show(image);

		text = ewl_text_new();
		ewl_text_text_set(EWL_TEXT(text), file_stat->file->filename);
		ewl_container_child_append(EWL_CONTAINER(hbox), text);
		ewl_widget_show(text);


	}

	

	/*---------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);



	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Location");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file_stat->file->path);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	/*--------------------------*/



	/*----------------------------------------*/
	/*hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Filename");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);*/




	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Type: ");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);






	text = ewl_text_new();
	if (strlen(file_stat->file->mime_type)) {
		ewl_text_text_set(EWL_TEXT(text), file_stat->file->mime_type);
	} else {
		ewl_text_text_set(EWL_TEXT(text), "object/unknown");
	}
	
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	button = ewl_button_new();
	ewl_callback_append(button, EWL_CALLBACK_CLICKED, ewl_properties_dialog_openwith_cb, NULL);
	ewl_button_label_set(EWL_BUTTON(button), "Open with..");
	ewl_object_custom_size_set(EWL_OBJECT(button), 70, 10);
	ewl_container_child_append(EWL_CONTAINER(hbox), button);
	ewl_widget_show(button);
	/*--------------------------------*/



	/*----------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Plugin URI");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);



	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file_stat->file->uri_base);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);




	/*---------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Size: ");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	snprintf(itext, 100, "%d kb", (int)(file_stat->stat_obj->st_size / 1024));
	ewl_text_text_set(EWL_TEXT(text), itext);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);
	/*-------------------------------------*/

	/*---------------------------------*/
	hbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
	ewl_widget_show(hbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "Modified Time");
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);

	text = ewl_text_new();
	stime = file_stat->stat_obj->st_mtime;
	ewl_text_text_set(EWL_TEXT(text), ctime(&stime));
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_widget_show(text);
	/*-------------------------------------*/


	

	button = ewl_button_new();
	ewl_button_label_set(EWL_BUTTON(button), "OK");
	ewl_container_child_append(EWL_CONTAINER(vbox), button);
	ewl_object_maximum_h_set(EWL_OBJECT(button), 15);
	ewl_widget_show(button);
	ewl_callback_append(EWL_WIDGET(button), EWL_CALLBACK_CLICKED, __destroy_properties_dialog, window);

	
	
	
	/*printf("Got a 'stat available' object\n");
	printf("File size: %d\n", file_stat->stat_obj->st_size);
	printf("File inode: %d\n", file_stat->stat_obj->st_ino);
	printf("File uid: %d\n", file_stat->stat_obj->st_uid);
	printf("File gid: %d\n", file_stat->stat_obj->st_gid);
	printf("Last access: %d\n", file_stat->stat_obj->st_atime);
	printf("Last modify : %d\n", file_stat->stat_obj->st_mtime);*/


	ewl_widget_show(window);
}

