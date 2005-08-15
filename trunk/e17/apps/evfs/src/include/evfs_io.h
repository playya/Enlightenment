void evfs_write_command(evfs_connection* conn, evfs_command* command);
void evfs_write_file_command(evfs_connection* conn, evfs_command* command);

typedef struct ecore_ipc_message ecore_ipc_message;
struct ecore_ipc_message {

        int major;
        int minor;
        int ref;
        int ref_to;
        int response;
        void* data;
        int len;

        Ecore_Ipc_Client* client;
        Ecore_Ipc_Client* server;
        int dest; /*1 = client, 2=server*/
};


typedef enum EVFS_IO_TYPE {
	EVFS_COMMAND,
	EVFS_EVENT
} EVFS_IO_TYPE;

typedef enum EVFS_IO_PART_TYPE {
	EVFS_COMMAND_TYPE,
	EVFS_FILE_REFERENCE,
	EVFS_COMMAND_END
} EVFS_IO_PART_TYPE;

ecore_ipc_message* ecore_ipc_message_new(int major, int minor, int ref, int ref_to, int response, void* data, int len);
int evfs_process_incoming_command(evfs_command* command, ecore_ipc_message* message);
void evfs_write_command_end(evfs_connection* conn);


