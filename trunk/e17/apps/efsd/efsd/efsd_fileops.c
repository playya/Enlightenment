/*

Copyright (C) 2000, 2001 Christian Kreibich <kreibich@aciri.org>.

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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <efsd_debug.h>
#include <efsd_fam.h>
#include <efsd_fileops.h>
#include <efsd_io.h>
#include <efsd_misc.h>

extern FAMConnection  famcon;
extern int            clientfd[EFSD_CLIENTS];

static mode_t         default_mode = (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP |
				      S_IXGRP | S_IROTH | S_IXOTH);

int
efsd_send_reply(EfsdCommand *cmd, EfsdStatus status, int errorcode,
		int data_len, void *data, int client)
{
  EfsdEvent  ee;
  int        sockfd;

  D_ENTER;

  sockfd = clientfd[client];

  if (sockfd < 0)
    D_RETURN_(-1);

  ee.type = EFSD_EVENT_REPLY;
  ee.efsd_reply_event.command = *cmd;
  ee.efsd_reply_event.status = status;
  ee.efsd_reply_event.errorcode = errorcode;
  ee.efsd_reply_event.data_len = data_len;
  ee.efsd_reply_event.data = data;

  D_RETURN_(efsd_write_event(sockfd, &ee));
}


int 
efsd_remove(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (remove(cmd->efsd_file_cmd.file) < 0)
    {
      D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
}


int 
efsd_move(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (rename(cmd->efsd_2file_cmd.file1, cmd->efsd_2file_cmd.file2) < 0)
    {
      D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
}


int 
efsd_copy(EfsdCommand *cmd, int client)
{
  D_ENTER;

  /*
  if (rename(cmd->efsd_2file_cmd.file1, cmd->efsd_2file_cmd.file2) < 0)
    {
      D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
  */

  D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
}


int 
efsd_symlink(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (symlink(cmd->efsd_2file_cmd.file1, cmd->efsd_2file_cmd.file2) < 0)
    {
      D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
}


int 
efsd_listdir(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (efsd_misc_file_is_dir(cmd->efsd_file_cmd.file))
    {
      /* List directory by adding a FAM monitor to the
	 directory, generating a bunch of "... exists" events,
	 then removing the monitor.
      */
      efsd_fam_force_startstop_monitor(cmd, client);
      D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, FAILURE, 0, 0, NULL, client));
}


int 
efsd_makedir(EfsdCommand *cmd, int client)
{
  char **path_dirs;
  int    num_dirs, cur_dir, i;
  int    success = 1;

  D_ENTER;

  path_dirs = efsd_misc_get_path_dirs(cmd->efsd_file_cmd.file, &num_dirs);
  
  /* This is a full path, see libefsd.c */
  chdir("/");

  for (cur_dir = 0; cur_dir < num_dirs; cur_dir++)
    {
      if ( (mkdir(path_dirs[cur_dir], default_mode) < 0) &&
	   ! (errno == EEXIST && efsd_misc_file_is_dir(path_dirs[cur_dir])))
	{
	  success = 0;
	  break;
	}      
      chdir(path_dirs[cur_dir]);
    }
  
  for (i = 0; i < num_dirs; i++)
    free(path_dirs[i]);
  free(path_dirs);
  
  /* XXX this does not clean up if we had partial success ... */
  if (!success)
    D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
}


int 
efsd_chmod(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (chmod(cmd->efsd_chmod_cmd.file, cmd->efsd_chmod_cmd.mode) < 0)
    {
      D_RETURN_(efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client));
    }

  D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));
}


int 
efsd_set_metadata(EfsdCommand *cmd, int client)
{
  D_ENTER;
  printf("The setting metadata people are still out for lunch.\n");
  D_RETURN_(-1);
}


int 
efsd_get_metadata(EfsdCommand *cmd, int client)
{
  D_ENTER;
  printf("Getting metadata not here yet. Come back later.\n");
  D_RETURN_(-1);
}


int 
efsd_start_monitor(EfsdCommand *cmd, int client)
{  
  D_ENTER;

  if (efsd_fam_start_monitor(cmd, client) >= 0)
    D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));

  D_RETURN_(efsd_send_reply(cmd, FAILURE, 0, 0, NULL, client));
}


int 
efsd_stop_monitor(EfsdCommand *cmd, int client)
{
  D_ENTER;

  if (efsd_fam_stop_monitor(cmd, client) >= 0)
    D_RETURN_(efsd_send_reply(cmd, SUCCESS, 0, 0, NULL, client));

  D_RETURN_(efsd_send_reply(cmd, FAILURE, 0, 0, NULL, client));
}


int 
efsd_stat(EfsdCommand *cmd, int client)
{
  struct stat   *st;
  int            result;

  D_ENTER;

  st = (struct stat*)malloc(sizeof(struct stat));

  if (lstat(cmd->efsd_file_cmd.file, st) >= 0)
    result = efsd_send_reply(cmd, SUCCESS, 0, sizeof(struct stat), st, client);
  else
    result = efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client);

  FREE(st);

  D_RETURN_(result);
}


int  
efsd_readlink(EfsdCommand *cmd, int client)
{
  char           s[MAXPATHLEN];
  int            result, n;

  D_ENTER;

  if ((n = readlink(cmd->efsd_file_cmd.file, s, MAXPATHLEN)) >= 0)
    result = efsd_send_reply(cmd, SUCCESS, 0, n, s, client);
  else
    result = efsd_send_reply(cmd, FAILURE, errno, 0, NULL, client);

  D_RETURN_(result);
}
