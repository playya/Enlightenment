#ifndef _ECORE_H
#define _ECORE_H

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

/**
 * @file Ecore.h
 * @brief The file that provides the program utility, main loop and timer
 *        functions.
 *
 * This header provides the Ecore event handling loop.  For more
 * details, see @ref Ecore_Main_Loop_Group.
 *
 * For the main loop to be of any use, you need to be able to add events
 * and event handlers.  Events for file descriptor events are covered in
 * @ref Ecore_FD_Handler_Group.
 *
 * Time functions are covered in @ref Ecore_Time_Group.
 *
 * There is also provision for callbacks for when the loop enters or
 * exits an idle state. See @ref Idle_Group for more information.
 *
 * Functions are also provided for spawning child processes using fork.
 * See @ref Ecore_Exe_Basic_Group and @ref Ecore_Exe_Signal_Group for
 * more details.
 */

#include <sys/types.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ECORE_EVENT_NONE         0
#define ECORE_EVENT_EXE_EXIT     1 /**< Spawned Exe has exit event */
#define ECORE_EVENT_SIGNAL_USER  2 /**< User signal event */
#define ECORE_EVENT_SIGNAL_HUP   3 /**< Hup signal event */
#define ECORE_EVENT_SIGNAL_EXIT  4 /**< Exit signal event */
#define ECORE_EVENT_SIGNAL_POWER 5 /**< Power signal event */
#define ECORE_EVENT_SIGNAL_REALTIME 6 /**< Realtime signal event */
#define ECORE_EVENT_COUNT        7
   
#ifndef _ECORE_PRIVATE_H   
   enum _Ecore_Fd_Handler_Flags
     {
	ECORE_FD_READ = 1, /**< Fd Read mask */
	ECORE_FD_WRITE = 2, /**< Fd Write mask */
	ECORE_FD_ERROR = 4 /**< Fd Error mask */
     };
   typedef enum _Ecore_Fd_Handler_Flags Ecore_Fd_Handler_Flags;
   
#ifndef WIN32
   typedef void Ecore_Exe; /**< A handle for spawned processes */
#endif
   typedef void Ecore_Timer; /**< A handle for timers */
   typedef void Ecore_Idler; /**< A handle for idlers */
   typedef void Ecore_Idle_Enterer; /**< A handle for idle enterers */
   typedef void Ecore_Idle_Exiter; /**< A handle for idle exiters */
   typedef void Ecore_Fd_Handler; /**< A handle for Fd hanlders */
   typedef void Ecore_Event_Handler; /**< A handle for an event handler */
   typedef void Ecore_Event_Filter; /**< A handle for an event filter */
   typedef void Ecore_Event; /**< A handle for an event */
   typedef void Ecore_Animator; /**< A handle for animators */
#endif
   typedef struct _Ecore_Event_Exe_Exit     Ecore_Event_Exe_Exit; /**< Spawned Exe exit event */
   typedef struct _Ecore_Event_Signal_User  Ecore_Event_Signal_User; /**< User signal event */
   typedef struct _Ecore_Event_Signal_Hup   Ecore_Event_Signal_Hup; /**< Hup signal event */
   typedef struct _Ecore_Event_Signal_Exit  Ecore_Event_Signal_Exit; /**< Exit signal event */
   typedef struct _Ecore_Event_Signal_Power Ecore_Event_Signal_Power; /**< Power signal event */
   typedef struct _Ecore_Event_Signal_Realtime Ecore_Event_Signal_Realtime; /**< Realtime signal event */

#ifndef WIN32
   struct _Ecore_Event_Exe_Exit /** Process exit event */
     {
	pid_t      pid; /**< The process ID of the process that exited */
	int        exit_code; /**< The exit code of the process */
	Ecore_Exe *exe; /**< The handle to the exited process, or NULL if not found */
	int        exit_signal; /** < The signal that caused the process to exit */
	char       exited    : 1; /** < set to 1 if the process exited of its own accord */
	char       signalled : 1; /** < set to 1 id the process exited due to uncaught signal */
	void      *ext_data; /**< Extension data - not used */
	siginfo_t  data; /**< Signal info */
     };
#endif

   struct _Ecore_Event_Signal_User /** User signal event */
     {
	int   number; /**< The signal number. Either 1 or 2 */
	void *ext_data; /**< Extension data - not used */

#ifndef WIN32
	siginfo_t data; /**< Signal info */
#endif
     };
   
   struct _Ecore_Event_Signal_Hup /** Hup signal event */
     {
	void *ext_data; /**< Extension data - not used */

#ifndef WIN32
	siginfo_t data; /**< Signal info */
#endif
     };
   
   struct _Ecore_Event_Signal_Exit /** Exit request event */
     {
	int   interrupt : 1; /**< Set if the exit request was an interrupt  signal*/
	int   quit      : 1; /**< set if the exit request was a quit signal */
	int   terminate : 1; /**< Set if the exit request was a terminate singal */
	void *ext_data;	/**< Extension data - not used */

#ifndef WIN32
	siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Power /** Power event */
     {
	void *ext_data;	/**< Extension data - not used */

#ifndef WIN32
	siginfo_t data; /**< Signal info */
#endif
     };

   struct _Ecore_Event_Signal_Realtime /** Realtime event */
     {
	int num; /**< The realtime signal's number */

#ifndef WIN32
	siginfo_t data; /**< Signal info */
#endif
     };

   EAPI int  ecore_init(void);
   EAPI int  ecore_shutdown(void);
       
   EAPI void ecore_app_args_set(int argc, const char **argv);
   EAPI void ecore_app_args_get(int *argc, char ***argv);
   EAPI void ecore_app_restart(void);

   EAPI Ecore_Event_Handler *ecore_event_handler_add(int type, int (*func) (void *data, int type, void *event), const void *data);
   EAPI void                *ecore_event_handler_del(Ecore_Event_Handler *event_handler);
   EAPI Ecore_Event         *ecore_event_add(int type, void *ev, void (*func_free) (void *data, void *ev), void *data);
   EAPI void                *ecore_event_del(Ecore_Event *event);
   EAPI int                  ecore_event_type_new(void);
   EAPI Ecore_Event_Filter  *ecore_event_filter_add(void * (*func_start) (void *data), int (*func_filter) (void *data, void *loop_data, int type, void *event), void (*func_end) (void *data, void *loop_data), const void *data);
   EAPI void                *ecore_event_filter_del(Ecore_Event_Filter *ef);
   EAPI int                  ecore_event_current_type_get(void);
   EAPI void                *ecore_event_current_event_get(void);
       
       
#ifndef WIN32
   EAPI Ecore_Exe *ecore_exe_run(const char *exe_cmd, const void *data);
   EAPI void      *ecore_exe_free(Ecore_Exe *exe);
   EAPI pid_t      ecore_exe_pid_get(Ecore_Exe *exe);
   EAPI void      *ecore_exe_data_get(Ecore_Exe *exe);
   EAPI void       ecore_exe_pause(Ecore_Exe *exe);
   EAPI void       ecore_exe_continue(Ecore_Exe *exe);
   EAPI void       ecore_exe_terminate(Ecore_Exe *exe);
   EAPI void       ecore_exe_kill(Ecore_Exe *exe);
   EAPI void       ecore_exe_signal(Ecore_Exe *exe, int num);
   EAPI void       ecore_exe_hup(Ecore_Exe *exe);
#endif
       
   EAPI Ecore_Idler *ecore_idler_add(int (*func) (void *data), const void *data);
   EAPI void        *ecore_idler_del(Ecore_Idler *idler);
   
   EAPI Ecore_Idle_Enterer *ecore_idle_enterer_add(int (*func) (void *data), const void *data);
   EAPI void               *ecore_idle_enterer_del(Ecore_Idle_Enterer *idle_enterer);

   EAPI Ecore_Idle_Exiter *ecore_idle_exiter_add(int (*func) (void *data), const void *data);
   EAPI void              *ecore_idle_exiter_del(Ecore_Idle_Exiter *idle_exiter);

   EAPI void              ecore_main_loop_iterate(void);
   EAPI void              ecore_main_loop_begin(void);
   EAPI void              ecore_main_loop_quit(void);
   EAPI Ecore_Fd_Handler *ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, int (*func) (void *data, Ecore_Fd_Handler *fd_handler), const void *data, int (*buf_func) (void *buf_data, Ecore_Fd_Handler *fd_handler), const void *buf_data);
   EAPI void              ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, void (*func) (void *data, Ecore_Fd_Handler *fd_handler), const void *data);
   EAPI void             *ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler);
   EAPI int               ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler);
   EAPI int               ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
   EAPI void              ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
   
   EAPI double ecore_time_get(void);
       
   EAPI Ecore_Timer *ecore_timer_add(double in, int (*func) (void *data), const void *data);
   EAPI void        *ecore_timer_del(Ecore_Timer *timer);
   EAPI void         ecore_timer_interval_set(Ecore_Timer *timer, double in);
   
   EAPI Ecore_Animator *ecore_animator_add(int (*func) (void *data), const void *data);
   EAPI void           *ecore_animator_del(Ecore_Animator *animator);
   EAPI void            ecore_animator_frametime_set(double frametime);
   EAPI double          ecore_animator_frametime_get(void);
       
#ifdef __cplusplus
}
#endif
#endif
