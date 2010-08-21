/* Ecore_Con MCAST Usage Example
 */

#include <Ecore.h>
#include <Ecore_Con.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

Ecore_Con_Server *svr;

typedef int (*Handler_Func) (void *data, int type, void *event);

/* 
Please note that ev->client->data points to a sockaddr_in stucture.  If you overwrite that pointer 
your program will have a memory leak
*/
int
client_data (void *data,
	     int ev_type,
	     Ecore_Con_Event_Client_Data *ev) {

  printf("Client sent data 0x%08x!", ev->client);
  printf(":  %d %s\n", ev->size, (char *)(ev->data));
  return 1;
}

Eina_Bool event_hup(void *data, int ev_type, void *ev)
{
  printf("Hup signal! Remove the mcast server.\n");
  ecore_con_server_del(svr);
  return EINA_TRUE;
}

int main (int argc, char *argv[]) {
  if (argc != 3)
    {
      fprintf(stderr, "Usage: con_mcast_example host port\n");
    }
  ecore_con_init();

  ecore_event_handler_add(ECORE_EVENT_SIGNAL_HUP, 
                          event_hup, NULL);

  svr = ecore_con_server_add(ECORE_CON_REMOTE_MCAST, argv[1], atoi(argv[2]), NULL);
  if( !svr )
    {
      printf("Unable to add server.  Bad port?  No route to the multicast network?\n");
      return 1;
    }
  else
    {
      printf("Server is running: waiting for multicast datagrams on 239.255.2.1:1199 (try nc -u 239.255.2.1 1199) \n");
    }
  
  printf("Server handle: 0x%08x\n", svr);

  ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
			  (Handler_Func)client_data, NULL);
  ecore_main_loop_begin();

  ecore_con_shutdown();
  return 0;
}
