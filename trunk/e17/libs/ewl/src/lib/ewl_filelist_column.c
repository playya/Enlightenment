#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

static Ewl_View *ewl_filelist_column_view = NULL;

static void ewl_filelist_column_dir_clicked(Ewl_Widget *w, void *event, void *data);
static void ewl_filelist_column_file_clicked(Ewl_Widget *w, void *event, void *data);
static void ewl_filelist_column_row_add(Ewl_Filelist *fl, const char *dir, char *file);

/**
 * @return Returns the Ewl_View needed to display the filelist_column
 * @brief Retrieves the needed Ewl_View to use the column view
 */
Ewl_View *
ewl_filelist_column_view_get(void)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	if (!ewl_filelist_column_view)
	{
		ewl_filelist_column_view = ewl_view_new();
		ewl_view_constructor_set(ewl_filelist_column_view, ewl_filelist_column_new);
	}

	DRETURN_PTR(ewl_filelist_column_view, DLEVEL_STABLE);
}

/**
 * @return Returns a new Ewl_Filelist_Column or NULL on failure
 * @brief Creates a new Ewl_Filelist_Column widget
 */
Ewl_Widget *
ewl_filelist_column_new(void)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = NEW(Ewl_Filelist_Column, 1);
	if (!w)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	if (!ewl_filelist_column_init(EWL_FILELIST_COLUMN(w)))
	{
		ewl_widget_destroy(w);
		w = NULL;
	}

	DRETURN_PTR(w, DLEVEL_STABLE);
}

/**
 * @param fl: The filelist to work with
 * @return Returns TRUE on success or FALSE on failure
 * @brief Initializes an Ewl_Filelist_Column to default values
 */
int
ewl_filelist_column_init(Ewl_Filelist_Column *fl)
{
	Ewl_Filelist *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("fl", fl, FALSE);

	if (!ewl_filelist_init(EWL_FILELIST(fl)))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_appearance_set(EWL_WIDGET(fl), EWL_FILELIST_COLUMN_TYPE);
	ewl_widget_inherit(EWL_WIDGET(fl), EWL_FILELIST_COLUMN_TYPE);

	list = EWL_FILELIST(fl);
	list->dir_change = ewl_filelist_column_dir_change;
	list->filter_change = ewl_filelist_column_dir_change;
	

	fl->hbox = ewl_scrollpane_new();
	ewl_box_orientation_set(EWL_BOX(EWL_SCROLLPANE(fl->hbox)->box), EWL_ORIENTATION_HORIZONTAL);
	ewl_container_child_append(EWL_CONTAINER(fl), fl->hbox);
	ewl_widget_show(fl->hbox);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param fl: The filelist to work with
 * @return Returns no value
 * @brief The callback for a directory change
 */
void
ewl_filelist_column_dir_change(Ewl_Filelist *fl)
{
	const char *path;

	Ewl_Filelist_Column *list;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("fl", fl);

	list = EWL_FILELIST_COLUMN(fl);

	path = ewl_filelist_directory_get(EWL_FILELIST(fl));

	if ( list->tree != NULL ) {
		Ewl_Widget *tree;
		Ewl_Widget *tree2;
		int index;

		tree = ewl_widget_name_find(path);
		index = ewl_container_child_index_get(EWL_CONTAINER(list->hbox), tree);
		tree2 = ewl_container_child_get(EWL_CONTAINER(list->hbox), index+1);
		ewl_widget_destroy(tree2);
		while ( tree2 != NULL ) {
			tree2 = ewl_container_child_get(EWL_CONTAINER(list->hbox), index+1);
			ewl_widget_destroy(tree2);
		}
	}

	list->tree = ewl_tree_new(1);
	ewl_tree_headers_visible_set(EWL_TREE(list->tree), 0);
	ewl_tree_expandable_rows_set(EWL_TREE(list->tree), FALSE);
	ewl_container_child_append(EWL_CONTAINER(list->hbox), list->tree);
	ewl_widget_show(list->tree);

	ewl_filelist_directory_read(fl, ewl_filelist_column_row_add);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



static void
ewl_filelist_column_dir_clicked(Ewl_Widget *w, void *ev, void *data)
{
	Ewl_Filelist_Column *fl;
	Ewl_Box *hbox;
	const char *dir;
	char *path;
	Ewl_Event_Mouse_Down *event;
	Ewl_Filelist_Column *list;

	DENTER_FUNCTION(DLEVEL_STABLE);

	hbox = EWL_BOX(w);
	fl = data;
	event = ev;

	list = EWL_FILELIST_COLUMN(fl);

	/* only trigger on lmb */
	if (event->button != 1)
		DRETURN(DLEVEL_STABLE);

	dir = ewl_widget_name_get(hbox);

	ewl_filelist_directory_set(EWL_FILELIST(fl), dir);

	FREE(path);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_filelist_column_file_clicked(Ewl_Widget *w, void *ev, void *data)
{

}

static void
ewl_filelist_column_row_add(Ewl_Filelist *fl, const char *dir, char *file)
{
	Ewl_Filelist_Column *list;
	Ewl_Widget *row;
	Ewl_Widget *children[2];
	Ewl_Widget *hbox;
	Ewl_Widget *image;
	Ewl_Widget *text;

	char *stock, path[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("fl", fl);
	DCHECK_PARAM_PTR("dir", dir);
	DCHECK_PARAM_PTR("file", file);
	DCHECK_TYPE("fl", fl, EWL_FILELIST_TYPE);

	list = EWL_FILELIST_COLUMN(fl);

	snprintf(path, PATH_MAX, "%s/%s", dir, file);

	hbox = ewl_hbox_new();
	ewl_widget_name_set(hbox, path);
	ewl_object_alignment_set(EWL_OBJECT(hbox), EWL_FLAG_ALIGN_CENTER);
	ewl_box_spacing_set(EWL_BOX(hbox), 5);
	ewl_widget_show(hbox);
		
	image = ewl_icon_new();
	ewl_container_child_append(EWL_CONTAINER(hbox), image);
	ewl_object_alignment_set(EWL_OBJECT(image), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(image), EWL_FLAG_FILL_SHRINK);
	ewl_widget_show(image);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), file);
	ewl_container_child_append(EWL_CONTAINER(hbox), text);
	ewl_object_alignment_set(EWL_OBJECT(text), EWL_FLAG_ALIGN_LEFT);
	ewl_widget_show(text);

	if (ecore_file_is_dir(path)) {
		stock = "stock/open";
		ewl_icon_image_set(EWL_ICON(image), ewl_theme_path_get(),
			 ewl_theme_data_str_get(EWL_WIDGET(image), stock));
		ewl_callback_append(hbox, EWL_CALLBACK_CLICKED,
				ewl_filelist_column_dir_clicked, fl);
	}
	else {
		stock = "/stock/arrow/right";
		ewl_icon_image_set(EWL_ICON(image), ewl_theme_path_get(),
			 ewl_theme_data_str_get(EWL_WIDGET(image), stock));
		ewl_callback_append(hbox, EWL_CALLBACK_CLICKED,
				ewl_filelist_column_file_clicked, fl);
	}
	
	children[0] = hbox;
	children[1] = NULL;
	row = ewl_tree_row_add(EWL_TREE(list->tree), NULL, children);
	ewl_widget_name_set(list->tree, path);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
