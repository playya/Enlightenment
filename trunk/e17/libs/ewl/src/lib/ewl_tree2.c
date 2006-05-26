#include "ewl_private.h"
#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"

static void ewl_tree2_cb_column_free(void *data);

/**
 * @return Returns NULL on failure, a new tree widget on success.
 * @brief Allocate and initialize a new tree widget
 */
Ewl_Widget *
ewl_tree2_new(void)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = NEW(Ewl_Tree2, 1);
	if (!w)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_tree2_init(EWL_TREE2(w))) 
	{
		ewl_widget_destroy(w);
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	DRETURN_PTR(w, DLEVEL_STABLE);
}

/**
 * @param tree: the tree widget to be initialized
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize the contents of a tree widget
 *
 * The contents of the tree widget @a tree are initialized to their defaults.
 */
int
ewl_tree2_init(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, FALSE);

	if (!ewl_container_init(EWL_CONTAINER(tree)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_appearance_set(EWL_WIDGET(tree), EWL_TREE2_TYPE);
	ewl_widget_inherit(EWL_WIDGET(tree), EWL_TREE2_TYPE);

	ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);

	tree->columns = ecore_list_new();
	ecore_list_set_free_cb(tree->columns, ewl_tree2_cb_column_free);

	tree->mode = EWL_TREE_MODE_NONE;

	tree->header = ewl_hpaned_new();
	ewl_container_child_append(EWL_CONTAINER(tree), tree->header);
	ewl_widget_appearance_set(EWL_WIDGET(tree->header), "tree_header");
	ewl_object_fill_policy_set(EWL_OBJECT(tree->header), 
				EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_VSHRINK);
	ewl_widget_show(tree->header);

	tree->rows = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(tree), tree->rows);
	ewl_widget_show(tree->rows);

	ewl_tree2_headers_visible_set(tree, TRUE);
	ewl_tree2_fixed_rows_set(tree, FALSE);

	ewl_callback_append(EWL_WIDGET(tree), EWL_CALLBACK_CONFIGURE,
					ewl_tree2_cb_configure, NULL);
	ewl_callback_prepend(EWL_WIDGET(tree), EWL_CALLBACK_DESTROY,
					ewl_tree2_cb_destroy, NULL);

	ewl_container_resize_notify_set(EWL_CONTAINER(tree),
					ewl_tree2_cb_child_resize);

	ewl_widget_focusable_set(EWL_WIDGET(tree), FALSE);
	ewl_tree2_dirty_set(tree, TRUE);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to set the data into
 * @param data: The data to set into the tree
 * @return Returns no value.
 * @brief Set the data into the tree
 */
void
ewl_tree2_data_set(Ewl_Tree2 *tree, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	tree->data = data;
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to get the data from
 * @return Returns the data currently set into the tree or NULL on failure
 * @brief Get the data out of the tree
 */
void *
ewl_tree2_data_get(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, NULL);
	DCHECK_TYPE_RET("tree", tree, EWL_TREE2_TYPE, NULL);

	DRETURN_PTR(tree->data, DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to append the column too
 * @param model: The model to use for this column
 * @param view: The view to use for this column
 * @return Returns no value.
 * @brief Append a new column to the tree
 */
void
ewl_tree2_column_append(Ewl_Tree2 *tree, Ewl_Model *model, Ewl_View *view)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_PARAM_PTR("model", model);
	DCHECK_PARAM_PTR("view", view);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	c = ewl_tree2_column_new();
	if (!c)
	{
		DWARNING("Unable to create new tree column.\n");
		DRETURN(DLEVEL_STABLE);
	}

	ewl_tree2_column_model_set(c, model);
	ewl_tree2_column_view_set(c, view);

	ecore_list_append(tree->columns, c);
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to prepend the column too
 * @param model: The model to use for this column
 * @param view: The view to use for this column
 * @return Returns no value.
 * @brief Prepend a new column to the tree
 */
void
ewl_tree2_column_prepend(Ewl_Tree2 *tree, Ewl_Model *model, Ewl_View *view)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_PARAM_PTR("model", model);
	DCHECK_PARAM_PTR("view", view);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	c = ewl_tree2_column_new();
	if (!c)
	{
		DWARNING("Unable to create new tree column.\n");
		DRETURN(DLEVEL_STABLE);
	}

	ewl_tree2_column_model_set(c, model);
	ewl_tree2_column_view_set(c, view);

	ecore_list_prepend(tree->columns, c);
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to insert the column into
 * @param model: The model to use for this column
 * @param view: The view to use for this column
 * @param idx: The index to insert into 
 * @return Returns no value.
 * @brief Insert a new column into the tree
 */
void
ewl_tree2_column_insert(Ewl_Tree2 *tree, Ewl_Model *model, Ewl_View *view, 
							unsigned int idx)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_PARAM_PTR("model", model);
	DCHECK_PARAM_PTR("view", view);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	c = ewl_tree2_column_new();
	if (!c)
	{
		DWARNING("Unable to create new tree column.\n");
		DRETURN(DLEVEL_STABLE);
	}

	ewl_tree2_column_model_set(c, model);
	ewl_tree2_column_view_set(c, view);

	ecore_list_goto_index(tree->columns, idx);
	ecore_list_insert(tree->columns, c);
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to remove the column from
 * @param idx: The column index to remove
 * @return Returns no value
 * @brief Remove a column from the tree
 */
void
ewl_tree2_column_remove(Ewl_Tree2 *tree, unsigned int idx)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	ecore_list_goto_index(tree->columns, idx);
	c = ecore_list_remove(tree->columns);

	ewl_tree2_column_destroy(c);
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to toggle the header visibility
 * @param visible: The visiblity to set the tree to (TRUE == on, FALSE == off)
 * @return Returns no value
 * @brief Toggle if the header is visible in the tree
 */
void
ewl_tree2_headers_visible_set(Ewl_Tree2 *tree, unsigned char visible)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	if (tree->headers_visible == visible)
		DRETURN(DLEVEL_STABLE);

	tree->headers_visible = !!visible;

	if (!tree->headers_visible)
		ewl_widget_hide(tree->header);
	else
		ewl_widget_show(tree->header);

	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to get the header visiblity from
 * @return Returns the current header visiblity of the tree
 * @brief Retrieve if the header is visible in the tree
 */
unsigned int
ewl_tree2_headers_visible_get(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, FALSE);
	DCHECK_TYPE_RET("tree", tree, EWL_TREE2_TYPE, FALSE);

	DRETURN_INT(tree->headers_visible, DLEVEL_STABLE);
}

/**
 * @param tree: The tree to get the selected cells from
 * @return Returns an Ecore_List of cells selected in the tree
 * @brief Get the selected cells from the tree
 */
Ecore_List *
ewl_tree2_selected_cells_get(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, FALSE);
	DCHECK_TYPE_RET("tree", tree, EWL_TREE2_TYPE, FALSE);

	DRETURN_PTR(tree->selected, DLEVEL_STABLE);
}

/**
 * @param tree: The tree to clear the selected cells from
 * @return Returns no value.
 * @brief Clear the selected cells in the tree
 */
void
ewl_tree2_selected_cells_clear(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	if (tree->mode == EWL_TREE_MODE_NONE)
		DRETURN(DLEVEL_STABLE);

	ecore_list_clear(tree->selected);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to get the mode from
 * @return Returns the current Ewl_Tree_Mode of the tree
 * @brief Get the mode from the tree
 */
Ewl_Tree_Mode 
ewl_tree2_mode_get(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, EWL_TREE_MODE_NONE);
	DCHECK_TYPE_RET("tree", tree, EWL_TREE2_TYPE, EWL_TREE_MODE_NONE);

	DRETURN_INT(tree->mode, DLEVEL_STABLE);
}

/**
 * @param tree: The Ewl_Tree to set the mode into
 * @param mode: The Ewl_Tree_Mode to set into the tree
 * @return Returns no value.
 * @brief Set the mode of the tree
 */
void 
ewl_tree2_mode_set(Ewl_Tree2 *tree, Ewl_Tree_Mode mode)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	if (tree->mode == mode)
		DRETURN(DLEVEL_STABLE);

	tree->mode = mode;
 
	/* if the mode is none then we don't care about the selected list */
	if (tree->mode == EWL_TREE_MODE_NONE)
	{
		if (tree->selected)
			ecore_list_destroy(tree->selected);
	}
	else
	{
		if (!tree->selected)
			tree->selected = ecore_list_new();
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to set the fixed row flag into
 * @param fixed: The fixed row flag to set into the tree
 * @return Returns no value.
 * @brief Set the fixed row size of the tree
 */
void
ewl_tree2_fixed_rows_set(Ewl_Tree2 *tree, unsigned int fixed)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree", tree);
	DCHECK_TYPE("tree", tree, EWL_TREE2_TYPE);

	tree->fixed = fixed;
	ewl_tree2_dirty_set(tree, TRUE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree: The tree to get the fixed row flag from
 * @return Returns the current fixed row flag of the tree
 * @brief Retrieve the fixed row size of the tree
 */
unsigned int
ewl_tree2_fixed_rows_get(Ewl_Tree2 *tree)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree", tree, FALSE);
	DCHECK_TYPE_RET("tree", tree, EWL_TREE2_TYPE, FALSE);

	DRETURN_INT(tree->fixed, DLEVEL_STABLE);
}

/**
 * @param tree2: The Ewl_Tree2 to work with
 * @param dirty: Set to TRUE if the data in the tree has changed
 * @return Returns no value
 * @brief Setting this to TRUE tells the tree that it's data has changed
 * and it will need to re-create its contents
 */
void
ewl_tree2_dirty_set(Ewl_Tree2 *tree2, unsigned int dirty)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("tree2", tree2);
	DCHECK_TYPE("tree2", tree2, EWL_TREE2_TYPE);

	if (tree2->dirty == dirty)
		DRETURN(DLEVEL_STABLE);

	tree2->dirty = !!dirty;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param tree2: The Ewl_Tree2 to use
 * @return Returns the dirty status of the tree
 * @brief Returns if the tree is currently dirty or not
 */
unsigned int
ewl_tree2_dirty_get(Ewl_Tree2 *tree2)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("tree2", tree2, FALSE);
	DCHECK_TYPE_RET("tree2", tree2, EWL_TREE2_TYPE, FALSE);

	DRETURN_INT(tree2->dirty, DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev: UNUSED
 * @param data: UNUSED
 * @return Returns no value
 * @brief The destroy callback
 */
void
ewl_tree2_cb_destroy(Ewl_Widget *w, void *ev __UNUSED__, void *data __UNUSED__)
{
	Ewl_Tree2 *t;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, EWL_WIDGET_TYPE);

	t = EWL_TREE2(w);

	ecore_list_destroy(t->columns);
	if (t->selected) ecore_list_destroy(t->selected);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param w: The widget to work with
 * @param ev: UNUSED
 * @param data: UNUSED
 * @return Returns no value
 * @brief The configure callback
 */
void
ewl_tree2_cb_configure(Ewl_Widget *w, void *ev __UNUSED__, 
					void *data __UNUSED__)
{
	Ewl_Tree2 *tree;
	Ewl_Tree2_Column *col;
	int column = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, EWL_TREE2_TYPE);

	tree = EWL_TREE2(w);

	/* place the header */
	ewl_object_place(EWL_OBJECT(tree->header), CURRENT_X(tree), 
				CURRENT_Y(tree), CURRENT_W(tree), 
				CURRENT_H(tree));

	/* if the tree isn't dirty we're done */
	if (!ewl_tree2_dirty_get(tree)) 
		DRETURN(DLEVEL_STABLE);

	/* setup the headers */
	ewl_container_reset(EWL_CONTAINER(tree->header));
	ecore_list_goto_first(tree->columns);
	while ((col = ecore_list_next(tree->columns)))
	{
		ewl_container_child_append(EWL_CONTAINER(tree->header), 
				col->view->header_fetch(tree->data, column));
		column ++;
	}

	ewl_tree2_dirty_set(tree, FALSE);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param c: The container to work with
 * @param w: UNUSED
 * @param size: UNUSED
 * @param o: UNUSED
 * @return Returns no value
 * @brief The child resize callback
 */
void
ewl_tree2_cb_child_resize(Ewl_Container *c, Ewl_Widget *w __UNUSED__,
					int size __UNUSED__,
					Ewl_Orientation o __UNUSED__)
{
	Ewl_Tree2 *tree;
	int hw, hh, rw, rh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);
	DCHECK_TYPE("c", c, EWL_CONTAINER_TYPE);

	tree = EWL_TREE2(c);

	ewl_object_preferred_size_get(EWL_OBJECT(tree->header), &hw, &hh);
	ewl_object_preferred_size_get(EWL_OBJECT(tree->rows), &rw, &rh);

	ewl_object_preferred_inner_size_set(EWL_OBJECT(tree), hw, hh + rh);

	ewl_object_place(EWL_OBJECT(tree->header), CURRENT_X(tree), CURRENT_Y(tree), hw, hh);
	ewl_object_place(EWL_OBJECT(tree->rows), CURRENT_X(tree), CURRENT_Y(tree) + hh, rw, rh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_tree2_cb_column_free(void *data)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);

	c = data;
	ewl_tree2_column_destroy(c);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Ewl_Tree2_Column stuff
 */

/**
 * @return Returns a new Ewl_Tree2_Column
 * @brief Creates a new Ewl_Tree2_Column object
 */
Ewl_Tree2_Column *
ewl_tree2_column_new(void)
{
	Ewl_Tree2_Column *c;

	DENTER_FUNCTION(DLEVEL_STABLE);

	c = NEW(Ewl_Tree2_Column, 1);

	DRETURN_PTR(c, DLEVEL_STABLE);
}

/**
 * @param c: The column to work with
 * @return Returns no value
 * @brief Destroys the given column 
 */
void
ewl_tree2_column_destroy(Ewl_Tree2_Column *c)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);

	c->model = NULL;
	c->view = NULL;
	FREE(c);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param c: The column to work with
 * @param m: The model to set
 * @return Returns no value
 * @brief Sets the given model @a m into the column @a c
 */
void
ewl_tree2_column_model_set(Ewl_Tree2_Column *c, Ewl_Model *m)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);
	DCHECK_PARAM_PTR("m", m);

	c->model = m;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param c: The column to work with
 * @return Returns the model for the column
 * @brief Retrieves the model for the given column
 */
Ewl_Model * 
ewl_tree2_column_model_get(Ewl_Tree2_Column *c)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, NULL);

	DRETURN_PTR(c->model, DLEVEL_STABLE);
}

/**
 * @param c: The column to work with
 * @param v: The view to set
 * @return Returns no value
 * @brief Sets the given view @a v into the column @a c
 */
void
ewl_tree2_column_view_set(Ewl_Tree2_Column *c, Ewl_View *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);
	DCHECK_PARAM_PTR("v", v);

	c->view = v;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param c: The Ewl_Tree2_Column to work with
 * @return Returns the view set on the given column
 * @brief Retrieves the view for the given column
 */
Ewl_View *
ewl_tree2_column_view_get(Ewl_Tree2_Column *c)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, NULL);

	DRETURN_PTR(c->view, DLEVEL_STABLE);
}


