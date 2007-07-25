#include "evfs.h"
#include <Eet.h>
#include <Evas.h>

static int io_init = 0;
static int _evfs_object_client_is = 1;
static Eet_Data_Descriptor *_evfs_filereference_edd;
static Eet_Data_Descriptor *_evfs_progress_event_edd;
static Eet_Data_Descriptor *_evfs_operation_edd;
static Eet_Data_Descriptor *_evfs_filemonitor_edd;
static Eet_Data_Descriptor *_evfs_metalist_edd;
static Eet_Data_Descriptor *_evfs_metaobj_edd;


/*Functions so we know what to clean in various objects client/server side*/
void evfs_object_server_is_set()
{
	_evfs_object_client_is = 0;
}

int evfs_object_client_is_get()
{
	return _evfs_object_client_is;
}
/*-----------------------*/

Eet_Data_Descriptor *
evfs_io_filereference_edd_get()
{
   return _evfs_filereference_edd;
}

int
evfs_io_initialise()
{
   if (io_init)
      return 1;

   io_init = 1;

   /*File_reference eet */
   _evfs_filereference_edd =
      eet_data_descriptor_new("evfs_filereference", sizeof(evfs_filereference),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "file_type", file_type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "path", path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "plugin_uri", plugin_uri, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "username", username, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "password", password, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "attach", attach, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filereference_edd, evfs_filereference,
                                 "fd", fd, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(_evfs_filereference_edd, evfs_filereference, "parent", parent, 
		   _evfs_filereference_edd);

   /*Progress event eet */
   _evfs_progress_event_edd =
      eet_data_descriptor_new("evfs_progress_event",
                              sizeof(evfs_event_progress),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_progress_event_edd, evfs_event_progress,
                                 "progress", file_progress, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_progress_event_edd, evfs_event_progress,
                                 "file_from", file_from, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_progress_event_edd, evfs_event_progress,
                                 "file_to", file_to, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_progress_event_edd, evfs_event_progress,
                                 "type", type, EET_T_INT);


   /*Meta obj eet*/
   _evfs_metaobj_edd =       eet_data_descriptor_new("evfs_meta_obj",
                              sizeof(evfs_meta_obj),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);
   
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_metaobj_edd, evfs_meta_obj, "key", key, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_metaobj_edd, evfs_meta_obj, "value", value, EET_T_STRING);

   /*Meta list eet*/
   _evfs_metalist_edd =       eet_data_descriptor_new("evfs_metalist",
                              sizeof(evfs_event_meta),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);
   
   EET_DATA_DESCRIPTOR_ADD_LIST(_evfs_metalist_edd, evfs_event_meta, "evfs_event_meta", meta_list,
		   	_evfs_metaobj_edd);

   /*Evfs_operation eet */
   _evfs_operation_edd =
      eet_data_descriptor_new("evfs_operation", sizeof(evfs_operation),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation, "id", id,
                                 EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation,
		                                       "misc_str", misc_str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation,
		                                       "ret_str_1", misc_str, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation,
		                                       "ret_str_2", misc_str, EET_T_STRING);
   
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation, "status",
                                 status, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation,
                                 "substatus", substatus, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_operation_edd, evfs_operation,
                                 "response", response, EET_T_INT);


   /*File monitor edd*/
   _evfs_filemonitor_edd =
      eet_data_descriptor_new("evfs_filemonitor", sizeof(evfs_event_file_monitor),
                              (void *(*)(void *))evas_list_next,
                              (void *(*)(void *, void *))evas_list_append,
                              (void *(*)(void *))evas_list_data,
                              (void *(*)(void *))evas_list_free,
                              (void (*)
                               (void *,
                                int (*)(void *, const char *, void *, void *),
                                void *))evas_hash_foreach, (void *(*)(void *,
                                                                      const char
                                                                      *,
                                                                      void *))
                              evas_hash_add, (void (*)(void *))evas_hash_free);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filemonitor_edd, evfs_event_file_monitor, "fileev_type", fileev_type,
                                 EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filemonitor_edd, evfs_event_file_monitor,
		                                       "plugin", plugin, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filemonitor_edd, evfs_event_file_monitor,
		                                       "filename", filename, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filemonitor_edd, evfs_event_file_monitor,
		                                       "filename_len", filename_len, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_evfs_filemonitor_edd, evfs_event_file_monitor,
		                                       "filetype", filetype, EET_T_INT);
   

   
   return 0;

}

ecore_ipc_message *
ecore_ipc_message_new(int major, int minor, int ref, int ref_to, int response,
                      void *data, int len)
{
   ecore_ipc_message *msg = malloc(sizeof(ecore_ipc_message));

   msg->major = major;
   msg->minor = minor;
   msg->ref = ref;
   msg->ref_to = ref_to;
   msg->response = response;
   msg->data = data;
   msg->len = len;
   msg->client = NULL;
   msg->server = NULL;
   msg->dest = 0;

   return msg;
}

void
evfs_event_client_id_notify(evfs_client * client)
{

   /*printf("Notifying client of id %ld\n", client->id); */
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_NOTIFY_ID,
                                                             0, 0, 0, 0,
                                                             &client->id,
                                                             sizeof(long)));
}

void
evfs_write_event_file_monitor(evfs_client * client, evfs_event * event)
{
   int size_ret = 0;
   char *data;

	
   /*Write "From" file */
   data = eet_data_descriptor_encode(_evfs_filemonitor_edd, &event->file_monitor, &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_FILE_MONITOR,
                                                             client->id, 0, 0,
                                                             data, size_ret));

}

void
evfs_write_stat_event(evfs_client * client, evfs_event * event)
{
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_STAT_SIZE,
                                                             client->id, 0, 0,
                                                             &event->stat.
                                                             stat_obj,
                                                             sizeof
                                                             (evfs_stat)));

}

void
evfs_write_list_event(evfs_client * client, evfs_event * event)
{
   evfs_filereference *ref;
   char *data;
   int size_ret = 0;

   ecore_list_first_goto(event->file_list.list);
   while ((ref = ecore_list_next(event->file_list.list)))
     {

        data =
           eet_data_descriptor_encode(_evfs_filereference_edd, ref, &size_ret);

        evfs_write_ecore_ipc_client_message(client->client,
                                            ecore_ipc_message_new(EVFS_EV_REPLY,
                                                                  EVFS_EV_PART_FILE_REFERENCE,
                                                                  client->id, 0,
                                                                  0, data,
                                                                  size_ret));

        free(data);

     }

}

void evfs_write_metadata_groups_event(evfs_client* client, evfs_event* event)
{
	Evas_List* l;
	evfs_metadata_group_header* g;

	for (l = event->misc.string_list; l; ) {
		g = l->data;
		
		evfs_write_ecore_ipc_client_message(client->client,
                                            ecore_ipc_message_new(EVFS_EV_REPLY,
                                                                  EVFS_EV_PART_CHAR_PTR,
                                                                  client->id, 0,
                                                                  0, g->name,
                                                                  strlen(g->name)+1));
		
		l = l->next;
	}
}

void
evfs_write_file_read_event(evfs_client * client, evfs_event * event)
{
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_DATA,
                                                             client->id, 0, 0,
                                                             event->data.bytes,
                                                             event->data.size));
}

void evfs_write_meta_event(evfs_client * client, evfs_event * event)
{
   int size_ret = 0;
   char *data;

   data =
      eet_data_descriptor_encode(_evfs_metalist_edd, event->meta,
                                 &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_METALIST,
                                                             client->id, 0, 0,
                                                             data, size_ret));

   free(data);

}

void
evfs_write_progress_event(evfs_client * client, evfs_event * event)
{
   int size_ret = 0;
   evfs_filereference *ref;
   char *data;

   data =
      eet_data_descriptor_encode(_evfs_progress_event_edd, event->progress,
                                 &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_PROGRESS,
                                                             client->id, 0, 0,
                                                             data, size_ret));

   free(data);

   /*Write "From" file */
   ref = ecore_list_first_remove(event->file_list.list);
   data = eet_data_descriptor_encode(_evfs_filereference_edd, ref, &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_FILE_REFERENCE,
                                                             client->id, 0, 0,
                                                             data, size_ret));
   free(data);

   /*Write "to" file */
   ref = ecore_list_first_remove(event->file_list.list);
   data = eet_data_descriptor_encode(_evfs_filereference_edd, ref, &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_FILE_REFERENCE,
                                                             client->id, 0, 0,
                                                             data, size_ret));
   free(data);

}

void
evfs_write_operation_event(evfs_client * client, evfs_event * event)
{
   int size_ret = 0;

   char *data =
      eet_data_descriptor_encode(_evfs_operation_edd, event->op, &size_ret);

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_OPERATION,
                                                             client->id, 0, 0,
                                                             data, size_ret));

   free(data);
}

void
evfs_write_event(evfs_client * client, evfs_command * command,
                 evfs_event * event)
{
   //printf("Sending event type '%d'\n", event->type);
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_TYPE,
                                                             client->id, 0, 0,
                                                             &event->type,
                                                             sizeof
                                                             (evfs_eventtype)));

   /*Now write the source command, if any */
   if (command)
     {
        evfs_write_command_client(client, command);
     }

   switch (event->type)
     {
     case EVFS_EV_FILE_MONITOR:
        evfs_write_event_file_monitor(client, event);
        break;
     case EVFS_EV_STAT:
        evfs_write_stat_event(client, event);
        break;
     case EVFS_EV_DIR_LIST:
        evfs_write_list_event(client, event);
        break;
     case EVFS_EV_FILE_PROGRESS:
        evfs_write_progress_event(client, event);
        break;
     case EVFS_EV_METADATA:
	evfs_write_meta_event(client,event);
	break;

     case EVFS_EV_FILE_OPEN:
        printf("Open event send\n");
        break;                  /*File open has no additional info - fd is in filereference */

     case EVFS_EV_FILE_READ:
        evfs_write_file_read_event(client, event);
        break;

     case EVFS_EV_OPERATION:
        evfs_write_operation_event(client, event);
        break;

     case EVFS_EV_METADATA_GROUPS:
	evfs_write_metadata_groups_event(client,event);
	break;

     default:
        printf("Event type not handled in switch\n");
        break;
     }

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_EV_REPLY,
                                                             EVFS_EV_PART_END,
                                                             client->id, 0, 0,
                                                             NULL, 0));

}

int
evfs_read_event(evfs_event * event, ecore_ipc_message * msg)
{

   switch (msg->minor)
     {
     case EVFS_EV_PART_TYPE:

        memcpy(&event->type, msg->data, sizeof(evfs_eventtype));
        break;
     case EVFS_EV_PART_FILE_MONITOR: {
	evfs_event_file_monitor* fmev =
		eet_data_descriptor_decode(_evfs_filemonitor_edd, msg->data,
                             msg->len); 				     
        memcpy(&event->file_monitor, fmev,
               sizeof(evfs_event_file_monitor));

	free(fmev);
     }
     break;

     case EVFS_EV_PART_CHAR_PTR:
     	event->misc.string_list = evas_list_append(event->misc.string_list, strdup(msg->data));
     break;

     case EVFS_EV_PART_STAT_SIZE:

        memcpy(&event->stat.stat_obj, msg->data, sizeof(evfs_stat));

        break;

     case EVFS_EV_PART_PROGRESS:
        {
           evfs_event_progress *pg =
              eet_data_descriptor_decode(_evfs_progress_event_edd, msg->data,
                                         msg->len);
           event->progress = pg;

        }
        break;

     case EVFS_EV_PART_OPERATION:
        {
           evfs_operation *op =
              eet_data_descriptor_decode(_evfs_operation_edd, msg->data,
                                         msg->len);
           event->op = op;

        }
        break;

     
     case EVFS_EV_PART_METALIST:
	{
		evfs_meta_obj* obj;
		Evas_List* l;
		
		evfs_event_meta* meta = 
	              eet_data_descriptor_decode(_evfs_metalist_edd, msg->data,
                          msg->len);

		/*Now we have to push this list to a hash..*/
		event->meta = meta;
		if (event->meta) {
			event->meta->meta_hash = ecore_hash_new(ecore_str_hash, ecore_str_compare);

			if (event->meta->meta_list) {
				for (l  = event->meta->meta_list; l; ) {
					obj = l->data;
				
					ecore_hash_set(event->meta->meta_hash, obj->key, obj->value);
					free(obj);

					l = l->next;
				}
				evas_list_free(event->meta->meta_list);	
			} else {
				printf("Meta list empty\n");
			}
		} else {
			printf("No meta data object!\n");
		}
	
	}
     break;

     case EVFS_EV_PART_DATA:
        {
           event->data.size = msg->len;
           event->data.bytes = malloc(msg->len);
           memcpy(event->data.bytes, msg->data, msg->len);

        }
        break;

     case EVFS_EV_PART_FILE_REFERENCE:
        {

           evfs_filereference *ref;
           ref =
              eet_data_descriptor_decode(_evfs_filereference_edd, msg->data,
                                         msg->len);

           if (!event->file_list.list)
             {
                event->file_list.list = ecore_list_new();
             }

           if (ref)
             {
                ecore_list_append(event->file_list.list, ref);
             }
           else
             {
                printf("Error decoding eet!\n");
             }
        }
        break;

        /*The pieces of the incoming command */
     case EVFS_COMMAND_TYPE:
     case EVFS_COMMAND_EXTRA:
     case EVFS_FILE_REFERENCE:
     case EVFS_COMMAND_CLIENTID:
     case EVFS_COMMAND_END:
        evfs_process_incoming_command(NULL, &event->resp_command, msg);
        break;

     case EVFS_EV_PART_END:
        //printf("Created new ecore list at %p\n", event->file_list.list);
        return TRUE;
        break;
     default:
        printf("Unknown event part received! - %d\n", msg->minor);
        break;
     }

   return FALSE;

}

/*Writers*/
void
evfs_write_ecore_ipc_server_message(Ecore_Ipc_Server * server,
                                    ecore_ipc_message * msg)
{

   ecore_ipc_server_send(server, msg->major, msg->minor, msg->ref, msg->ref_to,
                         msg->response, msg->data, msg->len);
   free(msg);

}

void
evfs_write_ecore_ipc_client_message(Ecore_Ipc_Client * client,
                                    ecore_ipc_message * msg)
{

   ecore_ipc_client_send(client, msg->major, msg->minor, msg->ref, msg->ref_to,
                         msg->response, msg->data, msg->len);
   free(msg);
}

/*------------------------------------------------------------------------*/
//Some ugly duplication here - maybe we should consider reworking this so it can
//be generic
//
void
evfs_write_operation_command(evfs_connection * conn, evfs_command * command)
{
   int size_ret = 0;

   char *data =
      eet_data_descriptor_encode(_evfs_operation_edd, command->op, &size_ret);

   evfs_write_ecore_ipc_server_message(conn->server,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_PART_OPERATION,
                                                             0, 0, 0, data,
                                                             size_ret));
   free(data);
}

void
evfs_write_command(evfs_connection * conn, evfs_command * command)
{

   /*Write the command type structure */
   evfs_write_ecore_ipc_server_message(conn->server,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_TYPE,
                                                             0, 0, 0,
                                                             &command->type,
                                                             sizeof
                                                             (evfs_command_type)));

   evfs_write_ecore_ipc_server_message(conn->server,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_EXTRA,
                                                             0, 0, 0,
                                                             &command->
                                                             file_command.extra,
                                                             sizeof(int)));

   evfs_write_ecore_ipc_server_message(conn->server,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_CLIENTID,
                                                             0, 0, 0,
                                                             &command->client_identifier,
                                                             sizeof(long)));

   switch (command->type)
     {
     case EVFS_CMD_STOPMON_FILE:
     case EVFS_CMD_STARTMON_FILE:
     case EVFS_CMD_REMOVE_FILE:
     case EVFS_CMD_RENAME_FILE:
     case EVFS_CMD_FILE_STAT:
     case EVFS_CMD_LIST_DIR:
     case EVFS_CMD_FILE_TEST:
     case EVFS_CMD_FILE_COPY:
     case EVFS_CMD_FILE_MOVE:
     case EVFS_CMD_FILE_OPEN:
     case EVFS_CMD_FILE_READ:
     case EVFS_CMD_DIRECTORY_CREATE:
     case EVFS_CMD_METADATA_RETRIEVE:
     case EVFS_CMD_METADATA_FILE_GET:
     case EVFS_CMD_METADATA_FILE_SET:
     case EVFS_CMD_PING:
     case EVFS_CMD_METADATA_GROUPS_GET:
     case EVFS_CMD_METADATA_FILE_GROUP_ADD:
     case EVFS_CMD_METADATA_FILE_GROUP_REMOVE:
     case EVFS_CMD_TRASH_RESTORE:
        evfs_write_file_command(conn, command);
        break;
     case EVFS_CMD_OPERATION_RESPONSE:
        evfs_write_operation_command(conn, command);
        break;
     default:
        printf("Command type not handled in switch\n");
        break;
     }

   /*Send a final */
   evfs_write_command_end(conn);

}

void
evfs_write_command_client(evfs_client * client, evfs_command * command)
{

      /*Write the command type structure */
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_TYPE,
                                                             client->id, 0, 0,
                                                             &command->type,
                                                             sizeof
                                                             (evfs_command_type)));

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_EXTRA,
                                                             client->id, 0, 0,
                                                             &command->
                                                             file_command.extra,
                                                             sizeof(int)));

   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                            EVFS_COMMAND_CLIENTID,
                                                            client->id, 0, 0,
                                                            &command->client_identifier,
                                                            sizeof(long)));

	
   switch (command->type)
     {
     case EVFS_CMD_STOPMON_FILE:
     case EVFS_CMD_STARTMON_FILE:
     case EVFS_CMD_REMOVE_FILE:
     case EVFS_CMD_RENAME_FILE:
     case EVFS_CMD_FILE_STAT:
     case EVFS_CMD_LIST_DIR:
     case EVFS_CMD_FILE_TEST:
     case EVFS_CMD_FILE_COPY:
     case EVFS_CMD_FILE_MOVE:
     case EVFS_CMD_FILE_OPEN:
     case EVFS_CMD_FILE_READ:
     case EVFS_CMD_DIRECTORY_CREATE:
     case EVFS_CMD_METADATA_RETRIEVE:
     case EVFS_CMD_METADATA_FILE_GET:
     case EVFS_CMD_METADATA_FILE_SET:
     case EVFS_CMD_PING:
     case EVFS_CMD_METADATA_GROUPS_GET:
     case EVFS_CMD_METADATA_FILE_GROUP_ADD:
     case EVFS_CMD_METADATA_FILE_GROUP_REMOVE:
     case EVFS_CMD_TRASH_RESTORE:
        evfs_write_file_command_client(client, command);
        break;
     default:
        printf("Command type not handled in switch (client) : %d\n", command->type);
        break;

     }

   /*Send a final */
   evfs_write_command_end_client(client);
}

void
evfs_write_command_end(evfs_connection * conn)
{
   evfs_write_ecore_ipc_server_message(conn->server,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_END,
                                                             0, 0, 0, NULL, 0));
}

void
evfs_write_command_end_client(evfs_client * client)
{
   evfs_write_ecore_ipc_client_message(client->client,
                                       ecore_ipc_message_new(EVFS_COMMAND,
                                                             EVFS_COMMAND_END,
                                                             client->id, 0, 0,
                                                             NULL, 0));
}

void
evfs_write_file_command(evfs_connection * conn, evfs_command * command)
{
   int i;

   /*Write the files */
   /*Send them de-parsed to save time */
   for (i = 0; i < command->file_command.num_files; i++)
     {
	char* data;
	int size;
        evfs_filereference *ref = command->file_command.files[i];
	data = eet_data_descriptor_encode(_evfs_filereference_edd, ref, &size);


        evfs_write_ecore_ipc_server_message(conn->server,
                                            ecore_ipc_message_new(EVFS_COMMAND,
                                                                  EVFS_FILE_REFERENCE,
                                                                  0, 0, 0, data,
                                                                  size));

	free(data);

     }

    /*If there's a string ref, write it*/
    if (command->file_command.ref) {
	  evfs_write_ecore_ipc_server_message(conn->server, 
			  		ecore_ipc_message_new(EVFS_COMMAND,
						EVFS_COMMAND_PART_FILECOMMAND_REF1,
						0,0,0,command->file_command.ref,
						strlen(command->file_command.ref)+1));
    }

    if (command->file_command.ref2) {
	  evfs_write_ecore_ipc_server_message(conn->server, 
			  		ecore_ipc_message_new(EVFS_COMMAND,
						EVFS_COMMAND_PART_FILECOMMAND_REF2,
						0,0,0,command->file_command.ref2,
						strlen(command->file_command.ref2)+1));
    }

}

void
evfs_write_file_command_client(evfs_client * client, evfs_command * command)
{
   int i;
   char uri[1024];

   bzero(uri, 1024);



  for (i = 0; i < command->file_command.num_files; i++)
     {
	char* data;
	int size;
        evfs_filereference *ref = command->file_command.files[i];

	data = eet_data_descriptor_encode(_evfs_filereference_edd, ref, &size);


        evfs_write_ecore_ipc_client_message(client->client,
                                            ecore_ipc_message_new(EVFS_COMMAND,
                                                                  EVFS_FILE_REFERENCE,
                                                                  client->id, 0,
                                                                  0, data,
                                                                  size));

	free(data);

     }

    /*If there's a string ref, write it*/
    if (command->file_command.ref) {
	  evfs_write_ecore_ipc_client_message(client->client, 
			  		ecore_ipc_message_new(EVFS_COMMAND,
						EVFS_COMMAND_PART_FILECOMMAND_REF1,
						client->id,0,0,command->file_command.ref,
						strlen(command->file_command.ref)+1));
    }

    if (command->file_command.ref2) {
	  evfs_write_ecore_ipc_client_message(client->client, 
			  		ecore_ipc_message_new(EVFS_COMMAND,
						EVFS_COMMAND_PART_FILECOMMAND_REF2,
						client->id,0,0,command->file_command.ref2,
						strlen(command->file_command.ref2)+1));
    }

}

/*----------------------------*/

/*Readers*/
int
evfs_process_incoming_command(evfs_server * server, evfs_command * command,
                              ecore_ipc_message * message)
{
   evfs_filereference *ref;

   switch (message->minor)
     {
     case EVFS_COMMAND_TYPE:
        memcpy(&command->type, message->data, sizeof(evfs_command_type));
        break;

     case EVFS_COMMAND_EXTRA:
        memcpy(&command->file_command.extra, message->data, sizeof(int));
        break;

     case EVFS_COMMAND_CLIENTID:
        memcpy(&command->client_identifier, message->data, sizeof(long));
        break;	

     case EVFS_COMMAND_PART_FILECOMMAND_REF1:
	command->file_command.ref = strdup(message->data);
	break;

     case EVFS_COMMAND_PART_FILECOMMAND_REF2:
	command->file_command.ref2 = strdup(message->data);
	break;

     case EVFS_FILE_REFERENCE:
        {
           //printf("Parsing URI: '%s'\n", message->data);                   
           //evfs_file_uri_path *path = evfs_parse_uri(message->data);
	   
           ref =
              eet_data_descriptor_decode(_evfs_filereference_edd, message->data,
                                         message->len);
	   

           if (command->file_command.num_files == 0)
             {

                /*If we have a server ref, assign this ref to the files, so they
                 * know where they came from.  We'd do this in evfs_parse_uri,
                 * but that func can also be called from the client*/
                if (server)
                  {
                     evfs_filereference* aref = ref;
                     do
                       {
                          aref->server = server;
			  aref->plugin = evfs_get_plugin_for_uri(server, aref->plugin_uri);
                       }
                     while ((aref = aref->parent));
                  }

                command->file_command.num_files = 1;
                command->file_command.files =
                   malloc(sizeof(evfs_filereference *));
                command->file_command.files[0] = ref;

             }
           else
             {

                //printf("we already have %d files\n", command->file_command.num_files);
                /*TODO Handle multiple files */

                command->file_command.files =
                   realloc(command->file_command.files,
                           sizeof(evfs_filereference *) *
                           (command->file_command.num_files + 1));
                command->file_command.files[command->file_command.num_files] = ref;
                command->file_command.num_files++;
             }

        }
        break;

     case EVFS_COMMAND_PART_OPERATION:
        {
           evfs_operation *op =
              eet_data_descriptor_decode(_evfs_operation_edd, message->data,
                                         message->len);
           command->op = op;
        }
        break;

     case EVFS_COMMAND_END:
        /*TODO cleanp ref event */

        return TRUE;
        break;

     default:
        printf("Unknown incoming command part\n");
        break;
     }

   return FALSE;
}

/*----------------------------*/
