/*

Copyright (C) 2000, 2001 Christian Kreibich <cK@whoop.org>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <efsd_macros.h>
#include <efsd_debug.h>
#include <efsd_misc.h>
#include <efsd_meta.h>
#include <efsd_fs.h>
#include <efsd_statcache.h>

/* Real data-copying routine, handling holes etc. Returns outcome
   upon return: 0 when successful, -1 when not.
*/
static int data_copy(char *src_path, struct stat *src_st, char *dst_path);

static int file_copy(char *src_path, struct stat *src_st, char *dst_path);

static int dir_copy(char *src_path, struct stat *src_st, char *dst_path);

static int file_move(char *src_path, struct stat *src_st, char *dst_path);

static int dir_move(char *src_path, char *dst_path);

static int file_remove(char *path, struct stat *st);


static int
data_copy(char *src_path, struct stat *src_st, char *dst_path)
{
  int  src_fd, dst_fd, block_size = 4096;
  struct stat  dst_st;
  char *buf, hole_at_end = FALSE, check_for_holes = FALSE;

  D_ENTER;

  D("Data-copying %s to %s\n", src_path, dst_path);

  if ( (src_fd = open(src_path, O_RDONLY)) < 0)
    D_RETURN_(FALSE);

  umask(000);

  D("Using src's permissions.\n");

  if ( (dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC,
		      src_st->st_mode)) < 0)
    {
      close(src_fd);
      umask(077);
      D_RETURN_(FALSE);
    }

  umask(077);

#if HAVE_ST_BLKSIZE
  if (efsd_stat(dst_path, &dst_st))
    block_size = (int)dst_st.st_blksize;
# if HAVE_ST_BLOCKS
  if (S_ISREG(src_st->st_mode) &&
      (src_st->st_size / src_st->st_blksize > src_st->st_blocks))
    check_for_holes = TRUE;
# endif
#endif

  D("Using holes: %i, buffer size %i\n", check_for_holes, block_size);

  buf = malloc(block_size);

  for ( ; ; )
    {
      int i;
      int num_read;
      char write_hole = FALSE;

      if ( (num_read = read(src_fd, buf, block_size)) < 0)
	{
	  if (errno == EINTR)
	    continue;

	  close(src_fd);
	  close(dst_fd);
	  D_RETURN_(FALSE);
	}

      if (num_read == 0) /* End of file */
	break;

      if (check_for_holes)
	{
	  write_hole = TRUE;
	  
	  for (i = 0; i < num_read; i++)
	    {
	      if (buf[i] != 0)
		{
		  write_hole = FALSE;
		  break;
		}
	    }
	}

      if (write_hole)
	{
	  if (lseek (dst_fd, (off_t)num_read, SEEK_CUR) < 0L)
	    {
	      close(src_fd);
	      close(dst_fd);
	      D_RETURN_(FALSE);
	    }

	  hole_at_end = TRUE;
	}
      else
	{
	  char *buf_ptr;
	  int   n;

	  /* Write out stuff, decreasing num_read again */

	  buf_ptr = buf;

	  while (num_read > 0)
	    {
	      if ( (n = write(dst_fd, buf_ptr, num_read)) < 0)
		{
		  if (errno == EINTR)
		    continue;
		  
		  close(src_fd);
		  close(dst_fd);
		  D_RETURN_(FALSE);
		}

	      num_read -= n;
	      buf_ptr += n;
	    }

	  hole_at_end = FALSE;
	}
    }
  
  /* Make sure things end properly at destination file */

  if (hole_at_end)
    {
      if (lseek(dst_fd, -1, SEEK_CUR) < 0L)
	{
	  D("Fixing hole at eof.\n");

	  for ( ; ; )
	    {
	      if (write(dst_fd, "", 1) < 0)
		{
		  if (errno == EINTR)
		    continue;
		  
		  close(src_fd);
		  close(dst_fd);
		  D_RETURN_(FALSE);
		}
	      
	      break;
	    }
	}
    }

  D("Copy succeeded.\n");
  
  close(src_fd);
  close(dst_fd);
  
  D_RETURN_(TRUE);
}


static int 
file_copy(char *src_path, struct stat *src_st, char *dst_path)
{
  int  success = FALSE;

  D_ENTER;

  D("Copying file %s to %s\n", src_path, dst_path);

  if (efsd_misc_file_exists(dst_path) &&
      efsd_misc_files_identical(src_path, dst_path) == TRUE)
    {
      D("Files identical, aborting.\n");
      errno = EEXIST;
      D_RETURN_(FALSE);
    }
    
  if (S_ISLNK(src_st->st_mode))
    {
      char realfile[MAXPATHLEN];
      int length;

      if ((length = readlink(src_path, realfile, MAXPATHLEN)) < 0)
	{
	  perror("Readlink error");
	  success = FALSE;
	}
      
      /* Terminate the thing, dammit. */
      realfile[length] = '\0';

      if (realfile[0] != '/')
	{
	  char realcopy[MAXPATHLEN];
	  char *lastslash;

	  snprintf(realcopy, MAXPATHLEN, src_path);
	  lastslash = strrchr(realcopy, '/');

	  if (!lastslash)
	    {
	      D("Huh? Src file %s is supposed to be a full path...\n",
		 src_path);	      
	      exit(-1);
	    }

	  snprintf(lastslash + 1, MAXPATHLEN - (lastslash - realcopy), "%s", realfile);
	  snprintf(realfile, MAXPATHLEN, realcopy);
	}

      if (symlink(realfile, dst_path) < 0)
	{
	  D("Error symlinking from %s to %s\n",
	     realfile, dst_path);
	  perror("Symlink error");
	  success = FALSE;
	}
      else
	{
	  success = TRUE;
	  D("Created symlink from %s to %s.\n",
	    realfile, dst_path);
	}
    }
  else if (S_ISFIFO(src_st->st_mode))
    {
      if (mkfifo(dst_path, src_st->st_mode) == 0)
	success = FALSE;
    }
  else if (S_ISCHR(src_st->st_mode) ||
	   S_ISBLK(src_st->st_mode) ||
	   S_ISSOCK(src_st->st_mode))
    {
      if (mknod(dst_path, src_st->st_mode, src_st->st_dev) == 0)
	success = FALSE;
    }
  else
    {
      success = data_copy(src_path, src_st, dst_path);
    }
  
  if (!success)
    {
      /* Whatever we have here as src (e.g a directory),
	 we cannot copy onto a normal file.
      */
      D("File copy error.\n");
      errno = EIO;
      D_RETURN_(FALSE);
    }

  D("File %s copied -- handling metadata.\n", src_path);
  
  efsd_meta_copy_data(src_path, dst_path);

  D("File %s finished.\n", src_path);

  /* Return final status. */
  D_RETURN_(success);
}


static int 
dir_copy(char *src_path, struct stat *src_st, char *dst_path)
{
  char           src[MAXPATHLEN];
  char           dst[MAXPATHLEN];
  char          *src_ptr, *dst_ptr;
  int            src_len, dst_len;
  struct stat    src_st2;
  struct stat    dst_st;
  DIR           *dir;
  struct dirent  de, *de_ptr;

  D_ENTER;
  D("Copying directory %s to %s\n", src_path, dst_path);

  if (!efsd_lstat(dst_path, &dst_st))
    {
      umask(000);
      
      if (mkdir(dst_path, src_st->st_mode) < 0)
	{
	  umask(077);
	  D_RETURN_(FALSE);
	}
      
      umask(077);
      D("Directory %s created.\n", dst_path);
    }
  
  /* Handle metadata for the directory itself, the metadata
     for the contained files is handled in file_copy(). */
  D("Handling metadata for directories %s and %s\n",
    src_path, dst_path);
  efsd_meta_copy_data(src_path, dst_path);

  if ( (dir = opendir(src_path)) == NULL)
    D_RETURN_(FALSE);

  snprintf(src, MAXPATHLEN, "%s/", src_path);
  snprintf(dst, MAXPATHLEN, "%s/", dst_path);
  src_len = strlen(src);
  dst_len = strlen(dst);
  src_ptr = src + src_len;
  dst_ptr = dst + dst_len;
  
  /* Read dir, using readdir_r() when we're
     threaded, readdir() otherwise ... */

  for (READDIR(dir, de, de_ptr); de_ptr; READDIR(dir, de, de_ptr))
    {
      if (!strcmp(de_ptr->d_name, ".")  ||
	  !strcmp(de_ptr->d_name, "..") ||
	  !strcmp(de_ptr->d_name, EFSD_META_DIR_NAME))
	continue;

      snprintf(src_ptr, MAXPATHLEN - src_len, "%s", de_ptr->d_name);
      snprintf(dst_ptr, MAXPATHLEN - dst_len, "%s", de_ptr->d_name);

      if (!efsd_lstat(src, &src_st2))
	continue;

      if (S_ISDIR(src_st2.st_mode))
	dir_copy(src, &src_st2, dst);
      else
	file_copy(src, &src_st2, dst);
    }

  closedir(dir);

  D_RETURN_(TRUE);
}


static int 
file_move(char *src_path, struct stat *src_st, char *dst_path)
{
  int  success = 0;

  D_ENTER;

  D("Moving file %s to %s\n", src_path, dst_path);

  /* Metadata is handled both in efsd_misc_rename()
     and file_copy().
  */

  /* Try simple rename ... */
  if (!efsd_misc_rename(src_path, dst_path))
    {
      D("Rename failed -- copying %s to %s, then removing.\n", src_path, dst_path);
      if (file_copy(src_path, src_st, dst_path))
	{
	  D("Removing source %s\n", src_path);
	  success = efsd_misc_remove(src_path);
	  D_RETURN_(success);
	}
    }
  
  D_RETURN_(TRUE);
}


static int 
dir_move(char *src_path, char *dst_path)
{
  char           src[MAXPATHLEN];
  char           dst[MAXPATHLEN];
  char          *src_ptr, *dst_ptr;
  int            src_len, dst_len;
  struct stat    src_st;
  DIR           *dir;
  struct dirent  de, *de_ptr;

  D_ENTER;
  D("Moving directory %s to %s\n", src_path, dst_path);

  if (!efsd_stat(src_path, &src_st))
    D_RETURN_(FALSE);

  umask(000);

  /* The dst directory should not exist yet. */

  if (mkdir(dst_path, src_st.st_mode) < 0)
    {
      D("Creating directory %s failed.\n", dst_path);
      umask(077);
      D_RETURN_(FALSE);
    }

  umask(077);

  D("Directory %s created.\n", dst_path);

  /* Handle metadata for the directory itself. */
  D("Handling metadata for directories %s and %s\n",
    src_path, dst_path);
  efsd_meta_move_data(src_path, dst_path);

  
  if ( (dir = opendir(src_path)) == NULL)
    D_RETURN_(FALSE);

  snprintf(src, MAXPATHLEN, "%s/", src_path);
  snprintf(dst, MAXPATHLEN, "%s/", dst_path);
  src_len = strlen(src);
  dst_len = strlen(dst);
  src_ptr = src + src_len;
  dst_ptr = dst + dst_len;
  
  /* Read dir, using readdir_r() when we're
     threaded, readdir() otherwise ... */

  for (READDIR(dir, de, de_ptr); de_ptr; READDIR(dir, de, de_ptr))
    {
      if (!strcmp(de_ptr->d_name, ".")  ||
	  !strcmp(de_ptr->d_name, "..") ||
	  !strcmp(de_ptr->d_name, EFSD_META_DIR_NAME))	  
	continue;

      snprintf(src_ptr, MAXPATHLEN - src_len, "%s", de_ptr->d_name);
      snprintf(dst_ptr, MAXPATHLEN - dst_len, "%s", de_ptr->d_name);

      if (!efsd_lstat(src, &src_st))
	{
	  D("Src %s not statable!\n", src);
	  goto error_exit;
	}
      
      if (efsd_misc_rename(src, dst))
	continue;
      
      if (S_ISDIR(src_st.st_mode))
	{
	  if (!dir_move(src, dst))
	    {
	      D("Error when moving %s to %s!\n", src, dst);
	      goto error_exit;
	    }
	}
      else if (file_move(src, &src_st, dst))
	{
	  continue;
	}
      else
	{
	  D("What is this!\n");
	  goto error_exit;
	}
    }

  /* Close dir. It's empty now, so also remove it. */

  closedir(dir);
  D_RETURN_(efsd_misc_remove(src_path));

 error_exit:

  closedir(dir);
  D_RETURN_(FALSE);  
}


static int 
file_remove(char *path, struct stat *st)
{
  char           s[MAXPATHLEN];
  char          *s_ptr;
  int            s_len;
  DIR           *dir;
  struct dirent  de, *de_ptr;
  struct stat    st2;

  D_ENTER;

  D("Removing %s\n", path);

  /* Simply try if it works. */
  if (efsd_misc_remove(path))
    D_RETURN_(TRUE);

  if (S_ISDIR(st->st_mode))
    {
      /* It's a directory. Remove everything in it recursively,
	 then try removing the empty directory.
      */

      if ( (dir = opendir(path)) == NULL)
	D_RETURN_(FALSE);
      
      snprintf(s, MAXPATHLEN, "%s/", path);
      s_len = strlen(s);
      s_ptr = s + s_len;
      
      /* Read dir, using readdir_r() when we're
	 threaded, readdir() otherwise ... */

      for (READDIR(dir, de, de_ptr); de_ptr; READDIR(dir, de, de_ptr))
	{
	  if (!strcmp(de_ptr->d_name, ".") || !strcmp(de_ptr->d_name, ".."))
	    continue;
	  
	  snprintf(s_ptr, MAXPATHLEN - s_len, "%s", de_ptr->d_name);
	  
	  if (efsd_misc_remove(s))
	    continue;

	  if (!efsd_lstat(s, &st2))
	    {
	      /* We couldn't stat it and we couldn't
		 remove it -- report error.
	      */

	      closedir(dir);
	      D_RETURN_(FALSE);
	    }
	  
	  if (S_ISDIR(st2.st_mode))
	    {
	      /* It's a directoy -- remove recursively */
	      if (!file_remove(s, &st2))
		{
		  closedir(dir);
		  D_RETURN_(FALSE);
		}
	    }
	  else
	    {
	      closedir(dir);
	      D_RETURN_(FALSE);
	    }
	}

      /* We deleted everything in the directory.
	 Report result of removing directory itself.
      */

      closedir(dir);            
      D_RETURN_(efsd_misc_remove(path));
    }

  /* It's not a directory either. Report error. */
  D_RETURN_(FALSE);
}


int 
efsd_fs_cp(int num_files, char **paths, EfsdFsOps ops)
{
  char        *dst_path = paths[num_files-1];
  char        *src_path;
  char         s[MAXPATHLEN];
  struct stat  dst_st;
  struct stat  src_st;
  char         orig_dst_stat_succeeded, dst_stat_succeeded;
  int          i;

  D_ENTER;

  if (!dst_path || dst_path[0] == '\0')
    {
      errno = EINVAL;
      D_RETURN_(FALSE);
    }

  dst_stat_succeeded = orig_dst_stat_succeeded = efsd_stat(dst_path, &dst_st);
  
  /* If we're copying multiple files to a target,
     it has to be a directory. */
  if ((num_files > 2) && orig_dst_stat_succeeded && !S_ISDIR(dst_st.st_mode))
    {
      errno = EINVAL;
      D_RETURN_(FALSE);    
    }
  
  /* Go through all files and copy to target (which
     is the last file in the char** array). */

  for (i = 0; i < num_files-1; i++)
    {     
      src_path = paths[i];
      dst_path = paths[num_files-1]; /* Yes -- need to set this every time. */

      if (!src_path || src_path[0] == '\0')
	{
	  errno = EINVAL;
	  D_RETURN_(FALSE);
	}
      
      if (efsd_misc_file_exists(dst_path) &&
	  efsd_misc_files_identical(src_path, dst_path) == TRUE)
	{
	  D("src and dst are equal in copy -- doing nothing.\n");
	  
	  if (ops & EFSD_FS_OP_FORCE)
	    continue;
	  
	  errno = EEXIST;
	  D_RETURN_(FALSE);
	}
      
      D("Copying %s to %s, rec [%s], force [%s]\n", src_path, dst_path,
	(ops & EFSD_FS_OP_RECURSIVE) ? "X" : " ",
	(ops & EFSD_FS_OP_FORCE) ? "X" : " ");
      
      if (!efsd_lstat(src_path, &src_st))
	{
	  D("Could not stat source.\n");
	  D_RETURN_(FALSE);
	}
         
      /* If dest exists and is a dir, adjust dest by
	 appending last component of file name.
      */
      
      if (orig_dst_stat_succeeded && S_ISDIR(dst_st.st_mode))
	{
	  D("Dest exists as dir.\n");
	  snprintf(s, MAXPATHLEN, "%s/%s", dst_path,
		   efsd_misc_get_filename_only(src_path));
	  dst_path = s;
	  D("Adjusted dest to: %s\n", dst_path);
	  dst_stat_succeeded = efsd_stat(dst_path, &dst_st);
	}
      
      /* Check target -- if it exists and FORCE is
	 not used, abort. Also abort if FORCE is given but
	 target cannot be removed.
      */
      
      if (dst_stat_succeeded && !S_ISDIR(dst_st.st_mode))
	{
	  if ((ops & EFSD_FS_OP_FORCE) == 0)
	    {
	      D("Dest exists and no force used -- aborting.\n");
	      errno = EEXIST;
	      D_RETURN_(FALSE);
	    }
	  
	  if (!efsd_fs_rm(dst_path, EFSD_FS_OP_RECURSIVE))
	    {
	      D("Could not remove existing dest.\n");
	      D_RETURN_(FALSE);
	    }
	  D("Existing target removed.\n");
	}
      
      /* If it's a dir and we're recursive, copy directory. */
      if (S_ISDIR(src_st.st_mode))
	{
	  if ((ops & EFSD_FS_OP_RECURSIVE) == 0)
	    {
	      D("src is directory but we're not recursive -- aborting.\n");
	      D_RETURN_(FALSE);
	    }
	  
	  if (!dir_copy(src_path, &src_st, dst_path))
	    D_RETURN_(FALSE);

	  continue;
	}

      /* Otherwise, copy single regular file. */
      if (!file_copy(src_path, &src_st, dst_path))
	D_RETURN_(FALSE);      
    }

  D_RETURN_(TRUE);
}


int 
efsd_fs_mv(int num_files, char **paths, EfsdFsOps ops)
{
  char        *dst_path = paths[num_files-1];
  char        *src_path;
  char         s[MAXPATHLEN];
  struct stat  dst_st;
  struct stat  src_st;
  char         orig_dst_stat_succeeded, dst_stat_succeeded;
  int          i;

  D_ENTER;

  if (!dst_path || dst_path[0] == '\0')
    {
      errno = EINVAL;
      D_RETURN_(FALSE);
    }

  dst_stat_succeeded = orig_dst_stat_succeeded = efsd_stat(dst_path, &dst_st);

  /* If we're moving multiple files to a target,
     it has to be a directory. */
  if ((num_files > 2) && orig_dst_stat_succeeded && !S_ISDIR(dst_st.st_mode))
    {
      errno = EINVAL;
      D_RETURN_(FALSE);
    }

  /* Go through all files and copy to target (which
     is the last file in the char** array). */

  for (i = 0; i < num_files-1; i++)
    {     
      src_path = paths[i];
      dst_path = paths[num_files-1]; /* Yes -- need to set this every time. */

      if (efsd_misc_file_exists(dst_path))
	{
	  if ((efsd_misc_files_identical(src_path, dst_path) == TRUE) ||
	      (strstr(dst_path, src_path) == dst_path))
	    {
	      D("Cannot move %s to %s -- doing nothing.\n",
		src_path, dst_path);
	      
	      if (ops & EFSD_FS_OP_FORCE)
		continue;

	      errno = EEXIST;
	      D_RETURN_(FALSE);
	    }	  
	}

      D("Moving %s to %s, force [%s]\n", src_path, dst_path,
	(ops & EFSD_FS_OP_FORCE) ? "X" : " ");
      
      if (!efsd_lstat(src_path, &src_st))
	{
	  D("Could not stat source.\n");
	  D_RETURN_(FALSE);
	}
  
      /* If dest exists and is a dir, adjust dest by
	 appending last component of file name.
      */
      
      if (orig_dst_stat_succeeded && S_ISDIR(dst_st.st_mode))
	{
	  D("Dest exists as dir.\n");
	  
	  snprintf(s, MAXPATHLEN, "%s/%s", dst_path,
		   efsd_misc_get_filename_only(src_path));
	  dst_path = s;
	  
	  D("Adjusted dest to: %s\n", dst_path);
	  
	  dst_stat_succeeded = efsd_stat(dst_path, &dst_st);
	}
  
      /* Check target -- if it exists and FORCE is
	 not used, abort. Also abort if FORCE is given but
	 target cannot be removed.
      */
  
      if (dst_stat_succeeded)
	{
	  if ((ops & EFSD_FS_OP_FORCE) == 0)
	    {
	      D("Dest exists and no force used -- aborting.\n");
	      errno = EEXIST;
	      D_RETURN_(FALSE);
	    }
	  
	  D("Dest %s exists, and force used. Removing.\n", dst_path);
	  
	  if (!efsd_fs_rm(dst_path, EFSD_FS_OP_RECURSIVE))
	    {
	      D("Could not remove existing dest.\n");
	      D_RETURN_(FALSE);
	    }
	}

      /* Try and see if simple rename() is enough. */

      D("Trying simple rename() from %s to %s ...\n",
	src_path, dst_path);

      if (!efsd_misc_rename(src_path, dst_path))
	{
	  int success;
	  
	  D("... failed.\n");
	  
	  /* If it's a dir, move directory. */
	  if (S_ISDIR(src_st.st_mode))
	    {	  
	      D("Src is dir. Moving %s to %s recursively.\n",
		src_path, dst_path);
	      success = dir_move(src_path, dst_path);
	    }
	  else
	    {
	      /* Otherwise, move single regular file. */
	      D("Src is file. Moving single file %s to %s.\n",
		src_path, dst_path);
	      success = file_move(src_path, &src_st, dst_path);
	    }

	  if (!success)
	    D_RETURN_(FALSE);
	}
    }

  /* Yay! A simple rename was enough ... */
  
  D("... succeeded.\n");
  
  D_RETURN_(TRUE);
}


int 
efsd_fs_rm(char *path, EfsdFsOps ops)
{
  int success = FALSE;
  struct stat st;

  D_ENTER;

  if (!efsd_lstat(path, &st))
    {
      /* File doesn't exist -- that's okay if we're
	 using force, otherwise it's an error.
      */

      if (ops & EFSD_FS_OP_FORCE)
	D_RETURN_(TRUE);
      
      D_RETURN_(FALSE);
    }

  /* If it's a directory and we're not
     recursive, it's an error.
  */
  if (S_ISDIR(st.st_mode) && !(ops & EFSD_FS_OP_RECURSIVE))
    {
      errno = EINVAL;
      D_RETURN_(FALSE);
    }

  /* Otherwise, try to remove and report result. */
  success = file_remove(path, &st);

  D_RETURN_(success);
}
