/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef ECORE_FILE_H
#define ECORE_FILE_H

/*
 * TODO:
 * - More events, move/rename of directory file
 */

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef GCC_HASCLASSVISIBILITY
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#endif

#include <Evas.h>
#include <time.h>

EAPI int        ecore_file_init     (void);
EAPI int        ecore_file_shutdown (void);
EAPI time_t     ecore_file_mod_time (const char *file);
EAPI int        ecore_file_exists   (const char *file);
EAPI int        ecore_file_is_dir   (const char *file);
EAPI int        ecore_file_mkdir    (const char *dir);
EAPI int        ecore_file_mkpath   (const char *path);
EAPI int        ecore_file_cp       (const char *src, const char *dst);
EAPI char      *ecore_file_realpath (const char *file);
EAPI char      *ecore_file_get_file (char *path);
EAPI char      *ecore_file_get_dir  (char *path);

EAPI int        ecore_file_can_exec (const char *file);
EAPI char      *ecore_file_readlink (const char *link);
EAPI Evas_List *ecore_file_ls       (const char *dir);

typedef struct _Ecore_File_Monitor       Ecore_File_Monitor;
typedef struct _Ecore_File_Monitor_Event Ecore_File_Monitor_Event;

typedef enum {
     ECORE_FILE_EVENT_NONE,
     ECORE_FILE_EVENT_CREATED_FILE,
     ECORE_FILE_EVENT_CREATED_DIRECTORY,
     ECORE_FILE_EVENT_DELETED_FILE,
     ECORE_FILE_EVENT_DELETED_DIRECTORY,
     ECORE_FILE_EVENT_DELETED_SELF,
     ECORE_FILE_EVENT_MODIFIED
} Ecore_File_Event;

EAPI Ecore_File_Monitor *ecore_file_monitor_add(const char *path,
						void (*func) (void *data,
							      Ecore_File_Monitor *ecore_file_monitor,
							      Ecore_File_Event event,
							      const char *path),
					       	void *data);
EAPI void                ecore_file_monitor_del(Ecore_File_Monitor *ecore_file_monitor);
EAPI const char         *ecore_file_monitor_path_get(Ecore_File_Monitor *ecore_file_monitor);


#endif
