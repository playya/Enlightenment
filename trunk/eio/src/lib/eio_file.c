/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <UnixTitan@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <libgen.h>
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "eio_private.h"

#include "Eio.h"

static void
_eio_file_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Char_Ls *async = data;
   Eina_Iterator *ls;
   const char *file;

   ls = eina_file_ls(async->ls.directory);
   if (!ls)
     {
	eio_file_thread_error(&async->ls.common);
	return ;
     }

   EINA_ITERATOR_FOREACH(ls, file)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb((void*) async->ls.common.data, file);
	  }

	if (filter) ecore_thread_feedback(thread, file);
	else eina_stringshare_del(file);

	if (ecore_thread_check(thread))
	  break;
     }

   eina_iterator_free(ls);
}

static void
_eio_file_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Char_Ls *async = data;
   const char *file = msg_data;

   async->main_cb((void*) async->ls.common.data, file);

   eina_stringshare_del(file);
}

static void
_eio_file_direct_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Direct_Ls *async = data;
   Eina_Iterator *ls;
   const Eina_File_Direct_Info *info;

   ls = eina_file_direct_ls(async->ls.directory);
   if (!ls)
     {
	eio_file_thread_error(&async->ls.common);
	return ;
     }

   EINA_ITERATOR_FOREACH(ls, info)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb((void*) async->ls.common.data, info);
	  }

	if (filter)
	  {
	     Eina_File_Direct_Info *send;

	     send = malloc(sizeof (Eina_File_Direct_Info) + sizeof (struct dirent));
	     if (!send) continue;

	     memcpy(send, info, sizeof (Eina_File_Direct_Info));
             send->dirent = (struct dirent*)(send + 1);
             memcpy((void*) send->dirent, info->dirent, sizeof (struct dirent));

	     ecore_thread_feedback(thread, send);
	  }

	if (ecore_thread_check(thread))
	  break;
     }

   eina_iterator_free(ls);
}

static void
_eio_file_direct_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Direct_Ls *async = data;
   Eina_File_Direct_Info *info = msg_data;

   async->main_cb((void*) async->ls.common.data, info);

   free(info);
}

static void
_eio_file_end(void *data)
{
   Eio_File_Ls *async = data;

   async->common.done_cb((void*) async->common.data);

   eina_stringshare_del(async->directory);
   free(async);
}

static void
_eio_file_error(void *data)
{
   Eio_File_Ls *async = data;

   eio_file_error(&async->common);

   eina_stringshare_del(async->directory);
   free(async);
}

static Eina_Bool
_eio_file_write(int fd, void *mem, ssize_t length)
{
   ssize_t count;

   if (length == 0) return EINA_TRUE;

   count = write(fd, mem, length);
   if (count != length) return EINA_FALSE;
   return EINA_TRUE;
}

void
eio_progress_cb(Eio_Progress *progress, Eio_File_Progress *op)
{
   op->progress_cb((void *) op->common.data, progress);

   eio_progress_free(progress);
}

#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif

static Eina_Bool
_eio_file_copy_mmap(Ecore_Thread *thread, Eio_File_Progress *op, int in, int out, off_t size)
{
   char *m = MAP_FAILED;
   off_t i;

   for (i = 0; i < size; i += EIO_PACKET_SIZE * EIO_PACKET_COUNT)
     {
	int j;
	int k;
	int c;

	m = mmap(NULL, EIO_PACKET_SIZE * EIO_PACKET_COUNT, PROT_READ, MAP_FILE | MAP_HUGETLB | MAP_SHARED, in, i);
	if (m == MAP_FAILED)
	  goto on_error;

	madvise(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT, MADV_SEQUENTIAL);

	c = size - i;
	if (c - EIO_PACKET_SIZE * EIO_PACKET_COUNT > 0)
	  c = EIO_PACKET_SIZE * EIO_PACKET_COUNT;
	else
	  c = size - i;

	for (j = EIO_PACKET_SIZE, k = 0; j < c; k = j, j += EIO_PACKET_SIZE)
	  {
	     if (!_eio_file_write(out, m + k, EIO_PACKET_SIZE))
	       goto on_error;

	     eio_progress_send(thread, op, i + j, size);
	  }

	if (!_eio_file_write(out, m + k, c - k))
	  goto on_error;

	eio_progress_send(thread, op, i + c, size);

	munmap(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT);
	m = MAP_FAILED;

	if (ecore_thread_check(thread))
          goto on_error;
     }

   return EINA_TRUE;

 on_error:
   if (m != MAP_FAILED) munmap(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT);
   return EINA_FALSE;
}

#ifdef EFL_HAVE_SPLICE
static int
_eio_file_copy_splice(Ecore_Thread *thread, Eio_File_Progress *op, int in, int out, off_t size)
{
   int result = 0;
   off_t count;
   off_t i;
   int pipefd[2];

   if (pipe(pipefd) < 0)
     return -1;

   for (i = 0; i < size; i += count)
     {
	count = splice(in, 0, pipefd[1], NULL, EIO_PACKET_SIZE * EIO_PACKET_COUNT, SPLICE_F_MORE | SPLICE_F_MOVE);
	if (count < 0) goto on_error;

	count = splice(pipefd[0], NULL, out, 0, count, SPLICE_F_MORE | SPLICE_F_MOVE);
	if (count < 0) goto on_error;

	eio_progress_send(thread, op, i, size);

	if (ecore_thread_check(thread))
          goto on_error;
     }

   result = 1;

 on_error:
   if (result != 1 && (errno == EBADF || errno == EINVAL)) result = -1;
   close(pipefd[0]);
   close(pipefd[1]);

   return result;
}
#endif

Eina_Bool
eio_file_copy_do(Ecore_Thread *thread, Eio_File_Progress *copy)
{
   struct stat buf;
   int result = -1;
   int in = -1;
   int out = -1;

   in = open(copy->source, O_RDONLY);
   if (in < 0)
     {
	eio_file_thread_error(&copy->common);
	return EINA_FALSE;
     }

   /*
     As we need file size for progression information and both copy method
     call fstat (better than stat as it avoid race condition).
    */
   if (fstat(in, &buf) < 0)
     goto on_error;

   /* open write */
   out = open(copy->dest, O_WRONLY | O_CREAT | O_TRUNC, buf.st_mode);
   if (out < 0)
     goto on_error;

#ifdef EFL_HAVE_SPLICE
   /* fast file copy code using Linux splice API */
   result = _eio_file_copy_splice(thread, copy, in, out, buf.st_size);
   if (result == 0)
     goto on_error;
#endif

   /* classic copy method using mmap and write */
   if (result == -1 && !_eio_file_copy_mmap(thread, copy, in, out, buf.st_size))
     goto on_error;

   /* change access right to match source */
   if (fchmod(out, buf.st_mode) != 0)
     goto on_error;

   close(out);
   close(in);

   return EINA_TRUE;

 on_error:
   eio_file_thread_error(&copy->common);

   if (in >= 0) close(in);
   if (out >= 0) close(out);
   if (out >= 0)
     unlink(copy->dest);
   return EINA_FALSE;
}

static void
_eio_file_copy_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Progress *copy = data;

   eio_file_copy_do(thread, copy);
}

static void
_eio_file_copy_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Progress *copy = data;

   eio_progress_cb(msg_data, copy);
}

static void
_eio_file_copy_end(void *data)
{
   Eio_File_Progress *copy = data;

   copy->common.done_cb((void*) copy->common.data);

   eina_stringshare_del(copy->source);
   eina_stringshare_del(copy->dest);
   free(copy);
}

static void
_eio_file_copy_error(void *data)
{
   Eio_File_Progress *copy = data;

   eio_file_error(&copy->common);

   eina_stringshare_del(copy->source);
   eina_stringshare_del(copy->dest);
   free(copy);
}

static void
_eio_file_move_copy_progress(void *data, const Eio_Progress *info)
{
   Eio_File_Move *move = data;

   move->progress.progress_cb((void*) move->progress.common.data, info);
}

static void
_eio_file_move_unlink_done(void *data)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data);

   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

static void
_eio_file_move_unlink_error(int error, void *data)
{
   Eio_File_Move *move = data;

   move->copy = NULL;

   move->progress.common.error = error;
   eio_file_error(&move->progress.common);

   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

static void
_eio_file_move_copy_done(void *data)
{
   Eio_File_Move *move = data;
   Eio_File *rm;

   rm = eio_file_unlink(move->progress.source,
			_eio_file_move_unlink_done,
			_eio_file_move_unlink_error,
			move);
   if (rm) move->copy = rm;
}

static void
_eio_file_move_copy_error(int error, void *data)
{
   Eio_File_Move *move = data;

   move->progress.common.error = error;
   eio_file_error(&move->progress.common);

   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

static void
_eio_file_move_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Move *move = data;

   if (rename(move->progress.source, move->progress.dest) < 0)
     eio_file_thread_error(&move->progress.common);
   else
     eio_progress_send(thread, &move->progress, 1, 1);
}

static void
_eio_file_move_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Move *move = data;

   eio_progress_cb(msg_data, &move->progress);
}

static void
_eio_file_move_end(void *data)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data);

   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

static void
_eio_file_move_error(void *data)
{
   Eio_File_Move *move = data;

   if (move->copy)
     {
	eio_file_cancel(move->copy);
	return ;
     }

   if (move->progress.common.error == EXDEV)
     {
	Eio_File *eio_cp;

	eio_cp = eio_file_copy(move->progress.source, move->progress.dest,
			       move->progress.progress_cb ? _eio_file_move_copy_progress : NULL,
			       _eio_file_move_copy_done,
			       _eio_file_move_copy_error,
			       move);

	if (eio_cp)
          {
             move->copy = eio_cp;

             move->progress.common.thread = ((Eio_File_Progress*)move->copy)->common.thread;
             return ;
          }
     }

   eio_file_error(&move->progress.common);

   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_ls run eina_file_ls in a separated thread using ecore_thread_feedback_run. This prevent
 * any lock in your apps.
 */
EAPI Eio_File *
eio_file_ls(const char *dir,
	    Eio_Filter_Cb filter_cb,
	    Eio_Main_Cb main_cb,
	    Eio_Done_Cb done_cb,
	    Eio_Error_Cb error_cb,
	    const void *data)
{
   Eio_File_Char_Ls *async = NULL;

   if (!dir || !main_cb || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Char_Ls));
   if (!async) return NULL;

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);

   if (!eio_long_file_set(&async->ls.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_heavy,
			  _eio_file_notify,
			  _eio_file_end,
			  _eio_file_error))
     return NULL;

   return &async->ls.common;
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_ls run eina_file_direct_ls in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_direct_ls(const char *dir,
		   Eio_Filter_Direct_Cb filter_cb,
		   Eio_Main_Direct_Cb main_cb,
		   Eio_Done_Cb done_cb,
		   Eio_Error_Cb error_cb,
		   const void *data)
{
   Eio_File_Direct_Ls *async = NULL;

   if (!dir || !main_cb || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Direct_Ls));
   if (!async) return NULL;

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);

   if (!eio_long_file_set(&async->ls.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_direct_heavy,
			  _eio_file_direct_notify,
			  _eio_file_end,
			  _eio_file_error))
     return NULL;

   return &async->ls.common;
}

/**
 * @brief Cancel any Eio_File.
 * @param ls The asynchronous IO operation to cancel.
 * @return EINA_FALSE if the destruction is delayed, EINA_TRUE if it's done.
 *
 * This will cancel any kind of IO operation and cleanup the mess. This means
 * that it could take time to cancel an IO.
 */
EAPI Eina_Bool
eio_file_cancel(Eio_File *ls)
{
   return ecore_thread_cancel(ls->thread);
}

/**
 * @brief Copy a file asynchronously
 * @param source Should be the name of the file to copy the data from.
 * @param dest Should be the name of the file to copy the data to.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Private data given to callback.
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.
 */
EAPI Eio_File *
eio_file_copy(const char *source,
	      const char *dest,
	      Eio_Progress_Cb progress_cb,
	      Eio_Done_Cb done_cb,
	      Eio_Error_Cb error_cb,
	      const void *data)
{
   Eio_File_Progress *copy = NULL;

   if (!source || !dest || !done_cb || !error_cb)
     return NULL;

   copy = malloc(sizeof (Eio_File_Progress));
   if (!copy) return NULL;

   copy->op = EIO_FILE_COPY;
   copy->progress_cb = progress_cb;
   copy->source = eina_stringshare_add(source);
   copy->dest = eina_stringshare_add(dest);

   if (!eio_long_file_set(&copy->common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_copy_heavy,
			  _eio_file_copy_notify,
			  _eio_file_copy_end,
			  _eio_file_copy_error))
     return NULL;

   return &copy->common;
}

/**
 * @brief Move a file asynchronously
 * @param source Should be the name of the file to move the data from.
 * @param dest Should be the name of the file to move the data to.
 * @param progress_cb Callback called to know the progress of the move.
 * @param done_cb Callback called when the move is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Private data given to callback.
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.

 */
EAPI Eio_File *
eio_file_move(const char *source,
	      const char *dest,
	      Eio_Progress_Cb progress_cb,
	      Eio_Done_Cb done_cb,
	      Eio_Error_Cb error_cb,
	      const void *data)
{
   Eio_File_Move *move = NULL;

   if (!source || !dest || !done_cb || !error_cb)
     return NULL;

   move = malloc(sizeof (Eio_File_Move));
   if (!move) return NULL;

   move->progress.op = EIO_FILE_MOVE;
   move->progress.progress_cb = progress_cb;
   move->progress.source = eina_stringshare_add(source);
   move->progress.dest = eina_stringshare_add(dest);
   move->copy = NULL;

   if (!eio_long_file_set(&move->progress.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_move_heavy,
			  _eio_file_move_notify,
			  _eio_file_move_end,
			  _eio_file_move_error))
     return NULL;

   return &move->progress.common;
}
