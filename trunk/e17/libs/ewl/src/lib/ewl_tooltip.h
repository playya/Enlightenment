#ifndef __EWL_TOOLTIP_H__
#define __EWL_TOOLTIP_H__

/**
 * @file ewl_tooltip.h
 * @defgroup Ewl_Tooltip Tooltip Add a tooltip to a widget
 * @{
 */

/**
 * @themekey /tooltip/file
 * @themekey /tooltip/group
 */

typedef struct _ewl_tooltip Ewl_Tooltip;

#define EWL_TOOLTIP(tt) ((Ewl_Tooltip *) tt)
#define EWL_TOOLTIP_OFFSET 15

struct _ewl_tooltip
{
	Ewl_Box      box;    /**< the floating box container */

	Ewl_Widget  *text;  /**< the text displaying in the tooltip */

	double       delay;  /**< time before tooltip will display in secs */
	int          hide;   /**< flag to enable/disable tooltip */

	int          x;
	int          y;

	Ecore_Timer *timer; /**< pointer to the focus timer */
};

Ewl_Widget 	*ewl_tooltip_new (Ewl_Widget *parent);
int 	 	 ewl_tooltip_init (Ewl_Tooltip *t, Ewl_Widget *parent);
void 		 ewl_tooltip_text_set (Ewl_Tooltip *t, char *text);
void 		 ewl_tooltip_delay_set (Ewl_Tooltip *t, double delay);

/*
 * Internally used callbacks, override at your own risk.
 */
int  ewl_tooltip_focus_timer (void *data);
void ewl_tooltip_parent_mouse_down_cb (Ewl_Widget * w, void *ev_data, 
		void *user_data);
void ewl_tooltip_parent_mouse_move_cb (Ewl_Widget * w, void *ev_data, 
		void *user_data);
void ewl_tooltip_parent_focus_out_cb (Ewl_Widget * w, void *ev_data, 
		void *user_data);
void ewl_tooltip_destroy_cb(Ewl_Widget * w, void *ev_data, void *user_data);

/**
 * @}
 */

#endif        /* __EWL_TOOLTIP_H__ */
