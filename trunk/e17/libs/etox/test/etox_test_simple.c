#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Evas.h>
#include <Ecore.h>
#include "src/Etox.h"
#include "etox-config.h"

#define MAX_EVAS_COLORS (216)
#define MAX_FONT_CACHE (512 * 1024)
#define MAX_IMAGE_CACHE (1 * (1024 * 1024))
#define RENDER_ENGINE RENDER_METHOD_ALPHA_SOFTWARE

/* the obstacle's coords */
#define OBST_X 100.0
#define OBST_Y 100.0

char string1[] = "This text should test most of the basic\n"
    "text display characteristics of etox.\n"
    "including multi-line text, \ttabs,\n"
    "and eventually text styles and inline formatting.\n"
    "\n"
    "The real test will be when there are huge\n"
    "strings of text that wrap around obstacles.\n"
    "At this point each line only contains 1 bit,\n"
    "but when wrapping obstacles, multiple bits\n"
    "will be needed to represent the lines.";

char string2[] = "This is the alternate text to help\n"
    "test the speed of changing text contents.\n"
    "Eventually, this text should be read in from a file\n"
    "so that long strings of text can be tested,\n"
    "but for now this should give a basic idea\n" "of updating speed.";
char *last = string1;

int obstacle_w = -1, obstacle_h = -1, obstacle_x, obstacle_y;

int layer = 1000;
int visible = 1;
int focused = 0;

static void e_idle(void *data);
static void ecore_window_expose(Ecore_Event * ev);
static void ecore_mouse_down(Ecore_Event * ev);
static void ecore_mouse_move(Ecore_Event * ev);
static void ecore_mouse_in(Ecore_Event * ev);
static void ecore_mouse_out(Ecore_Event * ev);

void setup(void);

Evas evas;
Evas_Render_Method render_method = RENDER_ENGINE;
Evas_Object cursor = NULL;
Etox *e;

static void e_idle(void *data)
{
	evas_render(evas);
	return;
	data = NULL;
}

static void ecore_window_expose(Ecore_Event * ev)
{
	Ecore_Event_Window_Expose *e;

	e = (Ecore_Event_Window_Expose *) ev->event;
	evas_update_rect(evas, e->x, e->y, e->w, e->h);
}

static void ecore_mouse_in(Ecore_Event * ev)
{
	focused = 1;
}

static void ecore_mouse_out(Ecore_Event * ev)
{
	focused = 0;
}

static void ecore_mouse_down(Ecore_Event * ev)
{
	int index, x, y, w, h;

	Ecore_Event_Mouse_Down *eemd =
	    (Ecore_Event_Mouse_Down *) ev->event;

	if (eemd->button == 1) {
		/*
		if (last == string1) {
			etox_context_set_style(e, "shadow");
			etox_set_text(e, string2);
			last = string2;
		} else {
			etox_context_set_style(e, "raised");
			etox_set_text(e, string1);
			last = string1;
		}
		*/
		index = etox_coord_to_geometry(e, eemd->x, eemd->y, &x, &y,
				&w, &h);
		printf("Clicked letter %c at %d, %d with size %d x %d\n",
				string1[index], x, y, w, h);
		evas_move(e->evas, cursor, x, y);
		evas_resize(e->evas, cursor, w, h);

	} else if (eemd->button == 2) {
		layer = -layer;
		etox_set_layer(e, layer);
	} else {
		if (visible) {
			etox_hide(e);
			visible = 0;
		} else {
			etox_show(e);
			visible = 1;
		}
	}
}

/*
 * Follow the mouse around the window
 */
static void ecore_mouse_move(Ecore_Event * ev)
{
	Ecore_Event_Mouse_Move *eemm =
	    (Ecore_Event_Mouse_Move *) ev->event;
	if (focused)
		etox_move(e, eemm->x, eemm->y);
}



void setup(void)
{
	Window win, ewin;

	ecore_event_filter_handler_add(ECORE_EVENT_WINDOW_EXPOSE,
				       ecore_window_expose);
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_DOWN,
				       ecore_mouse_down);
	/*
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_MOVE,
				       ecore_mouse_move);
       */
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_IN,
				       ecore_mouse_in);
	ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_OUT,
				       ecore_mouse_out);
	ecore_event_filter_idle_handler_add(e_idle, NULL);
	win = ecore_window_new(0, 0, 0, 400, 400);

	evas = evas_new_all(ecore_display_get(), win, 0, 0, 400, 400,
			    render_method, MAX_EVAS_COLORS, MAX_FONT_CACHE,
			    MAX_IMAGE_CACHE, PACKAGE_DATA_DIR "/fnt");

	ewin = evas_get_window(evas);

	ecore_window_show(ewin);
	ecore_window_set_events(ewin,
				XEV_EXPOSE | XEV_BUTTON | XEV_MOUSE_MOVE /* |
				XEV_IN_OUT */);
	ecore_window_show(win);
}

int main(int argc, char *argv[])
{
	int i;
	Evas_Object clip_rect;
	Evas_Object bg, et_bg, obst;

	obstacle_x = OBST_X;
	obstacle_y = OBST_Y;


	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "soft") ||
		    !strcmp(argv[i], "x11") || !strcmp(argv[i], "hard")) {
			if (!strcmp(argv[i], "soft"))
				render_method =
				    RENDER_METHOD_ALPHA_SOFTWARE;
			if (!strcmp(argv[i], "x11"))
				render_method =
				    RENDER_METHOD_BASIC_HARDWARE;
			if (!strcmp(argv[i], "hard"))
				render_method = RENDER_METHOD_3D_HARDWARE;
		} else {
			if (obstacle_w < 0.0)
				obstacle_w = atoi(argv[i]);
			else if (obstacle_h < 0.0)
				obstacle_h = atoi(argv[i]);
		}
	}

	if (obstacle_w < 0.0)
		obstacle_w = 100.0;
	if (obstacle_h < 0.0)
		obstacle_h = 100.0;

	ecore_display_init(NULL);
	ecore_event_signal_init();
	ecore_event_filter_init();
	ecore_event_x_init();

	setup();

	/* add a background */
	bg = evas_add_rectangle(evas);
	evas_resize(evas, bg, 400, 400);
	evas_move(evas, bg, 0, 0);
	evas_set_layer(evas, bg, -10000);
	evas_set_color(evas, bg, 255, 255, 255, 255);
	evas_show(evas, bg);

	/* add an etox-background */
	et_bg = evas_add_rectangle(evas);
	evas_resize(evas, et_bg, 380, 380);
	evas_move(evas, et_bg, 10, 10);
	evas_set_layer(evas, et_bg, -10000);
	evas_set_color(evas, et_bg, 0, 0, 255, 50);
	evas_show(evas, et_bg);

	/* draw obstacle-rect */
	obst = evas_add_rectangle(evas);
	evas_resize(evas, obst, obstacle_w, obstacle_h);
	evas_move(evas, obst, obstacle_x, obstacle_y);
	evas_set_color(evas, obst, 255, 0, 0, 50);
	evas_show(evas, obst);

	/*
	 * Create a clip rectangle for bounding where the text is drawn
	 */
	clip_rect = evas_add_rectangle(evas);
	evas_move(evas, clip_rect, 100, 100);
	evas_resize(evas, clip_rect, 200, 200);
	evas_show(evas, clip_rect);
	evas_set_color(evas, clip_rect, 255, 0, 255, 255);

	/*
	 * Create an etox.
	 */
	e = etox_new_all(evas, 30, 30, 300, 100, 255, ETOX_ALIGN_LEFT);
	etox_context_set_align(e, ETOX_ALIGN_CENTER);
	etox_context_set_style(e, "raised");
	etox_context_set_color(e, 128, 255, 255, 255);
	/* etox_set_clip(e, clip_rect); */
	etox_set_alpha(e, 128);
	etox_set_text(e, string1);
	etox_obstacle_add(e, obstacle_x, obstacle_y, obstacle_w, obstacle_h);

	etox_show(e);

	/* add a cursor */
	cursor = evas_add_rectangle(evas);
	evas_resize(evas, cursor, 10, 20);
	evas_move(evas, cursor, 30, 30);
	evas_set_layer(evas, cursor, 10000);
	evas_set_color(evas, cursor, 255, 255, 255, 128);
	evas_show(evas, cursor);

	ecore_event_loop();

	etox_free(e);
	evas_free(evas);

	return 0;
}
