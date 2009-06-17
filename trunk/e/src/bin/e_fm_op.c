/*
 * vim:cindent:ts=8:sw=3:sts=8:expandtab:cino=>5n-3f0^-2{2
 */

#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS  64
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include <errno.h>
#include <limits.h>

#include <Ecore.h>
#include <Ecore_File.h>

#include <eina_stringshare.h>

#define E_TYPEDEFS
#include "e_fm_op.h"
#undef E_TYPEDEFS
#include "e_fm_op.h"

#define READBUFSIZE 65536
#define COPYBUFSIZE 16384
#define REMOVECHUNKSIZE 4096

#define FREE(p) do { if (p) {free((void *)p); p = NULL;} } while (0)

typedef struct _E_Fm_Op_Task E_Fm_Op_Task;
typedef struct _E_Fm_Op_Copy_Data E_Fm_Op_Copy_Data;

static E_Fm_Op_Task *_e_fm_op_task_new();
static void _e_fm_op_task_free(void *t);

static void _e_fm_op_remove_link_task(E_Fm_Op_Task *task);
static int _e_fm_op_stdin_data(void *data, Ecore_Fd_Handler * fd_handler);
static void _e_fm_op_set_up_idlers();
static void _e_fm_op_delete_idler(int *mark);
static int _e_fm_op_idler_handle_error(int *mark, Eina_List **queue, Eina_List **node, E_Fm_Op_Task *task);

static int _e_fm_op_work_idler(void *data);
static int _e_fm_op_scan_idler(void *data);

static void _e_fm_op_send_error(E_Fm_Op_Task * task, E_Fm_Op_Type type, const char *fmt, ...);
static void _e_fm_op_rollback(E_Fm_Op_Task * task);
static void _e_fm_op_update_progress_report_simple(int percent, const char *src, const char *dst);
static void  _e_fm_op_update_progress(E_Fm_Op_Task *task, long long _plus_e_fm_op_done, long long _plus_e_fm_op_total);
static void _e_fm_op_copy_stat_info(E_Fm_Op_Task *task);
static int _e_fm_op_handle_overwrite(E_Fm_Op_Task *task);

static int _e_fm_op_copy_dir(E_Fm_Op_Task * task);
static int _e_fm_op_copy_link(E_Fm_Op_Task *task);
static int _e_fm_op_copy_fifo(E_Fm_Op_Task *task);
static int _e_fm_op_open_files(E_Fm_Op_Task *task);
static int _e_fm_op_copy_chunk(E_Fm_Op_Task *task);

static int _e_fm_op_copy_atom(E_Fm_Op_Task * task);
static int _e_fm_op_scan_atom(E_Fm_Op_Task * task);
static int _e_fm_op_copy_stat_info_atom(E_Fm_Op_Task * task);
static int _e_fm_op_symlink_atom(E_Fm_Op_Task * task);
static int _e_fm_op_remove_atom(E_Fm_Op_Task * task);

Ecore_Fd_Handler *_e_fm_op_stdin_handler = NULL;

Eina_List *_e_fm_op_work_queue = NULL, *_e_fm_op_scan_queue = NULL;
Ecore_Idler *_e_fm_op_work_idler_p = NULL, *_e_fm_op_scan_idler_p = NULL;

long long _e_fm_op_done, _e_fm_op_total; /* Type long long should be 64 bits wide everywhere,
                                            this means that it's max value is 2^63 - 1, which 
                                            is 8 388 608 terabytes, and this should be enough.
                                            Well, we'll be multipling _e_fm_op_done by 100, but 
                                            still, it is big enough. */

int _e_fm_op_abort = 0;	/* Abort mark. */
int _e_fm_op_scan_error = 0;
int _e_fm_op_work_error = 0;
int _e_fm_op_overwrite = 0;

int _e_fm_op_error_response = E_FM_OP_NONE;
int _e_fm_op_overwrite_response = E_FM_OP_NONE;

Eina_List *_e_fm_op_separator = NULL;

void *_e_fm_op_stdin_buffer = NULL;

struct _E_Fm_Op_Task
{
   struct
   {
      const char *name;
      struct stat st;
   } src;

   struct
   {
      const char *name;
      size_t done;
   } dst;

   int started, finished;

   void *data;

   E_Fm_Op_Type type;
   E_Fm_Op_Type overwrite;

   Eina_List *link;
};

struct _E_Fm_Op_Copy_Data
{
   FILE *from;
   FILE *to;
};

int
main(int argc, char **argv)
{
   int i, last;
   E_Fm_Op_Type type;

   ecore_init();
   eina_stringshare_init();

   _e_fm_op_stdin_buffer = malloc(READBUFSIZE);

   _e_fm_op_stdin_handler =
     ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _e_fm_op_stdin_data, NULL,
                               NULL, NULL);

   if (argc <= 2) return 0;

   last = argc - 1;
   i = 2;

   if (strcmp(argv[1], "cp") == 0)
     type = E_FM_OP_COPY;
   else if (strcmp(argv[1], "mv") == 0)
     type = E_FM_OP_MOVE;
   else if (strcmp(argv[1], "rm") == 0)
     type = E_FM_OP_REMOVE;
   else if (strcmp(argv[1], "lns") == 0)
     type = E_FM_OP_SYMLINK;
   else return 0;

   if ((type == E_FM_OP_COPY) || (type == E_FM_OP_MOVE) || (type == E_FM_OP_SYMLINK))
     {
	if (argc < 4) goto quit;

	if (type == E_FM_OP_MOVE)
	  {
	     _e_fm_op_work_queue = eina_list_append(_e_fm_op_work_queue, NULL);
	     _e_fm_op_separator = _e_fm_op_work_queue;
	  }

	if ((argc >= 4) && (ecore_file_is_dir(argv[last])))
	  {
 	     char buf[PATH_MAX];
	     char *p2, *p3;
	     int p2_len, last_len, done, total;

	     p2 = ecore_file_realpath(argv[last]);
	     if (!p2) goto quit;
	     p2_len = strlen(p2);

	     last_len = strlen(argv[last]);
	     if ((last_len < 1) || (last_len + 2 >= PATH_MAX))
	       {
		  free(p2);
		  goto quit;
	       }
	     memcpy(buf, argv[last], last_len);
	     if (buf[last_len - 1] != '/')
	       {
		  buf[last_len] = '/';
		  last_len++;
	       }

	     p3 = buf + last_len;

	     done = 0;
	     total = last - 2;

	     for (; i < last; i++)
	       {
		  char *p = ecore_file_realpath(argv[i]);
		  const char *name;
		  int name_len;

		  if (!p) continue;

		  /* Don't move a dir into itself */
		  if (ecore_file_is_dir(p) &&
		      (strncmp(p, p2, p2_len) == 0) &&
		      ((p[p2_len] == '/') || (p[p2_len] == '\0')))
		    goto skip_arg;

		  name = ecore_file_file_get(p);
		  if (!name) goto skip_arg;
		  name_len = strlen(name);
		  if (p2_len + name_len >= PATH_MAX) goto skip_arg;
		  memcpy(p3, name, name_len + 1);

		  if ((type == E_FM_OP_MOVE) &&
		      (rename(argv[i], buf) == 0))
		    {
		       done++;
		       _e_fm_op_update_progress_report_simple
			 (done * 100 / total, argv[i], buf);
		    }
		  else if ((type == E_FM_OP_SYMLINK) &&
			   (symlink(argv[i], buf) == 0))
		    {
		       done++;
		       _e_fm_op_update_progress_report_simple
			 (done * 100 / total, argv[i], buf);
		    }
		  else
		    {
		       E_Fm_Op_Task *task;

		       task = _e_fm_op_task_new();
		       task->type = type;
		       task->src.name = eina_stringshare_add(argv[i]);
		       task->dst.name = eina_stringshare_add(buf);

		       _e_fm_op_scan_queue =
			 eina_list_append(_e_fm_op_scan_queue, task);
		    }

	       skip_arg:
		  free(p);
	       }

	     free(p2);
	  }
	else if (argc == 4)
	  {
	     char *p, *p2;

	     p = ecore_file_realpath(argv[2]);
	     p2 = ecore_file_realpath(argv[3]);

	     /* Don't move a file on top of itself. */
	     i = (strcmp(p, p2) == 0);
	     free(p);
	     free(p2);
	     if (i) goto quit;

	     /* Try a rename */
	     if ((type == E_FM_OP_MOVE) && (rename(argv[2], argv[3]) == 0))
	       {
		  _e_fm_op_update_progress_report_simple(100, argv[2], argv[3]);
		  goto quit;
	       }
	     else if ((type == E_FM_OP_SYMLINK) &&
		      (symlink(argv[2], argv[3]) == 0))
	       {
		  _e_fm_op_update_progress_report_simple(100, argv[2], argv[3]);
		  goto quit;
	       }
	     else
	       {
		  E_Fm_Op_Task *task;

		  /* If that doesn't work, setup a copy and delete operation.
		     It's not atomic, but it's the best we can do. */
		  task = _e_fm_op_task_new();
		  task->type = type;
		  task->src.name = eina_stringshare_add(argv[2]);
		  task->dst.name = eina_stringshare_add(argv[3]);
		  _e_fm_op_scan_queue = eina_list_append(_e_fm_op_scan_queue, task);
	       }
	  }
	else
	  goto quit;
     }
   else if (type == E_FM_OP_REMOVE)
     {
	if (argc < 3) return 0;

        while (i <= last)
          {
	     E_Fm_Op_Task *task;

             task = _e_fm_op_task_new();
             task->type = type;
             task->src.name = eina_stringshare_add(argv[i]);
             _e_fm_op_scan_queue = eina_list_append(_e_fm_op_scan_queue, task);
             i++;
          }
     }

   _e_fm_op_set_up_idlers();

   ecore_main_loop_begin();

quit:
   eina_stringshare_shutdown();
   ecore_shutdown();

   free(_e_fm_op_stdin_buffer);

   E_FM_OP_DEBUG("Slave quit.\n");

   return 0;
}

/* Create new task. */
static E_Fm_Op_Task *
_e_fm_op_task_new()
{
   E_Fm_Op_Task *t;

   t = malloc(sizeof(E_Fm_Op_Task));
   t->src.name = NULL;
   memset(&(t->src.st), 0, sizeof(struct stat));

   t->dst.name = NULL;
   t->dst.done = 0;
   t->started = 0;
   t->finished = 0;
   t->data = NULL;
   t->type = E_FM_OP_NONE;
   t->overwrite = E_FM_OP_NONE;
   t->link = NULL;

   return t;
}

/* Free task. */
static void
_e_fm_op_task_free(void *t)
{
   E_Fm_Op_Task *task = t;
   E_Fm_Op_Copy_Data *data;

   if (!task) return;

   if (task->src.name) eina_stringshare_del(task->src.name);
   if (task->dst.name) eina_stringshare_del(task->dst.name);

   if (task->data)
     {
        data = task->data;
        if (task->type == E_FM_OP_COPY)
          {
             if (data->from) fclose(data->from);
             if (data->to) fclose(data->to);
         }
	FREE(task->data);
     }
   FREE(task);
}

/* Removes link task from work queue.
 * Link task is not NULL in case of MOVE. Then two tasks are created: copy and remove.
 * Remove task is a link task for the copy task. If copy task is aborted (e.g. error 
 * occured and user chooses to ignore this), then the remove task is removed from 
 * queue with this functions.
 */
static void 
_e_fm_op_remove_link_task(E_Fm_Op_Task *task)
{
   if (task->link)
     {
        _e_fm_op_work_queue = 
          eina_list_remove_list(_e_fm_op_work_queue, task->link);
        _e_fm_op_task_free(task->link);
        task->link = NULL;
     }
}

/*
 * Handles data from STDIN.
 * Received data must be in this format:
 * 1) (int) magic number,
 * 2) (int) id,
 * 3) (int) message length.
 * Right now message length is always 0. Id is what matters.
 *
 * This function uses a couple of static variables and a global 
 * variable _e_fm_op_stdin_buffer to deal with a situation, when read() 
 * did not actually read enough data.
 */
static int
_e_fm_op_stdin_data(void *data, Ecore_Fd_Handler * fd_handler)
{
   int fd;
   static void *buf = NULL;
   static int length = 0;
   void *begin = NULL;
   ssize_t num = 0;
   int msize;
   int identity;

   fd = ecore_main_fd_handler_fd_get(fd_handler);
   if (!buf) 
     {
        buf = _e_fm_op_stdin_buffer;
        length = 0;
     }

   num = read(fd, buf, READBUFSIZE - length);

   if (num == 0)
     {
        E_FM_OP_DEBUG("STDIN was closed. Abort. \n");
	_e_fm_op_abort = 1;
     }
   else if (num < 0)
     {
        E_FM_OP_DEBUG("Error while reading from STDIN: read returned -1. (%s) Abort. \n", strerror(errno));
	_e_fm_op_abort = 1;
     }
   else
     {
        length += num;

        buf = _e_fm_op_stdin_buffer;
        begin = _e_fm_op_stdin_buffer;

	while (length >= 3 * sizeof(int))
	  {
             begin = buf;

	     /* Check magic. */
	     if (*(int *)buf != E_FM_OP_MAGIC)
               {
                  E_FM_OP_DEBUG("Error while reading from STDIN: magic is not correct!\n");
                  break;
               }
	     buf += sizeof(int);

             /* Read indentifying data. */
	     memcpy(&identity, buf, sizeof(int));
	     buf += sizeof(int);

	     /* Read message length. */
	     memcpy(&msize, buf, sizeof(int));
	     buf += sizeof(int);

	     if ((length - 3 * sizeof(int)) < msize)
	       {
		  /* There is not enough data to read the whole message. */
                  break;
	       }

             length -= (3 * sizeof(int));

             /* You may want to read msize bytes of data too,
              * but currently commands here do not have any data.
              * msize is always 0.
              */
	     switch (identity)
	       {
	       case E_FM_OP_ABORT:
                  _e_fm_op_abort = 1;
                  E_FM_OP_DEBUG("Aborted.\n");
                  break;
	       case E_FM_OP_ERROR_RESPONSE_ABORT:
	       case E_FM_OP_ERROR_RESPONSE_IGNORE_THIS:
	       case E_FM_OP_ERROR_RESPONSE_IGNORE_ALL:
               case E_FM_OP_ERROR_RESPONSE_RETRY:
		  _e_fm_op_error_response = identity;
                  _e_fm_op_set_up_idlers();
                  break;
               case E_FM_OP_OVERWRITE_RESPONSE_NO:
               case E_FM_OP_OVERWRITE_RESPONSE_NO_ALL:
               case E_FM_OP_OVERWRITE_RESPONSE_YES:
               case E_FM_OP_OVERWRITE_RESPONSE_YES_ALL:
                  _e_fm_op_overwrite_response = identity;
                  _e_fm_op_set_up_idlers();
                  E_FM_OP_DEBUG("Overwrite response set.\n");
                  break;
	       }
	  }
        if (length > 0) memmove(_e_fm_op_stdin_buffer, begin, length);
        buf = _e_fm_op_stdin_buffer + length;
     }

   return 1;
}

static void 
_e_fm_op_set_up_idlers()
{
   if (!_e_fm_op_scan_idler_p)
     _e_fm_op_scan_idler_p = ecore_idler_add(_e_fm_op_scan_idler, NULL);
   if (!_e_fm_op_work_idler_p)
     _e_fm_op_work_idler_p = ecore_idler_add(_e_fm_op_work_idler, NULL);
}

#define _E_FM_OP_ERROR_SEND_SCAN(_task, _e_fm_op_error_type, _fmt, ...)\
   do\
     {\
        int _errno = errno;\
        _e_fm_op_scan_error = 1;\
        _e_fm_op_send_error(_task, _e_fm_op_error_type, _fmt, __VA_ARGS__, strerror(_errno));\
        return 1;\
     }\
   while (0)

#define _E_FM_OP_ERROR_SEND_WORK(_task, _e_fm_op_error_type, _fmt, ...)\
    do\
      {\
         int _errno = errno;\
         _e_fm_op_work_error = 1;\
         _e_fm_op_send_error(_task, _e_fm_op_error_type, _fmt, __VA_ARGS__, strerror(_errno));\
         return 1;\
      }\
    while (0)

static void 
_e_fm_op_delete_idler(int *mark)
{
   if (mark == &_e_fm_op_work_error)
     {
        ecore_idler_del(_e_fm_op_work_idler_p);
        _e_fm_op_work_idler_p = NULL;
     }
   else
     {
        ecore_idler_del(_e_fm_op_scan_idler_p);
        _e_fm_op_scan_idler_p = NULL;
     }
}

/* Code to deal with overwrites and errors in idlers.
 * Basically, it checks if we got a response. 
 * Returns 1 if we did; otherwise checks it and does what needs to be done.
 */
static int 
_e_fm_op_idler_handle_error(int *mark, Eina_List **queue, Eina_List **node, E_Fm_Op_Task *task)
{
   if (_e_fm_op_overwrite)
     {
        if (_e_fm_op_overwrite_response != E_FM_OP_NONE)
          {
             task->overwrite = _e_fm_op_overwrite_response;
             _e_fm_op_work_error = 0;
             _e_fm_op_scan_error = 0;
          }
        else
          {
             /* No response yet. */
             /* So, delete this idler. It'll be added back when response is there. */
             _e_fm_op_delete_idler(mark);
             return 1;
          }
     }
   else if (*mark)
     {
        if (_e_fm_op_error_response == E_FM_OP_NONE)
          { 
             /* No response yet. */
             /* So, delete this idler. It'll be added back when response is there. */
             _e_fm_op_delete_idler(mark);
             return 1;
          }
        else
          {
             E_FM_OP_DEBUG("Got response.\n");
             /* Got response. */ 
             if (_e_fm_op_error_response == E_FM_OP_ERROR_RESPONSE_ABORT)
               {
                  /* Mark as abort. */
                  _e_fm_op_abort = 1;
                  _e_fm_op_error_response = E_FM_OP_NONE;
                  _e_fm_op_rollback(task);
               }
             else if (_e_fm_op_error_response == E_FM_OP_ERROR_RESPONSE_RETRY)
               {
                  *mark = 0;
                  _e_fm_op_error_response = E_FM_OP_NONE;
               }
             else if (_e_fm_op_error_response == E_FM_OP_ERROR_RESPONSE_IGNORE_THIS)
               {
                  _e_fm_op_rollback(task);
                  _e_fm_op_remove_link_task(task);
                  *queue = eina_list_remove_list(*queue, *node);
                  _e_fm_op_error_response = E_FM_OP_NONE;
                  *mark = 0;
                  *node = NULL;
                  return 1;
               }
             else if (_e_fm_op_error_response == E_FM_OP_ERROR_RESPONSE_IGNORE_ALL)
               {
                  E_FM_OP_DEBUG("E_Fm_Op_Task '%s' --> '%s' was automatically aborted.\n",
                                task->src.name, task->dst.name);
                  _e_fm_op_rollback(task);
                  _e_fm_op_remove_link_task(task);
                  *queue = eina_list_remove_list(*queue, *node);
                  *node = NULL;
                  *mark = 0;
                  /* Do not clean out _e_fm_op_error_response. This way when another error occures, it would be handled automatically. */ 
                  return 1; 
               }
          }
     }
   else if (( _e_fm_op_work_error) || (_e_fm_op_scan_error))
     return 1;
   return 0;
} 

/* This works very simple. Take a task from queue and run appropriate _atom() on it.
 * If after _atom() is done, task->finished is 1 remove the task from queue. Otherwise,
 * run _atom() on the same task on next call.
 *
 * If we have an abort (_e_fm_op_abort = 1), then _atom() should recognize it and do smth. 
 * After this, just finish everything.
 */
static int
_e_fm_op_work_idler(void *data)
{
   /* E_Fm_Op_Task is marked static here because _e_fm_op_work_queue can be populated with another
    * tasks between calls. So it is possible when a part of file is copied and then 
    * another task is pushed into _e_fm_op_work_queue and the new one if performed while 
    * the first one is not finished yet. This is not cool, so we make sure one task 
    * is performed until it is finished. Well, this can be an issue with removing 
    * directories. For example, if we are trying to remove a non-empty directory, 
    * then this will go into infinite loop. But this should never happen. 
    *
    * BTW, the same is propably right for the _e_fm_op_scan_idler().
    */
   static Eina_List *node = NULL;
   E_Fm_Op_Task *task = NULL;

   if (!node) node = _e_fm_op_work_queue;
   task = eina_list_data_get(node);
   if (!task) 
     {
        node = _e_fm_op_work_queue;
        task = eina_list_data_get(node);
     }

   if (!task)
     {
        if ((_e_fm_op_separator) && 
            (_e_fm_op_work_queue == _e_fm_op_separator) && 
            (_e_fm_op_scan_idler_p == NULL))
          {
             /* You may want to look at the comment in _e_fm_op_scan_atom() about this separator thing. */
             _e_fm_op_work_queue = eina_list_remove_list(_e_fm_op_work_queue, _e_fm_op_separator);
             node = NULL;
             return 1;
          }

        if ((_e_fm_op_scan_idler_p == NULL) && (!_e_fm_op_work_error) && 
            (!_e_fm_op_scan_error))
          ecore_main_loop_quit();

        return 1;
     }

   if (_e_fm_op_idler_handle_error(&_e_fm_op_work_error, &_e_fm_op_work_queue, &node, task)) 
     return 1;

   task->started = 1;

   if (task->type == E_FM_OP_COPY)
     _e_fm_op_copy_atom(task);
   else if (task->type == E_FM_OP_REMOVE)
     _e_fm_op_remove_atom(task);
   else if (task->type == E_FM_OP_COPY_STAT_INFO)
     _e_fm_op_copy_stat_info_atom(task);
   else if (task->type == E_FM_OP_SYMLINK)
     _e_fm_op_symlink_atom(task);

   if (task->finished)
     {
       _e_fm_op_work_queue = eina_list_remove_list(_e_fm_op_work_queue, node);
       _e_fm_op_task_free(task);
       node = NULL;
     }

   if (_e_fm_op_abort)
     {
	/* So, _atom did what it whats in case of abort. Now to idler. */
	ecore_main_loop_quit();
	return 0;
     }

   return 1;
}

/* This works pretty much the same as _e_fm_op_work_idler(), except that 
 * if this is a dir, then look into its contents and create a task 
 * for those files. And we don't have _e_fm_op_separator here.
 */
int
_e_fm_op_scan_idler(void *data)
{
   static Eina_List *node = NULL;
   E_Fm_Op_Task *task = NULL;
   char buf[PATH_MAX];
   static struct dirent *de = NULL;
   static DIR *dir = NULL;
   E_Fm_Op_Task *ntask = NULL;

   if (!node) node = _e_fm_op_scan_queue;
   task = eina_list_data_get(node);
   if (!task) 
     {
        node = _e_fm_op_scan_queue;
        task = eina_list_data_get(node);
     }

   if (!task)
     {
	_e_fm_op_scan_idler_p = NULL;
	return 0;
     }

   if (_e_fm_op_idler_handle_error(&_e_fm_op_scan_error, &_e_fm_op_scan_queue, &node, task)) 
     return 1;

   if (_e_fm_op_abort)
     {
	/* We're marked for abortion. */
	ecore_main_loop_quit();
	return 0;
     }

   if (task->type == E_FM_OP_COPY_STAT_INFO)
     {
        _e_fm_op_scan_atom(task);
        if (task->finished)
          {
             _e_fm_op_scan_queue = 
               eina_list_remove_list(_e_fm_op_scan_queue, node);
             _e_fm_op_task_free(task);
             node = NULL;
          }
     }
   else if (!dir && !task->started)
     {
	if (lstat(task->src.name, &(task->src.st)) < 0)
          _E_FM_OP_ERROR_SEND_SCAN(task, E_FM_OP_ERROR, 
                                   "Cannot lstat '%s': %s.", task->src.name);

	if (S_ISDIR(task->src.st.st_mode))
	  {
	     /* If it's a dir, then look through it and add a task for each. */

             dir = opendir(task->src.name);
             if (!dir)
               _E_FM_OP_ERROR_SEND_SCAN(task, E_FM_OP_ERROR, 
                                        "Cannot open directory '%s': %s.", 
                                        task->dst.name);
          }
        else
          task->started = 1;
     }
   else if (dir && !task->started)
     {
        de = readdir(dir);

        if (!de)
          {
             ntask = _e_fm_op_task_new();
             ntask->type = E_FM_OP_COPY_STAT_INFO;
             ntask->src.name = eina_stringshare_add(task->src.name);
             memcpy(&(ntask->src.st), &(task->src.st), sizeof(struct stat));

             if (task->dst.name)
               ntask->dst.name = eina_stringshare_add(task->dst.name);
             else
               ntask->dst.name = NULL;

             if (task->type == E_FM_OP_REMOVE)
               _e_fm_op_scan_queue = 
               eina_list_prepend(_e_fm_op_scan_queue, ntask);
             else
               _e_fm_op_scan_queue = 
               eina_list_append(_e_fm_op_scan_queue, ntask);

             task->started = 1;
             closedir(dir);
             dir = NULL;
             node = NULL;
             return 1;
          }

        if ((!strcmp(de->d_name, ".") || (!strcmp(de->d_name, ".."))))
          return 1;

        ntask = _e_fm_op_task_new();
        ntask->type = task->type;
        snprintf(buf, sizeof(buf), "%s/%s", task->src.name, de->d_name);
        ntask->src.name = eina_stringshare_add(buf);

        if (task->dst.name)
          {
             snprintf(buf, sizeof(buf), "%s/%s", task->dst.name, de->d_name);
             ntask->dst.name = eina_stringshare_add(buf);
          }
        else
          ntask->dst.name = NULL;

        if (task->type == E_FM_OP_REMOVE)
          _e_fm_op_scan_queue = eina_list_prepend(_e_fm_op_scan_queue, ntask);
        else
          _e_fm_op_scan_queue = eina_list_append(_e_fm_op_scan_queue, ntask);
     }
   else
     {
	_e_fm_op_scan_atom(task);
	if (task->finished)
          {
             _e_fm_op_scan_queue = 
               eina_list_remove_list(_e_fm_op_scan_queue, node);
             _e_fm_op_task_free(task);
             node = NULL;
          }
     }

   return 1;
}

/* Packs and sends an error to STDOUT.
 * type is either E_FM_OP_ERROR or E_FM_OP_OVERWRITE.
 * fmt is a printf format string, the other arguments 
 * are for this format string,
 */
static void
_e_fm_op_send_error(E_Fm_Op_Task * task, E_Fm_Op_Type type, const char *fmt, ...)
{
   va_list ap;
   char buffer[READBUFSIZE];
   void *buf = &buffer[0];
   char *str = buf + 3 * sizeof(int);
   int len = 0;

   va_start(ap, fmt);

   if (_e_fm_op_error_response == E_FM_OP_ERROR_RESPONSE_IGNORE_ALL)
     {
	/* Do nothing. */
     }
   else
     {
        vsnprintf(str, READBUFSIZE - 3 * sizeof(int), fmt, ap);
        len = strlen(str);

        *(int *)buf = E_FM_OP_MAGIC;
        *(int *)(buf + sizeof(int)) = type;
        *(int *)(buf + 2 * sizeof(int)) = len + 1;

        write(STDOUT_FILENO, buf, 3*sizeof(int) + len + 1);

        E_FM_OP_DEBUG("%s", str);
	E_FM_OP_DEBUG(" Error sent.\n");
     }

   va_end(ap);
}

/* Unrolls task: makes a clean up and updates progress info. */
static void
_e_fm_op_rollback(E_Fm_Op_Task *task)
{
   E_Fm_Op_Copy_Data *data;

   if (!task) return;

   if (task->type == E_FM_OP_COPY)
     {
	data = task->data;
	if (data)
	  {
	     if (data->from)
	       {
		  fclose(data->from);
		  data->from = NULL;
	       }
	     if (data->to)
	       {
		  fclose(data->to);
		  data->to = NULL;
	       }
	  }
	FREE(task->data);
     }

   if (task->type == E_FM_OP_COPY)
     _e_fm_op_update_progress(task, -task->dst.done, -task->src.st.st_size);
   else
     _e_fm_op_update_progress(task, -REMOVECHUNKSIZE, -REMOVECHUNKSIZE);
}

static void
_e_fm_op_update_progress_report(int percent, int eta, double elapsed, size_t done, size_t total, const char *src, const char *dst)
{
   const int magic = E_FM_OP_MAGIC;
   const int id = E_FM_OP_PROGRESS;
   void *p, *data;
   int size, src_len, dst_len;

   src_len = strlen(src);
   dst_len = strlen(dst);

   size = 2 * sizeof(int) + 2 * sizeof(size_t) + src_len + 1 + dst_len + 1;
   data = alloca(3 * sizeof(int) + size);
   if (!data) return;
   p = data;

#define P(value) memcpy(p, &(value), sizeof(int)); p += sizeof(int)
   P(magic);
   P(id);
   P(size);
   P(percent);
   P(eta);
#undef P

#define P(value) memcpy(p, &(value), sizeof(size_t)); p += sizeof(size_t)
   P(done);
   P(total);
#undef P

#define P(value) memcpy(p, value, value ## _len + 1); p += value ## _len + 1
   P(src);
   P(dst);
#undef P

   write(STDOUT_FILENO, data, 3 * sizeof(int) + size);

   E_FM_OP_DEBUG("Time left: %d at %e\n", eta, elapsed);
   E_FM_OP_DEBUG("Progress %d. \n", percent);
}

static void
_e_fm_op_update_progress_report_simple(int percent, const char *src, const char *dst)
{
   size_t done = (percent * REMOVECHUNKSIZE) / 100;
   _e_fm_op_update_progress_report
     (percent, 0, 0, done, REMOVECHUNKSIZE, src, dst);
}

/* Updates progress.
 * _plus_data is how much more works is done and _plus_e_fm_op_total 
 * is how much more work we found out needs to be done 
 * (it is not zero primarily in _e_fm_op_scan_idler())
 *
 * It calculates progress in percent. And once a second calculates eta.
 * If either of them changes from their previuos values, then the are 
 * packed and written to STDOUT. 
 */
static void
_e_fm_op_update_progress(E_Fm_Op_Task *task, long long _plus_e_fm_op_done, long long _plus_e_fm_op_total)
{
   static int ppercent = -1;
   int percent;
   static double ctime = 0;
   static double stime = 0;
   double eta = 0;
   static int peta = -1;
   static E_Fm_Op_Task *ptask = NULL;

   _e_fm_op_done += _plus_e_fm_op_done;
   _e_fm_op_total += _plus_e_fm_op_total;

   /* Do not send progress until scan is done.*/
   if (_e_fm_op_scan_idler_p) return;

   if (_e_fm_op_total != 0)
     {
        /* % 101 is for the case when somehow work queue works faster 
         than scan queue. _e_fm_op_done * 100 should not cause arithmetic 
         overflow, since long long can hold really big values. */
	percent = _e_fm_op_done * 100 / _e_fm_op_total % 101;	

        eta = peta;

        if (!stime) stime = ecore_time_get();

        /* Update ETA once a second */
        if ((_e_fm_op_done) && (ecore_time_get() - ctime > 1.0 )) 
          {
             ctime = ecore_time_get();
             eta = (ctime - stime) * (_e_fm_op_total - _e_fm_op_done) / _e_fm_op_done;
             eta = (int) (eta + 0.5);
          }

	if ((percent != ppercent) || (eta != peta) || ((task) && (task != ptask)))
	  {
	     ppercent = percent;
             peta = eta;
             ptask = task;
	     _e_fm_op_update_progress_report(percent, eta, ctime - stime,
					     _e_fm_op_done, _e_fm_op_total,
					     task->src.name, task->dst.name);
	  }
     }
}

/* We just use this code in several places. */
static void 
_e_fm_op_copy_stat_info(E_Fm_Op_Task *task)
{
   struct utimbuf ut;

   if (!task->dst.name) return;

   chmod(task->dst.name, task->src.st.st_mode);
   chown(task->dst.name, task->src.st.st_uid, task->src.st.st_gid);
   ut.actime = task->src.st.st_atime;
   ut.modtime = task->src.st.st_mtime;
   utime(task->dst.name, &ut);
}

static int
_e_fm_op_handle_overwrite(E_Fm_Op_Task *task)
{
   struct stat st;

   if ((task->overwrite == E_FM_OP_OVERWRITE_RESPONSE_YES_ALL) 
       || (_e_fm_op_overwrite_response == E_FM_OP_OVERWRITE_RESPONSE_YES_ALL))
     {
        _e_fm_op_overwrite = 0;
        return 0;
     }
   else if ((task->overwrite == E_FM_OP_OVERWRITE_RESPONSE_YES) 
            || (_e_fm_op_overwrite_response == E_FM_OP_OVERWRITE_RESPONSE_YES))
     {
        _e_fm_op_overwrite_response = E_FM_OP_NONE;
        _e_fm_op_overwrite = 0;
        return 0;
     }
   else if ((task->overwrite == E_FM_OP_OVERWRITE_RESPONSE_NO) 
            || (_e_fm_op_overwrite_response == E_FM_OP_OVERWRITE_RESPONSE_NO))
     {
        task->finished = 1;
        _e_fm_op_rollback(task);
        _e_fm_op_remove_link_task(task);
        _e_fm_op_overwrite_response = E_FM_OP_NONE;
        _e_fm_op_overwrite = 0;
        return 1;
     }
   else if ((task->overwrite == E_FM_OP_OVERWRITE_RESPONSE_NO_ALL) 
            || (_e_fm_op_overwrite_response == E_FM_OP_OVERWRITE_RESPONSE_NO_ALL))
     {
        task->finished = 1;
        _e_fm_op_rollback(task);
        _e_fm_op_remove_link_task(task);
        _e_fm_op_overwrite = 0;
        return 1;
     }

   if ( stat(task->dst.name, &st) == 0)
     {
        /* File exists. */
        if ( _e_fm_op_overwrite_response == E_FM_OP_OVERWRITE_RESPONSE_NO_ALL)
          {
             task->finished = 1;
             _e_fm_op_rollback(task);
             return 1;
          }
        else
          {
             _e_fm_op_overwrite = 1;
             _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_OVERWRITE, "%s", task->dst.name);
          }
     }

   return 0;
}

static int
_e_fm_op_copy_dir(E_Fm_Op_Task * task)
{
   struct stat st;

   /* Directory. Just create one in destatation. */
   if (mkdir(task->dst.name,
             S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |
             S_IXOTH) == -1)
     {
        if (errno == EEXIST)
          {
             if (lstat(task->dst.name, &st) < 0)
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, 
                                        "Cannot lstat '%s': %s.", 
                                        task->dst.name);
             if (!S_ISDIR(st.st_mode))
               {
                  /* Let's try to delete the file and create a dir */
                  if (unlink(task->dst.name) == -1)
                    _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, 
                                             "Cannot unlink '%s': %s.", 
                                             task->dst.name);
                  if (mkdir(task->dst.name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
                    _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, 
                                             "Cannot make directory '%s': %s.", 
                                             task->dst.name);
               }
          }
        else
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, 
                                   "Cannot make directory '%s': %s.", 
                                   task->dst.name);
     }

   task->dst.done += task->src.st.st_size;
   _e_fm_op_update_progress(task, task->src.st.st_size, 0);

   /* Finish with this task. */
   task->finished = 1;

   return 0;
}

static int
_e_fm_op_copy_link(E_Fm_Op_Task *task)
{
   size_t len;
   char path[PATH_MAX];

   len = readlink(task->src.name, &path[0], PATH_MAX);
   path[len] = 0;

   if (symlink(path, task->dst.name) != 0)
     {
        if (errno == EEXIST)
          {
             if (unlink(task->dst.name) == -1)
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot unlink '%s': %s.", task->dst.name);
             if (symlink(path, task->dst.name) == -1)
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot create link from '%s' to '%s': %s.", path, task->dst.name);
          }
        else
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot create link from '%s' to '%s': %s.", path, task->dst.name);
     }

   task->dst.done += task->src.st.st_size;

   _e_fm_op_update_progress(task, task->src.st.st_size, 0);
   _e_fm_op_copy_stat_info(task);

   task->finished = 1;

   return 0;
}

static int
_e_fm_op_copy_fifo(E_Fm_Op_Task *task)
{
   if (mkfifo(task->dst.name, task->src.st.st_mode) == -1)
     {
        if (errno == EEXIST)
          {
             if (unlink(task->dst.name) == -1)
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot unlink '%s': %s.", task->dst.name);
             if (mkfifo(task->dst.name, task->src.st.st_mode) == -1)
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot make FIFO at '%s': %s.", task->dst.name);
          }
        else
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot make FIFO at '%s': %s.", task->dst.name);
     }

   _e_fm_op_copy_stat_info(task);

   task->dst.done += task->src.st.st_size;
   _e_fm_op_update_progress(task, task->src.st.st_size, 0);

   task->finished = 1;

   return 0;
}

static int
_e_fm_op_open_files(E_Fm_Op_Task *task)
{
   E_Fm_Op_Copy_Data *data = task->data;

   /* Ordinary file. */
   if (!data)
     {
        data = malloc(sizeof(E_Fm_Op_Copy_Data));
        task->data = data;
        data->to = NULL;
        data->from = NULL;
     }

   if (!data->from) 
     {
        data->from = fopen(task->src.name, "rb");
        if (data->from == NULL)
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot open file '%s' for reading: %s.", task->src.name);
     }

   if (!data->to)
     {
        data->to = fopen(task->dst.name, "wb");
        if (data->to == NULL)
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot open file '%s' for writing: %s.", task->dst.name);
     }

   return 0;
}

static int
_e_fm_op_copy_chunk(E_Fm_Op_Task *task)
{
   E_Fm_Op_Copy_Data *data;
   size_t dread, dwrite;
   char buf[COPYBUFSIZE];

   data = task->data;

   if (_e_fm_op_abort)
     {
        _e_fm_op_rollback(task);
        
        task->finished = 1;
        return 1;
     }

   dread = fread(buf, 1, sizeof(buf), data->from);
   if (dread <= 0)
     {
        if (!feof(data->from))
          _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot read data from '%s': %s.", task->dst.name);

        fclose(data->from);
        fclose(data->to);
        data->from = NULL;
        data->from = NULL;

        _e_fm_op_copy_stat_info(task);

        FREE(task->data);

        task->finished = 1;

        _e_fm_op_update_progress(task, 0, 0);

        return 1;
     }

   dwrite = fwrite(buf, 1, dread, data->to);

   if (dwrite < dread)
     _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot write data to '%s': %s.", task->dst.name);

   task->dst.done += dread;
   _e_fm_op_update_progress(task, dwrite, 0);

   return 0;
}

/*
 * _e_fm_op_copy_atom(), _e_fm_op_remove_atom() and _e_fm_op_scan_atom() are functions that 
 * perform very small operations.
 *
 * _e_fm_op_copy_atom(), for example, makes one of three things:
 * 1) opens files for writing and reading. This may take several calls -- until overwrite issues are not resolved.
 * 2) reads some bytes from one file and writes them to the other.
 * 3) closes both files if there is nothing more to read.
 *
 * _e_fm_op_remove_atom() removes smth.
 *
 * _e_fm_op_scan_atom() pushes new tasks for the _e_fm_op_work_idler(). One task for cp&rm and two tasks for mv.
 *
 * _e_fm_op_copy_atom() and _e_fm_op_remove_atom() are called from _e_fm_op_work_idler().
 * _e_fm_op_scan_atom() is called from _e_fm_op_scan_idler().
 *
 * These functions are called repeatedly until they put task->finished = 1. After that the task is removed from queue.
 *
 * Return value does not matter. It's there only to _E_FM_OP_ERROR_SEND macro to work correctly. (Well, it works fine, just don't want GCC to produce a warning.)
 */
static int
_e_fm_op_copy_atom(E_Fm_Op_Task * task)
{
   E_Fm_Op_Copy_Data *data;

   if (!task) return 1;

   data = task->data;

   if ((!data) || (!data->to) || (!data->from))		/* Did not touch the files yet. */
     {
        E_FM_OP_DEBUG("Copy: %s --> %s\n", task->src.name, task->dst.name);

        if (_e_fm_op_abort)
	  {
	     /* We're marked for abortion. Don't do anything. 
	      * Just return -- abort gets handled in _idler. 
	      */
	     task->finished = 1;
	     return 1;
	  }

        if (_e_fm_op_handle_overwrite(task)) return 1;

	if (S_ISDIR(task->src.st.st_mode))
	  {
             if (_e_fm_op_copy_dir(task)) return 1;
	  }
	else if (S_ISLNK(task->src.st.st_mode))
	  {
             if (_e_fm_op_copy_link(task)) return 1;
	  }
	else if (S_ISFIFO(task->src.st.st_mode))
	  {
             if (_e_fm_op_copy_fifo(task)) return 1;
	  }
	else if (S_ISREG(task->src.st.st_mode))
	  {
             if (_e_fm_op_open_files(task)) return 1;
          }
     }
   else
     {
        if (_e_fm_op_copy_chunk(task)) return 1;
     }

   return 1;
}

static int
_e_fm_op_scan_atom(E_Fm_Op_Task * task)
{
   E_Fm_Op_Task *ctask, *rtask;

   if (!task) return 1;

   task->finished = 1;

   /* Now push a new task to the work idler. */

   if (task->type == E_FM_OP_COPY)
     {
        _e_fm_op_update_progress(NULL, 0, task->src.st.st_size);

        ctask = _e_fm_op_task_new();
        ctask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(ctask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
          ctask->dst.name = eina_stringshare_add(task->dst.name);
        ctask->type = E_FM_OP_COPY;

        _e_fm_op_work_queue = eina_list_append(_e_fm_op_work_queue, ctask);
     }
   else if (task->type == E_FM_OP_COPY_STAT_INFO)
     {
        _e_fm_op_update_progress(NULL, 0, REMOVECHUNKSIZE);

        ctask = _e_fm_op_task_new();
        ctask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(ctask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
          ctask->dst.name = eina_stringshare_add(task->dst.name);
        ctask->type = E_FM_OP_COPY_STAT_INFO;

        _e_fm_op_work_queue = eina_list_append(_e_fm_op_work_queue, ctask);
     }
   else if (task->type == E_FM_OP_REMOVE)
     {
        _e_fm_op_update_progress(NULL, 0, REMOVECHUNKSIZE);

        rtask = _e_fm_op_task_new();
        rtask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(rtask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
          rtask->dst.name = eina_stringshare_add(task->dst.name);
        rtask->type = E_FM_OP_REMOVE;

        _e_fm_op_work_queue = eina_list_prepend(_e_fm_op_work_queue, rtask);
     }
   else if (task->type == E_FM_OP_MOVE)
     {
        /* Copy task. */
        _e_fm_op_update_progress(NULL, 0, task->src.st.st_size);
        ctask = _e_fm_op_task_new();

        ctask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(ctask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
          ctask->dst.name = eina_stringshare_add(task->dst.name);
        ctask->type = E_FM_OP_COPY;

        _e_fm_op_work_queue = eina_list_prepend(_e_fm_op_work_queue, ctask);

        /* Remove task. */
        _e_fm_op_update_progress(NULL, 0, REMOVECHUNKSIZE);
        rtask = _e_fm_op_task_new();

        rtask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(rtask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
          rtask->dst.name = eina_stringshare_add(task->dst.name);
        rtask->type = E_FM_OP_REMOVE;

        /* We put remove task after the separator. Work idler won't go 
         * there unless scan is done and this means that all tasks for 
         * copy are already in queue. And they will be performed before 
         * the delete tasks.
         *
         * If we don't use this separator trick, then there easily can be 
         * a situation when remove task is performed before all files are 
         * copied.
         */

        _e_fm_op_work_queue = eina_list_append_relative_list(_e_fm_op_work_queue, rtask, _e_fm_op_separator);

        ctask->link = _e_fm_op_separator->next;
     }
   else if (task->type == E_FM_OP_SYMLINK)
     {
        _e_fm_op_update_progress(NULL, 0, REMOVECHUNKSIZE);

        rtask = _e_fm_op_task_new();
        rtask->src.name = eina_stringshare_add(task->src.name);
        memcpy(&(rtask->src.st), &(task->src.st), sizeof(struct stat));
        if (task->dst.name)
	  rtask->dst.name = eina_stringshare_add(task->dst.name);
        rtask->type = E_FM_OP_SYMLINK;

        _e_fm_op_work_queue = eina_list_prepend(_e_fm_op_work_queue, rtask);
     }

   return 1;
}

static int 
_e_fm_op_copy_stat_info_atom(E_Fm_Op_Task * task)
{
   E_FM_OP_DEBUG("Stat: %s --> %s\n", task->src.name, task->dst.name);

   _e_fm_op_copy_stat_info(task);
   task->finished = 1;
   task->dst.done += REMOVECHUNKSIZE;

   _e_fm_op_update_progress(task, REMOVECHUNKSIZE, 0);

   return 0;
}

static int
_e_fm_op_symlink_atom(E_Fm_Op_Task *task)
{
   if (_e_fm_op_abort) return 1;

   E_FM_OP_DEBUG("Symlink: %s -> %s\n", task->src.name, task->dst.name);

   if (symlink(task->src.name, task->dst.name) != 0)
     _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot create link from '%s' to '%s': %s.", task->src.name, task->dst.name);

   task->dst.done += REMOVECHUNKSIZE;
   _e_fm_op_update_progress(task, REMOVECHUNKSIZE, 0);
   task->finished = 1;

   return 0;
}

static int
_e_fm_op_remove_atom(E_Fm_Op_Task * task)
{
   if (_e_fm_op_abort) return 1;

   E_FM_OP_DEBUG("Remove: %s\n", task->src.name);

   if (S_ISDIR(task->src.st.st_mode))
     {
	if (rmdir(task->src.name) == -1)
	  {
	     if (errno == ENOTEMPTY)
	       {
                  E_FM_OP_DEBUG("Attempt to remove non-empty directory.\n");
		  /* This should never happen due to way tasks are added to the work queue. If this happens (for example new files were created after the scan was complete), implicitly delete everything. */
                  ecore_file_recursive_rm(task->src.name);
                  task->finished = 1; /* Make sure that task is removed. */
		  return 1;
	       }
	     else
               _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot remove directory '%s': %s.", task->src.name);
	  }
     }
   else if (unlink(task->src.name) == -1)
     _E_FM_OP_ERROR_SEND_WORK(task, E_FM_OP_ERROR, "Cannot remove file '%s': %s.", task->src.name);

   task->dst.done += REMOVECHUNKSIZE;
   _e_fm_op_update_progress(task, REMOVECHUNKSIZE, 0);

   task->finished = 1;

   return 1;
}
