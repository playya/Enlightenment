#ifndef __EWL_FILESELECTOR_H__
#define __EWL_FILESELECTOR_H__

/**
 * @file ewl_fileselector.h
 * @defgroup Ewl_Fileselector Fileselector: Basic File And Directory Listing
 *
 * Displays a list of directories and a list of files for the user to choose.
 * The selector triggers a EWL_CALLBACK_VALUE_CHANGED callback when either a
 * click causes the selected file to change, when a double click causes the
 * currently displayed directory to change, or when a double click on a file
 * occurs to signal an open. The event data passed to the callback is NULL,
 * unless the double click event occurred on a file, in which case it is the
 * file's path.
 *
 * @{
 */

/**
 * @themekey /fileselector/file
 * @themekey /fileselector/group
 */

/**
 * The Ewl_Fileselector provides a fileselector
 */
typedef struct Ewl_Fileselector Ewl_Fileselector;

/**
 * @def EWL_FILESELECTOR(fd) 
 * Typecasts a pointer to an Ewl_Fileselector pointer.
 */
#define EWL_FILESELECTOR(fs) ((Ewl_Fileselector *) fs)

/**
 * @struct Ewl_Fileselector
 * Creates a fileselector with one tree for dirs and one for files
 */
struct Ewl_Fileselector
{
	Ewl_Box        box;         /* the box containing the widgets */
	Ewl_Widget    *list_dirs;   /* directory table */
	Ewl_Widget    *list_files;  /* file table */
	Ewl_Widget    *entry_dir;
	Ewl_Widget    *entry_file;
	Ewl_Widget    *entry_filter;
  
	char          *path;    /* current fileselector path */
	Ecore_List    *files;   /* current selected items in the fileselector */
	char          *dfilter; /* current filter to apply to directories */
	unsigned int   multi_select; /* is the selector multi select or not */
};

Ewl_Widget 	*ewl_fileselector_new();
int  		 ewl_fileselector_init(Ewl_Fileselector *fs);

char 		*ewl_fileselector_path_get(Ewl_Fileselector *fs);
char 		*ewl_fileselector_file_get(Ewl_Fileselector *fs);

void 		 ewl_fileselector_path_set(Ewl_Fileselector *fs, char *path);

void		 ewl_fileselector_multiselect_set(Ewl_Fileselector *fs, 
							unsigned int val);
unsigned int	 ewl_fileselector_multiselect_get(Ewl_Fileselector *fs);

Ecore_List	*ewl_fileselector_select_list_get(Ewl_Fileselector *fs);
void		ewl_fileselector_filter_set(Ewl_Fileselector *fs,
							char *filter);

/*
 * Internally used callbacks, override at your own risk.
 */

void ewl_fileselector_select_file_cb(Ewl_Widget *w, void *ev_data, void *data);
void ewl_fileselector_select_dir_cb(Ewl_Widget *w, void *ev_data, void *data);
void ewl_fileselector_go_up_cb(Ewl_Widget *w, void *ev_data, void *data);
void ewl_fileselector_go_home_cb(Ewl_Widget *w, void *ev_data, void *data);
void ewl_fileselector_filter_cb(Ewl_Widget *entry, void *ev_data, void *user_data);

/* Private: data for a file */

typedef struct Ewl_Fileselector_Data Ewl_Fileselector_Data;
struct Ewl_Fileselector_Data
{
	char  *name; /* name of the file */
	off_t  size; /* size of the file (bytes) */
	time_t time; /* last modification */
	mode_t mode; /* mode of the file (permissions) */
};

Ewl_Fileselector_Data *ewl_fileselector_data_new(const char *name, off_t  size,
						       time_t time, mode_t mode);
void  ewl_fileselector_data_free (Ewl_Fileselector_Data *d);

#endif /* __EWL_FILESELECTOR_H__ */

