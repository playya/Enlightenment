
#ifndef __EWL_TABLE_H__
#define __EWL_TABLE_H__

typedef struct _ewl_table Ewl_Table;

#define EWL_TABLE(table) ((Ewl_Table *)table)
struct _ewl_table {
	Ewl_Container   container;

	Ewl_Grid       *grid;

	char          **col_headers;

	int             row_select;	/* boolean: select entire rows */

	struct {
		int             start_r;
		int             start_c;
		int             end_r;
		int             end_c;
	} selected;

};


typedef struct _ewl_table_child Ewl_Table_Child;

#define EWL_TABLE_CHILD(child) ((Ewl_Table_Child *)child)
struct _ewl_table_child {
	Ewl_Container   container;
	Ewl_Widget     *widget;
};

Ewl_Widget     *ewl_table_new(int cols, int rows, char **col_headers);
void            ewl_table_init(Ewl_Table * t, int cols, int rows,
			       char **col_headers);
void            ewl_table_add(Ewl_Table * table, char *text, int start_col,
			      int end_col, int start_row, int end_row);
Ewl_Widget     *ewl_table_add_return(Ewl_Table * table, char *text,
				     int start_col, int end_col, int start_row,
				     int end_row);

void            ewl_table_reset(Ewl_Table * t, int cols, int rows,
				char **c_headers);
void            ewl_table_row_select(Ewl_Table * t, int boolean);

void            ewl_table_set_col_w(Ewl_Table * table, int col, int width);
void            ewl_table_set_row_h(Ewl_Table * table, int row, int height);

void            ewl_table_get_col_w(Ewl_Table * table, int col, int *width);
void            ewl_table_get_row_h(Ewl_Table * table, int row, int *height);

void            ewl_table_get_col_row(Ewl_Table * table, char *text,
				      int *start_col, int *end_col,
				      int *start_row, int *end_row);

Ewd_List       *ewl_table_find(Ewl_Table * table,
			       int start_col,
			       int end_col, int start_row, int emd_row);


char           *ewl_table_get_selected(Ewl_Table * table);

#endif				/* __EWL_TABLE_H__ */
