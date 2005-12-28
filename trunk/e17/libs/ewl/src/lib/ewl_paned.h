#ifndef __EWL_PANED_H__
#define __EWL_PANED_H__

/**
 * @file ewl_paned.h
 * @defgroup Ewl_Paned Paned: A paned widget
 * @brief Provides a widget to have a resizable paned container
 *
 * @{
 */

/**
 * @themekey /paned/file
 * @themekey /paned/group
 * @themekey /grabber/vertical/file
 * @themekey /grabber/vertical/group
 * @themekey /grabber/horizontal/file
 * @themekey /grabber/horizontal/group
 */

/**
 */
typedef struct Ewl_Paned Ewl_Paned;

/**
 * @def EWL_PANED(pane)
 * Typecasts a pointer to a Ewl_Paned pointer
 */
#define EWL_PANED(paned) ((Ewl_Paned *) paned)

/**
 * @struct Ewl_Paned
 * Inherits from Ewl_Container and extends to provided the paned widget
 */
struct Ewl_Paned
{
	Ewl_Container container;
	Ewl_Orientation	orientation;
};

Ewl_Widget	*ewl_paned_new(void);
Ewl_Widget	*ewl_hpaned_new(void);
Ewl_Widget	*ewl_vpaned_new(void);
int		 ewl_paned_init(Ewl_Paned *p);

void 		 ewl_paned_orientation_set(Ewl_Paned *p, Ewl_Orientation o);
Ewl_Orientation  ewl_paned_orientation_get(Ewl_Paned *p);

/* 
 * Internal functions. Override at your risk.
 */
void ewl_paned_cb_child_add(Ewl_Container *c, Ewl_Widget *w);
void ewl_paned_cb_child_remove(Ewl_Container *c, Ewl_Widget *w);
void ewl_paned_cb_child_resize(Ewl_Container *c, Ewl_Widget *w, int size,
							Ewl_Orientation o);
void ewl_paned_cb_child_show(Ewl_Container *c, Ewl_Widget *w);
void ewl_paned_cb_child_hide(Ewl_Container *c, Ewl_Widget *w);

void ewl_paned_cb_configure(Ewl_Widget *w, void *ev, void *data);

/**
 * @}
 */

#endif


