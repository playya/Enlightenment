#include "ephoto.h"

/****************CALLBACKS*******************/
void
destroy_cb(Ewl_Widget * w, void *event, void *data)
{
	/****Shutdown Ewl Completely(Main Window close)****/
	ewl_widget_destroy(w);
	ewl_main_quit();
	return;
	event = NULL;
	data = NULL;
	/*************************************************/
}

void
destroys_cb(Ewl_Widget * w, void *event, void *data)
{
	/****Shutdown the slideshow****/
	ewl_widget_destroy(w);
	ecore_timer_del(s->timer);
	wino = 0;
	if ( mainwin == 0 ) {
		ewl_main_quit();
	}
	return;
	data = NULL;
	/*****************************/
}

void
destroyp_cb(Ewl_Widget * w, void *event, void *data)
{
	/****Shut down the presentation****/
	ewl_widget_destroy(s->wins);
	wino = 0;
	if ( mainwin == 0 ) {
		ewl_main_quit();
	}
	return;
	data = NULL;
	/**********************************/
}
void
destroywin_cb(Ewl_Widget * w, void *event, void *data)
{
	ewl_widget_destroy(data);
	return;
	data = NULL;
}

void
rad_cb(Ewl_Widget * w, void *event, void *data)
{
	/****Setup Callbacks for the radio buttons for size/length****/
	if ( w == m->slidetime ) {
		ewl_checkbutton_checked_set(EWL_CHECKBUTTON(m->audiolen), FALSE);
	}
	if ( w == m->fullrad ) {
		ewl_text_text_set(EWL_TEXT(m->wsize), "Full");
		ewl_text_text_set(EWL_TEXT(m->hsize), "Full");
		ewl_entry_editable_set(EWL_ENTRY(m->wsize), 0);
		ewl_entry_editable_set(EWL_ENTRY(m->hsize), 0);
	}
	if ( w == m->rad4 ) {
		ewl_text_text_set(EWL_TEXT(m->wsize), argwidth);
		ewl_text_text_set(EWL_TEXT(m->hsize), argheight);
		ewl_entry_editable_set(EWL_ENTRY(m->wsize), 1);
		ewl_entry_editable_set(EWL_ENTRY(m->hsize), 1);
	}
	/****************************************************/
}
void
ephoto_men_cb(Ewl_Widget * w, void *event, void *data)
{
	Ewl_Widget *hwin;
	Ewl_Widget *vbox;
	Ewl_Widget *text;
	Ewl_Widget *cancel;
	static char *help_text = "-To view an image, select the simple image viewer tab, and then click an image from\n"
				 "the browser.To add this image to the slideshow/presentation, click the add to slideshow\n"
				 "button. Also, to add images to the slideshow/presentation, click the slideshow/presentation\n"
				 "tab, and then click an image from the browser.  Use the self explanatory settings box to add\n"
				 "features.  Length of slideshow is the amount of time each image will be shown in the slideshow.\n"
				 "Loop slideshow loops the slideshow. Fit to audio, fits the length of the slideshow to the\n"
				 "length of the audio.  You can set the width and height of the slideshow by manipulating the\n"
				 "Custom or Fullscreen settings.  Audio thats added via the combo box, will play during the\n"
				 "slideshow. Finally, select whether you want a slideshow, orpresentation via the button.\n"
				 "Command line options can be viewed by doing ephoto --help from a terminal.\n";
	static char *about_text = "-Ephoto is an advanced image viewer.  It is written in .c and ewl.  It allows you to\n"
				  "browse images, and view them either in a simple image viewer form, or in a slideshow or\n"
				  "controlled presentation.  If emotioned is compiled, you can have audio play while your\n"
				  "images are being shown in the slideshow.  For help see help from the ephoto menu. \n"
				  "For options, see ephoto --help from the terminal.\n";
	hwin = ewl_window_new();
        ewl_window_title_set(EWL_WINDOW(hwin), "Help");
        ewl_window_name_set(EWL_WINDOW(hwin), "Help");
        ewl_object_size_request(EWL_OBJECT(hwin), 400, 300);
        ewl_callback_append(hwin, EWL_CALLBACK_DELETE_WINDOW, destroywin_cb, hwin);
        ewl_widget_show(hwin);
	
        vbox = ewl_vbox_new();
        ewl_container_child_append(EWL_CONTAINER(hwin), vbox);
        ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
        ewl_widget_show(vbox);

        text = ewl_text_new();
        ewl_container_child_append(EWL_CONTAINER(vbox), text);
        ewl_object_alignment_set(EWL_OBJECT(text), EWL_FLAG_ALIGN_CENTER);
        ewl_text_font_size_set(EWL_TEXT(text), 12);
	if ( data == "help" ) {
        	ewl_text_text_set(EWL_TEXT(text), help_text);
	}
	if ( data == "about" ) {
		ewl_text_text_set(EWL_TEXT(text), about_text);
	}
	ewl_widget_show(text);
	
        cancel = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(cancel), "Close");
        ewl_container_child_append(EWL_CONTAINER(vbox), cancel);
        ewl_object_alignment_set(EWL_OBJECT(cancel), EWL_FLAG_ALIGN_CENTER);
        ewl_object_maximum_size_set(EWL_OBJECT(cancel), 80, 15);
        ewl_callback_append(cancel, EWL_CALLBACK_CLICKED, destroywin_cb, hwin);
        ewl_widget_show(cancel);
}

void slideshow_save_cb(Ewl_Widget *w, void *event, void *data)
{
	Ewl_Widget *vbox;
	Ewl_Widget *hbox;
	Ewl_Widget *text;
	Ewl_Widget *entry;
	Ewl_Widget *ok;
	Ewl_Widget *cancel;

        m->save_win = ewl_window_new();
        ewl_window_title_set(EWL_WINDOW(m->save_win), "Save As");
        ewl_window_name_set(EWL_WINDOW(m->save_win), "Save As");
        ewl_object_size_request(EWL_OBJECT(m->save_win), 200, 100);
        ewl_callback_append(m->save_win, EWL_CALLBACK_DELETE_WINDOW, destroywin_cb, m->save_win);
        ewl_widget_show(m->save_win);

        vbox = ewl_vbox_new();
        ewl_container_child_append(EWL_CONTAINER(m->save_win), vbox);
        ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
	ewl_widget_show(vbox);

	text = ewl_text_new();
        ewl_text_text_set(EWL_TEXT(text), "Save As(Enter a name, not a path)");
	ewl_container_child_append(EWL_CONTAINER(vbox), text);
        ewl_object_alignment_set(EWL_OBJECT(text), EWL_FLAG_ALIGN_CENTER);
        ewl_text_font_size_set(EWL_TEXT(text), 12);
        ewl_widget_show(text);

	entry = ewl_entry_new();
        ewl_object_maximum_size_set(EWL_OBJECT(entry), 120, 10);
        ewl_object_alignment_set(EWL_OBJECT(entry), EWL_FLAG_ALIGN_CENTER);
        ewl_container_child_append(EWL_CONTAINER(vbox), entry);
        ewl_text_font_size_set(EWL_TEXT(entry), 10);
        ewl_widget_show(entry);

        hbox = ewl_hbox_new();
        ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
        ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(hbox), EWL_FLAG_ALIGN_CENTER);
	ewl_widget_show(hbox);
	
        ok = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(ok), "Ok");
        ewl_container_child_append(EWL_CONTAINER(hbox), ok);
        ewl_object_alignment_set(EWL_OBJECT(ok), EWL_FLAG_ALIGN_CENTER);
        ewl_object_maximum_size_set(EWL_OBJECT(ok), 80, 15);
        ewl_callback_append(ok, EWL_CALLBACK_CLICKED, save_cb, entry);
        ewl_widget_show(ok);

        cancel = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(cancel), "Close");
        ewl_container_child_append(EWL_CONTAINER(hbox), cancel);
        ewl_object_alignment_set(EWL_OBJECT(cancel), EWL_FLAG_ALIGN_CENTER);
        ewl_object_maximum_size_set(EWL_OBJECT(cancel), 80, 15);
        ewl_callback_append(cancel, EWL_CALLBACK_CLICKED, destroywin_cb, m->save_win);
        ewl_widget_show(cancel);

}

void save_cb(Ewl_Widget *w, void *event, void *data)
{
	char homepath[PATH_MAX];
	char *home = getenv("HOME");
	char *name;
	char create_path[PATH_MAX];
	FILE *file_ptr;

	snprintf(homepath, PATH_MAX, "%s/.e/ephoto/", home);
	
	if ( !ecore_file_exists(homepath) ) {
		ecore_file_mkdir(homepath);
	}
	
	name = ewl_text_text_get(EWL_TEXT(data));

	if ( name != NULL ) {	
		snprintf(create_path, PATH_MAX, "%s%s", homepath, name);

		if ( ecore_file_exists(create_path) ) {
			ecore_file_recursive_rm(create_path);
		}
		
		file_ptr = fopen(create_path, "w");

		while ( !ecore_dlist_is_empty(m->imagelist) ) {
			char *tempo;
			char tempo2[PATH_MAX];
			tempo = ecore_dlist_remove_first(m->imagelist);
			snprintf(tempo2, PATH_MAX, "%s\n", tempo);
			fputs(tempo2, file_ptr);
		}
		fclose(file_ptr);
	}
	
	ewl_widget_destroy(m->save_win);
}
void slideshow_load_cb(Ewl_Widget *w, void *event, void *data)
{
        Ewl_Widget *vbox;
        Ewl_Widget *hbox;
	Ewl_Widget *rtext;
        Ewl_Widget *tree;
        Ewl_Widget *ok;
        Ewl_Widget *cancel;
	Ewl_Widget *row;
	Ewl_Widget *children[1];
	char *home = getenv("HOME");
	char homepath[PATH_MAX];	
	Ecore_List *slideshows;

	slideshows = ecore_list_new();

	snprintf(homepath, PATH_MAX, "%s/.e/ephoto", home);

        m->load_win = ewl_window_new();
        ewl_window_title_set(EWL_WINDOW(m->load_win), "Load");
        ewl_window_name_set(EWL_WINDOW(m->load_win), "Load");
        ewl_object_size_request(EWL_OBJECT(m->load_win), 200, 210);
        ewl_callback_append(m->load_win, EWL_CALLBACK_DELETE_WINDOW, destroywin_cb, m->load_win);
        ewl_widget_show(m->load_win);

        vbox = ewl_vbox_new();
        ewl_container_child_append(EWL_CONTAINER(m->load_win), vbox);
        ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
        ewl_widget_show(vbox);

        tree = ewl_tree_new(1);
        ewl_container_child_append(EWL_CONTAINER(vbox), tree);
        ewl_tree_headers_visible_set(EWL_TREE(tree), 0);
        ewl_tree_expandable_rows_set(EWL_TREE(tree), FALSE);
        ewl_object_maximum_size_set(EWL_OBJECT(tree), 200, 215);
        ewl_widget_show(tree);
	

	if ( ecore_file_is_dir(homepath) ) {
		slideshows = ecore_file_ls(homepath);
		
		while ( !ecore_list_is_empty(slideshows) ) {
			char *tempo;
			tempo = ecore_list_remove_first(slideshows);
	
       	                rtext = ewl_text_new();
       	                ewl_widget_name_set(rtext, tempo);
       	         	ewl_text_text_set(EWL_TEXT(rtext), tempo);
                	ewl_object_minimum_size_set(EWL_OBJECT(rtext), 10, 16);
        	        ewl_object_fill_policy_set(EWL_OBJECT(rtext), EWL_FLAG_FILL_ALL);
               		ewl_widget_show(rtext);

     	                children[0] = rtext;
        	        children[1] = NULL;
                	row = ewl_tree_row_add(EWL_TREE(tree), NULL, children);
                	ewl_callback_append(rtext, EWL_CALLBACK_CLICKED, loadclicked_cb, NULL);
		}
	}
        m->otext = ewl_text_new();
        ewl_container_child_append(EWL_CONTAINER(vbox), m->otext);
        ewl_object_alignment_set(EWL_OBJECT(m->otext), EWL_FLAG_ALIGN_CENTER);
        ewl_text_font_size_set(EWL_TEXT(m->otext), 12);
        ewl_widget_show(m->otext);

        hbox = ewl_hbox_new();
        ewl_container_child_append(EWL_CONTAINER(vbox), hbox);
        ewl_object_fill_policy_set(EWL_OBJECT(hbox), EWL_FLAG_FILL_SHRINK);
        ewl_object_alignment_set(EWL_OBJECT(hbox), EWL_FLAG_ALIGN_CENTER);
        ewl_widget_show(hbox);

        ok = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(ok), "Ok");
        ewl_container_child_append(EWL_CONTAINER(hbox), ok);
        ewl_object_alignment_set(EWL_OBJECT(ok), EWL_FLAG_ALIGN_CENTER);
        ewl_object_maximum_size_set(EWL_OBJECT(ok), 80, 15);
        ewl_callback_append(ok, EWL_CALLBACK_CLICKED, load_cb, NULL);
        ewl_widget_show(ok);

        cancel = ewl_button_new();
        ewl_button_label_set(EWL_BUTTON(cancel), "Close");
        ewl_container_child_append(EWL_CONTAINER(hbox), cancel);
        ewl_object_alignment_set(EWL_OBJECT(cancel), EWL_FLAG_ALIGN_CENTER);
        ewl_object_maximum_size_set(EWL_OBJECT(cancel), 80, 15);
        ewl_callback_append(cancel, EWL_CALLBACK_CLICKED, destroywin_cb, m->load_win);
        ewl_widget_show(cancel);
}

void loadclicked_cb(Ewl_Widget *w, void *event, void *data)
{
	char *rpath;
	rpath = ewl_widget_name_get(w);
	ewl_text_text_set(EWL_TEXT(m->otext), rpath);
}

void load_cb(Ewl_Widget *w, void *event, void *data)
{
	char *home = getenv("HOME");
	char homepath[PATH_MAX];
	char *apath;
	FILE *file_ptr;	

	apath = ewl_text_text_get(EWL_TEXT(m->otext));
	snprintf(homepath, PATH_MAX, "%s/.e/ephoto/%s", home, apath);
	printf("%s\n", homepath);

	file_ptr = fopen(homepath, "r");

	if (file_ptr != NULL) {
		char paths[PATH_MAX];
		while (fgets(paths,PATH_MAX,file_ptr)!=NULL) {
			char path2[PATH_MAX];
			int strleng;
			
			strleng = strlen(paths);
		
			snprintf(path2, strleng, "%s", paths);

			printf("%s\n", path2);			

			m->i = ewl_image_thumbnail_new();
                	ewl_widget_name_set(m->i, path2);
                	ewl_image_constrain_set(EWL_IMAGE(m->i), 64);
                	ewl_image_proportional_set(EWL_IMAGE(m->i), TRUE);
               		ewl_image_thumbnail_request(EWL_IMAGE(m->i), path2);
               	 	ewl_image_file_set(EWL_IMAGE(m->i), PACKAGE_DATA_DIR "images/camera.png", NULL);
               	 	ewl_container_child_append(EWL_CONTAINER(m->ib), m->i);
               	 	ewl_callback_append(m->i, EWL_CALLBACK_CLICKED, iremove_cb, NULL);
        	        ewl_widget_show(m->i);
	
                	ecore_dlist_append(m->imagelist, strdup(path2));
                	slidenum++;

			ewl_widget_enable(m->slideshow);
			ewl_widget_state_set(m->slideshow, "enabled");
			ewl_widget_enable(m->presentation);
			ewl_widget_state_set(m->presentation, "enabled");
		}
		
		fclose(file_ptr);
	}
	ewl_widget_destroy(m->load_win);	
}

