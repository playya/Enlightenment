/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

static int init = 0;

typedef enum {
     ECORE_FILE_MONITOR_TYPE_NONE,
#ifdef HAVE_INOTIFY
     ECORE_FILE_MONITOR_TYPE_INOTIFY,
#endif
#ifdef HAVE_FAM
     ECORE_FILE_MONITOR_TYPE_FAM,
#endif
#ifdef HAVE_POLL
     ECORE_FILE_MONITOR_TYPE_POLL
#endif
} Ecore_File_Monitor_Type;

static Ecore_File_Monitor_Type monitor_type = ECORE_FILE_MONITOR_TYPE_NONE;

int
ecore_file_monitor_init(void)
{
   if (++init != 1) return init;

#ifdef HAVE_INOTIFY
   monitor_type = ECORE_FILE_MONITOR_TYPE_INOTIFY;
   if (ecore_file_monitor_inotify_init())
     return init;
#endif
#ifdef HAVE_FAM
#if 0
   monitor_type = ECORE_FILE_MONITOR_TYPE_FAM;
   if (ecore_file_monitor_fam_init())
     return init;
#endif
#endif
#ifdef HAVE_POLL
   monitor_type = ECORE_FILE_MONITOR_TYPE_POLL;
   if (ecore_file_monitor_poll_init())
     return init;
#endif
   monitor_type = ECORE_FILE_MONITOR_TYPE_NONE;
   return --init;
}

int
ecore_file_monitor_shutdown(void)
{
   if (--init != 0) return init;

   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
	 break;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
	 ecore_file_monitor_inotify_shutdown();
	 break;
#endif
#ifdef HAVE_FAM
      case ECORE_FILE_MONITOR_TYPE_FAM:
	 ecore_file_monitor_fam_shutdown();
	 break;
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
	 ecore_file_monitor_poll_shutdown();
	 break;
#endif
     }
   return init;
}

EAPI Ecore_File_Monitor *
ecore_file_monitor_add(const char *path,
			    void (*func) (void *data, Ecore_File_Monitor *em,
					  Ecore_File_Event event,
					  const char *path),
			    void *data)
{
   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
	 return NULL;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
	 return ecore_file_monitor_inotify_add(path, func, data);
#endif
#ifdef HAVE_FAM
      case ECORE_FILE_MONITOR_TYPE_FAM:
	 return ecore_file_monitor_fam_add(path, func, data);
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
	 return ecore_file_monitor_poll_add(path, func, data);
#endif
     }
   return NULL;
}

EAPI void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
	 break;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
	 ecore_file_monitor_inotify_del(em);
	 break;
#endif
#ifdef HAVE_FAM
      case ECORE_FILE_MONITOR_TYPE_FAM:
	 ecore_file_monitor_fam_del(em);
	 break;
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
	 ecore_file_monitor_poll_del(em);
	 break;
#endif
     }
}

EAPI const char *
ecore_file_monitor_path_get(Ecore_File_Monitor *em)
{
   return em->path;
}
