#ifndef _ETOX_H
#define _ETOX_H

#include <Edb.h>
#include <Evas.h>
#include <Estyle.h>

/*
 * Simple alignment bitfield
 */
typedef enum _etox_alignment Etox_Alignment;
enum _etox_alignment
{
	ETOX_ALIGN_CENTER = 0,
	ETOX_ALIGN_LEFT = 1,
	ETOX_ALIGN_RIGHT = 2,
	ETOX_ALIGN_TOP = 4,
	ETOX_ALIGN_BOTTOM = 8
};

#define ETOX_ALIGN_MASK 0xF

/*
 * The color struct simply keeps track of the various colors available
 */
typedef struct _etox_color Etox_Color;
struct _etox_color
{
	int a, r, g, b;
};

/*
 * Text layout requires knowing the font layout, size, ascent and descent.
 */
typedef struct _etox_font Etox_Font;
struct _etox_font
{
	char *name;
	int size, ascent, descent;
};

/*
 * The info structure keeps the important information about the style, but not
 * the bits used to display the text.
 */
typedef struct _etox_style_info Etox_Style_Info;
struct _etox_style_info
{
	char *name;
	E_DB_File *style_db;
	int references;
};

/*
 * The context structure holds information relative to the appearance of text
 * that is added to the etox.
 */
typedef struct _etox_context Etox_Context;
struct _etox_context
{
	/*
	 * Color for displaying the text
	 */
	int r;
	int g;
	int b;
	int a;

	/*
	 * Font used for displaying the text
	 */
	char *font;

	/*
	 * The size of the font used for displaying the text
	 */
	int font_size;

	/*
	 * Style used for displaying the text
	 */
	char *style;

	/*
	 * Default alignment of text on etox
	 */
	char flags;

	/*
	 * Padding surrounding the text
	 */
	int padding;

	/*
	 * A marker for wrapped lines
	 */
	struct
	{
		char *text;
		char *style;
		int r, g, b, a;
	} marker;
};

/*
 * The etox keeps track of the display and layout information for all of the
 * text enclosed.
 */
typedef struct _etox Etox;
struct _etox
{
	/*
	 * Evas for drawing the text
	 */
	Evas *evas;

	/*
	 * Clip box on evas that bounds the text display and applies an alpha
	 * layer.
	 */
	Evas_Object *clip;

	/*
	 * The layer in the evas to set the text
	 */
	int layer;

	/*
	 * Geometry of the etox
	 */
	int x, y, w, h;

	/*
	 * Geometry the text prefers w/o wraps.
	 */
	int tw, th;

	/*
	 * The length text in the etox
	 */
	int length;

	/*
	 * The current context that is used when adding text
	 */
	Etox_Context *context;

	/*
	 * List of lines in the etox
	 */
	Evas_List *lines;

	/*
	 * List of obstacles in the etox
	 */
	Evas_List *obstacles;

	/*
	 * Determine if the etox has been displayed yet.
	 */
	char visible;

	/*
	 * Alpha level of clip box that is applied to the text
	 */
	int alpha;
};

/*
 * Line information helps process the bits layout
 */
typedef struct _etox_line Etox_Line;
struct _etox_line
{

	/*
	 * The etox containing this line, used for getting back to necessary
	 * etox info when drawing bits.
	 */
	Etox *et;

	/*
	 * This is a pointer to a list of bits
	 */
	Evas_List *bits;

	/*
	 * The dimensions of this line.
	 */
	int x, y, w, h;

	/*
	 * Flags indicating alignment, or if this is a "softline", ie. etox
	 * wrapped the line because it was too long to fit within the etox's
	 * bounds.
	 */
	char flags;

	/*
	 * Keep track of the length of text stored in this bit to avoid
	 * needing to recalculate this often.
	 */
	int length;
};

/*
 * Etox obstacles keep track of the lines that they intersect and the bit that
 * represents it.
 */
typedef struct _etox_obstacle Etox_Obstacle;
struct _etox_obstacle
{
	Etox *et;
	Estyle *bit;
	int start_line;
	int end_line;
};

/*
 * Selection are used to manipulate previously composed etox, it is
 * recommended to keep the number of active selections to a minimum, and if
 * possible, compose using contexts and setup time.
 */
typedef struct _etox_selection Etox_Selection;
struct _etox_selection
{
	Etox *etox;

	struct
	{
		Etox_Line *line;
		Estyle *bit;
	} start, end;

	Etox_Context *context;
};

/*
 * Etox creation and deletion functions
 */
Etox *etox_new(Evas *evas);
Etox *etox_new_all(Evas *evas, int x, int y, int w, int h, int alpha,
		   Etox_Alignment align);
void etox_free(Etox * et);

/*
 * Visibility altering functions
 */
void etox_show(Etox * et);
void etox_hide(Etox * et);

/*
 * Context management functions
 */
Etox_Context *etox_context_new();
Etox_Context *etox_context_save(Etox * et);
void etox_context_load(Etox * et, Etox_Context * context);
void etox_context_free(Etox_Context * context);

/*
 * Color context management functions
 */
void etox_context_get_color(Etox * et, int *r, int *g, int *b, int *a);
void etox_context_set_color(Etox * et, int r, int g, int b, int a);
void etox_context_set_color_db(Etox * et, char *name);

/*
 * Callback context management functions
 */
/*
int etox_context_clear_callbacks(Etox *et);
int etox_context_add_callback(Etox *et, int type, Etox_Cb_Func func, void *data);
int etox_context_del_callback(Etox *et, int index);
*/

/*
 * Font context managment functions
 */
char *etox_context_get_font(Etox * et, int *size);
void etox_context_set_font(Etox * et, char *fontname, int size);

/*
 * Style context management functions
 */
char *etox_context_get_style(Etox * et);
void etox_context_set_style(Etox * et, char *stylename);

/*
 * Alignment context management functions
 */
int etox_context_get_align(Etox * et);
void etox_context_set_align(Etox * et, int align);
void etox_context_set_soft_wrap(Etox * et, int boolean);

/* 
 * Wrap marker functions
 */
void etox_context_set_wrap_marker(Etox *et, char *marker, char *style);
void etox_context_set_wrap_marker_color(Etox *et, int r, int g, int b, int a);

/*
 * Text manipulation functions
 */
void etox_append_text(Etox * et, char *text);
void etox_prepend_text(Etox * et, char *text);
void etox_insert_text(Etox * et, char *text, int index);
void etox_set_text(Etox * et, char *text);
char *etox_get_text(Etox * et);
void etox_clear(Etox * et);

/*
 * Geometry altering functions
 */
void etox_move(Etox * et, int x, int y);
void etox_resize(Etox * et, int w, int h);

/*
 * Geometry retrieval functions
 */
void etox_get_geometry(Etox * et, int *x, int *y, int *w, int *h);
int etox_coord_to_index(Etox * et, int x, int y);
void etox_index_to_geometry(Etox * et, int index, int *x, int *y,
			    int *w, int *h);
int etox_coord_to_geometry(Etox * et, int xc, int yc, int *x, int *y,
			   int *w, int *h);

/*
 * Appearance altering functions
 */
void etox_set_layer(Etox * et, int layer);
void etox_set_clip(Etox * et, Evas_Object *clip);
void etox_set_alpha(Etox * et, int alpha);

/*
 * Region selection and release
 */
Evas_List *etox_region_select(Etox * et, int start, int end);
Evas_List *etox_region_select_str(Etox * et, char *search, char *last);
void etox_region_release(Evas_List *region);

/*
 * Region altering appearance modifiers
 */
void etox_region_set_font(Evas_List *region, char *name, int size);
void etox_region_set_color(Evas_List *region, int r, int g, int b, int a);
void etox_region_set_style(Evas_List *region, char *stylename);

/*
 * Obstacle manipulation functions
 */
Etox_Obstacle *etox_obstacle_add(Etox * et, int x, int y, int w, int h);
void etox_obstacle_remove(Etox_Obstacle * obstacle);
void etox_obstacle_move(Etox_Obstacle * obstacle, int x, int y);
void etox_obstacle_resize(Etox_Obstacle * obstacle, int w, int h);

/*
 * These functions select regions of the etox.
 */
Etox_Selection *etox_select_coords(Etox * et, int sx, int sy, int ex,
				   int ey);
Etox_Selection *etox_select_index(Etox * et, int si, int ei);
Etox_Selection *etox_select_str(Etox * et, char *match, char **last);

/*
 * Release a selection that is no longer needed.
 */
void etox_selection_free(Etox_Selection * selected);
void etox_selection_free_by_etox(Etox *etox);

/*
 * This function gets a rectangular bound on the selection.
 */
void etox_selection_bounds(Etox_Selection * selected, int *x, int *y,
			   int *w, int *h);

/*
 * These methods alter the appearance of the selected region.
 */
void etox_selection_set_font(Etox_Selection * selected, char *font,
			     int size);
void etox_selection_set_style(Etox_Selection * selected, char *style);
void etox_selection_set_color(Etox_Selection *selected, int r, int g, int b,
		int a);
void etox_selection_set_wrap_marker(Etox_Selection *selected, char *marker,
		char *style);
void etox_selection_set_wrap_marker_color(Etox_Selection *selected, int r,
		int g, int b, int a);

/*
 * These functions manipulate callbacks on the selected region.
 */
void etox_selection_add_callback(Etox_Selection * selected,
				 Evas_Callback_Type callback,
				 void (*func) (void *data, Evas *e,
					       Evas_Object *o, int b, int x,
					       int y), void *data);
void etox_selection_del_callback(Etox_Selection * selected,
				 Evas_Callback_Type callback);

#endif
