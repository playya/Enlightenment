
#ifndef __EWL_WINDOW_H__
#define __EWL_WINDOW_H__

/*
 * The window structure is mostly a container for holding widgets and a
 * wrapper around the xlib window.
 */
typedef struct _ewl_window Ewl_Window;

struct _ewl_window {
	Ewl_Container   widget;

	Window          window;

	/*
	 * The following fields allow for drawing the widgets
	 */
	Evas            evas;
	Window          evas_window;

	/*
	 * Main background of the window. A rectangle and a bit.
	 */
	Evas_Object     bg_rect;
	Ebits_Object   *ebits_object;

	char           *title;

	/*
	 * Flag to indicate if the window has a border.
	 */
	short           borderless;
	short           auto_resize;
};

#define EWL_WINDOW(widget) ((Ewl_Window *) widget)

Ewl_Widget     *ewl_window_new();
void            ewl_window_init(Ewl_Window * w);
Ewl_Window     *ewl_window_find_window(Window window);
Ewl_Window     *ewl_window_find_window_by_evas_window(Window window);
Ewl_Window     *ewl_window_find_window_by_widget(Ewl_Widget * w);
void            ewl_window_resize(Ewl_Window * widget, int w, int h);
void            ewl_window_set_min_size(Ewl_Window * widget, int w, int h);
void            ewl_window_set_max_size(Ewl_Window * widget, int w, int h);
void            ewl_window_set_title(Ewl_Window * widget, char *title);
char           *ewl_window_get_title(Ewl_Window * widget);
void            ewl_window_get_geometry(Ewl_Window * win, int *x, int *y,
					int *w, int *h);
void            ewl_window_set_geometry(Ewl_Window * widget, int x, int y,
					int w, int h);
void            ewl_window_set_borderless(Ewl_Window * w);
void            ewl_window_set_auto_size(Ewl_Window * win, int value);
void            ewl_window_move(Ewl_Window * w, int x, int y);
Ewl_Widget     *ewl_window_get_child_at(Ewl_Window * win, int x, int y);

#endif				/* __EWL_WINDOW_H__ */
