#include <Ewl.h>


static void __ewl_table_init(Ewl_Table * t);

static void __ewl_table_realize(Ewl_Widget * w, void *event_data,
				void *user_data);
static void __ewl_table_show(Ewl_Widget * w, void *event_data,
			     void *user_data);
static void __ewl_table_hide(Ewl_Widget * w, void *event_data,
			     void *user_data);
static void __ewl_table_destroy(Ewl_Widget * w, void *event_data,
				void *user_data);
static void __ewl_table_destroy_recursive(Ewl_Widget * w, void *event_data,
					  void *user_data);
static void __ewl_table_configure(Ewl_Widget * w, void *event_data,
				  void *user_data);
static void __ewl_table_theme_update(Ewl_Widget * w, void *event_data,
				     void *user_data);

/* make the configure callback smaller by spliting up big stuff into
 * small functions... */
static void __ewl_table_configure_gfx(Ewl_Widget * w);
static void __ewl_table_configure_children(Ewl_Widget * w);
Ewd_List *__ewl_table_fill_normal(Ewl_Widget * w, int *rem_w, int *rem_h);
void __ewl_table_fill_fillers(Ewl_Widget * w,
			      int rem_w, int rem_h, Ewd_List * l);
static void __ewl_table_layout_children(Ewl_Table * w);

Ewl_Widget *
ewl_table_new(unsigned int columns, unsigned int rows)
{
	return ewl_table_new_all(columns, rows, FALSE, 2, 2);
}

Ewl_Widget *
ewl_table_new_all(unsigned int homogeneous,
		  unsigned int columns,
		  unsigned int rows,
		  unsigned int col_spacing, unsigned int row_spacing)
{
	Ewl_Table *t;

	t = NEW(Ewl_Table, 1);

	__ewl_table_init(t);

	t->columns = columns;
	t->rows = rows;
	t->homogeneous = homogeneous;
	t->col_spacing = col_spacing;
	t->row_spacing = row_spacing;

	t->col_w = NEW(int, columns);
	memset(t->col_w, 0, columns * sizeof(int));

	t->custom_col_w = NEW(int, columns);
	memset(t->custom_col_w, 0, columns * sizeof(int));

	t->row_h = NEW(int, rows);
	memset(t->row_h, 0, rows * sizeof(int));

	return EWL_WIDGET(t);
}

void
ewl_table_attach(Ewl_Widget * t, Ewl_Widget * c, Ewl_Alignment alignment,
		 Ewl_Fill_Policy fill_policy, unsigned int start_col,
		 unsigned int end_col, unsigned int start_row,
		 unsigned int end_row)
{
	int layer;
	Ewl_Table_Child *child;

	CHECK_PARAM_POINTER("t", t);
	CHECK_PARAM_POINTER("c", c);

	child = NEW(Ewl_Table_Child, 1);
	memset(child, 0, sizeof(Ewl_Table_Child));

	child->widget = c;
	child->alignment = alignment;
	child->fill_policy = fill_policy;
	child->start_col = start_col - 1;
	child->end_col = end_col - 1;
	child->start_row = start_row - 1;
	child->end_row = end_row - 1;

	if (!EWL_CONTAINER(t)->children)
		EWL_CONTAINER(t)->children = ewd_list_new();

	c->parent = t;
	c->evas = t->evas;
	c->evas_window = t->evas_window;
	layer = ewl_object_get_layer(EWL_OBJECT(t));
	ewl_object_set_layer(EWL_OBJECT(t), layer + 1);

	ewd_list_append(EWL_CONTAINER(t)->children, child);
}

void
ewl_table_detach(Ewl_Widget * t, unsigned int c, unsigned int r)
{
	Ewl_Table_Child *child;

	CHECK_PARAM_POINTER("t", t);

	if (!EWL_CONTAINER(t)->children ||
	    ewd_list_is_empty(EWL_CONTAINER(t)->children))
		return;

	ewd_list_goto_first(EWL_CONTAINER(t)->children);

	while ((child = ewd_list_next(EWL_CONTAINER(t)->children)) != NULL)
	  {
		  if (child->start_col + 1 <= c &&
		      child->end_col + 1 >= c &&
		      child->start_row <= r && child->end_row >= r)
		    {
			    ewd_list_remove(EWL_CONTAINER(t)->children);
			    FREE(child);
			    break;
		    }
	  }
}

void
ewl_table_resize(Ewl_Widget * t, unsigned int c, unsigned int r)
{
	CHECK_PARAM_POINTER("t", t);


	EWL_TABLE(t)->columns = c;
	EWL_TABLE(t)->rows = r;

	REALLOC(EWL_TABLE(t)->custom_col_w, int, c);
}

unsigned int
ewl_table_get_columns(Ewl_Widget * t)
{
	CHECK_PARAM_POINTER_RETURN("t", t, 0);

	return EWL_TABLE(t)->columns;
}

unsigned int
ewl_table_get_rows(Ewl_Widget * t)
{
	CHECK_PARAM_POINTER_RETURN("t", t, 0);

	return EWL_TABLE(t)->rows;
}

void
ewl_table_set_homogeneous(Ewl_Widget * t, unsigned int h)
{
	CHECK_PARAM_POINTER("t", t);

	EWL_TABLE(t)->homogeneous = h;

	ewl_widget_configure(t);
}

void
ewl_table_set_col_spacing(Ewl_Widget * t, unsigned int cs)
{
	CHECK_PARAM_POINTER("t", t);

	EWL_TABLE(t)->col_spacing = cs;

	ewl_widget_configure(t);
}

void
ewl_table_set_row_spacing(Ewl_Widget * t, unsigned int rs)
{
	CHECK_PARAM_POINTER("t", t);

	EWL_TABLE(t)->row_spacing = rs;

	ewl_widget_configure(t);
}

void
ewl_table_set_alignment(Ewl_Widget * t, Ewl_Alignment a)
{
	CHECK_PARAM_POINTER("t", t);

	EWL_TABLE(t)->alignment = a;
}

/*
 * Set the width of the specified column
 */
void
ewl_table_column_set_width(Ewl_Widget * t, unsigned int c, unsigned int w)
{
	CHECK_PARAM_POINTER("t", t);

	EWL_TABLE(t)->custom_col_w[c - 1] = w;

	ewl_widget_configure(t);
}

/*
 * Get the width of the specified column
 */
void
ewl_table_get_column_width(Ewl_Widget * t, unsigned int c, unsigned int *w)
{
	CHECK_PARAM_POINTER("t", t);

	if (EWL_TABLE(t)->col_w[c - 1])
		*w = EWL_TABLE(t)->col_w[c - 1];
}

/*
 * Set the alignment of the child at the specified row and column
 */
void
ewl_table_child_set_alignment(Ewl_Widget * t,
			      unsigned int c, unsigned int r, Ewl_Alignment a)
{
	Ewl_Table_Child *child;

	CHECK_PARAM_POINTER("t", t);

	if (!EWL_CONTAINER(t)->children ||
	    ewd_list_is_empty(EWL_CONTAINER(t)->children))
		return;

	while ((child = ewd_list_next(EWL_CONTAINER(t)->children)) != NULL)
	  {
		  if (child->start_col + 1 <= c &&
		      child->end_col + 1 >= c &&
		      child->start_row <= r && child->end_row >= r)
		    {
			    child->alignment = a;
			    break;
		    }
	  }
}

/*
 * Determine the alignment of the child at the specified row and column
 */
Ewl_Alignment
ewl_table_child_get_alignment(Ewl_Widget * t, unsigned int c, unsigned int r)
{
	Ewl_Table_Child *child;

	CHECK_PARAM_POINTER_RETURN("t", t, FALSE);

	ewd_list_goto_first(EWL_CONTAINER(t)->children);

	while ((child = ewd_list_next(EWL_CONTAINER(t)->children)) != NULL)
	  {
		  if (child->start_col <= c &&
		      child->end_col >= c && child->start_row <= r &&
		      child->end_row >= r)
			  return child->alignment;
	  }

	return FALSE;
}

/*
 * Get the child at the specified row and column
 */
Ewl_Widget *
ewl_table_get_child(Ewl_Widget * t, unsigned int c, unsigned int r)
{
	Ewl_Table_Child *child;

	CHECK_PARAM_POINTER_RETURN("t", t, NULL);

	ewd_list_goto_first(EWL_CONTAINER(t)->children);

	while ((child = ewd_list_next(EWL_CONTAINER(t)->children)) != NULL)
	  {
		  if (child->start_col <= c &&
		      child->end_col >= c && child->start_row <= r &&
		      child->end_row >= r)
			  return child->widget;
	  }

	return NULL;
}

/*
 * Get the specified row's position and shape
 */
void
ewl_table_get_row_geometry(Ewl_Widget * t, unsigned int r, int *x, int *y,
			   int *w, int *h)
{
	CHECK_PARAM_POINTER("t", t);

	if (x);

	if (y);

	if (w);

	if (h);
}

/*
 * Initialize the tables fields
 */
static void
__ewl_table_init(Ewl_Table * t)
{
	CHECK_PARAM_POINTER("t", t);

	memset(t, 0, sizeof(Ewl_Table));
	ewl_container_init(EWL_CONTAINER(t), EWL_WIDGET_TABLE, 1, 1, 2048,
			   2048);

	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_REALIZE,
			    __ewl_table_realize, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_SHOW,
			    __ewl_table_show, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_HIDE,
			    __ewl_table_hide, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_DESTROY,
			    __ewl_table_destroy, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_DESTROY_RECURSIVE,
			    __ewl_table_destroy_recursive, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_CONFIGURE,
			    __ewl_table_configure, NULL);
	ewl_callback_append(EWL_WIDGET(t), EWL_CALLBACK_THEME_UPDATE,
			    __ewl_table_theme_update, NULL);

}

/*
 * Draw the table
 */
static void
__ewl_table_realize(Ewl_Widget * w, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("w", w);

	{
		Evas_Object *clip_box;

		clip_box = evas_add_rectangle(w->evas);
		evas_set_color(w->evas, clip_box, 255, 255, 255, 255);
		evas_set_layer(w->evas, clip_box, LAYER(w) - 1);
		if (w->parent && EWL_CONTAINER(w->parent)->clip_box)
			evas_set_clip(w->evas, clip_box,
				      EWL_CONTAINER(w->parent)->clip_box);
		w->fx_clip_box = clip_box;

	}

	{
		Evas_Object *clip_box;

		clip_box = evas_add_rectangle(w->evas);
		evas_set_color(w->evas, clip_box, 255, 255, 255, 255);
		evas_set_layer(w->evas, clip_box, LAYER(w));
		evas_set_clip(w->evas, clip_box, w->fx_clip_box);
		evas_show(w->evas, clip_box);

		EWL_CONTAINER(w)->clip_box = clip_box;
	}

	ewl_widget_theme_update(w);
}

/*
 * Display the table
 */
static void
__ewl_table_show(Ewl_Widget * w, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("w", w);

	evas_show(w->evas, w->fx_clip_box);
}

/*
 * Hide the table
 */
static void
__ewl_table_hide(Ewl_Widget * w, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("w", w);

	evas_hide(w->evas, w->fx_clip_box);
}

/*
 * Destroy the table, but don't destroy child widgets
 */
static void
__ewl_table_destroy(Ewl_Widget * w, void *event_data, void *user_data)
{
	DENTER_FUNCTION;
	CHECK_PARAM_POINTER("w", w);

	ebits_hide(EWL_TABLE(w)->ebits_bg);
	ebits_unset_clip(EWL_TABLE(w)->ebits_bg);
	ebits_free(EWL_TABLE(w)->ebits_bg);

	if (w->fx_clip_box)
	  {
		  evas_hide(w->evas, w->fx_clip_box);
		  evas_unset_clip(w->evas, w->fx_clip_box);
		  evas_del_object(w->evas, w->fx_clip_box);
	  }

	if (EWL_CONTAINER(w)->clip_box)
	  {
		  evas_hide(w->evas, EWL_CONTAINER(w)->clip_box);
		  evas_unset_clip(w->evas, EWL_CONTAINER(w)->clip_box);
		  evas_del_object(w->evas, EWL_CONTAINER(w)->clip_box);
	  }

	IF_FREE(EWL_TABLE(w)->col_w);
	IF_FREE(EWL_TABLE(w)->row_h);
	IF_FREE(EWL_TABLE(w)->custom_col_w);

	ewl_callback_del_all(w);

	ewl_theme_deinit_widget(w);

	FREE(w);

	DLEAVE_FUNCTION;
}

/*
 * Recursively destroy the table and it's children
 */
static void
__ewl_table_destroy_recursive(Ewl_Widget * w, void *event_data,
			      void *user_data)
{
	Ewl_Table_Child *child;

	DENTER_FUNCTION;
	CHECK_PARAM_POINTER("w", w);

	if (!EWL_CONTAINER(w)->children)
		DLEAVE_FUNCTION;

	while ((child =
		ewd_list_remove_last(EWL_CONTAINER(w)->children)) != NULL)
	  {
		  ewl_widget_destroy_recursive(child->widget);
		  FREE(child);
	  }

	DLEAVE_FUNCTION;
}

/*
 * This callback configures the table and it's child widgets
 */
static void
__ewl_table_configure(Ewl_Widget * w, void *event_data, void *user_data)
{
	__ewl_table_configure_children(w);
	__ewl_table_configure_gfx(w);
}

/*
 * Configure the graphics of the table
 */
void
__ewl_table_configure_gfx(Ewl_Widget * w)
{
	int t_req_x, t_req_y, t_req_w, t_req_h;

	CHECK_PARAM_POINTER("w", w);

	ewl_object_requested_geometry(EWL_OBJECT(w), &t_req_x, &t_req_y,
				      &t_req_w, &t_req_h);

	if (EWL_TABLE(w)->ebits_bg)
	  {
		  ebits_move(EWL_TABLE(w)->ebits_bg, t_req_x, t_req_y);
		  ebits_resize(EWL_TABLE(w)->ebits_bg, t_req_w, t_req_h);
	  }

	ewl_fx_clip_box_resize(w);
}

/*
 * Reassign sizes and positions to each of the child widgets
 */
void
__ewl_table_configure_children(Ewl_Widget * w)
{
	int rem_w, rem_h;
	Ewd_List *fillers;

	CHECK_PARAM_POINTER("w", w);

	/*
	 * Layout the normal children first, that returns a list of the filler
	 * children, which are then laid out.
	 */
	fillers = __ewl_table_fill_normal(w, &rem_w, &rem_h);
	if (fillers)
		__ewl_table_fill_fillers(w, rem_w, rem_h, fillers);

	__ewl_table_layout_children(EWL_TABLE(w));
}

/*
 * Place the child into the table and modify the remaining widths and heights
 * to indicate space used
 */
void
__ewl_table_normal_span(Ewl_Table * w, Ewl_Table_Child * child, int *rem_w,
			int *rem_h)
{
	int i;
	int c_span = 1;
	int r_span = 1;
	int req_w, req_h;
	int used_w = 0, used_h = 0;
	int x, y;

	ewl_object_requested_geometry(EWL_OBJECT
				      (child->widget),
				      &x, &y, &req_w, &req_h);
	used_w += req_w;
	used_h += req_h;

	c_span += child->start_col - child->end_col;

	/*
	 * Split up the size of the child between the col's
	 * it spans 
	 */
	for (i = child->start_col; i <= child->end_col; i++)
	  {
		  /*
		   * Only assign the column this width if another child doesn't
		   * need more.
		   */
		  if (req_w / c_span > w->col_w[i])
		    {
			    w->col_w[i] = req_w / c_span;
			    *rem_w -= (req_w / c_span) + w->col_spacing;
		    }
	  }

	/*
	 * The child may have been shorted some of the width it needs, if so
	 * tack it on the end
	 */
	if ((req_w % c_span) + (req_w / c_span) > w->col_w[i - 1])
	  {
		  w->col_w[i - 1] += req_w % c_span;
		  *rem_w -= req_w % c_span;
	  }

	r_span += child->start_row - child->end_row;

	/*
	 * Split up the size of the child between the row's it
	 * * spans 
	 */
	for (i = 0; i <= r_span; i++)
	  {
		  /*
		   * Only assign the row this height if another child doesn't
		   * need more.
		   */
		  if (req_h > w->row_h[i])
		    {
			    w->row_h[i] = req_h / r_span;
			    *rem_h -= (req_h / r_span) + w->row_spacing;
		    }
	  }

	/*
	 * The child has been shorted some of the height it needs, so tack it
	 * on the end.
	 */
	if ((req_w % r_span) + (req_h / r_span) > w->row_h[i - 1])
	  {
		  w->row_h[i - 1] += req_h % r_span;
		  *rem_h -= req_h % r_span;
	  }
}

/* Fill in the widgets that request their sizes normally and are not resized */
Ewd_List *
__ewl_table_fill_normal(Ewl_Widget * w, int *rem_w, int *rem_h)
{
	int x, y;
	Ewd_List *fillers = NULL;
	Ewl_Table_Child *child = NULL;

	CHECK_PARAM_POINTER_RETURN("w", w, 0);

	/*
	 * Grab the size so we know how much room we have to split the
	 * * children up into 
	 */
	ewl_object_requested_geometry(EWL_OBJECT(w), &x, &y, rem_w, rem_h);
	*rem_w -= EWL_TABLE(w)->columns * EWL_TABLE(w)->col_spacing;
	*rem_h -= EWL_TABLE(w)->rows * EWL_TABLE(w)->row_spacing;

	ewd_list_goto_first(EWL_CONTAINER(w)->children);

	/*
	 * Loop through and allocate the space for each normal child,
	 * * add any filler children to the list of fillers that will be
	 * * returned 
	 */
	while ((child = ewd_list_next(EWL_CONTAINER(w)->children)) != NULL)
	  {

		  /*
		   * Normal children are given their requested space, otherwise
		   * we add them to the list of children to assign the remaining
		   * space.
		   */
		  if (child->fill_policy == EWL_FILL_POLICY_NORMAL)
		    {
			    __ewl_table_normal_span(EWL_TABLE(w), child,
						    rem_w, rem_h);
		    }
		  else
		    {
			    if (!fillers)
				    fillers = ewd_list_new();
			    ewd_list_append(fillers, child);
		    }
	  }

	return fillers;
}

/* Fill in widgets that are stretched to fill the column they occupy */
void
__ewl_table_fill_fillers(Ewl_Widget * w, int rem_w, int rem_h, Ewd_List * l)
{
	int total_cols = 0, total_rows = 0;
	int num, r_size, c_size;
	Ewl_Table_Child *child;

	DENTER_FUNCTION;
	CHECK_PARAM_POINTER("w", w);
	CHECK_PARAM_POINTER("l", l);

	/*
	 * Determine the number of rows and columns remaining 
	 */
	num = ewd_list_nodes(l);
	ewd_list_goto_first(l);
	for (child = ewd_list_next(l); child; child = ewd_list_next(l))
	  {
		  total_cols += child->end_col - child->start_col + 1;
		  total_rows += child->end_row - child->start_row + 1;
	  }

	ewd_list_goto_first(l);
	for (child = ewd_list_next(l); child; child = ewd_list_next(l))
	  {
		  int i, x, y, cols, rows, req_w, req_h;

		  /*
		   * Determine the number of rows and columns this item uses 
		   */
		  cols = child->end_col - child->start_col + 1;
		  rows = child->end_row - child->start_row + 1;
		  ewl_object_requested_geometry(EWL_OBJECT(child->widget),
						&x, &y, &req_w, &req_h);

		  /*
		   * Now find the pixels each item will use 
		   */
		  r_size = rem_h * (float) rows / (float) total_rows;
		  c_size = rem_w * (float) cols / (float) total_rows;

		  /*
		   * Loop through and assign the allocation of each row and column
		   */
		  for (i = child->start_row; i < child->end_row; i++)
		    {
			    int test;

			    test = req_w / cols;
			    if (test > EWL_TABLE(w)->col_w[i])
				    EWL_TABLE(w)->col_w[i] = test;

			    test = req_h / rows;
			    if (test > EWL_TABLE(w)->row_h[i])
				    EWL_TABLE(w)->row_h[i] = test;
		    }
	  }

	DLEAVE_FUNCTION;
}

static void
__ewl_table_layout_children(Ewl_Table * w)
{
	Ewl_Table_Child *child;
	int i;
	int *x_offsets, *y_offsets;

	CHECK_PARAM_POINTER("w", w);

	/*
	 * Keep a temporary table of offsets so that we only have to compute
	 * the starting postition of each row/column once.
	 */
	x_offsets = (int *) malloc(w->rows * w->columns * sizeof(int));
	y_offsets = (int *) malloc(w->rows * w->columns * sizeof(int));

	/*
	 * Zero out the data in the location tables
	 */
	memset(x_offsets, 0, w->rows * w->columns * sizeof(int));
	memset(y_offsets, 0, w->rows * w->columns * sizeof(int));

	/*
	 * Now run through the table offsets and build up starting x and y
	 * positions for each column.
	 */
	for (i = 1, x_offsets[0] = CURRENT_X(w); i < w->rows; i++)
		x_offsets[i] = x_offsets[i - 1] + w->col_w[i - 1];

	for (i = 1, y_offsets[0] = CURRENT_Y(w); i < w->rows; i++)
		y_offsets[i] = y_offsets[i - 1] + w->row_h[i - 1];

	/*
	 * Loop through the children and determine their starting offsets
	 */
	ewd_list_goto_first(EWL_CONTAINER(w)->children);
	while ((child = ewd_list_next(EWL_CONTAINER(w)->children)))
	  {
		  /*
		   * Ok, now that we have offset tables put each child at its
		   * required offset.
		   */
		  ewl_object_set_current_geometry(EWL_OBJECT(child->widget),
						  x_offsets[child->
							    start_col - 1],
						  y_offsets[child->
							    start_row - 1],
						  CURRENT_W(child->widget),
						  CURRENT_H(child->widget));
		  printf("Placed child %p at (%d, %d) dim %dx%d\n",
			 child->widget, CURRENT_X(child->widget),
			 CURRENT_Y(child->widget), CURRENT_W(child->widget),
			 CURRENT_H(child->widget));
	  }
}


/*
 * Read in any changes to the theme and redraw any elements that are necessary
 */
static void
__ewl_table_theme_update(Ewl_Widget * w, void *event_data, void *user_data)
{
	Ewl_Table *b;
	char *i;
	char *v;

	DENTER_FUNCTION;

	DCHECK_PARAM_PTR("w", w);

	/*
	 * Shouldn't do anything if the widget isn't realized yet 
	 */
	if (!w->object.realized)
		DRETURN;

	b = EWL_TABLE(w);

	/*
	 * Check if GFX should be visible or not 
	 */
	v = ewl_theme_data_get(w, "/appearance/table/default/base/visible");

	/*
	 * Destroy old image (if any)
	 */
	if (w->ebits_object)
	  {
		  ebits_hide(w->ebits_object);
		  ebits_unset_clip(w->ebits_object);
		  ebits_free(w->ebits_object);
	  }

	/*
	 * If we don't want to show the new graphics, then we're pretty much
	 * finished. Lots of people say goto's are evile, but this is the type
	 * of case where they are probably a better choice than alternate
	 * approaches.
	 */
	if (!v || strncasecmp(v, "yes", 3) != 0)
		goto table_theme_update_end;

	/*
	 * Determine the path to the bit file we want.
	 */
	i = ewl_theme_image_get(w, "/appearance/table/default/base");

	if (!i)
		goto table_theme_update_end;

	/*
	 * Load our new theme bits
	 */
	w->ebits_object = ebits_load(i);
	FREE(i);

	if (w->ebits_object)
	  {
		  ebits_add_to_evas(w->ebits_object, w->evas);
		  ebits_set_layer(w->ebits_object, w->object.layer);
		  ebits_set_clip(w->ebits_object, w->fx_clip_box);

		  ebits_show(w->ebits_object);
	  }

      table_theme_update_end:
	/*
	 * We need to make sure v is not NULL when free()ing it.
	 */
	IF_FREE(v);

	/*
	 * Finally configure the widget to update changes 
	 */
	ewl_widget_configure(w);

}
