
#include <Ewl.h>
#include <Estyle.h>

void            ewl_text_init(Ewl_Text * t);
static void     __ewl_text_realize(Ewl_Widget * w, void *ev_data,
				   void *user_data);
static void     __ewl_text_destroy(Ewl_Widget * w, void *ev_data,
				   void *user_data);
static void     __ewl_text_configure(Ewl_Widget * w, void *ev_data,
				     void *user_data);
static void     __ewl_text_theme_update(Ewl_Widget * w, void *ev_data,
					void *user_data);
static void     __ewl_text_reparent(Ewl_Widget * w, void *ev_data,
				    void *user_data);
void            __ewl_text_update_size(Ewl_Text * t);

/**
 * ewl_text_new - allocate a new text widget
 *
 * Returns a pointer to a newly allocated text widget on success, NULL on
 * failure.
 */
Ewl_Widget     *
ewl_text_new()
{
	Ewl_Text       *t;

	DENTER_FUNCTION(DLEVEL_STABLE);

	t = NEW(Ewl_Text, 1);
	ZERO(t, Ewl_Text, 1);

	ewl_text_init(t);

	DRETURN_PTR(EWL_WIDGET(t), DLEVEL_STABLE);
}

/**
 * ewl_text_init - initialize a text widget to default values and callbacks
 * @t: the text widget to initialize to default values and callbacks
 *
 * Returns no value. Sets the fields and callbacks of the text widget @t to
 * their defaults.
 */
void
ewl_text_init(Ewl_Text * t)
{
	Ewl_Widget     *w;
	char            key[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	ewl_widget_init(w, "/appearance/text/default");
	ewl_object_set_fill_policy(EWL_OBJECT(w), EWL_FILL_POLICY_NONE);

	/*
	 * Setup the default font.
	 */
	snprintf(key, PATH_LEN, "%s/font", w->appearance);
	t->font = ewl_theme_data_get_str(w, key);

	snprintf(key, PATH_LEN, "%s/font_size", w->appearance);
	t->font_size = ewl_theme_data_get_int(w, key);

	/*
	 * Setup the default style alignment and text.
	 */
	t->text = strdup("");
	t->style = ewl_theme_data_get_str(w, "/appearance/text/default/style");
	t->align = EWL_ALIGNMENT_TOP | EWL_ALIGNMENT_LEFT;

	/*
	 * Set up appropriate callbacks for specific events
	 */
	ewl_callback_prepend(w, EWL_CALLBACK_REALIZE, __ewl_text_realize, NULL);
	ewl_callback_prepend(w, EWL_CALLBACK_DESTROY, __ewl_text_destroy, NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE, __ewl_text_configure,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_THEME_UPDATE,
			    __ewl_text_theme_update, NULL);
	ewl_callback_append(w, EWL_CALLBACK_REPARENT, __ewl_text_reparent,
			    NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_set_text - set the text of a text widget
 * @t: the text widget to set the text
 * @text: the new text for the text widget @t
 *
 * Returns no value. Sets the text of the text widget @t to @text.
 */
void
ewl_text_set_text(Ewl_Text * t, char *text)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	IF_FREE(t->text);

	/*
	 * Set the text to the value that was passed in, or an empty string if
	 * NULL was passed in.
	 */
	if (text == NULL)
		t->text = strdup("");
	else
		t->text = strdup(text);

	/*
	 * Update the estyle if it's been realized at this point.
	 */
	if (t->estyle) {
		estyle_set_text(t->estyle, t->text);
		__ewl_text_update_size(t);
	}


	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_text - retrieve the text of a text widget
 * @t: the text widget to retrieve the text
 *
 * Returns a pointer to a copy of the text in @t on success, NULL on failure.
 */
char           *
ewl_text_get_text(Ewl_Text * t)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("t", t, NULL);

	w = EWL_WIDGET(t);

	DRETURN_PTR(t->text ? strdup(t->text) : NULL, DLEVEL_STABLE);
}

/**
 * ewl_text_set_font - set the font of a text widget
 * @t: the text widget to set the font
 * @f: the name of the font to use for the text widget
 *
 * Returns no value. Sets the name of the font for text widget @t to @f and
 * updates the display to use that font.
 */
void
ewl_text_set_font(Ewl_Text * t, char *f)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);
	DCHECK_PARAM_PTR("f", f);

	w = EWL_WIDGET(t);

	IF_FREE(t->font);

	t->font = strdup(f);

	/*
	 * Change the font for the estyle.
	 */
	if (t->estyle) {
		/*
		 * Change the font and then update the size of the widget
		 */
		estyle_set_font(t->estyle, t->font, t->font_size);
		__ewl_text_update_size(t);
	}

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_font - retrieve the font used by a text widget
 * @t: the text widget to get the font
 *
 * Returns a pointer to a copy of the font name used by @t on success, NULL on
 * failure.
 */
char           *
ewl_text_get_font(Ewl_Text * t)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("t", t, NULL);

	w = EWL_WIDGET(t);

	DRETURN_PTR(t->font ? strdup(t->font) : NULL, DLEVEL_STABLE);
}

/**
 * ewl_text_set_font_size - set the font size of a text widget
 * @t: the text widget to set the font size
 * @s: the font size to use for the text widget
 *
 * Returns no value. Sets the font size for the text widget @t to @s.
 */
void
ewl_text_set_font_size(Ewl_Text * t, int s)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	t->font_size = s;

	/*
	 * Change the font for the estyle.
	 */
	if (t->estyle) {
		/*
		 * Change the font and then update the size of the widget
		 */
		estyle_set_font(t->estyle, t->font, t->font_size);
		__ewl_text_update_size(t);
	}

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_font_size - retrieve the font size of a text widget
 * @t: the text widget to retrieve the font size
 *
 * Returns the font size of the text widget on success, 0 on failure.
 */
int
ewl_text_get_font_size(Ewl_Text * t)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("t", t, 0);

	w = EWL_WIDGET(t);

	DRETURN_INT(t->font_size, DLEVEL_STABLE);
}

/**
 * ewl_text_set_color - set the color of the text for a text widget
 * @t: the text widget to set the color
 * @r: the red value for the color
 * @g: the green value for the color
 * @b: the blue value for the color
 * @a: the alpha value for the color
 *
 * Returns no value. Sets the color of the text in the text widget @t to the
 * new color values specified.
 */
void
ewl_text_set_color(Ewl_Text * t, int r, int g, int b, int a)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	if (t->estyle)
		estyle_set_color(t->estyle, r, g, b, a);

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_color - get the color of the text in a text widget
 * @t: the text widget to get the color
 * @r: a pointer to the integer to store the red value
 * @g: a pointer to the integer to store the green value
 * @b: a pointer to the integer to store the blue value
 * @a: a pointer to the integer to store the alpha value
 *
 * Returns no value. Stores the color values into any non-NULL color pointers.
 */
void
ewl_text_get_color(Ewl_Text * t, int *r, int *g, int *b, int *a)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	if (!r)
		*r = t->r;

	if (!g)
		*g = t->g;

	if (!b)
		*b = t->b;

	if (!a)
		*a = t->a;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_set_style - set the text style for a text widget
 * @t: the text widget to set the text style
 * @s: the name of the style to be set for the text
 *
 * Returns no value. Changes the text style of the text widget @t to the style
 * identified by the name @s.
 */
void
ewl_text_set_style(Ewl_Text * t, char *s)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);
	t->style = strdup(s);

	/*
	 * Change the font for the estyle.
	 */
	if (t->estyle) {
		/*
		 * Change the font and then update the size of the widget
		 */
		estyle_set_style(t->estyle, t->style);
		__ewl_text_update_size(t);
	}

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_text_geometry - retrieve the geometry of a text widget
 * @t: the text widget to retrieve geometry
 * @xx: a pointer to an integer to store the x coordinate
 * @yy: a pointer to an integer to store the y coordinate
 * @ww: a pointer to an integer to store the width
 * @hh: a pointer to an integer to store the height
 *
 * Returns no value. Stores the position and size of the text in the text
 * widget @t into the integers pointed to by @xx, @yy, @ww, and @hh
 * respectively.
 */
void
ewl_text_get_text_geometry(Ewl_Text * t, int *xx, int *yy, int *ww, int *hh)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	/*
	 * Need to check if the estyle has been created yet, it may won't be if
	 * the widget has not yet been realized.
	 */
	if (t->estyle)
		estyle_geometry(t->estyle, xx, yy, ww, hh);
	else {
		*xx = CURRENT_X(t);
		*yy = CURRENT_Y(t);
		*ww = CURRENT_W(t);
		*hh = CURRENT_H(t);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_letter_geometry - retrieve the geomtry of a specific letter
 * @t: the widget that holds the text to retrieve a letters geometry
 * @i: the index of the letter in the text to retrieve geometry
 * @xx: a pointer to an integer to store the x coordinate of the letter
 * @yy: a pointer to an integer to store the y coordinate of the letter
 * @ww: a pointer to an integer to store the width of the letter
 * @hh: a pointer to an integer to store the height of the letter
 *
 * Returns no value. Stores the geometry of the letter at index @i of the text
 * widget @t into @xx, @yy, @ww, and @hh respectively.
 */
void
ewl_text_get_letter_geometry(Ewl_Text * t, int i, int *xx, int *yy,
			     int *ww, int *hh)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	w = EWL_WIDGET(t);

	if (t->estyle)
		estyle_text_at(t->estyle, i, xx, yy, ww, hh);
	else {
		*xx = 0;
		*yy = 0;
		*ww = 0;
		*hh = 0;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_letter_geometry_at - get the letter geometry at coordinates
 * @t: the text widget to get the letter geometry by coordinates
 * @x: the x coordinate to check for letter geometry
 * @y: the y coordinate to check for letter geometry
 * @tx: the x coordinate of the letter that intersects @x, @y
 * @ty: the y coordinate of the letter that intersects @x, @y
 * @tw: the width of the letter that intersects @x, @y
 * @th: the height of the letter that intersects @x, @y
 *
 * Returns no value. Stores the geometry of a letter at specified coordinates
 * @x, @y of text widget @t into @tx, @ty, @tw, and @th.
 */
int
ewl_text_get_letter_geometry_at(Ewl_Text * t, int x, int y,
				int *tx, int *ty, int *tw, int *th)
{
	int             i = 0;
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("t", t, 0);

	w = EWL_WIDGET(t);


	if (t->estyle)
		i = estyle_text_at_position(t->estyle, x, y, tx, ty, tw, th);
	else {
		*tx = 0;
		*ty = 0;
		*tw = 0;
		*th = 0;
	}

	DRETURN_INT(i, DLEVEL_STABLE);
}

/**
 * ewl_text_set_alignment - set the alignment of the text in a text widget
 * @t: the text widget to change text alignment
 * @a: the new alignment for the text in @t
 *
 * Returns no value. Changes the alignment of the text in @t to @a.
 */
void
ewl_text_set_alignment(Ewl_Text * t, Ewl_Alignment a)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("t", t);

	ewl_object_set_alignment(EWL_OBJECT(t), a);

	ewl_widget_configure(EWL_WIDGET(t));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_text_get_index_at - get the index of the letter at coordinates
 * @t: the text widget to find the letter index by coordinates
 * @x: the x coordinate to check for the letter index
 * @y: the y coordinate to check for the letter index
 *
 * Returns the index of the letter at the coordinates @x, @y in the text
 * widget @t.
 */
int
ewl_text_get_index_at(Ewl_Text * t, int x, int y)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("t", t, 0);

	w = EWL_WIDGET(t);

	DRETURN_INT(t->
		    estyle ? estyle_text_at_position(t->estyle, (double) x,
						     (double) y, NULL,
						     NULL, NULL, NULL) : 0,
		    DLEVEL_STABLE);
}

static void
__ewl_text_realize(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Text       *t;
	Ewl_Window     *win;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	t = EWL_TEXT(w);
	win = ewl_window_find_window_by_widget(w);
	t->estyle = estyle_new(win->evas, t->text, t->style);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_text_destroy(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Text       *t;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	t = EWL_TEXT(w);

	if (t->estyle)
		estyle_free(t->estyle);

	IF_FREE(t->text);
	IF_FREE(t->font);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_text_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Text       *t;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	t = EWL_TEXT(w);

	if (t->estyle)
		estyle_move(t->estyle, CURRENT_X(t) + INSET_LEFT(w) +
			    PADDING_LEFT(w), CURRENT_Y(t) + INSET_TOP(w) +
			    PADDING_TOP(w));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_text_theme_update(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Text       *t;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	t = EWL_TEXT(w);

	if (!t->estyle)
		DRETURN(DLEVEL_STABLE);

	/*
	 * Set the correct font and size.
	 */
	estyle_set_font(t->estyle, t->font, t->font_size);

	/*
	 * Set move it into the correct position.
	 */
	estyle_set_layer(t->estyle, LAYER(w));

	/*
	 * Adjust the clip box for the estyle and then display it.
	 */
	estyle_set_clip(t->estyle, w->fx_clip_box);
	estyle_show(t->estyle);

	__ewl_text_update_size(t);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
__ewl_text_reparent(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Text       *t;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	t = EWL_TEXT(w);

	if (!t->estyle)
		DRETURN(DLEVEL_STABLE);

	estyle_set_clip(t->estyle, w->fx_clip_box);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
__ewl_text_update_size(Ewl_Text * t)
{
	int             x, y, width, height;

	/*
	 * Adjust the properties of the widget to indicate the size of the text.
	 */
	estyle_geometry(t->estyle, &x, &y, &width, &height);

	width += INSET_LEFT(t) + INSET_RIGHT(t);
	height += INSET_TOP(t) + INSET_BOTTOM(t);

	/*
	 * Set the preferred size to the size of the estyle and request that
	 * size for the widget.
	 */
	ewl_object_set_preferred_size(EWL_OBJECT(t), width, height);
	ewl_object_set_custom_size(EWL_OBJECT(t), width, height);
}
