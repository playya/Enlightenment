#include "evfs.h"

static long evfs_operation_count = 0;
static int evfs_operation_init = 0;
static Ecore_Hash *evfs_operation_hash;
static Ecore_List *evfs_operation_queue;

void
evfs_operation_initialise()
{
   if (evfs_operation_init)
      return;

   evfs_operation_init = 1;
   evfs_operation_count = 0;

   evfs_operation_hash =
      ecore_hash_new(ecore_direct_hash, ecore_direct_compare);

   evfs_operation_queue = ecore_list_new();
}

void
evfs_operation_base_init(evfs_operation* op)
{
   evfs_operation_count++;

   op->id = evfs_operation_count;
   op->status = EVFS_OPERATION_STATUS_NORMAL;

   ecore_hash_set(evfs_operation_hash, (long *)op->id, op);

   /*Init the subtask list*/
   op->sub_task = ecore_list_new();

}

evfs_operation_files* 
evfs_operation_files_new(evfs_client* client, evfs_command* command)
{
	evfs_operation_files* op = calloc(1, sizeof(evfs_operation_files));
	evfs_operation_base_init(EVFS_OPERATION(op));
	EVFS_OPERATION(op)->type = EVFS_OPERATION_TYPE_FILES;
	EVFS_OPERATION(op)->client = client;
	EVFS_OPERATION(op)->command = command;

	return op;
}

void
evfs_operation_destroy(evfs_operation * op)
{
   evfs_operation_task* task;
	
   ecore_hash_remove(evfs_operation_hash, (long *)op->id);

   if (op->sub_task) {
	   ecore_list_goto_first(op->sub_task);
	   while ( (task = ecore_list_remove_first(op->sub_task))) {
		   switch (task->type) {
			   case EVFS_OPERATION_TASK_TYPE_FILE_COPY:
				   evfs_cleanup_filereference(EVFS_OPERATION_TASK_FILE_COPY(task)->file_from);
				   evfs_cleanup_filereference(EVFS_OPERATION_TASK_FILE_COPY(task)->file_to);
				   
				   break;
			
			   case EVFS_OPERATION_TASK_TYPE_MKDIR:
				   evfs_cleanup_filereference(EVFS_OPERATION_TASK_MKDIR(task)->file);
				   break;
			   default:
				   printf("Destroying unknown task type\n");
				   break;
		   }
		   free(task);
	   }
   
	   ecore_list_destroy(op->sub_task);
   }

   if (op->command) {
	   evfs_cleanup_command(op->command, EVFS_CLEANUP_FREE_COMMAND);
   }

   free(op);
}

evfs_operation *
evfs_operation_get_by_id(long id)
{
   return ecore_hash_get(evfs_operation_hash, (long *)id);
}

void
evfs_operation_status_set(evfs_operation * op, int status)
{
   op->status = status;
}

void
evfs_operation_user_dispatch(evfs_client * client, evfs_command * command,
                             evfs_operation * op, char* misc)
{
   /*printf ("stub"); */
   evfs_operation_event_create(client, command, op,misc);
}



/*Sub task functions*/
void evfs_operation_copy_task_add(evfs_operation* op, evfs_filereference* file_from, evfs_filereference* file_to, struct stat from_stat,
		struct stat to_stat)
{
	evfs_operation_files* fop = EVFS_OPERATION_FILES(op);
	evfs_operation_task_file_copy* copy = calloc(1, sizeof(evfs_operation_task_file_copy));

	copy->file_from = file_from;
	copy->file_to = file_to;
	copy->next_byte = 0;
	memcpy(&copy->source_stat, &from_stat, sizeof(struct stat));
	memcpy(&copy->dest_stat, &to_stat, sizeof(struct stat));

	EVFS_OPERATION_TASK(copy)->status = EVFS_OPERATION_TASK_STATUS_PENDING;
	EVFS_OPERATION_TASK(copy)->type = EVFS_OPERATION_TASK_TYPE_FILE_COPY;

	fop->total_bytes += from_stat.st_size;
	fop->total_files += 1;

	ecore_list_append(op->sub_task, copy);
}


/*Sub task functions*/
void evfs_operation_mkdir_task_add(evfs_operation* op, evfs_filereference* dir)
{
	evfs_operation_files* fop = EVFS_OPERATION_FILES(op);
	evfs_operation_task_mkdir* mkdir = calloc(1, sizeof(evfs_operation_task_mkdir));

	mkdir->file = dir;

	EVFS_OPERATION_TASK(mkdir)->status = EVFS_OPERATION_TASK_STATUS_PENDING;
	EVFS_OPERATION_TASK(mkdir)->type = EVFS_OPERATION_TASK_TYPE_MKDIR;

	fop->total_bytes += 0; /*Should we bother adding the size of a dir here?*/
	fop->total_files += 1;

	ecore_list_append(op->sub_task, mkdir);
}



/*MISC/DEBUG*/

void evfs_operation_tasks_print(evfs_operation* op)
{
	evfs_operation_task* task;

	printf("Operation subtasks:\n");
	
	ecore_list_goto_first(op->sub_task);
	while ( (task = ecore_list_next(op->sub_task))) {
		switch (task->type) {
			case EVFS_OPERATION_TASK_TYPE_FILE_COPY:
				printf("COPY: %s://%s to %s://%s\n", EVFS_OPERATION_TASK_FILE_COPY(task)->file_from->plugin_uri,
								     EVFS_OPERATION_TASK_FILE_COPY(task)->file_from->path,
								     EVFS_OPERATION_TASK_FILE_COPY(task)->file_to->plugin_uri,
								     EVFS_OPERATION_TASK_FILE_COPY(task)->file_to->path);
									
				break;
			case EVFS_OPERATION_TASK_TYPE_FILE_REMOVE:
				break;

			case EVFS_OPERATION_TASK_TYPE_MKDIR:
				printf("MKDIR: %s://%s \n", EVFS_OPERATION_TASK_MKDIR(task)->file->plugin_uri,
								     EVFS_OPERATION_TASK_MKDIR(task)->file->path);
				break;

			default:
				break;
		}
	}

	if (op->type == EVFS_OPERATION_TYPE_FILES) 
		printf("Total bytes: %lld, Total files: %ld\n", EVFS_OPERATION_FILES(op)->total_bytes, EVFS_OPERATION_FILES(op)->total_files);

	printf("** DONE\n");
}

void evfs_operation_queue_run() 
{
	evfs_operation* op;
	
	ecore_list_goto_first(evfs_operation_queue);
	op = ecore_list_current(evfs_operation_queue);
	
	if (op) {
		switch (op->type) {
			case EVFS_OPERATION_TYPE_FILES:
				/*printf("Files operation to process!\n");*/
				
				evfs_operation_run_tasks(op);
				break;
			default:
				printf("Unknown operation type in run queue - %d!\n", op->type);
				break;
		}

		if (op->status == EVFS_OPERATION_STATUS_COMPLETED) {
			ecore_list_remove_first(evfs_operation_queue);
			evfs_operation_destroy(op);
			
			printf("Finished running operation, and cleaned up!\n");
		}

	} else {
		/*printf("No operations to process!\n");*/
	}
}

void evfs_operation_run_tasks(evfs_operation* op)
{
	evfs_operation_task* task = NULL;

	task = ecore_list_current(op->sub_task);
	if (task) {
		if (task->status == EVFS_OPERATION_TASK_STATUS_PENDING)
			task->status = EVFS_OPERATION_TASK_STATUS_EXEC;

		
		switch (task->type) {
			case EVFS_OPERATION_TASK_TYPE_FILE_COPY: {
				int prog = 0;
				double progress;
				double calc;
									 
				/*printf("...Processing file copy task type!\n");*/
				prog = evfs_operation_tasks_file_copy_run(op, EVFS_OPERATION_TASK_FILE_COPY(task));
				EVFS_OPERATION_FILES(op)->progress_bytes += prog;

				calc = (double)EVFS_OPERATION_FILES(op)->total_bytes / (double)EVFS_OPERATION_FILES(op)->progress_bytes;
				progress = 1.0 / calc;
				progress *= 100.0;

				evfs_file_progress_event_create(op->client,  EVFS_OPERATION_TASK_FILE_COPY(task)->file_from,
						EVFS_OPERATION_TASK_FILE_COPY(task)->file_to,
						op->command, progress, 
						EVFS_PROGRESS_TYPE_CONTINUE);


				if (task->status == EVFS_OPERATION_TASK_STATUS_COMMITTED) {
					EVFS_OPERATION_FILES(op)->progress_files += 1;
					op->processed_tasks++;
				}

				//FIXME - ther's probably a better place to put this*/
				if (op->processed_tasks == ecore_list_nodes(op->sub_task) && task->status == EVFS_OPERATION_TASK_STATUS_COMMITTED) {
					evfs_file_progress_event_create(op->client,  EVFS_OPERATION_TASK_FILE_COPY(task)->file_from,
						EVFS_OPERATION_TASK_FILE_COPY(task)->file_to,
						op->command, 100, 
						EVFS_PROGRESS_TYPE_DONE);					
				}
			}
			break;
			
			case EVFS_OPERATION_TASK_TYPE_MKDIR:
				/*printf("...Processing mkdir task type!\n");*/
				evfs_operation_tasks_mkdir_run(op, EVFS_OPERATION_TASK_MKDIR(task));

				if (task->status == EVFS_OPERATION_TASK_STATUS_COMMITTED) {
					EVFS_OPERATION_FILES(op)->progress_files += 1;
					op->processed_tasks++;
				}
				break;				
			default:
				printf("Can't process task - unknown type!\n");
				break;

		}

		if (task->status == EVFS_OPERATION_TASK_STATUS_COMMITTED) {
			ecore_list_next(op->sub_task);
		}
	} else {
		/*If task is null, operation is completed!*/
		op->status = EVFS_OPERATION_STATUS_COMPLETED;

	}
}

void evfs_operation_queue_pending_add(evfs_operation* op)
{
	ecore_list_goto_first(op->sub_task);
	ecore_list_append(evfs_operation_queue, op);
}
