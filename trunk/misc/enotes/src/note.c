
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


#include "note.h"

extern MainConfig *main_config;

Evas_List      *gbl_notes = NULL;

/* High Level */

/**
 * @brief: Opens a new note.
 */
void
new_note(void)
{
	Evas_List      *new;

	dml("Creating a Note", 2);

	new = append_note();
	setup_note(&new, 0, 0, 0, 0, DEF_CONTENT);
	return;
}

/**
 * @param width: Width of the new note.
 * @param height: Height of the new note.
 * @param title: Title text to begin with.
 * @param content: Content text to begin with.
 * @brief: Opens a new note.
 */
void
new_note_with_values(int x, int y, int width, int height, char *content)
{
	Evas_List      *new;

	dml("Creating a Note", 2);

	new = append_note();
	setup_note(&new, x, y, width, height, content);
	return;
}

/* Lists and Allocation */

/**
 * @return: Evas_List pointer to the new note created in the list.
 * @brief: Initialise the Note and add it to the list.
 */
Evas_List      *
append_note(void)
{
	Note           *note = malloc(sizeof(Note));

	/* Set NULL's */
	note->txt_title = NULL;

	gbl_notes = evas_list_append(gbl_notes, note);
	return (evas_list_find_list(gbl_notes, note));
}

/**
 * @param note: The pointer to an Evas_List containing the note.
 * @brief: Closes and frees a note.
 */
void
remove_note(Evas_List * note)
{
	Note           *p = evas_list_data(note);
	char           *note_title;

	dml("Closing a Note", 2);

	ecore_timer_del(p->timcomp);
	p->timcomp = NULL;

	edje_object_part_unswallow(p->edje, p->eo);
	ewl_widget_destroy(p->emb);
	evas_object_del(p->edje);

	ecore_evas_free(p->win);
	free(p);
	gbl_notes = evas_list_remove_list(gbl_notes, note);

	/** 
	 * FIXME: When you can get the row and its child text, compare
	 * it to the ewl_entry_text_get(p->title) value and remove the row
	 * from the tree at this point.  Reporting that you've done so with
	 * dml ("Removed note from save/load list", 2); or something.  When ewl
	 * will let you do these things.
	 */

	if (saveload != NULL) {
		dml("Removing note entry from saveload list", 2);
		ewl_tree_row_destroy((Ewl_Tree *) saveload->tree,
				     p->saveload_row);
	}

	/*  Check if it was the last note  */
	if (evas_list_next(note) == NULL && evas_list_prev(note) == NULL &&
	    controlcentre == NULL)
		ecore_main_loop_quit();

	return;
}

/* GUI Setup */

/**
 * @param note: The note to setup (pointer to a pointer).
 * @param width: Width of the new notes window.
 * @param height: Height of the new notes window.
 * @param title: Title to begin with.
 * @param content: Content to begin with.
 * @brief: Sets up the note objects, window, callbacks, etc...
 */
void
setup_note(Evas_List ** note, int x, int y, int width, int height,
	   char *content)
{
	Evas_List      *pl;
	Note           *p;

	char           *fontpath = malloc(PATH_MAX);
	char           *edjefn = malloc(PATH_MAX);
	char           *datestr;
	char           *fcontent;
	char           *prop;

	Evas_Coord      edje_w, edje_h;

	/* Fix Newlines in Content */
	fcontent = fix_newlines(content);

	/* Get the Note from the Evas_List** */
	pl = *note;
	p = evas_list_data(pl);


	/* Setup the Window */
	if (!strcmp(main_config->render_method, "gl")) {
#ifdef HAVE_ECORE_EVAS_GL
		p->win = ecore_evas_gl_x11_new(NULL, 0, x, y, width, height);
#else
		dml("GL not in Ecore_Evas module.  Falling back on software!",
		    1);
		free(main_config->render_method);
		main_config->render_method = strdup("software");
		p->win = ecore_evas_software_x11_new(NULL, 0, x, y, width,
						     height);
#endif
	} else
		p->win = ecore_evas_software_x11_new(NULL, 0, x, y, width,
						     height);

	ecore_evas_borderless_set(p->win, 1);
	ecore_evas_shaped_set(p->win, 1);
	ecore_evas_title_set(p->win, "An E-Note");

	if (main_config->ontop == 1)
		if (!strcmp(main_config->render_method, "gl")) {
			ecore_x_window_prop_layer_set
				(ecore_evas_gl_x11_window_get(p->win),
				 ECORE_X_WINDOW_LAYER_ABOVE);
		} else {
			ecore_x_window_prop_layer_set
				(ecore_evas_software_x11_window_get(p->win),
				 ECORE_X_WINDOW_LAYER_ABOVE);
		}

	if (main_config->sticky == 1)
		ecore_evas_sticky_set(p->win, 1);
	else
		ecore_evas_sticky_set(p->win, 0);

	ecore_evas_show(p->win);


	/* Move the damn thing  */
	if (!strcmp(main_config->render_method, "gl"))
		ecore_x_window_prop_xy_set(ecore_evas_gl_x11_window_get(p->win),
					   x, y);
	else
		ecore_x_window_prop_xy_set(ecore_evas_software_x11_window_get
					   (p->win), x, y);

	/* Setup the Canvas, fonts, etc... */
	p->evas = ecore_evas_get(p->win);
	evas_output_method_set(p->evas,
			       evas_render_method_lookup(main_config->
							 render_method));
	snprintf(fontpath, PATH_MAX, "%s/fonts", PACKAGE_DATA_DIR);
	evas_font_path_append(p->evas, fontpath);

	/* Draggable Setup */
	p->dragger = (Evas_Object *) esmart_draggies_new(p->win);
	evas_object_name_set(p->dragger, "dragger");
	evas_object_move(p->dragger, 0, 0);
	evas_object_resize(p->dragger, width, height);
	evas_object_layer_set(p->dragger, 0);
	evas_object_color_set(p->dragger, 255, 255, 255, 0);
	esmart_draggies_button_set(p->dragger, 1);
	evas_object_show(p->dragger);

	p->eventer = evas_object_rectangle_add(p->win);
	evas_object_color_set(p->eventer, 0, 0, 0, 0);
	evas_object_resize(p->eventer, width, height);
	evas_object_move(p->eventer, 0.0, 0.0);
	evas_object_layer_set(p->eventer, 9999);
	evas_object_repeat_events_set(p->eventer, 1);
	evas_object_show(p->eventer);

	evas_object_event_callback_add(p->eventer, EVAS_CALLBACK_MOUSE_DOWN,
				       (void *) cb_menu_rightclick, p);

	/* Setup the Edje */
	p->edje = edje_object_add(p->evas);
	snprintf(edjefn,
		 PATH_MAX, NOTE_EDJE, PACKAGE_DATA_DIR, main_config->theme);
	edje_object_file_set(p->edje, edjefn, NOTE_PART);
	evas_object_name_set(p->edje, "edje");
	evas_object_move(p->edje, 0, 0);
	evas_object_layer_set(p->edje, 1);

	edje_object_size_max_get(p->edje, &edje_w, &edje_h);
	ecore_evas_size_max_set(p->win, edje_w, edje_h);
	edje_object_size_min_get(p->edje, &edje_w, &edje_h);
	ecore_evas_size_min_set(p->win, edje_w, edje_h);

	if (width == 0 && height == 0) {
		ecore_evas_resize(p->win, (int) edje_w, (int) edje_h);
		evas_object_resize(p->edje, (int) edje_w, (int) edje_h);
	} else {
		ecore_evas_resize(p->win, width, height);
		evas_object_resize(p->edje, width, height);
	}

	evas_object_show(p->edje);

	/* Setup the date, user and initial title */
	edje_object_part_text_set(p->edje, EDJE_TEXT_USER, getenv("USER"));
	datestr = get_date_string();
	edje_object_part_text_set(p->edje, EDJE_TEXT_DATE, datestr);
	update_enote_title(p->edje, content);

	/* Ewl */
	p->emb = ewl_embed_new();
	ewl_object_fill_policy_set((Ewl_Object *) p->emb, EWL_FLAG_FILL_ALL);
	ewl_widget_show(p->emb);
	p->eo = ewl_embed_evas_set(EWL_EMBED(p->emb),
				   ecore_evas_get(p->win), (void *)
				   ecore_evas_software_x11_window_get(p->win));
	evas_object_layer_set(p->eo, 2);
	edje_object_part_swallow(p->edje, EDJE_CONTAINER, p->eo);
	evas_object_show(p->eo);

	evas_object_focus_set(p->eo, TRUE);
	ewl_embed_focus_set((Ewl_Embed *) p->emb, TRUE);

	p->pane = ewl_scrollpane_new();
	ewl_container_child_append((Ewl_Container *) p->emb, p->pane);

	if (edje_object_data_get(p->edje, EDJE_INFO_SCROLLBARS) != NULL) {
		configure_scrollbars(p->pane, edjefn);
	}

	ewl_widget_show(p->pane);

	p->content = ewl_entry_multiline_new("");
	ewl_container_child_append((Ewl_Container *) p->pane, p->content);
	ewl_entry_multiline_set((Ewl_Entry *) p->content, 1);

	ewl_theme_data_str_set(p->content, "/entry/group", "none");

	prop = (char *) edje_object_data_get(p->edje, EDJE_INFO_FONTNAME);
	if (prop != NULL)
		ewl_theme_data_str_set(p->content, "/entry/text/font", prop);

	prop = (char *) edje_object_data_get(p->edje, EDJE_INFO_FONTSTYLE);
	if (prop != NULL)
		ewl_theme_data_str_set(p->content, "/entry/text/style", prop);

	prop = (char *) edje_object_data_get(p->edje, EDJE_INFO_FONTSIZE);
	if (prop != NULL)
		ewl_theme_data_int_set(p->content, "/entry/text/font_size",
				       atoi(prop));

	ewl_entry_text_set((Ewl_Entry *) p->content, fcontent);
	ewl_widget_show(p->content);

	ewl_callback_append(p->emb, EWL_CALLBACK_CONFIGURE, note_move_embed,
			    p->pane);

	/* Ecore Callbacks */
	ecore_evas_callback_resize_set(p->win, note_ecore_resize);
	ecore_evas_callback_destroy_set(p->win, note_ecore_close);
	ecore_evas_callback_delete_request_set(p->win, note_ecore_close);

	/* Edje Callbacks */
	edje_object_signal_callback_add(p->edje,
					EDJE_SIGNAL_NOTE_CLOSE, "",
					(void *) note_edje_close, *note);
	edje_object_signal_callback_add(p->edje,
					EDJE_SIGNAL_NOTE_MINIMISE, "",
					(void *) note_edje_minimise, *note);

	/* Free Temporarily used Variables */
	if (datestr != NULL)
		free(datestr);
	if (edjefn != NULL)
		free(edjefn);
	if (fontpath != NULL)
		free(fontpath);
	if (fcontent != NULL)
		free(fcontent);

	/* Values Comparison Timer */
	p->timcomp = ecore_timer_add(COMPARE_INTERVAL, &timer_val_compare, p);

	if (saveload != NULL) {
		setup_saveload_opt(saveload->tree, (char *)
				   get_title_by_note(*note), *note);
		dml("Added new note to saveload list", 2);
	}

	return;
}

void
configure_scrollbars(Ewl_Widget * pane, char *edjefn)
{
	ewl_theme_data_str_set(pane,
			       "/vscrollbar/button_increment/file", edjefn);
	ewl_theme_data_str_set(pane,
			       "/vscrollbar/button_decrement/file", edjefn);
	ewl_theme_data_str_set(pane, "/vscrollbar/vseeker/file", edjefn);
	ewl_theme_data_str_set(pane, "/vscrollbar/vseeker/button/file", edjefn);
	ewl_theme_data_str_set(pane,
			       "/hscrollbar/button_increment/file", edjefn);
	ewl_theme_data_str_set(pane,
			       "/hscrollbar/button_decrement/file", edjefn);
	ewl_theme_data_str_set(pane, "/hscrollbar/hseeker/file", edjefn);
	ewl_theme_data_str_set(pane, "/hscrollbar/hseeker/button/file", edjefn);

	ewl_theme_data_str_set(pane,
			       "/vscrollbar/button_increment/group",
			       EDJE_VSCROLLBAR_BTN_INCR);
	ewl_theme_data_str_set(pane,
			       "/vscrollbar/button_decrement/group",
			       EDJE_VSCROLLBAR_BTN_DECR);
	ewl_theme_data_str_set(pane, "/vscrollbar/vseeker/group",
			       EDJE_VSCROLLBAR_SEEKER);
	ewl_theme_data_str_set(pane,
			       "/vscrollbar/vseeker/button/group",
			       EDJE_SCROLLBAR_BUTTON);
	ewl_theme_data_str_set(pane,
			       "/hscrollbar/button_increment/group",
			       EDJE_HSCROLLBAR_BTN_INCR);
	ewl_theme_data_str_set(pane,
			       "/hscrollbar/button_decrement/group",
			       EDJE_HSCROLLBAR_BTN_DECR);
	ewl_theme_data_str_set(pane, "/hscrollbar/hseeker/group",
			       EDJE_HSCROLLBAR_SEEKER);
	ewl_theme_data_str_set(pane,
			       "/hscrollbar/hseeker/button/group",
			       EDJE_SCROLLBAR_BUTTON);
	return;
}

/* MENU Callbacks */

void
cb_menu_rightclick(Note * p, Evas * e, Evas_Object * obj, void *ev_info)
{
	Menu           *menu = menu_create();

	menu_item_add(p->menu, "New Note", (void *) cb_ewl_new_note, NULL);
	menu_show(menu);
	return;
}

void
cb_ewl_new_note(void *data)
{
}

/* ECORE Callbacks */

/**
 * @param ee: The Ecore_Evas which has been resized.
 * @brief: Ecore callback on a window resizing.
 *         Resizes the objects inside the window to
 *         compensate to the new size.
 */
void
note_ecore_resize(Ecore_Evas * ee)
{
	int             x, y, w, h;

	dml("Resizing Note", 2);

	ecore_evas_geometry_get(ee, &x, &y, &w, &h);
	evas_object_resize(evas_object_name_find
			   (ecore_evas_get(ee), "edje"), w, h);
	evas_object_resize(evas_object_name_find(ecore_evas_get(ee), "dragger"),
			   w, h);

	return;
}

/**
 * @param ee: Ecore_Evas which has been requested to close.
 * @brief: Ecore callback which dictates that the wm wants the note closing.
 *         Closes the note.
 */
void
note_ecore_close(Ecore_Evas * ee)
{
	Evas_List      *p = gbl_notes;
	Note           *note;

	if (ee == NULL)
		return;
	while (p != NULL) {
		note = evas_list_data(p);
		if (note->win == ee) {
			remove_note((Evas_List *) p);
			return;
		}
		p = evas_list_next(p);
	}
	return;
}

/* EDJE Callbacks */

/**
 * @param note: Evas_List of the note which is to be closed.
 * @param o: Evas_Object of the object clicked (not used).
 * @param emission: The signal string (not used).
 * @param source: The source of the signal (not used).
 * @brief: Edje callback to close.  Closes the note via a timer
 *         to save trouble with signals, etc... when it all gets freed.
 */
void
note_edje_close(Evas_List * note, Evas_Object * o,
		const char *emission, const char *source)
{
	Ecore_Timer    *timer;

	timer = ecore_timer_add(0.001, &note_edje_close_timer, note);
	return;
}

/**
 * @param note: Evas_List of the note which is to be minimised.
 * @param o: Evas_Object of the object clicked (not used).
 * @param emission: The signal string (not used).
 * @param source: The source of the signal (not used).
 * @brief: Edje callback to minimise.  Minimises the window.
 */
void
note_edje_minimise(Evas_List * note, Evas_Object * o,
		   const char *emission, const char *source)
{
	Note           *p;

	dml("Minimising a Note", 2);

	p = evas_list_data(note);

	/* FIXME: The line below should be removed when
	 * ecore_evas is fixed. */
	ecore_evas_iconified_set(p->win, 0);

	ecore_evas_iconified_set(p->win, 1);

	return;
}

/* Misc */

/**
 * @return: Returns the string containing the date (needs free'ing)
 * @brief: Grabs and formats the time into a string.
 */
char           *
get_date_string(void)
{
	char           *retval = malloc(20);
	time_t          tmp;
	struct tm      *localtm;

	tmp = time(NULL);
	localtm = (struct tm *) gmtime(&tmp);
	strftime(retval, 19, "%d/%m/%y", localtm);

	return (retval);
}

/**
 * @param p: Evas_List pointing to the note to be closed.
 * @return: Integer dictating whether the timer ends.
 * @brief: This timer is called from the edje callback to close
 *         the window to save problems with signals when the objects
 *         are freed.  It close the note and ends its own timer by
 *         returning 0.
 */
int
note_edje_close_timer(void *p)
{
	remove_note((Evas_List *) p);
	return (0);
}

/**
 * @param data: The Note of the note which is being checked.
 * @return: Integer dictating whether the timer ends.
 * @brief: Compares the values of the title to the stored values (keep getting
 *         updated) to decide whether to change the value inside of saveload if
 *         required.  This is a timer.
 */
int
timer_val_compare(void *data)
{
	Note           *p = (Note *) data;
	char           *tmp;

	if (p->timcomp == NULL)
		return (0);

	if (p->txt_title != NULL) {
		tmp = get_title_by_note_struct(p);
		if (strcmp(p->txt_title, tmp)) {
			if (saveload != NULL)
				ewl_saveload_revert(NULL, NULL, saveload->tree);

			if (p->txt_title != NULL)
				free(p->txt_title);
			p->txt_title = get_title_by_note_struct(p);
		}
		if (tmp != NULL)
			free(tmp);
	} else {
		p->txt_title = get_title_by_note_struct(p);
	}

	update_enote_title(p->edje, p->txt_title);

	return (1);
}

/* External Interaction */

int
get_note_count()
{
	int             a;
	Evas_List      *p;

	p = get_cycle_begin();
	if (p == NULL)
		return (0);
	else
		a = 1;
	while ((p = get_cycle_next_note(p)) != NULL)
		a++;

	return (a);
}

void
notes_update_themes(void)
{
	int             edje_w, edje_h;
	Evas_List      *working;
	Note           *note;
	int             count = get_note_count();

	char           *edjefn = malloc(PATH_MAX);

	snprintf(edjefn,
		 PATH_MAX, NOTE_EDJE, PACKAGE_DATA_DIR, main_config->theme);

	working = get_cycle_begin();
	if (working != NULL) {
		while (working != NULL) {
			note = (Note *) evas_list_data(working);
			if (note != NULL) {
				edje_object_file_set(note->edje, edjefn,
						     NOTE_PART);
				edje_object_size_max_get(note->edje, &edje_w,
							 &edje_h);
				ecore_evas_size_max_set(note->win, edje_w,
							edje_h);
				edje_object_size_min_get(note->edje, &edje_w,
							 &edje_h);
				ecore_evas_size_min_set(note->win, edje_w,
							edje_h);
				edje_object_part_swallow(note->edje,
							 EDJE_CONTAINER,
							 note->eo);
				if (edje_object_data_get
				    (note->edje,
				     EDJE_INFO_SCROLLBARS) != NULL) {
					/* FIXME: What the fuck is happening when
					 * we enable this?: */
//                                      configure_scrollbars(note->pane,edjefn);
				}
			}
			working = get_cycle_next_note(working);
		}
	}

	free(edjefn);
	return;
}

/**
 * @param title: The title to search for.
 * @return: Returns the Evas_List of the note requested by "title".
 * @brief: Searches for and returns the note with the title being "title"
 */
Evas_List      *
get_note_by_title(char *title)
{
	Evas_List      *a;

	a = get_cycle_begin();
	if (!strcmp(get_title_by_note(a), title)) {
		return (a);
	}
	while ((a = get_cycle_next_note(a)) != NULL) {
		if (!strcmp(get_title_by_note(a), title)) {
			return (a);
		}
	}
	return (NULL);
}

/**
 * @param content: The content to search for.
 * @return: Returns the Evas_List of the note requested by "content".
 * @brief: Searches for and returns the note with the content being "content"
 */
Evas_List      *
get_note_by_content(char *content)
{
	Evas_List      *a;

	a = get_cycle_begin();
	if (!strcmp(get_content_by_note(a), content)) {
		return (a);
	}
	while ((a = get_cycle_next_note(a)) != NULL) {
		if (!strcmp(get_content_by_note(a), content)) {
			return (a);
		}
	}
	return (NULL);
}

/**
 * @param note: The note to grab the title from.
 * @return: Returns the title of the supplied note.
 * @brief: Returns the title text of the supplied note.
 */
char           *
get_title_by_note(Evas_List * note)
{
	return (get_title_by_content(get_content_by_note(note)));
}

/**
 * @param note: The note to grab the title from (actual).
 * @return: Returns the title of the supplied note.
 * @brief: Returns the title text of the supplied note.
 */
char           *
get_title_by_note_struct(Note * note)
{
	return (get_title_by_content(get_content_by_note_struct(note)));
}

/**
 * @param note: The note to grab the content from.
 * @return: Returns the content of the supplied note.
 * @brief: Returns the content text of the supplied note.
 */
char           *
get_content_by_note(Evas_List * note)
{
	Note           *p = evas_list_data(note);

	return ((char *) ewl_entry_text_get((Ewl_Entry *) p->content));
}

/**
 * @param note: The note to grab the content from (actual).
 * @return: Returns the content of the supplied note.
 * @brief: Returns the content text of the supplied note.
 */
char           *
get_content_by_note_struct(Note * note)
{
	return ((char *) ewl_entry_text_get((Ewl_Entry *) note->content));
}

/**
 * @param content: The content of the note.
 * @return: The title from the content.
 * @brief: Takes TITLE_LENGTH worth of characters
 *         from the front (or newline).
 */
char           *
get_title_by_content(char *content)
{
	char           *cont = content;
	int             a = 0;
	int             newlength = 0;

	while (a < TITLE_LENGTH && cont != NULL) {
		if (!strncmp(cont, "\n", 1)) {
			newlength = a;
			break;
		}
		a++;
		cont++;
	}
	a = 0;

	if (newlength == 0)
		newlength = TITLE_LENGTH;

	return ((char *) strndup(content, newlength));
}

/**
 * @return: Returns the beginning node of the note list cycle.
 * @brief: Begin the note list cycle.
 */
Evas_List      *
get_cycle_begin(void)
{
	return (gbl_notes);
}

/**
 * @param note: The note to move forward from.
 * @return: Returns the node to the next note in the cycle.
 * @brief: Move to the next note in the cycle.
 */
Evas_List      *
get_cycle_next_note(Evas_List * note)
{
	return (evas_list_next(note));
}

/**
 * @param note: The note to move backwards from.
 * @return: Returns the node to the previous note in the cycle.
 * @brief: Move to the previous note in the cycle.
 */
Evas_List      *
get_cycle_previous_note(Evas_List * note)
{
	return (evas_list_prev(note));
}

/**
 * @param w: The widget to size according to the embed.
 * @params ev_data and user_data: Callback info.
 * @brief: Moves embed contents to correct location.
 */
void
note_move_embed(Ewl_Widget * w, void *ev_data, void *user_data)
{
	ewl_object_geometry_request(EWL_OBJECT(user_data), CURRENT_X(w),
				    CURRENT_Y(w), CURRENT_W(w), CURRENT_H(w));
}

/**
 * @param content: The content to use for title setting.
 * @brief: Sets the title in the edje.
 */
void
update_enote_title(Evas_Object * edje, char *content)
{
	edje_object_part_text_set(edje, EDJE_TEXT_TITLE,
				  get_title_by_content(content));
	return;
}
