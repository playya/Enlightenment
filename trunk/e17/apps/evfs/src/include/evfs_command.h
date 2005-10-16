#ifndef __EVFS_COMMAND_H_
#define __EVFS_COMMAND_H_

#include "evfs_filereference.h"

/*Command structures*/
typedef enum evfs_command_type
{
  EVFS_CMD_STARTMON_FILE = 1 ,
  EVFS_CMD_STOPMON_FILE = 2,
  EVFS_CMD_FILE_COPY = 3,
  EVFS_CMD_MOVE_FILE = 4,
  EVFS_CMD_RENAME_FILE = 5,
  EVFS_CMD_REMOVE_FILE=6,
  EVFS_CMD_LIST_DIR = 7,
  EVFS_CMD_FILE_STAT = 8,
  EVFS_CMD_FILE_OPEN = 9,
  EVFS_CMD_FILE_TEST = 10
}
evfs_command_type;


typedef struct evfs_command_file {
	evfs_command_type type;
	int num_files;
	evfs_filereference** files;
}
evfs_command_file;

typedef union evfs_command {
	evfs_command_type type;
	evfs_command_file file_command;
}
evfs_command;

#endif
