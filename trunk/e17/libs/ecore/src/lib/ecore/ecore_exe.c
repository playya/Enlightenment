#include <errno.h>
#include <sys/wait.h>
#include "ecore_private.h"
#include "Ecore.h"

#ifndef WIN32

struct _ecore_exe_dead_exe
{
   pid_t        pid;
   char        *cmd;
};

static inline void _ecore_exe_exec_it(const char *exe_cmd);

static int _ecore_exe_data_generic_handler(void *data, Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags);
static int _ecore_exe_data_error_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_exe_data_read_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_exe_data_write_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_exe_flush(Ecore_Exe *exe);
static void _ecore_exe_event_exe_data_free(void *data __UNUSED__, void *ev);
static Ecore_Exe *_ecore_exe_is_it_alive(pid_t pid);
static int _ecore_exe_make_sure_its_dead(void *data);
static int _ecore_exe_make_sure_its_really_dead(void *data);

static Ecore_Exe *exes = NULL;
static char *shell = NULL;


/* FIXME: This errno checking stuff should be put elsewhere for everybody to use.
 * For now it lives here though, just to make testing easier.
 */
static int _ecore_exe_check_errno(int result, char *file, int line);
#define E_IF_NO_ERRNO(result, foo, ok) \
  while (((ok) = _ecore_exe_check_errno( (result) = (foo), __FILE__, __LINE__)) == -1)   sleep(1); \
  if (ok)

#define E_NO_ERRNO(result, foo, ok) \
  while (((ok) = _ecore_exe_check_errno( (result) = (foo), __FILE__, __LINE__)) == -1)   sleep(1)

#define E_IF_NO_ERRNO_NOLOOP(result, foo, ok) \
  if (((ok) = _ecore_exe_check_errno( (result) = (foo), __FILE__, __LINE__)))

static int
_ecore_exe_check_errno(int result, char *file, int line)
{
   int saved_errno = errno;
   
   if (result == -1)
     {
	perror("*** errno reports ");
/* What is currently supported -
 *
 *   pipe
 *     EFAULT  Argument is not valid.
 *     EMFILE  Too many file descriptors used by process.
 *     ENFILE  Too many open files by system.
 *   read
 *     EAGAIN  No data now, try again.
 *     EBADF   This is not an fd that can be read.
 *     EFAULT  This is not a valid buffer.
 *     EINTR   Interupted by signal, try again.
 *     EINVAL  This is not an fd that can be read.
 *     EIO     I/O error.
 *     EISDIR  This is a directory, and cannot be read.
 *     others  Depending on what sort of thing we are reading from.
 *   close
 *     EBADF   This is not an fd that can be closed.
 *     EINTR   Interupted by signal, try again.
 *     EIO     I/O error.
 *   dup2
 *     EBADF   This is not an fd that can be dup2'ed.
 *     EBUSY   Race condition between open() and dup()
 *     EINTR   Interupted by signal, try again.
 *     EMFILE  Too many file descriptors used by process.
 *   fcntl
 *     EACCES, EAGAIN  Locked or mapped by something else, try again later.
 *     EBADF   This is not an fd that can be fcntl'ed.
 *     EDEADLK This will cause a deadlock.
 *     EFAULT  This is not a valid lock.
 *     EINTR   Interupted by signal, try again.
 *     EINVAL  This is not a valid arg.
 *     EMFILE  Too many file descriptors used by process.
 *     ENOLCK  Problem getting a lock.
 *     EPERM   Not allowed to do that.
 *   fsync
 *     EBADF   This is not an fd that is open for writing.
 *     EINVAL, EROFS  This is not an fd that can be fsynced.
 *     EIO     I/O error.
 *
 * How to use it -
 *    int ok = 0;
 *    int result;
 *
 *    E_IF_NO_ERRNO(result, foo(bar), ok)
 *      {
 *         E_IF_NO_ERRNO_NOLOOP(result, foo(bar), ok)
 *            {
 *            }
 *      }
 *
 *   if (!ok)
 *     {
 *        // Something failed, cleanup.
 *     }
 */
	switch (saved_errno)
	  {
	   case EACCES :
	   case EAGAIN :
	   case EINTR :
	       {   /* Not now, try later. */
		  fprintf(stderr, "*** Must try again in %s @%u.\n", file, line);
		  result = -1;
		  break;
	       }
	     
	   case EMFILE :
	   case ENFILE :
	   case ENOLCK :
	       {   /* Low on resources. */
		  fprintf(stderr, "*** Low on resources in %s @%u.\n", file, line);
		  result = 0;
		  break;
	       }
	     
	   case EIO :
	       {   /* I/O error. */
		  fprintf(stderr, "*** I/O error in %s @%u.\n", file, line);
		  result = 0;
		  break;
	       }
	     
	   case EFAULT :
	   case EBADF :
	   case EINVAL :
	   case EROFS :
	   case EISDIR :
	   case EDEADLK :
	   case EPERM :
	   case EBUSY :
	       {   /* Programmer fucked up. */
		  fprintf(stderr, 
			  "*** NAUGHTY PROGRAMMER!!!\n"
			  "*** SPANK SPANK SPANK!!!\n"
			  "*** Now go fix your code in %s @%u. Tut tut tut!\n"
			  "\n", file, line);
		  result = 0;
		  break;
	       }
	     
	   default :
	       {   /* Unsupported errno code, please add this one. */
		  fprintf(stderr, 
			  "*** NAUGHTY PROGRAMMER!!!\n"
			  "*** SPANK SPANK SPANK!!!\n"
			  "*** Unsupported errno code %d, please add this one.\n"
			  "*** Now go fix your code in %s @%u, from %s @%u. Tut tut tut!\n"
			  "\n", saved_errno, __FILE__, __LINE__, file, line);
		  result = 0;
		  break;
	       }
	  }
     }
   else   /* Everything is fine. */
     result = 1;
   
   errno = saved_errno;
   return result;
}



/**
 * @defgroup Ecore_Exe_Basic_Group Process Spawning Functions
 *
 * Functions that deal with spawned processes.
 */

/**
 * Spawns a child process.
 *
 * This is now just a thin wrapper around ecore_exe_pipe_run()
 *
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI Ecore_Exe *
ecore_exe_run(const char *exe_cmd, const void *data)
{
/* I'm just being paranoid again, leaving in the original code in case there is a problem. */
#if 0
   Ecore_Exe *exe;
   pid_t pid;
   
   if (!exe_cmd) return NULL;
   pid = fork();
   if (pid)
     {
	exe = calloc(1, sizeof(Ecore_Exe));
	if (!exe)
	  {
	     kill(pid, SIGKILL);
	     return NULL;
	  }
	ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
	exe->pid = pid;
	exe->data = (void *)data;
        exe->cmd = strdup(exe_cmd);
	exes = _ecore_list2_append(exes, exe);
	return exe;
     }
   _ecore_exe_exec_it(exe_cmd);
   exit(127);
   return NULL;
#else
   return ecore_exe_pipe_run(exe_cmd, 0, data);
#endif
}

/**
 * Spawns a child process with its stdin/out available for communication.
 *
 * This function forks and runs the given command using @c /bin/sh.
 *
 * Note that the process handle is only valid until a child process
 * terminated event is received.  After all handlers for the child process
 * terminated event have been called, the handle will be freed by Ecore.
 *
 * This function does the same thing as ecore_exe_run(), but also makes the
 * standard in and/or out as wel las stderr from the child process available
 * for reading or writing.  To write use ecore_exe_send().  To read listen to
 * ECORE_EXE_EVENT_DATA or ECORE_EXE_EVENT_ERROR events (set up handlers).  
 * Ecore may buffer read and error data until a newline character if asked 
 * for with the @p flags.  All data will be included in the events (newlines 
 * will be replaced with NULLS if line buffered).  ECORE_EXE_EVENT_DATA events 
 * will only happen if the process is run with ECORE_EXE_PIPE_READ enabled 
 * in the flags.  The same with the error version.  Writing will only be 
 * allowed with ECORE_EXE_PIPE_WRITE enabled in the flags.
 *
 * @param   exe_cmd The command to run with @c /bin/sh.
 * @param   flags   The flag parameters for how to deal with inter-process I/O
 * @param   data    Data to attach to the returned process handle.
 * @return  A process handle to the spawned process.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI Ecore_Exe *
ecore_exe_pipe_run(const char *exe_cmd, Ecore_Exe_Flags flags, const void *data)
{
   Ecore_Exe *exe = NULL;
   int statusPipe[2] = { -1, -1 };
   int errorPipe[2]  = { -1, -1 };
   int readPipe[2]   = { -1, -1 };
   int writePipe[2]  = { -1, -1 };
   int n = 0;
   int ok = 1;
   int result;

   if (!exe_cmd) return NULL;

   exe = calloc(1, sizeof(Ecore_Exe));
   if (exe == NULL) return NULL;
   
   /*  Create some pipes. */
   if (ok)   E_IF_NO_ERRNO_NOLOOP(result, pipe(statusPipe), ok)
        ;
   if (ok && (flags & ECORE_EXE_PIPE_ERROR))   E_IF_NO_ERRNO_NOLOOP(result, pipe(errorPipe), ok)
      exe->child_fd_error = errorPipe[0];
   if (ok && (flags & ECORE_EXE_PIPE_READ))    E_IF_NO_ERRNO_NOLOOP(result, pipe(readPipe),  ok)
      exe->child_fd_read = readPipe[0];
   if (ok && (flags & ECORE_EXE_PIPE_WRITE))   E_IF_NO_ERRNO_NOLOOP(result, pipe(writePipe), ok)
      exe->child_fd_write = writePipe[1];

   if (ok)
      {
         pid_t pid = 0;
         volatile int vfork_exec_errno = 0;
		  
         /* FIXME: I should double check this.  After a quick look around, this is already done, but via a more modern method. */
         /* signal(SIGPIPE, SIG_IGN);    We only want EPIPE on errors */
         pid = fork();
		  
         if (pid == -1)
            {
               fprintf(stderr, "Failed to fork process\n");
               pid = 0;
            }
         else if (pid == 0)   /* child */
            {
               /* dup2 STDERR, STDIN, and STDOUT.  dup2() allegedly closes the second pipe if it's open. */
	       if (flags & ECORE_EXE_PIPE_ERROR)           E_NO_ERRNO(result, dup2(errorPipe[1], STDERR_FILENO), ok);
               if (ok && (flags & ECORE_EXE_PIPE_READ))    E_NO_ERRNO(result, dup2(readPipe[1],  STDOUT_FILENO), ok);
               if (ok && (flags & ECORE_EXE_PIPE_WRITE))   E_NO_ERRNO(result, dup2(writePipe[0], STDIN_FILENO),  ok);

               if (ok)
	          {
		     /* Setup the status pipe. */
		     E_NO_ERRNO(result, close(statusPipe[0]), ok);
		     E_IF_NO_ERRNO(result, fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC), ok)   /* close on exec shows sucess */
		        {
			   /* Run the actual command. */
			   _ecore_exe_exec_it(exe_cmd);   /* Should not return from this. */
			}
		  }

               /* Something went 'orribly wrong. */
               vfork_exec_errno = errno;

               /* Close the pipes. */
               if (flags & ECORE_EXE_PIPE_ERROR)  E_NO_ERRNO(result, close(errorPipe[1]), ok);
               if (flags & ECORE_EXE_PIPE_READ)   E_NO_ERRNO(result, close(readPipe[1]),  ok);
               if (flags & ECORE_EXE_PIPE_WRITE)  E_NO_ERRNO(result, close(writePipe[0]), ok);
               E_NO_ERRNO(result, close(statusPipe[1]), ok);

               _exit(-1);
            }
         else   /* parent */
            {
               /* Close the unused pipes. */
               E_NO_ERRNO(result, close(statusPipe[1]), ok);
		       
               /* FIXME: after having a good look at the current e fd handling, investigate fcntl(dataPipe[x], F_SETSIG, ...) */
		       
               /* Wait for it to start executing. */
               while (1)
	          {
	             char buf;

	             E_NO_ERRNO(result, read(statusPipe[0], &buf, 1), ok);
	             if (result == 0)
	                {
		           if (vfork_exec_errno != 0)
		              {
		                 n = vfork_exec_errno;
		                 fprintf(stderr, "Could not start \"%s\"\n", exe_cmd);
		                 pid = 0;
		              }
		           break;
	                }
	          }

               /* Close the status pipe. */
               E_NO_ERRNO(result, close(statusPipe[0]), ok);
            }
		  
         if (pid)
            {
               /* Setup the exe structure. */
               ECORE_MAGIC_SET(exe, ECORE_MAGIC_EXE);
               exe->pid = pid;
               exe->flags = flags;
               exe->data = (void *)data;
               if ((exe->cmd = strdup(exe_cmd)))
	          {
	             if (flags & ECORE_EXE_PIPE_ERROR)
	                {   /* Setup the error stuff. */
		           E_IF_NO_ERRNO(result, fcntl(exe->child_fd_error, F_SETFL, O_NONBLOCK), ok)
		              {
		                 exe->error_fd_handler = ecore_main_fd_handler_add(exe->child_fd_error,
					       ECORE_FD_ERROR, _ecore_exe_data_error_handler, exe,
					       NULL, NULL);
		                 if (exe->error_fd_handler == NULL)
			           ok = 0;
		              }
	                }
	             if (ok && (flags & ECORE_EXE_PIPE_READ))
	               {   /* Setup the read stuff. */
		          E_IF_NO_ERRNO(result, fcntl(exe->child_fd_read, F_SETFL, O_NONBLOCK), ok)
		             {
		                exe->read_fd_handler = ecore_main_fd_handler_add(exe->child_fd_read,
				       ECORE_FD_READ, _ecore_exe_data_read_handler, exe,
				       NULL, NULL);
		                if (exe->read_fd_handler == NULL)
			           ok = 0;
		             }
	               }
	           if (ok && (flags & ECORE_EXE_PIPE_WRITE))
	              {   /* Setup the write stuff. */
		         E_IF_NO_ERRNO(result, fcntl(exe->child_fd_write, F_SETFL, O_NONBLOCK), ok)
		            {
		               exe->write_fd_handler = ecore_main_fd_handler_add(exe->child_fd_write,
					ECORE_FD_WRITE, _ecore_exe_data_write_handler, exe,
					NULL, NULL);
		               if (exe->write_fd_handler)
			          ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);   /* Nothing to write to start with. */
		               else
			          ok = 0;
		            }
	              }

	             exes = _ecore_list2_append(exes, exe);
	             n = 0;
	          }
               else
	          ok = 0;
            }
         else
            ok = 0;
      }
   
   if (!ok)
     {   /* Something went wrong, so pull down everything. */
	if (exe->pid)   ecore_exe_terminate(exe);
	IF_FN_DEL(_ecore_exe_free, exe);
     }
   else
     printf("Running as %d for %s.\n", exe->pid, exe->cmd);
   
   errno = n;
   return exe;
}

/**
 * Sends data to the given child process which it recieves on stdin.
 * 
 * This function writes to a child processes standard in, with unlimited
 * buffering. This call will never block. It may fail if the system runs out
 * of memory.
 * 
 * @param exe  The child process to send to
 * @param data The data to send
 * @param size The size of the data to send, in bytes
 * @return 1 if successful, 0 on failure.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI int
ecore_exe_send(Ecore_Exe *exe, void *data, int size)
{
   void *buf;

   buf = realloc(exe->write_data_buf, exe->write_data_size + size);
   if (buf == NULL)   return 0;

   exe->write_data_buf = buf;
   memcpy(exe->write_data_buf + exe->write_data_size, data, size);
   exe->write_data_size += size;

   if (exe->write_fd_handler)
     ecore_main_fd_handler_active_set(exe->write_fd_handler, ECORE_FD_WRITE);
      
   return 1;
}

/**
 * The stdin of the given child process will close when the write buffer is empty.
 * 
 * @param exe  The child process
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI void
ecore_exe_close_stdin(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_close_stdin");
	return;
     }
   exe->close_stdin = 1;
}


/**
 * Sets the string tag for the given process handle
 *
 * @param   exe The given process handle.
 * @param   tag The string tag to set on the process handle.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI void
ecore_exe_tag_set(Ecore_Exe *exe, const char *tag)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_tag_set");
	return;
     }
   IF_FREE(exe->tag);
   if (tag)   exe->tag = strdup(tag);
}

/**
 * Retrieves the tag attached to the given process handle. There is no need to
 * free it as it just returns the internal pointer value. This value is only
 * valid as long as the @p exe is valid or until the tag is set to something
 * else on this @p exe.
 * 
 * @param   exe The given process handle.
 * @return  The string attached to @p exe.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI char *
ecore_exe_tag_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_tag_get");
	return NULL;
     }
   return exe->tag;
}

/**
 * Frees the given process handle.
 *
 * Note that the process that the handle represents is unaffected by this
 * function.
 *
 * @param   exe The given process handle.
 * @return  The data attached to the handle when @ref ecore_exe_run was
 *          called.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI void *
ecore_exe_free(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_free");
	return NULL;
     }
   return _ecore_exe_free(exe);
}

/**
 * Retrieves the process ID of the given spawned process.
 * @param   exe Handle to the given spawned process.
 * @return  The process ID on success.  @c -1 otherwise.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI pid_t
ecore_exe_pid_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_pid_get");
	return -1;
     }
   return exe->pid;
}

/**
 * Retrieves the data attached to the given process handle.
 * @param   exe The given process handle.
 * @return  The data pointer attached to @p exe.
 * @ingroup Ecore_Exe_Basic_Group
 */
EAPI void *
ecore_exe_data_get(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_data_get");
	return NULL;
     }
   return exe->data;
}

/**
 * @defgroup Ecore_Exe_Signal_Group Spawned Process Signal Functions
 *
 * Functions that send signals to spawned processes.
 */

/**
 * Pauses the given process by sending it a @c SIGSTOP signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_pause(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_pause");
	return;
     }
   kill(exe->pid, SIGSTOP);
}

/**
 * Continues the given paused process by sending it a @c SIGCONT signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_continue(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_continue");
	return;
     }
   kill(exe->pid, SIGCONT);
}

/**
 * Sends the given spawned process a terminate (@c SIGTERM) signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_terminate(Ecore_Exe *exe)
{
   struct _ecore_exe_dead_exe *dead;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_terminate");
	return;
     }

   dead = calloc(1, sizeof(struct _ecore_exe_dead_exe));
   if (dead)
      {
         dead->pid = exe->pid;
         dead->cmd = strdup(exe->cmd);
         IF_FN_DEL(ecore_timer_del, exe->doomsday_clock);
         exe->doomsday_clock = ecore_timer_add(10.0, _ecore_exe_make_sure_its_dead, dead);
      }

   printf("Sending TERM signal to %s (%d).\n", exe->cmd, exe->pid);
   kill(exe->pid, SIGTERM);
}

/**
 * Kills the given spawned process by sending it a @c SIGKILL signal.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_kill(Ecore_Exe *exe)
{
   struct _ecore_exe_dead_exe *dead;

   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_kill");
	return;
     }

   dead = calloc(1, sizeof(struct _ecore_exe_dead_exe));
   if (dead)
      {
         dead->pid = exe->pid;
         dead->cmd = strdup(exe->cmd);
         IF_FN_DEL(ecore_timer_del, exe->doomsday_clock);
         exe->doomsday_clock = ecore_timer_add(10.0, _ecore_exe_make_sure_its_really_dead, dead);
      }

   printf("Sending KILL signal to %s (%d).\n", exe->cmd, exe->pid);
   kill(exe->pid, SIGKILL);
}

/**
 * Sends a @c SIGUSR signal to the given spawned process.
 * @param   exe Process handle to the given process.
 * @param   num The number user signal to send.  Must be either 1 or 2, or
 *              the signal will be ignored.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_signal(Ecore_Exe *exe, int num)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_signal");
	return;
     }
   if (num == 1)
     kill(exe->pid, SIGUSR1);
   else if (num == 2)
     kill(exe->pid, SIGUSR2);
}

/**
 * Sends a @c SIGHUP signal to the given spawned process.
 * @param   exe Process handle to the given process.
 * @ingroup Ecore_Exe_Signal_Group
 */
EAPI void
ecore_exe_hup(Ecore_Exe *exe)
{
   if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
     {
	ECORE_MAGIC_FAIL(exe, ECORE_MAGIC_EXE,
			 "ecore_exe_hup");
	return;
     }
   kill(exe->pid, SIGHUP);
}

static Ecore_Exe *
_ecore_exe_is_it_alive(pid_t pid)
{
   Ecore_Exe *exe = NULL;

   /* FIXME: There is no nice, safe, OS independant way to tell if a 
    * particular PID is still alive.  I have written code to do so
    * for my urunlevel busybox applet (http://urunlevel.sourceforge.net/), 
    * but it's for linux only, and still not guaranteed.
    *
    * So for now, we just check that a valid Ecore_Exe structure 
    * exists for it.  Even that is not a guarantee, as the structure
    * can be freed without killing the process.
    *
    * I think we can safely put exe's into two categories, those users
    * that care about the life of the exe, and the run and forget type.
    * The run and forget type starts up the exe, then free's the 
    * Ecore_Exe structure straight away.  They can never call any of 
    * the functions that can call this, so we don't worry about them.
    *
    * Those user's that care about the life of exe's will keep the 
    * Ecore_Exe structure around, terminate them eventually, or
    * register for exit events.  For these ones the assumption
    * that valid Ecore_Exe struct == live exe is almost valid.
    *
    * I will probably copy my urunlevel code into here someday.
    */
   exe = _ecore_exe_find(pid);
   if (exe)
      {
         if (!ECORE_MAGIC_CHECK(exe, ECORE_MAGIC_EXE))
            exe = NULL;
      }

   return exe;
}

static int
_ecore_exe_make_sure_its_dead(void *data)
{
   struct _ecore_exe_dead_exe *dead;

   dead = data;
   if (dead)
      {
         Ecore_Exe *exe = NULL;

         if ((exe =_ecore_exe_is_it_alive(dead->pid)) != NULL)
	    {
	       if (dead->cmd)
                  printf("Sending KILL signal to alledgedly dead %s (%d).\n", dead->cmd, dead->pid);
	       else
                  printf("Sending KILL signal to alledgedly dead PID %d.\n", dead->pid);
               exe->doomsday_clock = ecore_timer_add(10.0, _ecore_exe_make_sure_its_really_dead, dead);
               kill(dead->pid, SIGKILL);
	    }
         else
	    {
	       IF_FREE(dead->cmd);
	       free(dead);
	    }
      }
   return 0;
}

static int
_ecore_exe_make_sure_its_really_dead(void *data)
{
   struct _ecore_exe_dead_exe *dead;

   dead = data;
   if (dead)
      {
         Ecore_Exe *exe = NULL;

         if ((exe =_ecore_exe_is_it_alive(dead->pid)) != NULL)
	    {
	       printf("RUN!  The zombie wants to eat your brains!  And your CPU!\n");
	       if (dead->cmd)
                  printf("%s (%d) is not really dead.\n", dead->cmd, dead->pid);
	       else
                  printf("PID %d is not really dead.\n", dead->pid);
                 exe->doomsday_clock = NULL;
	    }
	 IF_FREE(dead->cmd);
	 free(dead);
      }
   return 0;
}


void
_ecore_exe_init(void)
{
}

void
_ecore_exe_shutdown(void)
{
   while (exes) _ecore_exe_free(exes);
}

Ecore_Exe *
_ecore_exe_find(pid_t pid)
{
   Ecore_List2 *l;
   
   for (l = (Ecore_List2 *)exes; l; l = l->next)
     {
	Ecore_Exe *exe;
	
	exe = (Ecore_Exe *)l;
	if (exe->pid == pid) return exe;
     }
   return NULL;
}

static inline void
_ecore_exe_exec_it(const char *exe_cmd)
{
   char use_sh = 1;
   char* buf = NULL;
   char** args = NULL;
   int save_errno = 0;
   
   /* So what is this doing?
    *
    * We are trying to avoid wrapping the exe call with /bin/sh -c.
    * We conservatively search for certain shell meta characters,
    * If we don't find them, we can call the exe directly.
    */
   if (!strpbrk(exe_cmd, "|&;<>()$`\\\"'*?#"))
     {
	char* token;
	char pre_command = 1;
	int num_tokens = 0;
	
	if (! (buf = strdup(exe_cmd)))
	  return;
	
	token = strtok(buf, " \t\n\v");
	while(token)
	  {
	     if (token[0] == '~')
	       break;
	     if (pre_command)
	       {
		  if (token[0] == '[')
		    break;
		  if (strchr(token, '='))
		    break;
		  else
		    pre_command = 0;
	       }
	     num_tokens ++;
	     token = strtok(NULL, " \t\n\v");
	  }
	IF_FREE(buf);
	if (! token && num_tokens)
	  {
	     int i = 0;
	     char* token;
	     
	     if (! (buf = strdup(exe_cmd)))
	       return;
	     
	     token = strtok(buf, " \t\n\v");
	     use_sh = 0;
	     if (! (args = (char**) calloc(num_tokens + 1, sizeof(char*)))) 
	       {
		  IF_FREE(buf);
		  return;
	       }
	     for (i = 0; i < num_tokens; i ++)
	       {
		  if (token)
		    args[i] = token;
		  token = strtok(NULL, " \t\n\v");
	       }
	     args[num_tokens] = NULL;
	  }
     }
   
   setsid();
   if (use_sh)
     {   /* We have to use a shell to run this. */
	if (shell == NULL) 
	  {   /* Find users preferred shell. */
	     shell = getenv("SHELL");
	     if (shell == 0)
	       shell = "/bin/sh";
	  }
	errno = 0;
	execl(shell, shell, "-c", exe_cmd, (char *)NULL);
     }
   else
     {   /* We can run this directly. */
	errno = 0;
	execvp(args[0], args);
     }
   
   save_errno = errno;
   IF_FREE(buf);
   IF_FREE(args);
   errno = save_errno;
   return;
}

void *
_ecore_exe_free(Ecore_Exe *exe)
{
   void *data;
   int ok = 0;
   int result;

   data = exe->data;

   IF_FN_DEL(ecore_timer_del, exe->doomsday_clock);
   IF_FN_DEL(ecore_main_fd_handler_del, exe->write_fd_handler);
   IF_FN_DEL(ecore_main_fd_handler_del, exe->read_fd_handler);
   IF_FN_DEL(ecore_main_fd_handler_del, exe->error_fd_handler);
   if (exe->child_fd_write)  E_NO_ERRNO(result, close(exe->child_fd_write), ok);
   if (exe->child_fd_read)   E_NO_ERRNO(result, close(exe->child_fd_read), ok);
   if (exe->child_fd_error)  E_NO_ERRNO(result, close(exe->child_fd_error), ok);
   IF_FREE(exe->write_data_buf);
   IF_FREE(exe->read_data_buf);
   IF_FREE(exe->error_data_buf);
   IF_FREE(exe->cmd);
   
   exes = _ecore_list2_remove(exes, exe);
   ECORE_MAGIC_SET(exe, ECORE_MAGIC_NONE);
   IF_FREE(exe->tag);
   free(exe);
   return data;
}


static int
_ecore_exe_data_generic_handler(void *data, Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags)
{
   Ecore_Exe *exe;
   int child_fd;
   int is_buffered = 0;
   int event_type;

   exe = data;
   if (flags & ECORE_FD_READ)
      {
         flags = ECORE_FD_READ;
	 event_type = ECORE_EXE_EVENT_DATA;
	 child_fd = exe->child_fd_read;
         if (exe->flags & ECORE_EXE_PIPE_READ_LINE_BUFFERED)
            is_buffered = 1;
      }
   else
      {
         flags = ECORE_FD_ERROR;
	 event_type = ECORE_EXE_EVENT_ERROR;
	 child_fd = exe->child_fd_error;
         if (exe->flags & ECORE_EXE_PIPE_ERROR_LINE_BUFFERED)
            is_buffered = 1;
      }

   if ((fd_handler) && (ecore_main_fd_handler_active_get(fd_handler, flags)))
      {
         unsigned char *inbuf;
	 int inbuf_num;

         /* Get any left over data from last time. */
         if (flags & ECORE_FD_READ)
	    {
               inbuf = exe->read_data_buf;
               inbuf_num = exe->read_data_size;
	       exe->read_data_buf = NULL;
	       exe->read_data_size = 0;
	    }
	 else
	    {
               inbuf = exe->error_data_buf;
               inbuf_num = exe->error_data_size;
	       exe->error_data_buf = NULL;
	       exe->error_data_size = 0;
	    }

	 for (;;)
	    {
	       int num, lost_exe;
	       char buf[READBUFSIZ];

	       lost_exe = 0;
	       errno = 0;
	       if ((num = read(child_fd, buf, READBUFSIZ)) < 1)  /* FIXME: SPEED/SIZE TRADE OFF - add a smaller READBUFSIZE (currently 64k) to inbuf, use that instead of buf, and save ourselves a memcpy(). */
	          {
		     lost_exe = ((errno == EIO) || 
			         (errno == EBADF) ||
				 (errno == EPIPE) || 
				 (errno == EINVAL) ||
				 (errno == ENOSPC));
                     if ((errno != EAGAIN) && (errno != EINTR))
                        perror("_ecore_exe_generic_handler() read problem ");
                  }
	       if (num > 0)
	          {   /* data got read. */
		     inbuf = realloc(inbuf, inbuf_num + num);
		     memcpy(inbuf + inbuf_num, buf, num);
		     inbuf_num += num;
	          }
	       else
	          {   /* No more data to read. */
		     if (inbuf) 
		        {
		           Ecore_Event_Exe_Data *e;
		       
		           e = calloc(1, sizeof(Ecore_Event_Exe_Data));
		           if (e)
			      {
			         e->exe = exe;
			         e->data = inbuf;
			         e->size = inbuf_num;

                                 if (is_buffered)
				    {   /* Deal with line buffering. */
				       int max = 0;
				       int count = 0;
				       int i;
				       int last = 0;
				       char *c;

                                       c = (char *)inbuf;
				       for (i = 0; i < inbuf_num; i++) /* Find the lines. */
				          {
					     if (inbuf[i] == '\n')
					        {
					           if (count >= max)
					              {
						         /* In testing, the lines seem to arrive in batches of 500 to 1000 lines at most, roughly speaking. */
						         max += 10;  /* FIXME: Maybe keep track of the largest number of lines ever sent, and add half that many instead of 10. */
		                                         e->lines = realloc(e->lines, sizeof(Ecore_Event_Exe_Data_Line) * (max + 1)); /* Allow room for the NULL termination. */
						      }
						   /* raster said to leave the line endings as line endings, however -
						    * This is line buffered mode, we are not dealing with binary here, but lines.
						    * If we are not dealing with binary, we must be dealing with ASCII, unicode, or some other text format.
						    * Thus the user is most likely gonna deal with this text as strings.
						    * Thus the user is most likely gonna pass this data to str functions.
						    * rasters way - the endings are always gonna be '\n';  onefangs way - they will always be '\0'
						    * We are handing them the string length as a convenience.
						    * Thus if they really want it in raw format, they can e->lines[i].line[e->lines[i].size - 1] = '\n'; easily enough.
						    * In the default case, we can do this conversion quicker than the user can, as we already have the index and pointer.
						    * Let's make it easy on them to use these as standard C strings.
						    *
						    * onefang is proud to announce that he has just set a new personal record for the
						    * most over documentation of a simple assignment statement.  B-)
						    */
						   inbuf[i] = '\0';
						   e->lines[count].line = c;
						   e->lines[count].size = i - last;
						   last = i + 1;
						   c = (char *)&inbuf[last];
					           count++;
					        }
					  }
					  if (count == 0) /* No lines to send, cancel the event. */
					     {
                                                _ecore_exe_event_exe_data_free(NULL, e);
					        e = NULL;
					     }
					  else /* NULL terminate the array, so that people know where the end is. */
					     {
						e->lines[count].line = NULL;
						e->lines[count].size = 0;
					     }
					  if (i > last) /* Partial line left over, save it for next time. */
					     {
					        e->size = last;
                                                if (flags & ECORE_FD_READ)
						   {
	                                              exe->read_data_size = i - last;
	                                              exe->read_data_buf = malloc(exe->read_data_size);
		                                      memcpy(exe->read_data_buf, c, exe->read_data_size);
						   }
						else
						   {
	                                              exe->error_data_size = i - last;
	                                              exe->error_data_buf = malloc(exe->error_data_size);
		                                      memcpy(exe->error_data_buf, c, exe->error_data_size);
						   }
					     }
				    }

				 if (e)   /* Send the event. */
			            ecore_event_add(event_type, e,
					    _ecore_exe_event_exe_data_free, NULL);
			      }
		        }
		     if (lost_exe)
		        {
                           if (flags & ECORE_FD_READ)
			      {
                                 if (exe->read_data_size)
                                    printf("There are %d bytes left unsent from the dead exe %s.\n", exe->read_data_size, exe->cmd);
			      }
			   else
			      {
                                 if (exe->error_data_size)
                                    printf("There are %d bytes left unsent from the dead exe %s.\n", exe->error_data_size, exe->cmd);
			      }
			   /* Thought about this a bit.  If the exe has actually 
			    * died, this won't do any harm as it must have died 
			    * recently and the pid has not had a chance to recycle.
			    * It is also a paranoid catchall, coz the usual ecore_signal
			    * mechenism should kick in.  But let's give it a good
			    * kick anyway.
			    */
                           ecore_exe_terminate(exe);   
                        }
		     break;
	          }
	    }
      }

   return 1;
}

static int
_ecore_exe_data_error_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   return _ecore_exe_data_generic_handler(data, fd_handler, ECORE_FD_ERROR);
}

static int
_ecore_exe_data_read_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   return _ecore_exe_data_generic_handler(data, fd_handler, ECORE_FD_READ);
}

static int
_ecore_exe_data_write_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Exe *exe;

   exe = data;
   if ((exe->write_fd_handler) && (ecore_main_fd_handler_active_get(exe->write_fd_handler, ECORE_FD_WRITE)))
      _ecore_exe_flush(exe);

   /* If we have sent all there is to send, and we need to close the pipe, then close it. */
   if ((exe->close_stdin == 1) && (exe->write_data_size == exe->write_data_offset))
      {
         int ok = 0;
         int result;

printf("Closing stdin for %s\n", exe->cmd);
         /* if (exe->child_fd_write)  E_NO_ERRNO(result, fsync(exe->child_fd_write), ok);   This a) doesn't work, and b) isn't needed. */
         IF_FN_DEL(ecore_main_fd_handler_del, exe->write_fd_handler);
         if (exe->child_fd_write)  E_NO_ERRNO(result, close(exe->child_fd_write), ok);
	 exe->child_fd_write = 0;
         IF_FREE(exe->write_data_buf);
      }

   return 1;
}

static void
_ecore_exe_flush(Ecore_Exe *exe)
{
   int count;

   /* check whether we need to write anything at all. */
   if ((!exe->child_fd_write) && (!exe->write_data_buf))   return;
   if (exe->write_data_size == exe->write_data_offset)     return;

   count = write(exe->child_fd_write, 
                 exe->write_data_buf  + exe->write_data_offset, 
		 exe->write_data_size - exe->write_data_offset);
   if (count < 1)
      {
         if (errno == EIO   || errno == EBADF ||
 	     errno == EPIPE || errno == EINVAL ||
	     errno == ENOSPC)   /* we lost our exe! */
	    {
               ecore_exe_terminate(exe);
               if (exe->write_fd_handler)
                 ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);
	    }
      }
   else
      {
         exe->write_data_offset += count;
         if (exe->write_data_offset >= exe->write_data_size)
            {   /* Nothing left to write, clean up. */
	       exe->write_data_size = 0;
	       exe->write_data_offset = 0;
	       IF_FREE(exe->write_data_buf);
               if (exe->write_fd_handler)
                 ecore_main_fd_handler_active_set(exe->write_fd_handler, 0);
            }
      }
}

static void
_ecore_exe_event_exe_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Event_Exe_Data *e;

   e = ev;

   IF_FREE(e->lines);
   IF_FREE(e->data);
   free(e);
}
#endif
