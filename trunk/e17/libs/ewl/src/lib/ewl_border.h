#ifndef __EWL_BORDER_H__
#define __EWL_BORDER_H__

/**
 * @file ewl_border.h
 *
 * @defgroup Ewl_Border Border: A container with a border and label
 * @brief Defines the Ewl_Border class used for adding a border decoration
 * around a group of widgets.
 *
 * @{
 */

/**
 * @themekey /border/file
 * @themekey /border/group
 */

/**
 * Provides an Ewl_Widget to simply act as a separator between other
 * Ewl_Widget's.
 */
typedef struct Ewl_Border Ewl_Border;

/**
 * @def EWL_BORDER(border)
 * Typecast a pointer to an Ewl_Separator pointer.
 */
#define EWL_BORDER(border) ((Ewl_Border *) border)

/**
 * @struct Ewl_Border
 * @brief Inherits from Ewl_Container to allow drawing a border and label
 * decoration around widgets.
 */
struct Ewl_Border
{
	Ewl_Box         box;       /**< Inherit from Ewl_Box */
	Ewl_Widget     *label;     /**< Text label for the border */
	Ewl_Widget     *body;      /**< Box for holding children */
	Ewl_Position    label_position;     /**< Flags for placing the label */
};

Ewl_Widget     *ewl_border_new(char *label);
int             ewl_border_init(Ewl_Border * b, char *label);

void            ewl_border_text_set(Ewl_Border * b, char *t);
char           *ewl_border_text_get(Ewl_Border * b);

void            ewl_border_label_position_set(Ewl_Border *b, Ewl_Position pos);
Ewl_Position    ewl_border_label_position_get(Ewl_Border *b);

void		ewl_border_label_alignment_set(Ewl_Border *b, 
						unsigned int align);
unsigned int    ewl_border_label_alignment_get(Ewl_Border *b);

/**
 * @}
 */

#endif				/* __EWL_BORDER_H__ */

