/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "ewl_base.h"
#include "ewl_filelist_tree.h"
#include "ewl_label.h"
#include "ewl_tree2.h"
#include "ewl_macros.h"
#include "ewl_private.h"
#include "ewl_debug.h"

/**
 * Ewl_Filelist_Tree_Data
 */
typedef struct Ewl_Filelist_Tree_Data Ewl_Filelist_Tree_Data;
#define EWL_FILELIST_TREE_DATA(data) ((Ewl_Filelist_Tree_Data *)(data))

/**
 * @brief Contains information on a filelist tree data
 */
struct Ewl_Filelist_Tree_Data
{
	Ewl_Filelist_Tree *list;
	Ecore_List *files;
};

static Ewl_View *ewl_filelist_tree_view = NULL;

static Ewl_Widget * ewl_filelist_tree_view_widget_fetch(void *data,
							unsigned int row,
							unsigned int col);
static void ewl_filelist_tree_add(Ewl_Filelist *fl, const char *dir,
						char *file, void *data);

static Ewl_Widget *ewl_filelist_tree_cb_widget_fetch(void *data,
							unsigned int row,
							unsigned int column);
static Ewl_Widget *ewl_filelist_tree_cb_header_fetch(void *data,
							unsigned int column);

/* Model callbacks */
static void * ewl_filelist_tree_data_fetch(void *data, unsigned int row,
						unsigned int column);
static void ewl_filelist_tree_data_sort(void *data, unsigned int column,
						Ewl_Sort_Direction sort);
static unsigned int ewl_filelist_tree_data_count(void *data);
static int ewl_filelist_tree_data_expandable_get(void *data, unsigned int row);
static void *ewl_filelist_tree_data_expansion_data_fetch(void *data,
						unsigned int parent);

/**
 * @return Returns the view for the filelist tree
 * @brief Retrieves the Ewl_View needed to use the filelist tree view
 */
Ewl_View *
ewl_filelist_tree_view_get(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!ewl_filelist_tree_view)
	{
		ewl_filelist_tree_view = ewl_view_new();
		ewl_view_widget_fetch_set(ewl_filelist_tree_view,
						ewl_filelist_tree_view_widget_fetch);
	}

	DRETURN_PTR(ewl_filelist_tree_view, DLEVEL_STABLE);
}

static Ewl_Widget *
ewl_filelist_tree_view_widget_fetch(void *data __UNUSED__,
					unsigned int row __UNUSED__,
					unsigned int col __UNUSED__)
{
	Ewl_Widget *tree;

	DENTER_FUNCTION(DLEVEL_STABLE);

	tree = ewl_filelist_tree_new();

	DRETURN_PTR(tree, DLEVEL_STABLE);
}

/**
 * @return Returns a new Ewl_Filelist_Tree widget or NULL on failure
 * @brief Creates a new Ewl_Filelist_Tree widget
 */
Ewl_Widget *
ewl_filelist_tree_new(void)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = NEW(Ewl_Filelist_Tree, 1);
	if (!w)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_filelist_tree_init(EWL_FILELIST_TREE(w)))
	{
		ewl_widget_destroy(w);
		w = NULL;
	}

	DRETURN_PTR(w, DLEVEL_STABLE);
}

/**
 * @param fl: The Ewl_Filelist_Tree to initialize
 * @return Returns TRUE on success or FALSE on failure
 * @brief Initializes an Ewl_Filelist_Tree widget to default values
 */
int
ewl_filelist_tree_init(Ewl_Filelist_Tree *fl)
{
	Ewl_View *view;
	Ewl_Model *model;
	Ewl_Filelist *tree;
	Ewl_Filelist_Tree_Data *data;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(fl, FALSE);

	if (!ewl_filelist_init(EWL_FILELIST(fl)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_appearance_set(EWL_WIDGET(fl), EWL_FILELIST_TREE_TYPE);
	ewl_widget_inherit(EWL_WIDGET(fl), EWL_FILELIST_TREE_TYPE);

	tree = EWL_FILELIST(fl);
	tree->dir_change = ewl_filelist_tree_dir_change;
	tree->filter_change = ewl_filelist_tree_dir_change;
	tree->show_dot_change = ewl_filelist_tree_dir_change;
	tree->selected_file_add = ewl_filelist_tree_selected_file_add;
	tree->file_name_get = ewl_filelist_tree_filename_get;
	tree->selected_unselect = ewl_filelist_tree_selected_unselect;
	tree->shift_handle = ewl_filelist_tree_shift_handle;

	/* Wrapper struct to keep a pointer to our filelist */
	data = NEW(Ewl_Filelist_Tree_Data, 1);
	data->list = fl;
	data->files = ecore_list_new();
	ecore_list_free_cb_set(data->files, ECORE_FREE_CB(free));

	/* Setup the tree model */
	model = ewl_model_new();
	ewl_model_data_count_set(model, ewl_filelist_tree_data_count);
	ewl_model_data_fetch_set(model, ewl_filelist_tree_data_fetch);
	ewl_model_data_sort_set(model, ewl_filelist_tree_data_sort);
	ewl_model_data_expandable_set(model, ewl_filelist_tree_data_expandable_get);
	ewl_model_expansion_data_fetch_set(model,
			ewl_filelist_tree_data_expansion_data_fetch);

	view = ewl_view_new();
	ewl_view_widget_fetch_set(view, ewl_filelist_tree_cb_widget_fetch);
	ewl_view_header_fetch_set(view, ewl_filelist_tree_cb_header_fetch);

	fl->tree = ewl_tree2_new();
	ewl_tree2_column_count_set(EWL_TREE2(fl->tree), 2);
	ewl_mvc_data_set(EWL_MVC(fl->tree), data);
	ewl_mvc_model_set(EWL_MVC(fl->tree), model);
	ewl_mvc_view_set(EWL_MVC(fl->tree), view);
	ewl_container_child_append(EWL_CONTAINER(fl), fl->tree);
	ewl_widget_show(fl->tree);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param fl: The filelist to work with
 * @return Returns no value
 * @brief The callback to notify of a directory change
 */
void
ewl_filelist_tree_dir_change(Ewl_Filelist *fl)
{
	Ewl_Filelist_Tree *tree;;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR(fl);

	tree = EWL_FILELIST_TREE(fl);
	ewl_filelist_directory_read(fl, ewl_filelist_directory_get(fl),
				FALSE, ewl_filelist_tree_add, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param fl: The filelist to work with
 * @param file: The file to set selected
 * @return Returns no value
 * @brief Callback when the selected files are changed
 */
void
ewl_filelist_tree_selected_file_add(Ewl_Filelist *fl,
				const char *file __UNUSED__)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR(fl);

	/* XXX Write me ... */

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param fl: The filelist to work with
 * @param item: The item to get the name from
 * @return Returns the filename for the given item
 * @brief Retrieves the filename for the given item
 */
const char *
ewl_filelist_tree_filename_get(Ewl_Filelist *fl, void *item)
{
	Ewl_Filelist_Tree_Data *data;
	Ewl_Selection_Idx *selected;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(fl, NULL);
	DCHECK_PARAM_PTR_RET(item, NULL);
	DCHECK_TYPE_RET(fl, EWL_FILELIST_TYPE, NULL);

	selected = ewl_mvc_selected_get(EWL_MVC(fl));
	data = selected->sel.data;

	/* XXX Don't think this is right */

	DRETURN_PTR(ecore_list_index_goto(data->files, selected->row), DLEVEL_STABLE);
}

/**
 * @internal
 * @param fl: The filelist to work with
 * @return Returns no value.
 * @brief This will set all of the rows back to their unselected state
 */
void
ewl_filelist_tree_selected_unselect(Ewl_Filelist *fl)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR(fl);
	DCHECK_TYPE(fl, EWL_FILELIST_TYPE);

	ewl_filelist_selected_signal_all(fl, "row,unselect");

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 * @param fl: The filelist to deal with
 * @param clicked: The currently clicked item
 * @return Returns no value
 * @brief Select the appropriate widgets to deal with a shift click
 */
void
ewl_filelist_tree_shift_handle(Ewl_Filelist *fl, Ewl_Widget *clicked)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR(fl);
	DCHECK_PARAM_PTR(clicked);
	DCHECK_TYPE(fl, EWL_FILELIST_TYPE);
	DCHECK_TYPE(clicked, EWL_WIDGET_TYPE);

	/* XXX fix me */

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_filelist_tree_add(Ewl_Filelist *fl, const char *dir, char *file,
						void *data __UNUSED__)
{
	Ewl_Filelist_Tree *flt;
	Ewl_Filelist_Tree_Data *d;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR(fl);
	DCHECK_PARAM_PTR(dir);
	DCHECK_PARAM_PTR(file);
	DCHECK_TYPE(fl, EWL_FILELIST_TYPE);

	flt = EWL_FILELIST_TREE(fl);
	d = ewl_mvc_data_get(EWL_MVC(flt->tree));
	ecore_list_append(d->files, strdup(file));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 */
static unsigned int
ewl_filelist_tree_data_count(void *data)
{
	Ewl_Filelist_Tree_Data *td = data;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(data, 0);

	DRETURN_INT(ecore_list_count(td->files), DLEVEL_STABLE);
}

/**
 * @internal
 */
static void *
ewl_filelist_tree_data_fetch(void *data, unsigned int row, unsigned int col)
{
	Ewl_Filelist_Tree_Data *td;
	char *ret = NULL, *tmp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(data, 0);

	td = data;
	tmp = ecore_list_index_goto(td->files, row);
	if (col == 0)
	{
		ret = tmp;
	}
	else
	{
		ret = "";
	}

printf("RET %p %s\n", data, ret);

	DRETURN_PTR(ret, DLEVEL_STABLE);
}

/**
 * @internal
 */
static void
ewl_filelist_tree_data_sort(void *data, unsigned int column __UNUSED__,
						Ewl_Sort_Direction sort)
{
	Ewl_Filelist_Tree_Data *td = data;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (sort == EWL_SORT_DIRECTION_NONE)
		DRETURN(DLEVEL_STABLE);

	/* Handle the correct sort order */
	if (sort == EWL_SORT_DIRECTION_ASCENDING)
		ecore_list_sort(td->files, ECORE_COMPARE_CB(strcoll), 
				ECORE_SORT_MIN);
	else
		ecore_list_sort(td->files, ECORE_COMPARE_CB(strcoll), 
				ECORE_SORT_MAX);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @internal
 */
static int
ewl_filelist_tree_data_expandable_get(void *data, unsigned int row)
{
	int result;
	char *path;
	const char *file;
	Ewl_Filelist_Tree_Data *td = data;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(data, 0);

	file = ecore_list_index_goto(td->files, row);
	if (!strcmp(file, ".."))
		DRETURN_INT(0, DLEVEL_STABLE);

	path = ewl_filelist_expand_path(EWL_FILELIST(td->list), file);
	result = ecore_file_is_dir(path);
	FREE(path);

	DRETURN_INT(result, DLEVEL_STABLE);
}

/**
 * @internal
 */
static void *
ewl_filelist_tree_data_expansion_data_fetch(void *data, unsigned int parent)
{
	char *path;
	const char *file;
	Ecore_List *subdir = NULL;
	Ewl_Filelist_Tree_Data *td = data;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET(data, NULL);

	file = ecore_list_index_goto(td->files, parent);
	path = ewl_filelist_expand_path(EWL_FILELIST(td->list), file);

//	subdir = ecore_file_ls(path);
	FREE(path);

	DRETURN_PTR(subdir, DLEVEL_STABLE);
}

static
Ewl_Widget *ewl_filelist_tree_cb_widget_fetch(void *data,
						unsigned int row __UNUSED__,
						unsigned int column __UNUSED__)
{
	Ewl_Widget *l;

	DENTER_FUNCTION(DLEVEL_STABLE);

	l = ewl_label_new();
	ewl_label_text_set(EWL_LABEL(l), data);
	ewl_widget_show(l);

	DRETURN_PTR(l, DLEVEL_STABLE);
}

static
Ewl_Widget *ewl_filelist_tree_cb_header_fetch(void *data __UNUSED__,
							unsigned int column)
{
	Ewl_Widget *l;
	const char *t;


	DENTER_FUNCTION(DLEVEL_STABLE);

	l = ewl_label_new();
	if (column == 0) t = "filename";
	else if (column == 1) t = "size";
	else if (column == 2) t = "modifed";
	else if (column == 3) t = "permissions";
	else if (column == 4) t = "group";
	else t = "";

	ewl_label_text_set(EWL_LABEL(l), t);
	ewl_widget_show(l);

	DRETURN_PTR(l, DLEVEL_STABLE);
}


