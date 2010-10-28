/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *	     Stephen "okra" Houston <unixtitan@gmail.com>
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

#ifndef EIO_H__
# define EIO_H__

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EIO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EIO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Eio_Group Eio Reference API
 *
 * @brief This are the core asynchronous input/output operation
 *
 * All the function in this group do perform input/output operation
 * in a separated thread and use the infrastructure provided by
 * Ecore_Thread and Eina to work.
 *
 * @{
 */

/**
 * @typedef Eio_File_Op
 * Input/Output operations on files.
 */
typedef enum _Eio_File_Op Eio_File_Op;

/**
 * @enum _Eio_File_Op
 * Input/Output operations on files.
 */
enum _Eio_File_Op
{
  EIO_FILE_COPY, /**< IO operation is about a specific file copy */
  EIO_FILE_MOVE, /**< IO operation is about a specific file move */
  EIO_DIR_COPY, /**< IO operation is about a specific directory copy */
  EIO_DIR_MOVE, /**< IO operation is about a specific directory move */
  EIO_UNLINK, /**< IO operation is about a destroying a path (source will point to base path to be destroyed and dest to path destroyed by this IO */
  EIO_FILE_GETPWNAM, /**< IO operation is trying to get uid from user name */
  EIO_FILE_GETGRNAM /**< IO operation is trying to get gid from user name */
};

/**
 * @typedef Eio_File
 * Generic asynchronous IO reference.
 */
typedef struct _Eio_File Eio_File;

/**
 * @typedef Eio_Progress
 * Progress information on a specific operation.
 */
typedef struct _Eio_Progress Eio_Progress;

typedef Eina_Bool (*Eio_Filter_Cb)(void *data, const char *file);
typedef void (*Eio_Main_Cb)(void *data, const char *file);

typedef Eina_Bool (*Eio_Filter_Direct_Cb)(void *data, const Eina_File_Direct_Info *info);
typedef void (*Eio_Main_Direct_Cb)(void *data, const Eina_File_Direct_Info *info);

typedef void (*Eio_Stat_Cb)(void *data, const struct stat *stat);
typedef void (*Eio_Progress_Cb)(void *data, const Eio_Progress *info);

typedef void (*Eio_Done_Cb)(void *data);
typedef void (*Eio_Error_Cb)(int error, void *data);

struct _Eio_Progress
{
   Eio_File_Op op; /**< IO type */

   off_t current; /**< Current step in the IO operation */
   off_t max; /**< Number of step to do to complete this IO */
   float percent; /**< Percent done of the IO operation */

   const char *source; /**< source of the IO operation */
   const char *dest; /**< target of the IO operation */
};

EAPI int eio_init(void);
EAPI int eio_shutdown(void);

EAPI Eio_File *eio_file_ls(const char *dir,
			   Eio_Filter_Cb filter_cb,
			   Eio_Main_Cb main_cb,
			   Eio_Done_Cb done_cb,
			   Eio_Error_Cb error_cb,
			   const void *data);

EAPI Eio_File *eio_file_direct_ls(const char *dir,
				  Eio_Filter_Direct_Cb filter_cb,
				  Eio_Main_Direct_Cb main_cb,
				  Eio_Done_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);

EAPI Eio_File *eio_file_direct_stat(const char *path,
				    Eio_Stat_Cb done_cb,
				    Eio_Error_Cb error_cb,
				    const void *data);

EAPI Eio_File *eio_file_chmod(const char *path,
                              mode_t mode,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

EAPI Eio_File *eio_file_chown(const char *path,
                              const char *user,
                              const char *group,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

EAPI Eio_File *eio_file_unlink(const char *path,
			       Eio_Done_Cb done_cb,
			       Eio_Error_Cb error_cb,
			       const void *data);

EAPI Eio_File *eio_file_mkdir(const char *path,
			      mode_t mode,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI Eio_File *eio_file_move(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_file_copy(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_dir_move(const char *source,
			    const char *dest,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_copy(const char *source,
			    const char *dest,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_unlink(const char *path,
			      Eio_Progress_Cb progress_cb,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI Eina_Bool eio_file_cancel(Eio_File *ls);

/**
 * @}
 */

/**
 * @defgroup Eio_Helper Eio Reference helper API
 *
 * @brief This are helper provided around core Eio API.
 *
 * This set of functions do provide helper to work around data
 * provided by Eio without the need to look at system header.
 *
 * @{
 */

EAPI double eio_file_atime(const struct stat *stat);
EAPI double eio_file_mtime(const struct stat *stat);
EAPI long long eio_file_size(const struct stat *stat);
EAPI Eina_Bool eio_file_is_dir(const struct stat *stat);
EAPI Eina_Bool eio_file_is_lnk(const struct stat *stat);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif
