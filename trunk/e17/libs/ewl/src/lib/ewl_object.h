#ifndef EWL_OBJECT_H
#define EWL_OBJECT_H

/**
 * @defgroup Ewl_Object Ewl_Object: Basic Object Inherited by Ewl_Widget
 * @brief Defines the Ewl_Object class along with methods and macros related
 * to it.
 *
 * @{
 */

/**
 * @def EWL_OBJECT_MIN_SIZE
 * The minimum possible size any object can receive.
 */
#define EWL_OBJECT_MIN_SIZE (1)
/**
 * @def EWL_OBJECT_MAX_SIZE 
 * The maximum possible size any object can receive.
 */
#define EWL_OBJECT_MAX_SIZE (INT_MAX)

/**
 * The base class inherited by all widgets. Provides methods for size and
 * position.
 */
typedef struct Ewl_Object Ewl_Object;

/**
 * @def EWL_OBJECT(object) 
 * A typecast for accessing the inherited object fields.
 */
#define EWL_OBJECT(object) ((Ewl_Object *) object)

/**
 * @brief Provides facilities for sizing, position, alignment and fill policy.
 *
 * The fields of the object, while their explanations are fairly clear, can be
 * visualized with the following diagram:
 *
 * @image html object_fields.png
 *
 * The CURRENT_W(w) and CURRENT_H(w) are macros that provide easy access to the
 * data fields describing the internal area of the Ewl_Object. While the
 * functions ewl_object_get_current_w(w) and ewl_object_get_current_h(w) are
 * to access the overall size of the area this Ewl_Object resides in. There
 * are corresponding macros and functions for preferred, minimum and maximum
 * sizes as well. There are also functions for setting each of these values.
 *
 * The affect of the fields when performing layout is as follows:
 *
 * @image html object_sizing.png
 *
 * As illustrated, the fill policy determines how much space an object will
 * use when the request for a specific size is made. When the fill policy
 * contains EWL_FLAG_FILL_HSHRINK, EWL_FLAG_FILL_VSHRINK or both, the
 * Ewl_Object can be resized down to it's minimum size in width, height or both
 * respectively.
 *
 * The opposite is true for a fill policy containing EWL_FLAG_FILL_HFILL,
 * EWL_FLAG_FILL_VFILL or both, The Ewl_Object will now expand to fill the
 * space up to it's maximum size in the respective direction.
 */
struct Ewl_Object
{
	struct
	{
		int             x, /**< Horizontal position */
				y; /**< Vertical position */
		int             w, /**< Width */
				h; /**< Height */
	} current; /**< The current size and position of an object. */

	struct
	{
		int             w, /**< Width */
				h; /**< Height */
	}
	preferred, /**< The optimal size of the object in ideal circumstances */
	maximum, /**< The guaranteed maximum size this object will receive. */
	minimum; /**< The guaranteed minimum size this object will receive. */

	struct
	{
		int             l, /**< Left value */
				r, /**< Right value */
				t, /**< Top value */
				b; /**< Bottom value */
	} pad, /**< The space padded around the outside of the object. */
	insets; /**< The space inside where children should not be laid out. */

	unsigned int flags; /**< Bitmask indicating fill policy and alignment */
};

int             ewl_object_init(Ewl_Object *o);
void            ewl_object_current_geometry_get(Ewl_Object *o, int *x, int *y,
						int *w, int *h);

void            ewl_object_current_size_get(Ewl_Object *o, int *w, int *h);
int             ewl_object_current_x_get(Ewl_Object *o);
int             ewl_object_current_y_get(Ewl_Object *o);
int             ewl_object_current_w_get(Ewl_Object *o);
int             ewl_object_current_h_get(Ewl_Object *o);

void            ewl_object_preferred_inner_size_set(Ewl_Object *o, int w, int h);
void            ewl_object_preferred_inner_size_get(Ewl_Object *o, int *w, int *h);
void            ewl_object_preferred_size_get(Ewl_Object *o, int *w, int *h);

void            ewl_object_preferred_inner_w_set(Ewl_Object *o, int w);
int             ewl_object_preferred_w_get(Ewl_Object *o);
int             ewl_object_preferred_inner_w_get(Ewl_Object *o);

void            ewl_object_preferred_inner_h_set(Ewl_Object *o, int h);
int             ewl_object_preferred_inner_h_get(Ewl_Object *o);
int             ewl_object_preferred_h_get(Ewl_Object *o);

void            ewl_object_geometry_request(Ewl_Object *o, int x, int y,
					    int w, int h);
void            ewl_object_size_request(Ewl_Object *o, int w, int h);
void            ewl_object_position_request(Ewl_Object *o, int x, int y);
void     ewl_object_x_request(Ewl_Object *o, int x);
void     ewl_object_y_request(Ewl_Object *o, int y);
void            ewl_object_w_request(Ewl_Object *o, int w);
void            ewl_object_h_request(Ewl_Object *o, int h);

void            ewl_object_minimum_size_set(Ewl_Object *o, int w, int h);
void     ewl_object_minimum_w_set(Ewl_Object *o, int w);
void     ewl_object_minimum_h_set(Ewl_Object *o, int h);

void            ewl_object_minimum_size_get(Ewl_Object *o, int *w, int *h);
int      ewl_object_minimum_w_get(Ewl_Object *o);
int      ewl_object_minimum_h_get(Ewl_Object *o);

void            ewl_object_maximum_size_set(Ewl_Object *o, int w, int h);
void     ewl_object_maximum_w_set(Ewl_Object *o, int w);
void     ewl_object_maximum_h_set(Ewl_Object *o, int h);

void            ewl_object_maximum_size_get(Ewl_Object *o, int *w, int *h);
int	ewl_object_maximum_w_get(Ewl_Object *o);
int	ewl_object_maximum_h_get(Ewl_Object *o);

unsigned int    ewl_object_alignment_get(Ewl_Object *o);
void     ewl_object_alignment_set(Ewl_Object *o, unsigned int align);
void            ewl_object_place(Ewl_Object *o, int x, int y, int w, int h);

unsigned int    ewl_object_fill_policy_get(Ewl_Object *o);
void     ewl_object_fill_policy_set(Ewl_Object *o, unsigned int fill);

/*
 * Padding setting and retrieval functions.
 */
void            ewl_object_padding_set(Ewl_Object *o, int l, int r, int t,
				       int b);
void            ewl_object_padding_get(Ewl_Object *o, int *l, int *r, int *t,
				       int *b);
int             ewl_object_padding_top_get(Ewl_Object *o);
int             ewl_object_padding_bottom_get(Ewl_Object *o);
int             ewl_object_padding_left_get(Ewl_Object *o);
int             ewl_object_padding_right_get(Ewl_Object *o);

/*
 * Inset setting and retrieval functions.
 */
void            ewl_object_insets_set(Ewl_Object *o, int l, int r, int t,
				      int b);
void            ewl_object_insets_get(Ewl_Object *o, int *l, int *r, int *t,
				      int *b);
int             ewl_object_insets_top_get(Ewl_Object *o);
int             ewl_object_insets_bottom_get(Ewl_Object *o);
int             ewl_object_insets_left_get(Ewl_Object *o);
int             ewl_object_insets_right_get(Ewl_Object *o);

void            ewl_object_flags_add(Ewl_Object *o, unsigned int flags,
				     unsigned int mask);
void            ewl_object_flags_remove(Ewl_Object *o, unsigned int flags,
					unsigned int mask);
unsigned int    ewl_object_flags_has(Ewl_Object *o, unsigned int flags,
				     unsigned int mask);
unsigned int    ewl_object_flags_get(Ewl_Object *o, unsigned int mask);

/**
 * @def ewl_object_recursive_set(o)
 * @param o: the object to change the recursive flag
 * @param val: a boolean indicating the value of the recursive flag
 * @return Returns no value.
 * @brief Changes the recursive flag value to match @a val.
 */
#define ewl_object_recursive_set(o, val) \
	(val ? ewl_object_flags_add(o, EWL_FLAG_PROPERTY_RECURSIVE, \
				    EWL_FLAGS_PROPERTY_MASK) : \
	 ewl_object_flags_remove(o, EWL_FLAG_PROPERTY_RECURSIVE, \
				    EWL_FLAGS_PROPERTY_MASK));

/**
 * @def ewl_object_recursive_get(o)
 * @param o: the parameter to retrieve the current value of recursive flag
 * @return Returns the current setting of the recursive flag for @a o.
 * @brief Retrieves the current setting of the recursive flag for @a o.
 */
#define ewl_object_recursive_get(o) \
	(ewl_object_flags_get(o, EWL_FLAGS_PROPERTY_MASK) & \
	 EWL_FLAG_PROPERTY_RECURSIVE)

/**
 * @def ewl_object_toplevel_set(o, val)
 * @param o: the object to change the top level flag
 * @param val: a boolean indicating the value of the top level flag
 * @return Returns no value.
 * @brief Changes the top level flag value to match @a val.
 */
#define ewl_object_toplevel_set(o, val) \
	(val ? ewl_object_flags_add(o, EWL_FLAG_PROPERTY_TOPLEVEL, \
				    EWL_FLAGS_PROPERTY_MASK) : \
	 ewl_object_flags_remove(o, EWL_FLAG_PROPERTY_RECURSIVE, \
				    EWL_FLAGS_PROPERTY_MASK));

/**
 * @def ewl_object_toplevel_get(o)
 * @param o: the parameter to retrieve the current value of top level flag
 * @return Returns the current setting of the top level flag for @a o.
 * @brief Retrieves the current setting of the top level flag for @a o.
 */
#define ewl_object_toplevel_get(o) \
	(ewl_object_flags_get(o, EWL_FLAGS_PROPERTY_MASK) & \
	 EWL_FLAG_PROPERTY_TOPLEVEL)

#define ewl_object_state_add(o, state) \
	ewl_object_flags_add(o, state, EWL_FLAGS_STATE_MASK)
#define ewl_object_state_remove(o, state) \
	ewl_object_flags_remove(o, state, EWL_FLAGS_STATE_MASK)
#define ewl_object_state_has(o, state) \
	ewl_object_flags_has(o, state, EWL_FLAGS_STATE_MASK)
#define ewl_object_state_get(o, state) \
	ewl_object_flags_get(o, state, EWL_FLAGS_STATE_MASK)

#define ewl_object_queued_add(o, queued) \
	ewl_object_flags_add(o, queued, EWL_FLAGS_QUEUED_MASK)
#define ewl_object_queued_remove(o, queued) \
	ewl_object_flags_remove(o, queued, EWL_FLAGS_QUEUED_MASK)
#define ewl_object_queued_has(o, queued) \
	ewl_object_flags_has(o, queued, EWL_FLAGS_QUEUED_MASK)
#define ewl_object_queued_get(o, queued) \
	ewl_object_flags_get(o, queued, EWL_FLAGS_QUEUED_MASK)

#define ewl_object_visible_add(o, visible) \
	ewl_object_flags_add(o, visible, EWL_FLAGS_VISIBLE_MASK)
#define ewl_object_visible_remove(o, visible) \
	ewl_object_flags_remove(o, visible, EWL_FLAGS_VISIBLE_MASK)
#define ewl_object_visible_has(o, visible) \
	ewl_object_flags_has(o, visible, EWL_FLAGS_VISIBLE_MASK)
#define ewl_object_visible_get(o, visible) \
	ewl_object_flags_get(o, visible, EWL_FLAGS_VISIBLE_MASK)

#define PADDING_TOP(o) EWL_OBJECT(o)->pad.t
#define PADDING_BOTTOM(o) EWL_OBJECT(o)->pad.b
#define PADDING_LEFT(o) EWL_OBJECT(o)->pad.l
#define PADDING_RIGHT(o) EWL_OBJECT(o)->pad.r

#define PADDING_HORIZONTAL(o) (EWL_OBJECT(o)->pad.l + EWL_OBJECT(o)->pad.r)
#define PADDING_VERTICAL(o) (EWL_OBJECT(o)->pad.t + EWL_OBJECT(o)->pad.b)

#define INSET_LEFT(o) EWL_OBJECT(o)->insets.l
#define INSET_RIGHT(o) EWL_OBJECT(o)->insets.r
#define INSET_TOP(o) EWL_OBJECT(o)->insets.t
#define INSET_BOTTOM(o) EWL_OBJECT(o)->insets.b

#define INSET_HORIZONTAL(o) (EWL_OBJECT(o)->insets.l + EWL_OBJECT(o)->insets.r)
#define INSET_VERTICAL(o) (EWL_OBJECT(o)->insets.t + EWL_OBJECT(o)->insets.b)

#define CURRENT_X(o) EWL_OBJECT(o)->current.x
#define CURRENT_Y(o) EWL_OBJECT(o)->current.y
#define CURRENT_W(o) EWL_OBJECT(o)->current.w
#define CURRENT_H(o) EWL_OBJECT(o)->current.h

#define PREFERRED_W(o) EWL_OBJECT(o)->preferred.w
#define PREFERRED_H(o) EWL_OBJECT(o)->preferred.h

#define MAXIMUM_W(o) EWL_OBJECT(o)->maximum.w
#define MAXIMUM_H(o) EWL_OBJECT(o)->maximum.h

#define MINIMUM_W(o) EWL_OBJECT(o)->minimum.w
#define MINIMUM_H(o) EWL_OBJECT(o)->minimum.h

#define ewl_object_custom_size_set(o, w, h) \
	{ \
		ewl_object_minimum_size_set(o, w, h); \
		ewl_object_maximum_size_set(o, w, h); \
 		ewl_object_fill_policy_set(o, EWL_FLAG_FILL_NONE); \
	}

#define ewl_object_custom_w_set(o, w) \
	{ \
		ewl_object_maximum_w_set(o, w); \
		ewl_object_minimum_w_set(o, w); \
		ewl_object_fill_policy_set(o, ewl_object_fill_policy_get(o) & \
				~(EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_HSHRINK)); \
	}

#define ewl_object_custom_h_set(o, h) \
	{ \
		ewl_object_maximum_h_set(o, h); \
		ewl_object_minimum_h_set(o, h); \
		ewl_object_fill_policy_set(o, ewl_object_fill_policy_get(o) & \
				~(EWL_FLAG_FILL_VFILL | EWL_FLAG_FILL_VSHRINK)); \
	}

/**
 * @def RECURSIVE(o)
 * Used to test if a widget is recursive, aka. an Ewl_Container
 */
#define RECURSIVE(o) (EWL_OBJECT(o)->flags & EWL_FLAG_PROPERTY_RECURSIVE)

/**
 * @def REALIZED(o)
 * Used to test if a widget has been realized.
 */
#define REALIZED(o) (EWL_OBJECT(o)->flags & EWL_FLAG_VISIBLE_REALIZED)

/**
 * @def VISIBLE(o)
 * Used to test if a widget is visible.
 */
#define VISIBLE(o) (EWL_OBJECT(o)->flags & EWL_FLAG_VISIBLE_SHOWN)

/**
 * @def OBSCURED(o)
 * Used to determine if a widget is marked as obscured.
 */
#define OBSCURED(o) (EWL_OBJECT(o)->flags & EWL_FLAG_VISIBLE_OBSCURED)

/**
 * @def HIDDEN(o)
 * Used to determine if a widget is hidden.
 */
#define HIDDEN(o) (!(EWL_OBJECT(o)->flags & EWL_FLAG_VISIBLE_SHOWN))

/**
 * @}
 */

#endif
