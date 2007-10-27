/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#ifndef EWL_TREE2_VIEW_FREEBOX_H
#define EWL_TREE2_VIEW_FREEBOX_H

#include "ewl_tree2_view.h"

/**
 * @addtogroup Ewl_Tree2_View_Freebox Ewl_Tree2_View_Freebox The freebox tree view
 * @brief A freebox view for the tree
 * @remarks Inherits from Ewl_Tree2_View
 * @{
 */

/**
 * @def EWL_TREE2_VIEW_FREEBOX_TYPE
 * The type name
 */
#define EWL_TREE2_VIEW_FREEBOX_TYPE "tree2_view_freebox"

/**
 * @def EWL_TREE2_VIEW_FREEBOX_IS(w)
 * Returns TRUE if the widget is an Ewl_Tree2_View_Freebox, FALSE otherwise
 */
#define EWL_TREE2_VIEW_FREEBOX_IS(w) (ewl_widget_type_is(EWL_WIDGET(w), EWL_TREE2_VIEW_FREEBOX_TYPE))

/**
 * Ewl_Tree2_View_Freebox
 */
typedef struct Ewl_Tree2_View_Freebox Ewl_Tree2_View_Freebox;

/**
 * @def EWL_TREE2_FREEBOX(tv)
 * Typecasts a pointer to an Ewl_Tree2_View_Freebox pointer
 */
#define EWL_TREE2_VIEW_FREEBOX(tv) ((Ewl_Tree2_View_Freebox*)tv)

/**
 * @brief Inherits from Ewl_Tree2_View and provides a freebox tree layout
 */
struct Ewl_Tree2_View_Freebox
{
	Ewl_Tree2_View view;		/**< Inherit from Ewl_Tree2_View */
	Ewl_Widget *fbox;		/**< The freebox region */
	Ewl_Widget *scroll;		/**< The scroll region */
};

Ewl_View 	*ewl_tree2_view_freebox_get(void);
Ewl_Widget	*ewl_tree2_view_freebox_new(void);
int		 ewl_tree2_view_freebox_init(Ewl_Tree2_View_Freebox *tv);

/**
 * @}
 */

#endif
