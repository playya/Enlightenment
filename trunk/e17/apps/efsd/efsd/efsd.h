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
#ifndef efsd_h
#define efsd_h

#include <sys/types.h>
#include <sys/stat.h>
#include <fam.h>

#ifndef __EMX__
#define EFSD_PATH         "/tmp/.efsd.serv"
#else
#define EFSD_PATH         "\\socket\\tmp\\.efsd.serv"
#endif
#define EFSD_CLIENTS      100

/* Miscellaneous, needed below. */

typedef int EfsdCmdId;

typedef enum efsd_status
{
  SUCCESS,
  FAILURE
}
EfsdStatus;

typedef enum efsd_event_type
{
  FILECHANGE,
  REPLY
}
EfsdEventType;

typedef enum efsd_command_type
{
  REMOVE, MOVE, COPY, SYMLINK,
  LISTDIR, MAKEDIR, CHMOD,
  SETMETA, GETMETA, STARTMON,
  STOPMON, STAT, CLOSE
}
EfsdCommandType;

typedef enum efsd_datatype
{
  INTEGER,
  FLOAT,
  STRING,
  BINARY
}
EfsdDatatype;

typedef enum efsd_option
{
  FORCE         = (1 << 0),
  RECURSIVE     = (1 << 1)
}
EfsdOption;


/* Commands, sent from client to daemon. */

/* General datastructure for simple commands
   on a single file (rm, mkdir, ls ...)
*/
typedef struct efsd_file_cmd
{
  EfsdCommandType     type;
  EfsdCmdId           id;
  int                 options;
  char               *file;
}
EfsdFileCmd;

/* General datastructure for simple commands
   on 2 files (move, ln -s, ...)
*/
typedef struct efsd_2file_cmd
{
  EfsdCommandType     type;
  EfsdCmdId           id;
  int                 options;
  char               *file1;
  char               *file2;
}
Efsd2FileCmd;

/* For chmodding files, contains new mode too. */
typedef struct efsd_chmod_cmd
{
  EfsdCommandType     type;
  EfsdCmdId           id;
  char               *file;
  mode_t              mode;
}
EfsdChmodCmd;

/* For setting metadata. */
typedef struct efsd_set_metadata_cmd
{
  EfsdCommandType     type;
  EfsdCmdId           id;
  EfsdDatatype        datatype;
  int                 data_len;
  void               *data;
  char               *key;
  char               *file;
}
EfsdSetMetadataCmd;

/* For getting metadata. */
typedef struct efsd_get_metadata_cmd
{
  EfsdCommandType     type;
  EfsdCmdId           id;
  char               *key;
  char               *file;
}
EfsdGetMetadataCmd;

/* Empty -- just for closing a connection. */
typedef struct efsd_close_cmd
{
  EfsdCommandType     type;
}
EfsdCloseCmd;

/* X-Event-like union of commands. */
typedef union efsd_command
{
  EfsdCommandType     type;
  EfsdFileCmd         efsd_file_cmd;
  Efsd2FileCmd        efsd_2file_cmd;
  EfsdChmodCmd        efsd_chmod_cmd;
  EfsdGetMetadataCmd  efsd_get_metadata_cmd;
  EfsdSetMetadataCmd  efsd_set_metadata_cmd;
  EfsdCloseCmd        efsd_close_cmd;
}
EfsdCommand;


/* Events, sent from daemon to client. */

/* Filechange event, generated through FAM.
   XXX - this should contain the command ID as well.
*/
typedef struct efsd_filechange_event
{
  EfsdEventType       type;
  EfsdCmdId           id;
  int                 changecode;
  char               *file;
}
EfsdFileChangeEvent;


/* General reply to commands, contains
   entire command as well
*/
typedef struct efsd_reply_event
{
  EfsdEventType       type;
  EfsdCommand         command;
  EfsdStatus          status;
  int                 errorcode;
  int                 data_len;
  void               *data;
}
EfsdReplyEvent;

/* General event structure */
typedef union efsd_event
{
  EfsdEventType       type;
  EfsdFileChangeEvent efsd_filechange_event;
  EfsdReplyEvent      efsd_reply_event;
}
EfsdEvent;


#endif
