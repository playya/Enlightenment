#ifndef __EWL_WIDGET_H__
#define __EWL_WIDGET_H__

/**
 * @file ewl_widget.h
 * Defines the Ewl_Widget class and it's accessor/modifier functions.
 */

/**
 * The class that all widgets should inherit. Provides reference to a parent
 * widget/container, callbacks, and appearance information.
 */
typedef struct Ewl_Widget Ewl_Widget;

/**
 * @def EWL_WIDGET(widget)
 * @brief Typecast a pointer to an Ewl_Widget pointer.
 */
#define EWL_WIDGET(widget) ((Ewl_Widget *) widget)

/**
 * @struct Ewl_Widget
 * The class inheriting from Ewl_Object that provides appearance, parent, and
 * callback capabilities.
 */
struct Ewl_Widget
{
	Ewl_Object      object; /**< Inherit the base Object class */
	Ewl_Widget     *parent; /**< The parent widget, actually a container */

	Ewd_List       *callbacks[EWL_CALLBACK_MAX]; /**< Callback list array */

	Evas_Object    *fx_clip_box; /**< Clipping rectangle of widget */

	Evas_Object    *theme_object; /**< Appearance shown on canvas */
	char           *bit_state; /**< State of the appaarance */
	char           *appearance; /**< Key to lookup appearance in theme */
	int             layer; /**< Current layer of widget on canvas */

	Ewl_State       state; /**< Present widget state bitmask */
	Ewd_Hash       *theme; /**< Overriding theme settings of this widget */
	Ewd_Hash       *data; /**< Arbitrary data attached to this widget */

	Ewl_Widget_Flags flags; /**< Bitmask indicating visibility and status */
};

/*
 * Initialize a widget to it's default values
 */
void            ewl_widget_init(Ewl_Widget * w, char *appearance);

/*
 * Signal the widget that it's parent has changed.
 */
void            ewl_widget_reparent(Ewl_Widget * widget);

/*
 * Realize the appearance of the widget.
 */
void            ewl_widget_realize(Ewl_Widget * widget);

/*
 * Unrealize the appearance of the widget.
 */
void            ewl_widget_unrealize(Ewl_Widget * w);

/*
 * Mark the widget to be displayed.
 */
void            ewl_widget_show(Ewl_Widget * widget);

/*
 * Mark the widget to be hidden.
 */
void            ewl_widget_hide(Ewl_Widget * widget);

/*
 * Free the widget and all ot its contained data.
 */
void            ewl_widget_destroy(Ewl_Widget * widget);

/*
 * Queue the widget to be configured.
 */
void            ewl_widget_configure(Ewl_Widget * widget);

/*
 * Update the widget's appearance based on it's theme data.
 */
void            ewl_widget_theme_update(Ewl_Widget * w);

/*
 * Attach a key/value pair to a widget.
 */
void            ewl_widget_set_data(Ewl_Widget * w, void *k, void *v);

/*
 * Remove a key value pair from a widget.
 */
void            ewl_widget_del_data(Ewl_Widget * w, void *k);

/*
 * Retrieve a key value pair from a widget.
 */
void           *ewl_widget_get_data(Ewl_Widget * w, void *k);

/*
 * Change the appearance of a widget based on a state string.
 */
void            ewl_widget_set_state(Ewl_Widget * w, char *state);

/*
 * Change the appearance string used for determining the correct theme data.
 */
void            ewl_widget_set_appearance(Ewl_Widget * w, char *appearance);

/*
 * Retrieve the appearance string of a widget.
 */
char           *ewl_widget_get_appearance(Ewl_Widget * w);

/*
 * Change the parent of a widget.
 */
void            ewl_widget_set_parent(Ewl_Widget * w, Ewl_Widget * p);

/*
 * Activate a widget.
 */
void            ewl_widget_enable(Ewl_Widget * w);

/*
 * Deactivate a widget.
 */
void            ewl_widget_disable(Ewl_Widget * w);

/*
 * Notify a widget to rebuild it's appearance string.
 */
void            ewl_widget_rebuild_appearance(Ewl_Widget *w);

#define RECURSIVE(w) (EWL_WIDGET(w)->flags & EWL_FLAGS_RECURSIVE)

#define REALIZED(w) (EWL_WIDGET(w)->flags & EWL_FLAGS_REALIZED)
#define VISIBLE(w) (EWL_WIDGET(w)->flags & EWL_FLAGS_SHOWN)
#define OBSCURED(w) (EWL_WIDGET(w)->flags & EWL_FLAGS_OBSCURED)
#define HIDDEN(w) (!(EWL_WIDGET(w)->flags & EWL_FLAGS_SHOWN))
#define LAYER(w) EWL_WIDGET(w)->layer

#endif				/* __EWL_WIDGET_H__ */
