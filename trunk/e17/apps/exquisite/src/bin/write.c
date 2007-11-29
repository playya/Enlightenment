#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_Ipc.h>

#include "ipc.h"

void ipc_init(void);
void ipc_shutdown(void);

static void _help(void);

static int _ipc_cb_server_add(void *data, int type, void *event);
static int _ipc_cb_server_del(void *data, int type, void *event);
static int _ipc_cb_server_data(void *data, int type, void *event);

static Ecore_Ipc_Server *_ipc_server = NULL;

void
ipc_init(void)
{
   ecore_ipc_init();
   _ipc_server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_SYSTEM, "exquisite", 0, NULL);
   if (!_ipc_server)
     {
	_help();
	ecore_main_loop_quit();
     }
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, _ipc_cb_server_add, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, _ipc_cb_server_del, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, _ipc_cb_server_data, NULL);
}

void
ipc_shutdown(void)
{
   if (_ipc_server)
     {
	ecore_ipc_server_del(_ipc_server);
	_ipc_server = NULL;
     }
   ecore_ipc_shutdown();
}

int
main(int argc, char **argv)
{
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ipc_init();
   ecore_main_loop_begin();
   ipc_shutdown();
   ecore_shutdown();
   return 0;
}

static void
_help(void)
{
   printf("Usage:\n"
	  "  -h            This help\n"
	  "  QUIT          Tell splash to exit immediately\n"
	  "  PROGRESS N    Indicate boot progress is at N percent\n"
	  "  MSG X         Display string message X\n"
	  "  TITLE X       Diplsay title string X\n"
	  "  END           Shut down splash gracefully and exit when done\n"
	  "  TICK          Send hearbeat tick to splash\n"
	  "  PULSATE       Set exquisite into pulsate mode\n"
          "  TEXT X        Add a line of text to the text box\n"
          "  TEXT-URGENT X Add a line of text even in quiet mode\n"
          "  STATUS  X     Set a general status for the last line of text\n"
          "  SUCCESS X     Set a success status for the last line of text\n"
          "  FAILURE X     Set a failure status for the last line of text\n"
          "  CLEAR         Clear all text and hide text box\n"
          "  TIMEOUT N     Exquisite will timeout in N seconds if no commands recv'd\n"
   );
}

static int
_ipc_cb_server_add(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Add *e;
   int argc;
   char **argv;
   char *p,*q;
   int i;
   char buf[4096];
	
   e = event;
   ecore_app_args_get(&argc, &argv);
   /* parse options */
   if (argc != 2)
     {
	_help();
	ecore_main_loop_quit();
	return 0;
     }
		 
   /* Split argument string */ 
   p = buf;
   for (q = argv[1]; *q && (*q != ' ') && (p < (buf + sizeof(buf) - 1)); q++)
     {
	*p = *q;
	p++;
     }
   *p = 0;
   if (*q == ' ') q++;
   
   /* psplash compat */
   if (!strcmp(buf, "-h"))
     {
	_help();
	ecore_main_loop_quit();
	return 0;
     }
   else if (!strcmp(buf, "QUIT"))
     {
	ecore_ipc_server_send(e->server, EXIT_NOW, 0, 0, 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "PROGRESS"))
     {
	ecore_ipc_server_send(e->server, PROGRESS, 0, atoi(q), 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "MSG"))
     {
	ecore_ipc_server_send(e->server, MESSAGE, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   /* extras */
   else if (!strcmp(buf, "TITLE"))
     {
	ecore_ipc_server_send(e->server, TITLE, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "END"))
     {
	ecore_ipc_server_send(e->server, EXIT, 0, 0, 0, 0, NULL, 0);
     }
   else if (!strcmp(buf, "TICK"))
     {
	ecore_ipc_server_send(e->server, TICK, 0, 0, 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "PULSATE"))
     {
	ecore_ipc_server_send(e->server, PULSATE, 0, 0, 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "TEXT"))
     {
	ecore_ipc_server_send(e->server, TEXT, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "TEXT-URGENT"))
     {
	ecore_ipc_server_send(e->server, TEXT_URGENT, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "STATUS"))
     {
	ecore_ipc_server_send(e->server, STATUS, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "SUCCESS"))
     {
	ecore_ipc_server_send(e->server, SUCCESS, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "FAILURE"))
     {
	ecore_ipc_server_send(e->server, FAILURE, 0, 0, 0, 0, q, strlen(q) + 1);
	ecore_main_loop_quit();
     }   
   else if (!strcmp(buf, "CLEAR"))
     {
	ecore_ipc_server_send(e->server, CLEAR, 0, 0, 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else if (!strcmp(buf, "TIMEOUT"))
     {
	ecore_ipc_server_send(e->server, TIMEOUT, 0, atoi(q), 0, 0, NULL, 0);
	ecore_main_loop_quit();
     }
   else
     {
	printf("Exquisite error: Command '%s' unknown\n", buf);
	ecore_main_loop_quit();
     }
   
   ecore_ipc_server_flush(e->server);
   return 1;
}

static int
_ipc_cb_server_del(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Del *e;
   
   e = event;
   ecore_main_loop_quit();
   return 1;
}

static int
_ipc_cb_server_data(void *data, int type, void *event)
{
   Ecore_Ipc_Event_Server_Data *e;
   
   e = event;
   ecore_main_loop_quit();
   return 1;
}
