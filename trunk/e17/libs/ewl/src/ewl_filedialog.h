#ifndef __EWL_FILEDIALOG_H__
#define __EWL_FILEDIALOG_H__

/**
 * @defgroup Ewl_Filedialog Filedialog: A Dialog For Picking Files
 *
 * The filedialog is intended to be used for a simple file chooser. It can be
 * placed inside any other container, and provides the ability to pack extra
 * buttons or widgets along the left side. It currently supports two types, an
 * Open and a Save dialog.
 *
 * The normal use of the filedialog is to create a new one the first time an
 * event occurs that requires one. Setting a callback for
 * EWL_CALLBACK_VALUE_CHANGED, allows the programmer to determine when the
 * Open/Save buttons were chosen. If the event data on the callback is NULL,
 * Cancel was clicked, otherwise, the event data is a pointer to the chosen
 * file(s).
 *
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
	Ewl_Box        box;          /**< the box container */
	Ewl_Filedialog_Type type;    /**< define what type of filedialog */
	
	Ewl_Widget     *selector;    /**< Ewl_Fileselector */
	Ewl_Widget     *path_label;  /**< label to display current path */
	Ewl_Widget     *entry;       /**< entry for manual input or current selected */
	
	Ewl_Widget     *decor_box;   /**< box to hold additional widgets */
	Ewl_Widget     *button_box;  /**< box to hold the buttons */

	Ewl_Widget     *ok;          /**< open/save button */
	Ewl_Widget     *cancel;      /**< cancel button */
};


Ewl_Widget *ewl_filedialog_new (Ewl_Filedialog_Type type);
void ewl_filedialog_init (Ewl_Filedialog * fd, Ewl_Filedialog_Type type);
void ewl_filedialog_set_directory(Ewl_Filedialog *fd, char *path);

/*
 * Internally used callback, override at your own risk.
 */
void ewl_filedialog_change_labels_cb (Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_change_path_cb (Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_ok_cb(Ewl_Widget * w, void *ev_data, void *user_data);
void ewl_filedialog_cancel_cb(Ewl_Widget * w, void *ev_data, void *user_data);


/**
 * @}
 */

#endif /* __EWL_FILEDIALOG_H__ */
