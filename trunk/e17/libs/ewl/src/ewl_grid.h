#ifndef __EWL_GRID_H__
#define __EWL_GRID_H__

typedef struct Ewl_Grid_Info Ewl_Grid_Info;

struct Ewl_Grid_Info
{
	int             override;
	int             size;
	Ewl_Widget     *max;
	Ewd_List       *cross;
};

typedef struct _ewl_grid Ewl_Grid;

#define EWL_GRID(grid) ((Ewl_Grid *)grid)

struct _ewl_grid {
	Ewl_Container   container;

	/*
	 * horisontal/vertical size of the columns and rows
	 */
	Ewl_Grid_Info  *col_size;
	Ewl_Grid_Info  *row_size;

	int             rows, cols;

	/*
	 * total size of the grid widget
	 */
	int             grid_h;
	int             grid_w;

	/*
	 * list of old children after a reset call
	 */
	Ewd_List       *rchildren;
};


typedef struct _ewl_grid_child Ewl_Grid_Child;
struct _ewl_grid_child
{
	int             start_col;
	int             start_row;

	int             end_col;
	int             end_row;
};

Ewl_Widget     *ewl_grid_new(int cols, int rows);
int             ewl_grid_init(Ewl_Grid * g, int cols, int rows);

void            ewl_grid_add(Ewl_Grid * g, Ewl_Widget * w,
			     int start_col, int end_col, int start_row,
			     int end_row);

void            ewl_grid_set_col_w(Ewl_Grid * g, int col, int width);
void            ewl_grid_set_row_h(Ewl_Grid * g, int row, int height);

void            ewl_grid_get_col_w(Ewl_Grid * g, int col, int *width);
void            ewl_grid_get_row_h(Ewl_Grid * g, int row, int *height);

void            ewl_grid_reset(Ewl_Grid * g, int rows, int cols);

/*
 * Internally used callbacks, override at your own risk.
 */
void ewl_grid_realize_cb(Ewl_Widget * w, void *ev_data,
			 void *user_data);
void ewl_grid_configure_cb(Ewl_Widget * w, void *ev_data,
			   void *user_data);
void ewl_grid_resize_cb(Ewl_Grid * g);
void ewl_grid_add_cb(Ewl_Container * p, Ewl_Widget * c);
void ewl_grid_auto_resize_cb(Ewl_Container * p, Ewl_Widget * child,
			     int size, Ewl_Orientation o);

#endif				/* __EWL_GRID_H__ */
