#include <Ewl.h>

static void __ewl_row_configure(Ewl_Widget * w, void *ev_data, void *user_data);

static void __ewl_row_add(Ewl_Container *c, Ewl_Widget *w);
static void __ewl_row_resize(Ewl_Container *c, Ewl_Widget *w, int size,
		Ewl_Orientation o);

/**
 * ewl_row_new - allocate and initialize a new row
 *
 * Returns a newly allocated row on success, NULL on failure.
 */
Ewl_Widget *ewl_row_new()
{
	Ewl_Widget *row;

	DENTER_FUNCTION(DLEVEL_STABLE);

	row = NEW(Ewl_Row, 1);
	if (!row)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	ZERO(row, Ewl_Row, 1);
	if (!ewl_row_init(EWL_ROW(row))) {
		FREE(row);
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	DRETURN_PTR(row, DLEVEL_STABLE);
}

/**
 * ewl_row_init - initialize the row fields of an inheriting object
 * @row: the row object to initialize
 *
 * Returns TRUE on success, FALSE on failure. The fields of the @row object
 * are initialized to their defaults.
 */
int ewl_row_init(Ewl_Row *row)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR_RET("row", row, FALSE);

	ewl_container_init(EWL_CONTAINER(row), "row", __ewl_row_add,
			__ewl_row_resize, NULL);

	ewl_callback_append(EWL_WIDGET(row), EWL_CALLBACK_CONFIGURE,
			__ewl_row_configure, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * ewl_row_set_column_bounds - set the table of constraints on cell widths
 * @row: the row to change the column table
 * @base: base position for finding column widths, NULL causes no constraints
 * @bounds: the array of column width bounds, NULL causes no constraints
 *
 * Returns no value. The table of widths for @row is changed to @bounds, rows
 * calculate their actual width based on the position of @base. You must set
 * both of these variables to set one. It is strongly recommended that you add
 * a configure callback to the widget that contains this table and base to be
 * notified when the values change. If @base or @bounds is NULL, then each cell
 * is given it's preferred size.
 */
void
ewl_row_set_column_bounds(Ewl_Row *row, int n, unsigned int *base,
		unsigned int **bounds)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("row", row);

	if (!base || !bounds) {
		row->base = NULL;
		row->bounds = NULL;
	}
	else {
		row->base = base;
		row->bounds = bounds;
	}

	ewl_widget_configure(EWL_WIDGET(row));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_row_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Row *row;
	Ewl_Container *c;
	Ewl_Object *child;
	int x;
	int i = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	row = EWL_ROW(w);
	c = EWL_CONTAINER(w);

	x = CURRENT_X(w);
	ewd_list_goto_first(c->children);

	while ((child = ewd_list_next(c->children))) {
		ewl_object_request_position(child, x, CURRENT_Y(w));

		if (row->bounds && row->bounds[i]) {
			ewl_object_request_w(child,
					*row->base + *row->bounds[i] - x);
		}
		else {
			ewl_object_request_w(child,
					ewl_object_get_preferred_w(child));
		}
		ewl_object_request_h(child,
				ewl_object_get_preferred_h(EWL_OBJECT(w)));
		i++;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_row_add(Ewl_Container *c, Ewl_Widget *w)
{
	int size;
	Ewl_Row *row;

	row = EWL_ROW(c);

	/*
	 * Adjust the preferred height to the largest widget added.
	 */
	size = ewl_object_get_preferred_h(EWL_OBJECT(w));
	if (ewl_object_get_preferred_h(row->max) > size) {
		row->max = EWL_OBJECT(w);
		ewl_object_set_preferred_h(EWL_OBJECT(c), size);
	}

	ewl_object_set_preferred_w(EWL_OBJECT(c), PREFERRED_W(c) +
			ewl_object_get_preferred_w(EWL_OBJECT(w)));
}

static void
__ewl_row_resize(Ewl_Container *c, Ewl_Widget *w, int size, Ewl_Orientation o)
{
	Ewl_Row *row;

	row = EWL_ROW(c);
	if (o == EWL_ORIENTATION_VERTICAL) {
		if (EWL_OBJECT(w) == row->max && size > 0)
			PREFERRED_H(c) += size;
		else {
			int h;
			int max_h = 0;
			Ewl_Object *child;

			/*
			 * Search out the tallest widget in the row
			 */
			ewd_list_goto_first(c->children);
			while ((child = ewd_list_next(c->children))) {
				h = ewl_object_get_preferred_h(child);
				if (h > max_h) {
					max_h = h;
					row->max = child;
				}
			}

			PREFERRED_H(c) = max_h;
		}
	}
	else {
		PREFERRED_W(c) += size;
	}
}
