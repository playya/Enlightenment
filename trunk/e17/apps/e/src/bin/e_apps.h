#ifndef E_APPS_H
#define E_APPS_H

typedef enum _E_App_Change
{
   E_APP_ADD,
   E_APP_DEL,
   E_APP_CHANGE,
   E_APP_ORDER,
   E_APP_EXEC,
   E_APP_READY,
   E_APP_EXIT
} E_App_Change;

typedef struct _E_App          E_App;

struct _E_App
{
   E_Object             e_obj_inherit;
   
   E_App         *parent; /* the parent e_app node */
   
   char          *name; /* app name */
   char          *generic; /* generic app name */
   char          *comment; /* a longer description */
   char          *exe; /* command to execute, NULL if directory */
   char          *path; /* path to .eet containing icons etc. etc. */

   char          *win_name; /* window name */
   char          *win_class; /* window class */
   
   Evas_List     *subapps; /* if this a directory, a list of more E_App's */
   
   time_t         mod_time; /* last modified time for file or dir */
   time_t         order_mod_time; /* secondary modified time for .order */
   time_t         directory_mod_time; /* secondary modified time for .directory.eet */

   Evas_List     *instances; /* a list of all the exe handles for executions */
   
   unsigned char  startup_notify : 1; /* disable while starting etc. */
   unsigned char  starting : 1; /* this app is starting */

   unsigned char  scanned : 1; /* have we scanned a subdir app yet */
};

EAPI int    e_app_init(void);
EAPI int    e_app_shutdown(void);

EAPI E_App *e_app_new(char *path, int scan_subdirs);
EAPI void   e_app_subdir_scan(E_App *a, int scan_subdirs);
EAPI int    e_app_exec(E_App *a);
EAPI int    e_app_starting_get(E_App *a);
EAPI int    e_app_running_get(E_App *a);
    
EAPI void   e_app_change_callback_add(void (*func) (void *data, E_App *a, E_App_Change ch), void *data);
EAPI void   e_app_change_callback_del(void (*func) (void *data, E_App *a, E_App_Change ch), void *data);

EAPI E_App *e_app_window_name_class_find(char *name, char *class);

#endif
