#ifndef _EWL_TREE_H
#define _EWL_TREE_H

/**
 * @file ewl_tree.h
 * @defgroup Ewl_Tree Tree: A Widget for List or Tree Layout
 * @brief Defines a widget for laying out other widgets in a tree or list like
 * manner.
 *
 * @{
 */

/**
 * @themekey /tree/file
 * @themekey /tree/group
 */

enum Ewl_Tree_Mode
{
	EWL_TREE_MODE_NONE,
	EWL_TREE_MODE_SINGLE,
	EWL_TREE_MODE_MULTI
};

typedef enum Ewl_Tree_Mode Ewl_Tree_Mode;

typedef struct Ewl_Tree Ewl_Tree;

/**
 * @def EWL_TREE(t)
 * Typecasts a pointer to an Ewl_Tree pointer.
 */
#define EWL_TREE(t) ((Ewl_Tree *)t)

/**
 * @struct Ewl_Tree
 * The tree is a columnar listing, where items in the list may be nested below
 * other items.
 */
struct Ewl_Tree
{
	Ewl_Container  container;  /**< Inherit from Ewl_Container */

	unsigned short ncols;      /**< Number of columns in tree */
	Ewl_Widget    *header;     /**< Array of widgets in the header */
	Ewl_Widget    *scrollarea; /**< Scrollable area of rows */
	Ecore_List    *selected;   /**< The currently selected rows */
	Ewl_Tree_Mode  mode;       /**< Mode for selecting rows */
	unsigned int   headers_visible; /**< Are the headers visible */
};

typedef struct _ewl_tree_node Ewl_Tree_Node;

/**
 * @def EWL_TREE_NODE(t)
 * Typecasts a pointer to an Ewl_Tree_Node pointer.
 */
#define EWL_TREE_NODE(t) ((Ewl_Tree_Node *)t)

/*
 * The tree_node exists for each row, at this level the tree_node contains rows
 * that contain cells which display the data.
 */
struct _ewl_tree_node
{
	Ewl_Container container; /**< Inherit from Ewl_Container */
	Ewl_Tree *tree; /**< Pointer to the tree this is inside */
	Ewl_Widget *row; /**< The child row */
	Ewl_Widget *handle; /**< The child row */
	Ewl_Tree_Node_Flags expanded; /**< Indicator of expansion state */
};

/*
 * Node creation/initialization functions.
 */
Ewl_Widget 	*ewl_tree_node_new(void);
int 		 ewl_tree_node_init(Ewl_Tree_Node *tree_node);

/*
 * Node state modifying functions
 */
void 		 ewl_tree_node_collapse(Ewl_Tree_Node *tree);
void 		 ewl_tree_node_expand(Ewl_Tree_Node *tree);

/*
 * Tree creation/initialization functions.
 */
Ewl_Widget 	*ewl_tree_new(unsigned short columns);
int 		 ewl_tree_init(Ewl_Tree *tree, unsigned short columns);

void 		 ewl_tree_headers_set(Ewl_Tree *tree, char **headers);
void  		 ewl_tree_columns_set(Ewl_Tree *tree, unsigned short columns);

void		ewl_tree_headers_visible_set(Ewl_Tree *tree, unsigned int visible);
unsigned int	ewl_tree_headers_visible_get(Ewl_Tree *tree);


Ecore_List 	*ewl_tree_selected_get(Ewl_Tree *tree);
void 		 ewl_tree_selected_clear(Ewl_Tree *tree);
Ewl_Widget	*ewl_tree_row_column_get(Ewl_Row *row, int i);

Ewl_Tree_Mode  	 ewl_tree_mode_get(Ewl_Tree *tree);
void 		 ewl_tree_mode_set(Ewl_Tree *tree, Ewl_Tree_Mode mode);

Ewl_Widget 	*ewl_tree_row_add(Ewl_Tree *tree, Ewl_Row *prow,
					     Ewl_Widget **children);
Ewl_Widget 	*ewl_tree_text_row_add(Ewl_Tree *tree, Ewl_Row *prow, char **text);
Ewl_Widget 	*ewl_tree_entry_row_add(Ewl_Tree *tree, Ewl_Row *prow, char **text);

void 		 ewl_tree_row_destroy(Ewl_Tree *tree, Ewl_Row *row);
void 		 ewl_tree_row_expand_set(Ewl_Row *row, Ewl_Tree_Node_Flags expanded);
Ewl_Row 	*ewl_tree_row_find(Ewl_Tree *tree, int row);

/*
 * Internally used callbacks, override at your own risk.
 */
void ewl_tree_node_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_node_destroy_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_node_toggle_cb(Ewl_Widget *w, void *ev_data, void *user_data);

void ewl_tree_node_child_show_cb(Ewl_Container *c, Ewl_Widget *w);
void ewl_tree_node_child_hide_cb(Ewl_Container *c, Ewl_Widget *w);
void ewl_tree_node_resize_cb(Ewl_Container *c, Ewl_Widget *w, int size,
						     Ewl_Orientation o);

void ewl_tree_row_select_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_row_hide_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_configure_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_destroy_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_hscroll_cb(Ewl_Widget *w, void *ev_data, void *user_data);
void ewl_tree_child_resize_cb(Ewl_Container *c);
void ewl_tree_node_child_add_cb(Ewl_Container *c, Ewl_Widget *w);

/**
 * @}
 */

#endif
