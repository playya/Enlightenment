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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#ifdef __EMX__
#include <sys/select.h>
#include <strings.h>
#endif 

#include <efsd_debug.h>
#include <efsd_misc.h>
#include <efsd_io.h>

/* The maximum number of data chunks (ints, char*s, void*s ...)
   a command/event consists of. Major laziness.
*/
#define MAX_IOVEC  256

static int     read_data(int sockfd, void *dest, int size);
static int     read_int(int sockfd, int *dest);
static int     read_string(int sockfd, char **s);
static int     write_data(int sockfd, struct msghdr *msg);

static int     read_file_cmd(int sockfd, EfsdCommand *cmd);
static int     read_2file_cmd(int sockfd, EfsdCommand *cmd);
static int     read_chmod_cmd(int sockfd, EfsdCommand *cmd);
static int     read_set_metadata_cmd(int sockfd, EfsdCommand *cmd);
static int     read_get_metadata_cmd(int sockfd, EfsdCommand *cmd);
static int     read_filechange_event(int sockfd, EfsdEvent *ee);
static int     read_reply_event(int sockfd, EfsdEvent *ee);
static int     read_ls_getmeta_op(int sockfd, EfsdOption *eo);

static int     fill_file_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_2file_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_chmod_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_set_metadata_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_get_metadata_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_close_cmd(struct iovec *iov, EfsdCommand *ec);
static int     fill_filechange_event(struct iovec *iov, EfsdEvent *ee);
static int     fill_reply_event(struct iovec *iov, EfsdEvent *ee);
static int     fill_event(struct iovec *iov, EfsdEvent *ee);
static int     fill_command(struct iovec *iov, EfsdCommand *ec);
static int     fill_option(struct iovec *iov, EfsdOption *eo);

static int     len[256];
static int     len_index = 0;

static int 
read_data(int sockfd, void *dest, int size)
{
  int             n, result;
  fd_set          fdset;
  struct timeval  tv;
  struct msghdr   msg;
  struct iovec    iov[1];

  D_ENTER;

  if (sockfd < 0)
    D_RETURN_(-1);

  tv.tv_sec  = 1;
  tv.tv_usec = 0;
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  while ((result = select(sockfd + 1, &fdset, NULL, NULL, &tv)) < 0)
    {
      if (errno == EINTR)
	{
	  D(("read_data select() interrupted\n"));
	  tv.tv_sec  = 1;
	  tv.tv_usec = 0;
	  FD_ZERO(&fdset);
	}
      else
	{
	  fprintf(stderr, "Select error -- exiting.\n");
	  exit(-1);
	}
    }

  if (result == 0)
    {
      D(("Read timed out...\n"));
      D_RETURN_(-1);
    }

  iov[0].iov_base = dest;
  iov[0].iov_len  = size;
  bzero(&msg, sizeof(struct msghdr));
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

#ifndef __EMX__
  if ((n = recvmsg(sockfd, &msg, MSG_WAITALL)) < 0)
#else
  if ((n = recvmsg(_getsockhandle(sockfd), &msg, MSG_WAITALL)) < 0)
#endif
    {
      perror("Error");
      D_RETURN_(-1);
    }

  D_RETURN_(n);
}


static int     
read_int(int sockfd, int *dest)
{
  int count = 0;
   
  D_ENTER;

  if ((count = read_data(sockfd, dest, sizeof(int))) != sizeof(int))
    D_RETURN_(-1);

  D_RETURN_(count);
}


/* Reads a character string from the socket.
   It is assumed that the length of the string
   including the terminating 0 is sent in an
   integer before the string itself.
*/
static int     
read_string(int sockfd, char **s)
{
  int i;
  int count = 0, count2 = 0;

  D_ENTER;

  if ((count = read_data(sockfd, &i, sizeof(int))) != sizeof(int))
    D_RETURN_(-1);

  *s = (char*)malloc(sizeof(char) * i);
  if ((count2 = read_data(sockfd, *s, i)) != i)
    D_RETURN_(-1);

  D_RETURN_(count + count2);
}


static int     
write_data(int sockfd, struct msghdr *msg)
{
  int             n, result;
  fd_set          fdset;
  struct timeval  tv;

  D_ENTER;

  if (sockfd < 0)
    D_RETURN_(-1);

  tv.tv_sec  = 1;
  tv.tv_usec = 0;
  FD_ZERO(&fdset);
  FD_SET(sockfd, &fdset);

  while ((result = select(sockfd + 1, NULL, &fdset, NULL, &tv)) < 0)
    {
      if (errno == EINTR)
	{
	  D(("read_data select() interrupted\n"));
	  tv.tv_sec  = 1;
	  tv.tv_usec = 0;
	  FD_ZERO(&fdset);
	}
      else
	{
	  fprintf(stderr, "Select error -- exiting.\n");
	  exit(-1);
	}
    }

  if (result == 0)
    {
      D(("Write timed out.\n"));
      D_RETURN_(-1);
    }
#ifndef __EMX__
  if ((n = sendmsg(sockfd, msg, 0)) < 0)
#else
  if ((n = sendmsg(_getsockhandle(sockfd), msg, 0)) < 0)
#endif
    {
      perror("error");      
    }

  D_RETURN_(n);
}


static int     
read_file_cmd(int sockfd, EfsdCommand *ec)
{
  int count = 0, count2, i;
   
  D_ENTER;

  if ((count = read_int(sockfd, &(ec->efsd_file_cmd.id))) < 0)
    D_RETURN_(-1);
  count2 = count;
   
  if ((count = read_string(sockfd, &(ec->efsd_file_cmd.file))) < 0)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_int(sockfd, &(ec->efsd_file_cmd.num_options))) < 0)
    D_RETURN_(-1);
  count2 += count;

  ec->efsd_file_cmd.options = (EfsdOption*)
    malloc(sizeof(EfsdOption) *	ec->efsd_file_cmd.num_options);
  
  for (i = 0; i < ec->efsd_file_cmd.num_options; i++)
    {
      efsd_io_read_option(sockfd, &(ec->efsd_file_cmd.options[i]));
    }

  D_RETURN_(count2);
}


static int     
read_2file_cmd(int sockfd, EfsdCommand *ec)
{
  int count = 0, count2, i;
   
  D_ENTER;

  if ((count = read_int(sockfd, &(ec->efsd_2file_cmd.id))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_string(sockfd, &(ec->efsd_2file_cmd.file1))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  if ((count = read_string(sockfd, &(ec->efsd_2file_cmd.file2))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  if ((count = read_int(sockfd, &(ec->efsd_2file_cmd.num_options))) < 0)
    D_RETURN_(-1);
  count2 += count;

  ec->efsd_2file_cmd.options = (EfsdOption*)
    malloc(sizeof(EfsdOption) *	ec->efsd_2file_cmd.num_options);
  
  for (i = 0; i < ec->efsd_2file_cmd.num_options; i++)
    {
      efsd_io_read_option(sockfd, &(ec->efsd_2file_cmd.options[i]));
    }

  D_RETURN_(count2);
}


static int     
read_chmod_cmd(int sockfd, EfsdCommand *ec)
{
  int count = 0, count2;
   
  D_ENTER;

  if ((count = read_int(sockfd, &(ec->efsd_chmod_cmd.id))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_string(sockfd, &(ec->efsd_chmod_cmd.file))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  if ((count = read_data(sockfd, &(ec->efsd_chmod_cmd.mode), sizeof(mode_t))) != sizeof(mode_t))
    D_RETURN_(-1);
  count2 += count;
  
  D_RETURN_(count2);
}


static int     
read_set_metadata_cmd(int sockfd, EfsdCommand *ec)
{
  int  i;
  int count = 0, count2;

  D_ENTER;

  if ((count = read_int(sockfd, &(ec->efsd_set_metadata_cmd.id))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_data(sockfd, &(ec->efsd_set_metadata_cmd.datatype),
		sizeof(EfsdDatatype))) != sizeof(EfsdDatatype))
    D_RETURN_(-1);
  count2 += count;
  
  if ((count = read_int(sockfd, &(ec->efsd_set_metadata_cmd.data_len))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  i = ec->efsd_set_metadata_cmd.data_len;

  ec->efsd_set_metadata_cmd.data = malloc(i);
  if ((count = read_data(sockfd, ec->efsd_set_metadata_cmd.data, i)) != i)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_string(sockfd, &(ec->efsd_set_metadata_cmd.key))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  if ((count = read_string(sockfd, &(ec->efsd_set_metadata_cmd.file))) < 0)
    D_RETURN_(-1);
  count2 += count;
   
  D_RETURN_(count2);
}


static int     
read_get_metadata_cmd(int sockfd, EfsdCommand *ec)
{
  int count = 0, count2;
   
  D_ENTER;

  if ((count = read_int(sockfd, &(ec->efsd_get_metadata_cmd.id))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_data(sockfd, &(ec->efsd_get_metadata_cmd.datatype),
		sizeof(EfsdDatatype))) != sizeof(EfsdDatatype))
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_string(sockfd, &(ec->efsd_get_metadata_cmd.key))) < 0)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_string(sockfd, &(ec->efsd_get_metadata_cmd.file))) < 0)
    D_RETURN_(-1);
  count2 += count;

  D_RETURN_(count2);
}


static int     
read_filechange_event(int sockfd, EfsdEvent *ee)
{
  int count, count2;
   
  D_ENTER;

  if ((count = read_int(sockfd, &(ee->efsd_filechange_event.id))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_int(sockfd, (int*)&(ee->efsd_filechange_event.changetype))) < 0)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_string(sockfd, &(ee->efsd_filechange_event.file))) < 0)
    D_RETURN_(-1);
  count2 += count;

  D_RETURN_(count2);
}


static int     
read_reply_event(int sockfd, EfsdEvent *ee)
{
  int count = 0, count2;
   
  D_ENTER;

  if ((count = efsd_io_read_command(sockfd, &(ee->efsd_reply_event.command))) < 0)
    D_RETURN_(-1);
  count2 = count;

  if ((count = read_int(sockfd, (int*)&(ee->efsd_reply_event.status))) < 0)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_int(sockfd, &(ee->efsd_reply_event.errorcode))) < 0)
    D_RETURN_(-1);
  count2 += count;

  if ((count = read_int(sockfd, &(ee->efsd_reply_event.data_len))) < 0)
    D_RETURN_(-1);
  count2 += count;
  
  if (ee->efsd_reply_event.data_len > 0)
    {
      ee->efsd_reply_event.data = malloc(ee->efsd_reply_event.data_len);
      if ((count = read_data(sockfd, (ee->efsd_reply_event.data),
			     ee->efsd_reply_event.data_len)) < 0)
	D_RETURN_(-1);
      count2 += count;
    }
  else
    {
      ee->efsd_reply_event.data_len = 0;
      ee->efsd_reply_event.data = NULL;
    }

  D_RETURN_(count2);  
}


static int     
read_ls_getmeta_op(int sockfd, EfsdOption *eo)
{
  int count = 0, count2;
   
  D_ENTER;

  if ((count = read_string(sockfd, &(eo->efsd_op_ls_getmeta.key))) < 0)
    D_RETURN_(-1);
  count2 = count;
   
  if ((count = read_int(sockfd, (int*)&(eo->efsd_op_ls_getmeta.datatype))) < 0)
    D_RETURN_(-1);
  count2 += count;

  D_RETURN_(count2);
}


static int
fill_file_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int    i, n = 0;

  D_ENTER;

  len[len_index] = strlen(ec->efsd_file_cmd.file) + 1;

  iov[n].iov_base   = &ec->type;
  iov[n].iov_len    = sizeof(EfsdCommandType);
  iov[++n].iov_base = &ec->efsd_file_cmd.id;
  iov[n].iov_len    = sizeof(EfsdCmdId);
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_file_cmd.file;
  iov[n].iov_len    = len[len_index];
  iov[++n].iov_base = &ec->efsd_file_cmd.num_options;
  iov[n].iov_len    = sizeof(int);

  len_index++;
  n++;

  /* Fill in options, if they exist */
  if (ec->efsd_file_cmd.num_options > 0)
    {

      for (i = 0; i < ec->efsd_file_cmd.num_options; i++)
	{
	  n += fill_option(&iov[n], &(ec->efsd_file_cmd.options[i]));
	}
    }

  D_RETURN_(n);
}


static int
fill_2file_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int   i, n = 0;
  
  D_ENTER;
  
  len[len_index]   = strlen(ec->efsd_2file_cmd.file1) + 1;
  len[len_index+1] = strlen(ec->efsd_2file_cmd.file2) + 1;

  iov[n].iov_base   = &ec->type;
  iov[n].iov_len    = sizeof(EfsdCommandType);
  iov[++n].iov_base = &ec->efsd_2file_cmd.id;
  iov[n].iov_len    = sizeof(EfsdCmdId);
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_2file_cmd.file1;
  iov[n].iov_len    = len[len_index];
  iov[++n].iov_base = &len[len_index+1];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_2file_cmd.file2;
  iov[n].iov_len    = len[len_index+1];
  iov[++n].iov_base = &ec->efsd_2file_cmd.num_options;
  iov[n].iov_len    = sizeof(int);

  len_index += 2;
  n++;
      
  /* Fill in options, if they exist */
  if (ec->efsd_2file_cmd.num_options > 0)
    {
      for (i = 0; i < ec->efsd_2file_cmd.num_options; i++)
	{
	  n += fill_option(&iov[n], &(ec->efsd_2file_cmd.options[i]));
	}
    }

  D_RETURN_(n);
}


static int    
fill_chmod_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int   n = 0;

  D_ENTER;

  len[len_index] = strlen(ec->efsd_file_cmd.file) + 1;

  iov[n].iov_base   = &ec->type;
  iov[n].iov_len    = sizeof(EfsdCommandType);
  iov[++n].iov_base = &ec->efsd_file_cmd.id;
  iov[n].iov_len    = sizeof(EfsdCmdId);
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_chmod_cmd.file;
  iov[n].iov_len    = len[len_index];
  iov[++n].iov_base = &ec->efsd_chmod_cmd.mode;
  iov[n].iov_len    = sizeof(mode_t);

  len_index++;

  D_RETURN_(n+1);
}
  

static int
fill_set_metadata_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int   n = 0;

  D_ENTER;

  len[len_index]   = strlen(ec->efsd_set_metadata_cmd.key) + 1;
  len[len_index+1] = strlen(ec->efsd_set_metadata_cmd.file) + 1;
  
  iov[n].iov_base   = &ec->type;
  iov[n].iov_len    = sizeof(EfsdCommandType);
  iov[++n].iov_base = &ec->efsd_set_metadata_cmd.id;
  iov[n].iov_len    = sizeof(EfsdCmdId);
  iov[++n].iov_base = &ec->efsd_set_metadata_cmd.datatype;
  iov[n].iov_len    = sizeof(EfsdDatatype);
  iov[++n].iov_base = &ec->efsd_set_metadata_cmd.data_len;
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_set_metadata_cmd.data;
  iov[n].iov_len    = ec->efsd_set_metadata_cmd.data_len;
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_set_metadata_cmd.key;
  iov[n].iov_len    = len[len_index];
  iov[++n].iov_base = &len[len_index+1];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_set_metadata_cmd.file;
  iov[n].iov_len    = len[len_index+1];

  len_index += 2;

  D_RETURN_(n+1);
}


static int
fill_get_metadata_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int   n = 0;

  D_ENTER;

  len[len_index]   = strlen(ec->efsd_get_metadata_cmd.key) + 1;
  len[len_index+1] = strlen(ec->efsd_get_metadata_cmd.file) + 1;

  iov[n].iov_base   = &ec->type;
  iov[n].iov_len    = sizeof(EfsdCommandType);
  iov[++n].iov_base = &ec->efsd_set_metadata_cmd.id;
  iov[n].iov_len    = sizeof(EfsdCmdId);
  iov[++n].iov_base = &ec->efsd_set_metadata_cmd.datatype;
  iov[n].iov_len    = sizeof(EfsdDatatype);
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_get_metadata_cmd.key;
  iov[n].iov_len    = len[len_index];
  iov[++n].iov_base = &len[len_index+1];
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ec->efsd_get_metadata_cmd.file;
  iov[n].iov_len    = len[len_index+1];

  len_index += 2;

  D_RETURN_(n+1);
}


static int
fill_close_cmd(struct iovec *iov, EfsdCommand *ec)
{
  int  n = 0;

  D_ENTER;

  iov[n].iov_base = &ec->type;
  iov[n].iov_len  = sizeof(EfsdCommandType);

  D_RETURN_(n+1);
}


static int    
fill_filechange_event(struct iovec *iov, EfsdEvent *ee)
{
  int  n = 0;

  D_ENTER;

  len[len_index] = strlen(ee->efsd_filechange_event.file) + 1;

  iov[n].iov_base = &ee->type;
  iov[n].iov_len  = sizeof(EfsdEventType);
  iov[++n].iov_base = &ee->efsd_filechange_event.id;
  iov[n].iov_len  = sizeof(EfsdCmdId);
  iov[++n].iov_base = &ee->efsd_filechange_event.changetype;
  iov[n].iov_len  = sizeof(EfsdFilechangeType);
  iov[++n].iov_base = &len[len_index];
  iov[n].iov_len  = sizeof(int);
  iov[++n].iov_base = ee->efsd_filechange_event.file;
  iov[n].iov_len  = len[len_index];

  len_index++;

  D_RETURN_(n+1);
}


static int     
fill_reply_event(struct iovec *iov, EfsdEvent *ee)
{
  int          n = 0;

  D_ENTER;

  iov[n].iov_base = &ee->type;
  iov[n].iov_len  = sizeof(EfsdEventType);

  n++;

  n += fill_command(&iov[n], &ee->efsd_reply_event.command);

  iov[n].iov_base = &ee->efsd_reply_event.status;
  iov[n].iov_len    = sizeof(EfsdStatus);
  iov[++n].iov_base = &ee->efsd_reply_event.errorcode;
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = &ee->efsd_reply_event.data_len;
  iov[n].iov_len    = sizeof(int);
  iov[++n].iov_base = ee->efsd_reply_event.data;
  iov[n].iov_len    = ee->efsd_reply_event.data_len;

  D_RETURN_(n+1);
}



static int
fill_event(struct iovec *iov, EfsdEvent *ee)
{
  int n = 0;

  D_ENTER;

  switch (ee->type)
    {
    case EFSD_EVENT_FILECHANGE:
      n = fill_filechange_event(iov, ee);
      break;
    case EFSD_EVENT_REPLY:
      n = fill_reply_event(iov, ee);
      break;
    default:
      D(("Unknown event.\n"));
    }

  D_RETURN_(n);
}


static int
fill_command(struct iovec *iov, EfsdCommand *ec)
{
  int   n = 0;

  D_ENTER;

  /* Reset length array index */
  len_index = 0;

  switch (ec->type)
    {
    case EFSD_CMD_REMOVE:
    case EFSD_CMD_LISTDIR:
    case EFSD_CMD_MAKEDIR:
    case EFSD_CMD_STARTMON:
    case EFSD_CMD_STOPMON:
    case EFSD_CMD_STAT:
    case EFSD_CMD_READLINK:
    case EFSD_CMD_GETMIME:
      n = fill_file_cmd(iov, ec);
      break;
    case EFSD_CMD_MOVE:
    case EFSD_CMD_SYMLINK:
      n = fill_2file_cmd(iov, ec);
      break;
    case EFSD_CMD_CHMOD:
      n = fill_chmod_cmd(iov, ec);
      break;
    case EFSD_CMD_SETMETA:
      n = fill_set_metadata_cmd(iov, ec);
      break;
    case EFSD_CMD_GETMETA:
      n = fill_get_metadata_cmd(iov, ec);
      break;
    case EFSD_CMD_CLOSE:
      n = fill_close_cmd(iov, ec);
      break;
    default:
      D(("Unknown command.\n"));
    }

  D_RETURN_(n);
}


static int     
fill_option(struct iovec *iov, EfsdOption *eo)
{
  int   n = 0;

  D_ENTER;

  iov[n].iov_base   = &eo->type;
  iov[n].iov_len    = sizeof(EfsdOptionType);
  
  switch (eo->type)
    {
    case EFSD_OP_FS_FORCE:
      break;
    case EFSD_OP_FS_RECURSIVE:
      break;
    case EFSD_OP_LS_GET_STAT:
      break;
    case EFSD_OP_LS_GET_MIME:
      break;
    case EFSD_OP_LS_GET_META:
      len[len_index] = strlen(eo->efsd_op_ls_getmeta.key) + 1;
      
      iov[++n].iov_base = &len[len_index];
      iov[n].iov_len    = sizeof(int);
      iov[++n].iov_base = eo->efsd_op_ls_getmeta.key;
      iov[n].iov_len    = len[len_index];
      iov[++n].iov_base = &(eo->efsd_op_ls_getmeta.datatype);
      iov[n].iov_len    = sizeof(int);

      len_index++;
      break;
    default:
      D(("Unknown option.\n"));
    }

  D_RETURN_(n+1);
}


/* Non-static stuff below: */

int      
efsd_io_write_command(int sockfd, EfsdCommand *ec)
{
  struct iovec    iov[MAX_IOVEC];
  struct msghdr   msg;
  int             n;

  D_ENTER;

  if (!ec)
    D_RETURN_(-1);

  bzero(&msg, sizeof(struct msghdr));
  msg.msg_iov = iov;
  msg.msg_iovlen = fill_command(iov, ec);

  if ((n = write_data(sockfd, &msg)) < 0)
    {
      perror("Error writing command:");
      D_RETURN_(-1);
    }

  D_RETURN_(0);
}


int      
efsd_io_read_command(int sockfd, EfsdCommand *ec)
{
  int result = -1;
  int count = 0;

  D_ENTER;

  if (!ec)
    D_RETURN_(-1);  

  if ((count = read_int(sockfd, (int*)&(ec->type))) >= 0)
    {
      switch (ec->type)
	{
	case EFSD_CMD_REMOVE:
	case EFSD_CMD_LISTDIR:
	case EFSD_CMD_MAKEDIR:
	case EFSD_CMD_STARTMON:
	case EFSD_CMD_STOPMON:
	case EFSD_CMD_STAT:
	case EFSD_CMD_READLINK:
	case EFSD_CMD_GETMIME:
	  result = read_file_cmd(sockfd, ec);
	  break;
	case EFSD_CMD_MOVE:
	case EFSD_CMD_SYMLINK:
	  result = read_2file_cmd(sockfd, ec);
	  break;
	case EFSD_CMD_CHMOD:
	  result = read_chmod_cmd(sockfd, ec);
	  break;
	case EFSD_CMD_SETMETA:
	  result = read_set_metadata_cmd(sockfd, ec);
	  break;
	case EFSD_CMD_GETMETA:
	  result = read_get_metadata_cmd(sockfd, ec); 
	  break;
	case EFSD_CMD_CLOSE:
	  result = 0;
	  break;
	default:
	  D(("Unknown command\n"));
	}
    }

  
  D_RETURN_(result + count);
}


int      
efsd_io_write_event(int sockfd, EfsdEvent *ee)
{
  struct iovec    iov[MAX_IOVEC];
  struct msghdr   msg;
  int             n;

  D_ENTER;

  if (!ee)
    D_RETURN_(-1);

  bzero(&msg, sizeof(struct msghdr));
  msg.msg_iov = iov;
  msg.msg_iovlen = fill_event(iov, ee);

  if ((n = write_data(sockfd, &msg)) < 0)
    {
      perror("Error writing event:");
      D_RETURN_(-1);
    }

  D_RETURN_(0);
}


int      
efsd_io_read_event(int sockfd, EfsdEvent *ee)
{
  int result = -1;
  int count = 0;
   
  D_ENTER;

  if (!ee)
    D_RETURN_(-1);

  if ((count = read_int(sockfd, (int*)&(ee->type))) >= 0)
    {
      switch (ee->type)
	{
	case EFSD_EVENT_FILECHANGE:
	  result = read_filechange_event(sockfd, ee);    
	  break;
	case EFSD_EVENT_REPLY:
	  result = read_reply_event(sockfd, ee);    
	  break;
	default:
	  D(("Unknown event.\n"));
	}
    }

  D_RETURN_(count + result);
}


int      
efsd_io_write_option(int sockfd, EfsdOption *eo)
{
  struct iovec    iov[MAX_IOVEC];
  struct msghdr   msg;
  int             n;

  D_ENTER;

  if (!eo)
    D_RETURN_(-1);

  bzero(&msg, sizeof(struct msghdr));
  msg.msg_iov = iov;
  msg.msg_iovlen = fill_option(iov, eo);

  if ((n = write_data(sockfd, &msg)) < 0)
    {
      perror("Error writing option:");
      D_RETURN_(-1);
    }

  D_RETURN_(0);
}


int      
efsd_io_read_option(int sockfd, EfsdOption *eo)
{
  int result = -1;
  int count = 0;
   
  D_ENTER;

  if (!eo)
    D_RETURN_(-1);

  if ((count = read_int(sockfd, (int*)&(eo->type))) >= 0)
    {
      switch (eo->type)
	{
	case EFSD_OP_FS_FORCE:
	  break;
	case EFSD_OP_FS_RECURSIVE:
	  break;
	case EFSD_OP_LS_GET_STAT:
	  break;
	case EFSD_OP_LS_GET_MIME:
	  break;
	case EFSD_OP_LS_GET_META:
	  result = read_ls_getmeta_op(sockfd, eo);
	  break;
	default:
	  D(("Unknown option.\n"));
	}
    }

  D_RETURN_(count + result);
}

