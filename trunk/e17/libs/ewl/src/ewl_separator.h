
#ifndef __EWL_SEPARATOR_H__
#define __EWL_SEPARATOR_H__

/**
 * @defgroup Ewl_Separator A Visual Separator Between Widgets
 * Defines the Ewl_Separator class used for drawing lines between widgets when
 * visual separation is needed.
 */

/**
 * Provides an Ewl_Widget to simply act as a separator between other
 * Ewl_Widget's.
 */
typedef struct Ewl_Separator Ewl_Separator;

/**
 * @def EWL_SEPARATOR(separator)
 * Typecast a pointer to an Ewl_Separator pointer.
 */
#define EWL_SEPARATOR(separator) ((Ewl_Separator *) separator)

/**
 * @struct Ewl_Separator
 * @brief Inherits from Ewl_Widget to allow drawing a horizontal or vertical
 * line between widgets, depending on the orientation the Ewl_Separator.
 */
struct Ewl_Separator
{
	Ewl_Widget      widget; /**< Inherit from Ewl_Widget */
	Ewl_Orientation orientation; /**< Sets drawing horizontal or vertical */
};

/**
 * @def ewl_hseparator_new()
 * @brief Convenience macro to easily define a horizontal separator.
 */
#define ewl_hseparator_new() ewl_separator_new(EWL_ORIENTATION_HORIZONTAL)

/**
 * @def ewl_vseparator_new()
 * @brief Convenience macro to easily define a vertical separator.
 */
#define ewl_vseparator_new() ewl_separator_new(EWL_ORIENTATION_VERTICAL)

Ewl_Widget     *ewl_separator_new(Ewl_Orientation o);
void            ewl_separator_init(Ewl_Separator * s, Ewl_Orientation o);


#endif				/* __EWL_SEPARATOR_H__ */
