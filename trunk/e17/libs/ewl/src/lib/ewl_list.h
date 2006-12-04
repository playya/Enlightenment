#ifndef EWL_LIST_H
#define EWL_LIST_H

/**
 * @addtogroup Ewl_List Ewl_List: A list widget
 * @brief Defines a class to layout data in a list
 *
 * @remarks Inherits from Ewl_MVC.
 * @image html Ewl_List_inheritance.png
 *
 * @{
 */

/**
 * @def EWL_LIST_TYPE
 * The type name for the Ewl_List widget
 */
#define EWL_LIST_TYPE "list"

/**
 * @def EWL_LIST(list)
 * Typecasts a pointer to an Ewl_List pointer
 */
#define EWL_LIST(list) ((Ewl_List *)list)

/**
 * The list strucutre 
 */
typedef struct Ewl_List Ewl_List;

/**
 * Inherits from Ewl_MVC and extends to provide a list widget
 */
struct Ewl_List
{
	Ewl_MVC mvc;		/**< The mvc parent */
};

Ewl_Widget 	*ewl_list_new(void);
int 		 ewl_list_init(Ewl_List *list);

/*
 * Internal stuff.
 */
void ewl_list_cb_configure(Ewl_Widget *w, void *ev, void *data);
void ewl_list_cb_item_clicked(Ewl_Widget *w, void *ev, void *data);
void ewl_list_cb_child_add(Ewl_Container *c, Ewl_Widget *w);
void ewl_list_cb_selected_change(Ewl_MVC *mvc);

/**
 * @}
 */

#endif

