
#ifndef __EWL_SEPARATOR_H
#define __EWL_SEPARATOR_H

typedef struct _ewl_separator Ewl_Separator;

#define EWL_SEPARATOR(separator) ((Ewl_Separator *) separator)

struct _ewl_separator
{
	Ewl_Widget widget;
	Ewl_Orientation orientation;

	int u_padding;
	int l_padding;
	double fill_percentage;
};

#define ewl_hseparator_new() ewl_separator_new(EWL_ORIENTATION_HORIZONTAL)
#define ewl_vseparator_new() ewl_separator_new(EWL_ORIENTATION_VERTICAL)

Ewl_Widget *ewl_separator_new (Ewl_Orientation o);
void ewl_separator_set_padding (Ewl_Widget * w, int u, int l);
void ewl_separator_set_fill_percentage (Ewl_Widget * w, double p);

#endif /* __EWL_SEPARATOR_H__ */
