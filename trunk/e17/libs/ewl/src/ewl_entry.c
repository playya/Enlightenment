#include <Ewl.h>

static int ewl_entry_timer();

/*
 * Private functions for applying operations to the text at realize time.
 */
static void ewl_entry_ops_apply(Ewl_Entry *e);
static void ewl_entry_ops_reset(Ewl_Entry *e);
static void ewl_entry_op_prune_list(Ewl_Entry *e, int rstart, int rend, 
				    int bstart, int bend);
static void ewl_entry_op_free(void *data);

static Ewl_Entry_Op *ewl_entry_op_relevant_find(Ewl_Entry *e,
					        Ewl_Entry_Op_Type type);
static Ewl_Entry_Op *ewl_entry_op_color_new(Ewl_Entry *e, int r, int g, int b,
					    int a);
static void ewl_entry_op_color_apply(Ewl_Entry *e, Ewl_Entry_Op *op);

static Ewl_Entry_Op *ewl_entry_op_font_new(Ewl_Entry *e, char *font, int size);
static void ewl_entry_op_font_apply(Ewl_Entry *e, Ewl_Entry_Op *op);
static void ewl_entry_op_font_free(void *op);

static Ewl_Entry_Op *ewl_entry_op_style_new(Ewl_Entry *e, char *style);
static void ewl_entry_op_style_apply(Ewl_Entry *e, Ewl_Entry_Op *op);
static void ewl_entry_op_style_free(void *op);

static Ewl_Entry_Op *ewl_entry_op_align_new(Ewl_Entry *e, unsigned int align);
static void ewl_entry_op_align_apply(Ewl_Entry *e, Ewl_Entry_Op *op);

static Ewl_Entry_Op * ewl_entry_op_text_set_new(Ewl_Entry *e, char *text);
static Ewl_Entry_Op *ewl_entry_op_text_append_new(Ewl_Entry *e, char *text);
static Ewl_Entry_Op *ewl_entry_op_text_prepend_new(Ewl_Entry *e, char *text);
static Ewl_Entry_Op *ewl_entry_op_text_insert_new(Ewl_Entry *e, char *text,
						  int index);
static Ewl_Entry_Op *ewl_entry_op_text_delete_new(Ewl_Entry *e, 
						  unsigned int start,
						  unsigned int len);
static void ewl_entry_op_text_apply(Ewl_Entry *e, Ewl_Entry_Op *op);
static void ewl_entry_op_text_free(void *op);

static void ewl_entry_update_size(Ewl_Entry * e);

/**
 * @param text: the initial text to display in the widget
 * @return Returns a new entry widget on success, NULL on failure.
 * @brief Allocate and initialize a new multiline input entry widget
 */
Ewl_Widget *ewl_entry_multiline_new(char *text)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);

	w = ewl_entry_new(text);
	if (!w)
		return NULL;

	ewl_entry_multiline_set( EWL_ENTRY(w), TRUE );

	DRETURN_PTR(w, DLEVEL_STABLE);
}

/**
 * @param text: the initial text to display in the widget
 * @return Returns a new entry widget on success, NULL on failure.
 * @brief Allocate and initialize a new entry widget
 */
Ewl_Widget *ewl_entry_new(char *text)
{
	Ewl_Entry      *e;

	DENTER_FUNCTION(DLEVEL_STABLE);

	e = NEW(Ewl_Entry, 1);
	if (!e)
		return NULL;

	ewl_entry_init(e, text);

	DRETURN_PTR(EWL_WIDGET(e), DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to initialize
 * @param text: the initial text to display in the widget
 * @return Returns TRUE on success, FALSE on failure.
 * @brief Initialize an entry widget to default values
 *
 * Initializes the entry widget @a e to it's default values and callbacks.
 */
int ewl_entry_init(Ewl_Entry * e, char *text)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, FALSE);

	w = EWL_WIDGET(e);

	if (!ewl_container_init(EWL_CONTAINER(w), "entry"))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_widget_inherit(EWL_WIDGET(w), "entry");

	e->in_select_mode = FALSE;
	e->multiline = FALSE;

	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_HSHRINK |
				   EWL_FLAG_FILL_HFILL);
	ewl_container_callback_intercept(EWL_CONTAINER(w), EWL_CALLBACK_SELECT);
	ewl_container_callback_intercept(EWL_CONTAINER(w),
					 EWL_CALLBACK_DESELECT);

	e->ops = ecore_dlist_new();
	e->applied = ecore_dlist_new();

	ecore_dlist_set_free_cb(e->ops, ewl_entry_op_free);
	ecore_dlist_set_free_cb(e->applied, ewl_entry_op_free);

	if (text)
		ewl_entry_text_set(e, text);

	/*
	 * setup the cursor 
	 */
	e->cursor = ewl_entry_cursor_new();
	ewl_container_child_append(EWL_CONTAINER(e), e->cursor);
	ewl_widget_internal_set(e->cursor, TRUE);
	ewl_object_fill_policy_set(EWL_OBJECT(e->cursor), EWL_FLAG_FILL_SHRINK);
	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), 0);

	/*
	 * Attach necessary callback mechanisms 
	 */
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, ewl_entry_configure_cb,
			    NULL);

	ewl_callback_append(w, EWL_CALLBACK_SELECT, ewl_entry_select_cb, NULL);
	ewl_callback_append(w, EWL_CALLBACK_DESELECT, ewl_entry_deselect_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_REALIZE, ewl_entry_realize_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_UNREALIZE, ewl_entry_unrealize_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_SHOW, ewl_entry_show_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_HIDE, ewl_entry_hide_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_DESTROY, ewl_entry_destroy_cb,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_REPARENT, ewl_entry_reparent_cb,
			    NULL);

	ewl_entry_editable_set(e, TRUE);

	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
			    ewl_entry_mouse_down_cb, NULL);
	ewl_callback_del(w, EWL_CALLBACK_MOUSE_UP, ewl_entry_mouse_up_cb);
	/*
	ewl_callback_append(w, EWL_CALLBACK_DOUBLE_CLICKED,
					ewl_entry_mouse_double_click_cb, NULL);
					*/
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE,
					ewl_entry_mouse_move_cb, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to set multiline
 * @param m: the value to set multiline to
 * @return Returns no value.
 * @brief Set multiline for an entry widget
 *
 * Set the multiline flag for $a e to @a m
 */
void ewl_entry_multiline_set(Ewl_Entry * e, int m)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	e->multiline = m;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to retrieve the multiline flag
 * @return Returns the multiline flag on success, -1 on failure.
 * @brief Get the text from an entry widget
 */
int ewl_entry_multiline_get(Ewl_Entry * e)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, -1);

	DRETURN_INT(e->multiline, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to change the text
 * @param text: the text to set for the entry widget
 * @return Returns no value.
 * @brief Set the text for an entry widget
 *
 * Change the text of the entry widget @a e to the string @a t.
 */
void ewl_entry_text_set(Ewl_Entry * e, char *text)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	op = ewl_entry_op_text_set_new(e, text);
	ewl_entry_op_prune_list(e, EWL_ENTRY_OP_TYPE_TEXT_SET,
			EWL_ENTRY_OP_TYPE_TEXT_DELETE, -1, -1);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	IF_FREE(e->text);
	if (text) {
		e->text = strdup(text);
		e->length = strlen(text);
	} else
		e->length = 0;

	if (e->cursor)
		ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), 0);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to retrieve the text
 * @return Returns the entry text on success, NULL on failure.
 * @brief Get the text from an entry widget
 */
char *ewl_entry_text_get(Ewl_Entry * e)
{
	char *txt = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, NULL);

	if (e->etox)
		txt = etox_get_text(e->etox);
	else if (e->text)
		txt = strdup(e->text);

	DRETURN_PTR(txt, DLEVEL_STABLE);
}

/**
 * @param e: the entrywidget to append the text
 * @param text: the text to append in the entry widget @a e
 * @return Returns no value.
 * @brief Append text to a entry widget
 *
 * Appends text to the entry widget @a e.
 */
void ewl_entry_text_append(Ewl_Entry * e, char *text)
{
	int len = 0;
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);
	DCHECK_PARAM_PTR("text", text);

	if (e->text) {
		len = strlen(e->text) + strlen(text);
		e->text = realloc(e->text, sizeof(char) * (len + 1));
		strcat(e->text, text);

	} else {
		e->text = strdup(text);
		len = strlen(text);
	}

	e->length = len;

	op = ewl_entry_op_text_append_new(e, text);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to prepend the text
 * @param text: the text to prepend in the entry widget @a e
 * @return Returns no value.
 * @brief Append text to a entry widget
 *
 * Appends text to the entry widget @a e.
 */
void ewl_entry_text_prepend(Ewl_Entry * e, char *text)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);
	DCHECK_PARAM_PTR("text", text);

	op = ewl_entry_op_text_prepend_new(e, text);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}   

/**
 * @param e: the entry widget to insert the text
 * @param text: the text to insert in the entry widget @a e 
 * @param index: the index into the text to start inserting new text
 * @return Returns no value.
 * @brief Append text to a entry widget
 *
 * Appends text to the entry widget @a e.
 */
void ewl_entry_text_insert(Ewl_Entry * e, char *text, int index)
{
	Ewl_Entry_Op *op;
	int len = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);
	DCHECK_PARAM_PTR("text", text);

	op = ewl_entry_op_text_insert_new(e, text, index);
	len = strlen(text);
	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor),
                                      index + len);
	e->length += len;

	ecore_dlist_prepend(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to insert the text
 * @param text: the text to insert in the entry widget @a e 
 * @return Returns no value.
 * @brief Inserts text at cursor position
 *
 * Inserts text to the entry widget @a e at the current cursor position.
 */
void
ewl_entry_text_at_cursor_insert(Ewl_Entry * e, char *text)
{
	int pos = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);
	DCHECK_PARAM_PTR("text", text);

	pos = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));
	ewl_entry_text_insert(e, text, pos);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: then entry to change
 * @param edit: a boolean value indicating the ability to edit the entry
 * @return Returns no value.
 * @brief Change the ability to edit the text in an entry
 */
void
ewl_entry_editable_set(Ewl_Entry *e, unsigned int edit)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (e->editable == edit) {
		DRETURN(DLEVEL_STABLE);
	}

	w = EWL_WIDGET(e);

	e->editable = edit;

	if (edit) {
		ewl_callback_append(w, EWL_CALLBACK_KEY_DOWN,
				ewl_entry_key_down_cb, NULL);
	}
	else {
		ewl_callback_del(w, EWL_CALLBACK_KEY_DOWN,
				ewl_entry_key_down_cb);

		ewl_widget_hide(e->cursor);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry to retrieve length
 * @return Returns the length of the text contained in the widget.
 * @brief Retrieve the length of the text displayed by the entry widget.
 */
int ewl_entry_length_get(Ewl_Entry *e)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, 0);

	DRETURN_INT(e->length, DLEVEL_STABLE);
}

/**
 * @param e: the entry to change color
 * @param r: the new red value
 * @param g: the new green value
 * @param b: the new blue value
 * @param a: the new alpha value
 * @brief Changes the currently applied color of the text to specified values
 * @return Returns no value.
 */
void ewl_entry_color_set(Ewl_Entry *e, int r, int g, int b, int a)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	op = ewl_entry_op_color_new(e, r, g, b, a);
	/*
	 * Remove all color sets prior to a text addition/set operation.
	 */
	ewl_entry_op_prune_list(e, EWL_ENTRY_OP_TYPE_COLOR_SET,
				   EWL_ENTRY_OP_TYPE_COLOR_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_INSERT);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to change font
 * @param font: the name of the font
 * @param size: the size of the font
 * @brief Changes the currently applied font of the text to specified values
 * @return Returns no value.
 */
void ewl_entry_font_set(Ewl_Entry *e, char *font, int size)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	op = ewl_entry_op_font_new(e, font, size);
	ewl_entry_op_prune_list(e, EWL_ENTRY_OP_TYPE_FONT_SET,
				   EWL_ENTRY_OP_TYPE_FONT_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_INSERT);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to retrieve the current font
 * @brief Retrieve the name of the currently used font.
 * @return Returns a copied string containing the name of the current font.
 */
char *ewl_entry_font_get(Ewl_Entry *e)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Font *opf;
	char *font = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, NULL);

	if (REALIZED(e)) {
		int size;
		font = etox_context_get_font(etox_get_context(e->etox), &size);
	}
	else {
		op = ewl_entry_op_relevant_find(e, EWL_ENTRY_OP_TYPE_FONT_SET);
		opf = (Ewl_Entry_Op_Font *)op;
		if (opf && opf->font)
			font = strdup(opf->font);
	}

	DRETURN_PTR(font, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to retrieve the current font size
 * @brief Retrieve the size of the currently used font.
 * @return Returns the currently used size of the font.
 */
int ewl_entry_font_size_get(Ewl_Entry *e)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Font *opf;
	int size = 1;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, 1);

	if (REALIZED(e)) {
		char *font = NULL;
		font = etox_context_get_font(etox_get_context(e->etox), &size);
		if (font)
			FREE(font);
	}
	else {
		op = ewl_entry_op_relevant_find(e, EWL_ENTRY_OP_TYPE_FONT_SET);
		opf = (Ewl_Entry_Op_Font *)op;
		if (opf)
			size = opf->size;
	}

	DRETURN_INT(size, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to change style
 * @param style: the name of the style
 * @brief Changes the currently applied style of the text to specified values
 * @return Returns no value.
 */
void ewl_entry_style_set(Ewl_Entry *e, char *style)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	op = ewl_entry_op_style_new(e, style);
	ewl_entry_op_prune_list(e, EWL_ENTRY_OP_TYPE_STYLE_SET,
				   EWL_ENTRY_OP_TYPE_STYLE_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_INSERT);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to get the current style
 * @brief Retrieves the currently used text style from a text widget.
 * @return Returns the currently used text style.
 */
char *ewl_entry_style_get(Ewl_Entry *e)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Style *ops;
	char *style = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, NULL);

	op = ewl_entry_op_relevant_find(e, EWL_ENTRY_OP_TYPE_FONT_SET);
	ops = (Ewl_Entry_Op_Style *)op;
	if (ops && ops->style) {
		style = strdup(ops->style);
	}

	DRETURN_PTR(style, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to change alignment
 * @param align: the new alignment of the entry widget
 * @brief Changes the currently applied alignment of the text to specified value
 * @return Returns no value.
 */
void ewl_entry_align_set(Ewl_Entry *e, unsigned int align)
{
	Ewl_Entry_Op *op;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	op = ewl_entry_op_align_new(e, align);
	ewl_entry_op_prune_list(e, EWL_ENTRY_OP_TYPE_ALIGN_SET,
				   EWL_ENTRY_OP_TYPE_ALIGN_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_SET,
				   EWL_ENTRY_OP_TYPE_TEXT_INSERT);
	ecore_dlist_append(e->ops, op);
	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to get the current alignment
 * @brief Retrieves the currently used text alignment from an entry widget.
 * @return Returns the currently used text alignment.
 */
unsigned int ewl_entry_align_get(Ewl_Entry *e)
{
	unsigned int align = 0;
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Align *opa;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, 0);

	op = ewl_entry_op_relevant_find(e, EWL_ENTRY_OP_TYPE_FONT_SET);
	opa = (Ewl_Entry_Op_Align *)op;
	if (opa) {
		align = opa->align;
	}

	DRETURN_INT(align, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to change wrapping
 * @param wrap: a boolean that indicates wrapping
 * @brief Changes whether the entry wraps or clips to it's available area
 * @return Returns no value.
 *
 * Changes 
 */
void ewl_entry_wrap_set(Ewl_Entry *e, int wrap)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	/*
	 * If the wrapping changes notify the parent to configure the entry.
	 */
	if (wrap != e->wrap) {
		e->wrap = wrap;
		if (EWL_WIDGET(e)->parent)
			ewl_widget_configure(EWL_WIDGET(e)->parent);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to retrieve wrapping
 * @brief Retrieves the current wrapping of the entry
 * @return Returns TRUE if wrapping is enabled, otherwise FALSE.
 */
int ewl_entry_wrap_get(Ewl_Entry *e)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DRETURN_INT(e->wrap, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to map a coordinate to a character index
 * @param x: the x coordinate over the desired character
 * @param y: the y coordinate over the desired character
 * @brief Finds the index of the character under the specified coordinates
 * @return Returns the index of the found character on success, 0 otherwise.
 */
int ewl_entry_coord_index_map(Ewl_Entry *e, int x, int y)
{
	int index;
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("e", e, 0);

	if (!e->etox)
		DRETURN_INT(0, DLEVEL_STABLE);

	index = etox_coord_to_index(e->etox, (Evas_Coord)(x), (Evas_Coord)(y));
	DRETURN_INT(index, DLEVEL_STABLE);
}

/**
 * @param e: the entry widget to map index to character geometry
 * @param index: character index to be mapped
 * @param x: pointer to store determined character x coordinate
 * @param y: pointer to store determined character y coordinate
 * @param w: pointer to store determined character width
 * @param h: pointer to store determined character height
 * @return Returns no value.
 * @brief Maps a character index to a set of coordinates and sizes.
 *
 * Any of the coordinate parameters may be NULL, they will be ignored. If the
 * index fails to map successfully, the values at the locations pointed to by
 * the coordinate pointers will not be altered. This function can only succeed
 * after the entry widget has been realized.
 */
void ewl_entry_index_geometry_map(Ewl_Entry *e, int index, int *x, int *y,
				  int *w, int *h)
{
	Evas_Coord tx, ty, tw, th;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (!e->etox)
		DRETURN(DLEVEL_STABLE);

	etox_index_to_geometry(e->etox, index, &tx, &ty, &tw, &th);
	if (x)
		*x = (int)(tx);
	if (y)
		*y = (int)(ty);
	if (w)
		*w = (int)(tw);
	if (h)
		*h = (int)(th);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Layout the text and cursor within the entry widget.
 */
void ewl_entry_configure_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry    *e;
	int           xx, yy, ww, hh;
	int	      c_pos = 0, pos, l;
	int           cx = 0, cy = 0;
	unsigned int  cw = 0, ch = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	e = EWL_ENTRY(w);

	/*
	 * The contents are clipped starting at these positions
	 */
	xx = CURRENT_X(w);
	yy = CURRENT_Y(w);
	ww = CURRENT_W(w);
	hh = CURRENT_H(w);

	/*
	 * Update the etox position and size.
	 */
	if (e->etox) {
		etox_set_word_wrap(e->etox, e->wrap);
		evas_object_move(e->etox, xx, yy);
		evas_object_resize(e->etox, ww, hh);
		evas_object_layer_set(e->etox, ewl_widget_layer_sum_get(w));
	}

	if (!e->editable) {
		DLEAVE_FUNCTION(DLEVEL_STABLE);
	}

	l = ewl_entry_length_get(e);
	c_pos = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

	if (c_pos >= l)
		pos = l;
	else
		pos = c_pos;

	if (l)
		ewl_entry_index_geometry_map(e, pos, &cx, &cy, &cw, &ch);
	else
		ewl_object_current_geometry_get(EWL_OBJECT(w), &cx, &cy, &cw,
						&ch);

	if (e->offset < 0)
		e->offset = 0;

	if (!cw)
		cw = CURRENT_W(e->cursor);

	if (!ch)
		ch = CURRENT_H(e->cursor);

	/* printf("Map %d(%d) of %d to %d, %d: %d x %d\n", pos, c_pos, l, cx, cy, cw, ch); */
	ewl_object_geometry_request(EWL_OBJECT(e->cursor), cx, cy, 
				    cw, ch);
}

void ewl_entry_realize_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	char      *tmp;
	Ewl_Entry  *e;
	Ewl_Embed *emb;
	int r, g, b, a;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);

	/*
	 * Find the embed so we know which evas to draw onto.
	 */
	emb = ewl_embed_widget_find(w);
	if (!emb)
		DRETURN(DLEVEL_STABLE);

	/*
	 * Create the etox and save the context for future reference.
	 */
	e->etox = etox_new(emb->evas);
	e->context = etox_get_context(e->etox);

	/*
	 * Apply the default theme information to the context, this
	 * information may be altered programmatically through the operation
	 * queues.
	 */
	tmp = ewl_theme_data_str_get(w, "font");
	etox_context_set_font(e->context, tmp,
			      ewl_theme_data_int_get(w, "font_size"));
	IF_FREE(tmp);

	tmp = ewl_theme_data_str_get(w, "style");
	etox_context_set_style(e->context, tmp);
	IF_FREE(tmp);

	r = ewl_theme_data_int_get(w, "color/r");
	g = ewl_theme_data_int_get(w, "color/g");
	b = ewl_theme_data_int_get(w, "color/b");
	a = ewl_theme_data_int_get(w, "color/a");
	etox_context_set_color(e->context, r, g, b, a);

	if (w->fx_clip_box)
		evas_object_clip_set(e->etox, w->fx_clip_box);

	ewl_entry_ops_apply(e);

	/*
	 * Update the size of the entry
	 */
	ewl_entry_update_size(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_unrealize_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry   *e;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);

	/*
	 * Reset the text to initial state in order to recreate the operations
	 * if it is re-realized.
	 */
	ewl_entry_ops_reset(e);
	evas_object_clip_unset(e->etox);
	evas_object_del(e->etox);
	e->etox = NULL;
	e->context = NULL;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_show_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewl_Entry   *e;
	DENTER_FUNCTION(DLEVEL_STABLE);

	e = EWL_ENTRY(w);

	evas_object_show(e->etox);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_hide_cb(Ewl_Widget *w, void *ev_data, void *user_data)
{
	Ewl_Entry   *e;
	DENTER_FUNCTION(DLEVEL_STABLE);

	e = EWL_ENTRY(w);

	evas_object_hide(e->etox);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_destroy_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry   *e;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);
	ecore_dlist_destroy(e->ops);
	e->ops = NULL;

	ecore_dlist_destroy(e->applied);
	e->applied = NULL;

	/*
	 * Finished with the etox, so now would be a good time to cleanup
	 * extra resources hanging around. FIXME: Should be called at some
	 * regular interval too, in case the text changes, but is never freed.
	 */
	etox_gc_collect();

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_reparent_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry *e;

	DENTER_FUNCTION(DLEVEL_STABLE);

	e = EWL_ENTRY(w);
	if (e->etox)
		evas_object_layer_set(e->etox, ewl_widget_layer_sum_get(w));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Handle key events to modify the text of the entry widget.
 */
void ewl_entry_key_down_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry *e;
	char *evd = NULL;
	Ewl_Event_Key_Down *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);
	ev = ev_data;

	e->in_select_mode = (ev->modifiers & EWL_KEY_MODIFIER_SHIFT);

	if (!strcmp(ev->keyname, "Left"))
		ewl_entry_cursor_left_move(e);
	else if (!strcmp(ev->keyname, "Right"))
		ewl_entry_cursor_right_move(e);
	else if (!strcmp(ev->keyname, "Down"))
		ewl_entry_cursor_down_move(e);
	else if (!strcmp(ev->keyname, "Up"))
		ewl_entry_cursor_up_move(e);
	else if (!strcmp(ev->keyname, "Home"))
		ewl_entry_cursor_home_move(e);
	else if (!strcmp(ev->keyname, "End"))
		ewl_entry_cursor_end_move(e);
	else if (!strcmp(ev->keyname, "BackSpace"))
		ewl_entry_left_delete(e);
	else if (!strcmp(ev->keyname, "Delete"))
		ewl_entry_right_delete(e);
	else if (((!strcmp(ev->keyname, "w")) && 
		  (ev->modifiers & EWL_KEY_MODIFIER_CTRL)) ||
		 ((!strcmp(ev->keyname, "W")) && 
		  (ev->modifiers & EWL_KEY_MODIFIER_CTRL)))
		ewl_entry_word_begin_delete(e);
	else if (!strcmp(ev->keyname, "Return") || !strcmp(ev->keyname,
				"KP_Return") || !strcmp(ev->keyname, "Enter")
				|| !strcmp(ev->keyname, "KP_Enter")) {
		if (!e->multiline) {
			evd = ewl_entry_text_get(e);
			ewl_callback_call_with_event_data(w, EWL_CALLBACK_VALUE_CHANGED,
					evd);
			FREE(evd);
		} else {
			ewl_entry_text_insert(e, "\n",
				ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor)));
		}
	}
	else if (ev->keyname && strlen(ev->keyname) == 1) {
		char *tmp = (char *)calloc(2, sizeof(char));
		snprintf(tmp, 2, "%s", ev->keyname);

		ewl_entry_text_insert(e, tmp,
			ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor)));
		free(tmp);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Place the cursor appropriately on a mouse down event.
 */
void ewl_entry_mouse_down_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Event_Mouse_Down *ev;
	Ewl_Entry      *e;
	int             index = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ev = ev_data;
	e = EWL_ENTRY(w);

	e->in_select_mode = TRUE;

	index = ewl_entry_coord_index_map(e, ev->x, ev->y);
	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), index);

	if (ev->modifiers & EWL_KEY_MODIFIER_SHIFT) {

	}
	else {
		ewl_container_reset(EWL_CONTAINER(e));
	}

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Stop the scrolling timer.
 */
void ewl_entry_mouse_up_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Event_Mouse_Down *ev = ev_data;
	Ewl_Entry *e = EWL_ENTRY(w);

	DENTER_FUNCTION(DLEVEL_STABLE);

	/*
	if (e->timer) {
		ecore_timer_del(e->timer);
		e->timer = NULL;
		e->start_time = 0.0;
	}
	*/

	e->in_select_mode = (ev->modifiers & EWL_KEY_MODIFIER_SHIFT);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Hilight text when the mouse moves when the button is pressed
 */
void ewl_entry_mouse_move_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	int             index = 0;
	Ewl_Event_Mouse_Move *ev;
	Ewl_Entry      *e;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	ev = ev_data;
	e = EWL_ENTRY(w);

	/*
	 * Check for the button pressed state, otherwise, do nothing.
	 */
	if (!(ewl_object_state_has(EWL_OBJECT(e), EWL_FLAG_STATE_PRESSED)) || 
			!(e->in_select_mode))
		DRETURN(DLEVEL_STABLE);

	index = ewl_entry_coord_index_map(e, ev->x, ev->y);

	/*
	 * Should begin scrolling in either direction?
	 */
	if (ev->x > CURRENT_X(e) || ev->x < CURRENT_X(e)) {
		/*
		e->start_time = ecore_time_get();
		e->timer = ecore_timer_add(0.02, ewl_entry_timer, e);
		*/
	}

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_entry_mouse_double_click_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Event_Mouse_Down *ev;
	Ewl_Entry            *e;
	int                   len = 0;
  
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
  
	ev = ev_data;
	e = EWL_ENTRY(w);
  
	len = ewl_entry_length_get(e);

	if (ev->clicks == 2) {
		char   *s;
		int 	start, end;

		start = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

		s = ewl_entry_text_get(e);

		while ((s[start] != ' ') && (s[start] != '\t')
				&& (s[start] != '\n') && (--start > 0));

		if (start < 0) start++;
		end = start;

		while ((s[end + 1] != ' ') && (s[end + 1] != '\t')
				&& (s[end + 1] != '\n') && (++end < len));

		ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), end);

	}
	else {
		ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), len);
	}

	e->in_select_mode = (ev->modifiers & EWL_KEY_MODIFIER_SHIFT);
	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_select_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry      *e;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);

	if (e->cursor && e->editable && !VISIBLE(e->cursor))
		ewl_widget_show(e->cursor);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_deselect_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Entry      *e;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	e = EWL_ENTRY(w);

	if (e->cursor && e->editable && VISIBLE(e->cursor))
		ewl_widget_hide(e->cursor);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_cursor_left_move(Ewl_Entry * e)
{
	int             pos;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	pos = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

	if (--pos < 0) pos = 0;

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), pos);

	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_cursor_right_move(Ewl_Entry * e)
{
	char           *str;
	int             len = 0;
	int             pos;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	pos = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));
	str = ewl_entry_text_get(e);

	len = strlen(str);
	FREE(str);

	if (++pos > len) pos = len;

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), pos);

	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Position the cursor at the current position in the next line.
 */
void ewl_entry_cursor_down_move(Ewl_Entry * e)
{
	char *s;
	int nlen, nline, nend;
	int lpos, start, len = 0;
	int bp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (e->multiline) {
		len = ewl_entry_length_get(e);
		nline = start = bp =
			ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

		s = ewl_entry_text_get(e);

		while (--start > 1 && s[start] != '\n');

		if( s[start] == '\n' )
			start++;

		lpos = bp - start - 1;

		while (nline < len && s[nline++] != '\n');

		nend = nline;

		while (nend < len && s[nend++] != '\n');

		nlen = nend - nline;

		if( nlen >= lpos )
			lpos += nline + 1;
		else
			lpos = nend - 1;

		ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor),
					      lpos);

		ewl_widget_configure(EWL_WIDGET(e));
	}
}

/*
 * Position the cursor at the current position in the previous line.
 */
void ewl_entry_cursor_up_move(Ewl_Entry * e)
{
	char *s;
	int plen, pline, pend;
	int lpos, start, len = 0;
	int bp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (e->multiline) {
		len = ewl_entry_length_get(e);
		start = bp = 
			ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

		s = ewl_entry_text_get(e);

		while (--start > 1 && s[start] != '\n');

		if (s[start] == '\n')
			start++;

		lpos = bp - start - 1;
		pline = start - 1;

		while (--pline > 1 && s[pline] != '\n');

		pend = pline;

		while (++pend < len && s[pend] != '\n');

		plen = pend - pline;

		if (plen >= lpos)
			lpos += pline + 1;
		else
			lpos = pend - 1;

		ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor),
					      lpos);

		ewl_widget_configure(EWL_WIDGET(e));
	}
}

/*
 * Position the cursor at the beginning of the widget. This is internal, so the
 * parameter is not checked.
 */
void ewl_entry_cursor_home_move(Ewl_Entry * e)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), 0);
	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Position the cursor at the end of the widget. This is internal, so the
 * parameter is not checked.
 */
void ewl_entry_cursor_end_move(Ewl_Entry * e)
{
	char           *s;
	int             l = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = ewl_entry_text_get(e);

	if (s) {
		l = strlen(s);
		FREE(s);
	}

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), l);
	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_left_delete(Ewl_Entry * e)
{
	Ewl_Entry_Op *op;
	int sp = 0, ep = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (!e->editable) {
		DLEAVE_FUNCTION(DLEVEL_STABLE)
	}

	sp = ep = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));
	sp--;

	if (sp < 0) return;

	op = ewl_entry_op_text_delete_new(e, sp, ep - sp);
	ecore_dlist_append(e->ops, op);

	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	e->length --;

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), sp);
	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void ewl_entry_right_delete(Ewl_Entry * e)
{
	Ewl_Entry_Op *op;
	int sp = 0, len = 0, ep = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (!e->editable) {
		DLEAVE_FUNCTION(DLEVEL_STABLE)
	}

	len = ewl_entry_length_get(e);
	sp = ep = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));

	if (sp == len) return;
	ep ++;

	op = ewl_entry_op_text_delete_new(e, sp, ep - sp);
	ecore_dlist_append(e->ops, op);

	if (REALIZED(e))
		ewl_entry_ops_apply(e);

	e->length --;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_entry_word_begin_delete(Ewl_Entry * e)
{
	char           *s = NULL;
	int             bp, index;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("e", e);

	if (!e->editable) {
		DLEAVE_FUNCTION(DLEVEL_STABLE)
	}

	s = ewl_entry_text_get(e);
	if (!s)
		DRETURN(DLEVEL_STABLE);

	bp = ewl_entry_cursor_position_get(EWL_ENTRY_CURSOR(e->cursor));
	index = bp-2;
	
	while ((index-->0) && (s[index] == ' ')){}
	if (index < 0)
		index = 0;
	while ((index-->0) && (s[index] != ' ')){}
	index++;
	strcpy(&(s[index]), &(s[bp]));
	ewl_entry_text_set(e, s);

	FREE(s);

	if (index <= 0) 
	  index = 1;

	ewl_entry_cursor_position_set(EWL_ENTRY_CURSOR(e->cursor), index);
	ewl_widget_configure(EWL_WIDGET(e));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int ewl_entry_timer(void *data)
{
	Ewl_Entry      *e;
	double          dt;
	double          value;
	int             velocity, direction, tmp;

	e = EWL_ENTRY(data);

	dt = ecore_time_get() - e->start_time;
	
	direction = CURRENT_X(e->cursor) - CURRENT_X(e);
	tmp = (CURRENT_X(e) + CURRENT_W(e)) - (CURRENT_X(e->cursor) +
			CURRENT_W(e->cursor));
	if (direction < tmp)
		direction = -1;
	else
		direction = 1;

	/*
	 * Check the theme for a velocity setting and bring it within normal
	 * useable bounds.
	 */
	velocity = ewl_theme_data_int_get(EWL_WIDGET(e), "velocity");
	if (velocity < 1)
		velocity = 1;
	else if (velocity > 10)
		velocity = 10;

	/*
	 * Move the value of the seeker based on the direction of it's motion
	 * and the velocity setting.
	 */
	value = (double)(direction) * 10.0 * (1 - exp(-dt)) *
		 ((double)(velocity) / 100.0);

/*	e->offset = value * ewl_object_current_w_get(EWL_OBJECT(e->text));
*/
	return 1;
}

/*
 * Cursor functions
 */
Ewl_Widget *
ewl_entry_cursor_new(void)
{
	Ewl_Entry_Cursor *c;

	DENTER_FUNCTION(DLEVEL_STABLE);

	c = NEW(Ewl_Entry_Cursor, 1);
	if (!c)
		return NULL;

	ewl_entry_cursor_init(c);

	DRETURN_PTR(EWL_WIDGET(c), DLEVEL_STABLE);
}

void
ewl_entry_cursor_init(Ewl_Entry_Cursor *c)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);

	w = EWL_WIDGET(c);
	ewl_widget_init(w, "cursor");
	c->position = 0;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_entry_cursor_position_set(Ewl_Entry_Cursor *c, int pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("c", c);

	c->position = pos;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

int
ewl_entry_cursor_position_get(Ewl_Entry_Cursor *c)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("c", c, 0);

	DRETURN_INT(c->position, DLEVEL_STABLE);
}

/*
 * Selection functions
 */
Ewl_Widget *
ewl_entry_selection_new(void)
{
	Ewl_Entry_Selection *s;

	DENTER_FUNCTION(DLEVEL_STABLE);

	s = NEW(Ewl_Entry_Selection, 1);
	if (!s)
		return NULL;

	ewl_entry_selection_init(s);

	DRETURN_PTR(EWL_WIDGET(s), DLEVEL_STABLE);
}

void
ewl_entry_selection_init(Ewl_Entry_Selection *s)
{
	Ewl_Widget *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	w = EWL_WIDGET(s);
	ewl_widget_init(w, "selection");
	s->start = 0;
	s->end = 0;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_entry_selection_start_position_set(Ewl_Entry_Selection *s, 
				       unsigned int start)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	s->start = start;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

int
ewl_entry_selection_start_position_get(Ewl_Entry_Selection *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0);

	DRETURN_INT(s->start, DLEVEL_STABLE);
}

void
ewl_entry_selection_end_position_set(Ewl_Entry_Selection *s,
				     unsigned int end)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	s->end = end;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

int
ewl_entry_selection_end_position_get(Ewl_Entry_Selection *s)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("s", s, 0);

	DRETURN_INT(s->end, DLEVEL_STABLE);
}

void
ewl_entry_selection_select_to(Ewl_Entry_Selection *s, unsigned int pos)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("s", s);

	if (pos < s->start)
		s->start = pos;
	else 
		s->end = pos;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * The Entry Operation functions
 */
static Ewl_Entry_Op *
ewl_entry_op_relevant_find(Ewl_Entry *e, Ewl_Entry_Op_Type type)
{
	Ecore_DList *l;
	void *(*traverse)(Ecore_DList *l);
	Ewl_Entry_Op *op = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (REALIZED(e)) {
		l = e->applied;
		ecore_dlist_goto_first(l);
		traverse = ecore_dlist_next;
	} else {
		l = e->ops;
		ecore_dlist_goto_last(l);
		traverse = ecore_dlist_previous;
	}

	while ((op = traverse(e->ops)))
		if (op->type == type)
			break;

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void ewl_entry_ops_apply(Ewl_Entry *e)
{
	Ewl_Entry_Op *op;

	while ((op = ecore_dlist_remove_first(e->ops))) {
		op->apply(e, op);
		ecore_dlist_append(e->applied, op);
	}
}

static void ewl_entry_ops_reset(Ewl_Entry *e)
{
	Ewl_Entry_Op *op;

	while ((op = ecore_dlist_remove_first(e->applied))) {
		op->apply(e, op);
		ecore_dlist_append(e->ops, op);
	}
}

static void ewl_entry_op_free(void *data)
{
	Ewl_Entry_Op *op = data;
	if (op->free)
		op->free(op);
	else
		FREE(op)
}

static Ewl_Entry_Op *
ewl_entry_op_color_new(Ewl_Entry *e, int r, int g, int b, int a)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Color *opc;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Color, 1);
	if (op) {
		opc = (Ewl_Entry_Op_Color *)op;
		op->type = EWL_ENTRY_OP_TYPE_COLOR_SET;
		op->apply = ewl_entry_op_color_apply;
		op->free = free;
		opc->r = r;
		opc->g = g;
		opc->b = b;
		opc->a = a;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void
ewl_entry_op_color_apply(Ewl_Entry *e, Ewl_Entry_Op *op)
{
	int or, og, ob, oa;
	Ewl_Entry_Op_Color *opc = (Ewl_Entry_Op_Color *)op;

	DENTER_FUNCTION(DLEVEL_STABLE);

	etox_context_get_color(e->context, &or, &og, &ob, &oa);
	etox_context_set_color(e->context, opc->r, opc->g, opc->b, opc->a);

	/*
	 * Store the previous values for undoing.
	 */
	opc->r = or;
	opc->g = og;
	opc->b = ob;
	opc->a = oa;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_font_new(Ewl_Entry *e, char *font, int size)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Font *opf;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Font, 1);
	if (op) {
		opf = (Ewl_Entry_Op_Font *)op;
		op->type = EWL_ENTRY_OP_TYPE_FONT_SET;
		op->apply = ewl_entry_op_font_apply;
		op->free = ewl_entry_op_font_free;
		opf->font = strdup(font);
		opf->size = size;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void
ewl_entry_op_font_apply(Ewl_Entry *e, Ewl_Entry_Op *op)
{
	char *of;
	int size;
	Ewl_Entry_Op_Font *opf = (Ewl_Entry_Op_Font *)op;

	DENTER_FUNCTION(DLEVEL_STABLE);

	of = etox_context_get_font(e->context, &size);

	etox_context_set_font(e->context, opf->font, opf->size);

	FREE(opf->font);
	opf->font = of;
	opf->size = size;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_entry_op_font_free(void *op)
{
	Ewl_Entry_Op_Font *opf = (Ewl_Entry_Op_Font *)op;
	DENTER_FUNCTION(DLEVEL_STABLE);

	FREE(opf->font);
	FREE(opf);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_style_new(Ewl_Entry *e, char *style)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Style *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Style, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Style *)op;
		op->type = EWL_ENTRY_OP_TYPE_STYLE_SET;
		op->apply = ewl_entry_op_style_apply;
		op->free = ewl_entry_op_style_free;
		ops->style = strdup(style);
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void
ewl_entry_op_style_apply(Ewl_Entry *e, Ewl_Entry_Op *op)
{
	char *style;
	Ewl_Entry_Op_Style *ops = (Ewl_Entry_Op_Style *)op;

	DENTER_FUNCTION(DLEVEL_STABLE);

	style = etox_context_get_style(e->context);
	etox_context_set_style(e->context, ops->style);

	FREE(ops->style);
	ops->style = style;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_entry_op_style_free(void *op)
{
	Ewl_Entry_Op_Style *ops = (Ewl_Entry_Op_Style *)op;
	DENTER_FUNCTION(DLEVEL_STABLE);

	FREE(ops->style);
	FREE(ops);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_align_new(Ewl_Entry *e, unsigned int align)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Align *opa;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Align, 1);
	if (op) {
		opa = (Ewl_Entry_Op_Align *)op;
		op->type = EWL_ENTRY_OP_TYPE_ALIGN_SET;
		op->apply = ewl_entry_op_align_apply;
		op->free = free;
		opa->align = align;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void
ewl_entry_op_align_apply(Ewl_Entry *e, Ewl_Entry_Op *op)
{
	unsigned int align;
	Ewl_Entry_Op_Align *opa = (Ewl_Entry_Op_Align *)op;

	DENTER_FUNCTION(DLEVEL_STABLE);

	align = etox_context_get_align(e->context);
	etox_context_set_align(e->context, opa->align);

	opa->align = align;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_text_set_new(Ewl_Entry *e, char *text)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Text *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Text, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Text *)op;
		op->type = EWL_ENTRY_OP_TYPE_TEXT_SET;
		op->apply = ewl_entry_op_text_apply;
		op->free = ewl_entry_op_text_free;
		ops->text = text ? strdup(text) : NULL;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_text_append_new(Ewl_Entry *e, char *text)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Text *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Text, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Text *)op;
		op->type = EWL_ENTRY_OP_TYPE_TEXT_APPEND;
		op->apply = ewl_entry_op_text_apply;
		op->free = ewl_entry_op_text_free;
		ops->text = strdup(text);
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_text_prepend_new(Ewl_Entry *e, char *text)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Text *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Text, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Text *)op;
		op->type = EWL_ENTRY_OP_TYPE_TEXT_PREPEND;
		op->apply = ewl_entry_op_text_apply;
		op->free = ewl_entry_op_text_free;
		ops->text = strdup(text);
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_text_insert_new(Ewl_Entry *e, char *text, int index)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Text *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Text, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Text *)op;
		op->type = EWL_ENTRY_OP_TYPE_TEXT_INSERT;
		op->apply = ewl_entry_op_text_apply;
		op->free = ewl_entry_op_text_free;
		ops->text = strdup(text);
		ops->index = index;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static Ewl_Entry_Op *
ewl_entry_op_text_delete_new(Ewl_Entry *e, unsigned int start, 
			     unsigned int len)
{
	Ewl_Entry_Op *op;
	Ewl_Entry_Op_Text *ops;

	DENTER_FUNCTION(DLEVEL_STABLE);

	op = NEW(Ewl_Entry_Op_Text, 1);
	if (op) {
		ops = (Ewl_Entry_Op_Text *)op;
		op->type = EWL_ENTRY_OP_TYPE_TEXT_DELETE;
		op->apply = ewl_entry_op_text_apply;
		op->free = ewl_entry_op_text_free;
		ops->text = NULL;
		ops->index = start;
		ops->len = len;
	}

	DRETURN_PTR(op, DLEVEL_STABLE);
}

static void
ewl_entry_op_text_apply(Ewl_Entry *e, Ewl_Entry_Op *op)
{
	Ewl_Entry_Op_Text *opt = (Ewl_Entry_Op_Text *)op;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (op->type == EWL_ENTRY_OP_TYPE_TEXT_SET)
		etox_set_text(e->etox, opt->text);
	else if (op->type == EWL_ENTRY_OP_TYPE_TEXT_APPEND)
		etox_append_text(e->etox, opt->text);
	else if (op->type == EWL_ENTRY_OP_TYPE_TEXT_PREPEND)
		etox_prepend_text(e->etox, opt->text);
	else if (op->type == EWL_ENTRY_OP_TYPE_TEXT_INSERT)
		etox_insert_text(e->etox, opt->text, opt->index);
	else if (op->type == EWL_ENTRY_OP_TYPE_TEXT_DELETE)
		etox_delete_text(e->etox, opt->index, opt->len);

	ewl_entry_update_size(e);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_entry_op_text_free(void *op)
{
	Ewl_Entry_Op_Text *opt = (Ewl_Entry_Op_Text *)op;
	DENTER_FUNCTION(DLEVEL_STABLE);

	IF_FREE(opt->text);
	FREE(opt);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_entry_op_prune_list(Ewl_Entry *e, int rstart, int rend, int bstart, int bend)
{
	Ewl_Entry_Op *op;

	ecore_dlist_goto_last(e->ops);
	while ((op = ecore_dlist_current(e->ops))) {
		/*
		 * Stop searching the list if we hit these events.
		 */
		if (op->type >= bstart && op->type <= bend)
			break;
		if (op->type >= rstart && op->type <= rend) {
			ecore_dlist_remove(e->ops);
			if (op->free)
				op->free(op);
			else {
				FREE(op);
			}
		}
		ecore_dlist_previous(e->ops);
	}
}

/*
 * Set the size of the entry to the size of the etox.
 */
static void ewl_entry_update_size(Ewl_Entry * e)
{
	Evas_Coord width, height;

	/*
	 * Adjust the properties of the widget to indicate the size of the text.
	 */
	etox_text_geometry_get(e->etox, &width, &height);

	if (!width)
		width = 1;
	if (!height)
		height = 1;

	evas_object_resize(e->etox, (width), (height));

	/*
	 * Set the preferred size to the size of the etox and request that
	 * size for the widget.
	 */
	ewl_object_preferred_inner_size_set(EWL_OBJECT(e), (int)(width),
					    (int)(height));
}

