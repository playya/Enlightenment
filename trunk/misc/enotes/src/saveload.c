
/**************************************************
 **               E  -  N O T E S                **
 **                                              **
 **  The contents of this file are released to   **
 **  the public under the General Public Licence **
 **  Version 2.                                  **
 **                                              **
 **  By  Thomas Fletcher (www.fletch.vze.com)    **
 **                                              **
 **************************************************/


#include "saveload.h"

char           *saveload_selected;
SaveLoad       *saveload = NULL;

char           *load_selected;
Load           *load = NULL;



/** SAVE/LOAD WINDOW **/

/**
 * @brief: This checks whether the saveload window is already opened
 *         and if not, will open it.
 */
void
setup_saveload(void)
{
	if (saveload == NULL) {
		dml("Opening the Saveload Window", 2);
		saveload = malloc(sizeof(SaveLoad));
		setup_saveload_win();
		fill_saveload_tree();
	} else {
		error_msg("Won't Open Another Saveload Window");
	}
	return;
}

/**
 * @brief: Sets up the objects, widgets, callbacks and window for the saveload.
 */
void
setup_saveload_win(void)
{
	char           *headers[1];
	Ecore_Timer    *revtim;

	/* Setup the Window */
	saveload->win = ewl_window_new();
	ewl_window_title_set((Ewl_Window *) saveload->win, "E-Notes Save/Load");
	ewl_object_fill_policy_set((Ewl_Object*)saveload->win, EWL_FLAG_FILL_ALL);
	ewl_object_size_request((Ewl_Object*)saveload->win, 400,350);
	ewl_widget_show(saveload->win);

	saveload->vbox = ewl_vbox_new();
	ewl_container_child_append((Ewl_Container *) saveload->win,
				   saveload->vbox);
	ewl_object_fill_policy_set((Ewl_Object*)saveload->vbox, EWL_FLAG_FILL_ALL);
	ewl_widget_show(saveload->vbox);

	ewl_callback_append(saveload->win, EWL_CALLBACK_CONFIGURE,
			    save_and_load_move_embed, saveload->vbox);

	saveload->tree = ewl_tree_new(1);
	ewl_container_child_append((Ewl_Container *) saveload->vbox,
				   saveload->tree);

	headers[0] = strdup("Note Title");
	ewl_tree_headers_set((Ewl_Tree *) saveload->tree, headers);
	free(headers[0]);

	ewl_widget_show(saveload->tree);

	saveload->txt_selected = ewl_text_new("Selected: N/A");
	ewl_container_child_append((Ewl_Container *) saveload->vbox,
				   saveload->txt_selected);
	ewl_widget_show(saveload->txt_selected);

	saveload->hbox = ewl_hbox_new();
	ewl_container_child_append((Ewl_Container *) saveload->vbox,
				   saveload->hbox);
	ewl_object_fill_policy_set((Ewl_Object*)saveload->hbox,EWL_FLAG_FILL_VSHRINK);
	ewl_object_fill_policy_set((Ewl_Object*)saveload->hbox,EWL_FLAG_FILL_HFILL);
	ewl_widget_show(saveload->hbox);

	saveload_setup_button(saveload->hbox, &saveload->savebtn, "Save.");
	saveload_setup_button(saveload->hbox, &saveload->loadbtn, "Load.");
	saveload_setup_button(saveload->hbox, &saveload->refreshbtn,
			      "Refresh.");
	saveload_setup_button(saveload->hbox, &saveload->closebtn, "Close.");

	/* EWL Callbacks */
	ewl_callback_append(saveload->refreshbtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_saveload_revert,
			    (void *) saveload->tree);
	ewl_callback_append(saveload->closebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_saveload_close,
			    (void *) saveload->win);
	ewl_callback_append(saveload->win, EWL_CALLBACK_DELETE_WINDOW,
			    (void *) ewl_saveload_close,
			    (void *) saveload->win);
	ewl_callback_append(saveload->savebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_saveload_save, NULL);
	ewl_callback_append(saveload->loadbtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_saveload_load, NULL);

	return;
}

/**
 * @param c: The container widget to append the button to.
 * @param b: Pointer to a pointer of the widget to make the button in.
 * @param label: The label to apply to the button.
 * @brief: Creates and appends the button into the container (c) with
 *         the lable of label.  The widget used to make the button is
 *         b (a pointer to a pointer).
 */
void
saveload_setup_button(Ewl_Widget * c, Ewl_Widget ** b, char *label)
{
	*b = ewl_button_new(label);
	ewl_container_child_append((Ewl_Container *) c, *b);
	ewl_widget_show(*b);
	return;
}

/**
 * @param: Reads through all of the notes and in a cycle appends new
 *         rows to the tree.
 */
void
fill_saveload_tree(void)
{
	Evas_List      *p;

	p = (Evas_List *) get_cycle_begin();
	if (p != NULL) {
		setup_saveload_opt(saveload->tree,
				   (char *) get_title_by_note(p), p);
		while ((p = (Evas_List *) get_cycle_next_note(p)) != NULL) {
			if (strcmp(get_title_by_note(p), "")) {
				setup_saveload_opt(saveload->tree, (char *)
						   get_title_by_note(p), p);
			}
		}
	}
	return;
}

/**
 * @param tree: The tree widget to append the new row too.
 * @param caption: The text to apply to the new row (note title).
 * @param p: The evas_list which contains the Note structure of the note
 *           the new row refers to for accessibility.
 * @brief: This will setup a row into the tree (tree) with the text (caption)
 *         but will store the row pointer in the note it refers to (p).  This
 *         allows for more efficient sync'ing and updating.
 */
void
setup_saveload_opt(Ewl_Widget * tree, char *caption, Evas_List * p)
{
	Ewl_Widget     *capt;
	Note           *d = evas_list_data(p);

	capt = (Ewl_Widget *) ewl_text_new(caption);
	ewl_callback_append(capt, EWL_CALLBACK_CLICKED,
			    (void *) ewl_saveload_listitem_click, NULL);
	ewl_object_fill_policy_set((Ewl_Object*)capt, EWL_FLAG_FILL_ALL);
	ewl_widget_show(capt);
	d->saveload_row =
		(Ewl_Row *) ewl_tree_row_add((Ewl_Tree *) tree, 0, &capt);
	ewl_object_fill_policy_set((Ewl_Object*)d->saveload_row, EWL_FLAG_FILL_ALL);
	return;
}

/* Callbacks */

/**
 * @param widget: The widget clicked (we don't use this).
 * @param ev_data: Event data, we don't use this either.
 * @param p: Thats our data, its the tree we're going to empty and refill.
 * @brief: Callback for the refresh button being clicked.  This is the
 *         complete refreshing, so we clear all contents of the tree and
 *         rebuild it from scratch. :)
 */
void
ewl_saveload_revert(Ewl_Widget * widget, void *ev_data, Ewl_Widget * p)
{
	dml("Refreshing the Saveload List", 2);

	ewl_container_reset((Ewl_Container *) p);
	fill_saveload_tree();

	return;
}

/**
 * @param o: The widget clicked.  We don't use this.
 * @param ev_data: The event data, we don't use this either.
 * @param ee: Our predefined Ecore_Evas (window) which has been closed,
 *            so we can supply it to the ecore close callback. :-)
 * @brief: Ewl close button clicked callback.  So we call the ecore close
 *         callback which does the work. :)
 */
void
ewl_saveload_close(Ewl_Widget * o, void *ev_data, Ecore_Evas * ee)
{
	ewl_widget_destroy(saveload->win);
	free(saveload);
	saveload = NULL;
	return;
}

/**
 * @param o: Ewl widget which was clicked.  We don't use this.
 * @param ev_data: The event data.  We don't use this either.
 * @param null: A NULL pointer to please the compiler.  We don't use this.
 * @brief: When a row from the tree is clicked, we set the saveload_selected
 *         string so when we wanna do soemthing, we know what to do it to.
 */
void
ewl_saveload_listitem_click(Ewl_Widget * o, void *ev_data, void *null)
{
	char           *tmp = malloc(MAX_TITLE);

	saveload_selected = ewl_text_text_get((Ewl_Text *) o);
	snprintf(tmp, MAX_TITLE, "Selected: %s", saveload_selected);
	ewl_text_text_set((Ewl_Text *) saveload->txt_selected, tmp);
	free(tmp);
	return;
}

/**
 * @param o: Widget which was clicked.  We don't use this.
 * @param ev_data: Event data, we don't use this.
 * @param null: A NULL pointer to please the compiler.  We don't use this.
 * @brief: The load button callback, we call the function which does the
 *         work (setup_load) so we get a nice load window up. :-)
 */
void
ewl_saveload_load(Ewl_Widget * o, void *ev_data, void *null)
{
	setup_load();
	return;
}

/**
 * @param o: The widget which was clicked.  We don't use this.
 * @param ev_data: The event data, we don't use this.
 * @param null: A NULL pointer to keep the compiler happy.  We don't use this.
 * @brief: The save button is clicked, so we're going to search through the
 *         note structures (from the lists) and find the title which 
 *         corrosponds with that in the saveload_selected character array
 *         and save it using the storage/xml backend to an xml file for
 *         future loading.  A mouthful. :-)
 */
void
ewl_saveload_save(Ewl_Widget * o, void *ev_data, void *null)
{
	int             x, y, w, h;
	Note           *note;
	Evas_List      *p;
	NoteStor       *n = alloc_note_stor();

	dml("Saving Selected Note", 2);

	if (saveload_selected == NULL)
		return;
	if ((p = get_note_by_title(saveload_selected)) == NULL) {
		dml("Trying to save a note that doesn't exist", 2);
		return;
	}
	note = evas_list_data(p);
	ecore_evas_geometry_get(note->win, &x, &y, &w, &h);
	n->width = w;
	n->height = h;
	n->title = strdup(get_title_by_note(p));
	n->content = strdup(get_content_by_note(p));

	append_note_stor(n);

	free_note_stor(n);
	return;
}



/** LOAD WINDOW **/

/**
 * @brief: Checks whether the load window is open and if not, opens it.
 */
void
setup_load(void)
{
	if (load == NULL) {
		dml("Opening the Load Note Window", 2);
		load = malloc(sizeof(Load));
		setup_load_win();
		fill_load_tree();
	} else {
		error_msg("Won't Open Another Load Note Window");
	}
	return;
}

/**
 * @brief: Sets up the widgets, objects, callbacks and window for the
 *         loading window.
 */
void
setup_load_win(void)
{
	char           *headers[1];

	/* Setup the Window */
	load->win = ewl_window_new();
	ewl_window_title_set((Ewl_Window *) load->win, "E-Notes Load");
	ewl_object_fill_policy_set((Ewl_Object*)load->win, EWL_FLAG_FILL_ALL);
	ewl_object_size_request((Ewl_Object*)load->win, 400,350);
	ewl_widget_show(load->win);

	load->vbox = ewl_vbox_new();
	ewl_container_child_append((Ewl_Container *) load->win, load->vbox);
	ewl_object_fill_policy_set((Ewl_Object *) load->vbox,
				   EWL_FLAG_FILL_ALL);
	ewl_widget_show(load->vbox);

	ewl_callback_append(load->win, EWL_CALLBACK_CONFIGURE,
			    save_and_load_move_embed, load->vbox);

	load->tree = ewl_tree_new(1);
	ewl_container_child_append((Ewl_Container *) load->vbox, load->tree);

	headers[0] = strdup("Note Title");
	ewl_tree_headers_set((Ewl_Tree *) load->tree, headers);
	free(headers[0]);

	ewl_widget_show(load->tree);

	load->txt_selected = ewl_text_new("Selected: N/A");
	ewl_container_child_append((Ewl_Container *) load->vbox,
				   load->txt_selected);
	ewl_widget_show(load->txt_selected);

	load->hbox = ewl_hbox_new();
	ewl_container_child_append((Ewl_Container *) load->vbox, load->hbox);
	ewl_object_fill_policy_set((Ewl_Object *) load->hbox,
				   EWL_FLAG_FILL_VSHRINK);
	ewl_object_fill_policy_set((Ewl_Object *) load->hbox,
				   EWL_FLAG_FILL_HFILL);
	ewl_widget_show(load->hbox);

	load_setup_button(load->hbox, &load->loadbtn, "Load.");
	load_setup_button(load->hbox, &load->deletebtn, "Delete.");
	load_setup_button(load->hbox, &load->refreshbtn, "Refresh.");
	load_setup_button(load->hbox, &load->closebtn, "Close.");

	/* EWL Callbacks */
	ewl_callback_append(load->refreshbtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_load_revert, (void *) load->tree);
	ewl_callback_append(load->closebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_load_close, (void *) load->win);
	ewl_callback_append(load->win, EWL_CALLBACK_DELETE_WINDOW,
			    (void *) ewl_load_close, (void *) load->win);
	ewl_callback_append(load->loadbtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_load_load, NULL);
	ewl_callback_append(load->deletebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_load_delete, NULL);

	return;
}

void
load_setup_button(Ewl_Widget * c, Ewl_Widget ** b, char *label)
{
	*b = ewl_button_new(label);
	ewl_container_child_append((Ewl_Container *) c, *b);
	ewl_widget_show(*b);
	return;
}

/**
 * @param: Reads through all of the saved notes in the xml storage file 
 *         and in a cycle appends new rows to the tree.
 */
void
fill_load_tree(void)
{
	NoteStor       *p;
	XmlReadHandle  *r;

	r = stor_cycle_begin();

	if (r != NULL) {
		while (r->cur != NULL) {
			p = stor_cycle_get_notestor(r);
			setup_load_opt(load->tree, p->title);
			free_note_stor(p);
			stor_cycle_next(r);
		}
		stor_cycle_end(r);
	}
	return;
}

/**
 * @param tree: The tree widget to append the new row too.
 * @param caption: The text to apply to the new row (note title).
 * @brief: This will setup a row into the load tree (tree) with
 *         the text (caption).
 */
void
setup_load_opt(Ewl_Widget * tree, char *caption)
{
	Ewl_Widget     *capt;

	capt = ewl_text_new(caption);
	ewl_callback_append(capt, EWL_CALLBACK_CLICKED,
			    (void *) ewl_load_listitem_click, NULL);
	ewl_widget_show(capt);
	ewl_tree_row_add((Ewl_Tree *) tree, 0, &capt);
	return;
}

/**                        
 * @param widget: The widget clicked (we don't use this).
 * @param ev_data: Event data, we don't use this either.
 * @param p: Thats our data, its the tree we're going to empty and refill.
 * @brief: Callback for the refresh button being clicked.  This is the
 *         complete refreshing, so we clear all contents of the tree and
 *         rebuild it from scratch. :)
 */
void
ewl_load_revert(Ewl_Widget * widget, void *ev_data, Ewl_Widget * p)
{
	dml("Refreshing the Load Note List", 2);
	ewl_container_reset((Ewl_Container *) load->tree);
	fill_load_tree();
	return;
}

/**
 * @param o: The widget clicked.  We don't use this.
 * @param ev_data: The event data, we don't use this either.
 * @param ee: Our predefined Ecore_Evas (window) which has been closed,
 *            so we can supply it to the ecore close callback. :-)
 * @brief: Ewl close button clicked callback.  So we call the ecore close
 *         callback which does the work. :)
 */
void
ewl_load_close(Ewl_Widget * o, void *ev_data, Ecore_Evas * ee)
{
	ewl_widget_destroy(load->win);
	free(load);
	load = NULL;
	return;
}

/**
 * @param o: Widget which was clicked.  We don't use this.
 * @param ev_data: Event data, we don't use this.
 * @param null: A NULL pointer to please the compiler.  We don't use this.
 * @brief: The load button callback, this cycles through the xml file
 *         to find the selected note, gathers the rest of the information
 *         and opens the note.
 */
void
ewl_load_load(Ewl_Widget * o, void *ev_data, void *null)
{
	NoteStor       *p;
	XmlReadHandle  *r;

	dml("Loading Saved Note", 2);

	r = stor_cycle_begin();

	if (r != NULL) {
		while (r->cur != NULL) {
			p = stor_cycle_get_notestor(r);
			if (!strcmp(p->title, load_selected))
				new_note_with_values(p->width, p->height,
						     p->title, p->content);
			free_note_stor(p);
			stor_cycle_next(r);
		}
		stor_cycle_end(r);
	}
	return;
}

/**     
 * @param o: Ewl widget which was clicked.  We don't use this.
 * @param ev_data: The event data.  We don't use this either.
 * @param null: A NULL pointer to please the compiler.  We don't use this.
 * @brief: When a row from the tree is clicked, we set the load_selected
 *         string so when we wanna do soemthing, we know what to do it to.
 */
void
ewl_load_listitem_click(Ewl_Widget * o, void *ev_data, void *null)
{
	char           *tmp = malloc(MAX_TITLE);

	load_selected = ewl_text_text_get((Ewl_Text *) o);
	snprintf(tmp, MAX_TITLE, "Selected: %s", load_selected);
	ewl_text_text_set((Ewl_Text *) load->txt_selected, tmp);
	free(tmp);
	return;
}

/**     
 * @param o: Ewl widget which was clicked.  We don't use this.
 * @param ev_data: The event data.  We don't use this either.
 * @param null: A NULL pointer to please the compiler.  We don't use this.
 * @brief: Removes the selected note entry from the xml file.
 */
void
ewl_load_delete(Ewl_Widget * o, void *ev_data, void *null)
{
	NoteStor       *p;
	XmlReadHandle  *r;

	dml("Deleting Saved Note", 2);

	r = stor_cycle_begin();
	if (r != NULL) {
		while (r->cur != NULL) {
			p = stor_cycle_get_notestor(r);
			if (!strcmp(p->title, load_selected)) {
				break;
			}
			free_note_stor(p);
			stor_cycle_next(r);
		}
	}

	stor_cycle_end(r);
	remove_note_stor(p);

	free_note_stor(p);
	ewl_load_revert(NULL, NULL, NULL);
	return;
}

/**
 * @param w: The widget to size according to the embed.
 * @params ev_data and user_data: Callback info.
 * @brief: Moves embed contents to correct location.
 */
void
save_and_load_move_embed(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_object_geometry_request(EWL_OBJECT(user_data), CURRENT_X(w),
				    CURRENT_Y(w), CURRENT_W(w), CURRENT_H(w));
}
