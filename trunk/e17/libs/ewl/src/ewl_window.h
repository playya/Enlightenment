
#ifndef __EWL_WINDOW_H__
#define __EWL_WINDOW_H__

/**
 * @defgroup Ewl_Window Window: A Container for Displaying in a New Window
 * @brief Defines the Ewl_Window class which extends the Ewl_Embed class by
 * creating it's own window and evas.
 *
 * @{
 */

/**
 * @themekey /window/file
 * @themekey /window/group
 */

/**
 * The window structure is mostly a container for holding widgets and a
 * wrapper around the xlib window.
 */
typedef struct Ewl_Window Ewl_Window;

/**
 * @def EWL_WINDOW(win)
 * Typecasts a pointer to an Ewl_Window pointer.
 */
#define EWL_WINDOW(win) ((Ewl_Window *) win)

/**
 * @struct Ewl_Window
 * Extends the Ewl_Embed class to create it's own window and evas for drawing,
 * sizing and positioning.
 */
struct Ewl_Window
{
	Ewl_Embed       embed; /**< Inherits from the Ewl_Embed class */

	void           *window; /**< Provides a window for drawing */

	char           *title; /**< The current title on the provided window */
	char           *name; /**< Current name on the provided window */
	char           *classname; /**< Current class on the provided window */

	 
	Ewl_Window_Flags flags; /**< Flags indicating window properties */

	int             x; /**< Screen relative horizontal position of window */
	int             y; /**< Screen relative vertical position of window */
	char           *render; /**< The render engine in use */
};

Ewl_Widget     *ewl_window_new(void);
int             ewl_window_init(Ewl_Window * win);
Ewl_Window     *ewl_window_window_find(void *window);
void            ewl_window_title_set(Ewl_Window * win, char *title);
char           *ewl_window_title_get(Ewl_Window * win);
void            ewl_window_name_set(Ewl_Window * win, char *name);
char           *ewl_window_name_get(Ewl_Window * win);
void            ewl_window_class_set(Ewl_Window * win, char *classname);
char           *ewl_window_class_get(Ewl_Window * win);
void            ewl_window_borderless_set(Ewl_Window * win);
void            ewl_window_move(Ewl_Window * win, int x, int y);
void            ewl_window_position_get(Ewl_Window * win, int *x, int *y);

/*
 * Internally used callbacks, override at your own risk.
 */
void            ewl_window_realize_cb(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            ewl_window_postrealize_cb(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            ewl_window_unrealize_cb(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            ewl_window_show_cb(Ewl_Widget * w, void *ev_data,
				  void *user_data);
void            ewl_window_hide_cb(Ewl_Widget * w, void *ev_data,
				  void *user_data);
void            ewl_window_destroy_cb(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            ewl_window_configure_cb(Ewl_Widget * w, void *ev_data,
				       void *user_data);

/**
 * @}
 */

#endif				/* __EWL_WINDOW_H__ */
