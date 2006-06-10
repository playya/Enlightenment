#include "evfs.h"

long
evfs_monitor_add(evfs_connection * conn, evfs_filereference * ref)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   /*printf("Adding a monitor on: '%s' using '%s'\n", ref->path, ref->plugin_uri); */

   command->type = EVFS_CMD_STARTMON_FILE;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *));
   command->file_command.files[0] = ref;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
   
}

long
evfs_monitor_remove(evfs_connection * conn, evfs_filereference * ref)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_STOPMON_FILE;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *));
   command->file_command.files[0] = ref;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}

long
evfs_client_file_remove(evfs_connection * conn, evfs_filereference * ref)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_REMOVE_FILE;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *));
   command->file_command.files[0] = ref;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}

long
evfs_client_file_rename(evfs_connection * conn, evfs_filereference * from,
                        evfs_filereference * to)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_RENAME_FILE;
   command->file_command.num_files = 2;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 2);
   command->file_command.files[0] = from;
   command->file_command.files[1] = to;

   evfs_write_command(conn, command);
   free(command->file_command.files);

   free(command);

   return id;
}

long
evfs_client_file_stat(evfs_connection * conn, evfs_filereference * file)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_FILE_STAT;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}

long
evfs_client_dir_list(evfs_connection * conn, evfs_filereference * file)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_LIST_DIR;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}

long
evfs_client_file_copy(evfs_connection * conn, evfs_filereference * from,
                      evfs_filereference * to)
{

   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_FILE_COPY;
   command->file_command.num_files = 2;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 2);
   command->file_command.files[0] = from;
   command->file_command.files[1] = to;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}

long
evfs_client_file_move(evfs_connection * conn, evfs_filereference * from,
                      evfs_filereference * to)
{

   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_FILE_MOVE;
   command->file_command.num_files = 2;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 2);
   command->file_command.files[0] = from;
   command->file_command.files[1] = to;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

   return id;
}


long
evfs_client_file_open(evfs_connection * conn, evfs_filereference * file)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   command->type = EVFS_CMD_FILE_OPEN;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);
 
   return id;
}

long
evfs_client_file_read(evfs_connection * conn, evfs_filereference * file,
                      int read_size)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   //printf("Reading a file..\n");

   command->type = EVFS_CMD_FILE_READ;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;
   command->file_command.extra = read_size;

   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);

 
   return id;
}


long
evfs_client_directory_create(evfs_connection * conn, evfs_filereference * file)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;

   //printf("Reading a file..\n");

   command->type = EVFS_CMD_DIRECTORY_CREATE;
   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;

   evfs_write_command(conn, command);
	
   free(command->file_command.files);
   free(command);


   return id;
}


long
evfs_client_operation_respond(evfs_connection * conn, long opid,
                              evfs_operation_response response)
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;
   
   //printf("Reading a file..\n");

   command->type = EVFS_CMD_OPERATION_RESPONSE;

   command->op = NEW(evfs_operation);
   command->op->id = opid;
   command->op->response = response;

   evfs_write_command(conn, command);

   free(command->op);
   free(command);


   return id;
}


long 
evfs_client_metadata_retrieve(evfs_connection * conn, evfs_filereference* file )
{
   evfs_command *command = evfs_client_command_new();
   long id = command->client_identifier;
   
   command->type = EVFS_CMD_METADATA_RETRIEVE;

   command->file_command.num_files = 1;
   command->file_command.files = malloc(sizeof(evfs_filereference *) * 1);
   command->file_command.files[0] = file;


   evfs_write_command(conn, command);

   free(command->file_command.files);
   free(command);


   return id;

}

