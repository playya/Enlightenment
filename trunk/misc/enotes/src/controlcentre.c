
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


#include "controlcentre.h"

ControlCentre  *controlcentre;


/**
 * @brief: Sets up the control centre window and its contents and callbacks.
 */
void
setup_cc(void)
{
	ControlCentre  *cc;
	char           *edjefn = malloc(PATH_MAX);
	char           *fontpath = malloc(PATH_MAX);
	Evas_Coord      edje_w, edje_h;
	CCPos          *pos;

	cc = malloc(sizeof(ControlCentre));
	controlcentre = cc;

	pos = get_cc_pos();

	/* Setup the Window */
	cc->win =
		ecore_evas_software_x11_new(NULL, 0, pos->x, pos->y, pos->width,
					    pos->height);
	ecore_evas_title_set(cc->win, "Enotes");
	ecore_evas_name_class_set(cc->win, "Enotes", "Enotes");
	ecore_evas_borderless_set(cc->win, 1);
	ecore_evas_shaped_set(cc->win, 1);
	if (pos->x != 0 && pos->y != 0)
		ecore_evas_resize(cc->win, pos->x, pos->y);
	ecore_evas_show(cc->win);

	/* Moving the damn thing */
	ecore_x_window_prop_xy_set(ecore_evas_software_x11_window_get(cc->win),
				   pos->x, pos->y);

	/* Setup the Canvas, Render-Method and Font Path */
	cc->evas = ecore_evas_get(cc->win);
	evas_output_method_set(cc->evas,
			       evas_render_method_lookup(main_config->
							 render_method));
	snprintf(fontpath, PATH_MAX, "%s/data/fonts", PACKAGE_DATA_DIR);
	evas_font_path_append(cc->evas, fontpath);
	free(fontpath);

	/* Draggable Setup */
	cc->dragger = esmart_draggies_new(cc->win);
	evas_object_name_set(cc->dragger, "dragger");
	evas_object_move(cc->dragger, 0, 0);
	evas_object_layer_set(cc->dragger, 0);
	evas_object_color_set(cc->dragger, 255, 255, 255, 0);
	esmart_draggies_button_set(cc->dragger, 1);
	evas_object_show(cc->dragger);

	/* Setup the EDJE */
	cc->edje = edje_object_add(cc->evas);
	snprintf(edjefn, PATH_MAX, CC_EDJE, PACKAGE_DATA_DIR,
		 main_config->theme);
	edje_object_file_set(cc->edje, edjefn, CC_PART);
	free(edjefn);
	evas_object_move(cc->edje, 0, 0);
	evas_object_layer_set(cc->edje, 1);
	evas_object_name_set(cc->edje, "edje");
	evas_object_pass_events_set(cc->edje, 0);
	evas_object_show(cc->edje);

	/* EDJE and ECORE min, max and resizing */
	edje_object_size_max_get(cc->edje, &edje_w, &edje_h);
	ecore_evas_size_max_set(cc->win, edje_w, edje_h);
	edje_object_size_min_get(cc->edje, &edje_w, &edje_h);
	ecore_evas_size_min_set(cc->win, edje_w, edje_h);
	ecore_evas_resize(cc->win, (int) edje_w, (int) edje_h);
	evas_object_resize(cc->edje, edje_w, edje_h);
	evas_object_resize(cc->dragger, edje_w, edje_h);

	/* Ecore Callbacks */
	ecore_evas_callback_resize_set(cc->win, cc_resize);
	ecore_evas_callback_destroy_set(cc->win, cc_close);
	ecore_evas_callback_delete_request_set(cc->win, cc_close);

	/* Edje Callbacks */
	edje_object_signal_callback_add(cc->edje,
					EDJE_SIGNAL_CC_MINIMIZE, "",
					(void *) cc_minimize, cc->win);
	edje_object_signal_callback_add(cc->edje, EDJE_SIGNAL_CC_CLOSE, "",
					(void *) cc_close, NULL);
	edje_object_signal_callback_add(cc->edje, EDJE_SIGNAL_CC_SAVELOAD, "",
					(void *) cc_saveload, NULL);
	edje_object_signal_callback_add(cc->edje, EDJE_SIGNAL_CC_SETTINGS, "",
					(void *) cc_settings, NULL);
	edje_object_signal_callback_add(cc->edje, EDJE_SIGNAL_CC_NEW, "",
					(void *) cc_newnote, NULL);

	free(pos);
	return;
}

CCPos          *
get_cc_pos()
{
	CCPos          *p = malloc(sizeof(CCPos));
	char           *locfn = malloc(PATH_MAX);
	XmlReadHandle  *h;
	XmlEntry       *tmp;
	FILE           *fp;

	p->x = -50;
	p->y = -50;
	p->width = -50;
	p->height = -50;

	snprintf(locfn, PATH_MAX, DEF_CC_CONFIG_LOC, getenv("HOME"));

	fp = fopen(locfn, "r");
	if (fp == NULL) {
		free(locfn);
		p->x = 0;
		p->y = 0;
		p->width = 250;
		p->height = 250;
		return (p);
	} else {
		fclose(fp);
	}

	h = xml_read(locfn);
	while (h->cur != NULL) {
		tmp = xml_read_entry_get_entry(h);
		if (!strcmp(tmp->name, "x")) {
			if (tmp->value != NULL)
				p->x = atoi(tmp->value);
			else
				p->x = 0;
		} else if (!strcmp(tmp->name, "y")) {
			if (tmp->value != NULL)
				p->y = atoi(tmp->value);
			else
				p->y = 0;
		} else if (!strcmp(tmp->name, "width")) {
			if (tmp->value != NULL)
				p->width = atoi(tmp->value);
			else
				p->width = 250;
		} else if (!strcmp(tmp->name, "height")) {
			if (tmp->value != NULL)
				p->height = atoi(tmp->value);
			else
				p->height = 250;
		}
		free_xmlentry(tmp);
		if (p->x != -50 && p->y != -50 && p->width != -50 &&
		    p->height != -50)
			break;
		xml_read_next_entry(h);
	}
	xml_read_end(h);

	free(locfn);
	return (p);
}

void
set_cc_pos_by_ccpos(CCPos * p)
{
	set_cc_pos(p->x, p->y, p->width, p->height);
	return;
}

void
set_cc_pos()
{
	char           *locfn = malloc(PATH_MAX);
	XmlWriteHandle *p;
	int             x, y, width, height;

	ecore_evas_geometry_get(controlcentre->win, &x, &y, &width, &height);

	snprintf(locfn, PATH_MAX, DEF_CC_CONFIG_LOC, getenv("HOME"));
	p = xml_write(locfn);

	xml_write_append_entry_int(p, "x", x);
	xml_write_append_entry_int(p, "y", y);
	xml_write_append_entry_int(p, "width", width);
	xml_write_append_entry_int(p, "height", height);

	xml_write_end(p);
	free(locfn);
	return;
}

/**
 * @param ee: The Ecore_Evas the event occurred on.
 * @brief: Ecore callback for the resising of the window.  This function
 *         resises the edje and esmart dragger according to the new
 *         window size.
 */
void
cc_resize(Ecore_Evas * ee)
{
	int             x, y, w, h;

	ecore_evas_geometry_get(ee, &x, &y, &w, &h);
	evas_object_resize(evas_object_name_find
			   (ecore_evas_get(ee), "edje"), w, h);
	evas_object_resize(evas_object_name_find(ecore_evas_get(ee), "dragger"),
			   w, h);
	return;
}

/**
 * @param ee: The Ecore_Evas that has been closed by the wm.
 * @brief: Ecore + Edje signal callback for the closing of the window
 *         (window-manager side).  This function simply ends the loop which
 *         concequently allows for the freeing of variables, etc... before
 *         enotes exits.
 */
void
cc_close(Ecore_Evas * ee)
{
	ecore_main_loop_quit();
	return;
}

/**
 * @param data: This variable isn't used.  It is data that could be supplied when
 *              the callback is made.
 * @brief: Edje signal callback for the clicking or selecting of the saveload option.
 *         This calls up the saveload window.
 */
void
cc_saveload(void *data)
{
	setup_saveload();
	return;
}

/**
 * @param data: This variable isn't used.  It is data that could be supplied when
 *              the callback is made.
 * @brief: Edje signal callback for the clicking or selecting of the new note option.
 *         This calls up a new note.
 */
void
cc_newnote(void *data)
{
	new_note(NOTE_CONTENT);
	return;
}

/**
 * @param data: This variable isn't used.  It is data that could be supplied when
 *              the callback is made.
 * @brief: Edje signal callback for the clicking or selecting of the settings option.
 *         This calls up the settings window.
 */
void
cc_settings(void *data)
{
	setup_settings();
	return;
}

/**
 * @param data: This variable isn't used.  It is data that could be supplied when
 *              the callback is made.
 * @brief: Edje signal callback for the clicking or selecting of the minimize button.
 */
void
cc_minimize(void *data)
{
	ecore_evas_iconified_set((Ecore_Evas *) data, 1);
	return;
}
