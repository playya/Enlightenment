/**************************************************/
/**               E  -  N O T E S                **/
/**                                              **/
/**  The contents of this file are released to   **/
/**  the public under the General Public Licence **/
/**  Version 2.                                  **/
/**                                              **/
/**  By  Thomas Fletcher (www.fletch.vze.com)    **/
/**                                              **/
/**************************************************/


#include "settings.h"

Settings       *settings;


/* High Level */
void
setup_settings(void)
{
	if (settings == NULL) {
		dml("Opening the Settings Window", 2);
		settings = malloc(sizeof(Settings));
		setup_settings_win(settings);
		fill_tree();
	} else {
		dml("Won't Open Another Settings Window", 2);
	}
	return;
}


/* Setting up the Window */

void
setup_settings_win(Settings * s)
{
	char           *headers[2];

	/* Setup the Window */
	s->win = ecore_evas_software_x11_new(NULL, 0, SETTINGS_X, SETTINGS_Y,
					     SETTINGS_W, SETTINGS_H);
	ecore_evas_title_set(s->win, "E-Notes Settings");
	ecore_evas_show(s->win);

	/* Setup the Canvas, Render-Method */
	s->evas = ecore_evas_get(s->win);
	evas_output_method_set(s->evas,
			       evas_render_method_lookup(main_config->
							 render_method));

	/* Setup the EWL Widgets */
	s->emb = ewl_embed_new();
	ewl_object_set_fill_policy((Ewl_Object *) s->emb, EWL_FLAG_FILL_FILL);
	ewl_widget_set_appearance(s->emb, "window");
	ewl_widget_show(s->emb);

	s->eo = ewl_embed_set_evas((Ewl_Embed *) s->emb, s->evas,
				   ecore_evas_software_x11_window_get(s->win));
	evas_object_name_set(s->eo, "eo");
	evas_object_layer_set(s->eo, 0);
	evas_object_move(s->eo, 0, 0);
	evas_object_resize(s->eo, SETTINGS_W, SETTINGS_H);
	evas_object_show(s->eo);

	s->vbox = ewl_vbox_new();
	ewl_container_append_child((Ewl_Container *) s->emb, s->vbox);
	ewl_object_set_fill_policy((Ewl_Object *) s->vbox, EWL_FLAG_FILL_FILL);
	ewl_widget_show(s->vbox);

	s->tree = ewl_tree_new(2);
	ewl_container_append_child((Ewl_Container *) s->vbox, s->tree);
	ewl_object_set_fill_policy((Ewl_Object *) s->tree, EWL_FLAG_FILL_FILL);

	headers[0] = strdup("Setting");
	headers[1] = strdup("Value");
	ewl_tree_set_headers((Ewl_Tree *) s->tree, headers);
	free(headers[0]);
	free(headers[1]);

	ewl_widget_show(s->tree);

	s->hbox = ewl_hbox_new();
	ewl_container_append_child((Ewl_Container *) s->vbox, s->hbox);
	ewl_object_set_fill_policy((Ewl_Object *) s->hbox, EWL_FLAG_FILL_HFILL);
	ewl_widget_show(s->hbox);

	settings_setup_button(s->hbox, &(s->savebtn), "Save.");
	settings_setup_button(s->hbox, &(s->revertbtn), "Revert.");
	settings_setup_button(s->hbox, &(s->closebtn), "Close.");

	/* Ecore Callbacks */
	ecore_evas_callback_resize_set(s->win, ecore_settings_resize);
	ecore_evas_callback_delete_request_set(s->win, ecore_settings_close);
	ecore_evas_callback_destroy_set(s->win, ecore_settings_close);

	/* EWL Callbacks */
	ewl_callback_append(s->revertbtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_settings_revert, (void *) s->tree);
	ewl_callback_append(s->closebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_settings_close, (void *) s->win);
	ewl_callback_append(s->savebtn, EWL_CALLBACK_CLICKED,
			    (void *) ewl_settings_save, NULL);

	return;
}

void
fill_tree(void)
{
	settings->render_method =
		setup_settings_opt(settings->tree, "Render Method:",
				   main_config->render_method);
	settings->theme =
		setup_settings_opt(settings->tree, "Theme:",
				   main_config->theme);
	settings->intro =
		setup_settings_opt_int(settings->tree, "Intro:",
				       main_config->intro);

	settings->note_x =
		setup_settings_opt_int(settings->tree, "Note - X:",
				       main_config->note->x);
	settings->note_y =
		setup_settings_opt_int(settings->tree, "Note - Y:",
				       main_config->note->y);
	settings->note_w =
		setup_settings_opt_int(settings->tree, "Note - Width:",
				       main_config->note->width);
	settings->note_h =
		setup_settings_opt_int(settings->tree, "Note - Height:",
				       main_config->note->height);

	settings->cc_x =
		setup_settings_opt_int(settings->tree, "Control Centre - X:",
				       main_config->cc->x);
	settings->cc_y =
		setup_settings_opt_int(settings->tree, "Control Centre - Y:",
				       main_config->cc->y);
	settings->cc_w =
		setup_settings_opt_int(settings->tree,
				       "Control Centre - Width:",
				       main_config->cc->width);
	settings->cc_h =
		setup_settings_opt_int(settings->tree,
				       "Control Centre - Height:",
				       main_config->cc->height);
	settings->debug =
		setup_settings_opt_int(settings->tree, "Debugging Level [0-2]:",
				       main_config->debug);
	return;
}

void
settings_setup_button(Ewl_Widget * c, Ewl_Widget ** b, char *label)
{
	*b = ewl_button_new(label);
	ewl_container_append_child((Ewl_Container *) c, *b);
	ewl_widget_show(*b);
	return;
}


/* Setting up the Options */
Settings_Opt
setup_settings_opt(Ewl_Widget * tree, char *caption, char *value)
{
	Settings_Opt    oa;
	Settings_Opt   *o = &oa;
	Ewl_Widget     *entries[2];

	o->caption = ewl_text_new(caption);
	ewl_widget_show(o->caption);

	o->entry = ewl_entry_new(value);
	ewl_widget_show(o->entry);

	entries[0] = o->caption;
	entries[1] = o->entry;

	ewl_tree_add_row((Ewl_Tree *) tree, 0, entries);

	return (oa);
}

Settings_Opt
setup_settings_opt_int(Ewl_Widget * tree, char *caption, int value)
{
	Settings_Opt    o;
	char           *retval = malloc(1028);
	snprintf(retval, sizeof(int) + 1, "%d", value);
	o = setup_settings_opt(tree, caption, retval);
	free(retval);
	return (o);
}


/* Callbacks */
void
ecore_settings_resize(Ecore_Evas * ee)
{
	int             x, y, w, h;

	dml("Resizing the Settings Window", 2);

	ecore_evas_geometry_get(ee, &x, &y, &w, &h);
	evas_object_resize(evas_object_name_find(ecore_evas_get(ee), "eo"),
			   w, h);
	return;
}

void
ecore_settings_close(Ecore_Evas * ee)
{
	dml("Closing the Settings Window", 2);
	ecore_evas_free(ee);
	free(settings);
	settings = NULL;
	return;
}

void
ewl_settings_revert(Ewl_Widget * widget, void *ev_data, Ewl_Widget * p)
{
	dml("Refreshing the Settings Values", 2);
	ewl_container_reset((Ewl_Container *) p);
	fill_tree();

	return;
}

void
ewl_settings_close(Ewl_Widget * o, void *ev_data, Ecore_Evas * ee)
{
	ecore_settings_close(ee);
	return;
}

void
ewl_settings_save(Ewl_Widget * o, void *ev_data, void *data)
{
	dml("Saving Settings", 2);
	save_settings();
	return;
}


/* XML */
void
save_settings(void)
{
	char           *locfn = malloc(PATH_MAX);
	XmlWriteHandle *p;

	snprintf(locfn, strlen(getenv("HOME")) + strlen(DEF_CONFIG_LOC),
		 DEF_CONFIG_LOC, getenv("HOME"));

	p = xml_write(locfn);

	xml_write_append_entry(p, "render_method",
			       ewl_entry_get_text((Ewl_Entry *) settings->
						  render_method.entry));
	xml_write_append_entry(p, "theme",
			       ewl_entry_get_text((Ewl_Entry *) settings->theme.
						  entry));
	xml_write_append_entry(p, "intro",
			       ewl_entry_get_text((Ewl_Entry *) settings->intro.
						  entry));
	xml_write_append_entry(p, "cc_x",
			       ewl_entry_get_text((Ewl_Entry *) settings->cc_x.
						  entry));
	xml_write_append_entry(p, "cc_y",
			       ewl_entry_get_text((Ewl_Entry *) settings->cc_y.
						  entry));
	xml_write_append_entry(p, "cc_w",
			       ewl_entry_get_text((Ewl_Entry *) settings->cc_w.
						  entry));
	xml_write_append_entry(p, "cc_h",
			       ewl_entry_get_text((Ewl_Entry *) settings->cc_h.
						  entry));
	xml_write_append_entry(p, "note_x",
			       ewl_entry_get_text((Ewl_Entry *) settings->
						  note_x.entry));
	xml_write_append_entry(p, "note_y",
			       ewl_entry_get_text((Ewl_Entry *) settings->
						  note_y.entry));
	xml_write_append_entry(p, "note_w",
			       ewl_entry_get_text((Ewl_Entry *) settings->
						  note_w.entry));
	xml_write_append_entry(p, "note_h",
			       ewl_entry_get_text((Ewl_Entry *) settings->
						  note_h.entry));
	xml_write_append_entry(p, "debug",
			       ewl_entry_get_text((Ewl_Entry *) settings->debug.
						  entry));

	xml_write_end(p);

	free(locfn);
	return;
}
