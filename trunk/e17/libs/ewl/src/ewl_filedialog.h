#ifndef __EWL_FILEDIALOG_H__
#define __EWL_FILEDIALOG_H__

/**
 * @defgroup Ewl_Filedialog Filedialog: a filedialog
 * @{
 */

/**
 * @themekey /filedialog/file
 * @themekey /filedialog/group
 */

/**
 * The Ewl_Filedialog provides a filedialog
 */
typedef struct Ewl_Filedialog Ewl_Filedialog;

/**
 * @def EWL_FILEDIALOG(fd)
 * Typecasts a pointer to an Ewl_Filedialog pointer.
 */
#define EWL_FILEDIALOG(fd) ((Ewl_Filedialog *) fd)

/**
 * @struct Ewl_Filedialog
 * Creates a floating widget with different filedialog components.
 */
struct Ewl_Filedialog
{
	Ewl_Box        box;        /**< the box container */
	Ewl_Filedialog_Type type;   /**< define what type of filedialog */
	
	Ewl_Widget     *selector;   /**< Ewl_Fileselector */
	Ewl_Widget     *path_label; /**< label to display current path */
	Ewl_Widget     *entry;      /**< entry for manual input or current selected */
	
	Ewl_Widget     *button_box;  /**< box to hold the buttons */

	Ewl_Widget     *ok; /**< open/save button */
	Ewl_Widget     *cancel; /**< cancel button */
};


Ewl_Widget *ewl_filedialog_new (Ewl_Filedialog_Type type,
		Ewl_Callback_Function ok_cb, Ewl_Callback_Function cancel_cb);
void ewl_filedialog_init (Ewl_Filedialog * fd, Ewl_Filedialog_Type type,
		Ewl_Callback_Function cb, Ewl_Callback_Function cancel_cb);
void ewl_filedialog_open_init (Ewl_Filedialog * fd, Ewl_Callback_Function cb,
		Ewl_Callback_Function cancel_cb);
void ewl_filedialog_save_init (Ewl_Filedialog * fd, Ewl_Callback_Function cb,
		Ewl_Callback_Function cancel_cb);


/*
 * Internally used callback, override at your own risk.
 */
void ewl_filedialog_change_labels (Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_change_entry (Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_change_path (Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_hide_cb (Ewl_Widget * w, void *ev_data, void *user_data);


/**
 * @}
 */

#endif /* __EWL_FILEDIALOG_H__ */
