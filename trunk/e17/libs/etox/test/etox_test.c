#include "Etox_test.h"

/* globals */
Evas_Object clip_msg;
Evas_Object clip_test;
Evas_Object o_next_box;
Evas_Object o_txt_next_box;
Evas_Object o_prev_box;
Evas_Object o_txt_prev_box;
Evas_List pbuttons;

Evas evas;
Evas_Render_Method render_method = RENDER_ENGINE;
int max_colors = MAX_EVAS_COLORS;

Etox *e_msg;
Etox *e_test;

Evas_List current_test = NULL;

int win_w = W, win_h = H;
int win_x = 0, win_y = 0;
Window main_win;

double get_time(void)
{
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double) timev.tv_sec +
	    (((double) timev.tv_usec) / 1000000);
}

/* Events */
void e_idle(void *data)
{
	evas_render(evas);

	return;
	data = NULL;
}

void e_window_expose(Ecore_Event * ev)
{
	Ecore_Event_Window_Expose *e;

	e = (Ecore_Event_Window_Expose *) ev->event;
	if ((e->win != evas_get_window(evas)))
		return;
	evas_update_rect(evas, e->x, e->y, e->w, e->h);
}

void e_mouse_move(Ecore_Event * ev)
{
	Ecore_Event_Mouse_Move *e;

	e = (Ecore_Event_Mouse_Move *) ev->event;
	if ((e->win != evas_get_window(evas)))
		return;
	evas_event_move(evas, e->x, e->y);
}

void e_mouse_down(Ecore_Event * ev)
{
	Ecore_Event_Mouse_Down *e;

	e = (Ecore_Event_Mouse_Down *) ev->event;
	if ((e->win != evas_get_window(evas)))
		return;
	evas_event_button_down(evas, e->x, e->y, e->button);
}

void e_mouse_up(Ecore_Event * ev)
{
	Ecore_Event_Mouse_Up *e;

	e = (Ecore_Event_Mouse_Up *) ev->event;
	if ((e->win != evas_get_window(evas)))
		return;
	evas_event_button_up(evas, e->x, e->y, e->button);
}

void
button_mouse_in(void *_data, Evas _e, Evas_Object _o, int _b, int _x,
		int _y)
{
	if ((_e = evas))
		evas_set_color(_e, _o, 0, 0, 0, 255);

	return;
	_data = NULL;
	_e = NULL;
	_o = NULL;
	_b = 0;
	_x = 0;
	_y = 0;
}

void
button_mouse_out(void *_data, Evas _e, Evas_Object _o, int _b, int _x,
		 int _y)
{
	if ((_e = evas))
		evas_set_color(_e, _o, 0, 0, 0, 160);

	return;
	_data = NULL;
	_e = NULL;
	_o = NULL;
	_b = 0;
	_x = 0;
	_y = 0;
}

void
button_mouse_down(void *_data, Evas _e, Evas_Object _o, int _b, int _x,
		 int _y)
{
	Evas_List l = _data;
	void (*run)();

	etox_clear(e_msg);
	etox_clear(e_test);

	if (l) {
		evas_hide(evas, o_prev_box);
		evas_hide(evas, o_txt_prev_box);

		if (l->next) {
			evas_show(evas, o_next_box);
			evas_show(evas, o_txt_next_box);
		}
		else {
			evas_hide(evas, o_next_box);
			evas_hide(evas, o_txt_next_box);
		}

		current_test = l;
		run = l->data;
		run();
	}

	return;
	_data = NULL;
	_e = NULL;
	_o = NULL;
	_b = 0;
	_x = 0;
	_y = 0;
}

void
next_test(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	void (*run) ();

	if (!current_test || !current_test->next)
		return;

	/*
	 * Step to the next test, hide the "Next" button if appropriate, and
	 * run the current test.
	 */
	current_test = current_test->next;
	if (!current_test->next) {
		evas_hide(evas, o_next_box);
		evas_hide(evas, o_txt_next_box);
	}

	evas_show(evas, o_prev_box);
	evas_show(evas, o_txt_prev_box);

	run = current_test->data;
	run();

	return;
	_data = NULL;
	_e = NULL;
	_o = NULL;
	_b = 0;
	_x = 0;
	_y = 0;
}

void
prev_test(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	void (*run) ();

	if (!current_test || !current_test->prev)
		return;

	/*
	 * Step to the next test, hide the "Next" button if appropriate, and
	 * run the current test.
	 */
	current_test = current_test->prev;
	if (!current_test->prev) {
		evas_hide(evas, o_prev_box);
		evas_hide(evas, o_txt_prev_box);
	}

	evas_show(evas, o_next_box);
	evas_show(evas, o_txt_next_box);

	run = current_test->data;
	run();

	return;
	_data = NULL;
	_e = NULL;
	_o = NULL;
	_b = 0;
	_x = 0;
	_y = 0;
}

void setup(void)
{
	double width, height;
	Window win, ewin;
	Evas_Object o_bg;
	Evas_Object o_bg_etox;
	char msg[] =
	    "            The Etox Test utility consists in a series\n"
	    "            of test suites designed to exercise all of\n"
	    "            the etox functions.\n"
	    "            Informational messages will be displayed here,\n"
	    "            the test text will be presented in the colored\n"
	    "            rectangle below.\n"
	    "            To start a test suite, select it from the\n"
	    "            navigation panel on the left.\n";

	/* setup callbacks for events */
	ecore_event_filter_handler_add(ECORE_EVENT_WINDOW_EXPOSE,
				       e_window_expose);
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_MOVE,
				       e_mouse_move);
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_DOWN,
				       e_mouse_down);
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_UP, e_mouse_up);

	/* handler for when the event queue goes idle */
	ecore_event_filter_idle_handler_add(e_idle, NULL);

	/* create a toplevel window */
	win = ecore_window_new(0, 0, 0, win_w, win_h);
	ecore_window_set_title(win, "Etox Test");
	ecore_window_set_min_size(win, win_w, win_h);
	ecore_window_set_max_size(win, win_w, win_h);
	main_win = win;

	/* create a evas rendering in software - convenience function that */
	/* also creates the window for us in the right colormap & visual */
	evas =
	    evas_new_all(ecore_display_get(), win, 0, 0, win_w, win_h,
			 render_method, max_colors, MAX_FONT_CACHE,
			 MAX_IMAGE_CACHE, FONT_DIRECTORY);
	/* get the window ID for the evas created for us */
	ewin = evas_get_window(evas);

	/* show the evas window */
	ecore_window_show(ewin);

	/* set the events this window accepts */
	ecore_window_set_events(ewin,
				XEV_EXPOSE | XEV_BUTTON | XEV_MOUSE_MOVE);

	/* show the toplevel */
	ecore_window_show(win);

	/* Create interface */

	/* Background */
	o_bg = evas_add_image_from_file(evas, IM "bg.png");
	evas_resize(evas, o_bg, win_w, win_h);
	evas_move(evas, o_bg, 0, 0);
	evas_set_layer(evas, o_bg, 0);
	evas_show(evas, o_bg);

	/* Panel */
	setup_panel(evas);

	/* Setup message etox */
	/* Clip rectangle for bounding where the message text is drawn */
	clip_msg = evas_add_rectangle(evas);
	evas_show(evas, clip_msg);
	evas_set_color(evas, clip_msg, 255, 0, 255, 255);
	evas_move(evas, clip_msg, 40, 40);
	evas_resize(evas, clip_msg, 520, 140);

	/* Create message etox */
	e_msg = etox_new_all(evas, 40, 40, 520, 140, 255, ETOX_ALIGN_LEFT);
	etox_context_set_align(e_msg, ETOX_ALIGN_LEFT);
	etox_context_set_font(e_msg, "sinon", 14);
	etox_context_set_style(e_msg, "plain");
	etox_context_set_color(e_msg, 255, 255, 255, 255);
	etox_set_text(e_msg, msg);
	etox_set_clip(e_msg, clip_msg);
	etox_set_alpha(e_msg, 255);
	etox_set_layer(e_msg, 1000);
	etox_show(e_msg);

	/* Setup test etox */
	/* Setup test etox background */
	o_bg_etox = evas_add_rectangle(evas);
	evas_move(evas, o_bg_etox, 40, 200);
	evas_resize(evas, o_bg_etox, 520, 260);
	evas_set_color(evas, o_bg_etox, 0, 100, 100, 100);
	evas_set_layer(evas, o_bg_etox, 100);
	evas_show(evas, o_bg_etox);

	/* Clip rectangle for bounding where the test text is drawn */
	clip_test = evas_add_rectangle(evas);
	evas_set_color(evas, clip_test, 255, 0, 255, 255);
	evas_move(evas, clip_test, 40, 200);
	evas_resize(evas, clip_test, 520, 260);
	evas_show(evas, clip_test);

	/* Create test etox */
	e_test =
	    etox_new_all(evas, 40, 200, 520, 260, 255, ETOX_ALIGN_CENTER);
	etox_context_set_align(e_test, ETOX_ALIGN_CENTER);
	etox_context_set_font(e_test, "sinon", 14);
	etox_context_set_style(e_test, "plain");
	etox_context_set_color(e_test, 255, 255, 255, 255);
	etox_set_text(e_test, "");
	etox_set_clip(e_test, clip_test);
	etox_set_alpha(e_test, 255);
	etox_set_layer(e_test, 1000);

	/*
	 * Create the next test button
	 */
	o_next_box = evas_add_image_from_file(evas, IM "panel_button1.png");
	evas_set_image_border(evas, o_next_box, 3, 3, 3, 3);
	evas_set_layer(evas, o_next_box, 1000);
	evas_hide(evas, o_next_box);

	o_txt_next_box = evas_add_text(evas, "andover", 24, "Next");
	evas_set_color(evas, o_txt_next_box, 0, 0, 0, 160);
	evas_set_layer(evas, o_txt_next_box, 1000);
	evas_move(evas, o_next_box, 516, 150);
	evas_move(evas, o_txt_next_box, 519, 152);

	width = evas_get_text_width(evas, o_txt_next_box);
	height = evas_get_text_height(evas, o_txt_next_box);

	width += 10;
	height += 5;

	evas_resize(evas, o_next_box, width, height);
	evas_set_image_fill(evas, o_next_box, 0, 0, width, height);
	evas_hide(evas, o_txt_next_box);
	evas_callback_add(evas, o_txt_next_box, CALLBACK_MOUSE_DOWN,
			  next_test, NULL);

	/*
	 * Create the previous test button
	 */
	o_prev_box = evas_add_image_from_file(evas, IM "panel_button1.png");
	evas_set_image_border(evas, o_prev_box, 3, 3, 3, 3);
	evas_set_layer(evas, o_prev_box, 1000);
	evas_hide(evas, o_prev_box);

	o_txt_prev_box = evas_add_text(evas, "andover", 24, "Previous");
	evas_set_color(evas, o_txt_prev_box, 0, 0, 0, 160);
	evas_set_layer(evas, o_txt_prev_box, 1000);
	evas_move(evas, o_prev_box, 16, 150);
	evas_move(evas, o_txt_prev_box, 19, 152);

	width = evas_get_text_width(evas, o_txt_prev_box);
	height = evas_get_text_height(evas, o_txt_prev_box);

	width += 10;
	height += 5;

	evas_resize(evas, o_prev_box, width, height);
	evas_set_image_fill(evas, o_prev_box, 0, 0, width, height);
	evas_hide(evas, o_txt_prev_box);
	evas_callback_add(evas, o_txt_prev_box, CALLBACK_MOUSE_DOWN,
			  prev_test, NULL);

	e_slide_panel_in(0, NULL);
}

int main(int argc, char **argv)
{
	Evas_List l;

	/* init X */
	if (!ecore_display_init(NULL)) {
		if (getenv("DISPLAY")) {
			printf("Cannot initialize default display:\n");
			printf("DISPLAY=%s\n", getenv("DISPLAY"));
		} else {
			printf("No DISPLAY variable set!\n");
		}
		printf("Exit.\n");
		exit(-1);
	}

	/* setup handlers for system signals */
	ecore_event_signal_init();

	/* setup the event filter */
	ecore_event_filter_init();

	/* setup the X event internals */
	ecore_event_x_init();

	/* program does its data setup here */
	setup();

	/* and now loop forever handling events */
	ecore_event_loop();

	etox_free(e_msg);
	etox_free(e_test);

	for (l = pbuttons; l; l = l->next)
		panel_button_free(evas, pbuttons->data);

	evas_list_free(pbuttons);
	evas_free(evas);

	return 0;
	argc = 0;
	argv = NULL;
}
