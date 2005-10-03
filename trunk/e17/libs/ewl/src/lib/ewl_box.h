#ifndef __EWL_BOX_H__
#define __EWL_BOX_H__

/**
 * @file ewl_box.h
 * @defgroup Ewl_Box Box: The Box Layout Container.
 * @brief Defines the Ewl_Box class used for laying out Ewl_Widget's in a
 * horizontal or vertical line.
 *
 * @{
 */

/**
 * @themekey /box/file
 * @themekey /box/group
 */

/**
 * The box widget is an Ewl_Container and lays out Ewl_Widget's either
 * horizontally or vertically.
 */
typedef struct Ewl_Box Ewl_Box;

/**
 * @def EWL_BOX(box)
 * Typecast a pointer to an Ewl_Box pointer.
 */
#define EWL_BOX(box) ((Ewl_Box *) box)

/**
 * @struct Ewl_Box
 * Inherits from an Ewl_Container to provide layout facilities for child
 * widgets placed inside. Layout is done horizontally (left-to-right), or
 * vertically (top-to-bottom). There is also a flag for homogeneous layout,
 * which gives each child Ewl_Widget equal space inside the Ewl_Box.
 */
struct Ewl_Box
{
	Ewl_Container   container; /**< Inherit from Ewl_Container */

	Ewl_Orientation orientation; /**< Indicate the orientation of layout */
	int             spacing; /**< Space between each widget in the box */

	unsigned int    homogeneous; /**< Flag indicating space assignemnt */
};

Ewl_Widget     *ewl_box_new();
Ewl_Widget     *ewl_hbox_new();
Ewl_Widget     *ewl_vbox_new();
int             ewl_box_init(Ewl_Box * box);
void            ewl_box_orientation_set(Ewl_Box * b, Ewl_Orientation o);
Ewl_Orientation ewl_box_orientation_get(Ewl_Box * b);
void            ewl_box_spacing_set(Ewl_Box * b, int spacing);
void            ewl_box_homogeneous_set(Ewl_Box *b, unsigned int h);

/*
 * Internally used callbacks, override at your own risk.
 */
void            ewl_box_child_add_cb(Ewl_Container * c, Ewl_Widget * w);
void            ewl_box_child_remove_cb(Ewl_Container * c, Ewl_Widget * w);
void            ewl_box_child_resize_cb(Ewl_Container * c, Ewl_Widget * w,
				        int size, Ewl_Orientation o);
void            ewl_box_child_show_cb(Ewl_Container * c, Ewl_Widget * w);
void            ewl_box_child_hide_cb(Ewl_Container * c, Ewl_Widget * w);
void            ewl_box_child_homogeneous_show_cb(Ewl_Container * c,
						  Ewl_Widget * w);

void            ewl_box_configure_cb(Ewl_Widget * w, void *ev_data,
						     void *user_data);
void            ewl_box_configure_homogeneous_cb(Ewl_Widget *w, void *ev_data,
						 void *user_data);

/**
 * @}
 */

#endif				/* __EWL_BOX_H__ */
