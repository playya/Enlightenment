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
#include <string.h>
#include <unistd.h>

#include <efsd_debug.h>
#include <efsd_misc.h>
#include <efsd_misc.h>
#include <efsd_macros.h>
#include <efsd_options.h>
#include <efsd_types.h>

void
efsd_cmd_duplicate(EfsdCommand *ec_src, EfsdCommand *ec_dst)
{
  D_ENTER;

  if (!ec_src || !ec_dst)
    {
      D_RETURN;
    }

  /* Shallow copy: */
  *ec_dst = *ec_src;

  /* Deep copy: */
  switch (ec_src->type)
    {
    case EFSD_CMD_REMOVE:
    case EFSD_CMD_LISTDIR:
    case EFSD_CMD_MAKEDIR:
    case EFSD_CMD_CHMOD:
    case EFSD_CMD_STARTMON:
    case EFSD_CMD_STOPMON:
    case EFSD_CMD_STAT:
    case EFSD_CMD_GETFILETYPE:
    case EFSD_CMD_READLINK:
      ec_dst->efsd_file_cmd.file = strdup(ec_src->efsd_file_cmd.file);
      break;
    case EFSD_CMD_MOVE:
    case EFSD_CMD_COPY:
    case EFSD_CMD_SYMLINK:
      ec_dst->efsd_2file_cmd.file1 = strdup(ec_src->efsd_2file_cmd.file1);
      ec_dst->efsd_2file_cmd.file2 = strdup(ec_src->efsd_2file_cmd.file1);
      break;
    case EFSD_CMD_SETMETA:
      ec_dst->efsd_set_metadata_cmd.data =
	efsd_misc_memdup(ec_src->efsd_set_metadata_cmd.data, ec_src->efsd_set_metadata_cmd.data_len);
      ec_dst->efsd_set_metadata_cmd.key  = strdup(ec_src->efsd_set_metadata_cmd.key);
      ec_dst->efsd_set_metadata_cmd.file = strdup(ec_src->efsd_set_metadata_cmd.file);
      break;
    case EFSD_CMD_GETMETA:
      ec_dst->efsd_get_metadata_cmd.key  = strdup(ec_src->efsd_get_metadata_cmd.key);
      ec_dst->efsd_get_metadata_cmd.file = strdup(ec_src->efsd_get_metadata_cmd.file);
      break;
    case EFSD_CMD_CLOSE:
      break;
    default:
      D(("Warning -- unknown command type in duplicate().\n"));
    }

  D_RETURN;
}


void          
efsd_cmd_free(EfsdCommand *ec)
{
  D_ENTER;

  if (!ec)
    {
      D_RETURN;
    }

  efsd_cmd_cleanup(ec);
  FREE(ec);

  D_RETURN;
}


void
efsd_event_duplicate(EfsdEvent *ee_src, EfsdEvent *ee_dst)
{
  void      *d = NULL;

  D_ENTER;

  if (!ee_src || !ee_dst)
    {
      D_RETURN;
    }

  /* The easy part -- shallow copy. */
  *ee_dst = *ee_src;

  /* Now duplicate memory that is pointed to ... */
  switch (ee_src->type)
    {
    case EFSD_EVENT_FILECHANGE:
      ee_dst->efsd_filechange_event.file = strdup(ee_src->efsd_filechange_event.file);
      break;
    case EFSD_EVENT_REPLY:
      efsd_cmd_duplicate(&(ee_src->efsd_reply_event.command),
			 &(ee_dst->efsd_reply_event.command));
      d = malloc(sizeof(char) * ee_src->efsd_reply_event.data_len);
      memcpy(d, ee_src->efsd_reply_event.data, ee_src->efsd_reply_event.data_len);
      ee_dst->efsd_reply_event.data = d;
      break;
    default:
      D(("Warning -- unknown event type.\n"));
    }
}


void        
efsd_event_free(EfsdEvent *ee)
{
  D_ENTER;

  if (!ee)
    {
      D_RETURN;
    }

  efsd_event_cleanup(ee);
  FREE(ee);

  D_RETURN;
}


void     
efsd_cmd_cleanup(EfsdCommand *ec)
{
  D_ENTER;

  if (!ec)
    D_RETURN;

  switch (ec->type)
    {
    case EFSD_CMD_REMOVE:
    case EFSD_CMD_MAKEDIR:
    case EFSD_CMD_CHMOD:
    case EFSD_CMD_STARTMON:
    case EFSD_CMD_STOPMON:
    case EFSD_CMD_STAT:
    case EFSD_CMD_GETFILETYPE:
    case EFSD_CMD_READLINK:
      FREE(ec->efsd_file_cmd.file);
      break;
    case EFSD_CMD_LISTDIR:
      {
	int i;

	FREE(ec->efsd_file_cmd.file);
	if (ec->efsd_file_cmd.num_options > 0)
	  {
	    for (i = 0; i < ec->efsd_file_cmd.num_options; i++)
	      {
		efsd_option_cleanup(&(ec->efsd_file_cmd.options[i]));
	      }
	  }
	FREE(ec->efsd_file_cmd.options);
      }
      break;
    case EFSD_CMD_COPY:
    case EFSD_CMD_MOVE:
    case EFSD_CMD_SYMLINK:
      FREE(ec->efsd_2file_cmd.file1);
      FREE(ec->efsd_2file_cmd.file2);
      break;
    case EFSD_CMD_SETMETA:
      /* FREE(ec->efsd_set_metadata_cmd.data);*/
      FREE(ec->efsd_set_metadata_cmd.key);
      FREE(ec->efsd_set_metadata_cmd.file);
      break;
    case EFSD_CMD_GETMETA:
      FREE(ec->efsd_get_metadata_cmd.key);
      FREE(ec->efsd_get_metadata_cmd.file);
      break;
    case EFSD_CMD_CLOSE:
      break;
    default:
      D(("Warning -- unknown command type in cleanup().\n"));
    }
  D_RETURN;
}


void     
efsd_event_cleanup(EfsdEvent *ev)
{
  D_ENTER;

  if (!ev)
    D_RETURN;
  
  switch (ev->type)
    {
    case EFSD_EVENT_REPLY:
      FREE(ev->efsd_reply_event.data);
      efsd_cmd_cleanup(&ev->efsd_reply_event.command);
      break;
    case EFSD_EVENT_FILECHANGE:
      FREE(ev->efsd_filechange_event.file);
      break;
    default:
      D(("Warning -- unknown event type.\n"));
    }  

  D_RETURN;
}
