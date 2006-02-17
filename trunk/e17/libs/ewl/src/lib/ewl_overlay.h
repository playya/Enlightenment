#ifndef EWL_OVERLAY_H
#define EWL_OVERLAY_H

/**
 * @defgroup Ewl_Overlay Ewl_Overlay: A Container for Displaying on an Evas
 * Defines the Ewl_Overlay class to provide EWL with the ability to work with an
 * evas.
 *
 * @{
 */

/**
 * @themekey /overlay/file
 * @themekey /overlay/group
 */

#define EWL_OVERLAY_TYPE "overlay"

/**
 * The overlay structure is mostly a container for holding widgets and a
 * wrapper evas smart object.
 */
typedef struct Ewl_Overlay Ewl_Overlay;

/**
 * @def EWL_OVERLAY(widget)
 * @brief Typecast a pointer to an Ewl_Overlay pointer.
 */
#define EWL_OVERLAY(widget) ((Ewl_Overlay *) widget)

/**
 * @brief The class inheriting from Ewl_Container that acts as a top level
 * widget for interacting with the evas.
 */
struct Ewl_Overlay
{
	Ewl_Container   container; /**< Inherits from the Ewl_Container class */
};

Ewl_Widget     *ewl_overlay_new(void);
int             ewl_overlay_init(Ewl_Overlay *win);

/*
 * Internally used callbacks, override at your own risk.
 */
void ewl_overlay_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data);

void ewl_overlay_child_show_cb(Ewl_Container *emb, Ewl_Widget *child);
void ewl_overlay_child_resize_cb(Ewl_Container *c, Ewl_Widget *w, int size,
			       Ewl_Orientation o);

/**
 * @}
 */

#endif
