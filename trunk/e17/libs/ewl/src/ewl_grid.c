#include <Ewl.h>

void            __ewl_grid_realize(Ewl_Widget * w, void *ev_data,
				   void *user_data);
void            __ewl_grid_configure(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            __ewl_grid_resize(Ewl_Grid * g);
void            __ewl_grid_add(Ewl_Container * p, Ewl_Widget * c);
void            __ewl_grid_auto_resize(Ewl_Container * p, Ewl_Widget * child,
				       int size, Ewl_Orientation o);

/**
 * ewl_grid_new - create a new grid
 * @cols: number of columns
 * @rows: number of rows
 *
 * Returns a pointer to a newly allocated grid on success, NULL on
 * failure.
 */
Ewl_Widget     *
ewl_grid_new(int cols, int rows)
{
	Ewl_Grid       *g;

	DENTER_FUNCTION(DLEVEL_STABLE);

	g = NEW(Ewl_Grid, 1);
	if (!g)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	memset(g, 0, sizeof(Ewl_Grid));

	ewl_grid_init(g, cols, rows);

	DRETURN_PTR(EWL_WIDGET(g), DLEVEL_STABLE);
}



/**
 * ewl_grid_init - initialize the grid to starting values
 * @g: the grid
 * @cols: number of columns
 * @rows: number of rows
 *
 * Returns no value. Responsible for setting up default values and
 * callbacks within a grid structure
 */
void
ewl_grid_init(Ewl_Grid * g, int cols, int rows)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	/*
	 * Initialize the grids inherited fields
	 */
	ewl_container_init(EWL_CONTAINER(g),
			   "/appearance/box/vertical", __ewl_grid_add,
			   __ewl_grid_auto_resize);

	/*
	 * Initialize the lists that keep track of the
	 * horisontal and vertical size of cols/rows
	 */
	g->col_size = NEW(Ewl_Grid_Info, cols);
	ZERO(g->col_size, Ewl_Grid_Info, cols);

	g->row_size = NEW(Ewl_Grid_Info, rows);
	ZERO(g->row_size, Ewl_Grid_Info, rows);

	/*
	 * Store the cols/rows in the grid
	 */
	g->cols = cols;
	g->rows = rows;

	g->rchildren = NULL;

	/*
	 * Append callbacks
	 */
	ewl_callback_append(EWL_WIDGET(g), EWL_CALLBACK_REALIZE,
			    __ewl_grid_realize, NULL);
	ewl_callback_append(EWL_WIDGET(g), EWL_CALLBACK_CONFIGURE,
			    __ewl_grid_configure, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_grid_reset - clear the grid and set new geometry
 * @g: the grid
 * @cols: the new number of columns
 * @rows: the new number of rows
 *
 * Returns no value
 */
void
ewl_grid_reset(Ewl_Grid * g, int cols, int rows)
{
	Ewl_Widget     *w;
	int             i;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	w = EWL_WIDGET(g);

	g->rchildren = EWL_CONTAINER(w)->children;
	EWL_CONTAINER(w)->children = NULL;
	EWL_CONTAINER(w)->children = ewd_list_new();

	IF_FREE(g->col_size);
	IF_FREE(g->row_size);

	g->col_size = NEW(Ewl_Grid_Info, cols);
	ZERO(g->col_size, Ewl_Grid_Info, cols);

	g->row_size = NEW(Ewl_Grid_Info, rows);
	ZERO(g->row_size, Ewl_Grid_Info, rows);

	g->cols = cols;
	g->rows = rows;


	// store the total size of the grid widget /
	g->grid_w = CURRENT_W(EWL_OBJECT(w));
	g->grid_h = CURRENT_H(EWL_OBJECT(w));


	// initialize the column width to default values /
	for (i = 0; i < g->cols; i++)
		g->col_size[i].size = CURRENT_W(g) / g->cols;

	// initialize the row height to default values /
	for (i = 0; i < g->rows; i++)
		g->row_size[i].size = CURRENT_H(g) / g->rows;

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



/**
 * ewl_grid_add - add a child widget to the grid
 * @g: the grid
 * @w: the child widget
 * @start_col: the start column
 * @end_col: the end column
 * @start_row: the start row
 * @end_row: the end row
 *
 * Returns no value
 */
void
ewl_grid_add(Ewl_Grid * g, Ewl_Widget * w,
	     int start_col, int end_col, int start_row, int end_row)
{
	Ewl_Grid_Child *child;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);
	DCHECK_PARAM_PTR("w", w);

	/*
	 * check bounds
	 */
	if (start_col < 1) {
		printf("start_col out of bounds. min is 1\n");
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}
	if (end_col > g->cols) {
		printf("end_col out of bounds. max is %d\n", g->cols);
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}
	if (start_row < 1) {
		printf("start_row out of bounds. min is 1\n");
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}
	if (end_row > g->rows) {
		printf("end_row out of bounds. max is %d\n", g->rows);
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}


	/* create a new child */
	child = NEW(Ewl_Grid_Child, 1);
	if (!child)
		DLEAVE_FUNCTION(DLEVEL_STABLE);

	memset(child, 0, sizeof(Ewl_Grid_Child));

	child->start_col = start_col;
	child->end_col = end_col;
	child->start_row = start_row;
	child->end_row = end_row;

	/* store the child info in the child widget */
	ewl_widget_set_data(w, (void *) g, child);
	ewl_container_append_child(EWL_CONTAINER(g), w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



/**
 * ewl_grid_set_col_w - set the width of a column
 * @g: the grid
 * @col: the column
 * @width: the new width
 *
 * Returns no value.
 */
void
ewl_grid_set_col_w(Ewl_Grid * g, int col, int width)
{

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	/*
	 * check bounds
	 */
	if (col < 1 || col > g->cols) {
		printf("parameter 'col' is out of bounds\n");
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}

	g->col_size[col - 1].override = 1;
	ewl_object_request_size(EWL_OBJECT(g),
				ewl_object_get_current_w(EWL_OBJECT(g)) +
				(width - g->col_size[col - 1].size),
				ewl_object_get_current_h(EWL_OBJECT(g)));

	g->col_size[col - 1].size = width -	/* this will be reverted in resize */
		((ewl_object_get_current_w(EWL_OBJECT(g)) -
		  g->grid_w) / g->cols);


	ewl_widget_configure(EWL_WIDGET(g));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_grid_get_col_w - get the width of a column
 *
 * @g: the grid
 * @col: the column
 * @width: integer pointer to store the width in
 *
 * Returns no value.
 */
void
ewl_grid_get_col_w(Ewl_Grid * g, int col, int *width)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	*width = g->col_size[col].size;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


/**
 * ewl_grid_set_row_h - set the height of a row
 * @g: the grid
 * @col: the row
 * @width: the new height
 *
 * Returns no value.
 */
void
ewl_grid_set_row_h(Ewl_Grid * g, int row, int height)
{

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	/* check bounds */
	if (row < 1 || row > g->rows) {
		printf("parameter 'row' is out of bounds\n");
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}

	g->row_size[row - 1].override = 1;
	ewl_object_request_size(EWL_OBJECT(g),
				ewl_object_get_current_w(EWL_OBJECT(g)),
				ewl_object_get_current_h(EWL_OBJECT(g)) +
				(height - g->row_size[row - 1].size));

	g->row_size[row - 1].size = height -	/* this will be reverted in resize */
		((ewl_object_get_current_h(EWL_OBJECT(g)) -
		  g->grid_h) / g->rows);


	ewl_widget_configure(EWL_WIDGET(g));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_grid_get_row_h - get the height of a row
 *
 * @g: the grid
 * @row: the row
 * @height: integer pointer to store the height in
 *
 * Returns no value.
 */
void
ewl_grid_get_row_h(Ewl_Grid * g, int row, int *height)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("g", g);

	*height = g->row_size[row].size;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



void
__ewl_grid_realize(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Grid       *g;
	int             i;

	DENTER_FUNCTION(DLEVEL_STABLE);

	g = EWL_GRID(w);

	ewl_widget_show(w);

	// store the total size of the grid widget /
	g->grid_w = CURRENT_W(EWL_OBJECT(w));
	g->grid_h = CURRENT_H(EWL_OBJECT(w));


	// initialize the column width to default values /
	for (i = 0; i < g->cols; i++)
		g->col_size[i].size = g->grid_w / g->cols;

	// initialize the row height to default values /
	for (i = 0; i < g->rows; i++)
		g->row_size[i].size = g->grid_h / g->rows;

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


void
__ewl_grid_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Grid       *g;
	Ewl_Grid_Child *c;
	Ewl_Widget     *child;
	int             c_w = 0, c_h = 0;	/* child width/height */
	int             c_x = 0, c_y = 0;	/* child x/y coordinate */
	int             i;

	DENTER_FUNCTION(DLEVEL_STABLE);


	g = EWL_GRID(w);


	/*
	 * first check if the grid has been reset
	 * if so, we need to destroy the old children
	 */
	if (g->rchildren) {
		while ((child = ewd_list_remove_first(g->rchildren)) != NULL)
			ewl_widget_destroy(child);
		g->rchildren = NULL;
	}


	__ewl_grid_resize(g);

	c_x = CURRENT_X(EWL_OBJECT(w));
	c_y = CURRENT_Y(EWL_OBJECT(w));

	ewd_list_goto_first(EWL_CONTAINER(w)->children);
	while ((child = ewd_list_next(EWL_CONTAINER(w)->children)) != NULL) {
		c = (Ewl_Grid_Child *) ewl_widget_get_data(child, (void *) g);

		/* calculate child widgets width */
		for (i = c->start_col - 1; i < c->end_col; i++)
			c_w += g->col_size[i].size;

		/* calculate child widgets height */
		for (i = c->start_row - 1; i < c->end_row; i++)
			c_h += g->row_size[i].size;

		/* calculate child widgets x coordinate */
		for (i = 0; i < (c->start_col - 1); i++)
			c_x += g->col_size[i].size;

		/* calculate child widgets y coordinate */
		for (i = 0; i < (c->start_row - 1); i++)
			c_y += g->row_size[i].size;


		ewl_object_request_geometry(EWL_OBJECT(child), c_x, c_y, c_w,
					    c_h);
		ewl_widget_configure(child);

		/* reset geometry values for the next child */
		c_x = CURRENT_X(EWL_OBJECT(w));
		c_y = CURRENT_Y(EWL_OBJECT(w));
		c_w = 0;
		c_h = 0;
	}


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



void
__ewl_grid_resize(Ewl_Grid * g)
{
	int             w_flag = 0, h_flag = 0;
	int             i, new_w = 0, new_h = 0;
	int             left_over;

	DENTER_FUNCTION(DLEVEL_STABLE);


	/* store the total size of the grid widget */
	if (ewl_object_get_current_w(EWL_OBJECT(g)) != g->grid_w) {
		new_w = ewl_object_get_current_w(EWL_OBJECT(g));
		w_flag = 1;
	}

	if (ewl_object_get_current_h(EWL_OBJECT(g)) != g->grid_h) {
		new_h = ewl_object_get_current_h(EWL_OBJECT(g));
		h_flag = 1;
	}


	/* 
	 * if grid with has changed we need to store the new column
	 * width
	 */
	if (w_flag) {
		for (i = 0; i < g->cols; i++) {
			g->col_size[i].size += ((new_w - g->grid_w) / g->cols);

/*			printf("col: %d, width: %d\n", i, g->col_size[i].size); */
		}
		g->grid_w = new_w;
	}

	/*
	 * if grid height has changed we need to store the new row
	 * height
	 */
	if (h_flag) {
		for (i = 0; i < g->rows; i++) {
			g->row_size[i].size += ((new_h - g->grid_h) / g->rows);

/*			printf("a row: %d, height: %d\n", i, g->row_size[i].size); */
		}
		g->grid_h = new_h;
	}


	/*
	 * since the above set values may be doubles rounded down there
	 * might be some more space to fill at the right and bottom.
	 * this claims the left over space
	 */
	left_over = g->grid_w;
	for (i = 0; i < g->cols; i++)
		left_over -= g->col_size[i].size;
	if (g->cols == 0)
		g->cols = 1;
	while (left_over > 0) {
		g->col_size[left_over % g->cols].size += 1;
		left_over--;
	}

	left_over = g->grid_h;
	for (i = 0; i < g->rows; i++)
		left_over -= g->row_size[i].size;
	if (g->rows == 0)
		g->rows = 1;
	while (left_over > 0) {
		g->row_size[left_over % g->rows].size += 1;
		left_over--;
	}


	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Notify the grid that a child has been added.
 */
void
__ewl_grid_add(Ewl_Container * p, Ewl_Widget * c)
{
	int             i;
	int             temp;
	Ewl_Grid       *g;
	Ewl_Grid_Child *cdata;

	g = EWL_GRID(p);

	cdata = ewl_widget_get_data(c, (void *) g);

	/*
	 * If no data exists, we should add some defaults.
	 */
	if (!cdata) {
		cdata = NEW(Ewl_Grid_Child, 1);
		cdata->start_col = cdata->end_col = 1;
		cdata->start_row = cdata->end_row = 1;

		ewl_widget_set_data(c, g, cdata);
	}

	/*
	 * Add the widget to columns that it intersects.
	 */
	for (i = cdata->start_col - 1; i < cdata->end_col; i++) {
		if (!g->col_size[i].cross)
			g->col_size[i].cross = ewd_list_new();

		ewd_list_append(g->col_size[i].cross, c);

		/*
		 * Calculate the amount of space the widget would need in this
		 * column.
		 */
		temp = ewl_object_get_preferred_w(EWL_OBJECT(c)) /
			(cdata->end_col - cdata->start_col + 1);

		/*
		 * Give the column a new preferred size based on the added
		 * widget.
		 */
		if (g->col_size[i].size < temp) {
			if (!g->col_size[i].override)
				g->col_size[i].size = temp;

			/*
			 * Save a pointer to the largest child.
			 */
			g->col_size[i].max = c;
		}

	}

	/*
	 * Add the widget to rows that it intersects.
	 */
	for (i = cdata->start_row - 1; i < cdata->end_row; i++) {
		if (!g->row_size[i].cross)
			g->row_size[i].cross = ewd_list_new();

		ewd_list_append(g->row_size[i].cross, c);

		/*
		 * Calculate the amount of space the widget would need in this
		 * row.
		 */
		temp = ewl_object_get_preferred_h(EWL_OBJECT(c)) /
			(cdata->end_row - cdata->start_row + 1);

		/*
		 * Give the row a new preferred size based on the added
		 * widget.
		 */
		if (g->row_size[i].size < temp) {
			if (!g->row_size[i].override)
				g->row_size[i].size = temp;

			/*
			 * Save a pointer to the largest child.
			 */
			g->row_size[i].max = c;
		}
	}
}

/*
 * Catch notification of child resizes.
 */
void
__ewl_grid_auto_resize(Ewl_Container * p, Ewl_Widget * child, int size,
		       Ewl_Orientation o)
{
	int             give;
	Ewl_Grid       *g;
	int             used = 0;
	int             start_off, end_off;
	Ewl_Grid_Info  *info;
	int             i, num_spread = 0;
	Ewl_Grid_Child *cdata;
	int             (*widget_size) (Ewl_Object * o);

	DENTER_FUNCTION(DLEVEL_STABLE);

	g = EWL_GRID(p);
	cdata = ewl_widget_get_data(child, (void *) g);

	/*
	 * Setup a couple orientation specific variables.
	 */
	if (o == EWL_ORIENTATION_HORIZONTAL) {
		info = g->col_size;
		start_off =
			(unsigned int) &cdata->start_col - (unsigned int) cdata;
		end_off = (unsigned int) &cdata->end_col - (unsigned int) cdata;
		widget_size = ewl_object_get_preferred_w;
	} else {
		info = g->row_size;
		start_off =
			(unsigned int) &cdata->start_row - (unsigned int) cdata;
		end_off = (unsigned int) &cdata->end_row - (unsigned int) cdata;
		widget_size = ewl_object_get_preferred_h;
	}

	/*
	 * Count the number of resizable columns to give the size change to.
	 */
	for (i = *(int *) (cdata + start_off) - 1;
	     i < *(int *) (cdata + end_off); i++) {
		if (!info[i].override)
			num_spread++;
	}

	/*
	 * No need to continue if none of the grid spaces will accept space.
	 */
	if (!num_spread)
		DRETURN(DLEVEL_STABLE);

	/*
	 * Give out space to each of the grid spaces that will accept it.
	 */
	give = size / num_spread;
	for (i = *(int *) (cdata + start_off) - 1;
	     i < *(int *) (cdata + end_off) && num_spread; i++) {
		if (!info[i].override) {

			/*
			 * This case is simple, just add in the amount
			 * specified.
			 */
			if (child == info[i].max && give > 0) {
				info[i].size += give;
				used += give;
				num_spread--;
			} else {
				int             max = 0;
				Ewl_Widget     *temp;

				/*
				 * Otherwise we need to search for the largest
				 * widget in this space in the grid.
				 */
				ewd_list_goto_first(info[i].cross);
				while ((temp = ewd_list_next(info[i].cross))) {
					if (widget_size(EWL_OBJECT(temp)) > max) {
						max = widget_size(EWL_OBJECT
								  (temp));
						info[i].max = temp;
					}
				}
			}
		}
	}

	/*
	 * Hand out any remaining space available.
	 */
	info[i - 1].size += size % num_spread;
	used += size % num_spread;

	/*
	 * Now resize the grid appropriately.
	 */
	if (o == EWL_ORIENTATION_HORIZONTAL)
		ewl_object_request_w(EWL_OBJECT(g), CURRENT_W(g) -
				     (INSET_LEFT(g) + INSET_RIGHT(g)) + used);
	else
		ewl_object_request_h(EWL_OBJECT(g), CURRENT_H(g) -
				     (INSET_TOP(g) + INSET_BOTTOM(g)) + used);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
