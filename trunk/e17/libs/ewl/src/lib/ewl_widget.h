#ifndef __EWL_WIDGET_H__
#define __EWL_WIDGET_H__

/**
 * @file ewl_widget.h
 * @defgroup Ewl_Widget Widget: The Parent Widget Class Common to All Widgets
 * @brief Defines the Ewl_Widget class and it's accessor/modifier functions.
 *
 * The Ewl_Widget extends the Ewl_Object to provide the basic facilities
 * necessary for widgets to interact with the end user. This includes basic
 * callbacks for input events, window information changes, and drawing to the
 * display.
 *
 * @{
 */

typedef struct Ewl_Attach_List Ewl_Attach_List;
struct Ewl_Attach_List
{
	void **list;
	unsigned int direct:1;
	unsigned int len:31;
};

/**
 * Callback chain container a list and bitmask of chain properties.
 */
typedef struct Ewl_Callback_Chain Ewl_Callback_Chain;

struct Ewl_Callback_Chain
{
	void **list;
	unsigned short mask;
	unsigned short len;
};

typedef struct Ewl_Color_Set Ewl_Color_Set;

struct Ewl_Color_Set
{
	int r, g, b, a;
};

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
 * @class Ewl_Widget
 * The class inheriting from Ewl_Object that provides appearance, parent, and
 * callback capabilities.
 */
struct Ewl_Widget
{
	Ewl_Object       object; /**< Inherit the base Object class */
	Ewl_Widget      *parent; /**< The parent widget, actually a container */

	Ewl_Callback_Chain callbacks[EWL_CALLBACK_MAX]; /**< Callback chain array */
	Ewl_Attach_List *attach;       /**< List of attachments on the widget */

	Evas_Object     *fx_clip_box;  /**< Clipping rectangle of widget */

	Evas_Object     *theme_object; /**< Appearance shown on canvas */
	char            *bit_state;    /**< State of the appaarance */
	char            *appearance;   /**< Key to lookup appearance in theme */
	char            *inheritance;  /**< Inheritance of path widget */
	int              layer; /**< Current layer of widget on canvas */

	Ecore_Hash       *theme; /**< Overriding theme settings of this widget */
	Ecore_Hash       *theme_text; /**< Overriding text in widgets theme */
	Ecore_Hash       *data; /**< Arbitrary data attached to this widget */
};

/*
 * Initialize a widget to it's default values
 */
int             ewl_widget_init(Ewl_Widget * w);

/*
 * Assign the given name to a widget
 */
void            ewl_widget_name_set(Ewl_Widget * w, const char *name);

/*
 * Retrieve the given name of a widget
 */
const char *    ewl_widget_name_get(Ewl_Widget * w);

/*
 * Find the widget identified by a given name.
 */
Ewl_Widget *    ewl_widget_name_find(const char *name);

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
 *  * Mark the widget to be revealed.
 *   */
void            ewl_widget_reveal(Ewl_Widget *w);

/*      
 *       * Mark the widget to be obscured.    
 *        */     
void            ewl_widget_obscure(Ewl_Widget *w);

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
void            ewl_widget_data_set(Ewl_Widget * w, void *k, void *v);

/*
 * Remove a key value pair from a widget.
 */
void           *ewl_widget_data_del(Ewl_Widget * w, void *k);

/*
 * Retrieve a key value pair from a widget.
 */
void           *ewl_widget_data_get(Ewl_Widget * w, void *k);

/*
 * Change the appearance of a widget based on a state string.
 */
void            ewl_widget_state_set(Ewl_Widget * w, char *state);

/*
 * Change the appearance string used for determining the correct theme data.
 */
void            ewl_widget_appearance_set(Ewl_Widget * w, char *appearance);

/*
 * Retrieve the appearance string of a widget.
 */
char           *ewl_widget_appearance_get(Ewl_Widget * w);

/*
 * Retrieve the full appearance string of the widget.
 */
char	       *ewl_widget_appearance_path_get(Ewl_Widget * w);

/*
 * Change the text of the given theme part of a widget.
 */
void           ewl_widget_appearance_part_text_set(Ewl_Widget * w, char *part,
						   char *text);     

/*
 * Change the text of the theme-defined theme part of a widget.
 */
void           ewl_widget_appearance_text_set(Ewl_Widget * w, char *text);

/*
 * Append to the inherited string 
 */
void            ewl_widget_inherit(Ewl_Widget *widget, char *type);

unsigned int    ewl_widget_type_is(Ewl_Widget *widget, char *type);

/*
 * Change the parent of a widget.
 */
void            ewl_widget_parent_set(Ewl_Widget * w, Ewl_Widget * p);

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
void            ewl_widget_print_tree(Ewl_Widget *w);
void            ewl_widget_print(Ewl_Widget *w);

int             ewl_widget_layer_sum_get(Ewl_Widget *w);
void            ewl_widget_layer_set(Ewl_Widget *w, int layer);
int             ewl_widget_layer_get(Ewl_Widget *w);

void            ewl_widget_internal_set(Ewl_Widget *w, unsigned int val);
unsigned int    ewl_widget_internal_is(Ewl_Widget *w);

void            ewl_widget_clipped_set(Ewl_Widget *w, unsigned int val);
unsigned int    ewl_widget_clipped_is(Ewl_Widget *w);

void            ewl_widget_focus_send(Ewl_Widget *w);

Ewl_Widget     *ewl_widget_focused_get(void);

void            ewl_widget_tab_order_append(Ewl_Widget *w);
void            ewl_widget_tab_order_prepend(Ewl_Widget *w);
void            ewl_widget_tab_order_insert(Ewl_Widget *w, unsigned int idx);
void		ewl_widget_tab_order_insert_before(Ewl_Widget *w, Ewl_Widget *before);
void		ewl_widget_tab_order_insert_after(Ewl_Widget *w, Ewl_Widget *after);
void 		ewl_widget_tab_order_remove(Ewl_Widget *w);

void		ewl_widget_ignore_focus_change_set(Ewl_Widget *w, unsigned int val);
unsigned int	ewl_widget_ignore_focus_change_get(Ewl_Widget *w);

void            ewl_widget_color_set(Ewl_Widget *w, int r, int g, 
						int b, int a);
void            ewl_widget_color_get(Ewl_Widget *w, int *r, int *g, 
						int *b, int *a);

/**
 * @def LAYER(w)
 * Used to retrieve the layer of a widget.
 */
#define LAYER(w) EWL_WIDGET(w)->layer

/*
 * Internally used callbacks, override at your own risk.
 */
void ewl_widget_show_cb(Ewl_Widget * w, void *ev_data,
			void *user_data);
void ewl_widget_hide_cb(Ewl_Widget * w, void *ev_data,
			void *user_data);
void ewl_widget_realize_cb(Ewl_Widget * w, void *ev_data,
			   void *user_data);
void ewl_widget_unrealize_cb(Ewl_Widget * w, void *ev_data,
			     void *user_data);
void ewl_widget_configure_cb(Ewl_Widget * w, void *ev_data,
			     void *user_data);
void ewl_widget_destroy_cb(Ewl_Widget * w, void *ev_data,
			   void *user_data);
void ewl_widget_reparent_cb(Ewl_Widget * w, void *ev_data,
			 void *user_data);
void ewl_widget_enable_cb(Ewl_Widget * w, void *ev_data,
			     void *user_data);
void ewl_widget_disable_cb(Ewl_Widget * w, void *ev_data,
		           void *user_data);
void ewl_widget_focus_in_cb(Ewl_Widget * w, void *ev_data,
			    void *user_data);
void ewl_widget_focus_out_cb(Ewl_Widget * w, void *ev_data,
			     void *user_data);
void ewl_widget_mouse_down_cb(Ewl_Widget * w, void *ev_data,
			      void *user_data);
void ewl_widget_mouse_up_cb(Ewl_Widget * w, void *ev_data,
			    void *user_data);
void ewl_widget_child_destroy_cb(Ewl_Widget * w, void *ev_data,
				 void *user_data);
void ewl_widget_mouse_move_cb(Ewl_Widget * w, void *ev_data,
			      void *user_data);

/**
 * @}
 */

#endif				/* __EWL_WIDGET_H__ */
